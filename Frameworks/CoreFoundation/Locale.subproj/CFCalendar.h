/*	CFCalendar.h
	Copyright (c) 2004-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFCALENDAR__)
#define __COREFOUNDATION_CFCALENDAR__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFTimeZone.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct CF_BRIDGED_MUTABLE_TYPE(NSCalendar) __CFCalendar * CFCalendarRef;

CF_EXPORT
CFTypeID CFCalendarGetTypeID(void);

CF_EXPORT
CFCalendarRef CFCalendarCopyCurrent(void);

CF_EXPORT
CFCalendarRef CFCalendarCreateWithIdentifier(CFAllocatorRef allocator, CFCalendarIdentifier identifier);
	// Create a calendar.  The identifiers are the kCF*Calendar
	// constants in CFLocale.h.

CF_EXPORT
CFCalendarIdentifier CFCalendarGetIdentifier(CFCalendarRef calendar);
	// Returns the calendar's identifier.

CF_EXPORT
CFLocaleRef CFCalendarCopyLocale(CFCalendarRef calendar);

CF_EXPORT
void CFCalendarSetLocale(CFCalendarRef calendar, CFLocaleRef locale);

CF_EXPORT
CFTimeZoneRef CFCalendarCopyTimeZone(CFCalendarRef calendar);

CF_EXPORT
void CFCalendarSetTimeZone(CFCalendarRef calendar, CFTimeZoneRef tz);

CF_EXPORT
CFIndex CFCalendarGetFirstWeekday(CFCalendarRef calendar);

CF_EXPORT
void CFCalendarSetFirstWeekday(CFCalendarRef calendar, CFIndex wkdy);

CF_EXPORT
CFIndex CFCalendarGetMinimumDaysInFirstWeek(CFCalendarRef calendar);

CF_EXPORT
void CFCalendarSetMinimumDaysInFirstWeek(CFCalendarRef calendar, CFIndex mwd);


typedef CF_OPTIONS(CFOptionFlags, CFCalendarUnit) {
	kCFCalendarUnitEra = (1UL << 1),
	kCFCalendarUnitYear = (1UL << 2),
	kCFCalendarUnitMonth = (1UL << 3),
	kCFCalendarUnitDay = (1UL << 4),
	kCFCalendarUnitHour = (1UL << 5),
	kCFCalendarUnitMinute = (1UL << 6),
	kCFCalendarUnitSecond = (1UL << 7),
	kCFCalendarUnitWeek API_DEPRECATED("Use kCFCalendarUnitWeekOfYear or kCFCalendarUnitWeekOfMonth instead", macos(10.4,10.10), ios(2.0,8.0), watchos(2.0,2.0), tvos(9.0,9.0)) = (1UL << 8),
	kCFCalendarUnitWeekday = (1UL << 9),
	kCFCalendarUnitWeekdayOrdinal = (1UL << 10),
	kCFCalendarUnitQuarter API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0)) = (1UL << 11),
	kCFCalendarUnitWeekOfMonth API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0)) = (1UL << 12),
	kCFCalendarUnitWeekOfYear API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0)) = (1UL << 13),
	kCFCalendarUnitYearForWeekOfYear API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0)) = (1UL << 14),
};

CF_EXPORT
CFRange CFCalendarGetMinimumRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit unit);

CF_EXPORT
CFRange CFCalendarGetMaximumRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit unit);

CF_EXPORT
CFRange CFCalendarGetRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime at);

CF_EXPORT
CFIndex CFCalendarGetOrdinalityOfUnit(CFCalendarRef calendar, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime at);

CF_EXPORT
Boolean CFCalendarGetTimeRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit unit, CFAbsoluteTime at, CFAbsoluteTime *startp, CFTimeInterval *tip) API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));

CF_EXPORT
Boolean CFCalendarComposeAbsoluteTime(CFCalendarRef calendar, /* out */ CFAbsoluteTime *at, const char *componentDesc, ...);

CF_EXPORT
Boolean CFCalendarDecomposeAbsoluteTime(CFCalendarRef calendar, CFAbsoluteTime at, const char *componentDesc, ...);


CF_ENUM(CFOptionFlags) {
    kCFCalendarComponentsWrap = (1UL << 0)  // option for adding
};

CF_EXPORT
Boolean CFCalendarAddComponents(CFCalendarRef calendar, /* inout */ CFAbsoluteTime *at, CFOptionFlags options, const char *componentDesc, ...);

CF_EXPORT
Boolean CFCalendarGetComponentDifference(CFCalendarRef calendar, CFAbsoluteTime startingAT, CFAbsoluteTime resultAT, CFOptionFlags options, const char *componentDesc, ...);


CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFCALENDAR__ */

