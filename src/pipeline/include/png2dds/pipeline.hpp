/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/arguments.hpp"
#include "png2dds/vector.hpp"

#include <boost/filesystem/path.hpp>

namespace png2dds::pipeline {
using paths_vector = png2dds::vector<std::pair<boost::filesystem::path, boost::filesystem::path>>;

/**
 * Setup the pipeline before enqueueing new work. This operation is NOT thread safe.
 * @param parallelism Maximum parallelism allowed for the internal TBB pipeline.
 */
void setup(std::size_t parallelism);

/**
 * Encodes a list of PNG files as DDS.
 * @param arguments Program arguments to use.
 * @param paths List of files to encode, along with their corresponding desired dds paths.
 */
void encode_as_dds(const args::data& arguments, const paths_vector& paths);

} // namespace png2dds::pipeline
