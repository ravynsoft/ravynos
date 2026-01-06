/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Null.h>

using plist::Object;
using plist::Null;

std::unique_ptr<Null> Null::
New()
{
    return std::unique_ptr<Null>(new Null());
}

std::unique_ptr<Object> Null::
_copy() const
{
    return plist::static_unique_pointer_cast<Object>(Null::New());
}

std::unique_ptr<Null> Null::
Coerce(Object const *obj)
{
    if (Null const *null = CastTo<Null>(obj)) {
        return null->copy();
    }

    return nullptr;
}
