/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Resizing_h
#define __xcassets_Resizing_h

#include <xcassets/Insets.h>
#include <plist/Dictionary.h>

#include <ext/optional>

namespace xcassets {

/*
 * Resizing information for a stretchable image.
 */
class Resizing {
public:
    class Center {
    public:
        enum class Mode {
            Tile,
            Stretch,
        };

    private:
        ext::optional<Mode>   _mode;
        ext::optional<double> _width;
        ext::optional<double> _height;

    public:
        ext::optional<Mode> const &mode() const
        { return _mode; }
        ext::optional<double> const &width() const
        { return _width; }
        ext::optional<double> const &height() const
        { return _height; }

    public:
        bool parse(plist::Dictionary const *dict);
    };

public:
    enum class Mode {
        ThreePartHorizontal,
        ThreePartVertical,
        NinePart,
    };

private:
    ext::optional<Mode>   _mode;
    ext::optional<Center> _center;
    ext::optional<Insets> _capInsets;

public:
    ext::optional<Mode> const &mode() const
    { return _mode; }
    ext::optional<Center> const &center() const
    { return _center; }
    ext::optional<Insets>const &capInsets() const
    { return _capInsets; }

public:
    bool parse(plist::Dictionary const *dict);
};

}

#endif // !__xcassets_Resizing_h
