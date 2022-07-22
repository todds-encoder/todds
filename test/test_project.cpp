/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/project.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("png2dds::project coherence checks", "[project]") {
	STATIC_REQUIRE(!png2dds::project::name().empty());
	STATIC_REQUIRE(!png2dds::project::description().empty());
	STATIC_REQUIRE(!png2dds::project::version().empty());
}
