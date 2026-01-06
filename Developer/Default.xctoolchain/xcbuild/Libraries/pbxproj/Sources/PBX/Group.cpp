/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/Group.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::Group;
using pbxproj::Context;

Group::
Group() :
    BaseGroup   (Isa(), GroupItem::Type::Group),
    _indentWidth(0),
    _tabWidth   (0)
{
}

bool Group::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!BaseGroup::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("Group", dict, seen);

    auto IW = unpack.coerce <plist::Integer> ("indentWidth");
    auto TW = unpack.coerce <plist::Integer> ("tabWidth");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (IW != nullptr) {
        _indentWidth = IW->value();
    }

    if (TW != nullptr) {
        _tabWidth = TW->value();
    }

    return true;
}
