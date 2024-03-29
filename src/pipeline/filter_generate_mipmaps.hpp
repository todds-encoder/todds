/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/mipmap_image.hpp"

#include "filter_common.hpp"
#include "filter_decode_png.hpp"

namespace todds::pipeline::impl {
oneapi::tbb::filter<std::unique_ptr<mipmap_image>, std::unique_ptr<mipmap_image>> generate_mipmaps_filter(
	filter::type filter, double blur);
} // namespace todds::pipeline::impl
