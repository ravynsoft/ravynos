#define Uses_TEvent
#define Uses_TKeys
#define Uses_THardwareInfo
#include <tvision/tv.h>
#include <internal/win32con.h>
#include <internal/stdioctl.h>
#include <internal/winwidth.h>
#include <internal/codepage.h>
#include <internal/ansidisp.h>
#include <internal/terminal.h>
#include <internal/utf8.h>
#include <locale.h>

namespace tvision
{

#ifdef _WIN32

static bool isWine() noexcept
{
    return GetProcAddress(GetModuleHandleW(L"ntdll"), "wine_get_version");
}

Win32ConsoleStrategy &Win32ConsoleStrategy::create() noexcept
{
    auto &io = StdioCtl::getInstance();
    // Set the input mode.
    {
        DWORD consoleMode = 0;
        GetConsoleMode(io.in(), &consoleMode);
        consoleMode |= ENABLE_WINDOW_INPUT; // Report changes in buffer size
        consoleMode &= ~ENABLE_PROCESSED_INPUT; // Report CTRL+C and SHIFT+Arrow events.
        consoleMode |= ENABLE_EXTENDED_FLAGS;   /* Disable the Quick Edit mode, */
        consoleMode &= ~ENABLE_QUICK_EDIT_MODE; /* which inhibits the mouse.    */
        SetConsoleMode(io.in(), consoleMode);
    }
    // Set the output mode.
    bool supportsVT;
    {
        DWORD consoleMode = 0;
        GetConsoleMode(io.out(), &consoleMode);
        consoleMode &= ~ENABLE_WRAP_AT_EOL_OUTPUT; // Avoid scrolling when reaching end of line.
        SetConsoleMode(io.out(), consoleMode);
        // Try enabling VT sequences.
        if (isWine())
            // Wine does not support them, but unlike the legacy console,
            // it does not return error when attempting to enable it, so we
            // have to handle this case separately.
            supportsVT = false;
        else
        {
            consoleMode |= DISABLE_NEWLINE_AUTO_RETURN; // Do not do CR on LF.
            consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // Allow ANSI escape sequences.
            SetConsoleMode(io.out(), consoleMode);
            GetConsoleMode(io.out(), &consoleMode);
            supportsVT = consoleMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        }
    }

    // Set the console and the environment in UTF-8 mode.
    UINT cpInput = GetConsoleCP();
    UINT cpOutput = GetConsoleOutputCP();
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".utf8"); // Note that this must be done again after SetConsoleCP().
    if (!supportsVT)
    {
        // Disable bitmap font in legacy console because multibyte characters
        // are not displayed correctly.
        CONSOLE_FONT_INFOEX fontInfo {};
        fontInfo.cbSize = sizeof(fontInfo);
        auto isBitmap = [](UINT family)
        {
            // https://docs.microsoft.com/en-us/windows/console/console-font-infoex
            // "FontFamily: see the description of the tmPitchAndFamily member
            //  of the TEXTMETRIC structure."
            // https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-textmetricw
            // "A monospace bitmap font has all of these low-order bits clear".
            return !(family & (TMPF_FIXED_PITCH | TMPF_VECTOR | TMPF_TRUETYPE | TMPF_DEVICE));
        };
        if ( GetCurrentConsoleFontEx(io.out(), FALSE, &fontInfo)
             && isBitmap(fontInfo.FontFamily) )
        {
            // Compute the new font height based on the bitmap font size.
            short fontY = 2*min(fontInfo.dwFontSize.X, fontInfo.dwFontSize.Y);
            for (auto *name : {L"Consolas", L"Lucida Console"})
            {
                fontInfo.nFont = 0;
                fontInfo.FontFamily = FF_DONTCARE;
                fontInfo.FontWeight = FW_NORMAL;
                fontInfo.dwFontSize = {0, fontY}; // Width estimated automatically, it seems.
                wcscpy(fontInfo.FaceName, name);
                // SetCurrentConsoleFontEx succeeds even if the font is not available.
                // We need to check whether the font has actually been set.
                SetCurrentConsoleFontEx(io.out(), FALSE, &fontInfo);
                GetCurrentConsoleFontEx(io.out(), FALSE, &fontInfo);
                if (wcscmp(fontInfo.FaceName, name) == 0)
                    break;
            }
        }
    }
    WinWidth::reset();
    auto &display = supportsVT ? *new AnsiDisplay<Win32Display>(io)
                               : *new Win32Display(io);
    auto &input = *new Win32Input(io);
    return *new Win32ConsoleStrategy(io, cpInput, cpOutput, display, input);
}

Win32ConsoleStrategy::~Win32ConsoleStrategy()
{
    delete &display;
    delete &input;
    SetConsoleCP(cpInput);
    SetConsoleOutputCP(cpOutput);
    StdioCtl::destroyInstance();
}

bool Win32ConsoleStrategy::isAlive() noexcept
{
    DWORD events = 0;
    return GetNumberOfConsoleInputEvents(io.in(), &events);
}

static bool openClipboard() noexcept
{
    for (int i = 0; i < 5; ++i)
    {
        if (OpenClipboard(nullptr))
            return true;
        Sleep(5);
    }
    return false;
}

bool Win32ConsoleStrategy::setClipboardText(TStringView text) noexcept
{
    bool result = false;
    if (openClipboard())
    {
        HGLOBAL hData = NULL;
        wchar_t *pData;
        int dataLen;
        if ( EmptyClipboard() &&
             !(result = text.empty()) &&
             (dataLen = MultiByteToWideChar(CP_UTF8, 0, text.data(), text.size(), nullptr, 0)) &&
             (hData = GlobalAlloc(GMEM_MOVEABLE, (dataLen + 1)*sizeof(wchar_t))) &&
             (pData = (wchar_t *) GlobalLock(hData))
           )
        {
            MultiByteToWideChar(CP_UTF8, 0, text.data(), text.size(), pData, dataLen);
            pData[dataLen] = L'\0';
            GlobalUnlock(hData);
            result = SetClipboardData(CF_UNICODETEXT, hData);
        }
        CloseClipboard();
        if (hData && !result)
            GlobalFree(hData);
    }
    return result;
}

bool Win32ConsoleStrategy::requestClipboardText(void (&accept)(TStringView)) noexcept
{
    bool result = false;
    if (openClipboard())
    {
        HGLOBAL hData;
        wchar_t *pData;
        if ( (hData = GetClipboardData(CF_UNICODETEXT)) &&
             (result = (pData = (wchar_t *) GlobalLock(hData))) )
        {
            size_t dataLen = wcslen(pData);
            int textLen = WideCharToMultiByte(CP_UTF8, 0, pData, dataLen, nullptr, 0, nullptr, nullptr);
            char *text = new char[textLen];
            WideCharToMultiByte(CP_UTF8, 0, pData, dataLen, text, textLen, nullptr, nullptr);
            GlobalUnlock(hData);
            accept({text, size_t(textLen)});
            delete[] text;
        }
        CloseClipboard();
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////
// Win32Input

int Win32Input::getButtonCount() noexcept
{
    DWORD num;
    GetNumberOfConsoleMouseButtons(&num);
    return num;
}

void Win32Input::cursorOn() noexcept
{
    DWORD consoleMode = 0;
    GetConsoleMode(io.in(), &consoleMode);
    SetConsoleMode(io.in(), consoleMode | ENABLE_MOUSE_INPUT);
}

void Win32Input::cursorOff() noexcept
{
    DWORD consoleMode = 0;
    GetConsoleMode(io.in(), &consoleMode);
    SetConsoleMode(io.in(), consoleMode & ~ENABLE_MOUSE_INPUT);
}

bool Win32Input::getEvent(TEvent &ev) noexcept
{
    // ReadConsoleInput can sleep the process, so we first check the number
    // of available input events.
    DWORD events;
    while (GetNumberOfConsoleInputEvents(io.in(), &events) && events)
    {
        // getEvent(ir, ev) often returns false due to discarded events. But this
        // function should not return false if there are pending events, as that
        // defeats the event queue in THardwareInfo.
        while (events--)
        {
            INPUT_RECORD ir;
            DWORD ok;
            if (!ReadConsoleInputW(io.in(), &ir, 1, &ok) || !ok)
                return false;
            if (getEvent(ir, ev))
                return true;
        }
    }
    return false;
}

bool Win32Input::getEvent(const INPUT_RECORD &ir, TEvent &ev) noexcept
{
    switch (ir.EventType)
    {
    case KEY_EVENT:
        if ( ir.Event.KeyEvent.bKeyDown || // KeyDown
             (ir.Event.KeyEvent.wVirtualKeyCode == VK_MENU && // Pasted surrogate character
                ir.Event.KeyEvent.uChar.UnicodeChar) )
            return getWin32Key(ir.Event.KeyEvent, ev, state);
        break;
    case MOUSE_EVENT:
        getWin32Mouse(ir.Event.MouseEvent, ev, state);
        return true;
    case WINDOW_BUFFER_SIZE_EVENT:
        ev.what = evCommand;
        ev.message.command = cmScreenChanged;
        ev.message.infoPtr = 0;
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////
// Win32Display

void Win32Display::reloadScreenInfo() noexcept
{
    size = io.getSize();
    CONSOLE_SCREEN_BUFFER_INFO sbInfo {};
    GetConsoleScreenBufferInfo(io.out(), &sbInfo);
    // Set the cursor temporally to (0, 0) to prevent the console from crashing
    // due to https://github.com/microsoft/terminal/issues/7511.
    auto curPos = sbInfo.dwCursorPosition;
    SetConsoleCursorPosition(io.out(), {0, 0});
    // Make sure the buffer size matches the viewport size so that the
    // scrollbars are not shown.
    SetConsoleScreenBufferSize(io.out(), {(short) size.x, (short) size.y});
    // Restore the cursor position (it does not matter if it is out of bounds).
    SetConsoleCursorPosition(io.out(), curPos);
}

bool Win32Display::screenChanged() noexcept
{
    bool changed = TerminalDisplay::screenChanged();
    CONSOLE_FONT_INFO fontInfo;
    if ( GetCurrentConsoleFont(io.out(), FALSE, &fontInfo)
         && memcmp(&fontInfo, &lastFontInfo, sizeof(fontInfo)) != 0 )
    {
        changed = true;
        WinWidth::reset();
        lastFontInfo = fontInfo;
    }
    return changed;
}

TPoint Win32Display::getScreenSize() noexcept
{
    return size;
}

int Win32Display::getCaretSize() noexcept
{
    CONSOLE_CURSOR_INFO crInfo {};
    GetConsoleCursorInfo(io.out(), &crInfo);
    return crInfo.bVisible ? crInfo.dwSize : 0;
}

int Win32Display::getColorCount() noexcept
{
    // Conhost has had high color support for some time:
    // https://devblogs.microsoft.com/commandline/24-bit-color-in-the-windows-console/
    DWORD consoleMode = 0;
    GetConsoleMode(io.out(), &consoleMode);
    if (consoleMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
        return 256*256*256;
    return 16;
}

void Win32Display::lowlevelCursorSize(int size) noexcept
{
    CONSOLE_CURSOR_INFO crInfo;
    if (size) {
        crInfo.bVisible = TRUE;
        crInfo.dwSize = size;
    } else {
        crInfo.bVisible = FALSE;
        crInfo.dwSize = 1;
    }
    SetConsoleCursorInfo(io.out(), &crInfo);
}

void Win32Display::clearScreen() noexcept
{
    COORD coord = {0, 0};
    DWORD length = size.x * size.y;
    BYTE attr = 0x07;
    DWORD read;
    FillConsoleOutputAttribute(io.out(), attr, length, coord, &read);
    FillConsoleOutputCharacterA(io.out(), ' ', length, coord, &read);
    lastAttr = attr;
}

// Fallback display support with rudimentary buffering.

void Win32Display::lowlevelWriteChars(TStringView chars, TColorAttr attr) noexcept
{
    uchar bios = attr.toBIOS();
    if (bios != lastAttr)
    {
        lowlevelFlush();
        SetConsoleTextAttribute(io.out(), bios);
        lastAttr = bios;
    }
    buf.insert(buf.end(), chars.data(), chars.data()+chars.size());
}

void Win32Display::lowlevelMoveCursor(uint x, uint y) noexcept
{
    lowlevelFlush();
    SetConsoleCursorPosition(io.out(), {(short) x, (short) y});
}

void Win32Display::lowlevelFlush() noexcept
{
    io.write(buf.data(), buf.size());
    buf.resize(0);
}

#endif // _WIN32

/////////////////////////////////////////////////////////////////////////
// Global functions

static bool getWin32KeyText(const KEY_EVENT_RECORD &KeyEvent, TEvent &ev, InputState &state) noexcept
// Returns true unless the event contains a UTF-16 surrogate (Windows only),
// in which case we need the next event.
{
    uint32_t ch = KeyEvent.uChar.UnicodeChar;
    ev.keyDown.textLength = 0;

    // Do not treat non-printable characters as text.
    if (' ' <= ch && ch != 0x7F)
    {
#ifdef _WIN32
        if (0xD800 <= ch && ch <= 0xDBFF)
        {
            state.surrogate = ch;
            return false;
        }

        wchar_t utf16[2] = {(wchar_t) ch, 0};
        if (state.surrogate)
        {
            if (0xDC00 <= ch && ch <= 0xDFFF)
            {
                utf16[1] = (wchar_t) ch;
                utf16[0] = state.surrogate;
            }
            state.surrogate = 0;
        }

        ev.keyDown.textLength = WideCharToMultiByte(
            CP_UTF8, 0,
            utf16, utf16[1] ? 2 : 1,
            ev.keyDown.text, sizeof(ev.keyDown.text),
            nullptr, nullptr
        );
#else
        (void) state;

        if (ch < 0xD800 || (0xDFFF < ch && ch < 0x10FFFF))
            ev.keyDown.textLength = (uchar) utf32To8(ch, ev.keyDown.text);
#endif // _WIN32
    }
    return true;
}

bool getWin32Key(const KEY_EVENT_RECORD &KeyEvent, TEvent &ev, InputState &state) noexcept
{
    if (!getWin32KeyText(KeyEvent, ev, state))
        return false;

    ev.what = evKeyDown;
    ev.keyDown.charScan.scanCode = KeyEvent.wVirtualScanCode;
    ev.keyDown.charScan.charCode = KeyEvent.uChar.AsciiChar;
    ev.keyDown.controlKeyState = KeyEvent.dwControlKeyState & (
        kbShift | kbCtrlShift | kbAltShift |
        kbScrollState | kbNumState | kbCapsState | kbEnhanced
    );

    if (ev.keyDown.textLength)
    {
        ev.keyDown.charScan.charCode = CpTranslator::fromUtf8(ev.keyDown.getText());
        if (KeyEvent.wVirtualKeyCode == VK_MENU)
            // This is enabled when pasting certain characters, and it confuses
            // applications. Clear it.
            ev.keyDown.charScan.scanCode = 0;
        if (!ev.keyDown.charScan.charCode || ev.keyDown.keyCode <= kbCtrlZ)
            // If the character cannot be represented in the current codepage,
            // or if it would accidentally trigger a Ctrl+Key combination,
            // make the whole keyCode zero to avoid side effects.
            ev.keyDown.keyCode = kbNoKey;
    }

    if ( ev.keyDown.keyCode == 0x2A00 || ev.keyDown.keyCode == 0x1D00 ||
         ev.keyDown.keyCode == 0x3600 || ev.keyDown.keyCode == 0x3800 ||
         ev.keyDown.keyCode == 0x3A00 )
        // Discard standalone Shift, Ctrl, Alt, Caps Lock keys.
        ev.keyDown.keyCode = kbNoKey;
    else if ( (ev.keyDown.controlKeyState & kbCtrlShift) &&
              (ev.keyDown.controlKeyState & kbAltShift) ) // Ctrl+Alt is AltGr.
    {
        // When AltGr+Key does not produce a character, a
        // keyCode with unwanted effects may be read instead.
        if (!ev.keyDown.textLength)
            ev.keyDown.keyCode = kbNoKey;
    }
    else if (KeyEvent.wVirtualScanCode < 89)
    {
        // Convert NT style virtual scan codes to PC BIOS codes.
        uchar index = KeyEvent.wVirtualScanCode;
        ushort keyCode = 0;
        if ((ev.keyDown.controlKeyState & kbAltShift) && THardwareInfo::AltCvt[index])
            keyCode = THardwareInfo::AltCvt[index];
        else if ((ev.keyDown.controlKeyState & kbCtrlShift) && THardwareInfo::CtrlCvt[index])
            keyCode = THardwareInfo::CtrlCvt[index];
        else if ((ev.keyDown.controlKeyState & kbShift) && THardwareInfo::ShiftCvt[index])
            keyCode = THardwareInfo::ShiftCvt[index];
        else if ( !(ev.keyDown.controlKeyState & (kbShift | kbCtrlShift | kbAltShift)) &&
                  THardwareInfo::NormalCvt[index] )
            keyCode = THardwareInfo::NormalCvt[index];

        if (keyCode != 0)
        {
            ev.keyDown.keyCode = keyCode;
            if (ev.keyDown.charScan.charCode < ' ')
                ev.keyDown.textLength = 0;
            else if (ev.keyDown.charScan.charCode < 0x7F && !ev.keyDown.textLength)
            {
                ev.keyDown.text[0] = ev.keyDown.charScan.charCode;
                ev.keyDown.textLength = 1;
            }
        }
    }

    return ev.keyDown.keyCode != kbNoKey || ev.keyDown.textLength;
}

void getWin32Mouse(const MOUSE_EVENT_RECORD &MouseEvent, TEvent &ev, InputState &state) noexcept
{
    ev.what = evMouse;
    ev.mouse.where.x = MouseEvent.dwMousePosition.X;
    ev.mouse.where.y = MouseEvent.dwMousePosition.Y;
    ev.mouse.buttons = state.buttons = MouseEvent.dwButtonState;
    ev.mouse.eventFlags = MouseEvent.dwEventFlags;
    ev.mouse.controlKeyState = MouseEvent.dwControlKeyState & (
        kbShift | kbCtrlShift | kbAltShift |
        kbScrollState | kbNumState | kbCapsState | kbEnhanced
    );

    // Rotation sense is represented by the sign of dwButtonState's high word
    Boolean positive = !(MouseEvent.dwButtonState & 0x80000000);
    if( MouseEvent.dwEventFlags & MOUSE_WHEELED )
        ev.mouse.wheel = positive ? mwUp : mwDown;
    else if( MouseEvent.dwEventFlags & MOUSE_HWHEELED )
        ev.mouse.wheel = positive ? mwRight : mwLeft;
    else
        ev.mouse.wheel = 0;
}

} // namespace tvision

