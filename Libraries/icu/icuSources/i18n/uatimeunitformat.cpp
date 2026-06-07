/*
*****************************************************************************************
* Copyright (C) 2014-2015 Apple Inc. All Rights Reserved.
*****************************************************************************************
*/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/uatimeunitformat.h"
#include "unicode/fieldpos.h"
#include "unicode/localpointer.h"
#include "unicode/numfmt.h"
#include "unicode/measunit.h"
#include "unicode/measure.h"
#include "unicode/measfmt.h"
#include "unicode/unistr.h"
#include "unicode/unum.h"
#include "unicode/ures.h"
#include "unicode/ustring.h"
#include "ureslocs.h"
#include "uresimp.h"
#include "ustr_imp.h"

U_NAMESPACE_USE


U_CAPI UATimeUnitFormat* U_EXPORT2
uatmufmt_open(const char*  locale,
              UATimeUnitStyle style,
              UErrorCode*  status)
{
    return uatmufmt_openWithNumberFormat(locale, style, NULL, status);
}


U_CAPI UATimeUnitFormat* U_EXPORT2
uatmufmt_openWithNumberFormat(const char*  locale,
                            UATimeUnitStyle style,
                            UNumberFormat*  nfToAdopt,
                            UErrorCode*  status)
{
    if (U_FAILURE(*status)) {
        return NULL;
    }
    UMeasureFormatWidth mfWidth = UMEASFMT_WIDTH_WIDE;
    switch (style) {
        case UATIMEUNITSTYLE_FULL:
            break;
        case UATIMEUNITSTYLE_ABBREVIATED:
            mfWidth = UMEASFMT_WIDTH_SHORT; break;
        case UATIMEUNITSTYLE_NARROW:
            mfWidth = UMEASFMT_WIDTH_NARROW; break;
        case UATIMEUNITSTYLE_SHORTER:
            mfWidth = UMEASFMT_WIDTH_SHORTER; break;
        default:
            *status = U_ILLEGAL_ARGUMENT_ERROR; return NULL;
    }
    LocalPointer<MeasureFormat> mfmt( new MeasureFormat(Locale(locale), mfWidth, (NumberFormat*)nfToAdopt, *status) );
    if (U_FAILURE(*status)) {
        return NULL;
    }
    return (UATimeUnitFormat*)mfmt.orphan();
}


U_CAPI void U_EXPORT2
uatmufmt_close(UATimeUnitFormat *mfmt)
{
    delete (MeasureFormat*)mfmt;
}


U_CAPI void U_EXPORT2
uatmufmt_setNumberFormat(UATimeUnitFormat* mfmt,
                        UNumberFormat*  numfmt,
                        UErrorCode*     status)
{
    if (U_FAILURE(*status)) {
        return;
    }
    ((MeasureFormat*)mfmt)->adoptNumberFormat( (NumberFormat*)(((NumberFormat*)numfmt)->clone()), *status );
}


U_CAPI int32_t U_EXPORT2
uatmufmt_format(const UATimeUnitFormat* mfmt,
                double          value,
                UATimeUnitField field,
                UChar*          result,
                int32_t         resultCapacity,
                UErrorCode*     status)
{
    if (U_FAILURE(*status)) {
        return 0;
    }
    if ( ((result==NULL)? resultCapacity!=0: resultCapacity<0) ) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    MeasureUnit * munit = NULL;
    switch (field) {
        case UATIMEUNITFIELD_YEAR:    munit = MeasureUnit::createYear(*status);    break;
        case UATIMEUNITFIELD_MONTH:   munit = MeasureUnit::createMonth(*status);   break;
        case UATIMEUNITFIELD_DAY:     munit = MeasureUnit::createDay(*status);     break;
        case UATIMEUNITFIELD_WEEK:    munit = MeasureUnit::createWeek(*status);    break;
        case UATIMEUNITFIELD_HOUR:    munit = MeasureUnit::createHour(*status);    break;
        case UATIMEUNITFIELD_MINUTE:  munit = MeasureUnit::createMinute(*status);  break;
        case UATIMEUNITFIELD_SECOND:  munit = MeasureUnit::createSecond(*status);  break;
        case UATIMEUNITFIELD_MILLISECOND: munit = MeasureUnit::createMillisecond(*status); break;
        case UATIMEUNITFIELD_MICROSECOND: munit = MeasureUnit::createMicrosecond(*status); break;
        case UATIMEUNITFIELD_NANOSECOND:  munit = MeasureUnit::createNanosecond(*status);  break;
        default: *status = U_ILLEGAL_ARGUMENT_ERROR; break;
    }
    if (U_FAILURE(*status)) {
        return 0;
    }
    LocalPointer<Measure> meas(new Measure(value, munit, *status));
    if (U_FAILURE(*status)) {
        return 0;
    }
    Formattable fmt;
    fmt.adoptObject(meas.orphan());
    UnicodeString res;
    res.setTo(result, 0, resultCapacity);
    FieldPosition pos(0);
    ((MeasureFormat*)mfmt)->format(fmt, res, pos, *status);
    return res.extract(result, resultCapacity, *status);
}


U_CAPI double U_EXPORT2
uatmufmt_parse( const UATimeUnitFormat*,
                const UChar*,
                int32_t,
                int32_t*,
                UATimeUnitField*,
                UErrorCode*     status)
{
    if (!U_FAILURE(*status)) {
        *status = U_UNSUPPORTED_ERROR;
    }
    return 0.0;
}


U_CAPI int32_t U_EXPORT2
uatmufmt_getTimePattern(const char*     locale,
                        UATimeUnitTimePattern type,
                        UChar*          result,
                        int32_t         resultCapacity,
                        UErrorCode*     status)
{
    if (U_FAILURE(*status)) {
        return 0;
    }
    if ( (result==NULL)? resultCapacity!=0: resultCapacity<0 ) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    const char* key = NULL;
    switch (type) {
        case UATIMEUNITTIMEPAT_HM:  { key = "hm"; break; }
        case UATIMEUNITTIMEPAT_HMS: { key = "hms"; break; }
        case UATIMEUNITTIMEPAT_MS:  { key = "ms"; break; }
        default: { *status = U_ILLEGAL_ARGUMENT_ERROR; return 0; }
    }
    int32_t resLen = 0;
    const UChar* resPtr = NULL;
    UResourceBundle* rb =  ures_open(U_ICUDATA_UNIT, locale, status);
    rb = ures_getByKeyWithFallback(rb, "durationUnits", rb, status);
    resPtr = ures_getStringByKeyWithFallback(rb, key, &resLen, status);
    if (U_SUCCESS(*status)) {
        u_strncpy(result, resPtr, resultCapacity);
    }
    ures_close(rb);
    return u_terminateUChars(result, resultCapacity, resLen, status);
}


U_CAPI int32_t U_EXPORT2
uatmufmt_getListPattern(const char*     locale,
                        UATimeUnitStyle style,
                        UATimeUnitListPattern type,
                        UChar*          result,
                        int32_t         resultCapacity,
                        UErrorCode*     status)
{
    if (U_FAILURE(*status)) {
        return 0;
    }
    if ( (result==NULL)? resultCapacity!=0: resultCapacity<0 ) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    const char* styleKey = NULL;
    switch (style) {
        case UATIMEUNITSTYLE_FULL:          { styleKey = "unit"; break; }
        case UATIMEUNITSTYLE_ABBREVIATED:   { styleKey = "unit-short"; break; }
        case UATIMEUNITSTYLE_NARROW:        { styleKey = "unit-narrow"; break; }
        case UATIMEUNITSTYLE_SHORTER:       { styleKey = "unit-narrow"; break; }
        default: { *status = U_ILLEGAL_ARGUMENT_ERROR; return 0; }
    }
    const char* typeKey = NULL;
    switch (type) {
        case UATIMEUNITLISTPAT_TWO_ONLY:        { typeKey = "2"; break; }
        case UATIMEUNITLISTPAT_END_PIECE:       { typeKey = "end"; break; }
        case UATIMEUNITLISTPAT_MIDDLE_PIECE:    { typeKey = "middle"; break; }
        case UATIMEUNITLISTPAT_START_PIECE:     { typeKey = "start"; break; }
        default: { *status = U_ILLEGAL_ARGUMENT_ERROR; return 0; }
    }
    int32_t resLen = 0;
    const UChar* resPtr = NULL;
    UResourceBundle* rb =  ures_open(NULL, locale, status);
    rb = ures_getByKeyWithFallback(rb, "listPattern", rb, status);
    rb = ures_getByKeyWithFallback(rb, styleKey, rb, status);
    resPtr = ures_getStringByKeyWithFallback(rb, typeKey, &resLen, status);
    if (U_SUCCESS(*status)) {
        u_strncpy(result, resPtr, resultCapacity);
    }
    ures_close(rb);
    return u_terminateUChars(result, resultCapacity, resLen, status);
}


#endif /* #if !UCONFIG_NO_FORMATTING */
