/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/arguments.hpp"
#include "png2dds/project.hpp"

#include <argparse/argparse.hpp>
#include <boost/nowide/args.hpp>
#include <boost/nowide/iostream.hpp>
#include <fmt/format.h>

using boost::nowide::cerr;

int main(int argc, char** argv) {
	boost::nowide::args _(argc, argv);
	int execution_status = EXIT_FAILURE;

	auto arguments = png2dds::arguments();
	try {
		arguments.parse_args(argc, argv);
		execution_status = EXIT_SUCCESS;
	} catch (const std::runtime_error& ex) {
		cerr << fmt::format("{:s}\n{:s}", ex.what(), arguments.help().str());
	} catch (const std::exception& ex) {
		cerr << fmt::format("{:s} has been terminated because of an exception: {:s}\n", png2dds::project::name(), ex.what());
	} catch (...) {
		cerr << fmt::format("{:s} has been terminated because of an unknown exception.\n", png2dds::project::name());
	}

	return execution_status;
}
