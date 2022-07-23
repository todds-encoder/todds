# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.22)

project(png2dds
	VERSION 0.1.0
	DESCRIPTION "Batch conversion of PNG files to DDS."
	HOMEPAGE_URL "https://gitlab.com/joseasoler/png2dds"
	)

# Set the C++ standard used by this project.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Disable non-standard compiler extensions.
set(CMAKE_CXX_EXTENSIONS OFF)
# Export compile commands information.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/platform.cmake)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(Boost 1.73 REQUIRED COMPONENTS
	# Support UTF-8 on Windows.
	nowide
	# Use UTF-8 when converting char*/string to filesystem::path and vice-versa.
	filesystem
	# Parse command-line arguments.
	program_options
	)
find_package(fmt CONFIG REQUIRED)
find_package(Hyperscan REQUIRED)
find_package(Threads REQUIRED)
find_package(TBB CONFIG REQUIRED)

add_subdirectory(thirdparty)

include(cmake/clang_tidy.cmake)
include(cmake/warnings.cmake)

add_subdirectory(src)
add_subdirectory(test)
