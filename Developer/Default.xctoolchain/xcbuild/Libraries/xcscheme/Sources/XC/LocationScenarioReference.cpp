/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/LocationScenarioReference.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcscheme::XC::LocationScenarioReference;

LocationScenarioReference::
LocationScenarioReference() :
    _referenceType(0)
{
}

bool LocationScenarioReference::
parse(plist::Dictionary const *dict)
{
    auto I  = dict->value <plist::String> ("identifier");
    auto RT = dict->value <plist::String> ("referenceType");

    if (I != nullptr) {
        _identifier = I->value();
    }

    if (RT != nullptr) {
        _referenceType = std::stoi(RT->value());
    }

    return true;
}
