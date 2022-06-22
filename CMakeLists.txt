# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.22)

project(png2dds
	VERSION 0.1.0
	DESCRIPTION "Convert from PNG to DDS."
	HOMEPAGE_URL "https://gitlab.com/joseasoler/png2dds"
	)

# Set the C++ standard used by this project.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Disable non-standard compiler extensions.
set(CMAKE_CXX_EXTENSIONS OFF)
# Export compile commands information.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/clang_tidy.cmake)
include(cmake/warnings.cmake)

find_package(argparse CONFIG REQUIRED)

add_executable(png2dds
	include/png2dds/arguments.hpp
	src/arguments.cpp
	src/main.cpp)

target_include_directories(png2dds PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include)

target_link_libraries(png2dds PRIVATE argparse::argparse)
