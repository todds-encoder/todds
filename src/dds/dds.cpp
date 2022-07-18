/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/dds.hpp"

#include "png2dds/dds_image.hpp"

#include <bc7e_ispc.h>

#include <algorithm>
#include <array>
#include <cassert>

namespace {

// Number of blocks to process on each encoding pass.
constexpr std::size_t blocks_to_process = 64UL;
} // anonymous namespace

namespace png2dds::dds {

encoder::encoder(unsigned int level)
	: _pimpl(std::make_unique<ispc::bc7e_compress_block_params>()) {
	// Encoder initialization.
	ispc::bc7e_compress_block_init();
	// Encoder parameter initialization. Perceptual is currently not supported.
	constexpr bool perceptual = false;
	switch (level) {
	case 0U: ispc::bc7e_compress_block_params_init_ultrafast(_pimpl.get(), perceptual); break;
	case 1U: ispc::bc7e_compress_block_params_init_veryfast(_pimpl.get(), perceptual); break;
	case 2U: ispc::bc7e_compress_block_params_init_fast(_pimpl.get(), perceptual); break;
	case 3U: ispc::bc7e_compress_block_params_init_basic(_pimpl.get(), perceptual); break;
	case 4U: ispc::bc7e_compress_block_params_init_slow(_pimpl.get(), perceptual); break;
	case 5U: ispc::bc7e_compress_block_params_init_veryslow(_pimpl.get(), perceptual); break;
	default:
	case 6U: ispc::bc7e_compress_block_params_init_slowest(_pimpl.get(), perceptual); break;
	}
}

encoder::~encoder() = default;

dds_image encoder::encode(const pixel_block_image& image) const {
	dds_image dds_img(image);

	for (std::size_t block_y = 0UL; block_y < dds_img.height(); ++block_y) {
		for (std::size_t block_x = 0UL; block_x < dds_img.width(); block_x += blocks_to_process) {
			// In some cases the number of blocks of the image may not be divisible by 64.
			const std::size_t current_blocks_to_process = std::min(blocks_to_process, dds_img.width() - block_x);
			auto* dds_block = dds_img.block(block_x, block_y);
			const pixel_block_image::block pixel_block = image.get_block(block_x, block_y);
			ispc::bc7e_compress_blocks(
				static_cast<std::uint32_t>(current_blocks_to_process), dds_block, pixel_block.data(), _pimpl.get());
		}
	}

	return dds_img;
}

} // namespace png2dds::dds
