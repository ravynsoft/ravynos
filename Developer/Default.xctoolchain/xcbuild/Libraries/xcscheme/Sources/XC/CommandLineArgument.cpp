/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/CommandLineArgument.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcscheme::XC::CommandLineArgument;

CommandLineArgument::
CommandLineArgument() :
    _isEnabled(false)
{
}

bool CommandLineArgument::
parse(plist::Dictionary const *dict)
{
    auto IE = dict->value <plist::Boolean> ("isEnabled");
    auto A  = dict->value <plist::String> ("argument");

    if (IE != nullptr) {
        _isEnabled = IE->value();
    }

    if (A != nullptr) {
        _argument = A->value();
    }

    return true;
}
