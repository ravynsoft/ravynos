#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(xcscheme
            Sources/SchemeGroup.cpp
            Sources/XC/Action.cpp
            Sources/XC/ActionContent.cpp
            Sources/XC/AdditionalOption.cpp
            Sources/XC/AnalyzeAction.cpp
            Sources/XC/ArchiveAction.cpp
            Sources/XC/CommandLineArgument.cpp
            Sources/XC/BuildAction.cpp
            Sources/XC/BuildActionEntry.cpp
            Sources/XC/BuildableReference.cpp
            Sources/XC/ExecuteAction.cpp
            Sources/XC/LaunchAction.cpp
            Sources/XC/LocationScenarioReference.cpp
            Sources/XC/ProfileAction.cpp
            Sources/XC/Scheme.cpp
            Sources/XC/Test.cpp
            Sources/XC/TestAction.cpp
            Sources/XC/TestableReference.cpp
            )

target_link_libraries(xcscheme PUBLIC pbxsetting util_xcbuild plist)
target_include_directories(xcscheme PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS xcscheme DESTINATION usr/lib)
