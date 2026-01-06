/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_GCLeaderboard_h
#define __xcassets_Asset_GCLeaderboard_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/Asset/ImageStack.h>
#include <xcassets/ContentReference.h>
#include <plist/Dictionary.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class GCLeaderboard : public Asset {
private:
    ext::optional<ContentReference> _contentReference;

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<ContentReference> const &contentReference() const
    { return _contentReference; }

public:
    ImageStack const *contentEmbedded() const
    { return this->child<ImageStack>(); }

public:
    static AssetType Type()
    { return AssetType::GCLeaderboard; }
    virtual AssetType type() const
    { return AssetType::GCLeaderboard; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("gcleaderboard"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_GCLeaderboard_h
