/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <libutil/Strings.h>
#include <plist/Boolean.h>
#include <plist/String.h>
#include <plist/Integer.h>
#include <plist/Real.h>

using plist::Object;
using plist::Boolean;
using plist::Integer;
using plist::Real;
using plist::String;

std::unique_ptr<Boolean> Boolean::
New(bool value)
{
    return std::unique_ptr<Boolean>(new Boolean(value));
}

bool Boolean::
equals(Object const *obj) const
{
    if (Object::equals(obj))
        return true;

    Boolean const *objt = CastTo <Boolean> (obj);
    return (objt != nullptr && equals(objt));
}

bool Boolean::
equals(Boolean const *obj) const
{
    return (obj != nullptr && (obj == this || value() == obj->value()));
}

std::unique_ptr<Object> Boolean::
_copy() const
{
    return plist::static_unique_pointer_cast<Object>(Boolean::New(value()));
}

std::unique_ptr<Boolean> Boolean::
Coerce(Object const *obj)
{
    if (Boolean const *boolean = CastTo<Boolean>(obj)) {
        return boolean->copy();
    } else if (String const *string = CastTo<String>(obj)) {
        bool value = (
            libutil::strcasecmp(string->value().c_str(), "yes") == 0 ||
            libutil::strcasecmp(string->value().c_str(), "true") == 0 ||
            string->value() == "1");
        return Boolean::New(value);
    } else if (Integer const *integer = CastTo<Integer>(obj)) {
        return Boolean::New(integer->value() != 0);
    } else if (Real const *real = CastTo<Real>(obj)) {
        return Boolean::New(real->value() != 0.0);
    }

    return nullptr;
}
