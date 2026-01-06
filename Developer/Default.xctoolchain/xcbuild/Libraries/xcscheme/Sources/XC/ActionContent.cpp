/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/ActionContent.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcscheme::XC::ActionContent;

ActionContent::
ActionContent()
{
}

bool ActionContent::
parse(plist::Dictionary const *dict)
{
    auto T   = dict->value <plist::String> ("title");
    auto ST  = dict->value <plist::String> ("scriptText");
    auto EB  = dict->value <plist::Dictionary> ("EnvironmentBuildable");
    auto ELR = dict->value <plist::String> ("emailRecipient");
    auto ELS = dict->value <plist::String> ("emailSubject");
    auto ELB = dict->value <plist::String> ("emailBody");
    auto ALE = dict->value <plist::Boolean> ("attachLogToEmail");

    if (T != nullptr) {
        _title = T->value();
    }

    if (ST != nullptr) {
        _scriptText = ST->value();
    }

    if (EB != nullptr) {
        auto BR = EB->value <plist::Dictionary> ("BuildableReference");
        if (BR != nullptr) {
            _environmentBuildable = std::make_shared <BuildableReference> ();
            if (!_environmentBuildable->parse(BR))
                return false;
        }
    }

    if (ELR != nullptr) {
        _emailRecipient = ELR->value();
    }

    if (ELS != nullptr) {
        _emailSubject = ELS->value();
    }

    if (ELB != nullptr) {
        _emailBody = ELB->value();
    }

    if (ALE != nullptr) {
        _attachLogToEmail = ALE->value();
    }

    return true;
}
