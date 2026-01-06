/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __libutil_Wildcard_h
#define __libutil_Wildcard_h

#include <string>

namespace libutil {

struct Wildcard {
    static bool Match(std::string const &pattern, std::string const &string);
};

}

#endif  // !__libutil_Wildcard_h
