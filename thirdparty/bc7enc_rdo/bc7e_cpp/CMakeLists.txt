add_library(bc7e_cpp STATIC
	bc7enc.h
	bc7enc.cpp
	)

target_include_directories(bc7e_cpp SYSTEM PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	)

target_compile_definitions(bc7e_cpp PUBLIC
	SPNG_STATIC
	PRIVATE
	SPNG_USE_MINIZ
	)
