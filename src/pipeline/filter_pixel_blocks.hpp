/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/image_types.hpp"
#include "png2dds/input.hpp"
#include "png2dds/mipmap_image.hpp"

#include <oneapi/tbb/parallel_pipeline.h>

#include "filter_common.hpp"

namespace png2dds::pipeline::impl {

struct pixel_block_data {
	pixel_block_image image;
	std::size_t file_index;
};

oneapi::tbb::filter<mipmap_image, pixel_block_data> load_file_filter();

} // namespace png2dds::pipeline::impl
