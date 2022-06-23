find_package(Catch2 REQUIRED)

add_executable(png2dds_test
	tests.cpp # Catch2 main file
	test_project.cpp
	)

target_link_libraries(png2dds_test PRIVATE
	Catch2::Catch2
	png2dds_project
	)