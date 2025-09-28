#ifndef TVISION_TERMINAL_H
#define TVISION_TERMINAL_H

#define Uses_TPoint
#define Uses_TEvent
#include <tvision/tv.h>

#include <tvision/compat/windows/windows.h>

namespace tvision
{

class StdioCtl;

struct Far2lState
{
    bool enabled {false};
};

struct InputState
{
    uchar buttons {0};
#ifdef _WIN32
    wchar_t surrogate {0};
#endif
    Far2lState far2l;
    bool hasFullOsc52 {false};
    bool bracketedPaste {false};
    bool gotDsrResponse {false};
    void (*putPaste)(TStringView) {nullptr};
};

class InputGetter
{
public:

    virtual int get() noexcept = 0;
    virtual void unget(int) noexcept = 0;
};

class GetChBuf
{
    enum { maxSize = 31 };

    uint size {0};
    int keys[maxSize];

public:

    InputGetter &in;

    GetChBuf(InputGetter &aIn) noexcept :
        in(aIn)
    {
    }

    inline int getUnbuffered() noexcept;
    inline int get(bool keepErr = false) noexcept;
    inline int last(size_t i) noexcept;
    inline void unget() noexcept;
    void reject() noexcept;
    bool getNum(uint &) noexcept;
    bool getInt(int &) noexcept;
    bool readStr(TStringView) noexcept;
};

inline int GetChBuf::getUnbuffered() noexcept
{
    return in.get();
}

inline int GetChBuf::get(bool keepErr) noexcept
{
    if (size < maxSize)
    {
        int k = in.get();
        if (keepErr || k != -1)
            keys[size++] = k;
        return k;
    }
    return -1;
}

inline int GetChBuf::last(size_t i=0) noexcept
{
    if (i < size)
        return keys[size - 1 - i];
    return -1;
}

inline void GetChBuf::unget() noexcept
{
    int k;
    if (size && (k = keys[--size]) != -1)
        in.unget(k);
}

enum ParseResult { Rejected = 0, Accepted, Ignored };

struct CSIData
{
    // Represents the data stored in a CSI escape sequence:
    // \x1B [ _val[0] ; _val[1] ; ... terminator

    // CSIs can be longer, but this is the largest we need for now.
    enum { maxLength = 6 };

    uint _val[maxLength];
    uint terminator {0};
    uint length {0};

    bool readFrom(GetChBuf &buf) noexcept;
    inline uint getValue(uint i, uint defaultValue = 1) const noexcept;
};

inline uint CSIData::getValue(uint i, uint defaultValue) const noexcept
{
    return i < length && _val[i] != UINT_MAX ? _val[i] : defaultValue;
}

namespace TermIO
{
    void mouseOn(StdioCtl &) noexcept;
    void mouseOff(StdioCtl &) noexcept;
    void keyModsOn(StdioCtl &) noexcept;
    void keyModsOff(StdioCtl &) noexcept;

    void normalizeKey(KeyDownEvent &keyDown) noexcept;

    bool setClipboardText(StdioCtl &, TStringView, InputState &) noexcept;
    bool requestClipboardText(StdioCtl &, void (&)(TStringView), InputState &) noexcept;

    ParseResult parseEvent(GetChBuf&, TEvent&, InputState&) noexcept;
    ParseResult parseEscapeSeq(GetChBuf&, TEvent&, InputState&) noexcept;
    ParseResult parseX10Mouse(GetChBuf&, TEvent&, InputState&) noexcept;
    ParseResult parseSGRMouse(GetChBuf&, TEvent&, InputState&) noexcept;
    ParseResult parseCSIKey(const CSIData &csi, TEvent&, InputState&) noexcept;
    ParseResult parseFKeyA(GetChBuf&, TEvent&) noexcept;
    ParseResult parseSS3Key(GetChBuf&, TEvent&) noexcept;
    ParseResult parseArrowKeyA(GetChBuf&, TEvent&) noexcept;
    ParseResult parseFixTermKey(const CSIData &csi, TEvent&) noexcept;
    ParseResult parseDCS(GetChBuf&, InputState&) noexcept;
    ParseResult parseOSC(GetChBuf&, InputState&) noexcept;
    ParseResult parseCPR(const CSIData &csi, InputState&) noexcept;
    ParseResult parseWin32InputModeKeyOrEscapeSeq(const CSIData &, InputGetter&, TEvent&, InputState&) noexcept;

    char *readUntilBelOrSt(GetChBuf &) noexcept;
    void consumeUnprocessedInput(StdioCtl &, InputGetter &, InputState &) noexcept;
}

} // namespace tvision

#endif // TVISION_TERMINAL_H
