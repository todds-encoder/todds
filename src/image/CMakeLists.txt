# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(png2dds_image STATIC
	include/png2dds/alpha_coverage.hpp
	include/png2dds/image.hpp
	include/png2dds/image_types.hpp
	include/png2dds/mipmap_image.hpp
	alpha_coverage.cpp
	image.cpp
	image_types.cpp
	mipmap_image.cpp
	)

target_include_directories(png2dds_image PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)

target_compile_options(png2dds_image PRIVATE ${PNG2DDS_CPP_WARNING_FLAGS})

target_link_libraries(png2dds_image PUBLIC
	png2dds_format
	png2dds_util
	PRIVATE
	bc7enc_dds_defs
	)
