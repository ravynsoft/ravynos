/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_Any_h
#define __plist_Format_Any_h

#include <plist/Format/Format.h>
#include <plist/Format/Binary.h>
#include <plist/Format/XML.h>
#include <plist/Format/ASCII.h>

namespace plist {
namespace Format {

class Any : public Format<Any> {
private:
    union Contents {
        Contents() { };
        ~Contents() { };

        Binary    binary;
        XML       xml;
        ASCII     ascii;
    };

private:
    Type     _type;
    Contents _contents;

public:
    Any(Type type, Contents const &contents);
    Any(Any const &);
    ~Any();

public:
    inline Type type() const
    { return _type; }

public:
    template<typename T>
    inline T const *format() const
    {
        if (_type == T::FormatType()) {
            return reinterpret_cast<T const *>(&_contents);
        } else {
            return nullptr;
        }
    }

public:
    template<typename T>
    static Any Create(T const &format)
    {
        Contents contents;
        *reinterpret_cast<T *>(&contents) = format;
        return Any(T::FormatType(), contents);
    }
};

}
}

#endif  // !__plist_Format_Any_h
