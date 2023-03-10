/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/resize.hpp"

#include <cassert>

namespace png2dds {
void box_downscale(const image& source, image& destination) {
	assert((source.width() >> 1U) == destination.width());
	assert((source.height() >> 1U) == destination.height());

	const std::size_t bytes_per_row = destination.width() * image::bytes_per_pixel;
	for (std::size_t pixel_y = 0ULL; pixel_y < destination.height(); ++pixel_y) {
		const std::uint8_t* source_byte_0 = &source.row_start(pixel_y * 2ULL);
		const std::uint8_t* source_byte_1 = &source.row_start(pixel_y * 2ULL + 1ULL);
		std::uint8_t* destination_byte = &destination.row_start(pixel_y);
		for (std::size_t byte_x = 0ULL; byte_x < bytes_per_row; ++byte_x) {
			const std::uint16_t avg = (source_byte_0[0U] + source_byte_0[1U] + source_byte_1[0U] + source_byte_1[1U]) / 4;
			*destination_byte = static_cast<std::uint8_t>(avg);
		}
	}
}
} // namespace png2dds
