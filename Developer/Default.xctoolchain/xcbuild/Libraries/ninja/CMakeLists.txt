#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(ninja
            Sources/Writer.cpp
            Sources/Value.cpp
            )

target_link_libraries(ninja PUBLIC)
target_include_directories(ninja PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS ninja DESTINATION usr/lib)

if (BUILD_TESTING)
  ADD_UNIT_GTEST(ninja Value Tests/test_Value.cpp)
  ADD_UNIT_GTEST(ninja Writer Tests/test_Writer.cpp)
endif ()

