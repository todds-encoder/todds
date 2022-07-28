/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/format.hpp"

#include <rgbcx.h>

#include <catch2/catch_test_macros.hpp>

using png2dds::format::max_level;
using png2dds::format::name;
using png2dds::format::type;

TEST_CASE("png2dds::format::name", "[format]") {
	STATIC_REQUIRE(!name(type::bc1).empty());
	STATIC_REQUIRE(!name(type::bc7).empty());
	STATIC_REQUIRE(!name(type::bc1_alpha_bc7).empty());
}

TEST_CASE("png2dds::format::max_level", "[format]") {
	STATIC_REQUIRE(max_level(type::bc1) == rgbcx::MAX_LEVEL);
	STATIC_REQUIRE(max_level(type::bc7) == 6U);
	STATIC_REQUIRE(max_level(type::bc1_alpha_bc7) == 6U);
}