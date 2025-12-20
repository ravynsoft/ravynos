/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_Group_h
#define __pbxproj_PBX_Group_h

#ifdef __linux__
#include <cstdint>
#endif
#include <pbxproj/PBX/BaseGroup.h>

namespace pbxproj { namespace PBX {

class Group : public BaseGroup {
public:
    typedef std::shared_ptr <Group> shared_ptr;

private:
    uint32_t _indentWidth;
    uint32_t _tabWidth;

public:
    Group();

public:
    inline uint32_t indentWidth() const
    { return _indentWidth; }

    inline uint32_t tabWidth() const
    { return _tabWidth; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::PBXGroup; }
};

} }

#endif  // !__pbxproj_PBX_Group_h
