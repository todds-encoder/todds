# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

configure_file(include/todds/project.hpp.in include/todds/project.hpp @ONLY)

add_library(todds_project INTERFACE)

target_sources(todds_project INTERFACE
	${CMAKE_CURRENT_BINARY_DIR}/include/todds/project.hpp
	)

target_include_directories(todds_project INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>/include
	)

