/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_PIXEL_BLOCK_IMAGE_HPP
#define PNG2DDS_PIXEL_BLOCK_IMAGE_HPP

#include "png2dds/image.hpp"
#include "png2dds/memory.hpp"
#include "png2dds/vector.hpp"

#include <cstdint>
#include <span>

namespace png2dds {

class pixel_block_image final {
public:
	// Pixel blocks are squares of size block_side * block_side.
	static constexpr std::size_t block_side = 4UL;

	using block = std::span<const std::uint32_t, block_side * block_side>;

	explicit pixel_block_image(const image& png);
	pixel_block_image(const pixel_block_image&) = delete;
	pixel_block_image(pixel_block_image&&) = default;
	pixel_block_image& operator=(const pixel_block_image&) = delete;
	pixel_block_image& operator=(pixel_block_image&&) = default;

	~pixel_block_image() = default;

	[[nodiscard]] std::size_t width() const noexcept;
	[[nodiscard]] std::size_t height() const noexcept;
	[[nodiscard]] std::size_t image_width() const noexcept;
	[[nodiscard]] std::size_t image_height() const noexcept;
	[[nodiscard]] std::size_t file_index() const noexcept;
	[[nodiscard]] block get_block(std::size_t block_x, std::size_t block_y) const noexcept;

private:
	std::size_t _width;
	std::size_t _height;
	png2dds::vector<std::uint32_t> _buffer;
	std::size_t _image_width;
	std::size_t _image_height;
	std::size_t _file_index;
};

} // namespace png2dds

#endif // PNG2DDS_PIXEL_BLOCK_IMAGE_HPP
