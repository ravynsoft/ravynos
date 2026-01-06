#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#


add_library(xcexecution
            Sources/Parameters.cpp
            Sources/Executor.cpp
            Sources/SimpleExecutor.cpp
            Sources/NinjaExecutor.cpp
            )

target_link_libraries(xcexecution PUBLIC xcformatter pbxbuild xcscheme xcworkspace pbxproj pbxsetting process util_xcbuild dependency ninja builtin)
target_include_directories(xcexecution PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS xcexecution DESTINATION usr/lib)

if (BUILD_TESTING)
  ADD_UNIT_GTEST(xcexecution SimpleExecutor Tests/test_SimpleExecutor.cpp)
endif ()
