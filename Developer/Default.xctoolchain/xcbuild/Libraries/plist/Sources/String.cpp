/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/String.h>
#include <plist/Boolean.h>
#include <plist/Date.h>
#include <plist/Real.h>
#include <plist/Integer.h>

#include <sstream>
#include <iomanip>

using plist::Object;
using plist::String;
using plist::Boolean;
using plist::Integer;
using plist::Real;

std::unique_ptr<String> String::
New(std::string const &value)
{
    return std::unique_ptr<String>(new String(value));
}

std::unique_ptr<String> String::
New(std::string &&value)
{
    return std::unique_ptr<String>(new String(std::move(value)));
}

std::unique_ptr<Object> String::
_copy() const
{
    return plist::static_unique_pointer_cast<Object>(String::New(value()));
}

std::unique_ptr<String> String::
Coerce(Object const *obj)
{
    if (String const *string = CastTo<String>(obj)) {
        return string->copy();
    } else if (Real const *real = CastTo<Real>(obj)) {
        std::ostringstream out;
        out << real->value();
        return String::New(out.str());
    } else if (Integer const *integer = CastTo<Integer>(obj)) {
        return String::New(std::to_string(integer->value()));
    } else if (Boolean const *boolean = CastTo<Boolean>(obj)) {
        return String::New(boolean->value() ? "YES" : "NO");
    } else if (Date const *date = CastTo<Date>(obj)) {
        return String::New(date->stringValue());
    }

    return nullptr;
}
