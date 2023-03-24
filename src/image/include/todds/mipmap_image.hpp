/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/image.hpp"
#include "todds/vector.hpp"

namespace todds {
/**
 * Array composed of a main image and its mipmaps, loaded in memory in an RGBA memory layout.
 * Each image is padded to keep the width and height divisible by 4.
 * mipmap_image is the owner of the memory used for each image.
 */
class mipmap_image final {
public:
	mipmap_image(std::size_t file_index, std::size_t width, std::size_t height, bool mipmaps);
	mipmap_image(const mipmap_image&) = delete;
	mipmap_image(mipmap_image&&) noexcept = default;
	mipmap_image& operator=(const mipmap_image&) = delete;
	mipmap_image& operator=(mipmap_image&&) noexcept = default;
	~mipmap_image() = default;

	/**
	 * File index of the image in the list of files being loaded by todds.
	 * @return Index of the file.
	 */
	[[nodiscard]] std::size_t file_index() const noexcept;

	/**
	 * Number of mipmaps in the image.
	 * @return Mipmap count.
	 */
	[[nodiscard]] std::size_t mipmap_count() const noexcept;

	[[nodiscard]] const image& get_image(std::size_t index) const noexcept;

	[[nodiscard]] image& get_image(std::size_t index) noexcept;

	[[nodiscard]] std::size_t data_size() const noexcept;

private:
	std::size_t _file_index;
	todds::vector<std::uint8_t> _data;
	todds::vector<image> _images;
};

} // namespace todds
