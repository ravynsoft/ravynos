# This file can be built either individually or as part of Turbo's
# build hierarchy.

cmake_minimum_required(VERSION 3.5)
project(turbo-examples)

set(MASTER_PROJECT FALSE)
if (CMAKE_CURRENT_LIST_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MASTER_PROJECT TRUE)
    set(TURBO_BINARY_DIR ${CMAKE_BINARY_DIR})
endif()

if (MASTER_PROJECT)
    # We have to import these when building this file standalone.
    find_package(tvision CONFIG REQUIRED)
    find_package(turbo CONFIG REQUIRED)
    add_library(turbo-core ALIAS turbo::turbo-core)
endif()

if (TURBO_BUILD_EXAMPLES OR MASTER_PROJECT)
    function(add_example_generic app)
        add_executable(${app} ${app}.cc)
        target_link_libraries(${app} PRIVATE turbo-core)
        # Place everything in the root build directory, for ease of use.
        set_target_properties(${app} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${TURBO_BINARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY ${TURBO_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY ${TURBO_BINARY_DIR}
        )
    endfunction()

    add_example_generic(demo-with-custom-editor-window)
    add_example_generic(demo-pascal-editor)
endif()
