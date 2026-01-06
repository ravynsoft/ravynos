/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/Object.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::Object;
using pbxproj::Context;

Object::
Object(std::string const &isa) :
    _isa(isa)
{
}

bool Object::
parseObject(Context &context, plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    return parse(context, dict, &seen, true);
}

bool Object::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    auto unpack = plist::Keys::Unpack("Object", dict, seen);

    auto I = unpack.cast <plist::String> ("isa");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (I != nullptr) {
        (void)I;
    }

    return true;
}

