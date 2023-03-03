/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/image.hpp"

#include "png2dds/util.hpp"

namespace {

constexpr std::size_t get_byte_position(std::size_t padded_width, std::size_t byte_x, std::size_t byte_y) noexcept {
	return byte_x + byte_y * padded_width * png2dds::image::bytes_per_pixel;
}

} // anonymous namespace

namespace png2dds {
image::image(std::size_t index, std::size_t width, std::size_t height)
	: _padded_width{util::next_divisible_by_4(width)}
	, _padded_height{util::next_divisible_by_4(height)}
	, _buffer(_padded_width * _padded_height * bytes_per_pixel)
	, _width{width}
	, _height{height}
	, _file_index{index}
	, _encode_as_alpha{false} {}

std::size_t image::width() const noexcept { return _width; }

std::size_t image::height() const noexcept { return _height; }

std::size_t image::padded_width() const noexcept { return _padded_width; }

std::size_t image::padded_height() const noexcept { return _padded_height; }

bool image::encode_as_alpha() const noexcept { return _encode_as_alpha; }

void image::set_encode_as_alpha() noexcept { _encode_as_alpha = true; }

std::size_t image::file_index() const noexcept { return _file_index; }

[[nodiscard]] std::span<std::uint8_t> image::buffer() noexcept { return _buffer; }

[[nodiscard]] std::span<const std::uint8_t> image::buffer() const noexcept { return _buffer; }

std::uint8_t& image::row_start(std::size_t row) noexcept {
	return _buffer[get_byte_position(padded_width(), 0UL, row)];
}

std::span<std::uint8_t, image::bytes_per_pixel> image::get_pixel(std::size_t pixel_x, std::size_t pixel_y) noexcept {
	auto index = static_cast<std::ptrdiff_t>(get_byte_position(padded_width(), pixel_x * bytes_per_pixel, pixel_y));
	return std::span<std::uint8_t, image::bytes_per_pixel>(
		_buffer.begin() + index, _buffer.begin() + index + bytes_per_pixel);
}

std::span<const std::uint8_t, image::bytes_per_pixel> image::get_pixel(
	std::size_t pixel_x, std::size_t pixel_y) const noexcept {
	auto index = static_cast<std::ptrdiff_t>(get_byte_position(padded_width(), pixel_x * bytes_per_pixel, pixel_y));
	return std::span<const std::uint8_t, image::bytes_per_pixel>(
		_buffer.begin() + index, _buffer.begin() + index + bytes_per_pixel);
}

} // namespace png2dds
