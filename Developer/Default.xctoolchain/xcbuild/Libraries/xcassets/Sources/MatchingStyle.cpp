/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/MatchingStyle.h>

#include <cstdlib>

using xcassets::MatchingStyle;
using xcassets::MatchingStyles;

ext::optional<MatchingStyle> MatchingStyles::
Parse(std::string const &value)
{
    if (value == "fully-qualified-name") {
        return MatchingStyle::FullyQualifiedName;
    } else {
        fprintf(stderr, "warning: unknown matching style %s\n", value.c_str());
        return ext::nullopt;
    }
}

std::string MatchingStyles::
String(MatchingStyle style)
{
    switch (style) {
        case MatchingStyle::FullyQualifiedName:
            return "fully-qualified-name";
    }

    abort();
}
