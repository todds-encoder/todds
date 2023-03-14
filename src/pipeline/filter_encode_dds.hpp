/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/image_types.hpp"

#include <oneapi/tbb/parallel_pipeline.h>

#include "filter_pixel_blocks.hpp"

namespace png2dds::pipeline::impl {

struct dds_data {
	dds_image image;
	std::size_t file_index;
};

oneapi::tbb::filter<pixel_block_data, dds_data> encode_dds_filter(
	std::vector<file_data>& files_data, png2dds::format::type format_type, png2dds::format::quality quality);
} // namespace png2dds::pipeline::impl
