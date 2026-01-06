/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/AppIconSet.h>
#include <plist/Keys/Unpack.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcassets::Asset::AppIconSet;

bool AppIconSet::Image::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("AppIconSetImage", dict, &seen);

    auto FN = unpack.cast <plist::String> ("filename");
    auto I  = unpack.cast <plist::String> ("idiom");
    auto Z  = unpack.cast <plist::String> ("size");
    auto S  = unpack.cast <plist::String> ("scale");
    auto R  = unpack.cast <plist::String> ("role");
    auto B  = unpack.cast <plist::String> ("subtype");
    auto U  = unpack.cast <plist::Boolean> ("unassigned");
    auto MS = unpack.cast <plist::String> ("matching-style");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (FN != nullptr) {
        _fileName = FN->value();
    }

    if (I != nullptr) {
        _idiom = Slot::Idioms::Parse(I->value());
    }

    if (Z != nullptr) {
        _imageSize = Slot::ImageSize::Parse(Z->value());
    }

    if (S != nullptr) {
        _scale = Slot::Scale::Parse(S->value());
    }

    if (R != nullptr) {
        _role = Slot::WatchIconRoles::Parse(R->value());
    }

    if (B != nullptr) {
        _subtype = Slot::WatchSubtypes::ParsePhysicalSize(B->value());
    }

    if (U != nullptr) {
        _unassigned = U->value();
    }

    if (MS != nullptr) {
        _matchingStyle = MatchingStyles::Parse(MS->value());
    }

    return true;
}

bool AppIconSet::
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

    auto unpack = plist::Keys::Unpack("AppIconSet", dict, seen);

    auto P = unpack.cast <plist::Dictionary> ("properties");
    auto I = unpack.cast <plist::Array> ("images");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (P != nullptr) {
        std::unordered_set<std::string> seen;
        auto unpack = plist::Keys::Unpack("AppIconSet", P, &seen);

        auto PR = unpack.cast <plist::Boolean> ("pre-rendered");

        if (!unpack.complete(check)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
        }

        if (PR != nullptr) {
            _preRendered = PR->value();
        }
    }

    if (I != nullptr) {
        _images = std::vector<Image>();

        for (size_t n = 0; n < I->count(); ++n) {
            if (auto dict = I->value<plist::Dictionary>(n)) {
                Image image;
                if (image.parse(dict)) {
                    _images->push_back(image);
                }
            }
        }
    }

    return true;
}

