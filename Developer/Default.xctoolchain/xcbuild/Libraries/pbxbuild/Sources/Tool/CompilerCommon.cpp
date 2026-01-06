/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Tool/CompilerCommon.h>
#include <pbxbuild/Tool/SearchPaths.h>
#include <pbxbuild/Tool/HeadermapInfo.h>
#include <pbxsetting/Environment.h>

namespace Tool = pbxbuild::Tool;

void Tool::CompilerCommon::
AppendCompoundFlags(std::vector<std::string> *args, std::string const &prefix, bool concatenate, std::vector<std::string> const &values)
{
    for (std::string const &value : values) {
        if (concatenate) {
            args->push_back(prefix + value);
        } else {
            args->push_back(prefix);
            args->push_back(value);
        }
    }
}

void Tool::CompilerCommon::
AppendIncludePathFlags(std::vector<std::string> *args, pbxsetting::Environment const &environment, Tool::SearchPaths const &searchPaths, Tool::HeadermapInfo const &headermapInfo)
{
    AppendCompoundFlags(args, "-I", true, headermapInfo.systemHeadermapFiles());
    AppendCompoundFlags(args, "-iquote", false, headermapInfo.userHeadermapFiles());

    if (environment.resolve("USE_HEADER_SYMLINKS") == "YES") {
        // TODO(grp): Create this symlink tree as needed.
        AppendCompoundFlags(args, "-I", true, { environment.resolve("CPP_HEADER_SYMLINKS_DIR") });
    }

    AppendCompoundFlags(args, "-I", true, {
        environment.resolve("BUILT_PRODUCTS_DIR") + "/include",
    });
    AppendCompoundFlags(args, "-I", true, searchPaths.userHeaderSearchPaths());
    AppendCompoundFlags(args, "-I", true, searchPaths.headerSearchPaths());
    AppendCompoundFlags(args, "-I", true, {
        environment.resolve("DERIVED_FILE_DIR") + "/" + environment.resolve("arch"),
        environment.resolve("DERIVED_FILE_DIR"),
    });
}

