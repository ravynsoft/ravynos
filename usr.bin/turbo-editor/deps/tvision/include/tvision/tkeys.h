/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   TKEYS.H                                                               */
/*                                                                         */
/*   defines constants for all control key combinations                    */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if defined( Uses_TKeys ) && !defined( __TKeys )
#define __TKeys

#if defined( __FLAT__ ) && !defined( __WINDOWS_H )
#include <tvision/compat/windows/windows.h>
#endif

//// Key codes
//
// The following constants can be used to define menu hotkeys and to
// examine the 'keyCode' field of 'evKeyDown' event records.
//
// Not all key combinations can be used in all platforms. In particular:
//
// * In 16-bit mode, keystrokes corresponding to the 'additional extended
//   key codes' are not reported.
//
// * Keystrokes corresponding to system shortcuts (e.g. Alt+F4 for 'close
//   window' in graphical environments) are not reported either.
//
// * In Unix, support for key combinations varies among terminal emulators.
//   Ctrl+H, Ctrl+I, Ctrl+J and Ctrl+M are not likely to work since they
//   usually represent Backspace, Tab and Enter.
//
// These constants do not cover all possible combinations of the Shift, Ctrl
// and Alt modifiers. For that purpose, see the TKey class below.

const ushort

// Control keys

    kbCtrlA     = 0x0001,   kbCtrlB     = 0x0002,   kbCtrlC     = 0x0003,
    kbCtrlD     = 0x0004,   kbCtrlE     = 0x0005,   kbCtrlF     = 0x0006,
    kbCtrlG     = 0x0007,   kbCtrlH     = 0x0008,   kbCtrlI     = 0x0009,
    kbCtrlJ     = 0x000a,   kbCtrlK     = 0x000b,   kbCtrlL     = 0x000c,
    kbCtrlM     = 0x000d,   kbCtrlN     = 0x000e,   kbCtrlO     = 0x000f,
    kbCtrlP     = 0x0010,   kbCtrlQ     = 0x0011,   kbCtrlR     = 0x0012,
    kbCtrlS     = 0x0013,   kbCtrlT     = 0x0014,   kbCtrlU     = 0x0015,
    kbCtrlV     = 0x0016,   kbCtrlW     = 0x0017,   kbCtrlX     = 0x0018,
    kbCtrlY     = 0x0019,   kbCtrlZ     = 0x001a,

// Extended key codes

    kbEsc       = 0x011b,   kbAltSpace  = 0x0200,   kbCtrlIns   = 0x0400,
    kbShiftIns  = 0x0500,   kbCtrlDel   = 0x0600,   kbShiftDel  = 0x0700,
    kbBack      = 0x0e08,   kbCtrlBack  = 0x0e7f,   kbShiftTab  = 0x0f00,
    kbTab       = 0x0f09,   kbAltQ      = 0x1000,   kbAltW      = 0x1100,
    kbAltE      = 0x1200,   kbAltR      = 0x1300,   kbAltT      = 0x1400,
    kbAltY      = 0x1500,   kbAltU      = 0x1600,   kbAltI      = 0x1700,
    kbAltO      = 0x1800,   kbAltP      = 0x1900,   kbCtrlEnter = 0x1c0a,
    kbEnter     = 0x1c0d,   kbAltA      = 0x1e00,   kbAltS      = 0x1f00,
    kbAltD      = 0x2000,   kbAltF      = 0x2100,   kbAltG      = 0x2200,
    kbAltH      = 0x2300,   kbAltJ      = 0x2400,   kbAltK      = 0x2500,
    kbAltL      = 0x2600,   kbAltZ      = 0x2c00,   kbAltX      = 0x2d00,
    kbAltC      = 0x2e00,   kbAltV      = 0x2f00,   kbAltB      = 0x3000,
    kbAltN      = 0x3100,   kbAltM      = 0x3200,   kbF1        = 0x3b00,
    kbF2        = 0x3c00,   kbF3        = 0x3d00,   kbF4        = 0x3e00,
    kbF5        = 0x3f00,   kbF6        = 0x4000,   kbF7        = 0x4100,
    kbF8        = 0x4200,   kbF9        = 0x4300,   kbF10       = 0x4400,
    kbHome      = 0x4700,   kbUp        = 0x4800,   kbPgUp      = 0x4900,
    kbGrayMinus = 0x4a2d,   kbLeft      = 0x4b00,   kbRight     = 0x4d00,
    kbGrayPlus  = 0x4e2b,   kbEnd       = 0x4f00,   kbDown      = 0x5000,
    kbPgDn      = 0x5100,   kbIns       = 0x5200,   kbDel       = 0x5300,
    kbShiftF1   = 0x5400,   kbShiftF2   = 0x5500,   kbShiftF3   = 0x5600,
    kbShiftF4   = 0x5700,   kbShiftF5   = 0x5800,   kbShiftF6   = 0x5900,
    kbShiftF7   = 0x5a00,   kbShiftF8   = 0x5b00,   kbShiftF9   = 0x5c00,
    kbShiftF10  = 0x5d00,   kbCtrlF1    = 0x5e00,   kbCtrlF2    = 0x5f00,
    kbCtrlF3    = 0x6000,   kbCtrlF4    = 0x6100,   kbCtrlF5    = 0x6200,
    kbCtrlF6    = 0x6300,   kbCtrlF7    = 0x6400,   kbCtrlF8    = 0x6500,
    kbCtrlF9    = 0x6600,   kbCtrlF10   = 0x6700,   kbAltF1     = 0x6800,
    kbAltF2     = 0x6900,   kbAltF3     = 0x6a00,   kbAltF4     = 0x6b00,
    kbAltF5     = 0x6c00,   kbAltF6     = 0x6d00,   kbAltF7     = 0x6e00,
    kbAltF8     = 0x6f00,   kbAltF9     = 0x7000,   kbAltF10    = 0x7100,
    kbCtrlPrtSc = 0x7200,   kbCtrlLeft  = 0x7300,   kbCtrlRight = 0x7400,
    kbCtrlEnd   = 0x7500,   kbCtrlPgDn  = 0x7600,   kbCtrlHome  = 0x7700,
    kbAlt1      = 0x7800,   kbAlt2      = 0x7900,   kbAlt3      = 0x7a00,
    kbAlt4      = 0x7b00,   kbAlt5      = 0x7c00,   kbAlt6      = 0x7d00,
    kbAlt7      = 0x7e00,   kbAlt8      = 0x7f00,   kbAlt9      = 0x8000,
    kbAlt0      = 0x8100,   kbAltMinus  = 0x8200,   kbAltEqual  = 0x8300,
    kbCtrlPgUp  = 0x8400,   kbNoKey     = 0x0000,

// Additional extended key codes

    kbAltEsc    = 0x0100,   kbAltBack   = 0x0e00,   kbF11       = 0x8500,
    kbF12       = 0x8600,   kbShiftF11  = 0x8700,   kbShiftF12  = 0x8800,
    kbCtrlF11   = 0x8900,   kbCtrlF12   = 0x8a00,   kbAltF11    = 0x8b00,
    kbAltF12    = 0x8c00,   kbCtrlUp    = 0x8d00,   kbCtrlDown  = 0x9100,
    kbCtrlTab   = 0x9400,   kbAltHome   = 0x9700,   kbAltUp     = 0x9800,
    kbAltPgUp   = 0x9900,   kbAltLeft   = 0x9b00,   kbAltRight  = 0x9d00,
    kbAltEnd    = 0x9f00,   kbAltDown   = 0xa000,   kbAltPgDn   = 0xa100,
    kbAltIns    = 0xa200,   kbAltDel    = 0xa300,   kbAltTab    = 0xa500,
    kbAltEnter  = 0xa600,

//// Keyboard state and shift masks
//
// The following constants can be used when examining the 'controlKeyState'
// field of an 'evKeyDown' event record.
//
// Distinguishing between the left and right Shift keys is only supported in
// 16-bit mode, while distinguishing between the left and right Ctrl and Alt
// keys is only supported in 32-bit DPMI and on Windows.
//
// The kbScrollState, kbNumState, kbCapsState and kbEnhanced flags are only
// reported on DOS and Windows.

#if !defined( __FLAT__ )
    kbLeftShift   = 0x0001,
    kbRightShift  = 0x0002,
    kbShift       = kbLeftShift | kbRightShift,
    kbLeftCtrl    = 0x0004,
    kbRightCtrl   = 0x0004,
    kbCtrlShift   = kbLeftCtrl | kbRightCtrl,
    kbLeftAlt     = 0x0008,
    kbRightAlt    = 0x0008,
    kbAltShift    = kbLeftAlt | kbRightAlt,
    kbScrollState = 0x0010,
    kbNumState    = 0x0020,
    kbCapsState   = 0x0040,
    kbInsState    = 0x0080,
    kbPaste       = 0x0100;
#else
    kbLeftShift   = SHIFT_PRESSED,
    kbRightShift  = SHIFT_PRESSED,
    kbShift       = kbLeftShift | kbRightShift,
    kbLeftCtrl    = LEFT_CTRL_PRESSED,
    kbRightCtrl   = RIGHT_CTRL_PRESSED,
    kbCtrlShift   = kbLeftCtrl | kbRightCtrl,
    kbLeftAlt     = LEFT_ALT_PRESSED,
    kbRightAlt    = RIGHT_ALT_PRESSED,
    kbAltShift    = kbLeftAlt | kbRightAlt,
    kbScrollState = SCROLLLOCK_ON,
    kbNumState    = NUMLOCK_ON,
    kbCapsState   = CAPSLOCK_ON,
    kbEnhanced    = ENHANCED_KEY,
    kbInsState    = 0x200,  // Ensure this doesn't overlap above values
    kbPaste       = 0x400;
#endif

#endif // __TKeys

#if !defined( __TKey )
#define __TKey

//// TKey
//
// This class provides the ability to define new key combinations by
// specifying a key code and a mask of key modifiers. These key combinations
// can then be used to define menu hotkeys and examine 'evKeyDown' event
// records.
//
// The only modifiers taken into account are Shift, Ctrl and Alt, making no
// distinction between the left and right key on platforms that support it.
//
// Given that some key code constants already imply the presence of a key
// modifier, there are key combinations which can be defined in multiple
// ways. TKey ensures equality between all of them. For example:
//
//     assert(kbCtrlA == TKey('A', kbCtrlShift));
//     assert(TKey(kbCtrlTab, kbShift) == TKey(kbTab, kbShift | kbCtrlShift));
//     assert(TKey(kbAltDel, kbCtrlShift) == TKey(kbCtrlDel, kbAltShift));

class TKey
{
public:

    constexpr TKey() noexcept;
    TKey(ushort keyCode, ushort shiftState = 0) noexcept;

    ushort code;
    ushort mods;
};

inline constexpr TKey::TKey() noexcept :
    code(0),
    mods(0)
{
}

inline constexpr Boolean operator==(TKey a, TKey b) noexcept
{
    return Boolean(
        (a.code | ((int32_t) a.mods << 16)) == (b.code | ((int32_t) b.mods << 16))
    );
}

inline constexpr Boolean operator!=(TKey a, TKey b) noexcept
{
    return Boolean( !(a == b) );
}

inline ipstream& operator >> ( ipstream& is, TKey& p )
    { return is >> p.code >> p.mods; }
inline ipstream& operator >> ( ipstream& is, TKey*& p )
    { return is >> p->code >> p->mods; }

inline opstream& operator << ( opstream& os, TKey& p )
    { return os << p.code << p.mods; }
inline opstream& operator << ( opstream& os, TKey* p )
    { return os << p->code << p->mods; }

#endif  // __TKey
