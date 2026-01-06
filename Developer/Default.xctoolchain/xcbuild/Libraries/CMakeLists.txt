#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

set(OLD_DEFAULT_COMPONENT ${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME})
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME libraries)

add_subdirectory(acdriver)
add_subdirectory(builtin)
add_subdirectory(dependency)
add_subdirectory(ext)
add_subdirectory(graphics)
add_subdirectory(libcar)
add_subdirectory(libbom)
add_subdirectory(libutil)
add_subdirectory(ninja)
add_subdirectory(pbxbuild)
add_subdirectory(pbxproj)
add_subdirectory(pbxsetting)
add_subdirectory(pbxspec)
add_subdirectory(plist)
add_subdirectory(process)
add_subdirectory(xcassets)
add_subdirectory(xcdriver)
add_subdirectory(xcexecution)
add_subdirectory(xcformatter)
add_subdirectory(xcscheme)
add_subdirectory(xcsdk)
add_subdirectory(xcworkspace)

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME ${OLD_DEFAULT_COMPONENT})

