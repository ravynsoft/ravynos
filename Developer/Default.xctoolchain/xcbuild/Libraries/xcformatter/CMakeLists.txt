#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(xcformatter
            Sources/Formatter.cpp
            Sources/DefaultFormatter.cpp
            Sources/NullFormatter.cpp
            )

target_link_libraries(xcformatter PUBLIC pbxbuild pbxproj pbxsetting)
target_include_directories(xcformatter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS xcformatter DESTINATION usr/lib)
