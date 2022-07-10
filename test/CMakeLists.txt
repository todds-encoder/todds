# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package(Catch2 REQUIRED)

add_executable(png2dds_test
	tests.cpp # Catch2 main file
	test_dds_image.cpp
	test_image.cpp
	test_project.cpp
	test_util.cpp
  )

target_link_libraries(png2dds_test PRIVATE
	bc7enc_dds_defs
	Catch2::Catch2
	png2dds_project
	png2dds_image
	png2dds_util
	)