/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Real.h>
#include <plist/Integer.h>
#include <plist/String.h>

#include <cstdlib>

using plist::Object;
using plist::Real;
using plist::String;

std::unique_ptr<Real> Real::
New(double value)
{
    return std::unique_ptr<Real>(new Real(value));
}

std::unique_ptr<Object> Real::
_copy() const
{
    return plist::static_unique_pointer_cast<Object>(Real::New(value()));
}

std::unique_ptr<Real> Real::
Coerce(Object const *obj)
{
    if (Real const *real = CastTo<Real>(obj)) {
        return real->copy();
    } else if (Integer const *integer = CastTo<Integer>(obj)) {
        return Real::New(static_cast<double>(integer->value()));
    } else if (String const *string = CastTo<String>(obj)) {
        char *end = NULL;
        double real = std::strtod(string->value().c_str(), &end);

        if (end != string->value().c_str()) {
            return Real::New(real);
        }
    }

    return nullptr;
}
