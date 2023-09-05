/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "todds/task.hpp"

#include "todds/pipeline.hpp"
#include "todds/regex.hpp"
#include "todds/string.hpp"

#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/nowide/iostream.hpp>
#include <fmt/format.h>
#include <oneapi/tbb/tick_count.h>

#include <algorithm>
#include <string_view>

#if BOOST_OS_WINDOWS
#include <filesystem>
#endif // BOOST_OS_WINDOWS

namespace fs = boost::filesystem;
using todds::pipeline::paths_vector;

namespace {

constexpr std::string_view png_extension{".png"};
constexpr std::string_view txt_extension{".txt"};

bool has_extension(const fs::path& path, const std::string_view extension) {
	const todds::string path_extension = path.extension().string();
	if (path_extension.size() != extension.size()) { return false; }
	return todds::to_lower_copy(path_extension) == extension;
}

/**
 * Checks if a source path is a valid input file.
 * @param path Source path to be checked.
 * @param regex A regex constructed with an empty pattern will always return true.
 * @return True if the path should be processed.
 */
bool is_valid_input_file(const fs::path& path, const todds::regex& regex) {
	return has_extension(path, png_extension) && regex.match(path.string());
}

fs::path to_output_path(todds::format::type format, const fs::path& input_file, const fs::path& output_path) {
	constexpr std::string_view dds_extension{".dds"};
	const std::string_view extension = format != todds::format::type::png ? dds_extension : png_extension;
	return (output_path / input_file.stem()) += extension.data();
}

std::time_t last_write_time(const fs::path& path) {
#if BOOST_OS_WINDOWS
	// On Windows, boost::filesystem::last_write_time seems to be multiple orders of magnitude slower than in other
	// operative systems. Rely on std instead in this case, which should be fine regarding UTF-8 and long paths.
	return std::filesystem::last_write_time(path.string()).time_since_epoch().count();
#else
	return fs::last_write_time(path);
#endif // BOOST_OS_WINDOWS
}

class should_generate_file final {
public:
	should_generate_file(bool overwrite, bool overwrite_new)
		: _overwrite{overwrite}
		, _overwrite_new{overwrite_new} {}

	bool operator()(const fs::path& input_path, const fs::path& output_path) const {
		return input_path != output_path &&
					 (_overwrite || !fs::exists(output_path) ||
						 (_overwrite_new && (fs::last_write_time(input_path) > fs::last_write_time(output_path))));
	}

private:
	bool _overwrite;
	bool _overwrite_new;
};

void add_files(const fs::path& input_path, const fs::path& output_path, paths_vector& paths,
	const should_generate_file& should_generate) {
	if (should_generate(input_path, output_path)) { paths.emplace_back(input_path, output_path); }
}

void process_directory(paths_vector& paths, const fs::path& input_path, const fs::path& output_path,
	bool different_output, const should_generate_file& should_generate, const todds::args::data& arguments) {
	const todds::regex& regex = arguments.regex;
	const todds::format::type format = arguments.format;
	const std::size_t depth = arguments.depth;

	fs::path current_output = output_path;
	const fs::directory_entry dir{input_path};
	for (fs::recursive_directory_iterator itr{dir}; itr != fs::recursive_directory_iterator{}; ++itr) {
		const fs::path& input_file = itr->path();
		if (is_valid_input_file(input_file, regex)) {
			if (different_output) {
				const auto relative = fs::relative(input_file.parent_path(), input_path);
				if (!relative.filename_is_dot()) { current_output = output_path / relative; }
			}
			const fs::path output_file =
				to_output_path(format, input_file, different_output ? current_output : input_file.parent_path());
			add_files(input_file, output_file, paths, should_generate);
			if (different_output && !fs::exists(current_output)) {
				// Create the output_path folder if necessary.
				fs::create_directories(current_output);
			}
		}
		if (static_cast<unsigned int>(itr.depth()) >= depth) { itr.disable_recursion_pending(); }
	}
}

paths_vector get_paths(const todds::args::data& arguments) {
	const fs::path& input_path = arguments.input;
	const bool different_output = static_cast<bool>(arguments.output);
	const fs::path output_path = different_output ? arguments.output.value() : input_path.parent_path();

	const should_generate_file should_generate(arguments.overwrite, arguments.overwrite_new);
	const todds::format::type format = arguments.format;

	paths_vector paths{};
	if (fs::is_directory(input_path)) {
		process_directory(paths, input_path, output_path, different_output, should_generate, arguments);
	} else if (is_valid_input_file(input_path, arguments.regex)) {
		const fs::path dds_path = to_output_path(format, input_path, output_path);
		add_files(input_path, dds_path, paths, should_generate);
	} else if (has_extension(input_path, txt_extension)) {
		boost::nowide::fstream stream{input_path};
		todds::string buffer;
		while (std::getline(stream, buffer)) {
			const fs::path current_path{buffer};
			if (fs::is_directory(current_path)) {
				process_directory(paths, current_path, current_path, false, should_generate, arguments);
			} else if (is_valid_input_file(current_path, arguments.regex)) {
				const fs::path dds_path = to_output_path(format, current_path, current_path.parent_path());
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

void verbose_output(const paths_vector& files, bool clean) {
	for (const auto& [png_file, dds_file] : files) {
		const std::string& path = clean ? dds_file.string() : png_file.string();
		boost::nowide::cerr << path << '\n';
	}
}

void clean_dds_files(const paths_vector& files) {
	for (const auto& [_, dds_file] : files) { fs::remove(dds_file); }
}

} // anonymous namespace

namespace todds {

void run(const args::data& arguments) {
	pipeline::input input_data;
	if (arguments.progress) { boost::nowide::cout << fmt::format("Retrieving files to be encoded.\n"); }
	const auto start_time = oneapi::tbb::tick_count::now();
	input_data.paths = get_paths(arguments);
	if (arguments.time) {
		const auto end_time = oneapi::tbb::tick_count::now();
		boost::nowide::cout << "File retrieval time: " << (end_time - start_time).seconds() << " seconds \n";
	}
	if (input_data.paths.empty()) { return; }

#if defined(TODDS_PIPELINE_DUMP)
	// Limit to a single file to avoid overwriting memory dumps, and any potential concurrency issues.
	if (input_data.paths.size() > 1U) { input_data.paths = paths_vector{input_data.paths[0]}; }
#endif // defined(TODDS_PIPELINE_DUMP)
	// Process arguments that affect the input.
	if (arguments.verbose) { verbose_output(input_data.paths, arguments.clean); }
	if (arguments.dry_run) { return; }
	if (arguments.clean) {
		clean_dds_files(input_data.paths);
		return;
	}

	input_data.parallelism = arguments.threads;
	input_data.mipmaps = arguments.mipmaps;
	input_data.format = arguments.format;
	input_data.alpha_format = arguments.alpha_format;
	input_data.quality = arguments.quality;
	input_data.fix_size = arguments.fix_size;
	input_data.vflip = arguments.vflip;
	input_data.mipmap_filter = arguments.mipmap_filter;
	input_data.mipmap_blur = arguments.mipmap_blur;
	input_data.scale = arguments.scale;
	input_data.max_size = arguments.max_size;
	input_data.scale_filter = arguments.scale_filter;
	input_data.progress = arguments.progress;
	input_data.alpha_black = arguments.alpha_black;
	input_data.report = arguments.report;

	// Launch the parallel pipeline.
	pipeline::encode_as_dds(input_data);
}

} // namespace todds
