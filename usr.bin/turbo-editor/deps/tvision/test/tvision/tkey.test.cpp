#define Uses_TKeys
#include <tvision/tv.h>

#include <test.h>

struct KeyCodeAndMods
{
    ushort code, mods;
};

static bool operator==(KeyCodeAndMods a, KeyCodeAndMods b)
{
    return a.code == b.code && a.mods == b.mods;
}

static std::ostream &operator<<(std::ostream &os, KeyCodeAndMods key)
{
    os << "{{";
    printKeyCode(os, key.code);
    os << "}, {";
    printControlKeyState(os, key.mods);
    os << "}}";
    return os;
}

const ushort
    kb0 = 0x0b30, kbA = 0x1e61, kbCtrlKpDiv = 0x3500, kbBiosCtrlN = 0x310e;

TEST(TKey, ShouldConstructProperly)
{
    static constexpr TestCase<KeyCodeAndMods> testCases[] =
    {
        {{kbNoKey}, {kbNoKey}},
        {{kbNoKey, kbShift | kbCtrlShift | kbAltShift}, {kbNoKey}},
        {{kbCtrlA}, {'A', kbCtrlShift}},
        {{kbCtrlA, kbCtrlShift}, {'A', kbCtrlShift}},
        {{kbCtrlA, kbShift | kbCtrlShift}, {'A', kbShift | kbCtrlShift}},
        {{kbCtrlA, kbAltShift}, {'A', kbCtrlShift | kbAltShift}},
        {{kbCtrlZ}, {'Z', kbCtrlShift}},
        {{kbCtrlZ, kbCtrlShift}, {'Z', kbCtrlShift}},
        {{kbCtrlZ, kbShift | kbCtrlShift}, {'Z', kbShift | kbCtrlShift}},
        {{kbCtrlZ, kbAltShift}, {'Z', kbCtrlShift | kbAltShift}},
        {{kbCtrlZ + 1}, {kbCtrlZ + 1}},
        {{kbCtrlZ + 1, kbShift | kbCtrlShift | kbAltShift}, {kbCtrlZ + 1, kbShift | kbCtrlShift | kbAltShift}},
        {{kbGrayPlus}, {kbGrayPlus}},
        {{kbGrayPlus, kbShift | kbCtrlShift | kbAltShift}, {kbGrayPlus, kbShift | kbCtrlShift | kbAltShift}},
        {{kbCtrlPrtSc}, {kbCtrlPrtSc, kbCtrlShift}},
        {{kbCtrlPrtSc, kbShift | kbCtrlShift | kbAltShift}, {kbCtrlPrtSc, kbShift | kbCtrlShift | kbAltShift}},
        {{'A'}, {'A'}},
        {{'A', kbShift}, {'A', kbShift}},
        {{'A', kbCtrlShift}, {'A', kbCtrlShift}},
        {{'A', kbShift | kbCtrlShift}, {'A', kbShift | kbCtrlShift}},
        {{'A', kbAltShift}, {'A', kbAltShift}},
        {{'A', kbShift | kbAltShift}, {'A', kbShift | kbAltShift}},
        {{'A', kbCtrlShift | kbAltShift}, {'A', kbCtrlShift | kbAltShift}},
        {{'A', kbShift | kbCtrlShift | kbAltShift}, {'A', kbShift | kbCtrlShift | kbAltShift}},
        {{'a'}, {'A'}},
        {{'a', kbShift}, {'A', kbShift}},
        {{'a', kbCtrlShift}, {'A', kbCtrlShift}},
        {{'a', kbShift | kbCtrlShift}, {'A', kbShift | kbCtrlShift}},
        {{'a', kbAltShift}, {'A', kbAltShift}},
        {{'a', kbShift | kbAltShift}, {'A', kbShift | kbAltShift}},
        {{'a', kbCtrlShift | kbAltShift}, {'A', kbCtrlShift | kbAltShift}},
        {{'a', kbShift | kbCtrlShift | kbAltShift}, {'A', kbShift | kbCtrlShift | kbAltShift}},
        {{'0'}, {'0'}},
        {{'0', kbShift}, {'0', kbShift}},
        {{'0', kbCtrlShift}, {'0', kbCtrlShift}},
        {{'0', kbShift | kbCtrlShift}, {'0', kbShift | kbCtrlShift}},
        {{'0', kbAltShift}, {'0', kbAltShift}},
        {{'0', kbShift | kbAltShift}, {'0', kbShift | kbAltShift}},
        {{'0', kbCtrlShift | kbAltShift}, {'0', kbCtrlShift | kbAltShift}},
        {{'0', kbShift | kbCtrlShift | kbAltShift}, {'0', kbShift | kbCtrlShift | kbAltShift}},
        {{kbA}, {'A'}},
        {{kbA, kbShift}, {'A', kbShift}},
        {{kbA, kbCtrlShift}, {'A', kbCtrlShift}},
        {{kbA, kbShift | kbCtrlShift}, {'A', kbShift | kbCtrlShift}},
        {{kbA, kbAltShift}, {'A', kbAltShift}},
        {{kbA, kbShift | kbAltShift}, {'A', kbShift | kbAltShift}},
        {{kbA, kbCtrlShift | kbAltShift}, {'A', kbCtrlShift | kbAltShift}},
        {{kbA, kbShift | kbCtrlShift | kbAltShift}, {'A', kbShift | kbCtrlShift | kbAltShift}},
        {{kb0}, {'0'}},
        {{kb0, kbShift}, {'0', kbShift}},
        {{kb0, kbCtrlShift}, {'0', kbCtrlShift}},
        {{kb0, kbShift | kbCtrlShift}, {'0', kbShift | kbCtrlShift}},
        {{kb0, kbAltShift}, {'0', kbAltShift}},
        {{kb0, kbShift | kbAltShift}, {'0', kbShift | kbAltShift}},
        {{kb0, kbCtrlShift | kbAltShift}, {'0', kbCtrlShift | kbAltShift}},
        {{kb0, kbShift | kbCtrlShift | kbAltShift}, {'0', kbShift | kbCtrlShift | kbAltShift}},
        {{kbRight, kbShift}, {kbRight, kbShift}},
        {{kbAltX}, {'X', kbAltShift}},
        {{kbTab}, {kbTab}},
        {{kbTab, kbShift}, {kbTab, kbShift}},
        {{kbShiftTab}, {kbTab, kbShift}},
        {{kbTab, kbCtrlShift}, {kbTab, kbCtrlShift}},
        {{kbTab, kbShift | kbCtrlShift}, {kbTab, kbShift | kbCtrlShift}},
        {{kbShiftTab, kbCtrlShift}, {kbTab, kbShift | kbCtrlShift}},
        {{kbShiftTab, kbShift | kbCtrlShift}, {kbTab, kbShift | kbCtrlShift}},
        {{kbCtrlTab, kbShift}, {kbTab, kbShift | kbCtrlShift}},
        {{kbTab, kbAltShift}, {kbTab, kbAltShift}},
        {{kbTab, kbShift | kbCtrlShift | kbAltShift}, {kbTab, kbShift | kbCtrlShift | kbAltShift}},
        {{kbShiftTab, kbAltShift}, {kbTab, kbShift | kbAltShift}},
        {{kbShiftTab, kbCtrlShift | kbAltShift}, {kbTab, kbShift | kbCtrlShift | kbAltShift}},
        {{kbCtrlTab, kbAltShift}, {kbTab, kbCtrlShift | kbAltShift}},
        {{kbCtrlTab, kbShift | kbAltShift}, {kbTab, kbShift | kbCtrlShift | kbAltShift}},
        {{kbEnter}, {kbEnter}},
        {{kbEnter, kbShift}, {kbEnter, kbShift}},
        {{kbEnter, kbCtrlShift}, {kbEnter, kbCtrlShift}},
        {{kbEnter, kbShift | kbCtrlShift}, {kbEnter, kbShift | kbCtrlShift}},
        {{kbCtrlEnter, kbShift}, {kbEnter, kbShift | kbCtrlShift}},
        {{kbEnter, kbAltShift}, {kbEnter, kbAltShift}},
        {{kbEnter, kbShift | kbCtrlShift | kbAltShift}, {kbEnter, kbShift | kbCtrlShift | kbAltShift}},
        {{kbCtrlEnter, kbAltShift}, {kbEnter, kbCtrlShift | kbAltShift}},
        {{kbCtrlEnter, kbShift | kbAltShift}, {kbEnter, kbShift | kbCtrlShift | kbAltShift}},
        {{kbTab, kbLeftCtrl}, {kbTab, kbCtrlShift}},
        {{kbTab, kbRightCtrl}, {kbTab, kbCtrlShift}},
        {{kbTab, kbLeftCtrl}, {kbTab, kbCtrlShift}},
        {{kbCtrlTab, kbLeftCtrl}, {kbTab, kbCtrlShift}},
        {{kbCtrlTab, kbRightCtrl}, {kbTab, kbCtrlShift}},
        {{kbCtrlTab, kbLeftCtrl}, {kbTab, kbCtrlShift}},
        {{kbA, kbLeftCtrl}, {'A', kbCtrlShift}},
        {{kbA, kbRightCtrl}, {'A', kbCtrlShift}},
        {{kbCtrlKpDiv}, {'/', kbCtrlShift}},
        {{kbBiosCtrlN}, {'N', kbCtrlShift}},
    };

    for (auto &testCase : testCases)
    {
        TKey key(testCase.input.code, testCase.input.mods);
        expectResultMatches(KeyCodeAndMods {key.code, key.mods}, testCase);
    }
}
