/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   COLORS.H                                                              */
/*                                                                         */
/*   Defines the structs TColorBIOS, TColorRGB, TColorXTerm,               */
/*   TColorDesired, TColorAttr and TAttrPair.                              */
/*                                                                         */
/* ------------------------------------------------------------------------*/

#ifndef TVISION_COLORS_H
#define TVISION_COLORS_H

#ifdef __BORLANDC__

inline TColorDesired getFore(const TColorAttr &attr)
{
    return uchar(attr & 0xF);
}

inline TColorDesired getBack(const TColorAttr &attr)
{
    return uchar(attr >> 4);
}

inline void setFore(TColorAttr &attr, TColorDesired color)
{
    attr = uchar(attr & 0xF0) | uchar(color & 0xF);
}

inline void setBack(TColorAttr &attr, TColorDesired color)
{
    attr = uchar(attr & 0xF) | uchar(color << 4);
}

inline TColorAttr reverseAttribute(TColorAttr attr)
{
    return uchar(attr << 4) | uchar(attr >> 4);
}

#else // __BORLANDC__

#include <string.h>

//// Color Formats

////// TColorRGB
//
// Can be initialized like this:
//     TColorRGB rgb = {127, 0, 187}; // {red, green, blue}.
// Or with an integer:
//     TColorRGB rgb = 0x7F00BB;      // 0xRRGGBB
// Can be converted back to integer types:
//     uint32_t asInt = TColorRGB(127, 0, 187);
// When doing so, the unused bits are discarded:
//     uint32_t(TColorRGB(0xAABBCCDD)) == 0xBBCCDD;

struct TColorRGB
{
    uint32_t
#ifndef TV_BIG_ENDIAN
        b       : 8,
        g       : 8,
        r       : 8,
        _unused : 8;
#else
        _unused : 8,
        r       : 8,
        g       : 8,
        b       : 8;
#endif

    TV_TRIVIALLY_CONVERTIBLE(TColorRGB, uint32_t, 0xFFFFFF)
    constexpr inline TColorRGB(uint8_t r, uint8_t g, uint8_t b);
};

constexpr inline TColorRGB::TColorRGB(uint8_t r, uint8_t g, uint8_t b) :
#ifndef TV_BIG_ENDIAN
    b(b),
    g(g),
    r(r),
    _unused(0)
#else
    _unused(0),
    r(r),
    g(g),
    b(b)
#endif
{
}

////// TColorBIOS
//
// This is the 4-bit color encoding used originally by Turbo Vision on MS-DOS.
//
// Can be initialized with an integer:
//     TColorBIOS black = 0x0,
//                blue = 0x1,
//                dark_gray = 0x8;
// And converted back to a integer types:
//     uint8_t asChar = TColorBIOS(0xC);
// When doing so, the unused bits are discarded:
//     uint8_t(TColorBIOS(0xAB)) == 0xB;

struct TColorBIOS
{
    uint8_t
#ifndef TV_BIG_ENDIAN
        b       : 1,
        g       : 1,
        r       : 1,
        bright  : 1,
        _unused : 4;
#else
        _unused : 4,
        bright  : 1,
        r       : 1,
        g       : 1,
        b       : 1;
#endif

    TV_TRIVIALLY_CONVERTIBLE(TColorBIOS, uint8_t, 0xF)
};

////// TColorXTerm
//
// Index into an 256-color palette recognized by the 'xterm-256color' terminal type.
// Some terminal emulators support modifying the palette, but Turbo Vision does
// not make use of this feature. So we assume index values 16 to 255 can be
// unequivocally converted from and into RGB as per the table in:
//
// https://jonasjacek.github.io/colors/
//
// Indices 0 to 15 will be displayed just like BIOS colors.
//
// This type can be converted from and into integer types:
//     TColorXTerm xterm = 0xFE;
//     uint8_t asChar = xterm;

struct TColorXTerm
{
    uint8_t idx;

    TV_TRIVIALLY_CONVERTIBLE(TColorXTerm, uint8_t, 0xFF)
};

//// Color Conversion Functions
//
// They convert between the color types defined previously and other common
// color formats.
//
// No conversion from XTerm16 to RGB is provided because there is no consensus
// on how these colors should be represented and we don't want to encourage
// developers to assume the opposite. Most terminal emulators allow users to
// configure these colors through color schemes.
//
// That's not the case of XTerm256 indices 16 to 255 (with 0 to 15 being the
// same as XTerm16). So we assume these can be unambiguously mapped to a single
// RGB value as per the tables in:
//
// https://jonasjacek.github.io/colors/

inline uint8_t BIOStoXTerm16(TColorBIOS);
inline TColorBIOS RGBtoBIOS(TColorRGB);
inline uint8_t RGBtoXTerm16(TColorRGB);
inline uint8_t RGBtoXTerm256(TColorRGB);
inline TColorBIOS XTerm16toBIOS(uint8_t);
inline TColorRGB XTerm256toRGB(uint8_t); // Only for indices 16..255.
inline uint8_t XTerm256toXTerm16(uint8_t);

namespace tvision
{
    template <class T, size_t N>
    struct constarray;

    uint8_t RGBtoXTerm16Impl(TColorRGB) noexcept;
    extern const constarray<uint8_t, 256> XTerm256toXTerm16LUT;
    extern const constarray<uint32_t, 256> XTerm256toRGBLUT;
}

inline uint8_t BIOStoXTerm16(TColorBIOS c)
{
    // Swap the Red and Blue bits.
    uchar aux = c.b;
    c.b = c.r;
    c.r = aux;
    return c;
}

inline TColorBIOS RGBtoBIOS(TColorRGB c)
{
    return XTerm16toBIOS(RGBtoXTerm16(c));
}

inline uint8_t RGBtoXTerm16(TColorRGB c)
{
    using namespace tvision;
    return RGBtoXTerm16Impl(c);
}

inline uint8_t RGBtoXTerm256(TColorRGB c)
{
    // The xterm-256color palette consists of:
    //
    // * [0..15]: 16 colors as in xterm-16color.
    // * [16..231]: 216 colors in a 6x6x6 cube.
    // * [232..255]: 24 grayscale colors.
    //
    // This function does not return indices in the range [0..15]. For that,
    // use 'RGBtoXTerm16' instead.
    //
    // Dark colors are underrepresented in the 6x6x6 cube. The channel values
    // [0, 1, 2, 3, 4, 5] correspond to the 8-bit values
    // [0, 95, 135, 175, 215, 255]. Thus there is a distance of 40 between
    // values, except for 0. Any 8-bit value smaller than 95 - 40/2 = 75
    // would have to be mapped into 0. To compensate a bit for this, we allow
    // values [55..74] to also be mapped into 1.
    //
    // Additionally, we fall back on the grayscale colors whenever using
    // the 6x6x6 color cube would round the color to pure black. This
    // makes it possible to preserve details that would otherwise be lost.
    auto cnvColor = [] (TColorRGB c)
    {
        auto scale = [] (uchar c)
        {
            c += 20 & -(c < 75);
            return uchar(max<uchar>(c, 35) - 35)/40;
        };
        uchar r = scale(c.r),
              g = scale(c.g),
              b = scale(c.b);
        return 16 + uchar(r*uchar(6) + g)*uchar(6) + b;
    };
    auto cnvGray = [] (uchar l)
    {
        if (l < 8 - 5)
            return 16;
        if (l >= 238 + 5)
            return 231;
        return 232 + uchar(max<uchar>(l, 3) - 3)/uchar(10);
    };

    uchar idx = cnvColor(c);
    if (c != XTerm256toRGB(idx))
    {
        uchar Xmin = min(min(c.r, c.g), c.b),
              Xmax = max(max(c.r, c.g), c.b);
        uchar C = Xmax - Xmin; // Chroma in the HSL/HSV theory.
        if (C < 12 || idx == 16) // Grayscale if Chroma < 12 or rounded to black.
        {
            uchar L = ushort(Xmax + Xmin)/2; // Lightness, as in HSL.
            idx = cnvGray(L);
        }
    }
    return idx;
}

inline TColorBIOS XTerm16toBIOS(uint8_t idx)
{
    return BIOStoXTerm16(idx);
}

inline uint8_t XTerm256toXTerm16(uint8_t idx)
{
    using namespace tvision;
    return ((const uint8_t (&) [256]) XTerm256toXTerm16LUT)[idx];
}

inline TColorRGB XTerm256toRGB(uint8_t idx)
{
    using namespace tvision;
    return ((const uint32_t (&) [256]) XTerm256toRGBLUT)[idx];
}

//// TColorDesired
//
// This is a union type of the different possible kinds of color: BIOS, RGB,
// XTerm or Default.
// The purpose of this type is to describe the foreground *or* background color
// of a screen cell.
//
// You can initialize as BIOS color with a char literal, as RGB with an integer
// literal and as Default with zero-initialization:
//     TColorDesired bios = '\xF',
//                   rgb  = 0x7F00BB,
//                   def  = {};
//
// In a terminal emulator, the 'default color' is the color of text that has no
// display attributes (bold, color...) enabled.

const uchar
    ctDefault       = 0x0,  // Terminal default.
    ctBIOS          = 0x1,  // TColorBIOS.
    ctRGB           = 0x2,  // TColorRGB.
    ctXTerm         = 0x3;  // TColorXTerm.

struct TColorDesired
{

    uint32_t _data;

    TColorDesired() = default;

    // Constructors for use with literals.

    constexpr inline TColorDesired(char bios);   // e.g. {'\xF'}
    constexpr inline TColorDesired(uchar bios);
    constexpr inline TColorDesired(int rgb);     // e.g. {0x7F00BB}
    // Use zero-initialization for type Default: {}

    // Constructors with explicit type names.

    inline TColorDesired(TColorBIOS bios);
    inline TColorDesired(TColorRGB rgb);
    inline TColorDesired(TColorXTerm xterm);

    TV_TRIVIALLY_ASSIGNABLE(TColorDesired)

    constexpr inline uchar type() const;
    constexpr inline bool isDefault() const;
    constexpr inline bool isBIOS() const;
    constexpr inline bool isRGB() const;
    constexpr inline bool isXTerm() const;

    // No conversion is performed! Make sure to check the type first.

    inline TColorBIOS asBIOS() const;
    inline TColorRGB asRGB() const;
    inline TColorXTerm asXTerm() const;

    // Quantization to TColorBIOS.

    inline TColorBIOS toBIOS(bool isForeground) const;

    inline bool operator==(TColorDesired other) const;
    inline bool operator!=(TColorDesired other) const;

    constexpr inline uint32_t bitCast() const;
    constexpr inline void bitCast(uint32_t val);

};

constexpr inline TColorDesired::TColorDesired(char bios) :
    TColorDesired(uchar(bios))
{
}

constexpr inline TColorDesired::TColorDesired(uchar bios) :
    _data((bios & 0xF) | (ctBIOS << 24))
{
}

constexpr inline TColorDesired::TColorDesired(int rgb) :
    _data((rgb & 0xFFFFFF) | (ctRGB << 24))
{
}

inline TColorDesired::TColorDesired(TColorBIOS bios) :
    TColorDesired(uchar(bios))
{
}

inline TColorDesired::TColorDesired(TColorRGB rgb) :
    TColorDesired(int(rgb))
{
}

inline TColorDesired::TColorDesired(TColorXTerm xterm) :
    _data(xterm | (ctXTerm << 24))
{
}

constexpr inline uchar TColorDesired::type() const
{
    return _data >> 24;
}

constexpr inline bool TColorDesired::isDefault() const
{
    return type() == ctDefault;
}

constexpr inline bool TColorDesired::isBIOS() const
{
    return type() == ctBIOS;
}

constexpr inline bool TColorDesired::isRGB() const
{
    return type() == ctRGB;
}

constexpr inline bool TColorDesired::isXTerm() const
{
    return type() == ctXTerm;
}

inline TColorBIOS TColorDesired::asBIOS() const
{
    return _data;
}

inline TColorRGB TColorDesired::asRGB() const
{
    return _data;
}

inline TColorXTerm TColorDesired::asXTerm() const
{
    return _data;
}

inline TColorBIOS TColorDesired::toBIOS(bool isForeground) const
{
    switch (type())
    {
        case ctBIOS:
            return asBIOS();
        case ctRGB:
            return RGBtoBIOS(asRGB());
        case ctXTerm:
        {
            uint8_t idx = asXTerm();
            if (idx >= 16)
                idx = XTerm256toXTerm16(idx);
            return XTerm16toBIOS(idx);
        }
        default:
            return isForeground ? 0x7 : 0x0;
    }
}

inline bool TColorDesired::operator==(TColorDesired other) const
{
    return memcmp(this, &other, sizeof(*this)) == 0;
}

inline bool TColorDesired::operator!=(TColorDesired other) const
{
    return !(*this == other);
}

constexpr inline uint32_t TColorDesired::bitCast() const
{
    return _data;
}

constexpr inline void TColorDesired::bitCast(uint32_t val)
{
    _data = val;
}

//// TColorAttr
//
// Represents the color attributes of a screen cell.
// Examples:
//
//     /* Foreground: BIOS 0x7.             */
//     /* Background: RGB 0x7F00BB.         */
//     /* Style: Bold, Italic.              */
//     TColorAttr a = {'\x07', 0x7F00BB, slBold | slItalic};
//
//     /* Foreground: Default.              */
//     /* Background: BIOS 0xF.             */
//     /* Style: Normal.                    */
//     TColorAttr b = {{}, '\xF'};
//
// For backward-compatibility, you can also use initialize a TColorAttr
// with a BIOS color attribute:
//
//     /* Foreground: BIOS 0xD.             */
//     /* Background: BIOS 0x3.             */
//     /* Style: Normal.                    */
//     TColorAttr c = 0x3D;
//
// A zero-initialized TColorAttr has both the foreground and background
// colors set to 'default'. Therefore, a zero-initialized TColorAttr produces
// visible text.

const ushort

// TColorAttr Style masks

    slBold          = 0x001,
    slItalic        = 0x002,
    slUnderline     = 0x004,
    slBlink         = 0x008,
    slReverse       = 0x010, // Prefer using 'reverseAttribute()' instead.
    slStrike        = 0x020,

// Private masks

    slNoShadow      = 0x200; // Don't draw window shadows over this cell.

struct TAttrPair;

struct TColorAttr
{
    using Style = ushort;

    uint64_t
        _style      : 10,
        _fg         : 27,
        _bg         : 27;

    TColorAttr() = default;
    constexpr inline TColorAttr(int bios);
    constexpr inline TColorAttr(TColorDesired fg, TColorDesired bg, ushort style=0);
    inline TColorAttr(const TAttrPair &attrs);
    TV_TRIVIALLY_ASSIGNABLE(TColorAttr)

    inline bool isBIOS() const;
    inline uchar asBIOS() const; // Result is meaningful only if it actually is BIOS.
    inline uchar toBIOS() const; // Quantization.

    inline operator uchar() const;
    inline TAttrPair operator<<(int shift) const;

    inline bool operator==(const TColorAttr &other) const;
    inline bool operator!=(const TColorAttr &other) const;

    inline bool operator==(int bios) const;
    inline bool operator!=(int bios) const;

};

constexpr inline TColorDesired getFore(const TColorAttr &attr);
constexpr inline TColorDesired getBack(const TColorAttr &attr);
constexpr inline ushort getStyle(const TColorAttr &attr);
constexpr inline void setFore(TColorAttr &attr, TColorDesired fg);
constexpr inline void setBack(TColorAttr &attr, TColorDesired bg);
constexpr inline void setStyle(TColorAttr &attr, ushort style);
constexpr inline TColorAttr reverseAttribute(TColorAttr attr);

constexpr inline TColorAttr::TColorAttr(int bios) :
    _style(0),
    _fg(TColorDesired(uchar(bios)).bitCast()),
    _bg(TColorDesired(uchar(bios >> 4)).bitCast())
{
}

constexpr inline TColorAttr::TColorAttr(TColorDesired fg, TColorDesired bg, ushort style) :
    _style(style),
    _fg(fg.bitCast()),
    _bg(bg.bitCast())
{
}

inline bool TColorAttr::isBIOS() const
{
    return (int) ::getFore(*this).isBIOS() & ::getBack(*this).isBIOS() & !::getStyle(*this);
}

inline uchar TColorAttr::asBIOS() const
{
    // 'this' must be a BIOS attribute. If it is not, the result will be
    // bogus but harmless. The important is that the result isn't '\x0'
    // unless this is BIOS attribute '\x0'.
    uchar bios = uchar(_fg) | uchar(_bg << 4);
    return isBIOS() ? bios : 0x5F;
}

inline uchar TColorAttr::toBIOS() const
{
    auto fg = ::getFore(*this),
         bg = ::getBack(*this);
    return fg.toBIOS(true) | (bg.toBIOS(false) << 4);
}

inline TColorAttr::operator uchar() const
{
    return asBIOS();
}

inline bool TColorAttr::operator==(const TColorAttr &other) const
{
    return memcmp(this, &other, sizeof(*this)) == 0;
}

inline bool TColorAttr::operator!=(const TColorAttr &other) const
{
    return !(*this == other);
}

inline bool TColorAttr::operator==(int bios) const
{
    return *this == TColorAttr {(uchar) bios};
}

inline bool TColorAttr::operator!=(int bios) const
{
    return !(*this == bios);
}

constexpr inline TColorDesired getFore(const TColorAttr &attr)
{
    TColorDesired color {};
    color.bitCast(attr._fg);
    return color;
}

constexpr inline TColorDesired getBack(const TColorAttr &attr)
{
    TColorDesired color {};
    color.bitCast(attr._bg);
    return color;
}

constexpr inline ushort getStyle(const TColorAttr &attr)
{
    return attr._style;
}

constexpr inline void setFore(TColorAttr &attr, TColorDesired color)
{
    attr._fg = color.bitCast();
}

constexpr inline void setBack(TColorAttr &attr, TColorDesired color)
{
    attr._bg = color.bitCast();
}

constexpr inline void setStyle(TColorAttr &attr, ushort style)
{
    attr._style = style;
}

constexpr inline TColorAttr reverseAttribute(TColorAttr attr)
{
    auto fg = ::getFore(attr),
         bg = ::getBack(attr);
    // The 'slReverse' attribute is represented differently by every terminal,
    // so it is better to swap the colors manually unless any of them is default.
    if ((int) fg.isDefault() | bg.isDefault())
        ::setStyle(attr, ::getStyle(attr) ^ slReverse);
    else
    {
        ::setFore(attr, bg);
        ::setBack(attr, fg);
    }
    return attr;
}

//// TAttrPair
//
// Represents a pair of color attributes.
// Example:
//
//     TColorAttr cNormal = {0x234983, 0x267232};
//     TColorAttr cHigh = {0x309283, 0x127844};
//     TAttrPair attrs = {cNormal, cHigh};
//     TDrawBuffer b;
//     b.moveCStr(0, "Normal text, ~Highlighted text~", attrs);

struct TAttrPair
{

    TColorAttr _attrs[2];

    TAttrPair() = default;
    constexpr inline TAttrPair(int bios);
    constexpr inline TAttrPair(const TColorAttr &lo, const TColorAttr &hi=uchar(0));
    TV_TRIVIALLY_ASSIGNABLE(TAttrPair)

    inline ushort asBIOS() const;

    inline operator ushort() const;
    inline TAttrPair operator>>(int shift) const;
    inline TAttrPair& operator|=(TColorAttr attr);

    inline TColorAttr& operator[](size_t i);
    inline const TColorAttr& operator[](size_t i) const;

};

constexpr inline TAttrPair::TAttrPair(int bios) :
    _attrs {uchar(bios & 0xFF), uchar(bios >> 8)}
{
}

constexpr inline TAttrPair::TAttrPair(const TColorAttr &lo, const TColorAttr &hi) :
    _attrs {lo, hi}
{
}

inline ushort TAttrPair::asBIOS() const
{
    return _attrs[0].asBIOS() | ushort(_attrs[1].asBIOS() << 8);
}

inline TAttrPair::operator ushort() const
{
    return asBIOS();
}

inline TAttrPair TAttrPair::operator>>(int shift) const
{
    // Legacy code may use '>> 8' on an attribute pair to get the higher attribute.
    if (shift == 8)
        return {_attrs[1]};
    return asBIOS() >> shift;
}

inline TAttrPair& TAttrPair::operator|=(TColorAttr attr)
{
    // Legacy code may use '|=' on an attribute pair to set the lower attribute.
    _attrs[0] = attr;
    return *this;
}

inline TColorAttr& TAttrPair::operator[](size_t i)
{
    return _attrs[i];
}

inline const TColorAttr& TAttrPair::operator[](size_t i) const
{
    return _attrs[i];
}

// Pending methods from TColorAttr.

inline TColorAttr::TColorAttr(const TAttrPair &attrs)
{
    *this = attrs[0];
}

inline TAttrPair TColorAttr::operator<<(int shift) const
{
    // Legacy code may use '<< 8' on an attribute to construct an attribute pair.
    if (shift == 8)
        return {uchar(0), *this};
    return asBIOS() << shift;
}

#endif // __BORLANDC__

#endif // TVISION_COLORS_H
