/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "todds/task.hpp"

#include "todds/pipeline.hpp"
#include "todds/regex.hpp"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/nowide/iostream.hpp>

#include <algorithm>
#include <string_view>

namespace fs = boost::filesystem;
using todds::pipeline::paths_vector;

namespace {

constexpr std::string_view png_extension{".png"};
constexpr std::string_view txt_extension{".txt"};

bool has_extension(const fs::path& path, std::string_view extension) {
	return boost::to_lower_copy(path.extension().string()) == extension;
}

/**
 * Checks if a source path is a valid input.
 * @param path Source path to be checked.
 * @param regex A regex constructed with an empty pattern will always return true.
 * @param scratch Scratch used for regular expression evaluation.
 * @return True if the path should be processed.
 */
bool is_valid_source(const fs::path& path, const todds::regex& regex, todds::regex::scratch_type& scratch) {
	return has_extension(path, png_extension) && regex.match(scratch, path.string());
}

fs::path to_dds_path(const fs::path& png_path, const fs::path& output) {
	constexpr std::string_view dds_extension{".dds"};
	return (output / png_path.stem()) += dds_extension.data();
}

class should_generate_dds final {
public:
	should_generate_dds(bool overwrite, bool overwrite_new)
		: _overwrite{overwrite}
		, _overwrite_new{overwrite_new} {}

	bool operator()(const fs::path& png_path, const fs::path& dds_path) const {
		return _overwrite || !fs::exists(dds_path) ||
					 (_overwrite_new && (fs::last_write_time(png_path) > fs::last_write_time(dds_path)));
	}

private:
	bool _overwrite;
	bool _overwrite_new;
};

void add_files(
	const fs::path& png_path, const fs::path& dds_path, paths_vector& paths, const should_generate_dds& should_generate) {
	if (should_generate(png_path, dds_path)) { paths.emplace_back(png_path, dds_path); }
}

void process_directory(paths_vector& paths, const fs::path& input, const fs::path& output, bool different_output,
	const todds::regex& regex, todds::regex::scratch_type& scratch, const should_generate_dds& should_generate,
	std::size_t depth) {
	fs::path current_output = output;
	const fs::directory_entry dir{input};
	for (fs::recursive_directory_iterator itr{dir}; itr != fs::recursive_directory_iterator{}; ++itr) {
		const fs::path& current_input = itr->path();
		if (is_valid_source(current_input, regex, scratch)) {
			if (different_output) {
				const auto relative = fs::relative(current_input.parent_path(), input);
				if (!relative.filename_is_dot()) { current_output = output / relative; }
			}
			const fs::path dds_path =
				to_dds_path(current_input, different_output ? current_output : current_input.parent_path());
			add_files(current_input, dds_path, paths, should_generate);
			if (different_output && !fs::exists(current_output)) {
				// Create the output folder if necessary.
				fs::create_directories(current_output);
			}
		}
		if (static_cast<unsigned int>(itr.depth()) >= depth) { itr.disable_recursion_pending(); }
	}
}

paths_vector get_paths(const todds::args::data& arguments) {
	const fs::path& input = arguments.input;
	const bool different_output = static_cast<bool>(arguments.output);
	const fs::path output = different_output ? arguments.output.value() : input.parent_path();

	const should_generate_dds should_generate(arguments.overwrite, arguments.overwrite_new);
	const auto depth = arguments.depth;

	const auto& regex = arguments.regex;
	todds::regex::scratch_type scratch = regex.allocate_scratch();

	paths_vector paths{};
	if (fs::is_directory(input)) {
		process_directory(paths, input, output, different_output, regex, scratch, should_generate, depth);
	} else if (is_valid_source(input, arguments.regex, scratch)) {
		const fs::path dds_path = to_dds_path(input, output);
		add_files(input, dds_path, paths, should_generate);
	} else if (has_extension(input, txt_extension)) {
		boost::nowide::fstream stream{input};
		std::string buffer;
		while (std::getline(stream, buffer)) {
			const fs::path current_path{buffer};
			if (fs::is_directory(current_path)) {
				process_directory(paths, current_path, current_path, false, regex, scratch, should_generate, depth);
			} else if (is_valid_source(current_path, arguments.regex, scratch)) {
				const fs::path dds_path = to_dds_path(current_path, current_path.parent_path());
				add_files(current_path, dds_path, paths, should_generate);
			} else {
				boost::nowide::cerr << current_path.string() << " is not a PNG file or a directory.\n";
			}
		}
	}

	// Process the list in order ignoring duplicates.
	std::sort(paths.begin(), paths.end());
	paths.erase(std::unique(paths.begin(), paths.end()), paths.end());

	return paths;
}

void clean_dds_files(const paths_vector& files) {
	for (const auto& [_, dds_file] : files) { fs::remove(dds_file); }
}

} // anonymous namespace

namespace todds {

void run(const args::data& arguments) {
	pipeline::input input_data;
	input_data.paths = get_paths(arguments);
	if (input_data.paths.empty()) { return; }
	if (arguments.clean) {
		clean_dds_files(input_data.paths);
		return;
	}
	input_data.parallelism = arguments.threads;
	input_data.mipmaps = arguments.mipmaps;
	input_data.format = arguments.format;
	input_data.quality = arguments.quality;
	input_data.fix_size = arguments.fix_size;
	input_data.vflip = arguments.vflip;
	input_data.mipmap_filter = arguments.mipmap_filter;
	input_data.mipmap_blur = arguments.mipmap_blur;
	input_data.scale = arguments.scale;
	input_data.max_size = arguments.max_size;
	input_data.scale_filter = arguments.scale_filter;
	input_data.verbose = arguments.verbose;

	// Launch the parallel pipeline.
	pipeline::encode_as_dds(input_data);
}

} // namespace todds
