/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_EXCEPTION_HPP
#define PNG2DDS_EXCEPTION_HPP

#include <stdexcept>

namespace png2dds {

class runtime_error : public std::runtime_error {
public:
	explicit runtime_error(const std::string& str) : std::runtime_error(str) {}
};

} // namespace png2dds

#endif // PNG2DDS_EXCEPTION_HPP
