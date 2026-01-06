/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __dependency_DirectoryDependencyInfo_h
#define __dependency_DirectoryDependencyInfo_h

#include <dependency/DependencyInfo.h>
#include <dependency/DependencyInfoFormat.h>

#include <memory>
#include <string>
#include <ext/optional>

namespace libutil { class Filesystem; }

namespace dependency {

/*
 * Dependency info created from the contents of a directory.
 */
class DirectoryDependencyInfo {
private:
    std::string    _directory;

private:
    DependencyInfo _dependencyInfo;

public:
    DirectoryDependencyInfo(
        std::string const &directory,
        DependencyInfo const &dependencyInfo);

public:
    /*
     * The path to the directory.
     */
    std::string const &directory() const
    { return _directory; }

public:
    /*
     * The loaded dependency info.
     */
    DependencyInfo const &dependencyInfo() const
    { return _dependencyInfo; }

public:
    /*
     * Create dependency info for a directory.
     */
    static ext::optional<DirectoryDependencyInfo>
    Deserialize(libutil::Filesystem const *filesystem, std::string const &directory);

public:
    /*
     * The dependency info format.
     */
    DependencyInfoFormat Format() { return DependencyInfoFormat::Directory; }
};

}

#endif /* __dependency_DirectoryDependencyInfo_h */
