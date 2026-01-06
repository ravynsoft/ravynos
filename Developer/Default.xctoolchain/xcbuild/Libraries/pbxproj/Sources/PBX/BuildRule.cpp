/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/BuildRule.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::BuildRule;
using pbxproj::Context;

BuildRule::BuildRule() :
    Object(Isa())
{
}

bool BuildRule::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!Object::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("BuildRule", dict, seen);

    auto CS = unpack.cast <plist::String> ("compilerSpec");
    auto FP = unpack.cast <plist::String> ("filePatterns");
    auto FT = unpack.cast <plist::String> ("fileType");
    auto IE = unpack.coerce <plist::Boolean> ("isEditable");
    auto OF = unpack.cast <plist::Array> ("outputFiles");
    auto S  = unpack.cast <plist::String> ("script");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (CS != nullptr) {
        _compilerSpec = CS->value();
    }

    if (FP != nullptr) {
        _filePatterns = FP->value();
    }

    if (FT != nullptr) {
        _fileType = FT->value();
    }

    if (IE != nullptr) {
        _isEditable = IE->value();
    }

    if (OF != nullptr) {
        for (size_t n = 0; n < OF->count(); n++) {
            auto FN = OF->value <plist::String> (n);
            if (FN != nullptr) {
                _outputFiles.push_back(FN->value());
            }
        }
    }

    if (S != nullptr) {
        _script = S->value();
    }

    return true;
}
