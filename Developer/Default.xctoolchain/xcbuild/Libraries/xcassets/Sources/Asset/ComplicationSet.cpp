/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/ComplicationSet.h>
#include <plist/Array.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using xcassets::Asset::ComplicationSet;

bool ComplicationSet::ComplicationAsset::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("ComplicationSetComplicationAsset", dict, &seen);

    auto F = unpack.cast <plist::String> ("filename");
    auto I = unpack.cast <plist::String> ("idiom");
    auto R = unpack.cast <plist::String> ("role");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (I != nullptr) {
        _idiom = Slot::Idioms::Parse(I->value());
    }

    if (F != nullptr) {
        _fileName = F->value();
    }

    if (R != nullptr) {
        _role = WatchComplicationRoles::Parse(R->value());
    }

    return true;
}

bool ComplicationSet::
parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Asset::parse(dict, seen, false)) {
        return false;
    }

    /* Contents is required. */
    if (dict == nullptr) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("ComplicationSet", dict, seen);

    auto As = unpack.cast <plist::Array> ("assets");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (As != nullptr) {
        _assets = std::vector<ComplicationAsset>();

        for (size_t n = 0; n < As->count(); ++n) {
            if (auto dict = As->value<plist::Dictionary>(n)) {
                ComplicationAsset asset;
                if (asset.parse(dict)) {
                    _assets->push_back(asset);
                }
            }
        }
    }

    return true;
}


