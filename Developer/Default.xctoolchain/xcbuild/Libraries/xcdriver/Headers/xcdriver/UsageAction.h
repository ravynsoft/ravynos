/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcdriver_UsageAction_h
#define __xcdriver_UsageAction_h

namespace process { class Context; }

namespace xcdriver {

/*
 * Prints usage details for correct invocation.
 */
class UsageAction {
private:
    UsageAction();
    ~UsageAction();

public:
    static int
    Run(process::Context const *processContext);
};

}

#endif // !__xcdriver_UsageAction_h
