# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_executable(png2dds_ui
	include/png2dds/window.hpp
	window.cpp
	ui_main.cpp
	)

target_include_directories(png2dds_ui PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)

target_compile_options(png2dds PRIVATE ${PNG2DDS_CPP_WARNING_FLAGS})

configure_file(${PROJECT_SOURCE_DIR}/thirdparty/manrope/Manrope-Regular.ttf ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

target_link_libraries(png2dds_ui PRIVATE
	png2dds_project
	Boost::filesystem
	Boost::nowide
	fmt::fmt
	imgui
	)
