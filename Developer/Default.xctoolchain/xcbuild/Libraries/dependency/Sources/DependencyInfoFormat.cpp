/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <dependency/DependencyInfoFormat.h>

using dependency::DependencyInfoFormat;
using dependency::DependencyInfoFormats;

bool DependencyInfoFormats::
Name(DependencyInfoFormat format, std::string *name)
{
    switch (format) {
        case DependencyInfoFormat::Directory:
            *name = "directory";
            return true;
        case DependencyInfoFormat::Binary:
            *name = "binary";
            return true;
        case DependencyInfoFormat::Makefile:
            *name = "makefile";
            return true;
        default:
            return false;
    }
}

bool DependencyInfoFormats::
Parse(std::string const &name, DependencyInfoFormat *format)
{
    if (name == "binary") {
        *format = DependencyInfoFormat::Binary;
        return true;
    } else if (name == "directory") {
        *format = DependencyInfoFormat::Directory;
        return true;
    } else if (name == "makefile") {
        *format = DependencyInfoFormat::Makefile;
        return true;
    } else {
        return false;
    }
}
