/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/task.hpp"

#include "png2dds/dds.hpp"
#include "png2dds/pipeline.hpp"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <boost/nowide/filesystem.hpp>
#include <oneapi/tbb/global_control.h>

#include <algorithm>
#include <string>
#include <string_view>

namespace fs = boost::filesystem;
namespace otbb = oneapi::tbb;
using png2dds::pipeline::paths_vector;

namespace {

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

void process_directory(const fs::path& path, paths_vector& paths, bool overwrite, std::size_t depth) {
	const fs::directory_entry dir{path};
	if (!fs::exists(dir) || !fs::is_directory(dir)) { return; }

	for (fs::recursive_directory_iterator itr{dir}; itr != fs::recursive_directory_iterator{}; ++itr) {
		try_add_file(itr->path(), itr->status(), paths, overwrite);
		if (static_cast<unsigned int>(itr.depth()) >= depth) { itr.disable_recursion_pending(); }
	}
}

} // anonymous namespace

namespace png2dds {

task::task(const args::data& arguments) {
	paths_vector paths;
	if (!try_add_file(arguments.input, fs::status(arguments.input), paths, arguments.overwrite)) {
		process_directory(arguments.input, paths, arguments.overwrite, arguments.depth);
	}

	// Process the list in order ignoring duplicates.
	std::sort(paths.begin(), paths.end());
	paths.erase(std::unique(paths.begin(), paths.end()), paths.end());

	if (paths.empty()) { return; }

	// Configure the maximum parallelism allowed for tbb.
	otbb::global_control control(otbb::global_control::max_allowed_parallelism, arguments.threads);
	const std::size_t num_tokens = arguments.threads * 4UL;

	// Initialize the BC7 DDS encoder.
	dds::initialize_bc7_encoding();

	// Launch the parallel pipeline.
	pipeline::encode_as_dds(num_tokens, arguments.level, arguments.flip, paths);
}

} // namespace png2dds
