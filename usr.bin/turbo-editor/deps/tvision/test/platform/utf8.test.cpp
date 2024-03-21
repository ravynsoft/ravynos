#include <internal/utf8.h>

#include <test.h>

namespace tvision
{

TEST(Utf8, ShouldConvertUtf16StringToUtf8)
{
    static const TestCase<TSpan<const char>, TStringView> testCases[] =
    {
        {"", ""},
        // Use Unicode string literals so that the resulting UTF-16 characters
        // are encoded using the system's native endianess.
        {(const char (&)[8]) u"\u0061\u0062\u0063\u0064", "abcd"},
        {(const char (&)[8]) u"\u006F\U0001F495\u0057", "o💕W"},
    };

    for (auto &testCase : testCases)
    {
        TSpan<const uint16_t> u16Input {
            (const uint16_t *) testCase.input.data(),
            testCase.input.size()/2,
        };
        char *buf = new char[u16Input.size()*3];
        size_t length = utf16To8(u16Input, buf);
        TStringView actual {buf, length};
        expectResultMatches(actual, testCase);
        delete[] buf;
    }
}

} // namespace tvision
