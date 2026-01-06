/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/ImageStack.h>
#include <plist/Array.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using xcassets::Asset::ImageStack;

bool ImageStack::Layer::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("ImageStackLayer", dict, &seen);

    auto F = unpack.cast <plist::String> ("filename");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (F != nullptr) {
        _fileName = F->value();
    }

    return true;
}

bool ImageStack::
parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Asset::parse(dict, seen, false)) {
        return false;
    }

    /* Contents is required. */
    if (dict == nullptr) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("ImageStack", dict, seen);

    auto P  = unpack.cast <plist::Dictionary> ("properties");
    auto Ls = unpack.cast <plist::Array> ("layers");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (P != nullptr) {
        std::unordered_set<std::string> seen;
        auto unpack = plist::Keys::Unpack("Properties", P, &seen);

        auto ODRT = unpack.cast <plist::Array> ("on-demand-resource-tags");
        // TODO: canvasSize

        if (!unpack.complete(true)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
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
    }

    if (Ls != nullptr) {
        _layers = std::vector<Layer>();

        for (size_t n = 0; n < Ls->count(); ++n) {
            if (auto dict = Ls->value<plist::Dictionary>(n)) {
                Layer layer;
                if (layer.parse(dict)) {
                    _layers->push_back(layer);
                }
            }
        }
    }

    return true;
}

