/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Data.h>
#include <plist/Base64.h>

using plist::Object;
using plist::Data;

using plist::Base64;

Data::
Data(std::string const &value)
{
    Base64::Decode(value, _value);
}

std::unique_ptr<Data> Data::
New(std::vector<uint8_t> const &value)
{
    return std::unique_ptr<Data>(new Data(value));
}

std::unique_ptr<Data> Data::
New(std::vector<uint8_t> &&value)
{
    return std::unique_ptr<Data>(new Data(std::move(value)));
}

std::unique_ptr<Data> Data::
New(std::string const &value)
{
    return std::unique_ptr<Data>(new Data(value));
}

std::unique_ptr<Data> Data::
New(void const *bytes, size_t length)
{
    return std::unique_ptr<Data>(new Data(bytes, length));
}

void Data::
setBase64Value(std::string const &value)
{
    Base64::Decode(value, _value);
}

std::string Data::
base64Value() const
{
    return Base64::Encode(_value);
}

std::unique_ptr<Object> Data::
_copy() const
{
    return plist::static_unique_pointer_cast<Object>(Data::New(_value));
}

std::unique_ptr<Data> Data::
Coerce(Object const *obj)
{
    if (Data const *data = CastTo<Data>(obj)) {
        return data->copy();
    }

    return nullptr;
}
