/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_PIPELINE_HPP
#define PNG2DDS_PIPELINE_HPP

#include "png2dds/vector.hpp"

#include <boost/filesystem/path.hpp>

namespace png2dds::pipeline {
using paths_vector = png2dds::vector<std::pair<boost::filesystem::path, boost::filesystem::path>>;

/**
 * Encodes a list of PNG files as DDS.
 * @param tokens Maximum number of files that the pipeline can process at the same time.
 * @param level BC7 encoding level to use.
 * @param vflip Flip source images vertically before encoding.
 * @param verbose If true it will report progress and errors.
 * @param paths List of files to encode, along with their corresponding desired dds paths.
 */
void encode_as_dds(std::size_t tokens, unsigned int level, bool vflip, bool verbose, const paths_vector& paths);

} // namespace png2dds::pipeline

#endif // PNG2DDS_PIPELINE_HPP
