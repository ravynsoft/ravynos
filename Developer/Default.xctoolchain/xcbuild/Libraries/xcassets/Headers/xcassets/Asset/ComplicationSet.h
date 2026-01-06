/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_ComplicationSet_h
#define __xcassets_Asset_ComplicationSet_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/Slot/Idiom.h>
#include <xcassets/WatchComplicationRole.h>
#include <plist/Dictionary.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class ComplicationSet : public Asset {
public:
    class ComplicationAsset {
    private:
        ext::optional<Slot::Idiom>           _idiom;
        ext::optional<std::string>           _fileName;
        ext::optional<WatchComplicationRole> _role;

    public:
        ext::optional<Slot::Idiom> const &idiom() const
        { return _idiom; }
        ext::optional<std::string> const &fileName() const
        { return _fileName; }
        ext::optional<WatchComplicationRole> const &role() const
        { return _role; }

    private:
        friend class ComplicationSet;
        bool parse(plist::Dictionary const *dict);
    };

private:
    ext::optional<std::vector<ComplicationAsset>> _assets;

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<std::vector<ComplicationAsset>> const &assets() const
    { return _assets; }

public:
    static AssetType Type()
    { return AssetType::ComplicationSet; }
    virtual AssetType type() const
    { return AssetType::ComplicationSet; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("complicationset"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_ComplicationSet_h
