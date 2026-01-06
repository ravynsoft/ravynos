/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __process_MemoryUser_h
#define __process_MemoryUser_h

#include <process/User.h>

namespace process {

/*
 * A process user with arbitrary values.
 */
class MemoryUser : public User {
private:
    std::string _userID;
    std::string _groupID;
    std::string _userName;
    std::string _groupName;

public:
    MemoryUser(
        std::string const &userID,
        std::string const &groupID,
        std::string const &userName,
        std::string const &groupName);
    explicit MemoryUser(User const *user);
    virtual ~MemoryUser();

public:
    virtual std::string const &userID() const
    { return _userID; }
    std::string &userID()
    { return _userID; }

    virtual std::string const &groupID() const
    { return _groupID; }
    std::string &groupID()
    { return _groupID; }

    virtual std::string const &userName() const
    { return _userName; }
    std::string &userName()
    { return _userName; }

    virtual std::string const &groupName() const
    { return _groupName; }
    std::string &groupName()
    { return _groupName; }
};

}

#endif  // !__process_MemoryUser_h
