/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/png.hpp"

#include "spng.h"
#include <boost/nowide/args.hpp>
#include <boost/nowide/fstream.hpp>

/**
 * Simple program that decodes and then re-encodes the same PNG file.
 * Intended as a quick coherence check for the PNG decode functionality.
 * @param argc Number of arguments.
 * @param argv Arguments.
 * @return EXIT_SUCCESS if the reencoding was made successfully.
 */
int main(int argc, char** argv) {
	boost::nowide::args nowide_args(argc, argv);

	int execution_status = EXIT_FAILURE;
	if (argc == 3) {
		const std::string input_png = argv[1];
		boost::nowide::ifstream ifs{input_png, std::ios::in | std::ios::binary};
		const std::vector<std::uint8_t> input_buffer{std::istreambuf_iterator<char>{ifs}, {}};
		const auto img = png2dds::png::decode(0U, input_png, input_buffer, false);

		const std::string output_png = argv[2];
		const auto output_buffer = png2dds::png::encode(output_png, img);

		boost::nowide::ofstream ofs{output_png, std::ios::out | std::ios::binary};
		ofs.write(output_buffer.span().data(), static_cast<std::ptrdiff_t>(output_buffer.span().size()));
		ofs.close();

		execution_status = EXIT_SUCCESS;
	}

	return execution_status;
}
