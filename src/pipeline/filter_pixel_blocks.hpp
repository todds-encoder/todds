/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/image_types.hpp"
#include "todds/input.hpp"
#include "todds/mipmap_image.hpp"

#include <oneapi/tbb/parallel_pipeline.h>

#include "filter_common.hpp"

namespace todds::pipeline::impl {

struct pixel_block_data {
	pixel_block_image image;
	std::size_t file_index;
};

oneapi::tbb::filter<std::unique_ptr<mipmap_image>, pixel_block_data> pixel_blocks_filter();

} // namespace todds::pipeline::impl
