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

#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>
#include <string_view>

namespace fs = boost::filesystem;

namespace {
using paths_vector = png2dds::task::paths_vector;

bool try_add_file(const fs::path& png_path, const fs::file_status& status, paths_vector& paths, bool overwrite) {
	if (!fs::is_regular_file(status)) { return false; }
	const std::string extension = boost::to_lower_copy(png_path.extension().string());
	constexpr std::string_view png_extension{".png"};
	if (boost::to_lower_copy(png_path.extension().string()) != png_extension) { return false; }
	constexpr std::string_view dds_extension{".dds"};
	fs::path dds_path{png_path};
	dds_path.replace_extension(dds_extension.data());
	if (overwrite || !fs::exists(dds_path)) { paths.emplace_back(png_path, fs::path{png_path}.replace_extension()); }
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

std::vector<std::uint8_t> load_png(const paths_vector& paths, std::size_t path_index) {
	boost::nowide::ifstream ifs{paths[path_index].first, std::ios::in | std::ios::binary};
	return std::vector<std::uint8_t>{std::istreambuf_iterator<char>{ifs}, {}};
}

void save_dds(const paths_vector& paths, std::size_t path_index, const png2dds::dds_image& dds_img) {
	// Write the DDS header, header extension and encoded data into the file.
	boost::nowide::ofstream ofs{paths[path_index].second, std::ios::out | std::ios::binary};
	ofs << "DDS ";
	const std::size_t block_size_bytes = dds_img.blocks().size() * sizeof(png2dds::dds_image::block_type);
	const auto header = dds_img.header();
	ofs.write(header.data(), header.size());
	ofs.write(reinterpret_cast<const char*>(dds_img.blocks().data()), static_cast<std::ptrdiff_t>(block_size_bytes));
	ofs.close();
}

} // anonymous namespace

namespace png2dds {

task::task(png2dds::args::data arguments)
	: _arguments{std::move(arguments)}
	, _encoder{_arguments.level}
	, _paths{} {
	// Use UTF-8 as the default encoding for Boost.Filesystem.
	boost::nowide::nowide_filesystem();
}

void task::start() {
	process_directory(_arguments.path, _paths, _arguments.overwrite, _arguments.depth);

	if (fs::exists(_arguments.list) || fs::is_regular_file(_arguments.list)) {
		boost::nowide::fstream stream{_arguments.list};
		std::string buffer;
		while (std::getline(stream, buffer)) {
			fs::path path{buffer};

			if (!try_add_file(path, fs::status(path), _paths, _arguments.overwrite)) {
				process_directory(path, _paths, _arguments.overwrite, _arguments.depth);
			}
		}
	}

	// Move duplicates to the end and ignore them.
	std::sort(_paths.begin(), _paths.end());
	const auto size = static_cast<std::size_t>(std::distance(_paths.begin(), std::unique(_paths.begin(), _paths.end())));

	// ToDo multi-threading support.
	for (std::size_t index = 0U; index < size; ++index) {
		try {
			const std::vector<std::uint8_t> buffer = load_png(_paths, index);
			const auto png_img = decode(_paths[index].first.string(), buffer);
			const auto dds_img = _encoder.encode(png_img);
			save_dds(_paths, index, dds_img);
		} catch (const std::runtime_error& ex) { boost::nowide::cerr << ex.what() << '\n'; }
	}
}

} // namespace png2dds
