/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "todds/string.hpp"

#include <algorithm>

namespace todds {

string to_upper_copy(const string& original) {
	string result{original};
	std::transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}

string to_lower_copy(const string& original) {
	string result{original};
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

} // namespace todds
