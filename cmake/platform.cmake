# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

include_guard(GLOBAL)

# Platform or compiler-specific defines.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_definitions(UNICODE _UNICODE)
	message(STATUS "Defining UNICODE and _UNICODE for MSVC")
	add_compile_options("/utf-8")
	message(STATUS "Adding /utf-8 option for MSVC ")
	add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
	message(STATUS "Defining NOMINMAX and WIN32_LEAN_AND_MEAN for windows.h")
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# Disable stack protection in debug builds to allow inspecting issues with the debugger.
	add_compile_options("$<$<CONFIG:DEBUG>:-fno-stack-protector>")
endif ()
