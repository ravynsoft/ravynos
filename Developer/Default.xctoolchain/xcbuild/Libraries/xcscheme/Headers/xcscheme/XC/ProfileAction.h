/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_ProfileAction_h
#define __xcscheme_XC_ProfileAction_h

#include <xcscheme/XC/Action.h>
#include <xcscheme/XC/BuildableReference.h>

namespace xcscheme { namespace XC {

class ProfileAction : public Action {
public:
    typedef std::shared_ptr <ProfileAction> shared_ptr;

private:
    bool                           _debugDocumentVersioning;
    bool                           _shouldUseLaunchSchemeArgsEnv;
    bool                           _useCustomWorkingDirectory;
    std::string                    _savedToolIdentifier;
    BuildableReference::shared_ptr _buildableProductRunnable;
 
public:
    ProfileAction();

public:
    inline bool debugDocumentVersioning() const
    { return _debugDocumentVersioning; }

public:
    inline bool shouldUseLaunchSchemeArgsEnv() const
    { return _shouldUseLaunchSchemeArgsEnv; }

public:
    inline bool useCustomWorkingDirectory() const
    { return _useCustomWorkingDirectory; }

public:
    inline std::string const &savedToolIdentifier() const
    { return _savedToolIdentifier; }

public:
    inline BuildableReference::shared_ptr const &buildableProductRunnable() const
    { return _buildableProductRunnable; }
    inline BuildableReference::shared_ptr &buildableProductRunnable()
    { return _buildableProductRunnable; }

public:
    bool parse(plist::Dictionary const *dict) override;
};

} }

#endif  // !__xcscheme_XC_ProfileAction_h
