/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/pixel_block_image.hpp"

#include "png2dds/image.hpp"

#include <oneapi/tbb/parallel_for.h>

#include <cassert>

namespace png2dds {
pixel_block_image::pixel_block_image(const image& png)
	: _width{png.padded_width() / block_side}
	, _height{png.padded_height() / block_side}
	, _buffer(png.padded_width() * png.padded_height())
	, _image_width{png.width()}
	, _image_height{png.height()}
	, _file_index{png.file_index()} {
	assert(png.padded_width() % block_side == 0);
	assert(png.padded_height() % block_side == 0);
	assert(png.buffer().size() == _buffer.size() * image::bytes_per_pixel);

	auto* pixel_block_current = reinterpret_cast<std::uint8_t*>(_buffer.data());

	using blocked_range = oneapi::tbb::blocked_range<size_t>;

	oneapi::tbb::parallel_for(blocked_range(0UL, _height), [&](const blocked_range& range) {
		for (std::size_t block_y = range.begin(); block_y < range.end(); ++block_y) {
			const auto pixel_y = block_y * block_side;
			for (std::size_t block_x = 0UL; block_x < _width; ++block_x) {
				const auto pixel_x = block_x * block_side;
				for (std::size_t pixel_offset_y = 0U; pixel_offset_y < block_side; ++pixel_offset_y) {
					const std::uint8_t* pixel_start = png.get_pixel(pixel_x, pixel_y + pixel_offset_y).data();
					assert(pixel_start < &png.buffer().back());
					const std::uint8_t* pixel_end = png.get_pixel(pixel_x + block_side, pixel_y + pixel_offset_y).data();
					assert(pixel_end <= &*png.buffer().end());
					assert(std::distance(pixel_start, pixel_end) == block_side * png2dds::image::bytes_per_pixel);
					pixel_block_current = std::copy(pixel_start, pixel_end, pixel_block_current);
				}
			}
		}
	});
}

std::size_t pixel_block_image::width() const noexcept { return _width; }

std::size_t pixel_block_image::height() const noexcept { return _height; }

std::size_t pixel_block_image::image_width() const noexcept { return _image_width; }

std::size_t pixel_block_image::image_height() const noexcept { return _image_height; }

std::size_t pixel_block_image::file_index() const noexcept { return _file_index; }

pixel_block_image::block pixel_block_image::get_block(std::size_t block_x, std::size_t block_y) const noexcept {
	auto index =
		static_cast<std::ptrdiff_t>(block_x * block_side * block_side + block_y * block_side * block_side * _width);
	return block(_buffer.begin() + index, _buffer.begin() + index + block_side * block_side);
}

} // namespace png2dds