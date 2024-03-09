#include <tvision/tv.h>
#include <internal/constarr.h>
#include <internal/strings.h>

namespace tvision
{

static constexpr
size_t _fast_utoa(uint32_t value, char *buffer) noexcept
{
    // Copyright(c) 2014-2016 Milo Yip (https://github.com/miloyip/itoa-benchmark)
    size_t digits =
        value < 10          ? 1
      : value < 100         ? 2
      : value < 1000        ? 3
      : value < 10000       ? 4
      : value < 100000UL    ? 5
      : value < 1000000UL   ? 6
      : value < 10000000UL  ? 7
      : value < 100000000UL ? 8
      : value < 1000000000UL? 9
                            : 10;
    buffer += digits;

    do {
        *--buffer = char(value % 10) + '0';
        value /= 10;
    } while (value > 0);

    return digits;
}

char *fast_utoa(uint32_t value, char *buffer) noexcept
{
    return buffer + _fast_utoa(value, buffer);
}

// Several versions of GCC crash when generating the table below at compile time.
#if !defined(__GNUC__ ) || __GNUC__ >= 9 || (__GNUC__ == 5 && __GNUC_MINOR__ <= 3)
#define BTOA_CONSTEXPR constexpr
#define BTOA_CONSTEXPR_VAR constexpr
#else
#define BTOA_CONSTEXPR
#define BTOA_CONSTEXPR_VAR const
#endif

static BTOA_CONSTEXPR
btoa_lut_t init_btoa_lut() noexcept
{
    btoa_lut_t res {};
    for (uint32_t i = 0; i < 256; ++i)
        res[i].digits = _fast_utoa(i, res[i].chars);
    return res;
}

extern BTOA_CONSTEXPR_VAR
btoa_lut_t btoa_lut = init_btoa_lut();

} // namespace tvision

#ifndef _MSC_VER

#include <strings.h>

int stricmp( const char *s1, const char *s2 ) noexcept
{
    return strcasecmp(s1, s2);
}

int strnicmp( const char *s1, const char *s2, size_t maxlen ) noexcept
{
    return strncasecmp(s1, s2, maxlen);
}

#include <cctype>

char *strupr(char *s) noexcept
{
    char* p = s;
    while ((*p = toupper(*p)))
        p++;
    return s;
}

// Quick and dirty implementation of itoa, ltoa, ultoa based on sprintf.
// It won't provide the expected results in some cases, but at least will not
// crash. Support for arbitrary bases can be later added if needed.

#include <cstdio>

static inline char printfFmt(int radix)
{
    switch (radix)
    {
        case 8: return 'o';
        case 16: return 'x';
        default: return 'd';
    }
}

char *itoa( int value, char *buffer, int radix ) noexcept
{
    char format[] = {'%', printfFmt(radix), '\0'};
    sprintf(buffer, format, value);
    return buffer;
}

char *ltoa( long value, char *buffer, int radix ) noexcept
{
    char format[] = {'%', 'l', printfFmt(radix), '\0'};
    sprintf(buffer, format, value);
    return buffer;
}

char *ultoa( unsigned long value, char *buffer, int radix ) noexcept
{
    if (radix == 10)
    {
        char format[] = "%lu";
        sprintf(buffer, format, value);
        return buffer;
    }
    return ltoa(value, buffer, radix);
}

#endif // _MSC_VER
