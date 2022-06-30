/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/arguments.hpp"
#include "png2dds/project.hpp"
#include "png2dds/task.hpp"

#include <boost/nowide/iostream.hpp>
#include <fmt/format.h>

using boost::nowide::cerr;
using boost::nowide::cout;

int main(int argc, char** argv) {
	int execution_status = EXIT_FAILURE;

	try {
		auto data = png2dds::args::get(argc, argv);
		if (!data.text.empty()) {
			auto& stream = data.error ? cerr : cout;
			stream << data.text;
		}
		if (!data.error) {
			png2dds::task(std::move(data)).start();
			execution_status = EXIT_SUCCESS;
		}
	} catch (const std::exception& ex) {
		cerr << fmt::format(
			"{:s} has been terminated because of an exception: {:s}\n", png2dds::project::name(), ex.what());
	} catch (...) {
		cerr << fmt::format("{:s} has been terminated because of an unknown exception.\n", png2dds::project::name());
	}

	return execution_status;
}
