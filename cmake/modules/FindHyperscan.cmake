# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package(PkgConfig)
pkg_check_modules(PC_Hyperscan QUIET libhs)

find_path(Hyperscan_INCLUDE_DIR
	NAMES hs.h
	PATHS ${PC_Hyperscan_INCLUDE_DIRS}
	PATH_SUFFIXES hs
	)

find_library(Hyperscan_LIBRARY
	NAMES hs
	PATHS ${PC_Hyperscan_LIBRARY_DIRS}
	)

set(Hyperscan_VERSION ${PC_Hyperscan_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Hyperscan
	FOUND_VAR Hyperscan_FOUND
	REQUIRED_VARS
	Hyperscan_LIBRARY
	Hyperscan_INCLUDE_DIR
	VERSION_VAR Hyperscan_VERSION
	)

if (Hyperscan_FOUND)
	set(Hyperscan_LIBRARIES ${Hyperscan_LIBRARY})
	set(Hyperscan_INCLUDE_DIRS ${Hyperscan_INCLUDE_DIR})
	set(Hyperscan_DEFINITIONS ${PC_Hyperscan_CFLAGS_OTHER})
endif ()

if (Hyperscan_FOUND AND NOT TARGET Hyperscan::Hyperscan)
	add_library(Hyperscan::Hyperscan UNKNOWN IMPORTED)
	set_target_properties(Hyperscan::Hyperscan PROPERTIES
		IMPORTED_LOCATION "${Hyperscan_LIBRARY}"
		INTERFACE_COMPILE_OPTIONS "${PC_Hyperscan_CFLAGS_OTHER}"
		INTERFACE_INCLUDE_DIRECTORIES "${Hyperscan_INCLUDE_DIR}"
		)
endif ()

mark_as_advanced(
	Hyperscan_INCLUDE_DIR
	Hyperscan_LIBRARY
)
