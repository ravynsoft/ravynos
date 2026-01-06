/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/StickerPack.h>
#include <plist/Array.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using xcassets::Asset::StickerPack;

bool StickerPack::Sticker::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("StickerPackSticker", dict, &seen);

    auto F = unpack.cast <plist::String> ("filename");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (F != nullptr) {
        _fileName = F->value();
    }

    return true;
}

bool StickerPack::
parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Asset::parse(dict, seen, false)) {
        return false;
    }

    /* Contents is required. */
    if (dict == nullptr) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("StickerPack", dict, seen);

    auto P  = unpack.cast <plist::Dictionary> ("properties");
    auto Ss = unpack.cast <plist::Array> ("stickers");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (P != nullptr) {
        std::unordered_set<std::string> seen;
        auto unpack = plist::Keys::Unpack("Properties", P, &seen);

        auto GS = unpack.cast <plist::String> ("grid-size");

        if (!unpack.complete(true)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
        }

        if (GS != nullptr) {
            _gridSize = StickerGridSizes::Parse(GS->value());
        }
    }

    if (Ss != nullptr) {
        _stickers = std::vector<Sticker>();

        for (size_t n = 0; n < Ss->count(); ++n) {
            if (auto dict = Ss->value<plist::Dictionary>(n)) {
                Sticker sticker;
                if (sticker.parse(dict)) {
                    _stickers->push_back(sticker);
                }
            }
        }
    }

    return true;
}
