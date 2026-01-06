/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/GroupItem.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::GroupItem;
using pbxproj::Context;

GroupItem::
GroupItem(std::string const &isa, Type type) :
    Object (isa),
    _type  (type),
    _parent(nullptr)
{
}

pbxsetting::Value GroupItem::
resolve(void) const
{
    std::string path = _path;
    if (_type != Type::Group && _type != Type::VariantGroup) {
        path = path.empty() ? _name : path;
    }
    path = path.empty() ? path : "/" + path;

    if (_sourceTree.empty() || _sourceTree == "<absolute>") {
        return pbxsetting::Value::String(path);
    } else if (_sourceTree == "<group>") {
        if (_parent != nullptr) {
            return _parent->resolve() + pbxsetting::Value::String(path);
        } else {
            return pbxsetting::Value::Variable("SOURCE_ROOT") + pbxsetting::Value::String(path);
        }
    } else {
        return pbxsetting::Value::Variable(_sourceTree) + pbxsetting::Value::String(path);
    }
}

bool GroupItem::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Object::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("GroupItem", dict, seen);

    auto N  = unpack.cast <plist::String> ("name");
    auto ST = unpack.cast <plist::String> ("sourceTree");
    auto P  = unpack.cast <plist::String> ("path");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (N != nullptr) {
        _name = N->value();
    }

    if (ST != nullptr) {
        _sourceTree = ST->value();
    }

    if (P != nullptr) {
        _path = P->value();
    }

    return true;
}
