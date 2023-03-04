/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

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
 * @param width Width of the image.
 * @param height Height of the image.
 * @param mipmaps True if mipmaps should be calculated.
 * @param mipmap_count Number of mipmaps.
 * @return Decoded PNG image loaded in memory in an RGBA memory layout. Width and height are increased if needed to
 * make their number of pixels divisible by 4. If mipmaps is true, the buffer will have enough extra space allocated to
 * fit all mipmaps.
 */
image decode(std::size_t file_index, const std::string& png, std::span<const std::uint8_t> buffer, bool flip,
	std::size_t& width, std::size_t& height, bool mipmaps, std::size_t& mipmap_count);

} // namespace png2dds::png
