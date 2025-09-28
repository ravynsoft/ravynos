#ifdef HAVE_NCURSES

#define Uses_THardwareInfo
#define Uses_TKeys
#define Uses_TEvent
#include <tvision/tv.h>

#include <ncurses.h>
#include <internal/ncursinp.h>
#include <internal/getenv.h>
#include <internal/utf8.h>
#include <internal/stdioctl.h>
#include <internal/codepage.h>
#include <internal/constmap.h>
#include <string>
#include <chrono>

using std::chrono::milliseconds;
using std::chrono::steady_clock;

namespace tvision
{

/* Turbo Vision is designed to work with BIOS key codes. Mnemonics for some
 * key codes are defined in tkeys.h. Unless those are changed, it is
 * necessary to translate ncurses keys to key codes. */

/* Turbo Vision stores key events in a KeyDownEvent struct, defined in
 * system.h. Its first field is a key code (16 bit), which can be decomposed
 * into the ASCII equivalent (lower byte) and a scan code (higher byte).
 * It has a second field with the state of the modifier keys, which can be
 * retrieved by performing a bit-wise AND with the kbShift, kbCtrlShift and
 * kbAltShift bitmasks. Turbo Vision expects this field to be filled even
 * if the key code is already named Shift/Ctrl/Alt+something. */

/* The support for key combinations is the following:
   - PrintScreen, Break are not likely to be captured by the terminal, but
     Ctrl+C could be used as a replacement of the Ctrl+Break interrupt.
   - Ctrl/Alt+F(n) don't work on the linux console and I strongly advice against
     using them.
   - Ctrl+Letter works, except for ^H, ^I, ^J and ^M, which have a special
     meaning.
   - Alt+Letter/Number seem to work quite well.
   - Ctrl+Backspace/Enter can't be recognized on terminal emulators.
   - Shift/Ctrl+Ins/Del/Home/End/PgDn/PgUp seem to work, too.
   - Arrow keys work, as well as combined with Shift, but Turbo Vision doesn't
     support Ctrl+Up/Down (EDIT: it now does).
   - Tab and Backtab are supported too, although the linux console confuses the
     latter with Alt+Tab.
   - Some other key combinations are supported on terminal but not in Turbo Vision.
 * Still, it's up to your luck that ncurses manages to grab any of these
 * combinations from your terminal application. */

static constexpr KeyDownEvent fromNonPrintableAscii[32] =
{
    {{'@'},         kbLeftCtrl, {'@'}, 1}, // ^@, Null
    {{kbCtrlA},     kbLeftCtrl},
    {{kbCtrlB},     kbLeftCtrl},
    {{kbCtrlC},     kbLeftCtrl},
    {{kbCtrlD},     kbLeftCtrl},
    {{kbCtrlE},     kbLeftCtrl},
    {{kbCtrlF},     kbLeftCtrl},
    {{kbCtrlG},     kbLeftCtrl},
    {{kbBack},      0}, // ^H, Backspace
    {{kbTab},       0}, // ^I, Tab
    {{kbEnter},     0}, // ^J, Line Feed
    {{kbCtrlK},     kbLeftCtrl},
    {{kbCtrlL},     kbLeftCtrl},
    {{kbEnter},     0}, // ^M, Carriage Return
    {{kbCtrlN},     kbLeftCtrl},
    {{kbCtrlO},     kbLeftCtrl},
    {{kbCtrlP},     kbLeftCtrl},
    {{kbCtrlQ},     kbLeftCtrl},
    {{kbCtrlR},     kbLeftCtrl},
    {{kbCtrlS},     kbLeftCtrl},
    {{kbCtrlT},     kbLeftCtrl},
    {{kbCtrlU},     kbLeftCtrl},
    {{kbCtrlV},     kbLeftCtrl},
    {{kbCtrlW},     kbLeftCtrl},
    {{kbCtrlX},     kbLeftCtrl},
    {{kbCtrlY},     kbLeftCtrl},
    {{kbCtrlZ},     kbLeftCtrl},
    {{kbEsc},       0}, // ^[, Escape
    {{'\\'},        kbLeftCtrl, {'\\'}, 1}, // ^\, File Separator
    {{']'},         kbLeftCtrl, {']'}, 1}, // ^], Group Separator
    {{'^'},         kbLeftCtrl, {'^'}, 1}, // ^^, Record Separator
    {{'_'},         kbLeftCtrl, {'_'}, 1}, // ^_, Unit Separator
};

static const const_unordered_map<ushort, KeyDownEvent> fromCursesKeyCode =
{
    { KEY_DOWN,         {{kbDown},      0}          },
    { KEY_UP,           {{kbUp},        0}          },
    { KEY_LEFT,         {{kbLeft},      0}          },
    { KEY_RIGHT,        {{kbRight},     0}          },
    { KEY_HOME,         {{kbHome},      0}          },
    { KEY_BACKSPACE,    {{kbBack},      0}          },
    { KEY_DC,           {{kbDel},       0}          },
    { KEY_IC,           {{kbIns},       0}          },
    { KEY_SF,           {{kbDown},      kbShift}    },
    { KEY_SR,           {{kbUp},        kbShift}    },
    { KEY_NPAGE,        {{kbPgDn},      0}          },
    { KEY_PPAGE,        {{kbPgUp},      0}          },
    { KEY_ENTER,        {{kbEnter},     0}          },
    { KEY_BTAB,         {{kbShiftTab},  kbShift}    },
    { KEY_END,          {{kbEnd},       0}          },
    { KEY_SDC,          {{kbShiftDel},  kbShift}    },
    { KEY_SEND,         {{kbEnd},       kbShift}    },
    { KEY_SHOME,        {{kbHome},      kbShift}    },
    { KEY_SIC,          {{kbShiftIns},  kbShift}    },
    { KEY_SLEFT,        {{kbLeft},      kbShift}    },
    { KEY_SRIGHT,       {{kbRight},     kbShift}    },
    { KEY_SUSPEND,      {{kbCtrlZ},     kbLeftCtrl} },
    // Avoid using these, as they are reserved by the Linux console.
    { KEY_SPREVIOUS,    {{kbPgUp},      kbShift}    },
    { KEY_SNEXT,        {{kbPgDn},      kbShift}    },
    // Keypad
    { KEY_A1,           {{kbHome},      0}          },
    { KEY_A3,           {{kbPgUp},      0}          },
    { KEY_C1,           {{kbEnd},       0}          },
    { KEY_C3,           {{kbPgDn},      0}          },
    // Function keys F1-F12
    { KEY_F0 + 1,       {{kbF1},        0}          },
    { KEY_F0 + 2,       {{kbF2},        0}          },
    { KEY_F0 + 3,       {{kbF3},        0}          },
    { KEY_F0 + 4,       {{kbF4},        0}          },
    { KEY_F0 + 5,       {{kbF5},        0}          },
    { KEY_F0 + 6,       {{kbF6},        0}          },
    { KEY_F0 + 7,       {{kbF7},        0}          },
    { KEY_F0 + 8,       {{kbF8},        0}          },
    { KEY_F0 + 9,       {{kbF9},        0}          },
    { KEY_F0 + 10,      {{kbF10},       0}          },
    { KEY_F0 + 11,      {{kbF11},       0}          },
    { KEY_F0 + 12,      {{kbF12},       0}          },
    // Shift+F1-F12
    { KEY_F0 + 13,      {{kbShiftF1},   kbShift}    },
    { KEY_F0 + 14,      {{kbShiftF2},   kbShift}    },
    { KEY_F0 + 15,      {{kbShiftF3},   kbShift}    },
    { KEY_F0 + 16,      {{kbShiftF4},   kbShift}    },
    { KEY_F0 + 17,      {{kbShiftF5},   kbShift}    },
    { KEY_F0 + 18,      {{kbShiftF6},   kbShift}    },
    { KEY_F0 + 19,      {{kbShiftF7},   kbShift}    },
    { KEY_F0 + 20,      {{kbShiftF8},   kbShift}    },
    { KEY_F0 + 21,      {{kbShiftF9},   kbShift}    },
    { KEY_F0 + 22,      {{kbShiftF10},  kbShift}    },
    { KEY_F0 + 23,      {{kbShiftF11},  kbShift}    },
    { KEY_F0 + 24,      {{kbShiftF12},  kbShift}    },
    /* Linux console support for function keys ends here, so please
     * avoid using any of the following: */
    // Ctrl+F1-F12
    { KEY_F0 + 25,      {{kbCtrlF1},    kbLeftCtrl} },
    { KEY_F0 + 26,      {{kbCtrlF2},    kbLeftCtrl} },
    { KEY_F0 + 27,      {{kbCtrlF3},    kbLeftCtrl} },
    { KEY_F0 + 28,      {{kbCtrlF4},    kbLeftCtrl} },
    { KEY_F0 + 29,      {{kbCtrlF5},    kbLeftCtrl} },
    { KEY_F0 + 30,      {{kbCtrlF6},    kbLeftCtrl} },
    { KEY_F0 + 31,      {{kbCtrlF7},    kbLeftCtrl} },
    { KEY_F0 + 32,      {{kbCtrlF8},    kbLeftCtrl} },
    { KEY_F0 + 33,      {{kbCtrlF9},    kbLeftCtrl} },
    { KEY_F0 + 34,      {{kbCtrlF10},   kbLeftCtrl} },
    { KEY_F0 + 35,      {{kbCtrlF11},   kbLeftCtrl} },
    { KEY_F0 + 36,      {{kbCtrlF12},   kbLeftCtrl} },
    // Ctrl+Shift+F1-12
    { KEY_F0 + 37,      {{kbCtrlF1},    kbShift | kbLeftCtrl}},
    { KEY_F0 + 38,      {{kbCtrlF2},    kbShift | kbLeftCtrl}},
    { KEY_F0 + 39,      {{kbCtrlF3},    kbShift | kbLeftCtrl}},
    { KEY_F0 + 40,      {{kbCtrlF4},    kbShift | kbLeftCtrl}},
    { KEY_F0 + 41,      {{kbCtrlF5},    kbShift | kbLeftCtrl}},
    { KEY_F0 + 42,      {{kbCtrlF6},    kbShift | kbLeftCtrl}},
    { KEY_F0 + 43,      {{kbCtrlF7},    kbShift | kbLeftCtrl}},
    { KEY_F0 + 44,      {{kbCtrlF8},    kbShift | kbLeftCtrl}},
    { KEY_F0 + 45,      {{kbCtrlF9},    kbShift | kbLeftCtrl}},
    { KEY_F0 + 46,      {{kbCtrlF10},   kbShift | kbLeftCtrl}},
    { KEY_F0 + 47,      {{kbCtrlF11},   kbShift | kbLeftCtrl}},
    { KEY_F0 + 48,      {{kbCtrlF12},   kbShift | kbLeftCtrl}},
    // Alt+F1-F12
    { KEY_F0 + 49,      {{kbAltF1},     kbLeftAlt}  },
    { KEY_F0 + 50,      {{kbAltF2},     kbLeftAlt}  },
    { KEY_F0 + 51,      {{kbAltF3},     kbLeftAlt}  },
    { KEY_F0 + 52,      {{kbAltF4},     kbLeftAlt}  },
    { KEY_F0 + 53,      {{kbAltF5},     kbLeftAlt}  },
    { KEY_F0 + 54,      {{kbAltF6},     kbLeftAlt}  },
    { KEY_F0 + 55,      {{kbAltF7},     kbLeftAlt}  },
    { KEY_F0 + 56,      {{kbAltF8},     kbLeftAlt}  },
    { KEY_F0 + 57,      {{kbAltF9},     kbLeftAlt}  },
    { KEY_F0 + 58,      {{kbAltF10},    kbLeftAlt}  },
    { KEY_F0 + 59,      {{kbAltF11},    kbLeftAlt}  },
    { KEY_F0 + 60,      {{kbAltF12},    kbLeftAlt}  }
};

static const auto fromCursesHighKey =
    const_unordered_map<uint64_t, KeyDownEvent>::with_string_keys({
    /* These keys are identified by name. The int value is not known
     * at compilation time. */
    { "kDC3",       {{kbAltDel},        kbLeftAlt} },
    { "kEND3",      {{kbAltEnd},        kbLeftAlt} },
    { "kHOM3",      {{kbAltHome},       kbLeftAlt} },
    { "kIC3",       {{kbAltIns},        kbLeftAlt} },
    { "kLFT3",      {{kbAltLeft},       kbLeftAlt} },
    { "kNXT3",      {{kbAltPgDn},       kbLeftAlt} },
    { "kPRV3",      {{kbAltPgUp},       kbLeftAlt} },
    { "kRIT3",      {{kbAltRight},      kbLeftAlt} },
    { "kUP3",       {{kbAltUp},         kbLeftAlt} },
    { "kDN3",       {{kbAltDown},       kbLeftAlt} },
    { "kDC4",       {{kbAltDel},        kbShift | kbLeftAlt} },
    { "kEND4",      {{kbAltEnd},        kbShift | kbLeftAlt} },
    { "kHOM4",      {{kbAltHome},       kbShift | kbLeftAlt} },
    { "kIC4",       {{kbAltIns},        kbShift | kbLeftAlt} },
    { "kLFT4",      {{kbAltLeft},       kbShift | kbLeftAlt} },
    { "kNXT4",      {{kbAltPgDn},       kbShift | kbLeftAlt} },
    { "kPRV4",      {{kbAltPgUp},       kbShift | kbLeftAlt} },
    { "kRIT4",      {{kbAltRight},      kbShift | kbLeftAlt} },
    { "kUP4",       {{kbAltUp},         kbShift | kbLeftAlt} },
    { "kDN4",       {{kbAltDown},       kbShift | kbLeftAlt} },
    { "kDC5",       {{kbCtrlDel},       kbLeftCtrl} },
    { "kEND5",      {{kbCtrlEnd},       kbLeftCtrl} },
    { "kHOM5",      {{kbCtrlHome},      kbLeftCtrl} },
    { "kIC5",       {{kbCtrlIns},       kbLeftCtrl} },
    { "kLFT5",      {{kbCtrlLeft},      kbLeftCtrl} },
    { "kNXT5",      {{kbCtrlPgDn},      kbLeftCtrl} },
    { "kPRV5",      {{kbCtrlPgUp},      kbLeftCtrl} },
    { "kRIT5",      {{kbCtrlRight},     kbLeftCtrl} },
    { "kUP5",       {{kbCtrlUp},        kbLeftCtrl} },
    { "kDN5",       {{kbCtrlDown},      kbLeftCtrl} },
    { "kDC6",       {{kbCtrlDel},       kbLeftCtrl | kbShift} },
    { "kEND6",      {{kbCtrlEnd},       kbLeftCtrl | kbShift} },
    { "kHOM6",      {{kbCtrlHome},      kbLeftCtrl | kbShift} },
    { "kIC6",       {{kbCtrlIns},       kbLeftCtrl | kbShift} },
    { "kLFT6",      {{kbCtrlLeft},      kbLeftCtrl | kbShift} },
    { "kNXT6",      {{kbCtrlPgDn},      kbLeftCtrl | kbShift} },
    { "kPRV6",      {{kbCtrlPgUp},      kbLeftCtrl | kbShift} },
    { "kRIT6",      {{kbCtrlRight},     kbLeftCtrl | kbShift} },
    { "kUP6",       {{kbCtrlUp},        kbLeftCtrl | kbShift} },
    { "kDN6",       {{kbCtrlDown},      kbLeftCtrl | kbShift} },
    { "kDC7",       {{kbAltDel},        kbLeftCtrl | kbLeftAlt} }, // Please do not attempt this one
    { "kEND7",      {{kbAltEnd},        kbLeftCtrl | kbLeftAlt} },
    { "kHOM7",      {{kbAltHome},       kbLeftCtrl | kbLeftAlt} },
    { "kIC7",       {{kbAltIns},        kbLeftCtrl | kbLeftAlt} },
    { "kLFT7",      {{kbAltLeft},       kbLeftCtrl | kbLeftAlt} },
    { "kNXT7",      {{kbAltPgDn},       kbLeftCtrl | kbLeftAlt} },
    { "kPRV7",      {{kbAltPgUp},       kbLeftCtrl | kbLeftAlt} },
    { "kRIT7",      {{kbAltRight},      kbLeftCtrl | kbLeftAlt} },
    { "kUP7",       {{kbAltUp},         kbLeftCtrl | kbLeftAlt} },
    { "kDN7",       {{kbAltDown},       kbLeftCtrl | kbLeftAlt} },
    { "kpCMA",      {{'+'},             0, {'+'}, 1} },
    { "kpADD",      {{'+'},             0, {'+'}, 1} },
    { "kpSUB",      {{'-'},             0, {'-'}, 1} },
    { "kpMUL",      {{'*'},             0, {'*'}, 1} },
    { "kpDIV",      {{'/'},             0, {'/'}, 1} },
    { "kpZRO",      {{'0'},             0, {'0'}, 1} },
    { "kpDOT",      {{'.'},             0, {'.'}, 1} },
    { "ka2",        {{kbUp},            0} },
    { "kb1",        {{kbLeft},          0} },
    { "kb3",        {{kbRight},         0} },
    { "kc2",        {{kbDown},          0} },
});

int NcursesInputGetter::get() noexcept
{
    int k = wgetch(stdscr);
    if (pendingCount > 0)
        --pendingCount;
    return k != ERR ? k : -1;
}

void NcursesInputGetter::unget(int k) noexcept
{
    if (ungetch(k) != ERR)
        ++pendingCount;
}

NcursesInput::NcursesInput( StdioCtl &aIo, NcursesDisplay &,
                            InputState &aState, bool mouse ) noexcept :
    InputStrategy(aIo.in()),
    io(aIo),
    state(aState),
    mouseEnabled(mouse)
{
    // Capture all keyboard input.
    raw();
    // Disable echoing of pressed keys.
    noecho();
    // No need for ncurses to translate CR into LF.
    nonl();
    // Allow capturing function keys.
    keypad(stdscr, true);
    // Make getch practically non-blocking. Some terminals may feed input slowly.
    // Note that we only risk blocking when reading multibyte characters
    // or parsing escape sequences.
    wtimeout(stdscr, readTimeoutMs);
    /* Do not delay too much on ESC key presses, as the Alt modifier works well
     * in most modern terminals. Still, this delay helps ncurses distinguish
     * special key sequences, I believe. */
    set_escdelay(getEnv<int>("TVISION_ESCDELAY", 10));

    TermIO::keyModsOn(io);
    if (mouseEnabled)
        TermIO::mouseOn(io);
}

NcursesInput::~NcursesInput()
{
    if (mouseEnabled)
        TermIO::mouseOff(io);
    TermIO::keyModsOff(io);
    TermIO::consumeUnprocessedInput(io, in, state);
}

int NcursesInput::getButtonCount() noexcept
{
    // The exact button count is not really important. Turbo Vision
    // only checks whether it is non-zero.
    return mouseEnabled ? 2 : 0;
}

int NcursesInput::getChNb() noexcept
{
    wtimeout(stdscr, 0);
    int k = in.get();
    wtimeout(stdscr, readTimeoutMs);
    return k;
}

bool NcursesInput::hasPendingEvents() noexcept
{
    return in.pendingCount > 0;
}

bool NcursesInput::getEvent(TEvent &ev) noexcept
{
    GetChBuf buf(in);
    switch (TermIO::parseEvent(buf, ev, state))
    {
        case Rejected: buf.reject(); break;
        case Accepted: return true;
        case Ignored: return false;
    }

    int k = in.get();

    if (k == KEY_RESIZE)
        return false; // Handled by SigwinchHandler.
    else if (k == KEY_MOUSE)
        return parseCursesMouse(ev);

    if (k != ERR)
    {
        // A Unicode character might be composed of up to 4 UTF-8 bytes.
        int keys[4] = {k}, num_keys = 1;

        ev.what = evKeyDown;
        bool Alt = false;

        if (keys[0] == KEY_ESC)
            detectAlt(keys, Alt);

        if ((uint) keys[0] < 32)
            ev.keyDown = fromNonPrintableAscii[keys[0]];
        else if (keys[0] == 127)
            ev.keyDown = {{kbBack}, 0}; // ^?, Delete
        else if (KEY_MIN < keys[0] && keys[0] < KEY_MAX)
            ev.keyDown = fromCursesKeyCode[keys[0]];
        else if (KEY_MAX < keys[0])
            ev.keyDown = fromCursesHighKey[keyname(keys[0])];

        // If it hasn't been transformed by any of the previous tables,
        // and it's not a curses key, treat it like a printable character.
        if (ev.keyDown.keyCode == kbNoKey && keys[0] < KEY_MIN)
            parsePrintableChar(ev, keys, num_keys);

        if (Alt)
        {
            ev.keyDown.controlKeyState |= kbAltShift;
            TermIO::normalizeKey(ev.keyDown);
        }
        if (state.bracketedPaste)
            ev.keyDown.controlKeyState |= kbPaste;

        return ev.keyDown.keyCode != kbNoKey || ev.keyDown.textLength;
    }
    return false;
}

void NcursesInput::detectAlt(int keys[4], bool &Alt) noexcept
{
/* Alt+Key combinations begin with the character ESC. To tell the difference,
 * we check if another character has been received. If it has, we consider this
 * an Alt+Key combination. Of course, many other things sent by the terminal
 * begin with ESC, but ncurses already identifies most of them. */
    int k = getChNb();
    if (k != ERR)
    {
        keys[0] = k;
        Alt = true;
    }
}

void NcursesInput::parsePrintableChar(TEvent &ev, int keys[4], int &num_keys) noexcept
{
    // Read any possible remaining bytes.
    readUtf8Char(keys, num_keys);
    for (int i = 0; i < num_keys; ++i)
        ev.keyDown.text[i] = (char) keys[i];
    ev.keyDown.textLength = (uchar) num_keys;
    // If we are lucky enough, the character will be representable in
    // the active codepage.
    ev.keyDown.charScan.charCode = CpTranslator::fromUtf8({ev.keyDown.text, size_t(num_keys)});
    // Prevent text from triggering Ctrl+Key shortcuts.
    if (ev.keyDown.keyCode <= kbCtrlZ)
        ev.keyDown.keyCode = kbNoKey;
}

void NcursesInput::readUtf8Char(int keys[4], int &num_keys) noexcept
{
/* Unicode characters are sent by the terminal byte by byte. To read one, we
 * have to predict the number of bytes it is composed of, then read as many. */
    num_keys += Utf8BytesLeft((char) keys[0]);
    for (int i = 1; i < num_keys; ++i)
        if ((keys[i] = in.get()) == -1)
        {
            num_keys = i;
            break;
        }
}

bool NcursesInput::parseCursesMouse(TEvent &ev) noexcept
{
    MEVENT mevent;
    if (getmouse(&mevent) == OK)
    {
        ev.what = evMouse;
        ev.mouse = {};
        ev.mouse.where = {mevent.x, mevent.y};
        if (mevent.bstate & BUTTON1_PRESSED)
            state.buttons |= mbLeftButton;
        if (mevent.bstate & BUTTON1_RELEASED)
            state.buttons &= ~mbLeftButton;
        if (mevent.bstate & BUTTON2_PRESSED)
            state.buttons |= mbMiddleButton;
        if (mevent.bstate & BUTTON2_RELEASED)
            state.buttons &= ~mbMiddleButton;
        if (mevent.bstate & BUTTON3_PRESSED)
            state.buttons |= mbRightButton;
        if (mevent.bstate & BUTTON3_RELEASED)
            state.buttons &= ~mbRightButton;
        ev.mouse.buttons = state.buttons;

#if NCURSES_MOUSE_VERSION > 1
        // Mouse wheel support was added in Ncurses v6. Before that, only
        // scroll up would work. It's better not to support wheel scrolling
        // in that case.
        if (mevent.bstate & BUTTON4_PRESSED)
            ev.mouse.wheel = mwUp;
        else if (mevent.bstate & BUTTON5_PRESSED)
            ev.mouse.wheel = mwDown;
#endif
        return true;
    }
    else
    {
        // Ncurses sends KEY_MOUSE when reading "\x1B[M" or "\x1B[<" even if mouse support
        // is not enabled. We don't know which one was read. We could query terminal
        // capabilities to deduce it, but it is also possible to just follow
        // a trial and error approach. 'parseSGRMouse' is more likely to fail, so try it first.
        for (auto &parseMouse : {TermIO::parseSGRMouse,
                                 TermIO::parseX10Mouse})
        {
            GetChBuf buf(in);
            switch (parseMouse(buf, ev, state))
            {
                case Rejected: buf.reject(); break;
                case Accepted: return true;
                case Ignored: return false;
            }
        }
        return false;
    }
}

} // namespace tvision

#endif // HAVE_NCURSES
