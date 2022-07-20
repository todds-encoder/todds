/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/arguments.hpp"
#include "png2dds/project.hpp"
#include "png2dds/task.hpp"

#include <boost/nowide/filesystem.hpp>
#include <boost/nowide/iostream.hpp>
#include <fmt/format.h>
#include <oneapi/tbb/tick_count.h>

using boost::nowide::cerr;
using boost::nowide::cout;

int main(int argc, char** argv) {
	const auto start_time = oneapi::tbb::tick_count::now();

	int execution_status = EXIT_FAILURE;

	// Use UTF-8 as the default encoding for Boost.Filesystem and the global C++ locale.
	std::locale::global(boost::nowide::nowide_filesystem());

	try {
		auto data = png2dds::args::get(argc, argv);
		if (!data.text.empty()) {
			auto& stream = data.error ? cerr : cout;
			stream << data.text;
		}
		if (!data.error) {
			png2dds::run(data);
			execution_status = EXIT_SUCCESS;
			if (data.time) {
				const auto end_time = oneapi::tbb::tick_count::now();
				cout << "Total time: " << (end_time - start_time).seconds() << " seconds \n";
			}
		}
	} catch (const std::exception& ex) {
		cerr << fmt::format(
			"{:s} has been terminated because of an exception: {:s}\n", png2dds::project::name(), ex.what());
	} catch (...) {
		cerr << fmt::format("{:s} has been terminated because of an unknown exception.\n", png2dds::project::name());
	}

	return execution_status;
}
