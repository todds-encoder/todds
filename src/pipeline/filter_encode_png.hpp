/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/input.hpp"
#include "todds/mipmap_image.hpp"
#include "todds/png.hpp"

#include <oneapi/tbb/parallel_pipeline.h>

#include "filter_common.hpp"

namespace todds::pipeline::impl {

struct png_data {
	vector<std::uint8_t> image{};
	std::size_t file_index{};
};

oneapi::tbb::filter<std::unique_ptr<mipmap_image>, png_data> encode_png_filter(
	const paths_vector& paths, error_queue& errors);

} // namespace todds::pipeline::impl
