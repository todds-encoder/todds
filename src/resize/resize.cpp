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

	for (std::size_t destination_y = 0ULL; destination_y < destination.height(); ++destination_y) {
		std::uint8_t* destination_byte = &destination.row_start(destination_y);

		const std::size_t source_y = destination_y * 2ULL;
		const std::uint8_t* source_byte_0 = &source.row_start(source_y);
		const std::uint8_t* source_byte_1 = &source.row_start(source_y + 1ULL);

		for (std::size_t destination_x = 0ULL; destination_x < destination.width(); ++destination_x) {
			for (std::size_t byte = 0ULL; byte < image::bytes_per_pixel; ++byte) {
				std::uint32_t avg = source_byte_0[0U];
				avg += source_byte_0[image::bytes_per_pixel];
				avg += source_byte_1[0U];
				avg += source_byte_1[image::bytes_per_pixel];
				avg /= 4U;
				*destination_byte = static_cast<std::uint8_t>(avg);
				++destination_byte;
				++source_byte_0;
				++source_byte_1;
			}
			source_byte_0 += image::bytes_per_pixel;
			source_byte_1 += image::bytes_per_pixel;
		}
	}
}
} // namespace png2dds
