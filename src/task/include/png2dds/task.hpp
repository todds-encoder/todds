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
	using paths_vector = std::vector<std::pair<boost::filesystem::path, boost::filesystem::path>>;
	explicit task(args::data arguments);
	void start();

private:
	args::data _arguments;
	encoder _encoder;
	paths_vector _paths;
};

} // namespace png2dds

#endif // PNG2DDS_TASK_HPP
