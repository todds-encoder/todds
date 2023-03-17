/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

namespace todds {

/** Allocator to use in todds types. */
template<typename Type> using allocator = std::allocator<Type>;

} // namespace todds
