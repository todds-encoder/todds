/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_DDS_HPP
#define PNG2DDS_DDS_HPP

#include "png2dds/dds_image.hpp"
#include "png2dds/pixel_block_image.hpp"

#include <bc7e_ispc.h>

#include <memory>

namespace png2dds::dds {

/**
 * Initialize the BC1 DDS encoder.
 * This function is not thread safe and it should be called only once.
 */
void initialize_bc1_encoding();

/**
 * Encode an image to BC1.
 * @param level DDS encoding level.
 * @param image Source block image.
 * @return BC1 encoded image.
 */
[[nodiscard]] dds_image bc1_encode(unsigned int level, const pixel_block_image& image);

/**
 * Initialize the BC7 DDS encoder.
 * This function is not thread safe and it should be called only once.
 */
void initialize_bc7_encoding();

/**
 * Generate the parameters to use for BC7 DDS encoding.
 * @param level Encoder quality level.
 * @return Parameters for the given quality level.
 */
ispc::bc7e_compress_block_params bc7_encode_params(unsigned int level) noexcept;

/**
 * Encode an image to BC7.
 * @param params BC7 block encoding parameters.
 * @param image Source block image.
 * @return BC7 encoded image.
 */
[[nodiscard]] dds_image bc7_encode(const ispc::bc7e_compress_block_params& params, const pixel_block_image& image);

} // namespace png2dds::dds

#endif // PNG2DDS_DDS_HPP
