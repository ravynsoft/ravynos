/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_TestAction_h
#define __xcscheme_XC_TestAction_h

#include <xcscheme/XC/Action.h>
#include <xcscheme/XC/TestableReference.h>

#include <string>

namespace plist { class Dictionary; }

namespace xcscheme { namespace XC {

class TestAction : public Action {
public:
    typedef std::shared_ptr <TestAction> shared_ptr;

private:
    std::string                    _selectedDebuggerIdentifier;
    std::string                    _selectedLauncherIdentifier;
    bool                           _shouldUseLaunchSchemeArgsEnv;
    TestableReference::vector      _testables;
    BuildableReference::shared_ptr _macroExpansion;

public:
    TestAction();

public:
    inline std::string const &selectedDebuggerIdentifier() const
    { return _selectedDebuggerIdentifier; }

    inline std::string const &selectedLauncherIdentifier() const
    { return _selectedLauncherIdentifier; }

public:
    inline bool shouldUseLaunchSchemeArgsEnv() const
    { return _shouldUseLaunchSchemeArgsEnv; }

public:
    inline TestableReference::vector const &testables() const
    { return _testables; }
    inline TestableReference::vector &testables()
    { return _testables; }

public:
    inline BuildableReference::shared_ptr const &macroExpansion() const
    { return _macroExpansion; }
    inline BuildableReference::shared_ptr &macroExpansion()
    { return _macroExpansion; }

public:
    bool parse(plist::Dictionary const *dict) override;
};

} }

#endif  // !__xcscheme_XC_TestAction_h
