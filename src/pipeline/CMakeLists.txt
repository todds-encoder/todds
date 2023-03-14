# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(png2dds_pipeline STATIC
	include/png2dds/input.hpp
	include/png2dds/pipeline.hpp
	filter_common.hpp
	filter_decode_png.hpp
	filter_decode_png.cpp
	filter_encode_dds.hpp
	filter_encode_dds.cpp
	filter_load_png.hpp
	filter_load_png.cpp
	filter_pixel_blocks.hpp
	filter_pixel_blocks.cpp
	filter_save_dds.hpp
	filter_save_dds.cpp
	pipeline.cpp
	)

target_include_directories(png2dds_pipeline PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	)

target_compile_options(png2dds_pipeline PRIVATE ${PNG2DDS_CPP_WARNING_FLAGS})

target_link_libraries(png2dds_pipeline
	PUBLIC
	png2dds_arguments
	png2dds_format
	PRIVATE
	png2dds_dds
	png2dds_png
	png2dds_regex
	png2dds_resize
	png2dds_util
	bc7enc_dds_defs
	Boost::headers
	Boost::filesystem
	fmt::fmt
	TBB::tbb
	)
