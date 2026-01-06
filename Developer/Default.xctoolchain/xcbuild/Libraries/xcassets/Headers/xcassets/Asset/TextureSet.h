/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_TextureSet_h
#define __xcassets_Asset_TextureSet_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/Slot/ColorSpace.h>
#include <xcassets/Slot/GraphicsFeatureSet.h>
#include <xcassets/Slot/Idiom.h>
#include <xcassets/Slot/MemoryRequirement.h>
#include <xcassets/Slot/Scale.h>
#include <xcassets/TextureInterpretation.h>
#include <xcassets/TextureOrigin.h>
#include <xcassets/TexturePixelFormat.h>
#include <plist/Dictionary.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class TextureSet : public Asset {
public:
    class Texture {
    private:
        ext::optional<Slot::ColorSpace>         _colorSpace;
        ext::optional<std::string>              _fileName;
        ext::optional<Slot::GraphicsFeatureSet> _graphicsFeatureSet;
        ext::optional<Slot::Idiom>              _idiom;
        ext::optional<Slot::MemoryRequirement>  _memory;
        ext::optional<TexturePixelFormat>       _pixelFormat;
        ext::optional<Slot::Scale>              _scale;

    public:
        ext::optional<Slot::ColorSpace> const &colorSpace() const
        { return _colorSpace; }
        ext::optional<std::string> const &fileName() const
        { return _fileName; }
        ext::optional<Slot::GraphicsFeatureSet> const &graphicsFeatureSet() const
        { return _graphicsFeatureSet; }
        ext::optional<Slot::Idiom> const &idiom() const
        { return _idiom; }
        ext::optional<Slot::MemoryRequirement> const &memory() const
        { return _memory; }
        ext::optional<TexturePixelFormat> const &pixelFormat() const
        { return _pixelFormat; }
        ext::optional<Slot::Scale> const &scale() const
        { return _scale; }

    private:
        friend class TextureSet;
        bool parse(plist::Dictionary const *dict);
    };

private:
    ext::optional<TextureInterpretation>    _interpretation;
    ext::optional<TextureOrigin>            _origin;
    ext::optional<std::vector<std::string>> _onDemandResourceTags;

private:
    ext::optional<std::vector<Texture>>     _textures;

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<TextureInterpretation> const &interpretation() const
    { return _interpretation; }
    ext::optional<TextureOrigin> const &origin() const
    { return _origin; }
    ext::optional<std::vector<std::string>> const &onDemandResourceTags() const
    { return _onDemandResourceTags; }

public:
    ext::optional<std::vector<Texture>> const &textures() const
    { return _textures; }

public:
    static AssetType Type()
    { return AssetType::TextureSet; }
    virtual AssetType type() const
    { return AssetType::TextureSet; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("textureset"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_TextureSet_h
