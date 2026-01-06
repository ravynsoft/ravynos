/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <process/MemoryUser.h>

using process::MemoryUser;

MemoryUser::
MemoryUser(
    std::string const &userID,
    std::string const &groupID,
    std::string const &userName,
    std::string const &groupName) :
    User      (),
    _userID   (userID),
    _groupID  (groupID),
    _userName (userName),
    _groupName(groupName)
{
}

MemoryUser::
MemoryUser(User const *user) :
    MemoryUser(
        user->userID(),
        user->groupID(),
        user->userName(),
        user->groupName())
{
}

MemoryUser::
~MemoryUser()
{
}

