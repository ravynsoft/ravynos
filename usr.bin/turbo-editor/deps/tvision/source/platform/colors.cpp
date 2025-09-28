#define Uses_TColorAttr
#include <tvision/tv.h>
#include <internal/constarr.h>

namespace tvision
{

//// RGB to XTerm16 conversion algorithm
//
// XTerm16 is actually what's known as a 4-bit RGBI color palette, which
// is not regular. Many solutions out there overlook this fact. Others rely
// on the so-called CIEDE2000 formula, which doesn't convince me either.
//
// The algorithm here consists in converting RGB to HSL as an intermediate
// step. Then we do the following:
//
// * Decide whether the color should be approximated to grayscale or not.
// * If it is grayscale, pick between the 4 levels of gray. Otherwise, pick
//   between the dark and bright color variants. The L component is used for this.
// * If it is color, choose the final color based on the H component.
//
// The result is perceptually closer to the original than the other solutions
// I have seen around. Additionally, this algorithm can be computed in real-time.
//
// This implementation uses integer arithmetic and performs at most one integer
// division.

struct HCL
{
    uint8_t h; // [0..HUE_MAX)
    uint8_t c; // [0..255]
    uint8_t l; // [0..255]
};

constexpr uint8_t HUE_PRECISION = 32;
constexpr uint8_t HUE_MAX = 6*HUE_PRECISION;

static constexpr
HCL RGBtoHCL(uint8_t R, uint8_t G, uint8_t B) noexcept
{
    uint8_t Xmin = min(min(R, G), B);
    uint8_t Xmax = max(max(R, G), B);
    uint8_t V = Xmax;
    uint8_t L = uint16_t(Xmax + Xmin)/2;
    uint8_t C = Xmax - Xmin;
    int16_t H = 0;
    if (C)
    {
        if (V == R)
            H = int16_t(HUE_PRECISION*(G - B))/C;
        else if (V == G)
            H = int16_t(HUE_PRECISION*(B - R))/C + 2*HUE_PRECISION;
        else if (V == B)
            H = int16_t(HUE_PRECISION*(R - G))/C + 4*HUE_PRECISION;

        if (H < 0)
            H += HUE_MAX;
        else if (H >= HUE_MAX)
            H -= HUE_MAX;
    }

    return {(uint8_t) H, C, L};
}

static constexpr uint8_t u8(double d) noexcept
{
    return uint8_t(d*255);
}

static constexpr
uint8_t RGBtoXTerm16(uint8_t r, uint8_t g, uint8_t b) noexcept
{
    HCL c = RGBtoHCL(r, g, b);

    if (c.c >= 12) // Color if Chroma >= 12.
    {
        constexpr uint8_t normal[6] = {0x1, 0x3, 0x2, 0x6, 0x4, 0x5};
        constexpr uint8_t bright[6] = {0x9, 0xB, 0xA, 0xE, 0xC, 0xD};
        uint8_t index = uint8_t(c.h < HUE_MAX - HUE_PRECISION/2 ?
                                    c.h + HUE_PRECISION/2
                                  : c.h - (HUE_MAX - HUE_PRECISION/2)
                               )/HUE_PRECISION;
        if (c.l < u8(0.5))
            return normal[index];
        if (c.l < u8(0.925))
            return bright[index];
        return 15;
    }
    else
    {
        if (c.l < u8(0.25))
            return 0;
        if (c.l < u8(0.625))
            return 8;
        if (c.l < u8(0.875))
            return 7;
        return 15;
    }
}

static constexpr
constarray<uint8_t, 256> initXTerm256toXTerm16LUT() noexcept
{
    constarray<uint8_t, 256> res {};
    for (uint8_t i = 0; i < 16; ++i)
        res[i] = i;
    for (uint8_t i = 0; i < 6; ++i)
    {
        uint8_t R = i ? 55 + i*40 : 0;
        for (uint8_t j = 0; j < 6; ++j)
        {
            uint8_t G = j ? 55 + j*40 : 0;
            for (uint8_t k = 0; k < 6; ++k)
            {
                uint8_t B = k ? 55 + k*40 : 0;
                uint8_t idx16 = RGBtoXTerm16(R, G, B);
                res[16 + (i*6 + j)*6 + k] = idx16;
            }
        }
    }
    for (uint8_t i = 0; i < 24; ++i)
    {
        uint8_t L = i * 10 + 8;
        uint8_t idx16 = RGBtoXTerm16(L, L, L);
        res[232 + i] = idx16;
    }
    return res;
}

static constexpr
uint32_t pack(uint8_t R, uint8_t G, uint8_t B) noexcept
{
    return (((R << 8) | G) << 8) | B;
};

static constexpr
constarray<uint32_t, 256> initXTerm256toRGBLUT() noexcept
{
    // Indices 16..255 only.
    constarray<uint32_t, 256> res {};
    for (uint8_t i = 0; i < 6; ++i)
    {
        uint8_t R = i ? 55 + i*40 : 0;
        for (uint8_t j = 0; j < 6; ++j)
        {
            uint8_t G = j ? 55 + j*40 : 0;
            for (uint8_t k = 0; k < 6; ++k)
            {
                uint8_t B = k ? 55 + k*40 : 0;
                res[16 + (i*6 + j)*6 + k] = pack(R, G, B);
            }
        }
    }
    for (uint8_t i = 0; i < 24; ++i)
    {
        uint8_t L = i * 10 + 8;
        res[232 + i] = pack(L, L, L);
    }
    return res;
}

extern constexpr
constarray<uint8_t, 256> XTerm256toXTerm16LUT = initXTerm256toXTerm16LUT();

extern constexpr
constarray<uint32_t, 256> XTerm256toRGBLUT = initXTerm256toRGBLUT();

uint8_t RGBtoXTerm16Impl(TColorRGB c) noexcept
{
    return RGBtoXTerm16(c.r, c.g, c.b);
}

} // namespace tvision
