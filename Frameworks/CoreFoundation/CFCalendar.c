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

/*	CFCalendar.c
	Copyright (c) 2004-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
*/


#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFRuntime.h>
#include "CFInternal.h"
#include "CFPriv.h"
#include <unicode/ucal.h>

#define BUFFER_SIZE 512

struct __CFCalendar {
    CFRuntimeBase _base;
    CFStringRef _identifier;	// canonical identifier, never NULL
    CFLocaleRef _locale;
    CFStringRef _localeID;
    CFTimeZoneRef _tz;
    UCalendar *_cal;
};

static Boolean __CFCalendarEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFCalendarRef calendar1 = (CFCalendarRef)cf1;
    CFCalendarRef calendar2 = (CFCalendarRef)cf2;
    return CFEqual(calendar1->_identifier, calendar2->_identifier);
}

static CFHashCode __CFCalendarHash(CFTypeRef cf) {
    CFCalendarRef calendar = (CFCalendarRef)cf;
    return CFHash(calendar->_identifier);
}

static CFStringRef __CFCalendarCopyDescription(CFTypeRef cf) {
    CFCalendarRef calendar = (CFCalendarRef)cf;
    return CFStringCreateWithFormat(CFGetAllocator(calendar), NULL, CFSTR("<CFCalendar %p [%p]>{identifier = '%@'}"), cf, CFGetAllocator(calendar), calendar->_identifier);
}

static void __CFCalendarDeallocate(CFTypeRef cf) {
    CFCalendarRef calendar = (CFCalendarRef)cf;
    CFRelease(calendar->_identifier);
    if (calendar->_locale) CFRelease(calendar->_locale);
    if (calendar->_localeID) CFRelease(calendar->_localeID);
    CFRelease(calendar->_tz);
    if (calendar->_cal) ucal_close(calendar->_cal);
}

static CFTypeID __kCFCalendarTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFCalendarClass = {
    0,
    "CFCalendar",
    NULL,	// init
    NULL,	// copy
    __CFCalendarDeallocate,
    __CFCalendarEqual,
    __CFCalendarHash,
    NULL,	// 
    __CFCalendarCopyDescription
};

CFTypeID CFCalendarGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFCalendarTypeID = _CFRuntimeRegisterClass(&__CFCalendarClass); });
    return __kCFCalendarTypeID;
}

CF_PRIVATE UCalendar *__CFCalendarCreateUCalendar(CFStringRef calendarID, CFStringRef localeID, CFTimeZoneRef tz) {
    if (calendarID) {
	CFDictionaryRef components = CFLocaleCreateComponentsFromLocaleIdentifier(kCFAllocatorSystemDefault, localeID);
	CFMutableDictionaryRef mcomponents = CFDictionaryCreateMutableCopy(kCFAllocatorSystemDefault, 0, components);
	CFDictionarySetValue(mcomponents, kCFLocaleCalendarIdentifier, calendarID);
	localeID = CFLocaleCreateLocaleIdentifierFromComponents(kCFAllocatorSystemDefault, mcomponents);
	CFRelease(mcomponents);
	CFRelease(components);
    }

    char buffer[BUFFER_SIZE];
    const char *cstr = CFStringGetCStringPtr(localeID, kCFStringEncodingASCII);
    if (NULL == cstr) {
	if (CFStringGetCString(localeID, buffer, BUFFER_SIZE, kCFStringEncodingASCII)) cstr = buffer;
    }
    if (NULL == cstr) {
	if (calendarID) CFRelease(localeID);
	return NULL;
    }
    
    UChar ubuffer[BUFFER_SIZE];
    CFStringRef tznam = CFTimeZoneGetName(tz);
    CFIndex cnt = CFStringGetLength(tznam);
    if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
    CFStringGetCharacters(tznam, CFRangeMake(0, cnt), (UniChar *)ubuffer);

    UErrorCode status = U_ZERO_ERROR;
    UCalendar *cal = ucal_open(ubuffer, cnt, cstr, UCAL_DEFAULT, &status);
    if (calendarID) CFRelease(localeID);
    return cal;
}

static void __CFCalendarSetupCal(CFCalendarRef calendar) {
    calendar->_cal = __CFCalendarCreateUCalendar(calendar->_identifier, calendar->_localeID, calendar->_tz);
}

static void __CFCalendarZapCal(CFCalendarRef calendar) {
    ucal_close(calendar->_cal);
    calendar->_cal = NULL;
}

CFCalendarRef CFCalendarCopyCurrent(void) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFCalendarRef calID = (CFCalendarRef)CFLocaleGetValue(locale, kCFLocaleCalendarIdentifier);
    if (calID) {
        CFCalendarRef calendar = CFCalendarCreateWithIdentifier(kCFAllocatorSystemDefault, (CFStringRef)calID);
        CFCalendarSetLocale(calendar, locale);
	CFRelease(locale);
        return calendar;
    }
    return NULL;
}

CFCalendarRef CFCalendarCreateWithIdentifier(CFAllocatorRef allocator, CFStringRef identifier) {
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    __CFGenericValidateType(identifier, CFStringGetTypeID());
    // return NULL until Chinese calendar is available
    if (identifier != kCFGregorianCalendar && identifier != kCFBuddhistCalendar && identifier != kCFJapaneseCalendar && identifier != kCFIslamicCalendar && identifier != kCFIslamicCivilCalendar && identifier != kCFHebrewCalendar) {
//    if (identifier != kCFGregorianCalendar && identifier != kCFBuddhistCalendar && identifier != kCFJapaneseCalendar && identifier != kCFIslamicCalendar && identifier != kCFIslamicCivilCalendar && identifier != kCFHebrewCalendar && identifier != kCFChineseCalendar) {
	if (CFEqual(kCFGregorianCalendar, identifier)) identifier = kCFGregorianCalendar;
	else if (CFEqual(kCFBuddhistCalendar, identifier)) identifier = kCFBuddhistCalendar;
	else if (CFEqual(kCFJapaneseCalendar, identifier)) identifier = kCFJapaneseCalendar;
	else if (CFEqual(kCFIslamicCalendar, identifier)) identifier = kCFIslamicCalendar;
	else if (CFEqual(kCFIslamicCivilCalendar, identifier)) identifier = kCFIslamicCivilCalendar;
	else if (CFEqual(kCFHebrewCalendar, identifier)) identifier = kCFHebrewCalendar;
//	else if (CFEqual(kCFChineseCalendar, identifier)) identifier = kCFChineseCalendar;
	else return NULL;
    }
    struct __CFCalendar *calendar = NULL;
    uint32_t size = sizeof(struct __CFCalendar) - sizeof(CFRuntimeBase);
    calendar = (struct __CFCalendar *)_CFRuntimeCreateInstance(allocator, CFCalendarGetTypeID(), size, NULL);
    if (NULL == calendar) {
	return NULL;
    }
    calendar->_identifier = (CFStringRef)CFRetain(identifier);
    calendar->_locale = NULL;
    calendar->_localeID = CFLocaleGetIdentifier(CFLocaleGetSystem());
    calendar->_tz = CFTimeZoneCopyDefault();
    calendar->_cal = NULL;
    return (CFCalendarRef)calendar;
}

CFStringRef CFCalendarGetIdentifier(CFCalendarRef calendar) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFStringRef, calendar, calendarIdentifier);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return calendar->_identifier;
}

CFLocaleRef CFCalendarCopyLocale(CFCalendarRef calendar) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFLocaleRef, calendar, _copyLocale);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return (CFLocaleRef)CFLocaleCreate(kCFAllocatorSystemDefault, calendar->_localeID);
}

void CFCalendarSetLocale(CFCalendarRef calendar, CFLocaleRef locale) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), void, calendar, setLocale:locale);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    __CFGenericValidateType(locale, CFLocaleGetTypeID());
    CFStringRef localeID = CFLocaleGetIdentifier(locale);
    if (localeID != calendar->_localeID) {
	CFRelease(calendar->_localeID);
	CFRetain(localeID);
	calendar->_localeID = localeID;
        if (calendar->_cal) __CFCalendarZapCal(calendar);
    }
}

CFTimeZoneRef CFCalendarCopyTimeZone(CFCalendarRef calendar) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFTimeZoneRef, calendar_copyTimeZone);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return (CFTimeZoneRef)CFRetain(calendar->_tz);
}

void CFCalendarSetTimeZone(CFCalendarRef calendar, CFTimeZoneRef tz) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), void, calendar, setTimeZone:tz);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (tz) __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    if (tz != calendar->_tz) {
	CFRelease(calendar->_tz);
	calendar->_tz = tz ? (CFTimeZoneRef)CFRetain(tz) : CFTimeZoneCopyDefault();
        if (calendar->_cal) __CFCalendarZapCal(calendar);
    }
}

CFIndex CFCalendarGetFirstWeekday(CFCalendarRef calendar) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFIndex, calendar, firstWeekday);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
	return ucal_getAttribute(calendar->_cal, UCAL_FIRST_DAY_OF_WEEK);
    }
    return -1;
}

void CFCalendarSetFirstWeekday(CFCalendarRef calendar, CFIndex wkdy) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), void, calendar, setFirstWeekday:wkdy);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
	ucal_setAttribute(calendar->_cal, UCAL_FIRST_DAY_OF_WEEK, wkdy);
    }
}

CFIndex CFCalendarGetMinimumDaysInFirstWeek(CFCalendarRef calendar) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFIndex, calendar, minimumDaysInFirstWeek);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    return calendar->_cal ? ucal_getAttribute(calendar->_cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK) : -1;
}

void CFCalendarSetMinimumDaysInFirstWeek(CFCalendarRef calendar, CFIndex mwd) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), void, calendar, setMinimumDaysInFirstWeek:mwd);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) ucal_setAttribute(calendar->_cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, mwd);
}

CFDateRef CFCalendarCopyGregorianStartDate(CFCalendarRef calendar) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFDateRef, calendar, _gregorianStartDate);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    UErrorCode status = U_ZERO_ERROR;
    UDate udate = calendar->_cal ? ucal_getGregorianChange(calendar->_cal, &status) : 0;
    if (calendar->_cal && U_SUCCESS(status)) {
	CFAbsoluteTime at = (double)udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970;
	return CFDateCreate(CFGetAllocator(calendar), at);
    }
    return NULL;
}

void CFCalendarSetGregorianStartDate(CFCalendarRef calendar, CFDateRef date) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), void, calendar, _setGregorianStartDate:date);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (date) __CFGenericValidateType(date, CFDateGetTypeID());
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (!calendar->_cal) return;
    if (!date) {
	UErrorCode status = U_ZERO_ERROR;
	UCalendar *cal = __CFCalendarCreateUCalendar(calendar->_identifier, calendar->_localeID, calendar->_tz);
	UDate udate = cal ? ucal_getGregorianChange(cal, &status) : 0;
	if (cal && U_SUCCESS(status)) {
	    status = U_ZERO_ERROR;
	    if (calendar->_cal) ucal_setGregorianChange(calendar->_cal, udate, &status);
	}
	if (cal) ucal_close(cal);
    } else {
	CFAbsoluteTime at = CFDateGetAbsoluteTime(date);
	UDate udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
	UErrorCode status = U_ZERO_ERROR;
	if (calendar->_cal) ucal_setGregorianChange(calendar->_cal, udate, &status);
    }
}


static UCalendarDateFields __CFCalendarGetICUFieldCode(CFCalendarUnit unit) {
    switch (unit) {
    case kCFCalendarUnitEra: return UCAL_ERA;
    case kCFCalendarUnitYear: return UCAL_YEAR;
    case kCFCalendarUnitMonth: return UCAL_MONTH;
    case kCFCalendarUnitDay: return UCAL_DAY_OF_MONTH;
    case kCFCalendarUnitHour: return UCAL_HOUR_OF_DAY;
    case kCFCalendarUnitMinute: return UCAL_MINUTE;
    case kCFCalendarUnitSecond: return UCAL_SECOND;
    case kCFCalendarUnitWeek: return UCAL_WEEK_OF_YEAR;
    case kCFCalendarUnitWeekOfYear: return UCAL_WEEK_OF_YEAR;
    case kCFCalendarUnitWeekOfMonth: return UCAL_WEEK_OF_MONTH;
    case kCFCalendarUnitYearForWeekOfYear: return UCAL_YEAR_WOY;
    case kCFCalendarUnitWeekday: return UCAL_DAY_OF_WEEK;
    case kCFCalendarUnitWeekdayOrdinal: return UCAL_DAY_OF_WEEK_IN_MONTH;
    }
    return (UCalendarDateFields)-1;
}

static UCalendarDateFields __CFCalendarGetICUFieldCodeFromChar(char ch) {
    switch (ch) {
    case 'G': return UCAL_ERA;
    case 'y': return UCAL_YEAR;
    case 'M': return UCAL_MONTH;
    case 'd': return UCAL_DAY_OF_MONTH;
    case 'h': return UCAL_HOUR;
    case 'H': return UCAL_HOUR_OF_DAY;
    case 'm': return UCAL_MINUTE;
    case 's': return UCAL_SECOND;
    case 'S': return UCAL_MILLISECOND;
    case 'w': return UCAL_WEEK_OF_YEAR;
    case 'W': return UCAL_WEEK_OF_MONTH;
    case 'Y': return UCAL_YEAR_WOY;
    case 'E': return UCAL_DAY_OF_WEEK;
    case 'D': return UCAL_DAY_OF_YEAR;
    case 'F': return UCAL_DAY_OF_WEEK_IN_MONTH;
    case 'a': return UCAL_AM_PM;
    case 'g': return UCAL_JULIAN_DAY;
    }
    return (UCalendarDateFields)-1;
}

static CFCalendarUnit __CFCalendarGetCalendarUnitFromChar(char ch) {
    switch (ch) {
    case 'G': return kCFCalendarUnitEra;
    case 'y': return kCFCalendarUnitYear;
    case 'M': return kCFCalendarUnitMonth;
    case 'd': return kCFCalendarUnitDay;
    case 'H': return kCFCalendarUnitHour;
    case 'm': return kCFCalendarUnitMinute;
    case 's': return kCFCalendarUnitSecond;
    case 'w': return kCFCalendarUnitWeekOfYear;
    case 'W': return kCFCalendarUnitWeekOfMonth;
    case 'Y': return kCFCalendarUnitYearForWeekOfYear;
    case 'E': return kCFCalendarUnitWeekday;
    case 'F': return kCFCalendarUnitWeekdayOrdinal;
    }
    return (UCalendarDateFields)-1;
}

CFRange CFCalendarGetMinimumRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit unit) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFRange, calendar, _minimumRangeOfUnit:unit);
    CFRange range = {kCFNotFound, kCFNotFound};
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
	ucal_clear(calendar->_cal);
	UCalendarDateFields field = __CFCalendarGetICUFieldCode(unit);
	UErrorCode status = U_ZERO_ERROR;
	range.location = ucal_getLimit(calendar->_cal, field, UCAL_GREATEST_MINIMUM, &status);
	range.length = ucal_getLimit(calendar->_cal, field, UCAL_LEAST_MAXIMUM, &status) - range.location + 1;
	if (UCAL_MONTH == field) range.location++;
	if (100000 < range.length) range.length = 100000;
    }
    return range;
}

CFRange CFCalendarGetMaximumRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit unit) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFRange, calendar, _maximumRangeOfUnit:unit);
    CFRange range = {kCFNotFound, kCFNotFound};
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
	ucal_clear(calendar->_cal);
	UCalendarDateFields field = __CFCalendarGetICUFieldCode(unit);
	UErrorCode status = U_ZERO_ERROR;
	range.location = ucal_getLimit(calendar->_cal, field, UCAL_MINIMUM, &status);
	range.length = ucal_getLimit(calendar->_cal, field, UCAL_MAXIMUM, &status) - range.location + 1;
	if (UCAL_MONTH == field) range.location++;
	if (100000 < range.length) range.length = 100000;
    }
    return range;
}

static void __CFCalendarSetToFirstInstant(CFCalendarRef calendar, CFCalendarUnit unit, CFAbsoluteTime at) {
    // Set UCalendar to first instant of unit prior to 'at'
    UErrorCode status = U_ZERO_ERROR;
    UDate udate = floor((at + kCFAbsoluteTimeIntervalSince1970) * 1000.0);
    ucal_setMillis(calendar->_cal, udate, &status);
    int target_era = INT_MIN;
    switch (unit) { // largest to smallest, we set the fields to their minimum value
    case kCFCalendarUnitYearForWeekOfYear:;
        ucal_set(calendar->_cal, UCAL_WEEK_OF_YEAR, ucal_getLimit(calendar->_cal, UCAL_WEEK_OF_YEAR, UCAL_ACTUAL_MINIMUM, &status));
    case kCFCalendarUnitWeek:
    case kCFCalendarUnitWeekOfMonth:;
    case kCFCalendarUnitWeekOfYear:;
	{
	// reduce to first day of week, then reduce the rest of the day
        int32_t goal = ucal_getAttribute(calendar->_cal, UCAL_FIRST_DAY_OF_WEEK);
	int32_t dow = ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status);
	while (dow != goal) {
	    ucal_add(calendar->_cal, UCAL_DAY_OF_MONTH, -1, &status);
	    dow = ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status);
	}
	goto day;
	}
    case kCFCalendarUnitEra:
	{
	target_era = ucal_get(calendar->_cal, UCAL_ERA, &status);
	ucal_set(calendar->_cal, UCAL_YEAR, ucal_getLimit(calendar->_cal, UCAL_YEAR, UCAL_ACTUAL_MINIMUM, &status));
	}
    case kCFCalendarUnitYear:
	ucal_set(calendar->_cal, UCAL_MONTH, ucal_getLimit(calendar->_cal, UCAL_MONTH, UCAL_ACTUAL_MINIMUM, &status));
    case kCFCalendarUnitMonth:
	ucal_set(calendar->_cal, UCAL_DAY_OF_MONTH, ucal_getLimit(calendar->_cal, UCAL_DAY_OF_MONTH, UCAL_ACTUAL_MINIMUM, &status));
    case kCFCalendarUnitWeekday:
    case kCFCalendarUnitDay:
    day:;
	ucal_set(calendar->_cal, UCAL_HOUR_OF_DAY, ucal_getLimit(calendar->_cal, UCAL_HOUR_OF_DAY, UCAL_ACTUAL_MINIMUM, &status));
    case kCFCalendarUnitHour:
	ucal_set(calendar->_cal, UCAL_MINUTE, ucal_getLimit(calendar->_cal, UCAL_MINUTE, UCAL_ACTUAL_MINIMUM, &status));
    case kCFCalendarUnitMinute:
	ucal_set(calendar->_cal, UCAL_SECOND, ucal_getLimit(calendar->_cal, UCAL_SECOND, UCAL_ACTUAL_MINIMUM, &status));
    case kCFCalendarUnitSecond:
	ucal_set(calendar->_cal, UCAL_MILLISECOND, 0);
    }
    if (INT_MIN != target_era && ucal_get(calendar->_cal, UCAL_ERA, &status) < target_era) {
	// In the Japanese calendar, and possibly others, eras don't necessarily
	// start on the first day of a year, so the previous code may have backed
	// up into the previous era, and we have to correct forward.
	UDate bad_udate = ucal_getMillis(calendar->_cal, &status);
	ucal_add(calendar->_cal, UCAL_MONTH, 1, &status);
	while (ucal_get(calendar->_cal, UCAL_ERA, &status) < target_era) {
	    bad_udate = ucal_getMillis(calendar->_cal, &status);
	    ucal_add(calendar->_cal, UCAL_MONTH, 1, &status);
	}
	udate = ucal_getMillis(calendar->_cal, &status);
	// target date is between bad_udate and udate
	for (;;) {
	    UDate test_udate = (udate + bad_udate) / 2;
	    ucal_setMillis(calendar->_cal, test_udate, &status);
	    if (ucal_get(calendar->_cal, UCAL_ERA, &status) < target_era) {
		bad_udate = test_udate;
	    } else {
		udate = test_udate;
	    }
	    if (fabs(udate - bad_udate) < 1000) break;
	}
	do {
	    bad_udate = floor((bad_udate + 1000) / 1000) * 1000;
	    ucal_setMillis(calendar->_cal, bad_udate, &status);
	} while (ucal_get(calendar->_cal, UCAL_ERA, &status) < target_era);
    }
}

static Boolean __validUnits(CFCalendarUnit smaller, CFCalendarUnit bigger) {
    switch (bigger) {
    case kCFCalendarUnitEra:
	if (kCFCalendarUnitEra == smaller) return false;
	if (kCFCalendarUnitWeekday == smaller) return false;
	if (kCFCalendarUnitMinute == smaller) return false;	// this causes CFIndex overflow in range.length
	if (kCFCalendarUnitSecond == smaller) return false;	// this causes CFIndex overflow in range.length
	return true;
    case kCFCalendarUnitYearForWeekOfYear:
    case kCFCalendarUnitYear:
	if (kCFCalendarUnitEra == smaller) return false;
	if (kCFCalendarUnitYear == smaller) return false;
        if (kCFCalendarUnitYearForWeekOfYear == smaller) return false;
	if (kCFCalendarUnitWeekday == smaller) return false;
	return true;
    case kCFCalendarUnitMonth:
	if (kCFCalendarUnitEra == smaller) return false;
	if (kCFCalendarUnitYear == smaller) return false;
	if (kCFCalendarUnitMonth == smaller) return false;
	if (kCFCalendarUnitWeekday == smaller) return false;
	return true;
    case kCFCalendarUnitDay:
	if (kCFCalendarUnitHour == smaller) return true;
	if (kCFCalendarUnitMinute == smaller) return true;
	if (kCFCalendarUnitSecond == smaller) return true;
	return false;
    case kCFCalendarUnitHour:
	if (kCFCalendarUnitMinute == smaller) return true;
	if (kCFCalendarUnitSecond == smaller) return true;
	return false;
    case kCFCalendarUnitMinute:
	if (kCFCalendarUnitSecond == smaller) return true;
	return false;
    case kCFCalendarUnitWeek:
    case kCFCalendarUnitWeekOfMonth:
    case kCFCalendarUnitWeekOfYear:
	if (kCFCalendarUnitWeekday == smaller) return true;
	if (kCFCalendarUnitDay == smaller) return true;
	if (kCFCalendarUnitHour == smaller) return true;
	if (kCFCalendarUnitMinute == smaller) return true;
	if (kCFCalendarUnitSecond == smaller) return true;
	return false;
    case kCFCalendarUnitSecond:
    case kCFCalendarUnitWeekday:
    case kCFCalendarUnitWeekdayOrdinal:
	return false;
    }
    return false;
};

static CFRange __CFCalendarGetRangeOfUnit2(CFCalendarRef calendar, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime at) __attribute__((noinline));
static CFRange __CFCalendarGetRangeOfUnit2(CFCalendarRef calendar, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime at) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFRange, calendar, _rangeOfUnit:smallerUnit inUnit:biggerUnit forAT:at);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    CFRange range = {kCFNotFound, kCFNotFound};
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
	switch (smallerUnit) {
	case kCFCalendarUnitSecond:
            switch (biggerUnit) {
            case kCFCalendarUnitMinute:
            case kCFCalendarUnitHour:
            case kCFCalendarUnitDay:
            case kCFCalendarUnitWeekday:
            case kCFCalendarUnitWeek:
            case kCFCalendarUnitMonth:
            case kCFCalendarUnitYear:
            case kCFCalendarUnitEra:
		// goto calculate;
                range.location = 0;
                range.length = 60;
		break;
            }
	    break;
	case kCFCalendarUnitMinute:
            switch (biggerUnit) {
            case kCFCalendarUnitHour:
            case kCFCalendarUnitDay:
            case kCFCalendarUnitWeekday:
            case kCFCalendarUnitWeek:
            case kCFCalendarUnitMonth:
            case kCFCalendarUnitYear:
            case kCFCalendarUnitEra:
		// goto calculate;
                range.location = 0;
                range.length = 60;
		break;
            }
	    break;
	case kCFCalendarUnitHour:
            switch (biggerUnit) {
            case kCFCalendarUnitDay:
            case kCFCalendarUnitWeekday:
            case kCFCalendarUnitWeek:
            case kCFCalendarUnitMonth:
            case kCFCalendarUnitYear:
            case kCFCalendarUnitEra:
		// goto calculate;
                range.location = 0;
                range.length = 24;
		break;
            }
	    break;
	case kCFCalendarUnitDay:
            switch (biggerUnit) {
            case kCFCalendarUnitWeek:
            case kCFCalendarUnitMonth:
            case kCFCalendarUnitYear:
            case kCFCalendarUnitEra:
		goto calculate;
		break;
            }
	    break;
	case kCFCalendarUnitWeekday:
            switch (biggerUnit) {
            case kCFCalendarUnitWeek:
            case kCFCalendarUnitMonth:
            case kCFCalendarUnitYear:
            case kCFCalendarUnitEra:
		goto calculate;
		break;
            }
	    break;
	case kCFCalendarUnitWeekdayOrdinal:
            switch (biggerUnit) {
            case kCFCalendarUnitMonth:
            case kCFCalendarUnitYear:
            case kCFCalendarUnitEra:
		goto calculate;
		break;
            }
	    break;
	case kCFCalendarUnitWeek:
            switch (biggerUnit) {
            case kCFCalendarUnitMonth:
            case kCFCalendarUnitYear:
            case kCFCalendarUnitEra:
		goto calculate;
		break;
            }
	    break;
	case kCFCalendarUnitMonth:
            switch (biggerUnit) {
            case kCFCalendarUnitYear:
            case kCFCalendarUnitEra:
		goto calculate;
		break;
            }
	    break;
	case kCFCalendarUnitYear:
            switch (biggerUnit) {
            case kCFCalendarUnitEra:
		goto calculate;
		break;
            }
	    break;
	case kCFCalendarUnitEra:
	    break;
	}
    }
    return range;

    calculate:;
    ucal_clear(calendar->_cal);
    UCalendarDateFields smallField = __CFCalendarGetICUFieldCode(smallerUnit);
    UCalendarDateFields bigField = __CFCalendarGetICUFieldCode(biggerUnit);
    UCalendarDateFields yearField = __CFCalendarGetICUFieldCode(kCFCalendarUnitYear);
    UCalendarDateFields fieldToAdd = smallField;
    if (kCFCalendarUnitWeekday == smallerUnit) {
        fieldToAdd = __CFCalendarGetICUFieldCode(kCFCalendarUnitDay);
    }
    int32_t dow = -1;
    if (kCFCalendarUnitWeekdayOrdinal == smallerUnit) {
        UErrorCode status = U_ZERO_ERROR;
        UDate udate = floor((at + kCFAbsoluteTimeIntervalSince1970) * 1000.0);
        ucal_setMillis(calendar->_cal, udate, &status);
        dow = ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status);
        fieldToAdd = __CFCalendarGetICUFieldCode(kCFCalendarUnitWeek);
    }
    // Set calendar to first instant of big unit
    __CFCalendarSetToFirstInstant(calendar, biggerUnit, at);
    if (kCFCalendarUnitWeekdayOrdinal == smallerUnit) {
        UErrorCode status = U_ZERO_ERROR;
        // roll day forward to first 'dow'
        while (ucal_get(calendar->_cal, (kCFCalendarUnitMonth == biggerUnit) ? UCAL_WEEK_OF_MONTH : UCAL_WEEK_OF_YEAR, &status) != 1) {
	    ucal_add(calendar->_cal, UCAL_DAY_OF_MONTH, 1, &status);
        }
        while (ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status) != dow) {
	    ucal_add(calendar->_cal, UCAL_DAY_OF_MONTH, 1, &status);
        }
    }
    int32_t minSmallValue = INT32_MAX;
    int32_t maxSmallValue = INT32_MIN;
    UErrorCode status = U_ZERO_ERROR;
    int32_t bigValue = ucal_get(calendar->_cal, bigField, &status);
    for (;;) {
        int32_t smallValue = ucal_get(calendar->_cal, smallField, &status);
        if (smallValue < minSmallValue) minSmallValue = smallValue;
        if (smallValue > maxSmallValue) maxSmallValue = smallValue;
        ucal_add(calendar->_cal, fieldToAdd, 1, &status);
        if (bigValue != ucal_get(calendar->_cal, bigField, &status)) break;
        if (biggerUnit == kCFCalendarUnitEra && ucal_get(calendar->_cal, yearField, &status) > 10000) break;
	// we assume an answer for 10000 years can be extrapolated to 100000 years, to save time
    }
    status = U_ZERO_ERROR;
    range.location = minSmallValue;
    if (smallerUnit == kCFCalendarUnitMonth) range.location = 1;
    range.length = maxSmallValue - minSmallValue + 1;
    if (biggerUnit == kCFCalendarUnitEra && ucal_get(calendar->_cal, yearField, &status) > 10000) range.length = 100000;

    return range;
}

CFRange CFCalendarGetRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime at) {
	return __CFCalendarGetRangeOfUnit2(calendar, smallerUnit, biggerUnit, at);
}

CFIndex CFCalendarGetOrdinalityOfUnit(CFCalendarRef calendar, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime at) {
    CFIndex result = kCFNotFound;
    if (!__validUnits(smallerUnit, biggerUnit)) return result;
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFIndex, calendar, _ordinalityOfUnit:smallerUnit inUnit:biggerUnit forAT:at);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
	UErrorCode status = U_ZERO_ERROR;
	ucal_clear(calendar->_cal);
	if (kCFCalendarUnitWeek == smallerUnit && kCFCalendarUnitYear == biggerUnit) {
	    UDate udate = floor((at + kCFAbsoluteTimeIntervalSince1970) * 1000.0);
	    ucal_setMillis(calendar->_cal, udate, &status);
	    int32_t val = ucal_get(calendar->_cal, UCAL_WEEK_OF_YEAR, &status);
	    return val;
	} else if (kCFCalendarUnitWeek == smallerUnit && kCFCalendarUnitMonth == biggerUnit) {
	    UDate udate = floor((at + kCFAbsoluteTimeIntervalSince1970) * 1000.0);
	    ucal_setMillis(calendar->_cal, udate, &status);
	    int32_t val = ucal_get(calendar->_cal, UCAL_WEEK_OF_MONTH, &status);
	    return val;
	}
	UCalendarDateFields smallField = __CFCalendarGetICUFieldCode(smallerUnit);
	// Set calendar to first instant of big unit
	__CFCalendarSetToFirstInstant(calendar, biggerUnit, at);
	UDate curr = ucal_getMillis(calendar->_cal, &status);
        UDate goal = floor((at + kCFAbsoluteTimeIntervalSince1970) * 1000.0);
	result = 1;
	const int multiple_table[] = {0, 0, 16, 19, 24, 26, 24, 28, 14, 14, 14};
	int multiple = (1 << multiple_table[flsl(smallerUnit) - 1]);
	Boolean divide = false, alwaysDivide = false;
	while (curr < goal) {
	    ucal_add(calendar->_cal, smallField, multiple, &status);
	    UDate newcurr = ucal_getMillis(calendar->_cal, &status);
	    if (curr < newcurr && newcurr <= goal) {
		result += multiple;
		curr = newcurr;
	    } else {
		// Either newcurr is going backwards, or not making
		// progress, or has overshot the goal; reset date
		// and try smaller multiples.
		ucal_setMillis(calendar->_cal, curr, &status);
		divide = true;
		// once we start overshooting the goal, the add at
		// smaller multiples will succeed at most once for
		// each multiple, so we reduce it every time through
		// the loop.
		if (goal < newcurr) alwaysDivide = true;
	    }
	    if (divide) {
		multiple = multiple / 2;
		if (0 == multiple) break;
		divide = alwaysDivide;
	    }
	}
    }
    return result;
}

Boolean _CFCalendarComposeAbsoluteTimeV(CFCalendarRef calendar, /* out */ CFAbsoluteTime *atp, const char *componentDesc, int *vector, int count) {
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
	UErrorCode status = U_ZERO_ERROR;
	ucal_clear(calendar->_cal);
	ucal_set(calendar->_cal, UCAL_YEAR, 1);
	ucal_set(calendar->_cal, UCAL_MONTH, 0);
	ucal_set(calendar->_cal, UCAL_DAY_OF_MONTH, 1);
	ucal_set(calendar->_cal, UCAL_HOUR_OF_DAY, 0);
	ucal_set(calendar->_cal, UCAL_MINUTE, 0);
	ucal_set(calendar->_cal, UCAL_SECOND, 0);
	const char *desc = componentDesc;
	Boolean doWOY = false;
	char ch = *desc;
	while (ch) {
	    UCalendarDateFields field = __CFCalendarGetICUFieldCodeFromChar(ch);
	    if (UCAL_WEEK_OF_YEAR == field) {
		doWOY = true;
	    }
	    desc++;
	    ch = *desc;
	}
	desc = componentDesc;
	ch = *desc;
	while (ch) {
	    UCalendarDateFields field = __CFCalendarGetICUFieldCodeFromChar(ch);
	    int value = *vector;
	    if (UCAL_YEAR == field && doWOY) field = UCAL_YEAR_WOY;
	    if (UCAL_MONTH == field) value--;
	    ucal_set(calendar->_cal, field, value);
	    vector++;
	    desc++;
	    ch = *desc;
	}
	UDate udate = ucal_getMillis(calendar->_cal, &status);
	CFAbsoluteTime at = (udate / 1000.0) - kCFAbsoluteTimeIntervalSince1970;
        if (atp) *atp = at;
	return U_SUCCESS(status) ? true : false;
    }
    return false;
}

Boolean _CFCalendarDecomposeAbsoluteTimeV(CFCalendarRef calendar, CFAbsoluteTime at, const char *componentDesc, int **vector, int count) {
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
	UErrorCode status = U_ZERO_ERROR;
	ucal_clear(calendar->_cal);
	UDate udate = floor((at + kCFAbsoluteTimeIntervalSince1970) * 1000.0);
	ucal_setMillis(calendar->_cal, udate, &status);
	char ch = *componentDesc;
	while (ch) {
	    UCalendarDateFields field = __CFCalendarGetICUFieldCodeFromChar(ch);
	    int value = ucal_get(calendar->_cal, field, &status);
	    if (UCAL_MONTH == field) value++;
	    *(*vector) = value;
	    vector++;
	    componentDesc++;
	    ch = *componentDesc;
	}
	return U_SUCCESS(status) ? true : false;
    }
    return false;
}

Boolean _CFCalendarAddComponentsV(CFCalendarRef calendar, /* inout */ CFAbsoluteTime *atp, CFOptionFlags options, const char *componentDesc, int *vector, int count) {
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
	UErrorCode status = U_ZERO_ERROR;
	ucal_clear(calendar->_cal);
	UDate udate = floor((*atp + kCFAbsoluteTimeIntervalSince1970) * 1000.0);
	ucal_setMillis(calendar->_cal, udate, &status);
	char ch = *componentDesc;
	while (ch) {
	    UCalendarDateFields field = __CFCalendarGetICUFieldCodeFromChar(ch);
            int amount = *vector;
	    if (options & kCFCalendarComponentsWrap) {
		ucal_roll(calendar->_cal, field, amount, &status);
	    } else {
		ucal_add(calendar->_cal, field, amount, &status);
	    }
	    vector++;
	    componentDesc++;
	    ch = *componentDesc;
	}
	udate = ucal_getMillis(calendar->_cal, &status);
	*atp = (udate / 1000.0) - kCFAbsoluteTimeIntervalSince1970;
	return U_SUCCESS(status) ? true : false;
    }
    return false;
}

Boolean _CFCalendarGetComponentDifferenceV(CFCalendarRef calendar, CFAbsoluteTime startingAT, CFAbsoluteTime resultAT, CFOptionFlags options, const char *componentDesc, int **vector, int count) {
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
	UErrorCode status = U_ZERO_ERROR;
	ucal_clear(calendar->_cal);
	UDate curr = floor((startingAT + kCFAbsoluteTimeIntervalSince1970) * 1000.0);
	UDate goal = floor((resultAT + kCFAbsoluteTimeIntervalSince1970) * 1000.0);
	ucal_setMillis(calendar->_cal, curr, &status);
	int direction = (startingAT <= resultAT) ? 1 : -1;
	char ch = *componentDesc;
	while (ch) {
	    UCalendarDateFields field = __CFCalendarGetICUFieldCodeFromChar(ch);
	    const int multiple_table[] = {0, 0, 16, 19, 24, 26, 24, 28, 14, 14, 14};
	    int multiple = direction * (1 << multiple_table[flsl(__CFCalendarGetCalendarUnitFromChar(ch)) - 1]);
	    Boolean divide = false, alwaysDivide = false;
	    int result = 0;
	    while ((direction > 0 && curr < goal) || (direction < 0 && goal < curr)) {
		ucal_add(calendar->_cal, field, multiple, &status);
		UDate newcurr = ucal_getMillis(calendar->_cal, &status);
		if ((direction > 0 && curr < newcurr && newcurr <= goal) || (direction < 0 && newcurr < curr && goal <= newcurr)) {
		    result += multiple;
		    curr = newcurr;
		} else {
		    // Either newcurr is going backwards, or not making
		    // progress, or has overshot the goal; reset date
		    // and try smaller multiples.
		    ucal_setMillis(calendar->_cal, curr, &status);
		    divide = true;
		    // once we start overshooting the goal, the add at
		    // smaller multiples will succeed at most once for
		    // each multiple, so we reduce it every time through
		    // the loop.
		    if ((direction > 0 && goal < newcurr) || (direction < 0 && newcurr < goal)) alwaysDivide = true;
		}
		if (divide) {
		    multiple = multiple / 2;
		    if (0 == multiple) break;
		    divide = alwaysDivide;
		}
	    }
	    *(*vector) = result;
	    vector++;
	    componentDesc++;
	    ch = *componentDesc;
	}
	return U_SUCCESS(status) ? true : false;
    }
    return false;
}

Boolean CFCalendarComposeAbsoluteTime(CFCalendarRef calendar, /* out */ CFAbsoluteTime *atp, const char *componentDesc, ...) {
    va_list args;
    va_start(args, componentDesc);
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), Boolean, calendar, _composeAbsoluteTime:atp :componentDesc :args);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    int idx, cnt = strlen((char *)componentDesc);
    STACK_BUFFER_DECL(int, vector, cnt);
    for (idx = 0; idx < cnt; idx++) {
	int arg = va_arg(args, int);
	vector[idx] = arg;
    }
    va_end(args);
    return _CFCalendarComposeAbsoluteTimeV(calendar, atp, componentDesc, vector, cnt);
}

Boolean CFCalendarDecomposeAbsoluteTime(CFCalendarRef calendar, CFAbsoluteTime at, const char *componentDesc, ...) {
    va_list args;
    va_start(args, componentDesc);
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), Boolean, calendar, _decomposeAbsoluteTime:at :componentDesc :args);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    int idx, cnt = strlen((char *)componentDesc);
    STACK_BUFFER_DECL(int *, vector, cnt);
    for (idx = 0; idx < cnt; idx++) {
	int *arg = va_arg(args, int *);
	vector[idx] = arg;
    }
    va_end(args);
    return _CFCalendarDecomposeAbsoluteTimeV(calendar, at, componentDesc, vector, cnt);
}

Boolean CFCalendarAddComponents(CFCalendarRef calendar, /* inout */ CFAbsoluteTime *atp, CFOptionFlags options, const char *componentDesc, ...) {
    va_list args;
    va_start(args, componentDesc);
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), Boolean, calendar, _addComponents:atp :options :componentDesc :args);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    int idx, cnt = strlen((char *)componentDesc);
    STACK_BUFFER_DECL(int, vector, cnt);
    for (idx = 0; idx < cnt; idx++) {
	int arg = va_arg(args, int);
	vector[idx] = arg;
    }
    va_end(args);
    return _CFCalendarAddComponentsV(calendar, atp, options, componentDesc, vector, cnt);    
}

Boolean CFCalendarGetComponentDifference(CFCalendarRef calendar, CFAbsoluteTime startingAT, CFAbsoluteTime resultAT, CFOptionFlags options, const char *componentDesc, ...) {
    va_list args;
    va_start(args, componentDesc);
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), Boolean, calendar, _diffComponents:startingAT :resultAT :options :componentDesc :args);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    int idx, cnt = strlen((char *)componentDesc);
    STACK_BUFFER_DECL(int *, vector, cnt);
    for (idx = 0; idx < cnt; idx++) {
	int *arg = va_arg(args, int *);
	vector[idx] = arg;
    }
    va_end(args);
    Boolean ret = _CFCalendarGetComponentDifferenceV(calendar, startingAT, resultAT, options, componentDesc, vector, cnt);
    return ret;
}

Boolean CFCalendarGetTimeRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit unit, CFAbsoluteTime at, CFAbsoluteTime *startp, CFTimeInterval *tip) {
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), Boolean, calendar, _rangeOfUnit:unit startTime:startp interval:tip forAT:at);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (kCFCalendarUnitWeekdayOrdinal == unit) return false;
    if (kCFCalendarUnitWeekday == unit) unit = kCFCalendarUnitDay;
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
        ucal_clear(calendar->_cal);
        __CFCalendarSetToFirstInstant(calendar, unit, at);
        UErrorCode status = U_ZERO_ERROR;
        UDate start = ucal_getMillis(calendar->_cal, &status);
	UCalendarDateFields field = __CFCalendarGetICUFieldCode(unit);
	ucal_add(calendar->_cal, field, 1, &status);
        UDate end = ucal_getMillis(calendar->_cal, &status);
	if (end == start && kCFCalendarUnitEra == unit) {
            // ICU refuses to do the addition, probably because we are
            // at the limit of UCAL_ERA.  Use alternate strategy.
            CFIndex limit = ucal_getLimit(calendar->_cal, UCAL_YEAR, UCAL_MAXIMUM, &status);
            if (100000 < limit) limit = 100000;
            ucal_add(calendar->_cal, UCAL_YEAR, limit, &status);
	    end = ucal_getMillis(calendar->_cal, &status);
	}
	if (U_SUCCESS(status)) {
	    if (startp) *startp = (double)start / 1000.0 - kCFAbsoluteTimeIntervalSince1970;
	    if (tip) *tip = (double)(end - start) / 1000.0;
	    return true;
	}
    }

    return false;
}

#undef BUFFER_SIZE

