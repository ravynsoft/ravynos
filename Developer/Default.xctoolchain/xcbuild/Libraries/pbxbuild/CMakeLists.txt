#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

add_library(pbxbuild
            Sources/DirectedGraph.cpp
            Sources/HeaderMap.cpp
            Sources/DerivedDataHash.cpp
            Sources/WorkspaceContext.cpp
            Sources/FileTypeResolver.cpp
            Sources/Tool/AuxiliaryFile.cpp
            Sources/Tool/Context.cpp
            Sources/Tool/Environment.cpp
            Sources/Tool/Input.cpp
            Sources/Tool/Invocation.cpp
            Sources/Tool/Tokens.cpp
            Sources/Tool/OptionsResult.cpp
            Sources/Tool/CompilationInfo.cpp
            Sources/Tool/SwiftModuleInfo.cpp
            Sources/Tool/HeadermapInfo.cpp
            Sources/Tool/ModuleMapInfo.cpp
            Sources/Tool/PrecompiledHeaderInfo.cpp
            Sources/Tool/SearchPaths.cpp
            Sources/Tool/CopyResolver.cpp
            Sources/Tool/DittoResolver.cpp
            Sources/Tool/SymlinkResolver.cpp
            Sources/Tool/MakeDirectoryResolver.cpp
            Sources/Tool/HeadermapResolver.cpp
            Sources/Tool/ModuleMapResolver.cpp
            Sources/Tool/InfoPlistResolver.cpp
            Sources/Tool/AssetCatalogResolver.cpp
            Sources/Tool/InterfaceBuilderResolver.cpp
            Sources/Tool/InterfaceBuilderStoryboardLinkerResolver.cpp
            Sources/Tool/InterfaceBuilderCommon.cpp
            Sources/Tool/ClangResolver.cpp
            Sources/Tool/CompilerCommon.cpp
            Sources/Tool/LinkerResolver.cpp
            Sources/Tool/ScriptResolver.cpp
            Sources/Tool/SwiftResolver.cpp
            Sources/Tool/SwiftStandardLibraryResolver.cpp
            Sources/Tool/TouchResolver.cpp
            Sources/Tool/ToolResolver.cpp
            Sources/Phase/Context.cpp
            Sources/Phase/Environment.cpp
            Sources/Phase/File.cpp
            Sources/Phase/PhaseInvocations.cpp
            Sources/Phase/CopyFilesResolver.cpp
            Sources/Phase/HeadersResolver.cpp
            Sources/Phase/LegacyTargetResolver.cpp
            Sources/Phase/ModuleMapResolver.cpp
            Sources/Phase/ProductTypeResolver.cpp
            Sources/Phase/SwiftResolver.cpp
            Sources/Phase/ResourcesResolver.cpp
            Sources/Phase/FrameworksResolver.cpp
            Sources/Phase/SourcesResolver.cpp
            Sources/Phase/ShellScriptResolver.cpp
            Sources/Target/BuildRules.cpp
            Sources/Target/Environment.cpp
            Sources/Build/Context.cpp
            Sources/Build/Environment.cpp
            Sources/Build/DependencyResolver.cpp
            )

target_link_libraries(pbxbuild PUBLIC xcsdk xcworkspace xcscheme pbxproj pbxspec pbxsetting dependency process util_xcbuild plist ext)
target_include_directories(pbxbuild PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS pbxbuild DESTINATION usr/lib)

add_executable(dump_hmap Tools/dump_hmap.cpp)
target_link_libraries(dump_hmap pbxbuild util_xcbuild plist)

if (BUILD_TESTING)
  ADD_UNIT_GTEST(pbxbuild DirectedGraph Tests/test_DirectedGraph.cpp)
  ADD_UNIT_GTEST(pbxbuild OptionsResult Tests/test_OptionsResult.cpp)
  target_link_libraries(test_pbxbuild_OptionsResult PRIVATE pbxspec pbxsetting plist)
  ADD_UNIT_GTEST(pbxbuild DerivedDataHash Tests/test_DerivedDataHash.cpp)
endif ()

