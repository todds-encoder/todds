/*
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
* distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "png2dds/pixel_block_image.hpp"

#include <type_traits>

#include <catch2/catch_test_macros.hpp>

using png2dds::pixel_block_image;

TEST_CASE("png2dds::pixel_block_image type trait checks", "[pixel_block_image]") {
	// pixel_block_image is a move-only type.
	STATIC_REQUIRE(std::is_nothrow_move_constructible_v<pixel_block_image>);
	STATIC_REQUIRE(std::is_nothrow_move_assignable_v<pixel_block_image>);
	STATIC_REQUIRE(!std::is_copy_constructible_v<pixel_block_image>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<pixel_block_image>);
	STATIC_REQUIRE(std::is_nothrow_destructible_v<pixel_block_image>);
	STATIC_REQUIRE(std::is_nothrow_swappable_v<pixel_block_image>);
}
