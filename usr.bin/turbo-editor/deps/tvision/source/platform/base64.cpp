#include <internal/base64.h>

namespace tvision
{

// Based on the base64 decoder by polfosol and Gaspard Petit:
// https://github.com/gaspardpetit/base64/blob/21943915e99b45600c30a33f0299f4abc47e63db/src/polfosol/polfosol.h

static constexpr char b64e[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static constexpr uint8_t b64d[256] =
{
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 62 , 128, 62 , 128, 63 ,
    52 , 53 , 54 , 55 , 56 , 57 , 58 , 59 , 60 , 61 , 128, 128, 128, 128, 128, 128,
    128, 0  , 1  , 2  , 3  , 4  , 5  , 6  , 7  , 8  , 9  , 10 , 11 , 12 , 13 , 14 ,
    15 , 16 , 17 , 18 , 19 , 20 , 21 , 22 , 23 , 24 , 25 , 128, 128, 128, 128, 63 ,
    128, 26 , 27 , 28 , 29 , 30 , 31 , 32 , 33 , 34 , 35 , 36 , 37 , 38 , 39 , 40 ,
    41 , 42 , 43 , 44 , 45 , 46 , 47 , 48 , 49 , 50 , 51 , 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
};

TStringView encodeBase64(TStringView input, char *output) noexcept
{
    auto* p = (const uint8_t *) input.data();
    size_t iLen = input.size();
    size_t padLen = iLen % 3;

    size_t j = 0;
    for (size_t i = 0; i < iLen - padLen; i += 3, j += 4)
    {
        uint32_t n = p[i] << 16 | p[i + 1] << 8 | p[i + 2];
        output[j    ] = b64e[n >> 18];
        output[j + 1] = b64e[n >> 12 & 0x3F];
        output[j + 2] = b64e[n >> 6 & 0x3F];
        output[j + 3] = b64e[n & 0x3F];
    }
    if (padLen == 1)
    {
        uint32_t n = p[iLen - 1];
        output[j    ] = b64e[n >> 2];
        output[j + 1] = b64e[(n & 3) << 4];
        output[j + 2] = '=';
        output[j + 3] = '=';
        j += 4;
    }
    else if (padLen == 2)
    {
        uint32_t n = p[iLen - 2] << 8 | p[iLen - 1];
        output[j    ] = b64e[n >> 10];
        output[j + 1] = b64e[n >> 4 & 0x03F];
        output[j + 2] = b64e[(n & 0xF) << 2];
        output[j + 3] = '=';
        j += 4;
    }

    return {output, j};
}

TStringView decodeBase64(TStringView input, char *aOutput) noexcept
{
    auto *p = (const uint8_t *) input.data();
    size_t iLen = input.size();
    bool hasPadding = iLen > 0 && (iLen % 4 != 0 || p[iLen - 1] == '=');
    size_t noPadLen = ((iLen + 3) / 4 - hasPadding) * 4;
    auto *output = (uint8_t *) aOutput;

    size_t j = 0;
    for (size_t i = 0; i < noPadLen; i += 4, j += 3)
    {
        uint32_t n = b64d[p[i]] << 18 | b64d[p[i + 1]] << 12 | b64d[p[i + 2]] << 6 | b64d[p[i + 3]];
        output[j    ] = n >> 16;
        output[j + 1] = n >> 8 & 0xFF;
        output[j + 2] = n & 0xFF;
    }
    if (hasPadding && noPadLen + 1 < iLen)
    {
        uint32_t n = b64d[p[noPadLen]] << 18 | b64d[p[noPadLen + 1]] << 12;
        output[j++] = n >> 16;
        if (noPadLen + 2 < iLen && p[noPadLen + 2] != '=')
        {
            n |= b64d[p[noPadLen + 2]] << 6;
            output[j++] = n >> 8 & 0xFF;
        }
    }

    return {aOutput, j};
}

} // namespace tvision
