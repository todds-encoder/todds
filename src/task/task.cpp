/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/task.hpp"

#include <boost/filesystem.hpp>
#include <boost/nowide/filesystem.hpp>
#include <boost/nowide/iostream.hpp>

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

void process_directory(std::string_view directory, std::vector<std::string>& png) {
	const fs::path path{directory.data()};
	const fs::directory_entry dir{path};
	if (!fs::exists(dir) || !fs::is_directory(dir)) { return; }

	fs::recursive_directory_iterator itr{dir};
	fs::recursive_directory_iterator end{};

	for (; itr != end; ++itr) {
		if (has_png_extension(itr->status(), itr->path())) { png.emplace_back(itr->path().string()); }
	}
}
} // anonymous namespace

namespace png2dds {

task::task(png2dds::args::data arguments)
	: _arguments{std::move(arguments)}
	, _png{} {
	// Use UTF-8 as the default encoding for Boost.Filesystem.
	boost::nowide::nowide_filesystem();
}

void task::start() { process_directory(_arguments.path, _png); }

} // namespace png2dds
