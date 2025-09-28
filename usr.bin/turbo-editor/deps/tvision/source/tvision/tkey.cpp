/*------------------------------------------------------------*/
/* filename - tkey.cpp                                        */
/*                                                            */
/* function(s)                                                */
/*            TKey member functions                           */
/*------------------------------------------------------------*/

#define Uses_TKeys
#include <tvision/tv.h>

struct TKeyCodeLookupEntry
{
    ushort normalKeyCode;
    uchar shiftState;
};

static const TKeyCodeLookupEntry ctrlKeyLookup[] =
{
    /* 0x0000 */  { 0,      0           },
    /* 0x0001 */  { 'A',    kbCtrlShift },
    /* 0x0002 */  { 'B',    kbCtrlShift },
    /* 0x0003 */  { 'C',    kbCtrlShift },
    /* 0x0004 */  { 'D',    kbCtrlShift },
    /* 0x0005 */  { 'E',    kbCtrlShift },
    /* 0x0006 */  { 'F',    kbCtrlShift },
    /* 0x0007 */  { 'G',    kbCtrlShift },
    /* 0x0008 */  { 'H',    kbCtrlShift },
    /* 0x0009 */  { 'I',    kbCtrlShift },
    /* 0x000A */  { 'J',    kbCtrlShift },
    /* 0x000B */  { 'K',    kbCtrlShift },
    /* 0x000C */  { 'L',    kbCtrlShift },
    /* 0x000D */  { 'M',    kbCtrlShift },
    /* 0x000E */  { 'N',    kbCtrlShift },
    /* 0x000F */  { 'O',    kbCtrlShift },
    /* 0x0010 */  { 'P',    kbCtrlShift },
    /* 0x0011 */  { 'Q',    kbCtrlShift },
    /* 0x0012 */  { 'R',    kbCtrlShift },
    /* 0x0013 */  { 'S',    kbCtrlShift },
    /* 0x0014 */  { 'T',    kbCtrlShift },
    /* 0x0015 */  { 'U',    kbCtrlShift },
    /* 0x0016 */  { 'V',    kbCtrlShift },
    /* 0x0017 */  { 'W',    kbCtrlShift },
    /* 0x0018 */  { 'X',    kbCtrlShift },
    /* 0x0019 */  { 'Y',    kbCtrlShift },
    /* 0x001A */  { 'Z',    kbCtrlShift },
};

static const TKeyCodeLookupEntry extKeyLookup[] =
{
    /* 0x0000 */  { 0,              0               },
    /* 0x0100 */  { kbEsc,          kbAltShift      },
    /* 0x0200 */  { ' ',            kbAltShift      },
    /* 0x0300 */  { 0,              0               },
    /* 0x0400 */  { kbIns,          kbCtrlShift     },
    /* 0x0500 */  { kbIns,          kbShift,        },
    /* 0x0600 */  { kbDel,          kbCtrlShift     },
    /* 0x0700 */  { kbDel,          kbShift         },
    /* 0x0800 */  { 0,              0               },
    /* 0x0900 */  { 0,              0               },
    /* 0x0A00 */  { 0,              0               },
    /* 0x0B00 */  { 0,              0               },
    /* 0x0C00 */  { 0,              0               },
    /* 0x0D00 */  { 0,              0               },
    /* 0x0E00 */  { kbBack,         kbAltShift      },
    /* 0x0F00 */  { kbTab,          kbShift         },
    /* 0x1000 */  { 'Q',            kbAltShift      },
    /* 0x1100 */  { 'W',            kbAltShift      },
    /* 0x1200 */  { 'E',            kbAltShift      },
    /* 0x1300 */  { 'R',            kbAltShift      },
    /* 0x1400 */  { 'T',            kbAltShift      },
    /* 0x1500 */  { 'Y',            kbAltShift      },
    /* 0x1600 */  { 'U',            kbAltShift      },
    /* 0x1700 */  { 'I',            kbAltShift      },
    /* 0x1800 */  { 'O',            kbAltShift      },
    /* 0x1900 */  { 'P',            kbAltShift      },
    /* 0x1A00 */  { 0,              0               },
    /* 0x1B00 */  { 0,              0               },
    /* 0x1C00 */  { 0,              0               },
    /* 0x1D00 */  { 0,              0               },
    /* 0x1E00 */  { 'A',            kbAltShift      },
    /* 0x1F00 */  { 'S',            kbAltShift      },
    /* 0x2000 */  { 'D',            kbAltShift      },
    /* 0x2100 */  { 'F',            kbAltShift      },
    /* 0x2200 */  { 'G',            kbAltShift      },
    /* 0x2300 */  { 'H',            kbAltShift      },
    /* 0x2400 */  { 'J',            kbAltShift      },
    /* 0x2500 */  { 'K',            kbAltShift      },
    /* 0x2600 */  { 'L',            kbAltShift      },
    /* 0x2700 */  { 0,              0               },
    /* 0x2800 */  { 0,              0               },
    /* 0x2900 */  { 0,              0               },
    /* 0x2A00 */  { 0,              0               },
    /* 0x2B00 */  { 0,              0               },
    /* 0x2C00 */  { 'Z',            kbAltShift      },
    /* 0x2D00 */  { 'X',            kbAltShift      },
    /* 0x2E00 */  { 'C',            kbAltShift      },
    /* 0x2F00 */  { 'V',            kbAltShift      },
    /* 0x3000 */  { 'B',            kbAltShift      },
    /* 0x3100 */  { 'N',            kbAltShift      },
    /* 0x3200 */  { 'M',            kbAltShift      },
    /* 0x3300 */  { 0,              0               },
    /* 0x3400 */  { 0,              0               },
    /* 0x3500 */  { '/',            kbCtrlShift     },
    /* 0x3600 */  { 0,              0               },
    /* 0x3700 */  { '*',            kbCtrlShift     },
    /* 0x3800 */  { 0,              0               },
    /* 0x3900 */  { 0,              0               },
    /* 0x3A00 */  { 0,              0               },
    /* 0x3B00 */  { kbF1,           0               },
    /* 0x3C00 */  { kbF2,           0               },
    /* 0x3D00 */  { kbF3,           0               },
    /* 0x3E00 */  { kbF4,           0               },
    /* 0x3F00 */  { kbF5,           0               },
    /* 0x4000 */  { kbF6,           0               },
    /* 0x4100 */  { kbF7,           0               },
    /* 0x4200 */  { kbF8,           0               },
    /* 0x4300 */  { kbF9,           0               },
    /* 0x4400 */  { kbF10,          0               },
    /* 0x4500 */  { 0,              0               },
    /* 0x4600 */  { 0,              0               },
    /* 0x4700 */  { kbHome,         0               },
    /* 0x4800 */  { kbUp,           0               },
    /* 0x4900 */  { kbPgUp,         0               },
    /* 0x4A00 */  { '-',            kbCtrlShift     },
    /* 0x4B00 */  { kbLeft,         0               },
    /* 0x4C00 */  { 0,              0               },
    /* 0x4D00 */  { kbRight,        0               },
    /* 0x4E00 */  { '+',            kbCtrlShift     },
    /* 0x4F00 */  { kbEnd,          0               },
    /* 0x5000 */  { kbDown,         0               },
    /* 0x5100 */  { kbPgDn,         0               },
    /* 0x5200 */  { kbIns,          0               },
    /* 0x5300 */  { kbDel,          0               },
    /* 0x5400 */  { kbF1,           kbShift         },
    /* 0x5500 */  { kbF2,           kbShift         },
    /* 0x5600 */  { kbF3,           kbShift         },
    /* 0x5700 */  { kbF4,           kbShift         },
    /* 0x5800 */  { kbF5,           kbShift         },
    /* 0x5900 */  { kbF6,           kbShift         },
    /* 0x5A00 */  { kbF7,           kbShift         },
    /* 0x5B00 */  { kbF8,           kbShift         },
    /* 0x5C00 */  { kbF9,           kbShift         },
    /* 0x5D00 */  { kbF10,          kbShift         },
    /* 0x5E00 */  { kbF1,           kbCtrlShift     },
    /* 0x5F00 */  { kbF2,           kbCtrlShift     },
    /* 0x6000 */  { kbF3,           kbCtrlShift     },
    /* 0x6100 */  { kbF4,           kbCtrlShift     },
    /* 0x6200 */  { kbF5,           kbCtrlShift     },
    /* 0x6300 */  { kbF6,           kbCtrlShift     },
    /* 0x6400 */  { kbF7,           kbCtrlShift     },
    /* 0x6500 */  { kbF8,           kbCtrlShift     },
    /* 0x6600 */  { kbF9,           kbCtrlShift     },
    /* 0x6700 */  { kbF10,          kbCtrlShift     },
    /* 0x6800 */  { kbF1,           kbAltShift      },
    /* 0x6900 */  { kbF2,           kbAltShift      },
    /* 0x6A00 */  { kbF3,           kbAltShift      },
    /* 0x6B00 */  { kbF4,           kbAltShift      },
    /* 0x6C00 */  { kbF5,           kbAltShift      },
    /* 0x6D00 */  { kbF6,           kbAltShift      },
    /* 0x6E00 */  { kbF7,           kbAltShift      },
    /* 0x6F00 */  { kbF8,           kbAltShift      },
    /* 0x7000 */  { kbF9,           kbAltShift      },
    /* 0x7100 */  { kbF10,          kbAltShift      },
    /* 0x7200 */  { kbCtrlPrtSc,    kbCtrlShift     },
    /* 0x7300 */  { kbLeft,         kbCtrlShift     },
    /* 0x7400 */  { kbRight,        kbCtrlShift     },
    /* 0x7500 */  { kbEnd,          kbCtrlShift     },
    /* 0x7600 */  { kbPgDn,         kbCtrlShift     },
    /* 0x7700 */  { kbHome,         kbCtrlShift     },
    /* 0x7800 */  { '1',            kbAltShift      },
    /* 0x7900 */  { '2',            kbAltShift      },
    /* 0x7A00 */  { '3',            kbAltShift      },
    /* 0x7B00 */  { '4',            kbAltShift      },
    /* 0x7C00 */  { '5',            kbAltShift      },
    /* 0x7D00 */  { '6',            kbAltShift      },
    /* 0x7E00 */  { '7',            kbAltShift      },
    /* 0x7F00 */  { '8',            kbAltShift      },
    /* 0x8000 */  { '9',            kbAltShift      },
    /* 0x8100 */  { '0',            kbAltShift      },
    /* 0x8200 */  { '-',            kbAltShift      },
    /* 0x8300 */  { '=',            kbAltShift      },
    /* 0x8400 */  { kbPgUp,         kbCtrlShift     },
#if defined( __FLAT__  )
    /* 0x8500 */  { kbF11,          0               },
    /* 0x8600 */  { kbF12,          0               },
    /* 0x8700 */  { kbF11,          kbShift         },
    /* 0x8800 */  { kbF12,          kbShift         },
    /* 0x8900 */  { kbF11,          kbCtrlShift     },
    /* 0x8A00 */  { kbF12,          kbCtrlShift     },
    /* 0x8B00 */  { kbF11,          kbAltShift      },
    /* 0x8C00 */  { kbF12,          kbAltShift      },
    /* 0x8D00 */  { kbUp,           kbCtrlShift     },
    /* 0x8E00 */  { 0,              0               },
    /* 0x8F00 */  { 0,              0               },
    /* 0x9000 */  { 0,              0               },
    /* 0x9100 */  { kbDown,         kbCtrlShift     },
    /* 0x9200 */  { 0,              0               },
    /* 0x9300 */  { 0,              0               },
    /* 0x9400 */  { kbTab,          kbCtrlShift     },
    /* 0x9500 */  { 0,              0               },
    /* 0x9600 */  { 0,              0               },
    /* 0x9700 */  { kbHome,         kbAltShift      },
    /* 0x9800 */  { kbUp,           kbAltShift      },
    /* 0x9900 */  { kbPgUp,         kbAltShift      },
    /* 0x9A00 */  { 0,              0               },
    /* 0x9B00 */  { kbLeft,         kbAltShift      },
    /* 0x9C00 */  { 0,              0               },
    /* 0x9D00 */  { kbRight,        kbAltShift      },
    /* 0x9E00 */  { 0,              0               },
    /* 0x9F00 */  { kbEnd,          kbAltShift      },
    /* 0xA000 */  { kbDown,         kbAltShift      },
    /* 0xA100 */  { kbPgDn,         kbAltShift      },
    /* 0xA200 */  { kbIns,          kbAltShift      },
    /* 0xA300 */  { kbDel,          kbAltShift      },
    /* 0xA400 */  { 0,              0               },
    /* 0xA500 */  { kbTab,          kbAltShift      },
    /* 0xA600 */  { kbEnter,        kbAltShift      },
#endif
};

enum { extKeyLookupSize = sizeof(extKeyLookup)/sizeof(extKeyLookup[0]) };

static const TKeyCodeLookupEntry
    kbCtrlBackEntry =   { kbBack,   kbCtrlShift },
    kbCtrlEnterEntry =  { kbEnter,  kbCtrlShift };

static int isRawCtrlKey(uchar scanCode, uchar charCode)
{
    static const char scanKeys[35 + 1] =
        "QWERTYUIOP\0\0\0\0ASDFGHJKL\0\0\0\0\0ZXCVBNM";
    return 16 <= scanCode && scanCode < 16 + 35
        && scanKeys[scanCode - 16] == charCode - 1 + 'A';
};

static int isPrintableCharacter(uchar charCode) noexcept
{
    return ' ' <= charCode && charCode != 0x7F && charCode != 0xFF;
};

static int isKeypadCharacter(uchar scanCode) noexcept
{
    return scanCode == 0x35 || scanCode == 0x37
        || scanCode == 0x4A || scanCode == 0x4E;
}

TKey::TKey(ushort keyCode, ushort shiftState) noexcept
{
    ushort code = keyCode;
    ushort mods =
        (shiftState & kbShift ? kbShift : 0) |
        (shiftState & kbCtrlShift ? kbCtrlShift : 0) |
        (shiftState & kbAltShift ? kbAltShift : 0);
    uchar scanCode = keyCode >> 8;
    uchar charCode = keyCode & 0xFF;

    const TKeyCodeLookupEntry *entry = 0;
    if (keyCode <= kbCtrlZ || isRawCtrlKey(scanCode, charCode))
        entry = &ctrlKeyLookup[charCode];
    else if ((keyCode & 0xFF) == 0)
    {
        if (scanCode < extKeyLookupSize)
            entry = &extKeyLookup[scanCode];
    }
    else if (isPrintableCharacter(charCode))
    {
        if ('a' <= charCode && charCode <= 'z')
            code = charCode - 'a' + 'A';
        else if (!isKeypadCharacter(scanCode))
            code &= 0xFF;
    }
    else switch(keyCode)
    {
        case kbCtrlBack: entry = &kbCtrlBackEntry; break;
        case kbCtrlEnter: entry = &kbCtrlEnterEntry; break;
    }

    if (entry)
    {
        mods |= entry->shiftState;
        if (entry->normalKeyCode != 0)
            code = entry->normalKeyCode;
    }

    this->code = code;
    this->mods = code != kbNoKey ? mods : 0;
}
