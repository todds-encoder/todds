# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_executable(png2dds
	include/png2dds/arguments.hpp
	arguments.cpp
	main.cpp)

target_include_directories(png2dds PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)

target_compile_options(png2dds PRIVATE ${PNG2DDS_CPP_WARNING_FLAGS})

target_link_libraries(png2dds PRIVATE
	png2dds_project
	argparse::argparse
	fmt::fmt
	)
