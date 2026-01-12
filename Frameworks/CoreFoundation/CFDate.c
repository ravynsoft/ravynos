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

/*	CFDate.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
*/

#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFTimeZone.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>
#include "CFInternal.h"
#include <math.h>
#include <dispatch/dispatch.h>

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#include <sys/time.h>
#endif

#define DEFINE_CFDATE_FUNCTIONS 1

/* cjk: The Julian Date for the reference date is 2451910.5,
        I think, in case that's ever useful. */

#if DEFINE_CFDATE_FUNCTIONS

const CFTimeInterval kCFAbsoluteTimeIntervalSince1970 = 978307200.0L;
const CFTimeInterval kCFAbsoluteTimeIntervalSince1904 = 3061152000.0L;

CF_PRIVATE double __CFTSRRate = 0.0;
static double __CF1_TSRRate = 0.0;

CF_PRIVATE uint64_t __CFTimeIntervalToTSR(CFTimeInterval ti) {
    if ((ti * __CFTSRRate) > INT64_MAX / 2) return (INT64_MAX / 2);
    return (uint64_t)(ti * __CFTSRRate);
}

CF_PRIVATE CFTimeInterval __CFTSRToTimeInterval(uint64_t tsr) {
    return (CFTimeInterval)((double)tsr * __CF1_TSRRate);
}

CF_PRIVATE CFTimeInterval __CFTimeIntervalUntilTSR(uint64_t tsr) {
    CFDateGetTypeID();
    uint64_t now = mach_absolute_time();
    if (tsr >= now) {
        return __CFTSRToTimeInterval(tsr - now);
    } else {
        return -__CFTSRToTimeInterval(now - tsr);
    }
}

// Technically this is 'TSR units' not a strict 'TSR' absolute time
CF_PRIVATE uint64_t __CFTSRToNanoseconds(uint64_t tsr) {
    double tsrInNanoseconds = floor(tsr * __CF1_TSRRate * NSEC_PER_SEC);
    uint64_t ns = (uint64_t)tsrInNanoseconds;
    return ns;
}

CF_PRIVATE dispatch_time_t __CFTSRToDispatchTime(uint64_t tsr) {
    uint64_t tsrInNanoseconds = __CFTSRToNanoseconds(tsr);
    
    // It's important to clamp this value to INT64_MAX or it will become interpreted by dispatch_time as a relative value instead of absolute time
    if (tsrInNanoseconds > INT64_MAX - 1) tsrInNanoseconds = INT64_MAX - 1;
    
    // 2nd argument of dispatch_time is a value in nanoseconds, but tsr does not equal nanoseconds on all platforms.
    return dispatch_time(1, (int64_t)tsrInNanoseconds);
}

CFAbsoluteTime CFAbsoluteTimeGetCurrent(void) {
    CFAbsoluteTime ret;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ret = (CFTimeInterval)tv.tv_sec - kCFAbsoluteTimeIntervalSince1970;
    ret += (1.0E-6 * (CFTimeInterval)tv.tv_usec);
    return ret;
}

struct __CFDate {
    CFRuntimeBase _base;
    CFAbsoluteTime _time;       /* immutable */
};

static Boolean __CFDateEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFDateRef date1 = (CFDateRef)cf1;
    CFDateRef date2 = (CFDateRef)cf2;
    if (date1->_time != date2->_time) return false;
    return true;
}

static CFHashCode __CFDateHash(CFTypeRef cf) {
    CFDateRef date = (CFDateRef)cf;
    return (CFHashCode)(float)floor(date->_time);
}

static CFStringRef __CFDateCopyDescription(CFTypeRef cf) {
    CFDateRef date = (CFDateRef)cf;
    return CFStringCreateWithFormat(CFGetAllocator(date), NULL, CFSTR("<CFDate %p [%p]>{time = %0.09g}"), cf, CFGetAllocator(date), date->_time);
}

static CFTypeID __kCFDateTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFDateClass = {
    0,
    "CFDate",
    NULL,       // init
    NULL,       // copy
    NULL,       // dealloc
    __CFDateEqual,
    __CFDateHash,
    NULL,       //
    __CFDateCopyDescription
};

CFTypeID CFDateGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{
        __kCFDateTypeID = _CFRuntimeRegisterClass(&__CFDateClass); 

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    struct mach_timebase_info info;
    mach_timebase_info(&info);
    __CFTSRRate = (1.0E9 / (double)info.numer) * (double)info.denom;
    __CF1_TSRRate = 1.0 / __CFTSRRate;
#elif DEPLOYMENT_TARGET_WINDOWS
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
        HALT;
    }
    __CFTSRRate = (double)freq.QuadPart;
    __CF1_TSRRate = 1.0 / __CFTSRRate;
#elif DEPLOYMENT_TARGET_LINUX
    struct timespec res;
    if (clock_getres(CLOCK_MONOTONIC, &res) != 0) {
        HALT;
    }
    __CFTSRRate = res.tv_sec + (1000000000 * res.tv_nsec);
    __CF1_TSRRate = 1.0 / __CFTSRRate;
#else
#error Unable to initialize date
#endif
    });
    return __kCFDateTypeID;
}

CFDateRef CFDateCreate(CFAllocatorRef allocator, CFAbsoluteTime at) {
    CFDateRef memory; 
    uint32_t size;
    size = sizeof(struct __CFDate) - sizeof(CFRuntimeBase);
    memory = (CFDateRef)_CFRuntimeCreateInstance(allocator, CFDateGetTypeID(), size, NULL);
    if (NULL == memory) {
        return NULL;
    }
    ((struct __CFDate *)memory)->_time = at;
    return memory;
}

CFTimeInterval CFDateGetAbsoluteTime(CFDateRef date) {
    CF_OBJC_FUNCDISPATCHV(CFDateGetTypeID(), CFTimeInterval, (NSDate *)date, timeIntervalSinceReferenceDate);
    __CFGenericValidateType(date, CFDateGetTypeID());
    return date->_time;
}

CFTimeInterval CFDateGetTimeIntervalSinceDate(CFDateRef date, CFDateRef otherDate) {
    CF_OBJC_FUNCDISPATCHV(CFDateGetTypeID(), CFTimeInterval, (NSDate *)date, timeIntervalSinceDate:(NSDate *)otherDate);
    __CFGenericValidateType(date, CFDateGetTypeID());
    __CFGenericValidateType(otherDate, CFDateGetTypeID());
    return date->_time - otherDate->_time;
}   
    
CFComparisonResult CFDateCompare(CFDateRef date, CFDateRef otherDate, void *context) {
    CF_OBJC_FUNCDISPATCHV(CFDateGetTypeID(), CFComparisonResult, (NSDate *)date, compare:(NSDate *)otherDate);
    __CFGenericValidateType(date, CFDateGetTypeID());
    __CFGenericValidateType(otherDate, CFDateGetTypeID());
    if (date->_time < otherDate->_time) return kCFCompareLessThan;
    if (date->_time > otherDate->_time) return kCFCompareGreaterThan;
    return kCFCompareEqualTo;
}

#endif

CF_INLINE int32_t __CFDoubleModToInt(double d, int32_t modulus) {
    int32_t result = (int32_t)(float)floor(d - floor(d / modulus) * modulus);
    if (result < 0) result += modulus;
    return result;
}

CF_INLINE double __CFDoubleMod(double d, int32_t modulus) {
    double result = d - floor(d / modulus) * modulus;
    if (result < 0.0) result += (double)modulus;
    return result;
}

static const uint8_t daysInMonth[16] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0, 0, 0};
static const uint16_t daysBeforeMonth[16] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365, 0, 0};
static const uint16_t daysAfterMonth[16] = {365, 334, 306, 275, 245, 214, 184, 153, 122, 92, 61, 31, 0, 0, 0, 0};

CF_INLINE bool isleap(int64_t year) {
    int64_t y = (year + 1) % 400;	/* correct to nearest multiple-of-400 year, then find the remainder */
    if (y < 0) y = -y;
    return (0 == (y & 3) && 100 != y && 200 != y && 300 != y);
}

/* year arg is absolute year; Gregorian 2001 == year 0; 2001/1/1 = absolute date 0 */
CF_INLINE uint8_t __CFDaysInMonth(int8_t month, int64_t year, bool leap) {
    return daysInMonth[month] + (2 == month && leap);
}

/* year arg is absolute year; Gregorian 2001 == year 0; 2001/1/1 = absolute date 0 */
CF_INLINE uint16_t __CFDaysBeforeMonth(int8_t month, int64_t year, bool leap) {
    return daysBeforeMonth[month] + (2 < month && leap);
}

/* year arg is absolute year; Gregorian 2001 == year 0; 2001/1/1 = absolute date 0 */
CF_INLINE uint16_t __CFDaysAfterMonth(int8_t month, int64_t year, bool leap) {
    return daysAfterMonth[month] + (month < 2 && leap);
}

/* year arg is absolute year; Gregorian 2001 == year 0; 2001/1/1 = absolute date 0 */
static void __CFYMDFromAbsolute(int64_t absolute, int64_t *year, int8_t *month, int8_t *day) {
    int64_t b = absolute / 146097; // take care of as many multiples of 400 years as possible
    int64_t y = b * 400;
    uint16_t ydays;
    absolute -= b * 146097;
    while (absolute < 0) {
	y -= 1;
	absolute += __CFDaysAfterMonth(0, y, isleap(y));
    }
    /* Now absolute is non-negative days to add to year */
    ydays = __CFDaysAfterMonth(0, y, isleap(y));
    while (ydays <= absolute) {
	y += 1;
	absolute -= ydays;
	ydays = __CFDaysAfterMonth(0, y, isleap(y));
    }
    /* Now we have year and days-into-year */
    if (year) *year = y;
    if (month || day) {
	int8_t m = absolute / 33 + 1; /* search from the approximation */
	bool leap = isleap(y);
	while (__CFDaysBeforeMonth(m + 1, y, leap) <= absolute) m++;
	if (month) *month = m;
	if (day) *day = absolute - __CFDaysBeforeMonth(m, y, leap) + 1;
    }
}

/* year arg is absolute year; Gregorian 2001 == year 0; 2001/1/1 = absolute date 0 */
static double __CFAbsoluteFromYMD(int64_t year, int8_t month, int8_t day) {
    double absolute = 0.0;
    int64_t idx;
    int64_t b = year / 400; // take care of as many multiples of 400 years as possible
    absolute += b * 146097.0;
    year -= b * 400;
    if (year < 0) {
	for (idx = year; idx < 0; idx++)
	    absolute -= __CFDaysAfterMonth(0, idx, isleap(idx));
    } else {
	for (idx = 0; idx < year; idx++)
	    absolute += __CFDaysAfterMonth(0, idx, isleap(idx));
    }
    /* Now add the days into the original year */
    absolute += __CFDaysBeforeMonth(month, year, isleap(year)) + day - 1;
    return absolute;
}

Boolean CFGregorianDateIsValid(CFGregorianDate gdate, CFOptionFlags unitFlags) {
    if ((unitFlags & kCFGregorianUnitsYears) && (gdate.year <= 0)) return false;
    if ((unitFlags & kCFGregorianUnitsMonths) && (gdate.month < 1 || 12 < gdate.month)) return false;
    if ((unitFlags & kCFGregorianUnitsDays) && (gdate.day < 1 || 31 < gdate.day)) return false;
    if ((unitFlags & kCFGregorianUnitsHours) && (gdate.hour < 0 || 23 < gdate.hour)) return false;
    if ((unitFlags & kCFGregorianUnitsMinutes) && (gdate.minute < 0 || 59 < gdate.minute)) return false;
    if ((unitFlags & kCFGregorianUnitsSeconds) && (gdate.second < 0.0 || 60.0 <= gdate.second)) return false;
    if ((unitFlags & kCFGregorianUnitsDays) && (unitFlags & kCFGregorianUnitsMonths) && (unitFlags & kCFGregorianUnitsYears) && (__CFDaysInMonth(gdate.month, gdate.year - 2001, isleap(gdate.year - 2001)) < gdate.day)) return false;
    return true;
}

CFAbsoluteTime CFGregorianDateGetAbsoluteTime(CFGregorianDate gdate, CFTimeZoneRef tz) {
    CFAbsoluteTime at;
    at = 86400.0 * __CFAbsoluteFromYMD(gdate.year - 2001, gdate.month, gdate.day);
    at += 3600.0 * gdate.hour + 60.0 * gdate.minute + gdate.second;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    if (NULL != tz) {
	__CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    }
    CFTimeInterval offset0, offset1;
    if (NULL != tz) {
	offset0 = CFTimeZoneGetSecondsFromGMT(tz, at);
	offset1 = CFTimeZoneGetSecondsFromGMT(tz, at - offset0);
	at -= offset1;
    }
#endif
    return at;
}

CFGregorianDate CFAbsoluteTimeGetGregorianDate(CFAbsoluteTime at, CFTimeZoneRef tz) {
    CFGregorianDate gdate;
    int64_t absolute, year;
    int8_t month, day;
    CFAbsoluteTime fixedat;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    if (NULL != tz) {
	__CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    }
    fixedat = at + (NULL != tz ? CFTimeZoneGetSecondsFromGMT(tz, at) : 0.0);
#else
    fixedat = at;
#endif
    absolute = (int64_t)floor(fixedat / 86400.0);
    __CFYMDFromAbsolute(absolute, &year, &month, &day);
    if (INT32_MAX - 2001 < year) year = INT32_MAX - 2001;
    gdate.year = year + 2001;
    gdate.month = month;
    gdate.day = day;
    gdate.hour = __CFDoubleModToInt(floor(fixedat / 3600.0), 24);
    gdate.minute = __CFDoubleModToInt(floor(fixedat / 60.0), 60);
    gdate.second = __CFDoubleMod(fixedat, 60);
    if (0.0 == gdate.second) gdate.second = 0.0;	// stomp out possible -0.0
    return gdate;
}

/* Note that the units of years and months are not equal length, but are treated as such. */
CFAbsoluteTime CFAbsoluteTimeAddGregorianUnits(CFAbsoluteTime at, CFTimeZoneRef tz, CFGregorianUnits units) {
    CFGregorianDate gdate;
    CFGregorianUnits working;
    CFAbsoluteTime candidate_at0, candidate_at1;
    uint8_t monthdays;

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    if (NULL != tz) {
	__CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    }
#endif
    
    /* Most people seem to expect years, then months, then days, etc.
	to be added in that order.  Thus, 27 April + (4 days, 1 month)
	= 31 May, and not 1 June. This is also relatively predictable.

	On another issue, months not being equal length, people also
	seem to expect late day-of-month clamping (don't clamp as you
	go through months), but clamp before adding in the days. Late
	clamping is also more predictable given random starting points
	and random numbers of months added (ie Jan 31 + 2 months could
	be March 28 or March 29 in different years with aggressive
	clamping). Proportionality (28 Feb + 1 month = 31 March) is
	also not expected.

	Also, people don't expect time zone transitions to have any
	effect when adding years and/or months and/or days, only.
	Hours, minutes, and seconds, though, are added in as humans
	would experience the passing of that time. What this means
	is that if the date, after adding years, months, and days
	lands on some date, and then adding hours, minutes, and
	seconds crosses a time zone transition, the time zone
	transition is accounted for. If adding years, months, and
	days gets the date into a different time zone offset period,
	that transition is not taken into account.
    */
    gdate = CFAbsoluteTimeGetGregorianDate(at, tz);
    /* We must work in a CFGregorianUnits, because the fields in the CFGregorianDate can easily overflow */
    working.years = gdate.year;
    working.months = gdate.month;
    working.days = gdate.day;
    working.years += units.years;
    working.months += units.months;
    while (12 < working.months) {
	working.months -= 12;
	working.years += 1;
    }
    while (working.months < 1) {
	working.months += 12;
	working.years -= 1;
    }
    monthdays = __CFDaysInMonth(working.months, working.years - 2001, isleap(working.years - 2001));
    if (monthdays < working.days) {	/* Clamp day to new month */
	working.days = monthdays;
    }
    working.days += units.days;
    while (monthdays < working.days) {
	working.months += 1;
	if (12 < working.months) {
	    working.months -= 12;
	    working.years += 1;
	}
	working.days -= monthdays;
	monthdays = __CFDaysInMonth(working.months, working.years - 2001, isleap(working.years - 2001));
    }
    while (working.days < 1) {
	working.months -= 1;
	if (working.months < 1) {
	    working.months += 12;
	    working.years -= 1;
	}
	monthdays = __CFDaysInMonth(working.months, working.years - 2001, isleap(working.years - 2001));
	working.days += monthdays;
    }
    gdate.year = working.years;
    gdate.month = working.months;
    gdate.day = working.days;
    /* Roll in hours, minutes, and seconds */
    candidate_at0 = CFGregorianDateGetAbsoluteTime(gdate, tz);
    candidate_at1 = candidate_at0 + 3600.0 * units.hours + 60.0 * units.minutes + units.seconds;
    /* If summing in the hours, minutes, and seconds delta pushes us
     * into a new time zone offset, that will automatically be taken
     * care of by the fact that we just add the raw time above. To
     * undo that effect, we'd have to get the time zone offsets for
     * candidate_at0 and candidate_at1 here, and subtract the
     * difference (offset1 - offset0) from candidate_at1. */
    return candidate_at1;
}

/* at1 - at2.  The only constraint here is that this needs to be the inverse
of CFAbsoluteTimeByAddingGregorianUnits(), but that's a very rigid constraint.
Unfortunately, due to the nonuniformity of the year and month units, this
inversion essentially has to approximate until it finds the answer. */
CFGregorianUnits CFAbsoluteTimeGetDifferenceAsGregorianUnits(CFAbsoluteTime at1, CFAbsoluteTime at2, CFTimeZoneRef tz, CFOptionFlags unitFlags) {
    const int32_t seconds[5] = {366 * 24 * 3600, 31 * 24 * 3600, 24 * 3600, 3600, 60};
    CFGregorianUnits units = {0, 0, 0, 0, 0, 0.0};
    CFAbsoluteTime atold, atnew = at2;
    int32_t idx, incr;
    incr = (at2 < at1) ? 1 : -1;
    /* Successive approximation: years, then months, then days, then hours, then minutes. */
    for (idx = 0; idx < 5; idx++) {
	if (unitFlags & (1 << idx)) {
	    ((int32_t *)&units)[idx] = -3 * incr + (int32_t)((at1 - atnew) / seconds[idx]);
	    do {
		atold = atnew;
		((int32_t *)&units)[idx] += incr;
		atnew = CFAbsoluteTimeAddGregorianUnits(at2, tz, units);
	    } while ((1 == incr && atnew <= at1) || (-1 == incr && at1 <= atnew));
	    ((int32_t *)&units)[idx] -= incr;
	    atnew = atold;
	}
    }
    if (unitFlags & kCFGregorianUnitsSeconds) {
	units.seconds = at1 - atnew;
    }
    if (0.0 == units.seconds) units.seconds = 0.0;	// stomp out possible -0.0
    return units;
}

SInt32 CFAbsoluteTimeGetDayOfWeek(CFAbsoluteTime at, CFTimeZoneRef tz) {
    int64_t absolute;
    CFAbsoluteTime fixedat;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    if (NULL != tz) {
	__CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    }
    fixedat = at + (NULL != tz ? CFTimeZoneGetSecondsFromGMT(tz, at) : 0.0);
#else
    fixedat = at;
#endif
    absolute = (int64_t)floor(fixedat / 86400.0);
    return (absolute < 0) ? ((absolute + 1) % 7 + 7) : (absolute % 7 + 1); /* Monday = 1, etc. */
}

SInt32 CFAbsoluteTimeGetDayOfYear(CFAbsoluteTime at, CFTimeZoneRef tz) {
    CFAbsoluteTime fixedat;
    int64_t absolute, year;
    int8_t month, day;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    if (NULL != tz) {
	__CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    }
    fixedat = at + (NULL != tz ? CFTimeZoneGetSecondsFromGMT(tz, at) : 0.0);
#else
    fixedat = at;
#endif
    absolute = (int64_t)floor(fixedat / 86400.0);
    __CFYMDFromAbsolute(absolute, &year, &month, &day);
    return __CFDaysBeforeMonth(month, year, isleap(year)) + day;
}

/* "the first week of a year is the one which includes the first Thursday" (ISO 8601) */
SInt32 CFAbsoluteTimeGetWeekOfYear(CFAbsoluteTime at, CFTimeZoneRef tz) {
    int64_t absolute, year;
    int8_t month, day;
    CFAbsoluteTime fixedat;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    if (NULL != tz) {
	__CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    }
    fixedat = at + (NULL != tz ? CFTimeZoneGetSecondsFromGMT(tz, at) : 0.0);
#else
    fixedat = at;
#endif
    absolute = (int64_t)floor(fixedat / 86400.0);
    __CFYMDFromAbsolute(absolute, &year, &month, &day);
    double absolute0101 = __CFAbsoluteFromYMD(year, 1, 1);
    int64_t dow0101 = __CFDoubleModToInt(absolute0101, 7) + 1;
    /* First three and last three days of a year can end up in a week of a different year */
    if (1 == month && day < 4) {
	if ((day < 4 && 5 == dow0101) || (day < 3 && 6 == dow0101) || (day < 2 && 7 == dow0101)) {
	    return 53;
	}
    }
    if (12 == month && 28 < day) {
	double absolute20101 = __CFAbsoluteFromYMD(year + 1, 1, 1);
	int64_t dow20101 = __CFDoubleModToInt(absolute20101, 7) + 1;
	if ((28 < day && 4 == dow20101) || (29 < day && 3 == dow20101) || (30 < day && 2 == dow20101)) {
	    return 1;
	}
    }
    /* Days into year, plus a week-shifting correction, divided by 7. First week is 1. */
    return (__CFDaysBeforeMonth(month, year, isleap(year)) + day + (dow0101 - 11) % 7 + 2) / 7 + 1;
}


