/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/AppleScriptBuildPhase.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::AppleScriptBuildPhase;
using pbxproj::Context;

AppleScriptBuildPhase::
AppleScriptBuildPhase() :
    BuildPhase(Isa(), Type::AppleScript)
{
}

bool AppleScriptBuildPhase::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!BuildPhase::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("AppleScriptBuildPhase", dict, seen);

    auto CN = unpack.cast <plist::String> ("contextName");
    auto SC = unpack.coerce <plist::Boolean> ("isSharedContext");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (CN != nullptr) {
        _contextName = CN->value();
    }

    if (SC != nullptr) {
        _isSharedContext = SC->value();
    }

    return true;
}
