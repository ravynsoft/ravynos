/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/Action.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcscheme::XC::Action;

Action::
Action()
{
}

bool Action::
parse(plist::Dictionary const *dict)
{
    auto BC = dict->value <plist::String> ("buildConfiguration");

    if (BC != nullptr) {
        _buildConfiguration = BC->value();
    }

    if (auto PA = dict->value <plist::Dictionary> ("PreActions")) {
        if (auto EAd = PA->value <plist::Dictionary> ("ExecuteAction")) {
            auto EA = std::make_shared <ExecuteAction> ();
            if (!EA->parse(EAd))
                return false;

            _preActions.push_back(EA);
        } else if (auto EAa = PA->value <plist::Array> ("ExecuteAction")) {
            for (size_t n = 0; n < EAa->count(); n++) {
                auto EAd = EAa->value <plist::Dictionary> (n);
                if (EAd == nullptr)
                    continue;

                auto EA = std::make_shared <ExecuteAction> ();
                if (!EA->parse(EAd))
                    return false;

                _preActions.push_back(EA);
            }
        }
    }

    if (auto PA = dict->value <plist::Dictionary> ("PostActions")) {
        if (auto EAd = PA->value <plist::Dictionary> ("ExecuteAction")) {
            auto EA = std::make_shared <ExecuteAction> ();
            if (!EA->parse(EAd))
                return false;

            _postActions.push_back(EA);
        } else if (auto EAa = PA->value <plist::Array> ("ExecuteAction")) {
            for (size_t n = 0; n < EAa->count(); n++) {
                auto EAd = EAa->value <plist::Dictionary> (n);
                if (EAd == nullptr)
                    continue;

                auto EA = std::make_shared <ExecuteAction> ();
                if (!EA->parse(EAd))
                    return false;

                _postActions.push_back(EA);
            }
        }
    }

    return true;
}
