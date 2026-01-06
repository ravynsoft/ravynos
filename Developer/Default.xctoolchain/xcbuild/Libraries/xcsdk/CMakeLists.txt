#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(xcsdk
            Sources/Configuration.cpp
            Sources/Environment.cpp
            Sources/SDK/Manager.cpp
            Sources/SDK/Platform.cpp
            Sources/SDK/PlatformVersion.cpp
            Sources/SDK/Product.cpp
            Sources/SDK/Target.cpp
            Sources/SDK/Toolchain.cpp
            )

target_link_libraries(xcsdk PUBLIC pbxsetting process util_xcbuild plist)
target_include_directories(xcsdk PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS xcsdk DESTINATION usr/lib)

add_executable(xcrun_xcodebuild Tools/xcrun.cpp)
target_link_libraries(xcrun_xcodebuild xcsdk util_xcbuild)
set_target_properties(xcrun_xcodebuild PROPERTIES OUTPUT_NAME xcrun)
install(TARGETS xcrun_xcodebuild DESTINATION usr/bin)

add_executable(xcode-select_xcbuild Tools/xcode-select.cpp)
target_link_libraries(xcode-select_xcbuild xcsdk util_xcbuild)
set_target_properties(xcode-select_xcbuild PROPERTIES OUTPUT_NAME xcode-select)
install(TARGETS xcode-select_xcbuild DESTINATION usr/bin)

if (BUILD_TESTING)
  ADD_UNIT_GTEST(xcsdk PlatformVersion Tests/test_PlatformVersion.cpp)
  ADD_UNIT_GTEST(xcsdk Toolchain Tests/test_Toolchain.cpp)
  ADD_UNIT_GTEST(xcsdk Configuration Tests/test_Configuration.cpp)
  ADD_UNIT_GTEST(xcsdk Manager Tests/test_Manager.cpp)
endif ()
