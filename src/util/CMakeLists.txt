# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(todds_util STATIC
	include/todds/memory.hpp
	include/todds/profiler.hpp
	include/todds/string.hpp
	include/todds/util.hpp
	include/todds/vector.hpp
	string.cpp
	)

target_include_directories(todds_util PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)

if (TODDS_TBB_ALLOCATOR)
	target_link_libraries(todds_util PRIVATE TBB::tbbmalloc)
elseif(TODDS_MIMALLOC_ALLOCATOR)
	target_link_libraries(todds_util PRIVATE mimalloc-static)
endif()
