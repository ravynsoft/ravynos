/*    CFCalendar_Internal.h
    Copyright (c) 2004-2018, Apple Inc. and the Swift project authors

    Portions Copyright (c) 2014-2018, Apple Inc. and the Swift project authors
    Licensed under Apache License v2.0 with Runtime Library Exception
    See http://swift.org/LICENSE.txt for license information
    See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFCALENDAR_INTERNAL__)
#define __COREFOUNDATION_CFCALENDAR_INTERNAL__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CFTimeZone.h>
#include <CoreFoundation/CFString.h>

#include "CFDateComponents.h"
#include "CFDateInterval.h"

#if __has_include(<unicode/ucal.h>) && !defined(__cplusplus)
#include <unicode/ucal.h>
#else
typedef void *UCalendar;
#endif

CF_ASSUME_NONNULL_BEGIN

/*
typedef void (^CFCalendarEnumerateBlock)(CFDateRef date, bool exactMatch, bool *stop);

CF_EXPORT
void CFCalendarEnumerateDatesStartingAfterDate(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef components, CFCalendarEnumerateBlock block);
*/

struct __CFCalendar {
    CFRuntimeBase _base;
    CFStringRef _identifier;    // canonical identifier, never NULL
    CFLocaleRef _Nullable _locale;
    CFTimeZoneRef _Nullable _tz;
    CFIndex _firstWeekday;
    CFIndex _minDaysInFirstWeek;
    CFDateRef _Nullable _gregorianStart;  // NULL if not Gregorian calendar
    UCalendar _Nonnull * _Nullable _cal;
    Boolean _userSet_firstWeekday;
    Boolean _userSet_minDaysInFirstWeek;
    Boolean _userSet_gregorianStart;
};

struct __CFDateComponents {
    CFRuntimeBase _base;
    CFCalendarRef _Nullable _calendar;
    CFTimeZoneRef _Nullable _timeZone;
    CFIndex _era;
    CFIndex _year;
    CFIndex _month;
    CFIndex _leapMonth;
    CFIndex _day;
    CFIndex _hour;
    CFIndex _minute;
    CFIndex _second;
    CFIndex _week;  // Deprecated
    CFIndex _weekday;
    CFIndex _weekdayOrdinal;
    CFIndex _quarter;
    CFIndex _weekOfMonth;
    CFIndex _weekOfYear;
    CFIndex _yearForWeekOfYear;
    CFIndex _nanosecond;
};

typedef struct {
    CFTimeInterval onsetTime;
    CFTimeInterval ceaseTime;
    CFIndex start;
    CFIndex end;
} _CFCalendarWeekendRange;

// Additional options for enumeration
CF_ENUM(CFOptionFlags) {
    kCFCalendarMatchStrictly = (1ULL << 1),
    kCFCalendarSearchBackwards = (1ULL << 2),
    kCFCalendarMatchPreviousTimePreservingSmallerUnits = (1ULL << 8),
    kCFCalendarMatchNextTimePreservingSmallerUnits = (1ULL << 9),
    kCFCalendarMatchNextTime = (1ULL << 10),
    kCFCalendarMatchFirst = (1ULL << 12),
    kCFCalendarMatchLast = (1ULL << 13)
};

CF_PRIVATE void __CFCalendarSetupCal(CFCalendarRef calendar);
CF_PRIVATE void __CFCalendarZapCal(CFCalendarRef calendar);

CF_CROSS_PLATFORM_EXPORT Boolean _CFCalendarInitWithIdentifier(CFCalendarRef calendar, CFStringRef identifier);

CF_PRIVATE CFCalendarRef _CFCalendarCreateCopy(CFAllocatorRef allocator, CFCalendarRef calendar);

CF_CROSS_PLATFORM_EXPORT Boolean _CFCalendarComposeAbsoluteTimeV(CFCalendarRef calendar, /* out */ CFAbsoluteTime *atp, const char *componentDesc, int32_t *vector, int32_t count);
CF_CROSS_PLATFORM_EXPORT Boolean _CFCalendarDecomposeAbsoluteTimeV(CFCalendarRef calendar, CFAbsoluteTime at, const char *componentDesc, int32_t *_Nonnull * _Nonnull vector, int32_t count);
CF_CROSS_PLATFORM_EXPORT Boolean _CFCalendarAddComponentsV(CFCalendarRef calendar, /* inout */ CFAbsoluteTime *atp, CFOptionFlags options, const char *componentDesc, int32_t *vector, int32_t count);
CF_CROSS_PLATFORM_EXPORT Boolean _CFCalendarGetComponentDifferenceV(CFCalendarRef calendar, CFAbsoluteTime startingAT, CFAbsoluteTime resultAT, CFOptionFlags options, const char *componentDesc, int32_t *_Nonnull * _Nonnull vector, int32_t count);
CF_PRIVATE Boolean _CFCalendarIsDateInWeekend(CFCalendarRef calendar, CFDateRef date);
CF_PRIVATE Boolean _CFCalendarGetNextWeekend(CFCalendarRef calendar, _CFCalendarWeekendRange *range);

CF_PRIVATE CFStringRef _CFDateComponentsCopyDescriptionInner(CFDateComponentsRef dc);
CF_PRIVATE _Nullable CFDateRef _CFCalendarCreateDateByAddingDateComponentsToDate(CFAllocatorRef allocator, CFCalendarRef calendar, CFDateComponentsRef dateComp, CFDateRef date, CFOptionFlags opts);

CF_PRIVATE Boolean _CFCalendarGetTimeRangeOfUnitForDate(CFCalendarRef calendar, CFCalendarUnit unit, CFDateRef _Nonnull * _Nullable startDate, CFTimeInterval * _Nullable tip, CFDateRef date);
CF_PRIVATE _Nullable CFDateRef _CFCalendarCreateStartDateForTimeRangeOfUnitForDate(CFCalendarRef calendar, CFCalendarUnit unit, CFDateRef date, CFTimeInterval * _Nullable tip);
CF_PRIVATE _Nullable CFDateIntervalRef _CFCalendarCreateDateInterval(CFAllocatorRef allocator, CFCalendarRef calendar, CFCalendarUnit unit, CFDateRef date);
CF_PRIVATE _Nullable CFDateRef _CFCalendarCreateDateByAddingValueOfUnitToDate(CFCalendarRef calendar, CFIndex val, CFCalendarUnit unit, CFDateRef date);
    
CF_CROSS_PLATFORM_EXPORT void _CFCalendarEnumerateDates(CFCalendarRef calendar, CFDateRef start, CFDateComponentsRef matchingComponents, CFOptionFlags opts, void (^block)(CFDateRef _Nullable, Boolean, Boolean*));

CF_EXPORT void CFCalendarSetGregorianStartDate(CFCalendarRef calendar, CFDateRef _Nullable date);
CF_EXPORT _Nullable CFDateRef CFCalendarCopyGregorianStartDate(CFCalendarRef calendar);

CF_ASSUME_NONNULL_END

#endif // __COREFOUNDATION_CFCALENDAR_INTERNAL__

