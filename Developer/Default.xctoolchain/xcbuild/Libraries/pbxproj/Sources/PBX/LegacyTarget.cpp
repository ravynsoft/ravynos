/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/LegacyTarget.h>
#include <pbxproj/PBX/BuildPhases.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::LegacyTarget;
using pbxproj::Context;

LegacyTarget::
LegacyTarget() :
    Target                         (Isa(), Type::Legacy),
    _buildArgumentsString          (pbxsetting::Value::Empty()),
    _passBuildSettingsInEnvironment(false)
{
}

bool LegacyTarget::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Target::parse(context, dict, seen, false))
        return false;

    auto unpack = plist::Keys::Unpack("LegacyTarget", dict, seen);

    auto BWD   = unpack.cast <plist::String> ("buildWorkingDirectory");
    auto BTP   = unpack.cast <plist::String> ("buildToolPath");
    auto BAS   = unpack.cast <plist::String> ("buildArgumentsString");
    auto PBSIE = unpack.coerce <plist::Integer> ("passBuildSettingsInEnvironment");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (BWD != nullptr) {
        _buildWorkingDirectory = BWD->value();
    }

    if (BTP != nullptr) {
        _buildToolPath = BTP->value();
    }

    if (BAS != nullptr) {
        _buildArgumentsString = pbxsetting::Value::Parse(BAS->value());
    }

    if (PBSIE != nullptr) {
        _passBuildSettingsInEnvironment = (PBSIE->value() != 0);
    }

    return true;
}
