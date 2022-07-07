/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/task.hpp"

#include "png2dds/png.hpp"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
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

bool has_png_extension(const fs::file_status& status, const fs::path& path) {
	if (!fs::is_regular_file(status)) { return false; }
	const std::string extension = boost::to_lower_copy(path.extension().string());
	constexpr std::string_view png_extension{".png"};
	return boost::to_lower_copy(path.extension().string()) == png_extension;
}

void process_directory(const fs::path& path, std::vector<std::string>& png, const unsigned int depth) {
	const fs::directory_entry dir{path};
	if (!fs::exists(dir) || !fs::is_directory(dir)) { return; }

	fs::recursive_directory_iterator itr{dir};
	fs::recursive_directory_iterator end{};

	for (; itr != end; ++itr) {
		if (has_png_extension(itr->status(), itr->path())) { png.emplace_back(itr->path().string()); }
		if (static_cast<unsigned int>(itr.depth()) >= depth) { itr.disable_recursion_pending(); }
	}
}

std::vector<std::uint8_t> load_png(bool overwrite, const std::string& png_path, const fs::path& dds_path) {
	if (!overwrite && fs::exists(dds_path)) { return {}; }
	boost::nowide::ifstream ifs{png_path, std::ios::in | std::ios::binary};
	return std::vector<std::uint8_t>{std::istreambuf_iterator<char>{ifs}, {}};
}

void save_dds(const png2dds::dds_image& dds_img) {
	// Write the DDS header, header extension and encoded data into the file.
	boost::nowide::ofstream ofs{dds_img.name(), std::ios::out | std::ios::binary};
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
	, _png{} {
	// Use UTF-8 as the default encoding for Boost.Filesystem.
	boost::nowide::nowide_filesystem();
}

void task::start() {
	process_directory(_arguments.path, _png, _arguments.depth);

	if (fs::exists(_arguments.list) || fs::is_regular_file(_arguments.list)) {
		boost::nowide::fstream stream{_arguments.list};
		std::string buffer;
		while (std::getline(stream, buffer)) {
			const fs::path path{buffer};
			const fs::file_status status = fs::status(path);
			if (has_png_extension(status, path)) {
				_png.emplace_back(path.string());
			} else {
				process_directory(path, _png, _arguments.depth);
			}
		}
	}

	// Move duplicates to the end and ignore them.
	std::sort(_png.begin(), _png.end());
	const auto size = static_cast<std::size_t>(std::distance(_png.begin(), std::unique(_png.begin(), _png.end())));

	// ToDo multi-threading support.
	for (std::size_t index = 0U; index < size; ++index) {
		// Load PNG file into a buffer.
		const std::string& png_path = _png[index];
		const fs::path dds_path = fs::path{png_path}.replace_extension(".dds");

		try {
			const std::vector<std::uint8_t> buffer = load_png(_arguments.overwrite, png_path, dds_path);
			if (buffer.empty()) { continue; }
			const auto png_img = decode(png_path, buffer);
			const auto dds_img = _encoder.encode(dds_path.string(), png_img);
			save_dds(dds_img);
		} catch (const std::runtime_error& ex) { boost::nowide::cerr << ex.what() << '\n'; }
	}
}

} // namespace png2dds
