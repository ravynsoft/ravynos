/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_ASCIIWriter_h
#define __plist_Format_ASCIIWriter_h

#include <plist/Objects.h>

namespace plist {
namespace Format {

class ASCIIWriter {
private:
    Object const         *_root;
    bool                  _strings;
    std::vector<uint8_t>  _contents;
    int                   _indent;
    bool                  _lastKey;

public:
    ASCIIWriter(Object const *root, bool strings);
    ~ASCIIWriter();

public:
    std::vector<uint8_t> contents() const
    { return _contents; }

public:
    bool write();

private:
    bool primitiveWriteString(std::string const &string);
    bool primitiveWriteEscapedString(std::string const &string);

private:
    bool writeString(std::string const &string, bool final);
    bool writeEscapedString(std::string const &string, bool final);

private:
    bool handleObject(Object const *object, bool root);
    bool handleArray(Array const *array, bool root);
    bool handleDictionary(Dictionary const *dictionary, bool root);
    bool handleBoolean(Boolean const *boolean, bool root);
    bool handleReal(Real const *real, bool root);
    bool handleInteger(Integer const *integer, bool root);
    bool handleString(String const *string, bool root);
    bool handleDate(Date const *date, bool root);
    bool handleData(Data const *data, bool root);
    bool handleUID(UID const *uid, bool root);
};

}
}

#endif  // !__plist_Format_ASCIIWriter_h
