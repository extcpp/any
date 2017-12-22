cmake_minimum_required(VERSION 3.9.0)
cmake_policy(VERSION 3.9.0)

project(any)

add_library(any INTERFACE)
target_include_directories(any INTERFACE
	"${PROJECT_SOURCE_DIR}/include/any"
)
