/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxsetting_DefaultSettings_h
#define __pbxsetting_DefaultSettings_h

#include <pbxsetting/Level.h>
#include <pbxsetting/Setting.h>

#include <vector>

namespace process { class Context; }
namespace process { class User; }

namespace pbxsetting {

/*
 * Environment-specific default settings. These levels are generally assumed
 * to be at the far back of an `Environment` for setting resolution.
 */
class DefaultSettings {
private:
    DefaultSettings();
    ~DefaultSettings();

public:
    static Level
    Environment(process::User const *user, process::Context const *processContext);
    static Level
    Internal(void);
    static Level
    Local(void);
    static Level
    System(void);
    static Level
    Architecture(void);
    static Level
    Build(void);

public:
    /*
     * All of the default setting levels, in order.
     */
    static std::vector<Level>
    Levels(process::User const *user, process::Context const *processContext);
};

}

#endif  // !__pbxsetting_Default_h
