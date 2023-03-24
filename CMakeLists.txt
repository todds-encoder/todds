# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.22)

project(todds
	VERSION 0.2.0
	DESCRIPTION "A CPU-based DDS encoder optimized for fast batch conversions with high encoding quality."
	HOMEPAGE_URL "https://github.com/joseasoler/todds"
	)

option(TODDS_UNIT_TESTS "Build todds unit tests" OFF)
option(TODDS_TBB_ALLOCATOR "Use OneAPI TBB scalable_allocator." OFF)
option(TODDS_MIMALLOC_ALLOCATOR "Use mimalloc." OFF)

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
	)
find_package(fmt REQUIRED)
find_package(Hyperscan REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
find_package(TBB REQUIRED)
if(TODDS_MIMALLOC_ALLOCATOR)
	find_package(mimalloc REQUIRED)
endif()

add_subdirectory(thirdparty)

include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})

include(cmake/clang_tidy.cmake)
include(cmake/warnings.cmake)
include(cmake/sanitizer.cmake)
include(cmake/options.cmake)

add_subdirectory(src)
if (TODDS_UNIT_TESTS)
	add_subdirectory(test)
endif ()
