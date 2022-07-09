/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_TASK_HPP
#define PNG2DDS_TASK_HPP

#include "png2dds/arguments.hpp"
#include "png2dds/dds.hpp"

#include <boost/filesystem.hpp>

#include <string>
#include <utility>
#include <vector>

namespace png2dds {

class task final {
public:
	explicit task(const args::data& arguments);
};

} // namespace png2dds

#endif // PNG2DDS_TASK_HPP
