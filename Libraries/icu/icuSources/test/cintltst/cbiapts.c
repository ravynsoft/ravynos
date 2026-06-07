// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/********************************************************************
 * COPYRIGHT:
 * Copyright (c) 1997-2016, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/********************************************************************************
*
* File CBIAPTS.C
*
* Modification History:
*        Name                     Description
*     Madhu Katragadda              Creation
*********************************************************************************/
/*C API TEST FOR BREAKITERATOR */
/**
* This is an API test.  It doesn't test very many cases, and doesn't
* try to test the full functionality.  It just calls each function in the class and
* verifies that it works on a basic level.
**/

#include "unicode/utypes.h"

#if !UCONFIG_NO_BREAK_ITERATION

#include <stdlib.h>
#include <string.h>
#include "unicode/uloc.h"
#include "unicode/ubrk.h"
#include "unicode/ustring.h"
#include "unicode/ucnv.h"
#include "unicode/utext.h"
#include "cintltst.h"
#include "cbiapts.h"
#include "cmemory.h"

#define TEST_ASSERT_SUCCESS(status) {if (U_FAILURE(status)) { \
log_data_err("Failure at file %s, line %d, error = %s (Are you missing data?)\n", __FILE__, __LINE__, u_errorName(status));}}

#define TEST_ASSERT(expr) {if ((expr)==FALSE) { \
log_data_err("Test Failure at file %s, line %d (Are you missing data?)\n", __FILE__, __LINE__);}}

#define APPLE_ADDITIONS 1

#if !UCONFIG_NO_FILE_IO
static void TestBreakIteratorSafeClone(void);
#endif
static void TestBreakIteratorRules(void);
static void TestBreakIteratorRuleError(void);
static void TestBreakIteratorStatusVec(void);
static void TestBreakIteratorUText(void);
static void TestBreakIteratorTailoring(void);
static void TestBreakIteratorRefresh(void);
static void TestBug11665(void);
static void TestBreakIteratorSuppressions(void);
#if APPLE_ADDITIONS
static void TestRuleBasedTokenizer(void);
#endif

void addBrkIterAPITest(TestNode** root);

void addBrkIterAPITest(TestNode** root)
{
#if !UCONFIG_NO_FILE_IO
    addTest(root, &TestBreakIteratorCAPI, "tstxtbd/cbiapts/TestBreakIteratorCAPI");
    addTest(root, &TestBreakIteratorSafeClone, "tstxtbd/cbiapts/TestBreakIteratorSafeClone");
    addTest(root, &TestBreakIteratorUText, "tstxtbd/cbiapts/TestBreakIteratorUText");
#endif
    addTest(root, &TestBreakIteratorRules, "tstxtbd/cbiapts/TestBreakIteratorRules");
    addTest(root, &TestBreakIteratorRuleError, "tstxtbd/cbiapts/TestBreakIteratorRuleError");
    addTest(root, &TestBreakIteratorStatusVec, "tstxtbd/cbiapts/TestBreakIteratorStatusVec");
    addTest(root, &TestBreakIteratorTailoring, "tstxtbd/cbiapts/TestBreakIteratorTailoring");
    addTest(root, &TestBreakIteratorRefresh, "tstxtbd/cbiapts/TestBreakIteratorRefresh");
    addTest(root, &TestBug11665, "tstxtbd/cbiapts/TestBug11665");
#if !UCONFIG_NO_FILTERED_BREAK_ITERATION
    addTest(root, &TestBreakIteratorSuppressions, "tstxtbd/cbiapts/TestBreakIteratorSuppressions");
#endif
#if APPLE_ADDITIONS
    addTest(root, &TestRuleBasedTokenizer, "tstxtbd/cbiapts/TestRuleBasedTokenizer");
#endif
}

#define CLONETEST_ITERATOR_COUNT 2

/*
 *   Utility function for converting char * to UChar * strings, to
 *     simplify the test code.   Converted strings are put in heap allocated
 *     storage.   A hook (probably a local in the caller's code) allows all
 *     strings converted with that hook to be freed with a single call.
 */
typedef struct StringStruct {
        struct StringStruct   *link;
        UChar                 str[1];
    } StringStruct;


static UChar* toUChar(const char *src, void **freeHook) {
    /* Structure of the memory that we allocate on the heap */

    int32_t    numUChars;
    int32_t    destSize;
    UChar      stackBuf[2000 + sizeof(void *)/sizeof(UChar)];
    StringStruct  *dest;
    UConverter *cnv;

    UErrorCode status = U_ZERO_ERROR;
    if (src == NULL) {
        return NULL;
    };

    cnv = ucnv_open(NULL, &status);
    if(U_FAILURE(status) || cnv == NULL) {
        return NULL;
    }
    ucnv_reset(cnv);
    numUChars = ucnv_toUChars(cnv,
                  stackBuf,
                  2000,
                  src, -1,
                  &status);

    destSize = (numUChars+1) * sizeof(UChar) + sizeof(struct StringStruct);
    dest = (StringStruct *)malloc(destSize);
    if (dest != NULL) {
        if (status == U_BUFFER_OVERFLOW_ERROR || status == U_STRING_NOT_TERMINATED_WARNING) {
            ucnv_toUChars(cnv, dest->str, numUChars+1, src, -1, &status);
        } else if (status == U_ZERO_ERROR) {
            u_strcpy(dest->str, stackBuf);
        } else {
            free(dest);
            dest = NULL;
        }
    }

    ucnv_reset(cnv); /* be good citizens */
    ucnv_close(cnv);
    if (dest == NULL) {
        return NULL;
    }

    dest->link = (StringStruct*)(*freeHook);
    *freeHook = dest;
    return dest->str;
}

static void freeToUCharStrings(void **hook) {
    StringStruct  *s = *(StringStruct **)hook;
    while (s != NULL) {
        StringStruct *next = s->link;
        free(s);
        s = next;
    }
}


#if !UCONFIG_NO_FILE_IO
static void TestBreakIteratorCAPI()
{
    UErrorCode status = U_ZERO_ERROR;
    UBreakIterator *word, *sentence, *line, *character, *b, *bogus;
    int32_t start,pos,end,to;
    int32_t i;
    int32_t count = 0;

    UChar text[50];

    /* Note:  the adjacent "" are concatenating strings, not adding a \" to the
       string, which is probably what whoever wrote this intended.  Don't fix,
       because it would throw off the hard coded break positions in the following
       tests. */
    u_uastrcpy(text, "He's from Africa. ""Mr. Livingston, I presume?"" Yeah");


/*test ubrk_open()*/
    log_verbose("\nTesting BreakIterator open functions\n");

    /* Use french for fun */
    word         = ubrk_open(UBRK_WORD, "en_US", text, u_strlen(text), &status);
    if(status == U_FILE_ACCESS_ERROR) {
        log_data_err("Check your data - it doesn't seem to be around\n");
        return;
    } else if(U_FAILURE(status)){
        log_err_status(status, "FAIL: Error in ubrk_open() for word breakiterator: %s\n", myErrorName(status));
    }
    else{
        log_verbose("PASS: Successfully opened  word breakiterator\n");
    }

    sentence     = ubrk_open(UBRK_SENTENCE, "en_US", text, u_strlen(text), &status);
    if(U_FAILURE(status)){
        log_err_status(status, "FAIL: Error in ubrk_open() for sentence breakiterator: %s\n", myErrorName(status));
        return;
    }
    else{
        log_verbose("PASS: Successfully opened  sentence breakiterator\n");
    }

    line         = ubrk_open(UBRK_LINE, "en_US", text, u_strlen(text), &status);
    if(U_FAILURE(status)){
        log_err("FAIL: Error in ubrk_open() for line breakiterator: %s\n", myErrorName(status));
        return;
    }
    else{
        log_verbose("PASS: Successfully opened  line breakiterator\n");
    }

    character     = ubrk_open(UBRK_CHARACTER, "en_US", text, u_strlen(text), &status);
    if(U_FAILURE(status)){
        log_err("FAIL: Error in ubrk_open() for character breakiterator: %s\n", myErrorName(status));
        return;
    }
    else{
        log_verbose("PASS: Successfully opened  character breakiterator\n");
    }
    /*trying to open an illegal iterator*/
    bogus     = ubrk_open((UBreakIteratorType)5, "en_US", text, u_strlen(text), &status);
    if(bogus != NULL) {
        log_err("FAIL: expected NULL from opening an invalid break iterator.\n");
    }
    if(U_SUCCESS(status)){
        log_err("FAIL: Error in ubrk_open() for BOGUS breakiterator. Expected U_ILLEGAL_ARGUMENT_ERROR\n");
    }
    if(U_FAILURE(status)){
        if(status != U_ILLEGAL_ARGUMENT_ERROR){
            log_err("FAIL: Error in ubrk_open() for BOGUS breakiterator. Expected U_ILLEGAL_ARGUMENT_ERROR\n Got %s\n", myErrorName(status));
        }
    }
    status=U_ZERO_ERROR;


/* ======= Test ubrk_countAvialable() and ubrk_getAvialable() */

    log_verbose("\nTesting ubrk_countAvailable() and ubrk_getAvailable()\n");
    count=ubrk_countAvailable();
    /* use something sensible w/o hardcoding the count */
    if(count < 0){
        log_err("FAIL: Error in ubrk_countAvialable() returned %d\n", count);
    }
    else{
        log_verbose("PASS: ubrk_countAvialable() successful returned %d\n", count);
    }
    for(i=0;i<count;i++)
    {
        log_verbose("%s\n", ubrk_getAvailable(i));
        if (ubrk_getAvailable(i) == 0)
            log_err("No locale for which breakiterator is applicable\n");
        else
            log_verbose("A locale %s for which breakiterator is applicable\n",ubrk_getAvailable(i));
    }

/*========Test ubrk_first(), ubrk_last()...... and other functions*/

    log_verbose("\nTesting the functions for word\n");
    start = ubrk_first(word);
    if(start!=0)
        log_err("error ubrk_start(word) did not return 0\n");
    log_verbose("first (word = %d\n", (int32_t)start);
       pos=ubrk_next(word);
    if(pos!=4)
        log_err("error ubrk_next(word) did not return 4\n");
    log_verbose("next (word = %d\n", (int32_t)pos);
    pos=ubrk_following(word, 4);
    if(pos!=5)
        log_err("error ubrl_following(word,4) did not return 6\n");
    log_verbose("next (word = %d\n", (int32_t)pos);
    end=ubrk_last(word);
    if(end!=49)
        log_err("error ubrk_last(word) did not return 49\n");
    log_verbose("last (word = %d\n", (int32_t)end);

    pos=ubrk_previous(word);
    log_verbose("%d   %d\n", end, pos);

    pos=ubrk_previous(word);
    log_verbose("%d \n", pos);

    if (ubrk_isBoundary(word, 2) != FALSE) {
        log_err("error ubrk_isBoundary(word, 2) did not return FALSE\n");
    }
    pos=ubrk_current(word);
    if (pos != 4) {
        log_err("error ubrk_current() != 4 after ubrk_isBoundary(word, 2)\n");
    }
    if (ubrk_isBoundary(word, 4) != TRUE) {
        log_err("error ubrk_isBoundary(word, 4) did not return TRUE\n");
    }



    log_verbose("\nTesting the functions for character\n");
    ubrk_first(character);
    pos = ubrk_following(character, 5);
    if(pos!=6)
       log_err("error ubrk_following(character,5) did not return 6\n");
    log_verbose("Following (character,5) = %d\n", (int32_t)pos);
    pos=ubrk_following(character, 18);
    if(pos!=19)
       log_err("error ubrk_following(character,18) did not return 19\n");
    log_verbose("Followingcharacter,18) = %d\n", (int32_t)pos);
    pos=ubrk_preceding(character, 22);
    if(pos!=21)
       log_err("error ubrk_preceding(character,22) did not return 21\n");
    log_verbose("preceding(character,22) = %d\n", (int32_t)pos);


    log_verbose("\nTesting the functions for line\n");
    pos=ubrk_first(line);
    if(pos != 0)
        log_err("error ubrk_first(line) returned %d, expected 0\n", (int32_t)pos);
    pos = ubrk_next(line);
    pos=ubrk_following(line, 18);
    if(pos!=22)
        log_err("error ubrk_following(line) did not return 22\n");
    log_verbose("following (line) = %d\n", (int32_t)pos);


    log_verbose("\nTesting the functions for sentence\n");
    pos = ubrk_first(sentence);
    pos = ubrk_current(sentence);
    log_verbose("Current(sentence) = %d\n", (int32_t)pos);
       pos = ubrk_last(sentence);
    if(pos!=49)
        log_err("error ubrk_last for sentence did not return 49\n");
    log_verbose("Last (sentence) = %d\n", (int32_t)pos);
    pos = ubrk_first(sentence);
    to = ubrk_following( sentence, 0 );
    if (to == 0) log_err("ubrk_following returned 0\n");
    to = ubrk_preceding( sentence, to );
    if (to != 0) log_err("ubrk_preceding didn't return 0\n");
    if (ubrk_first(sentence)!=ubrk_current(sentence)) {
        log_err("error in ubrk_first() or ubrk_current()\n");
    }


    /*---- */
    /*Testing ubrk_open and ubrk_close()*/
   log_verbose("\nTesting open and close for us locale\n");
    b = ubrk_open(UBRK_WORD, "fr_FR", text, u_strlen(text), &status);
    if (U_FAILURE(status)) {
        log_err("ubrk_open for word returned NULL: %s\n", myErrorName(status));
    }
    ubrk_close(b);

    /* Test setText and setUText */
    {
        UChar s1[] = {0x41, 0x42, 0x20, 0};
        UChar s2[] = {0x41, 0x42, 0x43, 0x44, 0x45, 0};
        UText *ut = NULL;
        UBreakIterator *bb;
        int j;

        log_verbose("\nTesting ubrk_setText() and ubrk_setUText()\n");
        status = U_ZERO_ERROR;
        bb = ubrk_open(UBRK_WORD, "en_US", NULL, 0, &status);
        TEST_ASSERT_SUCCESS(status);
        ubrk_setText(bb, s1, -1, &status);
        TEST_ASSERT_SUCCESS(status);
        ubrk_first(bb);
        j = ubrk_next(bb);
        TEST_ASSERT(j == 2);
        ut = utext_openUChars(ut, s2, -1, &status);
        ubrk_setUText(bb, ut, &status);
        TEST_ASSERT_SUCCESS(status);
        j = ubrk_next(bb);
        TEST_ASSERT(j == 5);

        ubrk_close(bb);
        utext_close(ut);
    }

    ubrk_close(word);
    ubrk_close(sentence);
    ubrk_close(line);
    ubrk_close(character);
}

static void TestBreakIteratorSafeClone(void)
{
    UChar text[51];     /* Keep this odd to test for 64-bit memory alignment */
                        /*  NOTE:  This doesn't reliably force mis-alignment of following items. */
    uint8_t buffer [CLONETEST_ITERATOR_COUNT] [U_BRK_SAFECLONE_BUFFERSIZE];
    int32_t bufferSize = U_BRK_SAFECLONE_BUFFERSIZE;

    UBreakIterator * someIterators [CLONETEST_ITERATOR_COUNT];
    UBreakIterator * someClonedIterators [CLONETEST_ITERATOR_COUNT];

    UBreakIterator * brk;
    UErrorCode status = U_ZERO_ERROR;
    int32_t start,pos;
    int32_t i;

    /*Testing ubrk_safeClone */

    /* Note:  the adjacent "" are concatenating strings, not adding a \" to the
       string, which is probably what whoever wrote this intended.  Don't fix,
       because it would throw off the hard coded break positions in the following
       tests. */
    u_uastrcpy(text, "He's from Africa. ""Mr. Livingston, I presume?"" Yeah");

    /* US & Thai - rule-based & dictionary based */
    someIterators[0] = ubrk_open(UBRK_WORD, "en_US", text, u_strlen(text), &status);
    if(!someIterators[0] || U_FAILURE(status)) {
      log_data_err("Couldn't open en_US word break iterator - %s\n", u_errorName(status));
      return;
    }

    someIterators[1] = ubrk_open(UBRK_WORD, "th_TH", text, u_strlen(text), &status);
    if(!someIterators[1] || U_FAILURE(status)) {
      log_data_err("Couldn't open th_TH word break iterator - %s\n", u_errorName(status));
      return;
    }

    /* test each type of iterator */
    for (i = 0; i < CLONETEST_ITERATOR_COUNT; i++)
    {

        /* Check the various error & informational states */

        /* Null status - just returns NULL */
        if (NULL != ubrk_safeClone(someIterators[i], buffer[i], &bufferSize, NULL))
        {
            log_err("FAIL: Cloned Iterator failed to deal correctly with null status\n");
        }
        /* error status - should return 0 & keep error the same */
        status = U_MEMORY_ALLOCATION_ERROR;
        if (NULL != ubrk_safeClone(someIterators[i], buffer[i], &bufferSize, &status) || status != U_MEMORY_ALLOCATION_ERROR)
        {
            log_err("FAIL: Cloned Iterator failed to deal correctly with incoming error status\n");
        }
        status = U_ZERO_ERROR;

        /* Null buffer size pointer is ok */
        if (NULL == (brk = ubrk_safeClone(someIterators[i], buffer[i], NULL, &status)) || U_FAILURE(status))
        {
            log_err("FAIL: Cloned Iterator failed to deal correctly with null bufferSize pointer\n");
        }
        ubrk_close(brk);
        status = U_ZERO_ERROR;

        /* buffer size pointer is 0 - fill in pbufferSize with a size */
        bufferSize = 0;
        if (NULL != ubrk_safeClone(someIterators[i], buffer[i], &bufferSize, &status) ||
                U_FAILURE(status) || bufferSize <= 0)
        {
            log_err("FAIL: Cloned Iterator failed a sizing request ('preflighting')\n");
        }
        /* Verify our define is large enough  */
        if (U_BRK_SAFECLONE_BUFFERSIZE < bufferSize)
        {
          log_err("FAIL: Pre-calculated buffer size is too small - %d but needed %d\n", U_BRK_SAFECLONE_BUFFERSIZE, bufferSize);
        }
        /* Verify we can use this run-time calculated size */
        if (NULL == (brk = ubrk_safeClone(someIterators[i], buffer[i], &bufferSize, &status)) || U_FAILURE(status))
        {
            log_err("FAIL: Iterator can't be cloned with run-time size\n");
        }
        if (brk)
            ubrk_close(brk);
        /* size one byte too small - should allocate & let us know */
        if (bufferSize > 1) {
            --bufferSize;
        }
        if (NULL == (brk = ubrk_safeClone(someIterators[i], NULL, &bufferSize, &status)) || status != U_SAFECLONE_ALLOCATED_WARNING)
        {
            log_err("FAIL: Cloned Iterator failed to deal correctly with too-small buffer size\n");
        }
        if (brk)
            ubrk_close(brk);
        status = U_ZERO_ERROR;
        bufferSize = U_BRK_SAFECLONE_BUFFERSIZE;

        /* Null buffer pointer - return Iterator & set error to U_SAFECLONE_ALLOCATED_ERROR */
        if (NULL == (brk = ubrk_safeClone(someIterators[i], NULL, &bufferSize, &status)) || status != U_SAFECLONE_ALLOCATED_WARNING)
        {
            log_err("FAIL: Cloned Iterator failed to deal correctly with null buffer pointer\n");
        }
        if (brk)
            ubrk_close(brk);
        status = U_ZERO_ERROR;

        /* Mis-aligned buffer pointer. */
        {
            char  stackBuf[U_BRK_SAFECLONE_BUFFERSIZE+sizeof(void *)];

            brk = ubrk_safeClone(someIterators[i], &stackBuf[1], &bufferSize, &status);
            if (U_FAILURE(status) || brk == NULL) {
                log_err("FAIL: Cloned Iterator failed with misaligned buffer pointer\n");
            }
            if (status == U_SAFECLONE_ALLOCATED_WARNING) {
                log_verbose("Cloned Iterator allocated when using a mis-aligned buffer.\n");
            }
            if (brk)
                ubrk_close(brk);
        }


        /* Null Iterator - return NULL & set U_ILLEGAL_ARGUMENT_ERROR */
        if (NULL != ubrk_safeClone(NULL, buffer[i], &bufferSize, &status) || status != U_ILLEGAL_ARGUMENT_ERROR)
        {
            log_err("FAIL: Cloned Iterator failed to deal correctly with null Iterator pointer\n");
        }
        status = U_ZERO_ERROR;

        /* Do these cloned Iterators work at all - make a first & next call */
        bufferSize = U_BRK_SAFECLONE_BUFFERSIZE;
        someClonedIterators[i] = ubrk_safeClone(someIterators[i], buffer[i], &bufferSize, &status);

        start = ubrk_first(someClonedIterators[i]);
        if(start!=0)
            log_err("error ubrk_start(clone) did not return 0\n");
        pos=ubrk_next(someClonedIterators[i]);
        if(pos!=4)
            log_err("error ubrk_next(clone) did not return 4\n");

        ubrk_close(someClonedIterators[i]);
        ubrk_close(someIterators[i]);
    }
}
#endif


/*
//  Open a break iterator from char * rules.  Take care of conversion
//     of the rules and error checking.
*/
static UBreakIterator * testOpenRules(char *rules) {
    UErrorCode      status       = U_ZERO_ERROR;
    UChar          *ruleSourceU  = NULL;
    void           *strCleanUp   = NULL;
    UParseError     parseErr;
    UBreakIterator *bi;

    ruleSourceU = toUChar(rules, &strCleanUp);

    bi = ubrk_openRules(ruleSourceU,  -1,     /*  The rules  */
                        NULL,  -1,            /*  The text to be iterated over. */
                        &parseErr, &status);

    if (U_FAILURE(status)) {
        log_data_err("FAIL: ubrk_openRules: ICU Error \"%s\" (Are you missing data?)\n", u_errorName(status));
        bi = 0;
    };
    freeToUCharStrings(&strCleanUp);
    return bi;

}

/*
 *  TestBreakIteratorRules - Verify that a break iterator can be created from
 *                           a set of source rules.
 */
static void TestBreakIteratorRules() {
    /*  Rules will keep together any run of letters not including 'a', OR
     *             keep together 'abc', but only when followed by 'def', OTHERWISE
     *             just return one char at a time.
     */
    char         rules[]  = "abc/def{666};\n   [\\p{L} - [a]]* {2};  . {1};";
    /*                        0123456789012345678 */
    char         data[]   =  "abcdex abcdefgh-def";     /* the test data string                     */
    char         breaks[] =  "**    **  *    **  *";    /*  * the expected break positions          */
    char         tags[]   =  "01    21  6    21  2";    /*  expected tag values at break positions  */
    int32_t      tagMap[] = {0, 1, 2, 3, 4, 5, 666};

    UChar       *uData;
    void        *freeHook = NULL;
    UErrorCode   status   = U_ZERO_ERROR;
    int32_t      pos;
    int          i;

    UBreakIterator *bi = testOpenRules(rules);
    if (bi == NULL) {return;}
    uData = toUChar(data, &freeHook);
    ubrk_setText(bi,  uData, -1, &status);

    pos = ubrk_first(bi);
    for (i=0; i<sizeof(breaks); i++) {
        if (pos == i && breaks[i] != '*') {
            log_err("FAIL: unexpected break at position %d found\n", pos);
            break;
        }
        if (pos != i && breaks[i] == '*') {
            log_err("FAIL: expected break at position %d not found.\n", i);
            break;
        }
        if (pos == i) {
            int32_t tag, expectedTag;
            tag = ubrk_getRuleStatus(bi);
            expectedTag = tagMap[tags[i]&0xf];
            if (tag != expectedTag) {
                log_err("FAIL: incorrect tag value.  Position = %d;  expected tag %d, got %d",
                    pos, expectedTag, tag);
                break;
            }
            pos = ubrk_next(bi);
        }
    }

    /* #12914 add basic sanity test for ubrk_getBinaryRules, ubrk_openBinaryRules */
    /* Underlying functionality checked in C++ rbbiapts.cpp TestRoundtripRules */
    status = U_ZERO_ERROR;
    int32_t rulesLength = ubrk_getBinaryRules(bi, NULL, 0, &status); /* preflight */
    if (U_FAILURE(status)) {
        log_err("FAIL: ubrk_getBinaryRules preflight err: %s", u_errorName(status));
    } else {
        uint8_t* binaryRules = (uint8_t*)uprv_malloc(rulesLength);
        if (binaryRules == NULL) {
            log_err("FAIL: unable to malloc rules buffer, size %u", rulesLength);
        } else {
            rulesLength = ubrk_getBinaryRules(bi, binaryRules, rulesLength, &status);
            if (U_FAILURE(status)) {
                log_err("FAIL: ubrk_getBinaryRules err: %s", u_errorName(status));
            } else {
                UBreakIterator* bi2 = ubrk_openBinaryRules(binaryRules, rulesLength, uData, -1, &status);
                if (U_FAILURE(status)) {
                    log_err("FAIL: ubrk_openBinaryRules err: %s", u_errorName(status));
                } else {
                    int32_t maxCount = sizeof(breaks); /* fail-safe test limit */
                    int32_t pos2 = ubrk_first(bi2);
                    pos = ubrk_first(bi);
                    do {
                        if (pos2 != pos) {
                            log_err("FAIL: interator from ubrk_openBinaryRules does not match original, get pos = %d instead of %d", pos2, pos);
                        }
                        pos2 = ubrk_next(bi2);
                        pos = ubrk_next(bi);
                    } while ((pos != UBRK_DONE || pos2 != UBRK_DONE) && maxCount-- > 0);

                    ubrk_close(bi2);
                }
            }
            uprv_free(binaryRules);
        }
    }

    freeToUCharStrings(&freeHook);
    ubrk_close(bi);
}

static void TestBreakIteratorRuleError() {
/*
 *  TestBreakIteratorRuleError -   Try to create a BI from rules with syntax errors,
 *                                 check that the error is reported correctly.
 */
    char            rules[]  = "           #  This is a rule comment on line 1\n"
                               "[:L:];     # this rule is OK.\n"
                               "abcdefg);  # Error, mismatched parens\n";
    UChar          *uRules;
    void           *freeHook = NULL;
    UErrorCode      status   = U_ZERO_ERROR;
    UParseError     parseErr;
    UBreakIterator *bi;

    uRules = toUChar(rules, &freeHook);
    bi = ubrk_openRules(uRules,  -1,          /*  The rules  */
                        NULL,  -1,            /*  The text to be iterated over. */
                        &parseErr, &status);
    if (U_SUCCESS(status)) {
        log_err("FAIL: construction of break iterator succeeded when it should have failed.\n");
        ubrk_close(bi);
    } else {
        if (parseErr.line != 3 || parseErr.offset != 8) {
            log_data_err("FAIL: incorrect error position reported. Got line %d, char %d, expected line 3, char 7 (Are you missing data?)\n",
                parseErr.line, parseErr.offset);
        }
    }
    freeToUCharStrings(&freeHook);
}


/*
*   TestsBreakIteratorStatusVals()   Test the ubrk_getRuleStatusVec() funciton
*/
static void TestBreakIteratorStatusVec() {
    #define RULE_STRING_LENGTH 200
    UChar          rules[RULE_STRING_LENGTH];

    #define TEST_STRING_LENGTH 25
    UChar           testString[TEST_STRING_LENGTH];
    UBreakIterator *bi        = NULL;
    int32_t         pos       = 0;
    int32_t         vals[10];
    int32_t         numVals;
    UErrorCode      status    = U_ZERO_ERROR;

    u_uastrncpy(rules,  "[A-N]{100}; \n"
                             "[a-w]{200}; \n"
                             "[\\p{L}]{300}; \n"
                             "[\\p{N}]{400}; \n"
                             "[0-5]{500}; \n"
                              "!.*;\n", RULE_STRING_LENGTH);
    u_uastrncpy(testString, "ABC", TEST_STRING_LENGTH);


    bi = ubrk_openRules(rules, -1, testString, -1, NULL, &status);
    TEST_ASSERT_SUCCESS(status);
    TEST_ASSERT(bi != NULL);

    /* The TEST_ASSERT above should change too... */
    if (bi != NULL) {
        pos = ubrk_next(bi);
        TEST_ASSERT(pos == 1);

        memset(vals, -1, sizeof(vals));
        numVals = ubrk_getRuleStatusVec(bi, vals, 10, &status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(numVals == 2);
        TEST_ASSERT(vals[0] == 100);
        TEST_ASSERT(vals[1] == 300);
        TEST_ASSERT(vals[2] == -1);

        numVals = ubrk_getRuleStatusVec(bi, vals, 0, &status);
        TEST_ASSERT(status == U_BUFFER_OVERFLOW_ERROR);
        TEST_ASSERT(numVals == 2);
    }

    ubrk_close(bi);
}


/*
 *  static void TestBreakIteratorUText(void);
 *
 *         Test that ubrk_setUText() is present and works for a simple case.
 */
static void TestBreakIteratorUText(void) {
    const char *UTF8Str = "\x41\xc3\x85\x5A\x20\x41\x52\x69\x6E\x67";  /* c3 85 is utf-8 for A with a ring on top */
                      /*   0  1   2 34567890  */

    UErrorCode      status = U_ZERO_ERROR;
    UBreakIterator *bi     = NULL;
    int32_t         pos    = 0;


    UText *ut = utext_openUTF8(NULL, UTF8Str, -1, &status);
    TEST_ASSERT_SUCCESS(status);

    bi = ubrk_open(UBRK_WORD, "en_US", NULL, 0, &status);
    if (U_FAILURE(status)) {
        log_err_status(status, "Failure at file %s, line %d, error = %s\n", __FILE__, __LINE__, u_errorName(status));
        return;
    }

    ubrk_setUText(bi, ut, &status);
    if (U_FAILURE(status)) {
        log_err("Failure at file %s, line %d, error = %s\n", __FILE__, __LINE__, u_errorName(status));
        return;
    }

    pos = ubrk_first(bi);
    TEST_ASSERT(pos == 0);

    pos = ubrk_next(bi);
    TEST_ASSERT(pos == 4);

    pos = ubrk_next(bi);
    TEST_ASSERT(pos == 5);

    pos = ubrk_next(bi);
    TEST_ASSERT(pos == 10);

    pos = ubrk_next(bi);
    TEST_ASSERT(pos == UBRK_DONE);
    ubrk_close(bi);
    utext_close(ut);
}

/*
 *  static void TestBreakIteratorTailoring(void);
 *
 *         Test break iterator tailorings from CLDR data.
 */

/* Thai/Lao grapheme break tailoring */
static const UChar thTest[] = { 0x0020, 0x0E40, 0x0E01, 0x0020,
                                0x0E01, 0x0E30, 0x0020, 0x0E01, 0x0E33, 0x0020, 0 };
/*in Unicode 6.1 en should behave just like th for this*/
/*static const int32_t thTestOffs_enFwd[] = {  1,      3,  4,      6,  7,      9, 10 };*/
static const int32_t thTestOffs_thFwd[] = {  1,  2,  3,  4,  5,  6,  7,      9, 10 };
/*static const int32_t thTestOffs_enRev[] = {  9,      7,  6,      4,  3,      1,  0 };*/
static const int32_t thTestOffs_thRev[] = {  9,      7,  6,  5,  4,  3,  2,  1,  0 };

/* Hebrew line break tailoring, for cldrbug 3028 */
static const UChar heTest[] = { 0x0020, 0x002D, 0x0031, 0x0032, 0x0020,
                                0x0061, 0x002D, 0x006B, 0x0020,
                                0x0061, 0x0300, 0x2010, 0x006B, 0x0020,
                                0x05DE, 0x05D4, 0x002D, 0x0069, 0x0020,
                                0x05D1, 0x05BC, 0x2010, 0x0047, 0x0020, 0 };
/*in Unicode 6.1 en should behave just like he for this*/
/*static const int32_t heTestOffs_enFwd[] = {  1,  5,  7,  9, 12, 14, 17, 19, 22, 24 };*/
static const int32_t heTestOffs_heFwd[] = {  1,  5,  7,  9, 12, 14,     19,     24 };
/*static const int32_t heTestOffs_enRev[] = { 22, 19, 17, 14, 12,  9,  7,  5,  1,  0 };*/
static const int32_t heTestOffs_heRev[] = {     19,     14, 12,  9,  7,  5,  1,  0 };

/* Finnish line break tailoring, for cldrbug 3029.
 * As of ICU 63, Finnish tailoring moved to root, Finnish and English should be the same. */
static const UChar fiTest[] = { /* 00 */ 0x0020, 0x002D, 0x0031, 0x0032, 0x0020,
                                /* 05 */ 0x0061, 0x002D, 0x006B, 0x0020,
                                /* 09 */ 0x0061, 0x0300, 0x2010, 0x006B, 0x0020,
                                /* 14 */ 0x0061, 0x0020, 0x002D, 0x006B, 0x0020,
                                /* 19 */ 0x0061, 0x0300, 0x0020, 0x2010, 0x006B, 0x0020, 0 };
//static const int32_t fiTestOffs_enFwd[] =  {  1,  5,  7,  9, 12, 14, 16, 17, 19, 22, 23, 25 };
static const int32_t fiTestOffs_enFwd[] =  {  1,  5,  7,  9, 12, 14, 16,     19, 22,     25 };
static const int32_t fiTestOffs_fiFwd[] =  {  1,  5,  7,  9, 12, 14, 16,     19, 22,     25 };
//static const int32_t fiTestOffs_enRev[] =  { 23, 22, 19, 17, 16, 14, 12,  9,  7,  5,  1,  0 };
static const int32_t fiTestOffs_enRev[] =  {     22, 19,     16, 14, 12,  9,  7,  5,  1,  0 };
static const int32_t fiTestOffs_fiRev[] =  {     22, 19,     16, 14, 12,  9,  7,  5,  1,  0 };

/* Khmer dictionary-based work break, for ICU ticket #8329 */
static const UChar kmTest[] = { /* 00 */ 0x179F, 0x17BC, 0x1798, 0x1785, 0x17C6, 0x178E, 0x17B6, 0x1799, 0x1796, 0x17C1,
                                /* 10 */ 0x179B, 0x1794, 0x1793, 0x17D2, 0x178F, 0x17B7, 0x1785, 0x178A, 0x17BE, 0x1798,
                                /* 20 */ 0x17D2, 0x1794, 0x17B8, 0x17A2, 0x1792, 0x17B7, 0x179F, 0x17D2, 0x178B, 0x17B6,
                                /* 30 */ 0x1793, 0x17A2, 0x179A, 0x1796, 0x17D2, 0x179A, 0x17C7, 0x1782, 0x17BB, 0x178E,
                                /* 40 */ 0x178A, 0x179B, 0x17CB, 0x1796, 0x17D2, 0x179A, 0x17C7, 0x17A2, 0x1784, 0x17D2,
                                /* 50 */ 0x1782, 0 };
static const int32_t kmTestOffs_kmFwd[] =  {  3, /*8,*/ 11, 17, 23, 31, /*33,*/  40,  43, 51 }; /* TODO: Investigate failure to break at offset 8 */
static const int32_t kmTestOffs_kmRev[] =  { 43,  40,   /*33,*/ 31, 23, 17, 11, /*8,*/ 3,  0 };


/* Korean keepAll vs Normal */
static const UChar koTest[] = { /* 00 */ 0xBAA8, 0xB4E0, 0x0020, 0xC778, 0xB958, 0x0020, 0xAD6C, 0xC131, 0xC6D0, 0xC758,
                                /* 10 */ 0x0020, 0xCC9C, 0xBD80, 0xC758, 0x0020, 0xC874, 0xC5C4, 0xC131, 0xACFC, 0x0020,
                                /* 20 */ 0xB3D9, 0xB4F1, 0xD558, 0xACE0, 0x0020, 0xC591, 0xB3C4, 0xD560, 0x002E, 0x8BA1, 
                                /* 30 */ 0x7B97, 0x673A, 0x53EA, 0x662F, 0x5904, 0x7406, 0x6570, 0x5B57, 0x3002, 0 }; // 基本上，计算机只是处理数字。
static const int32_t koTestOffs_koKeepAllFwd[] =  {   3,  6, 11, 15, 20, 25, 29, 39 };
static const int32_t koTestOffs_koKeepAllRev[] =  {  29, 25, 20, 15, 11,  6,  3,  0 };
static const int32_t koTestOffs_koKeepHngFwd[] =  {   3,  6, 11, 15, 20, 25, 29, 30, 31, 32, 33, 34, 35, 36, 37, 39 };
static const int32_t koTestOffs_koKeepHngRev[] =  {  37, 36, 35, 34, 33, 32, 31, 30, 29, 25, 20, 15, 11,  6,  3,  0 };
static const int32_t koTestOffs_koNormFwd[]    =  {  1,  3,  4,  6,  7,  8,  9, 11, 12, 13, 15, 16, 17, 18, 20, 21, 22, 23, 25, 26, 27, 29, 30, 31, 32, 33, 34, 35, 36, 37, 39 };
static const int32_t koTestOffs_koNormRev[]    =  { 37, 36, 35, 34, 33, 32, 31, 30, 29, 27, 26, 25, 23, 22, 21, 20, 18, 17, 16, 15, 13, 12, 11,  9,  8,  7,  6,  4,  3,  1,  0 };

typedef struct {
    const char * locale;
    UBreakIteratorType type;
    ULineWordOptions lineWordOpts;
    const UChar * test;
    const int32_t * offsFwd;
    const int32_t * offsRev;
    int32_t numOffsets;
} RBBITailoringTest;

static const RBBITailoringTest tailoringTests[] = {
    { "en",                UBRK_CHARACTER, UBRK_LINEWORD_NORMAL,      thTest, thTestOffs_thFwd,        thTestOffs_thRev,        UPRV_LENGTHOF(thTestOffs_thFwd) },
    { "en_US_POSIX",       UBRK_CHARACTER, UBRK_LINEWORD_NORMAL,      thTest, thTestOffs_thFwd,        thTestOffs_thRev,        UPRV_LENGTHOF(thTestOffs_thFwd) },
    { "en",                UBRK_LINE,      UBRK_LINEWORD_NORMAL,      heTest, heTestOffs_heFwd,        heTestOffs_heRev,        UPRV_LENGTHOF(heTestOffs_heFwd) },
    { "he",                UBRK_LINE,      UBRK_LINEWORD_NORMAL,      heTest, heTestOffs_heFwd,        heTestOffs_heRev,        UPRV_LENGTHOF(heTestOffs_heFwd) },
    { "en",                UBRK_LINE,      UBRK_LINEWORD_NORMAL,      fiTest, fiTestOffs_enFwd,        fiTestOffs_enRev,        UPRV_LENGTHOF(fiTestOffs_enFwd) },
    { "fi",                UBRK_LINE,      UBRK_LINEWORD_NORMAL,      fiTest, fiTestOffs_fiFwd,        fiTestOffs_fiRev,        UPRV_LENGTHOF(fiTestOffs_fiFwd) },
    { "km",                UBRK_WORD,      UBRK_LINEWORD_NORMAL,      kmTest, kmTestOffs_kmFwd,        kmTestOffs_kmRev,        UPRV_LENGTHOF(kmTestOffs_kmFwd) },
    { "ko",                UBRK_LINE,      UBRK_LINEWORD_NORMAL,      koTest, koTestOffs_koNormFwd,    koTestOffs_koNormRev,    UPRV_LENGTHOF(koTestOffs_koNormFwd) },
    { "ko@lw=keepall",     UBRK_LINE,      UBRK_LINEWORD_NORMAL,      koTest, koTestOffs_koKeepAllFwd, koTestOffs_koKeepAllRev, UPRV_LENGTHOF(koTestOffs_koKeepAllFwd) },
    { "ko",                UBRK_LINE,      UBRK_LINEWORD_KEEP_ALL,    koTest, koTestOffs_koKeepAllFwd, koTestOffs_koKeepAllRev, UPRV_LENGTHOF(koTestOffs_koKeepAllFwd) },
    { "ko@lw=keep-hangul", UBRK_LINE,      UBRK_LINEWORD_NORMAL,      koTest, koTestOffs_koKeepHngFwd, koTestOffs_koKeepHngRev, UPRV_LENGTHOF(koTestOffs_koKeepHngFwd) },
    { "ko",                UBRK_LINE,      UBRK_LINEWORD_KEEP_HANGUL, koTest, koTestOffs_koKeepHngFwd, koTestOffs_koKeepHngRev, UPRV_LENGTHOF(koTestOffs_koKeepHngFwd) },
    { "ko@lw=normal",      UBRK_LINE,      UBRK_LINEWORD_NORMAL,      koTest, koTestOffs_koNormFwd,    koTestOffs_koNormRev,    UPRV_LENGTHOF(koTestOffs_koNormFwd) },
    { NULL, 0, 0, NULL, NULL, 0 },
};

static void TestBreakIteratorTailoring(void) {
    const RBBITailoringTest * testPtr;
    for (testPtr = tailoringTests; testPtr->locale != NULL; ++testPtr) {
        UErrorCode status = U_ZERO_ERROR;
        UBreakIterator* ubrkiter = ubrk_open(testPtr->type, testPtr->locale, testPtr->test, -1, &status);
        if ( U_SUCCESS(status) ) {
            int32_t offset, offsindx;
            UBool foundError;

            if (testPtr->lineWordOpts != UBRK_LINEWORD_NORMAL) {
                ubrk_setLineWordOpts(ubrkiter, testPtr->lineWordOpts);
            }
            foundError = FALSE;
            ubrk_first(ubrkiter);
            for (offsindx = 0; (offset = ubrk_next(ubrkiter)) != UBRK_DONE; ++offsindx) {
                if (!foundError && offsindx >= testPtr->numOffsets) {
                    log_err("FAIL: locale %s, break type %d, ubrk_next expected UBRK_DONE, got %d\n",
                            testPtr->locale, testPtr->type, offset);
                    foundError = TRUE;
                } else if (!foundError && offset != testPtr->offsFwd[offsindx]) {
                    log_err("FAIL: locale %s, break type %d, ubrk_next expected %d, got %d\n",
                            testPtr->locale, testPtr->type, testPtr->offsFwd[offsindx], offset);
                    foundError = TRUE;
                }
            }
            if (!foundError && offsindx < testPtr->numOffsets) {
                log_err("FAIL: locale %s, break type %d, ubrk_next expected %d, got UBRK_DONE\n",
                        testPtr->locale, testPtr->type, testPtr->offsFwd[offsindx]);
            }

            foundError = FALSE;
            ubrk_last(ubrkiter);
            for (offsindx = 0; (offset = ubrk_previous(ubrkiter)) != UBRK_DONE; ++offsindx) {
                if (!foundError && offsindx >= testPtr->numOffsets) {
                    log_err("FAIL: locale %s, break type %d, ubrk_previous expected UBRK_DONE, got %d\n",
                            testPtr->locale, testPtr->type, offset);
                    foundError = TRUE;
                } else if (!foundError && offset != testPtr->offsRev[offsindx]) {
                    log_err("FAIL: locale %s, break type %d, ubrk_previous expected %d, got %d\n",
                            testPtr->locale, testPtr->type, testPtr->offsRev[offsindx], offset);
                    foundError = TRUE;
                }
            }
            if (!foundError && offsindx < testPtr->numOffsets) {
                log_err("FAIL: locale %s, break type %d, ubrk_previous expected %d, got UBRK_DONE\n",
                        testPtr->locale, testPtr->type, testPtr->offsRev[offsindx]);
            }

            ubrk_close(ubrkiter);
        } else {
            log_err_status(status, "FAIL: locale %s, break type %d, ubrk_open status: %s\n", testPtr->locale, testPtr->type, u_errorName(status));
        }
    }
}


static void TestBreakIteratorRefresh(void) {
    /*
     *  RefreshInput changes out the input of a Break Iterator without
     *    changing anything else in the iterator's state.  Used with Java JNI,
     *    when Java moves the underlying string storage.   This test
     *    runs a ubrk_next() repeatedly, moving the text in the middle of the sequence.
     *    The right set of boundaries should still be found.
     */
    UChar testStr[]  = {0x20, 0x41, 0x20, 0x42, 0x20, 0x43, 0x20, 0x44, 0x0};  /* = " A B C D"  */
    UChar movedStr[] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,  0};
    UErrorCode status = U_ZERO_ERROR;
    UBreakIterator *bi;
    UText ut1 = UTEXT_INITIALIZER;
    UText ut2 = UTEXT_INITIALIZER;

    bi = ubrk_open(UBRK_LINE, "en_US", NULL, 0, &status);
    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }

    utext_openUChars(&ut1, testStr, -1, &status);
    TEST_ASSERT_SUCCESS(status);
    ubrk_setUText(bi, &ut1, &status);
    TEST_ASSERT_SUCCESS(status);

    if (U_SUCCESS(status)) {
        /* Line boundaries will occur before each letter in the original string */
        TEST_ASSERT(1 == ubrk_next(bi));
        TEST_ASSERT(3 == ubrk_next(bi));

        /* Move the string, kill the original string.  */
        u_strcpy(movedStr, testStr);
        u_memset(testStr, 0x20, u_strlen(testStr));
        utext_openUChars(&ut2, movedStr, -1, &status);
        TEST_ASSERT_SUCCESS(status);
        ubrk_refreshUText(bi, &ut2, &status);
        TEST_ASSERT_SUCCESS(status);

        /* Find the following matches, now working in the moved string. */
        TEST_ASSERT(5 == ubrk_next(bi));
        TEST_ASSERT(7 == ubrk_next(bi));
        TEST_ASSERT(8 == ubrk_next(bi));
        TEST_ASSERT(UBRK_DONE == ubrk_next(bi));
        TEST_ASSERT_SUCCESS(status);

        utext_close(&ut1);
        utext_close(&ut2);
    }
    ubrk_close(bi);
}


static void TestBug11665(void) {
    // The problem was with the incorrect breaking of Japanese text beginning
    // with Katakana characters when no prior Japanese or Chinese text had been
    // encountered.
    //
    // Tested here in cintltst, rather than in intltest, because only cintltst
    // tests have the ability to reset ICU, which is needed to get the bug
    // to manifest itself.

    static UChar japaneseText[] = {0x30A2, 0x30EC, 0x30EB, 0x30AE, 0x30FC, 0x6027, 0x7D50, 0x819C, 0x708E};
    int32_t boundaries[10] = {0};
    UBreakIterator *bi = NULL;
    int32_t brk;
    int32_t brkIdx = 0;
    int32_t totalBreaks = 0;
    UErrorCode status = U_ZERO_ERROR;

    ctest_resetICU();
    bi = ubrk_open(UBRK_WORD, "en_US", japaneseText, UPRV_LENGTHOF(japaneseText), &status);
    TEST_ASSERT_SUCCESS(status);
    if (!bi) {
        return;
    }
    for (brk=ubrk_first(bi); brk != UBRK_DONE; brk=ubrk_next(bi)) {
        boundaries[brkIdx] = brk;
        if (++brkIdx >= UPRV_LENGTHOF(boundaries) - 1) {
            break;
        }
    }
    if (brkIdx <= 2 || brkIdx >= UPRV_LENGTHOF(boundaries)) {
        log_err("%s:%d too few or many breaks found.\n", __FILE__, __LINE__);
    } else {
        totalBreaks = brkIdx;
        brkIdx = 0;
        for (brk=ubrk_first(bi); brk != UBRK_DONE; brk=ubrk_next(bi)) {
            if (brk != boundaries[brkIdx]) {
                log_err("%s:%d Break #%d differs between first and second iteration.\n", __FILE__, __LINE__, brkIdx);
                break;
            }
            if (++brkIdx >= UPRV_LENGTHOF(boundaries) - 1) {
                log_err("%s:%d Too many breaks.\n", __FILE__, __LINE__);
                break;
            }
        }
        if (totalBreaks != brkIdx) {
            log_err("%s:%d Number of breaks differ between first and second iteration.\n", __FILE__, __LINE__);
        }
    }
    ubrk_close(bi);
}

/*
 * expOffset is the set of expected offsets, ending with '-1'.
 * "Expected expOffset -1" means "expected the end of the offsets"
 */

static const char testSentenceSuppressionsEn[]  = "Mr. Jones comes home. Dr. Smith Ph.D. is out. In the U.S.A. it is hot.";
static const int32_t testSentSuppFwdOffsetsEn[] = { 22, 46, 70, -1 };         /* With suppressions */
static const int32_t testSentFwdOffsetsEn[]     = {  4, 22, 26, 46, 70, -1 }; /* Without suppressions */
static const int32_t testSentSuppRevOffsetsEn[] = { 46, 22,  0, -1 };         /* With suppressions */
static const int32_t testSentRevOffsetsEn[]     = { 46, 26, 22,  4,  0, -1 }; /* Without suppressions */

static const char testSentenceSuppressionsDe[]  = "Wenn ich schon h\\u00F6re zu Guttenberg kommt evtl. zur\\u00FCck.";
//                                                "Wenn ich schon höre zu Guttenberg kommt evtl. zurück."
static const int32_t testSentSuppFwdOffsetsDe[] = { 53, -1 };       /* With suppressions */
static const int32_t testSentFwdOffsetsDe[]     = { 53, -1 };       /* Without suppressions; no break in evtl. zur due to casing */
static const int32_t testSentSuppRevOffsetsDe[] = {  0, -1 };       /* With suppressions */
static const int32_t testSentRevOffsetsDe[]     = {  0, -1 };       /* Without suppressions */

static const char testSentenceSuppressionsEs[]  = "Te esperamos todos los miercoles en Bravo 416, Col. El Pueblo a las 7 PM.";
static const int32_t testSentSuppFwdOffsetsEs[] = { 73, -1 };       /* With suppressions */
static const int32_t testSentFwdOffsetsEs[]     = { 52, 73, -1 };   /* Without suppressions */
static const int32_t testSentSuppRevOffsetsEs[] = {  0, -1 };       /* With suppressions */
static const int32_t testSentRevOffsetsEs[]     = { 52,  0, -1 };   /* Without suppressions */

static const char testSentenceSuppressionsE1[]  = "Add or detract. The world will little note.";
static const char testSentenceSuppressionsE1u[] = "ADD OR DETRACT. THE WORLD WILL LITTLE NOTE.";
static const int32_t testSentFwdOffsetsE1[]     = { 16, 43, -1 };   /* Suppressions and case should make no difference */
static const int32_t testSentRevOffsetsE1[]     = { 16,  0, -1 };   /* Suppressions and case should make no difference */

static const char testSentenceSuppressionsE2[]  = "Coming up, the sprints at NCAA. Are you watching?";
static const char testSentenceSuppressionsE2u[] = "COMING UP, THE SPRINTS AT NCAA. ARE YOU WATCHING?";
static const int32_t testSentFwdOffsetsE2[]     = { 32, 49, -1 };   /* Suppressions and case should make no difference */
static const int32_t testSentRevOffsetsE2[]     = { 32,  0, -1 };   /* Suppressions and case should make no difference */

static const char testSentenceSuppressionsFr[]  = "Tr\\u00E8s bonne prise de parole de M. Junod, municipal \\u00E0 la culture de Lausanne.";
//                                                "Très bonne prise de parole de M. Junod, municipal à la culture de Lausanne."
static const int32_t testSentFwdOffsetsFr[]     = { 33, 75, -1 };   /* Without suppressions */
static const int32_t testSentSuppFwdOffsetsFr[] = { 75, -1 };       /* With suppressions */
static const int32_t testSentRevOffsetsFr[]     = { 33,  0, -1 };   /* Without suppressions */
static const int32_t testSentSuppRevOffsetsFr[] = {  0, -1 };       /* With suppressions */

static const char testSentenceSuppressionsE3[]  = "G8 countries e.g. U.K., Japan. Sanctions i.e. restrictions. Test E. Xx G. Xx I. Xx.";
static const char testSentenceSuppressionsE3u[] = "G8 COUNTRIES E.G. U.K., JAPAN. SANCTIONS I.E. RESTRICTIONS. TEST E. XX G. XX I. XX.";
static const int32_t testSentSuppFwdOffsetsE3[] = { 31, 60, 83, -1 };                 /* With suppressions */
static const int32_t testSentSuppRevOffsetsE3[] = { 60, 31,  0, -1 };                 /* With suppressions */
static const int32_t testSentFwdOffsetsE3[]     = { 18, 31, 60, 68, 74, 80, 83, -1 }; /* Without suppressions */
static const int32_t testSentRevOffsetsE3[]     = { 80, 74, 68, 60, 31, 18,  0, -1 }; /* Without suppressions */
static const int32_t testSentFwdOffsetsE3u[]    = { 18, 31, 46, 60, 68, 74, 80, 83, -1 }; /* Without suppressions */
static const int32_t testSentRevOffsetsE3u[]    = { 80, 74, 68, 60, 46, 31, 18,  0, -1 }; /* Without suppressions */

enum { kTextULenMax = 128, kTextBLenMax = 192 };

typedef struct {
    const char * locale;
    const char * text;
    const int32_t * expFwdOffsets;
    const int32_t * expRevOffsets;
} TestBISuppressionsItem;

static const TestBISuppressionsItem testBISuppressionsItems[] = {
    { "en@ss=standard", testSentenceSuppressionsEn, testSentSuppFwdOffsetsEn, testSentSuppRevOffsetsEn },
    { "en",             testSentenceSuppressionsEn, testSentFwdOffsetsEn,     testSentRevOffsetsEn     },
    { "en_CA",             testSentenceSuppressionsEn, testSentFwdOffsetsEn,     testSentRevOffsetsEn     },
    { "en_CA@ss=standard", testSentenceSuppressionsEn, testSentSuppFwdOffsetsEn, testSentSuppRevOffsetsEn },
    { "fr@ss=standard", testSentenceSuppressionsEn, testSentFwdOffsetsEn,     testSentRevOffsetsEn     },
    { "af@ss=standard", testSentenceSuppressionsEn, testSentFwdOffsetsEn,     testSentRevOffsetsEn     }, /* no brkiter data => nosuppressions? */
    { "af_ZA@ss=standard", testSentenceSuppressionsEn, testSentFwdOffsetsEn,     testSentRevOffsetsEn     }, /* no brkiter data => nosuppressions? */
    { "zh@ss=standard", testSentenceSuppressionsEn, testSentFwdOffsetsEn,     testSentRevOffsetsEn     }, /* brkiter data, no suppressions data => no suppressions */
    { "zh_Hant@ss=standard", testSentenceSuppressionsEn, testSentFwdOffsetsEn, testSentRevOffsetsEn    }, /* brkiter data, no suppressions data => no suppressions */
    { "fi@ss=standard", testSentenceSuppressionsEn, testSentFwdOffsetsEn,     testSentRevOffsetsEn     }, /* brkiter data, no suppressions data => no suppressions */
    { "ja@ss=standard", testSentenceSuppressionsEn, testSentFwdOffsetsEn,     testSentRevOffsetsEn     }, /* brkiter data, no suppressions data => no suppressions */
    { "de@ss=standard", testSentenceSuppressionsDe, testSentSuppFwdOffsetsDe, testSentSuppRevOffsetsDe },
    { "de",             testSentenceSuppressionsDe, testSentFwdOffsetsDe,     testSentRevOffsetsDe     },
    { "es@ss=standard", testSentenceSuppressionsEs, testSentSuppFwdOffsetsEs, testSentSuppRevOffsetsEs },
    { "es",             testSentenceSuppressionsEs, testSentFwdOffsetsEs,     testSentRevOffsetsEs     },
    { "en",             testSentenceSuppressionsE1,  testSentFwdOffsetsE1,    testSentRevOffsetsE1     },
    { "en@ss=standard", testSentenceSuppressionsE1,  testSentFwdOffsetsE1,    testSentRevOffsetsE1     },
    { "en",             testSentenceSuppressionsE1u, testSentFwdOffsetsE1,    testSentRevOffsetsE1     },
    { "en@ss=standard", testSentenceSuppressionsE1u, testSentFwdOffsetsE1,    testSentRevOffsetsE1     },
    { "en",             testSentenceSuppressionsE2,  testSentFwdOffsetsE2,    testSentRevOffsetsE2     },
    { "en@ss=standard", testSentenceSuppressionsE2,  testSentFwdOffsetsE2,    testSentRevOffsetsE2     },
    { "en",             testSentenceSuppressionsE2u, testSentFwdOffsetsE2,    testSentRevOffsetsE2     },
    { "en@ss=standard", testSentenceSuppressionsE2u, testSentFwdOffsetsE2,    testSentRevOffsetsE2     },
    { "fr",             testSentenceSuppressionsFr, testSentFwdOffsetsFr,     testSentRevOffsetsFr     },
    { "fr@ss=standard", testSentenceSuppressionsFr, testSentSuppFwdOffsetsFr, testSentSuppRevOffsetsFr },
    { "en@ss=standard", testSentenceSuppressionsE3, testSentSuppFwdOffsetsE3, testSentSuppRevOffsetsE3 },
    { "en",             testSentenceSuppressionsE3, testSentFwdOffsetsE3,     testSentRevOffsetsE3     },
    { "en@ss=standard", testSentenceSuppressionsE3u, testSentSuppFwdOffsetsE3, testSentSuppRevOffsetsE3 },
    { "en",             testSentenceSuppressionsE3u, testSentFwdOffsetsE3u,    testSentRevOffsetsE3u    },
    { NULL, NULL, NULL }
};

static void TestBreakIteratorSuppressions(void) {
    const TestBISuppressionsItem * itemPtr;

    for (itemPtr = testBISuppressionsItems; itemPtr->locale != NULL; itemPtr++) {
        UChar textU[kTextULenMax + 1];
        char  textB[kTextBLenMax];
        int32_t textULen = u_unescape(itemPtr->text, textU, kTextULenMax);
        textU[kTextULenMax] = 0; // ensure zero termination
        UErrorCode status = U_ZERO_ERROR;
        UBreakIterator *bi = ubrk_open(UBRK_SENTENCE, itemPtr->locale, textU, textULen, &status);
        log_verbose("#%d: %s\n", (itemPtr-testBISuppressionsItems), itemPtr->locale);
        if (U_SUCCESS(status)) {
            int32_t offset, start;
            const int32_t * expOffsetPtr;
            const int32_t * expOffsetStart;
            u_austrcpy(textB, textU);

            expOffsetStart = expOffsetPtr = itemPtr->expFwdOffsets;
            ubrk_first(bi);
            for (; (offset = ubrk_next(bi)) != UBRK_DONE && *expOffsetPtr >= 0; expOffsetPtr++) {
                if (offset != *expOffsetPtr) {
                    log_err("FAIL: ubrk_next loc \"%s\", expected %d, got %d, text \"%s\"\n",
                            itemPtr->locale, *expOffsetPtr, offset, textB);
                }
            }
            if (offset != UBRK_DONE || *expOffsetPtr >= 0) {
                log_err("FAIL: ubrk_next loc \"%s\", expected UBRK_DONE & expOffset -1, got %d and %d, text \"%s\"\n",
                        itemPtr->locale, offset, *expOffsetPtr, textB);
            }

            expOffsetStart = expOffsetPtr = itemPtr->expFwdOffsets;
            start = ubrk_first(bi) + 1;
            for (; (offset = ubrk_following(bi, start)) != UBRK_DONE && *expOffsetPtr >= 0; expOffsetPtr++) {
                if (offset != *expOffsetPtr) {
                    log_err("FAIL: ubrk_following(%d) loc \"%s\", expected %d, got %d, text \"%s\"\n",
                            start, itemPtr->locale, *expOffsetPtr, offset, textB);
                }
                start = *expOffsetPtr + 1;
            }
            if (offset != UBRK_DONE || *expOffsetPtr >= 0) {
                log_err("FAIL: ubrk_following(%d) loc \"%s\", expected UBRK_DONE & expOffset -1, got %d and %d, text \"%s\"\n",
                        start, itemPtr->locale, offset, *expOffsetPtr, textB);
            }

            expOffsetStart = expOffsetPtr = itemPtr->expRevOffsets;
            offset = ubrk_last(bi);
            log_verbose("___ @%d ubrk_last\n", offset);
            if(offset == 0) {
              log_err("FAIL: ubrk_last loc \"%s\" unexpected %d\n", itemPtr->locale, offset);
            }
            for (; (offset = ubrk_previous(bi)) != UBRK_DONE && *expOffsetPtr >= 0; expOffsetPtr++) {
                if (offset != *expOffsetPtr) {
                    log_err("FAIL: ubrk_previous loc \"%s\", expected %d, got %d, text \"%s\"\n",
                            itemPtr->locale, *expOffsetPtr, offset, textB);
                } else {
                    log_verbose("[%d] @%d ubrk_previous()\n", (expOffsetPtr - expOffsetStart), offset);
                }
            }
            if (offset != UBRK_DONE || *expOffsetPtr >= 0) {
                log_err("FAIL: ubrk_previous loc \"%s\", expected UBRK_DONE & expOffset[%d] -1, got %d and %d, text \"%s\"\n",
                        itemPtr->locale, expOffsetPtr - expOffsetStart, offset, *expOffsetPtr, textB);
            }

            expOffsetStart = expOffsetPtr = itemPtr->expRevOffsets;
            start = ubrk_last(bi) - 1;
            for (; (offset = ubrk_preceding(bi, start)) != UBRK_DONE && *expOffsetPtr >= 0; expOffsetPtr++) {
                if (offset != *expOffsetPtr) {
                    log_err("FAIL: ubrk_preceding(%d) loc \"%s\", expected %d, got %d, text \"%s\"\n",
                            start, itemPtr->locale, *expOffsetPtr, offset, textB);
                }
                start = *expOffsetPtr - 1;
            }
            if (start >=0 && (offset != UBRK_DONE || *expOffsetPtr >= 0)) {
                log_err("FAIL: ubrk_preceding loc(%d) \"%s\", expected UBRK_DONE & expOffset -1, got %d and %d, text \"%s\"\n",
                        start, itemPtr->locale, offset, *expOffsetPtr, textB);
            }

            ubrk_close(bi);
        } else {
            log_data_err("FAIL: ubrk_open(UBRK_SENTENCE, \"%s\", ...) status %s (Are you missing data?)\n",
                    itemPtr->locale, u_errorName(status));
        }
    }
}

#if APPLE_ADDITIONS
#include <stdio.h>
#if U_PLATFORM_IS_DARWIN_BASED
#include <unistd.h>
#include <mach/mach_time.h>
#define GET_START() mach_absolute_time()
#define GET_DURATION(start, info) ((mach_absolute_time() - start) * info.numer)/info.denom
#elif !U_PLATFORM_HAS_WIN32_API
#include <unistd.h>
#include "putilimp.h"
#define GET_START() (uint64_t)uprv_getUTCtime()
#define GET_DURATION(start, info) ((uint64_t)uprv_getUTCtime() - start)
#else
#include "putilimp.h"
#define GET_START() (unsigned long long)uprv_getUTCtime()
#define GET_DURATION(start, info) ((unsigned long long)uprv_getUTCtime() - start)
#endif
#include "unicode/urbtok.h"
#include "cstring.h"

typedef struct {
    RuleBasedTokenRange token;
    unsigned long       flags;
} RBTokResult;

static const UChar tokTextLatn[] = {
/*
"Short phrase! Another (with parens); done.\n
At 4:00, tea-time.\n
He wouldn't've wanted y'all to ... come at 3:30pm for $3 coffee @funman :)\n
x3:30 -- x1.0"
*/
    0x53,0x68,0x6F,0x72,0x74,0x20,0x70,0x68,0x72,0x61,0x73,0x65,0x21,0x20,
    0x41,0x6E,0x6F,0x74,0x68,0x65,0x72,0x20,0x28,0x77,0x69,0x74,0x68,0x20,0x70,0x61,0x72,0x65,0x6E,0x73,0x29,0x3B,0x20,0x64,0x6F,0x6E,0x65,0x2E,0x0A,
    0x41,0x74,0x20,0x34,0x3A,0x30,0x30,0x2C,0x20,0x74,0x65,0x61,0x2D,0x74,0x69,0x6D,0x65,0x2E,0x0A,
    0x48,0x65,0x20,0x77,0x6F,0x75,0x6C,0x64,0x6E,0x27,0x74,0x27,0x76,0x65,0x20,0x77,0x61,0x6E,0x74,0x65,0x64,0x20,
    0x79,0x27,0x61,0x6C,0x6C,0x20,0x74,0x6F,0x20,0x2E,0x2E,0x2E,0x20, 0x63,0x6F,0x6D,0x65,0x20,0x61,0x74,0x20,
    0x33,0x3A,0x33,0x30,0x70,0x6D,0x20,0x66,0x6F,0x72,0x20,0x24,0x33,0x20,0x63,0x6F,0x66,0x66,0x65,0x65,0x20,
    0x40,0x66,0x75,0x6E,0x6D,0x61,0x6E,0x20,0x3A,0x29,0x0A,
    0x78,0x33,0x3A,0x33,0x30,0x20,0x2D,0x2D,0x20,0x78,0x31,0x2E,0x30,0
};


static const RBTokResult expTokLatnCFST[] = { // 26 tokens
    { {   0,  5 }, 0x002 },  // Short
    { {   6,  7 }, 0x020 },  // phrase!
    { {  14,  7 }, 0x002 },  // Another
    { {  22,  5 }, 0x020 },  // (with
    { {  28,  8 }, 0x020 },  // parens);
    { {  37,  5 }, 0x020 },  // done.
    { {  43,  2 }, 0x002 },  // 
    { {  46,  5 }, 0x030 },  // 
    { {  52,  9 }, 0x020 },  // 
    { {  62,  2 }, 0x002 },  // 
    { {  65, 11 }, 0x020 },  // 
    { {  77,  6 }, 0x000 },  // 
    { {  84,  5 }, 0x020 },  // 
    { {  90,  2 }, 0x000 },  // 
    { {  93,  3 }, 0x020 },  // 
    { {  97,  4 }, 0x000 },  // 
    { { 102,  2 }, 0x000 },  // 
    { { 105,  6 }, 0x030 },  // 
    { { 112,  3 }, 0x000 },  // 
    { { 116,  2 }, 0x030 },  // 
    { { 119,  6 }, 0x000 },  // 
    { { 126,  7 }, 0x020 },  // 
    { { 134,  2 }, 0x020 },  // 
    { { 137,  5 }, 0x030 },  // 
    { { 143,  2 }, 0x020 },  // 
    { { 146,  4 }, 0x030 },  // 
};

// The following was expTokLatnNLLT for ICU 61-based Apple ICU
static const RBTokResult expTokLatnNLLT_Old[] = { // 66 tokens
    { {   0, 5 }, 0xC8 },  // Short
    { {   5, 1 }, 0x00 },  // _sp_
    { {   6, 6 }, 0xC8 },  // phrase
    { {  12, 1 }, 0x00 },  // !
    { {  13, 1 }, 0x00 },  // _sp_
    { {  14, 7 }, 0xC8 },  // Another
    { {  21, 1 }, 0x00 },  // _sp_
    { {  22, 1 }, 0x00 },  // (
    { {  23, 4 }, 0xC8 },  // with
    { {  27, 1 }, 0x00 },  // _sp_
    { {  28, 6 }, 0xC8 },  // parens
    { {  34, 1 }, 0x00 },  // )
    { {  35, 1 }, 0x00 },  // ;
    { {  36, 1 }, 0x00 },  // _sp_
    { {  37, 4 }, 0xC8 },  // done
    { {  41, 1 }, 0x14 },  // .
    { {  42, 1 }, 0x00 },  // _nl_

    { {  43, 2 }, 0xC8 },  // At
    { {  45, 1 }, 0x00 },  // _sp_
    { {  46, 4 }, 0x76 },  // 4:00
    { {  50, 1 }, 0x00 },  // ,
    { {  51, 1 }, 0x00 },  // _sp_
    { {  52, 3 }, 0xC8 },  // tea
    { {  55, 1 }, 0x15 },  // -
    { {  56, 4 }, 0xC8 },  // time
    { {  60, 1 }, 0x14 },  // .
    { {  61, 1 }, 0x00 },  // _nl_

    { {  62, 2 }, 0xC8 },  // He
    { {  64, 1 }, 0x00 },  // _sp_
    { {  65, 8 }, 0xCA },  // wouldn't
    { {  73, 1 }, 0x16 },  // '
    { {  74, 2 }, 0xC8 },  // ve
    { {  76, 1 }, 0x00 },  // _sp_
    { {  77, 6 }, 0xC8 },  // wanted
    { {  83, 1 }, 0x00 },  // _sp_
    { {  84, 5 }, 0xCA },  // y'all
    { {  89, 1 }, 0x00 },  // _sp_
    { {  90, 2 }, 0xC8 },  // to
    { {  92, 1 }, 0x00 },  // _sp_
    { {  93, 3 }, 0x3C },  // ...
    { {  96, 1 }, 0x00 },  // _sp_
    { {  97, 4 }, 0xC8 },  // come
    { { 101, 1 }, 0x00 },  // _sp_
    { { 102, 2 }, 0xC8 },  // at
    { { 104, 1 }, 0x00 },  // _sp_
    { { 105, 4 }, 0xC8 },  // 3:30
    { { 109, 2 }, 0xC8 },  // pm
    { { 111, 1 }, 0x00 },  // _sp_
    { { 112, 3 }, 0xC8 },  // for
    { { 115, 1 }, 0x00 },  // _sp_
    { { 116, 1 }, 0x00 },  // $
    { { 117, 1 }, 0x64 },  // 3
    { { 118, 1 }, 0x00 },  // _sp_
    { { 119, 6 }, 0xC8 },  // coffee
    { { 125, 1 }, 0x00 },  // _sp_
    { { 126, 7 }, 0xDF },  // @funman
    { { 133, 1 }, 0x00 },  // _sp_
    { { 134, 2 }, 0x20 },  // :)
    { { 136, 1 }, 0x00 },  // _nl_
    { { 137, 1 }, 0x76 },  // x
    { { 138, 4 }, 0x76 },  // 3:30
    { { 142, 1 }, 0x00 },  // _sp_
    { { 143, 2 }, 0x3D },  // --
    { { 145, 1 }, 0x00 },  //  _sp_
    { { 146, 1 }, 0x77 },  // x
    { { 147, 3 }, 0x77 },  // 1.0
};

// For ICU 62-based Apple ICU, expTokLatnNLLT matches expTokLatnNLLT_File
#define expTokLatnNLLT expTokLatnNLLT_File
static const RBTokResult expTokLatnNLLT_File[] = { // 67 tokens
    { {   0, 5 }, 0xC8 },  // Short
    { {   5, 1 }, 0x00 },  // _sp_
    { {   6, 6 }, 0xC8 },  // phrase
    { {  12, 1 }, 0x00 },  // !
    { {  13, 1 }, 0x00 },  // _sp_
    { {  14, 7 }, 0xC8 },  // Another
    { {  21, 1 }, 0x00 },  // _sp_
    { {  22, 1 }, 0x00 },  // (
    { {  23, 4 }, 0xC8 },  // with
    { {  27, 1 }, 0x00 },  // _sp_
    { {  28, 6 }, 0xC8 },  // parens
    { {  34, 1 }, 0x00 },  // )
    { {  35, 1 }, 0x00 },  // ;
    { {  36, 1 }, 0x00 },  // _sp_
    { {  37, 4 }, 0xC8 },  // done
    { {  41, 1 }, 0x14 },  // .
    { {  42, 1 }, 0x00 },  // _nl_

    { {  43, 2 }, 0xC8 },  // At
    { {  45, 1 }, 0x00 },  // _sp_
    { {  46, 4 }, 0x76 },  // 4:00
    { {  50, 1 }, 0x00 },  // ,
    { {  51, 1 }, 0x00 },  // _sp_
    { {  52, 3 }, 0xC8 },  // tea
    { {  55, 1 }, 0x15 },  // -
    { {  56, 4 }, 0xC8 },  // time
    { {  60, 1 }, 0x14 },  // .
    { {  61, 1 }, 0x00 },  // _nl_

    { {  62, 2 }, 0xC8 },  // He
    { {  64, 1 }, 0x00 },  // _sp_
    { {  65, 8 }, 0xCA },  // wouldn't
    { {  73, 1 }, 0x16 },  // '
    { {  74, 2 }, 0xC8 },  // ve
    { {  76, 1 }, 0x00 },  // _sp_
    { {  77, 6 }, 0xC8 },  // wanted
    { {  83, 1 }, 0x00 },  // _sp_
    { {  84, 5 }, 0xCA },  // y'all
    { {  89, 1 }, 0x00 },  // _sp_
    { {  90, 2 }, 0xC8 },  // to
    { {  92, 1 }, 0x00 },  // _sp_
    { {  93, 3 }, 0x3C },  // ...
    { {  96, 1 }, 0x00 },  // _sp_
    { {  97, 4 }, 0xC8 },  // come
    { { 101, 1 }, 0x00 },  // _sp_
    { { 102, 2 }, 0xC8 },  // at
    { { 104, 1 }, 0x00 },  // _sp_
    { { 105, 4 }, 0xC8 },  // 3:30
    { { 109, 2 }, 0xC8 },  // pm
    { { 111, 1 }, 0x00 },  // _sp_
    { { 112, 3 }, 0xC8 },  // for
    { { 115, 1 }, 0x00 },  // _sp_
    { { 116, 1 }, 0x00 },  // $
    { { 117, 1 }, 0x64 },  // 3
    { { 118, 1 }, 0x00 },  // _sp_
    { { 119, 6 }, 0xC8 },  // coffee
    { { 125, 1 }, 0x00 },  // _sp_
    { { 126, 7 }, 0xDF },  // @funman
    { { 133, 1 }, 0x00 },  // _sp_
    { { 134, 2 }, 0x20 },  // :)
    { { 136, 1 }, 0x00 },  // _nl_
    { { 137, 1 }, 0x76 },  // x
    { { 138, 4 }, 0x76 },  // 3:30
    { { 142, 1 }, 0x00 },  // _sp_
    { { 143, 2 }, 0x3D },  // --
    { { 145, 1 }, 0x00 },  //  _sp_
    { { 146, 2 }, 0xEC },  // x1
    { { 148, 1 }, 0x14 },  // .
    { { 149, 1 }, 0x64 },  // 0
};

static const RBTokResult expTokLatnNLLT_57[] = { // 67 tokens
    { {   0, 5 }, 0xC8 },  //
    { {   5, 1 }, 0x00 },  //
    { {   6, 6 }, 0xC8 },  //
    { {  12, 1 }, 0x00 },  //
    { {  13, 1 }, 0x00 },  //
    { {  14, 7 }, 0xC8 },  //
    { {  21, 1 }, 0x00 },  //
    { {  22, 1 }, 0x00 },  //
    { {  23, 4 }, 0xC8 },  //
    { {  27, 1 }, 0x00 },  //
    { {  28, 6 }, 0xC8 },  //
    { {  34, 1 }, 0x00 },  //
    { {  35, 1 }, 0x00 },  //
    { {  36, 1 }, 0x00 },  //
    { {  37, 4 }, 0xC8 },  //
    { {  41, 1 }, 0x14 },  //
    { {  42, 1 }, 0x00 },  //

    { {  43, 2 }, 0xC8 },  //
    { {  45, 1 }, 0x00 },  //
    { {  46, 4 }, 0x76 },  //
    { {  50, 1 }, 0x00 },  //
    { {  51, 1 }, 0x00 },  //
    { {  52, 3 }, 0xC8 },  //
    { {  55, 1 }, 0x15 },  //
    { {  56, 4 }, 0xC8 },  //
    { {  60, 1 }, 0x14 },  //
    { {  61, 1 }, 0x00 },  //

    { {  62, 2 }, 0xC8 },  //
    { {  64, 1 }, 0x00 },  //
    { {  65, 8 }, 0xCA },  //
    { {  73, 1 }, 0x16 },  //
    { {  74, 2 }, 0xC8 },  //
    { {  76, 1 }, 0x00 },  //
    { {  77, 6 }, 0xC8 },  //
    { {  83, 1 }, 0x00 },  //
    { {  84, 5 }, 0xCA },  //
    { {  89, 1 }, 0x00 },  //
    { {  90, 2 }, 0xC8 },  //
    { {  92, 1 }, 0x00 },  //
    { {  93, 3 }, 0x3C },  //
    { {  96, 1 }, 0x00 },  //
    { {  97, 4 }, 0xC8 },  //
    { { 101, 1 }, 0x00 },  //
    { { 102, 2 }, 0xC8 },  //
    { { 104, 1 }, 0x00 },  //
    { { 105, 6 }, 0xC8 },  //
    { { 111, 1 }, 0x00 },  //
    { { 112, 3 }, 0xC8 },  //
    { { 115, 1 }, 0x00 },  //
    { { 116, 1 }, 0x00 },  //
    { { 117, 1 }, 0x64 },  //
    { { 118, 1 }, 0x00 },  //
    { { 119, 6 }, 0xC8 },  //
    { { 125, 1 }, 0x00 },  //
    { { 126, 7 }, 0xDF },  //
    { { 133, 1 }, 0x00 },  //
    { { 134, 2 }, 0x20 },  //
    { { 136, 1 }, 0x00 },  //
    { { 137, 2 }, 0xEC },  //
    { { 139, 1 }, 0x00 },  //
    { { 140, 2 }, 0x64 },  //
    { { 142, 1 }, 0x00 },  //
    { { 143, 2 }, 0x3D },  //
    { { 145, 1 }, 0x00 },  //
    { { 146, 2 }, 0xEC },  //
    { { 148, 1 }, 0x14 },  //
    { { 149, 1 }, 0x64 },  //
};


static const RBTokResult expTokLatnStdW[] = { // 72 tokens
    { {   0,  5 }, 0x0C8 }, //
    { {   5,  1 }, 0x000 }, //
    { {   6,  6 }, 0x0C8 }, //
    { {  12,  1 }, 0x000 }, //
    { {  13,  1 }, 0x000 }, //
    { {  14,  7 }, 0x0C8 }, //
    { {  21,  1 }, 0x000 }, //
    { {  22,  1 }, 0x000 }, //
    { {  23,  4 }, 0x0C8 }, //
    { {  27,  1 }, 0x000 }, //
    { {  28,  6 }, 0x0C8 }, //
    { {  34,  1 }, 0x000 }, //
    { {  35,  1 }, 0x000 }, //
    { {  36,  1 }, 0x000 }, //
    { {  37,  4 }, 0x0C8 }, //
    { {  41,  1 }, 0x000 }, //
    { {  42,  1 }, 0x000 }, //
    { {  43,  2 }, 0x0C8 }, //
    { {  45,  1 }, 0x000 }, //
    { {  46,  1 }, 0x064 }, //
    { {  47,  1 }, 0x000 }, //
    { {  48,  2 }, 0x064 }, //
    { {  50,  1 }, 0x000 }, //
    { {  51,  1 }, 0x000 }, //
    { {  52,  3 }, 0x0C8 }, //
    { {  55,  1 }, 0x000 }, //
    { {  56,  4 }, 0x0C8 }, //
    { {  60,  1 }, 0x000 }, //
    { {  61,  1 }, 0x000 }, //
    { {  62,  2 }, 0x0C8 }, //
    { {  64,  1 }, 0x000 }, //
    { {  65, 11 }, 0x0C8 }, //
    { {  76,  1 }, 0x000 }, //
    { {  77,  6 }, 0x0C8 }, //
    { {  83,  1 }, 0x000 }, //
    { {  84,  5 }, 0x0C8 }, //
    { {  89,  1 }, 0x000 }, //
    { {  90,  2 }, 0x0C8 }, //
    { {  92,  1 }, 0x000 }, //
    { {  93,  1 }, 0x000 }, //
    { {  94,  1 }, 0x000 }, //
    { {  95,  1 }, 0x000 }, //
    { {  96,  1 }, 0x000 }, //
    { {  97,  4 }, 0x0C8 }, //
    { { 101,  1 }, 0x000 }, //
    { { 102,  2 }, 0x0C8 }, //
    { { 104,  1 }, 0x000 }, //
    { { 105,  1 }, 0x064 }, //
    { { 106,  1 }, 0x000 }, //
    { { 107,  4 }, 0x0C8 }, //
    { { 111,  1 }, 0x000 }, //
    { { 112,  3 }, 0x0C8 }, //
    { { 115,  1 }, 0x000 }, //
    { { 116,  1 }, 0x000 }, //
    { { 117,  1 }, 0x064 }, //
    { { 118,  1 }, 0x000 }, //
    { { 119,  6 }, 0x0C8 }, //
    { { 125,  1 }, 0x000 }, //
    { { 126,  1 }, 0x000 }, //
    { { 127,  6 }, 0x0C8 }, //
    { { 133,  1 }, 0x000 }, //
    { { 134,  1 }, 0x000 }, //
    { { 135,  1 }, 0x000 }, //
    { { 136,  1 }, 0x000 }, //
    { { 137,  2 }, 0x0EC }, //
    { { 139,  1 }, 0x000 }, //
    { { 140,  2 }, 0x064 }, //
    { { 142,  1 }, 0x000 }, //
    { { 143,  1 }, 0x000 }, //
    { { 144,  1 }, 0x000 }, //
    { { 145,  1 }, 0x000 }, //
    { { 146,  4 }, 0x064 }, //
};

static const UChar tokTextMisc[] = {
/*
"4‑inch phone.\n
3월 30일\n
从你开始使用 iPhone 6s 的那一刻起，你就会感觉到它是如此不同。\n""
*/
    0x0034,0x2011,0x0069,0x006E,0x0063,0x0068,0x0020,0x0070,0x0068,0x006F,0x006E,0x0065,0x002E,0x000A,
    0x0033,0xC6D4,0x0020,0x0033,0x0030,0xC77C,0x000A,
    0x4ECE,0x4F60,0x5F00,0x59CB,0x4F7F,0x7528,0x0020,0x0069,0x0050,0x0068,0x006F,0x006E,0x0065,0x0020,
    0x0036,0x0073,0x0020,0x7684,0x90A3,0x4E00,0x523B,0x8D77,0xFF0C,
    0x4F60,0x5C31,0x4F1A,0x611F,0x89C9,0x5230,0x5B83,0x662F,0x5982,0x6B64,0x4E0D,0x540C,0x3002,0x000A,0
};

static const RBTokResult expTokMiscCFST[] = { // 8 tokens
    // ranges   flags            text
    { {   0,  6}, 0x131      }, // 4‑inch
    { {   7,  6}, 0x20       }, // phone.
    { {  14,  2}, 0x1110     }, // 3월
    { {  17,  3}, 0x1110     }, // 30일
    { {  21,  6}, 0x40000000 }, // 从你开始使用
    { {  28,  6}, 0x04       }, // iPhone
    { {  35,  2}, 0x10       }, // 6s
    { {  38, 19}, 0x40000121 }, // 的那一刻起，你就会感觉到它是如此不同。
};

static const RBTokResult expTokMiscCFST_57Bulk[] = { // 5 tokens
    // ranges   flags            text
    { {   0,  6}, 0x131      }, // 4‑inch
    { {   7,  6}, 0x20       }, // phone.
    { {  14,  2}, 0x1110     }, // 3월
    { {  17,  3}, 0x1110     }, // 30일
    { {  21,  6}, 0x40000000 }, // 从你开始使用
    // missing the last 3 entries
};


static const RBTokResult expTokMiscNLLT[] = { // 36 tokens
    { {   0,  1 }, 0x064 }, //
    { {   1,  1 }, 0x000 }, //
    { {   2,  4 }, 0x0C8 }, //
    { {   6,  1 }, 0x000 }, //
    { {   7,  5 }, 0x0C8 }, //
    { {  12,  1 }, 0x014 }, //
    { {  13,  1 }, 0x000 }, //
    { {  14,  1 }, 0x064 }, //
    { {  15,  1 }, 0x0C8 }, //
    { {  16,  1 }, 0x000 }, //
    { {  17,  2 }, 0x064 }, //
    { {  19,  1 }, 0x0C8 }, //
    { {  20,  1 }, 0x000 }, //
    { {  21,  1 }, 0x190 }, //
    { {  22,  1 }, 0x190 }, //
    { {  23,  2 }, 0x190 }, //
    { {  25,  2 }, 0x190 }, //
    { {  27,  1 }, 0x000 }, //
    { {  28,  6 }, 0x0C8 }, //
    { {  34,  1 }, 0x000 }, //
    { {  35,  2 }, 0x0C8 }, //
    { {  37,  1 }, 0x000 }, //
    { {  38,  1 }, 0x190 }, //
    { {  39,  2 }, 0x190 }, //
    { {  41,  2 }, 0x190 }, //
    { {  43,  1 }, 0x000 }, //
    { {  44,  1 }, 0x190 }, //
    { {  45,  1 }, 0x190 }, //
    { {  46,  1 }, 0x190 }, //
    { {  47,  2 }, 0x190 }, //
    { {  49,  1 }, 0x190 }, //
    { {  50,  2 }, 0x190 }, //
    { {  52,  2 }, 0x190 }, //
    { {  54,  2 }, 0x190 }, //
    { {  56,  1 }, 0x000 }, //
    { {  57,  1 }, 0x000 }, //
};

static const RBTokResult expTokMiscNLLT_57Loop[] = { // 36 tokens
    { {   0,  1 }, 0x064 }, //
    { {   1,  1 }, 0x000 }, //
    { {   2,  4 }, 0x0C8 }, //
    { {   6,  1 }, 0x000 }, //
    { {   7,  5 }, 0x0C8 }, //
    { {  12,  1 }, 0x014 }, //
    { {  13,  1 }, 0x000 }, //
    { {  14,  1 }, 0x064 }, //
    { {  15,  1 }, 0x0C8 }, //
    { {  16,  1 }, 0x000 }, //
    { {  17,  2 }, 0x064 }, //
    { {  19,  1 }, 0x0C8 }, //
    { {  20,  1 }, 0x000 }, //
    { {  21,  1 }, 0x190 }, //
    { {  22,  1 }, 0x000 }, //
    { {  23,  2 }, 0x000 }, //
    { {  25,  2 }, 0x000 }, //
    { {  27,  1 }, 0x000 }, //
    { {  28,  6 }, 0x0C8 }, //
    { {  34,  1 }, 0x000 }, //
    { {  35,  2 }, 0x0C8 }, //
    { {  37,  1 }, 0x000 }, //
    { {  38,  1 }, 0x190 }, //
    { {  39,  2 }, 0x000 }, //
    { {  41,  2 }, 0x000 }, //
    { {  43,  1 }, 0x000 }, //
    { {  44,  1 }, 0x190 }, //
    { {  45,  1 }, 0x000 }, //
    { {  46,  1 }, 0x000 }, //
    { {  47,  2 }, 0x000 }, //
    { {  49,  1 }, 0x000 }, //
    { {  50,  2 }, 0x000 }, //
    { {  52,  2 }, 0x000 }, //
    { {  54,  2 }, 0x000 }, //
    { {  56,  1 }, 0x000 }, //
    { {  57,  1 }, 0x000 }, //
};

static const RBTokResult expTokMiscStdW[] = { // 36 tokens
    { {   0,  1 }, 0x064 }, //
    { {   1,  1 }, 0x000 }, //
    { {   2,  4 }, 0x0C8 }, //
    { {   6,  1 }, 0x000 }, //
    { {   7,  5 }, 0x0C8 }, //
    { {  12,  1 }, 0x000 }, // 0x014 for NLLT
    { {  13,  1 }, 0x000 }, //
    { {  14,  1 }, 0x064 }, //
    { {  15,  1 }, 0x0C8 }, //
    { {  16,  1 }, 0x000 }, //
    { {  17,  2 }, 0x064 }, //
    { {  19,  1 }, 0x0C8 }, //
    { {  20,  1 }, 0x000 }, //
    { {  21,  1 }, 0x190 }, //
    { {  22,  1 }, 0x190 }, //
    { {  23,  2 }, 0x190 }, //
    { {  25,  2 }, 0x190 }, //
    { {  27,  1 }, 0x000 }, //
    { {  28,  6 }, 0x0C8 }, //
    { {  34,  1 }, 0x000 }, //
    { {  35,  2 }, 0x0C8 }, //
    { {  37,  1 }, 0x000 }, //
    { {  38,  1 }, 0x190 }, //
    { {  39,  2 }, 0x190 }, //
    { {  41,  2 }, 0x190 }, //
    { {  43,  1 }, 0x000 }, //
    { {  44,  1 }, 0x190 }, //
    { {  45,  1 }, 0x190 }, //
    { {  46,  1 }, 0x190 }, //
    { {  47,  2 }, 0x190 }, //
    { {  49,  1 }, 0x190 }, //
    { {  50,  2 }, 0x190 }, //
    { {  52,  2 }, 0x190 }, //
    { {  54,  2 }, 0x190 }, //
    { {  56,  1 }, 0x000 }, //
    { {  57,  1 }, 0x000 }, //
};

static const UChar tokTextJpan[] = {
    0x30B3,0x30F3,0x30D4,0x30E5,0x30FC,0x30BF,0x30FC, // {{ 0, 7 }, 400 }
    0x306F,                      // {{  7, 1 }, 400 }
    0x3001,                      // {{  8, 1 }, 0 } 、
    0x672C,0x8CEA,               // {{  9, 2 }, 400 }
    0x7684,                      // {{ 11, 1 }, 400 }
    0x306B,                      // {{ 12, 1 }, 400 }
    0x306F,                      // {{ 13, 1 }, 400 }
    0x6570,0x5B57,               // {{ 14, 2 }, 400 }
    0x3057,0x304B,               // {{ 16, 2 }, 400 }
    0x6271,0x3046,               // {{ 18, 2 }, 400 }
    0x3053,0x3068,               // {{ 20, 2 }, 400 }
    0x304C,                      // {{ 22, 1 }, 400 }
    0x3067,0x304D,               // {{ 23, 2 }, 400 }
    0x307E,                      // {{ 25, 1 }, 400 }
    0x305B,0x3093,               // {{ 26, 2 }, 400 }
    0x3002,                      // {{ 28, 1 }, 0 } 。
    0x30B3,0x30F3,0x30D4,0x30E5,0x30FC,0x30BF,0x30FC, // {{ 29, 7 }, 400 }
    0x306F,                      // {{ 36, 1 }, 400 }
    0x3001,                      // {{ 37, 1 }, 0 } 、
    0x6587,0x5B57,               // {{ 38, 2 }, 400 }
    0x3084,                      // {{ 40, 1 }, 400 }
    0x8A18,0x53F7,               // {{ 41, 2 }, 400 }
    0x306A,0x3069,               // {{ 43, 2 }, 400 }
    0x306E,                      // {{ 45, 1 }, 400 }
    0x305D,0x308C,0x305E,0x308C,0x306B, // {{ 46, 5 }, 400 }
    0x756A,0x53F7,               // {{ 51, 2 }, 400 }
    0x3092,                      // {{ 53, 1 }, 400 }
    0x5272,0x308A,0x632F,0x308B, // {{ 54, 4 }, 400 }
    0x3053,0x3068,               // {{ 58, 2 }, 400 }
    0x306B,0x3088,0x3063,0x3066, // {{ 60, 4 }, 400 }
    0x6271,0x3048,0x308B,        // {{ 64, 3 }, 400 }
    0x3088,0x3046,               // {{ 67, 2 }, 400 }
    0x306B,0x3057,               // {{ 69, 2 }, 400 }
    0x307E,0x3059,               // {{ 71, 2 }, 400 }
    0x3002,                      // {{ 73, 1 }, 0 } 。
    0x30E6,0x30CB,               // {{ 74, 2 }, 400 }
    0x30B3,0x30FC,0x30C9,        // {{ 76, 3 }, 400 }
    0x304C,                      // {{ 79, 1 }, 400 }
    0x51FA,0x6765,0x308B,        // {{ 80, 3 }, 400 }
    0x307E,0x3067,               // {{ 83, 2 }, 400 }
    0x306F,                      // {{ 85, 1 }, 400 }
    0x3001,                      // {{ 86, 1 }, 0 } 、
    0x3053,0x308C,0x3089,0x306E, // {{ 87, 4 }, 400 }
    0x756A,0x53F7,               // {{ 91, 2 }, 400 }
    0x3092,                      // {{ 93, 1 }, 400 }
    0x5272,0x308A,0x632F,0x308B, // {{ 94, 4 }, 400 }
    0x4ED5,0x7D44,0x307F,        // {{ 98, 3 }, 400 }
    0x304C,                      // {{ 101, 1 }, 400 }
    0x767E,                      // {{ 102, 1 }, 400 }
    0x7A2E,0x985E,               // {{ 103, 2 }, 400 }
    0x3082,                      // {{ 105, 1 }, 400 }
    0x5B58,0x5728,               // {{ 106, 2  }, 400 }
    0x3057,0x307E,               // {{ 108, 2 }, 400 }
    0x3057,0x305F,               // {{ 110, 2 }, 400 }
    0x3002,                      // {{ 112, 1 }, 0 } 。
    0
};

static const RBTokResult expTokJpanCFST[] = { // 1 token (??)
    // ranges   flags            text
    {{  0, 113 }, 0x40000120 },
};

static const RBTokResult expTokJpan[] = { // 55 tokens
    // ranges   flags            text
    {{   0, 7 }, 400 },
    {{   7, 1 }, 400 },
    {{   8, 1 }, 0 },              // 、
    {{   9, 2 }, 400 },
    {{  11, 1 }, 400 },
    {{  12, 1 }, 400 },
    {{  13, 1 }, 400 },
    {{  14, 2 }, 400 },
    {{  16, 2 }, 400 },
    {{  18, 2 }, 400 },
    {{  20, 2 }, 400 },
    {{  22, 1 }, 400 },
    {{  23, 2 }, 400 },
    {{  25, 1 }, 400 },
    {{  26, 2 }, 400 },
    {{  28, 1 }, 0 },               // 。
    {{  29, 7 }, 400 },
    {{  36, 1 }, 400 },
    {{  37, 1 }, 0 },               // 、
    {{  38, 2 }, 400 },
    {{  40, 1 }, 400 },
    {{  41, 2 }, 400 },
    {{  43, 2 }, 400 },
    {{  45, 1 }, 400 },
    {{  46, 5 }, 400 },
    {{  51, 2 }, 400 },
    {{  53, 1 }, 400 },
    {{  54, 4 }, 400 },
    {{  58, 2 }, 400 },
    {{  60, 4 }, 400 },
    {{  64, 3 }, 400 },
    {{  67, 2 }, 400 },
    {{  69, 2 }, 400 },
    {{  71, 2 }, 400 },
    {{  73, 1 }, 0 },                // 。
    {{  74, 2 }, 400 },
    {{  76, 3 }, 400 },
    {{  79, 1 }, 400 },
    {{  80, 3 }, 400 },
    {{  83, 2 }, 400 },
    {{  85, 1 }, 400 },
    {{  86, 1 }, 0 },                // 、
    {{  87, 4 }, 400 },
    {{  91, 2 }, 400 },
    {{  93, 1 }, 400 },
    {{  94, 4 }, 400 },
    {{  98, 3 }, 400 },
    {{ 101, 1 }, 400 },
    {{ 102, 1 }, 400 },
    {{ 103, 2 }, 400 },
    {{ 105, 1 }, 400 },
    {{ 106, 2 }, 400 },
    {{ 108, 2 }, 400 },
    {{ 110, 2 }, 400 },
    {{ 112, 1 }, 0 },              // 。
};

static const RBTokResult expTokJpanNLLT_57Loop[] = { // 55 tokens
    {{   0, 7 }, 0x190 }, //
    {{   7, 1 }, 0x000 }, //
    {{   8, 1 }, 0x000 }, //
    {{   9, 2 }, 0x190 }, //
    {{  11, 1 }, 0x000 }, //
    {{  12, 1 }, 0x000 }, //
    {{  13, 1 }, 0x000 }, //
    {{  14, 2 }, 0x000 }, //
    {{  16, 2 }, 0x000 }, //
    {{  18, 2 }, 0x000 }, //
    {{  20, 2 }, 0x000 }, //
    {{  22, 1 }, 0x000 }, //
    {{  23, 2 }, 0x000 }, //
    {{  25, 1 }, 0x000 }, //
    {{  26, 2 }, 0x000 }, //
    {{  28, 1 }, 0x000 }, //
    {{  29, 7 }, 0x190 }, //
    {{  36, 1 }, 0x000 }, //
    {{  37, 1 }, 0x000 }, //
    {{  38, 2 }, 0x190 }, //
    {{  40, 1 }, 0x000 }, //
    {{  41, 2 }, 0x000 }, //
    {{  43, 2 }, 0x000 }, //
    {{  45, 1 }, 0x000 }, //
    {{  46, 5 }, 0x000 }, //
    {{  51, 2 }, 0x000 }, //
    {{  53, 1 }, 0x000 }, //
    {{  54, 4 }, 0x000 }, //
    {{  58, 2 }, 0x000 }, //
    {{  60, 4 }, 0x000 }, //
    {{  64, 3 }, 0x000 }, //
    {{  67, 2 }, 0x000 }, //
    {{  69, 2 }, 0x000 }, //
    {{  71, 2 }, 0x000 }, //
    {{  73, 1 }, 0x000 }, //
    {{  74, 2 }, 0x190 }, //
    {{  76, 3 }, 0x000 }, //
    {{  79, 1 }, 0x000 }, //
    {{  80, 3 }, 0x000 }, //
    {{  83, 2 }, 0x000 }, //
    {{  85, 1 }, 0x000 }, //
    {{  86, 1 }, 0x000 }, //
    {{  87, 4 }, 0x190 }, //
    {{  91, 2 }, 0x000 }, //
    {{  93, 1 }, 0x000 }, //
    {{  94, 4 }, 0x000 }, //
    {{  98, 3 }, 0x000 }, //
    {{ 101, 1 }, 0x000 }, //
    {{ 102, 1 }, 0x000 }, //
    {{ 103, 2 }, 0x000 }, //
    {{ 105, 1 }, 0x000 }, //
    {{ 106, 2 }, 0x000 }, //
    {{ 108, 2 }, 0x000 }, //
    {{ 110, 2 }, 0x000 }, //
    {{ 112, 1 }, 0x000 }, //
};


static const UChar tokTextThai[] = {
    0x55,0x6E,0x69,0x63,0x6F,0x64,0x65, // {{  0, 7 }, 200 },
    0x20,                               // {{  7, 1 }, 1 },
    0x0E04,0x0E37,0x0E2D,               // {{  8, 3 }, 200 },
    0x0E2D,0x0E30,0x0E44,0x0E23,        // {{ 11, 4 }, 200 },
    0x3F,                               // {{ 15, 1 }, 0 },
    0x0A,                               // {{ 16, 1 }, 0 },
    0x55,0x6E,0x69,0x63,0x6F,0x64,0x65, // {{ 17, 7 }, 200 },
    0x20,                               // {{ 24, 1 }, 1 },
    0x0E01,0x0E33,0x0E2B,0x0E19,0x0E14, // {{ 25, 5 }, 200 },
    0x0E2B,0x0E21,0x0E32,0x0E22,0x0E40,0x0E25,0x0E02, // {{ 30, 7 }, 200 },
    0x0E40,0x0E09,0x0E1E,0x0E32,0x0E30,        // {{ 37, 5 }, 200 },
    0x0E2A,0x0E33,0x0E2B,0x0E23,0x0E31,0x0E1A, // {{ 42, 6 }, 200 },
    0x0E17,0x0E38,0x0E01,                      // {{ 48, 3 }, 200 },
    0x0E2D,0x0E31,0x0E01,0x0E02,0x0E23,0x0E30, // {{ 51, 6 }, 200 },
    0x0A,                               // {{ 57, 1 }, 0 },
    0x0E42,0x0E14,0x0E22,               // {{ 58, 3 }, 200 },
    0x0E44,0x0E21,0x0E48,               // {{ 61, 3 }, 200 },
    0x0E2A,0x0E19,0x0E43,0x0E08,        // {{ 64, 4 }, 200 },
    0x0E27,0x0E48,0x0E32,               // {{ 68, 3 }, 200 },
    0x0E40,0x0E1B,0x0E47,0x0E19,        // {{ 71, 4 }, 200 },
    0x0E41,0x0E1E,                      // {{ 75, 2 }, 200 },
    0x0E25,0x0E47,0x0E15,               // {{ 77, 3 }, 200 },
    0x0E1F,0x0E2D,0x0E23,0x0E4C,0x0E21, // {{ 80, 5 }, 200 },
    0x0E43,0x0E14,                      // {{ 85, 2 }, 200 },
    0x0A,                               // {{ 87, 1 }, 0 },
    0x0E44,0x0E21,0x0E48,               // {{ 88, 3 }, 200 },
    0x0E02,0x0E36,0x0E49,0x0E19,        // {{ 91, 4 }, 200 },
    0x0E01,0x0E31,0x0E1A,               // {{ 95, 3 }, 200 },
    0x0E27,0x0E48,0x0E32,               // {{ 98, 3 }, 200 },
    0x0E08,0x0E30,                      // {{ 101, 2 }, 200 },
    0x0E40,0x0E1B,0x0E47,0x0E19,        // {{ 103, 4 }, 200 },
    0x0E42,0x0E1B,0x0E23,0x0E41,0x0E01,0x0E23,0x0E21, // {{ 107, 7 }, 200 },
    0x0E43,0x0E14,                      // {{ 114, 2 }, 200 },
    0x0A,                               // {{ 116, 1 }, 0 },
    0x0E41,0x0E25,0x0E30,               // {{ 117, 3 }, 200 },
    0x0E44,0x0E21,0x0E48,               // {{ 120, 3 }, 200 },
    0x0E27,0x0E48,0x0E32,               // {{ 123, 3 }, 200 },
    0x0E08,0x0E30,                      // {{ 126, 2 }, 200 },
    0x0E40,0x0E1B,0x0E47,0x0E19,        // {{ 128, 4 }, 200 },
    0x0E20,0x0E32,0x0E29,0x0E32,        // {{ 132, 4 }, 200 },
    0x0E43,0x0E14,                      // {{ 136, 2 }, 200 },
    0x0A,                               // {{ 138, 1 }, 0 },
    0
};

 static const RBTokResult expTokThaiCFST[] = { // 34 tokens
    {{   0,  7 }, 0x002 }, //
    {{   8,  3 }, 0x020 }, //
    {{  11,  5 }, 0x020 }, //
    {{  17,  7 }, 0x002 }, //
    {{  25,  5 }, 0x109 }, //
    {{  30,  7 }, 0x109 }, //
    {{  37,  5 }, 0x109 }, //
    {{  42,  6 }, 0x109 }, //
    {{  48,  3 }, 0x109 }, //
    {{  51,  6 }, 0x109 }, //
    {{  58,  3 }, 0x009 }, //
    {{  61,  3 }, 0x009 }, //
    {{  64,  4 }, 0x009 }, //
    {{  68,  3 }, 0x009 }, //
    {{  71,  4 }, 0x009 }, //
    {{  75,  2 }, 0x009 }, //
    {{  77,  3 }, 0x009 }, //
    {{  80,  5 }, 0x009 }, //
    {{  85,  2 }, 0x009 }, //
    {{  88,  3 }, 0x009 }, //
    {{  91,  4 }, 0x009 }, //
    {{  95,  3 }, 0x009 }, //
    {{  98,  3 }, 0x009 }, //
    {{ 101,  2 }, 0x009 }, //
    {{ 103,  4 }, 0x009 }, //
    {{ 107,  7 }, 0x009 }, //
    {{ 114,  2 }, 0x009 }, //
    {{ 117,  3 }, 0x009 }, //
    {{ 120,  3 }, 0x009 }, //
    {{ 123,  3 }, 0x009 }, //
    {{ 126,  2 }, 0x009 }, //
    {{ 128,  4 }, 0x009 }, //
    {{ 132,  4 }, 0x009 }, //
    {{ 136,  2 }, 0x009 }, //
};

 static const RBTokResult expTokThaiCFST_57Loop[] = { // 34 tokens
    {{   0,  7 }, 0x002 }, //
    {{   8,  3 }, 0x020 }, //
    {{  11,  5 }, 0x000 }, //
    {{  17,  7 }, 0x002 }, //
    {{  25,  5 }, 0x109 }, //
    {{  30,  7 }, 0x000 }, //
    {{  37,  5 }, 0x000 }, //
    {{  42,  6 }, 0x000 }, //
    {{  48,  3 }, 0x000 }, //
    {{  51,  6 }, 0x000 }, //
    {{  58,  3 }, 0x009 }, //
    {{  61,  3 }, 0x000 }, //
    {{  64,  4 }, 0x000 }, //
    {{  68,  3 }, 0x000 }, //
    {{  71,  4 }, 0x000 }, //
    {{  75,  2 }, 0x000 }, //
    {{  77,  3 }, 0x000 }, //
    {{  80,  5 }, 0x000 }, //
    {{  85,  2 }, 0x000 }, //
    {{  88,  3 }, 0x009 }, //
    {{  91,  4 }, 0x000 }, //
    {{  95,  3 }, 0x000 }, //
    {{  98,  3 }, 0x000 }, //
    {{ 101,  2 }, 0x000 }, //
    {{ 103,  4 }, 0x000 }, //
    {{ 107,  7 }, 0x000 }, //
    {{ 114,  2 }, 0x000 }, //
    {{ 117,  3 }, 0x009 }, //
    {{ 120,  3 }, 0x000 }, //
    {{ 123,  3 }, 0x000 }, //
    {{ 126,  2 }, 0x000 }, //
    {{ 128,  4 }, 0x000 }, //
    {{ 132,  4 }, 0x000 }, //
    {{ 136,  2 }, 0x000 }, //
};

static const RBTokResult expTokThai[] = { // 42 tokens
    // ranges   flags            text
    {{   0, 7 }, 0xC8 },  // 0xC8 = 200
    {{   7, 1 }, 0x00 },  //
    {{   8, 3 }, 0xC8 },
    {{  11, 4 }, 0xC8 },
    {{  15, 1 }, 0x00 },  //
    {{  16, 1 }, 0x00 },  //
    {{  17, 7 }, 0xC8 },
    {{  24, 1 }, 0x00 },  //
    {{  25, 5 }, 0xC8 },
    {{  30, 7 }, 0xC8 },
    {{  37, 5 }, 0xC8 },
    {{  42, 6 }, 0xC8 },
    {{  48, 3 }, 0xC8 },
    {{  51, 6 }, 0xC8 },
    {{  57, 1 }, 0x00 },  //
    {{  58, 3 }, 0xC8 },
    {{  61, 3 }, 0xC8 },
    {{  64, 4 }, 0xC8 },
    {{  68, 3 }, 0xC8 },
    {{  71, 4 }, 0xC8 },
    {{  75, 2 }, 0xC8 },
    {{  77, 3 }, 0xC8 },
    {{  80, 5 }, 0xC8 },
    {{  85, 2 }, 0xC8 },
    {{  87, 1 }, 0x00 },  //
    {{  88, 3 }, 0xC8 },
    {{  91, 4 }, 0xC8 },
    {{  95, 3 }, 0xC8 },
    {{  98, 3 }, 0xC8 },
    {{ 101, 2 }, 0xC8 },
    {{ 103, 4 }, 0xC8 },
    {{ 107, 7 }, 0xC8 },
    {{ 114, 2 }, 0xC8 },
    {{ 116, 1 }, 0x00 },  //
    {{ 117, 3 }, 0xC8 },
    {{ 120, 3 }, 0xC8 },
    {{ 123, 3 }, 0xC8 },
    {{ 126, 2 }, 0xC8 },
    {{ 128, 4 }, 0xC8 },
    {{ 132, 4 }, 0xC8 },
    {{ 136, 2 }, 0xC8 },
    {{ 138, 1 }, 0x00 },  //
};

static const RBTokResult expTokThaiNLLT_57Loop[] = { // 42 tokens
    {{   0, 7 }, 0xC8 },  // 0xC8 = 200
    {{   7, 1 }, 0x00 },  //
    {{   8, 3 }, 0xC8 },  //
    {{  11, 4 }, 0x00 },  //
    {{  15, 1 }, 0x00 },  //
    {{  16, 1 }, 0x00 },  //
    {{  17, 7 }, 0xC8 },  //
    {{  24, 1 }, 0x00 },  //
    {{  25, 5 }, 0xC8 },  //
    {{  30, 7 }, 0x00 },  //
    {{  37, 5 }, 0x00 },  //
    {{  42, 6 }, 0x00 },  //
    {{  48, 3 }, 0x00 },  //
    {{  51, 6 }, 0x00 },  //
    {{  57, 1 }, 0x00 },  //
    {{  58, 3 }, 0xC8 },  //
    {{  61, 3 }, 0x00 },  //
    {{  64, 4 }, 0x00 },  //
    {{  68, 3 }, 0x00 },  //
    {{  71, 4 }, 0x00 },  //
    {{  75, 2 }, 0x00 },  //
    {{  77, 3 }, 0x00 },  //
    {{  80, 5 }, 0x00 },  //
    {{  85, 2 }, 0x00 },  //
    {{  87, 1 }, 0x00 },  //
    {{  88, 3 }, 0xC8 },  //
    {{  91, 4 }, 0x00 },  //
    {{  95, 3 }, 0x00 },  //
    {{  98, 3 }, 0x00 },  //
    {{ 101, 2 }, 0x00 },  //
    {{ 103, 4 }, 0x00 },  //
    {{ 107, 7 }, 0x00 },  //
    {{ 114, 2 }, 0x00 },  //
    {{ 116, 1 }, 0x00 },  //
    {{ 117, 3 }, 0xC8 },  //
    {{ 120, 3 }, 0x00 },  //
    {{ 123, 3 }, 0x00 },  //
    {{ 126, 2 }, 0x00 },  //
    {{ 128, 4 }, 0x00 },  //
    {{ 132, 4 }, 0x00 },  //
    {{ 136, 2 }, 0x00 },  //
    {{ 138, 1 }, 0x00 },  //
};


typedef struct {
    const char*         descrip;
    const UChar*        tokText; // zero-terminated text
    const RBTokResult*  expTok;
    int32_t             expTokLen;
    const RBTokResult*  expTokFile;
    int32_t             expTokFileLen;
    const RBTokResult*  expTok57Loop;    // Actual urbtok results in ICU 59 for this test
    int32_t             expTok57LoopLen; // Actual urbtok results in ICU 59 for this test
    const RBTokResult*  expTok57Bulk;    // Actual urbtok results in ICU 59 for this test
    int32_t             expTok57BulkLen; // Actual urbtok results in ICU 59 for this test
}  TokTextAndResults;

static const TokTextAndResults tokTextAndResCFST[] = {
    { "CFST/Latn", tokTextLatn, expTokLatnCFST, UPRV_LENGTHOF(expTokLatnCFST), expTokLatnCFST, UPRV_LENGTHOF(expTokLatnCFST), expTokLatnCFST,        UPRV_LENGTHOF(expTokLatnCFST),        expTokLatnCFST,        UPRV_LENGTHOF(expTokLatnCFST) },
    { "CFST/Misc", tokTextMisc, expTokMiscCFST, UPRV_LENGTHOF(expTokMiscCFST), expTokMiscCFST, UPRV_LENGTHOF(expTokMiscCFST), expTokMiscCFST,        UPRV_LENGTHOF(expTokMiscCFST),        expTokMiscCFST_57Bulk, UPRV_LENGTHOF(expTokMiscCFST_57Bulk) },
    { "CFST/Jpan", tokTextJpan, expTokJpanCFST, UPRV_LENGTHOF(expTokJpanCFST), expTokJpanCFST, UPRV_LENGTHOF(expTokJpanCFST), expTokJpanCFST,        UPRV_LENGTHOF(expTokJpanCFST),        expTokJpanCFST,        UPRV_LENGTHOF(expTokJpanCFST) },
    { "CFST/Thai", tokTextThai, expTokThaiCFST, UPRV_LENGTHOF(expTokThaiCFST), expTokThaiCFST, UPRV_LENGTHOF(expTokThaiCFST), expTokThaiCFST_57Loop, UPRV_LENGTHOF(expTokThaiCFST_57Loop), expTokThaiCFST,        UPRV_LENGTHOF(expTokThaiCFST) },
    { NULL, NULL, NULL, 0, NULL, 0, NULL, 0 }
};

static const TokTextAndResults tokTextAndResNLLT[] = {
    { "NLLT/Latn", tokTextLatn, expTokLatnNLLT, UPRV_LENGTHOF(expTokLatnNLLT), expTokLatnNLLT_File, UPRV_LENGTHOF(expTokLatnNLLT_File), expTokLatnNLLT_57,     UPRV_LENGTHOF(expTokLatnNLLT_57),     expTokLatnNLLT_57,     UPRV_LENGTHOF(expTokLatnNLLT_57)},
    { "NLLT/Misc", tokTextMisc, expTokMiscNLLT, UPRV_LENGTHOF(expTokMiscNLLT), expTokMiscNLLT,      UPRV_LENGTHOF(expTokMiscNLLT),      expTokMiscNLLT_57Loop, UPRV_LENGTHOF(expTokMiscNLLT_57Loop), expTokMiscNLLT,        UPRV_LENGTHOF(expTokMiscNLLT)},
    { "NLLT/Jpan", tokTextJpan, expTokJpan,     UPRV_LENGTHOF(expTokJpan),     expTokJpan,          UPRV_LENGTHOF(expTokJpan),          expTokJpanNLLT_57Loop, UPRV_LENGTHOF(expTokJpanNLLT_57Loop), expTokJpan,            UPRV_LENGTHOF(expTokJpan) },
    { "NLLT/Thai", tokTextThai, expTokThai,     UPRV_LENGTHOF(expTokThai),     expTokThai,          UPRV_LENGTHOF(expTokThai),          expTokThaiNLLT_57Loop, UPRV_LENGTHOF(expTokThaiNLLT_57Loop), expTokThai,            UPRV_LENGTHOF(expTokThai) },
    { NULL, NULL, NULL, 0, NULL, 0, NULL, 0 }
};

static const TokTextAndResults tokTextAndResStdW[] = {
    { "StdW/Latn", tokTextLatn, expTokLatnStdW, UPRV_LENGTHOF(expTokLatnStdW), expTokLatnStdW, UPRV_LENGTHOF(expTokLatnStdW), expTokLatnStdW,        UPRV_LENGTHOF(expTokLatnStdW),        expTokLatnStdW,       UPRV_LENGTHOF(expTokLatnStdW) },
    { "StdW/Misc", tokTextMisc, expTokMiscStdW, UPRV_LENGTHOF(expTokMiscStdW), expTokMiscStdW, UPRV_LENGTHOF(expTokMiscStdW), expTokMiscStdW,        UPRV_LENGTHOF(expTokMiscStdW),        expTokMiscStdW,       UPRV_LENGTHOF(expTokMiscStdW) },
    { "StdW/Jpan", tokTextJpan, expTokJpan,     UPRV_LENGTHOF(expTokJpan),     expTokJpan,     UPRV_LENGTHOF(expTokJpan),     expTokJpan,            UPRV_LENGTHOF(expTokJpan),            expTokJpan,           UPRV_LENGTHOF(expTokJpan) },
    { "StdW/Thai", tokTextThai, expTokThai,     UPRV_LENGTHOF(expTokThai),     expTokThai,     UPRV_LENGTHOF(expTokThai),     expTokThai,            UPRV_LENGTHOF(expTokThai),            expTokThai,           UPRV_LENGTHOF(expTokThai) },
    { NULL, NULL, NULL, 0, NULL, 0, NULL, 0 }
};

typedef struct {
    const char*             descrip;
    const char*             rulesSource; // relative to cintltst directory; UTF8 rule source; NULL for std word rules
    const char*             rulesBin;    // relative to cintltst directory; current binary version
    const char*             rulesBin57;  // relative to cintltst directory; ICU 57 binary version
    const TokTextAndResults* textAndResults;
} TokRulesAndTests;

static const TokRulesAndTests tokRulesTests[] = { // icu60 binary files invalid in ICU 62
    { "CFST", "../testdata/tokCFSTrules.txt",     NULL,/*tokCFSTrulesLE_icu60.data invalid*/ "../testdata/tokCFSTrulesLE_icu57.data", tokTextAndResCFST },
    { "NLLT", "../testdata/wordNLLTu8.txt",       NULL,/*wordNLLT_icu60.dat invalid */       "../testdata/wordNLLT_icu57.dat",        tokTextAndResNLLT },
    { "StdW", "../../data/brkitr/rules/word.txt", NULL,                                      NULL,                                    tokTextAndResStdW },
    { "WORD", NULL,                               NULL,                                      NULL,                                    tokTextAndResStdW },
    { NULL, NULL, NULL, NULL, NULL }
};

enum {
    kMaxTokens = 96
};

// read data from file into a malloc'ed buf, which must be freed by caller.
// returns NULL if error.
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

static void handleTokResults(const char* testItem, const char* tokClass, const char* ruleSource, const char* algType,
                             uint64_t duration, int32_t expTokLen, const RBTokResult* expTokRes,
                             int32_t getTokLen, RuleBasedTokenRange* getTokens, unsigned long *getFlags) {
    int32_t iToken;
    UBool fail = (getTokLen != expTokLen);
    for (iToken = 0; !fail && iToken < getTokLen; iToken++) {
        if (  getTokens[iToken].location != expTokRes[iToken].token.location || getTokens[iToken].length != expTokRes[iToken].token.length ||
              getFlags[iToken] != expTokRes[iToken].flags ) {
            fail = TRUE;
        }
    }
    if (fail) {
        log_err("FAIL: %s %s %s %s expected %d tokens, got %d\n", testItem, tokClass, ruleSource, algType, expTokLen, getTokLen);
        printf("# expect               get\n");
        printf("# loc len flags        loc len flags\n");
        int32_t maxTokens = (getTokLen >= expTokLen)? getTokLen: expTokLen;
        for (iToken = 0; iToken < maxTokens; iToken++) {
            if (iToken < expTokLen) {
                printf("  %3ld %3ld 0x%-8lX", expTokRes[iToken].token.location,
                    expTokRes[iToken].token.length, expTokRes[iToken].flags);
            } else {
                printf("                  ");
            }
            if (iToken < getTokLen) {
                printf("   %3ld %3ld 0x%-8lX\n", getTokens[iToken].location, getTokens[iToken].length, getFlags[iToken] );
            } else {
                printf("\n");
            }
        }
    } else {
        log_info("%s %s %s %s get %d tokens, nsec %llu\n", testItem, tokClass, ruleSource, algType, getTokLen, duration);
    }
}

static void TestRuleBasedTokenizer(void) {
    const TokRulesAndTests* ruleTypePtr;
#if !U_PLATFORM_HAS_WIN32_API
    uint64_t start, duration;
#else
    unsigned long long start, duration;
#endif
#if U_PLATFORM_IS_DARWIN_BASED
    mach_timebase_info_data_t info;

    mach_timebase_info(&info);
#endif
    for (ruleTypePtr = tokRulesTests; ruleTypePtr->descrip != NULL; ruleTypePtr++) {
        UBreakIterator* ubrkFromSource      = NULL;
        UBreakIterator* ubrkBinFromSource   = NULL;
        UBreakIterator* utokFromSource      = NULL;
        UBreakIterator* utokBinFromSource   = NULL;
        UBreakIterator* utokBinFromFile     = NULL;
        UBreakIterator* utok57FromSource    = NULL;
        UBreakIterator* utok57BinFromSource = NULL;
        UBreakIterator* utok57BinFromFile   = NULL;
        uint8_t* ubrkBinRules = NULL; // these must be retained while ubrkBinFromSource is open
        UErrorCode status = U_ZERO_ERROR;

        log_info("- starting tests for rule type %s\n", ruleTypePtr->descrip);

        // Get UBreakIterators
        if (ruleTypePtr->rulesSource == NULL) {
            // use standard WORD rules for root
            start = GET_START();
            ubrkFromSource = ubrk_open(UBRK_WORD, "root", NULL, 0, &status);
            duration = GET_DURATION(start, info);
            if (U_FAILURE(status)) {
                log_err("FAIL: ubrk_open WORD for root, status: %s\n", u_errorName(status));
            } else {
                log_info(" ubrk_open nsec %llu\n", duration);
                int32_t rulesBinFromStdSize = ubrk_getBinaryRules(ubrkFromSource, NULL, 0, &status);
                if (U_FAILURE(status)) {
                    log_err("FAIL: ubrk_getBinaryRules preflight status: %s, rulesBinFromStdSize %d\n", u_errorName(status), rulesBinFromStdSize);
                } else {
                    ubrkBinRules = (uint8_t *)uprv_malloc(rulesBinFromStdSize);
                    if (ubrkBinRules == NULL) {
                        log_data_err("FAIL: uprv_malloc fails for ubrkBinRules[%d]\n", rulesBinFromStdSize);
                    } else {
                        start = GET_START();
                        rulesBinFromStdSize = ubrk_getBinaryRules(ubrkFromSource, ubrkBinRules, rulesBinFromStdSize, &status);
                        duration = GET_DURATION(start, info);
                        if (U_FAILURE(status)) {
                            log_err("FAIL: ubrk_getBinaryRules status: %s, rulesBinFromStdSize %d\n", u_errorName(status), rulesBinFromStdSize);
                        } else {
                            log_info(" ubrk_getBinaryRules size %d, nsec %llu\n", rulesBinFromStdSize, duration);

                            status = U_ZERO_ERROR;
                            start = GET_START();
                            // ubrk_openBinaryRules does not copy the binary rules, they must be kept around while ubrkBinFromSource is open
                            ubrkBinFromSource = ubrk_openBinaryRules(ubrkBinRules, rulesBinFromStdSize, NULL, 0, &status);
                            duration = GET_DURATION(start, info);
                            if (U_FAILURE(status)) {
                                log_err("FAIL: ubrk_openBinaryRules status: %s\n", u_errorName(status));
                            } else {
                                log_info(" ubrk_openBinaryRules nsec %llu\n", duration);
                            }

                            status = U_ZERO_ERROR;
                            start = GET_START();
                            utokBinFromSource = urbtok_openBinaryRules(ubrkBinRules, &status);
                            duration = GET_DURATION(start, info);
                            if (U_FAILURE(status)) {
                                log_err("FAIL: urbtok_openBinaryRules status: %s\n", u_errorName(status));
                            } else {
                                log_info(" urbtok_openBinaryRules nsec %llu\n", duration);
                            }

                            status = U_ZERO_ERROR;
                            utok57BinFromSource = urbtok57_openBinaryRules(ubrkBinRules, &status);
                            if (U_SUCCESS(status)) {
                                log_err("FAIL: urbtok57_openBinaryRules with new rules succeeded but should have failed\n");
                                ubrk_close(utok57BinFromSource);
                                utok57BinFromSource = NULL;
                            }
                        }
                        // ubrkBinRules is freed at the end of the main loop
                    }
                }
            }
            status = U_ZERO_ERROR;
            start = GET_START();
            utokFromSource = urbtok_open(UBRK_WORD, "root", &status);
            duration = GET_DURATION(start, info);
            if (U_FAILURE(status)) {
                log_err("FAIL: urbtok_open WORD for root, status: %s\n", u_errorName(status));
            } else {
                log_info(" urbtok_open nsec %llu\n", duration);
            }
        } else {
            // use source rules, including custom rules from CoreNLP
            int32_t rulesUTF16Size;
            UChar* rulesUTF16Buf = NULL;
            long rulesUTF8Size;
            char * rulesUTF8Buf = (char *)dataBufFromFile(ruleTypePtr->rulesSource, &rulesUTF8Size);
            // dataBufFromFile already logged any errors leading to NULL return
            if (rulesUTF8Buf) {
                long rulesUTF8Offset = 0;
                /* Handle UTF8 BOM: */
                if (uprv_strncmp(rulesUTF8Buf, "\xEF\xBB\xBF", 3) == 0) {
                    rulesUTF8Offset = 3;
                    rulesUTF8Size -= rulesUTF8Offset;
                }
                u_strFromUTF8(NULL, 0, &rulesUTF16Size, rulesUTF8Buf+rulesUTF8Offset, rulesUTF8Size, &status); /* preflight */
                if (status == U_BUFFER_OVERFLOW_ERROR) { /* expected for preflight */
                    status = U_ZERO_ERROR;
                }
                if (U_FAILURE(status)) {
                    log_data_err("FAIL: for %s, u_strFromUTF8 preflight fails: %s\n", ruleTypePtr->rulesSource, u_errorName(status));
                } else {
                    rulesUTF16Buf = (UChar *)uprv_malloc(rulesUTF16Size*sizeof(UChar));
                    if (rulesUTF16Buf == NULL) {
                        log_data_err("FAIL: for %s, uprv_malloc fails for rulesUTF16Buf[%ld]\n", ruleTypePtr->rulesSource, rulesUTF16Size*sizeof(UChar));
                    } else {
                        u_strFromUTF8(rulesUTF16Buf, rulesUTF16Size, &rulesUTF16Size, rulesUTF8Buf+rulesUTF8Offset, rulesUTF8Size, &status);
                        if (U_FAILURE(status)) {
                            log_data_err("FAIL: for %s, u_strFromUTF8 fails: %s\n", ruleTypePtr->rulesSource, u_errorName(status));
                            uprv_free(rulesUTF16Buf);
                            rulesUTF16Buf = NULL;
                        }
                    }
                }
                uprv_free(rulesUTF8Buf);
            }
            if (rulesUTF16Buf) {
                UParseError parseErr;

                status = U_ZERO_ERROR;
                start = GET_START();
                ubrkFromSource = ubrk_openRules(rulesUTF16Buf, rulesUTF16Size, NULL, 0, &parseErr, &status);
                duration = GET_DURATION(start, info);
                if (U_FAILURE(status)) {
                    log_err("FAIL: ubrk_openRules %s status: %s, line %d, col %d\n", ruleTypePtr->rulesSource, u_errorName(status), parseErr.line, parseErr.offset);
                } else {
                    log_info(" ubrk_openRules nsec %llu\n", duration);
                }

                status = U_ZERO_ERROR;
                start = GET_START();
                utokFromSource = urbtok_openRules(rulesUTF16Buf, rulesUTF16Size, &parseErr, &status);
                duration = GET_DURATION(start, info);
                if (U_FAILURE(status)) {
                    log_err("FAIL: urbtok_openRules %s status: %s, line %d, col %d\n", ruleTypePtr->rulesSource, u_errorName(status), parseErr.line, parseErr.offset);
                } else {
                    log_info(" urbtok_openRules nsec %llu\n", duration);
                }

                status = U_ZERO_ERROR;
                start = GET_START();
                utok57FromSource = urbtok57_openRules(rulesUTF16Buf, rulesUTF16Size, &parseErr, &status);
                duration = GET_DURATION(start, info);
                if (U_FAILURE(status)) {
                    if (ruleTypePtr->rulesBin57) {
                        // Have binary, source should word
                        log_err("FAIL: urbtok57_openRules %s status: %s, line %d, col %d\n", ruleTypePtr->rulesSource, u_errorName(status), parseErr.line, parseErr.offset);
                    } else {
                        // Source may use new syntax not compatible with urbtok57
                        log_info(" urbtok57_openRules cannot handle %s, status: %s, line %d, col %d\n", ruleTypePtr->rulesSource, u_errorName(status), parseErr.line, parseErr.offset);
                    }
                } else {
                    log_info(" urbtok57_openRules nsec %llu\n", duration);
                }

                uprv_free(rulesUTF16Buf);
            }
            if (ubrkFromSource) {
                status = U_ZERO_ERROR;
                int32_t rulesBinFromSourceSize = ubrk_getBinaryRules(ubrkFromSource, NULL, 0, &status);
                if (U_FAILURE(status)) {
                    log_err("FAIL: ubrk_getBinaryRules preflight status: %s, rulesBinFromSourceSize %d\n", u_errorName(status), rulesBinFromSourceSize);
                } else {
                    ubrkBinRules = (uint8_t *)uprv_malloc(rulesBinFromSourceSize);
                    if (ubrkBinRules == NULL) {
                        log_data_err("FAIL: uprv_malloc fails for ubrk ubrkBinRules[%d]\n", rulesBinFromSourceSize);
                    } else {
                        start = GET_START();
                        rulesBinFromSourceSize = ubrk_getBinaryRules(ubrkFromSource, ubrkBinRules, rulesBinFromSourceSize, &status);
                        duration = GET_DURATION(start, info);
                        if (U_FAILURE(status)) {
                            log_err("FAIL: ubrk_getBinaryRules status: %s, rulesBinFromSourceSize %d\n", u_errorName(status), rulesBinFromSourceSize);
                        } else {
                            log_info(" ubrk_getBinaryRules size %d, nsec %llu\n", rulesBinFromSourceSize, duration);

                            start = GET_START();
                            // ubrk_openBinaryRules does not copy the binary rules, they must be kept around while ubrkBinFromSource is open
                            ubrkBinFromSource = ubrk_openBinaryRules(ubrkBinRules, rulesBinFromSourceSize, NULL, 0, &status);
                            duration = GET_DURATION(start, info);
                            if (U_FAILURE(status)) {
                                log_err("FAIL: ubrk_openBinaryRules status: %s\n", u_errorName(status));
                            } else {
                                log_info(" ubrk_openBinaryRules nsec %llu\n", duration);
                            }
                        }
                        // ubrkBinRules is freed at the end of the main loop
                    }
                }
            }

            if (utokFromSource) {
                status = U_ZERO_ERROR;
                int32_t rulesBinFromSourceSize = urbtok_getBinaryRules(utokFromSource, NULL, 0, &status);
                if (U_FAILURE(status)) {
                    log_err("FAIL: urbtok_getBinaryRules preflight status: %s, rulesBinFromSourceSize %d\n", u_errorName(status), rulesBinFromSourceSize);
                } else {
                    uint8_t* rulesBinFromSource = (uint8_t *)uprv_malloc(rulesBinFromSourceSize);
                    if (rulesBinFromSource == NULL) {
                        log_data_err("FAIL: uprv_malloc fails for urbtok rulesBinFromSource[%d]\n", rulesBinFromSourceSize);
                    } else {
                        start = GET_START();
                        rulesBinFromSourceSize = urbtok_getBinaryRules(utokFromSource, rulesBinFromSource, rulesBinFromSourceSize, &status);
                        duration = GET_DURATION(start, info);
                        if (U_FAILURE(status)) {
                            log_err("FAIL: urbtok_getBinaryRules status: %s, rulesBinFromSourceSize %d\n", u_errorName(status), rulesBinFromSourceSize);
                        } else {
                            log_info(" urbtok_getBinaryRules size %d, nsec %llu\n", rulesBinFromSourceSize, duration);

                            status = U_ZERO_ERROR;
                            start = GET_START();
                            // ubrk_openBinaryRules does copy the binary rules, they can be freed at the end of this block
                            utokBinFromSource = urbtok_openBinaryRules(rulesBinFromSource, &status);
                            duration = GET_DURATION(start, info);
                            if (U_FAILURE(status)) {
                                log_err("FAIL: urbtok_openBinaryRules from source status: %s\n", u_errorName(status));
                            } else {
                                log_info(" urbtok_openBinaryRules from source nsec %llu\n", duration);
                            }

                            status = U_ZERO_ERROR;
                            utok57BinFromSource = urbtok57_openBinaryRules(rulesBinFromSource, &status);
                            if (U_SUCCESS(status)) {
                                log_err("FAIL: urbtok57_openBinaryRules with new rules succeeded but should have failed\n");
                                ubrk_close(utok57BinFromSource);
                                utok57BinFromSource = NULL;
                            }

                            if (ruleTypePtr->rulesBin) {
                                long rulesBinSize;
                                uint8_t* rulesBinBuf = (uint8_t*)dataBufFromFile(ruleTypePtr->rulesBin, &rulesBinSize);
                                // dataBufFromFile already logged any errors leading to NULL return
                                if (rulesBinBuf) {
                                    log_info(" get urbtok binary rules from file, rulesBinSize %d\n", rulesBinSize);
                                    status = U_ZERO_ERROR;
                                    start = GET_START();
                                    utokBinFromFile = urbtok_openBinaryRules(rulesBinBuf, &status);
                                    duration = GET_DURATION(start, info);
                                    if (U_FAILURE(status)) {
                                        log_err("FAIL: urbtok_openBinaryRules from file status: %s\n", u_errorName(status));
                                    } else {
                                        log_info(" urbtok_openBinaryRules from file nsec %llu\n", duration);
                                    }
                                    uprv_free(rulesBinBuf);
                                }
                            }
                        }
                        uprv_free(rulesBinFromSource);
                    }
                }
            }

            if (utok57FromSource) {
                status = U_ZERO_ERROR;
                int32_t rulesBinFromSourceSize = urbtok57_getBinaryRules(utok57FromSource, NULL, 0, &status);
                if (U_FAILURE(status)) {
                    log_err("FAIL: urbtok57_getBinaryRules preflight status: %s, rulesBinFromSourceSize %d\n", u_errorName(status), rulesBinFromSourceSize);
                } else {
                    uint8_t* rulesBinFromSource = (uint8_t *)uprv_malloc(rulesBinFromSourceSize);
                    if (rulesBinFromSource == NULL) {
                        log_data_err("FAIL: uprv_malloc fails for urbtok57 rulesBinFromSource[%d]\n", rulesBinFromSourceSize);
                    } else {
                        start = GET_START();
                        rulesBinFromSourceSize = urbtok57_getBinaryRules(utok57FromSource, rulesBinFromSource, rulesBinFromSourceSize, &status);
                        duration = GET_DURATION(start, info);
                        if (U_FAILURE(status)) {
                            log_err("FAIL: urbtok57_getBinaryRules status: %s, rulesBinFromSourceSize %d\n", u_errorName(status), rulesBinFromSourceSize);
                        } else {
                            log_info(" urbtok57_getBinaryRules size %d, nsec %llu\n", rulesBinFromSourceSize, duration);

                            status = U_ZERO_ERROR;
                            start = GET_START();
                            // ubrk_openBinaryRules does copy the binary rules, they can be freed at the end of this block
                            utok57BinFromSource = urbtok57_openBinaryRules(rulesBinFromSource, &status);
                            duration = GET_DURATION(start, info);
                            if (U_FAILURE(status)) {
                                log_err("FAIL: urbtok57_openBinaryRules from source status: %s\n", u_errorName(status));
                            } else {
                                log_info(" urbtok57_openBinaryRules from source nsec %llu\n", duration);
                            }

                            if (ruleTypePtr->rulesBin57) {
                                long rulesBinSize;
                                uint8_t* rulesBinBuf = (uint8_t*)dataBufFromFile(ruleTypePtr->rulesBin57, &rulesBinSize);
                                // dataBufFromFile already logged any errors leading to NULL return
                                if (rulesBinBuf) {
                                    log_info(" get urbtok57 binary rules from file, rulesBinSize %d\n", rulesBinSize);
                                    status = U_ZERO_ERROR;
                                    start = GET_START();
                                    utok57BinFromFile = urbtok57_openBinaryRules(rulesBinBuf, &status);
                                    duration = GET_DURATION(start, info);
                                    if (U_FAILURE(status)) {
                                        log_err("FAIL: urbtok57_openBinaryRules from file status: %s\n", u_errorName(status));
                                    } else {
                                        log_info(" urbtok57_openBinaryRules file nsec %llu\n", duration);
                                    }
                                    uprv_free(rulesBinBuf);
                                }
                            }
                        }
                        uprv_free(rulesBinFromSource);
                    }
                }
            }
        }

        if (ubrkFromSource) {
            // Test tokenization
            const TokTextAndResults* textResultsPtr;
            for (textResultsPtr = ruleTypePtr->textAndResults; textResultsPtr->descrip != NULL; textResultsPtr++) {
                RuleBasedTokenRange tokens[kMaxTokens];
                unsigned long       flags[kMaxTokens];
                RuleBasedTokenRange *tokenLimit = tokens + kMaxTokens;
                RuleBasedTokenRange *tokenP;
                unsigned long *flagsP;
                int32_t iTest, numTokens;
                const char* testType[] = { "source", "binFromSource", "binFromFile" };
                int32_t textLen = u_strlen(textResultsPtr->tokText);
                log_info("-- starting tests for rule/text combo %s, text UTF16 units: %d\n", textResultsPtr->descrip, textLen);

                if (ubrkFromSource || ubrkBinFromSource) {
                    for (iTest = 0; iTest < 2; iTest++) {
                        UBreakIterator* ubrk = (iTest==0)? ubrkFromSource: ubrkBinFromSource;
                        if (ubrk) {
                            int32_t offset, lastOffset;
                            // Do ubrk loop tests
                            status = U_ZERO_ERROR;
                            ubrk_setText(ubrk, textResultsPtr->tokText, textLen, &status);
                            if (U_FAILURE(status)) {
                                log_err("FAIL: %s ubrk_setText ubrk %s status: %s\n", textResultsPtr->descrip, testType[iTest], u_errorName(status));
                            } else {
                                start = GET_START();
                                lastOffset = ubrk_current(ubrk);
                                for (tokenP = tokens, flagsP = flags; tokenP < tokenLimit && (offset = ubrk_next(ubrk)) != UBRK_DONE;) {
                                    int32_t flagSet = ubrk_getRuleStatus(ubrk);
                                    if (flagSet != -1) {
                                        int32_t flagVec[8];
                                        int32_t flagCount;
                                        UErrorCode locStatus = U_ZERO_ERROR;

                                        tokenP->location = lastOffset;
                                        tokenP++->length = offset - lastOffset;
                                        flagCount = ubrk_getRuleStatusVec(ubrk, flagVec, 8, &locStatus);
                                        if (U_SUCCESS(locStatus) && flagCount-- > 1) {
                                            // skip last flagVec entry since we have from ubrk_getRuleStatus above
                                            int32_t flagIdx;
                                            for (flagIdx = 0; flagIdx < flagCount; flagIdx++) {
                                                flagSet |= flagVec[flagIdx];
                                            }
                                        }
                                        *flagsP++ = (unsigned long)flagSet;
                                    }
                                    lastOffset = offset;
                                }
                                numTokens = tokenP - tokens;
                                duration = GET_DURATION(start, info);

                                handleTokResults(textResultsPtr->descrip, "ubrk", testType[iTest], "(loop)", duration,
                                            textResultsPtr->expTokLen, textResultsPtr->expTok, numTokens, tokens, flags);
                            }
                        }
                    }
                }

                if (utokFromSource || utokBinFromSource) {
                    for (iTest = 0; iTest < 3; iTest++) {
                        UBreakIterator* utok = (iTest==0)? utokFromSource: ((iTest==1)? utokBinFromSource: utokBinFromFile);
                        if (utok) {
                            // Do utok loop & bulk tests
                            status = U_ZERO_ERROR;
                            ubrk_setText(utok, textResultsPtr->tokText, textLen, &status);
                            if (U_FAILURE(status)) {
                                log_err("FAIL: %s ubrk_setText utok %s (loop) status: %s\n", textResultsPtr->descrip, testType[iTest], u_errorName(status));
                            } else {
                                start = GET_START();
                                for (tokenP = tokens, flagsP = flags; tokenP < tokenLimit && urbtok_tokenize(utok, 1, tokenP, flagsP) == 1; tokenP++, flagsP++) {
                                    ;
                                }
                                numTokens = tokenP - tokens;
                                duration = GET_DURATION(start, info);

                                if (iTest < 2) {
                                    handleTokResults(textResultsPtr->descrip, "utok", testType[iTest], "(loop)", duration,
                                                textResultsPtr->expTokLen, textResultsPtr->expTok, numTokens, tokens, flags);
                                } else {
                                    handleTokResults(textResultsPtr->descrip, "utok", testType[iTest], "(loop)", duration,
                                                textResultsPtr->expTokFileLen, textResultsPtr->expTokFile, numTokens, tokens, flags);
                                }
                            }

                            status = U_ZERO_ERROR;
                            ubrk_setText(utok, textResultsPtr->tokText, textLen, &status);
                            if (U_FAILURE(status)) {
                                log_err("FAIL: %s ubrk_setText utok %s (bulk) status: %s\n", textResultsPtr->descrip, testType[iTest], u_errorName(status));
                            } else {
                                start = GET_START();
                                numTokens = urbtok_tokenize(utok, kMaxTokens, tokens, flags);
                                duration = GET_DURATION(start, info);

                                if (iTest < 2) {
                                    handleTokResults(textResultsPtr->descrip, "utok", testType[iTest], "(bulk)", duration,
                                                textResultsPtr->expTokLen, textResultsPtr->expTok, numTokens, tokens, flags);
                                } else {
                                    handleTokResults(textResultsPtr->descrip, "utok", testType[iTest], "(bulk)", duration,
                                                textResultsPtr->expTokFileLen, textResultsPtr->expTokFile, numTokens, tokens, flags);
                                }
                            }
                       }
                    }
                }

                if (utok57FromSource || utok57BinFromSource) {
                    for (iTest = 0; iTest < 3; iTest++) {
                        UBreakIterator* utok57 = (iTest==0)? utok57FromSource: ((iTest==1)? utok57BinFromSource: utok57BinFromFile);;
                        if (utok57) {
                            // Do utok57 loop & bulk tests
                            status = U_ZERO_ERROR;
                            ubrk_setText(utok57, textResultsPtr->tokText, textLen, &status);
                            if (U_FAILURE(status)) {
                                log_err("FAIL: %s ubrk_setText utok57 %s (loop) status: %s\n", textResultsPtr->descrip, testType[iTest], u_errorName(status));
                            } else {
                                start = GET_START();
                                for (tokenP = tokens, flagsP = flags; tokenP < tokenLimit && urbtok57_tokenize(utok57, 1, tokenP, flagsP) == 1; tokenP++, flagsP++) {
                                    ;
                                }
                                numTokens = tokenP - tokens;
                                duration = GET_DURATION(start, info);

                                handleTokResults(textResultsPtr->descrip, "utok57", testType[iTest], "(loop)", duration,
                                            textResultsPtr->expTok57LoopLen, textResultsPtr->expTok57Loop, numTokens, tokens, flags);
                           }

                            status = U_ZERO_ERROR;
                            ubrk_setText(utok57, textResultsPtr->tokText, textLen, &status);
                            if (U_FAILURE(status)) {
                                log_err("FAIL: %s ubrk_setText utok57 %s (bulk) status: %s\n", textResultsPtr->descrip, testType[iTest], u_errorName(status));
                            } else {
                                start = GET_START();
                                numTokens = urbtok57_tokenize(utok57, kMaxTokens, tokens, flags);
                                duration = GET_DURATION(start, info);

                                handleTokResults(textResultsPtr->descrip, "utok57", testType[iTest], "(bulk)", duration,
                                            textResultsPtr->expTok57BulkLen, textResultsPtr->expTok57Bulk, numTokens, tokens, flags);
                            }
                        }
                    }
                }
            }
        }

        // Close UBreakIterators and rule memory, don't need to check for NULL
        ubrk_close(ubrkFromSource);
        ubrk_close(ubrkBinFromSource);
        ubrk_close(utokFromSource);
        ubrk_close(utokBinFromSource);
        ubrk_close(utokBinFromFile);
        ubrk_close(utok57FromSource);
        ubrk_close(utok57BinFromSource);
        ubrk_close(utok57BinFromFile);
        uprv_free(ubrkBinRules);
    }
}



#endif


#endif /* #if !UCONFIG_NO_BREAK_ITERATION */
