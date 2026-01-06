/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/Test.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcscheme::XC::Test;

Test::
Test()
{
}

bool Test::
parse(plist::Dictionary const *dict)
{
    auto I = dict->value <plist::String> ("Identifier");

    if (I != nullptr) {
        _identifier = I->value();
    }

    return true;
}
