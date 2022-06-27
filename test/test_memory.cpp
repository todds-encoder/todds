/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <png2dds/memory.hpp>

#include <type_traits>

#include <catch2/catch.hpp>

using png2dds::memory::chunk;

TEST_CASE("png2dds::memory::chunk type traits", "[memory][chunk]") {
	STATIC_REQUIRE(std::is_nothrow_move_constructible_v<chunk>);
	STATIC_REQUIRE(std::is_nothrow_move_assignable_v<chunk>);
	STATIC_REQUIRE(std::is_nothrow_swappable_v<chunk>);
}

TEST_CASE("png2dds::memory::chunk usage", "[memory][chunk]") {
	SECTION("Zero sized chunk") {
		constexpr std::size_t size_zero = 0U;
		chunk chunk_zero{size_zero};
		REQUIRE(chunk_zero.span().size() == size_zero);
	}

	SECTION("Non-zero sized chunk") {
		constexpr std::size_t size_non_zero = 100U;
		chunk chunk_non_zero{size_non_zero};
		const auto span = chunk_non_zero.span();
		REQUIRE(span.size() == size_non_zero);
		REQUIRE(span.data() != nullptr);
	}

	SECTION("Swap") {
		constexpr std::size_t size_1 = 100U;
		constexpr std::size_t size_2 = 200U;
		chunk chunk_1{size_1};
		chunk chunk_2{size_2};
		const auto span_1 = chunk_1.span();
		const auto span_2 = chunk_2.span();
		std::swap(chunk_1, chunk_2);
		REQUIRE(chunk_1.span().data() == span_2.data());
		REQUIRE(chunk_2.span().size() == span_1.size());
	}
}
