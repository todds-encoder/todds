/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "todds/image_types.hpp"

#include "todds/mipmap_image.hpp"
#include "todds/util.hpp"

namespace {
constexpr auto to_next_block = todds::pixel_block_side * todds::image::bytes_per_pixel;

}

namespace todds {

// Every mipmap level will be stored together in a contiguous vector of pixel blocks.
// pixel_block_image stores entire RGBA pixels inside of a single std::uint32_t value.
pixel_block_image to_pixel_blocks(const mipmap_image& img) {
	using todds::util::next_divisible_by_4;

	// Calculate the total number of pixels of all mipmap levels and alocate a pixel block image to store all of them.
	std::size_t block_image_size{};
	for (std::size_t level_index{}; level_index < img.mipmap_count(); ++level_index) {
		const image& level = img.get_image(level_index);
		block_image_size += (next_divisible_by_4(level.width()) * next_divisible_by_4(level.height()));
	}
	pixel_block_image buffer(block_image_size);

	auto* pixel_block_current = reinterpret_cast<std::uint8_t*>(buffer.data());

	for (std::size_t level_index{}; level_index < img.mipmap_count(); ++level_index) {
		const image& level = img.get_image(level_index);
		const std::size_t padded_width = next_divisible_by_4(level.width());
		const std::size_t padded_height = next_divisible_by_4(level.height());
		const std::size_t width_blocks = padded_width / pixel_block_side;
		const std::size_t height_blocks = padded_height / pixel_block_side;

		const std::size_t max_y = level.height() - 1UL;
		for (std::size_t block_y = 0U; block_y < height_blocks; ++block_y) {
			const auto pixel_y = block_y * pixel_block_side;
			// Each block is created by alternating pixel from each one of these four rows.
			const std::uint8_t* row_0 = &level.row_start(std::min(max_y, pixel_y));
			const std::uint8_t* row_1 = &level.row_start(std::min(max_y, pixel_y + 1UL));
			const std::uint8_t* row_2 = &level.row_start(std::min(max_y, pixel_y + 2UL));
			const std::uint8_t* row_3 = &level.row_start(std::min(max_y, pixel_y + 3UL));

			// ToDo WIP implementation of the new algorithm. Fix out of bounds error.
			for (std::size_t block_x = 0UL; block_x < width_blocks; ++block_x) {
				pixel_block_current = std::copy(row_0, row_0 + to_next_block, pixel_block_current);
				row_0 += to_next_block;
				pixel_block_current = std::copy(row_1, row_1 + to_next_block, pixel_block_current);
				row_1 += to_next_block;
				pixel_block_current = std::copy(row_2, row_2 + to_next_block, pixel_block_current);
				row_2 += to_next_block;
				pixel_block_current = std::copy(row_3, row_3 + to_next_block, pixel_block_current);
				row_3 += to_next_block;
			}
		}
	}

	return buffer;

}

} // namespace todds
