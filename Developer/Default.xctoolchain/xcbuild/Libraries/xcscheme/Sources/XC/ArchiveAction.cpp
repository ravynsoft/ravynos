/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/ArchiveAction.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>

using xcscheme::XC::ArchiveAction;

ArchiveAction::
ArchiveAction() :
    _revealArchiveInOrganizer(false)
{
}

bool ArchiveAction::
parse(plist::Dictionary const *dict)
{
    if (!Action::parse(dict))
        return false;

    auto RAIO = dict->value <plist::Boolean> ("revealArchiverInOrganize");

    if (RAIO != nullptr) {
        _revealArchiveInOrganizer = RAIO->value();
    }

    return true;
}
