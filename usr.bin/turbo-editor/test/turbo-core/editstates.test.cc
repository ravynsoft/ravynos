#include <test/test.h>
#include <turbo/turbo.h>

namespace turbo
{

/////////////////////////////////////////////////////////////////////////
// Search

struct SearchTestInput
{
    std::string_view body;
    std::string_view textToSearch;
    SearchDirection direction {sdForward};
    SearchSettings searchSettings;
};

std::ostream &operator<<(std::ostream &os, const SearchTestInput &input)
{
    os << "Search '" << input.textToSearch << "' in:" << std::endl
       << input.body;
    return os;
}

static std::string searchOnce(const SearchTestInput &input)
{
    return modifyScintillaAndGetTextState(input.body, [&] (auto &scintilla) {
        search(scintilla, input.textToSearch, input.direction, input.searchSettings);
    });
}

static std::string searchOnceAndAgain(const SearchTestInput &input)
{
    return modifyScintillaAndGetTextState(input.body, [&] (auto &scintilla) {
        for (size_t i = 0; i < 2; ++i)
            search(scintilla, input.textToSearch, input.direction, input.searchSettings);
    });
}

static std::string searchIncremental(const SearchTestInput &input)
{
    return modifyScintillaAndGetTextState(input.body, [&] (auto &scintilla) {
        for (size_t i = 1; i <= input.textToSearch.size(); ++i)
            search(scintilla, input.textToSearch.substr(0, i), input.direction, input.searchSettings);
    });
}

TEST(Search, ShouldSearchForwardOnce)
{
    static constexpr TestCase<SearchTestInput, std::string_view> testCases[] =
    {
        {   {   "|1234567890\n",
                "",
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "i",
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "4",
            },

            "123^4|567890\n",
        },
        {   {   "1234|567890\n",
                "4",
            },

            "123^4|567890\n",
        },
        {   {   "1234|561\n",
                "1",
            },

            "123456^1|\n",
        },
        {   {   "1234561|\n",
                "1",
            },

            "^1|234561\n",
        },
        {   {   "|aBcAbC\n",
                "AbC",
             },

            "^aBc|AbC\n",
        },
        {   {   "|ñÇÑç\n",
                "Ñç",
             },

            "^ñÇ|Ñç\n",
        },
        // With previous selection
        {   {   "1234^5|67890\n",
                "",
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "i",
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "4",
            },

            "123^4|567890\n",
        },
        {   {   "^1|234561\n",
                "1",
            },

            "123456^1|\n",
        },
        {   {   "|1^234561\n",
                "1",
            },

            "123456^1|\n",
        },
        {   {   "^12|34561\n",
                "1",
            },

            "123456^1|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = searchOnce(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(Search, ShouldSearchForwardOnceAndAgain)
{
    static constexpr TestCase<SearchTestInput, std::string_view> testCases[] =
    {
        {   {   "|1234567890\n",
                "",
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "i",
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "4",
            },

            "123^4|567890\n",
        },
        {   {   "|1234561\n",
                "1",
            },

            "123456^1|\n",
        },
        {   {   "1|234561\n",
                "1",
            },

            "^1|234561\n",
        },
        // With previous selection
        {   {   "1234^5|67890\n",
                "",
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "i",
            },

            "1234^5|67890\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = searchOnceAndAgain(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(Search, ShouldSearchBackwardsOnce)
{
    static constexpr TestCase<SearchTestInput, std::string_view> testCases[] =
    {
        {   {   "|1234567890\n",
                "",
                sdBackwards,
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "i",
                sdBackwards,
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "4",
                sdBackwards,
            },

            "123^4|567890\n",
        },
        {   {   "1234|567890\n",
                "4",
                sdBackwards,
            },

            "123^4|567890\n",
        },
        {   {   "1234|561\n",
                "1",
                sdBackwards,
            },

            "^1|234561\n",
        },
        {   {   "1234561|\n",
                "1",
                sdBackwards,
            },

            "123456^1|\n",
        },
        // With previous selection
        {   {   "1234^5|67890\n",
                "",
                sdBackwards,
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "i",
                sdBackwards,
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "4",
                sdBackwards,
            },

            "123^4|567890\n",
        },
        {   {   "^1|234561\n",
                "1",
                sdBackwards,
            },

            "123456^1|\n",
        },
        {   {   "|1^234561\n",
                "1",
                sdBackwards,
            },

            "123456^1|\n",
        },
        {   {   "^12|34561\n",
                "1",
                sdBackwards,
            },

            "123456^1|\n",
        },
        {   {   "|12^34561\n",
                "1",
                sdBackwards,
            },

            "123456^1|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = searchOnce(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(Search, ShouldSearchBackwardsOnceAndAgain)
{
    static constexpr TestCase<SearchTestInput, std::string_view> testCases[] =
    {
        {   {   "|1234567890\n",
                "",
                sdBackwards,
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "i",
                sdBackwards,
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "4",
                sdBackwards,
            },

            "123^4|567890\n",
        },
        {   {   "1234561|\n",
                "1",
                sdBackwards,
            },

            "^1|234561\n",
        },
        {   {   "123456|1\n",
                "1",
                sdBackwards,
            },

            "123456^1|\n",
        },
        {   {   "|1234561\n",
                "1",
                sdBackwards,
            },

            "^1|234561\n",
        },
        // With previous selection
        {   {   "1234^5|67890\n",
                "",
                sdBackwards,
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "i",
                sdBackwards,
            },

            "1234^5|67890\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = searchOnceAndAgain(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(Search, ShouldSearchCaseSensitive)
{
    static constexpr TestCase<SearchTestInput, std::string_view> testCases[] =
    {
        {   {   "|aBcAbC\n",
                "",
                sdForward,
                {smPlainText, sfCaseSensitive},
            },

            "|aBcAbC\n",
        },
        {   {   "|aBcAbC\n",
                "AbC",
                sdForward,
                {smPlainText, sfCaseSensitive},
            },

            "aBc^AbC|\n",
        },
        {   {   "|ñÇÑç\n",
                "Ñç",
                sdForward,
                {smPlainText, sfCaseSensitive},
            },

            "ñÇ^Ñç|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = searchOnce(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(Search, ShouldSearchWholeWords)
{
    static constexpr TestCase<SearchTestInput, std::string_view> testCases[] =
    {
        {   {   "|aBcAbC\n",
                "",
                sdForward,
                {smWholeWords},
            },

            "|aBcAbC\n",
        },
        {   {   "|aBcAbC\n",
                "AbC",
                sdForward,
                {smWholeWords},
            },

            "|aBcAbC\n",
        },
        {   {   "|aBcD aBc AbCd AbC\n",
                "AbC",
                sdForward,
                {smWholeWords},
            },

            "aBcD ^aBc| AbCd AbC\n",
        },
        {   {   "|aBcD aBc AbCd AbC\n",
                "AbC",
                sdForward,
                {smWholeWords, sfCaseSensitive},
            },

            "aBcD aBc AbCd ^AbC|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = searchOnce(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(Search, ShouldSearchWithRegex)
{
    static constexpr TestCase<SearchTestInput, std::string_view> testCases[] =
    {
        {   {   "z|zxayyzz\n",
                ".*(a|xayy)",
                sdForward,
                {smRegularExpression},
            },

            "z^zxa|yyzz\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = searchOnce(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(Search, ShouldSearchWithRegexOnceAndAgain)
{
    static constexpr TestCase<SearchTestInput, std::string_view> testCases[] =
    {
        {   {   "z|zxayyzz\n",
                ".*(a|xayy)",
                sdForward,
                {smRegularExpression},
            },

            "^zzxa|yyzz\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = searchOnceAndAgain(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(Search, ShouldSearchIncremental)
{
    static constexpr TestCase<SearchTestInput, std::string_view> testCases[] =
    {
        {   {   "|1234567890\n",
                "",
                sdForwardIncremental,
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "i",
                sdForwardIncremental,
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "4",
                sdForwardIncremental,
            },

            "123^4|567890\n",
        },
        {   {   "|1234561\n",
                "1",
                sdForwardIncremental,
            },

            "^1|234561\n",
        },
        {   {   "1|234561\n",
                "1",
                sdForwardIncremental,
            },

            "123456^1|\n",
        },
        {   {   "|1234561\n",
                "124",
                sdForwardIncremental,
            },

            "12|34561\n",
        },
        {   {   "|1234561\n",
                "1245",
                sdForwardIncremental,
            },

            "12|34561\n",
        },
        {   {   "|1231241\n",
                "124",
                sdForwardIncremental,
            },

            "123^124|1\n",
        },
        {   {   "|123abc123\n",
                "123",
                sdForwardIncremental,
            },

            "^123|abc123\n",
        },
        // With previous selection
        {   {   "1234^5|67890\n",
                "",
                sdForwardIncremental,
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "i",
                sdForwardIncremental,
            },

            "12345|67890\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = searchIncremental(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

/////////////////////////////////////////////////////////////////////////
// Replace

struct ReplaceTestInput
{
    std::string_view body;
    std::string_view textToSearch;
    std::string_view textToReplaceWith;
    SearchSettings searchSettings;
};

std::ostream &operator<<(std::ostream &os, const ReplaceTestInput &input)
{
    os << "Replace '" << input.textToSearch << "' with '" << input.textToReplaceWith
       << "' in:" << std::endl
       << input.body;
    return os;
}

static std::string replaceOne(const ReplaceTestInput &input)
{
    return modifyScintillaAndGetTextState(input.body, [&] (auto &scintilla) {
        replace(scintilla, input.textToSearch, input.textToReplaceWith, rmReplaceOne, input.searchSettings);
    });
}

static std::string replaceAll(const ReplaceTestInput &input)
{
    return modifyScintillaAndGetTextState(input.body, [&] (auto &scintilla) {
        replace(scintilla, input.textToSearch, input.textToReplaceWith, rmReplaceAll, input.searchSettings);
    });
}

TEST(Replace, ShouldReplaceOne)
{
    static constexpr TestCase<ReplaceTestInput, std::string_view> testCases[] =
    {
        {   {   "|1234567890\n",
                "",
                "",
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "i",
                "j",
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "4",
                "aa",
            },

            "123^4|567890\n",
        },
        {   {   "1234|567890\n",
                "4",
                "aa",
            },

            "123^4|567890\n",
        },
        {   {   "123|4^567890\n",
                "4",
                "aa",
            },

            "123aa|567890\n",
        },
        {   {   "1234|561\n",
                "1",
                "aa",
            },

            "123456^1|\n",
        },
        {   {   "1234561|\n",
                "1",
                "aa",
            },

            "^1|234561\n",
        },
        {   {   "^1|234561\n",
                "1",
                "aa",
            },

            "aa23456^1|\n",
        },
        {   {   "^12|34561\n",
                "1",
                "aa",
            },

            "^1|234561\n",
        },
        {   {   "1234^5|67890\n",
                "",
                "aa",
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "i",
                "aa",
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "4",
                "aa",
            },

            "123^4|567890\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = replaceOne(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(Replace, ShouldReplaceAll)
{
    static constexpr TestCase<ReplaceTestInput, std::string_view> testCases[] =
    {
        {   {   "|1234567890\n",
                "",
                "",
            },

            "|1234567890\n",
        },
        {   {   "|1234567890\n",
                "i",
                "j",
            },

            "|1234567890\n",
        },
        {   {   "|12345647890\n",
                "4",
                "aa",
            },

            "|123aa56aa7890\n",
        },
        {   {   "1234|5647890\n",
                "4",
                "aa",
            },

            "123|aa56aa7890\n",
        },
        {   {   "123|4^5678490\n",
                "4",
                "aa",
            },

            "123|aa5678aa90\n",
        },
        {   {   "1234|561\n",
                "1",
                "aa",
            },

            "aa234|56aa\n",
        },
        {   {   "1234^5|67890\n",
                "",
                "aa",
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "i",
                "aa",
            },

            "1234^5|67890\n",
        },
        {   {   "1234^5|67890\n",
                "4",
                "aa",
            },

            "123aa^5|67890\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = replaceAll(testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

/////////////////////////////////////////////////////////////////////////
// Comment toggling

static std::string toggleComment(const Language &language, std::string_view input)
{
    auto &&inputState = TextState::decode(input);
    auto &scintilla = createScintilla(inputState);
    toggleComment(scintilla, &language);
    auto &&outputState = getTextState(scintilla);
    destroyScintilla(scintilla);
    return TextState::encode(std::move(outputState));
}

TEST(ToggleComment, ShouldRemoveHtmlBlockComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "<!DOCTYPE html>\n"
            "<!--<ht|ml>-->\n"
            "<head>\n",

            "<!DOCTYPE html>\n"
            "<ht|ml>\n"
            "<head>\n",
        },
        {   "<!DO^<!--CTYPE html>\n"
            "<html>\n"
            "<he-->|ad>\n",

            "<!DO^CTYPE html>\n"
            "<html>\n"
            "<he|ad>\n",
        },
        {   "<!DO|<!--CTYPE html>\n"
            "<html>\n"
            "<he-->^ad>\n",

            "<!DO|CTYPE html>\n"
            "<html>\n"
            "<he^ad>\n",
        },
        {   "<!DOCTYPE html>\n"
            "^<!--<html>-->\n"
            "|<head>\n",

            "<!DOCTYPE html>\n"
            "^<html>\n"
            "|<head>\n",
        },
        {   "^<!--\n"
            "\n"
            "-->\n"
            "|\n",

            "^\n"
            "\n"
            "\n"
            "|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::HTML, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(ToggleComment, ShouldRemoveCppBlockComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "^/*         */\n"
            "/*         */\n"
            "/*         */\n"
            "|/**/\n",

            "^         */\n"
            "/*         */\n"
            "/*         \n"
            "|/**/\n",
        },
        {   "void foo(int |/*unused*/^) { }\n",

            "void foo(int |unused^) { }\n",
        },
        {   "void foo(int |/* unused */^) { }\n",

            "void foo(int | unused ^) { }\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::CPP, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(ToggleComment, ShouldRemoveBashLineComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "|\n"
            "# echo a\n"
            "   #sleep 3\n"
            "\n"
            " #  cat^\n",

            "|\n"
            "echo a\n"
            "   sleep 3\n"
            "\n"
            "  cat^\n",
        },
        {   "\n"
            "# ech|o a\n"
            "   #sleep 3 # && echo b\n"
            "\n"
            " ^#  cat\n",

            "\n"
            "ech|o a\n"
            "   sleep 3 # && echo b\n"
            "\n"
            " ^ cat\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::Bash, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(ToggleComment, ShouldRemoveBatchLineComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "^rem echo a\n"
            "rem  echo b\n"
            "|\n",

            "^echo a\n"
            " echo b\n"
            "|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::Batch, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(ToggleComment, ShouldRemoveCppLineComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "// int i| = 0;\n",

            "int i| = 0;\n",
        },
        {   "|// int i = 0;\n"
            "// int j = 0;\n"
            "// int z = 0;^\n",

            "|int i = 0;\n"
            "int j = 0;\n"
            "int z = 0;^\n",
        },
        {   "// i^nt i = 0;\n"
            "// in|t j = 0;\n",

            "i^nt i = 0;\n"
            "in|t j = 0;\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::CPP, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(ToggleComment, ShouldInsertHtmlBlockComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "|\n",

            "<!--|-->\n",
        },
        {   "|<!DOCTYPE html>\n",

            "<!--|<!DOCTYPE html>-->\n",
        },
        {   "<|!DOCTYPE html>\n",

            "<!--<|!DOCTYPE html>-->\n",
        },
        {   "^\n"
            "\n"
            "\n"
            "|\n",

            "^<!--\n"
            "\n"
            "-->\n"
            "|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::HTML, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(ToggleComment, ShouldInsertCppBlockComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "void foo(int |unused^) { }\n",

            "void foo(int |/*unused*/^) { }\n",
        },
        {   "void foo(int | unused ^) { }\n",

            "void foo(int |/* unused */^) { }\n",
        },
        {   "^int i = 0;\n"
            "int j = 0|;\n",

            "^/*int i = 0;\n"
            "int j = 0*/|;\n",
        },
        {   "i^nt i = 0;\n"
            "int j = 0;\n"
            "|\n",

            "i^/*nt i = 0;\n"
            "int j = 0;*/\n"
            "|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::CPP, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(ToggleComment, ShouldInsertCppLineComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "|\n",

            "// |\n",
        },
        {   " |\n",

            "//  |\n",
        },
        {   "|int i = 0;\n",

            "// |int i = 0;\n",
        },
        {   "int i| = 0;\n",

            "// int i| = 0;\n",
        },
        {   "|int i = 0;\n"
            "int j = 0;\n"
            "int z = 0;^\n",

            "|// int i = 0;\n"
            "// int j = 0;\n"
            "// int z = 0;^\n",
        },
        {   "^ int i = 0;\n"
            "\n"
            "        int j = 0;\n"
            "    int z = 0;|\n",

            "^ // int i = 0;\n"
            " // \n"
            " //        int j = 0;\n"
            " //    int z = 0;|\n",
        },
        {   "|            int i = 0;\n"
            "\t\tint j = 0;^\n",

            "|  //           int i = 0;\n"
            "\t\t// int j = 0;^\n",
        },
        {   "int i = 0;|\n"
            "^\n",

            "// int i = 0;|\n"
            "^\n",
        },
        {   "int i = 0;^\n"
            "|\n",

            "// int i = 0;^\n"
            "|\n",
        },
        {   "|\n"
            "\n"
            "\n"
            "^\n",

            "|// \n"
            "// \n"
            "// \n"
            "^\n",
        }
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::CPP, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(ToggleComment, ShouldInsertBatchLineComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "ech|o a\n",

            "rem ech|o a\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::Batch, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

} // namespace turbo
