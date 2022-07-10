/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/dds_image.hpp"

#include <dds_defs.h>

#include <type_traits>

#include <catch2/catch.hpp>

using png2dds::dds_image;

TEST_CASE("png2dds::dds_image type trait checks", "[dds_image]") {
	// dds_image is a move-only type.
	STATIC_REQUIRE(std::is_nothrow_move_constructible_v<dds_image>);
	STATIC_REQUIRE(std::is_nothrow_move_assignable_v<dds_image>);
	STATIC_REQUIRE(!std::is_copy_constructible_v<dds_image>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<dds_image>);
	STATIC_REQUIRE(std::is_nothrow_destructible_v<dds_image>);
	STATIC_REQUIRE(std::is_nothrow_swappable_v<dds_image>);
}

TEST_CASE("png2dds::dds_image type assumptions", "[dds_image]") {
	// The header must have exactly enough memory to store all data from these two types.
	static_assert(sizeof(DDSURFACEDESC2) + sizeof(DDS_HEADER_DXT10) == sizeof(dds_image::header_type));

	// Blocks must meet these constraints.
	static_assert(std::is_same_v<dds_image::block_type::value_type, std::uint64_t>);
	static_assert(sizeof(png2dds::dds_image::block_type) == 16U);
}
