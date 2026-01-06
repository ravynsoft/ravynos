#include <darwintest.h>

#include <string.h>

#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <IOKit/kext/misc_util.h>

T_GLOBAL_META(T_META_NAMESPACE("IOKitUser.kext"));

#pragma mark --- Prototypes ---

static CFComparisonResult compare_CFString(const void *val1, const void *val2, void *context);
static void test_OSKextCopySymbolReferences(const char * const path, const CFStringRef expectedSymbols[]);

#pragma mark --- Helper Functions ---

static CFComparisonResult compare_CFString(const void *val1, const void *val2, void *context){
    return CFStringCompare(val1, val2, 0);
}


/*!
 * @function test_OSKextCopySymbolReferences
 * @abstract
 * Generic helper function for testing OSKextCopySymbolReferences().
 *
 * @param path
 * path to kext bundle
 * @param expectedSymbols
 * NULL terminated C array of undefined symbol names that are expected  to be referenced by kext.
 * Pass NULL if it is expected that OSKextCopySymbolReferences()  would fail.
 * @discussion
 * Calls OSKextCopySymbolReferences() on with an OSKext derived from <path>.  It then ensures the number of symbols match the
 * number of expected.  It also ensures all <expectedSymbols> are present.  When comparing symbol lists, we ignore any kasan
 * symbols ('_asan_') as those can change depending on build tools.
 */
static void test_OSKextCopySymbolReferences(const char * const path, const CFStringRef expectedSymbols[]){
    size_t pathLength = 0;
    CFURLRef url = NULL;
    OSKextRef kext = NULL;
    CFMutableArrayRef symbols = NULL;
    CFMutableArrayRef expectedSymbolsArray = NULL;
    long symbolsCount = 0;
    long expectedSymbolsCount = 0;

    // Test setup

    T_SETUPBEGIN;

    T_QUIET; T_ASSERT_NOTNULL(path, "invalid test.  path == NULL");
    pathLength = strlen(path);

    if(expectedSymbols){
        // create expected symbol array

        for(int i=0; expectedSymbols[i] != NULL; ++i){
            ++expectedSymbolsCount;
        }

        expectedSymbolsArray = CFArrayCreateMutable(kCFAllocatorDefault, expectedSymbolsCount, &kCFTypeArrayCallBacks);
        T_QUIET; T_ASSERT_NOTNULL(expectedSymbolsArray, "failed to create array of expected symbols");

        CFArrayReplaceValues(expectedSymbolsArray, CFRangeMake(0, 0), (const void **)expectedSymbols, expectedSymbolsCount);
        T_QUIET; T_ASSERT_EQ(expectedSymbolsCount, (long)CFArrayGetCount(expectedSymbolsArray), "failed to populate expected symbols array");
    }

    // create OSKext from char* > CFURLRef

    url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8 *)path, pathLength, false);
    T_QUIET; T_ASSERT_NOTNULL(url, "failed to create CFURL for path: %s", path);

    kext = OSKextCreate(kCFAllocatorDefault, url);
    T_QUIET; T_ASSERT_NOTNULL(kext, "failed to create OSKext for path: %s", path);

    T_SETUPEND;


    // Testing begins here

    symbols = OSKextCopySymbolReferences(kext);
    if(expectedSymbols){
        T_ASSERT_NOTNULL(symbols, "non-NULL result from OSKextCopySymbolReferences(\"%s\")", path);
    } else{
        T_ASSERT_NULL(symbols, "NULL result from OSKextCopySymbolReferences(\"%s\")", path);
        goto cleanup;
    }

    // verify symbol count matches
    symbolsCount = CFArrayGetCount(symbols);
    if(expectedSymbolsCount != symbolsCount){
        // symbol counts don't match.  try again, ignoring kasan symbols.
        long nonKasanSymbolsCount = 0;
        for(int s=0; s<symbolsCount; ++s){
            CFStringRef symbol = CFArrayGetValueAtIndex(symbols, s);
            CFRange matchRange = CFStringFind(symbol, CFSTR("_asan_"), 0x0 /* CFStringCompareFlags */);
            if(matchRange.location == kCFNotFound){
                ++nonKasanSymbolsCount;
            }
        }
        T_ASSERT_EQ(expectedSymbolsCount, nonKasanSymbolsCount, "symbol count (ignoring kasan) matches expected: %ld", expectedSymbolsCount);
    } else{
        T_ASSERT_EQ(expectedSymbolsCount, symbolsCount, "symbol count matches expected: %ld", expectedSymbolsCount);
    }

    // sort arrays so it is easier to compare
    CFArraySortValues(symbols, CFRangeMake(0, symbolsCount), compare_CFString, NULL);
    CFArraySortValues(expectedSymbolsArray, CFRangeMake(0, expectedSymbolsCount), compare_CFString, NULL);

    // verify we have all the same symbols.  first mismatch asserts.
    // we walk each array of symbols separately since we may need to skip / ignore kasan symbols
    // in one array, but not the other.
    long symbolIndex = 0;
    long expectedSymbolIndex = 0;
    while(symbolIndex < symbolsCount && expectedSymbolIndex < expectedSymbolsCount){
        CFStringRef symbol = CFArrayGetValueAtIndex(symbols, symbolIndex);
        CFStringRef expectedSymbol = CFArrayGetValueAtIndex(expectedSymbolsArray, expectedSymbolIndex);

        CFComparisonResult compare = CFStringCompare(symbol, expectedSymbol, 0);
        if(compare != kCFCompareEqualTo){
            // symbols don't match... maybe we're looking at a kasan symbol we should be ignoring

            CFRange matchRange = CFStringFind(symbol, CFSTR("_asan_"), 0x0 /* CFStringCompareFlags */);
            if(matchRange.location != kCFNotFound){
                // ignore kasan symbols
                ++symbolIndex;
                continue;
            }

            // symbols really don't match.  try to give a better error message with the symbol names.

            char *symbolName = createUTF8CStringForCFString(symbol);
            char *expectedSymbolName = createUTF8CStringForCFString(expectedSymbol);

            if(symbolName && expectedSymbolName){
                T_ASSERT_FAIL("symbol mismatch! expecting: %s, actual: %s", expectedSymbolName, symbolName);
            } else{
                T_ASSERT_FAIL("symbol mismatch at sorted index: %ld, expected index: %ld.  unable to allocate memory for better log message", symbolIndex, expectedSymbolIndex);
            }

            // should never make it here... however, it is helpful during debugging
            // to change one of the T_ASSERT_FAIL()'s above to T_FAIL() and output
            // the remaining symbols that mismatch.  In that case, this clean-up is needed.
            SAFE_FREE_NULL(symbolName);
            SAFE_FREE_NULL(expectedSymbolName);
        }

        ++symbolIndex;
        ++expectedSymbolIndex;
    }

    T_QUIET; T_ASSERT_EQ(symbolIndex, symbolsCount, "symbol mismatch!  extra symbols encountered");
    T_QUIET; T_ASSERT_EQ(expectedSymbolIndex, expectedSymbolsCount, "symbol mismatch!  missing expected symbols");

    T_PASS("all symbols match!");

cleanup:
    SAFE_RELEASE(url);
    SAFE_RELEASE(kext);
    SAFE_RELEASE(symbols);
    SAFE_RELEASE(expectedSymbolsArray);
}


#pragma mark --- OSKextCopySymbolReferences() Tests ---

T_DECL(OSKextCopySymbolReferences_0_undefined,
       "check OSKextCopySymbolReferences() returns empty array for kext without any undefined symbols"
       )
{
    const char *path = "/AppleInternal/Tests/IOKitUser/Kexts/DoNothing.kext";
    const CFStringRef expectedSymbols[] = {
        NULL
    };

    test_OSKextCopySymbolReferences(path, expectedSymbols);
}

T_DECL(OSKextCopySymbolReferences_10_undefined,
       "check OSKextCopySymbolReferences() returns array of symbols for kext with undefined symbols"
       )
{
    const char *path = "/AppleInternal/Tests/IOKitUser/Kexts/DoNothing-10undef.kext";
    const CFStringRef expectedSymbols[] = {
        CFSTR("_do_nothing_1"),
        CFSTR("_do_nothing_2"),
        CFSTR("_do_nothing_3"),
        CFSTR("_do_nothing_4"),
        CFSTR("_do_nothing_5"),
        CFSTR("_do_nothing_6"),
        CFSTR("_do_nothing_7"),
        CFSTR("_do_nothing_8"),
        CFSTR("_do_nothing_9"),
        CFSTR("_do_nothing_10"),
        NULL
    };

    test_OSKextCopySymbolReferences(path, expectedSymbols);
}

T_DECL(OSKextCopySymbolReferences_10_undefined_KASAN,
       "KASAN: check OSKextCopySymbolReferences() returns array of symbols for kext with undefined symbols"
       )
{
    const char *path = "/AppleInternal/Tests/IOKitUser/Kexts/DoNothing-10undef.kext";
    const CFStringRef expectedSymbols[] = {
        CFSTR("_do_nothing_1"),
        CFSTR("_do_nothing_2"),
        CFSTR("_do_nothing_3"),
        CFSTR("_do_nothing_4"),
        CFSTR("_do_nothing_5"),
        CFSTR("_do_nothing_6"),
        CFSTR("_do_nothing_7"),
        CFSTR("_do_nothing_8"),
        CFSTR("_do_nothing_9"),
        CFSTR("_do_nothing_10"),
        NULL
    };

    // make sure we process the kasan binary
    OSKextSetExecutableSuffix("_kasan", NULL);

    test_OSKextCopySymbolReferences(path, expectedSymbols);

    // clear the suffix we set above
    OSKextSetExecutableSuffix(NULL, NULL);
}

