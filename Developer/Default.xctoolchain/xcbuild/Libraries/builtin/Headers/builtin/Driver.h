/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __builtin_Driver_h
#define __builtin_Driver_h

#include <string>
#include <vector>
#include <unordered_map>

namespace libutil { class Filesystem; }
namespace process { class Context; }

namespace builtin {

class Driver {
protected:
    Driver();
    ~Driver();

public:
    virtual std::string name() = 0;

public:
    virtual int run(process::Context const *processContext, libutil::Filesystem *filesystem) = 0;
};

}

#endif // !__builtin_Driver_h
