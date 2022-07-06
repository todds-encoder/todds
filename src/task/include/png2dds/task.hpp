/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_TASK_HPP
#define PNG2DDS_TASK_HPP

#include "png2dds/arguments.hpp"
#include "png2dds/dds.hpp"

#include <string>
#include <vector>

namespace png2dds {

class task final {
public:
	explicit task(args::data arguments);
	void start();

private:
	args::data _arguments;
	encoder _encoder;
	std::vector<std::string> _png;
};

} // namespace png2dds

#endif // PNG2DDS_TASK_HPP
