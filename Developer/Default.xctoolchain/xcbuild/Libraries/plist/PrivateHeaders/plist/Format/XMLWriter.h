/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_XMLWriter_h
#define __plist_Format_XMLWriter_h

#include <plist/Format/BaseXMLParser.h>
#include <plist/Objects.h>

namespace plist {
namespace Format {

class XMLWriter {
private:
    Object const         *_root;
    std::vector<uint8_t>  _contents;
    int                   _indent;

public:
    XMLWriter(Object const *root);
    ~XMLWriter();

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
    bool handleObject(Object const *object);
    bool handleArray(Array const *array);
    bool handleDictionary(Dictionary const *dictionary);
    bool handleBoolean(Boolean const *boolean);
    bool handleReal(Real const *real);
    bool handleInteger(Integer const *integer);
    bool handleString(String const *string);
    bool handleDate(Date const *date);
    bool handleData(Data const *data);
    bool handleUID(UID const *uid);
};

}
}

#endif  // !__plist_Format_XMLWriter_h
