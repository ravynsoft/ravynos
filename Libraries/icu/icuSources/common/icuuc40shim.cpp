/*
*****************************************************************************************
* Copyright (C) 2010 Apple Inc. All Rights Reserved.
*****************************************************************************************
*/

#include <unicode/utypes.h>
#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/uidna.h>
#include <unicode/uscript.h>
#include <unicode/ustring.h>
#include <unicode/utf8.h>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/locid.h>

extern "C" {

/*
 * imp: common/uchar.c
 * hdr: common/unicode/uchar.h
 * @stable ICU 2.0
 */
U_CAPI int8_t U_EXPORT2
u_charType_4_0(UChar32 c)
{
    return u_charType(c);
}

} // end extern "C"

/*
 * imp: common/uchar.c
 * hdr: common/unicode/uchar.h
 * @stable ICU 2.0
 */
U_CAPI UBool U_EXPORT2
u_isUWhiteSpace_4_0(UChar32 c)
{
    return u_isUWhiteSpace(c);
}

/*
 * imp: common/uchar.c
 * hdr: common/unicode/uchar.h
 * @stable ICU 2.0
 */
U_CAPI UBool U_EXPORT2
u_isprint_4_0(UChar32 c)
{
    return u_isprint(c);
}

/*
 * imp: common/uchar.c
 * hdr: common/unicode/uchar.h
 * @stable ICU 2.0
 */
U_CAPI UBool U_EXPORT2
u_isWhitespace_4_0(UChar32 c)
{
    return u_isWhitespace(c);
}

/*
 * imp: common/uchar.c
 * hdr: common/unicode/uchar.h
 * @stable ICU 2.0
 */
U_CAPI UBool U_EXPORT2
u_isspace_4_0(UChar32 c)
{
    return u_isspace(c);
}

/*
 * imp: common/ucase.c
 * hdr: common/unicode/uchar.h
 * @stable ICU 2.0
 */
U_CAPI UChar32 U_EXPORT2
u_foldCase_4_0(UChar32 c, uint32_t options) {
    return u_foldCase(c, options);
}

/*
 * imp: common/uprops.cpp
 * hdr: common/unicode/uchar.h
 * @stable ICU 2.1
 */
U_CAPI UBool U_EXPORT2
u_hasBinaryProperty_4_0(UChar32 c, UProperty which)
{
    return u_hasBinaryProperty(c, which);
}

/*
 * imp: common/propname.cpp
 * hdr: common/unicode/uchar.h
 * @stable ICU 2.4
 */
U_CAPI int32_t U_EXPORT2
u_getPropertyValueEnum_4_0(UProperty property,
                       const char* alias)
{
    return u_getPropertyValueEnum(property, alias);
}

/*
 * imp: common/uchar.c
 * hdr: common/unicode/uscript.h
 * @stable ICU 2.4
 */
U_CAPI UScriptCode  U_EXPORT2 
uscript_getScript_4_0(UChar32 codepoint, UErrorCode *err)

{
    return uscript_getScript(codepoint, err);
}

/*
 * imp: common/ustrcase.c
 * hdr: common/unicode/ustring.h
 * @stable ICU 2.0
 */
U_CAPI int32_t U_EXPORT2
u_memcasecmp_4_0(const UChar *s1, const UChar *s2, int32_t length, uint32_t options) {
    return u_memcasecmp(s1, s2, length, options);
}

/*
 * imp: common/utypes.c
 * hdr: common/unicode/utypes.h
 * @stable ICU 2.0
 */
U_CAPI const char * U_EXPORT2
u_errorName_4_0(UErrorCode code)
{
    return u_errorName(code);
}

/*
 * imp: common/ubrk.cpp
 * hdr: common/unicode/ubrk.h
 * @stable ICU 2.0
 * #if !UCONFIG_NO_BREAK_ITERATION
 * (don't actually conditionalize this, if the underlying library is not
 * built with break iteration, we want to fail at build time, not runtime)
 */
U_CAPI UBreakIterator* U_EXPORT2
ubrk_open_4_0(UBreakIteratorType type,
      const char *locale,
      const UChar *text,
      int32_t textLength,
      UErrorCode *status)
{
    return ubrk_open(type, locale, text, textLength, status);
}

/*
 * imp: common/ubrk.cpp
 * hdr: common/unicode/ubrk.h
 * @stable ICU 2.0
 * #if !UCONFIG_NO_BREAK_ITERATION
 * (don't actually conditionalize this, if the underlying library is not
 * built with break iteration, we want to fail at build time, not runtime)
 */
U_CAPI void U_EXPORT2
ubrk_close_4_0(UBreakIterator *bi)
{
    return ubrk_close(bi);
}

/*
 * imp: common/ubrk.cpp
 * hdr: common/unicode/ubrk.h
 * @stable ICU 2.0
 * #if !UCONFIG_NO_BREAK_ITERATION
 * (don't actually conditionalize this, if the underlying library is not
 * built with break iteration, we want to fail at build time, not runtime)
 */
U_CAPI void U_EXPORT2
ubrk_setText_4_0(UBreakIterator* bi,
             const UChar*    text,
             int32_t         textLength,
             UErrorCode*     status)
{
    return ubrk_setText(bi, text, textLength, status);
}

/*
 * imp: common/ubrk.cpp
 * hdr: common/unicode/ubrk.h
 * @stable ICU 2.0
 * #if !UCONFIG_NO_BREAK_ITERATION
 * (don't actually conditionalize this, if the underlying library is not
 * built with break iteration, we want to fail at build time, not runtime)
 */
U_CAPI int32_t U_EXPORT2
ubrk_first_4_0(UBreakIterator *bi)
{
    return ubrk_first(bi);
}

/*
 * imp: common/ubrk.cpp
 * hdr: common/unicode/ubrk.h
 * @stable ICU 2.0
 * #if !UCONFIG_NO_BREAK_ITERATION
 * (don't actually conditionalize this, if the underlying library is not
 * built with break iteration, we want to fail at build time, not runtime)
 */
U_CAPI int32_t U_EXPORT2
ubrk_last_4_0(UBreakIterator *bi)
{
    return ubrk_last(bi);
}

/*
 * imp: common/ubrk.cpp
 * hdr: common/unicode/ubrk.h
 * @stable ICU 2.0
 * #if !UCONFIG_NO_BREAK_ITERATION
 * (don't actually conditionalize this, if the underlying library is not
 * built with break iteration, we want to fail at build time, not runtime)
 */
U_CAPI int32_t U_EXPORT2
ubrk_next_4_0(UBreakIterator *bi)
{
    return ubrk_next(bi);
}

/*
 * imp: common/ubrk.cpp
 * hdr: common/unicode/ubrk.h
 * @stable ICU 2.0
 * #if !UCONFIG_NO_BREAK_ITERATION
 * (don't actually conditionalize this, if the underlying library is not
 * built with break iteration, we want to fail at build time, not runtime)
 */
U_CAPI int32_t U_EXPORT2
ubrk_previous_4_0(UBreakIterator *bi)
{
    return ubrk_previous(bi);
}

/*
 * imp: common/ubrk.cpp
 * hdr: common/unicode/ubrk.h
 * @stable ICU 2.0
 * #if !UCONFIG_NO_BREAK_ITERATION
 * (don't actually conditionalize this, if the underlying library is not
 * built with break iteration, we want to fail at build time, not runtime)
 */
U_CAPI int32_t U_EXPORT2
ubrk_preceding_4_0(UBreakIterator *bi,
           int32_t offset)
{
    return ubrk_preceding(bi, offset);
}

/*
 * imp: common/ubrk.cpp
 * hdr: common/unicode/ubrk.h
 * @stable ICU 2.0
 * #if !UCONFIG_NO_BREAK_ITERATION
 * (don't actually conditionalize this, if the underlying library is not
 * built with break iteration, we want to fail at build time, not runtime)
 */
U_CAPI int32_t U_EXPORT2
ubrk_following_4_0(UBreakIterator *bi,
           int32_t offset)
{
    return ubrk_following(bi, offset);
}


/*
 * imp: common/uidna.cpp
 * hdr: common/unicode/uidna.h
 * @stable ICU 2.6
 */
U_CAPI int32_t U_EXPORT2
uidna_IDNToUnicode_4_0(  const UChar* src, int32_t srcLength,
                     UChar* dest, int32_t destCapacity,
                     int32_t options,
                     UParseError* parseError,
                     UErrorCode* status)
{
    return uidna_IDNToUnicode(src, srcLength, dest, destCapacity, options, parseError, status);
}

/*
 * imp: common/uidna.cpp
 * hdr: common/unicode/uidna.h
 * @stable ICU 2.6
 */
U_CAPI int32_t U_EXPORT2
uidna_IDNToASCII_4_0(  const UChar* src, int32_t srcLength,
                   UChar* dest, int32_t destCapacity,
                   int32_t options,
                   UParseError* parseError,
                   UErrorCode* status)
{
    return uidna_IDNToASCII(src, srcLength, dest, destCapacity, options, parseError, status);
}


/*
 * imp: common/utf_impl.c
 * hdr: common/unicode/utf8.h
 * @internal
 */
U_CAPI int32_t U_EXPORT2
utf8_appendCharSafeBody_4_0(uint8_t *s, int32_t i, int32_t length, UChar32 c, UBool *pIsError)
{
    return utf8_appendCharSafeBody(s, i, length, c, pIsError);
}


namespace icu_4_0 {

/*
 * imp: common/locid.cpp
 * hdr: common/unicode/locid.h
 */
class __declspec(dllexport) Locale : public icu::Locale {
public:
    // @stable ICU 2.0
    Locale(const char* language, const char* country = 0, const char* variant = 0, const char* keywordsAndValues = 0)
    : icu::Locale(language, country, variant, keywordsAndValues)
    {
    }
};


/*
 * imp: common/unistr.cpp
 * hdr: common/unicode/unistr.h
 */
class __declspec(dllexport) UnicodeString : public icu::UnicodeString {
public:
    UnicodeString(UBool isTerminated, const UChar *text, int32_t textLength)
    : icu::UnicodeString(isTerminated, text, textLength)
    {
    }

    UnicodeString(int32_t capacity, UChar32 c, int32_t count)
    : icu::UnicodeString(capacity, c, count)
    {
    }

    UnicodeString()
    : icu::UnicodeString()
    {
    }

    UChar *getBuffer(int32_t minCapacity)
    {
        return icu::UnicodeString::getBuffer(minCapacity);
    }

    void releaseBuffer(int32_t newLength=-1)
    {
        return icu::UnicodeString::releaseBuffer(newLength);
    }
};


/*
 * imp: common/brkiter.cpp
 * hdr: common/unicode/brkiter.h
 */
class __declspec(dllexport) BreakIterator : public icu::BreakIterator {
public:
    // @stable ICU 2.0
    static BreakIterator* U_EXPORT2
    createWordInstance(const icu_4_0::Locale& where, UErrorCode& status)
    {
        return static_cast<BreakIterator*>(icu::BreakIterator::createWordInstance(where, status));
    }
};

}; // end namespace icu_4_0
