#ifndef TVISION_CODEPAGE_H
#define TVISION_CODEPAGE_H

#ifndef _TV_VERSION
#include <tvision/tv.h>
#endif

#include <string.h>
#include <unordered_map>

namespace tvision
{

class CpTranslator
{
    static const char (*currentToUtf8)[256][4];
    static const std::unordered_map<uint32_t, char> *currentFromUtf8;

public:

    static void init() noexcept;

    static uint32_t toPackedUtf8(unsigned char c) noexcept
    {
        uint32_t asInt;
        memcpy(&asInt, (*currentToUtf8)[c], sizeof(asInt));
        return asInt;
    }

    static char fromUtf8(TStringView s) noexcept;

    static char printableFromUtf8(TStringView s) noexcept
    {
        uchar c = fromUtf8(s);
        if (c < ' ')
            return '\0';
        return c;
    }
};

} // namespace tvision

#endif // TVISION_CODEPAGE_H
