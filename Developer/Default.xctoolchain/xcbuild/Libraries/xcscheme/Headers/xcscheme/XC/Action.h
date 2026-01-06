/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_Action_h
#define __xcscheme_XC_Action_h

#include <xcscheme/XC/ExecuteAction.h>

#include <string>

namespace plist { class Dictionary; }

namespace xcscheme { namespace XC {

class Action {
private:
    std::string           _buildConfiguration;
    ExecuteAction::vector _preActions;
    ExecuteAction::vector _postActions;

public:
    Action();

public:
    inline std::string const &buildConfiguration() const
    { return _buildConfiguration; }

public:
    inline ExecuteAction::vector const &preActions() const
    { return _preActions; }
    inline ExecuteAction::vector &preActions()
    { return _preActions; }

public:
    inline ExecuteAction::vector const &postActions() const
    { return _postActions; }
    inline ExecuteAction::vector &postActions()
    { return _postActions; }

protected:
    virtual bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcscheme_XC_Action_h
