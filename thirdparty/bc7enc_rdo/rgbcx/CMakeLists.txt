add_library(rgbcx INTERFACE)

target_sources(bc7enc_dds_defs INTERFACE
	rgbcx.h
	rgbcx_png2dds.h
	rgbcx_table4.h
	)

target_include_directories(rgbcx SYSTEM INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
	)
