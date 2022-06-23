/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/project.hpp"

#include <argparse/argparse.hpp>

namespace png2dds {

argparse::ArgumentParser arguments() {
	argparse::ArgumentParser program(std::string{project::name()}, std::string{project::version()});

	program.add_description(std::string{project::description()});

	program.add_argument("--only").help("Only convert PNGs that contain a folder with the specified folder in its path. "
																			"This comparison is case insensitive.");

	program.add_argument("path").help(
		"Convert PNG files in this path. DDS files will be created next to their PNG counterpart.");

	return program;
}

} // namespace png2dds
