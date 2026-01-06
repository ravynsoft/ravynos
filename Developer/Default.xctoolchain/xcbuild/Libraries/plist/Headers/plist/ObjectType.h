/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_ObjectType_h
#define __plist_ObjectType_h

#include <plist/Base.h>

#include <string>

namespace plist {

/*
 * All possible object types.
 */
enum class ObjectType {
    None,
    Integer,
    Real,
    String,
    Boolean,
    Null,
    Array,
    Dictionary,
    Data,
    Date,
    UID,
};

class ObjectTypes {
private:
    ObjectTypes();
    ~ObjectTypes();

public:
    /*
     * Convert an object type to a user-facing name.
     */
    static std::string Name(ObjectType type);
};

}

#endif  // !__plist_ObjectType_h
