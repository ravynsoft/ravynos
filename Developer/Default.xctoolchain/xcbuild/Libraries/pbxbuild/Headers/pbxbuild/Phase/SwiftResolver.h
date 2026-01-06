/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_Phase_SwiftResolver_h
#define __pbxbuild_Phase_SwiftResolver_h

#include <pbxbuild/Base.h>

namespace pbxbuild {
namespace Phase {

class Environment;
class Context;

/*
 * Copies the Swift standard library into the product, if needed.
 */
class SwiftResolver {
public:
    explicit SwiftResolver();

public:
    bool resolve(Phase::Environment const &phaseEnvironment, Phase::Context *phaseContext) const;
};

}
}

#endif // !__pbxbuild_Phase_SwiftResolver_h
