add_library(bc7enc_dds_defs INTERFACE)

target_sources(bc7enc_dds_defs INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/dds_defs.h
	)

target_include_directories(bc7enc_dds_defs SYSTEM INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	)
