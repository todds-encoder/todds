/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/task.hpp"

#include "png2dds/dds.hpp"
#include "png2dds/pipeline.hpp"
#include "png2dds/regex.hpp"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <oneapi/tbb/global_control.h>

#include <algorithm>
#include <optional>
#include <string_view>

namespace fs = boost::filesystem;
namespace otbb = oneapi::tbb;
using png2dds::pipeline::paths_vector;

namespace {

bool has_png_extension(const fs::path& path) {
	constexpr std::string_view png_extension{".png"};
	return boost::to_lower_copy(path.extension().string()) == png_extension;
}

/**
 * Checks if a source path is a valid input.
 * @param path Source path to be checked.
 * @param regex A regex constructed with an empty pattern will always return true.
 * @param scratch Scratch used for regular expression evaluation.
 * @return True if the path should be processed.
 */
bool is_valid_source(const fs::path& path, const png2dds::regex& regex, png2dds::regex::scratch_type& scratch) {
	return has_png_extension(path) && regex.match(scratch, path.string());
}

fs::path to_dds_path(const fs::path& png_path, const fs::path& output) {
	constexpr std::string_view dds_extension{".dds"};
	return (output / png_path.stem()) += dds_extension.data();
}

void add_files(const fs::path& png_path, const fs::path& dds_path, paths_vector& paths, bool overwrite) {
	if (overwrite || !fs::exists(dds_path)) { paths.emplace_back(png_path, dds_path); }
}

paths_vector get_paths(const png2dds::args::data& arguments) {
	const fs::path& input = arguments.input;
	const bool different_output = static_cast<bool>(arguments.output);
	const fs::path output = different_output ? arguments.output.value() : input.parent_path();
	const bool overwrite = arguments.overwrite;
	const auto depth = arguments.depth;

	const auto& regex = arguments.regex;
	png2dds::regex::scratch_type scratch = regex.allocate_scratch();

	paths_vector paths;
	if (fs::is_directory(input)) {
		fs::path current_output = output;
		const fs::directory_entry dir{input};
		for (fs::recursive_directory_iterator itr{dir}; itr != fs::recursive_directory_iterator{}; ++itr) {
			const fs::path& current_input = itr->path();
			if (is_valid_source(current_input, regex, scratch)) {
				fs::path output_current = current_input.parent_path();
				const fs::path dds_path =
					to_dds_path(current_input, different_output ? current_output : current_input.parent_path());
				add_files(current_input, dds_path, paths, overwrite);
				if (different_output && !fs::exists(current_output)) {
					// Create the output folder if necessary.
					fs::create_directories(current_output);
				}
			} else if (different_output && fs::is_directory(current_input)) {
				// Update current_output to match the relative path of the current input.
				current_output = output / fs::relative(current_input, input);
			}
			if (static_cast<unsigned int>(itr.depth()) >= depth) { itr.disable_recursion_pending(); }
		}
	} else if (is_valid_source(input, arguments.regex, scratch)) {
		const fs::path dds_path = to_dds_path(input, output);
		add_files(input, dds_path, paths, overwrite);
	}

	// Process the list in order ignoring duplicates.
	std::sort(paths.begin(), paths.end());
	paths.erase(std::unique(paths.begin(), paths.end()), paths.end());

	return paths;
}

} // anonymous namespace

namespace png2dds {

void run(const args::data& arguments) {
	const paths_vector paths = get_paths(arguments);
	if (paths.empty()) { return; }

	// Configure the maximum parallelism allowed for tbb.
	otbb::global_control control(otbb::global_control::max_allowed_parallelism, arguments.threads);
	const std::size_t num_tokens = arguments.threads * 4UL;

	// Initialize the BC7 DDS encoder.
	dds::initialize_bc7_encoding();

	// Launch the parallel pipeline.
	pipeline::encode_as_dds(num_tokens, arguments.level, arguments.vflip, paths);
}

} // namespace png2dds
