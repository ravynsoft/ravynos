/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_ImageSet_h
#define __xcassets_Asset_ImageSet_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/Slot/ColorSpace.h>
#include <xcassets/Slot/DeviceSubtype.h>
#include <xcassets/Slot/GraphicsFeatureSet.h>
#include <xcassets/Slot/Idiom.h>
#include <xcassets/Slot/MemoryRequirement.h>
#include <xcassets/Slot/Scale.h>
#include <xcassets/Slot/SizeClass.h>
#include <xcassets/Slot/WatchSubtype.h>
#include <xcassets/Insets.h>
#include <xcassets/Compression.h>
#include <xcassets/Resizing.h>
#include <xcassets/TemplateRenderingIntent.h>
#include <plist/Dictionary.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class ImageSet : public Asset {
public:
    class Image {
    private:
        ext::optional<std::string>              _fileName;
        ext::optional<bool>                     _unassigned;

    private:
        ext::optional<Slot::ColorSpace>         _colorSpace;
        ext::optional<Compression>              _compression;
        ext::optional<Slot::GraphicsFeatureSet> _graphicsFeatureSet;
        ext::optional<Slot::Idiom>              _idiom;
        ext::optional<Slot::MemoryRequirement>  _memory;
        ext::optional<Slot::Scale>              _scale;
        ext::optional<Slot::DeviceSubtype>      _subtype;
        ext::optional<Slot::WatchSubtype>       _screenWidth;
        ext::optional<Slot::SizeClass>          _widthClass;
        ext::optional<Slot::SizeClass>          _heightClass;

    private:
        ext::optional<Insets>                   _alignmentInsets;
        ext::optional<Resizing>                 _resizing;

    public:
        ext::optional<std::string> const &fileName() const
        { return _fileName; }
        bool unassigned() const
        { return _unassigned.value_or(false); }
        ext::optional<bool> const &unassignedOptional() const
        { return _unassigned; }

    public:
        ext::optional<Slot::ColorSpace> const &colorSpace() const
        { return _colorSpace; }
        ext::optional<Compression> const &compression() const
        { return _compression; }
        ext::optional<Slot::GraphicsFeatureSet> const &graphicsFeatureSet() const
        { return _graphicsFeatureSet; }
        ext::optional<Slot::Idiom> const &idiom() const
        { return _idiom; }
        ext::optional<Slot::MemoryRequirement> const &memory() const
        { return _memory; }
        ext::optional<Slot::Scale> const &scale() const
        { return _scale; }
        ext::optional<Slot::DeviceSubtype> const &subtype() const
        { return _subtype; }
        ext::optional<Slot::WatchSubtype> const &screenWidth() const
        { return _screenWidth; }
        ext::optional<Slot::SizeClass> const &widthClass() const
        { return _widthClass; }
        ext::optional<Slot::SizeClass> const &heightClass() const
        { return _heightClass; }

    public:
        ext::optional<Insets> const &alignmentInsets() const
        { return _alignmentInsets; }
        ext::optional<Resizing> const &resizing() const
        { return _resizing; }

    private:
        friend class ImageSet;
        bool parse(plist::Dictionary const *dict);
    };

private:
    ext::optional<std::vector<std::string>> _onDemandResourceTags;
    ext::optional<std::vector<Image>>       _images;
    ext::optional<TemplateRenderingIntent>  _templateRenderingIntent;

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<std::vector<std::string>> const &onDemandResourceTags() const
    { return _onDemandResourceTags; }
    ext::optional<std::vector<Image>> const &images() const
    { return _images; }
    ext::optional<TemplateRenderingIntent> const &templateRenderingIntent() const
    { return _templateRenderingIntent; }

public:
    static AssetType Type()
    { return AssetType::ImageSet; }
    virtual AssetType type() const
    { return AssetType::ImageSet; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("imageset"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_ImageSet_h
