# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=undefined,address>")
	add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=undefined,address>")
endif ()
