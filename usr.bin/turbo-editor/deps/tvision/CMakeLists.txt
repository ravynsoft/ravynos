# Options summary:
#
# * TV_BUILD_EXAMPLES (default ON) to build example applications.
# * TV_BUILD_USING_GPM (default ON) (only on linux) to enable linking to libgpm
#   (warning if not found).
# * TV_USE_STATIC_RTL (default OFF) to link against the static version of the
#   runtime library (MSVC only).
# * TV_REDUCE_APP_SIZE (default OFF) to reduce executable size by asking the
#   linker to strip out unused functions and data. This affects the library's PUBLIC
#   linker flags.
# * TV_OPTIMIZE_BUILD (default ON) to build with Precompiled Headers (CMake 3.16
#   or newer).
# * TV_LIBRARY_UNITY_BUILD (default OFF) to reduce the build time of the main library
#   (CMake 3.16 or newer), at the cost of possibly increasing its size.
# * TV_BUILD_AVSCOLOR (default OFF) to build an AviSynth plugin for testing the
#   terminal color quantization (Unix only).
# * TV_BUILD_TESTS (default OFF) to build and run tests.

cmake_minimum_required (VERSION 3.5)
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.13.0")
    cmake_policy(SET CMP0077 NEW) # 'option()' honors normal variables.
endif()

set(MASTER_PROJECT FALSE)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MASTER_PROJECT TRUE)
endif()

function(tv_message mode)
    if (MASTER_PROJECT)
        set(msg)
    else()
        set(msg "(${PROJECT_NAME}) ")
    endif()
    foreach(i ${ARGN})
        set(msg "${msg}${i}")
    endforeach()
    message(${mode} ${msg})
endfunction()

function(tv_message_mp)
    if (MASTER_PROJECT)
        tv_message(${ARGN})
    endif()
endfunction()

project(tvision)

# Project options

include(CheckCXXSourceCompiles)

check_cxx_source_compiles("
    #define _doDeclare(a, b) int a ## b;
    #define _expand(a, ...) _doDeclare(a, __VA_ARGS__)
    #define _declare(a) _expand(a, __COUNTER__)

    _declare(a)
    _declare(a)
    _declare(a)

    int main() { return a1; }
"   SUPPORTS_COUNTER_MACRO)

function(tv_enable_unity target)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16.0" AND SUPPORTS_COUNTER_MACRO)
        set_target_properties(${target} PROPERTIES UNITY_BUILD ON)
    endif()
endfunction()

set(TV_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

function(tv_set_output_dir target)
    # Place everything in the build directory, for ease of use.
    set_target_properties(${target} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${TV_BINARY_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${TV_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${TV_BINARY_DIR}
    )
endfunction()

function(tv_add_private_includes target)
    target_include_directories(${target} PRIVATE
        "${PROJECT_SOURCE_DIR}/include/tvision"
        "${PROJECT_SOURCE_DIR}/include/tvision/compat/borland"
    )
    if (NOT WIN32)
        target_include_directories(${target} PRIVATE
            "${PROJECT_SOURCE_DIR}/include/tvision/compat/windows"
        )
    endif()
    if (NOT WIN32 AND NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android"))
        target_include_directories(${target} PRIVATE
            "${PROJECT_SOURCE_DIR}/include/tvision/compat/malloc"
        )
    endif()
endfunction()

function(tv_set_warnings target)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target} PRIVATE
            /wd4068 # Unrecognized pragmas
            /wd4146 # Unsigned despite minus sign
            /wd4166 # Illegal calling convention for constructor/destructor
            /wd4244 # Possible data loss in type conversion
            /wd4250 # Inheritance via dominance
            /wd4267 # Possible data loss in conversion from size_t to other type
        )
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wno-deprecated
            -Wno-unknown-pragmas
            -Wno-pragmas
        )
    endif()
endfunction()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(TV_BUILD_USING_GPM "Use GPM" ON)
    set(MAY_BUILD_USING_GPM TRUE)
endif()

if (MASTER_PROJECT)
    option(TV_BUILD_EXAMPLES "Build example apps" ON)
endif()

option(TV_USE_STATIC_RTL "Link against the static version of the runtime library (MSVC only)" OFF)
if (MSVC)
    set(MAY_USE_STATIC_RTL TRUE)
elseif (TV_USE_STATIC_RTL)
    tv_message(WARNING "'TV_USE_STATIC_RTL' requested but only available in MSVC or equivalent.")
    set(TV_USE_STATIC_RTL OFF)
endif()

option(TV_REDUCE_APP_SIZE "Strip out unused functions and data from executables" OFF)
if ((CMAKE_VERSION VERSION_LESS "3.13.0") AND TV_REDUCE_APP_SIZE)
    tv_message(WARNING "'TV_REDUCE_APP_SIZE' requested but only available in CMake 3.13.0 or newer.")
    set(TV_REDUCE_APP_SIZE OFF)
endif()

option(TV_LIBRARY_UNITY_BUILD "Use CMake's Unity Build" OFF)
if ((CMAKE_VERSION VERSION_LESS "3.16.0") AND TV_LIBRARY_UNITY_BUILD)
    tv_message(WARNING "'TV_LIBRARY_UNITY_BUILD' requested but only available in CMake 3.16.0 or newer.")
    set(TV_LIBRARY_UNITY_BUILD OFF)
endif()

option(TV_OPTIMIZE_BUILD "Use CMake's Precompiled Headers" ON)

option(TV_BUILD_AVSCOLOR "Build AviSynth TermColor plugin" OFF)

tv_message_mp(STATUS "Install path: ${CMAKE_INSTALL_PREFIX}")
tv_message(STATUS "Build Examples: ${TV_BUILD_EXAMPLES}")
if (MAY_BUILD_USING_GPM)
    tv_message(STATUS "Build w/GPM: ${TV_BUILD_USING_GPM}")
endif()
if (MAY_USE_STATIC_RTL)
    tv_message_mp(STATUS "Link w/static RTL: ${TV_USE_STATIC_RTL}")
endif()

option(TV_BUILD_TESTS "Build and run tests" OFF)

# Library

add_subdirectory(source)

# Examples

add_subdirectory(examples)

# Test

add_subdirectory(test)
