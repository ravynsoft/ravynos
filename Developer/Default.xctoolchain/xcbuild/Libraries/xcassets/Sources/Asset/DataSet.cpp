/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/DataSet.h>
#include <plist/Keys/Unpack.h>
#include <plist/Array.h>
#include <plist/String.h>

using xcassets::Asset::DataSet;

bool DataSet::Data::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("ImageSetImage", dict, &seen);

    auto CS  = unpack.cast <plist::String> ("color-space");
    auto F   = unpack.cast <plist::String> ("filename");
    auto I   = unpack.cast <plist::String> ("idiom");
    auto GFS = unpack.cast <plist::String> ("graphics-feature-set");
    auto M   = unpack.cast <plist::String> ("memory");
    auto UTI = unpack.cast <plist::String> ("universal-type-identifier");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (CS != nullptr) {
        _colorSpace = Slot::ColorSpaces::Parse(CS->value());
    }

    if (F != nullptr) {
        _fileName = F->value();
    }

    if (I != nullptr) {
        _idiom = Slot::Idioms::Parse(I->value());
    }

    if (GFS != nullptr) {
        _graphicsFeatureSet = Slot::GraphicsFeatureSets::Parse(GFS->value());
    }

    if (M != nullptr) {
        _memory = Slot::MemoryRequirements::Parse(M->value());
    }

    if (UTI != nullptr) {
        _UTI = UTI->value();
    }

    return true;
}

bool DataSet::
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

    auto unpack = plist::Keys::Unpack("DataSet", dict, seen);

    auto P  = unpack.cast <plist::Dictionary> ("properties");
    auto Ds = unpack.cast <plist::Array> ("data");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (P != nullptr) {
        std::unordered_set<std::string> seen;
        auto unpack = plist::Keys::Unpack("Properties", P, &seen);

        auto ODRT = unpack.cast <plist::Array> ("on-demand-resource-tags");

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

    if (Ds != nullptr) {
        _data = std::vector<Data>();

        for (size_t n = 0; n < Ds->count(); ++n) {
            if (auto dict = Ds->value<plist::Dictionary>(n)) {
                Data data;
                if (data.parse(dict)) {
                    _data->push_back(data);
                }
            }
        }
    }

    return true;
}

