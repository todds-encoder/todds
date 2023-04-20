/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/regex.hpp"

namespace todds {

class regex_pimpl final {};

regex::regex(std::string_view) // NOLINT
	: _pimpl{nullptr} {}

regex::regex()
	: regex("") {}

regex::regex(regex&& other) noexcept = default;

regex& regex::operator=(regex&& other) noexcept = default;

regex::~regex() = default;

std::string_view regex::error() const noexcept { return "Missing Hyperscan support."; } // NOLINT

bool regex::match(std::string_view) const { return true; }															// NOLINT

} // namespace todds
