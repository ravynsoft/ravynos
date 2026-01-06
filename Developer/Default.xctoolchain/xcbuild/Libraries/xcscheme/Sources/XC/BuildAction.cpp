/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/BuildAction.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>

using xcscheme::XC::BuildAction;

BuildAction::
BuildAction() :
    _buildImplicitDependencies(false),
    _parallelizeBuildables    (false)
{
}

bool BuildAction::
parse(plist::Dictionary const *dict)
{
    if (!Action::parse(dict))
        return false;

    auto BID = dict->value <plist::Boolean> ("buildImplicitDependencies");
    auto PB  = dict->value <plist::Boolean> ("parallelizeBuildables");

    if (BID != nullptr) {
        _buildImplicitDependencies = BID->value();
    }

    if (PB != nullptr) {
        _parallelizeBuildables = PB->value();
    }

    if (auto BAEs = dict->value <plist::Dictionary> ("BuildActionEntries")) {
        if (auto BAEd = BAEs->value <plist::Dictionary> ("BuildActionEntry")) {
            auto BAE = std::make_shared <BuildActionEntry> ();
            if (!BAE->parse(BAEd))
                return false;

            _buildActionEntries.push_back(BAE);
        } else if (auto BAEa = BAEs->value <plist::Array> ("BuildActionEntry")) {
            for (size_t n = 0; n < BAEa->count(); n++) {
                auto BAEd = BAEa->value <plist::Dictionary> (n);
                if (BAEd == nullptr)
                    continue;

                auto BAE = std::make_shared <BuildActionEntry> ();
                if (!BAE->parse(BAEd))
                    return false;

                _buildActionEntries.push_back(BAE);
            }
        }
    }

    return true;
}
