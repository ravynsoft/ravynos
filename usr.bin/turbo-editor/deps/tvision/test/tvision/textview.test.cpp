#define Uses_TRect
#define Uses_TTerminal
#include <tvision/tv.h>

#include <test.h>

struct TTerminalState
{
    ushort queBack;
    ushort queFront;
    char buffer[8 + 1];
};

class TestTerminal : public TTerminal
{
public:

    TestTerminal(const TTerminalState &state) :
        TTerminal(TRect(0, 0, 0, 0), nullptr, nullptr, sizeof(state.buffer) - 1)
    {
        memcpy(buffer, state.buffer, bufSize);
        queBack = state.queBack;
        queFront = state.queFront;
    }

};

struct PrevLinesTestInput
{
    TTerminalState state;
    ushort pos;
    ushort lines;
};

static std::ostream &operator<<(std::ostream &os, const TTerminalState &state)
{
    os << "{"
       << "queBack=" << state.queBack
       << ", queFront=" << state.queFront
       << ", buffer=" << ::testing::PrintToString((const char (&)[sizeof(state.buffer) - 1]) state.buffer)
       << "}";
    return os;
}

static std::ostream &operator<<(std::ostream &os, const PrevLinesTestInput &input)
{
    os << "{"
       << input.state
       << ", pos=" << input.pos
       << ", lines=" << input.lines
       << "}";
    return os;
}

TEST(TTerminal, prevLinesShouldWorkProperly)
{
    static constexpr TestCase<PrevLinesTestInput, ushort> testCases[] =
    {
        {{{0, 0, ""}, 0, 1}, 0},
        {{{0, 7, "ab\ncd\ne"}, 7, 1}, 6},
        {{{0, 7, "ab\ncd\ne"}, 6, 1}, 6},
        {{{0, 7, "ab\ncd\ne"}, 5, 1}, 3},
        {{{0, 7, "ab\ncd\ne"}, 4, 1}, 3},
        {{{0, 7, "ab\ncd\ne"}, 3, 1}, 3},
        {{{0, 7, "ab\ncd\ne"}, 2, 1}, 0},
        {{{0, 7, "ab\ncd\ne"}, 1, 1}, 0},
        {{{0, 7, "ab\ncd\ne"}, 0, 1}, 0},
        {{{0, 4, "a\nb\n"}, 4, 1}, 4},
        {{{0, 4, "a\nb\n"}, 4, 2}, 2},
        {{{0, 4, "a\nb\n"}, 4, 3}, 0},
        {{{0, 4, "a\nb\n"}, 3, 1}, 2},
        {{{0, 4, "a\nb\n"}, 3, 2}, 0},
        {{{0, 4, "a\nb\n"}, 3, 3}, 0},
        {{{4, 3, "aaa\0aaaa"}, 3, 1}, 4},
        {{{4, 3, "a\na\0aaaa"}, 5, 1}, 4},
        {{{7, 0, "\0\0\0\0\0\0\0a"}, 0, 1}, 7},
    };

    for (auto &testCase : testCases)
    {
        TestTerminal terminal(testCase.input.state);

        auto result = terminal.prevLines(testCase.input.pos, testCase.input.lines);
        expectResultMatches(result, testCase);
    }
}
