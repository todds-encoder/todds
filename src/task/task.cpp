/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/task.hpp"

#include "png2dds/png.hpp"

#include <boost/filesystem.hpp>
#include <boost/nowide/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/nowide/iostream.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <string>

namespace fs = boost::filesystem;

namespace {

bool has_png_extension(const fs::file_status& status, const fs::path& path) {
	if (!fs::is_regular_file(status)) { return false; }
	const std::string extension = path.extension().string();

	constexpr std::size_t extension_size = 4U;
	if (extension.size() != extension_size) { return false; }

	return (extension[1U] == 'p' || extension[1U] == 'P') && (extension[2U] == 'n' || extension[2U] == 'N') &&
				 (extension[3U] == 'g' || extension[3U] == 'G');
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
		const std::string& png = _png[index];
		const fs::path dds_path = fs::path{png}.replace_extension(".dds");

		if (!_arguments.overwrite && fs::exists(dds_path)) { continue; }

		boost::nowide::ifstream ifs{png, std::ios::in | std::ios::binary};
		const std::vector<std::uint8_t> buffer{std::istreambuf_iterator<char>{ifs}, {}};
		try {
			// PNG decoding.
			auto img = decode(png, buffer);
			// DDS encoding
			_encoder.encode(dds_path.string(), img);
		} catch (const std::runtime_error& ex) { boost::nowide::cerr << ex.what() << '\n'; }
	}
}

} // namespace png2dds
