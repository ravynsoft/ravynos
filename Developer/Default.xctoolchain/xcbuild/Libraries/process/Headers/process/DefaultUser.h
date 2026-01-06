/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __process_DefaultUser_h
#define __process_DefaultUser_h

#include <process/User.h>

namespace process {

/*
 * The user for the current process. Generally, this should only be
 * created in `main()` and passed down to anywhere else that needs it.
 */
class DefaultUser : public User {
public:
    explicit DefaultUser();
    virtual ~DefaultUser();

public:
    virtual std::string const &userID() const;
    virtual std::string const &groupID() const;
    virtual std::string const &userName() const;
    virtual std::string const &groupName() const;

public:
    virtual ext::optional<std::string> userHomeDirectory() const;
};

}

#endif  // !__process_DefaultUser_h
