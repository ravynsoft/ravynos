#include <internal/ansidisp.h>
#include <internal/stdioctl.h>
#include <internal/strings.h>
#include <stdlib.h>

#define CSI "\x1B["

namespace tvision
{

inline AnsiDisplayBase::Buffer::~Buffer()
{
    free(head);
}

inline char *AnsiDisplayBase::Buffer::data() noexcept
{
    return head;
}

inline size_t AnsiDisplayBase::Buffer::size() const noexcept
{
    return tail - head;
}

inline void AnsiDisplayBase::Buffer::clear() noexcept
{
    tail = head;
}

inline void AnsiDisplayBase::Buffer::push(TStringView s) noexcept
{
    memcpy(tail, s.data(), s.size());
    tail += s.size();
}

inline void AnsiDisplayBase::Buffer::push(char c) noexcept
{
    *tail++ = c;
}

inline void AnsiDisplayBase::Buffer::reserve(size_t extraCapacity) noexcept
{
    size_t oldSize = size();
    if (oldSize + extraCapacity > capacity)
    {
        capacity = max<size_t>(max<size_t>(4096, 2*capacity), capacity + extraCapacity);
        if (!(head = (char *) realloc(head, capacity)))
            abort();
        tail = head + oldSize;
    }
}

AnsiDisplayBase::~AnsiDisplayBase()
{
    clearAttributes();
    lowlevelFlush();
}

inline void AnsiDisplayBase::bufWriteCSI1(uint a, char F) noexcept
{
    // CSI a F
    buf.reserve(32);
    buf.push(CSI);
    buf.tail = fast_utoa(a, buf.tail);
    buf.push(F);
}

inline void AnsiDisplayBase::bufWriteCSI2(uint a, uint b, char F) noexcept
{
    // CSI a ; b F
    buf.reserve(32);
    buf.push(CSI);
    buf.tail = fast_utoa(a, buf.tail);
    buf.push(';');
    buf.tail = fast_utoa(b, buf.tail);
    buf.push(F);
}

void AnsiDisplayBase::clearAttributes() noexcept
{
    buf.reserve(4);
    buf.push(CSI "0m");
    lastAttr = {};
}

void AnsiDisplayBase::clearScreen() noexcept
{
    buf.reserve(4);
    buf.push(CSI "2J");
}

static char *convertAttributes(const TColorAttr &, TermAttr &, const TermCap &, char*) noexcept;

void AnsiDisplayBase::lowlevelWriteChars( TStringView chars, TColorAttr attr,
                                          const TermCap &termcap ) noexcept
{
    buf.reserve(256);
    buf.tail = convertAttributes(attr, lastAttr, termcap, buf.tail);
    buf.push(chars);
}

void AnsiDisplayBase::lowlevelMoveCursorX(uint x, uint) noexcept
{
    // Optimized case where the cursor only moves horizontally.
    bufWriteCSI1(x + 1, 'G');
}

void AnsiDisplayBase::lowlevelMoveCursor(uint x, uint y) noexcept
{
    buf.reserve(32);
//     buf.push('\r'); // Make dumps readable.
    bufWriteCSI2(y + 1, x + 1, 'H');
}

void AnsiDisplayBase::lowlevelFlush() noexcept
{
    io.write(buf.data(), buf.size());
    buf.clear();
}

//////////////////////////////////////////////////////////////////////////
// Attribute conversion

static void convertColor(TColorDesired, TermColor &, TColorAttr::Style &, const TermCap &, bool) noexcept;
static char *writeAttributes(const TermAttr &, const TermAttr &, char *) noexcept;
static char *writeColor(TermColor, bool, char *) noexcept;

static inline char *convertAttributes( const TColorAttr &c, TermAttr &lastAttr,
                                       const TermCap &termcap, char *buf ) noexcept
{
    TermAttr attr {};
    attr.style = ::getStyle(c);

    convertColor(::getFore(c), attr.fg, attr.style, termcap, true);
    convertColor(::getBack(c), attr.bg, attr.style, termcap, false);

    if (termcap.quirks & qfNoItalic)
        attr.style &= ~slItalic;
    if (termcap.quirks & qfNoUnderline)
        attr.style &= ~slUnderline;

    char *p = writeAttributes(attr, lastAttr, buf);

    lastAttr = attr;

    return p;
}

// colorconv_r: return value of the color conversion funcions.

struct alignas(8) colorconv_r
{
    TermColor color;
    TColorAttr::Style extraStyle;
    uint8_t unused[2];

    colorconv_r() = default;
    colorconv_r(TermColor aColor, TColorAttr::Style aExtraFlags=0) noexcept
    {
        // Optimization: do bit-casting manually, just like with TermColor.
        uint64_t val = aColor | (uint64_t(aExtraFlags) << 32);
#ifdef TV_BIG_ENDIAN
        reverseBytes(val);
#endif
        memcpy(this, &val, 8);
        static_assert(sizeof(*this) == 8, "");
    }
};

static colorconv_r convertNoColor(TColorDesired, const TermCap &, bool) noexcept;
static colorconv_r convertIndexed8(TColorDesired color, const TermCap &, bool) noexcept;
static colorconv_r convertIndexed16(TColorDesired, const TermCap &, bool) noexcept;
static colorconv_r convertIndexed256(TColorDesired, const TermCap &, bool) noexcept;
static colorconv_r convertDirect(TColorDesired, const TermCap &, bool) noexcept;

// C++ doesn't allow creating an array of noexcept function pointers directly...
struct ColorConverter
{
    colorconv_r (&apply) (TColorDesired, const TermCap &, bool) noexcept;
};

static constexpr ColorConverter colorConverters[TermCapColorCount] =
{
    {convertNoColor},
    {convertIndexed8},
    {convertIndexed16},
    {convertIndexed256},
    {convertDirect},
};

static inline void convertColor( TColorDesired c,
                                 TermColor &resultColor, TColorAttr::Style &resultStyle,
                                 const TermCap &termcap, bool isFg ) noexcept
{
    auto cnv = colorConverters[termcap.colors].apply(c, termcap, isFg);
    resultColor = cnv.color;
    resultStyle |= cnv.extraStyle;
}

static inline void push(char *&p, TStringView text) noexcept
{
    memcpy(p, text.data(), text.size());
    p += text.size();
}

static inline void push(char *&p, bool b, const char *s1, const char *s2) noexcept
{
    // Use the ternary on the invocation and not on the argument because
    // some compilers are unable to optimize out strlen and memcpy otherwise.
    b ? push(p, s1) : push(p, s2);
}

static inline void writeFlag( char *&p, TermAttr attr, TermAttr lastAttr,
                              ushort mask, const char * const OnOff[2] ) noexcept
{
    if ((attr.style & mask) != (lastAttr.style & mask))
    {
        push(p, attr.style & mask, OnOff[0], OnOff[1]);
        push(p, ";");
    }
}

typedef const char *c_str;

static constexpr c_str
    boldOnOff[2] =      { "1", "22"},
    italicOnOff[2] =    { "3", "23"},
    underlineOnOff[2] = { "4", "24"},
    blinkOnOff[2] =     { "5", "25"},
    reverseOnOff[2] =   { "7", "27"},
    strikeOnOff[2] =    { "9", "29"};

static inline char *writeAttributes( const TermAttr &attr,
                                     const TermAttr &lastAttr, char *p ) noexcept
{
    push(p, CSI);

    writeFlag(p, attr, lastAttr, slBold, boldOnOff);
    writeFlag(p, attr, lastAttr, slItalic, italicOnOff);
    writeFlag(p, attr, lastAttr, slUnderline, underlineOnOff);
    writeFlag(p, attr, lastAttr, slBlink, blinkOnOff);
    writeFlag(p, attr, lastAttr, slReverse, reverseOnOff);
    writeFlag(p, attr, lastAttr, slStrike, strikeOnOff);

    if (attr.fg != lastAttr.fg)
        p = writeColor(attr.fg, true, p);
    if (attr.bg != lastAttr.bg)
        p = writeColor(attr.bg, false, p);

    if (p[-1] == ';')
        p[-1] = 'm';
    else
        p -= strlen(CSI);
    return p;
}

static inline void splitSGR(char *&p)
{
    if (p[-1] == ';')
    {
        p[-1] = 'm';
        push(p, CSI);
    }
}

static char *writeColor(TermColor color, bool isFg, char *p) noexcept
{
    // RGB and XTerm256 colors get a separate SGR sequence because some
    // terminal emulators may otherwise have trouble processing them.
    switch (color.type)
    {
        case TermColor::Default:
            push(p, isFg, "39;", "49;");
            break;
        case TermColor::Indexed:
            if (color.idx >= 16)
            {
                // <38,48>;5;i;
                splitSGR(p);
                push(p, isFg, "38;5;", "48;5;");
                p = fast_btoa(color.idx, p);
                push(p, ";");
                splitSGR(p);
            }
            else
            {
                if (color.idx >= 8)
                    // <90-97,100-107>;
                    p = fast_btoa(color.idx - 8 + (isFg ? 90 : 100), p);
                else
                    // <30-37,40-47>;
                    p = fast_btoa(color.idx + (isFg ? 30 : 40), p);
                push(p, ";");
            }
            break;
        case TermColor::RGB:
            // <38,48>;2;r;g;b;
            splitSGR(p);
            push(p, isFg, "38;2;", "48;2;");
            for (int i = 2; i >= 0; --i)
            {
                p = fast_btoa(color.bgr[i], p);
                push(p, ";");
            }
            splitSGR(p);
            break;
        case TermColor::NoColor:
            break;
    }
    return p;
}

// Color conversion functions

static colorconv_r convertNoColor(TColorDesired color, const TermCap &, bool isFg) noexcept
{
    colorconv_r cnv {{TermColor::NoColor}};
    // Mimic the mono palettes with styles.
    if (color.isBIOS())
    {
        auto bios = color.asBIOS();
        if (isFg)
        {
            if (bios & 0x8)
                cnv.extraStyle |= slBold;
            else if (bios == 0x1)
                cnv.extraStyle |= slUnderline;
        }
        else if ((bios & 0x7) == 0x7)
            cnv.extraStyle |= slReverse;
    }
    return cnv;
}

static colorconv_r convertIndexed8( TColorDesired color,
                                    const TermCap &termcap, bool isFg ) noexcept
{
    auto cnv = convertIndexed16(color, termcap, isFg);
    if ( cnv.color.type == TermColor::Indexed &&
         cnv.color.idx >= 8 )
    {
        cnv.color.idx -= 8;
        if (isFg)
        {
            if (termcap.quirks & qfBoldIsBright)
                cnv.extraStyle |= slBold;
        }
        else
        {
            if (termcap.quirks & qfBlinkIsBright)
                cnv.extraStyle |= slBlink;
        }
    }
    return cnv;
}

static colorconv_r convertIndexed16( TColorDesired color,
                                     const TermCap &, bool ) noexcept
{
    if (color.isBIOS())
    {
        uint8_t idx = BIOStoXTerm16(color.asBIOS());
        return {{idx, TermColor::Indexed}};
    }
    else if (color.isXTerm())
    {
        uint8_t idx = color.asXTerm();
        if (idx >= 16)
            idx = XTerm256toXTerm16(idx);
        return {{idx, TermColor::Indexed}};
    }
    else if (color.isRGB())
    {
        uint8_t idx = RGBtoXTerm16(color.asRGB());
        return {{idx, TermColor::Indexed}};
    }
    return {{TermColor::Default}};
}

static colorconv_r convertIndexed256( TColorDesired color,
                                      const TermCap &termcap, bool isFg ) noexcept
{
    if (color.isXTerm())
    {
        uint8_t idx = color.asXTerm();
        return {{idx, TermColor::Indexed}};
    }
    else if (color.isRGB())
    {
        uint8_t idx = RGBtoXTerm256(color.asRGB());
        return {{idx, TermColor::Indexed}};
    }
    return convertIndexed16(color, termcap, isFg);
}

static colorconv_r convertDirect( TColorDesired color,
                                  const TermCap &termcap, bool isFg ) noexcept
{
    if (color.isRGB())
    {
        auto rgb = color.asRGB();
        return {{rgb, TermColor::RGB}};
    }
    return convertIndexed256(color, termcap, isFg);
}

} // namespace tvision
