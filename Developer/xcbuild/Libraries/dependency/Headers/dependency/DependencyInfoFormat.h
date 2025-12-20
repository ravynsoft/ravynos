/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __dependency_DependencyInfoFormat_h
#define __dependency_DependencyInfoFormat_h

#ifdef __linux__
#include <cstdint>
#endif
#include <string>

namespace dependency {

/*
 * The available dependency info formats
 */
enum class DependencyInfoFormat {
    Binary,
    Directory,
    Makefile,
};

class DependencyInfoFormats {
private:
    DependencyInfoFormats();
    ~DependencyInfoFormats();

public:
    /*
     * The standard name of a dependency info format.
     */
    static bool Name(DependencyInfoFormat const format, std::string *name);

    /*
     * Convert an arbitrary string to a dependency info format.
     */
    static bool Parse(std::string const &name, DependencyInfoFormat *format);
};

}

#endif /* __dependency_DependencyInfoFormat_h */
