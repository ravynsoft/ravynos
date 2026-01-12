/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*	CFCalendar.h
	Copyright (c) 2004-2014, Apple Inc. All rights reserved.
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
CFCalendarRef CFCalendarCreateWithIdentifier(CFAllocatorRef allocator, CFStringRef identifier);
	// Create a calendar.  The identifiers are the kCF*Calendar
	// constants in CFLocale.h.

CF_EXPORT
CFStringRef CFCalendarGetIdentifier(CFCalendarRef calendar);
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
	kCFCalendarUnitWeek CF_ENUM_DEPRECATED(10_4, 10_10, 2_0, 8_0) = (1UL << 8),
	kCFCalendarUnitWeekday = (1UL << 9),
	kCFCalendarUnitWeekdayOrdinal = (1UL << 10),
	kCFCalendarUnitQuarter CF_ENUM_AVAILABLE(10_6, 4_0) = (1UL << 11),
	kCFCalendarUnitWeekOfMonth CF_ENUM_AVAILABLE(10_7, 5_0) = (1UL << 12),
	kCFCalendarUnitWeekOfYear CF_ENUM_AVAILABLE(10_7, 5_0) = (1UL << 13),
	kCFCalendarUnitYearForWeekOfYear CF_ENUM_AVAILABLE(10_7, 5_0) = (1UL << 14),
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
Boolean CFCalendarGetTimeRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit unit, CFAbsoluteTime at, CFAbsoluteTime *startp, CFTimeInterval *tip) CF_AVAILABLE(10_5, 2_0);

CF_EXPORT
Boolean CFCalendarComposeAbsoluteTime(CFCalendarRef calendar, /* out */ CFAbsoluteTime *at, const char *componentDesc, ...);

CF_EXPORT
Boolean CFCalendarDecomposeAbsoluteTime(CFCalendarRef calendar, CFAbsoluteTime at, const char *componentDesc, ...);


enum {
    kCFCalendarComponentsWrap = (1UL << 0)  // option for adding
};

CF_EXPORT
Boolean CFCalendarAddComponents(CFCalendarRef calendar, /* inout */ CFAbsoluteTime *at, CFOptionFlags options, const char *componentDesc, ...);

CF_EXPORT
Boolean CFCalendarGetComponentDifference(CFCalendarRef calendar, CFAbsoluteTime startingAT, CFAbsoluteTime resultAT, CFOptionFlags options, const char *componentDesc, ...);


CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFCALENDAR__ */

