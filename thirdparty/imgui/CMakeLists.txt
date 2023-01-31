add_library(imgui
	${CMAKE_CURRENT_LIST_DIR}/imgui.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui.h
	${CMAKE_CURRENT_LIST_DIR}/imconfig.h
	${CMAKE_CURRENT_LIST_DIR}/imgui_demo.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui_draw.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui_internal.h
	${CMAKE_CURRENT_LIST_DIR}/imgui_tables.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui_widgets.cpp
	${CMAKE_CURRENT_LIST_DIR}/imstb_rectpack.h
	${CMAKE_CURRENT_LIST_DIR}/imstb_textedit.h
	${CMAKE_CURRENT_LIST_DIR}/imstb_truetype.h
	${CMAKE_CURRENT_LIST_DIR}/backends/imgui_impl_sdl.h
	${CMAKE_CURRENT_LIST_DIR}/backends/imgui_impl_sdl.cpp
	${CMAKE_CURRENT_LIST_DIR}/backends/imgui_impl_sdlrenderer.h
	${CMAKE_CURRENT_LIST_DIR}/backends/imgui_impl_sdlrenderer.cpp
	${CMAKE_CURRENT_LIST_DIR}/misc/freetype/imgui_freetype.h
	${CMAKE_CURRENT_LIST_DIR}/misc/freetype/imgui_freetype.cpp
	)

target_include_directories(imgui SYSTEM PUBLIC ${CMAKE_CURRENT_LIST_DIR} PRIVATE ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(imgui PUBLIC SDL2::SDL2 ${FREETYPE_LIBRARIES})