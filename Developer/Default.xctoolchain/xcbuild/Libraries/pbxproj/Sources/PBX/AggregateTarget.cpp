/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/AggregateTarget.h>
#include <pbxproj/PBX/BuildPhases.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::AggregateTarget;
using pbxproj::Context;

AggregateTarget::
AggregateTarget() :
    Target(Isa(), Type::Aggregate)
{
}

bool AggregateTarget::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Target::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("AggregateTarget", dict, seen);

    auto PN  = unpack.cast <plist::String> ("productName");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (PN != nullptr) {
        _productName = PN->value();
    }

    return true;
}
