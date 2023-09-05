/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/mipmap_image.hpp"

#include "filter_common.hpp"
#include "filter_decode_png.hpp"

namespace todds::pipeline::impl {
oneapi::tbb::filter<std::unique_ptr<mipmap_image>, std::unique_ptr<mipmap_image>> scale_image_filter(
	vector<file_data>& files_data, bool mipmaps, std::uint16_t scale, std::uint32_t max_size, filter::type filter,
	const paths_vector& paths, error_queue& errors);
} // namespace todds::pipeline::impl
