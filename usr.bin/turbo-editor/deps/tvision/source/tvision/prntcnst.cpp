/*------------------------------------------------------------*/
/* filename - prntcnst.cpp                                    */
/*                                                            */
/* function(s)                                                */
/*            printKeyCode                                    */
/*            printControlKeyState                            */
/*            printEventCode                                  */
/*            printMouseButtonState                           */
/*            printMouseWheelState                            */
/*            printMouseEventFlags                            */
/*------------------------------------------------------------*/

#define Uses_EventCodes
#define Uses_TKeys
#include <tvision/tv.h>

#include <iostream.h>
#include <iomanip.h>

#ifdef __BORLANDC__
typedef long fmtflags;
#else
typedef ios::fmtflags fmtflags;
#endif

struct TConstant
{
    ushort value;
    const char *name;
};

#define NM(x) { x, #x }
#define NMEND() { 0, 0 }

static const TConstant keyCodes[] =
{
    NM(kbCtrlA),            NM(kbCtrlB),            NM(kbCtrlC),
    NM(kbCtrlD),            NM(kbCtrlE),            NM(kbCtrlF),
    NM(kbCtrlG),            NM(kbCtrlH),            NM(kbCtrlI),
    NM(kbCtrlJ),            NM(kbCtrlK),            NM(kbCtrlL),
    NM(kbCtrlM),            NM(kbCtrlN),            NM(kbCtrlO),
    NM(kbCtrlP),            NM(kbCtrlQ),            NM(kbCtrlR),
    NM(kbCtrlS),            NM(kbCtrlT),            NM(kbCtrlU),
    NM(kbCtrlV),            NM(kbCtrlW),            NM(kbCtrlX),
    NM(kbCtrlY),            NM(kbCtrlZ),
    NM(kbEsc),              NM(kbAltSpace),         NM(kbCtrlIns),
    NM(kbShiftIns),         NM(kbCtrlDel),          NM(kbShiftDel),
    NM(kbBack),             NM(kbCtrlBack),         NM(kbShiftTab),
    NM(kbTab),              NM(kbAltQ),             NM(kbAltW),
    NM(kbAltE),             NM(kbAltR),             NM(kbAltT),
    NM(kbAltY),             NM(kbAltU),             NM(kbAltI),
    NM(kbAltO),             NM(kbAltP),             NM(kbCtrlEnter),
    NM(kbEnter),            NM(kbAltA),             NM(kbAltS),
    NM(kbAltD),             NM(kbAltF),             NM(kbAltG),
    NM(kbAltH),             NM(kbAltJ),             NM(kbAltK),
    NM(kbAltL),             NM(kbAltZ),             NM(kbAltX),
    NM(kbAltC),             NM(kbAltV),             NM(kbAltB),
    NM(kbAltN),             NM(kbAltM),             NM(kbF1),
    NM(kbF2),               NM(kbF3),               NM(kbF4),
    NM(kbF5),               NM(kbF6),               NM(kbF7),
    NM(kbF8),               NM(kbF9),               NM(kbF10),
    NM(kbHome),             NM(kbUp),               NM(kbPgUp),
    NM(kbGrayMinus),        NM(kbLeft),             NM(kbRight),
    NM(kbGrayPlus),         NM(kbEnd),              NM(kbDown),
    NM(kbPgDn),             NM(kbIns),              NM(kbDel),
    NM(kbShiftF1),          NM(kbShiftF2),          NM(kbShiftF3),
    NM(kbShiftF4),          NM(kbShiftF5),          NM(kbShiftF6),
    NM(kbShiftF7),          NM(kbShiftF8),          NM(kbShiftF9),
    NM(kbShiftF10),         NM(kbCtrlF1),           NM(kbCtrlF2),
    NM(kbCtrlF3),           NM(kbCtrlF4),           NM(kbCtrlF5),
    NM(kbCtrlF6),           NM(kbCtrlF7),           NM(kbCtrlF8),
    NM(kbCtrlF9),           NM(kbCtrlF10),          NM(kbAltF1),
    NM(kbAltF2),            NM(kbAltF3),            NM(kbAltF4),
    NM(kbAltF5),            NM(kbAltF6),            NM(kbAltF7),
    NM(kbAltF8),            NM(kbAltF9),            NM(kbAltF10),
    NM(kbCtrlPrtSc),        NM(kbCtrlLeft),         NM(kbCtrlRight),
    NM(kbCtrlEnd),          NM(kbCtrlPgDn),         NM(kbCtrlHome),
    NM(kbAlt1),             NM(kbAlt2),             NM(kbAlt3),
    NM(kbAlt4),             NM(kbAlt5),             NM(kbAlt6),
    NM(kbAlt7),             NM(kbAlt8),             NM(kbAlt9),
    NM(kbAlt0),             NM(kbAltMinus),         NM(kbAltEqual),
    NM(kbCtrlPgUp),         NM(kbNoKey),
    NM(kbAltEsc),           NM(kbAltBack),          NM(kbF11),
    NM(kbF12),              NM(kbShiftF11),         NM(kbShiftF12),
    NM(kbCtrlF11),          NM(kbCtrlF12),          NM(kbAltF11),
    NM(kbAltF12),           NM(kbCtrlUp),           NM(kbCtrlDown),
    NM(kbCtrlTab),          NM(kbAltHome),          NM(kbAltUp),
    NM(kbAltPgUp),          NM(kbAltLeft),          NM(kbAltRight),
    NM(kbAltEnd),           NM(kbAltDown),          NM(kbAltPgDn),
    NM(kbAltIns),           NM(kbAltDel),           NM(kbAltTab),
    NM(kbAltEnter),
    NMEND(),
};

static const TConstant controlKeyStateFlags[] =
{
#if !defined( __FLAT__ )
    NM(kbLeftShift),
    NM(kbRightShift),
    NM(kbCtrlShift),
    NM(kbAltShift),
#else
    NM(kbShift),
    NM(kbScrollState),
    NM(kbLeftCtrl),
    NM(kbRightCtrl),
    NM(kbLeftAlt),
    NM(kbRightAlt),
#endif
    NM(kbNumState),
    NM(kbCapsState),
    NM(kbInsState),
#if defined( __FLAT__ )
    NM(kbEnhanced),
#endif
    NM(kbPaste),
    NMEND(),
};

static const TConstant eventCodes[] =
{
    NM(evNothing),
    NM(evMouseDown),
    NM(evMouseUp),
    NM(evMouseMove),
    NM(evMouseAuto),
    NM(evMouseWheel),
    NM(evMouse),
    NM(evKeyDown),
    NM(evCommand),
    NM(evBroadcast),
    NMEND(),
};

static const TConstant mouseButtonFlags[] =
{
    NM(mbLeftButton),
    NM(mbRightButton),
    NM(mbMiddleButton),
    NMEND(),
};

static const TConstant mouseWheelFlags[] =
{
    NM(mwUp),
    NM(mwDown),
    NM(mwLeft),
    NM(mwRight),
    NMEND(),
};

static const TConstant mouseEventFlags[] =
{
    NM(meMouseMoved),
    NM(meDoubleClick),
    NM(meTripleClick),
    NMEND(),
};

static void printFlags(ostream _FAR &os, ushort flags, const TConstant *constants)
{
    ushort foundFlags = 0;
    for (const TConstant *constant = constants; constant->name; ++constant)
    {
        if (flags & constant->value)
        {
            if (foundFlags != 0)
                os << " | ";
            os << constant->name;
            foundFlags |= flags & constant->value;
        }
    }
    if (foundFlags == 0 || foundFlags != flags)
    {
        fmtflags f = os.flags();
        char ch = os.fill('0');

        if (foundFlags != 0)
            os << " | ";
        os << "0x" << hex << setw(4) << (flags & ~foundFlags);

        os.flags(f);
        os.fill(ch);
    }
}

static void printCode(ostream _FAR &os, ushort code, const TConstant *constants)
{
    for (const TConstant *constant = constants; constant->name; ++constant)
    {
        if (code == constant->value)
        {
            os << constant->name;
            return;
        }
    }
    fmtflags f = os.flags();
    char ch = os.fill('0');

    os << "0x" << hex << setw(4) << code;

    os.flags(f);
    os.fill(ch);
}

void printKeyCode(ostream _FAR &os, ushort keyCode)
{
    printCode(os, keyCode, keyCodes);
}

void printControlKeyState(ostream _FAR &os, ushort controlKeyState)
{
    printFlags(os, controlKeyState, controlKeyStateFlags);
}

void printEventCode(ostream _FAR &os, ushort eventCode)
{
    printCode(os, eventCode, eventCodes);
}

void printMouseButtonState(ostream _FAR &os, ushort buttonState)
{
    printFlags(os, buttonState, mouseButtonFlags);
}

void printMouseWheelState(ostream _FAR &os, ushort wheelState)
{
    printFlags(os, wheelState, mouseWheelFlags);
}

void printMouseEventFlags(ostream _FAR &os, ushort eventFlags)
{
    printFlags(os, eventFlags, mouseEventFlags);
}
