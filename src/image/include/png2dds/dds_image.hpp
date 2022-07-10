/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_DDS_IMAGE_HPP
#define PNG2DDS_DDS_IMAGE_HPP

#include "png2dds/image.hpp"

#include <array>
#include <vector>

namespace png2dds {

class dds_image final {
public:
	using block_type = std::array<std::uint64_t, 2U>;
	using header_type = std::array<char, 144U>;

	// Pixel blocks are squares of size block_side * block_side.
	static constexpr std::size_t pixel_block_side = 4UL;

	explicit dds_image(const image& png);
	dds_image(const dds_image&) = delete;
	dds_image(dds_image&&) = default;
	dds_image& operator=(const dds_image&) = delete;
	dds_image& operator=(dds_image&&) = default;

	~dds_image() = default;

	[[nodiscard]] const header_type& header() const noexcept;
	[[nodiscard]] std::size_t width() const noexcept;
	[[nodiscard]] std::size_t height() const noexcept;
	[[nodiscard]] block_type::value_type* block(std::size_t block_x, std::size_t block_y) noexcept;
	[[nodiscard]] const std::vector<block_type>& blocks() const noexcept;
	[[nodiscard]] std::size_t file_index() const noexcept;

private:
	std::size_t _width;
	std::size_t _height;
	std::vector<block_type> _blocks;
	header_type _header;
	std::size_t _file_index;
};

} // namespace png2dds

#endif // PNG2DDS_DDS_IMAGE_HPP
