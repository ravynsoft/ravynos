/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_SwiftStandardLibraryResolver_h
#define __pbxbuild_Tool_SwiftStandardLibraryResolver_h

#include <pbxbuild/Tool/Input.h>
#include <pbxbuild/Tool/Invocation.h>

namespace pbxbuild {
namespace Tool {

class Context;

class SwiftStandardLibraryResolver {
private:
    pbxspec::PBX::Tool::shared_ptr _tool;

public:
    SwiftStandardLibraryResolver(pbxspec::PBX::Tool::shared_ptr const &tool);

public:
    void resolve(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        Tool::Input const &executable,
        std::vector<std::string> const &directories) const;

public:
    pbxspec::PBX::Tool::shared_ptr const &tool() const
    { return _tool; }

public:
    static std::string ToolIdentifier()
    { return "com.apple.build-tools.swift-stdlib-tool"; }

public:
    static std::unique_ptr<SwiftStandardLibraryResolver>
    Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains);
};

}
}

#endif // !__pbxbuild_Tool_SwiftStandardLibraryResolver_h
