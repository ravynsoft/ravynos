/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_MatchingStyle_h
#define __xcassets_MatchingStyle_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * A method to match an asset by name.
 */
enum class MatchingStyle {
    /*
     * The only supported style. Use the asset name and
     * any groups defining a namespace containing it.
     */
    FullyQualifiedName,
};

class MatchingStyles {
private:
    MatchingStyles();
    ~MatchingStyles();

public:
    /*
     * Parse a matching style from a string, if valid.
     */
    static ext::optional<MatchingStyle> Parse(std::string const &value);

    /*
     * Convert a style to a string.
     */
    static std::string String(MatchingStyle style);
};

}

#endif // !__xcassets_MatchingStyle_h
