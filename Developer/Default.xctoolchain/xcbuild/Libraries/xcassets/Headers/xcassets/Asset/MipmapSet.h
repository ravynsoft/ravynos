/** Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_MipmapSet_h
#define __xcassets_Asset_MipmapSet_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/MipmapLevel.h>
#include <xcassets/MipmapLevelMode.h>
#include <plist/Dictionary.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class MipmapSet : public Asset {
public:
    class Level {
    private:
        ext::optional<std::string> _fileName;
        ext::optional<MipmapLevel> _mipmapLevel;

    public:
        ext::optional<std::string> const &fileName() const
        { return _fileName; }
        ext::optional<MipmapLevel> const &mipmapLevel() const
        { return _mipmapLevel; }

    private:
        friend class MipmapSet;
        bool parse(plist::Dictionary const *dict);
    };

private:
    ext::optional<std::vector<Level>> _levels;
    ext::optional<MipmapLevelMode>    _levelMode;

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<std::vector<Level>> const &levels() const
    { return _levels; }
    ext::optional<MipmapLevelMode> const &levelMode() const
    { return _levelMode; }

public:
    static AssetType Type()
    { return AssetType::MipmapSet; }
    virtual AssetType type() const
    { return AssetType::MipmapSet; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("mipmapset"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_MipmapSet_h
