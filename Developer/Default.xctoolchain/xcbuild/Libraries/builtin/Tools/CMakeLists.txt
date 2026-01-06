#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_executable(builtin-copy copy.cpp)
target_link_libraries(builtin-copy builtin)

add_executable(builtin-copyPlist copyPlist.cpp)
target_link_libraries(builtin-copyPlist builtin)

add_executable(builtin-copyStrings copyStrings.cpp)
target_link_libraries(builtin-copyStrings builtin)

add_executable(builtin-copyTiff copyTiff.cpp)
target_link_libraries(builtin-copyTiff builtin)

add_executable(builtin-infoPlistUtility infoPlistUtility.cpp)
target_link_libraries(builtin-infoPlistUtility builtin)

add_executable(builtin-lsRegisterURL lsRegisterURL.cpp)
target_link_libraries(builtin-lsRegisterURL builtin)

add_executable(builtin-productPackagingUtility productPackagingUtility.cpp)
target_link_libraries(builtin-productPackagingUtility builtin)

add_executable(builtin-validationUtility validationUtility.cpp)
target_link_libraries(builtin-validationUtility builtin)

add_executable(builtin-embeddedBinaryValidationUtility embeddedBinaryValidationUtility.cpp)
target_link_libraries(builtin-embeddedBinaryValidationUtility builtin)
