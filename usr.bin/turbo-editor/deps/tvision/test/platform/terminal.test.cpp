#define Uses_TKeys
#include <tvision/tv.h>

#include <internal/terminal.h>

#include <test.h>
#include "terminal.test.h"
#include <vector>

static bool operator==(const KeyDownEvent &a, const KeyDownEvent &b)
{
    return a.keyCode == b.keyCode && a.controlKeyState == b.controlKeyState
        && a.getText() == b.getText();
}

static std::ostream &operator<<(std::ostream &os, const KeyDownEvent &keyDown)
{
    os << "{{";
    printKeyCode(os, keyDown.keyCode);
    os << "}, {";
    printControlKeyState(os, keyDown.controlKeyState);
    os << "}, {";
    os << "\"" << keyDown.getText() << "\"";
    os << "}}";
    return os;
}

namespace tvision
{

TEST(TermIO, ShouldNormalizeKeys)
{
    static constexpr TestCase<KeyDownEvent> testCases[] =
    {
        {{'a', 0, {'a'}, 1}, {'a', 0, {'a'}, 1}},
        {{'a', kbShift, {'a'}, 1}, {'a', kbShift, {'a'}, 1}},
        {{'a', kbCtrlShift, {'a'}, 1}, {kbCtrlA, kbCtrlShift}},
        {{'a', kbLeftAlt, {'a'}, 1}, {kbAltA, kbLeftAlt}},
        {{kbIns, kbShift}, {kbShiftIns, kbShift}},
        {{kbBack, kbLeftCtrl | kbLeftAlt}, {kbAltBack, kbLeftCtrl | kbLeftAlt}},
        {{kbCtrlBack, kbLeftCtrl}, {kbCtrlBack, kbLeftCtrl}},
        {{kbCtrlBack}, {kbCtrlBack, kbCtrlShift}},
        {{kbIns, kbLeftCtrl | kbEnhanced}, {kbCtrlIns, kbLeftCtrl | kbEnhanced}},
        {{kbCtrlDel, kbLeftAlt}, {kbAltDel, kbCtrlShift | kbLeftAlt}},
    };

    for (auto &testCase : testCases)
    {
        KeyDownEvent actual = testCase.input;
        TermIO::normalizeKey(actual);
        expectResultMatches(actual, testCase);
    }
}

TEST(TermIO, ShouldReadWin32InputModeKeys)
{
    static const TestCase<TStringView, std::vector<TEvent>> testCases[] =
    {
        {"\x1B[65;30;65;1;16;1_", {keyDownEv(0x1e41, kbShift, "A")}},
        {"\x1B[65;30;65;1;16_", {keyDownEv(0x1e41, kbShift, "A")}},
        {"\x1B[16;42;0;0;0;1_", {}},
        {"\x1B[65;30;97;1;0;1_", {keyDownEv(0x1e61, 0x0000, "a")}},
        {"\x1B[65;30;97;1_", {keyDownEv(0x1e61, 0x0000, "a")}},
        {"\x1B[112;59;0;1;8;1_", {keyDownEv(kbCtrlF1, kbLeftCtrl, "")}},
        {"\x1B[112;59;;1;8_", {keyDownEv(kbCtrlF1, kbLeftCtrl, "")}},
        {"\x1B[112;59;0;0;8;1_", {}},
        // https://github.com/microsoft/terminal/issues/15083
        { // SGR mouse event
            "\x1B[0;0;27;1;0;1_" // \x1B[<0;52;12M
            "\x1B[0;0;91;1;0;1_"
            "\x1B[0;0;60;1;0;1_"
            "\x1B[0;0;48;1;0;1_"
            "\x1B[0;0;59;1;0;1_"
            "\x1B[0;0;53;1;0;1_"
            "\x1B[0;0;50;1;0;1_"
            "\x1B[0;0;59;1;0;1_"
            "\x1B[0;0;49;1;0;1_"
            "\x1B[0;0;50;1;0;1_"
            "\x1B[0;0;77;1;0;1_",
            {mouseEv({51, 11}, 0x0000, 0x0000, mbLeftButton, 0x0000)},
        },
        { // Paste event
            "\x1B[17;29;0;1;8;1_" // Ctrl (press)
            "\x1B[16;42;0;1;24;1_" // Shift (press)
            "\x1B[0;0;27;1;0;1_" // \x1B[200~ (bracketed paste begin)
            "\x1B[0;0;91;1;0;1_"
            "\x1B[0;0;50;1;0;1_"
            "\x1B[0;0;48;1;0;1_"
            "\x1B[0;0;48;1;0;1_"
            "\x1B[0;0;126;1;0;1_"
            "\x1B[65;30;97;1;0;1_" // 'a' (press)
            "\x1B[65;30;97;0;0;1_" // 'a' (release)
            "\x1B[0;0;27;1;0;1_" // \x1B[201~ (bracketed paste end)
            "\x1B[0;0;91;1;0;1_"
            "\x1B[0;0;50;1;0;1_"
            "\x1B[0;0;48;1;0;1_"
            "\x1B[0;0;49;1;0;1_"
            "\x1B[0;0;126;1;0;1_"
            "\x1B[86;47;22;0;24;1_" // Ctrl+Shift+V (release)
            "\x1B[17;29;0;0;16;1_" // Shift (release)
            "\x1B[16;42;0;0;0;1_", // Ctrl (release)
            {keyDownEv(0x1e61, kbPaste, "a")},
        },
    };

    for (auto &testCase : testCases)
    {
        StrInputGetter in(testCase.input);
        std::vector<TEvent> actual {};
        InputState state {};
        while (true)
        {
            GetChBuf buf(in);
            TEvent ev {};
            ParseResult result = TermIO::parseEvent(buf, ev, state);
            if (state.bracketedPaste && ev.what == evKeyDown)
                ev.keyDown.controlKeyState |= kbPaste;

            if (result == Accepted)
                actual.push_back(ev);
            else if (result == Rejected)
                break;
        }
        expectResultMatches(actual, testCase);
    }
}

} // namespace tvision
