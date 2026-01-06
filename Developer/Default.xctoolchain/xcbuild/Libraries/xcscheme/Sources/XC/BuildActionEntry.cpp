/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/BuildActionEntry.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>

using xcscheme::XC::BuildActionEntry;

BuildActionEntry::
BuildActionEntry() :
    _buildForAnalyzing(false),
    _buildForArchiving(false),
    _buildForProfiling(false),
    _buildForRunning  (false),
    _buildForTesting  (false)
{
}

bool BuildActionEntry::
parse(plist::Dictionary const *dict)
{
    auto BFAn = dict->value <plist::Boolean> ("buildForAnalyzing");
    auto BFAr = dict->value <plist::Boolean> ("buildForArchiving");
    auto BFP  = dict->value <plist::Boolean> ("buildForProfiling");
    auto BFR  = dict->value <plist::Boolean> ("buildForRunning");
    auto BFT  = dict->value <plist::Boolean> ("buildForTesting");
    auto BR   = dict->value <plist::Dictionary> ("BuildableReference");

    if (BFAn != nullptr) {
        _buildForAnalyzing = BFAn->value();
    }

    if (BFAr != nullptr) {
        _buildForArchiving = BFAr->value();
    }

    if (BFP != nullptr) {
        _buildForProfiling = BFP->value();
    }

    if (BFR != nullptr) {
        _buildForRunning = BFR->value();
    }

    if (BFT != nullptr) {
        _buildForTesting = BFT->value();
    }

    if (BR != nullptr) {
        _buildableReference = std::make_shared <BuildableReference> ();
        if (!_buildableReference->parse(BR))
            return false;
    }

    return true;
}
