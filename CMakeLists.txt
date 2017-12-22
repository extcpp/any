cmake_minimum_required(VERSION 3.9.0)
cmake_policy(VERSION 3.9.0)

project(any)

add_library(any INTERFACE)
target_include_directories(any INTERFACE
	"${PROJECT_SOURCE_DIR}/include"
)

add_library(any::any ALIAS any)

if(ANY_BUILD_TESTS)
	find_package(Boost REQUIRED COMPONENTS unit_test_framework)
	add_executable(test_any test/test_any.cpp)
	target_link_libraries(test_any Boost::unit_test_framework any::any)
	set_property(TARGET test_any PROPERTY CXX_STANDARD 17)
	set_property(TARGET test_any PROPERTY CXX_EXTENSIONS OFF)
	add_test(test_any test_any)
	enable_testing()
endif()
