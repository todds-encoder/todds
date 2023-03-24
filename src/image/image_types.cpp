/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "todds/image_types.hpp"

#include "todds/mipmap_image.hpp"
#include "todds/util.hpp"

namespace {

constexpr auto to_next_block = todds::pixel_block_side * todds::image::bytes_per_pixel;

const std::uint8_t* row_start_address(const todds::image& level, std::size_t input_row) {
	const std::size_t row_index = std::min(level.height() - 1UL, input_row);
	const std::uint8_t* address = &level.row_start(row_index);
	assert(address < &level.data().back());
	return address;
}

} // Anonymous namespace

namespace todds {

// Every mipmap level will be stored together in a contiguous vector of pixel blocks.
// pixel_block_image stores entire RGBA pixels inside of a single std::uint32_t value.
pixel_block_image to_pixel_blocks(const mipmap_image& img) {
	using todds::util::next_divisible_by_4;

	// Allocate a pixel block image to store all mipmaps including with extra padding.
	std::size_t block_image_size{};
	for (std::size_t level_index{}; level_index < img.mipmap_count(); ++level_index) {
		const image& level = img.get_image(level_index);
		block_image_size += (next_divisible_by_4(level.width()) * next_divisible_by_4(level.height()));
	}
	pixel_block_image buffer(block_image_size);

	auto* pixel_block_current = reinterpret_cast<std::uint8_t*>(buffer.data());
#if !defined(NDEBUG)
	auto* buffer_end = reinterpret_cast<std::uint8_t*>(&buffer.back());
	auto buffer_size_in_bytes = static_cast<std::size_t>(buffer_end - pixel_block_current);
	assert(buffer_size_in_bytes > img.data_size());
#endif

	for (std::size_t level_index{}; level_index < img.mipmap_count(); ++level_index) {
		const image& level = img.get_image(level_index);
		const std::size_t padded_width = next_divisible_by_4(level.width());
		const std::size_t padded_height = next_divisible_by_4(level.height());
		const std::size_t width_blocks = padded_width / pixel_block_side;
		const std::size_t height_blocks = padded_height / pixel_block_side;

		const std::size_t width_complete_blocks = (level.width() % 4) == 0 ? width_blocks : width_blocks - 1UL;

		for (std::size_t block_y = 0U; block_y < height_blocks; ++block_y) {
			const auto input_row = block_y * pixel_block_side;
			assert(input_row < level.height());

			// Each matrix of 4x4 pixels in the input image will become a contiguous block in the pixel block image.
			// So we must copy 4 pixel of each row alternatively to construct these blocks.
			// When the height is not divisible by 4, the last row will be used to fill in the extra padding.
			const std::uint8_t* row_0 = row_start_address(level, input_row);
			const std::uint8_t* row_1 = row_start_address(level, input_row + 1UL);
			const std::uint8_t* row_2 = row_start_address(level, input_row + 2UL);
			const std::uint8_t* row_3 = row_start_address(level, input_row + 3UL);
			assert(row_3 < &level.data().back());

			for (std::size_t block_x = 0UL; block_x < width_complete_blocks; ++block_x) {
				pixel_block_current = std::copy(row_0, row_0 + to_next_block, pixel_block_current);
				row_0 += to_next_block;
				assert(pixel_block_current < buffer_end);
				pixel_block_current = std::copy(row_1, row_1 + to_next_block, pixel_block_current);
				row_1 += to_next_block;
				assert(pixel_block_current < buffer_end);
				pixel_block_current = std::copy(row_2, row_2 + to_next_block, pixel_block_current);
				row_2 += to_next_block;
				assert(pixel_block_current < buffer_end);
				pixel_block_current = std::copy(row_3, row_3 + to_next_block, pixel_block_current);
				row_3 += to_next_block;
				assert(pixel_block_current <= buffer_end);
			}

			// When the width is not divisible by 4, there is an extra block to calculate with incomplete information.
			// The border pixel is copied to this additional padding.
			// To do this, row_X variables are no longer increased. Instead we use offsets that increase as long as there is
			// still remaining information, but then stop and always copy the last pixel.
			if (width_complete_blocks != width_blocks) [[unlikely]] {
				// Pixels that have not been copied yet.
				const std::size_t last_pixel_x = level.width() - 1UL - width_complete_blocks * pixel_block_side;
				for (std::size_t pixel_x = 0UL; pixel_x < todds::pixel_block_side; ++pixel_x) {
					const std::size_t position_offset = std::min(last_pixel_x, pixel_x) * image::bytes_per_pixel;
					pixel_block_current =
						std::copy(row_0 + position_offset, row_0 + position_offset + image::bytes_per_pixel, pixel_block_current);
					pixel_block_current =
						std::copy(row_1 + position_offset, row_1 + position_offset + image::bytes_per_pixel, pixel_block_current);
					pixel_block_current =
						std::copy(row_2 + position_offset, row_2 + position_offset + image::bytes_per_pixel, pixel_block_current);
					pixel_block_current =
						std::copy(row_3 + position_offset, row_3 + position_offset + image::bytes_per_pixel, pixel_block_current);
				}
			}
		}
	}

	return buffer;
}

} // namespace todds
