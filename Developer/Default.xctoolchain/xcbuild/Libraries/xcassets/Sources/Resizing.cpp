/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Resizing.h>
#include <plist/Keys/Unpack.h>
#include <plist/Real.h>
#include <plist/String.h>

using xcassets::Resizing;

bool Resizing::Center::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("ResizingCenter", dict, &seen);

    auto M = unpack.cast <plist::String> ("mode");
    auto W = unpack.coerce <plist::Real> ("width");
    auto H = unpack.coerce <plist::Real> ("height");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (M != nullptr) {
        if (M->value() == "tile") {
            _mode = Resizing::Center::Mode::Tile;
        } else if (M->value() == "stretch") {
            _mode = Resizing::Center::Mode::Stretch;
        } else {
            fprintf(stderr, "warning: unknown center mode %s\n", M->value().c_str());
        }
    }

    if (W != nullptr) {
        _width = W->value();
    }

    if (H != nullptr) {
        _height = H->value();
    }

    return true;
}

bool Resizing::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("Resizing", dict, &seen);

    auto M  = unpack.cast <plist::String> ("mode");
    auto C  = unpack.cast <plist::Dictionary> ("center");
    auto CI = unpack.cast <plist::Dictionary> ("cap-insets");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (M != nullptr) {
        if (M->value() == "3-part-horizontal") {
            _mode = Resizing::Mode::ThreePartHorizontal;
        } else if (M->value() == "3-part-vertical") {
            _mode = Resizing::Mode::ThreePartVertical;
        } else if (M->value() == "9-part") {
            _mode = Resizing::Mode::NinePart;
        } else {
            fprintf(stderr, "warning: unknown resizing mode %s\n", M->value().c_str());
        }
    }

    if (C != nullptr) {
        Resizing::Center center;
        if (center.parse(C)) {
            _center = center;
        }
    }

    if (CI != nullptr) {
        Insets capInsets;
        if (capInsets.parse(CI)) {
            _capInsets = capInsets;
        }
    }

    return true;
}

