#ifndef TVISION_STRINGS_H
#define TVISION_STRINGS_H

#include <tvision/tv.h>

namespace tvision
{

template<class Int>
inline constexpr Int string_as_int(TStringView s) noexcept
// CAUTION: It is not endian-safe to reinterpret the result as an array of bytes.
{
    Int res = 0;
    for (size_t i = 0; i < min(s.size(), sizeof(res)); ++i)
        res |= uint64_t(uint8_t(s[i])) << 8*i;
    return res;
}

char *fast_utoa(uint32_t value, char *buffer) noexcept;

template <class T, size_t N>
struct constarray;

struct alignas(4) btoa_lut_elem_t
{
    char chars[3];
    uint8_t digits;
};

using btoa_lut_t = constarray<btoa_lut_elem_t, 256>;

inline char *fast_btoa(uint8_t value, char *buffer) noexcept
// Pre: the capacity of 'buffer' is at least 4 bytes.
{
    extern const btoa_lut_t btoa_lut;
    const auto &lut = (btoa_lut_elem_t (&) [256]) btoa_lut;
    // Optimization: read and write the whole LUT entry at once in order to
    // minimize memory accesses. We can afford to write more bytes into 'buffer'
    // than digits.
    memcpy(buffer, &lut[value], 4);
    return buffer + (uint8_t) buffer[3];
    static_assert(sizeof(btoa_lut_elem_t) == 4, "");
}

} // namespace tvision

#endif // TVISION_STRINGS_H
