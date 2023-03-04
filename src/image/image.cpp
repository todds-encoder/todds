/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/image.hpp"

#include "png2dds/util.hpp"

#include <cassert>

namespace {

constexpr std::size_t get_byte_position(std::size_t padded_width, std::size_t byte_x, std::size_t byte_y) noexcept {
	return byte_x + byte_y * padded_width * png2dds::image::bytes_per_pixel;
}

} // anonymous namespace

namespace png2dds {
image::image(std::size_t width, std::size_t height)
	: _width{util::next_divisible_by_4(width)}
	, _height{util::next_divisible_by_4(height)}
	, _data(static_cast<std::uint8_t*>(nullptr), 0UL) {}

std::size_t image::width() const noexcept { return _width; }

std::size_t image::height() const noexcept { return _height; }

void image::set_data(std::span<std::uint8_t> data) {
	assert(data.size() == _width * _height * image::bytes_per_pixel);
	_data = data;
}

[[nodiscard]] std::span<std::uint8_t> image::data() noexcept { return _data; }

[[nodiscard]] std::span<const std::uint8_t> image::data() const noexcept { return _data; }

std::uint8_t& image::row_start(std::size_t row) noexcept { return _data[get_byte_position(width(), 0UL, row)]; }

std::span<std::uint8_t, image::bytes_per_pixel> image::get_pixel(std::size_t pixel_x, std::size_t pixel_y) noexcept {
	auto index = static_cast<std::ptrdiff_t>(get_byte_position(width(), pixel_x * bytes_per_pixel, pixel_y));
	return std::span<std::uint8_t, image::bytes_per_pixel>(
		_data.begin() + index, _data.begin() + index + bytes_per_pixel);
}

std::span<const std::uint8_t, image::bytes_per_pixel> image::get_pixel(
	std::size_t pixel_x, std::size_t pixel_y) const noexcept {
	auto index = static_cast<std::ptrdiff_t>(get_byte_position(width(), pixel_x * bytes_per_pixel, pixel_y));
	return std::span<const std::uint8_t, image::bytes_per_pixel>(
		_data.begin() + index, _data.begin() + index + bytes_per_pixel);
}

} // namespace png2dds
