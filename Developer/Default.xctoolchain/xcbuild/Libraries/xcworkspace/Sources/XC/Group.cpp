/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcworkspace/XC/Group.h>
#include <xcworkspace/XC/FileRef.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcworkspace::XC::Group;

Group::
Group() :
    GroupItem(Type::Group)
{
}

bool Group::
parse(plist::Dictionary const *dict)
{
    if (!GroupItem::parse(dict)) {
        return false;
    }

    auto N = dict->value <plist::String> ("name");
    if (N != nullptr) {
        _name = N->value();
    }

    if (auto FRd = dict->value <plist::Dictionary> ("FileRef")) {
        auto FR = std::make_shared <FileRef> ();
        if (!FR->parse(FRd))
            return false;

        FR->_parent = this;
        _items.push_back(FR);
    } else if (auto FRs = dict->value <plist::Array> ("FileRef")) {
        for (size_t n = 0; n < FRs->count(); n++) {
            auto FRd = FRs->value <plist::Dictionary> (n);
            if (FRd == nullptr)
                continue;

            auto FR = std::make_shared <FileRef> ();
            if (!FR->parse(FRd))
                return false;

            FR->_parent = this;
            _items.push_back(FR);
        }
    }

    if (auto Gd = dict->value <plist::Dictionary> ("Group")) {
        auto G = std::make_shared <Group> ();
        if (!G->parse(Gd))
            return false;

        G->_parent = this;
        _items.push_back(G);
    } else if (auto Gs = dict->value <plist::Array> ("Group")) {
        for (size_t n = 0; n < Gs->count(); n++) {
            auto Gd = Gs->value <plist::Dictionary> (n);
            if (Gd == nullptr)
                continue;

            auto G = std::make_shared <Group> ();
            if (!G->parse(Gd))
                return false;

            G->_parent = this;
            _items.push_back(G);
        }
    }

    return true;
}
