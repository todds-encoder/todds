/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "todds/task.hpp"

#include "todds/file_retrieval.hpp"
#include "todds/input.hpp"
#include "todds/pipeline.hpp"

#include <oneapi/tbb/tick_count.h>

namespace fs = boost::filesystem;
using todds::pipeline::paths_vector;

namespace {

void verbose_output(const paths_vector& files, bool clean, todds::report_queue& updates) {
	for (const auto& [png_file, dds_file] : files) {
		updates.emplace(todds::report_type::file_verbose, clean ? dds_file.string() : png_file.string());
	}
}

void clean_dds_files(const paths_vector& files) {
	for (const auto& [_, dds_file] : files) { fs::remove(dds_file); }
}

void pipeline_execution(
	const todds::args::data& arguments, std::atomic<bool>& force_finish, todds::report_queue& updates) {
	todds::pipeline::input input_data;
	updates.emplace(todds::report_type::retrieving_files_started);

	const auto start_time = oneapi::tbb::tick_count::now();
	input_data.paths = get_paths(arguments, updates);

	const auto end_time = oneapi::tbb::tick_count::now();
	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	updates.emplace(todds::report_type::file_retrieval_time, milliseconds);

#if defined(TODDS_PIPELINE_DUMP)
	// Limit to a single file to avoid overwriting memory dumps, and any potential concurrency issues.
	if (input_data.paths.size() > 1U) { input_data.paths = paths_vector{input_data.paths[0]}; }
#endif // defined(TODDS_PIPELINE_DUMP)
	// Process arguments that affect the input.
	if (arguments.verbose) { verbose_output(input_data.paths, arguments.clean, updates); }
	if (arguments.dry_run) { return; }
	updates.emplace(todds::report_type::process_started, input_data.paths.size());
	if (input_data.paths.empty()) { return; }

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
	todds::pipeline::encode_as_dds(input_data, force_finish, updates);
}

} // anonymous namespace

namespace todds {

std::future<void> run(const args::data& arguments, std::atomic<bool>& force_finish, report_queue& updates) {
	return std::async(std::launch::async,
		[&arguments, &force_finish, &updates]() { pipeline_execution(arguments, force_finish, updates); });
}

} // namespace todds
