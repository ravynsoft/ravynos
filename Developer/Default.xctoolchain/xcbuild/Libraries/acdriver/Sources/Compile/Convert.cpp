/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Compile/Convert.h>

#include <sstream>

using acdriver::Compile::Convert;

std::string Convert::
IdiomSuffix(xcassets::Slot::Idiom idiom)
{
    switch (idiom) {
        case xcassets::Slot::Idiom::Universal:
            return std::string();
        case xcassets::Slot::Idiom::Phone:
            return std::string();
        case xcassets::Slot::Idiom::Pad:
            return "~ipad";
        case xcassets::Slot::Idiom::Desktop:
            // TODO: no idiom suffix for desktop
            return std::string();
        case xcassets::Slot::Idiom::TV:
            return "~tv";
        case xcassets::Slot::Idiom::Watch:
            return "~watch";
        case xcassets::Slot::Idiom::Car:
            return "~car";
        case xcassets::Slot::Idiom::iOSMarketing:
            // TODO: what does this mean here?
            return std::string();
    }

    abort();
}

std::string Convert::
ScaleSuffix(xcassets::Slot::Scale const &scale)
{
    if (scale.value() != 1.0) {
        std::ostringstream out;
        out << "@";
        out << scale.value();
        out << "x";
        return out.str();
    }

    return std::string();
}

std::string Convert::
DeviceSubtypeSuffix(xcassets::Slot::DeviceSubtype subtype)
{
    switch (subtype) {
        case xcassets::Slot::DeviceSubtype::Retina4:
            return "-568h";
        case xcassets::Slot::DeviceSubtype::Height667:
            return "-667h";
        case xcassets::Slot::DeviceSubtype::Height736:
            return "-736h";
    }

    abort();
}

uint16_t Convert::
IdiomAttribute(xcassets::Slot::Idiom idiom)
{
    switch (idiom) {
        case xcassets::Slot::Idiom::Universal:
            return car_attribute_identifier_idiom_value_universal;
            break;
        case xcassets::Slot::Idiom::Phone:
            return car_attribute_identifier_idiom_value_phone;
            break;
        case xcassets::Slot::Idiom::Pad:
            return car_attribute_identifier_idiom_value_pad;
            break;
        case xcassets::Slot::Idiom::Desktop:
            // TODO: desktop has no idiom value
            return car_attribute_identifier_idiom_value_universal;
        case xcassets::Slot::Idiom::TV:
            return car_attribute_identifier_idiom_value_tv;
        case xcassets::Slot::Idiom::Watch:
            return car_attribute_identifier_idiom_value_watch;
        case xcassets::Slot::Idiom::Car:
            return car_attribute_identifier_idiom_value_car;
        default:
            return car_attribute_identifier_idiom_value_universal;
    }
}

enum car_rendition_value_layout Convert::
LayoutForResizingAndCenterMode(xcassets::Resizing::Mode resizingMode, xcassets::Resizing::Center::Mode centerMode)
{
    switch (resizingMode) {
        case xcassets::Resizing::Mode::ThreePartHorizontal:
            switch (centerMode) {
                case xcassets::Resizing::Center::Mode::Tile:
                    return car_rendition_value_layout_three_part_horizontal_tile;
                case xcassets::Resizing::Center::Mode::Stretch:
                    return car_rendition_value_layout_three_part_horizontal_scale;
                default:
                    return car_rendition_value_layout_three_part_horizontal_tile;
            }
        case xcassets::Resizing::Mode::ThreePartVertical:
            switch (centerMode) {
                case xcassets::Resizing::Center::Mode::Tile:
                    return car_rendition_value_layout_three_part_vertical_tile;
                case xcassets::Resizing::Center::Mode::Stretch:
                    return car_rendition_value_layout_three_part_vertical_scale;
                default:
                    return car_rendition_value_layout_three_part_vertical_tile;
            }
        case xcassets::Resizing::Mode::NinePart:
            switch (centerMode) {
                case xcassets::Resizing::Center::Mode::Tile:
                    return car_rendition_value_layout_nine_part_tile;
                case xcassets::Resizing::Center::Mode::Stretch:
                    return car_rendition_value_layout_nine_part_scale;
                default:
                    return car_rendition_value_layout_nine_part_tile;
            }
        default: abort();
    }
}

std::vector<car::Rendition::Slice> Convert::
SlicesForResizingModeAndCapInsets(uint32_t width, uint32_t height, xcassets::Resizing::Mode resizingMode, ext::optional<xcassets::Insets> const &capInsets)
{
    double topCapInset = capInsets ? capInsets->top().value_or(0) : 0;
    double leftCapInset = capInsets ? capInsets->left().value_or(0) : 0;
    double bottomCapInset = capInsets ? capInsets->bottom().value_or(0) : 0;
    double rightCapInset = capInsets ? capInsets->right().value_or(0) : 0;

    uint32_t leftWidth = static_cast<uint32_t>(leftCapInset);
    uint32_t centerWidth = static_cast<uint32_t>(width - (leftCapInset + rightCapInset));
    uint32_t rightWidth = static_cast<uint32_t>(rightCapInset);

    uint32_t topHeight = static_cast<uint32_t>(topCapInset);
    uint32_t centerHeight = static_cast<uint32_t>(height - (topCapInset + bottomCapInset));
    uint32_t bottomHeight = static_cast<uint32_t>(bottomCapInset);

    uint32_t topVerticalOffset = static_cast<uint32_t>(height - topCapInset);
    uint32_t centerVerticalOffset = static_cast<uint32_t>(bottomCapInset);
    uint32_t bottomVerticalOffset = static_cast<uint32_t>(0);

    uint32_t leftHorizontalOffset = static_cast<uint32_t>(0);
    uint32_t centerHorizontalOffset = static_cast<uint32_t>(leftCapInset);
    uint32_t rightHorizontalOffset = static_cast<uint32_t>(width - rightCapInset);

    switch (resizingMode) {
        case xcassets::Resizing::Mode::ThreePartHorizontal:
            return {
                { leftHorizontalOffset,   0, leftWidth,   height },
                { centerHorizontalOffset, 0, centerWidth, height },
                { rightHorizontalOffset,  0, rightWidth,  height },
            };
        case xcassets::Resizing::Mode::ThreePartVertical:
            return {
                { 0, topVerticalOffset,    width, topHeight    },
                { 0, centerVerticalOffset, width, centerHeight },
                { 0, bottomVerticalOffset, width, bottomHeight },
            };
        case xcassets::Resizing::Mode::NinePart:
            return {
                { leftHorizontalOffset,   topVerticalOffset,    leftWidth,   topHeight    },
                { centerHorizontalOffset, topVerticalOffset,    centerWidth, topHeight    },
                { rightHorizontalOffset,  topVerticalOffset,    rightWidth,  topHeight    },
                { leftHorizontalOffset,   centerVerticalOffset, leftWidth,   centerHeight },
                { centerHorizontalOffset, centerVerticalOffset, centerWidth, centerHeight },
                { rightHorizontalOffset,  centerVerticalOffset, rightWidth,  centerHeight },
                { leftHorizontalOffset,   bottomVerticalOffset, leftWidth,   bottomHeight },
                { centerHorizontalOffset, bottomVerticalOffset, centerWidth, bottomHeight },
                { rightHorizontalOffset,  bottomVerticalOffset, rightWidth,  bottomHeight },
            };
        default: abort();
    }
}

