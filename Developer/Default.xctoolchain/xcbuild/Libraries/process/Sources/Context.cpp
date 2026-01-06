/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <process/Context.h>

#include <sstream>
#include <unordered_set>

using process::Context;

Context::
Context()
{
}

Context::
~Context()
{
}

std::vector<std::string> Context::
executableSearchPaths() const
{
    std::vector<std::string> paths;

    if (ext::optional<std::string> value = environmentVariable("PATH")) {
        std::unordered_set<std::string> seen;

        std::string path;
        std::istringstream is(*value);
        while (std::getline(is, path, ':')) {
            if (seen.find(path) != seen.end()) {
                continue;
            }

            paths.push_back(path);
            seen.insert(path);
        }
    }

    return paths;
}

