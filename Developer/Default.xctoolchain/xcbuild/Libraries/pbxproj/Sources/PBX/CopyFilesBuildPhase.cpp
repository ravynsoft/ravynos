/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/CopyFilesBuildPhase.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::CopyFilesBuildPhase;
using pbxproj::Context;

CopyFilesBuildPhase::
CopyFilesBuildPhase() :
    BuildPhase       (Isa(), Type::CopyFiles),
    _dstPath         (pbxsetting::Value::Empty()),
    _dstSubfolderSpec(kDestinationAbsolute)
{
}

bool CopyFilesBuildPhase::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!BuildPhase::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("CopyFilesBuildPhase", dict, seen);

    auto DP  = unpack.cast <plist::String> ("dstPath");
    auto DSS = unpack.coerce <plist::Integer> ("dstSubfolderSpec");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (DP != nullptr) {
        _dstPath = pbxsetting::Value::Parse(DP->value());
    }

    if (DSS != nullptr) {
        _dstSubfolderSpec = static_cast<Destination>(DSS->value());
    }

    return true;
}
