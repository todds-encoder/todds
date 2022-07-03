add_library(libspng STATIC
	spng.h
	spng.c
	)

target_include_directories(libspng SYSTEM PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	)

target_compile_definitions(libspng PUBLIC
	SPNG_STATIC
	PRIVATE
	SPNG_USE_MINIZ
	)

target_link_libraries(libspng PRIVATE
	miniz
	)
