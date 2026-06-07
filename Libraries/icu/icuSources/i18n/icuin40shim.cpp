/*
*****************************************************************************************
* Copyright (C) 2010 Apple Inc. All Rights Reserved.
*****************************************************************************************
*/

#include "unicode/utypes.h"

#include <unicode/ucol.h>
#include <unicode/utrans.h>

/*
 * imp: i18n/ucol_res.cpp
 * hdr: i18n/unicode/ucol.h
 * @stable ICU 2.0
 */
U_CAPI UCollator* U_EXPORT2
ucol_open_4_0(const char *loc, UErrorCode *status)
{
    return ucol_open(loc, status);
}

/*
 * imp: i18n/ucol.cpp
 * hdr: i18n/unicode/ucol.h
 * @stable ICU 2.0
 */
U_CAPI void U_EXPORT2
ucol_close_4_0(UCollator *coll)
{
    ucol_close(coll);
}

/*
 * imp: i18n/ucol.cpp
 * hdr: i18n/unicode/ucol.h
 * @stable ICU 2.1
 */
U_CAPI int32_t U_EXPORT2
ucol_getBound_4_0(const uint8_t *source,
                  int32_t        sourceLength,
                  UColBoundMode  boundType,
                  uint32_t       noOfLevels,
                  uint8_t       *result,
                  int32_t        resultLength,
                  UErrorCode    *status)
{
    return ucol_getBound(source, sourceLength, boundType, noOfLevels, result, resultLength, status);
}

/*
 * imp: i18n/ucol.cpp
 * hdr: i18n/unicode/ucol.h
 * @stable ICU 2.0
 */
U_CAPI int32_t U_EXPORT2
ucol_getSortKey_4_0(const UCollator *coll,
                    const UChar     *source,
                    int32_t          sourceLength,
                    uint8_t         *result,
                    int32_t          resultLength)
{
    return ucol_getSortKey(coll, source, sourceLength, result, resultLength);
}

/*
 * imp: i18n/ucol.cpp
 * hdr: i18n/unicode/ucol.h
 * @stable ICU 2.0
 */
U_CAPI void  U_EXPORT2
ucol_setAttribute_4_0(UCollator *coll, UColAttribute attr, UColAttributeValue value, UErrorCode *status)
{
    ucol_setAttribute(coll, attr, value, status);
}

/*
 * imp: i18n/utrans.cpp
 * hdr: i18n/unicode/utrans.h
 * @stable ICU 2.8
 */
U_CAPI UTransliterator* U_EXPORT2
utrans_openU_4_0(const UChar *id,
                 int32_t idLength,
                 UTransDirection dir,
                 const UChar *rules,
                 int32_t rulesLength,
                 UParseError *parseError,
                 UErrorCode *status)
{
    return utrans_openU(id, idLength, dir, rules, rulesLength, parseError, status);
}

/*
 * imp: i18n/utrans.cpp
 * hdr: i18n/unicode/utrans.h
 * @stable ICU 2.0
 */
U_CAPI void U_EXPORT2
utrans_close_4_0(UTransliterator* trans)
{
    utrans_close(trans);
}

/*
 * imp: i18n/utrans.cpp
 * hdr: i18n/unicode/utrans.h
 * @stable ICU 2.0
 */
U_CAPI void U_EXPORT2
utrans_transUChars_4_0(const UTransliterator* trans,
                       UChar* text,
                       int32_t* textLength,
                       int32_t textCapacity,
                       int32_t start,
                       int32_t* limit,
                       UErrorCode* status)
{
    return utrans_transUChars(trans, text, textLength, textCapacity, start, limit, status);
}

