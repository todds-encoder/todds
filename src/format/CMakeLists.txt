# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(png2dds_format INTERFACE)

target_sources(png2dds_format INTERFACE
	include/png2dds/filter.hpp
	include/png2dds/format.hpp
	)

target_include_directories(png2dds_format INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)
