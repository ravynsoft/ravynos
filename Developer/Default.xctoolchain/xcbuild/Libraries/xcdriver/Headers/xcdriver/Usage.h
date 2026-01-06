/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcdriver_Usage_h
#define __xcdriver_Usage_h

#include <string>

namespace xcdriver {

class Usage {
private:
    Usage();
    ~Usage();

public:
    /*
     * Text explaining the usage of the driver.
     */
    static std::string Text(std::string const &name);
};

}

#endif // !__xcdriver_Usage_h
