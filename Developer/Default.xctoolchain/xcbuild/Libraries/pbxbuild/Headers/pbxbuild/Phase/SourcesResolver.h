/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Phase_SourcesResolver_h
#define __pbxbuild_Phase_SourcesResolver_h

#include <pbxbuild/Base.h>

namespace pbxbuild {
namespace Phase {

class Environment;
class Context;

class SourcesResolver {
private:
    pbxproj::PBX::SourcesBuildPhase::shared_ptr _buildPhase;

public:
    explicit SourcesResolver(pbxproj::PBX::SourcesBuildPhase::shared_ptr const &buildPhase);
    ~SourcesResolver();

public:
    pbxproj::PBX::SourcesBuildPhase::shared_ptr const &buildPhase() const
    { return _buildPhase; }

public:
    bool resolve(Phase::Environment const &phaseEnvironment, Phase::Context *Context);
};

}
}

#endif // !__pbxbuild_Phase_SourcesResolver_h
