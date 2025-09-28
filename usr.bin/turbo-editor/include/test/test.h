#ifndef TURBO_TEST_H
#define TURBO_TEST_H

#include <gtest/gtest.h>

#include <turbo/scintilla/internals.h>
#include <turbo/scintilla.h>

namespace turbo
{

template <class T1, class T2 = T1>
struct TestCase
{
    T1 input;
    T2 result;
};

template <class T1, class T2, class T3>
inline void expectMatchingResult(const T1 &actual, const TestCase<T2, T3> &testCase)
{
    auto &expected = testCase.result;
    EXPECT_EQ(actual, expected) << "With test input:\n" << testCase.input;
}

struct TextState
{
    enum : char {
        chCaret = '|',
        chAnchor = '^',
    };

    std::string text;
    Sci::Position caret {-1};
    Sci::Position anchor {-1};

    static TextState decode(std::string_view);
    static std::string encode(TextState);
};

TScintilla &createScintilla(TextState state);
TextState getTextState(TScintilla &scintilla);

template <class Func>
std::string modifyScintillaAndGetTextState(std::string_view initialState, Func &&func)
{
    auto &&inputState = TextState::decode(initialState);
    auto &scintilla = createScintilla(inputState);
    func(scintilla);
    auto &&outputState = getTextState(scintilla);
    destroyScintilla(scintilla);
    return TextState::encode(std::move(outputState));
}

} // namespace turbo

#endif // TURBO_TEST_H
