/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __process_User_h
#define __process_User_h

#include <string>
#include <ext/optional>

namespace process {

/*
 * The information passed into a launched process.
 */
class User {
protected:
    User();
    virtual ~User();

public:
    /*
     * Active user ID.
     */
    virtual std::string const &userID() const = 0;

    /*
     * Active group ID.
     */
    virtual std::string const &groupID() const = 0;

    /*
     * Active user name.
     */
    virtual std::string const &userName() const = 0;

    /*
     * Active group name.
     */
    virtual std::string const &groupName() const = 0;

public:

    /*
     * The home directory from the environment.
     */
    virtual ext::optional<std::string> userHomeDirectory() const = 0;
};

}

#endif  // !__process_User_h
