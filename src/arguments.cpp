/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <png2dds/project.hpp>

#include <argparse/argparse.hpp>

#include <string>

namespace png2dds {

argparse::ArgumentParser arguments(int argc, char* argv[]) {
	argparse::ArgumentParser program(std::string{project::name()}, std::string{project::version()});

	program.add_description(std::string{project::description()});

	program.add_argument("input").help("input file");
	program.add_argument("output").help("output file");

	return program;
}

} // namespace png2dds
