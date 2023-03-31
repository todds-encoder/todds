/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string_view>

namespace todds::filter {

/**
 * Scaling interpolation filters. They correspond to values of cv::InterpolationFlags in OpenCV.
 */
enum class type : std::uint8_t {
	nearest = 0U,
	linear = 1U,
	cubic = 2U,
	area = 3U,
	lanczos = 4U,
};

[[nodiscard]] constexpr std::string_view name(type flt) noexcept {
	std::string_view name_str{};
	switch (flt) {
	case type::nearest: name_str = "NEAREST"; break;
	case type::linear: name_str = "LINEAR"; break;
	case type::cubic: name_str = "CUBIC"; break;
	case type::area: name_str = "AREA"; break;
	case type::lanczos: name_str = "LANCZOS"; break;
	}
	return name_str;
}

[[nodiscard]] constexpr std::string_view description(type flt) noexcept {
	std::string_view desc_str{};
	switch (flt) {
	case type::nearest:
		desc_str = "Nearest neighbor interpolation. Very fast, but it does not produce great results.";
		break;
	case type::linear: desc_str = "Bilinear interpolation. Fast, and with reasonable quality."; break;
	case type::cubic: desc_str = "Bicubic interpolation. Recommended filter for upscaling images."; break;
	case type::area:
		desc_str = "Resampling using pixel area relation. Good for downscaling images and mipmap generation.";
		break;
	case type::lanczos:
		desc_str = "Lanczos interpolation. Preserves edges and details better than other filters when dowsncaling images.";
		break;
	}
	return desc_str;
}

} // namespace todds::filter
