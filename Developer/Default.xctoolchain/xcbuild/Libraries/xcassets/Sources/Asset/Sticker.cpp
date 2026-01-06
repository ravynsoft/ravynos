/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/Sticker.h>
#include <plist/Keys/Unpack.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcassets::Asset::Sticker;

bool Sticker::
parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!this->children().empty()) {
        fprintf(stderr, "warning: unexpected child assets\n");
    }

    if (!Asset::parse(dict, seen, false)) {
        return false;
    }

    /* Contents is required. */
    if (dict == nullptr) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("Sticker", dict, seen);

    auto P = unpack.cast <plist::Dictionary> ("properties");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (P != nullptr) {
        std::unordered_set<std::string> seen;
        auto unpack = plist::Keys::Unpack("Properties", P, &seen);

        auto AL = unpack.cast <plist::String> ("accessibility-label");
        auto F  = unpack.cast <plist::String> ("filename");

        if (!unpack.complete(true)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
        }

        if (AL != nullptr) {
            _accessibilityLabel = AL->value();
        }

        if (F != nullptr) {
            _fileName = F->value();
        }
    }

    return true;
}

