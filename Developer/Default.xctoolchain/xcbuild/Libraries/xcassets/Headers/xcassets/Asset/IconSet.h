/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_IconSet_h
#define __xcassets_Asset_IconSet_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/Slot/Scale.h>
#include <plist/Dictionary.h>

#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class IconSet : public Asset {
public:
    class Icon {
    private:
        std::string                _path;
        double                     _width;
        double                     _height;

    private:
        ext::optional<Slot::Scale> _scale;

    private:
        Icon(std::string const &path, double width, double height, ext::optional<Slot::Scale> const &scale);

    public:
        std::string const &path() const
        { return _path; }
        double const &width() const
        { return _width; }
        double const &height() const
        { return _height; }

    public:
        ext::optional<Slot::Scale> const &scale() const
        { return _scale; }

    public:
        static ext::optional<Icon> Parse(std::string const &path);
    };

private:
    std::vector<Icon> _icons;

private:
    friend class Asset;
    using Asset::Asset;

public:
    std::vector<Icon> const &icons() const
    { return _icons; }

public:
    static AssetType Type()
    { return AssetType::IconSet; }
    virtual AssetType type() const
    { return AssetType::IconSet; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("iconset"); }

protected:
    virtual bool load(libutil::Filesystem const *filesystem);
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_IconSet_h

