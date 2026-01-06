/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/ABPReader.h>
#include <plist/Format/ABPRecordType.h>
#include <plist/Format/Encoding.h>

#include <cassert>

#if 0
#define dprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define dprintf(...)
#endif

void ABPReader::
error(std::string const &error)
{
    this->_error = error;
}

bool ABPReader::
readOffsetTable()
{
    off_t        offset;
    uint64_t     n, *offsets;
    plist::Object **objects;

    offset = -static_cast<off_t>(sizeof(this->_trailer) + this->_trailer.offsetIntByteSize * this->_trailer.objectsCount);

    if (this->seek(offset, SEEK_END) < 0)
        return false;

    size_t objectsCount = static_cast<size_t>(this->_trailer.objectsCount);
    offsets = new uint64_t[objectsCount];
    objects = new plist::Object *[objectsCount];

    for (n = 0; n < this->_trailer.objectsCount; n++) {
        objects[n] = nullptr;
        if (!this->readOffset(offsets + n)) {
            delete[] objects;
            delete[] offsets;
            return false;
        }

#if 1
        dprintf("Object #%-5llu: %llx%s\n",
                (unsigned long long)n,
                (unsigned long long)offsets[n],
                (n == this->_trailer.topLevelObject)
                ? " [TOP LEVEL OBJECT]" : "");
#endif
    }

    this->_offsets = offsets;
    this->_objects = objects;

    return true;
}

plist::Date *ABPReader::
readDate()
{
    uint64_t date;
    if (!this->readWord(8, &date)) {
        this->error("EOF reading date value");
        return NULL;
    }

    /* Reference time is 2001/1/1 */
    static uint64_t const ReferenceTimestamp = 978307200;
    return plist::Date::New(date - ReferenceTimestamp).release();
}

plist::Integer *ABPReader::
readInteger(size_t nbytes)
{
    uint64_t value;

    if (!this->readWord(nbytes, &value)) {
        this->error("EOF reading integer value");
        return NULL;
    }

    dprintf("int(%zu) - %llu\n", nbytes,
            (unsigned long long)value);

    return plist::Integer::New(value).release();
}

plist::Real *ABPReader::
readReal(size_t nbytes)
{
    uint64_t value;

    if (!this->readWord(nbytes, &value)) {
        this->error("EOF reading real value");
        return NULL;
    }

    dprintf("real(%zu) - %g\n", nbytes,
            *(double *)&value);

    switch (nbytes) {
        case 4: {
            float converted;
            memcpy(&converted, &value, sizeof(converted));
            return plist::Real::New(converted).release();
        }
        case 8: {
            double converted;
            memcpy(&converted, &value, sizeof(converted));
            return plist::Real::New(converted).release();
        }
        default: return plist::Real::New(0.0).release();
    }
}

plist::Data *ABPReader::
readData(size_t nbytes)
{
    if (!this->readLength(&nbytes)) {
        this->error("EOF reading data length value");
        return NULL;
    }

    dprintf("data - %zu bytes\n", nbytes);

    std::vector<uint8_t> bytes;

    if (nbytes > 0) {
        bytes.resize(nbytes);
        size_t nread = this->read(bytes.data(), nbytes);
        if (nread < nbytes) {
            return nullptr;
        }
    }

    return plist::Data::New(std::move(bytes)).release();
}

plist::UID *ABPReader::
readUid(size_t nbytes)
{
    uint64_t value = 0;

    if (!this->readLength(&nbytes))
        return NULL;

    dprintf("uid - %lu bytes", nbytes);

    if (nbytes > 4) {
        this->error("too many bytes in UID");
        return NULL;
    }

    if (nbytes > 0 && !this->readWord(nbytes, &value)) {
        this->error("error reading word for UID");
        return NULL;
    }

    return plist::UID::New(static_cast<uint32_t>(value)).release();
}

plist::String *ABPReader::
readStringASCII(size_t nchars)
{
    if (!this->readLength(&nchars)) {
        this->error("EOF reading ASCII string length value");
        return NULL;
    }

    dprintf("ascii string - %zu(0x%zx) chars\n", nchars, nchars);

    std::string string;

    if (nchars > 0) {
        size_t nbytes = sizeof(char) * nchars;
        string.resize(nbytes);
        size_t nread = this->read(&string[0], nbytes);
        if (nread < nbytes) {
            return nullptr;
        }
    }

    return plist::String::New(std::move(string)).release();
}

plist::String *ABPReader::
readStringUnicode(size_t nchars)
{
    if (!this->readLength(&nchars)) {
        this->error("EOF reading Unicode string length value");
        return NULL;
    }

    dprintf("unicode string - %zu chars\n", nchars);

    std::vector<uint8_t> buffer;

    if (nchars > 0) {
        size_t nbytes = sizeof(uint16_t) * nchars;
        buffer.resize(nbytes);
        size_t nread = this->read(buffer.data(), nbytes);
        if (nread < nbytes) {
            return nullptr;
        }
    }

    buffer = plist::Format::Encodings::Convert(buffer, plist::Format::Encoding::UTF16BE, plist::Format::Encoding::UTF8);

    std::string string = std::string(buffer.begin(), buffer.end());
    return plist::String::New(std::move(string)).release();
}

plist::Array *ABPReader::
readArray(size_t nitems)
{
    std::unique_ptr<plist::Array> array;

    if (!this->readLength(&nitems)) {
        this->error("EOF reading array count value");
        return NULL;
    }

    dprintf("array - %zu items\n", nitems);

    auto objrefs = new uint64_t[nitems];
    for (size_t n = 0; n < nitems; n++) {
        uint64_t objref;
        if (!this->readReference(&objref)) {
            this->error("corrupted array's object references table");
            goto fail;
        }

        dprintf("\titem #%zu: obj ref %llu\n", n,
                (unsigned long long)objref);

        objrefs[n] = objref;
    }

    array = plist::Array::New();
    for (size_t n = 0; n < nitems; n++) {
        uint64_t objref = objrefs[n];
        auto object = this->readObject(objref);
        if (object == nullptr) {
            goto fail;
        }

        //
        // Due to the memory layout of the `plist' objects,
        // we must always copy objects.
        //
        if (this->_seen.find(object) != this->_seen.end()) {
            this->_seen.insert(object);
        }
        object = object->copy().release();

        array->append(std::unique_ptr<plist::Object>(object));
    }

fail:
    delete[] objrefs;
    return array.release();
}

plist::Dictionary *ABPReader::
readDictionary(size_t nitems)
{
    std::unique_ptr<plist::Dictionary> dict;

    if (!this->readLength(&nitems)) {
        this->error("EOF reading dictionary count value");
        return NULL;
    }

    uint64_t *kvrefs = new uint64_t[nitems * 2];
    dprintf("dictionary - %zu items\n", nitems);

    for (size_t n = 0; n < nitems; n++) {
        uint64_t keyref;

        if (!this->readReference(&keyref)) {
            this->error("corrupted dictionary's key references table");
            goto fail;
        }

        kvrefs[n * 2 + 0] = keyref;
    }

    for (size_t n = 0; n < nitems; n++) {
        uint64_t objref;

        if (!this->readReference(&objref)) {
            this->error("corrupted dictionary's object references table");
            goto fail;
        }

        kvrefs[n * 2 + 1] = objref;
    }

    dict = plist::Dictionary::New();
    for (size_t n = 0; n < nitems; n++) {
        auto keyObject = this->readObject(kvrefs[n * 2 + 0]);
        if (keyObject == nullptr) {
            goto fail;
        }

        auto object = this->readObject(kvrefs[n * 2 + 1]);
        if (object == nullptr) {
            goto fail;
        }

        //
        // Key must be of string type.
        //
        auto keyString = plist::CastTo<plist::String>(keyObject);
        if (keyString == nullptr) {
            goto fail;
        }

        //
        // Due to the memory layout of the `plist' objects,
        // we must always copy objects.
        //
        if (this->_seen.find(object) != this->_seen.end()) {
            this->_seen.insert(object);
        }
        object = object->copy().release();

        dict->set(keyString->value(), std::unique_ptr<plist::Object>(object));
    }

fail:
    delete[] kvrefs;
    return dict.release();
}

plist::Null *ABPReader::
readNull()
{
    return plist::Null::New().release();
}

plist::Boolean *ABPReader::
readBool(bool value)
{
    return plist::Boolean::New(value).release();
}

plist::Object *ABPReader::
_readObject()
{
    int byte;

    for (;;) {
        byte = this->readByte();
        if (byte == EOF) {
            return NULL;
        }

        switch (__ABPByteToRecordType(byte)) {
            case kABPRecordTypeNull:
                return this->readNull();
            case kABPRecordTypeBoolFalse:
                return this->readBool(false);
            case kABPRecordTypeBoolTrue:
                return this->readBool(true);
            case kABPRecordTypeFill:
                break;
            case kABPRecordTypeDate:
                return this->readDate();
            case kABPRecordTypeInteger:
                return this->readInteger(1 << (byte & 0x0f));
            case kABPRecordTypeReal:
                return this->readReal(1 << (byte & 0x0f));
            case kABPRecordTypeData:
                return this->readData(byte & 0x0f);
            case kABPRecordTypeStringASCII:
                return this->readStringASCII(byte & 0x0f);
            case kABPRecordTypeStringUnicode:
                return this->readStringUnicode(byte & 0x0f);
            case kABPRecordTypeUid:
                return this->readUid((byte & 0x0f) + 1);
            case kABPRecordTypeArray:
                return this->readArray(byte & 0x0f);
            case kABPRecordTypeDictionary:
                return this->readDictionary(byte & 0x0f);
            default:
                this->error("unsupported type id");
                return NULL;
        }
    }
}

ABPReader::
ABPReader(std::vector<uint8_t> const *contents) :
    ABPContext(contents),
    _objects (nullptr)
{
}

ABPReader::
~ABPReader()
{
    if (this->_objects != NULL) {
        for (size_t n = 0; n < this->_trailer.objectsCount; n++) {
            if (this->_objects[n] != NULL) {
                this->_objects[n]->release();
            }
        }
        delete[] this->_objects;
    }
}

bool ABPReader::
open()
{
    if ((this->_flags & (kABPContextOpened | kABPContextComplete)) != 0)
        return false;

    if (!this->readHeader()) {
        this->error("not a binary property list or corrupted header");
        return false;
    }

    if (!this->readTrailer()) {
        this->error("corrupted trailer");
        return false;
    }

    if (!this->readOffsetTable()) {
        this->error("corrupted offsets table");
        return false;
    }

    this->_flags |= kABPContextOpened;

    return true;
}

bool ABPReader::
close()
{
    /* Fail if not opened. */
    if ((this->_flags & kABPContextOpened) == 0)
        return false;

    return true;
}

plist::Object *ABPReader::
readObject(uint64_t reference)
{
    plist::Object *object;

    /* Fail if complete, or not opened. */
    if ((this->_flags & kABPContextComplete) != 0 ||
        (this->_flags & kABPContextOpened) == 0)
        return NULL;

    if (reference >= this->_trailer.objectsCount) {
        this->error("reference out of range");
        return NULL;
    }

    object = this->_objects[reference];
    if (object == NULL) {
        if (this->seek(static_cast<size_t>(this->_offsets[reference]), SEEK_SET) < 0) {
            this->error("object reference's offset out of range");
            return NULL;
        }

        object = this->_readObject();
        if (object == NULL) {
            this->error("failed to create object");
            return NULL;
        }

        this->_objects[reference] = object;
    }

    return object;
}

plist::Object *ABPReader::
readTopLevelObject()
{
    return this->readObject(this->_trailer.topLevelObject);
}

int ABPReader::
read(void *data, size_t length)
{
    /* Adjust size for remaining contents. */
    size_t remaining = this->_contents->size() - this->_offset;
    if (remaining < length) {
        length = remaining;
    }

    /* Copy into read buffer. */
    ::memcpy(data, this->_contents->data() + this->_offset, length);

    this->_offset += length;
    return length;
}

int ABPReader::
readByte()
{
    uint8_t byte;

    if (this->read(&byte, sizeof(byte)) != sizeof(byte))
        return EOF;

    return byte;
}

bool ABPReader::
readWord(size_t nbytes, uint64_t *result)
{
    uint8_t  *p, bytes[8];
    uint64_t  v;

    if (this->read(bytes, nbytes) != nbytes)
        return false;

    p = bytes, v = 0;
    switch (nbytes) {
        default: return false;
        case 8: v |= (uint64_t)*p++ << 56;
        case 7: v |= (uint64_t)*p++ << 48;
        case 6: v |= (uint64_t)*p++ << 40;
        case 5: v |= (uint64_t)*p++ << 32;
        case 4: v |= (uint64_t)*p++ << 24;
        case 3: v |= (uint64_t)*p++ << 16;
        case 2: v |= (uint64_t)*p++ << 8;
        case 1: v |= (uint64_t)*p++;
        case 0: break;
    }

    *result = v;
    return true;
}


bool ABPReader::
readLength(size_t *nitems)
{
    if (*nitems == 0x0f) {
        int marker = this->readByte();
        if (marker == EOF)
            return false;

        if ((marker & 0xf0) == 0x10) {
            uint64_t value;

            if (!this->readWord(1 << (marker & 0x0f), &value))
                return false;

            *nitems = static_cast<size_t>(value);
        } else {
            return false;
        }
    }

    return true;
}

bool ABPReader::
readOffset(uint64_t *offset)
{
    return this->readWord(this->_trailer.offsetIntByteSize, offset);
}

bool ABPReader::
readReference(uint64_t *offset)
{
    return this->readWord(this->_trailer.objectRefByteSize, offset);
}

bool ABPReader::
readHeader()
{
    if (this->seek(0, SEEK_SET) == EOF)
        return false;

    if (this->read(&this->_header, sizeof(this->_header)) != sizeof(this->_header))
        return false;

    return (memcmp(this->_header.magic, ABPLIST_MAGIC, sizeof(this->_header.magic)) == 0 &&
            memcmp(this->_header.version, ABPLIST_VERSION, sizeof(this->_header.version)) == 0);
}

bool ABPReader::
readTrailer()
{
    if (this->seek(-static_cast<off_t>(sizeof(this->_trailer)), SEEK_END) == EOF)
        return false;

    if (this->read(this->_trailer.__filler, 8) != 8)
        return false;
    if (!this->readWord(8, &this->_trailer.objectsCount))
        return false;
    if (!this->readWord(8, &this->_trailer.topLevelObject))
        return false;
    if (!this->readWord(8, &this->_trailer.offsetTableEndOffset))
        return false;

    return true;
}
