# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_executable(todds main.cpp)

target_compile_options(todds PRIVATE ${TODDS_CPP_WARNING_FLAGS})

target_link_libraries(todds PRIVATE
	todds_project
	todds_arguments
	todds_task
	Boost::nowide
	fmt::fmt
	TBB::tbb
	)

install(TARGETS todds RUNTIME)
