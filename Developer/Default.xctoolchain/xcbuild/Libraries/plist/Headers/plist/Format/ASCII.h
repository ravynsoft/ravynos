/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_ASCII_h
#define __plist_Format_ASCII_h

#include <plist/Format/Format.h>
#include <plist/Format/Type.h>
#include <plist/Format/Encoding.h>

namespace plist {
namespace Format {

class ASCII : public Format<ASCII> {
private:
    bool     _strings;
    Encoding _encoding;

private:
    ASCII(bool strings, Encoding encoding);

public:
    static Type FormatType();

public:
    inline bool strings() const
    { return _strings; }
    inline Encoding encoding() const
    { return _encoding; }

public:
    static ASCII Create(bool strings, Encoding encoding);
};

}
}

#endif  // !__plist_Format_ASCII_h
