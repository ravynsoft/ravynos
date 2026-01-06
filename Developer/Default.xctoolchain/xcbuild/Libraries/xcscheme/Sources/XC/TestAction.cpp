/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/TestAction.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcscheme::XC::TestAction;

TestAction::
TestAction() :
    _shouldUseLaunchSchemeArgsEnv(false)
{
}

bool TestAction::
parse(plist::Dictionary const *dict)
{
    if (!Action::parse(dict))
        return false;

    auto SDI    = dict->value <plist::String> ("selectedDebuggerIdentifier");
    auto SLI    = dict->value <plist::String> ("selectedLauncherIdentifier");
    auto SULSAE = dict->value <plist::Boolean> ("shouldUseLaunchSchemeArgsEnv");
    auto TRs    = dict->value <plist::Dictionary> ("Testables");
    auto ME     = dict->value <plist::Dictionary> ("MacroExpansion");

    if (SDI != nullptr) {
        _selectedDebuggerIdentifier = SDI->value();
    }

    if (SLI != nullptr) {
        _selectedLauncherIdentifier = SLI->value();
    }

    if (SULSAE != nullptr) {
        _shouldUseLaunchSchemeArgsEnv = SULSAE->value();
    }

    if (TRs != nullptr) {
        if (auto TRd = TRs->value <plist::Dictionary> ("TestableReference")) {
            auto TR = std::make_shared <TestableReference> ();
            if (!TR->parse(TRd))
                return false;

            _testables.push_back(TR);
        } else if (auto TRa = TRs->value <plist::Array> ("TestableReference")) {
            for (size_t n = 0; n < TRa->count(); n++) {
                auto TRd = TRa->value <plist::Dictionary> (n);
                if (TRd == nullptr)
                    continue;

                auto TR = std::make_shared <TestableReference> ();
                if (!TR->parse(TRd))
                    return false;

                _testables.push_back(TR);
            }
        }
    }

    if (ME != nullptr) {
        auto BR = ME->value <plist::Dictionary> ("BuildableReference");
        if (BR != nullptr) {
            _macroExpansion = std::make_shared <BuildableReference> ();
            if (!_macroExpansion->parse(BR))
                return false;
        }
    }

    return true;
}
