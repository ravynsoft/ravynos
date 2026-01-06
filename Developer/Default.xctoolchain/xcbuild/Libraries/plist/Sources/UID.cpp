/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/UID.h>

using plist::Object;
using plist::UID;

std::unique_ptr<UID> UID::
New(uint32_t value)
{
    return std::unique_ptr<UID>(new UID(value));
}

std::unique_ptr<Object> UID::
_copy() const
{
    return plist::static_unique_pointer_cast<Object>(UID::New(value()));
}

std::unique_ptr<UID> UID::
Coerce(Object const *obj)
{
    if (UID const *integer = CastTo<UID>(obj)) {
        return integer->copy();
    }

    return nullptr;
}
