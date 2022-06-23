# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

configure_file(include/png2dds/project.hpp.in include/png2dds/project.hpp @ONLY)

add_library(png2dds_project INTERFACE)

target_sources(png2dds_project INTERFACE
	${CMAKE_CURRENT_BINARY_DIR}/include/png2dds/project.hpp
	)

target_include_directories(png2dds_project INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>/include
	)

