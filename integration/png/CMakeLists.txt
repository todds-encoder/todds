# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_executable(reencoder reencoder.cpp)

target_compile_options(reencoder PRIVATE ${PNG2DDS_CPP_WARNING_FLAGS})

target_link_libraries(reencoder PRIVATE
	png2dds_png
	Boost::nowide
	fmt::fmt
	libspng
	)
