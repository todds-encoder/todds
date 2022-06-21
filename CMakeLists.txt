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

add_executable(png2dds src/main.cpp)
