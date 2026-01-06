/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_SimpleXML_h
#define __plist_Format_SimpleXML_h

#include <plist/Format/Format.h>
#include <plist/Format/Type.h>
#include <plist/Format/Encoding.h>

namespace plist {
namespace Format {

class SimpleXML : public Format<SimpleXML> {
private:
    Encoding _encoding;

private:
    SimpleXML(Encoding encoding);

public:
    inline Encoding encoding() const
    { return _encoding; }

public:
    static SimpleXML Create(Encoding encoding);
};

}
}

#endif  // !__plist_Format_SimpleXML_h
