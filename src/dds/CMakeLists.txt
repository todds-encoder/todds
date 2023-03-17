# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(todds_dds STATIC
	include/todds/dds.hpp
	dds.cpp
	)

target_include_directories(todds_dds PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)

target_compile_options(todds_dds PRIVATE ${TODDS_CPP_WARNING_FLAGS})

target_compile_definitions(todds_dds PRIVATE RGBCX_IMPLEMENTATION)

target_link_libraries(todds_dds PRIVATE
	bc7enc_dds_defs
	rgbcx
	TBB::tbb
	PUBLIC
	todds_image
	bc7e_ispc
	)
