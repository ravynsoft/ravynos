/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __dependency_MakefileDependencyInfo_h
#define __dependency_MakefileDependencyInfo_h

#include <dependency/DependencyInfo.h>
#include <dependency/DependencyInfoFormat.h>

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <ext/optional>

namespace dependency {

/*
 * The Makefile-based dependency info format used by clang and swiftc.
 */
class MakefileDependencyInfo {
private:
    std::vector<DependencyInfo> _dependencyInfo;

public:
    MakefileDependencyInfo();

public:
    /*
     * The encoded dependency info.
     */
    std::vector<DependencyInfo> const &dependencyInfo() const
    { return _dependencyInfo; }
    std::vector<DependencyInfo> &dependencyInfo()
    { return _dependencyInfo; }

public:
    /*
     * Serialize the dependency info.
     */
    std::string serialize() const;

public:
    /*
     * Create the dependency info from the Makefile contents.
     */
    static ext::optional<MakefileDependencyInfo>
    Deserialize(std::string const &contents);

public:
    /*
     * The dependency info format.
     */
    DependencyInfoFormat Format() { return DependencyInfoFormat::Makefile; }
};

}

#endif /* __dependency_MakefileDependencyInfo_h */
