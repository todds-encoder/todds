/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/memory.hpp"
#include "todds/mipmap_image.hpp"

#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace todds::png {

/**
 * Decodes a PNG file stored in memory.
 * @param file_index File index of the image in the list of files to load.
 * @param png Path to the PNG file, used for reporting errors.
 * @param buffer Memory data holding a PNG file read from the filesystem.
 * @param flip Flip source image vertically during decoding.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param mipmaps True if mipmaps should be calculated.
 * @return Decoded PNG image loaded in memory in an RGBA memory layout. Width and height are increased if needed to
 * make their number of pixels divisible by 4. If mipmaps is true, memory for mipmaps is allocated but only the first
 * image is loaded in memory.
 */
std::unique_ptr<mipmap_image> decode(std::size_t file_index, const std::string& png,
	std::span<const std::uint8_t> buffer, bool flip, std::size_t& width, std::size_t& height, bool mipmaps);

} // namespace todds::png
