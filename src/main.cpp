/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <png2dds/arguments.hpp>

#include <argparse/argparse.hpp>

#include <iostream>

int main(int argc, char** argv) {
	int execution_status = EXIT_FAILURE;

	auto arguments = png2dds::arguments(argc, argv);
	try {
		arguments.parse_args(argc, argv);
		execution_status = EXIT_SUCCESS;
	} catch (const std::runtime_error& ex) {
		std::cerr << ex.what() << '\n' << arguments << '\n';
	} catch (const std::exception& ex) {
		std::cerr << "png2dds has been terminated because of an exception: " << ex.what() << ".\n";
	} catch (...) { std::cerr << "png2dds has been terminated because of an unknown exception.\n"; }

	return execution_status;
}
