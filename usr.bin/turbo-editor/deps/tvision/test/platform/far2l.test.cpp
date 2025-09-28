#define Uses_TKeys
#include <tvision/tv.h>

#include <internal/far2l.h>

#include <test.h>
#include "terminal.test.h"

namespace tvision
{

const ushort
    kbS = 0x1f73, kb9 = 0x0a39;

TEST(Far2l, ShouldReadFar2lInput)
{
    static constexpr char longString[1024*1024] = {0};
    static const TestCase<TStringView, ParseResultEvent> testCases[] =
    {
        {"", {Ignored}},
        {"\x07", {Ignored}},
        {"çśdfç32rç€v\x07", {Ignored}},
        {{longString, sizeof(longString)}, {Ignored}},
        {"AQBTAAAAAAAAAHMAAAB=", {Ignored}},
        {"AQBTAAAAAAAAAHMAAABLaa==", {Ignored}},
        {"AQBTAAAAAAAAAHMAAHMAAABL", {Ignored}},
        {"AQBTAAAAAAAAAHMAAABL", {Accepted, keyDownEv(kbS, 0x0000, "s")}},
        {"AQBTAAAAAAAAAHMAAABL\x07", {Accepted, keyDownEv(kbS, 0x0000, "s")}},
        {"AQC+AAAAAAAAAKwgAABL\x07", {Accepted, keyDownEv(kbNoKey, 0x0000, "€")}},
        {"AQBWAAAACAAAAAAAAABL\x07", {Accepted, keyDownEv(kbCtrlV, kbLeftCtrl, "")}},
        {"AQA5AAAACAAAADkAAABL\x07", {Accepted, keyDownEv(kb9, kbLeftCtrl, "9")}},
        {"AQBWAAAACgAAAFYAAABL\x07", {Accepted, keyDownEv(kbAltV, kbLeftCtrl | kbLeftAlt, "")}},
        {"AQBWAAAACgAAAAAAAABL\x07", {Ignored}}, // AltGr + V, UnicodeChar = 0
        {"AQAMAAAAIgAAAAAAAABL\x07", {Ignored}}, // Alt + VK_CLEAR
        {"AAAGAAAAAAAAAAAAAQAAAE0=\x07", {Accepted, mouseEv({0, 6}, meMouseMoved, 0, 0, 0)}},
        {"CQANAAQAAAAAAAAAAAAAAE0=\x07", {Accepted, mouseEv({9, 13}, 0, 0, mbMiddleButton, 0)}},
        {"CQANAAAAAAAAAAAAAAAAAE0=\x07", {Accepted, mouseEv({9, 13}, 0, 0, 0, 0)}}, // Button release.
        {"AAAAAAAAAAAAAAAAAAAAAE0=\x07", {Accepted, mouseEv({0, 0}, 0, 0, 0, 0)}}, // Button release.
    };

    for (auto &testCase : testCases)
    {
        StrInputGetter in(testCase.input);
        GetChBuf buf(in);
        ParseResultEvent actual {};
        InputState state {};
        actual.parseResult = parseFar2lInput(buf, actual.ev, state);
        expectResultMatches(actual, testCase);
    }
}

} // namespace tvision
