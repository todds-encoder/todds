/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/image_types.hpp"

#include "png2dds/mipmap_image.hpp"

#include <cassert>

namespace png2dds {
pixel_block_image to_pixel_blocks(const mipmap_image& img) {
	// Every mipmap level will be stored together in a contiguous vector of pixel blocks.
	pixel_block_image buffer(img.data_size());
	auto* pixel_block_current = reinterpret_cast<std::uint8_t*>(buffer.data());

	for (std::size_t level_index{}; level_index < img.mipmap_count(); ++level_index) {
		const image& level = img.get_image(level_index);
		const std::size_t width_blocks = level.width() / pixel_block_side;
		const std::size_t height_blocks = level.height() / pixel_block_side;

		for (std::size_t block_y = 0U; block_y < height_blocks; ++block_y) {
			const auto pixel_y = block_y * pixel_block_side;
			for (std::size_t block_x = 0UL; block_x < width_blocks; ++block_x) {
				const auto pixel_x = block_x * pixel_block_side;
				for (std::size_t pixel_offset_y = 0U; pixel_offset_y < pixel_block_side; ++pixel_offset_y) {
					const std::uint8_t* pixel_start = level.get_pixel(pixel_x, pixel_y + pixel_offset_y).data();
					assert(pixel_start < &level.data().back());
					const std::uint8_t* pixel_end = level.get_pixel(pixel_x + pixel_block_side, pixel_y + pixel_offset_y).data();
					assert(pixel_end <= &*level.data().end());
					assert(std::distance(pixel_start, pixel_end) == pixel_block_side * png2dds::image::bytes_per_pixel);
					pixel_block_current = std::copy(pixel_start, pixel_end, pixel_block_current);
				}
			}
		}
	}

	return buffer;
}

} // namespace png2dds
