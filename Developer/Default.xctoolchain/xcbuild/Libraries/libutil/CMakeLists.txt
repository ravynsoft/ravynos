#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(util_xcbuild
            Sources/FSUtil.cpp
            Sources/Filesystem.cpp
            Sources/DefaultFilesystem.cpp
            Sources/MemoryFilesystem.cpp
            Sources/Permissions.cpp
            Sources/Absolute.cpp
            Sources/Relative.cpp
            Sources/Unix.cpp
            Sources/Windows.cpp
            #
            Sources/Options.cpp
            #
            Sources/Escape.cpp
            Sources/Wildcard.cpp
            #
            Sources/md5.c
            )

target_link_libraries(util_xcbuild PUBLIC ext)
target_include_directories(util_xcbuild PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS util_xcbuild DESTINATION usr/lib)

if (BUILD_TESTING)
  ADD_UNIT_GTEST(util_xcbuild MemoryFilesystem Tests/test_MemoryFilesystem.cpp)
  ADD_UNIT_GTEST(util_xcbuild FSUtil Tests/test_FSUtil.cpp)
  ADD_UNIT_GTEST(util_xcbuild Wildcard Tests/test_Wildcard.cpp)
  ADD_UNIT_GTEST(util_xcbuild Escape Tests/test_Escape.cpp)
  ADD_UNIT_GTEST(util_xcbuild Unix Tests/test_Unix.cpp)
  ADD_UNIT_GTEST(util_xcbuild Windows Tests/test_Windows.cpp)
endif ()
