/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __builtin_copyTiff_Driver_h
#define __builtin_copyTiff_Driver_h

#include <builtin/Driver.h>

namespace builtin {
namespace copyTiff {

class Driver : public builtin::Driver {
public:
    Driver();
    ~Driver();

public:
    virtual std::string name();

public:
    virtual int run(process::Context const *processContext, libutil::Filesystem *filesystem);
};

}
}

#endif // !__builtin_copyTiff_Driver_h
