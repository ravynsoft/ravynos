if (TV_BUILD_TESTS)
    file(GLOB_RECURSE TEST_SRC "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

    add_executable(${PROJECT_NAME}-test ${TEST_SRC})

    target_compile_features(${PROJECT_NAME}-test PRIVATE cxx_std_14)
    target_include_directories(${PROJECT_NAME}-test PRIVATE "${CMAKE_CURRENT_LIST_DIR}")

    tv_add_private_includes(${PROJECT_NAME}-test)
    tv_set_warnings(${PROJECT_NAME}-test)

    find_library(GTEST gtest REQUIRED)
    tv_message(STATUS "Found 'gtest': ${GTEST}")
    find_library(GTEST_MAIN gtest_main REQUIRED)
    tv_message(STATUS "Found 'gtest_main': ${GTEST_MAIN}")
    target_link_libraries(${PROJECT_NAME}-test PRIVATE
        ${PROJECT_NAME}
        ${GTEST}
        ${GTEST_MAIN}
    )

    if (NOT WIN32)
        find_library(PTHREAD pthread REQUIRED)
        tv_message(STATUS "Found 'pthread': ${PTHREAD}")
        target_link_libraries(${PROJECT_NAME}-test PRIVATE
            ${PTHREAD}
        )
    endif()

    add_custom_command(
        OUTPUT ${PROJECT_NAME}-test-passed
        COMMAND ${PROJECT_NAME}-test
        COMMAND ${CMAKE_COMMAND} -E touch ${PROJECT_NAME}-test-passed
        DEPENDS ${PROJECT_NAME}-test
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_custom_target(${PROJECT_NAME}-test-run ALL DEPENDS ${PROJECT_NAME}-test-passed)

    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16.0" AND TV_OPTIMIZE_BUILD)
        target_precompile_headers(${PROJECT_NAME}-test PRIVATE "${CMAKE_CURRENT_LIST_DIR}/test.h")
        tv_enable_unity(${PROJECT_NAME}-test)
    endif()
endif()
