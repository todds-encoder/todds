/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_PNG_HPP
#define PNG2DDS_PNG_HPP

#include "png2dds/dds_image.hpp"
#include "png2dds/image.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace png2dds::png {
/**
 * Decodes a PNG file stored in a memory buffer.
 * @param file_index File index of the image in the list of files to load.
 * @param png Path to the PNG file, used for reporting errors.
 * @param buffer Memory buffer holding a PNG file read from the filesystem.
 * @param flip Flip source image vertically during decoding.
 * @return Decoded PNG image. Width and height are increased if needed to make their number of pixels divisible by 4.
 */
image decode(std::size_t file_index, const std::string& png, const std::vector<std::uint8_t>& buffer, bool flip);

class encode_buffer final {
public:
	encode_buffer(void* memory, std::size_t size);

	encode_buffer(const encode_buffer&) = delete;
	encode_buffer(encode_buffer&&) = default;
	encode_buffer& operator=(const encode_buffer&) = delete;
	encode_buffer& operator=(encode_buffer&&) = default;
	~encode_buffer();

	[[nodiscard]] std::span<const char> span() const noexcept;

private:
	void* _memory;
	std::size_t _size;
};

/**
 * Encodes an image as a PNG and stores it into a buffer.
 * @param png Path to the destination PNG file, used for reporting errors.
 * @param img Image in memory. Padding is not removed by this function.
 * @return Memory buffer holding the contents of a PNG file.
 */
encode_buffer encode(const std::string& png, const image& img);

} // namespace png2dds::png

#endif // PNG2DDS_PNG_HPP
