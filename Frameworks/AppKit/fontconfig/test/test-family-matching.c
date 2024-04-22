/*
 * fontconfig/test/test-family-matching.c
 *
 * Copyright © 2020 Zoltan Vandrus
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fontconfig/fontconfig.h>

#define FC_TEST_RESULT "testresult"

typedef enum _TestMatchResult {
    TestMatch,
    TestNoMatch,
    TestMatchError
} TestMatchResult;

typedef enum _TestResult {
    TestPassed,
    TestFailed,
    TestError
} TestResult;

static TestMatchResult
TestMatchPattern (const char *test, FcPattern *p)
{
    const FcChar8 *xml_pre = (const FcChar8 *) ""
        "<fontconfig>\n"
        "  <match>\n"
        "";

    const FcChar8 *xml_post = (const FcChar8 *) ""
        "    <edit name=\""FC_TEST_RESULT"\">\n"
        "      <bool>true</bool>\n"
        "    </edit>\n"
        "  </match>\n"
        "</fontconfig>\n"
        "";

    FcPattern *pat = NULL;
    FcChar8 *concat = NULL;
    FcChar8 *xml = NULL;
    FcConfig *cfg = NULL;
    FcResult result;
    FcBool dummy;
    TestMatchResult ret = TestMatchError;

    pat = FcPatternDuplicate (p);
    if (!pat)
    {
        fprintf (stderr, "Unable to duplicate pattern.\n");
        goto bail;
    }

    concat = FcStrPlus (xml_pre, (const FcChar8 *) test);
    if (!concat)
    {
        fprintf (stderr, "Concatenation failed.\n");
        goto bail;
    }

    xml = FcStrPlus (concat, xml_post);
    if (!xml)
    {
        fprintf (stderr, "Concatenation failed.\n");
        goto bail;
    }

    cfg = FcConfigCreate ();
    if (!cfg)
    {
        fprintf (stderr, "Unable to create a new empty config.\n");
        goto bail;
    }

    if (!FcConfigParseAndLoadFromMemory (cfg, xml, FcTrue))
    {
        fprintf (stderr, "Unable to load a config from memory.\n");
        goto bail;
    }

    if (!FcConfigSubstitute (cfg, pat, FcMatchPattern))
    {
        fprintf (stderr, "Unable to substitute config.\n");
        goto bail;
    }

    result = FcPatternGetBool (pat, FC_TEST_RESULT, 0, &dummy);
    switch (result) {
    case FcResultMatch:
        ret = TestMatch;
        break;
    case FcResultNoMatch:
        ret = TestNoMatch;
        break;
    default:
        fprintf (stderr, "Unable to check pattern.\n");
        break;
    }

bail:
    if (cfg)
	FcConfigDestroy (cfg);
    if (xml)
	FcStrFree (xml);
    if (concat)
	FcStrFree (concat);
    if (pat)
	FcPatternDestroy (pat);
    return ret;
}

static TestResult
TestShouldMatchPattern(const char* test, FcPattern *pat, int negate)
{
    switch (TestMatchPattern (test, pat)) {
    case TestMatch:
        if (!negate) {
            return TestPassed;
        }
        else
        {
            printf ("Following test unexpectedly matched:\n%s", test);
            printf ("on\n");
            FcPatternPrint (pat);
            return TestFailed;
        }
        break;
    case TestNoMatch:
        if (!negate) {
            printf ("Following test should have matched:\n%s", test);
            printf ("on\n");
            FcPatternPrint (pat);
            return TestFailed;
        }
        else
        {
            return TestPassed;
        }
        break;
    case TestMatchError:
        return TestError;
        break;
    default:
        fprintf (stderr, "This shouldn't have been reached.\n");
        return TestError;
    }
}

#define MAX(a,b) ((a) > (b) ? (a) : (b))

static TestResult
UpdateResult (TestResult* res, TestResult resNew)
{
    *res = MAX(*res, resNew);
    return *res;
}

static TestResult
TestFamily (void)
{
    const char *test;
    TestResult res = TestPassed;

    FcPattern *pat = FcPatternBuild (NULL,
        FC_FAMILY, FcTypeString, "family1",
        FC_FAMILY, FcTypeString, "family2",
        FC_FAMILY, FcTypeString, "family3",
        NULL);

    if (!pat)
    {
        fprintf (stderr, "Unable to build pattern.\n");
        return TestError;
    }

    #define SHOULD_MATCH(p,t) \
        UpdateResult (&res, TestShouldMatchPattern (t, p, 0))
    #define SHOULD_NOT_MATCH(p,t) \
        UpdateResult (&res, TestShouldMatchPattern (t, p, 1))

    test = "<test qual=\"all\" name=\"family\" compare=\"not_eq\">\n"
           "    <string>foo</string>\n"
           "</test>\n"
           "";
    SHOULD_MATCH(pat, test);

    test = ""
           "<test qual=\"all\" name=\"family\" compare=\"not_eq\">\n"
           "    <string>family2</string>\n"
           "</test>\n"
           "";
    SHOULD_NOT_MATCH(pat, test);

    test = ""
           "<test qual=\"any\" name=\"family\" compare=\"eq\">\n"
           "    <string>family3</string>\n"
           "</test>\n"
           "";
    SHOULD_MATCH(pat, test);

    test = ""
           "<test qual=\"any\" name=\"family\" compare=\"eq\">\n"
           "    <string>foo</string>\n"
           "</test>\n"
           "";
    SHOULD_NOT_MATCH(pat, test);

    FcPatternDestroy (pat);
    return res;
}

int
main (void)
{
    return (TestFamily ());
}
