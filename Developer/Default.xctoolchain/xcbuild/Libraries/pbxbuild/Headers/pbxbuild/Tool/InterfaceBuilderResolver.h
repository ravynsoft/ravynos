/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_InterfaceBuilderResolver_h
#define __pbxbuild_Tool_InterfaceBuilderResolver_h

#include <pbxbuild/Tool/Input.h>

namespace pbxbuild {
namespace Tool {

class Context;

class InterfaceBuilderResolver {
private:
    pbxspec::PBX::Compiler::shared_ptr _tool;

private:
    explicit InterfaceBuilderResolver(pbxspec::PBX::Compiler::shared_ptr const &tool);

public:
    void resolve(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        std::vector<Tool::Input> const &input) const;

public:
    static std::string CompilerToolIdentifier()
    { return "com.apple.xcode.tools.ibtool.compiler"; }
    static std::string StoryboardCompilerToolIdentifier()
    { return "com.apple.xcode.tools.ibtool.storyboard.compiler"; }

public:
    static std::string PostprocessorToolIdentifier()
    { return "com.apple.xcode.tools.ibtool.postprocessor"; }
    static std::string StoryboardPostprocessorToolIdentifier()
    { return "com.apple.xcode.tools.ibtool.storyboard.postprocessor"; }

public:
    static std::unique_ptr<InterfaceBuilderResolver>
    Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains, std::string const &toolIdentifier);
};

}
}

#endif // !__pbxbuild_Tool_InterfaceBuilderResolver_h
