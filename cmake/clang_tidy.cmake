# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

include_guard(GLOBAL)

option(TODDS_CLANG_TIDY "Analyze the project with clang-tidy." OFF)

if (TODDS_CLANG_TIDY)
	find_program(
		CLANG_TIDY_BINARY
		NAMES "clang-tidy"
		DOC "Clang-tidy binary"
	)

	if (CLANG_TIDY_BINARY)
		message(STATUS "Found clang-tidy binary - ${CLANG_TIDY_BINARY}")
		set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_BINARY}"
			"-extra-arg=-Wno-unknown-warning-option" # Ignore GCC-only warnings.
			"-warnings-as-errors=*"
			)
	else ()
		message(WARNING "clang-tidy binary not found")
	endif ()
endif ()
