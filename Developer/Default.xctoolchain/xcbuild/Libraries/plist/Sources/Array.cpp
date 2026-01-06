/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Array.h>

using plist::Object;
using plist::Array;

std::unique_ptr<Array> Array::
New()
{
    return std::unique_ptr<Array>(new Array());
}

std::unique_ptr<Object> Array::
_copy() const
{
    auto result = Array::New();
    for (size_t n = 0; n < count(); n++) {
        result->append(value(n)->copy());
    }
    return plist::static_unique_pointer_cast<Object>(std::move(result));
}

void Array::
merge(Array const *array)
{
    if (array == nullptr || array == this)
        return;

    for (auto const &obj : *array) {
        append(obj->copy());
    }
}

std::unique_ptr<Array> Array::
Coerce(Object const *obj)
{
    if (Array const *array = CastTo<Array>(obj)) {
        return array->copy();
    }

    return nullptr;
}
