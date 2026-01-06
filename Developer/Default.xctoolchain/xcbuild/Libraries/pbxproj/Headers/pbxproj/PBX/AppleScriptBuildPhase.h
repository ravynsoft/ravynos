/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_AppleScriptBuildPhase_h
#define __pbxproj_PBX_AppleScriptBuildPhase_h

#include <pbxproj/PBX/BuildPhase.h>

namespace pbxproj { namespace PBX {

class AppleScriptBuildPhase : public BuildPhase {
public:
    typedef std::shared_ptr <AppleScriptBuildPhase> shared_ptr;

private:
    std::string _contextName;
    bool        _isSharedContext;

public:
    AppleScriptBuildPhase();

public:
    inline std::string const &contextName() const
    { return _contextName; }

public:
    inline bool isSharedContext() const
    { return _isSharedContext; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXAppleScriptBuildPhase; }
};

} }

#endif  // !__pbxproj_PBX_BuildPhase_h
