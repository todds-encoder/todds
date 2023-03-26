# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(todds_pipeline STATIC
		include/todds/input.hpp
		include/todds/pipeline.hpp
		get_filters_from_settings.cpp
		get_filters_from_settings.hpp
		filter_common.hpp
		filter_decode_png.hpp
		filter_decode_png.cpp
		filter_encode_dds.hpp
		filter_encode_dds.cpp
		filter_generate_mipmaps.hpp
		filter_generate_mipmaps.cpp
		filter_load_png.hpp
		filter_load_png.cpp
		filter_pixel_blocks.hpp
		filter_pixel_blocks.cpp
		filter_save_dds.hpp
		filter_save_dds.cpp
		pipeline.cpp
		filter_scale_image.cpp filter_scale_image.hpp)

target_include_directories(todds_pipeline PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
		PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		)

target_compile_options(todds_pipeline PRIVATE ${TODDS_CPP_WARNING_FLAGS})

target_link_libraries(todds_pipeline
		PUBLIC
		todds_arguments
		todds_format
		PRIVATE
		todds_dds
		todds_png
		todds_regex
		todds_util
		bc7enc_dds_defs
		Boost::headers
		Boost::filesystem
		fmt::fmt
		TBB::tbb
		${OpenCV_LIBS}
		)
