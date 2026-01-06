/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_Sticker_h
#define __xcassets_Asset_Sticker_h

#include <xcassets/Asset/Asset.h>
#include <plist/Dictionary.h>

#include <string>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class Sticker : public Asset {
private:
    ext::optional<std::string> _accessibilityLabel;
    ext::optional<std::string> _fileName;

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<std::string> const &accessibilityLabel() const
    { return _accessibilityLabel; }
    ext::optional<std::string> const &fileName() const
    { return _fileName; }

public:
    static AssetType Type()
    { return AssetType::Sticker; }
    virtual AssetType type() const
    { return AssetType::Sticker; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("sticker"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_Sticker_h

