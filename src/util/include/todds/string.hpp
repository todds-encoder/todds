/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/memory.hpp"

#include <string>

namespace todds {

/** String type to use in todds. */
using string = std::basic_string<char, std::char_traits<char>, todds::allocator<char>>;

/**
 * Create an upper-case copy of a string.
 * @param original Input string.
 * @return Upper-case copy.
 */
string to_upper_copy(const string& original);

/**
 * Create a lower-case copy of a string.
 * @param original Input string.
 * @return Lower-case copy.
 */
string to_lower_copy(const string& original);

} // namespace todds
