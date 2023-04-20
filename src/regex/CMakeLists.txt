# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

if (TODDS_HYPERSCAN_SUPPORT)
	set(todds_regex_source_file regex.cpp)
else ()
	set(todds_regex_source_file regex_empty.cpp)
endif ()

add_library(todds_regex STATIC
	include/todds/regex.hpp
	${todds_regex_source_file}
	)

target_include_directories(todds_regex PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
	)

target_compile_options(todds_regex PRIVATE ${TODDS_CPP_WARNING_FLAGS})

if (TODDS_HYPERSCAN_SUPPORT)
	target_link_libraries(todds_regex PRIVATE
		Hyperscan::Hyperscan
		)
endif ()