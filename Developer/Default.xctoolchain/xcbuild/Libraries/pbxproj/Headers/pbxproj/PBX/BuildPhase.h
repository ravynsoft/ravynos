/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_BuildPhase_h
#define __pbxproj_PBX_BuildPhase_h

#ifdef __linux__
#include <cstdint>
#endif

#include <pbxproj/PBX/BuildFile.h>

namespace pbxproj { namespace PBX {

class BuildPhase : public Object {
public:
    typedef std::shared_ptr <BuildPhase> shared_ptr;
    typedef std::vector <shared_ptr> vector;

public:
    enum class Type {
        Headers,
        Sources,
        Resources,
        Frameworks,
        CopyFiles,
        ShellScript,
        AppleScript,
        Rez,
    };

    enum ActionMask {
        // TODO(grp): This is incomplete.
        kActionBuild   = 4,
        kActionInstall = 8,
    };

private:
    Type              _type;
    std::string       _name;
    BuildFile::vector _files;
    bool              _runOnlyForDeploymentPostprocessing;
    uint32_t          _buildActionMask;

protected:
    BuildPhase(std::string const &isa, Type type);

public:
    inline Type type() const
    { return _type; }

public:
    inline std::string const &name() const
    { return _name; }

public:
    inline BuildFile::vector const &files() const
    { return _files; }
    inline BuildFile::vector &files()
    { return _files; }

public:
    inline bool runOnlyForDeploymentPostprocessing() const
    { return _runOnlyForDeploymentPostprocessing; }

public:
    inline uint32_t buildActionMask() const
    { return _buildActionMask; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;
};

} }

#endif  // !__pbxproj_PBX_BuildPhase_h
