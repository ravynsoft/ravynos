/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/BuildPhaseInjection.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxspec::PBX::BuildPhaseInjection;

BuildPhaseInjection::
BuildPhaseInjection()
{
}

bool BuildPhaseInjection::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("BuildPhaseInjection", dict, &seen);

    auto BP     = unpack.cast <plist::String> ("BuildPhase");
    auto N      = unpack.cast <plist::String> ("Name");
    auto ROFDP  = unpack.coerce <plist::Boolean> ("RunOnlyForDeploymentPostprocessing");
    auto NRSPFF = unpack.coerce <plist::Boolean> ("NeedsRunpathSearchPathForFrameworks");
    auto DSS    = unpack.coerce <plist::Integer> ("DstSubFolderSpec");
    auto DP     = unpack.cast <plist::String> ("DstPath");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (BP != nullptr) {
        _buildPhase = BP->value();
    }

    if (N != nullptr) {
        _name = N->value();
    }

    if (ROFDP != nullptr) {
        _runOnlyForDeploymentPostprocessing = ROFDP->value();
    }

    if (NRSPFF != nullptr) {
        _needsRunpathSearchPathForFrameworks = NRSPFF->value();
    }

    if (DSS != nullptr) {
        _dstSubfolderSpec = DSS->value();
    }

    if (DP != nullptr) {
        _dstPath = pbxsetting::Value::Parse(DP->value());
    }

    return true;
}
