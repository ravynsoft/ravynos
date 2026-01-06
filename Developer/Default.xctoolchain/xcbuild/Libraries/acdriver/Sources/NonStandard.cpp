/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/NonStandard.h>
#include <libutil/Options.h>

using acdriver::NonStandard;

static std::pair<bool, std::string>
InsertNextImageType(NonStandard::ImageTypeSet &imageTypeSet, std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it)
{
    ext::optional<std::string> value;
    auto ret = libutil::Options::Next<std::string>(&value, args, it);
    if (!ret.first) {
        return ret;
    }
    ext::optional<NonStandard::ImageType> type = NonStandard::ImageTypeFromFileExtension(*value);
    if (!type) {
        return std::make_pair(false, "unknown image type " + *value);
    }
    imageTypeSet.insert(*type);
    return std::make_pair(true, std::string());
}

ext::optional<std::pair<bool, std::string>> NonStandard::ActoolOptions::
parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it)
{
    std::string const &arg = **it;

    if (arg == "--allow-non-standard-behavior") {
        return libutil::Options::Current<bool>(&_allowNonStandardBehavior, arg);
    } else if (arg == "--allow-image-type") {
        return InsertNextImageType(_allowImageTypes, args, it);
    } else {
        return ext::nullopt;
    }
}

bool NonStandard::ActoolOptions::
isValid(acdriver::Result *result) const
{
    if (!allowNonStandardBehavior() && !allowImageTypes().empty()) {
        result->normal(Result::Severity::Error, "--allow-image-type requires --allow-non-standard-behavior");
        return false;
    }
    return true;
}

car::Rendition::Data::Format NonStandard::
ImageTypeToDataFormat(ImageType type) {
    switch (type) {
        case NonStandard::ImageType::WEBP:
            return car::Rendition::Data::Format::NON_STANDARD_WEBP;
    }

    abort();
}

ext::optional<NonStandard::ImageType> NonStandard::
ImageTypeFromFileExtension(std::string const &extension)
{
    std::string lowerExtension;
    std::transform(extension.begin(), extension.end(), std::back_inserter(lowerExtension), ::tolower);
    if (lowerExtension == "webp") {
        return NonStandard::ImageType::WEBP;
    }
    return ext::nullopt;
}
