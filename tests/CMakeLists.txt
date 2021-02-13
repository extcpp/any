project(ext-math-tests)

set(test-files 
    "any"
)

foreach(suffix IN ITEMS "")
    #build one executable
    set(test_sources)
    foreach(test_name IN LISTS test-files${suffix}) # <- DO NOT EXPAND LIST
        list(APPEND test_sources "${test_name}.cpp")
    endforeach()

    set(test_target "test-ext-any${suffix}")
    add_executable("${test_target}" gtest.cpp ${test_sources})
    target_include_directories("${test_target}" SYSTEM PRIVATE ${gtest_SOURCE_DIR}/include)
    target_link_libraries("${test_target}"
        ext::any
        gtest_main gtest
    )
    target_compile_options("${test_target}" PRIVATE ${ext_stone-warnings})
    target_compile_definitions("${test_target}" PUBLIC EXT_CHECKED=1 EXT_IN_TEST=1)
    # -- repeated calls should append which does not happen for me (cmake 3.16 on linux)
    #target_compile_definitions("${test_target}" PUBLIC EXT_IN_TEST=1
	add_test(NAME "${test_target}_run" COMMAND $<TARGET_FILE:${test_target}>)
    set_target_properties (${test_target} PROPERTIES FOLDER tests/${test_target})
endforeach()
