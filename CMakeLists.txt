# Copyright - 2020 - Jan Christoph Uhde <Jan@UhdeJC.com>

cmake_minimum_required(VERSION 3.8)
project(ext-any VERSION 0.0.1 LANGUAGES CXX)
message(STATUS "extINFO -- entering ext-any")

## OPTIONS
option(EXTANY_WARNINGS "enable warnings" ON)
option(EXTANY_CHECKED "user assert" ON)
option(EXTANY_TESTS "build tests" OFF)
option(EXTANY_EXAMPLES "build examples" OFF)
option(EXTANY_NO_RTTI  "build without runtime type information support" OFF)

# enable extcpp cmake
include(${CMAKE_CURRENT_LIST_DIR}/ext_cmake_enable.cmake)
include(ext_cmake_setup)

# include build dir to find version.hpp
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# required by folder structure for XCode and VisualStudio (includes)
# sources are always required
include(src_files.cmake)
include(include_files.cmake)

add_library(ext-any INTERFACE ${ext-any-source} ${ext-any-header})
target_include_directories(ext-any INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)

target_compile_definitions(ext-any INTERFACE
    $<$<BOOL:${EXTANY_NO_RTTI}>:EXTANY_NO_RTTI>
)

# set up folder structure for XCode and VisualStudio
#set_target_properties (ext-any PROPERTIES FOLDER any)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ext-any-header} ${ext-any-source})

add_library(ext::any ALIAS ext-any)

## testing
if(EXTANY_TESTS)
    ext_log("ext-any tests enabled")
    include(CTest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    ext_add_test_subdirectory("google" tests)
else()
    ext_log("ext-any tests disabled")
endif()

## installation
if(COMMAND ext_install)
    set_target_properties(ext-any PROPERTIES EXPORT_NAME any)

    ext_install(ext-any include/ext)

    install(TARGETS ext-any
        EXPORT ext-any-targets
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
	install(
        EXPORT ext-any-targets
		FILE ext-any-config.cmake
        NAMESPACE ext::
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake
    )
endif()

ext_log("generating version information")
add_custom_target(
    ext_any_version_update ALL
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND}
                -D "EXT_GIT_VERSION_OUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/ext_any_version.hpp"
                -P "${ext_cmake_dir}/ext_script_git_version.cmake"
)
