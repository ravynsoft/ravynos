/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Compile/ImageSet.h>
#include <acdriver/Compile/Convert.h>
#include <acdriver/Compile/Output.h>
#include <acdriver/Result.h>
#include <graphics/PixelFormat.h>
#include <graphics/Format/PNG.h>
#include <xcassets/Asset/ImageSet.h>
#include <xcassets/Slot/Idiom.h>
#include <car/Facet.h>
#include <car/Rendition.h>
#include <car/Writer.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

#include <algorithm>
#include <map>
#include <string>

using acdriver::Compile::ImageSet;
using acdriver::Compile::Convert;
using acdriver::Compile::Output;
using acdriver::Result;
using libutil::Filesystem;
using libutil::FSUtil;

bool ImageSet::
Compile(
    xcassets::Asset::ImageSet const *imageSet,
    Filesystem *filesystem,
    Output *compileOutput,
    Result *result)
{
    bool success = true;

    if (imageSet->images()) {
        for (xcassets::Asset::ImageSet::Image const &image : *imageSet->images()) {
            if (!CompileAsset(imageSet, image, filesystem, compileOutput, result)) {
                success = false;
            }
        }
    }

    return success;
}

static uint16_t
GenerateIdentifier(void) {
    static uint16_t last = 0;
    last = last + 1;
    return last;
}

bool ImageSet::
CompileAsset(
    xcassets::Asset::ImageSet const *imageSet,
    xcassets::Asset::ImageSet::Image const &image,
    Filesystem *filesystem,
    Output *compileOutput,
    Result *result)
{
    static std::map<std::string, uint16_t> idMap = {};

    /* Skip any entry that is not attached to a file, or is explicitly unassigned. */
    if (!image.fileName() || image.unassigned()) {
        return true;
    }

    /* An image without an idiom is considered unassigned. */
    if (!image.idiom()) {
        return false;
    }

    std::string filename = FSUtil::ResolveRelativePath(*image.fileName(), imageSet->path());

    std::string name = imageSet->name().string();

    /* The default (0) is any scale. */
    double scale = 0;
    if (image.scale()) {
        scale = image.scale()->value();
    }

    // TODO: filter by target-device / device-model / os-version
    uint16_t idiom = Convert::IdiomAttribute(*image.idiom());

    std::vector<uint8_t> pixels;
    size_t width = 0;
    size_t height = 0;
    car::Rendition::Data::Format format = car::Rendition::Data::Format::Data;

    if (FSUtil::IsFileExtension(filename, "png", true)) {
        std::vector<uint8_t> contents;
        if (!filesystem->read(&contents, filename)) {
            result->normal(
                Result::Severity::Error,
                "unable to read PNG file",
                filename);
            return false;
        }

        auto png = graphics::Format::PNG::Read(contents);
        if (!png.first) {
            result->normal(Result::Severity::Error, png.second, filename);
            return false;
        }

        graphics::Image const &image = *png.first;
        width = image.width();
        height = image.height();

        /* Convert the image to the archive format. */
        switch (image.format().color()) {
            case graphics::PixelFormat::Color::RGB:
                format = car::Rendition::Data::Format::PremultipliedBGRA8;
                pixels = graphics::PixelFormat::Convert(
                    image.data(),
                    image.format(),
                    graphics::PixelFormat(
                        graphics::PixelFormat::Color::RGB,
                        graphics::PixelFormat::Order::Reversed,
                        graphics::PixelFormat::Alpha::PremultipliedFirst));
                break;
            case graphics::PixelFormat::Color::Grayscale:
                format = car::Rendition::Data::Format::PremultipliedGA8;
                pixels = graphics::PixelFormat::Convert(
                    image.data(),
                    image.format(),
                    graphics::PixelFormat(
                        graphics::PixelFormat::Color::Grayscale,
                        graphics::PixelFormat::Order::Reversed,
                        graphics::PixelFormat::Alpha::PremultipliedFirst));
                break;
        }
    } else if (FSUtil::IsFileExtension(filename, "jpg", true) || FSUtil::IsFileExtension(filename, "jpeg", true)) {
        if (!filesystem->read(&pixels, filename)) {
            result->normal(
                Result::Severity::Error,
                "unable to read JPEG file",
                filename);
            return false;
        }

        format = car::Rendition::Data::Format::JPEG;
    } else {
        ext::optional<NonStandard::ImageType> type = NonStandard::ImageTypeFromFileExtension(FSUtil::GetFileExtension(filename));
        if (!type) {
            result->normal(
                Result::Severity::Error,
                "unknown file type",
                filename);
            return false;
        }
        if (!compileOutput->allowedNonStandardImageTypes().count(*type)) {
            result->normal(
                Result::Severity::Error,
                "forbidden file type",
                filename);
            return false;
        }
        if (!filesystem->read(&pixels, filename)) {
            result->normal(
                Result::Severity::Error,
                "unable to read image file",
                filename);
            return false;
        }
        format = NonStandard::ImageTypeToDataFormat(*type);
    }

    bool createFacet = false;
    uint16_t facetIdentifier = 0;
    auto it = idMap.find(name);
    if (it == idMap.end()) {
        facetIdentifier = GenerateIdentifier();
        idMap[name] = facetIdentifier;
        createFacet = true;
    } else {
        facetIdentifier = it->second;
    }
    if (createFacet) {
        car::AttributeList attributes = car::AttributeList({
            { car_attribute_identifier_identifier, facetIdentifier },
        });

        car::Facet facet = car::Facet::Create(name, attributes);
        compileOutput->car()->addFacet(facet);
    }

    /*
     * Create rendition for the image.
     */
    car::AttributeList attributes = car::AttributeList({
        { car_attribute_identifier_idiom, idiom },
        { car_attribute_identifier_scale, static_cast<int>(scale) },
        { car_attribute_identifier_identifier, facetIdentifier },
    });

    auto data = ext::optional<car::Rendition::Data>(car::Rendition::Data(std::move(pixels), format));

    car::Rendition rendition = car::Rendition::Create(attributes, std::move(data));
    rendition.width() = width;
    rendition.height() = height;
    rendition.scale() = scale;
    rendition.fileName() = *image.fileName();

    if (image.resizing()) {
        xcassets::Resizing const &resizing = *image.resizing();

        xcassets::Resizing::Center::Mode centerMode = xcassets::Resizing::Center::Mode::Tile;
        if (resizing.center()) {
            xcassets::Resizing::Center const &center = *resizing.center();
            if (center.mode()) {
                centerMode = *center.mode();
            }

            /* TODO: center size is currently ingnored */
        }

        if (resizing.mode()) {
            xcassets::Resizing::Mode resizingMode = *resizing.mode();
            rendition.layout() = Convert::LayoutForResizingAndCenterMode(resizingMode, centerMode);
            rendition.slices() = Convert::SlicesForResizingModeAndCapInsets(width, height, resizingMode, resizing.capInsets());
        }
    }

    compileOutput->car()->addRendition(std::move(rendition));
    return true;
}
