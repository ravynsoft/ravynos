/*  CFRelativeDateTimeFormatter.c
    Copyright (c) 2018-2019, Apple Inc. and the Swift project authors

    Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
    Licensed under Apache License v2.0 with Runtime Library Exception
    See http://swift.org/LICENSE.txt for license information
    See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
    Responsibility: I-Ting Liu
 */

#include "CFRelativeDateTimeFormatter.h"

#include <assert.h>
#include "CFICULogging.h"
#include "CFInternal.h"
#include "CFRuntime_Internal.h"

struct __CFRelativeDateTimeFormatter {
    CFRuntimeBase _base;
    CFRelativeDateTimeFormatterStyle _style;
    CFRelativeDateTimeFormatterUnitsStyle _unitsStyle;
    CFLocaleRef _locale;
    CFRelativeDateTimeFormattingContext _formattingContext;
};

static UDateRelativeDateTimeFormatterStyle icuRelativeDateTimeStyleFromUnitsStyle(CFRelativeDateTimeFormatterStyle style) {
    switch (style) {
        case CFRelativeDateTimeFormatterUnitsStyleSpellOut:
        case CFRelativeDateTimeFormatterUnitsStyleFull:
            return UDAT_STYLE_LONG;
        case CFRelativeDateTimeFormatterUnitsStyleShort:
            return UDAT_STYLE_SHORT;
        case CFRelativeDateTimeFormatterUnitsStyleAbbreviated:
            return UDAT_STYLE_NARROW;
        default:
            HALT_MSG("Invalid CFRelativeDateTimeFormatterStyle");
    }
}

static URelativeDateTimeUnit icuRelativeDateTimeUnitFromCFUnit(CFCalendarUnit unit) {
    switch (unit) {
        case kCFCalendarUnitYear:
            return UDAT_REL_UNIT_YEAR;
        case kCFCalendarUnitMonth:
            return UDAT_REL_UNIT_MONTH;
        case kCFCalendarUnitWeekOfMonth:
            return UDAT_REL_UNIT_WEEK;
        case kCFCalendarUnitDay:
            return UDAT_REL_UNIT_DAY;
        case kCFCalendarUnitHour:
            return UDAT_REL_UNIT_HOUR;
        case kCFCalendarUnitMinute:
            return UDAT_REL_UNIT_MINUTE;
        case kCFCalendarUnitSecond:
            return UDAT_REL_UNIT_SECOND;
        default:
            HALT_MSG("Invalid CFCalendarUnit");
    }
}

static UDisplayContext icuFormattingContextFromCFContext(CFRelativeDateTimeFormattingContext context) {
    switch (context) {
        case CFRelativeDateTimeFormattingContextUnknown:
        case CFRelativeDateTimeFormattingContextDynamic:
            return UDISPCTX_CAPITALIZATION_NONE;
        case CFRelativeDateTimeFormattingContextStandalone:
            return UDISPCTX_CAPITALIZATION_FOR_STANDALONE;
        case CFRelativeDateTimeFormattingContextListItem:
            return UDISPCTX_CAPITALIZATION_FOR_UI_LIST_OR_MENU;
        case CFRelativeDateTimeFormattingContextBeginningOfSentence:
            return UDISPCTX_CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE;
        case CFRelativeDateTimeFormattingContextMiddleOfSentence:
            return UDISPCTX_CAPITALIZATION_FOR_MIDDLE_OF_SENTENCE;
        default:
            HALT_MSG("Invalid CFRelativeDateTimeFormattingContext");
    }
}

static void __CFRelativeDateTimeFormatterDeallocate(CFTypeRef cf) {
    assert(cf != NULL);
    CFRelativeDateTimeFormatterRef formatter = (CFRelativeDateTimeFormatterRef)cf;
    if (formatter->_locale) { CFRelease(formatter->_locale); }
}

static CFStringRef __CFRelativeDateTimeFormatterCopyDescription(CFTypeRef cf) {
    assert(cf != NULL);
    CFRelativeDateTimeFormatterRef formatter = (CFRelativeDateTimeFormatterRef)cf;
    return CFStringCreateWithFormat(CFGetAllocator(formatter), NULL, CFSTR("<CFRelativeDateTimeFormatter %p>[%p]"), formatter, CFGetAllocator(formatter));
}

const CFRuntimeClass __CFRelativeDateTimeFormatterClass = {
    0,
    "CFRelativeDateTimeFormatter",
    NULL,   // init
    NULL,   // copy
    __CFRelativeDateTimeFormatterDeallocate,
    NULL,   // equal
    NULL,   // hash
    NULL,   // copy formatting desc
    __CFRelativeDateTimeFormatterCopyDescription
};

CFTypeID _CFRelativeDateTimeFormatterGetTypeID(void) {
    return _kCFRuntimeIDCFRelativeDateTimeFormatter;
}

CFRelativeDateTimeFormatterRef _CFRelativeDateTimeFormatterCreate(CFAllocatorRef allocator, CFLocaleRef locale, CFRelativeDateTimeFormatterUnitsStyle unitsStyle, CFRelativeDateTimeFormatterStyle style, CFRelativeDateTimeFormattingContext formattingContext) {
    if (allocator == NULL) { allocator = __CFGetDefaultAllocator(); }
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());

    CFAssert1(locale != NULL, __kCFLogAssertion, "%s(): locale cannot be NULL", __PRETTY_FUNCTION__);
    __CFGenericValidateType(locale, CFLocaleGetTypeID());

    size_t size = sizeof(struct __CFRelativeDateTimeFormatter) - sizeof(CFRuntimeBase);
    struct __CFRelativeDateTimeFormatter *formatter;
    formatter = (struct __CFRelativeDateTimeFormatter*)_CFRuntimeCreateInstance(allocator, _CFRelativeDateTimeFormatterGetTypeID(), size, NULL);
    if (formatter == NULL) { return NULL; }

    formatter->_locale = CFRetain(locale);
    formatter->_formattingContext = formattingContext;
    formatter->_unitsStyle = unitsStyle;
    formatter->_style = style;

    return formatter;
}

CFStringRef _CFRelativeDateTimeFormatterCreateStringWithCalendarUnit(CFAllocatorRef allocator, CFRelativeDateTimeFormatterRef formatter, CFCalendarUnit unit, double offset) {
    CFLocaleRef locale = formatter->_locale;
    CFStringRef localeIdentifier = CFLocaleGetIdentifier(locale);
    char buffer[ULOC_FULLNAME_CAPACITY];
    const char *cLocaleIdentifier = CFStringGetCStringPtr(localeIdentifier, kCFStringEncodingASCII);
    if (NULL == cLocaleIdentifier) {
        if (CFStringGetCString(localeIdentifier, buffer, ULOC_FULLNAME_CAPACITY, kCFStringEncodingASCII)) {
            cLocaleIdentifier = buffer;
        }
    }

    UNumberFormat *numFmt = NULL;
    CFRelativeDateTimeFormatterUnitsStyle unitsStyle = formatter->_unitsStyle;
    if (unitsStyle == CFRelativeDateTimeFormatterUnitsStyleSpellOut) {
        UErrorCode status = U_ZERO_ERROR;
        numFmt = __cficu_unum_open(UNUM_SPELLOUT, NULL, 0, cLocaleIdentifier, NULL, &status);
        if (U_FAILURE(status)) {
            if (numFmt) {
                __cficu_unum_close(numFmt);
            }
            return NULL;
        }
    }

    UDateRelativeDateTimeFormatterStyle style = icuRelativeDateTimeStyleFromUnitsStyle(unitsStyle);

    UErrorCode status = U_ZERO_ERROR;
    UDisplayContext context = icuFormattingContextFromCFContext(formatter->_formattingContext);
    // This takes over the ownership of numFmt, so there's not need to close numFmt.
    URelativeDateTimeFormatter *fmt = __cficu_ureldatefmt_open(cLocaleIdentifier, numFmt, style, context, &status);
    if (U_FAILURE(status)) {
        if (fmt) {
            __cficu_ureldatefmt_close(fmt);
        }
        return NULL;
    }

    URelativeDateTimeUnit icuUnit = icuRelativeDateTimeUnitFromCFUnit(unit);

    int32_t len = 0;
    const int32_t RESULT_BUFFER_SIZE = 128;
    UChar result[RESULT_BUFFER_SIZE + 1];
    status = U_ZERO_ERROR;
    CFRelativeDateTimeFormatterStyle dateTimeStyle = formatter->_style;
    switch (dateTimeStyle) {
        case CFRelativeDateTimeFormatterStyleNumeric:
            len = __cficu_ureldatefmt_formatNumeric(fmt, offset, icuUnit, result, RESULT_BUFFER_SIZE, &status);
            break;
        case CFRelativeDateTimeFormatterStyleNamed:
            len = __cficu_ureldatefmt_format(fmt, offset, icuUnit, result, RESULT_BUFFER_SIZE, &status);
            break;
        default:
            HALT_MSG("Invalid CFRelativeDateTimeFormatterStyle");
    }

    CFAssert1(fmt != NULL, __kCFLogAssertion, "%s(): URelativeDateTimeFormatter should not be NULL", __PRETTY_FUNCTION__);
    __cficu_ureldatefmt_close(fmt);

    if (U_FAILURE(status)){ return NULL; }

    if (allocator == NULL) { allocator = __CFGetDefaultAllocator(); }
    return CFStringCreateWithCharacters(allocator, result, len);
}

