/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_ExecuteAction_h
#define __xcscheme_XC_ExecuteAction_h

#include <xcscheme/XC/ActionContent.h>

#include <memory>
#include <string>
#include <vector>

namespace plist { class Dictionary; }

namespace xcscheme { namespace XC {

class ExecuteAction {
public:
    typedef std::shared_ptr <ExecuteAction> shared_ptr;
    typedef std::vector <shared_ptr> vector;

private:
    std::string               _actionType;
    ActionContent::shared_ptr _actionContent;

public:
    ExecuteAction();

public:
    inline std::string const &actionType()
    { return _actionType; }

public:
    inline ActionContent::shared_ptr const &actionContent() const
    { return _actionContent; }
    inline ActionContent::shared_ptr &actionContent()
    { return _actionContent; }

public:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcscheme_XC_ExecuteAction_h
