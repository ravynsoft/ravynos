/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Date.h>
#include <plist/ISODate.h>
#include <plist/UnixTime.h>

using plist::Object;
using plist::Date;

using plist::ISODate;
using plist::UnixTime;

Date::
Date(std::string const &value)
{
    ISODate::Decode(value, _value);
}

Date::
Date(uint64_t value)
{
    UnixTime::Decode(value, _value);
}

std::unique_ptr<Date> Date::
New(struct tm const &value)
{
    return std::unique_ptr<Date>(new Date(value));
}

std::unique_ptr<Date> Date::
New(std::string const &value)
{
    return std::unique_ptr<Date>(new Date(value));
}

std::unique_ptr<Date> Date::
New(uint64_t value)
{
    return std::unique_ptr<Date>(new Date(value));
}

void Date::
setStringValue(std::string const &value)
{
    ISODate::Decode(value, _value);
}

std::string Date::
stringValue() const
{
    return ISODate::Encode(_value);
}

void Date::
setUnixTimeValue(uint64_t value)
{
    UnixTime::Decode(value, _value);
}

uint64_t Date::
unixTimeValue() const
{
    return UnixTime::Encode(_value);
}

std::unique_ptr<Object> Date::
_copy() const
{
    return plist::static_unique_pointer_cast<Object>(Date::New(_value));
}

std::unique_ptr<Date> Date::
Coerce(Object const *obj)
{
    if (Date const *date = CastTo<Date>(obj)) {
        return date->copy();
    }

    return nullptr;
}
