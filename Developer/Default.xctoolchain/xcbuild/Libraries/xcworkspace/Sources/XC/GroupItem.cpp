/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcworkspace/XC/GroupItem.h>
#include <xcworkspace/XC/Workspace.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcworkspace::XC::GroupItem;
using xcworkspace::XC::Workspace;

GroupItem::
GroupItem(Type type) :
    _type(type)
{
}

std::string GroupItem::
resolve(std::shared_ptr<Workspace> const &workspace) const
{
    std::string location = _location.empty() ? "" : "/" + _location;

    if (_locationType == "container") {
        return workspace->basePath() + location;
    } else if (_locationType == "self") {
        // TODO(grp): Verify this is correct.
        return workspace->basePath() + "/.." + location;
    } else if (_locationType == "group") {
        if (_parent != nullptr) {
            return _parent->resolve(workspace) + location;
        } else {
            return workspace->basePath() + location;
        }
    } else if (_locationType == "absolute") {
        return location;
    } else if (_locationType == "developer") {
        // TODO(grp): Look in DEVELOPER_DIR.
        return location;
    } else {
        fprintf(stderr, "error: unknown container type %s\n", _locationType.c_str());
        return location;
    }
}

bool GroupItem::
parse(plist::Dictionary const *dict)
{
    auto L = dict->value <plist::String> ("location");

    if (L != nullptr) {
        std::string location = L->value();
        size_t colon = location.find(':');
        if (colon != std::string::npos) {
            _location     = location.substr(colon + 1);
            _locationType = location.substr(0, colon);
        }
    }

    return true;
}
