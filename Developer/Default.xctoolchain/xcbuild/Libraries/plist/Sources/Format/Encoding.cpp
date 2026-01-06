/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/Encoding.h>
#include <plist/Format/unicode.h>

#include <cassert>

#if defined(__linux__)
#include <endian.h>
#endif

using plist::Format::Encoding;
using plist::Format::Encodings;

Encoding Encodings::
Detect(std::vector<uint8_t> const &contents)
{
    /*
     * Check for a UTF-32 BOM. First as bytes overlap with UTF-16 LE.
     */
    if (contents.size() >= 4) {
        std::vector<uint8_t> UTF32BE_BOM = Encodings::BOM(Encoding::UTF32BE);
        if (std::equal(UTF32BE_BOM.begin(), UTF32BE_BOM.end(), contents.begin())) {
            return Encoding::UTF32BE;
        }

        std::vector<uint8_t> UTF32LE_BOM = Encodings::BOM(Encoding::UTF32LE);
        if (std::equal(UTF32LE_BOM.begin(), UTF32LE_BOM.end(), contents.begin())) {
            return Encoding::UTF32LE;
        }
    }

    /*
     * Check for a UTF-16 BOM.
     */
    if (contents.size() >= 2) {
        std::vector<uint8_t> UTF16BE_BOM = Encodings::BOM(Encoding::UTF16BE);
        if (std::equal(UTF16BE_BOM.begin(), UTF16BE_BOM.end(), contents.begin())) {
            return Encoding::UTF16BE;
        }

        std::vector<uint8_t> UTF16LE_BOM = Encodings::BOM(Encoding::UTF16LE);
        if (std::equal(UTF16LE_BOM.begin(), UTF16LE_BOM.end(), contents.begin())) {
            return Encoding::UTF16LE;
        }
    }

    /* Any other encoding is assumed to be UTF-8. */
    return Encoding::UTF8;
}

std::vector<uint8_t> Encodings::
BOM(Encoding encoding)
{
    switch (encoding) {
        case Encoding::UTF8:
            return { 0xEF, 0xBB, 0xBF };
        case Encoding::UTF16BE:
            return { 0xFE, 0xFF };
        case Encoding::UTF16LE:
            return { 0xFF, 0xFE };
        case Encoding::UTF32BE:
            return { 0x00, 0x00, 0xFE, 0xFF };
        case Encoding::UTF32LE:
            return { 0xFF, 0xFE, 0x00, 0x00 };
    }

    abort();
}

enum class Endian {
    Big,
    Little,
};

#if defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN)
static Endian const HostEndian = Endian::Big;
#elif defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN) || _WIN32
static Endian const HostEndian = Endian::Little;
#else
#error Unknown endianness.
#endif

static Endian
EncodingEndian(Encoding encoding)
{
    switch (encoding) {
        case Encoding::UTF8:
            return HostEndian;
        case Encoding::UTF32LE:
        case Encoding::UTF16LE:
            return Endian::Little;
        case Encoding::UTF32BE:
        case Encoding::UTF16BE:
            return Endian::Big;
    }

    abort();
}

template<typename T>
static T EndianSwap(T value);

template<>
uint16_t EndianSwap<uint16_t>(uint16_t value)
{
    return (value >> 8) | (value << 8);
}

template<>
uint32_t EndianSwap<uint32_t>(uint32_t value)
{
    return static_cast<uint32_t>(EndianSwap<uint16_t>(value) << 16) | EndianSwap<uint16_t>(value >> 16);
}

template<typename T>
static void
EndianSwapBuffer(std::vector<uint8_t> *buffer, Endian src, Endian dest)
{
    if (src != dest) {
        for (size_t i = 0; i < buffer->size(); i += sizeof(T)) {
            T *value = reinterpret_cast<T *>(&(*buffer)[i]);
            *value = EndianSwap<T>(*value);
        }
    }
}

std::vector<uint8_t> Encodings::
Convert(std::vector<uint8_t> const &contents, Encoding from, Encoding to)
{
    std::vector<uint8_t> input = contents;

    /* Remove any BOM at the start. */
    std::vector<uint8_t> BOM = Encodings::BOM(from);
    if (input.size() >= BOM.size() && std::equal(BOM.begin(), BOM.end(), contents.begin())) {
        input.erase(input.begin(), input.begin() + BOM.size());
    }

    /* No conversion needed, just byte swap if necessary. */
    if (from == to) {
        return input;
    } else if ((from == Encoding::UTF16LE && to == Encoding::UTF16BE) ||
               (from == Encoding::UTF16BE && to == Encoding::UTF16LE)) {
        EndianSwapBuffer<uint16_t>(&input, EncodingEndian(from), EncodingEndian(to));
        return input;
    } else if ((from == Encoding::UTF32LE && to == Encoding::UTF32BE) ||
               (from == Encoding::UTF32BE && to == Encoding::UTF32LE)) {
        EndianSwapBuffer<uint32_t>(&input, EncodingEndian(from), EncodingEndian(to));
        return input;
    }

    /*
     * Convert to UTF-8 as an intermediate format.
     */
    std::vector<uint8_t> intermediate;
    if (from == Encoding::UTF8) {
        intermediate = input;
    } else {
        if (from == Encoding::UTF16LE || from == Encoding::UTF16BE) {
            EndianSwapBuffer<uint16_t>(&input, EncodingEndian(from), HostEndian);

            intermediate.resize(input.size() * sizeof(uint8_t) * 2);
            size_t length = ::utf16_to_utf8(
                reinterpret_cast<char *>(intermediate.data()), intermediate.size() / sizeof(char),
                reinterpret_cast<uint16_t *>(input.data()), input.size() / sizeof(uint16_t),
                0, nullptr);
            intermediate.resize(length);
        } else if (from == Encoding::UTF32LE || from == Encoding::UTF32BE) {
            EndianSwapBuffer<uint32_t>(&input, EncodingEndian(from), HostEndian);

            intermediate.resize(input.size() * sizeof(uint8_t));
            size_t length = ::utf32_to_utf8(
                reinterpret_cast<char *>(intermediate.data()), intermediate.size() / sizeof(char),
                reinterpret_cast<uint32_t *>(input.data()), input.size() / sizeof(uint32_t),
                0, nullptr);
            intermediate.resize(length);
        } else {
            assert(false && "unknown encoding");
        }
    }

    /*
     * Convert to the resulting format.
     */
    if (to == Encoding::UTF8) {
        return intermediate;
    } else {
        std::vector<uint8_t> result;

        if (to == Encoding::UTF16LE || to == Encoding::UTF16BE) {
            result.resize(contents.size() * sizeof(uint16_t) * 3);
            size_t length = ::utf8_to_utf16(
                reinterpret_cast<uint16_t *>(result.data()), result.size() / sizeof(uint16_t),
                reinterpret_cast<char *>(intermediate.data()), intermediate.size() / sizeof(char),
                0, nullptr);
            result.resize(length * sizeof(uint16_t));

            EndianSwapBuffer<uint16_t>(&result, HostEndian, EncodingEndian(to));
        } else if (to == Encoding::UTF32LE || to == Encoding::UTF32BE) {
            result.resize(intermediate.size() * sizeof(uint32_t));
            size_t length = ::utf8_to_utf32(
                reinterpret_cast<uint32_t *>(result.data()), result.size() / sizeof(uint32_t),
                reinterpret_cast<char *>(intermediate.data()), intermediate.size() / sizeof(char),
                0, nullptr);
            result.resize(length * sizeof(uint32_t));

            EndianSwapBuffer<uint32_t>(&result, HostEndian, EncodingEndian(to));
        } else {
            assert(false && "unknown encoding");
        }

        return result;
    }
}
