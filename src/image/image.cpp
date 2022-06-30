/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/image.hpp"

namespace png2dds {
image::image(std::size_t width, std::size_t height)
	: _buffer(width * height * bytes_per_pixel)
	, _width{width}
	, _height{height} {}

std::size_t image::width() const noexcept { return _width; }

std::size_t image::height() const noexcept { return _height; }

std::size_t image::size() const noexcept { return _buffer.size(); }

const std::uint8_t* image::buffer() const noexcept { return _buffer.data(); }

std::uint8_t* image::buffer() noexcept { return _buffer.data(); }

} // namespace png2dds