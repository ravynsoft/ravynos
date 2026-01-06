/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/ResourcesBuildPhase.h>

using pbxproj::PBX::ResourcesBuildPhase;

ResourcesBuildPhase::
ResourcesBuildPhase() :
    BuildPhase(Isa(), Type::Resources)
{
}
