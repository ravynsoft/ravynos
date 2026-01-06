/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_InfoPlistResolver_h
#define __pbxbuild_Tool_InfoPlistResolver_h

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

class InfoPlistResolver {
private:
    pbxspec::PBX::Tool::shared_ptr _tool;

public:
    explicit InfoPlistResolver(pbxspec::PBX::Tool::shared_ptr const &tool);
    ~InfoPlistResolver();

public:
    pbxspec::PBX::Tool::shared_ptr const &tool() const
    { return _tool; }

public:
    void resolve(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        Tool::Input const &input) const;

public:
    static std::string ToolIdentifier()
    { return "com.apple.tools.info-plist-utility"; }

public:
    static std::unique_ptr<InfoPlistResolver>
    Create(pbxspec::Manager::shared_ptr const &specManager, std::vector<std::string> const &specDomains);
};

}
}

#endif // !__pbxbuild_Tool_InfoPlistResolver_h
