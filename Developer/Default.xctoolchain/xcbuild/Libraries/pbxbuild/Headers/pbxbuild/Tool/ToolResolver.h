/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_ToolResolver_h
#define __pbxbuild_Tool_ToolResolver_h

#include <pbxbuild/Tool/Invocation.h>
#include <pbxbuild/Tool/Input.h>

namespace pbxbuild {
namespace Tool {

class Context;

class ToolResolver {
private:
    pbxspec::PBX::Tool::shared_ptr _tool;

public:
    explicit ToolResolver(pbxspec::PBX::Tool::shared_ptr const &tool);
    ~ToolResolver();

public:
    pbxspec::PBX::Tool::shared_ptr const &tool() const
    { return _tool; }

public:
    void resolve(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        std::vector<Tool::Input> const &inputs,
        std::string const &outputDirectory,
        std::string const &logMessage = "") const;
    void resolve(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        std::vector<Tool::Input> const &inputs,
        std::vector<std::string> const &outputs,
        std::string const &logMessage = "") const;

public:
    static std::unique_ptr<ToolResolver>
    Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains, std::string const &identifier);
};

}
}

#endif // !__pbxbuild_Tool_ToolResolver_h
