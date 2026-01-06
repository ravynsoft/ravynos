/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_ShellScriptBuildPhase_h
#define __pbxproj_PBX_ShellScriptBuildPhase_h

#include <pbxproj/PBX/BuildPhase.h>
#include <pbxsetting/Value.h>

namespace pbxproj { namespace PBX {

class ShellScriptBuildPhase : public BuildPhase {
public:
    typedef std::shared_ptr <ShellScriptBuildPhase> shared_ptr;

private:
    std::string                    _name;
    std::string                    _shellPath;
    std::string                    _shellScript;
    std::vector<pbxsetting::Value> _inputPaths;
    std::vector<pbxsetting::Value> _outputPaths;
    bool                           _showEnvVarsInLog;

public:
    ShellScriptBuildPhase();

public:
    inline std::string const &name() const
    { return _name; }

public:
    inline std::string const &shellPath() const
    { return _shellPath; }
    inline std::string const &shellScript() const
    { return _shellScript; }

public:
    inline std::vector<pbxsetting::Value> const &inputPaths() const
    { return _inputPaths; }
    inline std::vector<pbxsetting::Value> const &outputPaths() const
    { return _outputPaths; }

public:
    inline bool showEnvVarsInLog() const
    { return _showEnvVarsInLog; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXShellScriptBuildPhase; }
};

} }

#endif  // !__pbxproj_PBX_BuildPhase_h
