/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/ABPWriter.h>
#include <plist/Format/Encoding.h>
#include <plist/Objects.h>

#include <cassert>

using plist::Format::Encoding;
using plist::Format::Encodings;
using plist::ObjectType;
using plist::Object;
using plist::Boolean;
using plist::Real;
using plist::Integer;
using plist::String;
using plist::Data;
using plist::Date;
using plist::Null;
using plist::UID;
using plist::Array;
using plist::Dictionary;

enum {
    kABPWriteObjectTopLevel  = (1 << 0), /* Object is top level. */
    kABPWriteObjectReference = (1 << 1), /* Write only the reference, even if not written yet. */
    kABPWriteObjectValue     = (1 << 2)  /* Write only the value if not yet written. */
};

bool ABPWriter::
writeOffsetTable()
{
    uint64_t n;
    uint64_t highestOffset;

    /* Update offset in trailer. */
    this->_trailer.offsetTableEndOffset = this->tell();

    /* Highest offset is offset table offset */
    highestOffset = this->_trailer.offsetTableEndOffset;

    /* Estimate offset integer size. */
    if (highestOffset > UINT32_MAX) {
        this->_trailer.offsetIntByteSize = sizeof(uint64_t);
    } else if (highestOffset > UINT16_MAX) {
        this->_trailer.offsetIntByteSize = sizeof(uint32_t);
    } else if (highestOffset > UINT8_MAX) {
        this->_trailer.offsetIntByteSize = sizeof(uint16_t);
    } else {
        this->_trailer.offsetIntByteSize = sizeof(uint8_t);
    }

    /* Write out the offsets. */
    for (n = 0; n < this->_trailer.objectsCount; n++) {
        if (!this->writeOffset(this->_offsets[n])) {
            return false;
        }
    }

    return true;
}

bool ABPWriter::
writeNull(Null const *null)
{
    return this->writeByte(__ABPRecordTypeToByte(kABPRecordTypeNull, 0));
}

bool ABPWriter::
writeBool(Boolean const *boolean)
{
    return this->writeByte(__ABPRecordTypeToByte(
                boolean->value() ? kABPRecordTypeBoolTrue :
                kABPRecordTypeBoolFalse, 0));
}

bool ABPWriter::
writeDate(Date const *date)
{
    /* Reference time is 2001/1/1 */
    static uint64_t const ReferenceTimestamp = 978307200;
    double at;
    uint64_t value;

    /* Write object type. */
    if (!this->writeByte(__ABPRecordTypeToByte(kABPRecordTypeDate, 0)))
        return false;

    at = static_cast<double>(date->unixTimeValue() - ReferenceTimestamp);
    /* HACK(strager): We should not rely on C's representation of double. */
    memcpy(&value, &at, 8);
    return this->writeWord(8, value);
}

bool ABPWriter::
writeInteger(Integer const *integer)
{
    int     nbits = 3;
    int64_t value = integer->value();

    if ((value & 0xFF) == value) {
        nbits = 0;
    } else if ((value & 0xFFFF) == value) {
        nbits = 1;
    } else if ((value & 0xFFFFFFFF) == value) {
        nbits = 2;
    }

    /* Write object type. */
    if (!this->writeByte(__ABPRecordTypeToByte(kABPRecordTypeInteger, nbits)))
        return false;

    /* Write word. */
    return this->writeWord(1 << nbits, value);
}

bool ABPWriter::
writeReal(Real const *real)
{
    int         nbits;
    uint64_t    uvalue;
    double      value = real->value();
    float       value32 = static_cast<float>(value);

    if (static_cast<double>(value32) == value) {
        /* HACK(strager): We should not rely on C's representation of float. */
        memcpy(&uvalue, &value32, 4);
        nbits = 2;
    } else {
        /* HACK(strager): We should not rely on C's representation of double. */
        memcpy(&uvalue, &value, 8);
        nbits = 3;
    }

    /* Write object type. */
    if (!this->writeByte(__ABPRecordTypeToByte(kABPRecordTypeReal, nbits)))
        return false;

    return this->writeWord(1 << nbits, uvalue);
}

bool ABPWriter::
writeData(Data const *data)
{
    size_t length;

    /* Write the object type and the length. */
    length = data->value().size();
    if (!this->writeTypeAndLength(kABPRecordTypeData, length))
        return false;

    /* Write contents. */
    return (this->write(data->value().data(), length) == length);
}

bool ABPWriter::
writeUID(UID const *uid)
{
    uint32_t value;
    size_t   nbytes = sizeof(uint32_t);

    value = uid->value();
    if ((value & 0xFF) == value) {
        nbytes = 1;
    } else if ((value & 0xFFFF) == value) {
        nbytes = 2;
    }

    /* Write the object type and the length. */
    if (!this->writeTypeAndLength(kABPRecordTypeUid, nbytes))
        return false;

    /* Write word. */
    return this->writeWord(nbytes, value);
}

bool ABPWriter::
writeStringASCII(char const *chars, size_t nchars)
{
    /* Write the object type and the length. */
    if (!this->writeTypeAndLength(kABPRecordTypeStringASCII, nchars))
        return false;

    return (nchars == 0 || this->write(chars, nchars) == nchars);
}

bool ABPWriter::
writeStringUnicode(uint16_t const *chars, size_t nchars)
{
    /* Write the object type and the length. */
    if (!this->writeTypeAndLength(kABPRecordTypeStringUnicode, nchars))
        return false;

    if (nchars == 0)
        return true;

    return (this->write(chars, nchars * sizeof(uint16_t)) == static_cast<size_t>(nchars * sizeof(uint16_t)));
}

bool ABPWriter::
writeString(String const *string)
{
    bool success;

    bool ascii = true;
    for (uint8_t c : string->value()) {
        if (c > 0x80) {
            ascii = false;
            break;
        }
    }

    if (ascii) {
        success = this->writeStringASCII(string->value().c_str(), string->value().size());
    } else {
        std::vector<uint8_t> buffer = std::vector<uint8_t>(string->value().begin(), string->value().end());
        buffer = Encodings::Convert(buffer, Encoding::UTF8, Encoding::UTF16BE);
        success = this->writeStringUnicode(reinterpret_cast<uint16_t *>(buffer.data()), buffer.size() / sizeof(uint16_t));
    }

    return success;
}

void ABPWriter::
writeArrayReferences(Array const *array)
{
    for (size_t i = 0; i < array->count(); ++i) {
        this->writeObject(array->value(i), kABPWriteObjectReference);
    }
}

void ABPWriter::
writeArrayValues(Array const *array)
{
    for (size_t i = 0; i < array->count(); ++i) {
        this->writeObject(array->value(i), kABPWriteObjectValue);
    }
}

bool ABPWriter::
writeArray(Array const *array)
{
    int length = array->count();

    /* Write the object type and the length. */
    if (!this->writeTypeAndLength(kABPRecordTypeArray, length))
        return false;

    if (length > 0) {
        /* Write all the references and all the values. */
        this->writeArrayReferences(array);
        this->writeArrayValues(array);
    }

    return true;
}

/*
 * Workaround for Dictionaries not having keys with identity. Map from integer key
 * index to key value to use as the identity when writing the key object out.
 */
String const *ABPWriter::
dictionaryKeyString(Dictionary const *dict, int key)
{
    std::unordered_map<int, String *> *map = &this->_keyStrings[dict];

    auto it = map->find(key);
    if (it != map->end()) {
        return it->second;
    } else {
        auto string = String::New(dict->key(key));
        map->insert({ key, string.get() });
        return string.release();
    }
}

void ABPWriter::
writeDictionaryReferences(Dictionary const *dict)
{
    for (size_t i = 0; i < dict->count(); ++i) {
        String const *key = this->dictionaryKeyString(dict, i);
        this->writeObject(key, kABPWriteObjectReference);
    }

    for (size_t i = 0; i < dict->count(); ++i) {
        this->writeObject(dict->value(i), kABPWriteObjectReference);
    }
}

void ABPWriter::
writeDictionaryValues(Dictionary const *dict)
{
    for (size_t i = 0; i < dict->count(); ++i) {
        String const *key = this->dictionaryKeyString(dict, i);
        this->writeObject(key, kABPWriteObjectValue);
    }

    for (size_t i = 0; i < dict->count(); ++i) {
        this->writeObject(dict->value(i), kABPWriteObjectValue);
    }
}

bool ABPWriter::
writeDictionary(Dictionary const *dict)
{
    int length = dict->count();

    /* Write the object type and the length. */
    if (!this->writeTypeAndLength(kABPRecordTypeDictionary, length))
        return false;

    if (length > 0) {
        /* Write all the reference and value pairs. */
        this->writeDictionaryReferences(dict);
        this->writeDictionaryValues(dict);
    }

    return true;
}

/* Preflighting */

void ABPWriter::
writePreflightArrayReferences(Array const *array)
{
    for (size_t i = 0; i < array->count(); ++i) {
        this->writePreflightObject(array->value(i), 0);
    }
}

bool ABPWriter::
writePreflightArray(Array const *array)
{
    /* Preflight all the values. */
    this->writePreflightArrayReferences(array);
    return true;
}

void ABPWriter::
writePreflightDictionaryReferences(Dictionary const *dict)
{
    for (size_t i = 0; i < dict->count(); ++i) {
        String const *key = this->dictionaryKeyString(dict, i);
        this->writePreflightObject(key, 0);
    }

    for (size_t i = 0; i < dict->count(); ++i) {
        this->writePreflightObject(dict->value(i), 0);
    }
}

bool ABPWriter::
writePreflightDictionary(Dictionary const *dict)
{
    /* Preflight all the keys and values. */
    this->writePreflightDictionaryReferences(dict);
    return true;
}

/*
 * Process an object, calls the user callback in order to
 * return a suitable object for the encoding; the object
 * is added to the mapping table if different from the original
 * and a reference is associated with the object; valid references
 * are always non-zero.
 * If 'userProcess' is set, when an object is cached, the object
 * field is set to null and only the reference number is returned.
 */
bool ABPWriter::
processObject(Object const **object, uint32_t *refno, bool userProcess)
{
    Object const *newObject;
    Object const *origObject = *object;

    /* Is this object already mapped? */
    auto mit = this->_mappings.find(origObject);
    if (mit != this->_mappings.end()) {
        newObject = mit->second;
    } else {
        newObject = origObject;

        /* Process the object for mapping. */
        if (userProcess) {
            ObjectType type = newObject->type();
            /*
             * Cache mapping, but do so only if newObject is different
             * than the origObject or origObject is not a container.
             */
            if (newObject != origObject || (type != Array::Type() &&
                                            type != Dictionary::Type() &&
                                            type != Null::Type() &&
                                            type != Boolean::Type())) {
                this->_mappings.insert({ origObject, newObject });
            }
        } else {
            /*
             * The user callback can't handle this object, supposedly because
             * we can.
             */
        }
    }

    assert(newObject != NULL);

    /* Is this new object already written? */
    auto it = this->_references.find(newObject);
    if (it != this->_references.end()) {
        *refno = it->second;

        /*
         * Yes, invalidate newObject so the caller knows that this is
         * a reference.
         */
        if (userProcess) {
            newObject = NULL;
        }
    } else {
        /*
         * No, add a new reference for this newObject.
         */
        *refno = this->_references.size();
        this->_references.insert({ newObject, *refno });
    }

    /* Return the new object to the user. */
    *object = newObject;

    return true;
}

/*
 * Preflight objects so that we emit them ordered; this is needed in
 * order to create binary property lists compatible with standard format;
 * failing to do so will result in an a property list that cannot be
 * opened by any tool (although you can simply use our own which are
 * more than happy to process a "broken" plist).
 */
bool ABPWriter::
writePreflightObject(Object const *object, uint32_t flags)
{
    bool success = true;
    uint32_t refno = 0;

    /*
     * Process the object; if returned object is non-null, it
     * is the first time the object has been seen, otherwise
     * only refno is valid.
     */
    if (!this->processObject(&object, &refno, true))
        return false;

    if (object == NULL)
        return true;

    /* If this is the top level object, store reference in the header. */
    if (flags & kABPWriteObjectTopLevel) {
        this->_trailer.topLevelObject = refno;
    }

    /* Add this object to the offsets table. */
    if (refno >= this->_trailer.objectsCount) {
        this->_trailer.objectsCount = refno + 1;
    }

    /* Preflight dictionaries and arrays. */
    if (auto array = plist::CastTo<Array>(object)) {
        success = this->writePreflightArray(array);
    } else if (auto dict = plist::CastTo<Dictionary>(object)) {
        success = this->writePreflightDictionary(dict);
    }

    if (!success) abort();
    return success;
}

bool ABPWriter::
writeObject(Object const *object, uint32_t flags)
{
    off_t        offset;
    bool         success;
    uint32_t     refno = 0;

    /*
     * Process the object; if returned object is non-null, it
     * is the first time the object has been seen, otherwise
     * only refno is valid.
     */
    if (!this->processObject(&object, &refno, false))
        return false;

    if (this->_written.find(object) != this->_written.end()) {
        if (flags & kABPWriteObjectValue)
            return true;

        flags |= kABPWriteObjectReference;
    }

    if (flags & kABPWriteObjectReference) {
        return this->writeReference(refno);
    }

    /* Cache written object. */
    this->_written.insert(object);

    offset = this->tell();

    if (auto array = plist::CastTo<Array>(object)) {
        success = this->writeArray(array);
    } else if (auto dict = plist::CastTo<Dictionary>(object)) {
        success = this->writeDictionary(dict);
    } else if (auto integer = plist::CastTo<Integer>(object)) {
        success = this->writeInteger(integer);
    } else if (auto real = plist::CastTo<Real>(object)) {
        success = this->writeReal(real);
    } else if (auto string = plist::CastTo<String>(object)) {
        success = this->writeString(string);
    } else if (auto boolean = plist::CastTo<Boolean>(object)) {
        success = this->writeBool(boolean);
    } else if (auto null = plist::CastTo<Null>(object)) {
        success = this->writeNull(null);
    } else if (auto data = plist::CastTo<Data>(object)) {
        success = this->writeData(data);
    } else if (auto date = plist::CastTo<Date>(object)) {
        success = this->writeDate(date);
    } else if (auto uid = plist::CastTo<UID>(object)) {
        success = this->writeUID(uid);
    } else {
        abort();
    }

    if (success) {
        /* Update offsets table. */
        this->_offsets[refno] = offset;
    }

    return success;
}

/*
 * Public Writer API
 */

ABPWriter::
ABPWriter(std::vector<uint8_t> *contents) :
    ABPContext      (contents),
    _mutableContents(contents)
{
}

bool ABPWriter::
open()
{
    if (this->_flags & (kABPContextOpened | kABPContextComplete))
        return false;

    /* Initialize the header struct and write it. */
    memset(&this->_header, 0, sizeof(this->_header));
    memcpy(this->_header.magic, ABPLIST_MAGIC, sizeof(this->_header.magic));
    memcpy(this->_header.version, ABPLIST_VERSION, sizeof(this->_header.version));

    if (!this->writeHeader()) {
        return false;
    }

    /* Initialize the trailer struct. */
    memset(&this->_trailer, 0, sizeof(this->_trailer));

    this->_flags |= kABPContextOpened;

    return true;
}

bool ABPWriter::
finalize()
{
    /* Fail if not opened or not complete. */
    if ((this->_flags & (kABPContextOpened | kABPContextComplete)) != (kABPContextOpened | kABPContextComplete))
        return false;

    /* If we finalized already, do nothing. */
    if (this->_flags & kABPContextFlushed)
        return true;

    /* Write offset table and trailer. */
    if (!this->writeOffsetTable())
        return false;

    if (!this->writeTrailer(false))
        return false;

    this->_flags |= kABPContextFlushed;
    return true;
}

bool ABPWriter::
close()
{
    /* Fail if not opened. */
    if ((this->_flags & kABPContextOpened) == 0)
        return false;

    if ((this->_flags & kABPContextFlushed) == 0) {
        if (!this->finalize())
            return false;
    }

    for (auto const &map : this->_keyStrings) {
        for (auto const &item : map.second) {
            item.second->release();
        }
    }

    return true;
}

bool ABPWriter::
writeTopLevelObject(Object const *object)
{
    bool success;

    if (object == NULL)
        return false;

    /* Fail if complete or not opened. */
    if ((this->_flags & kABPContextComplete) != 0 || (this->_flags & kABPContextOpened) == 0)
        return false;

    /*
     * Preflight objects... the standard seems to support only binary plists
     * whose offsets are increasing *sigh*
     */
    success = this->writePreflightObject(object, kABPWriteObjectTopLevel);
    if (success) {
        /* Estimate the object reference size. */
        uint64_t nrefs = this->_references.size();
        if (nrefs > UINT32_MAX) {
            this->_trailer.objectRefByteSize = sizeof(uint64_t);
        } else if (nrefs > UINT16_MAX) {
            this->_trailer.objectRefByteSize = sizeof(uint32_t);
        } else if (nrefs > UINT8_MAX) {
            this->_trailer.objectRefByteSize = sizeof(uint16_t);
        } else {
            this->_trailer.objectRefByteSize = sizeof(uint8_t);
        }

        /* Allocate enough space for the offsets table. */
        this->_offsets = static_cast<uint64_t *>(calloc(static_cast<size_t>(this->_trailer.objectsCount), sizeof(uint64_t)));
        if (this->_offsets == NULL)
            return false;

        /* Write all the objects. */
        success = this->writeObject(object, kABPWriteObjectTopLevel);
        if (success) {
            this->_flags |= kABPContextComplete;
        }
    }

    return success;
}

int ABPWriter::
write(void const *data, size_t length)
{
    int needed = (this->_mutableContents->size() - this->_offset + length);
    if (needed > 0) {
        this->_mutableContents->resize(this->_mutableContents->size() + needed);
    }

    /* Copy into write buffer. */
    ::memcpy(this->_mutableContents->data() + this->_offset, data, length);

    this->_offset += length;
    return length;
}

bool ABPWriter::
writeByte(uint8_t byte)
{
    return (this->write(&byte, sizeof(byte)) == sizeof(byte));
}

bool ABPWriter::
writeWord0(size_t nbytes, uint64_t value, bool swap)
{
    size_t   n;
    uint8_t *p, bytes[8];

    assert(nbytes <= 8);
    memset(bytes, 0, sizeof(bytes));
    p = bytes;

    for (n = 0; n < nbytes; n++) {
        size_t m = swap ? n : (nbytes - n - 1);
        *p++ = static_cast<uint8_t>(value >> (m << 3));
    }

    return (this->write(bytes, nbytes) == nbytes);
}

bool ABPWriter::
writeWord(size_t nbytes, uint64_t value)
{
    return this->writeWord0(nbytes, value, false);
}

bool ABPWriter::
writeTypeAndLength(ABPRecordType type, uint64_t length)
{
    uint8_t marker       = 0;
    uint8_t directLength = 0;
    int     nbits        = 3;

    assert(length >= 0);
    if (length >= 0x0f) {
        /* Prepare marker byte. */
        if ((length & 0xFF) == length) {
            nbits = 0;
        } else if ((length & 0xFFFF) == length) {
            nbits = 1;
        } else if ((length & 0xFFFFFFFF) == length) {
            nbits = 2;
        }

        marker = 0x10 | nbits;
        directLength = 0xf;
    } else {
        directLength = static_cast<uint8_t>(length);
    }

    /* Write the object type and direct length. */
    if (!this->writeByte(__ABPRecordTypeToByte(type, directLength)))
        return false;

    /* Write the marker and complete length if needed. */
    if (marker != 0) {
        if (!this->writeByte(marker))
            return false;

        if (!this->writeWord(1 << nbits, length))
            return false;
    }

    return true;
}

bool ABPWriter::
writeOffset(uint64_t offset)
{
    return this->writeWord(this->_trailer.offsetIntByteSize, offset);
}

bool ABPWriter::
writeReference(uint64_t offset)
{
    return this->writeWord(this->_trailer.objectRefByteSize, offset);
}

bool ABPWriter::
writeHeader()
{
    if (this->seek(0, SEEK_SET) == EOF)
        return false;

    return (this->write(&this->_header, sizeof(this->_header)) == sizeof(this->_header));
}

bool ABPWriter::
writeTrailer(bool replace)
{
    if (this->seek(replace ? -static_cast<off_t>(sizeof(this->_trailer)) : 0, SEEK_END) == EOF)
        return false;

    if (this->write(this->_trailer.__filler, 8) != 8)
        return false;
    if (!this->writeWord(8, this->_trailer.objectsCount))
        return false;
    if (!this->writeWord(8, this->_trailer.topLevelObject))
        return false;
    if (!this->writeWord(8, this->_trailer.offsetTableEndOffset))
        return false;

    return true;
}
