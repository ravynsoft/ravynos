#ifndef TVISION_TERMINAL_TEST_H
#define TVISION_TERMINAL_TEST_H

#include <internal/terminal.h>

inline bool operator==(const TEvent &a, const TEvent &b)
{
    if (a.what != b.what)
        return false;
    if (a.what == evNothing)
        return true;
    if (a.what == evKeyDown)
        return
            a.keyDown.keyCode == b.keyDown.keyCode &&
            a.keyDown.controlKeyState == b.keyDown.controlKeyState &&
            a.keyDown.getText() == b.keyDown.getText();
    if (a.what == evMouse)
        return
            a.mouse.where == b.mouse.where &&
            a.mouse.eventFlags == b.mouse.eventFlags &&
            a.mouse.controlKeyState == b.mouse.controlKeyState &&
            a.mouse.buttons == b.mouse.buttons &&
            a.mouse.wheel == b.mouse.wheel;
    abort();
}

inline std::ostream &operator<<(std::ostream &os, const TEvent &ev)
{
    os << "{";
    if (ev.what == evKeyDown)
    {
        os << "{";
        printKeyCode(os, ev.keyDown.keyCode);
        os << "}, {";
        printControlKeyState(os, ev.keyDown.controlKeyState);
        os << "}, '" << ev.keyDown.getText() << "'";
    }
    else if (ev.what == evMouse)
    {
        os << "(" << ev.mouse.where.x << "," << ev.mouse.where.y << ")";
        os << ", ";
        printMouseEventFlags(os, ev.mouse.eventFlags);
        os << ", ";
        printControlKeyState(os, ev.mouse.controlKeyState);
        os << ", ";
        printMouseButtonState(os, ev.mouse.buttons);
        os << ", ";
        printMouseWheelState(os, ev.mouse.wheel);
    }
    os << "}";
    return os;
}

namespace tvision
{

class StrInputGetter : public InputGetter
{
    TStringView str;
    size_t i {0};

public:

    StrInputGetter(TStringView aStr) noexcept :
        str(aStr)
    {
    }

    int get() noexcept override
    {
        return i < str.size() ? str[i++] : -1;
    }

    void unget(int) noexcept override
    {
        if (i > 0)
            --i;
    }

    int bytesLeft() noexcept
    {
        return str.size() - i;
    }
};

struct ParseResultEvent
{
    ParseResult parseResult;
    TEvent ev;
};

inline bool operator==(const ParseResultEvent &a, const ParseResultEvent &b)
{
    if (a.parseResult != b.parseResult)
        return false;
    if (a.parseResult == Ignored)
        return true;
    return a.ev == b.ev;
}

inline std::ostream &operator<<(std::ostream &os, const ParseResultEvent &p)
{
    os << "{";
    switch (p.parseResult)
    {
        case Rejected: os << "Rejected"; break;
        case Ignored: os << "Ignored"; break;
        case Accepted:
        {
            os << "Accepted, {";
            printEventCode(os, p.ev.what);
            os << ", " << p.ev << "}";
        }
    }
    os << "}";
    return os;
}

constexpr TEvent keyDownEv(ushort keyCode, ushort controlKeyState, TStringView text)
{
    TEvent ev {};
    ev.what = evKeyDown;
    ev.keyDown.keyCode = keyCode;
    ev.keyDown.controlKeyState = controlKeyState;
    while (ev.keyDown.textLength <= sizeof(ev.keyDown.text) && ev.keyDown.textLength < text.size())
    {
        ev.keyDown.text[ev.keyDown.textLength] = text[ev.keyDown.textLength];
        ++ev.keyDown.textLength;
    }
    return ev;
}

constexpr TEvent mouseEv(TPoint where, ushort eventFlags, ushort controlKeyState, uchar buttons, uchar wheel)
{
    TEvent ev {};
    ev.what = evMouse;
    ev.mouse.where = where;
    ev.mouse.eventFlags = eventFlags;
    ev.mouse.controlKeyState = controlKeyState;
    ev.mouse.buttons = buttons;
    ev.mouse.wheel = wheel;
    return ev;
}

} // namespace tvision

#endif // TVISION_TERMINAL_TEST_H
