# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(todds_report STATIC
	include/todds/report.hpp
	report.cpp
)

target_include_directories(todds_report PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
)

target_compile_options(todds_report PRIVATE ${TODDS_CPP_WARNING_FLAGS})

target_link_libraries(todds_report
	PUBLIC
	todds_util
	TBB::tbb
)
