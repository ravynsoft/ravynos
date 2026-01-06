/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __acdriver_Compile_Convert_h
#define __acdriver_Compile_Convert_h

#include <xcassets/Insets.h>
#include <xcassets/Resizing.h>
#include <xcassets/Slot/DeviceSubtype.h>
#include <xcassets/Slot/Idiom.h>
#include <xcassets/Slot/Scale.h>
#include <car/Rendition.h>
#include <car/car_format.h>

#include <string>
#include <vector>

namespace acdriver {
namespace Compile {

/*
 * Convert various asset properties to a compiled form.
 */
class Convert {
private:
    Convert();
    ~Convert();

public:
    /*
     * The suffix for an image file name or plist key for an idiom.
     */
    static std::string IdiomSuffix(xcassets::Slot::Idiom idiom);

    /*
     * The suffix for an image file name for a specific scale.
     */
    static std::string ScaleSuffix(xcassets::Slot::Scale const &scale);

    /*
     * The suffix for an image file name for a specific phone subtype.
     */
    static std::string DeviceSubtypeSuffix(xcassets::Slot::DeviceSubtype subtype);

public:
    /*
     * The archive attribute corresponding to an idiom.
     */
    static uint16_t IdiomAttribute(xcassets::Slot::Idiom idiom);

    /*
     * The rendition layout value for resizing information.
     */
    static enum car_rendition_value_layout
    LayoutForResizingAndCenterMode(
        xcassets::Resizing::Mode resizingMode,
        xcassets::Resizing::Center::Mode centerMode);

    /*
     * The rendition slices for a resizing mode.
     */
    static std::vector<car::Rendition::Slice>
    SlicesForResizingModeAndCapInsets(
        uint32_t width,
        uint32_t height,
        xcassets::Resizing::Mode resizingMode,
        ext::optional<xcassets::Insets> const &capInsets);
};

}
}

#endif // !__acdriver_Compile_Convert_h
