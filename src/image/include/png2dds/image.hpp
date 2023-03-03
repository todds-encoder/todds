/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/memory.hpp"
#include "png2dds/vector.hpp"

#include <cstdint>
#include <span>

namespace png2dds {

/**
 * Image loaded in memory in an RGBA memory layout.
 * The size of the internal buffer is padded to ensure that the number of pixels in the width and the height
 * divisible by 4.
 */
class image final {
public:
	static constexpr std::uint8_t bytes_per_pixel = 4U;

	using buffer_type = png2dds::vector<std::uint8_t>;

	image(std::size_t index, std::size_t width, std::size_t height);
	image(const image&) = delete;
	image(image&&) noexcept = default;
	image& operator=(const image&) = delete;
	image& operator=(image&&) noexcept = default;
	~image() = default;

	/**
	 * Width of the image excluding extra columns.
	 * @return Real width in pixels.
	 */
	// ToDo remove
	[[nodiscard]] std::size_t width() const noexcept;

	/**
	 * Height of the image excluding extra rows.
	 * @return Real height in pixels.
	 */
	// ToDo remove
	[[nodiscard]] std::size_t height() const noexcept;

	/**
	 * Width of the memory buffer.
	 * @return Buffer width in pixels.
	 */
	[[nodiscard]] std::size_t padded_width() const noexcept;

	/**
	 * Height of the memory buffer.
	 * @return Buffer height in pixels.
	 */
	[[nodiscard]] std::size_t padded_height() const noexcept;

	/**
	 * File index of the image in the list of files to load.
	 * @return Index of the file.
	 */
	[[nodiscard]] std::size_t file_index() const noexcept;

	/**
	 * True if the file has alpha and the current pipeline is interested in knowing this value.
	 * Will always be false for pipelines which do not require this information.
	 * @return If the image has alpha or not.
	 */
	 // ToDo remove
	[[nodiscard]] bool encode_as_alpha() const noexcept;

	/**
	 * Marks the image as having alpha.
	 */
	void set_encode_as_alpha() noexcept;

	/**
	 * Memory buffer of the image.
	 * Its size must be padded_width * padded_height * bytes_per_pixel.
	 * @return Internal memory buffer.
	 */
	[[nodiscard]] std::span<std::uint8_t> buffer() noexcept;

	/**
	 * Memory buffer of the image.
	 * Its size must be padded_width * padded_height * bytes_per_pixel.
	 * @return Internal memory buffer.
	 */
	[[nodiscard]] std::span<const std::uint8_t> buffer() const noexcept;

	/**
	 * Reference to the first byte of the first pixel of a row.
	 * @param row Y coordinate of the row.
	 * @return Reference to the accessed byte.
	 */
	[[nodiscard]] std::uint8_t& row_start(std::size_t row) noexcept;

	/**
	 * Write-access to an specific pixel of the image.
	 * @param pixel_x X coordinate of the pixel.
	 * @param pixel_y Y coordinate of the pixel.
	 * @return Span containing the pixel.
	 */
	[[nodiscard]] std::span<std::uint8_t, bytes_per_pixel> get_pixel(std::size_t pixel_x, std::size_t pixel_y) noexcept;

	/**
	 * Read-access to an specific pixel of the image.
	 * @param pixel_x X coordinate of the pixel.
	 * @param pixel_y Y coordinate of the pixel.
	 * @return Span containing the pixel.
	 */
	[[nodiscard]] std::span<const std::uint8_t, bytes_per_pixel> get_pixel(
		std::size_t pixel_x, std::size_t pixel_y) const noexcept;

private:
	std::size_t _padded_width;
	std::size_t _padded_height;
	buffer_type _buffer;
	std::size_t _width; // ToDo remove// ToDo remove
	std::size_t _height; // ToDo remove
	std::size_t _file_index;
	bool _encode_as_alpha; // NOLINT
};

} // namespace png2dds
