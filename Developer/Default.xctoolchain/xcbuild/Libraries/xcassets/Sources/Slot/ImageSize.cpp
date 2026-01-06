/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/ImageSize.h>

#include <sstream>

using xcassets::Slot::ImageSize;

ImageSize::
ImageSize(double width, double height) :
    _width(width),
    _height(height)
{
}

ext::optional<ImageSize> ImageSize::
Parse(std::string const &value)
{
    /* Must not be empty */
    if (value.empty()) {
        fprintf(stderr, "warning: size not valid %s\n", value.c_str());
        return ext::nullopt;
    }

    std::string::size_type x = value.find('x');
    if (x == std::string::npos || x == 0 || x == value.size() - 1) {
        /* Must contain two strings separated by 'x' */
        fprintf(stderr, "warning: size not valid %s\n", value.c_str());
        return ext::nullopt;
    }

    std::string w = value.substr(0, x);
    std::string h = value.substr(x + 1, value.size() - x - 1);

    char *wend = NULL;
    double width = std::strtod(w.c_str(), &wend);
    if (wend != &w[w.size()]) {
        fprintf(stderr, "warning: width not valid %s\n", w.c_str());
        return ext::nullopt;
    }

    char *hend = NULL;
    double height = std::strtod(h.c_str(), &hend);
    if (hend != &h[h.size()]) {
        fprintf(stderr, "warning: height not valid %s\n", h.c_str());
        return ext::nullopt;
    }

    return ImageSize(width, height);
}

std::string ImageSize::
String(ImageSize imageSize)
{
    std::ostringstream out;
    out << imageSize.width();
    out << "x";
    out << imageSize.height();
    return out.str();
}
