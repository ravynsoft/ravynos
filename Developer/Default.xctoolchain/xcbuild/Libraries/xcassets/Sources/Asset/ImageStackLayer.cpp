/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/ImageStackLayer.h>
#include <plist/Keys/Unpack.h>

using xcassets::Asset::ImageStackLayer;

bool ImageStackLayer::
parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Asset::parse(dict, seen, false)) {
        return false;
    }

    /* Contents is required. */
    if (dict == nullptr) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("ImageStackLayer", dict, seen);

    auto P = unpack.cast <plist::Dictionary> ("properties");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (P != nullptr) {
        std::unordered_set<std::string> seen;
        auto unpack = plist::Keys::Unpack("Properties", P, &seen);

        auto CR = unpack.cast <plist::Dictionary> ("content-reference");
        // TODO: frame-size
        // TODO: frame-center

        if (!unpack.complete(true)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
        }

        if (CR != nullptr) {
            ContentReference contentReference;
            if (contentReference.parse(CR)) {
                _contentReference = contentReference;
            }
        }
    }

    return true;
}
