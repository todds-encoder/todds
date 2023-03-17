/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/filter.hpp"

#include <opencv2/imgproc.hpp>

#include <catch2/catch_test_macros.hpp>

using todds::filter::name;
using todds::filter::type;

TEST_CASE("todds::filter::name", "[filter]") {
	STATIC_REQUIRE(!name(type::nearest).empty());
	STATIC_REQUIRE(!name(type::cubic).empty());
	STATIC_REQUIRE(!name(type::area).empty());
	STATIC_REQUIRE(!name(type::lanczos).empty());
	STATIC_REQUIRE(!name(type::nearest_exact).empty());
}

TEST_CASE("todds::filter::type and OpenCV", "[filter]") {
	STATIC_REQUIRE(static_cast<std::uint8_t>(type::nearest) == static_cast<std::uint8_t>(cv::INTER_NEAREST));
	STATIC_REQUIRE(static_cast<std::uint8_t>(type::cubic) == static_cast<std::uint8_t>(cv::INTER_CUBIC));
	STATIC_REQUIRE(static_cast<std::uint8_t>(type::area) == static_cast<std::uint8_t>(cv::INTER_AREA));
	STATIC_REQUIRE(static_cast<std::uint8_t>(type::lanczos) == static_cast<std::uint8_t>(cv::INTER_LANCZOS4));
	STATIC_REQUIRE(static_cast<std::uint8_t>(type::nearest_exact) == static_cast<std::uint8_t>(cv::INTER_NEAREST_EXACT));
}
