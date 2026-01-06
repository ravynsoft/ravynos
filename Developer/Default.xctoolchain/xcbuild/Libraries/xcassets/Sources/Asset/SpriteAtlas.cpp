/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/SpriteAtlas.h>
#include <xcassets/Asset/ImageSet.h>
#include <plist/Keys/Unpack.h>
#include <plist/Array.h>
#include <plist/String.h>
#include <plist/Boolean.h>

using xcassets::Asset::SpriteAtlas;
using xcassets::Asset::ImageSet;

bool SpriteAtlas::
parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Asset::parse(dict, seen, false)) {
        return false;
    }

    /* No contents is allowed for groups. */
    if (dict != nullptr) {
        auto unpack = plist::Keys::Unpack("SpriteAtlas", dict, seen);

        auto P = unpack.cast <plist::Dictionary> ("properties");

        if (!unpack.complete(check)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
        }

        if (P != nullptr) {
            std::unordered_set<std::string> seen;
            auto unpack = plist::Keys::Unpack("Properties", P, &seen);

            auto CT   = unpack.cast <plist::String> ("compression-type");
            auto ODRT = unpack.cast <plist::Array> ("on-demand-resource-tags");
            auto PN   = unpack.cast <plist::Boolean> ("provides-namespace");

            if (!unpack.complete(true)) {
                fprintf(stderr, "%s", unpack.errorText().c_str());
            }

            if (CT != nullptr) {
                _compression = Compressions::Parse(CT->value());
            }

            if (ODRT != nullptr) {
                _onDemandResourceTags = std::vector<std::string>();
                _onDemandResourceTags->reserve(ODRT->count());

                for (size_t n = 0; n < ODRT->count(); n++) {
                    if (auto string = ODRT->value<plist::String>(n)) {
                        _onDemandResourceTags->push_back(string->value());
                    }
                }
            }

            if (PN != nullptr) {
                _providesNamespace = PN->value();
            }
        }
    }

    return true;
}

