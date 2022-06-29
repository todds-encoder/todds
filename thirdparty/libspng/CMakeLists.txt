add_library(libspng STATIC
	spng.h
	spng.c
	)

target_include_directories(libspng SYSTEM PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	)

target_compile_definitions(libspng PRIVATE
	SPNG_STATIC
	SPNG_USE_MINIZ
	)

target_link_libraries(libspng PRIVATE
	miniz
	)
