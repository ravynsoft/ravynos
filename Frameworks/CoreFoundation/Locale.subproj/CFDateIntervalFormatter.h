/*	CFDateIntervalFormatter.h
	Copyright (c) 1998-2018, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFDATEINTERVALFORMATTER__)
#define __COREFOUNDATION_CFDATEINTERVALFORMATTER__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFDateInterval.h>
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFTimeZone.h>

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct CF_BRIDGED_TYPE(id) __CFDateIntervalFormatter * CFDateIntervalFormatterRef;

typedef CF_ENUM(CFIndex, CFDateIntervalFormatterStyle) {
    kCFDateIntervalFormatterNoStyle = 0,
    kCFDateIntervalFormatterShortStyle = 1,
    kCFDateIntervalFormatterMediumStyle = 2,
    kCFDateIntervalFormatterLongStyle = 3,
    kCFDateIntervalFormatterFullStyle = 4,
};

typedef CF_ENUM(CFIndex, _CFDateIntervalFormatterBoundaryStyle) {
    kCFDateIntervalFormatterBoundaryStyleDefault = 0,
#if TARGET_OS_MAC
    kCFDateIntervalFormatterBoundaryStyleMinimizeAdjacentMonths = 1,
#endif
};

CF_EXPORT CFDateIntervalFormatterRef CFDateIntervalFormatterCreate(CFAllocatorRef _Nullable allocator, CFLocaleRef _Nullable locale, CFDateIntervalFormatterStyle dateStyle, CFDateIntervalFormatterStyle timeStyle);
CF_EXPORT CFDateIntervalFormatterRef CFDateIntervalFormatterCreateCopy(CFAllocatorRef _Nullable allocator, CFDateIntervalFormatterRef formatter);

CF_EXPORT CFLocaleRef CFDateIntervalFormatterCopyLocale(CFDateIntervalFormatterRef formatter);
CF_EXPORT void CFDateIntervalFormatterSetLocale(CFDateIntervalFormatterRef formatter, CFLocaleRef _Nullable locale);

CF_EXPORT CFCalendarRef CFDateIntervalFormatterCopyCalendar(CFDateIntervalFormatterRef formatter);
CF_EXPORT void CFDateIntervalFormatterSetCalendar(CFDateIntervalFormatterRef formatter, CFCalendarRef _Nullable calendar);

CF_EXPORT CFTimeZoneRef CFDateIntervalFormatterCopyTimeZone(CFDateIntervalFormatterRef formatter);
CF_EXPORT void CFDateIntervalFormatterSetTimeZone(CFDateIntervalFormatterRef formatter, CFTimeZoneRef _Nullable timeZone);

CF_EXPORT CFStringRef CFDateIntervalFormatterCopyDateTemplate(CFDateIntervalFormatterRef formatter);
CF_EXPORT void CFDateIntervalFormatterSetDateTemplate(CFDateIntervalFormatterRef formatter, CFStringRef _Nullable dateTemplate);

CF_EXPORT CFDateIntervalFormatterStyle CFDateIntervalFormatterGetDateStyle(CFDateIntervalFormatterRef formatter);
CF_EXPORT void CFDateIntervalFormatterSetDateStyle(CFDateIntervalFormatterRef formatter, CFDateIntervalFormatterStyle dateStyle);

CF_EXPORT CFDateIntervalFormatterStyle CFDateIntervalFormatterGetTimeStyle(CFDateIntervalFormatterRef formatter);
CF_EXPORT void CFDateIntervalFormatterSetTimeStyle(CFDateIntervalFormatterRef formatter, CFDateIntervalFormatterStyle timeStyle);

CF_EXPORT CFStringRef CFDateIntervalFormatterCreateStringFromDateToDate(CFDateIntervalFormatterRef formatter, CFDateRef fromDate, CFDateRef toDate);
CF_EXPORT CFStringRef CFDateIntervalFormatterCreateStringFromDateInterval(CFDateIntervalFormatterRef formatter, CFDateIntervalRef interval);

// SPI:
CF_EXPORT _CFDateIntervalFormatterBoundaryStyle _CFDateIntervalFormatterGetBoundaryStyle(CFDateIntervalFormatterRef formatter);
CF_EXPORT void _CFDateIntervalFormatterSetBoundaryStyle(CFDateIntervalFormatterRef formatter, _CFDateIntervalFormatterBoundaryStyle boundaryStyle);

CF_EXPORT void _CFDateIntervalFormatterInitializeFromCoderValues(CFDateIntervalFormatterRef formatter,
                                                                 int64_t dateStyle,
                                                                 int64_t timeStyle,
                                                                 CFStringRef _Nullable dateTemplate,
                                                                 CFStringRef _Nullable dateTemplateFromStyles,
                                                                 Boolean modified,
                                                                 Boolean useTemplate,
                                                                 CFLocaleRef _Nullable locale,
                                                                 CFCalendarRef _Nullable calendar,
                                                                 CFTimeZoneRef _Nullable timeZone);

CF_EXPORT void _CFDateIntervalFormatterCopyCoderValues(CFDateIntervalFormatterRef formatter,
                                                       int64_t *dateStyle,
                                                       int64_t *timeStyle,
                                                       CFStringRef _Nullable *_Nonnull dateTemplate,
                                                       CFStringRef _Nullable *_Nonnull dateTemplateFromStyles,
                                                       Boolean *modified,
                                                       Boolean *useTemplate,
                                                       CFLocaleRef _Nullable *_Nonnull locale,
                                                       CFCalendarRef _Nullable *_Nonnull calendar,
                                                       CFTimeZoneRef _Nullable *_Nonnull timeZone);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

#endif /* !defined(__COREFOUNDATION_CFDATEINTERVALFORMATTER__) */

