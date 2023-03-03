# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(png2dds_dds STATIC
	include/png2dds/dds.hpp
	dds.cpp
	)

target_include_directories(png2dds_dds PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)

target_compile_options(png2dds_dds PRIVATE ${PNG2DDS_CPP_WARNING_FLAGS})

target_compile_definitions(png2dds_dds PRIVATE RGBCX_IMPLEMENTATION)

target_link_libraries(png2dds_dds PRIVATE
	bc7enc_dds_defs
	rgbcx
	TBB::tbb
	PUBLIC
	png2dds_image
	bc7e_ispc
	)
