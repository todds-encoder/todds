/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/input.hpp"
#include "png2dds/mipmap_image.hpp"
#include "png2dds/vector.hpp"

#include <oneapi/tbb/concurrent_queue.h>
#include <oneapi/tbb/parallel_pipeline.h>

#include <cstdint>

#include "filter_common.hpp"
#include "filter_load_png.hpp"

namespace png2dds::pipeline::impl {
oneapi::tbb::filter<png_file, mipmap_image> decode_png_filter(
	std::vector<file_data>& files_data, const paths_vector& paths, bool vflip, bool mipmaps, error_queue& errors);
} // namespace png2dds::pipeline::impl
