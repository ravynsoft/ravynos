/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_Context_h
#define __pbxbuild_Tool_Context_h

#include <pbxbuild/Tool/Invocation.h>
#include <pbxbuild/Tool/HeadermapInfo.h>
#include <pbxbuild/Tool/CompilationInfo.h>
#include <pbxbuild/Tool/SwiftModuleInfo.h>
#include <pbxbuild/Tool/ModuleMapInfo.h>
#include <pbxbuild/Tool/PrecompiledHeaderInfo.h>
#include <pbxbuild/Tool/SearchPaths.h>
#include <xcsdk/SDK/Target.h>
#include <xcsdk/SDK/Toolchain.h>

#include <map>
#include <string>
#include <vector>

#define PHASE_INVOCATION_PRIORITY_BASE 0x100
#define PHASE_INVOCATION_PRIORITY_INCREMENT 0x100

namespace pbxbuild {
namespace Tool {

class Context {
private:
    xcsdk::SDK::Target::shared_ptr   _sdk;
    std::vector<xcsdk::SDK::Toolchain::shared_ptr> _toolchains;
    std::string                      _workingDirectory;

private:
    SearchPaths                      _searchPaths;

private:
    HeadermapInfo                    _headermapInfo;
    ModuleMapInfo                    _moduleMapInfo;
    CompilationInfo                  _compilationInfo;
    std::vector<SwiftModuleInfo>     _swiftModuleInfo;
    std::vector<std::string>         _additionalInfoPlistContents;

private:
    std::vector<Tool::Invocation>    _invocations;
    std::map<std::pair<std::string, std::string>, std::vector<Tool::Invocation>> _variantArchitectureInvocations;

private:
    std::vector<Tool::AuxiliaryFile> _auxiliaryFiles;

private:
    uint32_t                         _currentPhaseInvocationPriority;

public:
    Context(
        xcsdk::SDK::Target::shared_ptr const &sdk,
        std::vector<xcsdk::SDK::Toolchain::shared_ptr> const &toolchains,
        std::string const &workingDirectory,
        SearchPaths const &searchPaths);
    ~Context();

public:
    xcsdk::SDK::Target::shared_ptr const &sdk() const
    { return _sdk; }
    std::vector<xcsdk::SDK::Toolchain::shared_ptr> const &toolchains() const
    { return _toolchains; }
    std::string const &workingDirectory() const
    { return _workingDirectory; }

public:
    SearchPaths const &searchPaths() const
    { return _searchPaths; }

public:
    HeadermapInfo const &headermapInfo() const
    { return _headermapInfo; }
    ModuleMapInfo const &moduleMapInfo() const
    { return _moduleMapInfo; }
    CompilationInfo const &compilationInfo() const
    { return _compilationInfo; }
    std::vector<SwiftModuleInfo> const &swiftModuleInfo() const
    { return _swiftModuleInfo; }
    std::vector<std::string> const &additionalInfoPlistContents() const
    { return _additionalInfoPlistContents; }

public:
    HeadermapInfo &headermapInfo()
    { return _headermapInfo; }
    ModuleMapInfo &moduleMapInfo()
    { return _moduleMapInfo; }
    CompilationInfo &compilationInfo()
    { return _compilationInfo; }
    std::vector<SwiftModuleInfo> &swiftModuleInfo()
    { return _swiftModuleInfo; }
    std::vector<std::string> &additionalInfoPlistContents()
    { return _additionalInfoPlistContents; }

public:
    std::vector<Tool::Invocation> const &invocations() const
    { return _invocations; }
    std::map<std::pair<std::string, std::string>, std::vector<Tool::Invocation>> const &variantArchitectureInvocations() const
    { return _variantArchitectureInvocations; }

public:
    std::vector<Tool::Invocation> &invocations()
    { return _invocations; }
    std::map<std::pair<std::string, std::string>, std::vector<Tool::Invocation>> &variantArchitectureInvocations()
    { return _variantArchitectureInvocations; }

public:
    std::vector<Tool::AuxiliaryFile> const &auxiliaryFiles() const
    { return _auxiliaryFiles; }

public:
    std::vector<Tool::AuxiliaryFile> &auxiliaryFiles()
    { return _auxiliaryFiles; }

public:
    uint32_t currentPhaseInvocationPriority() const
    { return _currentPhaseInvocationPriority; }

public:
    uint32_t &currentPhaseInvocationPriority()
    { return _currentPhaseInvocationPriority; }

};

}
}

#endif // !__pbxbuild_Tool_Context_h
