include_guard(GLOBAL)

option(PNG2DDS_CLANG_TIDY "Analyze the project with clang-tidy." ON)

if (PNG2DDS_CLANG_TIDY)
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
