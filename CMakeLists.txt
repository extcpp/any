cmake_minimum_required(VERSION 3.9.0)
cmake_policy(VERSION 3.9.0)

project(any)

add_library(any INTERFACE)
target_include_directories(any INTERFACE
	"${PROJECT_SOURCE_DIR}/include/any"
)

if(ANY_BUILD_TESTS)
	find_package(Boost REQUIRED COMPONENTS unit_test_framework)
	add_executable(test_any test/test_any.cpp)
	target_link_libraries(test_any Boost::unit_test_framework)
endif()
