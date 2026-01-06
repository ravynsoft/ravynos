/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_VariantGroup_h
#define __pbxproj_PBX_VariantGroup_h

#include <pbxproj/PBX/BaseGroup.h>

namespace pbxproj { namespace PBX {

class VariantGroup : public BaseGroup {
public:
    typedef std::shared_ptr <VariantGroup> shared_ptr;

public:
    VariantGroup();

public:
    static inline char const *Isa()
    { return ISA::PBXVariantGroup; }
};

} }

#endif  // !__pbxproj_PBX_VariantGroup_h
