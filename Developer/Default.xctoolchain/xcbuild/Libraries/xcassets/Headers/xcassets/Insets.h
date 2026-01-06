/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Insets_h
#define __xcassets_Insets_h

#include <plist/Dictionary.h>

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * Edge insets are insets from edges on all sides.
 */
class Insets {
private:
    ext::optional<double> _top;
    ext::optional<double> _left;
    ext::optional<double> _bottom;
    ext::optional<double> _right;

public:
    ext::optional<double> const &top() const
    { return _top; }
    ext::optional<double> const &left() const
    { return _left; }
    ext::optional<double> const &bottom() const
    { return _bottom; }
    ext::optional<double> const &right() const
    { return _right; }

public:
    bool parse(plist::Dictionary const *dict);
};

}

#endif // !__xcassets_Insets_h
