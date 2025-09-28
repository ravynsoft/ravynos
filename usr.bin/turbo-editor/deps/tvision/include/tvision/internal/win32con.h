#ifndef WIN32CON_H
#define WIN32CON_H

#include <tvision/tv.h>
#include <compat/windows/windows.h>
#include <internal/stdioctl.h>
#include <internal/termdisp.h>
#include <internal/terminal.h>

namespace tvision
{

bool getWin32Key(const KEY_EVENT_RECORD &, TEvent &, InputState &) noexcept;
void getWin32Mouse(const MOUSE_EVENT_RECORD &, TEvent &, InputState &) noexcept;

#ifdef _WIN32

class Win32Input;
class Win32Display;

class Win32ConsoleStrategy final : public ConsoleStrategy
{
    StdioCtl &io;
    UINT cpInput, cpOutput;

    Win32ConsoleStrategy( StdioCtl &aIo,
                          UINT cpInput, UINT cpOutput,
                          DisplayStrategy &aDisplay,
                          InputStrategy &aInput ) noexcept :
        ConsoleStrategy(aDisplay, aInput, {&aInput}),
        io(aIo),
        cpInput(cpInput),
        cpOutput(cpOutput)
    {
    }

    ~Win32ConsoleStrategy();

    bool isAlive() noexcept override;
    bool setClipboardText(TStringView) noexcept override;
    bool requestClipboardText(void (&)(TStringView)) noexcept override;

public:

    static Win32ConsoleStrategy &create() noexcept;
    static int charWidth(uint32_t) noexcept;
};

class Win32Input final : public InputStrategy
{
    StdioCtl &io;
    InputState state;

    bool getEvent(const INPUT_RECORD &, TEvent &ev) noexcept;
    bool getMouseEvent(MOUSE_EVENT_RECORD, TEvent &ev) noexcept;

public:

    // The lifetime of 'aIo' must exceed that of 'this'.
    Win32Input(StdioCtl &aIo) noexcept :
        InputStrategy(aIo.in()),
        io(aIo)
    {
    }

    bool getEvent(TEvent &ev) noexcept override;
    int getButtonCount() noexcept override;
    void cursorOn() noexcept override;
    void cursorOff() noexcept override;
};

class Win32Display : public TerminalDisplay
{
    TPoint size {};
    uchar lastAttr {'\x00'};
    std::vector<char> buf;
    CONSOLE_FONT_INFO lastFontInfo {};

public:

    // The lifetime of 'aIo' must exceed that of 'this'.
    Win32Display(StdioCtl &aIo) noexcept :
        TerminalDisplay(aIo)
    {
        initCapabilities();
    }

    void reloadScreenInfo() noexcept override;
    TPoint getScreenSize() noexcept override;
    int getCaretSize() noexcept override;
    int getColorCount() noexcept override;
    void clearScreen() noexcept override;
    bool screenChanged() noexcept override;

protected:

    void lowlevelWriteChars(TStringView chars, TColorAttr attr) noexcept override;
    void lowlevelMoveCursor(uint x, uint y) noexcept override;
    void lowlevelCursorSize(int size) noexcept override;
    void lowlevelFlush() noexcept override;
};

#endif // _WIN32

} // namespace tvision

#endif // WIN32CON_H
