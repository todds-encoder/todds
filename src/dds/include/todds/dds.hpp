/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/format.hpp"
#include "todds/image_types.hpp"

#include <bc7e_ispc.h>

#include <array>
#include <memory>

namespace todds::dds {

using bc7_params = ispc::bc7e_compress_block_params;

/**
 * Initialize the BC1 DDS encoder.
 * This function is not thread safe and it should be called only once.
 */
void initialize_bc1_encoding();

/**
 * Encode an image to BC1.
 * @param quality DDS encoding quality level.
 * @param image Source pixel block image.
 * @return BC1 encoded image.
 */
[[nodiscard]] dds_image bc1_encode(todds::format::quality quality, const pixel_block_image& image);

/**
 * Initialize the BC7 DDS encoder.
 * This function is not thread safe and it should be called only once.
 */
void initialize_bc7_encoding();

/**
 * Generate the parameters to use for BC7 DDS encoding.
 * @param quality DDS encoding quality level.
 * @return Parameters for the given quality level.
 */
[[nodiscard]] bc7_params bc7_encode_params(todds::format::quality quality) noexcept;

/**
 * Encode an image to BC7.
 * @param params BC7 block encoding parameters.
 * @param image Source pixel block image.
 * @return BC7 encoded image.
 */
[[nodiscard]] dds_image bc7_encode(const ispc::bc7e_compress_block_params& params, const pixel_block_image& image);

/**
 * Construct a DDS header.
 * @param format_type Format of the file.
 * @param width Original width of the image.
 * @param height Original height of the image.
 * @param mipmaps Number of mipmaps generated. Zero means no mipmaps.
 * @return Array containing the DDS header information.
 */
std::array<char, 124> dds_header(
	todds::format::type format_type, std::size_t width, std::size_t height, std::size_t mipmaps);

} // namespace todds::dds
