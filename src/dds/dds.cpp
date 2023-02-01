/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/dds.hpp"

#include "png2dds/dds_image.hpp"

#include <bc7e_ispc.h>
#include <oneapi/tbb/parallel_for.h>
#include <rgbcx.h>

namespace {

// Number of blocks to process on each BC7 encoding pass.
constexpr std::size_t blocks_to_process = 64UL;
} // anonymous namespace

namespace png2dds::dds {

void initialize_bc1_encoding() { rgbcx::init(rgbcx::bc1_approx_mode::cBC1Ideal); }

dds_image bc1_encode(png2dds::format::quality quality, const pixel_block_image& image) {
	dds_image dds_img(image, png2dds::format::type::bc1);
	// Since it allows quality between [0, 18], multiply png2dds's [0, 6] range by 3.
	const auto level = static_cast<unsigned int>(quality) * 3U;

	using blocked_range = oneapi::tbb::blocked_range<size_t>;
	oneapi::tbb::parallel_for(
		blocked_range(0UL, dds_img.height()), [level, &image, &dds_img](const blocked_range& range) {
			for (std::size_t block_y = range.begin(); block_y < range.end(); ++block_y) {
				for (std::size_t block_x = 0UL; block_x < dds_img.width(); ++block_x) {
					auto* dds_block = dds_img.block(block_x, block_y);
					const pixel_block_image::block pixel_block = image.get_block(block_x, block_y);
					rgbcx::encode_bc1(level, dds_block, reinterpret_cast<const std::uint8_t*>(pixel_block.data()), true, true);
				}
			}
		});

	return dds_img;
}

void initialize_bc7_encoding() { ispc::bc7e_compress_block_init(); }

bc7_params bc7_encode_params(png2dds::format::quality quality) noexcept {
	// Perceptual is currently not supported.
	constexpr bool perceptual = false;
	bc7_params params{};
	switch (quality) {
	case format::quality::ultra_fast: ispc::bc7e_compress_block_params_init_ultrafast(&params, perceptual); break;
	case format::quality::very_fast: ispc::bc7e_compress_block_params_init_veryfast(&params, perceptual); break;
	case format::quality::fast: ispc::bc7e_compress_block_params_init_fast(&params, perceptual); break;
	case format::quality::basic: ispc::bc7e_compress_block_params_init_basic(&params, perceptual); break;
	case format::quality::slow: ispc::bc7e_compress_block_params_init_slow(&params, perceptual); break;
	case format::quality::very_slow: ispc::bc7e_compress_block_params_init_veryslow(&params, perceptual); break;
	case format::quality::slowest: ispc::bc7e_compress_block_params_init_slowest(&params, perceptual); break;
	}

	return params;
}

dds_image bc7_encode(const ispc::bc7e_compress_block_params& params, const pixel_block_image& image) {
	dds_image dds_img(image, png2dds::format::type::bc7);

	using blocked_range = oneapi::tbb::blocked_range<size_t>;
	oneapi::tbb::parallel_for(
		blocked_range(0UL, dds_img.height()), [&params, &image, &dds_img](const blocked_range& range) {
			for (std::size_t block_y = range.begin(); block_y < range.end(); ++block_y) {
				for (std::size_t block_x = 0UL; block_x < dds_img.width(); block_x += blocks_to_process) {
					// In some cases the number of blocks of the image may not be divisible by 64.
					const std::size_t current_blocks_to_process = std::min(blocks_to_process, dds_img.width() - block_x);
					auto* dds_block = dds_img.block(block_x, block_y);
					const pixel_block_image::block pixel_block = image.get_block(block_x, block_y);
					ispc::bc7e_compress_blocks(
						static_cast<std::uint32_t>(current_blocks_to_process), dds_block, pixel_block.data(), &params);
				}
			}
		});

	return dds_img;
}

} // namespace png2dds::dds
