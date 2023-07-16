/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/image_types.hpp"

#include <oneapi/tbb/parallel_pipeline.h>

#include "filter_pixel_blocks.hpp"

namespace todds::pipeline::impl {

struct dds_data {
	dds_image image;
	std::size_t file_index;
};

oneapi::tbb::filter<pixel_block_data, dds_data> encode_dds_filter(todds::vector<file_data>& files_data,
	todds::format::type format_type, todds::format::quality quality, bool alpha_black);
} // namespace todds::pipeline::impl
