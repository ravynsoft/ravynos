// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/********************************************************************
 * COPYRIGHT:
 * Copyright (c) 1997-2016, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/********************************************************************************
*
* File CDATTST.C
*
* Modification History:
*        Name                     Description
*     Madhu Katragadda               Creation
*********************************************************************************
*/

/* C API TEST FOR DATE FORMAT */

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/uloc.h"
#include "unicode/udat.h"
#include "unicode/udatpg.h"
#include "unicode/ucal.h"
#include "unicode/unum.h"
#include "unicode/ustring.h"
#include "unicode/ufieldpositer.h"
#include "cintltst.h"
#include "cdattst.h"
#include "cformtst.h"
#include "cmemory.h"
#if !U_PLATFORM_HAS_WIN32_API
#include "unicode/uatimeunitformat.h" /* Apple-specific */
#endif

#include <math.h>

#define ADD_ALLOC_TEST 0
#define WRITE_HOUR_MISMATCH_ERRS 0

#if ADD_ALLOC_TEST
static void TestPerf(void);
#endif
static void TestExtremeDates(void);
static void TestAllLocales(void);
static void TestRelativeCrash(void);
static void TestContext(void);
static void TestCalendarDateParse(void);
static void TestParseErrorReturnValue(void);
static void TestFormatForFields(void);
static void TestForceGannenNumbering(void);
static void TestStandardPatterns(void);
static void TestApplyPatnOverridesTimeSep(void);
static void Test12HrFormats(void);
#if !U_PLATFORM_HAS_WIN32_API
static void TestTimeUnitFormat(void); /* Apple-specific */
static void TestTimeUnitFormatWithNumStyle(void); /* Apple-specific */
#endif
static void TestRemapPatternWithOpts(void); /* Apple-specific */
#if WRITE_HOUR_MISMATCH_ERRS
static void WriteHourMismatchErrs(void); /* 52980140*/
#endif

void addDateForTest(TestNode** root);

#define TESTCASE(x) addTest(root, &x, "tsformat/cdattst/" #x)

void addDateForTest(TestNode** root)
{
#if ADD_ALLOC_TEST
    TESTCASE(TestPerf);
#endif
    TESTCASE(TestDateFormat);
    TESTCASE(TestRelativeDateFormat);
    TESTCASE(TestSymbols);
    TESTCASE(TestDateFormatCalendar);
    TESTCASE(TestExtremeDates);
    TESTCASE(TestAllLocales);
    TESTCASE(TestRelativeCrash);
    TESTCASE(TestContext);
    TESTCASE(TestCalendarDateParse);
    TESTCASE(TestOverrideNumberFormat);
    TESTCASE(TestParseErrorReturnValue);
    TESTCASE(TestFormatForFields);
    TESTCASE(TestForceGannenNumbering);
    TESTCASE(TestStandardPatterns);
    TESTCASE(TestApplyPatnOverridesTimeSep);
    TESTCASE(Test12HrFormats);
#if !U_PLATFORM_HAS_WIN32_API
    TESTCASE(TestTimeUnitFormat); /* Apple-specific */
    TESTCASE(TestTimeUnitFormatWithNumStyle); /* Apple-specific */
#endif
    TESTCASE(TestRemapPatternWithOpts); /* Apple-specific */
#if WRITE_HOUR_MISMATCH_ERRS
    TESTCASE(WriteHourMismatchErrs); /* 52980140 */
#endif
}
/* Testing the DateFormat API */
static void TestDateFormat()
{
    UDateFormat *def, *fr, *it, *de, *def1, *fr_pat;
    UDateFormat *any;
    UDateFormat *copy;
    UErrorCode status = U_ZERO_ERROR;
    UChar* result = NULL;
    const UCalendar *cal;
    const UNumberFormat *numformat1, *numformat2;
    UNumberFormat *adoptNF;
    UChar temp[50];
    int32_t numlocales;
    UDate d1;
    int i;
    int32_t resultlength;
    int32_t resultlengthneeded;
    int32_t parsepos;
    UDate d = 837039928046.0;
    double num = -10456.37;
    /*const char* str="yyyy.MM.dd G 'at' hh:mm:ss z";
    const char t[]="2/3/76 2:50 AM";*/
    /*Testing udat_open() to open a dateformat */

    ctest_setTimeZone(NULL, &status);

    log_verbose("\nTesting udat_open() with various parameters\n");
    fr = udat_open(UDAT_FULL, UDAT_DEFAULT, "fr_FR", NULL,0, NULL, 0,&status);
    if(U_FAILURE(status))
    {
        log_data_err("FAIL: error in creating the dateformat using full time style with french locale -> %s (Are you missing data?)\n",
            myErrorName(status) );
        return;
    }
    /* this is supposed to open default date format, but later on it treats it like it is "en_US"
       - very bad if you try to run the tests on machine where default locale is NOT "en_US" */
    /* def = udat_open(UDAT_SHORT, UDAT_SHORT, NULL, NULL, 0, &status); */
    def = udat_open(UDAT_SHORT, UDAT_SHORT, "en_US", NULL, 0,NULL, 0, &status);
    if(U_FAILURE(status))
    {
        log_err("FAIL: error in creating the dateformat using short date and time style\n %s\n",
            myErrorName(status) );
        return;
    }
    it = udat_open(UDAT_DEFAULT, UDAT_MEDIUM, "it_IT", NULL, 0, NULL, 0,&status);
    if(U_FAILURE(status))
    {
        log_err("FAIL: error in creating the dateformat using medium date style with italian locale\n %s\n",
            myErrorName(status) );
        return;
    }
    de = udat_open(UDAT_LONG, UDAT_LONG, "de_DE", NULL, 0, NULL, 0,&status);
    if(U_FAILURE(status))
    {
        log_err("FAIL: error in creating the dateformat using long time and date styles with german locale\n %s\n",
            myErrorName(status));
        return;
    }
    /*creating a default dateformat */
    def1 = udat_open(UDAT_SHORT, UDAT_SHORT, NULL, NULL, 0,NULL, 0, &status);
    if(U_FAILURE(status))
    {
        log_err("FAIL: error in creating the dateformat using short date and time style\n %s\n",
            myErrorName(status) );
        return;
    }


    /*Testing udat_getAvailable() and udat_countAvailable()*/
    log_verbose("\nTesting getAvailableLocales and countAvailable()\n");
    numlocales=udat_countAvailable();
    /* use something sensible w/o hardcoding the count */
    if(numlocales < 0)
        log_data_err("FAIL: error in countAvailable\n");
    log_verbose("The number of locales for which date/time formatting patterns are available is %d\n", numlocales);

    for(i=0;i<numlocales;i++) {
      UErrorCode subStatus = U_ZERO_ERROR;
      log_verbose("Testing open of %s\n", udat_getAvailable(i));
      any = udat_open(UDAT_SHORT, UDAT_SHORT, udat_getAvailable(i), NULL ,0, NULL, 0, &subStatus);
      if(U_FAILURE(subStatus)) {
        log_data_err("FAIL: date format %s (getAvailable(%d)) is not instantiable: %s\n", udat_getAvailable(i), i, u_errorName(subStatus));
      }
      udat_close(any);
    }

    /*Testing udat_clone()*/
    log_verbose("\nTesting the udat_clone() function of date format\n");
    copy=udat_clone(def, &status);
    if(U_FAILURE(status)){
        log_err("Error in creating the clone using udat_clone: %s\n", myErrorName(status) );
    }
    /*if(def != copy)
        log_err("Error in udat_clone");*/ /*how should i check for equality???? */

    /*Testing udat_format()*/
    log_verbose("\nTesting the udat_format() function of date format\n");
    u_uastrcpy(temp, "7/10/96, 4:05 PM");
    /*format using def */
    resultlength=0;
    resultlengthneeded=udat_format(def, d, NULL, resultlength, NULL, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR)
    {
        status=U_ZERO_ERROR;
        resultlength=resultlengthneeded+1;
        if(result != NULL) {
            free(result);
            result = NULL;
        }
        result=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_format(def, d, result, resultlength, NULL, &status);
    }
    if(U_FAILURE(status) || !result)
    {
        log_err("FAIL: Error in formatting using udat_format(.....) %s\n", myErrorName(status) );
        return;
    }
    else
        log_verbose("PASS: formatting successful\n");
    if(u_strcmp(result, temp)==0)
        log_verbose("PASS: Date Format for US locale successful using udat_format()\n");
    else {
        char xbuf[2048];
        char gbuf[2048];
        u_austrcpy(xbuf, temp);
        u_austrcpy(gbuf, result);
        log_err("FAIL: Date Format for US locale failed using udat_format() - expected %s got %s\n", xbuf, gbuf);
    }
    /*format using fr */

    u_unescape("10 juil. 1996 \\u00E0 16:05:28 heure d\\u2019\\u00E9t\\u00E9 du Pacifique", temp, 50);
    if(result != NULL) {
        free(result);
        result = NULL;
    }
    result=myDateFormat(fr, d);
    if(u_strcmp(result, temp)==0)
        log_verbose("PASS: Date Format for french locale successful using udat_format()\n");
    else
        log_data_err("FAIL: Date Format for french locale failed using udat_format().\n" );

    /*format using it */
    u_uastrcpy(temp, "10 lug 1996, 16:05:28");

    {
        UChar *fmtted;
        char g[100];
        char x[100];

        fmtted = myDateFormat(it,d);
        u_austrcpy(g, fmtted);
        u_austrcpy(x, temp);
        if(u_strcmp(fmtted, temp)==0) {
            log_verbose("PASS: Date Format for italian locale successful uisng udat_format() - wanted %s, got %s\n", x, g);
        } else {
            log_data_err("FAIL: Date Format for italian locale failed using udat_format() - wanted %s, got %s\n", x, g);
        }
    }

    /*Testing parsing using udat_parse()*/
    log_verbose("\nTesting parsing using udat_parse()\n");
    u_uastrcpy(temp,"2/3/76, 2:50 AM");
    parsepos=0;
    status=U_ZERO_ERROR;

    d1=udat_parse(def, temp, u_strlen(temp), &parsepos, &status);
    if(U_FAILURE(status))
    {
        log_err("FAIL: Error in parsing using udat_parse(.....) %s\n", myErrorName(status) );
    }
    else
        log_verbose("PASS: parsing succesful\n");
    /*format it back and check for equality */


    if(u_strcmp(myDateFormat(def, d1),temp)!=0)
        log_err("FAIL: error in parsing\n");

    /*Testing parsing using udat_parse()*/
    log_verbose("\nTesting parsing using udat_parse()\n");
    u_uastrcpy(temp,"2/Don't parse this part");
    status=U_ZERO_ERROR;

    d1=udat_parse(def, temp, u_strlen(temp), NULL, &status);
    if(status != U_PARSE_ERROR)
    {
        log_err("FAIL: udat_parse(\"bad string\") passed when it should have failed\n");
    }
    else
        log_verbose("PASS: parsing succesful\n");



    /*Testing udat_openPattern()  */
    status=U_ZERO_ERROR;
    log_verbose("\nTesting the udat_openPattern with a specified pattern\n");
    /*for french locale */
    fr_pat=udat_open(UDAT_PATTERN, UDAT_PATTERN,"fr_FR",NULL,0,temp, u_strlen(temp), &status);
    if(U_FAILURE(status))
    {
        log_err("FAIL: Error in creating a date format using udat_openPattern \n %s\n",
            myErrorName(status) );
    }
    else
        log_verbose("PASS: creating dateformat using udat_openPattern() succesful\n");


        /*Testing applyPattern and toPattern */
    log_verbose("\nTesting applyPattern and toPattern()\n");
    udat_applyPattern(def1, FALSE, temp, u_strlen(temp));
    log_verbose("Extracting the pattern\n");

    resultlength=0;
    resultlengthneeded=udat_toPattern(def1, FALSE, NULL, resultlength, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR)
    {
        status=U_ZERO_ERROR;
        resultlength=resultlengthneeded + 1;
        result=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_toPattern(def1, FALSE, result, resultlength, &status);
    }
    if(U_FAILURE(status))
    {
        log_err("FAIL: error in extracting the pattern from UNumberFormat\n %s\n",
            myErrorName(status) );
    }
    if(u_strcmp(result, temp)!=0)
        log_err("FAIL: Error in extracting the pattern\n");
    else
        log_verbose("PASS: applyPattern and toPattern work fine\n");

    if(result != NULL) {
        free(result);
        result = NULL;
    }


    /*Testing getter and setter functions*/
    /*isLenient and setLenient()*/
    log_verbose("\nTesting the isLenient and setLenient properties\n");
    udat_setLenient(fr, udat_isLenient(it));
    if(udat_isLenient(fr) != udat_isLenient(it))
        log_err("ERROR: setLenient() failed\n");
    else
        log_verbose("PASS: setLenient() successful\n");


    /*Test get2DigitYearStart set2DigitYearStart */
    log_verbose("\nTesting the get and set 2DigitYearStart properties\n");
    d1= udat_get2DigitYearStart(fr_pat,&status);
    if(U_FAILURE(status)) {
            log_err("ERROR: udat_get2DigitYearStart failed %s\n", myErrorName(status) );
    }
    status = U_ZERO_ERROR;
    udat_set2DigitYearStart(def1 ,d1, &status);
    if(U_FAILURE(status)) {
        log_err("ERROR: udat_set2DigitYearStart failed %s\n", myErrorName(status) );
    }
    if(udat_get2DigitYearStart(fr_pat, &status) != udat_get2DigitYearStart(def1, &status))
        log_err("FAIL: error in set2DigitYearStart\n");
    else
        log_verbose("PASS: set2DigitYearStart successful\n");
    /*try setting it to another value */
    udat_set2DigitYearStart(de, 2000.0, &status);
    if(U_FAILURE(status)){
        log_verbose("ERROR: udat_set2DigitYearStart failed %s\n", myErrorName(status) );
    }
    if(udat_get2DigitYearStart(de, &status) != 2000)
        log_err("FAIL: error in set2DigitYearStart\n");
    else
        log_verbose("PASS: set2DigitYearStart successful\n");



    /*Test getNumberFormat() and setNumberFormat() */
    log_verbose("\nTesting the get and set NumberFormat properties of date format\n");
    numformat1=udat_getNumberFormat(fr_pat);
    udat_setNumberFormat(def1, numformat1);
    numformat2=udat_getNumberFormat(def1);
    if(u_strcmp(myNumformat(numformat1, num), myNumformat(numformat2, num)) !=0)
        log_err("FAIL: error in setNumberFormat or getNumberFormat()\n");
    else
        log_verbose("PASS:setNumberFormat and getNumberFormat succesful\n");

    /*Test getNumberFormat() and adoptNumberFormat() */
    log_verbose("\nTesting the get and adopt NumberFormat properties of date format\n");
    adoptNF= unum_open(UNUM_DEFAULT, NULL, 0, NULL, NULL, &status);
    udat_adoptNumberFormat(def1, adoptNF);
    numformat2=udat_getNumberFormat(def1);
    if(u_strcmp(myNumformat(adoptNF, num), myNumformat(numformat2, num)) !=0)
        log_err("FAIL: error in adoptNumberFormat or getNumberFormat()\n");
    else
        log_verbose("PASS:adoptNumberFormat and getNumberFormat succesful\n");

    /*try setting the number format to another format */
    numformat1=udat_getNumberFormat(def);
    udat_setNumberFormat(def1, numformat1);
    numformat2=udat_getNumberFormat(def1);
    if(u_strcmp(myNumformat(numformat1, num), myNumformat(numformat2, num)) !=0)
        log_err("FAIL: error in setNumberFormat or getNumberFormat()\n");
    else
        log_verbose("PASS: setNumberFormat and getNumberFormat succesful\n");



    /*Test getCalendar and setCalendar*/
    log_verbose("\nTesting the udat_getCalendar() and udat_setCalendar() properties\n");
    cal=udat_getCalendar(fr_pat);


    udat_setCalendar(def1, cal);
    if(!ucal_equivalentTo(udat_getCalendar(fr_pat), udat_getCalendar(def1)))
        log_err("FAIL: Error in setting and getting the calendar\n");
    else
        log_verbose("PASS: getting and setting calendar successful\n");

    if(result!=NULL) {
        free(result);
    }

    /*Closing the UDateForamt */
    udat_close(def);
    udat_close(fr);
    udat_close(it);
    udat_close(de);
    udat_close(def1);
    udat_close(fr_pat);
    udat_close(copy);

    ctest_resetTimeZone();
}

/*
Test combined relative date formatting (relative date + non-relative time).
This is a bit tricky since we can't have static test data for comparison, the
relative date formatting is relative to the time the tests are run. We generate
the data for comparison dynamically. However, the tests could fail if they are
run right at midnight Pacific time and the call to ucal_getNow() is before midnight
while the calls to udat_format are after midnight or span midnight.
*/
static const UDate dayInterval = 24.0*60.0*60.0*1000.0;
static const UChar trdfZone[] = { 0x0055, 0x0053, 0x002F, 0x0050, 0x0061, 0x0063, 0x0069, 0x0066, 0x0069, 0x0063, 0 }; /* US/Pacific */
static const char trdfLocale[] = "en_US";
static const UChar minutesPatn[] = { 0x006D, 0x006D, 0 }; /* "mm" */
static const UChar monthLongPatn[] = { 0x004D, 0x004D, 0x004D, 0x004D, 0 }; /* "MMMM" */
static const UChar monthMediumPatn[] = { 0x004D, 0x004D, 0x004D, 0 }; /* "MMM" */
static const UChar monthShortPatn[] = { 0x004D, 0 }; /* "M" */
static const UDateFormatStyle dateStylesList[] = { UDAT_FULL, UDAT_LONG, UDAT_MEDIUM, UDAT_SHORT, UDAT_NONE };
static const UChar *monthPatnsList[] = { monthLongPatn, monthLongPatn, monthMediumPatn, monthShortPatn, NULL };
static const UChar newTimePatn[] = { 0x0048, 0x0048, 0x002C, 0x006D, 0x006D, 0 }; /* "HH,mm" */
static const UChar minutesStr[] = { 0x0034, 0x0039, 0 }; /* "49", minutes string to search for in output */
enum { kDateOrTimeOutMax = 96, kDateAndTimeOutMax = 192 };

static const UDate minutesTolerance = 2 * 60.0 * 1000.0;
static const UDate daysTolerance = 2 * 24.0 * 60.0 * 60.0 * 1000.0;

static void TestRelativeDateFormat()
{
    UDate today = 0.0;
    const UDateFormatStyle * stylePtr;
    const UChar ** monthPtnPtr;
    UErrorCode status = U_ZERO_ERROR;
    UCalendar * ucal = ucal_open(trdfZone, -1, trdfLocale, UCAL_GREGORIAN, &status);
    if ( U_SUCCESS(status) ) {
        int32_t    year, month, day;
        ucal_setMillis(ucal, ucal_getNow(), &status);
        year = ucal_get(ucal, UCAL_YEAR, &status);
        month = ucal_get(ucal, UCAL_MONTH, &status);
        day = ucal_get(ucal, UCAL_DATE, &status);
        ucal_setDateTime(ucal, year, month, day, 18, 49, 0, &status); /* set to today at 18:49:00 */
        today = ucal_getMillis(ucal, &status);
        ucal_close(ucal);
    }
    if ( U_FAILURE(status) || today == 0.0 ) {
        log_data_err("Generate UDate for a specified time today fails, error %s - (Are you missing data?)\n", myErrorName(status) );
        return;
    }
    for (stylePtr = dateStylesList, monthPtnPtr = monthPatnsList; *stylePtr != UDAT_NONE; ++stylePtr, ++monthPtnPtr) {
        UDateFormat* fmtRelDateTime;
        UDateFormat* fmtRelDate;
        UDateFormat* fmtTime;
        int32_t dayOffset, limit;
        UFieldPosition fp;
        UChar   strDateTime[kDateAndTimeOutMax];
        UChar   strDate[kDateOrTimeOutMax];
        UChar   strTime[kDateOrTimeOutMax];
        UChar * strPtr;
        int32_t dtpatLen;

        fmtRelDateTime = udat_open(UDAT_SHORT, *stylePtr | UDAT_RELATIVE, trdfLocale, trdfZone, -1, NULL, 0, &status);
        if ( U_FAILURE(status) ) {
            log_data_err("udat_open timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) fails, error %s (Are you missing data?)\n", *stylePtr, myErrorName(status) );
            continue;
        }
        fmtRelDate = udat_open(UDAT_NONE, *stylePtr | UDAT_RELATIVE, trdfLocale, trdfZone, -1, NULL, 0, &status);
        if ( U_FAILURE(status) ) {
            log_err("udat_open timeStyle NONE dateStyle (%d | UDAT_RELATIVE) fails, error %s\n", *stylePtr, myErrorName(status) );
            udat_close(fmtRelDateTime);
            continue;
        }
        fmtTime = udat_open(UDAT_SHORT, UDAT_NONE, trdfLocale, trdfZone, -1, NULL, 0, &status);
        if ( U_FAILURE(status) ) {
            log_err("udat_open timeStyle SHORT dateStyle NONE fails, error %s\n", myErrorName(status) );
            udat_close(fmtRelDateTime);
            udat_close(fmtRelDate);
            continue;
        }

        dtpatLen = udat_toPatternRelativeDate(fmtRelDateTime, strDate, kDateAndTimeOutMax, &status);
        if ( U_FAILURE(status) ) {
            log_err("udat_toPatternRelativeDate timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) fails, error %s\n", *stylePtr, myErrorName(status) );
            status = U_ZERO_ERROR;
        } else if ( u_strstr(strDate, *monthPtnPtr) == NULL || dtpatLen != u_strlen(strDate) ) {
            log_err("udat_toPatternRelativeDate timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) date pattern incorrect\n", *stylePtr );
        }
        dtpatLen = udat_toPatternRelativeTime(fmtRelDateTime, strTime, kDateAndTimeOutMax, &status);
        if ( U_FAILURE(status) ) {
            log_err("udat_toPatternRelativeTime timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) fails, error %s\n", *stylePtr, myErrorName(status) );
            status = U_ZERO_ERROR;
        } else if ( u_strstr(strTime, minutesPatn) == NULL || dtpatLen != u_strlen(strTime) ) {
            log_err("udat_toPatternRelativeTime timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) time pattern incorrect\n", *stylePtr );
        }
        dtpatLen = udat_toPattern(fmtRelDateTime, FALSE, strDateTime, kDateAndTimeOutMax, &status);
        if ( U_FAILURE(status) ) {
            log_err("udat_toPattern timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) fails, error %s\n", *stylePtr, myErrorName(status) );
            status = U_ZERO_ERROR;
        } else if ( u_strstr(strDateTime, strDate) == NULL || u_strstr(strDateTime, strTime) == NULL || dtpatLen != u_strlen(strDateTime) ) {
            log_err("udat_toPattern timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) dateTime pattern incorrect\n", *stylePtr );
        }
        udat_applyPatternRelative(fmtRelDateTime, strDate, u_strlen(strDate), newTimePatn, u_strlen(newTimePatn), &status);
        if ( U_FAILURE(status) ) {
            log_err("udat_applyPatternRelative timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) fails, error %s\n", *stylePtr, myErrorName(status) );
            status = U_ZERO_ERROR;
        } else {
            udat_toPattern(fmtRelDateTime, FALSE, strDateTime, kDateAndTimeOutMax, &status);
            if ( U_FAILURE(status) ) {
                log_err("udat_toPattern timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) fails, error %s\n", *stylePtr, myErrorName(status) );
                status = U_ZERO_ERROR;
            } else if ( u_strstr(strDateTime, newTimePatn) == NULL ) {
                log_err("udat_applyPatternRelative timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) didn't update time pattern\n", *stylePtr );
            }
        }
        udat_applyPatternRelative(fmtRelDateTime, strDate, u_strlen(strDate), strTime, u_strlen(strTime), &status); /* restore original */

        fp.field = UDAT_MINUTE_FIELD;
        for (dayOffset = -2, limit = 2; dayOffset <= limit; ++dayOffset) {
            UDate   dateToUse = today + (float)dayOffset*dayInterval;

            udat_format(fmtRelDateTime, dateToUse, strDateTime, kDateAndTimeOutMax, &fp, &status);
            if ( U_FAILURE(status) ) {
                log_err("udat_format timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) fails, error %s\n", *stylePtr, myErrorName(status) );
                status = U_ZERO_ERROR;
            } else {
                int32_t parsePos = 0;
                UDate dateResult = udat_parse(fmtRelDateTime, strDateTime, -1, &parsePos, &status);
                UDate dateDiff =  (dateResult >= dateToUse)? dateResult - dateToUse: dateToUse - dateResult;
                if ( U_FAILURE(status) || dateDiff > minutesTolerance ) {
                    log_err("udat_parse timeStyle SHORT dateStyle (%d | UDAT_RELATIVE) fails, error %s, expect approx %.1f, got %.1f, parsePos %d\n",
                            *stylePtr, myErrorName(status), dateToUse, dateResult, parsePos );
                    status = U_ZERO_ERROR;
                }

                udat_format(fmtRelDate, dateToUse, strDate, kDateOrTimeOutMax, NULL, &status);
                if ( U_FAILURE(status) ) {
                    log_err("udat_format timeStyle NONE dateStyle (%d | UDAT_RELATIVE) fails, error %s\n", *stylePtr, myErrorName(status) );
                    status = U_ZERO_ERROR;
                } else if ( u_strstr(strDateTime, strDate) == NULL ) {
                    log_err("relative date string not found in udat_format timeStyle SHORT dateStyle (%d | UDAT_RELATIVE)\n", *stylePtr );
                } else {
                    parsePos = 0;
                    dateResult = udat_parse(fmtRelDate, strDate, -1, &parsePos, &status);
                    dateDiff =  (dateResult >= dateToUse)? dateResult - dateToUse: dateToUse - dateResult;
                    if ( U_FAILURE(status) || dateDiff > daysTolerance ) {
                        log_err("udat_parse timeStyle NONE dateStyle (%d | UDAT_RELATIVE) fails, error %s, expect approx %.1f, got %.1f, parsePos %d\n",
                                *stylePtr, myErrorName(status), dateToUse, dateResult, parsePos );
                        status = U_ZERO_ERROR;
                    }
                }

                udat_format(fmtTime, dateToUse, strTime, kDateOrTimeOutMax, NULL, &status);
                if ( U_FAILURE(status) ) {
                    log_err("udat_format timeStyle SHORT dateStyle NONE fails, error %s\n", myErrorName(status) );
                    status = U_ZERO_ERROR;
                } else if ( u_strstr(strDateTime, strTime) == NULL ) {
                    log_err("time string not found in udat_format timeStyle SHORT dateStyle (%d | UDAT_RELATIVE)\n", *stylePtr );
                }

                strPtr = u_strstr(strDateTime, minutesStr);
                if ( strPtr != NULL ) {
                    int32_t beginIndex = (int32_t)(strPtr - strDateTime);
                    if ( fp.beginIndex != beginIndex ) {
                        log_err("UFieldPosition beginIndex %d, expected %d, in udat_format timeStyle SHORT dateStyle (%d | UDAT_RELATIVE)\n", fp.beginIndex, beginIndex, *stylePtr );
                    }
                } else {
                    log_err("minutes string not found in udat_format timeStyle SHORT dateStyle (%d | UDAT_RELATIVE)\n", *stylePtr );
                }
            }
        }

        udat_close(fmtRelDateTime);
        udat_close(fmtRelDate);
        udat_close(fmtTime);
     }
}

/*Testing udat_getSymbols() and udat_setSymbols() and udat_countSymbols()*/
static void TestSymbols()
{
    UDateFormat *def, *fr, *zhChiCal, *esMX;
    UErrorCode status = U_ZERO_ERROR;
    UChar *value=NULL;
    UChar *result = NULL;
    int32_t resultlength;
    int32_t resultlengthout;
    UChar *pattern;


    /*creating a dateformat with french locale */
    log_verbose("\ncreating a date format with french locale\n");
    fr = udat_open(UDAT_FULL, UDAT_DEFAULT, "fr_FR", NULL, 0, NULL, 0, &status);
    if(U_FAILURE(status))
    {
        log_data_err("error in creating the dateformat using full time style with french locale -> %s (Are you missing data?)\n",
            myErrorName(status) );
        return;
    }
    /*creating a default dateformat */
    log_verbose("\ncreating a date format with default locale\n");
    /* this is supposed to open default date format, but later on it treats it like it is "en_US"
       - very bad if you try to run the tests on machine where default locale is NOT "en_US" */
    /* def = udat_open(UDAT_DEFAULT,UDAT_DEFAULT ,NULL, NULL, 0, &status); */
    def = udat_open(UDAT_DEFAULT,UDAT_DEFAULT ,"en_US", NULL, 0, NULL, 0, &status);
    if(U_FAILURE(status))
    {
        log_err("error in creating the dateformat using short date and time style\n %s\n",
            myErrorName(status) );
        return;
    }
    /*creating a dateformat with zh locale */
    log_verbose("\ncreating a date format with zh locale for chinese calendar\n");
    zhChiCal = udat_open(UDAT_NONE, UDAT_FULL, "zh@calendar=chinese", NULL, 0, NULL, 0, &status);
    if(U_FAILURE(status))
    {
        log_data_err("error in creating the dateformat using full date, no time, locale zh@calendar=chinese -> %s (Are you missing data?)\n",
            myErrorName(status) );
        return;
    }
    /*creating a dateformat with es_MX locale */
    log_verbose("\ncreating a date format with es_MX locale\n");
    esMX = udat_open(UDAT_SHORT, UDAT_NONE, "es_MX", NULL, 0, NULL, 0, &status);
    if(U_FAILURE(status))
    {
        log_data_err("error in creating the dateformat using no date, short time, locale es_MX -> %s (Are you missing data?)\n",
            myErrorName(status) );
        return;
    }


    /*Testing countSymbols, getSymbols and setSymbols*/
    log_verbose("\nTesting countSymbols\n");
    /*since the month names has the last string empty and week names are 1 based 1.e first string in the weeknames array is empty */
    if(udat_countSymbols(def, UDAT_ERAS)!=2 || udat_countSymbols(def, UDAT_MONTHS)!=12 ||
        udat_countSymbols(def, UDAT_SHORT_MONTHS)!=12 || udat_countSymbols(def, UDAT_WEEKDAYS)!=8 ||
        udat_countSymbols(def, UDAT_SHORT_WEEKDAYS)!=8 || udat_countSymbols(def, UDAT_AM_PMS)!=2 ||
        udat_countSymbols(def, UDAT_QUARTERS) != 4 || udat_countSymbols(def, UDAT_SHORT_QUARTERS) != 4 ||
        udat_countSymbols(def, UDAT_LOCALIZED_CHARS)!=1 || udat_countSymbols(def, UDAT_SHORTER_WEEKDAYS)!=8 ||
        udat_countSymbols(zhChiCal, UDAT_CYCLIC_YEARS_NARROW)!=60 || udat_countSymbols(zhChiCal, UDAT_ZODIAC_NAMES_NARROW)!=12)
    {
        log_err("FAIL: error in udat_countSymbols\n");
    }
    else
        log_verbose("PASS: udat_countSymbols() successful\n");

    /*testing getSymbols*/
    log_verbose("\nTesting getSymbols\n");
    pattern=(UChar*)malloc(sizeof(UChar) * 10);
    u_uastrcpy(pattern, "jeudi");
    resultlength=0;
    resultlengthout=udat_getSymbols(fr, UDAT_WEEKDAYS, 5 , NULL, resultlength, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR)
    {
        status=U_ZERO_ERROR;
        resultlength=resultlengthout+1;
        if(result != NULL) {
            free(result);
            result = NULL;
        }
        result=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_getSymbols(fr, UDAT_WEEKDAYS, 5, result, resultlength, &status);

    }
    if(U_FAILURE(status))
    {
        log_err("FAIL: Error in udat_getSymbols().... %s\n", myErrorName(status) );
    }
    else
        log_verbose("PASS: getSymbols succesful\n");

    if(u_strcmp(result, pattern)==0)
        log_verbose("PASS: getSymbols retrieved the right value\n");
    else
        log_data_err("FAIL: getSymbols retrieved the wrong value\n");

    /*run series of tests to test getsymbols regressively*/
    log_verbose("\nTesting getSymbols() regressively\n");
    VerifygetSymbols(fr, UDAT_WEEKDAYS, 1, "dimanche");
    VerifygetSymbols(def, UDAT_WEEKDAYS, 1, "Sunday");
    VerifygetSymbols(fr, UDAT_SHORT_WEEKDAYS, 7, "sam.");
    VerifygetSymbols(fr, UDAT_SHORTER_WEEKDAYS, 7, "sa");
    VerifygetSymbols(def, UDAT_SHORT_WEEKDAYS, 7, "Sat");
    VerifygetSymbols(def, UDAT_MONTHS, 11, "December");
    VerifygetSymbols(def, UDAT_MONTHS, 0, "January");
    VerifygetSymbols(fr, UDAT_ERAS, 0, "av. J.-C.");
    VerifygetSymbols(def, UDAT_AM_PMS, 0, "AM");
    VerifygetSymbols(def, UDAT_AM_PMS, 1, "PM");
    VerifygetSymbols(fr, UDAT_SHORT_MONTHS, 0, "janv.");
    VerifygetSymbols(def, UDAT_SHORT_MONTHS, 11, "Dec");
    VerifygetSymbols(fr, UDAT_QUARTERS, 0, "1er trimestre");
    VerifygetSymbols(def, UDAT_QUARTERS, 3, "4th quarter");
    VerifygetSymbols(fr, UDAT_SHORT_QUARTERS, 1, "T2");
    VerifygetSymbols(def, UDAT_SHORT_QUARTERS, 2, "Q3");
    VerifygetSymbols(zhChiCal, UDAT_CYCLIC_YEARS_ABBREVIATED, 0, "\\u7532\\u5B50");
    VerifygetSymbols(zhChiCal, UDAT_CYCLIC_YEARS_NARROW, 59, "\\u7678\\u4EA5");
    VerifygetSymbols(zhChiCal, UDAT_ZODIAC_NAMES_ABBREVIATED, 0, "\\u9F20");
    VerifygetSymbols(zhChiCal, UDAT_ZODIAC_NAMES_WIDE, 11, "\\u732A");
    VerifygetSymbols(esMX, UDAT_AM_PMS, 0, "a.m."); // see <rdar://problem/52923924>
    VerifygetSymbols(esMX, UDAT_AM_PMS, 1, "p.m."); // see <rdar://problem/52923924>
#if UDAT_HAS_PATTERN_CHAR_FOR_TIME_SEPARATOR
    VerifygetSymbols(def,UDAT_LOCALIZED_CHARS, 0, "GyMdkHmsSEDFwWahKzYeugAZvcLQqVUOXxrbB:");
#else
    VerifygetSymbols(def,UDAT_LOCALIZED_CHARS, 0, "GyMdkHmsSEDFwWahKzYeugAZvcLQqVUOXxrbB");
#endif


    if(result != NULL) {
        free(result);
        result = NULL;
    }
free(pattern);

    log_verbose("\nTesting setSymbols\n");
    /*applying the pattern so that setSymbolss works */
    resultlength=0;
    resultlengthout=udat_toPattern(fr, FALSE, NULL, resultlength, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR)
    {
        status=U_ZERO_ERROR;
        resultlength=resultlengthout + 1;
        pattern=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_toPattern(fr, FALSE, pattern, resultlength, &status);
    }
    if(U_FAILURE(status))
    {
        log_err("FAIL: error in extracting the pattern from UNumberFormat\n %s\n",
            myErrorName(status) );
    }

    udat_applyPattern(def, FALSE, pattern, u_strlen(pattern));
    resultlength=0;
    resultlengthout=udat_toPattern(def, FALSE, NULL, resultlength,&status);
    if(status==U_BUFFER_OVERFLOW_ERROR)
    {
        status=U_ZERO_ERROR;
        resultlength=resultlengthout + 1;
        if(result != NULL) {
            free(result);
            result = NULL;
        }
        result=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_toPattern(fr, FALSE,result, resultlength, &status);
    }
    if(U_FAILURE(status))
    {
        log_err("FAIL: error in extracting the pattern from UNumberFormat\n %s\n",
            myErrorName(status) );
    }
    if(u_strcmp(result, pattern)==0)
        log_verbose("Pattern applied properly\n");
    else
        log_err("pattern could not be applied properly\n");

free(pattern);
    /*testing set symbols */
    resultlength=0;
    resultlengthout=udat_getSymbols(fr, UDAT_MONTHS, 11 , NULL, resultlength, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR){
        status=U_ZERO_ERROR;
        resultlength=resultlengthout+1;
        if(result != NULL) {
            free(result);
            result = NULL;
        }
        result=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_getSymbols(fr, UDAT_MONTHS, 11, result, resultlength, &status);

    }
    if(U_FAILURE(status))
        log_err("FAIL: error in getSymbols() %s\n", myErrorName(status) );
    resultlength=resultlengthout+1;

    udat_setSymbols(def, UDAT_MONTHS, 11, result, resultlength, &status);
    if(U_FAILURE(status))
        {
            log_err("FAIL: Error in udat_setSymbols() : %s\n", myErrorName(status) );
        }
    else
        log_verbose("PASS: SetSymbols successful\n");

    resultlength=0;
    resultlengthout=udat_getSymbols(def, UDAT_MONTHS, 11, NULL, resultlength, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR){
        status=U_ZERO_ERROR;
        resultlength=resultlengthout+1;
        value=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_getSymbols(def, UDAT_MONTHS, 11, value, resultlength, &status);
    }
    if(U_FAILURE(status))
        log_err("FAIL: error in retrieving the value using getSymbols i.e roundtrip\n");

    if(u_strcmp(result, value)!=0)
        log_data_err("FAIL: Error in settting and getting symbols\n");
    else
        log_verbose("PASS: setSymbols successful\n");


    /*run series of tests to test setSymbols regressively*/
    log_verbose("\nTesting setSymbols regressively\n");
    VerifysetSymbols(def, UDAT_ERAS, 0, "BeforeChrist");
    VerifysetSymbols(def, UDAT_ERA_NAMES, 1, "AnnoDomini");
    VerifysetSymbols(def, UDAT_WEEKDAYS, 1, "Sundayweek");
    VerifysetSymbols(def, UDAT_SHORT_WEEKDAYS, 7, "Satweek");
    VerifysetSymbols(def, UDAT_NARROW_WEEKDAYS, 4, "M");
    VerifysetSymbols(def, UDAT_STANDALONE_WEEKDAYS, 1, "Sonntagweek");
    VerifysetSymbols(def, UDAT_STANDALONE_SHORT_WEEKDAYS, 7, "Sams");
    VerifysetSymbols(def, UDAT_STANDALONE_NARROW_WEEKDAYS, 4, "V");
    VerifysetSymbols(fr, UDAT_MONTHS, 11, "december");
    VerifysetSymbols(fr, UDAT_SHORT_MONTHS, 0, "Jan");
    VerifysetSymbols(fr, UDAT_NARROW_MONTHS, 1, "R");
    VerifysetSymbols(fr, UDAT_STANDALONE_MONTHS, 11, "dezember");
    VerifysetSymbols(fr, UDAT_STANDALONE_SHORT_MONTHS, 7, "Aug");
    VerifysetSymbols(fr, UDAT_STANDALONE_NARROW_MONTHS, 2, "M");
    VerifysetSymbols(fr, UDAT_QUARTERS, 0, "1. Quart");
    VerifysetSymbols(fr, UDAT_SHORT_QUARTERS, 1, "QQ2");
    VerifysetSymbols(fr, UDAT_STANDALONE_QUARTERS, 2, "3rd Quar.");
    VerifysetSymbols(fr, UDAT_STANDALONE_SHORT_QUARTERS, 3, "4QQ");
    VerifysetSymbols(zhChiCal, UDAT_CYCLIC_YEARS_ABBREVIATED, 1, "yi-chou");
    VerifysetSymbols(zhChiCal, UDAT_ZODIAC_NAMES_ABBREVIATED, 1, "Ox");


    /*run series of tests to test get and setSymbols regressively*/
    log_verbose("\nTesting get and set symbols regressively\n");
    VerifygetsetSymbols(fr, def, UDAT_WEEKDAYS, 1);
    VerifygetsetSymbols(fr, def, UDAT_WEEKDAYS, 7);
    VerifygetsetSymbols(fr, def, UDAT_SHORT_WEEKDAYS, 1);
    VerifygetsetSymbols(fr, def, UDAT_SHORT_WEEKDAYS, 7);
    VerifygetsetSymbols(fr, def, UDAT_MONTHS, 0);
    VerifygetsetSymbols(fr, def, UDAT_SHORT_MONTHS, 0);
    VerifygetsetSymbols(fr, def, UDAT_ERAS,1);
    VerifygetsetSymbols(fr, def, UDAT_LOCALIZED_CHARS, 0);
    VerifygetsetSymbols(fr, def, UDAT_AM_PMS, 1);


    /*closing*/

    udat_close(fr);
    udat_close(def);
    udat_close(zhChiCal);
    udat_close(esMX);
    if(result != NULL) {
        free(result);
        result = NULL;
    }
    free(value);

}

/**
 * Test DateFormat(Calendar) API
 */
static void TestDateFormatCalendar() {
    UDateFormat *date=0, *time=0, *full=0;
    UCalendar *cal=0;
    UChar buf[256];
    char cbuf[256];
    int32_t pos;
    UDate when;
    UErrorCode ec = U_ZERO_ERROR;
    UChar buf1[256];
    int32_t len1;
    const char *expected;
    UChar uExpected[32];

    ctest_setTimeZone(NULL, &ec);

    /* Create a formatter for date fields. */
    date = udat_open(UDAT_NONE, UDAT_SHORT, "en_US", NULL, 0, NULL, 0, &ec);
    if (U_FAILURE(ec)) {
        log_data_err("FAIL: udat_open(NONE, SHORT, en_US) failed with %s (Are you missing data?)\n",
                u_errorName(ec));
        goto FAIL;
    }

    /* Create a formatter for time fields. */
    time = udat_open(UDAT_SHORT, UDAT_NONE, "en_US", NULL, 0, NULL, 0, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: udat_open(SHORT, NONE, en_US) failed with %s\n",
                u_errorName(ec));
        goto FAIL;
    }

    /* Create a full format for output */
    full = udat_open(UDAT_FULL, UDAT_FULL, "en_US", NULL, 0, NULL, 0, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: udat_open(FULL, FULL, en_US) failed with %s\n",
                u_errorName(ec));
        goto FAIL;
    }

    /* Create a calendar */
    cal = ucal_open(NULL, 0, "en_US", UCAL_GREGORIAN, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: ucal_open(en_US) failed with %s\n",
                u_errorName(ec));
        goto FAIL;
    }

    /* Parse the date */
    ucal_clear(cal);
    u_uastrcpy(buf, "4/5/2001");
    pos = 0;
    udat_parseCalendar(date, cal, buf, -1, &pos, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: udat_parseCalendar(4/5/2001) failed at %d with %s\n",
                pos, u_errorName(ec));
        goto FAIL;
    }

    /* Check if formatCalendar matches the original date */
    len1 = udat_formatCalendar(date, cal, buf1, UPRV_LENGTHOF(buf1), NULL, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: udat_formatCalendar(4/5/2001) failed with %s\n",
                u_errorName(ec));
        goto FAIL;
    }
    expected = "4/5/01";
    u_uastrcpy(uExpected, expected);
    if (u_strlen(uExpected) != len1 || u_strncmp(uExpected, buf1, len1) != 0) {
        log_err("FAIL: udat_formatCalendar(4/5/2001), expected: %s", expected);
    }

    /* Parse the time */
    u_uastrcpy(buf, "5:45 PM");
    pos = 0;
    udat_parseCalendar(time, cal, buf, -1, &pos, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: udat_parseCalendar(17:45) failed at %d with %s\n",
                pos, u_errorName(ec));
        goto FAIL;
    }

    /* Check if formatCalendar matches the original time */
    len1 = udat_formatCalendar(time, cal, buf1, UPRV_LENGTHOF(buf1), NULL, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: udat_formatCalendar(17:45) failed with %s\n",
                u_errorName(ec));
        goto FAIL;
    }
    expected = "5:45 PM";
    u_uastrcpy(uExpected, expected);
    if (u_strlen(uExpected) != len1 || u_strncmp(uExpected, buf1, len1) != 0) {
        log_err("FAIL: udat_formatCalendar(17:45), expected: %s", expected);
    }

    /* Check result */
    when = ucal_getMillis(cal, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: ucal_getMillis() failed with %s\n", u_errorName(ec));
        goto FAIL;
    }
    udat_format(full, when, buf, sizeof(buf), NULL, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: udat_format() failed with %s\n", u_errorName(ec));
        goto FAIL;
    }
    u_austrcpy(cbuf, buf);
    /* Thursday, April 5, 2001 5:45:00 PM PDT 986517900000 */
    if (when == 986517900000.0) {
        log_verbose("Ok: Parsed result: %s\n", cbuf);
    } else {
        log_err("FAIL: Parsed result: %s, exp 4/5/2001 5:45 PM\n", cbuf);
    }

 FAIL:
    udat_close(date);
    udat_close(time);
    udat_close(full);
    ucal_close(cal);

    ctest_resetTimeZone();
}



/**
 * Test parsing two digit year against "YY" vs. "YYYY" patterns
 */
static void TestCalendarDateParse() {

    int32_t result;
    UErrorCode ec = U_ZERO_ERROR;
    UDateFormat* simpleDateFormat = 0;
    int32_t parsePos = 0;
    int32_t twoDigitCenturyStart = 75;
    int32_t currentTwoDigitYear = 0;
    int32_t startCentury = 0;
    UCalendar* tempCal = 0;
    UCalendar* calendar = 0;

    U_STRING_DECL(pattern, "yyyy", 4);
    U_STRING_DECL(pattern2, "yy", 2);
    U_STRING_DECL(text, "75", 2);

    U_STRING_INIT(pattern, "yyyy", 4);
    U_STRING_INIT(pattern2, "yy", 2);
    U_STRING_INIT(text, "75", 2);

    simpleDateFormat = udat_open(UDAT_FULL, UDAT_FULL, "en-GB", 0, 0, 0, 0, &ec);
    if (U_FAILURE(ec)) {
        log_data_err("udat_open(UDAT_FULL, UDAT_FULL, \"en-GB\", 0, 0, 0, 0, &ec) failed: %s - (Are you missing data?)\n", u_errorName(ec));
        return;
    }
    udat_applyPattern(simpleDateFormat, 0, pattern, u_strlen(pattern));
    udat_setLenient(simpleDateFormat, 0);

    currentTwoDigitYear = getCurrentYear() % 100;
    startCentury = getCurrentYear() - currentTwoDigitYear;
    if (twoDigitCenturyStart > currentTwoDigitYear) {
      startCentury -= 100;
    }
    tempCal = ucal_open(NULL, -1, NULL, UCAL_GREGORIAN, &ec);
    ucal_setMillis(tempCal, 0, &ec);
    ucal_setDateTime(tempCal, startCentury + twoDigitCenturyStart, UCAL_JANUARY, 1, 0, 0, 0, &ec);
    udat_set2DigitYearStart(simpleDateFormat, ucal_getMillis(tempCal, &ec), &ec);

    calendar = ucal_open(NULL, -1, NULL, UCAL_GREGORIAN, &ec);
    ucal_setMillis(calendar, 0, &ec);
    ucal_setDateTime(calendar, twoDigitCenturyStart, UCAL_JANUARY, 1, 0, 0, 0, &ec);

    udat_parseCalendar(simpleDateFormat, calendar, text, u_strlen(text), &parsePos, &ec);

    /* Check result */
    result = ucal_get(calendar, UCAL_YEAR, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: ucal_get(UCAL_YEAR) failed with %s\n", u_errorName(ec));
        goto FAIL;
    }

    if (result != 75) {
        log_err("FAIL: parsed incorrect year: %d\n", result);
        goto FAIL;
    }

    parsePos = 0;
    udat_applyPattern(simpleDateFormat, 0, pattern2, u_strlen(pattern2));
    udat_parseCalendar(simpleDateFormat, calendar, text, u_strlen(text), &parsePos, &ec);

    /* Check result */
    result = ucal_get(calendar, UCAL_YEAR, &ec);
    if (U_FAILURE(ec)) {
        log_err("FAIL: ucal_get(UCAL_YEAR) failed with %s\n", u_errorName(ec));
        goto FAIL;
    }

    if (result != 1975) {
        log_err("FAIL: parsed incorrect year: %d\n", result);
        goto FAIL;
    }

 FAIL:
    udat_close(simpleDateFormat);
    udat_close(tempCal);
    udat_close(calendar);
}


/*INTERNAL FUNCTIONS USED*/
static int getCurrentYear() {
    static int currentYear = 0;
    if (currentYear == 0) {
        UErrorCode status = U_ZERO_ERROR;
        UCalendar *cal = ucal_open(NULL, -1, NULL, UCAL_GREGORIAN, &status);
        if (!U_FAILURE(status)) {
            /* Get the current year from the default UCalendar */
            currentYear = ucal_get(cal, UCAL_YEAR, &status);
            ucal_close(cal);
        }
    }

    return currentYear;
}

/* N.B.:  use idx instead of index to avoid 'shadow' warnings in strict mode. */
static void VerifygetSymbols(UDateFormat* datfor, UDateFormatSymbolType type, int32_t idx, const char* expected)
{
    UChar *pattern=NULL;
    UErrorCode status = U_ZERO_ERROR;
    UChar *result=NULL;
    int32_t resultlength, resultlengthout;
    int32_t patternSize = (int32_t)strlen(expected) + 1;

    pattern=(UChar*)malloc(sizeof(UChar) * patternSize);
    u_unescape(expected, pattern, patternSize);
    resultlength=0;
    resultlengthout=udat_getSymbols(datfor, type, idx , NULL, resultlength, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR)
    {
        status=U_ZERO_ERROR;
        resultlength=resultlengthout+1;
        result=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_getSymbols(datfor, type, idx, result, resultlength, &status);

    }
    if(U_FAILURE(status))
    {
        log_err("FAIL: Error in udat_getSymbols()... %s\n", myErrorName(status) );
        return;
    }
    if(u_strcmp(result, pattern)==0)
        log_verbose("PASS: getSymbols retrieved the right value\n");
    else{
        log_data_err("FAIL: getSymbols retrieved the wrong value\n Expected %s Got %s\n", expected,
            aescstrdup(result,-1) );
    }
    free(result);
    free(pattern);
}

static void VerifysetSymbols(UDateFormat* datfor, UDateFormatSymbolType type, int32_t idx, const char* expected)
{
    UChar *result=NULL;
    UChar *value=NULL;
    int32_t resultlength, resultlengthout;
    UErrorCode status = U_ZERO_ERROR;
    int32_t valueLen, valueSize = (int32_t)strlen(expected) + 1;

    value=(UChar*)malloc(sizeof(UChar) * valueSize);
    valueLen = u_unescape(expected, value, valueSize);
    udat_setSymbols(datfor, type, idx, value, valueLen, &status);
    if(U_FAILURE(status))
        {
            log_err("FAIL: Error in udat_setSymbols()  %s\n", myErrorName(status) );
            return;
        }

    resultlength=0;
    resultlengthout=udat_getSymbols(datfor, type, idx, NULL, resultlength, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR){
        status=U_ZERO_ERROR;
        resultlength=resultlengthout+1;
        result=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_getSymbols(datfor, type, idx, result, resultlength, &status);
    }
    if(U_FAILURE(status)){
        log_err("FAIL: error in retrieving the value using getSymbols after setting it previously\n %s\n",
            myErrorName(status) );
        return;
    }

    if(u_strcmp(result, value)!=0){
        log_err("FAIL:Error in setting and then getting symbols\n Expected %s Got %s\n", expected,
            aescstrdup(result,-1) );
    }
    else
        log_verbose("PASS: setSymbols successful\n");

    free(value);
    free(result);
}


static void VerifygetsetSymbols(UDateFormat* from, UDateFormat* to, UDateFormatSymbolType type, int32_t idx)
{
    UChar *result=NULL;
    UChar *value=NULL;
    int32_t resultlength, resultlengthout;
    UErrorCode status = U_ZERO_ERROR;

    resultlength=0;
    resultlengthout=udat_getSymbols(from, type, idx , NULL, resultlength, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR){
        status=U_ZERO_ERROR;
        resultlength=resultlengthout+1;
        result=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_getSymbols(from, type, idx, result, resultlength, &status);
    }
    if(U_FAILURE(status)){
        log_err("FAIL: error in getSymbols() %s\n", myErrorName(status) );
        return;
    }

    resultlength=resultlengthout+1;
    udat_setSymbols(to, type, idx, result, resultlength, &status);
    if(U_FAILURE(status))
        {
            log_err("FAIL: Error in udat_setSymbols() : %s\n", myErrorName(status) );
            return;
        }

    resultlength=0;
    resultlengthout=udat_getSymbols(to, type, idx, NULL, resultlength, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR){
        status=U_ZERO_ERROR;
        resultlength=resultlengthout+1;
        value=(UChar*)malloc(sizeof(UChar) * resultlength);
        udat_getSymbols(to, type, idx, value, resultlength, &status);
    }
    if(U_FAILURE(status)){
        log_err("FAIL: error in retrieving the value using getSymbols i.e roundtrip\n %s\n",
            myErrorName(status) );
        return;
    }

    if(u_strcmp(result, value)!=0){
        log_data_err("FAIL:Error in setting and then getting symbols\n Expected %s Got %s\n", austrdup(result),
            austrdup(value) );
    }
    else
        log_verbose("PASS: setSymbols successful\n");

    free(value);
    free(result);
}


static UChar* myNumformat(const UNumberFormat* numfor, double d)
{
    UChar *result2=NULL;
    int32_t resultlength, resultlengthneeded;
    UErrorCode status = U_ZERO_ERROR;

    resultlength=0;
    resultlengthneeded=unum_formatDouble(numfor, d, NULL, resultlength, NULL, &status);
    if(status==U_BUFFER_OVERFLOW_ERROR)
    {
        status=U_ZERO_ERROR;
        resultlength=resultlengthneeded+1;
        /*result2=(UChar*)malloc(sizeof(UChar) * resultlength);*/ /* this leaks */
        result2=(UChar*)ctst_malloc(sizeof(UChar) * resultlength); /*this won't*/
        unum_formatDouble(numfor, d, result2, resultlength, NULL, &status);
    }
    if(U_FAILURE(status))
    {
        log_err("FAIL: Error in formatting using unum_format(.....) %s\n", myErrorName(status) );
        return 0;
    }

    return result2;
}

/**
 * The search depth for TestExtremeDates.  The total number of
 * dates that will be tested is (2^EXTREME_DATES_DEPTH) - 1.
 */
#define EXTREME_DATES_DEPTH 8

/**
 * Support for TestExtremeDates (below).
 *
 * Test a single date to see whether udat_format handles it properly.
 */
static UBool _aux1ExtremeDates(UDateFormat* fmt, UDate date,
                               UChar* buf, int32_t buflen, char* cbuf,
                               UErrorCode* ec) {
    int32_t len = udat_format(fmt, date, buf, buflen, 0, ec);
    if (!assertSuccess("udat_format", ec)) return FALSE;
    u_austrncpy(cbuf, buf, buflen);
    if (len < 4) {
        log_err("FAIL: udat_format(%g) => \"%s\"\n", date, cbuf);
    } else {
        log_verbose("udat_format(%g) => \"%s\"\n", date, cbuf);
    }
    return TRUE;
}

/**
 * Support for TestExtremeDates (below).
 *
 * Recursively test between 'small' and 'large', up to the depth
 * limit specified by EXTREME_DATES_DEPTH.
 */
static UBool _aux2ExtremeDates(UDateFormat* fmt, UDate small, UDate large,
                               UChar* buf, int32_t buflen, char* cbuf,
                               int32_t count,
                               UErrorCode* ec) {
    /* Logarithmic midpoint; see below */
    UDate mid = (UDate) exp((log(small) + log(large)) / 2);
    if (count == EXTREME_DATES_DEPTH) {
        return TRUE;
    }
    return
        _aux1ExtremeDates(fmt, mid, buf, buflen, cbuf, ec) &&
        _aux2ExtremeDates(fmt, small, mid, buf, buflen, cbuf, count+1, ec) &&
        _aux2ExtremeDates(fmt, mid, large, buf, buflen, cbuf, count+1, ec);
}

/**
 * http://www.jtcsv.com/cgibin/icu-bugs?findid=3659
 *
 * For certain large dates, udat_format crashes on MacOS.  This test
 * attempts to reproduce this problem by doing a recursive logarithmic*
 * binary search of a predefined interval (from 'small' to 'large').
 *
 * The limit of the search is given by EXTREME_DATES_DEPTH, above.
 *
 * *The search has to be logarithmic, not linear.  A linear search of the
 *  range 0..10^30, for example, will find 0.5*10^30, then 0.25*10^30 and
 *  0.75*10^30, etc.  A logarithmic search will find 10^15, then 10^7.5
 *  and 10^22.5, etc.
 */
static void TestExtremeDates() {
    UDateFormat *fmt;
    UErrorCode ec;
    UChar buf[256];
    char cbuf[256];
    const double small = 1000; /* 1 sec */
    const double large = 1e+30; /* well beyond usable UDate range */

    /* There is no need to test larger values from 1e+30 to 1e+300;
       the failures occur around 1e+27, and never above 1e+30. */

    ec = U_ZERO_ERROR;
    fmt = udat_open(UDAT_LONG, UDAT_LONG, "en_US",
                    0, 0, 0, 0, &ec);
    if (U_FAILURE(ec)) {
        log_data_err("FAIL: udat_open (%s) (Are you missing data?)\n", u_errorName(ec));
        return;
    }

    _aux2ExtremeDates(fmt, small, large, buf, UPRV_LENGTHOF(buf), cbuf, 0, &ec);

    udat_close(fmt);
}

static void TestAllLocales(void) {
    int32_t idx, dateIdx, timeIdx, localeCount;
    static const UDateFormatStyle style[] = {
        UDAT_FULL, UDAT_LONG, UDAT_MEDIUM, UDAT_SHORT
    };
    localeCount = uloc_countAvailable();
    for (idx = 0; idx < localeCount; idx++) {
        for (dateIdx = 0; dateIdx < UPRV_LENGTHOF(style); dateIdx++) {
            for (timeIdx = 0; timeIdx < UPRV_LENGTHOF(style); timeIdx++) {
                UErrorCode status = U_ZERO_ERROR;
                udat_close(udat_open(style[dateIdx], style[timeIdx],
                    uloc_getAvailable(idx), NULL, 0, NULL, 0, &status));
                if (U_FAILURE(status)) {
                    log_err("FAIL: udat_open(%s) failed with (%s) dateIdx=%d, timeIdx=%d\n",
                        uloc_getAvailable(idx), u_errorName(status), dateIdx, timeIdx);
                }
            }
        }
    }
}

static void TestRelativeCrash(void) {
       static const UChar tzName[] = { 0x0055, 0x0053, 0x002F, 0x0050, 0x0061, 0x0063, 0x0069, 0x0066, 0x0069, 0x0063, 0 };
       static const UDate aDate = -631152000000.0;

    UErrorCode status = U_ZERO_ERROR;
    UErrorCode expectStatus = U_ILLEGAL_ARGUMENT_ERROR;
    UDateFormat icudf;

    icudf = udat_open(UDAT_NONE, UDAT_SHORT_RELATIVE, "en", tzName, -1, NULL, 0, &status);
    if ( U_SUCCESS(status) ) {
        const char *what = "???";
        {
            UErrorCode subStatus = U_ZERO_ERROR;
            what = "udat_set2DigitYearStart";
            log_verbose("Trying %s on a relative date..\n", what);
            udat_set2DigitYearStart(icudf, aDate, &subStatus);
            if(subStatus == expectStatus) {
                log_verbose("Success: did not crash on %s, but got %s.\n", what, u_errorName(subStatus));
            } else {
                log_err("FAIL: didn't crash on %s, but got success %s instead of %s. \n", what, u_errorName(subStatus), u_errorName(expectStatus));
            }
        }
        {
            /* clone works polymorphically. try it anyways */
            UErrorCode subStatus = U_ZERO_ERROR;
            UDateFormat *oth;
            what = "clone";
            log_verbose("Trying %s on a relative date..\n", what);
            oth = udat_clone(icudf, &subStatus);
            if(subStatus == U_ZERO_ERROR) {
                log_verbose("Success: did not crash on %s, but got %s.\n", what, u_errorName(subStatus));
                udat_close(oth); /* ? */
            } else {
                log_err("FAIL: didn't crash on %s, but got  %s instead of %s. \n", what, u_errorName(subStatus), u_errorName(expectStatus));
            }
        }
        {
            UErrorCode subStatus = U_ZERO_ERROR;
            what = "udat_get2DigitYearStart";
            log_verbose("Trying %s on a relative date..\n", what);
            udat_get2DigitYearStart(icudf, &subStatus);
            if(subStatus == expectStatus) {
                log_verbose("Success: did not crash on %s, but got %s.\n", what, u_errorName(subStatus));
            } else {
                log_err("FAIL: didn't crash on %s, but got success %s instead of %s. \n", what, u_errorName(subStatus), u_errorName(expectStatus));
            }
        }
        {
            /* Now udat_toPattern works for relative date formatters, unless localized is TRUE */
            UErrorCode subStatus = U_ZERO_ERROR;
            what = "udat_toPattern";
            log_verbose("Trying %s on a relative date..\n", what);
            udat_toPattern(icudf, TRUE,NULL,0, &subStatus);
            if(subStatus == expectStatus) {
                log_verbose("Success: did not crash on %s, but got %s.\n", what, u_errorName(subStatus));
            } else {
                log_err("FAIL: didn't crash on %s, but got success %s instead of %s. \n", what, u_errorName(subStatus), u_errorName(expectStatus));
            }
        }
        {
            UErrorCode subStatus = U_ZERO_ERROR;
            what = "udat_applyPattern";
            log_verbose("Trying %s on a relative date..\n", what);
            udat_applyPattern(icudf, FALSE,tzName,-1);
            subStatus = U_ILLEGAL_ARGUMENT_ERROR; /* what it should be, if this took an errorcode. */
            if(subStatus == expectStatus) {
                log_verbose("Success: did not crash on %s, but got %s.\n", what, u_errorName(subStatus));
            } else {
                log_err("FAIL: didn't crash on %s, but got success %s instead of %s. \n", what, u_errorName(subStatus), u_errorName(expectStatus));
            }
        }
        {
            UChar erabuf[32];
            UErrorCode subStatus = U_ZERO_ERROR;
            what = "udat_getSymbols";
            log_verbose("Trying %s on a relative date..\n", what);
            udat_getSymbols(icudf, UDAT_ERAS,0,erabuf,UPRV_LENGTHOF(erabuf), &subStatus);
            if(subStatus == U_ZERO_ERROR) {
                log_verbose("Success: %s returned %s.\n", what, u_errorName(subStatus));
            } else {
                log_err("FAIL: didn't crash on %s, but got %s instead of U_ZERO_ERROR.\n", what, u_errorName(subStatus));
            }
        }
        {
            UErrorCode subStatus = U_ZERO_ERROR;
            UChar symbolValue = 0x0041;
            what = "udat_setSymbols";
            log_verbose("Trying %s on a relative date..\n", what);
            udat_setSymbols(icudf, UDAT_ERAS,0,&symbolValue,1, &subStatus);  /* bogus values */
            if(subStatus == expectStatus) {
                log_verbose("Success: did not crash on %s, but got %s.\n", what, u_errorName(subStatus));
            } else {
                log_err("FAIL: didn't crash on %s, but got success %s instead of %s. \n", what, u_errorName(subStatus), u_errorName(expectStatus));
            }
        }
        {
            UErrorCode subStatus = U_ZERO_ERROR;
            what = "udat_countSymbols";
            log_verbose("Trying %s on a relative date..\n", what);
            udat_countSymbols(icudf, UDAT_ERAS);
            subStatus = U_ILLEGAL_ARGUMENT_ERROR; /* should have an errorcode. */
            if(subStatus == expectStatus) {
                log_verbose("Success: did not crash on %s, but got %s.\n", what, u_errorName(subStatus));
            } else {
                log_err("FAIL: didn't crash on %s, but got success %s instead of %s. \n", what, u_errorName(subStatus), u_errorName(expectStatus));
            }
        }

        udat_close(icudf);
    } else {
         log_data_err("FAIL: err calling udat_open() ->%s (Are you missing data?)\n", u_errorName(status));
    }
}

static const UChar skeleton_yMMMM[] = { 0x79,0x4D,0x4D,0x4D,0x4D,0 }; /* "yMMMM"; fr maps to "MMMM y", cs maps to "LLLL y" */
static const UChar july2008_frDefault[] = { 0x6A,0x75,0x69,0x6C,0x6C,0x65,0x74,0x20,0x32,0x30,0x30,0x38,0 }; /* "juillet 2008" */
static const UChar july2008_frTitle[] = { 0x4A,0x75,0x69,0x6C,0x6C,0x65,0x74,0x20,0x32,0x30,0x30,0x38,0 };  /* "Juillet 2008" sentence-begin, standalone */
static const UChar july2008_csDefault[] = { 0x10D,0x65,0x72,0x76,0x65,0x6E,0x65,0x63,0x20,0x32,0x30,0x30,0x38,0 }; /* "c(hacek)ervenec 2008" */
static const UChar july2008_csTitle[] = { 0x10C,0x65,0x72,0x76,0x65,0x6E,0x65,0x63,0x20,0x32,0x30,0x30,0x38,0 }; /* "C(hacek)ervenec 2008" sentence-begin, uiListOrMenu */

typedef struct {
    const char * locale;
    const UChar * skeleton;
    UDisplayContext capitalizationContext;
    const UChar * expectedFormat;
} TestContextItem;

static const TestContextItem textContextItems[] = {
    { "fr", skeleton_yMMMM, UDISPCTX_CAPITALIZATION_NONE,                   july2008_frDefault },
#if !UCONFIG_NO_BREAK_ITERATION
    { "fr", skeleton_yMMMM, UDISPCTX_CAPITALIZATION_FOR_MIDDLE_OF_SENTENCE, july2008_frDefault },
    { "fr", skeleton_yMMMM, UDISPCTX_CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE, july2008_frTitle },
    { "fr", skeleton_yMMMM, UDISPCTX_CAPITALIZATION_FOR_UI_LIST_OR_MENU,    july2008_frDefault },
    { "fr", skeleton_yMMMM, UDISPCTX_CAPITALIZATION_FOR_STANDALONE,         july2008_frTitle },
#endif
    { "cs", skeleton_yMMMM, UDISPCTX_CAPITALIZATION_NONE,                   july2008_csDefault },
#if !UCONFIG_NO_BREAK_ITERATION
    { "cs", skeleton_yMMMM, UDISPCTX_CAPITALIZATION_FOR_MIDDLE_OF_SENTENCE, july2008_csDefault },
    { "cs", skeleton_yMMMM, UDISPCTX_CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE, july2008_csTitle },
    { "cs", skeleton_yMMMM, UDISPCTX_CAPITALIZATION_FOR_UI_LIST_OR_MENU,    july2008_csTitle },
    { "cs", skeleton_yMMMM, UDISPCTX_CAPITALIZATION_FOR_STANDALONE,         july2008_csDefault },
#endif
    { NULL, NULL, (UDisplayContext)0, NULL }
};

static const UChar today_enDefault[]     = { 0x74,0x6F,0x64,0x61,0x79,0 }; /* "today" */
static const UChar today_enTitle[]       = { 0x54,0x6F,0x64,0x61,0x79,0 };  /* "Today" sentence-begin, uiListOrMenu, standalone */
static const UChar yesterday_enDefault[] = { 0x79,0x65,0x73,0x74,0x65,0x72,0x64,0x61,0x79,0 }; /* "yesterday" */
static const UChar yesterday_enTitle[]   = { 0x59,0x65,0x73,0x74,0x65,0x72,0x64,0x61,0x79,0 };  /* "Yesterday" sentence-begin, uiListOrMenu, standalone */
static const UChar today_nbDefault[]     = { 0x69,0x20,0x64,0x61,0x67,0 }; /* "i dag" */
static const UChar today_nbTitle[]       = { 0x49,0x20,0x64,0x61,0x67,0 };  /* "I dag" sentence-begin, standalone */
static const UChar yesterday_nbDefault[] = { 0x69,0x20,0x67,0xE5,0x72,0 };
static const UChar yesterday_nbTitle[]   = { 0x49,0x20,0x67,0xE5,0x72,0 };

typedef struct {
    const char * locale;
    UDisplayContext capitalizationContext;
    const UChar * expectedFormatToday;
    const UChar * expectedFormatYesterday;
} TestRelativeContextItem;

static const TestRelativeContextItem textContextRelativeItems[] = {
    { "en", UDISPCTX_CAPITALIZATION_NONE,                   today_enDefault, yesterday_enDefault },
#if !UCONFIG_NO_BREAK_ITERATION
    { "en", UDISPCTX_CAPITALIZATION_FOR_MIDDLE_OF_SENTENCE, today_enDefault, yesterday_enDefault },
    { "en", UDISPCTX_CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE, today_enTitle, yesterday_enTitle },
    { "en", UDISPCTX_CAPITALIZATION_FOR_UI_LIST_OR_MENU,    today_enTitle, yesterday_enTitle },
    { "en", UDISPCTX_CAPITALIZATION_FOR_STANDALONE,         today_enTitle, yesterday_enTitle },
#endif
    { "nb", UDISPCTX_CAPITALIZATION_NONE,                   today_nbDefault, yesterday_nbDefault },
#if !UCONFIG_NO_BREAK_ITERATION
    { "nb", UDISPCTX_CAPITALIZATION_FOR_MIDDLE_OF_SENTENCE, today_nbDefault, yesterday_nbDefault },
    { "nb", UDISPCTX_CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE, today_nbTitle, yesterday_nbTitle },
    { "nb", UDISPCTX_CAPITALIZATION_FOR_UI_LIST_OR_MENU,    today_nbDefault, yesterday_nbDefault },
    { "nb", UDISPCTX_CAPITALIZATION_FOR_STANDALONE,         today_nbTitle, yesterday_nbTitle },
#endif
    { NULL, (UDisplayContext)0, NULL, NULL }
};

static const UChar january_esDefault[] = { 0x65,0x6E,0x65,0x72,0x6F,0 }; /* "enero" */
static const UChar january_esTitle[] = { 0x45,0x6E,0x65,0x72,0x6F,0 };  /* "Enero */
static const UChar monday_daDefault[] = { 0x6D,0x61,0x6E,0x64,0x61,0x67,0 }; /* "mandag" */
static const UChar monday_daTitle[] = { 0x4D,0x61,0x6E,0x64,0x61,0x67,0 };  /* "Mandag */

typedef struct {
    const char * locale;
    UDateFormatSymbolType type;
    int32_t index;
    UDisplayContext capitalizationContext;
    const UChar * expectedFormat;
} TestSymbolContextItem;

static const TestSymbolContextItem testContextSymbolItems[] = {
    { "es", UDAT_MONTHS, 0, UDISPCTX_CAPITALIZATION_NONE,                       january_esDefault },
#if !UCONFIG_NO_BREAK_ITERATION
    { "es", UDAT_MONTHS, 0, UDISPCTX_CAPITALIZATION_FOR_MIDDLE_OF_SENTENCE,     january_esDefault },
    { "es", UDAT_MONTHS, 0, UDISPCTX_CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE,  january_esTitle },
    { "es", UDAT_MONTHS, 0, UDISPCTX_CAPITALIZATION_FOR_UI_LIST_OR_MENU,        january_esTitle },
    { "es", UDAT_MONTHS, 0, UDISPCTX_CAPITALIZATION_FOR_STANDALONE,             january_esTitle },
#endif
    { "da", UDAT_WEEKDAYS, 2, UDISPCTX_CAPITALIZATION_NONE,                      monday_daDefault },
#if !UCONFIG_NO_BREAK_ITERATION
    { "da", UDAT_WEEKDAYS, 2, UDISPCTX_CAPITALIZATION_FOR_MIDDLE_OF_SENTENCE,    monday_daDefault },
    { "da", UDAT_WEEKDAYS, 2, UDISPCTX_CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE, monday_daTitle },
    { "da", UDAT_WEEKDAYS, 2, UDISPCTX_CAPITALIZATION_FOR_UI_LIST_OR_MENU,       monday_daDefault },
    { "da", UDAT_WEEKDAYS, 2, UDISPCTX_CAPITALIZATION_FOR_STANDALONE,            monday_daDefault },
#endif
    { NULL, (UDateFormatSymbolType)0, 0, (UDisplayContext)0, NULL }
};

static const UChar zoneGMT[] = { 0x47,0x4D,0x54,0 }; /* "GMT" */
static const UDate july022008 = 1215000000000.0;
enum { kUbufMax = 64, kBbufMax = 3*kUbufMax };

static void TestContext(void) {
    const TestContextItem* textContextItemPtr;
    const TestRelativeContextItem* textRelContextItemPtr;
    const TestSymbolContextItem* testSymContextItemPtr;

    for (textContextItemPtr = textContextItems; textContextItemPtr->locale != NULL; ++textContextItemPtr) {
        UErrorCode status = U_ZERO_ERROR;
        UDateTimePatternGenerator* udtpg = udatpg_open(textContextItemPtr->locale, &status);
        if ( U_SUCCESS(status) ) {
            UChar ubuf[kUbufMax];
            int32_t len = udatpg_getBestPattern(udtpg, textContextItemPtr->skeleton, -1, ubuf, kUbufMax, &status);
            if ( U_SUCCESS(status) ) {
                UDateFormat* udfmt = udat_open(UDAT_PATTERN, UDAT_PATTERN, textContextItemPtr->locale, zoneGMT, -1, ubuf, len, &status);
                if ( U_SUCCESS(status) ) {
                    udat_setContext(udfmt, textContextItemPtr->capitalizationContext, &status);
                    if ( U_SUCCESS(status) ) {
                        UDisplayContext getContext;
                        len = udat_format(udfmt, july022008, ubuf, kUbufMax, NULL, &status);
                        if ( U_FAILURE(status) ) {
                            log_err("FAIL: udat_format for locale %s, capitalizationContext %d, status %s\n",
                                    textContextItemPtr->locale, (int)textContextItemPtr->capitalizationContext, u_errorName(status) );
                            status = U_ZERO_ERROR;
                        } else if (u_strncmp(ubuf, textContextItemPtr->expectedFormat, kUbufMax) != 0) {
                            char bbuf1[kBbufMax];
                            char bbuf2[kBbufMax];
                            log_err("FAIL: udat_format for locale %s, capitalizationContext %d, expected %s, got %s\n",
                                    textContextItemPtr->locale, (int)textContextItemPtr->capitalizationContext,
                                    u_austrncpy(bbuf1,textContextItemPtr->expectedFormat,kUbufMax), u_austrncpy(bbuf2,ubuf,kUbufMax) );
                        }
                        getContext = udat_getContext(udfmt, UDISPCTX_TYPE_CAPITALIZATION, &status);
                        if ( U_FAILURE(status) ) {
                            log_err("FAIL: udat_getContext for locale %s, capitalizationContext %d, status %s\n",
                                    textContextItemPtr->locale, (int)textContextItemPtr->capitalizationContext, u_errorName(status) );
                        } else if (getContext != textContextItemPtr->capitalizationContext) {
                            log_err("FAIL: udat_getContext for locale %s, capitalizationContext %d, got context %d\n",
                                    textContextItemPtr->locale, (int)textContextItemPtr->capitalizationContext, (int)getContext );
                        }
                    } else {
                        log_err("FAIL: udat_setContext for locale %s, capitalizationContext %d, status %s\n",
                                textContextItemPtr->locale, (int)textContextItemPtr->capitalizationContext, u_errorName(status) );
                    }
                    udat_close(udfmt);
                } else {
                    log_data_err("FAIL: udat_open for locale %s, status %s\n", textContextItemPtr->locale, u_errorName(status) );
                }
            } else {
                log_err("FAIL: udatpg_getBestPattern for locale %s, status %s\n", textContextItemPtr->locale, u_errorName(status) );
            }
            udatpg_close(udtpg);
        } else {
            log_data_err("FAIL: udatpg_open for locale %s, status %s\n", textContextItemPtr->locale, u_errorName(status) );
        }
    }
    for (textRelContextItemPtr = textContextRelativeItems; textRelContextItemPtr->locale != NULL; ++textRelContextItemPtr) {
        UErrorCode status = U_ZERO_ERROR;
        UCalendar* ucal = ucal_open(zoneGMT, -1, "root", UCAL_GREGORIAN, &status);
        if ( U_SUCCESS(status) ) {
            UDateFormat* udfmt = udat_open(UDAT_NONE, UDAT_LONG_RELATIVE, textRelContextItemPtr->locale, zoneGMT, -1, NULL, 0, &status);
            if ( U_SUCCESS(status) ) {
                udat_setContext(udfmt, textRelContextItemPtr->capitalizationContext, &status);
                if ( U_SUCCESS(status) ) {
                    UDate yesterday, today = ucal_getNow();
                    UChar ubuf[kUbufMax];
                    char bbuf1[kBbufMax];
                    char bbuf2[kBbufMax];
                    int32_t len = udat_format(udfmt, today, ubuf, kUbufMax, NULL, &status);
                    (void)len;
                    if ( U_FAILURE(status) ) {
                        log_err("FAIL: udat_format today for locale %s, capitalizationContext %d, status %s\n",
                                textRelContextItemPtr->locale, (int)textRelContextItemPtr->capitalizationContext, u_errorName(status) );
                    } else if (u_strncmp(ubuf, textRelContextItemPtr->expectedFormatToday, kUbufMax) != 0) {
                        log_err("FAIL: udat_format today for locale %s, capitalizationContext %d, expected %s, got %s\n",
                                textRelContextItemPtr->locale, (int)textRelContextItemPtr->capitalizationContext,
                                u_austrncpy(bbuf1,textRelContextItemPtr->expectedFormatToday,kUbufMax), u_austrncpy(bbuf2,ubuf,kUbufMax) );
                    }
                    status = U_ZERO_ERROR;
                    ucal_setMillis(ucal, today, &status);
                    ucal_add(ucal, UCAL_DATE, -1, &status);
                    yesterday = ucal_getMillis(ucal, &status);
                    if ( U_SUCCESS(status) ) {
                        len = udat_format(udfmt, yesterday, ubuf, kUbufMax, NULL, &status);
                        if ( U_FAILURE(status) ) {
                            log_err("FAIL: udat_format yesterday for locale %s, capitalizationContext %d, status %s\n",
                                    textRelContextItemPtr->locale, (int)textRelContextItemPtr->capitalizationContext, u_errorName(status) );
                        } else if (u_strncmp(ubuf, textRelContextItemPtr->expectedFormatYesterday, kUbufMax) != 0) {
                            log_err("FAIL: udat_format yesterday for locale %s, capitalizationContext %d, expected %s, got %s\n",
                                    textRelContextItemPtr->locale, (int)textRelContextItemPtr->capitalizationContext,
                                    u_austrncpy(bbuf1,textRelContextItemPtr->expectedFormatYesterday,kUbufMax), u_austrncpy(bbuf2,ubuf,kUbufMax) );
                        }
                    }
                } else {
                    log_err("FAIL: udat_setContext relative for locale %s, capitalizationContext %d, status %s\n",
                            textRelContextItemPtr->locale, (int)textRelContextItemPtr->capitalizationContext, u_errorName(status) );
                }
                udat_close(udfmt);
            } else {
                log_data_err("FAIL: udat_open relative for locale %s, status %s\n", textRelContextItemPtr->locale, u_errorName(status) );
            }
            ucal_close(ucal);
        } else {
            log_data_err("FAIL: ucal_open for locale root, status %s\n", u_errorName(status) );
        }
    }

    for (testSymContextItemPtr = testContextSymbolItems; testSymContextItemPtr->locale != NULL; ++testSymContextItemPtr) {
        UErrorCode status = U_ZERO_ERROR;
        UDateFormat* udfmt = udat_open(UDAT_MEDIUM, UDAT_FULL, testSymContextItemPtr->locale, zoneGMT, -1, NULL, 0, &status);
        if ( U_SUCCESS(status) ) {
            udat_setContext(udfmt, testSymContextItemPtr->capitalizationContext, &status);
            if ( U_SUCCESS(status) ) {
                UChar ubuf[kUbufMax];
                int32_t len = udat_getSymbols(udfmt, testSymContextItemPtr->type, testSymContextItemPtr->index, ubuf, kUbufMax, &status);
                if ( U_FAILURE(status) ) {
                    log_err("FAIL: udat_getSymbols for locale %s, capitalizationContext %d, status %s\n",
                            testSymContextItemPtr->locale, (int)testSymContextItemPtr->capitalizationContext, u_errorName(status) );
                } else if (u_strncmp(ubuf, testSymContextItemPtr->expectedFormat, kUbufMax) != 0) {
                    char bbuf1[kBbufMax];
                    char bbuf2[kBbufMax];
                    log_err("FAIL: udat_getSymbols for locale %s, capitalizationContext %d, expected %s, got %s\n",
                            testSymContextItemPtr->locale, (int)testSymContextItemPtr->capitalizationContext,
                            u_austrncpy(bbuf1,testSymContextItemPtr->expectedFormat,kUbufMax), u_austrncpy(bbuf2,ubuf,kUbufMax) );
                }
            } else {
                log_err("FAIL: udat_setContext std for locale %s, capitalizationContext %d, status %s\n",
                        testSymContextItemPtr->locale, (int)testSymContextItemPtr->capitalizationContext, u_errorName(status) );
            }
            udat_close(udfmt);
        } else {
            log_data_err("FAIL: udat_open std for locale %s, status %s\n", testSymContextItemPtr->locale, u_errorName(status) );
        }
    }
}


// overrideNumberFormat[i][0] is to tell which field to set,
// overrideNumberFormat[i][1] is the expected result
static const char * overrideNumberFormat[][2] = {
        {"", "\\u521D\\u4E03 \\u521D\\u4E8C"},
        {"d", "07 \\u521D\\u4E8C"},
        {"do", "07 \\u521D\\u4E8C"},
        {"Md", "\\u521D\\u4E03 \\u521D\\u4E8C"},
        {"MdMMd", "\\u521D\\u4E03 \\u521D\\u4E8C"},
        {"mixed", "\\u521D\\u4E03 \\u521D\\u4E8C"}
};

static void TestOverrideNumberFormat(void) {
    UErrorCode status = U_ZERO_ERROR;
    UChar pattern[50];
    UChar expected[50];
    UChar fields[50];
    char bbuf1[kBbufMax];
    char bbuf2[kBbufMax];
    const char* localeString = "zh@numbers=hanidays";
    UDateFormat* fmt;
    const UNumberFormat* getter_result;
    int32_t i;

    u_uastrcpy(fields, "d");
    u_uastrcpy(pattern,"MM d");

    fmt=udat_open(UDAT_PATTERN, UDAT_PATTERN, "en_US", zoneGMT, -1, pattern, u_strlen(pattern), &status);
    if (!assertSuccess("udat_open()", &status)) {
        return;
    }

    // loop 5 times to check getter/setter
    for (i = 0; i < 5; i++){
        status = U_ZERO_ERROR;
        UNumberFormat* overrideFmt;
        overrideFmt = unum_open(UNUM_DEFAULT, NULL, 0, localeString, NULL, &status);
        assertSuccess("unum_open()", &status);
        udat_adoptNumberFormatForFields(fmt, fields, overrideFmt, &status);
        overrideFmt = NULL; // no longer valid
        assertSuccess("udat_setNumberFormatForField()", &status);

        getter_result = udat_getNumberFormatForField(fmt, 0x0064 /*'d'*/);
        if(getter_result == NULL) {
            log_err("FAIL: udat_getNumberFormatForField did not return a valid pointer\n");
        }
    }
    {
      status = U_ZERO_ERROR;
      UNumberFormat* overrideFmt;
      overrideFmt = unum_open(UNUM_DEFAULT, NULL, 0, localeString, NULL, &status);
      assertSuccess("unum_open()", &status);
      if (U_SUCCESS(status)) {
        udat_setNumberFormat(fmt, overrideFmt); // test the same override NF will not crash
      }
      unum_close(overrideFmt);
    }
    udat_close(fmt);

    for (i=0; i<UPRV_LENGTHOF(overrideNumberFormat); i++){
        status = U_ZERO_ERROR;
        UChar ubuf[kUbufMax];
        UDateFormat* fmt2;
        UNumberFormat* overrideFmt2;

        fmt2 =udat_open(UDAT_PATTERN, UDAT_PATTERN,"en_US", zoneGMT, -1, pattern, u_strlen(pattern), &status);
        assertSuccess("udat_open() with en_US", &status);

        overrideFmt2 = unum_open(UNUM_DEFAULT, NULL, 0, localeString, NULL, &status);
        assertSuccess("unum_open() in loop", &status);

        if (U_FAILURE(status)) {
            continue;
        }

        u_uastrcpy(fields, overrideNumberFormat[i][0]);
        u_unescape(overrideNumberFormat[i][1], expected, UPRV_LENGTHOF(expected));

        if ( strcmp(overrideNumberFormat[i][0], "") == 0 ) { // use the one w/o field
            udat_adoptNumberFormat(fmt2, overrideFmt2);
        } else if ( strcmp(overrideNumberFormat[i][0], "mixed") == 0 ) { // set 1 field at first but then full override, both(M & d) should be override
            const char* singleLocale = "en@numbers=hebr";
            UNumberFormat* singleOverrideFmt;
            u_uastrcpy(fields, "d");

            singleOverrideFmt = unum_open(UNUM_DEFAULT, NULL, 0, singleLocale, NULL, &status);
            assertSuccess("unum_open() in mixed", &status);

            udat_adoptNumberFormatForFields(fmt2, fields, singleOverrideFmt, &status);
            assertSuccess("udat_setNumberFormatForField() in mixed", &status);

            udat_adoptNumberFormat(fmt2, overrideFmt2);
        } else if ( strcmp(overrideNumberFormat[i][0], "do") == 0 ) { // o is an invalid field
            udat_adoptNumberFormatForFields(fmt2, fields, overrideFmt2, &status);
            if(status == U_INVALID_FORMAT_ERROR) {
                udat_close(fmt2);
                status = U_ZERO_ERROR;
                continue;
            }
        } else {
            udat_adoptNumberFormatForFields(fmt2, fields, overrideFmt2, &status);
            assertSuccess("udat_setNumberFormatForField() in loop", &status);
        }

        udat_format(fmt2, july022008, ubuf, kUbufMax, NULL, &status);
        assertSuccess("udat_format() july022008", &status);

        if (u_strncmp(ubuf, expected, kUbufMax) != 0)
            log_err("fail: udat_format for locale, expected %s, got %s\n",
                    u_austrncpy(bbuf1,expected,kUbufMax), u_austrncpy(bbuf2,ubuf,kUbufMax) );

        udat_close(fmt2);
    }
}

/*
 * Ticket #11523
 * udat_parse and udat_parseCalendar should have the same error code when given the same invalid input.
 */
static void TestParseErrorReturnValue(void) {
    UErrorCode status = U_ZERO_ERROR;
    UErrorCode expectStatus = U_PARSE_ERROR;
    UDateFormat* df;
    UCalendar* cal;

    df = udat_open(UDAT_DEFAULT, UDAT_DEFAULT, NULL, NULL, -1, NULL, -1, &status);
    if (!assertSuccessCheck("udat_open()", &status, TRUE)) {
        return;
    }

    cal = ucal_open(NULL, 0, "en_US", UCAL_GREGORIAN, &status);
    if (!assertSuccess("ucal_open()", &status)) {
        return;
    }

    udat_parse(df, NULL, -1, NULL, &status);
    if (status != expectStatus) {
        log_err("%s should have been returned by udat_parse when given an invalid input, instead got - %s\n", u_errorName(expectStatus), u_errorName(status));
    }

    status = U_ZERO_ERROR;
    udat_parseCalendar(df, cal, NULL, -1, NULL, &status);
    if (status != expectStatus) {
        log_err("%s should have been returned by udat_parseCalendar when given an invalid input, instead got - %s\n", u_errorName(expectStatus), u_errorName(status));
    }

    ucal_close(cal);
    udat_close(df);
}

/*
 * Ticket #11553
 * Test new udat_formatForFields, udat_formatCalendarForFields (and UFieldPositionIterator)
 */
static const char localeForFields[] = "en_US";
/* zoneGMT[]defined above */
static const UDate date2015Feb25 = 1424841000000.0; /* Wednesday, February 25, 2015 at 5:10:00 AM GMT */
static const UChar patNoFields[] = { 0x0027, 0x0078, 0x0078, 0x0078, 0x0027, 0 }; /* "'xxx'" */

typedef struct {
    int32_t field;
    int32_t beginPos;
    int32_t endPos;
} FieldsData;
static const FieldsData expectedFields[] = {
    { UDAT_DAY_OF_WEEK_FIELD /* 9*/,      0,  9 },
    { UDAT_MONTH_FIELD /* 2*/,           11, 19 },
    { UDAT_DATE_FIELD /* 3*/,            20, 22 },
    { UDAT_YEAR_FIELD /* 1*/,            24, 28 },
    { UDAT_HOUR1_FIELD /*15*/,           32, 33 },
#if UDAT_HAS_PATTERN_CHAR_FOR_TIME_SEPARATOR
    { UDAT_TIME_SEPARATOR_FIELD /*35*/,  33, 34 },
#endif
    { UDAT_MINUTE_FIELD /* 6*/,          34, 36 },
#if UDAT_HAS_PATTERN_CHAR_FOR_TIME_SEPARATOR
    { UDAT_TIME_SEPARATOR_FIELD /*35*/,  36, 37 },
#endif
    { UDAT_SECOND_FIELD /* 7*/,          37, 39 },
    { UDAT_AM_PM_FIELD /*14*/,           40, 42 },
    { UDAT_TIMEZONE_FIELD /*17*/,        43, 46 },
    { -1,                                -1, -1 },
};

enum {kUBufFieldsLen = 128, kBBufFieldsLen = 256 };

static void TestFormatForFields(void) {
    UErrorCode status = U_ZERO_ERROR;
    UFieldPositionIterator* fpositer = ufieldpositer_open(&status);
    if ( U_FAILURE(status) ) {
        log_err("ufieldpositer_open fails, status %s\n", u_errorName(status));
    } else {
        UDateFormat* udfmt = udat_open(UDAT_LONG, UDAT_FULL, localeForFields, zoneGMT, -1, NULL, 0, &status);
        UCalendar* ucal = ucal_open(zoneGMT, -1, localeForFields, UCAL_DEFAULT, &status);
        if ( U_FAILURE(status) ) {
            log_data_err("udat_open or ucal_open fails for locale %s, status %s (Are you missing data?)\n", localeForFields, u_errorName(status));
        } else {
            int32_t ulen, field, beginPos, endPos;
            UChar ubuf[kUBufFieldsLen];
            const FieldsData * fptr;

            status = U_ZERO_ERROR;
            ulen = udat_formatForFields(udfmt, date2015Feb25, ubuf, kUBufFieldsLen, fpositer, &status);
            if ( U_FAILURE(status) ) {
                log_err("udat_formatForFields fails, status %s\n", u_errorName(status));
            } else {
                for (fptr = expectedFields; ; fptr++) {
                    field = ufieldpositer_next(fpositer, &beginPos, &endPos);
                    if (field != fptr->field || (field >= 0 && (beginPos != fptr->beginPos || endPos != fptr->endPos))) {
                        if (fptr->field >= 0) {
                            log_err("udat_formatForFields as \"%s\"; expect field %d range %d-%d, get field %d range %d-%d\n",
                                    aescstrdup(ubuf, ulen), fptr->field, fptr->beginPos, fptr->endPos, field, beginPos, endPos);
                        } else {
                            log_err("udat_formatForFields as \"%s\"; expect field < 0, get field %d range %d-%d\n",
                                    aescstrdup(ubuf, ulen), field, beginPos, endPos);
                        }
                        break;
                    }
                    if (field < 0) {
                        break;
                    }
                }
            }

            ucal_setMillis(ucal, date2015Feb25, &status);
            status = U_ZERO_ERROR;
            ulen = udat_formatCalendarForFields(udfmt, ucal, ubuf, kUBufFieldsLen, fpositer, &status);
            if ( U_FAILURE(status) ) {
                log_err("udat_formatCalendarForFields fails, status %s\n", u_errorName(status));
            } else {
                for (fptr = expectedFields; ; fptr++) {
                    field = ufieldpositer_next(fpositer, &beginPos, &endPos);
                    if (field != fptr->field || (field >= 0 && (beginPos != fptr->beginPos || endPos != fptr->endPos))) {
                        if (fptr->field >= 0) {
                            log_err("udat_formatFudat_formatCalendarForFieldsorFields as \"%s\"; expect field %d range %d-%d, get field %d range %d-%d\n",
                                    aescstrdup(ubuf, ulen), fptr->field, fptr->beginPos, fptr->endPos, field, beginPos, endPos);
                        } else {
                            log_err("udat_formatCalendarForFields as \"%s\"; expect field < 0, get field %d range %d-%d\n",
                                    aescstrdup(ubuf, ulen), field, beginPos, endPos);
                        }
                        break;
                    }
                    if (field < 0) {
                        break;
                    }
                }
            }

            udat_applyPattern(udfmt, FALSE, patNoFields, -1);
            status = U_ZERO_ERROR;
            ulen = udat_formatForFields(udfmt, date2015Feb25, ubuf, kUBufFieldsLen, fpositer, &status);
            if ( U_FAILURE(status) ) {
                log_err("udat_formatForFields with no-field pattern fails, status %s\n", u_errorName(status));
            } else {
                field = ufieldpositer_next(fpositer, &beginPos, &endPos);
                if (field >= 0) {
                    log_err("udat_formatForFields with no-field pattern as \"%s\"; expect field < 0, get field %d range %d-%d\n",
                            aescstrdup(ubuf, ulen), field, beginPos, endPos);
                }
            }

            ucal_close(ucal);
            udat_close(udfmt);
        }
        ufieldpositer_close(fpositer);
    }
}

static void TestForceGannenNumbering(void) {
    UErrorCode status;
    const char* locID = "ja_JP@calendar=japanese";
    UDate refDate = 600336000000.0; // 1989 Jan 9 Monday = Heisei 1
    const UChar* testSkeleton = u"yMMMd";

    // Test Gannen year forcing
    status = U_ZERO_ERROR;
    UDateTimePatternGenerator* dtpgen = udatpg_open(locID, &status);
    if (U_FAILURE(status)) {
        log_data_err("Fail in udatpg_open locale %s: %s", locID, u_errorName(status));
    } else {
        UChar pattern[kUbufMax];
        int32_t patlen = udatpg_getBestPattern(dtpgen, testSkeleton, -1, pattern, kUbufMax, &status);
        if (U_FAILURE(status)) {
            log_data_err("Fail in udatpg_getBestPattern locale %s: %s", locID, u_errorName(status));
        } else  {
            UDateFormat *testFmt = udat_open(UDAT_PATTERN, UDAT_PATTERN, locID, NULL, 0, pattern, patlen, &status);
            if (U_FAILURE(status)) {
                log_data_err("Fail in udat_open locale %s: %s", locID, u_errorName(status));
            } else {
                UChar testString[kUbufMax];
                int32_t testStrLen = udat_format(testFmt, refDate, testString, kUbufMax, NULL, &status);
                if (U_FAILURE(status)) {
                    log_err("Fail in udat_format locale %s: %s", locID, u_errorName(status));
                } else if (testStrLen < 3 || testString[2] != 0x5143) {
                    char bbuf[kBbufMax];
                    u_austrncpy(bbuf, testString, testStrLen);
                    log_err("Formatting year 1 as Gannen, got%s but expected 3rd char to be 0x5143", bbuf);
                }
                udat_close(testFmt);
            }
        }
        udatpg_close(dtpgen);
    }
}

/* defined above
static const UChar zoneGMT[] = { 0x47,0x4D,0x54,0 }; // "GMT"
static const UDate date2015Feb25 = 1424841000000.0; // Wednesday, February 25, 2015 at 5:10:00 AM GMT
*/

typedef struct {
    const char * locale;
    UDateFormatStyle dateStyle;
    UDateFormatStyle timeStyle;
    const char * expect; /* for zoneGMT and date2015Feb25 */
} StandardPatternItem;

static const StandardPatternItem stdPatternItems[] = {
    { "en_JP", UDAT_MEDIUM, UDAT_SHORT, "Feb 25, 2015 5:10" },
    { "en_CN", UDAT_MEDIUM, UDAT_SHORT, "Feb 25, 2015 at 5:10 AM" },
    { "en_TW", UDAT_MEDIUM, UDAT_SHORT, "Feb 25, 2015 at 5:10 AM" },
    { "en_KR", UDAT_MEDIUM, UDAT_SHORT, "Feb 25, 2015 5:10 AM" },
    // Add tests for Apple <rdar://problem/51014042>; currently no specific locales for these
    { "en_AZ", UDAT_MEDIUM, UDAT_SHORT, "Feb 25, 2015 at 05:10" }, // en uses h, AZ Azerbaijanvpref cycle H
    { "fr_US", UDAT_NONE, UDAT_SHORT, "5:10 AM" }, // fr uses H, US pref cycle h
    { "rkt",   UDAT_NONE, UDAT_SHORT, "5:10 AM" }, // rkt (no locale) => rkt_Beng_BD, BD pref cycle h unlike root H
    // Add tests for Apple <rdar://problem/47494884>
    { "ur_PK",      UDAT_MEDIUM, UDAT_SHORT, "25 \\u0641\\u0631\\u0648\\u060C 2015 5:10 \\u0642.\\u062F." },
    { "ur_IN",      UDAT_MEDIUM, UDAT_SHORT, "\\u06F2\\u06F5 \\u0641\\u0631\\u0648\\u060C \\u06F2\\u06F0\\u06F1\\u06F5 \\u06F5:\\u06F1\\u06F0 \\u0642.\\u062F." },
    { "ur_Arab",    UDAT_MEDIUM, UDAT_SHORT, "25 \\u0641\\u0631\\u0648\\u060C 2015 5:10 \\u0642.\\u062F." },
    { "ur_Aran",    UDAT_MEDIUM, UDAT_SHORT, "25 \\u0641\\u0631\\u0648\\u060C 2015 5:10 \\u0642.\\u062F." },
    { "ur_Arab_PK", UDAT_MEDIUM, UDAT_SHORT, "25 \\u0641\\u0631\\u0648\\u060C 2015 5:10 \\u0642.\\u062F." },
    { "ur_Aran_PK", UDAT_MEDIUM, UDAT_SHORT, "25 \\u0641\\u0631\\u0648\\u060C 2015 5:10 \\u0642.\\u062F." },
    { "ur_Arab_IN", UDAT_MEDIUM, UDAT_SHORT, "\\u06F2\\u06F5 \\u0641\\u0631\\u0648\\u060C \\u06F2\\u06F0\\u06F1\\u06F5 \\u06F5:\\u06F1\\u06F0 \\u0642.\\u062F." },
    { "ur_Aran_IN", UDAT_MEDIUM, UDAT_SHORT, "\\u06F2\\u06F5 \\u0641\\u0631\\u0648\\u060C \\u06F2\\u06F0\\u06F1\\u06F5 \\u06F5:\\u06F1\\u06F0 \\u0642.\\u062F." },
    { NULL, (UDateFormatStyle)0, (UDateFormatStyle)0, NULL } /* terminator */
};

enum { kUbufStdMax = 64, kBbufStdMax = 3*kUbufStdMax };

static void TestStandardPatterns(void) {
    const StandardPatternItem* itemPtr;
    for (itemPtr = stdPatternItems; itemPtr->locale != NULL; itemPtr++) {
        UErrorCode status = U_ZERO_ERROR;
        UDateFormat* udfmt = udat_open(itemPtr->timeStyle, itemPtr->dateStyle, itemPtr->locale, zoneGMT, -1, NULL, 0, &status);
        if ( U_FAILURE(status) ) {
            log_err("udat_open(%d, %d, \"%s\",...) fails, status %s\n",
                    (int)itemPtr->timeStyle, (int)itemPtr->dateStyle, itemPtr->locale, u_errorName(status));
        } else {
            UChar uget[kUbufStdMax];
            int32_t ugetlen = udat_format(udfmt, date2015Feb25, uget, kUbufStdMax, NULL, &status);
            if ( U_FAILURE(status) ) {
                log_err("udat_format for (%d, %d, \"%s\",...) fails, status %s\n",
                        (int)itemPtr->timeStyle, (int)itemPtr->dateStyle, itemPtr->locale, u_errorName(status));
            } else {
                UChar uexpect[kUbufStdMax];
                int32_t uexpectlen = u_unescape(itemPtr->expect, uexpect, kUbufStdMax);
                if (ugetlen != uexpectlen || u_strncmp(uget, uexpect, uexpectlen) != 0) {
                    char bexpect[kBbufStdMax];
                    char bget[kBbufStdMax];
                    u_austrcpy(bexpect, uexpect);
                    u_austrcpy(bget,    uget);
                    log_err("udat_format for (%d, %d, \"%s\",...):\n    expect %s\n    get    %s\n",
                        (int)itemPtr->timeStyle, (int)itemPtr->dateStyle, itemPtr->locale, bexpect, bget);
                }
            }
            udat_close(udfmt);
        }
    }
}

/* defined above
static const UChar zoneGMT[] = { 0x47,0x4D,0x54,0 }; // "GMT"
static const UDate date2015Feb25 = 1424841000000.0; // Wednesday, February 25, 2015 at 5:10:00 AM GMT
*/
static const UChar patternHmm[]   = { 0x48,0x3A,0x6D,0x6D,0 }; /* "H:mm" */
static const UChar formattedHmm[] = { 0x35,0x3A,0x31,0x30,0 }; /* "5:10" */

enum { kUBufOverrideSepMax = 32, kBBufOverrideSepMax = 64 };

static void TestApplyPatnOverridesTimeSep(void) {
    UErrorCode status;
    UDateFormat* udfmt;
    const char *locale = "da"; /* uses period for time separator */
    UChar ubuf[kUBufOverrideSepMax];
    int32_t ulen;
    
    status = U_ZERO_ERROR;
    udfmt = udat_open(UDAT_PATTERN, UDAT_PATTERN, locale, zoneGMT, -1, patternHmm, -1, &status);
    if ( U_FAILURE(status) ) {
        log_err("udat_open(UDAT_PATTERN, UDAT_PATTERN, \"%s\",...) fails, status %s\n", locale, u_errorName(status));
    } else {
        ulen = udat_format(udfmt, date2015Feb25, ubuf, kUBufOverrideSepMax, NULL, &status);
        if ( U_FAILURE(status) ) {
            log_err("udat_format fails for UDAT_PATTERN \"%s\", status %s\n", locale, u_errorName(status));
        } else if (u_strcmp(ubuf, formattedHmm) != 0) {
            char bbuf[kBBufOverrideSepMax];
            u_strToUTF8(bbuf, kBBufOverrideSepMax, NULL, ubuf, ulen, &status);
            log_err("udat_format fails for UDAT_PATTERN \"%s\", expected 5:10, got %s\n", locale, bbuf);
        }
        udat_close(udfmt);
    }
    
    status = U_ZERO_ERROR;
    udfmt = udat_open(UDAT_SHORT, UDAT_NONE, locale, zoneGMT, -1, NULL, 0, &status);
    if ( U_FAILURE(status) ) {
        log_err("udat_open(UDAT_SHORT, UDAT_NONE, \"%s\",...) fails, status %s\n", locale, u_errorName(status));
    } else {
        udat_applyPattern(udfmt, FALSE, patternHmm, -1);
        ulen = udat_format(udfmt, date2015Feb25, ubuf, kUBufOverrideSepMax, NULL, &status);
        if ( U_FAILURE(status) ) {
            log_err("udat_format fails for UDAT_SHORT \"%s\" + applyPattern, status %s\n", locale, u_errorName(status));
        } else if (u_strcmp(ubuf, formattedHmm) != 0) {
            char bbuf[kBBufOverrideSepMax];
            u_strToUTF8(bbuf, kBBufOverrideSepMax, NULL, ubuf, ulen, &status);
            log_err("udat_format fails for UDAT_SHORT \"%s\" + applyPattern, expected 5:10, got %s\n", locale, bbuf);
        }
        udat_close(udfmt);
    }
    
}

#define UDATE_SECOND (1000.0)
#define UDATE_MINUTE (60.0*UDATE_SECOND)
#define UDATE_HOUR   (60.0*UDATE_MINUTE)

static const double dayOffsets[] = {
    0.0,                                    /* 00:00:00 */
    UDATE_SECOND,                           /* 00:00:01 */
    UDATE_MINUTE,                           /* 00:01:00 */
    UDATE_HOUR,                             /* 01:00:00 */
    11.0*UDATE_HOUR + 59.0*UDATE_MINUTE,    /* 11:59:00 */
    12.0*UDATE_HOUR,                        /* 12:00:00 */
    12.0*UDATE_HOUR + UDATE_SECOND,         /* 12:00:01 */
    12.0*UDATE_HOUR + UDATE_MINUTE,         /* 12:01:00 */
    13.0*UDATE_HOUR,                        /* 13:00:00 */
    23.0*UDATE_HOUR + 59.0*UDATE_MINUTE,    /* 23:59:00 */
};
enum { kNumDayOffsets = UPRV_LENGTHOF(dayOffsets) };

static const char* ja12HrFmt_hm[kNumDayOffsets] = { /* aK:mm */
    "\\u5348\\u524D0:00", /* "午前0:00" */
    "\\u5348\\u524D0:00",
    "\\u5348\\u524D0:01",
    "\\u5348\\u524D1:00",
    "\\u5348\\u524D11:59",
    "\\u5348\\u5F8C0:00", /* "午後0:00" */
    "\\u5348\\u5F8C0:00",
    "\\u5348\\u5F8C0:01", /* "午後0:01" */
    "\\u5348\\u5F8C1:00",
    "\\u5348\\u5F8C11:59",
};

static const char* ja12HrFmt_h[kNumDayOffsets] = { /* aK時 */
    "\\u5348\\u524D0\\u6642", /* "午前0時" */
    "\\u5348\\u524D0\\u6642",
    "\\u5348\\u524D0\\u6642",
    "\\u5348\\u524D1\\u6642",
    "\\u5348\\u524D11\\u6642",
    "\\u5348\\u5F8C0\\u6642", /* "午後0時" */
    "\\u5348\\u5F8C0\\u6642",
    "\\u5348\\u5F8C0\\u6642", /* "午後0時" */
    "\\u5348\\u5F8C1\\u6642",
    "\\u5348\\u5F8C11\\u6642",
};
typedef struct {
    const char*   locale;
    const char*   skeleton;
    const char ** expected;
} Test12HrFmtItem;

static const Test12HrFmtItem test12HrFmtItems[] = {
    { "ja", "hm", ja12HrFmt_hm },
    { "ja", "h",  ja12HrFmt_h  },
    { NULL, NULL, NULL } /* terminator */
};

enum { kUBufMax = 128, };
static void Test12HrFormats(void) {
    const Test12HrFmtItem* itemPtr;
    for (itemPtr = test12HrFmtItems; itemPtr->locale != NULL; itemPtr++) {
        UErrorCode status = U_ZERO_ERROR;
        UCalendar* ucal = ucal_open(NULL, 0, itemPtr->locale, UCAL_DEFAULT, &status);
        if ( U_FAILURE(status) ) {
            log_data_err("ucal_open fails for locale %s: status %s (Are you missing data?)\n", itemPtr->locale, u_errorName(status));
        } else {
            ucal_clear(ucal);
            ucal_setDateTime(ucal, 2016, UCAL_JANUARY, 1, 0, 0, 0, &status);
            UDate baseDate = ucal_getMillis(ucal, &status);
            if ( U_FAILURE(status) ) {
                log_err("ucal_setDateTime or ucal_getMillis fails for locale %s: status %s\n", itemPtr->locale, u_errorName(status));
            } else {
                UDateTimePatternGenerator* udatpg = udatpg_open(itemPtr->locale, &status);
                if ( U_FAILURE(status) ) {
                    log_data_err("udatpg_open fails for locale %s: status %s (Are you missing data?)\n", itemPtr->locale, u_errorName(status));
                } else {
                    UChar ubuf1[kUbufMax], ubuf2[kUbufMax];
                    int32_t ulen1 = u_unescape(itemPtr->skeleton, ubuf1, kUbufMax);
                    int32_t ulen2 = udatpg_getBestPattern(udatpg, ubuf1, ulen1, ubuf2, kUbufMax, &status);
                    if ( U_FAILURE(status) ) {
                        log_err("udatpg_getBestPattern fails for locale %s, skeleton %s: status %s\n",
                                itemPtr->locale, itemPtr->skeleton, u_errorName(status));
                    } else {
                        UDateFormat* udat = udat_open(UDAT_PATTERN, UDAT_PATTERN, itemPtr->locale, NULL, 0, ubuf2, ulen2, &status);
                        if ( U_FAILURE(status) ) {
                            log_data_err("udat_open fails for locale %s, skeleton %s: status %s (Are you missing data?)\n",
                                    itemPtr->locale, itemPtr->skeleton, u_errorName(status));
                        } else {
                            int32_t iDayOffset;
                            for (iDayOffset = 0; iDayOffset < kNumDayOffsets; iDayOffset++) {
                                status = U_ZERO_ERROR;
                                ulen1 = udat_format(udat, baseDate + dayOffsets[iDayOffset], ubuf1, kUbufMax, NULL, &status);
                                if ( U_FAILURE(status) ) {
                                    log_err("udat_format fails for locale %s, skeleton %s, iDayOffset %d: status %s\n",
                                            itemPtr->locale, itemPtr->skeleton, iDayOffset, u_errorName(status));
                                } else {
                                    ulen2 = u_unescape(itemPtr->expected[iDayOffset], ubuf2, kUbufMax);
                                    if (ulen1 != ulen2 || u_strncmp(ubuf1, ubuf2, ulen2) != 0) {
                                        char bbuf1[kBbufMax], bbuf2[kBbufMax];
                                        u_austrncpy(bbuf1, ubuf1, ulen1);
                                        u_austrncpy(bbuf2, ubuf2, ulen2);
                                        log_err("udat_format fails for locale %s, skeleton %s, iDayOffset %d:\n    expect %s\n    get    %s\n",
                                                itemPtr->locale, itemPtr->skeleton, iDayOffset, bbuf2, bbuf1);
                                    }
                                }
                                
                            }
                            udat_close(udat);
                        }
                    }
                    udatpg_close(udatpg);
                }
            }
            ucal_close(ucal);
        }
    }
}

#if !U_PLATFORM_HAS_WIN32_API

typedef struct {
    const char*           locale;
    UATimeUnitTimePattern patType;
    const char*           expect; /* universal char subset + escaped Unicode chars */
} TimePatternItem;
static const TimePatternItem timePatternItems[] = {
    { "en", UATIMEUNITTIMEPAT_HM,  "h:mm"    },
    { "en", UATIMEUNITTIMEPAT_HMS, "h:mm:ss" },
    { "en", UATIMEUNITTIMEPAT_MS,  "m:ss"    },
    { "da", UATIMEUNITTIMEPAT_HM,  "h.mm"    },
    { "da", UATIMEUNITTIMEPAT_HMS, "h.mm.ss" },
    { "da", UATIMEUNITTIMEPAT_MS,  "m.ss"    },
    { NULL, 0,                     NULL      }
};

typedef struct {
    const char*           locale;
    UATimeUnitStyle       width;
    UATimeUnitListPattern patType;
    const char*           expect; /* universal char subset + escaped Unicode chars */
} ListPatternItem;
static const ListPatternItem listPatternItems[] = {
    { "en", UATIMEUNITSTYLE_FULL,   UATIMEUNITLISTPAT_TWO_ONLY,     "{0}, {1}"   },
    { "en", UATIMEUNITSTYLE_FULL,   UATIMEUNITLISTPAT_END_PIECE,    "{0}, {1}"   },
    { "en", UATIMEUNITSTYLE_FULL,   UATIMEUNITLISTPAT_MIDDLE_PIECE, "{0}, {1}"   },
    { "en", UATIMEUNITSTYLE_FULL,   UATIMEUNITLISTPAT_START_PIECE,  "{0}, {1}"   },
    { "en", UATIMEUNITSTYLE_NARROW, UATIMEUNITLISTPAT_TWO_ONLY,     "{0} {1}"    },
    { "en", UATIMEUNITSTYLE_NARROW, UATIMEUNITLISTPAT_END_PIECE,    "{0} {1}"    },
    { "en", UATIMEUNITSTYLE_NARROW, UATIMEUNITLISTPAT_MIDDLE_PIECE, "{0} {1}"    },
    { "en", UATIMEUNITSTYLE_NARROW, UATIMEUNITLISTPAT_START_PIECE,  "{0} {1}"    },
    { "en", UATIMEUNITSTYLE_SHORTER, UATIMEUNITLISTPAT_TWO_ONLY,    "{0} {1}"    },
    { "fr", UATIMEUNITSTYLE_FULL,   UATIMEUNITLISTPAT_TWO_ONLY,     "{0} et {1}" },
    { "fr", UATIMEUNITSTYLE_FULL,   UATIMEUNITLISTPAT_END_PIECE,    "{0} et {1}" },
    { "fr", UATIMEUNITSTYLE_FULL,   UATIMEUNITLISTPAT_MIDDLE_PIECE, "{0}, {1}"   },
    { "fr", UATIMEUNITSTYLE_FULL,   UATIMEUNITLISTPAT_START_PIECE,  "{0}, {1}"   },
    { "fr", UATIMEUNITSTYLE_NARROW, UATIMEUNITLISTPAT_TWO_ONLY,     "{0} {1}"    },
    { "fr", UATIMEUNITSTYLE_NARROW, UATIMEUNITLISTPAT_END_PIECE,    "{0} {1}"    },
    { "fr", UATIMEUNITSTYLE_NARROW, UATIMEUNITLISTPAT_MIDDLE_PIECE, "{0} {1}"    },
    { "fr", UATIMEUNITSTYLE_NARROW, UATIMEUNITLISTPAT_START_PIECE,  "{0} {1}"    },
    { "fr", UATIMEUNITSTYLE_SHORTER, UATIMEUNITLISTPAT_TWO_ONLY,    "{0} {1}"    },
    { NULL, 0,                      0,                              NULL         }
};

enum {kUBufTimeUnitLen = 128, kBBufTimeUnitLen = 256 };

static void TestTimeUnitFormat(void) { /* Apple-specific */
    const TimePatternItem* timePatItemPtr;
    const ListPatternItem* listPatItemPtr;
    UChar uActual[kUBufTimeUnitLen];
    UChar uExpect[kUBufTimeUnitLen];

    for (timePatItemPtr = timePatternItems; timePatItemPtr->locale != NULL; timePatItemPtr++) {
        UErrorCode status = U_ZERO_ERROR; 
        int32_t ulenActual = uatmufmt_getTimePattern(timePatItemPtr->locale, timePatItemPtr->patType, uActual, kUBufTimeUnitLen, &status);
        if ( U_FAILURE(status) ) {
            log_err("uatmufmt_getTimePattern for locale %s, patType %d: status %s\n", timePatItemPtr->locale, (int)timePatItemPtr->patType, u_errorName(status));
        } else {
            int32_t ulenExpect = u_unescape(timePatItemPtr->expect, uExpect, kUBufTimeUnitLen);
            if (ulenActual != ulenExpect || u_strncmp(uActual, uExpect, ulenExpect) != 0) {
                char bActual[kBBufTimeUnitLen];
                u_strToUTF8(bActual, kBBufTimeUnitLen, NULL, uActual, ulenActual, &status);
                log_err("uatmufmt_getTimePattern for locale %s, patType %d: unexpected result %s\n", timePatItemPtr->locale, (int)timePatItemPtr->patType, bActual);
            }
        }
    }

    for (listPatItemPtr = listPatternItems; listPatItemPtr->locale != NULL; listPatItemPtr++) {
        UErrorCode status = U_ZERO_ERROR; 
        int32_t ulenActual = uatmufmt_getListPattern(listPatItemPtr->locale, listPatItemPtr->width, listPatItemPtr->patType, uActual, kUBufTimeUnitLen, &status);
        if ( U_FAILURE(status) ) {
            log_err("uatmufmt_getListPattern for locale %s, width %d, patType %d: status %s\n", listPatItemPtr->locale, (int)listPatItemPtr->width, (int)listPatItemPtr->patType, u_errorName(status));
        } else {
            int32_t ulenExpect = u_unescape(listPatItemPtr->expect, uExpect, kUBufTimeUnitLen);
            if (ulenActual != ulenExpect || u_strncmp(uActual, uExpect, ulenExpect) != 0) {
                char bActual[kBBufTimeUnitLen];
                u_strToUTF8(bActual, kBBufTimeUnitLen, NULL, uActual, ulenActual, &status);
                log_err("uatmufmt_getListPattern for locale %s, width %d, patType %d: unexpected result %s\n", listPatItemPtr->locale, (int)listPatItemPtr->width, (int)listPatItemPtr->patType, bActual);
            }
        }
    }

}

typedef struct {
    const char*         locale;
    UNumberFormatStyle  numStyle;
    UATimeUnitStyle     width;
    UATimeUnitField     field;
    double              value;
    const UChar*        expect;
} TimeUnitWithNumStyleItem;
static const TimeUnitWithNumStyleItem tuNumStyleItems[] = {
    { "en_US", UNUM_PATTERN_DECIMAL/*0*/,  UATIMEUNITSTYLE_FULL/*0*/, UATIMEUNITFIELD_SECOND/*6*/, 0.0, u"0 seconds" },
    { "en_US", UNUM_DECIMAL,               UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"0 seconds" },
    { "en_US", UNUM_CURRENCY,              UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"$0.00 seconds" },
    { "en_US", UNUM_PERCENT,               UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"0% seconds" },
    { "en_US", UNUM_SCIENTIFIC/*4*/,       UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"0E0 seconds" },
    { "en_US", UNUM_SPELLOUT/*5*/,         UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"zero seconds" }, // uses RuleBasedNumberFormat, got U_UNSUPPORTED_ERROR
    { "en_US", UNUM_ORDINAL,               UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"0th seconds" },  // uses RuleBasedNumberFormat, got U_UNSUPPORTED_ERROR
    { "en_US", UNUM_DURATION,              UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"0 sec. seconds" }, // uses RuleBasedNumberFormat, got U_UNSUPPORTED_ERROR
    { "en_US", UNUM_NUMBERING_SYSTEM/*8*/, UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"௦ seconds" },    // uses RuleBasedNumberFormat, got U_UNSUPPORTED_ERROR
    // skip UNUM_PATTERN_RULEBASED/*9*/                                                                                    // uses RuleBasedNumberFormat
    // { "en_US", UNUM_CURRENCY_ISO/*10*/,    UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"USD 0.00 seconds" }, // currently produces u"USD 0.0 seconds0"
    { "en_US", UNUM_CURRENCY_PLURAL,       UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"0.00 US dollars seconds" },
    { "en_US", UNUM_CURRENCY_ACCOUNTING,   UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"$0.00 seconds" },
    { "en_US", UNUM_CASH_CURRENCY,         UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"$0.00 seconds" },
    { "en_US", UNUM_DECIMAL_COMPACT_SHORT, UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"0 seconds" },
    { "en_US", UNUM_DECIMAL_COMPACT_LONG,  UATIMEUNITSTYLE_FULL,      UATIMEUNITFIELD_SECOND,      0.0, u"0 seconds" },
    { "en_US", UNUM_CURRENCY_STANDARD/*16*/,UATIMEUNITSTYLE_FULL,     UATIMEUNITFIELD_SECOND,      0.0, u"$0.00 seconds" },
    { NULL, 0, 0, 0, 0.0, NULL }
};

enum { kBBufMax = 196 };

static void TestTimeUnitFormatWithNumStyle(void) { /* Apple-specific */
    const TimeUnitWithNumStyleItem* itemPtr;
    for (itemPtr = tuNumStyleItems; itemPtr->locale != NULL; itemPtr++) {
        UErrorCode status = U_ZERO_ERROR;
        UNumberFormat *numFormat = unum_open(itemPtr->numStyle, NULL, 0, itemPtr->locale, NULL, &status);
        if ( U_FAILURE(status) ) {
            log_data_err("unum_open for locale %s, style %d: status %s\n", itemPtr->locale, itemPtr->numStyle, u_errorName(status));
        } else {
            UATimeUnitFormat *tuFormat = uatmufmt_openWithNumberFormat(itemPtr->locale, itemPtr->width, numFormat, &status);
            if ( U_FAILURE(status) ) {
                log_data_err("uatmufmt_openWithNumberFormat for locale %s, numStyle %d, width %d: status %s\n", itemPtr->locale, itemPtr->numStyle, itemPtr->width, u_errorName(status));
            } else {
                UChar ubuf[kUBufMax] = {0};
                int32_t ulen = uatmufmt_format(tuFormat, itemPtr->value, itemPtr->field, ubuf, kUBufMax, &status);
                if ( U_FAILURE(status) ) {
                    log_err("uatmufmt_format for locale %s, numStyle %d, width %d, field %d: status %s\n", itemPtr->locale, itemPtr->numStyle, itemPtr->width, itemPtr->field, u_errorName(status));
                } else if (u_strcmp(ubuf, itemPtr->expect) != 0) {
                    char bbufExp[kBBufMax];
                    char bbufGet[kBBufMax];
                    u_strToUTF8(bbufExp, kBBufMax, NULL, itemPtr->expect, -1, &status);
                    u_strToUTF8(bbufGet, kBBufMax, NULL, ubuf, ulen, &status);
                    log_err("uatmufmt_format for locale %s, numStyle %d, width %d, field %d:\n  expect %s\n  get    %s\n", itemPtr->locale, itemPtr->numStyle, itemPtr->width, itemPtr->field, bbufExp, bbufGet);
                }
                uatmufmt_close(tuFormat);
            }
        }
    }
}

#endif

typedef enum RemapTesttype {
    REMAP_TESTTYPE_FULL     = UDAT_FULL,       // 0
    REMAP_TESTTYPE_LONG     = UDAT_LONG,       // 1
    REMAP_TESTTYPE_MEDIUM   = UDAT_MEDIUM,     // 2
    REMAP_TESTTYPE_SHORT    = UDAT_SHORT,      // 3
    REMAP_TESTTYPE_LONG_DF  = UDAT_LONG + 4,   // 5 long time, full date
    REMAP_TESTTYPE_SHORT_DS = UDAT_SHORT + 16, // 3 short time, short date
    REMAP_TESTTYPE_SKELETON = -1,
    REMAP_TESTTYPE_PATTERN  = -2,
} RemapTesttype;

typedef struct {
    const char *  pattern;
    RemapTesttype testtype;
    uint32_t      options;
} RemapPatternTestItem;

static const RemapPatternTestItem remapPatItems[] = {
    { "full",                               REMAP_TESTTYPE_FULL,       0                                                            },
    { "full",                               REMAP_TESTTYPE_FULL,       UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "full",                               REMAP_TESTTYPE_FULL,       UADATPG_FORCE_12_HOUR_CYCLE                                  },
    { "long",                               REMAP_TESTTYPE_LONG,       0                                                            },
    { "long",                               REMAP_TESTTYPE_LONG,       UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "long",                               REMAP_TESTTYPE_LONG,       UADATPG_FORCE_12_HOUR_CYCLE                                  },
    { "medium",                             REMAP_TESTTYPE_MEDIUM,     0                                                            },
    { "medium",                             REMAP_TESTTYPE_MEDIUM,     UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "medium",                             REMAP_TESTTYPE_MEDIUM,     UADATPG_FORCE_12_HOUR_CYCLE                                  },
    { "short",                              REMAP_TESTTYPE_SHORT,      0                                                            },
    { "short",                              REMAP_TESTTYPE_SHORT,      UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "short",                              REMAP_TESTTYPE_SHORT,      UADATPG_FORCE_12_HOUR_CYCLE                                  },
    { "long_df",                            REMAP_TESTTYPE_LONG_DF,    0                                                            },
    { "long_df",                            REMAP_TESTTYPE_LONG_DF,    UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "long_df",                            REMAP_TESTTYPE_LONG_DF,    UADATPG_FORCE_12_HOUR_CYCLE                                  },
    { "short_ds",                           REMAP_TESTTYPE_SHORT_DS,   0                                                            },
    { "short_ds",                           REMAP_TESTTYPE_SHORT_DS,   UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "short_ds",                           REMAP_TESTTYPE_SHORT_DS,   UADATPG_FORCE_12_HOUR_CYCLE                                  },

    { "jmmss",                              REMAP_TESTTYPE_SKELETON,   0                                                            },
    { "jmmss",                              REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "jmmss",                              REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_12_HOUR_CYCLE                                  },
    { "jjmmss",                             REMAP_TESTTYPE_SKELETON,   0                                                            },
    { "jjmmss",                             REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "jjmmss",                             REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_24_HOUR_CYCLE | UDATPG_MATCH_HOUR_FIELD_LENGTH },
    { "jjmmss",                             REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_12_HOUR_CYCLE                                  },
    { "jjmmss",                             REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_12_HOUR_CYCLE | UDATPG_MATCH_HOUR_FIELD_LENGTH },
    { "Jmm",                                REMAP_TESTTYPE_SKELETON,   0                                                            },
    { "Jmm",                                REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "Jmm",                                REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_12_HOUR_CYCLE                                  },
    { "jmsv",                               REMAP_TESTTYPE_SKELETON,   0                                                            },
    { "jmsv",                               REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "jmsv",                               REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_12_HOUR_CYCLE                                  },
    { "jmsz",                               REMAP_TESTTYPE_SKELETON,   0                                                            },
    { "jmsz",                               REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_24_HOUR_CYCLE                                  },
    { "jmsz",                               REMAP_TESTTYPE_SKELETON,   UADATPG_FORCE_12_HOUR_CYCLE                                  },

    { "h:mm:ss a",                          REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_12_HOUR_CYCLE }, // 12=hour patterns
    { "h:mm:ss a",                          REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_24_HOUR_CYCLE },
    { "a'xx'h:mm:ss d MMM y",               REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_12_HOUR_CYCLE },
    { "a'xx'h:mm:ss d MMM y",               REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_24_HOUR_CYCLE },
    { "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_12_HOUR_CYCLE },
    { "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_24_HOUR_CYCLE },
    { "EEE, d MMM y 'aha' a'xx'h:mm:ss",    REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_12_HOUR_CYCLE },
    { "EEE, d MMM y 'aha' a'xx'h:mm:ss",    REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_24_HOUR_CYCLE },
    { "yyMMddhhmmss",                       REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_12_HOUR_CYCLE },
    { "yyMMddhhmmss",                       REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_24_HOUR_CYCLE },

    { "H:mm:ss",                            REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_12_HOUR_CYCLE }, // 24=hour patterns
    { "H:mm:ss",                            REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_24_HOUR_CYCLE },
    { "H:mm:ss d MMM y",                    REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_12_HOUR_CYCLE },
    { "H:mm:ss d MMM y",                    REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_24_HOUR_CYCLE },
    { "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_12_HOUR_CYCLE },
    { "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_24_HOUR_CYCLE },
    { "EEE, d MMM y 'aha' H'h'mm'm'ss",     REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_12_HOUR_CYCLE },
    { "EEE, d MMM y 'aha' H'h'mm'm'ss",     REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_24_HOUR_CYCLE },

    // special cases per bugs
    { "uuuu-MM-dd HH:mm:ss '+0000'",        REMAP_TESTTYPE_PATTERN,    UADATPG_FORCE_12_HOUR_CYCLE }, // <rdar://problem/38826484> 

    { NULL,                                 (RemapTesttype)0,          0                           }
};

static const char * remapResults_root[] = {
    "HH:mm:ss zzzz",  // full
    "HH:mm:ss zzzz",  //   force24
    "h:mm:ss a zzzz", //   force12
    "HH:mm:ss z",     // long
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12
    "HH:mm:ss",       // medium
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "HH:mm",          // short
    "HH:mm",          //   force24
    "h:mm a",         //   force12
    "y MMMM d, EEEE HH:mm:ss z",  // long_df
    "y MMMM d, EEEE HH:mm:ss z",  //   force24
    "y MMMM d, EEEE h:mm:ss a z", //   force12
    "y-MM-dd HH:mm",  // short_ds
    "y-MM-dd HH:mm",  //   force24
    "y-MM-dd h:mm a", //   force12

    "HH:mm:ss",       // jmmss
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "HH:mm:ss",       // jjmmss
    "HH:mm:ss",       //   force24
    "HH:mm:ss",       //   force24 | match hour field length
    "h:mm:ss a",      //   force12
    "hh:mm:ss a",     //   force12 | match hour field length
    "HH:mm",          // Jmm
    "HH:mm",          //   force24
    "hh:mm",          //   force12
    "HH:mm:ss v",     // jmsv
    "HH:mm:ss v",     //   force24
    "h:mm:ss a v",    //   force12
    "HH:mm:ss z",     // jmsz
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "HH:mm:ss",                           //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "HH:mm:ss d MMM y",                   //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "h:mm:ss a",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "h:mm:ss a d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' h'h'mm'm'ss a",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd h:mm:ss a '+0000'",       //

    NULL
};

static const char * remapResults_en[] = {
    "h:mm:ss a zzzz", // full
    "HH:mm:ss zzzz",  //   force24
    "h:mm:ss a zzzz", //   force12
    "h:mm:ss a z",    // long
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12
    "h:mm:ss a",      // medium
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "h:mm a",         // short
    "HH:mm",          //   force24
    "h:mm a",         //   force12
    "EEEE, MMMM d, y 'at' h:mm:ss a z",   // long_df
    "EEEE, MMMM d, y 'at' HH:mm:ss z",    //   force24
    "EEEE, MMMM d, y 'at' h:mm:ss a z",   //   force12
    "M/d/yy, h:mm a", // short_ds
    "M/d/yy, HH:mm",  //   force24
    "M/d/yy, h:mm a", //   force12

    "h:mm:ss a",      // jmmss
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "h:mm:ss a",      // jjmmss
    "HH:mm:ss",       //   force24
    "HH:mm:ss",       //   force24 | match hour field length
    "h:mm:ss a",      //   force12
    "hh:mm:ss a",     //   force12 | match hour field length
    "hh:mm",          // Jmm
    "HH:mm",          //   force24
    "hh:mm",          //   force12
    "h:mm:ss a v",    // jmsv
    "HH:mm:ss v",     //   force24
    "h:mm:ss a v",    //   force12
    "h:mm:ss a z",    // jmsz
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "HH:mm:ss",                           //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "HH:mm:ss d MMM y",                   //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "h:mm:ss a",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "h:mm:ss a d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' h'h'mm'm'ss a",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd h:mm:ss a '+0000'",       //

    NULL
};

static const char * remapResults_ja[] = {
    "H\\u6642mm\\u5206ss\\u79D2 zzzz",    // full
    "H\\u6642mm\\u5206ss\\u79D2 zzzz",    //   force24
    "aK:mm:ss zzzz",                      //   force12
    "H:mm:ss z",     // long
    "H:mm:ss z",     //   force24
    "aK:mm:ss z",    //   force12
    "H:mm:ss",       // medium
    "H:mm:ss",       //   force24
    "aK:mm:ss",      //   force12
    "H:mm",          // short
    "H:mm",          //   force24
    "aK:mm",         //   force12
    "y\\u5E74M\\u6708d\\u65E5 EEEE H:mm:ss z",  // long_df
    "y\\u5E74M\\u6708d\\u65E5 EEEE H:mm:ss z",  //   force24
    "y\\u5E74M\\u6708d\\u65E5 EEEE aK:mm:ss z", //   force12
    "y/MM/dd H:mm",  // short_ds
    "y/MM/dd H:mm",  //   force24
    "y/MM/dd aK:mm", //   force12

    "H:mm:ss",       // jmmss
    "H:mm:ss",       //   force24
    "aK:mm:ss",      //   force12
    "H:mm:ss",       // jjmmss
    "H:mm:ss",       //   force24
    "HH:mm:ss",      //   force24 | match hour field length
    "aK:mm:ss",      //   force12
    "aKK:mm:ss",     //   force12 | match hour field length
    "H:mm",          // Jmm
    "H:mm",          //   force24
    "h:mm",          //   force12
    "H:mm:ss v",     // jmsv
    "H:mm:ss v",     //   force24
    "aK:mm:ss v",    //   force12
    "H:mm:ss z",     // jmsz
    "H:mm:ss z",     //   force24
    "aK:mm:ss z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "H:mm:ss",                            //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' H:mm:ss",         //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "aK:mm:ss",                           // "H:mm:ss"
    "H:mm:ss",                            //
    "aK:mm:ss d MMM y",                   // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' aK:mm:ss 'hrs'",  // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' aK'h'mm'm'ss",    // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd aK:mm:ss '+0000'",       //

    NULL
};

static const char * remapResults_ko[] = {
    "a h\\uC2DC m\\uBD84 s\\uCD08 zzzz", // full
    "H\\uC2DC m\\uBD84 s\\uCD08 zzzz",   //   force24
    "a h\\uC2DC m\\uBD84 s\\uCD08 zzzz", //   force12
    "a h\\uC2DC m\\uBD84 s\\uCD08 z",    // long
    "H\\uC2DC m\\uBD84 s\\uCD08 z",      //   force24
    "a h\\uC2DC m\\uBD84 s\\uCD08 z",    //   force12
    "a h:mm:ss",                         // medium
    "HH:mm:ss",                          //   force24
    "a h:mm:ss",                         //   force12
    "a h:mm",                            // short
    "HH:mm",                             //   force24
    "a h:mm",                            //   force12
    "y\\uB144 M\\uC6D4 d\\uC77C EEEE a h\\uC2DC m\\uBD84 s\\uCD08 z",    // long_df
    "y\\uB144 M\\uC6D4 d\\uC77C EEEE H\\uC2DC m\\uBD84 s\\uCD08 z",      //   force24
    "y\\uB144 M\\uC6D4 d\\uC77C EEEE a h\\uC2DC m\\uBD84 s\\uCD08 z",    //   force12
    "y. M. d. a h:mm",                   // short_ds
    "y. M. d. HH:mm",                    //   force24
    "y. M. d. a h:mm",                   //   force12

    "a h:mm:ss",  // jmmss
    "HH:mm:ss",   //   force24
    "a h:mm:ss",  //   force12
    "a h:mm:ss",  // jjmmss
    "HH:mm:ss",   //   force24
    "HH:mm:ss",   //   force24 | match hour field length
    "a h:mm:ss",  //   force12
    "a hh:mm:ss", //   force12 | match hour field length
    "hh:mm",      // Jmm
    "HH:mm",      //   force24
    "hh:mm",      //   force12
    "a h:mm:ss v",                        // jmsv
    "H\\uC2DC m\\uBD84 s\\uCD08 v",       //   force24
    "a h:mm:ss v",                        //   force12
    "a h\\uC2DC m\\uBD84 s\\uCD08 z",     // jmsz
    "H\\uC2DC m\\uBD84 s\\uCD08 z",       //   force24
    "a h\\uC2DC m\\uBD84 s\\uCD08 z",     //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "HH:mm:ss",                           //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "HH:mm:ss d MMM y",                   //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "a h:mm:ss",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "a h:mm:ss d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' a h:mm:ss 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' a h'h'mm'm'ss",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd a h:mm:ss '+0000'",       //

    NULL
};

static const char * remapResults_th[] = {
    "H \\u0E19\\u0E32\\u0E2C\\u0E34\\u0E01\\u0E32 mm \\u0E19\\u0E32\\u0E17\\u0E35 ss \\u0E27\\u0E34\\u0E19\\u0E32\\u0E17\\u0E35 zzzz",  // full
    "H \\u0E19\\u0E32\\u0E2C\\u0E34\\u0E01\\u0E32 mm \\u0E19\\u0E32\\u0E17\\u0E35 ss \\u0E27\\u0E34\\u0E19\\u0E32\\u0E17\\u0E35 zzzz",  //   force24
    "h:mm:ss a zzzz",                                                                                                                   //   force12
    "H \\u0E19\\u0E32\\u0E2C\\u0E34\\u0E01\\u0E32 mm \\u0E19\\u0E32\\u0E17\\u0E35 ss \\u0E27\\u0E34\\u0E19\\u0E32\\u0E17\\u0E35 z",     // long
    "H \\u0E19\\u0E32\\u0E2C\\u0E34\\u0E01\\u0E32 mm \\u0E19\\u0E32\\u0E17\\u0E35 ss \\u0E27\\u0E34\\u0E19\\u0E32\\u0E17\\u0E35 z",     //   force24
    "h:mm:ss a z",                                                                                                                      //   force12
    "HH:mm:ss",       // medium
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "HH:mm",          // short
    "HH:mm",          //   force24
    "h:mm a",         //   force12
    "EEEE\\u0E17\\u0E35\\u0E48 d MMMM G y H \\u0E19\\u0E32\\u0E2C\\u0E34\\u0E01\\u0E32 mm \\u0E19\\u0E32\\u0E17\\u0E35 ss \\u0E27\\u0E34\\u0E19\\u0E32\\u0E17\\u0E35 z", // long_df
    "EEEE\\u0E17\\u0E35\\u0E48 d MMMM G y H \\u0E19\\u0E32\\u0E2C\\u0E34\\u0E01\\u0E32 mm \\u0E19\\u0E32\\u0E17\\u0E35 ss \\u0E27\\u0E34\\u0E19\\u0E32\\u0E17\\u0E35 z", //   force24
    "EEEE\\u0E17\\u0E35\\u0E48 d MMMM G y h:mm:ss a z",                                                                                                                  //   force12
    "d/M/yy HH:mm",   // short_ds
    "d/M/yy HH:mm",   //   force24
    "d/M/yy h:mm a",  //   force12

    "HH:mm:ss",       // jmmss
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "HH:mm:ss",       // jjmmss
    "HH:mm:ss",       //   force24
    "HH:mm:ss",       //   force24 | match hour field length
    "h:mm:ss a",      //   force12
    "hh:mm:ss a",     //   force12 | match hour field length
    "HH:mm",          // Jmm
    "HH:mm",          //   force24
    "hh:mm",          //   force12
    "H \\u0E19\\u0E32\\u0E2C\\u0E34\\u0E01\\u0E32 mm \\u0E19\\u0E32\\u0E17\\u0E35 ss \\u0E27\\u0E34\\u0E19\\u0E32\\u0E17\\u0E35 v",     // jmsv
    "H \\u0E19\\u0E32\\u0E2C\\u0E34\\u0E01\\u0E32 mm \\u0E19\\u0E32\\u0E17\\u0E35 ss \\u0E27\\u0E34\\u0E19\\u0E32\\u0E17\\u0E35 v",     //   force24
    "h:mm:ss a v",                                                                                                                      //   force12
    "H \\u0E19\\u0E32\\u0E2C\\u0E34\\u0E01\\u0E32 mm \\u0E19\\u0E32\\u0E17\\u0E35 ss \\u0E27\\u0E34\\u0E19\\u0E32\\u0E17\\u0E35 z",     // jmsz
    "H \\u0E19\\u0E32\\u0E2C\\u0E34\\u0E01\\u0E32 mm \\u0E19\\u0E32\\u0E17\\u0E35 ss \\u0E27\\u0E34\\u0E19\\u0E32\\u0E17\\u0E35 z",     //   force24
    "h:mm:ss a z",                                                                                                                      //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "HH:mm:ss",                           //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "HH:mm:ss d MMM y",                   //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "h:mm:ss a",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "h:mm:ss a d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' h'h'mm'm'ss a",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd h:mm:ss a '+0000'",       //

    NULL
};

static const char * remapResults_hi[] = {
    "a h:mm:ss zzzz", // full
    "HH:mm:ss zzzz",  //   force24
    "a h:mm:ss zzzz", //   force12
    "a h:mm:ss z",    // long
    "HH:mm:ss z",     //   force24
    "a h:mm:ss z",    //   force12
    "a h:mm:ss",      // medium
    "HH:mm:ss",       //   force24
    "a h:mm:ss",      //   force12
    "a h:mm",         // short
    "HH:mm",          //   force24
    "a h:mm",         //   force12
    "EEEE, d MMMM y, a h:mm:ss z",    // long_df
    "EEEE, d MMMM y, HH:mm:ss z",     //   force24
    "EEEE, d MMMM y, a h:mm:ss z",    //   force12
    "d/M/yy, a h:mm", // short_ds
    "d/M/yy, HH:mm",  //   force24
    "d/M/yy, a h:mm", //   force12

    "a h:mm:ss",      // jmmss
    "HH:mm:ss",       //   force24
    "a h:mm:ss",      //   force12
    "a h:mm:ss",      // jjmmss
    "HH:mm:ss",       //   force24
    "HH:mm:ss",       //   force24 | match hour field length
    "a h:mm:ss",      //   force12
    "a hh:mm:ss",     //   force12 | match hour field length
    "hh:mm",          // Jmm
    "HH:mm",          //   force24
    "hh:mm",          //   force12
    "a h:mm:ss v",    // jmsv
    "HH:mm:ss v",     //   force24
    "a h:mm:ss v",    //   force12
    "a h:mm:ss z",    // jmsz
    "HH:mm:ss z",     //   force24
    "a h:mm:ss z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "HH:mm:ss",                           //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "HH:mm:ss d MMM y",                   //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "a h:mm:ss",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "a h:mm:ss d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' a h:mm:ss 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' a h'h'mm'm'ss",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd a h:mm:ss '+0000'",       //

    NULL
};

static const char * remapResults_ar[] = {
    "h:mm:ss\\u00A0a zzzz", // full
    "HH:mm:ss zzzz",        //   force24
    "h:mm:ss\\u00A0a zzzz", //   force12
    "h:mm:ss\\u00A0a z",    // long
    "HH:mm:ss z",           //   force24
    "h:mm:ss\\u00A0a z",    //   force12
    "h:mm:ss\\u00A0a",      // medium
    "HH:mm:ss",             //   force24
    "h:mm:ss\\u00A0a",      //   force12
    "h:mm\\u00A0a",         // short
    "HH:mm",                //   force24
    "h:mm\\u00A0a",         //   force12
    "EEEE\\u060C d MMMM\\u060C y\\u060C h:mm:ss\\u00A0a z", // long_df
    "EEEE\\u060C d MMMM\\u060C y\\u060C HH:mm:ss z",        //   force24
    "EEEE\\u060C d MMMM\\u060C y\\u060C h:mm:ss\\u00A0a z", //   force12
    "d\\u200F/M\\u200F/y\\u060C h:mm\\u00A0a",              // short_ds
    "d\\u200F/M\\u200F/y\\u060C HH:mm",                     //   force24
    "d\\u200F/M\\u200F/y\\u060C h:mm\\u00A0a",              //   force12

    "h:mm:ss\\u00A0a",      // jmmss
    "HH:mm:ss",             //   force24
    "h:mm:ss\\u00A0a",      //   force12
    "h:mm:ss\\u00A0a",      // jjmmss
    "HH:mm:ss",             //   force24
    "HH:mm:ss",             //   force24 | match hour field length
    "h:mm:ss\\u00A0a",      //   force12
    "hh:mm:ss\\u00A0a",     //   force12 | match hour field length
    "hh:mm",                // Jmm
    "HH:mm",                //   force24
    "hh:mm",                //   force12
    "h:mm:ss\\u00A0a v",    // jmsv
    "HH:mm:ss v",           //   force24
    "h:mm:ss\\u00A0a v",    //   force12
    "h:mm:ss\\u00A0a z",    // jmsz
    "HH:mm:ss z",           //   force24
    "h:mm:ss\\u00A0a z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "HH:mm:ss",                           //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "HH:mm:ss d MMM y",                   //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "h:mm:ssa",                           // "H:mm:ss" (should there be \\u00A0 before a?)
    "H:mm:ss",                            //
    "h:mm:ssa d MMM y",                   // "H:mm:ss d MMM y" (should there be \\u00A0 before a?)
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ssa 'hrs'",  // "EEE, d MMM y 'aha' H:mm:ss 'hrs'" (should there be \\u00A0 before a?)
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' h'h'mm'm'ssa",    // "EEE, d MMM y 'aha' H'h'mm'm'ss" (should there be \\u00A0 before a?)
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd h:mm:ss\\u00A0a '+0000'", //

    NULL
};

static const char * remapResults_en_IL[] = {
    "H:mm:ss zzzz",   // full
    "H:mm:ss zzzz",   //   force24
    "h:mm:ss a zzzz", //   force12
    "H:mm:ss z",      // long
    "H:mm:ss z",      //   force24
    "h:mm:ss a z",    //   force12
    "H:mm:ss",        // medium
    "H:mm:ss",        //   force24
    "h:mm:ss a",      //   force12
    "H:mm",           // short
    "H:mm",           //   force24
    "h:mm a",         //   force12
    "EEEE, d MMMM y 'at' H:mm:ss z",     // long_df
    "EEEE, d MMMM y 'at' H:mm:ss z",     //   force24
    "EEEE, d MMMM y 'at' h:mm:ss a z",   //   force12
    "dd/MM/y, H:mm",   // short_ds
    "dd/MM/y, H:mm",   //   force24
    "dd/MM/y, h:mm a", //   force12

    "H:mm:ss",        // jmmss
    "H:mm:ss",        //   force24
    "h:mm:ss a",      //   force12
    "H:mm:ss",        // jjmmss
    "H:mm:ss",        //   force24
    "HH:mm:ss",       //   force24 | match hour field length
    "h:mm:ss a",      //   force12
    "hh:mm:ss a",     //   force12 | match hour field length
    "H:mm",           // Jmm
    "H:mm",           //   force24
    "h:mm",           //   force12
    "H:mm:ss v",      // jmsv
    "H:mm:ss v",      //   force24
    "h:mm:ss a v",    //   force12
    "H:mm:ss z",      // jmsz
    "H:mm:ss z",      //   force24
    "h:mm:ss a z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "H:mm:ss",                            //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' H:mm:ss",         //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "h:mm:ss a",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "h:mm:ss a d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' h'h'mm'm'ss a",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd h:mm:ss a '+0000'",       //

    NULL
};

static const char * remapResults_es_PR_japanese[] = { // rdar://52461062
    "h:mm:ss a zzzz", // full
    "HH:mm:ss zzzz",  //   force24
    "h:mm:ss a zzzz", //   force12
    "h:mm:ss a z",    // long
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12
    "h:mm:ss a",      // medium
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "h:mm a",         // short
    "HH:mm",          //   force24
    "h:mm a",         //   force12
    "EEEE, d 'de' MMMM 'de' y G, h:mm:ss a z", // long_df
    "EEEE, d 'de' MMMM 'de' y G, HH:mm:ss z",  //   force24
    "EEEE, d 'de' MMMM 'de' y G, h:mm:ss a z", //   force12
    "MM/dd/yy GGGGG h:mm a", // short_ds
    "MM/dd/yy GGGGG HH:mm",  //   force24
    "MM/dd/yy GGGGG h:mm a", //   force12

    "h:mm:ss a",      // jmmss
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "h:mm:ss a",      // jjmmss
    "HH:mm:ss",       //   force24
    "HH:mm:ss",       //   force24 | match hour field length
    "h:mm:ss a",      //   force12
    "hh:mm:ss a",     //   force12 | match hour field length
    "hh:mm",          // Jmm
    "HH:mm",          //   force24
    "hh:mm",          //   force12
    "h:mm:ss a v",    // jmsv
    "HH:mm:ss v",     //   force24
    "h:mm:ss a v",    //   force12
    "h:mm:ss a z",    // jmsz
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "HH:mm:ss",                           //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "HH:mm:ss d MMM y",                   //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "h:mm:ss a",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "h:mm:ss a d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' h'h'mm'm'ss a",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd h:mm:ss a '+0000'",       //

    NULL
};

static const char * remapResults_en_IN[] = { // rdar://56309604
    "h:mm:ss a zzzz", // full
    "HH:mm:ss zzzz",  //   force24
    "h:mm:ss a zzzz", //   force12
    "h:mm:ss a z",    // long
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12
    "h:mm:ss a",      // medium
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "h:mm a",         // short
    "HH:mm",          //   force24
    "h:mm a",         //   force12
    "EEEE, d MMMM y 'at' h:mm:ss a z", // long_df
    "EEEE, d MMMM y 'at' HH:mm:ss z",  //   force24
    "EEEE, d MMMM y 'at' h:mm:ss a z", //   force12
    "dd/MM/yy, h:mm a", // short_ds
    "dd/MM/yy, HH:mm",  //   force24
    "dd/MM/yy, h:mm a", //   force12

    "h:mm:ss a",      // jmmss
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "h:mm:ss a",      // jjmmss
    "HH:mm:ss",       //   force24
    "HH:mm:ss",       //   force24 | match hour field length
    "h:mm:ss a",      //   force12
    "hh:mm:ss a",     //   force12 | match hour field length
    "hh:mm",          // Jmm
    "HH:mm",          //   force24
    "hh:mm",          //   force12
    "h:mm:ss a v",    // jmsv
    "HH:mm:ss v",     //   force24
    "h:mm:ss a v",    //   force12
    "h:mm:ss a z",    // jmsz
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "HH:mm:ss",                           //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "HH:mm:ss d MMM y",                   //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "h:mm:ss a",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "h:mm:ss a d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' h'h'mm'm'ss a",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd h:mm:ss a '+0000'",       //

    NULL
};

static const char * remapResults_en_IN_japanese[] = { // rdar://56309604
    "h:mm:ss a zzzz", // full
    "HH:mm:ss zzzz",  //   force24
    "h:mm:ss a zzzz", //   force12
    "h:mm:ss a z",    // long
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12
    "h:mm:ss a",      // medium
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "h:mm a",         // short
    "HH:mm",          //   force24
    "h:mm a",         //   force12
    "EEEE, d MMMM y G 'at' h:mm:ss a z", // long_df
    "EEEE, d MMMM y G 'at' HH:mm:ss z",  //   force24
    "EEEE, d MMMM y G 'at' h:mm:ss a z", //   force12
    "dd/MM/y GGGGG, h:mm a", // short_ds
    "dd/MM/y GGGGG, HH:mm",  //   force24
    "dd/MM/y GGGGG, h:mm a", //   force12

    "h:mm:ss a",      // jmmss
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "h:mm:ss a",      // jjmmss
    "HH:mm:ss",       //   force24
    "HH:mm:ss",       //   force24 | match hour field length
    "h:mm:ss a",      //   force12
    "hh:mm:ss a",     //   force12 | match hour field length
    "hh:mm",          // Jmm
    "HH:mm",          //   force24
    "hh:mm",          //   force12
    "h:mm:ss a v",    // jmsv
    "HH:mm:ss v",     //   force24
    "h:mm:ss a v",    //   force12
    "h:mm:ss a z",    // jmsz
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss"
    "HH:mm:ss",                           //
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y"
    "HH:mm:ss d MMM y",                   //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'"
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss"
    "yyMMddHHmmss",                       //

    "h:mm:ss a",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "h:mm:ss a d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' h'h'mm'm'ss a",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd h:mm:ss a '+0000'",       //

    NULL
};

static const char * remapResults_en_BE[] = { // rdar://56309604
    "HH:mm:ss zzzz",  // full
    "HH:mm:ss zzzz",  //   force24
    "h:mm:ss a zzzz", //   force12
    "HH:mm:ss z",     // long
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12
    "HH:mm:ss",       // medium
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "HH:mm",          // short
    "HH:mm",          //   force24
    "h:mm a",         //   force12
    "EEEE, d MMMM y 'at' HH:mm:ss z",  // long_df
    "EEEE, d MMMM y 'at' HH:mm:ss z",  //   force24
    "EEEE, d MMMM y 'at' h:mm:ss a z", //   force12
    "dd/MM/y, HH:mm",  // short_ds
    "dd/MM/y, HH:mm",  //   force24
    "dd/MM/y, h:mm a", //   force12

    "HH:mm:ss",       // jmmss
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "HH:mm:ss",       // jjmmss
    "HH:mm:ss",       //   force24
    "HH:mm:ss",       //   force24 | match hour field length
    "h:mm:ss a",      //   force12
    "hh:mm:ss a",     //   force12 | match hour field length
    "HH:mm",          // Jmm
    "HH:mm",          //   force24
    "hh:mm",          //   force12
    "HH:mm:ss v",     // jmsv
    "HH:mm:ss v",     //   force24
    "h:mm:ss a v",    //   force12
    "HH:mm:ss z",     // jmsz
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss" force12
    "HH:mm:ss",                           //           force24
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y" force12
    "HH:mm:ss d MMM y",                   //                        force24
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'" force12
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss" force12
    "yyMMddHHmmss",                       //

    "h:mm:ss a",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "h:mm:ss a d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' h'h'mm'm'ss a",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd h:mm:ss a '+0000'",       //

    NULL
};

static const char * remapResults_en_BE_japanese[] = { // rdar://56309604
    "HH:mm:ss zzzz",  // full
    "HH:mm:ss zzzz",  //   force24
    "h:mm:ss a zzzz", //   force12
    "HH:mm:ss z",     // long
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12
    "HH:mm:ss",       // medium
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "HH:mm",          // short
    "HH:mm",          //   force24
    "h:mm a",         //   force12
    "EEEE, d MMMM y G 'at' HH:mm:ss z",  // long_df
    "EEEE, d MMMM y G 'at' HH:mm:ss z",  //   force24
    "EEEE, d MMMM y G 'at' h:mm:ss a z", //   force12
    "dd/MM/y GGGGG, HH:mm",  // short_ds
    "dd/MM/y GGGGG, HH:mm",  //   force24
    "dd/MM/y GGGGG, h:mm a", //   force12

    "HH:mm:ss",       // jmmss
    "HH:mm:ss",       //   force24
    "h:mm:ss a",      //   force12
    "HH:mm:ss",       // jjmmss
    "HH:mm:ss",       //   force24
    "HH:mm:ss",       //   force24 | match hour field length
    "h:mm:ss a",      //   force12
    "hh:mm:ss a",     //   force12 | match hour field length
    "HH:mm",          // Jmm
    "HH:mm",          //   force24
    "hh:mm",          //   force12
    "HH:mm:ss v",     // jmsv
    "HH:mm:ss v",     //   force24
    "h:mm:ss a v",    //   force12
    "HH:mm:ss z",     // jmsz
    "HH:mm:ss z",     //   force24
    "h:mm:ss a z",    //   force12

    "h:mm:ss a",                          // "h:mm:ss" force12
    "HH:mm:ss",                           //           force24
    "a'xx'h:mm:ss d MMM y",               // "a'xx'h:mm:ss d MMM y" force12
    "HH:mm:ss d MMM y",                   //                        force24
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' h:mm:ss a 'hrs'" force12
    "EEE, d MMM y 'aha' HH:mm:ss 'hrs'",  //
    "EEE, d MMM y 'aha' a'xx'h:mm:ss",    // "EEE, d MMM y 'aha' a'xx'h:mm:ss"
    "EEE, d MMM y 'aha' HH:mm:ss",        //
    "yyMMddhhmmss",                       // "yyMMddhhmmss" force12
    "yyMMddHHmmss",                       //

    "h:mm:ss a",                          // "H:mm:ss"
    "H:mm:ss",                            //
    "h:mm:ss a d MMM y",                  // "H:mm:ss d MMM y"
    "H:mm:ss d MMM y",                    //
    "EEE, d MMM y 'aha' h:mm:ss a 'hrs'", // "EEE, d MMM y 'aha' H:mm:ss 'hrs'"
    "EEE, d MMM y 'aha' H:mm:ss 'hrs'",   //
    "EEE, d MMM y 'aha' h'h'mm'm'ss a",   // "EEE, d MMM y 'aha' H'h'mm'm'ss"
    "EEE, d MMM y 'aha' H'h'mm'm'ss",     //

    "uuuu-MM-dd h:mm:ss a '+0000'",       //

    NULL
};

typedef struct {
    const char * locale;
    const char ** resultsPtr;
} RemapPatternLocaleResults;

static const RemapPatternLocaleResults remapLocResults[] = {
    { "root",   remapResults_root },
    { "en",     remapResults_en   },
    { "ja",     remapResults_ja   },
    { "ko",     remapResults_ko   },
    { "th",     remapResults_th   },
    { "hi",     remapResults_hi   },
    { "ar",     remapResults_ar   },
    { "en_IL",  remapResults_en_IL },
    { "es_PR@calendar=japanese",  remapResults_es_PR_japanese },
    { "en_IN",  remapResults_en_IN },
    { "en_IN@calendar=japanese",  remapResults_en_IN_japanese },
    { "en_BE",  remapResults_en_BE },
    { "en_BE@calendar=japanese",  remapResults_en_BE_japanese },
    { NULL,     NULL }
};

enum { kUBufRemapMax = 64, kBBufRemapMax = 128 };

static void TestRemapPatternWithOpts(void) { /* Apple-specific */
    const RemapPatternLocaleResults * locResPtr;
    for (locResPtr = remapLocResults; locResPtr->locale != NULL; locResPtr++) {
        UErrorCode status = U_ZERO_ERROR;
        UDateTimePatternGenerator* dtpg = udatpg_open(locResPtr->locale, &status);
        if ( U_FAILURE(status) ) {
            log_data_err("udatpg_open fails for locale %s, status %s (Are you missing data?)\n", locResPtr->locale, u_errorName(status));
        } else {
            const RemapPatternTestItem * testItemPtr = remapPatItems;
            const char ** expResultsPtr = locResPtr->resultsPtr;
            for (; testItemPtr->pattern != NULL && *expResultsPtr != NULL; testItemPtr++, expResultsPtr++) {
                UChar uskel[kUBufRemapMax];
                UChar upatn[kUBufRemapMax];
                UChar uget[kUBufRemapMax];
                UChar uexp[kUBufRemapMax];
                int32_t uelen, ulen = 0;
                
                status = U_ZERO_ERROR;
                if (testItemPtr->testtype >= 0) {
                    UDateFormatStyle timeStyle = (UDateFormatStyle)((int32_t)testItemPtr->testtype & 0x03);
                    UDateFormatStyle dateStyle = (UDateFormatStyle)((((int32_t)testItemPtr->testtype >> 2) & 0x07) - 1);
                    UDateFormat* dfmt = udat_open(timeStyle, dateStyle, locResPtr->locale, NULL, 0, NULL, 0, &status);
                    if ( U_FAILURE(status) ) {
                        log_data_err("udat_open fails for locale %s, status %s (Are you missing data?)\n", locResPtr->locale, u_errorName(status));
                        continue;
                    } else {
                        ulen = udat_toPattern(dfmt, FALSE, upatn, kUBufRemapMax, &status);
                        udat_close(dfmt);
                        if ( U_FAILURE(status) ) {
                            log_err("udat_toPattern fails for locale %s, status %s\n", locResPtr->locale, u_errorName(status));
                            continue;
                        }
                    }
                } else if (testItemPtr->testtype == REMAP_TESTTYPE_SKELETON) {
                    u_strFromUTF8(uskel, kUBufRemapMax, &ulen, testItemPtr->pattern, -1, &status);
                    ulen = udatpg_getBestPatternWithOptions(dtpg, uskel, ulen, (UDateTimePatternMatchOptions)testItemPtr->options, upatn, kUBufRemapMax, &status);
                    if ( U_FAILURE(status) ) {
                        log_err("udatpg_getBestPatternWithOptions fails for locale %s, skeleton \"%s\": status %s\n", locResPtr->locale, testItemPtr->pattern, u_errorName(status));
                        continue;
                    }
                } else {
                    ulen = u_unescape(testItemPtr->pattern, upatn, kUBufRemapMax);
                }
                uelen = u_unescape(*expResultsPtr, uexp, kUBufRemapMax);
                ulen = uadatpg_remapPatternWithOptions(dtpg, upatn, ulen, (UDateTimePatternMatchOptions)testItemPtr->options, uget, kUBufRemapMax, &status);
                if ( U_FAILURE(status) ) {
                    log_err("uadatpg_remapPatternWithOptions fails for locale %s pattern \"%s\" opts %08X: status %s\n",
                            locResPtr->locale, testItemPtr->pattern, testItemPtr->options, u_errorName(status));
                } else if (uelen != ulen || u_strncmp(uget, uexp, ulen) != 0) {
                    char bebuf[kBBufRemapMax];
                    char bbuf[kBBufRemapMax];
                    UErrorCode tempStatus = U_ZERO_ERROR;
                    u_strToUTF8(bebuf, kBBufRemapMax, NULL, uexp, uelen, &tempStatus);
                    u_strToUTF8(bbuf, kBBufRemapMax, NULL, uget, ulen, &tempStatus);
                    log_err("uadatpg_remapPatternWithOptions for locale %s pattern \"%s\" opts %08X: expect \"%s\", get \"%s\"\n",
                            locResPtr->locale, testItemPtr->pattern, testItemPtr->options, bebuf, bbuf);
                }
            }
            udatpg_close(dtpg);
        }
    }
}

#if ADD_ALLOC_TEST
#include <stdio.h>
#include <unistd.h>
static const UChar* tzName = u"US/Pacific";
static const UDate udatToUse = 1290714600000.0; // Thurs, Nov. 25, 2010 11:50:00 AM PT
static const UChar* dateStrEST = u"Thursday, November 25, 2010 at 11:50:00 AM EST";
enum { kUCharsOutMax = 128, kBytesOutMax = 256, kRepeatCount = 100, SLEEPSECS = 6 };

static void TestPerf(void) {
    UDateFormat *udatfmt;
    UErrorCode status = U_ZERO_ERROR;
    printf("\n# TestPerf start; sleeping %d seconds to check heap.\n", SLEEPSECS); sleep(SLEEPSECS);
    udatfmt = udat_open(UDAT_FULL, UDAT_FULL, "en_US", tzName, -1, NULL, 0, &status);
    if ( U_SUCCESS(status) ) {
        UChar outUChars[kUCharsOutMax];
        int32_t count, datlen, datlen2, parsePos;
        UDate dateParsed, dateParsed2;

        datlen = udat_format(udatfmt, udatToUse, outUChars, kUCharsOutMax, NULL, &status);
        printf("# TestPerf after first open & format, status %d; sleeping %d seconds to check heap.\n", status, SLEEPSECS); sleep(SLEEPSECS);

        for (count = kRepeatCount; count-- > 0;) {
            status = U_ZERO_ERROR;
            datlen2 = udat_format(udatfmt, udatToUse, outUChars, kUCharsOutMax, NULL, &status);
            if ( U_FAILURE(status) || datlen2 != datlen ) {
                printf("# TestPerf udat_format unexpected result.\n");
                break;
            }
        }
        printf("# TestPerf after many more format, status %d; sleeping %d seconds to check heap.\n", status, SLEEPSECS); sleep(SLEEPSECS);

        udat_setLenient(udatfmt, TRUE);
        status = U_ZERO_ERROR;
        parsePos = 0;
        dateParsed = udat_parse(udatfmt, dateStrEST, -1, &parsePos, &status);
        printf("# TestPerf after first parse lenient diff style/zone, status %d; sleeping %d seconds to check heap.\n", status, SLEEPSECS); sleep(SLEEPSECS);
        
        for (count = kRepeatCount; count-- > 0;) {
            status = U_ZERO_ERROR;
            parsePos = 0;
            dateParsed2 = udat_parse(udatfmt, dateStrEST, -1, &parsePos, &status);
            if ( U_FAILURE(status) || dateParsed2 != dateParsed ) {
                printf("# TestPerf udat_parse unexpected result.\n");
                break;
            }
        }
        printf("# TestPerf after many more parse, status %d; sleeping %d seconds to check heap.\n", status, SLEEPSECS); sleep(SLEEPSECS);

        udat_close(udatfmt);
        printf("# TestPerf after udat_close; sleeping %d seconds to check heap.\n", SLEEPSECS); sleep(SLEEPSECS);
    }
}
#endif /* #if ADD_ALLOC_TEST */

#if WRITE_HOUR_MISMATCH_ERRS
// WriteHourMismatchErrs stuff 52980140
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char* langs[] = {
	"ar", "ca", "cs", "da", "de", "el", "en", "es", "fi", "fr", "he",
	"hi", "hr", "hu", "id", "it", "ja", "ko", "ms", "nb", "nl", "pl",
	"pt", "ro", "ru", "sk", "sv", "th", "tr", "uk", "vi", "zh" }; // handle "yue" separately
enum { kNlangs = sizeof(langs)/sizeof(langs[0]) };

static const char* cals[] = {
	"buddhist",
	"chinese",
	"coptic",
	"ethiopic",
	"gregorian",
	"hebrew",
	"indian",
	"islamic-umalqura",
	"islamic",
	"japanese",
	"persian",
	NULL
};

enum { kLBufMax = 63 };

static int compKeys(const void* keyval, const void* baseval) {
    const char* keystr = (const char*)keyval;
    const char* basestr = *(const char**)baseval;
    return strcmp(keystr,basestr);
}

static UBool useLocale(const char* locale) {
    if (strncmp(locale, "yue", 3) == 0) {
        return TRUE;
    }
    if (locale[2]==0 || locale[2]=='_') {
    	char lang[3] = {0,0,0};
    	strncpy(lang,locale,2);
    	if (bsearch(&lang, langs, kNlangs, sizeof(char*), compKeys) != NULL) {
    		return TRUE;
    	}
    }
    return FALSE;
}

static UBool patIsBad(const UChar* ubuf) {
    return (u_strchr(ubuf,0x251C)!=NULL || u_strchr(ubuf,0x2524)!=NULL || u_strstr(ubuf,u": ")!=NULL);
}


static void WriteHourMismatchErrs(void) { /* Apple-specific */
	int32_t iloc, nloc = uloc_countAvailable();
	int32_t errcnt = 0;
	printf("# uloc_countAvailable: %d\n", nloc);
	for (iloc = 0; iloc < nloc; iloc++) {
	    const char* locale = uloc_getAvailable(iloc);
	    if (useLocale(locale)) {
	    	const char** calsPtr = cals;
	    	const char* cal;
	    	while ((cal = *calsPtr++) != NULL) {
				char fullLocale[kLBufMax+1];
				UErrorCode status = U_ZERO_ERROR;
				strncpy(fullLocale, locale, kLBufMax);
				fullLocale[kLBufMax] = 0;
				uloc_setKeywordValue("calendar", cal, fullLocale, kLBufMax, &status);
				if ( U_SUCCESS(status) ) {
					fullLocale[kLBufMax] = 0;
					UDateFormat* udat = udat_open(UDAT_SHORT, UDAT_NONE, fullLocale, NULL, 0, NULL, 0, &status);
					UDateTimePatternGenerator* udatpg = udatpg_open(fullLocale, &status);
					if ( U_FAILURE(status) ) {
						printf("# udat_open/udatpg_open for locale %s: %s\n", fullLocale, u_errorName(status));
					} else {
						UChar ubufs[kUBufMax];
						UChar ubufp[kUBufMax];
						char bbufs[kBBufMax];
						char bbufpj[kBBufMax];
						char bbufpH[kBBufMax];
						char bbufph[kBBufMax];
						int32_t ulen;
						int8_t errors[4] = {0,0,0,0};
						
						status = U_ZERO_ERROR;
						ulen = udat_toPattern(udat, FALSE, ubufs, kUBufMax, &status);
						u_strToUTF8(bbufs, kBBufMax, NULL, ubufs, ulen, &status);
						if ( U_FAILURE(status) ) {
						    printf("# udat_toPattern for locale %s: %s\n", fullLocale, u_errorName(status));
						    strcpy(bbufs, "****");
						    errors[0] = 3;
						} else if (patIsBad(ubufs)) {
						    errors[0] = 2;
						}

						status = U_ZERO_ERROR;
						ulen = udatpg_getBestPattern(udatpg, u"jmm", 3, ubufp, kUBufMax, &status);
						u_strToUTF8(bbufpj, kBBufMax, NULL, ubufp, ulen, &status);
						if ( U_FAILURE(status) ) {
						    printf("# udatpg_getBestPat jmm for locale %s: %s\n", fullLocale, u_errorName(status));
						    strcpy(bbufpj, "****");
						    errors[1] = 3;
						} else if (patIsBad(ubufp)) {
						    errors[1] = 2;
						} else if (errors[0] == 0 && u_strcmp(ubufp,ubufs) != 0) {
						    errors[0] = 1;
						}

						status = U_ZERO_ERROR;
						ulen = udatpg_getBestPattern(udatpg, u"Hmm", 3, ubufp, kUBufMax, &status);
						u_strToUTF8(bbufpH, kBBufMax, NULL, ubufp, ulen, &status);
						if ( U_FAILURE(status) ) {
						    printf("# udatpg_getBestPat Hmm for locale %s: %s\n", fullLocale, u_errorName(status));
						    strcpy(bbufpH, "****");
						    errors[2] = 3;
						} else if (patIsBad(ubufp)) {
						    errors[2] = 2;
						}

						status = U_ZERO_ERROR;
						ulen = udatpg_getBestPattern(udatpg, u"hmm", 3, ubufp, kUBufMax, &status);
						u_strToUTF8(bbufph, kBBufMax, NULL, ubufp, ulen, &status);
						if ( U_FAILURE(status) ) {
						    printf("# udatpg_getBestPat hmm for locale %s: %s\n", fullLocale, u_errorName(status));
						    strcpy(bbufph, "****");
						    errors[3] = 3;
						} else if (patIsBad(ubufp)) {
						    errors[3] = 2;
						}
						
						if ( errors[0] || errors[1] || errors[2] || errors[3]) {
						    printf("%-36s\tshr-%d %-8s\tjmm-%d %-18s\tHmm-%d %-18s\thmm-%d %-18s\n", fullLocale,
						    		errors[0], bbufs, errors[1], bbufpj, errors[2], bbufpH, errors[3], bbufph);
						    errcnt++;
						}

						udatpg_close(udatpg);
						udat_close(udat);
					}
				}
	    	}
	    }
	}
	printf("# total err lines: %d\n", errcnt);
}
#endif /* #if WRITE_HOUR_MISMATCH_ERRS */

#endif /* #if !UCONFIG_NO_FORMATTING */
