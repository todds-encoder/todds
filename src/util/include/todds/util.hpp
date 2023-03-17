/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstddef>

namespace todds::util {
/**
 * Provides the smallest value divisible by 4 larger than the input.
 * @param value Input value.
 * @return Number larger than input and divisible by 4.
 */
[[nodiscard]] constexpr std::size_t next_divisible_by_4(std::size_t value) noexcept { return (value + 0b11U) & ~0b11U; }
} // namespace todds::util
