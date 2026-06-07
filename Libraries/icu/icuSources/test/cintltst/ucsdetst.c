// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
 ****************************************************************************
 * Copyright (c) 2005-2016, International Business Machines Corporation and *
 * others. All Rights Reserved.                                             *
 ****************************************************************************
 */

#include "unicode/utypes.h"

#include "unicode/ucsdet.h"
#include "unicode/ucnv.h"
#include "unicode/ustring.h"

#include "cintltst.h"
#include "cmemory.h"

#include <stdlib.h>
#include <string.h>

#define NEW_ARRAY(type,count) (type *) malloc((count) * sizeof(type))
#define DELETE_ARRAY(array) free(array)

static void TestConstruction(void);
static void TestUTF8(void);
static void TestUTF16(void);
static void TestC1Bytes(void);
static void TestInputFilter(void);
static void TestChaining(void);
static void TestBufferOverflow(void);
static void TestIBM424(void);
static void TestIBM420(void);
#if U_PLATFORM_IS_DARWIN_BASED
static void TestMailFilterCSS(void);
#endif

void addUCsdetTest(TestNode** root);

void addUCsdetTest(TestNode** root)
{
    addTest(root, &TestConstruction, "ucsdetst/TestConstruction");
    addTest(root, &TestUTF8, "ucsdetst/TestUTF8");
    addTest(root, &TestUTF16, "ucsdetst/TestUTF16");
    addTest(root, &TestC1Bytes, "ucsdetst/TestC1Bytes");
    addTest(root, &TestInputFilter, "ucsdetst/TestInputFilter");
    addTest(root, &TestChaining, "ucsdetst/TestErrorChaining");
    addTest(root, &TestBufferOverflow, "ucsdetst/TestBufferOverflow");
#if !UCONFIG_NO_LEGACY_CONVERSION
    addTest(root, &TestIBM424, "ucsdetst/TestIBM424");
    addTest(root, &TestIBM420, "ucsdetst/TestIBM420");
#endif
#if U_PLATFORM_IS_DARWIN_BASED
    addTest(root, &TestMailFilterCSS, "ucsdetst/TestMailFilterCSS");
#endif
}

static int32_t preflight(const UChar *src, int32_t length, UConverter *cnv)
{
    UErrorCode status;
    char buffer[1024];
    char *dest, *destLimit = buffer + sizeof(buffer);
    const UChar *srcLimit = src + length;
    int32_t result = 0;

    do {
        dest = buffer;
        status = U_ZERO_ERROR;
        ucnv_fromUnicode(cnv, &dest, destLimit, &src, srcLimit, 0, TRUE, &status);
        result += (int32_t) (dest - buffer);
    } while (status == U_BUFFER_OVERFLOW_ERROR);

    return result;
}

static char *extractBytes(const UChar *src, int32_t length, const char *codepage, int32_t *byteLength)
{
    UErrorCode status = U_ZERO_ERROR;
    UConverter *cnv = ucnv_open(codepage, &status);
    int32_t byteCount = preflight(src, length, cnv);
    const UChar *srcLimit = src + length;
    char *bytes = NEW_ARRAY(char, byteCount + 1);
    char *dest = bytes, *destLimit = bytes + byteCount + 1;

    ucnv_fromUnicode(cnv, &dest, destLimit, &src, srcLimit, 0, TRUE, &status);
    ucnv_close(cnv);

    *byteLength = byteCount;
    return bytes;
}

static void freeBytes(char *bytes)
{
    DELETE_ARRAY(bytes);
}

static void TestConstruction(void)
{
    UErrorCode status = U_ZERO_ERROR;
    UCharsetDetector *csd = ucsdet_open(&status);
    UEnumeration *e = ucsdet_getAllDetectableCharsets(csd, &status);
    const char *name;
    int32_t count = uenum_count(e, &status);
    int32_t i, length;

    for(i = 0; i < count; i += 1) {
        name = uenum_next(e, &length, &status);

        if(name == NULL || length <= 0) {
            log_err("ucsdet_getAllDetectableCharsets() returned a null or empty name!\n");
        }
    }
    /* one past the list of all names must return NULL */
    name = uenum_next(e, &length, &status);
    if(name != NULL || length != 0 || U_FAILURE(status)) {
        log_err("ucsdet_getAllDetectableCharsets(past the list) returned a non-null name!\n");
    }

    uenum_close(e);
    ucsdet_close(csd);
}

static void TestUTF8(void)
{
    UErrorCode status = U_ZERO_ERROR;
    static const char ss[] = "This is a string with some non-ascii characters that will "
               "be converted to UTF-8, then shoved through the detection process.  "
               "\\u0391\\u0392\\u0393\\u0394\\u0395"
               "Sure would be nice if our source could contain Unicode directly!";
    int32_t byteLength = 0, sLength = 0, dLength = 0;
    UChar s[sizeof(ss)];
    char *bytes;
    UCharsetDetector *csd = ucsdet_open(&status);
    const UCharsetMatch *match;
    UChar detected[sizeof(ss)];

    sLength = u_unescape(ss, s, sizeof(ss));
    bytes = extractBytes(s, sLength, "UTF-8", &byteLength);

    ucsdet_setText(csd, bytes, byteLength, &status);
    if (U_FAILURE(status)) {
        log_err("status is %s\n", u_errorName(status));
        goto bail;
    }

    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("Detection failure for UTF-8: got no matches.\n");
        goto bail;
    }

    dLength = ucsdet_getUChars(match, detected, sLength, &status);

    if (u_strCompare(detected, dLength, s, sLength, FALSE) != 0) {
        log_err("Round-trip test failed!\n");
    }

    ucsdet_setDeclaredEncoding(csd, "UTF-8", 5, &status); /* for coverage */

bail:
    freeBytes(bytes);
    ucsdet_close(csd);
}

static void TestUTF16(void)
{
    UErrorCode status = U_ZERO_ERROR;
    /* Notice the BOM on the start of this string */
    static const UChar chars[] = {
        0xFEFF, 0x0623, 0x0648, 0x0631, 0x0648, 0x0628, 0x0627, 0x002C,
        0x0020, 0x0628, 0x0631, 0x0645, 0x062c, 0x064a, 0x0627, 0x062a,
        0x0020, 0x0627, 0x0644, 0x062d, 0x0627, 0x0633, 0x0648, 0x0628,
        0x0020, 0x002b, 0x0020, 0x0627, 0x0646, 0x062a, 0x0631, 0x0646,
        0x064a, 0x062a, 0x0000};
    int32_t beLength = 0, leLength = 0, cLength = UPRV_LENGTHOF(chars);
    char *beBytes = extractBytes(chars, cLength, "UTF-16BE", &beLength);
    char *leBytes = extractBytes(chars, cLength, "UTF-16LE", &leLength);
    UCharsetDetector *csd = ucsdet_open(&status);
    const UCharsetMatch *match;
    const char *name;
    int32_t conf;

    ucsdet_setText(csd, beBytes, beLength, &status);
    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("Encoding detection failure for UTF-16BE: got no matches.\n");
        goto try_le;
    }

    name  = ucsdet_getName(match, &status);
    conf  = ucsdet_getConfidence(match, &status);

    if (strcmp(name, "UTF-16BE") != 0) {
        log_err("Encoding detection failure for UTF-16BE: got %s\n", name);
    }

    if (conf != 100) {
        log_err("Did not get 100%% confidence for UTF-16BE: got %d\n", conf);
    }

try_le:
    ucsdet_setText(csd, leBytes, leLength, &status);
    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("Encoding detection failure for UTF-16LE: got no matches.\n");
        goto bail;
    }

    name  = ucsdet_getName(match, &status);
    conf = ucsdet_getConfidence(match, &status);


    if (strcmp(name, "UTF-16LE") != 0) {
        log_err("Enconding detection failure for UTF-16LE: got %s\n", name);
    }

    if (conf != 100) {
        log_err("Did not get 100%% confidence for UTF-16LE: got %d\n", conf);
    }

bail:
    freeBytes(leBytes);
    freeBytes(beBytes);
    ucsdet_close(csd);
}

static void TestC1Bytes(void)
{
#if !UCONFIG_NO_LEGACY_CONVERSION
    UErrorCode status = U_ZERO_ERROR;
    static const char ssISO[] = "This is a small sample of some English text. Just enough to be sure that it detects correctly.";
    static const char ssWindows[] = "This is another small sample of some English text. Just enough to be sure that it detects correctly. It also includes some \\u201CC1\\u201D bytes.";
    int32_t sISOLength = 0, sWindowsLength = 0;
    UChar sISO[sizeof(ssISO)];
    UChar sWindows[sizeof(ssWindows)];
    int32_t lISO = 0, lWindows = 0;
    char *bISO;
    char *bWindows;
    UCharsetDetector *csd = ucsdet_open(&status);
    const UCharsetMatch *match;
    const char *name;

    sISOLength = u_unescape(ssISO, sISO, sizeof(ssISO));
    sWindowsLength = u_unescape(ssWindows, sWindows, sizeof(ssWindows));
    bISO = extractBytes(sISO, sISOLength, "ISO-8859-1", &lISO);
    bWindows = extractBytes(sWindows, sWindowsLength, "windows-1252", &lWindows);

    ucsdet_setText(csd, bWindows, lWindows, &status);
    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("English test with C1 bytes got no matches.\n");
        goto bail;
    }

    name  = ucsdet_getName(match, &status);

    if (strcmp(name, "windows-1252") != 0) {
        log_data_err("English text with C1 bytes does not detect as windows-1252, but as %s. (Are you missing data?)\n", name);
    }

    ucsdet_setText(csd, bISO, lISO, &status);
    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("English text without C1 bytes got no matches.\n");
        goto bail;
    }

    name  = ucsdet_getName(match, &status);

    if (strcmp(name, "ISO-8859-1") != 0) {
        log_err("English text without C1 bytes does not detect as ISO-8859-1, but as %s\n", name);
    }

bail:
    freeBytes(bWindows);
    freeBytes(bISO);

    ucsdet_close(csd);
#endif
}

static void TestInputFilter(void)
{
    UErrorCode status = U_ZERO_ERROR;
    static const char ss[] = "<a> <lot> <of> <English> <inside> <the> <markup> Un tr\\u00E8s petit peu de Fran\\u00E7ais. <to> <confuse> <the> <detector>";
    int32_t sLength = 0;
    UChar s[sizeof(ss)];
    int32_t byteLength = 0;
    char *bytes;
    UCharsetDetector *csd = ucsdet_open(&status);
    const UCharsetMatch *match;
    const char *lang, *name;

    sLength = u_unescape(ss, s, sizeof(ss));
    bytes = extractBytes(s, sLength, "ISO-8859-1", &byteLength);

    ucsdet_enableInputFilter(csd, TRUE);

    if (!ucsdet_isInputFilterEnabled(csd)) {
        log_err("ucsdet_enableInputFilter(csd, TRUE) did not enable input filter!\n");
    }


    ucsdet_setText(csd, bytes, byteLength, &status);
    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("Turning on the input filter resulted in no matches.\n");
        goto turn_off;
    }

    name = ucsdet_getName(match, &status);

    if (name == NULL || strcmp(name, "ISO-8859-1") != 0) {
        log_err("Turning on the input filter resulted in %s rather than ISO-8859-1\n", name);
    } else {
        lang = ucsdet_getLanguage(match, &status);

        if (lang == NULL || strcmp(lang, "fr") != 0) {
            log_err("Input filter did not strip markup!\n");
        }
    }

turn_off:
    ucsdet_enableInputFilter(csd, FALSE);
    ucsdet_setText(csd, bytes, byteLength, &status);
    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("Turning off the input filter resulted in no matches.\n");
        goto bail;
    }

    name = ucsdet_getName(match, &status);

    if (name == NULL || strcmp(name, "ISO-8859-1") != 0) {
        log_err("Turning off the input filter resulted in %s rather than ISO-8859-1\n", name);
    } else {
        lang = ucsdet_getLanguage(match, &status);

        if (lang == NULL || strcmp(lang, "en") != 0) {
            log_err("Unfiltered input did not detect as English!\n");
        }
    }

bail:
    freeBytes(bytes);
    ucsdet_close(csd);
}

static void TestChaining(void) {
    UErrorCode status = U_USELESS_COLLATOR_ERROR;

    ucsdet_open(&status);
    ucsdet_setText(NULL, NULL, 0, &status);
    ucsdet_getName(NULL, &status);
    ucsdet_getConfidence(NULL, &status);
    ucsdet_getLanguage(NULL, &status);
    ucsdet_detect(NULL, &status);
    ucsdet_setDeclaredEncoding(NULL, NULL, 0, &status);
    ucsdet_detectAll(NULL, NULL, &status);
    ucsdet_getUChars(NULL, NULL, 0, &status);
    ucsdet_getUChars(NULL, NULL, 0, &status);
    ucsdet_close(NULL);

    /* All of this code should have done nothing. */
    if (status != U_USELESS_COLLATOR_ERROR) {
        log_err("Status got changed to %s\n", u_errorName(status));
    }
}

static void TestBufferOverflow(void) {
    UErrorCode status = U_ZERO_ERROR;
    static const char *testStrings[] = {
        "\x80\x20\x54\x68\x69\x73\x20\x69\x73\x20\x45\x6E\x67\x6C\x69\x73\x68\x20\x1b", /* A partial ISO-2022 shift state at the end */
        "\x80\x20\x54\x68\x69\x73\x20\x69\x73\x20\x45\x6E\x67\x6C\x69\x73\x68\x20\x1b\x24", /* A partial ISO-2022 shift state at the end */
        "\x80\x20\x54\x68\x69\x73\x20\x69\x73\x20\x45\x6E\x67\x6C\x69\x73\x68\x20\x1b\x24\x28", /* A partial ISO-2022 shift state at the end */
        "\x80\x20\x54\x68\x69\x73\x20\x69\x73\x20\x45\x6E\x67\x6C\x69\x73\x68\x20\x1b\x24\x28\x44", /* A complete ISO-2022 shift state at the end with a bad one at the start */
        "\x1b\x24\x28\x44", /* A complete ISO-2022 shift state at the end */
        "\xa1", /* Could be a single byte shift-jis at the end */
        "\x74\x68\xa1", /* Could be a single byte shift-jis at the end */
        "\x74\x68\x65\xa1" /* Could be a single byte shift-jis at the end, but now we have English creeping in. */
    };
    static const char *testResults[] = {
        "windows-1252",
        "windows-1252",
        "windows-1252",
        "windows-1252",
        "ISO-2022-JP",
        NULL,
        NULL,
        "ISO-8859-1"
    };
    int32_t idx = 0;
    UCharsetDetector *csd = ucsdet_open(&status);
    const UCharsetMatch *match;

    ucsdet_setDeclaredEncoding(csd, "ISO-2022-JP", -1, &status);

    if (U_FAILURE(status)) {
        log_err("Couldn't open detector. %s\n", u_errorName(status));
        goto bail;
    }

    for (idx = 0; idx < UPRV_LENGTHOF(testStrings); idx++) {
        ucsdet_setText(csd, testStrings[idx], -1, &status);
        match = ucsdet_detect(csd, &status);

        if (match == NULL) {
            if (testResults[idx] != NULL) {
                log_err("Unexpectedly got no results at index %d.\n", idx);
            }
            else {
                log_verbose("Got no result as expected at index %d.\n", idx);
            }
            continue;
        }

        if (testResults[idx] == NULL || strcmp(ucsdet_getName(match, &status), testResults[idx]) != 0) {
            log_err("Unexpectedly got %s instead of %s at index %d with confidence %d.\n",
                ucsdet_getName(match, &status), testResults[idx], idx, ucsdet_getConfidence(match, &status));
            goto bail;
        }
    }

bail:
    ucsdet_close(csd);
}

static void TestIBM424(void)
{
    UErrorCode status = U_ZERO_ERROR;
    
    static const UChar chars[] = {
            0x05D4, 0x05E4, 0x05E8, 0x05E7, 0x05DC, 0x05D9, 0x05D8, 0x0020, 0x05D4, 0x05E6, 0x05D1, 0x05D0, 0x05D9, 0x0020, 0x05D4, 0x05E8,
            0x05D0, 0x05E9, 0x05D9, 0x002C, 0x0020, 0x05EA, 0x05EA, 0x0020, 0x05D0, 0x05DC, 0x05D5, 0x05E3, 0x0020, 0x05D0, 0x05D1, 0x05D9,
            0x05D7, 0x05D9, 0x0020, 0x05DE, 0x05E0, 0x05D3, 0x05DC, 0x05D1, 0x05DC, 0x05D9, 0x05D8, 0x002C, 0x0020, 0x05D4, 0x05D5, 0x05E8,
            0x05D4, 0x0020, 0x05E2, 0x05DC, 0x0020, 0x05E4, 0x05EA, 0x05D9, 0x05D7, 0x05EA, 0x0020, 0x05D7, 0x05E7, 0x05D9, 0x05E8, 0x05EA,
            0x0020, 0x05DE, 0x05E6, 0x0022, 0x05D7, 0x0020, 0x05D1, 0x05E2, 0x05E7, 0x05D1, 0x05D5, 0x05EA, 0x0020, 0x05E2, 0x05D3, 0x05D5,
            0x05D9, 0x05D5, 0x05EA, 0x0020, 0x05D7, 0x05D9, 0x05D9, 0x05DC, 0x05D9, 0x0020, 0x05E6, 0x05D4, 0x0022, 0x05DC, 0x0020, 0x05DE,
            0x05DE, 0x05D1, 0x05E6, 0x05E2, 0x0020, 0x05E2, 0x05D5, 0x05E4, 0x05E8, 0x05EA, 0x0020, 0x05D9, 0x05E6, 0x05D5, 0x05E7, 0x05D4,
            0x0020, 0x05D1, 0x002B, 0x0020, 0x05E8, 0x05E6, 0x05D5, 0x05E2, 0x05EA, 0x0020, 0x05E2, 0x05D6, 0x05D4, 0x002E, 0x0020, 0x05DC,
            0x05D3, 0x05D1, 0x05E8, 0x05D9, 0x0020, 0x05D4, 0x05E4, 0x05E6, 0x0022, 0x05E8, 0x002C, 0x0020, 0x05DE, 0x05D4, 0x05E2, 0x05D3,
            0x05D5, 0x05D9, 0x05D5, 0x05EA, 0x0020, 0x05E2, 0x05D5, 0x05DC, 0x05D4, 0x0020, 0x05EA, 0x05DE, 0x05D5, 0x05E0, 0x05D4, 0x0020,
            0x05E9, 0x05DC, 0x0020, 0x0022, 0x05D4, 0x05EA, 0x05E0, 0x05D4, 0x05D2, 0x05D5, 0x05EA, 0x0020, 0x05E4, 0x05E1, 0x05D5, 0x05DC,
            0x05D4, 0x0020, 0x05DC, 0x05DB, 0x05D0, 0x05D5, 0x05E8, 0x05D4, 0x0020, 0x05E9, 0x05DC, 0x0020, 0x05D7, 0x05D9, 0x05D9, 0x05DC,
            0x05D9, 0x05DD, 0x0020, 0x05D1, 0x05DE, 0x05D4, 0x05DC, 0x05DA, 0x0020, 0x05DE, 0x05D1, 0x05E6, 0x05E2, 0x0020, 0x05E2, 0x05D5,
            0x05E4, 0x05E8, 0x05EA, 0x0020, 0x05D9, 0x05E6, 0x05D5, 0x05E7, 0x05D4, 0x0022, 0x002E, 0x0020, 0x05DE, 0x05E0, 0x05D3, 0x05DC,
            0x05D1, 0x05DC, 0x05D9, 0x05D8, 0x0020, 0x05E7, 0x05D9, 0x05D1, 0x05DC, 0x0020, 0x05D0, 0x05EA, 0x0020, 0x05D4, 0x05D7, 0x05DC,
            0x05D8, 0x05EA, 0x05D5, 0x0020, 0x05DC, 0x05D0, 0x05D7, 0x05E8, 0x0020, 0x05E9, 0x05E2, 0x05D9, 0x05D9, 0x05DF, 0x0020, 0x05D1,
            0x05EA, 0x05DE, 0x05DC, 0x05D9, 0x05DC, 0x0020, 0x05D4, 0x05E2, 0x05D3, 0x05D5, 0x05D9, 0x05D5, 0x05EA, 0x0000
    };
    
    static const UChar chars_reverse[] = {
            0x05EA, 0x05D5, 0x05D9, 0x05D5, 0x05D3, 0x05E2, 0x05D4, 0x0020, 0x05DC, 0x05D9, 0x05DC, 0x05DE, 0x05EA,
            0x05D1, 0x0020, 0x05DF, 0x05D9, 0x05D9, 0x05E2, 0x05E9, 0x0020, 0x05E8, 0x05D7, 0x05D0, 0x05DC, 0x0020, 0x05D5, 0x05EA, 0x05D8,
            0x05DC, 0x05D7, 0x05D4, 0x0020, 0x05EA, 0x05D0, 0x0020, 0x05DC, 0x05D1, 0x05D9, 0x05E7, 0x0020, 0x05D8, 0x05D9, 0x05DC, 0x05D1,
            0x05DC, 0x05D3, 0x05E0, 0x05DE, 0x0020, 0x002E, 0x0022, 0x05D4, 0x05E7, 0x05D5, 0x05E6, 0x05D9, 0x0020, 0x05EA, 0x05E8, 0x05E4,
            0x05D5, 0x05E2, 0x0020, 0x05E2, 0x05E6, 0x05D1, 0x05DE, 0x0020, 0x05DA, 0x05DC, 0x05D4, 0x05DE, 0x05D1, 0x0020, 0x05DD, 0x05D9,
            0x05DC, 0x05D9, 0x05D9, 0x05D7, 0x0020, 0x05DC, 0x05E9, 0x0020, 0x05D4, 0x05E8, 0x05D5, 0x05D0, 0x05DB, 0x05DC, 0x0020, 0x05D4,
            0x05DC, 0x05D5, 0x05E1, 0x05E4, 0x0020, 0x05EA, 0x05D5, 0x05D2, 0x05D4, 0x05E0, 0x05EA, 0x05D4, 0x0022, 0x0020, 0x05DC, 0x05E9,
            0x0020, 0x05D4, 0x05E0, 0x05D5, 0x05DE, 0x05EA, 0x0020, 0x05D4, 0x05DC, 0x05D5, 0x05E2, 0x0020, 0x05EA, 0x05D5, 0x05D9, 0x05D5,
            0x05D3, 0x05E2, 0x05D4, 0x05DE, 0x0020, 0x002C, 0x05E8, 0x0022, 0x05E6, 0x05E4, 0x05D4, 0x0020, 0x05D9, 0x05E8, 0x05D1, 0x05D3,
            0x05DC, 0x0020, 0x002E, 0x05D4, 0x05D6, 0x05E2, 0x0020, 0x05EA, 0x05E2, 0x05D5, 0x05E6, 0x05E8, 0x0020, 0x002B, 0x05D1, 0x0020,
            0x05D4, 0x05E7, 0x05D5, 0x05E6, 0x05D9, 0x0020, 0x05EA, 0x05E8, 0x05E4, 0x05D5, 0x05E2, 0x0020, 0x05E2, 0x05E6, 0x05D1, 0x05DE,
            0x05DE, 0x0020, 0x05DC, 0x0022, 0x05D4, 0x05E6, 0x0020, 0x05D9, 0x05DC, 0x05D9, 0x05D9, 0x05D7, 0x0020, 0x05EA, 0x05D5, 0x05D9,
            0x05D5, 0x05D3, 0x05E2, 0x0020, 0x05EA, 0x05D5, 0x05D1, 0x05E7, 0x05E2, 0x05D1, 0x0020, 0x05D7, 0x0022, 0x05E6, 0x05DE, 0x0020,
            0x05EA, 0x05E8, 0x05D9, 0x05E7, 0x05D7, 0x0020, 0x05EA, 0x05D7, 0x05D9, 0x05EA, 0x05E4, 0x0020, 0x05DC, 0x05E2, 0x0020, 0x05D4,
            0x05E8, 0x05D5, 0x05D4, 0x0020, 0x002C, 0x05D8, 0x05D9, 0x05DC, 0x05D1, 0x05DC, 0x05D3, 0x05E0, 0x05DE, 0x0020, 0x05D9, 0x05D7,
            0x05D9, 0x05D1, 0x05D0, 0x0020, 0x05E3, 0x05D5, 0x05DC, 0x05D0, 0x0020, 0x05EA, 0x05EA, 0x0020, 0x002C, 0x05D9, 0x05E9, 0x05D0,
            0x05E8, 0x05D4, 0x0020, 0x05D9, 0x05D0, 0x05D1, 0x05E6, 0x05D4, 0x0020, 0x05D8, 0x05D9, 0x05DC, 0x05E7, 0x05E8, 0x05E4, 0x05D4,
            0x0000
    };

    int32_t bLength = 0, brLength = 0, cLength = UPRV_LENGTHOF(chars), crLength = UPRV_LENGTHOF(chars_reverse);
    
    char *bytes = extractBytes(chars, cLength, "IBM424", &bLength);
    char *bytes_r = extractBytes(chars_reverse, crLength, "IBM424", &brLength);
    
    UCharsetDetector *csd = ucsdet_open(&status);
    const UCharsetMatch *match;
    const char *name;

    ucsdet_setText(csd, bytes, bLength, &status);
    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("Encoding detection failure for IBM424_rtl: got no matches.\n");
        goto bail;
    }

    name  = ucsdet_getName(match, &status);
    if (strcmp(name, "IBM424_rtl") != 0) {
        log_data_err("Encoding detection failure for IBM424_rtl: got %s. (Are you missing data?)\n", name);
    }
    
    ucsdet_setText(csd, bytes_r, brLength, &status);
    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("Encoding detection failure for IBM424_ltr: got no matches.\n");
        goto bail;
    }

    name  = ucsdet_getName(match, &status);
    if (strcmp(name, "IBM424_ltr") != 0) {
        log_data_err("Encoding detection failure for IBM424_ltr: got %s. (Are you missing data?)\n", name);
    }

bail:
    freeBytes(bytes);
    freeBytes(bytes_r);
    ucsdet_close(csd);
}

static void TestIBM420(void)
{
    UErrorCode status = U_ZERO_ERROR;
    
    static const UChar chars[] = {
        0x0648, 0x064F, 0x0636, 0x0639, 0x062A, 0x0020, 0x0648, 0x0646, 0x064F, 0x0641, 0x0630, 0x062A, 0x0020, 0x0628, 0x0631, 0x0627,
        0x0645, 0x062C, 0x0020, 0x062A, 0x0623, 0x0645, 0x064A, 0x0646, 0x0020, 0x0639, 0x062F, 0x064A, 0x062F, 0x0629, 0x0020, 0x0641,
        0x064A, 0x0020, 0x0645, 0x0624, 0x0633, 0x0633, 0x0629, 0x0020, 0x0627, 0x0644, 0x062A, 0x0623, 0x0645, 0x064A, 0x0646, 0x0020,
        0x0627, 0x0644, 0x0648, 0x0637, 0x0646, 0x064A, 0x002C, 0x0020, 0x0645, 0x0639, 0x0020, 0x0645, 0x0644, 0x0627, 0x0626, 0x0645,
        0x062A, 0x0647, 0x0627, 0x0020, 0x062F, 0x0627, 0x0626, 0x0645, 0x0627, 0x064B, 0x0020, 0x0644, 0x0644, 0x0627, 0x062D, 0x062A,
        0x064A, 0x0627, 0x062C, 0x0627, 0x062A, 0x0020, 0x0627, 0x0644, 0x0645, 0x062A, 0x063A, 0x064A, 0x0631, 0x0629, 0x0020, 0x0644,
        0x0644, 0x0645, 0x062C, 0x062A, 0x0645, 0x0639, 0x0020, 0x0648, 0x0644, 0x0644, 0x062F, 0x0648, 0x0644, 0x0629, 0x002E, 0x0020,
        0x062A, 0x0648, 0x0633, 0x0639, 0x062A, 0x0020, 0x0648, 0x062A, 0x0637, 0x0648, 0x0631, 0x062A, 0x0020, 0x0627, 0x0644, 0x0645,
        0x0624, 0x0633, 0x0633, 0x0629, 0x0020, 0x0628, 0x0647, 0x062F, 0x0641, 0x0020, 0x0636, 0x0645, 0x0627, 0x0646, 0x0020, 0x0634,
        0x0628, 0x0643, 0x0629, 0x0020, 0x0623, 0x0645, 0x0627, 0x0646, 0x0020, 0x0644, 0x0633, 0x0643, 0x0627, 0x0646, 0x0020, 0x062F,
        0x0648, 0x0644, 0x0629, 0x0020, 0x0627, 0x0633, 0x0631, 0x0627, 0x0626, 0x064A, 0x0644, 0x0020, 0x0628, 0x0648, 0x062C, 0x0647,
        0x0020, 0x0627, 0x0644, 0x0645, 0x062E, 0x0627, 0x0637, 0x0631, 0x0020, 0x0627, 0x0644, 0x0627, 0x0642, 0x062A, 0x0635, 0x0627,
        0x062F, 0x064A, 0x0629, 0x0020, 0x0648, 0x0627, 0x0644, 0x0627, 0x062C, 0x062A, 0x0645, 0x0627, 0x0639, 0x064A, 0x0629, 0x002E,
        0x0000
    };
    static const UChar chars_reverse[] = {
        0x002E, 0x0629, 0x064A, 0x0639, 0x0627, 0x0645, 0x062A, 0x062C, 0x0627, 0x0644, 0x0627, 0x0648, 0x0020, 0x0629, 0x064A, 0x062F,
        0x0627, 0x0635, 0x062A, 0x0642, 0x0627, 0x0644, 0x0627, 0x0020, 0x0631, 0x0637, 0x0627, 0x062E, 0x0645, 0x0644, 0x0627, 0x0020,
        0x0647, 0x062C, 0x0648, 0x0628, 0x0020, 0x0644, 0x064A, 0x0626, 0x0627, 0x0631, 0x0633, 0x0627, 0x0020, 0x0629, 0x0644, 0x0648,
        0x062F, 0x0020, 0x0646, 0x0627, 0x0643, 0x0633, 0x0644, 0x0020, 0x0646, 0x0627, 0x0645, 0x0623, 0x0020, 0x0629, 0x0643, 0x0628,
        0x0634, 0x0020, 0x0646, 0x0627, 0x0645, 0x0636, 0x0020, 0x0641, 0x062F, 0x0647, 0x0628, 0x0020, 0x0629, 0x0633, 0x0633, 0x0624,
        0x0645, 0x0644, 0x0627, 0x0020, 0x062A, 0x0631, 0x0648, 0x0637, 0x062A, 0x0648, 0x0020, 0x062A, 0x0639, 0x0633, 0x0648, 0x062A,
        0x0020, 0x002E, 0x0629, 0x0644, 0x0648, 0x062F, 0x0644, 0x0644, 0x0648, 0x0020, 0x0639, 0x0645, 0x062A, 0x062C, 0x0645, 0x0644,
        0x0644, 0x0020, 0x0629, 0x0631, 0x064A, 0x063A, 0x062A, 0x0645, 0x0644, 0x0627, 0x0020, 0x062A, 0x0627, 0x062C, 0x0627, 0x064A,
        0x062A, 0x062D, 0x0627, 0x0644, 0x0644, 0x0020, 0x064B, 0x0627, 0x0645, 0x0626, 0x0627, 0x062F, 0x0020, 0x0627, 0x0647, 0x062A,
        0x0645, 0x0626, 0x0627, 0x0644, 0x0645, 0x0020, 0x0639, 0x0645, 0x0020, 0x002C, 0x064A, 0x0646, 0x0637, 0x0648, 0x0644, 0x0627,
        0x0020, 0x0646, 0x064A, 0x0645, 0x0623, 0x062A, 0x0644, 0x0627, 0x0020, 0x0629, 0x0633, 0x0633, 0x0624, 0x0645, 0x0020, 0x064A,
        0x0641, 0x0020, 0x0629, 0x062F, 0x064A, 0x062F, 0x0639, 0x0020, 0x0646, 0x064A, 0x0645, 0x0623, 0x062A, 0x0020, 0x062C, 0x0645,
        0x0627, 0x0631, 0x0628, 0x0020, 0x062A, 0x0630, 0x0641, 0x064F, 0x0646, 0x0648, 0x0020, 0x062A, 0x0639, 0x0636, 0x064F, 0x0648,
        0x0000,
    };
    
    int32_t bLength = 0, brLength = 0, cLength = UPRV_LENGTHOF(chars), crLength = UPRV_LENGTHOF(chars_reverse);
    
    char *bytes = extractBytes(chars, cLength, "IBM420", &bLength);
    char *bytes_r = extractBytes(chars_reverse, crLength, "IBM420", &brLength);
    
    UCharsetDetector *csd = ucsdet_open(&status);
    const UCharsetMatch *match;
    const char *name;

    ucsdet_setText(csd, bytes, bLength, &status);
    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("Encoding detection failure for IBM420_rtl: got no matches.\n");
        goto bail;
    }

    name  = ucsdet_getName(match, &status);
    if (strcmp(name, "IBM420_rtl") != 0) {
        log_data_err("Encoding detection failure for IBM420_rtl: got %s. (Are you missing data?)\n", name);
    }
    
    ucsdet_setText(csd, bytes_r, brLength, &status);
    match = ucsdet_detect(csd, &status);

    if (match == NULL) {
        log_err("Encoding detection failure for IBM420_ltr: got no matches.\n");
        goto bail;
    }

    name  = ucsdet_getName(match, &status);
    if (strcmp(name, "IBM420_ltr") != 0) {
        log_data_err("Encoding detection failure for IBM420_ltr: got %s. (Are you missing data?)\n", name);
    }

bail:
    freeBytes(bytes);
    freeBytes(bytes_r);
    ucsdet_close(csd);
}

#if U_PLATFORM_IS_DARWIN_BASED
#include <stdio.h>
// read data from file into a malloc'ed buf, which must be freed by caller.
// returns NULL if error. Copied from cbiapts.c
static void* dataBufFromFile(const char* path, long* dataBufSizeP) {
    FILE * dataFile;
    void * dataBuf;
    long dataBufSize, dataFileRead = 0;

    if (dataBufSizeP) {
        *dataBufSizeP = 0;
    }
    dataFile = fopen(path, "r");
    if (dataFile == NULL) {
        log_data_err("FAIL: for %s, fopen fails\n", path);
        return NULL;
    }
    fseek(dataFile, 0, SEEK_END);
    dataBufSize = ftell(dataFile);
    rewind(dataFile);

    dataBuf = uprv_malloc(dataBufSize);
    if (dataBuf != NULL) {
        dataFileRead = fread(dataBuf, 1, dataBufSize, dataFile);
    }
    fclose(dataFile);
    if (dataBuf == NULL) {
        log_data_err("FAIL: for %s, uprv_malloc fails for dataBuf[%ld]\n", path, dataBufSize);
        return NULL;
    }
    if (dataFileRead < dataBufSize) {
        log_data_err("FAIL: for %s, fread fails, read %ld of %ld\n", path, dataFileRead, dataBufSize);
        uprv_free(dataBuf);
        return NULL;
    }
    if (dataBufSizeP) {
        *dataBufSizeP = dataBufSize;
    }
    return dataBuf;
}

typedef struct {
    const char* sampleTextPath; // relative to cintltst directory
    const char* encodingName;   // expected
} SampleTextAndEncoding;

static const SampleTextAndEncoding mailSampleTests[] = {
    { "../testdata/encodingSamples/mailExample_Latin1_2.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1_3.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1_4.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1_6.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1_7.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1_8.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1_9.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1Esc_2.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1Esc_3.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1Esc_4.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1Esc_6.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1Esc_7.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1Esc_8.txt", "iso-8859-1" },
    { "../testdata/encodingSamples/mailExample_Latin1Esc_9.txt", "iso-8859-1" },
    // additions for <rdar://problem/56373519>
    { "../testdata/encodingSamples/mailExample_Latin1_11.txt",  "iso-8859-1" },
    { NULL, NULL }
};

static void TestMailFilterCSS(void) {
    UErrorCode status = U_ZERO_ERROR;
    UCharsetDetector *detector = ucsdet_open(&status);
    if (U_FAILURE(status)) {
        log_data_err("ucsdet_open fails. %s\n", u_errorName(status));
    } else {
        const SampleTextAndEncoding* testPtr;
        for (testPtr = mailSampleTests; testPtr->sampleTextPath != NULL; testPtr++) {
            long sampleTextLen;
            char * sampleText = (char *)dataBufFromFile(testPtr->sampleTextPath, &sampleTextLen);
            if (sampleText != NULL) { // dataBufFromFile reports the errors that would produce NULL
                status = U_ZERO_ERROR;
                ucsdet_setText(detector, sampleText, sampleTextLen, &status);
                if (U_FAILURE(status)) {
                    log_data_err("ucsdet_setText fails for text file %s: %s\n", testPtr->sampleTextPath, u_errorName(status));
                } else {
                    const UCharsetMatch *highestMatch = NULL;
                    ucsdet_enableInputFilter(detector, TRUE);
                    highestMatch = ucsdet_detect(detector, &status);
                    if (U_FAILURE(status) || highestMatch==NULL) {
                        log_err("ucsdet_detect fails for text file %s: %s\n", testPtr->sampleTextPath, u_errorName(status));
                    } else {
                        const char *icuName = ucsdet_getName(highestMatch, &status);
                        int32_t confidence = ucsdet_getConfidence(highestMatch, &status);
                        const char *langCode = ucsdet_getLanguage(highestMatch, &status);
                        if (U_FAILURE(status) || icuName==NULL) {
                            log_err("ucsdet_getName and/or ucsdet_getConfidence fails for text file %s: %s\n", testPtr->sampleTextPath, u_errorName(status));
                        } else {
                            log_info("For text file %s: expect %s; get %s with confidence %d, langCode %s; text length %ld\n",
                                    testPtr->sampleTextPath, testPtr->encodingName, icuName, confidence, langCode, sampleTextLen);
                        }
                    }
                }
                uprv_free(sampleText);
            }
        }
        ucsdet_close(detector);
    }
}
#endif /* U_PLATFORM_IS_DARWIN_BASED */
