/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcassets/ContentReference.h>
#include <plist/Keys/Unpack.h>
#include <plist/String.h>

using xcassets::ContentReference;

bool ContentReference::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("ContentReference", dict, &seen);

    auto T  = unpack.coerce <plist::String> ("type");
    auto N  = unpack.coerce <plist::String> ("name");
    auto MS = unpack.coerce <plist::String> ("matching-style");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (T != nullptr) {
        _type = T->value();
    }

    if (N != nullptr) {
        _name = FullyQualifiedName::Parse(N->value());
    }

    if (MS != nullptr) {
        _matchingStyle = MatchingStyles::Parse(MS->value());
    }

    return true;
}
