# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(todds_image STATIC
	include/todds/alpha_coverage.hpp
	include/todds/image.hpp
	include/todds/image_types.hpp
	include/todds/mipmap_image.hpp
	alpha_coverage.cpp
	image.cpp
	image_types.cpp
	mipmap_image.cpp
	)

target_include_directories(todds_image PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)

target_compile_options(todds_image PRIVATE ${TODDS_CPP_WARNING_FLAGS})

target_link_libraries(todds_image PUBLIC
	todds_format
	todds_util
	PRIVATE
	bc7enc_dds_defs
	)
