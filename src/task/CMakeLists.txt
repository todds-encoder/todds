# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_library(todds_task STATIC
	include/todds/file_retrieval.hpp
	include/todds/task.hpp
	file_retrieval.cpp
	task.cpp
)

target_include_directories(todds_task PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
)

target_compile_options(todds_task PRIVATE ${TODDS_CPP_WARNING_FLAGS})

target_link_libraries(todds_task
	PUBLIC
	todds_arguments
	todds_pipeline
	todds_report
	PRIVATE
	todds_format
)
