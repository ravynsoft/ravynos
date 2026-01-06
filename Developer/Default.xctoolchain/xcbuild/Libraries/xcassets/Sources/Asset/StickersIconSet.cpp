/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/StickersIconSet.h>
#include <plist/Keys/Unpack.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcassets::Asset::StickersIconSet;
using Platform = StickersIconSet::Platform;
using Platforms = StickersIconSet::Platforms;

bool StickersIconSet::Image::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("StickersIconSetImage", dict, &seen);

    auto FN = unpack.cast <plist::String> ("filename");
    auto I  = unpack.cast <plist::String> ("idiom");
    auto Z  = unpack.cast <plist::String> ("size");
    auto S  = unpack.cast <plist::String> ("scale");
    auto P  = unpack.cast <plist::String> ("platform");

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

    if (P != nullptr) {
        _platform = Platforms::Parse(P->value());
    }

    return true;
}

bool StickersIconSet::
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

    auto unpack = plist::Keys::Unpack("StickersIconSet", dict, seen);

    auto I = unpack.cast <plist::Array> ("images");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
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

ext::optional<Platform> Platforms::
Parse(std::string const &value)
{
    if (value == "ios") {
        return Platform::iOS;
    } else if (value == "macos") {
        return Platform::macOS;
    } else if (value == "tvos") {
        return Platform::tvOS;
    } else if (value == "watchos") {
        return Platform::watchOS;
    } else {
        fprintf(stderr, "warning: unknown platform %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string Platforms::
String(Platform platform)
{
    switch (platform) {
        case Platform::iOS:
            return "ios";
        case Platform::macOS:
            return "macos";
        case Platform::tvOS:
            return "tvos";
        case Platform::watchOS:
            return "watchos";
    }

    abort();
}
