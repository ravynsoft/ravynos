/*  CFRelativeDateTimeFormatter.h
    Copyright (c) 2018-2019, Apple Inc. and the Swift project authors

    Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
    Licensed under Apache License v2.0 with Runtime Library Exception
    See http://swift.org/LICENSE.txt for license information
    See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#ifndef __COREFOUNDATION_CFRELATIVEDATETIMEFORMATTER_h
#define __COREFOUNDATION_CFRELATIVEDATETIMEFORMATTER_h

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFCalendar.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN
CF_ASSUME_NONNULL_BEGIN

// Values must match NSRelativeDateTimeFormatterUnitsStyle.
typedef CF_ENUM(CFIndex, CFRelativeDateTimeFormatterUnitsStyle) {
    CFRelativeDateTimeFormatterUnitsStyleFull = 0, // "2 months ago"
    CFRelativeDateTimeFormatterUnitsStyleSpellOut, // "two months ago"
    CFRelativeDateTimeFormatterUnitsStyleShort, // "2 mo. ago"
    CFRelativeDateTimeFormatterUnitsStyleAbbreviated, // "2 mo. ago"; might give different results in languages other than English
} API_AVAILABLE(macosx(10.15), ios(13.0), watchos(6.0), tvos(13.0));

// Values must match NSRelativeDateTimeFormatterStyle.
typedef CF_ENUM(CFIndex, CFRelativeDateTimeFormatterStyle) {
    CFRelativeDateTimeFormatterStyleNumeric = 0, // "1 day ago", "2 days ago", "1 week ago", "in 1 week"
    CFRelativeDateTimeFormatterStyleNamed, // “yesterday”, "2 days ago", "last week", "next week"; falls back to the numeric style if no name is available
} API_AVAILABLE(macosx(10.15), ios(13.0), watchos(6.0), tvos(13.0));

/* Values must match NSFormattingContext.
 */
typedef CF_ENUM(CFIndex, CFRelativeDateTimeFormattingContext) {
    CFRelativeDateTimeFormattingContextUnknown = 0,
    CFRelativeDateTimeFormattingContextDynamic,
    CFRelativeDateTimeFormattingContextStandalone,
    CFRelativeDateTimeFormattingContextListItem,
    CFRelativeDateTimeFormattingContextBeginningOfSentence,
    CFRelativeDateTimeFormattingContextMiddleOfSentence,
} API_AVAILABLE(macosx(10.15), ios(13.0), watchos(6.0), tvos(13.0));

typedef struct CF_BRIDGED_MUTABLE_TYPE(id) __CFRelativeDateTimeFormatter *CFRelativeDateTimeFormatterRef;

CF_EXPORT
CFTypeID _CFRelativeDateTimeFormatterGetTypeID(void);

CF_EXPORT
CFRelativeDateTimeFormatterRef _Nullable _CFRelativeDateTimeFormatterCreate(CFAllocatorRef allocator, CFLocaleRef locale, CFRelativeDateTimeFormatterUnitsStyle unitsStyle, CFRelativeDateTimeFormatterStyle style, CFRelativeDateTimeFormattingContext formattingContext);

CF_EXPORT
CFStringRef _Nullable _CFRelativeDateTimeFormatterCreateStringWithCalendarUnit(CFAllocatorRef allocator, CFRelativeDateTimeFormatterRef formatter, CFCalendarUnit unit, CFTimeInterval offset);

CF_ASSUME_NONNULL_END
CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif // __COREFOUNDATION_CFRELATIVEDATETIMEFORMATTER_h
