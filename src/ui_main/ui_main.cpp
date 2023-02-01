/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/window.hpp"

#include <boost/nowide/filesystem.hpp>
#include <boost/nowide/iostream.hpp>

#include <cstdlib>

using boost::nowide::cerr;

int main(int /*argc*/, char** /*argv*/) {
	// Use UTF-8 as the default encoding for Boost.Filesystem and the global C++ locale.
	std::locale::global(boost::nowide::nowide_filesystem());

	const png2dds::window window{};
	if (!window.error().empty()) {
		cerr << window.error();
		return EXIT_FAILURE;
	}

	window.main_loop();

	return EXIT_SUCCESS;
}
