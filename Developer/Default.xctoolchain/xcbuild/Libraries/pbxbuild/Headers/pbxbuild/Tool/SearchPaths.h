/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_SearchPaths_h
#define __pbxbuild_Tool_SearchPaths_h

#include <string>
#include <vector>

namespace pbxsetting { class Environment; }

namespace pbxbuild {
namespace Tool {

class SearchPaths {
private:
    std::vector<std::string> _headerSearchPaths;
    std::vector<std::string> _userHeaderSearchPaths;
    std::vector<std::string> _frameworkSearchPaths;
    std::vector<std::string> _librarySearchPaths;

public:
    SearchPaths(
        std::vector<std::string> const &headerSearchPaths,
        std::vector<std::string> const &userHeaderSearchPaths,
        std::vector<std::string> const &frameworkSearchPaths,
        std::vector<std::string> const &librarySearchPaths);

public:
    std::vector<std::string> const &headerSearchPaths(void) const
    { return _headerSearchPaths; }
    std::vector<std::string> const &userHeaderSearchPaths(void) const
    { return _userHeaderSearchPaths; }
    std::vector<std::string> const &frameworkSearchPaths(void) const
    { return _frameworkSearchPaths; }
    std::vector<std::string> const &librarySearchPaths(void) const
    { return _librarySearchPaths; }

public:
    static Tool::SearchPaths
    Create(pbxsetting::Environment const &environment, std::string const &workingDirectory);

public:
    static std::vector<std::string>
    ExpandRecursive(std::vector<std::string> const &paths, pbxsetting::Environment const &environment, std::string const &workingDirectory);
};

}
}

#endif // !__pbxbuild_Tool_SearchPaths_h
