# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package(Catch2 REQUIRED)

add_executable(png2dds_test
	test_main.cpp
	test_arguments.cpp
	test_format.cpp
	test_image.cpp
	test_project.cpp
	test_util.cpp
	)

target_link_libraries(png2dds_test PRIVATE
	bc7enc_dds_defs
	Boost::nowide
	Boost::filesystem
	Catch2::Catch2
	rgbcx
	TBB::tbb
	png2dds_arguments
	png2dds_format
	png2dds_image
	png2dds_project
	png2dds_util
	)