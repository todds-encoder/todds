/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

#include "todds/memory.hpp"

namespace todds {

/** Vector to use in todds types. */
template<typename Type, typename Allocator = todds::allocator<Type>> using vector = std::vector<Type, Allocator>;

} // namespace todds
