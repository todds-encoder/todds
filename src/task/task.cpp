/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/task.hpp"

#include "png2dds/png.hpp"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/nowide/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/nowide/iostream.hpp>
#include <oneapi/tbb/global_control.h>
#include <oneapi/tbb/parallel_pipeline.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>

namespace fs = boost::filesystem;
namespace otbb = oneapi::tbb;
using paths_vector = std::vector<std::pair<boost::filesystem::path, boost::filesystem::path>>;

namespace {
constexpr std::size_t error_file_index = std::numeric_limits<std::size_t>::max();

bool try_add_file(const fs::path& png_path, const fs::file_status& status, paths_vector& paths, bool overwrite) {
	if (!fs::is_regular_file(status)) { return false; }
	const std::string extension = boost::to_lower_copy(png_path.extension().string());
	constexpr std::string_view png_extension{".png"};
	if (boost::to_lower_copy(png_path.extension().string()) != png_extension) { return false; }
	constexpr std::string_view dds_extension{".dds"};
	fs::path dds_path{png_path};
	dds_path.replace_extension(dds_extension.data());
	if (overwrite || !fs::exists(dds_path)) { paths.emplace_back(png_path, dds_path); }
	return true;
}

void process_directory(const fs::path& path, paths_vector& paths, bool overwrite, unsigned int depth) {
	const fs::directory_entry dir{path};
	if (!fs::exists(dir) || !fs::is_directory(dir)) { return; }

	for (fs::recursive_directory_iterator itr{dir}; itr != fs::recursive_directory_iterator{}; ++itr) {
		try_add_file(itr->path(), itr->status(), paths, overwrite);
		if (static_cast<unsigned int>(itr.depth()) >= depth) { itr.disable_recursion_pending(); }
	}
}

struct png_file {
	std::vector<std::uint8_t> buffer;
	std::size_t file_index;
};

class load_png_file final {
public:
	explicit load_png_file(const paths_vector& paths, std::atomic<std::size_t>& counter) noexcept
		: _paths{paths}
		, _counter{counter} {}

	png_file operator()(otbb::flow_control& flow) const {
		std::size_t index = _counter++;
		if (index >= _paths.size()) {
			flow.stop();
			return {};
		}
		boost::nowide::ifstream ifs{_paths[index].first, std::ios::in | std::ios::binary};
		return {{std::istreambuf_iterator<char>{ifs}, {}}, index};
	}

private:
	const paths_vector& _paths;
	std::atomic<std::size_t>& _counter;
};

class decode_png_image final {
public:
	explicit decode_png_image(const paths_vector& paths, bool flip) noexcept
		: _paths{paths}
		, _flip{flip} {}

	png2dds::image operator()(const png_file& file) const {
		try {
			return png2dds::decode(file.file_index, _paths[file.file_index].first.string(), file.buffer, _flip);
		} catch (const std::runtime_error& /*ex*/) {
			// ToDo error reporting when the verbose option is activated.
		}
		return {error_file_index, 0U, 0U};
	}

private:
	const paths_vector& _paths;
	bool _flip;
};

class encode_dds_image final {
public:
	explicit encode_dds_image(const png2dds::encoder& encoder) noexcept
		: _encoder{encoder} {}

	png2dds::dds_image operator()(const png2dds::image& png_image) const {
		return png_image.width() > 0U ? _encoder.encode(png_image) : png2dds::dds_image{png_image};
	}

private:
	const png2dds::encoder& _encoder;
};

class save_dds_file final {
public:
	explicit save_dds_file(const paths_vector& paths) noexcept
		: _paths{paths} {}

	void operator()(const png2dds::dds_image& dds_img) const {
		if (dds_img.file_index() == error_file_index) { return; }
		boost::nowide::ofstream ofs{_paths[dds_img.file_index()].second, std::ios::out | std::ios::binary};
		ofs << "DDS ";
		const std::size_t block_size_bytes = dds_img.blocks().size() * sizeof(png2dds::dds_image::block_type);
		const auto header = dds_img.header();
		ofs.write(header.data(), header.size());
		ofs.write(reinterpret_cast<const char*>(dds_img.blocks().data()), static_cast<std::ptrdiff_t>(block_size_bytes));
		ofs.close();
	}

private:
	const paths_vector& _paths;
};

} // anonymous namespace

namespace png2dds {

task::task(png2dds::args::data arguments)
	: _arguments{std::move(arguments)} {
	// Use UTF-8 as the default encoding for Boost.Filesystem.
	boost::nowide::nowide_filesystem();
}

void task::start() {
	paths_vector paths;
	if (!try_add_file(_arguments.path, fs::status(_arguments.path), paths, _arguments.overwrite)) {
		process_directory(_arguments.path, paths, _arguments.overwrite, _arguments.depth);
	}

	if (fs::exists(_arguments.list) && fs::is_regular_file(_arguments.list)) {
		boost::nowide::fstream stream{_arguments.list};
		std::string buffer;
		while (std::getline(stream, buffer)) {
			fs::path path{buffer};

			if (!try_add_file(path, fs::status(path), paths, _arguments.overwrite)) {
				process_directory(path, paths, _arguments.overwrite, _arguments.depth);
			}
		}
	}

	// Process the list in order ignoring duplicates.
	std::sort(paths.begin(), paths.end());
	paths.erase(std::unique(paths.begin(), paths.end()), paths.end());

	if (paths.empty()) { return; }

	// Variables referenced by filters.
	std::atomic<std::size_t> counter;
	png2dds::encoder encoder{_arguments.level};
	// Configure the maximum parallelism allowed for tbb.
	otbb::global_control control(otbb::global_control::max_allowed_parallelism, _arguments.threads);
	otbb::parallel_pipeline(_arguments.threads * 4UL,
		otbb::make_filter<void, png_file>(otbb::filter_mode::serial_out_of_order, load_png_file(paths, counter)) &
			otbb::make_filter<png_file, png2dds::image>(
				otbb::filter_mode::parallel, decode_png_image(paths, _arguments.flip)) &
			otbb::make_filter<png2dds::image, png2dds::dds_image>(otbb::filter_mode::parallel, encode_dds_image(encoder)) &
			otbb::make_filter<png2dds::dds_image, void>(otbb::filter_mode::serial_in_order, save_dds_file(paths)));
}

} // namespace png2dds
