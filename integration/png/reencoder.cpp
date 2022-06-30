/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/png.hpp"

#include "spng.h"
#include <boost/nowide/args.hpp>
#include <boost/nowide/cstdio.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/nowide/iostream.hpp>
#include <fmt/format.h>

using boost::nowide::cerr;

/**
 * Simple program that decodes and then re-encodes the same PNG file.
 * Intended as a quick coherence check for the PNG decode functionality.
 * @param argc Number of arguments.
 * @param argv Arguments.
 * @return EXIT_SUCCESS if the reencoding was made successfully.
 */
int main(int argc, char** argv) {
	boost::nowide::args _(argc, argv);

	int execution_status = EXIT_FAILURE;
	if (argc == 3) {
		std::string png = argv[1];
		boost::nowide::ifstream ifs{png, std::ios::in | std::ios::binary};
		const std::vector<std::uint8_t> buffer{std::istreambuf_iterator<char>{ifs}, {}};
		auto img = png2dds::decode(png, buffer);

		/* Creating an encoder context requires a flag */
		spng_ctx* ctx = spng_ctx_new(SPNG_CTX_ENCODER);

		std::FILE* file = boost::nowide::fopen(argv[2], "wbo");
		spng_set_png_file(ctx, file);
		spng_ihdr ihdr{};
		ihdr.width = static_cast<std::uint32_t>(img.width());
		ihdr.height = static_cast<std::uint32_t>(img.height());
		ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
		ihdr.bit_depth = 8;

		spng_set_ihdr(ctx, &ihdr);

		if (const int ret = spng_encode_image(ctx, img.buffer(), img.size(), SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
				ret != 0) {
			cerr << fmt::format("Could not encode png file {:s}: {:s}", png, spng_strerror(ret)) << '\n';
			return execution_status;
		}

		execution_status = EXIT_SUCCESS;
	}

	return execution_status;
}
