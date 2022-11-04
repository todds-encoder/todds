/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/format.hpp"
#include "png2dds/memory.hpp"
#include "png2dds/pixel_block_image.hpp"
#include "png2dds/vector.hpp"

#include <array>

namespace png2dds {

/** Image in which each pixel is a block of one or more uint64_t. */
class dds_image final {
public:
	/** Underlying buffer which stores the blocks. */
	using buffer_type = png2dds::vector<std::uint64_t>;

	/** DDS header for this image. Does not include header extensions, if any. */
	using header_type = std::array<char, 124>;

	dds_image();
	explicit dds_image(const pixel_block_image& image, format::type format_type);
	dds_image(const dds_image&) = delete;
	dds_image(dds_image&&) = default;
	dds_image& operator=(const dds_image&) = delete;
	dds_image& operator=(dds_image&&) = default;

	~dds_image() = default;

	[[nodiscard]] const header_type& header() const noexcept;
	[[nodiscard]] std::size_t width() const noexcept;
	[[nodiscard]] std::size_t height() const noexcept;
	[[nodiscard]] std::uint64_t* block(std::size_t block_x, std::size_t block_y) noexcept;
	[[nodiscard]] const buffer_type& blocks() const noexcept;
	[[nodiscard]] std::size_t file_index() const noexcept;
	[[nodiscard]] format::type format() const noexcept;

private:
	std::size_t _block_offset;
	std::size_t _width;
	std::size_t _height;
	buffer_type _blocks;
	header_type _header;
	std::size_t _file_index;
	format::type _format;
};

} // namespace png2dds
