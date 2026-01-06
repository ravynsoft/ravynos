#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(pbxspec
            Sources/Manager.cpp
            Sources/SpecificationType.cpp
            Sources/PBX/Architecture.cpp
            Sources/PBX/BuildPhase.cpp
            Sources/PBX/BuildPhaseInjection.cpp
            Sources/PBX/BuildRule.cpp
            Sources/PBX/BuildSettings.cpp
            Sources/PBX/BuildStep.cpp
            Sources/PBX/BuildSystem.cpp
            Sources/PBX/Compiler.cpp
            Sources/PBX/FileType.cpp
            Sources/PBX/Linker.cpp
            Sources/PBX/PackageType.cpp
            Sources/PBX/ProductType.cpp
            Sources/PBX/PropertyOption.cpp
            Sources/PBX/Specification.cpp
            Sources/PBX/Tool.cpp
            )

target_link_libraries(pbxspec PUBLIC pbxsetting util_xcbuild plist ext)
target_include_directories(pbxspec PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
target_include_directories(pbxspec PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/PrivateHeaders")
install(TARGETS pbxspec DESTINATION usr/lib)

add_executable(dump_xcspec Tools/dump_xcspec.cpp)
target_link_libraries(dump_xcspec pbxspec)

