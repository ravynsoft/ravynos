/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/MipmapSet.h>
#include <plist/Keys/Unpack.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/String.h>

using xcassets::Asset::MipmapSet;

bool MipmapSet::Level::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("MipmapSetLevel", dict, &seen);

    auto F  = unpack.cast <plist::String> ("filename");
    auto ML = unpack.cast <plist::String> ("mipmap-level");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (F != nullptr) {
        _fileName = F->value();
    }

    if (ML != nullptr) {
        _mipmapLevel = MipmapLevels::Parse(ML->value());
    }

    return true;
}

bool MipmapSet::
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

    auto unpack = plist::Keys::Unpack("MipmapSet", dict, seen);

    auto P  = unpack.cast <plist::Dictionary> ("properties");
    auto Ls = unpack.cast <plist::Array> ("levels");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (P != nullptr) {
        std::unordered_set<std::string> seen;
        auto unpack = plist::Keys::Unpack("Properties", P, &seen);

        auto LM = unpack.cast <plist::String> ("level-mode");

        if (!unpack.complete(true)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
        }

        if (LM != nullptr) {
            _levelMode = MipmapLevelModes::Parse(LM->value());
        }
    }

    if (Ls != nullptr) {
        _levels = std::vector<Level>();

        for (size_t n = 0; n < Ls->count(); ++n) {
            if (auto dict = Ls->value<plist::Dictionary>(n)) {
                Level level;
                if (level.parse(dict)) {
                    _levels->push_back(level);
                }
            }
        }
    }

    return true;
}
