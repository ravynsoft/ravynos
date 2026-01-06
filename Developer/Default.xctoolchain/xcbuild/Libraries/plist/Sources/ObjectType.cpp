/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/ObjectType.h>

using plist::ObjectType;
using plist::ObjectTypes;

std::string ObjectTypes::
Name(ObjectType type)
{
    switch (type) {
        case ObjectType::None:       return "object";
        case ObjectType::Integer:    return "integer";
        case ObjectType::Real:       return "real";
        case ObjectType::String:     return "string";
        case ObjectType::Boolean:    return "boolean";
        case ObjectType::Null:       return "null";
        case ObjectType::Array:      return "array";
        case ObjectType::Dictionary: return "dictionary";
        case ObjectType::Data:       return "data";
        case ObjectType::Date:       return "date";
        default: abort();
    }
}
