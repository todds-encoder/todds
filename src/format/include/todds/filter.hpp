/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string_view>

namespace todds::filter {

/**
 * These downscaling filters for mipmaps correspond to values of cv::InterpolationFlags in OpenCV.
 * Linear filters are omitted because hal::resize switches to area when the destination image is half of the source.
 */
enum class type : std::uint8_t {
	nearest = 0U,
	cubic = 2U,
	area = 3U,
	lanczos = 4U,
};

[[nodiscard]] constexpr std::string_view name(type flt) noexcept {
	std::string_view name_str{};
	switch (flt) {
	case type::nearest: name_str = "NEAREST"; break;
	case type::cubic: name_str = "CUBIC"; break;
	case type::area: name_str = "AREA"; break;
	case type::lanczos: name_str = "LANCZOS"; break;
	}
	return name_str;
}

} // namespace todds::mipmap_filter
