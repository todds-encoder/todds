/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/mipmap_image.hpp"
#include "png2dds/vector.hpp"

#include <cstdint>

namespace png2dds {

constexpr std::size_t pixel_block_side = 4UL;

/**
 * RGBA pixel block image.
 */
using pixel_block_image = vector<std::uint32_t>;

using dds_image = vector<std::uint64_t>;

pixel_block_image to_pixel_blocks(const mipmap_image& img);

} // namespace png2dds
