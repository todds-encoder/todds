add_library(miniz STATIC
	miniz.h
	miniz.c
	)

target_include_directories(miniz SYSTEM PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	)

# Compile only the parts of miniz required by libspng.
target_compile_definitions(miniz PUBLIC
	MINIZ_NO_STDIO
	MINIZ_NO_TIME
	MINIZ_NO_ARCHIVE_APIS
	MINIZ_NO_ARCHIVE_WRITING_APIS
	MINIZ_NO_MALLOC
	)