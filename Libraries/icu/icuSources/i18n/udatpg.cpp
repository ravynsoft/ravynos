// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
*******************************************************************************
*
*   Copyright (C) 2009-2015, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  udatpg.cpp
*   encoding:   UTF-8
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2007jul30
*   created by: Markus W. Scherer
*/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/udatpg.h"
#include "unicode/uenum.h"
#include "unicode/strenum.h"
#include "unicode/unistr.h"
#include "unicode/dtptngen.h"
#include "unicode/uchar.h"
#include "ustrenum.h"
#include "dtitv_impl.h"

U_NAMESPACE_USE

U_CAPI UDateTimePatternGenerator * U_EXPORT2
udatpg_open(const char *locale, UErrorCode *pErrorCode) {
    if(locale==NULL) {
        return (UDateTimePatternGenerator *)DateTimePatternGenerator::createInstance(*pErrorCode);
    } else {
        return (UDateTimePatternGenerator *)DateTimePatternGenerator::createInstance(Locale(locale), *pErrorCode);
    }
}

U_CAPI UDateTimePatternGenerator * U_EXPORT2
udatpg_openEmpty(UErrorCode *pErrorCode) {
    return (UDateTimePatternGenerator *)DateTimePatternGenerator::createEmptyInstance(*pErrorCode);
}

U_CAPI void U_EXPORT2
udatpg_close(UDateTimePatternGenerator *dtpg) {
    delete (DateTimePatternGenerator *)dtpg;
}

U_CAPI UDateTimePatternGenerator * U_EXPORT2
udatpg_clone(const UDateTimePatternGenerator *dtpg, UErrorCode *pErrorCode) {
    if(U_FAILURE(*pErrorCode)) {
        return NULL;
    }
    return (UDateTimePatternGenerator *)(((const DateTimePatternGenerator *)dtpg)->clone());
}

U_CAPI int32_t U_EXPORT2
udatpg_getBestPattern(UDateTimePatternGenerator *dtpg,
                      const UChar *skeleton, int32_t length,
                      UChar *bestPattern, int32_t capacity,
                      UErrorCode *pErrorCode) {
    return udatpg_getBestPatternWithOptions(dtpg, skeleton, length,
                                            UDATPG_MATCH_NO_OPTIONS,
                                            bestPattern, capacity, pErrorCode);
}

U_CAPI int32_t U_EXPORT2
udatpg_getBestPatternWithOptions(UDateTimePatternGenerator *dtpg,
                                 const UChar *skeleton, int32_t length,
                                 UDateTimePatternMatchOptions options,
                                 UChar *bestPattern, int32_t capacity,
                                 UErrorCode *pErrorCode) {
    if(U_FAILURE(*pErrorCode)) {
        return 0;
    }
    if(skeleton==NULL && length!=0) {
        *pErrorCode=U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    UnicodeString skeletonString((UBool)(length<0), skeleton, length);
    UnicodeString result=((DateTimePatternGenerator *)dtpg)->getBestPattern(skeletonString, options, *pErrorCode);
    return result.extract(bestPattern, capacity, *pErrorCode);
}

U_CAPI int32_t U_EXPORT2
udatpg_getSkeleton(UDateTimePatternGenerator * /* dtpg */,
                   const UChar *pattern, int32_t length,
                   UChar *skeleton, int32_t capacity,
                   UErrorCode *pErrorCode) {
    if(U_FAILURE(*pErrorCode)) {
        return 0;
    }
    if(pattern==NULL && length!=0) {
        *pErrorCode=U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    UnicodeString patternString((UBool)(length<0), pattern, length);
    UnicodeString result=DateTimePatternGenerator::staticGetSkeleton(
            patternString, *pErrorCode);
    return result.extract(skeleton, capacity, *pErrorCode);
}

U_CAPI int32_t U_EXPORT2
udatpg_getBaseSkeleton(UDateTimePatternGenerator * /* dtpg */,
                       const UChar *pattern, int32_t length,
                       UChar *skeleton, int32_t capacity,
                       UErrorCode *pErrorCode) {
    if(U_FAILURE(*pErrorCode)) {
        return 0;
    }
    if(pattern==NULL && length!=0) {
        *pErrorCode=U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    UnicodeString patternString((UBool)(length<0), pattern, length);
    UnicodeString result=DateTimePatternGenerator::staticGetBaseSkeleton(
            patternString, *pErrorCode);
    return result.extract(skeleton, capacity, *pErrorCode);
}

U_CAPI UDateTimePatternConflict U_EXPORT2
udatpg_addPattern(UDateTimePatternGenerator *dtpg,
                  const UChar *pattern, int32_t patternLength,
                  UBool override,
                  UChar *conflictingPattern, int32_t capacity, int32_t *pLength,
                  UErrorCode *pErrorCode) {
    if(U_FAILURE(*pErrorCode)) {
        return UDATPG_NO_CONFLICT;
    }
    if(pattern==NULL && patternLength!=0) {
        *pErrorCode=U_ILLEGAL_ARGUMENT_ERROR;
        return UDATPG_NO_CONFLICT;
    }
    UnicodeString patternString((UBool)(patternLength<0), pattern, patternLength);
    UnicodeString conflictingPatternString;
    UDateTimePatternConflict result=((DateTimePatternGenerator *)dtpg)->
            addPattern(patternString, override, conflictingPatternString, *pErrorCode);
    int32_t length=conflictingPatternString.extract(conflictingPattern, capacity, *pErrorCode);
    if(pLength!=NULL) {
        *pLength=length;
    }
    return result;
}

U_CAPI void U_EXPORT2
udatpg_setAppendItemFormat(UDateTimePatternGenerator *dtpg,
                           UDateTimePatternField field,
                           const UChar *value, int32_t length) {
    UnicodeString valueString((UBool)(length<0), value, length);
    ((DateTimePatternGenerator *)dtpg)->setAppendItemFormat(field, valueString);
}

U_CAPI const UChar * U_EXPORT2
udatpg_getAppendItemFormat(const UDateTimePatternGenerator *dtpg,
                           UDateTimePatternField field,
                           int32_t *pLength) {
    const UnicodeString &result=((const DateTimePatternGenerator *)dtpg)->getAppendItemFormat(field);
    if(pLength!=NULL) {
        *pLength=result.length();
    }
    return result.getBuffer();
}

U_CAPI void U_EXPORT2
udatpg_setAppendItemName(UDateTimePatternGenerator *dtpg,
                         UDateTimePatternField field,
                         const UChar *value, int32_t length) {
    UnicodeString valueString((UBool)(length<0), value, length);
    ((DateTimePatternGenerator *)dtpg)->setAppendItemName(field, valueString);
}

U_CAPI const UChar * U_EXPORT2
udatpg_getAppendItemName(const UDateTimePatternGenerator *dtpg,
                         UDateTimePatternField field,
                         int32_t *pLength) {
    const UnicodeString &result=((const DateTimePatternGenerator *)dtpg)->getAppendItemName(field);
    if(pLength!=NULL) {
        *pLength=result.length();
    }
    return result.getBuffer();
}

U_CAPI int32_t U_EXPORT2
udatpg_getFieldDisplayName(const UDateTimePatternGenerator *dtpg,
                           UDateTimePatternField field,
                           UDateTimePGDisplayWidth width,
                           UChar *fieldName, int32_t capacity,
                           UErrorCode *pErrorCode) {
    if (U_FAILURE(*pErrorCode))
        return -1;
    if (fieldName == NULL ? capacity != 0 : capacity < 0) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return -1;
    }
    UnicodeString result = ((const DateTimePatternGenerator *)dtpg)->getFieldDisplayName(field,width);
    if (fieldName == NULL) {
        return result.length();
    }
    return result.extract(fieldName, capacity, *pErrorCode);
}

U_CAPI void U_EXPORT2
udatpg_setDateTimeFormat(const UDateTimePatternGenerator *dtpg,
                         const UChar *dtFormat, int32_t length) {
    UnicodeString dtFormatString((UBool)(length<0), dtFormat, length);
    ((DateTimePatternGenerator *)dtpg)->setDateTimeFormat(dtFormatString);
}

U_CAPI const UChar * U_EXPORT2
udatpg_getDateTimeFormat(const UDateTimePatternGenerator *dtpg,
                         int32_t *pLength) {
    const UnicodeString &result=((const DateTimePatternGenerator *)dtpg)->getDateTimeFormat();
    if(pLength!=NULL) {
        *pLength=result.length();
    }
    return result.getBuffer();
}

U_CAPI void U_EXPORT2
udatpg_setDecimal(UDateTimePatternGenerator *dtpg,
                  const UChar *decimal, int32_t length) {
    UnicodeString decimalString((UBool)(length<0), decimal, length);
    ((DateTimePatternGenerator *)dtpg)->setDecimal(decimalString);
}

U_CAPI const UChar * U_EXPORT2
udatpg_getDecimal(const UDateTimePatternGenerator *dtpg,
                  int32_t *pLength) {
    const UnicodeString &result=((const DateTimePatternGenerator *)dtpg)->getDecimal();
    if(pLength!=NULL) {
        *pLength=result.length();
    }
    return result.getBuffer();
}

U_CAPI int32_t U_EXPORT2
udatpg_replaceFieldTypes(UDateTimePatternGenerator *dtpg,
                         const UChar *pattern, int32_t patternLength,
                         const UChar *skeleton, int32_t skeletonLength,
                         UChar *dest, int32_t destCapacity,
                         UErrorCode *pErrorCode) {
    return udatpg_replaceFieldTypesWithOptions(dtpg, pattern, patternLength, skeleton, skeletonLength,
                                               UDATPG_MATCH_NO_OPTIONS,
                                               dest, destCapacity, pErrorCode);
}

U_CAPI int32_t U_EXPORT2
udatpg_replaceFieldTypesWithOptions(UDateTimePatternGenerator *dtpg,
                                    const UChar *pattern, int32_t patternLength,
                                    const UChar *skeleton, int32_t skeletonLength,
                                    UDateTimePatternMatchOptions options,
                                    UChar *dest, int32_t destCapacity,
                                    UErrorCode *pErrorCode) {
    if(U_FAILURE(*pErrorCode)) {
        return 0;
    }
    if((pattern==NULL && patternLength!=0) || (skeleton==NULL && skeletonLength!=0)) {
        *pErrorCode=U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    UnicodeString patternString((UBool)(patternLength<0), pattern, patternLength);
    UnicodeString skeletonString((UBool)(skeletonLength<0), skeleton, skeletonLength);
    UnicodeString result=((DateTimePatternGenerator *)dtpg)->replaceFieldTypes(patternString, skeletonString, options, *pErrorCode);
    return result.extract(dest, destCapacity, *pErrorCode);
}

U_CAPI UEnumeration * U_EXPORT2
udatpg_openSkeletons(const UDateTimePatternGenerator *dtpg, UErrorCode *pErrorCode) {
    return uenum_openFromStringEnumeration(
                ((DateTimePatternGenerator *)dtpg)->getSkeletons(*pErrorCode),
                pErrorCode);
}

U_CAPI UEnumeration * U_EXPORT2
udatpg_openBaseSkeletons(const UDateTimePatternGenerator *dtpg, UErrorCode *pErrorCode) {
    return uenum_openFromStringEnumeration(
                ((DateTimePatternGenerator *)dtpg)->getBaseSkeletons(*pErrorCode),
                pErrorCode);
}

U_CAPI const UChar * U_EXPORT2
udatpg_getPatternForSkeleton(const UDateTimePatternGenerator *dtpg,
                             const UChar *skeleton, int32_t skeletonLength,
                             int32_t *pLength) {
    UnicodeString skeletonString((UBool)(skeletonLength<0), skeleton, skeletonLength);
    const UnicodeString &result=((const DateTimePatternGenerator *)dtpg)->getPatternForSkeleton(skeletonString);
    if(pLength!=NULL) {
        *pLength=result.length();
    }
    return result.getBuffer();
}

// Helper function for uadatpg_remapPatternWithOptionsLoc
static int32_t
_doReplaceAndReturnAdj( UDateTimePatternGenerator *dtpg, uint32_t options, UBool matchHourLen,
                        UnicodeString &patternString, const UnicodeString &skeleton, const UnicodeString &otherCycSkeleton,
                        int32_t timePatStart, int32_t timePatLimit, int32_t timeNonHourStart, int32_t timeNonHourLimit,
                        UErrorCode *pErrorCode) {
    if (matchHourLen) {
        options |= UDATPG_MATCH_HOUR_FIELD_LENGTH;
    }
    UnicodeString replacement=((DateTimePatternGenerator *)dtpg)->getBestPattern(otherCycSkeleton, (UDateTimePatternMatchOptions)options, *pErrorCode);
    if (U_FAILURE(*pErrorCode)) {
        return 0;
    }
    UnicodeString stringForOrigSkeleton=((DateTimePatternGenerator *)dtpg)->getBestPattern(skeleton, UDATPG_MATCH_ALL_FIELDS_LENGTH, *pErrorCode); // match orig field lengths
    if (U_SUCCESS(*pErrorCode)) {
        int32_t index = patternString.indexOf(stringForOrigSkeleton);
        if (index >= 0) {
            int32_t stringForOrigSkelLen = stringForOrigSkeleton.length();
            patternString.replace(index, stringForOrigSkelLen, replacement);
            return replacement.length() - stringForOrigSkelLen;
        }
    } else {
        *pErrorCode = U_ZERO_ERROR;
    }
    if (timeNonHourStart >= 0 && timeNonHourLimit > timeNonHourStart) {
        // find any minutes/seconds/milliseconds part of replacement, set that back to the
        // minutes/seconds/milliseconds part of the original pattern.
        // First get the minutes/seconds/milliseconds part of the original pattern.
        UnicodeString nonHour;
        patternString.extractBetween(timeNonHourStart, timeNonHourLimit, nonHour);
        // Now scan to find position from just after hours to end of minutes/seconds/milliseconds.
        timeNonHourStart = -1;
        timeNonHourLimit = 0;
        UBool inQuoted = FALSE;
        int32_t repPos, repLen = replacement.length();
        for (repPos = 0; repPos < repLen; repPos++) {
            UChar repChr = replacement.charAt(repPos);
            if (repChr == 0x27 /* ASCII-range single quote */) {
                inQuoted = !inQuoted;
            } else if (!inQuoted) {
                if (repChr==LOW_H || repChr==CAP_H || repChr==CAP_K || repChr==LOW_K) { // hHKk, hour
                    timeNonHourStart = repPos + 1;
                } else if (timeNonHourStart < 0 && (repChr==LOW_M || repChr==LOW_S)) { // 'm' or 's' and we did not have hour
                    timeNonHourStart = repPos;
                }
                if (!u_isWhitespace(repChr) && timeNonHourStart >= 0 && repChr!=LOW_A) { // NonHour portion should not include 'a'
                    timeNonHourLimit = repPos + 1;
                }
            }
        }
        // If we found minutes/seconds/milliseconds in replacement, restore that part to original.
        if (timeNonHourStart >= 0 && timeNonHourLimit > timeNonHourStart) {
            replacement.replaceBetween(timeNonHourStart, timeNonHourLimit, nonHour);
        }
    }
    patternString.replaceBetween(timePatStart, timePatLimit, replacement);
    return replacement.length() - (timePatLimit - timePatStart); // positive if replacement is longer
}

/*
 *  uadatpg_remapPatternWithOptionsLoc
 *
 *  Thee general idea is:
 *  1. Scan the pattern for one or more time subpatterns
 *  2. For each time subpattern, if the hour pattern characters don't match the
 *     time cycle that we want to force to, then:
 *     a) Save the nonHour portion of the subpattern (from just after hours to end
 *        of minutes/seconds/ milliseconds)
 *     b) Turn the pattern characters in that subpattern into a skeleton, but with
 *        the hour pattern characters switched to the desired time cycle
 *     c) Use that skeleton to get the locale's corresponding replacement pattern
 *        for the desired time cycle (with all desired elements - min, sec, etc.)
 *     d) In that replacement pattern, find the new nonHour portion, and restore
 *        that to the original nonHour portion
 *     e) Finally, replace the original time subpattern with the adjusted
 *        replacement.
 */
U_CAPI int32_t U_EXPORT2
uadatpg_remapPatternWithOptions(UDateTimePatternGenerator *dtpg,
                                const UChar *pattern, int32_t patternLength,
                                UDateTimePatternMatchOptions options,
                                UChar *newPattern, int32_t newPatternCapacity,
                                UErrorCode *pErrorCode) {
    if (U_FAILURE(*pErrorCode)) {
        return 0;
    }
    if ( pattern==NULL || ((newPattern==NULL)? newPatternCapacity!=0: newPatternCapacity<0) ) {
        *pErrorCode = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    UnicodeString patternString((patternLength < 0), pattern, patternLength);
    UBool force12 = ((options & UADATPG_FORCE_HOUR_CYCLE_MASK) == UADATPG_FORCE_12_HOUR_CYCLE);
    UBool force24 = ((options & UADATPG_FORCE_HOUR_CYCLE_MASK) == UADATPG_FORCE_24_HOUR_CYCLE);
    if (force12 || force24) {
        UBool inQuoted = FALSE;
        UBool inTimePat = FALSE;
        UBool needReplacement = FALSE;
        int32_t timePatStart = 0;
        int32_t timePatLimit = 0;
        int32_t timeNonHourStart = -1;
        int32_t timeNonHourLimit = 0;
        UnicodeString skeleton, otherCycSkeleton;
        UnicodeString timePatChars("abBhHKkmsSzZOvVXx", -1, US_INV); // all pattern chars for times
        int32_t numForcedH = 0;
        int32_t patPos, patLen = patternString.length();

        for (patPos = 0; patPos < patLen; patPos++) {
            UChar patChr = patternString.charAt(patPos);
            UChar otherCycPatChr = patChr;
            if (patChr == 0x27 /* ASCII-range single quote */) {
                inQuoted = !inQuoted;
            } else if (!inQuoted) {
                if (timePatChars.indexOf(patChr) >= 0) {
                    // in a time pattern
                    if (!inTimePat) {
                        inTimePat = TRUE;
                        timePatStart = patPos;
                        timeNonHourStart = -1;
                        skeleton.remove();
                        otherCycSkeleton.remove();
                        numForcedH = 0;
                    }
                    if (patChr==LOW_H || patChr==CAP_K) { // hK, hour, 12-hour cycle
                        if (force24) {
                            otherCycPatChr = CAP_H; // force to H
                            needReplacement = TRUE;
                            timeNonHourStart = patPos + 1;
                            // If we are switching from a 12-hour cycle to a 24-hour cycle
                            // and the pattern for 12-hour cycle was zero-padded to 2 digits,
                            // make sure the new pattern for 24-hour cycle is also padded to
                            // 2 digits regardless of locale default, to match the
                            // expectations of the pattern provider. However we don't need
                            // to do this going the other direction (from 24- to 12-hour
                            // cycles, don't require that the 12-hour cycle has zero padding
                            // just because the 24-hour cycle did; the 12-hour cycle will
                            // add other elements such as a, so there wil be a length change
                            // anyway).
                            numForcedH++;
                        }
                    } else if (patChr==CAP_H || patChr==LOW_K) { // Hk, hour, 24-hour cycle
                        if (force12) {
                            otherCycPatChr = LOW_H; // force to h
                            needReplacement = TRUE;
                            timeNonHourStart = patPos + 1;
                        }
                    } else if (timeNonHourStart < 0 && (patChr==LOW_M || patChr==LOW_S)) { // 'm' or 's' and we did not have hour
                        timeNonHourStart = patPos;
                    }
                    skeleton.append(patChr);
                    otherCycSkeleton.append(otherCycPatChr);
                } else if ((patChr >= 0x41 && patChr <= 0x5A) || (patChr >= 0x61 && patChr <= 0x7A)) {
                    // a non-time pattern character, forces end of any time pattern
                    if (inTimePat) {
                        inTimePat = FALSE;
                        if (needReplacement) {
                            needReplacement = FALSE;
                            // do replacement
                            int32_t posAdjust = _doReplaceAndReturnAdj(dtpg, options, numForcedH >= 2, patternString, skeleton, otherCycSkeleton,
                                                                       timePatStart, timePatLimit, timeNonHourStart, timeNonHourLimit, pErrorCode);
                            patLen += posAdjust;
                            patPos += posAdjust;
                        }
                    }
                }
                if (inTimePat && !u_isWhitespace(patChr)) {
                    timePatLimit = patPos + 1;
                    if (timeNonHourStart >= 0 && patChr!=LOW_A && patChr!=LOW_B && patChr!=CAP_B) { // NonHour portion should not include 'a','b','B'
                        timeNonHourLimit = timePatLimit;
                    }
                }
            }
        }
        // end of string
        if (needReplacement) {
            // do replacement
            _doReplaceAndReturnAdj(dtpg, options, numForcedH >= 2, patternString, skeleton, otherCycSkeleton,
                                   timePatStart, timePatLimit, timeNonHourStart, timeNonHourLimit, pErrorCode);
        }
    }
    return patternString.extract(newPattern, newPatternCapacity, *pErrorCode);
}

#endif
