/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/Asset/IconSet.h>
#include <plist/Keys/Unpack.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>

using xcassets::Asset::IconSet;
namespace Slot = xcassets::Slot;
using libutil::Filesystem;
using libutil::FSUtil;

IconSet::Icon::
Icon(std::string const &path, double width, double height, ext::optional<Slot::Scale> const &scale) :
    _path  (path),
    _width (width),
    _height(height),
    _scale (scale)
{
}

ext::optional<IconSet::Icon> IconSet::Icon::
Parse(std::string const &path)
{
    if (FSUtil::GetFileExtension(path) != "png") {
        fprintf(stderr, "warning: icon is not png\n");
        return ext::nullopt;
    }

    std::string name = FSUtil::GetBaseNameWithoutExtension(path);

    std::string::size_type underscore = name.find('_');
    if (underscore == std::string::npos) {
        fprintf(stderr, "warning: malformed icon name\n");
        return ext::nullopt;
    }

    if (name.substr(0, underscore) != "icon") {
        fprintf(stderr, "warning: malformed icon name\n");
        return ext::nullopt;
    }

    std::string::size_type x = name.find('x', underscore);
    if (x == std::string::npos) {
        fprintf(stderr, "warning: malformed icon name\n");
        return ext::nullopt;
    }

    std::string::size_type at = name.find('@', x);

    std::string w = name.substr(underscore + 1, x - underscore - 1);
    std::string h = name.substr(x + 1, (at != std::string::npos ? at - x - 1 : at));

    /*
     * Parse height. Must be the whole string.
     */
    char *wend = NULL;
    double width = ::strtod(w.c_str(), &wend);
    if (wend != &w[w.size()] || w.size() == 0) {
        fprintf(stderr, "warning: width not a number %s\n", w.c_str());
        return ext::nullopt;
    }

    /*
     * Parse height. Must be the whole string.
     */
    char *hend = NULL;
    double height = ::strtod(h.c_str(), &hend);
    if (hend != &h[h.size()] || h.size() == 0) {
        fprintf(stderr, "warning: height not a number %s\n", h.c_str());
        return ext::nullopt;
    }

    /* Scale is optional. */
    ext::optional<Slot::Scale> scale;
    if (at != std::string::npos) {
        std::string s = name.substr(at + 1);

        scale = Slot::Scale::Parse(s);
        if (!scale) {
            return ext::nullopt;
        }
    }

    return Icon(path, width, height, scale);
}

bool IconSet::
load(Filesystem const *filesystem)
{
    if (!Asset::load(filesystem)) {
        return false;
    }

    filesystem->readDirectory(this->path(), false, [this](std::string const &name) {
        std::string path = this->path() + "/" + name;
        if (ext::optional<Icon> icon = Icon::Parse(path)) {
            _icons.push_back(*icon);
        }
    });

    return true;
}

bool IconSet::
parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!this->children().empty()) {
        fprintf(stderr, "warning: unexpected child assets\n");
    }

    if (!Asset::parse(dict, seen, false)) {
        return false;
    }

    /* Contents is optional. */
    if (dict != nullptr) {
        auto unpack = plist::Keys::Unpack("IconSet", dict, seen);

        /* No properties. */

        if (!unpack.complete(check)) {
            fprintf(stderr, "%s", unpack.errorText().c_str());
        }
    }

    return true;
}

