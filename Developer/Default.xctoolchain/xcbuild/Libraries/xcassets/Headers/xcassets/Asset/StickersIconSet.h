/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_StickersIconSet_h
#define __xcassets_Asset_StickersIconSet_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/Slot/Idiom.h>
#include <xcassets/Slot/ImageSize.h>
#include <xcassets/Slot/Scale.h>
#include <plist/Dictionary.h>

#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class StickersIconSet : public Asset {
public:
    /*
     * A specific targeted platform.
     */
    enum class Platform {
        iOS,
        macOS,
        tvOS,
        watchOS,
    };

public:
    class Platforms {
    private:
        Platforms();
        ~Platforms();

    public:
        /*
         * Parse a matching platform from a string, if valid.
         */
        static ext::optional<Platform> Parse(std::string const &value);

        /*
         * Convert a platform to a string.
         */
        static std::string String(Platform platform);
    };

public:
    class Image {
    private:
        ext::optional<std::string>     _fileName;

    private:
        ext::optional<Slot::Idiom>     _idiom;
        ext::optional<Slot::ImageSize> _imageSize;
        ext::optional<Slot::Scale>     _scale;
        ext::optional<Platform>        _platform;

    public:
        ext::optional<std::string> const &fileName() const
        { return _fileName; }

    public:
        ext::optional<Slot::Idiom> const &idiom() const
        { return _idiom; }
        ext::optional<Slot::ImageSize> const &imageSize() const
        { return _imageSize; }
        ext::optional<Slot::Scale> const &scale() const
        { return _scale; }
        ext::optional<Platform> const &platform() const
        { return _platform; }

    private:
        friend class StickersIconSet;
        bool parse(plist::Dictionary const *dict);
    };

private:
    ext::optional<std::vector<Image>> _images;

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<std::vector<Image>> const &images() const
    { return _images; }

public:
    static AssetType Type()
    { return AssetType::StickersIconSet; }
    virtual AssetType type() const
    { return AssetType::StickersIconSet; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("stickersiconset"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_StickersIconSet_h
