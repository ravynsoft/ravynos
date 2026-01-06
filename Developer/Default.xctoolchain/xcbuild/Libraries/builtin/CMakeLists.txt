#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(builtin
            Sources/Driver.cpp
            Sources/Registry.cpp
            #
            Sources/copy/Options.cpp
            Sources/copy/Driver.cpp
            #
            Sources/copyPlist/Options.cpp
            Sources/copyPlist/Driver.cpp
            #
            Sources/copyStrings/Options.cpp
            Sources/copyStrings/Driver.cpp
            #
            Sources/copyTiff/Options.cpp
            Sources/copyTiff/Driver.cpp
            #
            Sources/infoPlistUtility/Options.cpp
            Sources/infoPlistUtility/Driver.cpp
            #
            Sources/lsRegisterURL/Options.cpp
            Sources/lsRegisterURL/Driver.cpp
            #
            Sources/productPackagingUtility/Options.cpp
            Sources/productPackagingUtility/Driver.cpp
            #
            Sources/validationUtility/Options.cpp
            Sources/validationUtility/Driver.cpp
            #
            Sources/embeddedBinaryValidationUtility/Options.cpp
            Sources/embeddedBinaryValidationUtility/Driver.cpp
            )

find_library(CORE_FOUNDATION CoreFoundation)
if ("${CORE_FOUNDATION}" STREQUAL "CORE_FOUNDATION-NOTFOUND")
  set(CORE_FOUNDATION "")
endif ()

find_library(CORE_SERVICES CoreServices)
if ("${CORE_SERVICES}" STREQUAL "CORE_SERVICES-NOTFOUND")
  set(CORE_SERVICES "")
endif ()

target_link_libraries(builtin PUBLIC dependency util_xcbuild plist pbxsetting ${CORE_FOUNDATION} ${CORE_SERVICES})
target_include_directories(builtin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS builtin DESTINATION usr/lib)

add_executable(builtin-copy Tools/copy.cpp)
target_link_libraries(builtin-copy builtin)
install(TARGETS builtin-copy DESTINATION usr/bin)

add_executable(builtin-copyPlist Tools/copyPlist.cpp)
target_link_libraries(builtin-copyPlist builtin)
install(TARGETS builtin-copyPlist DESTINATION usr/bin)

add_executable(builtin-copyStrings Tools/copyStrings.cpp)
target_link_libraries(builtin-copyStrings builtin)
install(TARGETS builtin-copyStrings DESTINATION usr/bin)

add_executable(builtin-copyTiff Tools/copyTiff.cpp)
target_link_libraries(builtin-copyTiff builtin)
install(TARGETS builtin-copyTiff DESTINATION usr/bin)

add_executable(builtin-infoPlistUtility Tools/infoPlistUtility.cpp)
target_link_libraries(builtin-infoPlistUtility builtin)
install(TARGETS builtin-infoPlistUtility DESTINATION usr/bin)

add_executable(builtin-lsRegisterURL Tools/lsRegisterURL.cpp)
target_link_libraries(builtin-lsRegisterURL builtin)
install(TARGETS builtin-lsRegisterURL DESTINATION usr/bin)

add_executable(builtin-productPackagingUtility Tools/productPackagingUtility.cpp)
target_link_libraries(builtin-productPackagingUtility builtin)
install(TARGETS builtin-productPackagingUtility DESTINATION usr/bin)

add_executable(builtin-validationUtility Tools/validationUtility.cpp)
target_link_libraries(builtin-validationUtility builtin)
install(TARGETS builtin-validationUtility DESTINATION usr/bin)

add_executable(builtin-embeddedBinaryValidationUtility Tools/embeddedBinaryValidationUtility.cpp)
target_link_libraries(builtin-embeddedBinaryValidationUtility builtin)
install(TARGETS builtin-embeddedBinaryValidationUtility DESTINATION usr/bin)

if (BUILD_TESTING)
  ADD_UNIT_GTEST(builtin copy Tests/test_copy.cpp)
  ADD_UNIT_GTEST(builtin copyStrings Tests/test_copyStrings.cpp)
  ADD_UNIT_GTEST(builtin copyPlist Tests/test_copyPlist.cpp)
endif ()
