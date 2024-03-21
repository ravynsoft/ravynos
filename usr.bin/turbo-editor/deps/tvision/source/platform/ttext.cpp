#define Uses_TText
#include <tvision/tv.h>

#include <internal/codepage.h>
#include <internal/platform.h>
#include <internal/unixcon.h>
#include <internal/linuxcon.h>
#include <internal/win32con.h>
#include <internal/winwidth.h>
#include <internal/utf8.h>
#include <wchar.h>

namespace ttext
{

// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

enum { UTF8_ACCEPT = 0, UTF8_REJECT = 12 };

static const uint8_t utf8d[] =
{
    // The first part of the table maps bytes to character classes that
    // to reduce the size of the transition table and create bitmasks.
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
     8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

    // The second part is a transition table that maps a combination
    // of a state of the automaton and a character class to a state.
     0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
    12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
    12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
    12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
    12,36,12,12,12,12,12,12,12,12,12,12,
};

static inline
uint32_t decode_utf8(uint32_t* state, uint32_t* codep, uint8_t byte) noexcept
{
    uint32_t type = utf8d[byte];

    *codep = (*state != UTF8_ACCEPT) ?
        (byte & 0x3F) | (*codep << 6) :
        (0xFF >> type) & (byte);

    *state = utf8d[256 + *state + type];
    return *state;
}

static inline
uint32_t decode_utf8(uint32_t* state, uint8_t byte) noexcept
{
    uint32_t type = utf8d[byte];
    *state = utf8d[256 + *state + type];
    return *state;
}

static int mbtowc(uint32_t &wc, TStringView text) noexcept
// Pre: text.size() > 0.
// Returns n >= 1 if 'text' begins with a UTF-8 multibyte character that's
// 'n' bytes long, -1 otherwise.
{
    uint32_t state = 0;
    uint32_t codep = 0;
    for (size_t i = 0; i < text.size(); ++i)
        switch (decode_utf8(&state, &codep, text[i]))
        {
            case UTF8_ACCEPT:
                return (wc = codep), i + 1;
            case UTF8_REJECT:
                return -1;
            default:
                break;
        }
    return -1;
}

static int mblen(TStringView text) noexcept
// Pre: text.size() > 0.
// Returns n >= 1 if 'text' begins with a UTF-8 multibyte character that's
// 'n' bytes long, -1 otherwise.
{
    uint32_t state = 0;
    for (size_t i = 0; i < text.size(); ++i)
        switch (decode_utf8(&state, text[i]))
        {
            case UTF8_ACCEPT:
                return i + 1;
            case UTF8_REJECT:
                return -1;
            default:
                break;
        }
    return -1;
}

struct mbstat_r { int length; int width; };

static mbstat_r mbstat(TStringView text) noexcept
// Pre: 'text.size() > 0'.
{
    using namespace tvision;
    uint32_t wc;
    int length = mbtowc(wc, text);
    int width = 1;
    if (length > 1)
        width = Platform::charWidth(wc);
    return {length, width};
}

} // namespace ttext

namespace tvision
{

#ifdef _TV_UNIX
int UnixConsoleStrategy::charWidth(uint32_t wc) noexcept
{
    return wcwidth(wc);
}
#endif // _TV_UNIX

#ifdef __linux__
int LinuxConsoleStrategy::charWidth(uint32_t wc) noexcept
{
    // The Linux Console does not support zero-width characters. It assumes
    // all characters are either single or double-width. Additionally, the
    // double-width characters are the same as in the wcwidth() implementation by
    // Markus Kuhn from 2007-05-26 (https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c).
    return 1 +
        (wc >= 0x1100 &&
         (wc <= 0x115f ||
          wc == 0x2329 || wc == 0x232a ||
          (wc >= 0x2e80 && wc <= 0xa4cf &&
           wc != 0x303f) ||
          (wc >= 0xac00 && wc <= 0xd7a3) ||
          (wc >= 0xf900 && wc <= 0xfaff) ||
          (wc >= 0xfe10 && wc <= 0xfe19) ||
          (wc >= 0xfe30 && wc <= 0xfe6f) ||
          (wc >= 0xff00 && wc <= 0xff60) ||
          (wc >= 0xffe0 && wc <= 0xffe6) ||
          (wc >= 0x20000 && wc <= 0x2fffd) ||
          (wc >= 0x30000 && wc <= 0x3fffd)));
}
#endif // __linux__

#ifdef _WIN32
int Win32ConsoleStrategy::charWidth(uint32_t wc) noexcept
{
    return WinWidth::width(wc);
}
#endif // _WIN32

} // namespace tvision

size_t TText::width(TStringView text) noexcept
{
    size_t i = 0, width = 0;
    while (TText::next(text, i, width));
    return width;
}

TTextMetrics TText::measure(TStringView text) noexcept
{
    TTextMetrics metrics {};
    size_t i = 0;
    while (true)
    {
        size_t width = 0;
        if (!TText::next(text, i, width))
            break;
        metrics.width += width;
        metrics.characterCount += 1;
        metrics.graphemeCount += (width > 0);
    }
    return metrics;
}

size_t TText::next(TStringView text) noexcept
{
    if (text.size())
        return max(ttext::mblen(text), 1);
    return 0;
}

TText::Lw TText::nextImpl(TStringView text) noexcept
{
    if (text.size())
    {
        auto mb = ttext::mbstat(text);
        if (mb.length <= 1)
            return {1, 1};
        return {
            size_t(mb.length),
            size_t(mb.width ? max(mb.width, 1) : 0),
        };
    }
    return {0, 0};
}

TText::Lw TText::nextImpl(TSpan<const uint32_t> text) noexcept
{
    using namespace tvision;
    if (text.size())
    {
        int width = Platform::charWidth(text[0]);
        return {
            1,
            size_t(width ? max(width, 1) : 0)
        };
    }
    return {0, 0};
}

size_t TText::prev(TStringView text, size_t index) noexcept
{
    if (index)
    {
        // Try reading backwards character by character, until a valid
        // character is found. This tolerates invalid characters.
        size_t lead = min<size_t>(index, 4);
        for (size_t i = 1; i <= lead; ++i)
        {
            int len = ttext::mblen({&text[index - i], i});
            if (len > 0)
                return size_t(len) == i ? i : 1;
        }
        return 1;
    }
    return 0;
}

char TText::toCodePage(TStringView text) noexcept
{
    using namespace tvision;
    size_t length = TText::next(text);
    if (length == 0)
        return '\0';
    if (length == 1 && (text[0] < ' ' || '\x7F' <= text[0]))
        return text[0];
    return CpTranslator::fromUtf8(text.substr(0, length));
}

template <class Text>
inline TText::Lw TText::scrollImplT(Text text, int count, Boolean includeIncomplete) noexcept
{
    if (count > 0)
    {
        size_t i = 0, w = 0;
        while (true)
        {
            size_t i2 = i, w2 = w;
            if (!TText::next(text, i, w) || w == (size_t) count)
                break;
            if (w > (size_t) count)
            {
                if (!includeIncomplete)
                    i = i2, w = w2;
                break;
            }
        }
        return {i, w};
    }
    return {0, 0};
}

TText::Lw TText::scrollImpl(TStringView text, int count, Boolean includeIncomplete) noexcept

{
    return scrollImplT(text, count, includeIncomplete);
}

TText::Lw TText::scrollImpl(TSpan<const uint32_t> text, int count, Boolean includeIncomplete) noexcept

{
    return scrollImplT(text, count, includeIncomplete);
}

namespace ttext
{

static inline bool isZWJ(TStringView mbc)
// We want to avoid printing certain characters which are usually represented
// differently by different terminal applications or which can combine different
// characters together, changing the width of a whole string.
{
    return mbc == "\xE2\x80\x8D"; // U+200D ZERO WIDTH JOINER.
}

} // namespace ttext

TText::Lw TText::drawOneImpl( TSpan<TScreenCell> cells, size_t i,
                              TStringView text, size_t j ) noexcept
{
    using namespace tvision;
    using namespace ttext;
    if (j < text.size())
    {
        auto mb = mbstat(text.substr(j));
        if (mb.length <= 1)
        {
            if (i < cells.size())
            {
                // We need to convert control characters here since we
                // might later try to append combining characters to this.
                if (text[j] == '\0')
                    cells[i]._ch.moveChar(' ');
                else if (text[j] < ' ' || '\x7F' <= text[j])
                    cells[i]._ch.moveInt(CpTranslator::toPackedUtf8(text[j]));
                else
                    cells[i]._ch.moveChar(text[j]);
                return {1, 1};
            }
        }
        else
        {
            if (mb.width < 0)
            {
                if (i < cells.size())
                {
                    cells[i]._ch.moveStr("�");
                    return {(size_t) mb.length, 1};
                }
            }
            else if (mb.width == 0)
            {
                TStringView zwc {&text[j], (size_t) mb.length};
                // Append to the previous cell, if present.
                if (i > 0 && !isZWJ(zwc))
                {
                    size_t k = i;
                    while (cells[--k]._ch.isWideCharTrail() && k > 0);
                    cells[k]._ch.appendZeroWidth(zwc);
                }
                return {(size_t) mb.length, 0};
            }
            else
            {
                if (i < cells.size())
                {
                    bool wide = mb.width > 1;
                    cells[i]._ch.moveStr({&text[j], (size_t) mb.length}, wide);
                    bool drawTrail = (wide && i + 1 < cells.size());
                    if (drawTrail)
                        cells[i + 1]._ch.moveWideCharTrail();
                    return {(size_t) mb.length, size_t(1 + drawTrail)};
                }
            }
        }
    }
    return {0, 0};
}

TText::Lw TText::drawOneImpl( TSpan<TScreenCell> cells, size_t i,
                              TSpan<const uint32_t> textU32, size_t j ) noexcept
{
    using namespace tvision;
    using namespace ttext;
    if (j < textU32.size())
    {
        char utf8[4] = {};
        size_t length = utf32To8(textU32[j], utf8);
        TStringView textU8(utf8, length);
        int width = Platform::charWidth(textU32[j]);
        if (width < 0)
        {
            if (i < cells.size())
            {
                cells[i]._ch.moveStr("�");
                return {1, 1};
            }
        }
        else if (textU32[j] != 0 && width == 0)
        {
            // Append to the previous cell, if present.
            if (i > 0 && !isZWJ(textU8))
            {
                size_t k = i;
                while (cells[--k]._ch.isWideCharTrail() && k > 0);
                cells[k]._ch.appendZeroWidth(textU8);
            }
            return {1, 0};
        }
        else
        {
            if (i < cells.size())
            {
                bool wide = width > 1;
                if (textU32[j] == '\0')
                    cells[i]._ch.moveChar(' ');
                else
                    cells[i]._ch.moveStr(textU8, wide);
                bool drawTrail = (wide && i + 1 < cells.size());
                if (drawTrail)
                    cells[i + 1]._ch.moveWideCharTrail();
                return {1, size_t(1 + drawTrail)};
            }
        }
    }
    return {0, 0};
}
