/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_LaunchAction_h
#define __xcscheme_XC_LaunchAction_h

#ifdef __linux__
#include <cstdint>
#endif
#include <xcscheme/XC/Action.h>
#include <xcscheme/XC/BuildableReference.h>
#include <xcscheme/XC/LocationScenarioReference.h>
#include <xcscheme/XC/CommandLineArgument.h>
#include <xcscheme/XC/AdditionalOption.h>

namespace xcscheme { namespace XC {

class LaunchAction : public Action {
public:
    typedef std::shared_ptr <LaunchAction> shared_ptr;

private:
    bool                                  _allowLocationSimulation;
    bool                                  _debugDocumentVersioning;
    bool                                  _ignoresPersistentStateOnLaunch;
    bool                                  _useCustomWorkingDirectory;
    uint32_t                              _launchStyle;
    std::string                           _selectedDebuggerIdentifier;
    std::string                           _selectedLauncherIdentifier;
    BuildableReference::shared_ptr        _buildableProductRunnable;
    LocationScenarioReference::shared_ptr _locationSecenarioReference;
    CommandLineArgument::vector           _commandLineArguments;
    AdditionalOption::vector              _additionalOptions;

public:
    LaunchAction();

public:
    inline bool allowLocationSimulation() const
    { return _allowLocationSimulation; }

public:
    inline bool debugDocumentVersioning() const
    { return _debugDocumentVersioning; }

public:
    inline bool ignoresPersistentStateOnLaunch() const
    { return _ignoresPersistentStateOnLaunch; }

public:
    inline bool useCustomWorkingDirectory() const
    { return _useCustomWorkingDirectory; }

public:
    inline uint32_t launchStyle() const
    { return _launchStyle; }

public:
    inline std::string const &selectedDebuggerIdentifier() const
    { return _selectedDebuggerIdentifier; }

    inline std::string const &selectedLauncherIdentifier() const
    { return _selectedLauncherIdentifier; }

public:
    inline BuildableReference::shared_ptr const &buildableProductRunnable() const
    { return _buildableProductRunnable; }
    inline BuildableReference::shared_ptr &buildableProductRunnable()
    { return _buildableProductRunnable; }

public:
    inline LocationScenarioReference::shared_ptr const &locationScenarioReference() const
    { return _locationSecenarioReference; }
    inline LocationScenarioReference::shared_ptr &locationScenarioReference()
    { return _locationSecenarioReference; }

public:
    inline CommandLineArgument::vector const &commandLineArguments() const
    { return _commandLineArguments; }
    inline CommandLineArgument::vector &commandLineArguments()
    { return _commandLineArguments; }

public:
    inline AdditionalOption::vector const &additionalOptions() const
    { return _additionalOptions; }
    inline AdditionalOption::vector &additionalOptions()
    { return _additionalOptions; }

public:
    bool parse(plist::Dictionary const *dict) override;
};

} }

#endif  // !__xcscheme_XC_LaunchAction_h
