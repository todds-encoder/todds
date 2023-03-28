/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "todds/mipmap_image.hpp"

#include <cassert>

namespace todds {

mipmap_image::mipmap_image(std::size_t file_index, std::size_t width, std::size_t height, bool mipmaps)
	: _file_index{file_index}
	, _data{}
	, _images{} {
	std::size_t pixels_required{};

	// The first image is always included.
	_images.emplace_back(width, height);
	pixels_required += _images.back().width() * _images.back().height();

	if (mipmaps) {
		constexpr std::size_t minimum_size = 1ULL;
		while (width > minimum_size || height > minimum_size) {
			if (width > minimum_size) { width >>= 1ULL; }
			if (height > minimum_size) { height >>= 1ULL; }
			_images.emplace_back(width, height);
			pixels_required += _images.back().width() * _images.back().height();
		}
	}

	if (pixels_required == 0ULL) { return; }

	// Allocate the memory required for every image in a single contiguous array.
	_data = todds::vector<std::uint8_t>(pixels_required * image::bytes_per_pixel);

	std::size_t memory_start = 0ULL;
	// Point each image to its memory chunk.
	for (image& current : _images) {
		const std::size_t required_memory = current.width() * current.height() * image::bytes_per_pixel;
		current.set_data(std::span<std::uint8_t>(&_data[memory_start], required_memory));
		memory_start += required_memory;
	}

	assert(memory_start == _data.size());
}

mipmap_image::mipmap_image(const mipmap_image& other)
	: _file_index{other.file_index()}
	, _data(other._data.size())
	, _images{} {
	std::size_t memory_start = 0ULL;
	for (const auto& original_img : other._images) {
		_images.emplace_back(original_img.width(), original_img.height());
		image& current = _images.back();
		const std::size_t required_memory = current.width() * current.height() * image::bytes_per_pixel;
		current.set_data(std::span<std::uint8_t>(&_data[memory_start], required_memory));
		memory_start += required_memory;
	}
	assert(memory_start == _data.size());
}

[[nodiscard]] std::size_t mipmap_image::file_index() const noexcept { return _file_index; }

[[nodiscard]] std::size_t mipmap_image::mipmap_count() const noexcept { return _images.size(); }

[[nodiscard]] const todds::image& mipmap_image::get_image(std::size_t index) const noexcept { return _images[index]; }

[[nodiscard]] todds::image& mipmap_image::get_image(std::size_t index) noexcept { return _images[index]; }

[[nodiscard]] std::size_t mipmap_image::data_size() const noexcept { return _data.size(); }

} // namespace todds
