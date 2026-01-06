/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Phase_ModuleMapResolver_h
#define __pbxbuild_Phase_ModuleMapResolver_h

#include <pbxbuild/Tool/AuxiliaryFile.h>
#include <pbxproj/PBX/Target.h>

#include <string>
#include <ext/optional>

namespace pbxsetting { class Environment; }

namespace pbxbuild {
namespace Phase {

class Context;
class Environment;

class ModuleMapResolver {
public:
    ModuleMapResolver();
    ~ModuleMapResolver();

public:
    bool resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext) const;

public:
    static ext::optional<Tool::AuxiliaryFile::Chunk>
    Contents(pbxsetting::Environment const &environment, pbxproj::PBX::Target::shared_ptr const &target, std::string const &workingDirectory);
};

}
}

#endif // !__pbxbuild_Phase_ModuleMapResolver_h
