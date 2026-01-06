/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_SwiftResolver_h
#define __pbxbuild_Tool_SwiftResolver_h

#include <pbxbuild/Tool/Input.h>
#include <pbxbuild/Tool/DittoResolver.h>

namespace pbxbuild {
namespace Tool {

class Context;

class SwiftResolver {
private:
    pbxspec::PBX::Compiler::shared_ptr _compiler;

private:
    explicit SwiftResolver(pbxspec::PBX::Compiler::shared_ptr const &compiler);

public:
    void resolve(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        std::vector<Tool::Input> const &input,
        std::string const &outputDirectory) const;

public:
    static std::string ToolIdentifier()
    { return "com.apple.xcode.tools.swift.compiler"; }

public:
    static std::unique_ptr<SwiftResolver>
    Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains);
};

}
}

#endif // !__pbxbuild_Tool_SwiftResolver_h
