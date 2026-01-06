/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/XC/VersionGroup.h>
#include <pbxproj/PBX/FileReference.h>
#include <pbxproj/Context.h>

using pbxproj::XC::VersionGroup;
using pbxproj::Context;

VersionGroup::
VersionGroup() :
    BaseGroup(Isa(), GroupItem::Type::VersionGroup)
{
}

bool VersionGroup::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!BaseGroup::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("VersionGroup", dict, seen);

    std::string CVID;

    auto CV  = context.indirect <PBX::FileReference> (&unpack, "currentVersion", &CVID);
    auto VGT = unpack.cast <plist::String> ("versionGroupType");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (CV != nullptr) {
        _currentVersion = context.parseObject(context.fileReferences, CVID, CV);
        if (!_currentVersion)
            return false;
    }

    if (VGT != nullptr) {
        _versionGroupType = VGT->value();
    }

    return true;
}
