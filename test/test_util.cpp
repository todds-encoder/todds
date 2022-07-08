/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/util.hpp"

#include <catch2/catch.hpp>

TEST_CASE("png2dds::util::next_divisible_by_4", "[util]") {
	using png2dds::util::next_divisible_by_4;
	STATIC_REQUIRE(next_divisible_by_4(1UL) == 4UL);
	STATIC_REQUIRE(next_divisible_by_4(16UL) == 16UL);
	STATIC_REQUIRE(next_divisible_by_4(18UL) == 20UL);
	STATIC_REQUIRE(next_divisible_by_4(31UL) == 32UL);
	STATIC_REQUIRE(next_divisible_by_4(997UL) == 1000UL);
}
