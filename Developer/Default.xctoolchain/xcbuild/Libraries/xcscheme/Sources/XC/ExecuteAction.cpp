/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/ExecuteAction.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcscheme::XC::ExecuteAction;

ExecuteAction::
ExecuteAction()
{
}

bool ExecuteAction::
parse(plist::Dictionary const *dict)
{
    auto AT = dict->value <plist::String> ("ActionType");
    auto AC = dict->value <plist::Dictionary> ("ActionContent");

    if (AT != nullptr) {
        _actionType = AT->value();
    }

    if (AC != nullptr) {
        _actionContent = std::make_shared <ActionContent> ();
        if (!_actionContent->parse(AC))
            return false;
    }

    return true;
}

