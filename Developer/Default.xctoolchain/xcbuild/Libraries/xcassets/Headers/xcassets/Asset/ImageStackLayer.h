/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_ImageStackLayer_h
#define __xcassets_Asset_ImageStackLayer_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/Asset/ImageSet.h>
#include <xcassets/ContentReference.h>
#include <plist/Dictionary.h>

#include <string>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class ImageStackLayer : public Asset {
private:
    ext::optional<ContentReference> _contentReference;
    // TODO: frame-size
    // TODO: frame-center

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<ContentReference> const &contentReference() const
    { return _contentReference; }
    // TODO: frame-size
    // TODO: frame-center

public:
    ImageSet const *contentEmbedded() const
    { return this->child<ImageSet>(); }

public:
    static AssetType Type()
    { return AssetType::ImageStackLayer; }
    virtual AssetType type() const
    { return AssetType::ImageStackLayer; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("imagestacklayer"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_ImageStackLayer_h

