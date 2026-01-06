/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_ModuleMapResolver_h
#define __pbxbuild_Tool_ModuleMapResolver_h

#include <pbxproj/PBX/Target.h>

namespace pbxsetting { class Environment; }

namespace pbxbuild {
namespace Tool {

class Context;

class ModuleMapResolver {
public:
    ModuleMapResolver();
    ~ModuleMapResolver();

public:
    void resolve(
        Tool::Context *toolContext,
        pbxsetting::Environment const &environment,
        pbxproj::PBX::Target::shared_ptr const &target) const;

public:
    static std::unique_ptr<ModuleMapResolver>
    Create();
};

}
}

#endif // !__pbxbuild_Tool_ModuleMapResolver_h
