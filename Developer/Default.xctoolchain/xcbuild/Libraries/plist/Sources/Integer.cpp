/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Integer.h>
#include <plist/String.h>

#include <cstdlib>

using plist::Object;
using plist::Integer;
using plist::String;

std::unique_ptr<Integer> Integer::
New(int64_t value)
{
    return std::unique_ptr<Integer>(new Integer(value));
}

std::unique_ptr<Object> Integer::
_copy() const
{
    return plist::static_unique_pointer_cast<Object>(Integer::New(value()));
}

std::unique_ptr<Integer> Integer::
Coerce(Object const *obj)
{
    if (Integer const *integer = CastTo<Integer>(obj)) {
        return integer->copy();
    } else if (String const *string = CastTo<String>(obj)) {
        char *end = NULL;
        long long integer = std::strtoll(string->value().c_str(), &end, 0);

        if (end != string->value().c_str()) {
            return Integer::New(integer);
        }
    }

    return nullptr;
}
