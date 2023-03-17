# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package(Catch2 REQUIRED)

add_executable(todds_test
	test_main.cpp
	test_arguments.cpp
	test_filter.cpp
	test_format.cpp
	test_project.cpp
	test_util.cpp
	)

target_link_libraries(todds_test PRIVATE
	bc7enc_dds_defs
	Boost::nowide
	Boost::filesystem
	Catch2::Catch2
	rgbcx
	TBB::tbb
	todds_arguments
	todds_format
	todds_image
	todds_project
	todds_util
	)