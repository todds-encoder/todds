find_program(ISPC_BINARY REQUIRED NAMES "ispc")

set(BC7E_ISPC_INPUT_FILE ${CMAKE_CURRENT_LIST_DIR}/bc7e.ispc)
set(BC7E_ISPC_HEADER_FILE ${CMAKE_CURRENT_BINARY_DIR}/bc7e_ispc.h)
set(BC7E_ISPC_OBJECT_FILE ${CMAKE_CURRENT_BINARY_DIR}/bc7e${CMAKE_CXX_OUTPUT_EXTENSION})

set(BC7E_ISPC_OUTPUT_FILES
	${BC7E_ISPC_HEADER_FILE}
	${BC7E_ISPC_OBJECT_FILE}
	${CMAKE_CURRENT_BINARY_DIR}/bc7e_avx${CMAKE_CXX_OUTPUT_EXTENSION}
	${CMAKE_CURRENT_BINARY_DIR}/bc7e_sse2${CMAKE_CXX_OUTPUT_EXTENSION}
	${CMAKE_CURRENT_BINARY_DIR}/bc7e_sse4${CMAKE_CXX_OUTPUT_EXTENSION}
	${CMAKE_CURRENT_BINARY_DIR}/bc7e_avx2${CMAKE_CXX_OUTPUT_EXTENSION}
	)

# The --pic flag has been added because most modern Linux Distributions enable position independent code by default.
# See https://wiki.gentoo.org/wiki/Hardened/Toolchain#Position_Independent_Executables_.28PIEs.29 for details.
add_custom_command(OUTPUT ${BC7E_ISPC_OUTPUT_FILES}
	DEPENDS ${BC7E_ISPC_INPUT_FILE}
	COMMAND ${ISPC_BINARY} --woff --pic -g -O3 ${BC7E_ISPC_INPUT_FILE} -o ${BC7E_ISPC_OBJECT_FILE} -h ${BC7E_ISPC_HEADER_FILE} --target=sse2,sse4,avx,avx2 --opt=disable-assertions
	COMMENT "Compiling bc7e_ispc object files using ${ISPC_BINARY}")

add_library(bc7e_ispc STATIC
	bc7e.ispc
	${BC7E_ISPC_OUTPUT_FILES}
	)

set_target_properties(bc7e_ispc PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(bc7e_ispc SYSTEM PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	)
