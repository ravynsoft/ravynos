/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/LaunchImage.h>
#include <plist/Keys/Unpack.h>
#include <plist/Array.h>
#include <plist/String.h>

using xcassets::Asset::LaunchImage;

bool LaunchImage::Image::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("LaunchImageImage", dict, &seen);

    auto F   = unpack.cast <plist::String> ("filename");
    auto I   = unpack.cast <plist::String> ("idiom");
    auto O   = unpack.cast <plist::String> ("orientation");
    auto S   = unpack.cast <plist::String> ("scale");
    auto ST  = unpack.cast <plist::String> ("subtype");
    auto MSV = unpack.cast <plist::String> ("minimum-system-version");
    auto E   = unpack.cast <plist::String> ("extent");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (F != nullptr) {
        _fileName = F->value();
    }

    if (I != nullptr) {
        _idiom = Slot::Idioms::Parse(I->value());
    }

    if (O != nullptr) {
        _orientation = Slot::Orientations::Parse(O->value());
    }

    if (S != nullptr) {
        _scale = Slot::Scale::Parse(S->value());
    }

    if (ST != nullptr) {
        _subtype = Slot::DeviceSubtypes::Parse(ST->value());
    }

    if (MSV != nullptr) {
        _minimumSystemVersion = Slot::SystemVersion::Parse(MSV->value());
    }

    if (E != nullptr) {
        _extent = Slot::LaunchImageExtents::Parse(E->value());
    }

    return true;
}

bool LaunchImage::
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

    auto unpack = plist::Keys::Unpack("LaunchImage", dict, seen);

    auto Is = unpack.cast <plist::Array> ("images");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (Is != nullptr) {
        _images = std::vector<Image>();

        for (size_t n = 0; n < Is->count(); ++n) {
            if (auto dict = Is->value<plist::Dictionary>(n)) {
                Image image;
                if (image.parse(dict)) {
                    _images->push_back(image);
                }
            }
        }
    }

    return true;
}

