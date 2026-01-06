/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Phase_Environment_h
#define __pbxbuild_Phase_Environment_h

#include <pbxbuild/Base.h>
#include <pbxbuild/Build/Environment.h>
#include <pbxbuild/Build/Context.h>
#include <pbxbuild/Target/Environment.h>

namespace pbxbuild {
namespace Phase {

class Environment {
private:
    Build::Environment               _buildEnvironment;
    Build::Context                   _buildContext;
    pbxproj::PBX::Target::shared_ptr _target;
    Target::Environment              _targetEnvironment;

public:
    Environment(Build::Environment const &buildEnvironment, Build::Context const &buildContext, pbxproj::PBX::Target::shared_ptr const &target, Target::Environment const &targetEnvironment);
    ~Environment();

public:
    Build::Environment const &buildEnvironment() const
    { return _buildEnvironment; }
    Build::Context const &buildContext() const
    { return _buildContext; }
    pbxproj::PBX::Target::shared_ptr const &target() const
    { return _target; }
    Target::Environment const &targetEnvironment() const
    { return _targetEnvironment; }

public:
    static pbxsetting::Level
    VariantLevel(std::string const &variant);
    static pbxsetting::Level
    ArchitectureLevel(std::string const &arch);
};

}
}

#endif // !__pbxbuild_Phase_Environment_h
