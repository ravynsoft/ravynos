#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

project(xcbuild C CXX)

cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0054 NEW)
include(../../cmake/Paths.cmake)

set(BUILD_SHARED_LIBS True CACHE BOOL "Build shared libraries." FORCE)

set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_BUILD_RPATH \$ORIGIN/../lib)
set(CMAKE_INSTALL_RPATH \$ORIGIN/../lib)
set(CMAKE_INSTALL_PREFIX ${TOOLCHAIN})

# Output into root build directory.
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/external/xcbuild")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/external/xcbuild")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

add_compile_options(-Wall)

# Enable unit testing.
#include(CTest)
set(BUILD_TESTING False)

if (BUILD_TESTING)
  add_subdirectory(ThirdParty/googletest/googletest)

  function (ADD_UNIT_GTEST LIBRARY NAME SOURCES)
    set(TARGET_NAME "test_${LIBRARY}_${NAME}")
    add_executable("${TARGET_NAME}" ${SOURCES})
    target_link_libraries("${TARGET_NAME}" PRIVATE "${LIBRARY}" gtest gtest_main)
    target_include_directories("${TARGET_NAME}" PRIVATE "${CMAKE_SOURCE_DIR}/ThirdParty/googletest/googletest/include")
    add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
  endfunction ()
endif ()

add_subdirectory(Libraries)
add_subdirectory(Specifications)
