/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_Group_h
#define __xcassets_Asset_Group_h

#include <xcassets/Asset/Asset.h>
#include <plist/Dictionary.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class Group : public Asset {
private:
    ext::optional<std::vector<std::string>> _onDemandResourceTags;
    ext::optional<bool>                     _providesNamespace;

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<std::vector<std::string>> const &onDemandResourceTags() const
    { return _onDemandResourceTags; }
    bool providesNamespace() const
    { return _providesNamespace.value_or(false); }
    ext::optional<bool> providesNamespaceOptional() const
    { return _providesNamespace; }

public:
    static AssetType Type()
    { return AssetType::Group; }
    virtual AssetType type() const
    { return AssetType::Group; }

public:
    /*
     * Groups have no extension.
     */
    static ext::optional<std::string> Extension()
    { return ext::nullopt; }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_Group_h

