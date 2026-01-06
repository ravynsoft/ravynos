/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxspec_Context_h
#define __pbxspec_Context_h

#include <pbxspec/Manager.h>

namespace pbxspec {

class Context {
public:
    std::string domain;
};

}

using pbxspec::Context;

#endif  // !__pbxspec_Context_h
