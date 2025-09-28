#include <internal/base64.h>

#include <test.h>

namespace tvision
{

TEST(Base64, ShouldDecodeProperly)
{
    static const TestCase<TStringView> testCases[] =
    {
        {""        , ""      },
        {"Zg=="    , "f"     },
        {"Zm8="    , "fo"    },
        {"Zm9v"    , "foo"   },
        {"Zm9vYg==", "foob"  },
        {"Zm9vYmE=", "fooba" },
        {"Zm9vYmFy", "foobar"},
        {"Zg"      , "f"     },
        {"Zm8"     , "fo"    },
        {"Zm9vYg"  , "foob"  },
        {"Zm9vYmE" , "fooba" },
    };
    for (auto &testCase : testCases)
    {
        auto &&actual = decodeBase64(testCase.input);
        expectResultMatches(actual, testCase);
    }
}

TEST(Base64, ShouldEncodeProperly)
{
    static const TestCase<TStringView> testCases[] =
    {
        {""      , ""        },
        {"f"     , "Zg=="    },
        {"fo"    , "Zm8="    },
        {"foo"   , "Zm9v"    },
        {"foob"  , "Zm9vYg=="},
        {"fooba" , "Zm9vYmE="},
        {"foobar", "Zm9vYmFy"},
    };
    for (auto &testCase : testCases)
    {
        auto &&actual = encodeBase64(testCase.input);
        expectResultMatches(actual, testCase);
    }
}

} // namespace tvision
