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
	todds::format::type format;
	todds::format::quality quality;
	bool mipmaps;
	todds::filter::type filter;
	std::size_t threads;
	std::size_t depth;
	bool overwrite;
	bool vflip;
	bool time;
	bool verbose;
	todds::regex regex;
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