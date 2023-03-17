/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/filter.hpp"
#include "png2dds/format.hpp"
#include "png2dds/vector.hpp"

#include <boost/filesystem/path.hpp>

namespace png2dds::pipeline {

/** Each entry is a PNG file to be encoded and the desired destination path for the resulting DDS file. */
using paths_vector = png2dds::vector<std::pair<boost::filesystem::path, boost::filesystem::path>>;

/** Input data for the pipeline. */
struct input {
	/** Maximum parallelism allowed for the internal TBB pipeline. */
	std::size_t parallelism{};

	/** True if mipmaps should be generated. */
	bool mipmaps{};

	/** PNG files to convert, and their destination paths. */
	paths_vector paths{};

	/** DDS file format to use for encoding. */
	png2dds::format::type format{};

	/** Quality level to use in this encoding. */
	png2dds::format::quality quality{};

	/** Flip source images vertically before encoding. */
	bool vflip{};

	/** Filter used to resize images during mipmap calculations. */
	png2dds::filter::type filter{};

	/** Display progress messages. */
	bool verbose{};
};

} // namespace png2dds::pipeline
