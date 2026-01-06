/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_SourcesBuildPhase_h
#define __pbxproj_PBX_SourcesBuildPhase_h

#include <pbxproj/PBX/BuildPhase.h>

namespace pbxproj { namespace PBX {

class SourcesBuildPhase : public BuildPhase {
public:
    typedef std::shared_ptr <SourcesBuildPhase> shared_ptr;

public:
    SourcesBuildPhase();

public:
    static inline char const *Isa()
    { return ISA::PBXSourcesBuildPhase; }
};

} }

#endif  // !__pbxproj_PBX_BuildPhase_h
