/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxproj_PBX_GroupItem_h
#define __pbxproj_PBX_GroupItem_h

#include <pbxproj/PBX/Object.h>
#include <pbxsetting/Value.h>

namespace pbxproj { namespace PBX {

class GroupItem : public Object {
public:
    typedef std::shared_ptr <GroupItem> shared_ptr;
    typedef std::vector <shared_ptr> vector;

public:
    enum class Type {
        Group,
        VariantGroup,
        VersionGroup,
        FileReference,
        ReferenceProxy
    };

private:
    Type        _type;

protected:
    friend class BaseGroup;
    GroupItem  *_parent;
    std::string _name;
    std::string _path;
    std::string _sourceTree;

protected:
    GroupItem(std::string const &isa, Type type);

public:
    inline Type type() const
    { return _type; }

public:
    inline std::string const &name() const
    { return _name.empty() ? _path : _name; }

public:
    inline std::string const &path() const
    { return _path; }

public:
    inline std::string const &sourceTree() const
    { return _sourceTree; }

public:
    pbxsetting::Value resolve(void) const;

protected:
    bool parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check) override;
};

} }

#endif  // !__pbxproj_PBX_GroupItem_h
