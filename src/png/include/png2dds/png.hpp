/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_PNG_HPP
#define PNG2DDS_PNG_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace png2dds {
std::vector<std::uint8_t> decode(const std::string& png, const std::vector<std::uint8_t>& buffer);
} // namespace png2dds

#endif // PNG2DDS_PNG_HPP
