/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_LaunchImage_h
#define __xcassets_Asset_LaunchImage_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/Slot/DeviceSubtype.h>
#include <xcassets/Slot/Idiom.h>
#include <xcassets/Slot/LaunchImageExtent.h>
#include <xcassets/Slot/Orientation.h>
#include <xcassets/Slot/Scale.h>
#include <xcassets/Slot/SystemVersion.h>
#include <plist/Dictionary.h>

#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class LaunchImage : public Asset {
public:
    class Image {
    private:
        ext::optional<std::string>             _fileName;

    private:
        ext::optional<Slot::Idiom>             _idiom;
        ext::optional<Slot::Orientation>       _orientation;
        ext::optional<Slot::Scale>             _scale;
        ext::optional<Slot::DeviceSubtype>     _subtype;
        ext::optional<Slot::SystemVersion>     _minimumSystemVersion;
        ext::optional<Slot::LaunchImageExtent> _extent;

    public:
        ext::optional<std::string> const &fileName() const
        { return _fileName; }

    public:
        ext::optional<Slot::Idiom> const &idiom() const
        { return _idiom; }
        ext::optional<Slot::Orientation> const &orientation() const
        { return _orientation; }
        ext::optional<Slot::Scale> const &scale() const
        { return _scale; }
        ext::optional<Slot::DeviceSubtype> const &subtype() const
        { return _subtype; }
        ext::optional<Slot::SystemVersion> const &minimumSystemVersion() const
        { return _minimumSystemVersion; }
        ext::optional<Slot::LaunchImageExtent> const &extent() const
        { return _extent; }

    private:
        friend class LaunchImage;
        bool parse(plist::Dictionary const *dict);
    };

private:
    friend class Asset;
    using Asset::Asset;

private:
    ext::optional<std::vector<Image>> _images;

public:
    ext::optional<std::vector<Image>> const &images() const
    { return _images; }

public:
    static AssetType Type()
    { return AssetType::LaunchImage; }
    virtual AssetType type() const
    { return AssetType::LaunchImage; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("launchimage"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_LaunchImage_h
