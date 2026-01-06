/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcdriver_HelpAction_h
#define __xcdriver_HelpAction_h

namespace process { class Context; }

namespace xcdriver {

/*
 * Prints usage, as well as explanations of each invocation option.
 */
class HelpAction {
private:
    HelpAction();
    ~HelpAction();

public:
    static int
    Run(process::Context const *processContext);
};

}

#endif // !__xcdriver_HelpAction_h
