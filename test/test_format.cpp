/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/format.hpp"

#include <catch2/catch_test_macros.hpp>

using todds::format::name;
using todds::format::type;

TEST_CASE("todds::format::name", "[format]") {
	STATIC_REQUIRE(!name(type::bc1).empty());
	STATIC_REQUIRE(!name(type::bc7).empty());
	STATIC_REQUIRE(!name(type::bc1_alpha_bc7).empty());
}
