/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/image.hpp"

#include <type_traits>

#include <catch2/catch.hpp>

using png2dds::image;

TEST_CASE("png2dds::image type trait checks", "[image]") {
	// Image is a move-only type.
	STATIC_REQUIRE(std::is_nothrow_move_constructible_v<image>);
	STATIC_REQUIRE(std::is_nothrow_move_assignable_v<image>);
	STATIC_REQUIRE(!std::is_copy_constructible_v<image>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<image>);
	STATIC_REQUIRE(std::is_nothrow_destructible_v<image>);
	STATIC_REQUIRE(std::is_nothrow_swappable_v<image>);
}

TEST_CASE("png2dds::image type assumptions", "[image]") {
	// Image stores R, G, B and A bytes for each pixel.
	STATIC_REQUIRE(image::bytes_per_pixel == 4U);
}

TEST_CASE("png2dds::image construction", "[image]") {
	constexpr std::size_t width = 1024;
	constexpr std::size_t height = 8888;
	const image img{width, height};
	REQUIRE(img.width() == width);
	REQUIRE(img.height() == height);
	REQUIRE(img.size() == width * height * 4U);
	REQUIRE(img.buffer() != nullptr);
}
