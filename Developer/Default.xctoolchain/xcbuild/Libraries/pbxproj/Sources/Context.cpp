/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/Context.h>
#include <pbxproj/PBX/Project.h>

using pbxproj::Context;
namespace PBX = pbxproj::PBX;

void Context::
cacheObject(PBX::Object::shared_ptr const &O, std::string const &id)
{
    if (project == nullptr && O->isa <PBX::Project> ()) {
        project = std::static_pointer_cast <PBX::Project> (O);
    }

    O->setBlueprintIdentifier(id);

    if (project != nullptr && project != O) {
        project->cacheObject(O);
    }
}
