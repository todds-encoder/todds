/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_IMAGE_HPP
#define PNG2DDS_IMAGE_HPP

#include <cstdint>
#include <vector>

namespace png2dds {

class image final {
public:
	static constexpr std::uint8_t bytes_per_pixel = 4U;

	image(std::size_t width, std::size_t height);
	image(const image&) = delete;
	image(image&&) noexcept = default;
	image& operator=(const image&) = delete;
	image& operator=(image&&) noexcept = default;
	~image() = default;

	[[nodiscard]] std::size_t width() const noexcept;
	[[nodiscard]] std::size_t height() const noexcept;
	[[nodiscard]] std::size_t size() const noexcept;
	[[nodiscard]] std::uint8_t* buffer() noexcept;
	[[nodiscard]] const std::uint8_t* buffer() const noexcept;

private:
	std::vector<std::uint8_t> _buffer;
	std::size_t _width;
	std::size_t _height;
};

} // namespace png2dds

#endif // PNG2DDS_IMAGE_HPP
