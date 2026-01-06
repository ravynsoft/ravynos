/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxspec/PBX/BuildRule.h>
#include <pbxspec/Context.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxspec::PBX::BuildRule;

BuildRule::
BuildRule()
{
}

BuildRule::
BuildRule(std::vector<std::string> const &fileTypes, std::string const &compilerSpec) :
    _fileTypes(fileTypes),
    _compilerSpec(compilerSpec)
{
}

bool BuildRule::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("BuildRule", dict, &seen);

    auto N  = unpack.cast <plist::String> ("Name");
    auto FT = unpack.cast <plist::String> ("FileType");
    auto CS = unpack.cast <plist::String> ("CompilerSpec");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (N != nullptr) {
        _name = N->value();
    }

    if (FT != nullptr) {
        _fileTypes = std::vector<std::string>({ FT->value() });
    }

    if (CS != nullptr) {
        _compilerSpec = CS->value();
    }

    return true;
}
