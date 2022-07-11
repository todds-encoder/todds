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
#include <vector>

namespace {

constexpr auto pixel_block_side = png2dds::dds_image::pixel_block_side;

// Number of bytes in a pixel block.
constexpr std::size_t pixel_block_byte_size = pixel_block_side * pixel_block_side * png2dds::image::bytes_per_pixel;

// Number of blocks to process on each encoding pass.
constexpr std::size_t blocks_to_process = 64UL;

// Temporary storage of pixel blocks stored as contiguous data. Used as the input for encoding.
using pixel_block_array = std::array<std::uint8_t, blocks_to_process * pixel_block_byte_size>;

void get_pixel_blocks(const png2dds::image& png, std::size_t num_blocks, pixel_block_array& pixel_blocks,
	std::size_t pixel_block_x, std::size_t pixel_block_y) {
	assert(pixel_blocks.size() > num_blocks);
	// This function only processes blocks in the same row. The starting vertical coordinate is constant.
	const auto pixel_y = pixel_block_y * pixel_block_side;
	assert(pixel_y < png.padded_height());
	// Blocks will be copied sequentially in the output array.
	auto pixel_block_itr = pixel_blocks.begin();

	for (std::size_t block_x_offset = 0UL; block_x_offset < num_blocks; ++block_x_offset) {
		// The starting x coordinate must be adjusted for each block to copy.
		const auto pixel_x = (pixel_block_x + block_x_offset) * pixel_block_side;
		assert(pixel_x < png.padded_width());
		for (std::size_t offset_y = 0U; offset_y < pixel_block_side; ++offset_y) {
			const std::uint8_t* pixel_start = png.get_pixel(pixel_x, pixel_y + offset_y).data();
			assert(pixel_start < &png.buffer().back());
			const std::uint8_t* pixel_end = png.get_pixel(pixel_x + pixel_block_side, pixel_y + offset_y).data();
			assert(pixel_end <= &*png.buffer().end());
			assert(std::distance(pixel_start, pixel_end) == pixel_block_side * png2dds::image::bytes_per_pixel);
			pixel_block_itr = std::copy(pixel_start, pixel_end, pixel_block_itr);
			assert(pixel_block_itr <= pixel_blocks.end());
		}
	}
}

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

dds_image encoder::encode(const image& png) const {
	assert((png.padded_width() % pixel_block_side) == 0);
	assert((png.padded_height() % pixel_block_side) == 0);

	dds_image dds_img(png);

	// Stores squares of pixel blocks as contiguous data, to serve as the encoding input.
	std::array<std::uint8_t, blocks_to_process * pixel_block_byte_size> pixel_blocks{};

	for (std::size_t block_y = 0UL; block_y < dds_img.height(); ++block_y) {
		for (std::size_t block_x = 0UL; block_x < dds_img.width(); block_x += blocks_to_process) {
			// In some cases the number of blocks of the image may not be divisible by 64.
			const std::size_t current_blocks_to_process = std::min(blocks_to_process, dds_img.width() - block_x);
			// Copy input data as pixel blocks.
			get_pixel_blocks(png, current_blocks_to_process, pixel_blocks, block_x, block_y);

			auto* dds_block = dds_img.block(block_x, block_y);
			ispc::bc7e_compress_blocks(static_cast<std::uint32_t>(current_blocks_to_process), dds_block,
				reinterpret_cast<const std::uint32_t*>(pixel_blocks.data()), _pimpl.get());
		}
	}

	return dds_img;
}

} // namespace png2dds::dds
