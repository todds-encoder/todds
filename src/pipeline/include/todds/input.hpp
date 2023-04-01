/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/filter.hpp"
#include "todds/format.hpp"
#include "todds/vector.hpp"

#include <boost/filesystem/path.hpp>

namespace todds::pipeline {

/** Each entry is a PNG file to be encoded and the desired destination path for the resulting DDS file. */
using paths_vector = todds::vector<std::pair<boost::filesystem::path, boost::filesystem::path>>;

/** Input data for the pipeline. */
struct input {
	/** Maximum parallelism allowed for the internal TBB pipeline. */
	std::size_t parallelism{};

	/** True if mipmaps should be generated. */
	bool mipmaps{};

	/** PNG files to convert, and their destination paths. */
	paths_vector paths{};

	/** DDS file format to use for encoding. */
	todds::format::type format{};

	/** Quality level to use in this encoding. */
	todds::format::quality quality{};

	/** Set image width and height to the next multiple of 4. */
	bool fix_size{};

	/** Flip source images vertically before encoding. */
	bool vflip{};

	/** Filter used to resize images during mipmap calculations. */
	todds::filter::type mipmap_filter{};

	/** Blur applied during mipmap calculations. Defaults to 0.55. */
	double mipmap_blur{};

	/** Image scaling in %. */
	uint16_t scale{};

	/** Maximum width and height of encoded images in pixels. */
	uint32_t max_size{};

	/** Filter used to resize images during scaling or max size changes. */
	todds::filter::type scale_filter{};

	/** Display progress messages. */
	bool verbose{};
};

} // namespace todds::pipeline
