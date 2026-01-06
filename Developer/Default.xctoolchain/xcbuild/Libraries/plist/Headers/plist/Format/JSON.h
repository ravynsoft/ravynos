/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_JSON_h
#define __plist_Format_JSON_h

#include <plist/Format/Format.h>
#include <plist/Format/Type.h>
#include <plist/Format/Encoding.h>

namespace plist {
namespace Format {

class JSON : public Format<JSON> {
private:
    JSON();

public:
    static JSON Create();
};

}
}

#endif  // !__plist_Format_JSON_h
