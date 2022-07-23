/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_ARGUMENTS_HPP
#define PNG2DDS_ARGUMENTS_HPP

#include "png2dds/regex.hpp"
#include "png2dds/vector.hpp"

#include <boost/filesystem.hpp>

#include <optional>
#include <string_view>

namespace png2dds::args {

struct data {
	bool error;
	std::string text;
	boost::filesystem::path input;
	std::optional<boost::filesystem::path> output;
	unsigned int level;
	std::size_t threads;
	std::size_t depth;
	bool overwrite;
	bool vflip;
	bool time;
	bool verbose;
	png2dds::regex regex;
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
data get(const png2dds::vector<std::string_view>& arguments);
} // namespace png2dds::args

#endif // PNG2DDS_ARGUMENTS_HPP
