/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/image_types.hpp"

#include <cassert>

namespace png2dds {
pixel_block_image to_pixel_blocks(const image& png, std::size_t padded_width, std::size_t padded_height) {
	pixel_block_image buffer(padded_width * padded_height);
	auto* pixel_block_current = reinterpret_cast<std::uint8_t*>(buffer.data());

	const std::size_t width_blocks = padded_width / pixel_block_side;
	const std::size_t height_blocks = padded_height / pixel_block_side;

	for (std::size_t block_y = 0U; block_y < height_blocks; ++block_y) {
		const auto pixel_y = block_y * pixel_block_side;
		for (std::size_t block_x = 0UL; block_x < width_blocks; ++block_x) {
			const auto pixel_x = block_x * pixel_block_side;
			for (std::size_t pixel_offset_y = 0U; pixel_offset_y < pixel_block_side; ++pixel_offset_y) {
				const std::uint8_t* pixel_start = png.get_pixel(pixel_x, pixel_y + pixel_offset_y).data();
				assert(pixel_start < &png.buffer().back());
				const std::uint8_t* pixel_end = png.get_pixel(pixel_x + pixel_block_side, pixel_y + pixel_offset_y).data();
				assert(pixel_end <= &*png.buffer().end());
				assert(std::distance(pixel_start, pixel_end) == pixel_block_side * png2dds::image::bytes_per_pixel);
				pixel_block_current = std::copy(pixel_start, pixel_end, pixel_block_current);
			}
		}
	}

	return buffer;
}

} // namespace png2dds
