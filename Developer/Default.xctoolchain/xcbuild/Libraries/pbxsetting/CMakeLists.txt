#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(pbxsetting
            Sources/Condition.cpp
            Sources/DefaultSettings.cpp
            Sources/Environment.cpp
            Sources/Level.cpp
            Sources/Setting.cpp
            Sources/Type.cpp
            Sources/Value.cpp
            Sources/XC/Config.cpp
            )

target_link_libraries(pbxsetting PUBLIC process util_xcbuild plist)
target_include_directories(pbxsetting PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
target_include_directories(pbxsetting PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/PrivateHeaders")
install(TARGETS pbxsetting DESTINATION usr/lib)

add_executable(dump_xcconfig Tools/dump_xcconfig.cpp)
target_link_libraries(dump_xcconfig pbxsetting util_xcbuild)

if (BUILD_TESTING)
  ADD_UNIT_GTEST(pbxsetting Condition Tests/test_Condition.cpp)
  ADD_UNIT_GTEST(pbxsetting Environment Tests/test_Environment.cpp)
  ADD_UNIT_GTEST(pbxsetting Setting Tests/test_Setting.cpp)
  ADD_UNIT_GTEST(pbxsetting Type Tests/test_Type.cpp)
  ADD_UNIT_GTEST(pbxsetting Value Tests/test_Value.cpp)
  ADD_UNIT_GTEST(pbxsetting Config Tests/test_Config.cpp)
endif ()

