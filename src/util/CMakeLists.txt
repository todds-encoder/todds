# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(png2dds_util INTERFACE)

target_sources(png2dds_util INTERFACE
	include/png2dds/memory.hpp
	include/png2dds/util.hpp
	include/png2dds/vector.hpp
	)

target_include_directories(png2dds_util INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)
