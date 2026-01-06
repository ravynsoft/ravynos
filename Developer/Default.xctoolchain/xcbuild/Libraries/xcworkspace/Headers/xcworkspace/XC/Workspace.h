/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcworkspace_XC_Workspace_h
#define __xcworkspace_XC_Workspace_h

#include <xcworkspace/XC/GroupItem.h>

namespace libutil { class Filesystem; }

namespace xcworkspace { namespace XC {

class Workspace {
public:
    typedef std::shared_ptr <Workspace> shared_ptr;

private:
    std::string       _projectFile;
    std::string       _dataFile;
    std::string       _basePath;
    std::string       _name;

private:
    GroupItem::vector _items;

public:
    Workspace();

public:
    static shared_ptr Open(libutil::Filesystem const *filesystem, std::string const &path);

public:
    inline std::string const &projectFile() const
    { return _projectFile; }
    inline std::string const &dataFile() const
    { return _dataFile; }
    inline std::string const &basePath() const
    { return _basePath; }
    inline std::string const &name() const
    { return _name; }

public:
    inline GroupItem::vector const &items() const
    { return _items; }
    inline GroupItem::vector &items()
    { return _items; }

private:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcworkspace_XC_Workspace_h
