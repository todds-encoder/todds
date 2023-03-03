/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/dds_image.hpp"
#include "png2dds/image.hpp"
#include "png2dds/memory.hpp"

#include <cstdint>
#include <span>
#include <string>

namespace png2dds::png {


/**
 * Decodes a PNG file stored in a memory buffer.
 * @param file_index File index of the image in the list of files to load.
 * @param png Path to the PNG file, used for reporting errors.
 * @param buffer Memory buffer holding a PNG file read from the filesystem.
 * @param flip Flip source image vertically during decoding.
 * @return Decoded PNG image loaded in memory in an RGBA memory layout. Width and height are increased if needed to
 * make their number of pixels divisible by 4.
 */
image decode(std::size_t file_index, const std::string& png, std::span<const std::uint8_t> buffer, bool flip);

} // namespace png2dds::png
