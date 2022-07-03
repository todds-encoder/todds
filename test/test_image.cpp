/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/image.hpp"
#include "png2dds/util.hpp"

#include <limits>
#include <numeric>
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
	using png2dds::util::next_divisible_by_16;
	constexpr std::size_t width = 1023UL;
	constexpr std::size_t padded_width = next_divisible_by_16(width);
	constexpr std::size_t height = 8888UL;
	constexpr std::size_t padded_height = next_divisible_by_16(height);
	const image img{width, height};
	REQUIRE(img.width() == width);
	REQUIRE(img.height() == height);
	REQUIRE(img.padded_width() == padded_width);
	REQUIRE(img.padded_height() == padded_height);
	REQUIRE(img.buffer().size() == padded_width * padded_height * image::bytes_per_pixel);
}

image iota_image() {
	constexpr std::size_t width = 16UL * 10UL;
	constexpr std::size_t height = 16UL * 10UL;
	image img{width, height};
	std::iota(img.buffer().begin(), img.buffer().end(), 0U);
	return img;
}

TEST_CASE("png2dds::image const buffer access", "[image]") {
	image img = iota_image();
	std::span<const std::uint8_t> buffer = img.buffer();
	REQUIRE(buffer.front() == 0U);
	REQUIRE(buffer.back() == std::numeric_limits<std::uint8_t>::max());
}

TEST_CASE("png2dds::image buffer access", "[image]") {
	image img = iota_image();
	std::span<std::uint8_t> buffer = img.buffer();
	buffer.back() = 0U;
	REQUIRE(buffer.front() == 0U);
	REQUIRE(buffer.back() == 0U);
}
