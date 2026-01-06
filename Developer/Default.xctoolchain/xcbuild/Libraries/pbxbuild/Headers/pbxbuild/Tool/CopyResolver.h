/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_CopyResolver_h
#define __pbxbuild_Tool_CopyResolver_h

#include <pbxspec/Manager.h>
#include <pbxspec/PBX/Tool.h>

#include <memory>
#include <string>
#include <vector>

namespace pbxsetting { class Environment; }

namespace pbxbuild {
namespace Tool {

class Context;
class Input;

class CopyResolver {
private:
    pbxspec::PBX::Tool::shared_ptr _tool;

private:
    explicit CopyResolver(pbxspec::PBX::Tool::shared_ptr const &tool);

public:
    void resolve(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        std::vector<Tool::Input> const &input,
        std::string const &outputDirectory,
        std::string const &logMessageTitle) const;

public:
    static std::string ToolIdentifier()
    { return "com.apple.compilers.pbxcp"; }

public:
    static std::unique_ptr<CopyResolver>
    Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains);
};

}
}

#endif // !__pbxbuild_Tool_CopyResolver_h
