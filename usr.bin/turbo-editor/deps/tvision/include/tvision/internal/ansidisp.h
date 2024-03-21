#ifndef TVISION_ANSIDISP_H
#define TVISION_ANSIDISP_H

#define Uses_TScreenCell
#include <tvision/tv.h>

#include <internal/termdisp.h>
#include <internal/endian.h>

namespace tvision
{

// TermColor represents a color that is to be printed to screen
// using certain ANSI escape sequences.

struct TermColor
{
    enum TermColorTypes : uint8_t { Default, Indexed, RGB, NoColor };

    union
    {
        uint8_t idx;
        uint8_t bgr[3];
    };
    TermColorTypes type;

    TermColor() = default;

    // GCC has issues optimizing the initialization of this struct.
    // So do bit-casting manually.

    TermColor& operator=(uint32_t val) noexcept
    {
#ifdef TV_BIG_ENDIAN
        reverseBytes(val);
#endif
        memcpy(this, &val, sizeof(*this));
        return *this;
        static_assert(sizeof(*this) == 4, "");
    }
    operator uint32_t() const noexcept
    {
        uint32_t val;
        memcpy(&val, this, sizeof(*this));
#ifdef TV_BIG_ENDIAN
        reverseBytes(val);
#endif
        return val;
    }
    TermColor(uint8_t aIdx, TermColorTypes aType) noexcept
    {
        *this = aIdx | (uint32_t(aType) << 24);
    }
    TermColor(TColorRGB c, TermColorTypes aType) noexcept
    {
        *this = uint32_t(c) | (uint32_t(aType) << 24);
    }
    TermColor(TermColorTypes aType) noexcept
    {
        *this = uint32_t(aType) << 24;
    }

};

struct TermAttr
{
    TermColor fg, bg;
    TColorAttr::Style style;
};

/* AnsiDisplay is a simple diplay backend which prints characters and ANSI
 * escape codes directly to stdout.
 *
 * AnsiDisplay implements only a subset of DisplayStrategy's pure virtual
 * functions, so it depends on another implementation from which it inherits,
 * which is the template parameter. In particular, AnsiDisplay implements the
 * lowlevel<*> functions.
 *
 * This templated inheritance also makes it possible to combine this class
 * with input strategies which depend on a certain display strategy,
 * as is the case of NcursesInput and NcursesDisplay. */

class AnsiDisplayBase
{
    class Buffer
    {
        char *head {nullptr};
        size_t capacity {0};
    public:
        char *tail {nullptr};

        ~Buffer();
        char *data() noexcept;
        size_t size() const noexcept;
        void clear() noexcept;
        void push(TStringView) noexcept;
        void push(char) noexcept;
        void reserve(size_t) noexcept;
    };

    const StdioCtl &io;
    Buffer buf;
    TermAttr lastAttr {};

    void bufWriteCSI1(uint a, char F) noexcept;
    void bufWriteCSI2(uint a, uint b, char F) noexcept;

protected:

    AnsiDisplayBase(const StdioCtl &aIo) noexcept :
        io(aIo)
    {
    }

    ~AnsiDisplayBase();

    void clearAttributes() noexcept;
    void clearScreen() noexcept;

    void lowlevelWriteChars(TStringView chars, TColorAttr attr, const TermCap &) noexcept;
    void lowlevelMoveCursor(uint x, uint y) noexcept;
    void lowlevelMoveCursorX(uint x, uint y) noexcept;
    void lowlevelFlush() noexcept;
};

template<class DisplayBase>
class AnsiDisplay : public DisplayBase, public AnsiDisplayBase
{

public:

    template <typename ...Args>
    AnsiDisplay(Args&& ...args) noexcept :
        DisplayBase(static_cast<Args&&>(args)...),
        AnsiDisplayBase(TerminalDisplay::io)
    {
        static_assert(std::is_base_of<TerminalDisplay, DisplayBase>::value,
            "The base class of AnsiDisplay must be a derived of TerminalDisplay."
        );
    }

    void lowlevelWriteChars(TStringView chars, TColorAttr attr) noexcept override
        { AnsiDisplayBase::lowlevelWriteChars(chars, attr, TerminalDisplay::termcap); }
    void lowlevelMoveCursor(uint x, uint y) noexcept override
        { AnsiDisplayBase::lowlevelMoveCursor(x, y); }
    void lowlevelMoveCursorX(uint x, uint y) noexcept override
        { AnsiDisplayBase::lowlevelMoveCursorX(x, y); }
    void lowlevelFlush() noexcept override
        { AnsiDisplayBase::lowlevelFlush(); }

    void clearScreen() noexcept override
    {
        clearAttributes();
        AnsiDisplayBase::clearScreen();
    }

    void reloadScreenInfo() noexcept override
    {
        DisplayBase::reloadScreenInfo();
        clearAttributes();
    }

};

} // namespace tvision

#endif // TVISION_ANSIDISP_H
