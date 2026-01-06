/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Compile/Output.h>
#include <acdriver/Version.h>
#include <acdriver/Options.h>
#include <acdriver/Result.h>
#include <xcassets/Asset/Asset.h>
#include <dependency/DependencyInfo.h>
#include <dependency/BinaryDependencyInfo.h>
#include <plist/Format/Format.h>
#include <plist/Format/XML.h>
#include <libutil/Filesystem.h>

using acdriver::Compile::Output;
using acdriver::Version;
using acdriver::Options;
using acdriver::Result;
using libutil::Filesystem;

Output::
Output(
    std::string const &root,
    Format format,
    ext::optional<std::string> const &appIcon,
    ext::optional<std::string> const &launchImage,
    NonStandard::ImageTypeSet const &allowedNonStandardImageTypes) :
    _root                        (root),
    _format                      (format),
    _appIcon                     (appIcon),
    _launchImage                 (launchImage),
    _allowedNonStandardImageTypes (allowedNonStandardImageTypes),
    _additionalInfo              (plist::Dictionary::New())
{
}

std::string Output::
AssetReference(xcassets::Asset::Asset const *asset)
{
    // TODO: include [] for each key
    return asset->path();
}
