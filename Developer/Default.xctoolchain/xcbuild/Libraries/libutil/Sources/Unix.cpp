/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/Unix.h>

#include <cstring>
#include <ext/optional>

namespace Path = libutil::Path;

char Path::Unix::
Separator = '/';

bool Path::Unix::
IsSeparator(char c)
{
    return (c == Path::Unix::Separator);
}

bool Path::Unix::
IsAbsolute(std::string const &path, size_t *start)
{
    bool absolute = (path.size() >= 1 && path[0] == Path::Unix::Separator);
    if (start != nullptr) {
        *start = (absolute ? 1 : 0);
    }
    return absolute;
}

bool Path::Unix::
Resolve(
    std::string const &path,
    std::string const &against,
    std::string *base,
    std::string *relative)
{
    *base = against;
    *relative = path;
    return true;
}

