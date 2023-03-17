# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(todds_png STATIC
	include/todds/png.hpp
	png.cpp
	)

target_include_directories(todds_png PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)

target_compile_options(todds_png PRIVATE ${TODDS_CPP_WARNING_FLAGS})

target_link_libraries(todds_png PUBLIC
	todds_image
	PRIVATE
	fmt::fmt
	libspng
	)