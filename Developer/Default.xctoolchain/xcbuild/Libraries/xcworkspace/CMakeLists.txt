#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(xcworkspace
            Sources/XC/Workspace.cpp
            Sources/XC/FileRef.cpp
            Sources/XC/Group.cpp
            Sources/XC/GroupItem.cpp
            )

target_link_libraries(xcworkspace PUBLIC pbxsetting process util_xcbuild plist)
target_include_directories(xcworkspace PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS xcworkspace DESTINATION usr/lib)

add_executable(dump_xcworkspace Tools/dump_xcworkspace.cpp)
target_link_libraries(dump_xcworkspace xcworkspace xcscheme pbxproj util_xcbuild plist)

