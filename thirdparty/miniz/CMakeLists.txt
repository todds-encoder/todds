add_library(miniz STATIC
	miniz.h
	miniz.c
	)

target_include_directories(miniz SYSTEM PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	)
