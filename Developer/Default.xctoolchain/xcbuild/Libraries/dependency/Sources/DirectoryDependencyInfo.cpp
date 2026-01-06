/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <dependency/DirectoryDependencyInfo.h>
#include <dependency/DependencyInfo.h>
#include <libutil/Filesystem.h>

using dependency::DirectoryDependencyInfo;
using dependency::DependencyInfo;
using libutil::Filesystem;

DirectoryDependencyInfo::
DirectoryDependencyInfo(std::string const &directory, DependencyInfo const &dependencyInfo) :
    _directory     (directory),
    _dependencyInfo(dependencyInfo)
{
}

ext::optional<DirectoryDependencyInfo> DirectoryDependencyInfo::
Deserialize(Filesystem const *filesystem, std::string const &directory)
{
    std::vector<std::string> inputs;

    /* Verify is directory. */
    if (filesystem->type(directory) != Filesystem::Type::Directory) {
        return ext::nullopt;
    }

    /* Recursively add all paths under this directory. */
    filesystem->readDirectory(directory, true, [&](std::string const &path) -> bool {
        inputs.push_back(directory + "/" + path);
        return true;
    });

    /* Create dependency info. */
    auto info = DependencyInfo(inputs, std::vector<std::string>());
    auto directoryInfo = DirectoryDependencyInfo(directory, info);

    return DirectoryDependencyInfo(directoryInfo);
}
