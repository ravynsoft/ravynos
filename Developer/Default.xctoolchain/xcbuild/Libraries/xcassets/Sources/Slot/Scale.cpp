/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Slot/Scale.h>

#include <sstream>

using xcassets::Slot::Scale;

Scale::
Scale(double value) :
    _value(value)
{
}

ext::optional<Scale> Scale::
Parse(std::string const &value)
{
    /* Must end in 'x'. */
    if (value.empty() || value[value.size() - 1] != 'x') {
        fprintf(stderr, "warning: scale not valid %s\n", value.c_str());
        return ext::nullopt;
    }

    /* Number should be the rest. */
    std::string number = value.substr(0, value.size() - 1);

    char *end = NULL;
    double scale = std::strtod(number.c_str(), &end);
    if (end == &number[number.size()]) {
        return Scale(scale);
    } else {
        fprintf(stderr, "warning: scale not a number %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string Scale::
String(Scale scale)
{
    std::ostringstream out;
    out << scale.value();
    out << "x";
    return out.str();
}
