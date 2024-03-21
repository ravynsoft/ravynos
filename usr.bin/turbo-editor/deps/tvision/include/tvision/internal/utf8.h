#ifndef TVISION_UTF8_H
#define TVISION_UTF8_H

#include <tvision/tv.h>

#include <internal/endian.h>

#include <stdint.h>
#include <string.h>

namespace tvision
{

inline constexpr uint Utf8BytesLeft(char first_byte) noexcept
{
    // https://en.wikipedia.org/wiki/UTF-8
    return (first_byte & 0b11100000) == 0b11000000 ? 1 : \
           (first_byte & 0b11110000) == 0b11100000 ? 2 : \
           (first_byte & 0b11111000) == 0b11110000 ? 3 : 0;
}

inline constexpr uint32_t utf8To32(TStringView s) noexcept
{
    // Precondition: s is a valid UTF-8 sequence.
    switch (s.size()) {
        case 1:
            return s[0];
        case 2:
            return ((s[0] & 0b00011111) << 6)  |  (s[1] & 0b00111111);
        case 3:
            return ((s[0] & 0b00001111) << 12) | ((s[1] & 0b00111111) << 6)  |  (s[2] & 0b00111111);
        case 4:
            return ((s[0] & 0b00001111) << 18) | ((s[1] & 0b00111111) << 12) | ((s[2] & 0b00111111) << 6) | (s[3] & 0b00111111);
    }
    return 0;
}

inline size_t utf32To8(uint32_t u32, char u8[4]) noexcept
{
    size_t length;
    uint32_t asInt;
    if (u32 <= 0x007F)
    {
        asInt = u32;
        length = 1;
    }
    else if (u32 <= 0x07FF)
    {
        asInt =
            (((u32 >> 6)  & 0b00011111) | 0b11000000) |
            (( u32        & 0b00111111) | 0b10000000) << 8;
        length = 2;
    }
    else if (u32 <= 0xFFFF)
    {
        asInt =
            (((u32 >> 12) & 0b00001111) | 0b11100000) |
            (((u32 >> 6)  & 0b00111111) | 0b10000000) << 8 |
            (( u32        & 0b00111111) | 0b10000000) << 16;
        length = 3;
    }
    else
    {
        asInt =
            (((u32 >> 18) & 0b00000111) | 0b11110000) |
            (((u32 >> 12) & 0b00111111) | 0b10000000) << 8 |
            (((u32 >> 6)  & 0b00111111) | 0b10000000) << 16 |
            (( u32        & 0b00111111) | 0b10000000) << 24;
        length = 4;
    }
#ifdef TV_BIG_ENDIAN
    reverseBytes(asInt);
#endif
    memcpy(u8, &asInt, 4);
    return length;
}

inline size_t utf16To8(uint16_t u16[2], char u8[4])
// Pre: 'u16' contains a valid UTF-16 sequence. If the character consists of a
// single code unit, then 'u16[1] == 0'.
{
    uint32_t u32;
    if (u16[1] == 0)
        u32 = u16[0];
    else
        u32 = (u16[0] - 0xD800) << 10 | (u16[1] - 0xDC00) | 0x10000;
    return utf32To8(u32, u8);
}

inline int utf32To16(uint32_t u32, uint16_t u16[2]) noexcept
{
    if (u32 <= 0xFFFF && (u32 < 0xD800 || 0xDFFF < u32))
    {
        u16[0] = uint16_t(u32);
        return 1;
    }
    else if (u32 < 0x10FFFF) // Two surrogates.
    {
        u16[0] = uint16_t(((u32 - 0x10000) >> 10) + 0xD800);
        u16[1] = uint16_t(((u32 - 0x10000) & 0x3FF) + 0xDC00);
        return 2;
    }
    return -1;
}

// Returns the length of the converted text.
// Pre: the capacity of 'output' is enough to hold the result.
size_t utf16To8(TSpan<const uint16_t> input, char *output) noexcept;

} // namespace tvision

#endif // TVISION_UTF8_H
