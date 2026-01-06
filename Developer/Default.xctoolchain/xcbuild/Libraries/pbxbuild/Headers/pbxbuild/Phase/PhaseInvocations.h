/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Phase_PhaseInvocations_h
#define __pbxbuild_Phase_PhaseInvocations_h

#include <pbxbuild/Base.h>
#include <pbxbuild/Tool/AuxiliaryFile.h>
#include <pbxbuild/Tool/Invocation.h>

namespace pbxbuild {
namespace Phase {

class Environment;

class PhaseInvocations {
private:
    std::vector<Tool::Invocation>    _invocations;
    std::vector<Tool::AuxiliaryFile> _auxiliaryFiles;

public:
    PhaseInvocations(
        std::vector<Tool::Invocation> const &invocations,
        std::vector<Tool::AuxiliaryFile> const &auxiliaryFiles);
    ~PhaseInvocations();

public:
    std::vector<Tool::Invocation> const &invocations() const
    { return _invocations; }
    std::vector<Tool::AuxiliaryFile> const &auxiliaryFiles() const
    { return _auxiliaryFiles; }

public:
    static PhaseInvocations
    Create(Phase::Environment const &phaseEnvironment, pbxproj::PBX::Target::shared_ptr const &target);
};

}
}

#endif // !__pbxbuild_Phase_PhaseInvocations_h
