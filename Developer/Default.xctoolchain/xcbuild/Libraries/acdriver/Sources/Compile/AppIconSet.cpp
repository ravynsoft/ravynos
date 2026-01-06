/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Compile/AppIconSet.h>
#include <acdriver/Compile/Convert.h>
#include <acdriver/Compile/Output.h>
#include <acdriver/Result.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

#include <sstream>

using acdriver::Compile::AppIconSet;
using acdriver::Compile::Convert;
using acdriver::Compile::Output;
using acdriver::Result;

static plist::Dictionary *
IconsDictionary(plist::Dictionary *info, std::string const &key)
{
    if (auto existing = info->value<plist::Dictionary>(key)) {
        return existing;
    } else {
        auto icons = plist::Dictionary::New();
        info->set(key, std::move(icons));
        return info->value<plist::Dictionary>(key);
    }
}

static std::string
SizeSuffix(xcassets::Slot::ImageSize const &size)
{
    std::ostringstream out;
    out << size.width();
    out << "x";
    out << size.height();
    return out.str();
}

bool AppIconSet::
Compile(
    xcassets::Asset::AppIconSet const *appIconSet,
    Output *compileOutput,
    Result *result)
{
    struct IdiomHash
    {
        std::size_t operator()(xcassets::Slot::Idiom idiom) const
        {
            return static_cast<std::size_t>(idiom);
        }
    };
    auto files = std::unordered_map<xcassets::Slot::Idiom, std::vector<std::string>, IdiomHash>();

    /*
     * Copy the app icon images into the output.
     */
    if (appIconSet->images()) {
        for (xcassets::Asset::AppIconSet::Image const &image : *appIconSet->images()) {
            /*
             * Verify the image has the required information.
             */
            if (image.unassigned() || !image.fileName() || !image.imageSize() || !image.scale()) {
                result->document(
                    Result::Severity::Warning,
                    appIconSet->path(),
                    { Output::AssetReference(appIconSet) },
                    "Ambiguous Content",
                    "an icon in \"" + appIconSet->name().name() + "\" is unassigned");
                continue;
            }

            /*
             * Determine information about the icon image.
             */
            xcassets::Slot::ImageSize const &size = *image.imageSize();
            // TODO: verify the dimensions of the image are correct
            std::string sizeSuffix = SizeSuffix(size);

            xcassets::Slot::Scale const &scale = *image.scale();
            std::string scaleSuffix = Convert::ScaleSuffix(scale);

            xcassets::Slot::Idiom idiom = image.idiom().value_or(xcassets::Slot::Idiom::Universal);
            // TODO: skip images with idioms inappropriate for target platform
            std::string idiomSuffix = Convert::IdiomSuffix(idiom);

            /*
             * Copy the icon image into the output.
             */
            std::string source = appIconSet->path() + "/" + *image.fileName();
            std::string destination = compileOutput->root() + "/" + appIconSet->name().name() + sizeSuffix + scaleSuffix + idiomSuffix + ".png";
            compileOutput->copies().push_back({ source, destination });
            compileOutput->outputs().push_back(destination);

            /*
             * Add the file to the output info.
             */
            std::string name = appIconSet->name().name() + sizeSuffix;
            files[idiom].push_back(name);
        }
    }

    /*
     * Build up the info about the icons.
     */
    for (auto const &pair : files) {
        /*
         * Record details about the icon itself.
         */
        auto primary = plist::Dictionary::New();

        /* List the files for the icon. */
        auto files = plist::Array::New();
        std::unordered_set<std::string> seen;
        for (std::string const &file : pair.second) {
            if (seen.find(file) == seen.end()) {
                seen.insert(file);
                files->append(plist::String::New(file));
            }
        }
        primary->set("CFBundleIconFiles", std::move(files));

        /* Record if the icon is pre-rendered. */
        if (appIconSet->preRendered()) {
            primary->set("UIPrerenderedIcon", plist::Boolean::New(true));
        }

        /*
         * Store the icon in the additional info.
         */
        std::string idiomSuffix = Convert::IdiomSuffix(pair.first);
        plist::Dictionary *icons = IconsDictionary(compileOutput->additionalInfo(), "CFBundleIcons" + idiomSuffix);
        icons->set("CFBundlePrimaryIcon", std::move(primary));
    }

    return true;
}
