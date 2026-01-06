/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcworkspace_XC_Group_h
#define __xcworkspace_XC_Group_h

#include <xcworkspace/XC/GroupItem.h>

namespace xcworkspace { namespace XC {

class Group : public GroupItem {
public:
    typedef std::shared_ptr <Group> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::string       _name;
    GroupItem::vector _items;

public:
    Group();

public:
    inline std::string const &name() const
    { return _name; }

public:
    inline GroupItem::vector const &items() const
    { return _items; }
    inline GroupItem::vector &items()
    { return _items; }

public:
    bool parse(plist::Dictionary const *dict) override;
};

} }

#endif  // !__xcworkspace_XC_Group_h
