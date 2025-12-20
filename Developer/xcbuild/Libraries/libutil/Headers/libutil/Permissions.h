/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __libutil_Permissions_h
#define __libutil_Permissions_h

#include <bitset>
#include <limits>
#ifdef __linux__
#include <cstdint>
#endif

namespace libutil {

/*
 * A complete set of file permissions.
 */
class Permissions {
public:
    using Base = uint8_t;

public:
    /*
     * A possible file permission.
     */
    enum class Permission : Base {
        Read,
        Write,
        Execute,
    };

    /*
     * Additional file permission flags.
     */
    enum class Flag : Base {
        Sticky,
        SetUserID,
        SetGroupID,
    };

public:
    /*
     * Possible operations for combining permissions.
     */
    enum class Operation {
        /*
         * Replace permissions.
         */
        Set,
        /*
         * Add permissions to existing permissions.
         */
        Add,
        /*
         * Remove permissions from existing permissions.
         */
        Remove,
    };

private:
    using Set = std::bitset<std::numeric_limits<Base>::digits>;

private:
    Set _user;
    Set _group;
    Set _other;
    Set _flags;

public:
    Permissions() = default;
    Permissions(
        std::initializer_list<Permission> user,
        std::initializer_list<Permission> group,
        std::initializer_list<Permission> other,
        std::initializer_list<Flag> flags = { });

public:
    bool user(Permission permission) const
    { return _user.test(static_cast<Base>(permission)); }
    bool group(Permission permission) const
    { return _group.test(static_cast<Base>(permission)); }
    bool other(Permission permission) const
    { return _other.test(static_cast<Base>(permission)); }
    bool flag(Flag flag) const
    { return _flags.test(static_cast<Base>(flag)); }

public:
    void user(Permission permission, bool value)
    { _user.set(static_cast<Base>(permission), value); }
    void group(Permission permission, bool value)
    { _group.set(static_cast<Base>(permission), value); }
    void other(Permission permission, bool value)
    { _other.set(static_cast<Base>(permission), value); }
    void flag(Flag flag, bool value)
    { _flags.set(static_cast<Base>(flag), value); }

public:
    /*
     * Combine permissions with other using the operation.
     */
    void combine(Operation operation, Permissions other);
};

}

#endif  // !__libutil_Permissions_h
