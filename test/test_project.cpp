/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/project.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("todds::project coherence checks", "[project]") {
	REQUIRE(std::strlen(todds::project::name()) > 0U);
	REQUIRE(std::strlen(todds::project::description()) > 0U);
	REQUIRE(std::strlen(todds::project::version()) > 0U);
}
