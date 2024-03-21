#define Uses_TKeys
#include <tvision/tv.h>

#include <internal/far2l.h>
#include <internal/win32con.h>
#include <internal/constmap.h>
#include <internal/base64.h>
#include <internal/events.h>
#include <internal/endian.h>

#include <time.h>

namespace tvision
{

// Request IDs
const char
    f2lNoAnswer = '\0',
    f2lClipGetData = '\xA0';

static char f2lClientIdData[32 + 1];
static TStringView f2lClientId =
(
    sprintf(f2lClientIdData, "%032llu", (unsigned long long) time(nullptr)),
    f2lClientIdData
);

static const const_unordered_map<uchar, ushort> virtualKeyCodeToKeyCode =
{
    { VK_BACK,      kbBack      }, { VK_TAB,        kbTab       },
    { VK_RETURN,    kbEnter     }, { VK_ESCAPE,     kbEsc       },
    { VK_PRIOR,     kbPgUp      }, { VK_NEXT,       kbPgDn      },
    { VK_END,       kbEnd       }, { VK_HOME,       kbHome      },
    { VK_LEFT,      kbLeft      }, { VK_UP,         kbUp        },
    { VK_RIGHT,     kbRight     }, { VK_DOWN,       kbDown      },
    { VK_INSERT,    kbIns       }, { VK_DELETE,     kbDel       },
    { VK_NUMPAD0,   0x5200 | '0'}, { VK_NUMPAD1,    0x4F00 | '1'},
    { VK_NUMPAD2,   0x5000 | '2'}, { VK_NUMPAD3,    0x5100 | '3'},
    { VK_NUMPAD4,   0x4B00 | '4'}, { VK_NUMPAD5,    0x4C00 | '5'},
    { VK_NUMPAD6,   0x4D00 | '6'}, { VK_NUMPAD7,    0x4700 | '7'},
    { VK_NUMPAD8,   0x4800 | '8'}, { VK_NUMPAD9,    0x4900 | '9'},
    { VK_MULTIPLY,  0x3700 | '*'}, { VK_ADD,        0x4E00 | '+'},
    { VK_SEPARATOR, 0x7E00 | ','}, { VK_SUBTRACT,   0x4A00 | '-'},
    { VK_DECIMAL,   0x5300 | '.'}, { VK_DIVIDE,     0x3500 | '/'},
    { VK_F1,        kbF1        }, { VK_F2,         kbF2        },
    { VK_F3,        kbF3        }, { VK_F4,         kbF4        },
    { VK_F5,        kbF5        }, { VK_F6,         kbF6        },
    { VK_F7,        kbF7        }, { VK_F8,         kbF8        },
    { VK_F9,        kbF9        }, { VK_F10,        kbF10       },
    { VK_F11,       kbF11       }, { VK_F12,        kbF12       },
};

static const const_unordered_map<uchar, uchar> virtualKeyCodeToScanCode =
{
    // 0-9 keys
    { 0x30,         0x0B        }, { 0x31,          0x02        },
    { 0x32,         0x03        }, { 0x33,          0x04        },
    { 0x34,         0x05        }, { 0x35,          0x06        },
    { 0x36,         0x07        }, { 0x37,          0x08        },
    { 0x38,         0x09        }, { 0x39,          0x0A        },
    // A-Z keys
    { 0x41,         0x1E        }, { 0x42,          0x30        },
    { 0x43,         0x2E        }, { 0x44,          0x20        },
    { 0x45,         0x12        }, { 0x46,          0x21        },
    { 0x47,         0x22        }, { 0x48,          0x23        },
    { 0x49,         0x17        }, { 0x4A,          0x24        },
    { 0x4B,         0x25        }, { 0x4C,          0x26        },
    { 0x4D,         0x32        }, { 0x4E,          0x31        },
    { 0x4F,         0x18        }, { 0x50,          0x19        },
    { 0x51,         0x10        }, { 0x52,          0x13        },
    { 0x53,         0x1F        }, { 0x54,          0x14        },
    { 0x55,         0x16        }, { 0x56,          0x2F        },
    { 0x57,         0x11        }, { 0x58,          0x2D        },
    { 0x59,         0x15        }, { 0x5A,          0x2C        },
};

ParseResult parseFar2lInput(GetChBuf &buf, TEvent &ev, InputState &state) noexcept
// Pre: "\x1B_f2l" has just been read.
{
    enum { k = 32 };
    char s[4*k];
    size_t len = 0;
    int c;
    while (c = buf.getUnbuffered(), c != -1 && c != '\x07')
        if (len < sizeof(s))
            s[len++] = (char) c;
    char o[3*k];
    TStringView out = decodeBase64({s, len}, o);
    if (!out.empty())
    {
        if (out.back() == 'K' && out.size() - 1 == 14)
        {
            KEY_EVENT_RECORD kev {};
            kev.bKeyDown = 1;
            memcpy(&kev.wRepeatCount,      &out[0],  2);
            memcpy(&kev.wVirtualKeyCode,   &out[2],  2);
            memcpy(&kev.wVirtualScanCode,  &out[4],  2);
            memcpy(&kev.dwControlKeyState, &out[6],  4);
            memcpy(&kev.uChar.UnicodeChar, &out[10], 4);

#ifdef TV_BIG_ENDIAN
            // The protocol states that "all integer values are in
            // little-endian format", so convert them.
            reverseBytes(kev.wRepeatCount);
            reverseBytes(kev.wVirtualKeyCode);
            reverseBytes(kev.wVirtualScanCode);
            reverseBytes(kev.dwControlKeyState);
            reverseBytes((uint32_t &) kev.uChar.UnicodeChar);
#endif

            if (uint16_t keyCode = virtualKeyCodeToKeyCode[kev.wVirtualKeyCode])
            {
                kev.wVirtualScanCode = keyCode >> 8;
                kev.uChar.UnicodeChar = keyCode & 0xFF;
            }
            else if (uint8_t scanCode = virtualKeyCodeToScanCode[kev.wVirtualKeyCode])
                kev.wVirtualScanCode = scanCode;

            if (getWin32Key(kev, ev, state))
            {
                TermIO::normalizeKey(ev.keyDown);
                return Accepted;
            }
        }
        else if (out.back() == 'M' && out.size() - 1 == 16)
        {
            MOUSE_EVENT_RECORD mev {};
            memcpy(&mev.dwMousePosition.X, &out[0],  2);
            memcpy(&mev.dwMousePosition.Y, &out[2],  2);
            memcpy(&mev.dwButtonState,     &out[4],  4);
            memcpy(&mev.dwControlKeyState, &out[8],  4);
            memcpy(&mev.dwEventFlags,      &out[12], 4);

#ifdef TV_BIG_ENDIAN
            reverseBytes((uint16_t &) mev.dwMousePosition.X);
            reverseBytes((uint16_t &) mev.dwMousePosition.Y);
            reverseBytes(mev.dwButtonState);
            reverseBytes(mev.dwControlKeyState);
            reverseBytes(mev.dwEventFlags);
#endif

            getWin32Mouse(mev, ev, state);
            return Accepted;
        }
    }
    return Ignored;
}

ParseResult parseFar2lAnswer(GetChBuf &buf, TEvent &ev, InputState &state) noexcept
// Pre: "\x1B_far2l" has just been read.
{
    ParseResult res = Ignored;
    if (char *s = TermIO::readUntilBelOrSt(buf))
    {
        TStringView encoded(s);
        if (encoded == "ok")
            state.far2l.enabled = true;
        else if (char *pDecoded = (char *) malloc((encoded.size() * 3)/4 + 3))
        {
            TStringView decoded = decodeBase64(encoded, pDecoded);
            if (decoded.size() >= 5 && decoded.back() == f2lClipGetData && state.putPaste)
            {
                uint32_t dataSize;
                memcpy(&dataSize, &decoded[decoded.size() - 5], 4);
#ifdef TV_BIG_ENDIAN
                reverseBytes(dataSize);
#endif
                if (dataSize < UINT_MAX - 5 && decoded.size() >= 5 + dataSize)
                {
                    TStringView text = decoded.substr(decoded.size() - 5 - dataSize, dataSize);
                    // Discard null terminator.
                    if (dataSize > 0 && text.back() == '\0')
                        text = text.substr(0, text.size() - 1);
                    state.putPaste(text);
                }
            }
            free(pDecoded);
        }
        free(s);
    }
    return res;
}

template <bool write = true, class... Args>
size_t concat(char *out, TStringView, Args ...args) noexcept;
template <bool write = true, class... Args>
size_t concat(char *out, char c, Args ...args) noexcept;
template <bool write = true, class... Args>
size_t concat(char *out, uint32_t i, Args ...args) noexcept;

template <bool write = true, class... Args>
inline size_t concat(char *out) noexcept
{
    return 0;
}

template <bool write, class... Args>
inline size_t concat(char *out, TStringView s, Args ...args) noexcept
{
    size_t len = s.size();
    if (write)
        memcpy(out, s.data(), len);
    return len + concat<write>(out + len, args...);
}

template <bool write, class... Args>
inline size_t concat(char *out, char c, Args ...args) noexcept
{
    size_t len = sizeof(c);
    if (write)
        memcpy(out, &c, len);
    return len + concat<write>(out + len, args...);
}

template <bool write, class... Args>
inline size_t concat(char *out, uint32_t i, Args ...args) noexcept
{
    size_t len = sizeof(i);
    if (write)
    {
#ifdef TV_BIG_ENDIAN
        reverseBytes(i);
#endif
        memcpy(out, &i, len);
    }
    return len + concat<write>(out + len, args...);
}

template <class... Args>
inline size_t concatLength(Args ...args) noexcept
{
    return concat<false>(nullptr, args...);
}

template <class... Args>
inline void pushFar2lRequest(std::vector<char> &out, std::vector<char> &tmp, Args ...args)
{
    size_t headLen = out.size();
    size_t argsLen = concatLength(args...);
    out.resize(headLen + argsLen);
    concat(&out[headLen], args...);
    tmp.resize((argsLen * 4)/3 + 4);
    TStringView b64 = encodeBase64({&out[headLen], argsLen}, &tmp[0]);
    TStringView prefix = "\x1B_far2l:";
    TStringView suffix = "\x1B\\";
    size_t pushLen = concatLength(prefix, b64, suffix);
    out.resize(headLen + pushLen);
    concat(&out[headLen], prefix, b64, suffix);
}

bool setFar2lClipboard(StdioCtl &io, TStringView text, InputState &state) noexcept
{
    if (state.far2l.enabled)
    {
        std::vector<char> out, tmp;
        // CLIP_OPEN
        pushFar2lRequest(out, tmp,
            f2lClientId,
            (uint32_t) f2lClientId.size(),
            "oc",
            f2lNoAnswer
        );
        // CLIP_SETDATA
        if (text.size() > UINT_MAX - 1)
            text = text.substr(0, UINT_MAX - 1);
        pushFar2lRequest(out, tmp,
            text,
            '\0',
            (uint32_t) (text.size() + 1),
            (uint32_t) CF_TEXT,
            "sc",
            f2lNoAnswer
        );
        // CLIP_CLOSE
        pushFar2lRequest(out, tmp,
            "cc",
            f2lNoAnswer
        );
        io.write(out.data(), out.size());
        return true;
    }
    return false;
}

bool requestFar2lClipboard(StdioCtl &io, InputState &state) noexcept
{
    if (state.far2l.enabled)
    {
        std::vector<char> out, tmp;
        // CLIP_OPEN
        pushFar2lRequest(out, tmp,
            f2lClientId,
            (uint32_t) f2lClientId.size(),
            "oc",
            f2lNoAnswer
        );
        // CLIP_GETDATA
        pushFar2lRequest(out, tmp,
            (uint32_t) CF_TEXT,
            "gc",
            f2lClipGetData
        );
        // CLIP_CLOSE
        pushFar2lRequest(out, tmp,
            "cc",
            f2lNoAnswer
        );
        io.write(out.data(), out.size());
        return true;
    }
    return false;
}

} // namespace tvision
