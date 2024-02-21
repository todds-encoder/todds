/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/arguments.hpp"
#include "todds/project.hpp"
#include "todds/report.hpp"
#include "todds/task.hpp"

#include <boost/nowide/filesystem.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/predef.h>
#include <fmt/format.h>
#include <oneapi/tbb/tick_count.h>

#include <charconv>

#if BOOST_OS_WINDOWS
#include <windows.h>
#else
#include <csignal>
#endif

using boost::nowide::cerr;
using boost::nowide::cout;

bool encoding_cancelled = false;
std::atomic<bool> force_finish{};

#if BOOST_OS_WINDOWS
BOOL WINAPI ctrl_c_signal(DWORD ctrl_type) {
	if (ctrl_type == CTRL_C_EVENT) {
		encoding_cancelled = true;
		// If force_finish was already true, it means that Ctrl+C has been pressed a second time.
		const bool should_stop = force_finish.exchange(true);
		return should_stop ? FALSE : TRUE;
	}

	return FALSE;
}

void handle_ctrl_c_signal() { SetConsoleCtrlHandler(ctrl_c_signal, TRUE); }
#else
struct sigaction act;
struct sigaction oldact;

void ctrl_c_signal(int signum) {
	if (signum == SIGINT) { force_finish.store(true); }
}

void handle_ctrl_c_signal() {
	act.sa_handler = ctrl_c_signal;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, &oldact);
}
#endif

void process_pipeline_reports(
	const todds::args::data& data, std::future<void>& pipeline, todds::report_queue& updates) {
	std::size_t current_texture_count{};
	std::size_t total_texture_count{};

	while (!updates.empty() || pipeline.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
		todds::report update{};
		const std::size_t previous_texture_count = current_texture_count;

		while (updates.try_pop(update)) {
			switch (update.type()) {
			case todds::report_type::RETRIEVING_FILES_STARTED: cout << "Retrieving files to be processed.\n"; break;
			case todds::report_type::RETRIEVING_FILES_PROGRESS: break;
			case todds::report_type::FILE_RETRIEVAL_TIME:
				cout << fmt::format("File retrieval time: {:.3f} seconds.\n", (static_cast<double>(update.value()) / 1000.0));
				break;
			case todds::report_type::FILE_VERBOSE:
				if (data.verbose) { cout << fmt::format("{:s}\n", update.data()); }
				break;
			case todds::report_type::PROCESS_STARTED:
				total_texture_count = update.value();
				cout << fmt::format("Processing {:d} textures.\n", total_texture_count);
				break;
			case todds::report_type::ENCODING_PROGRESS: ++current_texture_count; break;
			case todds::report_type::PIPELINE_ERROR: cerr << update.data() << '\n'; break;
			}
		}

		if (data.progress && previous_texture_count < current_texture_count) {
			// \r without a \n at the end to reuse the same line.
			cout << fmt::format("\rProgress: {:d}/{:d}", current_texture_count, total_texture_count);
		}

		cout.flush();
		cerr.flush();
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(50ms);
	}

	// Set up the stream for the next string.
	cout << '\n';
}

int main(int argc, char** argv) {
	const auto start_time = oneapi::tbb::tick_count::now();

	int execution_status = EXIT_FAILURE;

	// Use UTF-8 as the default encoding for Boost.Filesystem and the global C++ locale.
	std::locale::global(boost::nowide::nowide_filesystem());

#if defined(NDEBUG)
	try {
#endif // defined(_NDEBUG)
		auto data = todds::args::get(argc, argv);
		if (!data.stop_message.empty()) {
			if (data.help) { execution_status = EXIT_SUCCESS; }
			auto& stream = data.help ? cout : cerr;
			stream << data.stop_message;
		} else {
			if (!data.warning_message.empty()) { cout << data.warning_message; }

			handle_ctrl_c_signal();
			todds::report_queue updates;
			std::future<void> pipeline = todds::run(data, force_finish, updates);
			process_pipeline_reports(data, pipeline, updates);

			execution_status = EXIT_SUCCESS;
			if (encoding_cancelled) { cout << "Encoding cancelled.\n"; }
			if (data.time) {
				const auto end_time = oneapi::tbb::tick_count::now();
				const double total_time = (end_time - start_time).seconds();
				cout << fmt::format("Total time: {:.3f} seconds\n", total_time);
			}
		}
#if defined(NDEBUG)
	} catch (const std::exception& ex) {
		cerr << fmt::format("{:s} has been terminated because of an exception: {:s}\n", todds::project::name(), ex.what());
	} catch (...) {
		cerr << fmt::format("{:s} has been terminated because of an unknown exception.\n", todds::project::name());
	}
#endif // defined(_NDEBUG)

	return execution_status;
}
