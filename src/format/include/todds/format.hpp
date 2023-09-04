/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace todds::format {

enum class type : std::uint8_t { bc1, bc7, png, invalid };

enum class quality : std::uint8_t {
	ultra_fast = 0U,
	minimum = 0U,
	very_fast = 1U,
	fast = 2U,
	basic = 3U,
	slow = 4U,
	very_slow = 5U,
	really_slow = 6U,
	slowest = 7U,
	maximum = 7U
};

[[nodiscard]] constexpr std::string_view name(type fmt) noexcept {
	std::string_view name_str{};
	switch (fmt) {
	case type::bc1: name_str = "BC1"; break;
	case type::bc7: name_str = "BC7"; break;
	case type::png: name_str = "PNG"; break;
	case type::invalid: break;
	}
	return name_str;
}

[[nodiscard]] constexpr bool has_alpha(type fmt) noexcept { return fmt == type::bc7 || fmt == type::png; }

} // namespace todds::format
