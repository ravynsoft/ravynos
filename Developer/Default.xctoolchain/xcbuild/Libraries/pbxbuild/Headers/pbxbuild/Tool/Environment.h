/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Tool_Environment_h
#define __pbxbuild_Tool_Environment_h

#include <pbxbuild/Tool/Input.h>
#include <pbxsetting/Environment.h>

namespace pbxbuild {
namespace Tool {

class Environment {
private:
    pbxspec::PBX::Tool::shared_ptr _tool;
    pbxsetting::Environment        _environment;

private:
    std::vector<std::string>       _inputs;
    std::vector<std::string>       _outputs;

public:
    Environment(
        pbxspec::PBX::Tool::shared_ptr const &tool,
        pbxsetting::Environment const &environment,
        std::vector<std::string> const &inputs,
        std::vector<std::string> const &outputs);
    ~Environment();

public:
    pbxspec::PBX::Tool::shared_ptr const &tool() const
    { return _tool; }
    pbxsetting::Environment const &environment() const
    { return _environment; }

public:
    std::vector<std::string> const &inputs() const
    { return _inputs; }
    std::vector<std::string> const &outputs() const
    { return _outputs; }

public:
    std::vector<std::string> inputs(std::string const &workingDirectory) const;
    std::vector<std::string> outputs(std::string const &workingDirectory) const;

public:
    static Tool::Environment
    Create(
        pbxspec::PBX::Tool::shared_ptr const &tool,
        pbxsetting::Environment const &environment,
        std::string const &workingDirectory,
        std::vector<Tool::Input> const &inputs,
        std::vector<std::string> const &outputs = { });
};

}
}

#endif // !__pbxbuild_Tool_Environment_h
