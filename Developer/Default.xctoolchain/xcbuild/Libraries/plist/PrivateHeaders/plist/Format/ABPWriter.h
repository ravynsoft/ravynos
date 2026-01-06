/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_ABPWriter_h
#define __plist_Format_ABPWriter_h

#include <plist/Format/ABPContext.h>
#include <plist/Format/ABPRecordType.h>
#include <plist/Objects.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>

class ABPWriter : public ABPContext {
private:
    std::unordered_map<plist::Object const *, int>                   _references;
    std::unordered_map<plist::Object const *, plist::Object const *> _mappings;
    std::unordered_set<plist::Object const *>                        _written;
    std::unordered_map<plist::Dictionary const *, std::unordered_map<int, plist::String *>> _keyStrings;

public:
    std::vector<uint8_t>                                            *_mutableContents;

public:
    ABPWriter(std::vector<uint8_t> *contents);

public:
    bool open();
    bool finalize();
    bool close();

    bool writeTopLevelObject(plist::Object const *object);

public:
    int write(void const *data, size_t length);

private:
    bool writeByte(uint8_t byte);
    bool writeWord0(size_t nbytes, uint64_t value, bool swap);
    bool writeWord(size_t nbytes, uint64_t value);

private:
    bool writeTypeAndLength(ABPRecordType type, uint64_t length);
    bool writeOffset(uint64_t offset);
    bool writeReference(uint64_t offset);
    bool writeHeader();
    bool writeTrailer(bool replace);
    bool writeOffsetTable();

private:
    bool writeObject(plist::Object const *object, uint32_t flags);
    bool writeNull(plist::Null const *null);
    bool writeBool(plist::Boolean const *boolean);
    bool writeDate(plist::Date const *date);
    bool writeInteger(plist::Integer const *integer);
    bool writeReal(plist::Real const *real);
    bool writeData(plist::Data const *data);
    bool writeUID(plist::UID const *uid);
    bool writeStringASCII(char const *chars, size_t nchars);
    bool writeStringUnicode(uint16_t const *chars, size_t nchars);
    bool writeString(plist::String const *string);
    plist::String const *dictionaryKeyString(plist::Dictionary const *dict, int key);
    void writeArrayReferences(plist::Array const *array);
    void writeArrayValues(plist::Array const *array);
    bool writeArray(plist::Array const *array);
    void writeDictionaryReferences(plist::Dictionary const *dict);
    void writeDictionaryValues(plist::Dictionary const *dict);
    bool writeDictionary(plist::Dictionary const *dict);

private:
    bool writePreflightObject(plist::Object const *object, uint32_t flags);
    void writePreflightArrayReferences(plist::Array const *array);
    bool writePreflightArray(plist::Array const *array);
    void writePreflightDictionaryReferences(plist::Dictionary const *dict);
    bool writePreflightDictionary(plist::Dictionary const *dict);

private:
    bool processObject(plist::Object const **object, uint32_t *refno, bool userProcess);
};

#endif  /* !__plist_Format_ABPWriter_h */
