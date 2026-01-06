/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcscheme/XC/LaunchAction.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using xcscheme::XC::LaunchAction;

LaunchAction::
LaunchAction() :
    _allowLocationSimulation       (false),
    _debugDocumentVersioning       (false),
    _ignoresPersistentStateOnLaunch(false),
    _useCustomWorkingDirectory     (false),
    _launchStyle                   (0)
{
}

bool LaunchAction::
parse(plist::Dictionary const *dict)
{
    if (!Action::parse(dict))
        return false;

    auto ALS    = dict->value <plist::Boolean> ("allowLocationSimulation");
    auto DDV    = dict->value <plist::Boolean> ("debugDocumentVersioning");
    auto IPSOL  = dict->value <plist::Boolean> ("ignoresPersistentStateOnLaunch");
    auto UCWD   = dict->value <plist::Boolean> ("useCustomWorkingDirectory");
    auto LS     = dict->value <plist::String> ("launchStyle");
    auto SDI    = dict->value <plist::String> ("selectedDebuggerIdentifier");
    auto SLI    = dict->value <plist::String> ("selectedLauncherIdentifier");
    auto BPR    = dict->value <plist::Dictionary> ("BuildableProductRunnable");
    auto CLAs   = dict->value <plist::Dictionary> ("CommandLineArguments");
    auto AOs    = dict->value <plist::Dictionary> ("AdditionalOptions");
    auto LSR    = dict->value <plist::Dictionary> ("LocationScenarioReference");

    if (ALS != nullptr) {
        _allowLocationSimulation = ALS->value();
    }

    if (DDV != nullptr) {
        _debugDocumentVersioning = DDV->value();
    }

    if (IPSOL != nullptr) {
        _ignoresPersistentStateOnLaunch = IPSOL->value();
    }

    if (UCWD != nullptr) {
        _useCustomWorkingDirectory = UCWD->value();
    }

    if (LS != nullptr) {
        _launchStyle = std::stoi(LS->value());
    }

    if (SDI != nullptr) {
        _selectedDebuggerIdentifier = SDI->value();
    }

    if (SLI != nullptr) {
        _selectedLauncherIdentifier = SLI->value();
    }

    if (CLAs != nullptr) {
        if (auto CLAd = CLAs->value <plist::Dictionary> ("CommandLineArgument")) {
            auto CLA = std::make_shared <CommandLineArgument> ();
            if (!CLA->parse(CLAd))
                return false;

            _commandLineArguments.push_back(CLA);
        } else if (auto CLAa = CLAs->value <plist::Array> ("CommandLineArgument")) {
            for (size_t n = 0; n < CLAa->count(); n++) {
                auto CLAd = CLAa->value <plist::Dictionary> (n);
                if (CLAd == nullptr)
                    continue;

                auto CLA = std::make_shared <CommandLineArgument> ();
                if (!CLA->parse(CLAd))
                    return false;

                _commandLineArguments.push_back(CLA);
            }
        }
    }

    if (AOs != nullptr) {
        if (auto AOd = AOs->value <plist::Dictionary> ("AdditionalOption")) {
            auto AO = std::make_shared <AdditionalOption> ();
            if (!AO->parse(AOd))
                return false;

            _additionalOptions.push_back(AO);
        } else if (auto AOa = AOs->value <plist::Array> ("AdditionalOption")) {
            for (size_t n = 0; n < AOa->count(); n++) {
                auto AOd = AOa->value <plist::Dictionary> (n);
                if (AOd == nullptr)
                    continue;

                auto AO = std::make_shared <AdditionalOption> ();
                if (!AO->parse(AOd))
                    return false;

                _additionalOptions.push_back(AO);
            }
        }
    }

    if (BPR != nullptr) {
        auto BR = BPR->value <plist::Dictionary> ("BuildableReference");
        if (BR != nullptr) {
            _buildableProductRunnable = std::make_shared <BuildableReference> ();
            if (!_buildableProductRunnable->parse(BR))
                return false;
        }
    }

    if (LSR != nullptr) {
        _locationSecenarioReference = std::make_shared <LocationScenarioReference> ();
        if (!_locationSecenarioReference->parse(LSR))
            return false;
    }

    return true;
}
