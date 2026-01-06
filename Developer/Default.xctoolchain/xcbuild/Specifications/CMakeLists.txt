#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

set(OLD_DEFAULT_COMPONENT ${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME})
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME specifications)

add_subdirectory(BuildPhase)
add_subdirectory(BuildRules)
add_subdirectory(BuildSystem)
add_subdirectory(Compiler)
add_subdirectory(FileType)
add_subdirectory(Linker)
add_subdirectory(Tool)

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME ${OLD_DEFAULT_COMPONENT})
