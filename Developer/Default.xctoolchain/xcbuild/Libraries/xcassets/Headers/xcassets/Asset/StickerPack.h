/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_StickerPack_h
#define __xcassets_Asset_StickerPack_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/StickerGridSize.h>
#include <plist/Dictionary.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class StickerPack : public Asset {
public:
    class Sticker {
    private:
        ext::optional<std::string>     _fileName;

    public:
        ext::optional<std::string> const &fileName() const
        { return _fileName; }

    private:
        friend class StickerPack;
        bool parse(plist::Dictionary const *dict);
    };

private:
    ext::optional<StickerGridSize>      _gridSize;
    ext::optional<std::vector<Sticker>> _stickers;

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<StickerGridSize> const &gridSize() const
    { return _gridSize; }
    ext::optional<std::vector<Sticker>> const &stickers() const
    { return _stickers; }

public:
    static AssetType Type()
    { return AssetType::StickerPack; }
    virtual AssetType type() const
    { return AssetType::StickerPack; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("stickerpack"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_StickerPack_h
