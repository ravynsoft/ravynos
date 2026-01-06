/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_XC_VersionGroup_h
#define __pbxproj_XC_VersionGroup_h

#include <pbxproj/PBX/BaseGroup.h>

namespace pbxproj { namespace XC {

class VersionGroup : public PBX::BaseGroup {
public:
    typedef std::shared_ptr <VersionGroup> shared_ptr;

private:
    PBX::GroupItem::shared_ptr _currentVersion;
    std::string                _versionGroupType;

public:
    VersionGroup();

public:
    inline PBX::GroupItem::shared_ptr const &currentVersion() const
    { return _currentVersion; }

public:
    inline std::string const &versionGroupType() const
    { return _versionGroupType; }

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;

public:
    static inline char const *Isa()
    { return ISA::XCVersionGroup; }
};

} }

#endif  // !__pbxproj_XC_VersionGroup_h
