/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <argparse/argparse.hpp>

namespace png2dds {

argparse::ArgumentParser arguments(int argc, char* argv[]) {
	argparse::ArgumentParser program("png2dds");

	program.add_argument("input").help("input file");
	program.add_argument("output").help("output file");

	return program;
}

} // namespace png2dds
