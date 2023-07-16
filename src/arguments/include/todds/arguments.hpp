/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/filter.hpp"
#include "todds/format.hpp"
#include "todds/regex.hpp"
#include "todds/vector.hpp"

#include <boost/filesystem.hpp>

#include <optional>
#include <string_view>

namespace todds::args {

struct data {
	bool error;
	std::string text;
	boost::filesystem::path input;
	std::optional<boost::filesystem::path> output;
	bool clean;
	todds::format::type format;
	todds::format::quality quality;
	bool fix_size;
	bool mipmaps;
	todds::filter::type mipmap_filter;
	double mipmap_blur;
	uint16_t scale;
	uint32_t max_size;
	todds::filter::type scale_filter;
	std::size_t threads;
	std::size_t depth;
	bool overwrite;
	bool overwrite_new;
	bool vflip;
	bool time;
	bool verbose;
	todds::regex regex;
	bool dry_run;
	bool progress;
	bool alpha_black;
};

/**
 * Parse all argument data from command-line arguments.
 * Assumes that a UTF-8 locale has been set.
 * @param argc Number of arguments.
 * @param argv Array of arguments
 * @return Structure containing parsed arguments.
 */
data get(int argc, char** argv);

/**
 * Parse all argument data from command-line arguments.
 * Assumes that a UTF-8 locale has been set.
 * @param arguments Argument vector.
 * @return Structure containing parsed arguments.
 */
data get(const todds::vector<std::string_view>& arguments);
} // namespace todds::args
