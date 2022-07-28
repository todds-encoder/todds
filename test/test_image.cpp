/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/image.hpp"

#include <array>
#include <limits>
#include <numeric>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

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
	// png2dds assumes that spans with a static extent have a single member.
	STATIC_REQUIRE(sizeof(decltype(static_cast<image*>(nullptr)->get_pixel(0UL, 0UL))) == sizeof(std::uint8_t*));
	STATIC_REQUIRE(sizeof(decltype(static_cast<const image*>(nullptr)->get_pixel(0UL, 0UL))) == sizeof(std::uint8_t*));
}

TEST_CASE("png2dds::image construction", "[image]") {
	constexpr std::size_t file_index = 88838UL;
	constexpr std::size_t width = 1023UL;
	constexpr std::size_t padded_width = 1024UL;
	constexpr std::size_t height = 8887UL;
	constexpr std::size_t padded_height = 8888UL;
	const image img{file_index, width, height};
	REQUIRE(img.file_index() == file_index);
	REQUIRE(img.width() == width);
	REQUIRE(img.height() == height);
	REQUIRE(img.padded_width() == padded_width);
	REQUIRE(img.padded_height() == padded_height);
	REQUIRE(img.buffer().size() == padded_width * padded_height * image::bytes_per_pixel);
	REQUIRE(!img.encode_as_alpha());
}

constexpr std::size_t iota_image_side = 16UL * 10UL;
image iota_image() {
	image img{0U, iota_image_side, iota_image_side};
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

TEST_CASE("png2dds::image byte access", "[image]") {
	const image img = iota_image();
	REQUIRE(img.padded_width() == iota_image_side);
	REQUIRE(img.padded_height() == iota_image_side);

	SECTION("Byte in specific coordinate") {
		constexpr std::size_t coord = 10U;
		constexpr std::size_t expected = (coord + coord * iota_image_side * image::bytes_per_pixel) % 256U;
		REQUIRE(img.get_byte(coord, coord) == expected);
	}

	SECTION("Expected distance in memory between bytes.") {
		REQUIRE(std::distance(&img.get_byte(0U, 0U), &img.get_byte(0U, 1U)) == iota_image_side * image::bytes_per_pixel);
	}
}

TEST_CASE("png2dds::image pixel access", "[image]") {
	const image img = iota_image();

	SECTION("First pixel") {
		constexpr std::array<std::uint8_t, image::bytes_per_pixel> expected{0U, 1U, 2U, 3U};
		const auto pixel = img.get_pixel(0U, 0U);
		REQUIRE(std::equal(pixel.begin(), pixel.end(), expected.cbegin()));
	}

	SECTION("Pixel in the first row") {
		constexpr std::size_t pixel_x = 3U;
		constexpr std::uint8_t value = pixel_x * image::bytes_per_pixel;
		constexpr std::array<std::uint8_t, image::bytes_per_pixel> expected{value, value + 1U, value + 2U, value + 3U};
		const auto pixel = img.get_pixel(pixel_x, 0U);
		REQUIRE(std::equal(pixel.begin(), pixel.end(), expected.cbegin()));
		REQUIRE(pixel.data() == &img.get_byte(pixel_x * image::bytes_per_pixel, 0U));
	}

	SECTION("First pixel of the second row") {
		constexpr std::size_t pixel_y = 1U;
		constexpr std::uint8_t value = (pixel_y * iota_image_side * image::bytes_per_pixel) % 256U;
		constexpr std::array<std::uint8_t, image::bytes_per_pixel> expected{value, value + 1U, value + 2U, value + 3U};
		auto pixel = img.get_pixel(0U, pixel_y);
		REQUIRE(std::equal(pixel.begin(), pixel.end(), expected.cbegin()));
		REQUIRE(pixel.data() == &img.get_byte(0U, pixel_y));
	}
}