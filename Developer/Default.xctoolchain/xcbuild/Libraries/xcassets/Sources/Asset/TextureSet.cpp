/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/TextureSet.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using xcassets::Asset::TextureSet;

bool TextureSet::Texture::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("TextureSetTexture", dict, &seen);

    auto CS  = unpack.cast <plist::String> ("color-space");
    auto F   = unpack.cast <plist::String> ("filename");
    auto GFS = unpack.cast <plist::String> ("graphics-feature-set");
    auto I   = unpack.cast <plist::String> ("idiom");
    auto M   = unpack.cast <plist::String> ("memory");
    auto PF  = unpack.cast <plist::String> ("pixel-format");
    auto S   = unpack.cast <plist::String> ("scale");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (CS != nullptr) {
        _colorSpace = Slot::ColorSpaces::Parse(CS->value());
    }

    if (F != nullptr) {
        _fileName = F->value();
    }

    if (GFS != nullptr) {
        _graphicsFeatureSet = Slot::GraphicsFeatureSets::Parse(GFS->value());
    }

    if (I != nullptr) {
        _idiom = Slot::Idioms::Parse(I->value());
    }

    if (M != nullptr) {
        _memory = Slot::MemoryRequirements::Parse(M->value());
    }

    if (PF != nullptr) {
        _pixelFormat = TexturePixelFormats::Parse(PF->value());
    }

    if (S != nullptr) {
        _scale = Slot::Scale::Parse(S->value());
    }

    return true;
}

bool TextureSet::
parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Asset::parse(dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("TextureSet", dict, seen);

    auto P  = unpack.cast <plist::Dictionary> ("properties");
    auto Ts = unpack.cast <plist::Array> ("textures");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (P != nullptr) {
        std::unordered_set<std::string> seen;
        auto unpack = plist::Keys::Unpack("Properties", P, &seen);

        auto I    = unpack.cast <plist::String> ("interpretation");
        auto O    = unpack.cast <plist::String> ("origin");
        auto ODRT = unpack.cast <plist::Array> ("on-demand-resource-tags");

        if (!unpack.complete(true)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
        }

        if (I != nullptr) {
            _interpretation = TextureInterpretations::Parse(I->value());
        }

        if (O != nullptr) {
            _origin = TextureOrigins::Parse(O->value());
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

    if (Ts != nullptr) {
        _textures = std::vector<Texture>();

        for (size_t n = 0; n < Ts->count(); ++n) {
            if (auto dict = Ts->value<plist::Dictionary>(n)) {
                Texture texture;
                if (texture.parse(dict)) {
                    _textures->push_back(texture);
                }
            }
        }
    }

    return true;
}
