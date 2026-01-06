/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_CopyFilesBuildPhase_h
#define __pbxproj_PBX_CopyFilesBuildPhase_h

#include <pbxproj/PBX/BuildPhase.h>
#include <pbxsetting/Value.h>

namespace pbxproj { namespace PBX {

class CopyFilesBuildPhase : public BuildPhase {
public:
    typedef std::shared_ptr <CopyFilesBuildPhase> shared_ptr;

public:
    enum Destination {
        kDestinationAbsolute         = 0,
        kDestinationWrapper          = 1,
        kDestinationExecutables      = 6,
        kDestinationResources        = 7,
        kDestinationPublicHeaders    = 8,
        kDestinationPrivateHeaders   = 9,
        kDestinationFrameworks       = 10,
        kDestinationSharedFrameworks = 11,
        kDestinationSharedSupport    = 12,
        kDestinationPlugIns          = 13,
        kDestinationScripts          = 14,
        kDestinationJavaResources    = 15,
        kDestinationProducts         = 16,
    };

private:
    pbxsetting::Value _dstPath;
    Destination       _dstSubfolderSpec;

public:
    CopyFilesBuildPhase();

public:
    inline pbxsetting::Value const &dstPath() const
    { return _dstPath; }

public:
    inline Destination dstSubfolderSpec() const
    { return _dstSubfolderSpec; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXCopyFilesBuildPhase; }
};

} }

#endif  // !__pbxproj_PBX_BuildPhase_h
