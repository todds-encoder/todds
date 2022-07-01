/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/util.hpp"

#include <catch2/catch.hpp>

TEST_CASE("png2dds::util::next_divisible_by_16", "[util]") {
	using png2dds::util::next_divisible_by_16;
	STATIC_REQUIRE(next_divisible_by_16(1UL) == 16UL);
	STATIC_REQUIRE(next_divisible_by_16(16UL) == 16UL);
	STATIC_REQUIRE(next_divisible_by_16(18UL) == 32UL);
	STATIC_REQUIRE(next_divisible_by_16(31UL) == 32UL);
	STATIC_REQUIRE(next_divisible_by_16(32UL) == 32UL);
	STATIC_REQUIRE(next_divisible_by_16(999UL) == 1008UL);
}
