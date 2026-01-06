/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __acdriver_NonStandard_h
#define __acdriver_NonStandard_h

#include <acdriver/Result.h>
#include <car/Rendition.h>

#include <algorithm>
#include <ext/optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace acdriver {

/*
 * Contains non-standard related logic
 */
class NonStandard {
public:
    /*
     *  Extra image types (besides PNG and JPEG) to include in ImageSet
     */
    enum class ImageType {
        WEBP,
    };

    class ImageTypeHash
    {
    public:
        std::size_t operator()(ImageType t) const
        {
            return static_cast<std::size_t>(t);
        }
    };

    typedef std::unordered_set<ImageType, ImageTypeHash> ImageTypeSet;

public:
    /*
     * Extension options for actool
     */
    class ActoolOptions {
    private:
        ext::optional<bool>        _allowNonStandardBehavior;
        ImageTypeSet               _allowImageTypes;

    public:
        bool allowNonStandardBehavior() const
        { return _allowNonStandardBehavior.value_or(false); }
        ImageTypeSet allowImageTypes() const
        { return _allowImageTypes; }

    public:
        ext::optional<std::pair<bool, std::string>> parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);

    public:
        bool isValid(acdriver::Result *result) const;
    };

public:
    /*
     * Translates a filename extension of an acceptable type to ImageType enum.
     */
    static ext::optional<ImageType> ImageTypeFromFileExtension(std::string const &extension);

    /*
     * Translates an ImageType enum to the proper data format used in car::Rendition
     */
    static car::Rendition::Data::Format ImageTypeToDataFormat(ImageType type);
};

}

#endif // !__acdriver_NonStandard_h
