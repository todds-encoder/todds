/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

#include "memory.hpp"

namespace png2dds {

/** Vector to use in png2dds types. */
template<typename Type, typename Allocator = png2dds::allocator<Type>> using vector = std::vector<Type, Allocator>;

} // namespace png2dds
