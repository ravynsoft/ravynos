/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Phase/ShellScriptResolver.h>
#include <pbxbuild/Phase/Environment.h>
#include <pbxbuild/Phase/Context.h>
#include <pbxbuild/Tool/ScriptResolver.h>

namespace Phase = pbxbuild::Phase;
namespace Tool = pbxbuild::Tool;

Phase::ShellScriptResolver::
ShellScriptResolver(pbxproj::PBX::ShellScriptBuildPhase::shared_ptr const &buildPhase) :
    _buildPhase(buildPhase)
{
}

Phase::ShellScriptResolver::
~ShellScriptResolver()
{
}

bool Phase::ShellScriptResolver::
resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext)
{
    Tool::ScriptResolver const *scriptResolver = phaseContext->scriptResolver(phaseEnvironment);
    if (scriptResolver == nullptr) {
        return false;
    }

    pbxsetting::Environment const &environment = phaseEnvironment.targetEnvironment().environment();

    scriptResolver->resolve(&phaseContext->toolContext(), environment, _buildPhase);
    return true;
}
