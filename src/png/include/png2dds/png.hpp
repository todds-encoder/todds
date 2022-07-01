/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_PNG_HPP
#define PNG2DDS_PNG_HPP

#include "png2dds/image.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace png2dds {
/**
 * Decodes a PNG file stored in a memory buffer.
 * @param png Path to the PNG file, used for reporting errors.
 * @param buffer Memory buffer holding a PNG file read from the filesystem.
 * @return Decoded PNG image.
 */
image decode(const std::string& png, const std::vector<std::uint8_t>& buffer);
} // namespace png2dds

#endif // PNG2DDS_PNG_HPP
