/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_CompilerCommon_h
#define __pbxbuild_Tool_CompilerCommon_h

#include <string>
#include <vector>

namespace pbxsetting { class Environment; }

namespace pbxbuild {
namespace Tool {

class SearchPaths;
class HeadermapInfo;

class CompilerCommon {
private:
    CompilerCommon();
    ~CompilerCommon();

public:
    /*
     * Appends a potentially joined flag list. Each value in `values` is added,
     * prefixed by `prefix`. If `concatenate` is true, they are concatenated first.
     */
    static void AppendCompoundFlags(std::vector<std::string> *args, std::string const &prefix, bool concatenate, std::vector<std::string> const &values);

    /*
     * Append flags for include paths, including from header maps.
     */
    static void AppendIncludePathFlags(std::vector<std::string> *args, pbxsetting::Environment const &environment, Tool::SearchPaths const &searchPaths, Tool::HeadermapInfo const &headermapInfo);
};

}
}

#endif // !__pbxbuild_Tool_CompilerCommon_h
