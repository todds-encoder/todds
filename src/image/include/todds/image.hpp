/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/memory.hpp"

#include <opencv2/core.hpp>

#include <cstdint>
#include <span>

namespace todds {

/**
 * Image loaded in memory in an RGBA memory layout.
 * Image is only a view and is not the owner of the memory. See mipmap_image for details.
 */
class image final {
public:
	static constexpr std::uint8_t bytes_per_pixel = 4U;

	image(std::size_t width, std::size_t height);
	image(const image&) = delete;
	image(image&&) noexcept = default;
	image& operator=(const image&) = delete;
	image& operator=(image&&) noexcept = default;
	~image() = default;

	/**
	 * Original width of the image in pixels.
	 * @return Width of this image.
	 */
	[[nodiscard]] std::size_t width() const noexcept;

	/**
	 * Original height of the image in pixels.
	 * @return Height of this image.
	 */
	[[nodiscard]] std::size_t height() const noexcept;

	void set_data(std::span<std::uint8_t> data);

	/**
	 * Memory data of the image.
	 * Its size must be padded_width * padded_height * bytes_per_pixel.
	 * @return Internal memory data.
	 */
	[[nodiscard]] std::span<std::uint8_t> data() noexcept;

	/**
	 * Memory data of the image.
	 * @return Internal memory data.
	 */
	[[nodiscard]] std::span<const std::uint8_t> data() const noexcept;

	/**
	 * Reference to the first byte of the first pixel of a row.
	 * @param row Y coordinate of the row.
	 * @return Reference to the accessed byte.
	 */
	[[nodiscard]] const std::uint8_t& row_start(std::size_t row) const noexcept;

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

	explicit operator cv::Mat() {
		constexpr auto image_type = CV_8UC4; // NOLINT
		return {static_cast<int>(height()), static_cast<int>(width()), image_type, static_cast<void*>(_data.data())};
	}

private:
	std::size_t _width;
	std::size_t _height;
	std::span<std::uint8_t> _data;
};

} // namespace todds
