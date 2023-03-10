# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(png2dds_resize STATIC
	include/png2dds/resize.hpp
	resize.cpp
	)

target_include_directories(png2dds_resize PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)

target_compile_options(png2dds_resize PRIVATE ${PNG2DDS_CPP_WARNING_FLAGS})

target_link_libraries(png2dds_resize PUBLIC
	png2dds_dds
	)
