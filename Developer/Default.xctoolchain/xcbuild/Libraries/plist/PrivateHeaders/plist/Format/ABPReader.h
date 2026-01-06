/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_ABPReader_h
#define __plist_Format_ABPReader_h

#include <plist/Format/ABPContext.h>
#include <plist/Objects.h>

#include <string>
#include <unordered_set>

class ABPReader : public ABPContext {
public:
    plist::Object                       **_objects;
    std::unordered_set<plist::Object *>   _seen;
    std::string                           _error;

public:
    ABPReader(std::vector<uint8_t> const *contents);
    ~ABPReader();

public:
    bool open();
    bool close();

    plist::Object *readTopLevelObject();
    plist::Object *readObject(uint64_t reference);

public:
    std::string const &error() const
    { return _error; }

private:
    void error(std::string const &error);

private:
    int read(void *data, size_t length);
    int readByte();
    bool readWord(size_t nbytes, uint64_t *result);
    bool readLength(size_t *nitems);
    bool readOffset(uint64_t *offset);

private:
    bool readHeader();
    bool readTrailer();
    bool readOffsetTable();
    bool readReference(uint64_t *offset);

private:
    plist::Object *_readObject();
    plist::Date *readDate();
    plist::Integer *readInteger(size_t nbytes);
    plist::Real *readReal(size_t nbytes);
    plist::Data *readData(size_t nbytes);
    plist::UID *readUid(size_t nbytes);
    plist::String *readStringASCII(size_t nchars);
    plist::String *readStringUnicode(size_t nchars);
    plist::Array *readArray(size_t nitems);
    plist::Dictionary *readDictionary(size_t nitems);
    plist::Null *readNull();
    plist::Boolean *readBool(bool value);
};

#endif  /* !__plist_Format_ABPReader_h */
