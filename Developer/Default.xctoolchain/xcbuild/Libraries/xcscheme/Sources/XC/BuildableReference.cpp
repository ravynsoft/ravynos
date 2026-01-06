/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/BuildableReference.h>
#include <xcscheme/SchemeGroup.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcscheme::XC::BuildableReference;
using xcscheme::SchemeGroup;

BuildableReference::
BuildableReference()
{
}

std::string BuildableReference::
resolve(std::shared_ptr<SchemeGroup> const &container) const
{
    std::string referencedContainer = _referencedContainer.empty() ? "" : "/" + _referencedContainer;

    if (_referencedContainerType == "container") {
        return container->basePath() + referencedContainer;
    } else if (_referencedContainerType == "absolute") {
        return referencedContainer;
    } else if (_referencedContainerType == "developer") {
        // TODO(grp): Look in DEVELOPER_DIR.
        return referencedContainer;
    } else {
        fprintf(stderr, "error: unknown container type %s\n", _referencedContainerType.c_str());
        return referencedContainer;
    }
}

bool BuildableReference::
parse(plist::Dictionary const *dict)
{
    auto BlI = dict->value <plist::String> ("BlueprintIdentifier");
    auto BlN = dict->value <plist::String> ("BlueprintName");
    auto BuI = dict->value <plist::String> ("BuildableIdentifier");
    auto BuN = dict->value <plist::String> ("BuildableName");
    auto RC  = dict->value <plist::String> ("ReferencedContainer");

    if (BlI != nullptr) {
        _blueprintIdentifier = BlI->value();
    }

    if (BlN != nullptr) {
        _blueprintName = BlN->value();
    }

    if (BuI != nullptr) {
        _buildableIdentifier = BuI->value();
    }

    if (BuN != nullptr) {
        _buildableName = BuN->value();
    }

    if (RC != nullptr) {
        std::string location = RC->value();
        size_t      colon    = location.find(':');

        _referencedContainerType = location.substr(0, colon);
        _referencedContainer     = location.substr(colon + 1);
    }

    return true;
}
