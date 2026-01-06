/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_ClangResolver_h
#define __pbxbuild_Tool_ClangResolver_h

#include <pbxspec/Manager.h>
#include <pbxspec/PBX/Compiler.h>

#include <memory>
#include <string>
#include <vector>

namespace pbxsetting { class Environment; }

namespace pbxbuild {
namespace Tool {

class Context;
class Input;
class PrecompiledHeaderInfo;
class SearchPaths;

class ClangResolver {
private:
    pbxspec::PBX::Compiler::shared_ptr _compiler;

public:
    ClangResolver(pbxspec::PBX::Compiler::shared_ptr const &compiler);
    ~ClangResolver();

public:
    void resolveSource(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        Tool::Input const &input,
        std::string const &outputDirectory) const;
    void resolvePrecompiledHeader(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        PrecompiledHeaderInfo const &precompiledHeaderInfo) const;

public:
    pbxspec::PBX::Compiler::shared_ptr const &compiler() const
    { return _compiler; }

public:
    static std::string ToolIdentifier()
    { return "com.apple.compilers.gcc"; }

public:
    static std::unique_ptr<ClangResolver>
    Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains, std::string const &compilerIdentifier);
};

}
}

#endif // !__pbxbuild_Tool_ClangResolver_h
