/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcscheme_XC_AnalyzeAction_h
#define __xcscheme_XC_AnalyzeAction_h

#include <xcscheme/XC/Action.h>

namespace xcscheme { namespace XC {

class AnalyzeAction : public Action {
public:
    typedef std::shared_ptr <AnalyzeAction> shared_ptr;

public:
    AnalyzeAction();

public:
    bool parse(plist::Dictionary const *dict) override;
};

} }

#endif  // !__xcscheme_XC_AnalyzeAction_h
