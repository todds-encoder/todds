# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

add_subdirectory(miniz)
add_subdirectory(libspng)
add_subdirectory(bc7enc_rdo)
if (PNG2DDS_UI)
	add_subdirectory(imgui)
endif ()