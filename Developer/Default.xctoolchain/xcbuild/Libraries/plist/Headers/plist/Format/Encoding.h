/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_Encoding_h
#define __plist_Format_Encoding_h

#include <plist/Base.h>

#include <vector>

namespace plist {
namespace Format {

enum class Encoding {
    UTF8,
    UTF16BE,
    UTF16LE,
    UTF32BE,
    UTF32LE,
};

class Encodings {
private:
    Encodings();
    ~Encodings();

public:
    static Encoding
    Detect(std::vector<uint8_t> const &contents);

public:
    static std::vector<uint8_t>
    Convert(std::vector<uint8_t> const &contents, Encoding from, Encoding to);

public:
    static std::vector<uint8_t>
    BOM(Encoding encoding);
};

}
}

#endif  // !__plist_Format_Encoding_h
