/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/pipeline.hpp"

#include "todds/dds.hpp"
#include "todds/string.hpp"
#include "todds/vector.hpp"

#include <boost/nowide/iostream.hpp>
#include <boost/predef.h>
#include <fmt/format.h>
#include <oneapi/tbb/global_control.h>
#include <oneapi/tbb/parallel_pipeline.h>

#include <atomic>
#include <chrono>
#include <future>

#include "filter_common.hpp"
#include "get_filters_from_settings.hpp"

#if BOOST_OS_WINDOWS
#include <windows.h>
#else
#include <csignal>
#endif

namespace otbb = oneapi::tbb;
using todds::dds_image;
using todds::pipeline::paths_vector;

namespace {

otbb::concurrent_queue<std::string>* cancel_encoding_log{};
std::atomic<bool>* force_finish_flag{};

#if BOOST_OS_WINDOWS
BOOL WINAPI ctrl_c_signal(DWORD ctrl_type) {
	if (ctrl_type == CTRL_C_EVENT) {
		assert(cancel_encoding_log != nullptr);
		assert(force_finish_flag != nullptr);
		// If force_finish was already true, it means that Ctrl+C has been pressed a second time.
		cancel_encoding_log->push("Cancelling encoding...");
		const bool should_stop = force_finish_flag->exchange(true);
		return should_stop ? FALSE : TRUE;
	}

	return FALSE;
}

void handle_ctrl_c_signal() { SetConsoleCtrlHandler(ctrl_c_signal, TRUE); }
#else
struct sigaction act;
struct sigaction oldact;

void ctrl_c_signal(int signum) {
	if (signum == SIGINT) {
		assert(cancel_encoding_log != nullptr);
		assert(force_finish_flag != nullptr);
		// If force_finish was already true, it means that Ctrl+C has been pressed a second time.
		cancel_encoding_log->push("Cancelling encoding...");
		force_finish_flag->store(true);
	}
}

void handle_ctrl_c_signal() {
	act.sa_handler = ctrl_c_signal;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, &oldact);
}
#endif

void error_reporting(otbb::concurrent_queue<std::string>& error_log, std::atomic<std::size_t>& progress,
	std::size_t total, std::atomic<bool>& force_finish) {
	using namespace std::chrono_literals;
	std::size_t last_progress{};
	todds::string error_str;
	bool requires_newline = true;

	while (progress < total && !force_finish) {
		while (error_log.try_pop(error_str)) {
			if (requires_newline) {
				boost::nowide::cerr << '\n';
				requires_newline = false;
			}
			boost::nowide::cout.flush();
			boost::nowide::cerr << error_str << '\n';
			boost::nowide::cerr.flush();
		}
		const std::size_t current_progress = progress;

		if (current_progress > last_progress && current_progress < total) {
			last_progress = current_progress;
			boost::nowide::cout << fmt::format("\rProgress: {:d}/{:d}", current_progress, total);
			boost::nowide::cout.flush();
			requires_newline = true;
			std::this_thread::sleep_for(50ms);
		}
	}
}

} // Anonymous namespace

namespace todds::pipeline {

void encode_as_dds(const input& input_data) {
	dds::initialize_encoding(input_data.format, input_data.alpha_format);

	// Ensure that OpenCV is working in sequential mode.
	cv::setNumThreads(0);
	// Setup the parallel pipeline.
	const otbb::global_control control(otbb::global_control::max_allowed_parallelism, input_data.parallelism);
	// Maximum number of files that the pipeline can process at the same time.
	const std::size_t tokens = input_data.parallelism * 4UL;

	// Used to give each file processed in a token a unique file index to access the paths and files_data vectors.
	std::atomic<std::size_t> counter;
	// Contains extra data about each file being processed.
	// Pipeline stages may write or read from this vector at any time. Since each token has a unique index, these
	// accesses are thread-safe.
	vector<impl::file_data> files_data(input_data.paths.size());

	// Stores all warnings and errors gathered during the pipeline execution.
	otbb::concurrent_queue<std::string> error_log;
	// Used to force a clean finalization of the pipeline when the user presses Ctrl + C.
	std::atomic<bool> force_finish;

	std::future<void> error_report{};
	if (input_data.progress) {
		error_report = std::async(std::launch::async, error_reporting, std::ref(error_log), std::ref(counter),
			input_data.paths.size(), std::ref(force_finish));
	}

	const otbb::filter<void, void> filters =
		get_filters_from_settings(input_data, counter, force_finish, error_log, files_data);

	// Sending the Ctrl+C signal will stop loading new files, but files being currently processed will carry on.
	cancel_encoding_log = &error_log;
	force_finish_flag = &force_finish;
	handle_ctrl_c_signal();
	otbb::parallel_pipeline(tokens, filters);

	if (error_report.valid()) {
		// Wait until the error report task is done before finishing the error log report.
		error_report.get();
		todds::string error_str;
		while (error_log.try_pop(error_str)) { boost::nowide::cerr << error_str << '\n'; }
		boost::nowide::cerr.flush();
		const auto paths_size = input_data.paths.size();
		boost::nowide::cout << fmt::format("\rProgress: {:d}/{:d}\n", std::min(counter.load(), paths_size), paths_size);
		boost::nowide::cout.flush();
	}

	cancel_encoding_log = nullptr;
	force_finish_flag = nullptr;

	if (input_data.report) {
		boost::nowide::cout << "File;Width;Height;Mipmaps;Format\n";
		for (std::size_t index = 0U; index < input_data.paths.size(); ++index) {
			const string& dds_path = input_data.paths[index].second.string();
			const auto& data = files_data[index];
			boost::nowide::cout << fmt::format(
				"{:s};{:d};{:d};{:d};{:s}\n", dds_path, data.width, data.height, data.mipmaps, format::name(data.format));
		}
	}
}

} // namespace todds::pipeline
