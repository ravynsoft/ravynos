/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_OptionsResult_h
#define __pbxbuild_Tool_OptionsResult_h

#include <pbxspec/PBX/FileType.h>
#include <pbxspec/PBX/PropertyOption.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace pbxsetting { class Environment; }

namespace pbxbuild {
namespace Tool {

class Environment;

class OptionsResult {
private:
    std::vector<std::string>                     _arguments;
    std::unordered_map<std::string, std::string> _environment;
    std::vector<std::string>                     _linkerArgs;

public:
    OptionsResult(std::vector<std::string> const &arguments, std::unordered_map<std::string, std::string> const &environment, std::vector<std::string> const &linkerArgs);
    ~OptionsResult();

public:
    std::vector<std::string> const &arguments() const
    { return _arguments; }
    std::unordered_map<std::string, std::string> const &environment() const
    { return _environment; }
    std::vector<std::string> const &linkerArgs() const
    { return _linkerArgs; }

public:
    static OptionsResult Create(
        pbxsetting::Environment const &environment,
        std::string const &workingDirectory,
        std::vector<pbxspec::PBX::PropertyOption::shared_ptr> const &options,
        pbxspec::PBX::FileType::shared_ptr const &fileType,
        std::unordered_set<std::string> const &deletedSettings = std::unordered_set<std::string>());

    static OptionsResult Create(
        Tool::Environment const &toolEnvironment,
        std::string const &workingDirectory,
        pbxspec::PBX::FileType::shared_ptr const &fileType);
};

}
}

#endif // !__pbxbuild_Tool_OptionsResult_h
