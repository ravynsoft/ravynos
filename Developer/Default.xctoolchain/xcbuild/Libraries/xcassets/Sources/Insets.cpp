/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Insets.h>
#include <plist/Keys/Unpack.h>
#include <plist/Real.h>

using xcassets::Insets;

bool Insets::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("Insets", dict, &seen);

    auto T = unpack.coerce <plist::Real> ("top");
    auto L = unpack.coerce <plist::Real> ("left");
    auto B = unpack.coerce <plist::Real> ("bottom");
    auto R = unpack.coerce <plist::Real> ("right");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (T != nullptr) {
        _top = T->value();
    }

    if (L != nullptr) {
        _left = L->value();
    }

    if (B != nullptr) {
        _bottom = B->value();
    }

    if (R != nullptr) {
        _right = R->value();
    }

    return true;
}

