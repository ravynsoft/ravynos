/*	CFCalendar.c
	Copyright (c) 2004-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFRuntime.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#include "CFPriv.h"
#include "CFCalendar_Internal.h"
#include "CFLocaleInternal.h"
#include "CFICULogging.h"
#include "CFDateInterval.h"
#include <assert.h>


// This avoids having to use a deprecated enum value throughout the implementation
enum {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    kCFCalendarUnitWeek_Deprecated = kCFCalendarUnitWeek,
#pragma GCC diagnostic pop
};

// In Swift Foundation, nanoseconds are always available.
#define _CF_CALENDAR_NANOSECONDS_AVAILABLE 1

#define ICU_LOG(FMT, ...) do { } while (0)

#define MIN_CALENDAR_TIME -211845067200.0 // Julian day 0 (-4713-01-01 12:00:00 +0000) in CFAbsoluteTime.
#define MAX_CALENDAR_TIME 15927175497600.0 // 50000-01-01 00:00:00 +0000, smaller than the max time ICU supported.
#define __CFCalendarValidateAndCapTimeRange(at) do { \
    if (at < MIN_CALENDAR_TIME || at > MAX_CALENDAR_TIME) { \
        os_log_error(_CFOSLog(), "CFAbsoluteTime %lf exceeds calendar calculation range.", at); \
        if (at < MIN_CALENDAR_TIME) { at = MIN_CALENDAR_TIME; } \
        else { at = MAX_CALENDAR_TIME; } \
    } \
} while (0)

extern CFDictionaryRef __CFLocaleGetPrefs(CFLocaleRef locale);

#define MIN_TIMEZONE_UDATE -2177452800000.0  // 1901-01-01 00:00:00 +0000
#define MAX_TIMEZONE_UDATE  4133980800000.0  // 2101-01-01 00:00:00 +0000

CF_PRIVATE Boolean __calcNextDaylightSavingTimeTransition(UCalendar *ucal, UDate start_udate, UDate limit, UDate *answer) {
    if (start_udate < MIN_TIMEZONE_UDATE) start_udate = MIN_TIMEZONE_UDATE; // answer question as if this date for anything earlier
    if (MAX_TIMEZONE_UDATE < limit) limit = MAX_TIMEZONE_UDATE; // limit gets the smaller
    if (limit < start_udate) { // no transitions searched for after the limit arg, or the max time (for performance)
        return false;
    }
    UErrorCode status = U_ZERO_ERROR;
    ucal_setMillis(ucal, start_udate, &status);
#if TARGET_OS_WIN32 // TODO: Unify this and CFTimeZoneGetNextDaylightSavingTimeTransition.
    UBool b = false;
#else
    UBool b = ucal_getTimeZoneTransitionDate(ucal, UCAL_TZ_TRANSITION_NEXT, answer, &status);
#endif
    if (!U_SUCCESS(status) || limit < *answer) {
        b = false;
    }
    return b ? true : false;
}

// check if a given time is within the second repeated time frame of time zone transition
// for example, the midnight Oct 1 1978 to 1am in Rome has been repeated twice due to dst transition
// this function returns true for anytime in between the second midnight to the second 1am, excluding the second 1am
// if the function returns true, it will also return the transition point, and the length of the transition
static Boolean __CFCalendarGetTimeRangeOfTimeZoneTransition(CFCalendarRef cal, CFAbsoluteTime at, CFAbsoluteTime *transition, CFTimeInterval *length) {
    // if the given time is before 1900, assume there is no dst transition yet
    if (at < -3187299600.0) {
        return false;
    }

    UDate ans = 0.0;
    UDate start = (at - 48.0 * 60 * 60 + kCFAbsoluteTimeIntervalSince1970) * 1000.0; // start back 48 hours
    UErrorCode status = U_ZERO_ERROR;
    UDate orig_millis = __cficu_ucal_getMillis(cal->_cal, &status);
    Boolean b = __calcNextDaylightSavingTimeTransition(cal->_cal, start, start + 4 * 86400 * 1000.0, &ans);
    status = U_ZERO_ERROR;
    __cficu_ucal_setMillis(cal->_cal, orig_millis, &status);
    CFAbsoluteTime tran = (ans / 1000.0) - kCFAbsoluteTimeIntervalSince1970;
    // the transition must be at or before "at" if "at" is within the repeated time frame
    if (!b || tran > at) {
        return false;
    }
    
    // gmt offset includes dst offset
    CFTimeInterval preOffset = CFTimeZoneGetSecondsFromGMT(cal->_tz, tran - 1.0);
    CFTimeInterval nextOffset = CFTimeZoneGetSecondsFromGMT(cal->_tz, tran + 1.0);
    
    CFTimeInterval diff = preOffset - nextOffset;
    
    // gmt offset before the transition > gmt offset after the transition => backward dst transition
    if (diff > 0.0 && at >= tran && at < tran + diff) {
        if (transition) *transition = tran;
        if (length) *length = diff;
        return true;
    }
    
    return false;
}

static void __CFCalendarSetToFirstInstant(CFCalendarRef calendar, CFCalendarUnit unit, CFAbsoluteTime at) {
    // Set UCalendar to first instant of unit prior to 'at'
    UErrorCode status = U_ZERO_ERROR;
    UDate udate = floor((at + kCFAbsoluteTimeIntervalSince1970) * 1000.0);
    __cficu_ucal_setMillis(calendar->_cal, udate, &status);
    
    int32_t target_era = INT_MIN;
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (unit) { // largest to smallest, we set the fields to their minimum value
        case kCFCalendarUnitQuarter: {
            // #warning support UCAL_QUARTER better
            int32_t month = __cficu_ucal_get(calendar->_cal, UCAL_MONTH, &status);
//            int32_t leap = __cficu_ucal_get(calendar->_cal, UCAL_IS_LEAP_MONTH, &status);
            if (kCFCalendarIdentifierHebrew == CFCalendarGetIdentifier(calendar)) {
                int32_t qmonth[] = {0, 0, 0, 3, 3, 3, 3, 7, 7, 7, 10, 10, 10};
                month = qmonth[month];
            } else {
                // A lunar leap month is considered to be in the same quarter
                // that the base month number is in.
                int32_t qmonth[] = {0, 0, 0, 3, 3, 3, 6, 6, 6, 9, 9, 9, 9};
                month = qmonth[month];
            }
            // #warning if there is a lunar leap month of the same number *preceeding* month N,
            // then we should set the calendar to the leap month, not the regular month.
            __cficu_ucal_set(calendar->_cal, UCAL_MONTH, month);
            __cficu_ucal_set(calendar->_cal, UCAL_IS_LEAP_MONTH, 0);
            goto month;
        }
        case kCFCalendarUnitYearForWeekOfYear:;
            __cficu_ucal_set(calendar->_cal, UCAL_WEEK_OF_YEAR, __cficu_ucal_getLimit(calendar->_cal, UCAL_WEEK_OF_YEAR, UCAL_ACTUAL_MINIMUM, &status));
        case kCFCalendarUnitWeek_Deprecated:;
        case kCFCalendarUnitWeekOfMonth:;
        case kCFCalendarUnitWeekOfYear:;
            // reduce to first day of week, then reduce the rest of the day
            int32_t goal; goal = calendar->_firstWeekday;
            int32_t dow; dow = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status);
            while (dow != goal) {
                // 13161987: we may skip the first day of week if the midnight is missing
                __cficu_ucal_add(calendar->_cal, UCAL_DAY_OF_MONTH, -3, &status);
                __cficu_ucal_add(calendar->_cal, UCAL_DAY_OF_MONTH, 2, &status);
                dow = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status);
            }
            goto day;
            
        case kCFCalendarUnitEra:
            target_era = __cficu_ucal_get(calendar->_cal, UCAL_ERA, &status);
            __cficu_ucal_set(calendar->_cal, UCAL_YEAR, __cficu_ucal_getLimit(calendar->_cal, UCAL_YEAR, UCAL_ACTUAL_MINIMUM, &status));
        case kCFCalendarUnitYear:
            __cficu_ucal_set(calendar->_cal, UCAL_MONTH, __cficu_ucal_getLimit(calendar->_cal, UCAL_MONTH, UCAL_ACTUAL_MINIMUM, &status));
            // #warning if there is a lunar leap month of the same number *preceeding* month N,
            // then we should set the calendar to the leap month, not the regular month.
            __cficu_ucal_set(calendar->_cal, UCAL_IS_LEAP_MONTH, 0);
        case kCFCalendarUnitMonth:
        month:;
            __cficu_ucal_set(calendar->_cal, UCAL_DAY_OF_MONTH, __cficu_ucal_getLimit(calendar->_cal, UCAL_DAY_OF_MONTH, UCAL_ACTUAL_MINIMUM, &status));
        case kCFCalendarUnitWeekdayOrdinal:
        case kCFCalendarUnitWeekday:
        case kCFCalendarUnitDay:
        day:;
            __cficu_ucal_set(calendar->_cal, UCAL_HOUR_OF_DAY, __cficu_ucal_getLimit(calendar->_cal, UCAL_HOUR_OF_DAY, UCAL_ACTUAL_MINIMUM, &status));
        case kCFCalendarUnitHour:
            __cficu_ucal_set(calendar->_cal, UCAL_MINUTE, __cficu_ucal_getLimit(calendar->_cal, UCAL_MINUTE, UCAL_ACTUAL_MINIMUM, &status));
        case kCFCalendarUnitMinute:
            __cficu_ucal_set(calendar->_cal, UCAL_SECOND, __cficu_ucal_getLimit(calendar->_cal, UCAL_SECOND, UCAL_ACTUAL_MINIMUM, &status));
        case kCFCalendarUnitSecond:
            __cficu_ucal_set(calendar->_cal, UCAL_MILLISECOND, 0);
    }
#pragma GCC diagnostic pop // See 10693376
    if (INT_MIN != target_era && __cficu_ucal_get(calendar->_cal, UCAL_ERA, &status) < target_era) {
        // In the Japanese calendar, and possibly others, eras don't necessarily
        // start on the first day of a year, so the previous code may have backed
        // up into the previous era, and we have to correct forward.
        UDate bad_udate = __cficu_ucal_getMillis(calendar->_cal, &status);
        __cficu_ucal_add(calendar->_cal, UCAL_MONTH, 1, &status);
        while (__cficu_ucal_get(calendar->_cal, UCAL_ERA, &status) < target_era) {
            bad_udate = __cficu_ucal_getMillis(calendar->_cal, &status);
            __cficu_ucal_add(calendar->_cal, UCAL_MONTH, 1, &status);
        }
        udate = __cficu_ucal_getMillis(calendar->_cal, &status);
        // target date is between bad_udate and udate
        for (;;) {
            UDate test_udate = (udate + bad_udate) / 2;
            __cficu_ucal_setMillis(calendar->_cal, test_udate, &status);
            if (__cficu_ucal_get(calendar->_cal, UCAL_ERA, &status) < target_era) {
                bad_udate = test_udate;
            } else {
                udate = test_udate;
            }
            if (fabs(udate - bad_udate) < 1000) break;
        }
        do {
            bad_udate = floor((bad_udate + 1000) / 1000) * 1000;
            __cficu_ucal_setMillis(calendar->_cal, bad_udate, &status);
        } while (__cficu_ucal_get(calendar->_cal, UCAL_ERA, &status) < target_era);
    }
    if (unit == kCFCalendarUnitDay || unit == kCFCalendarUnitWeekday || unit == kCFCalendarUnitWeekdayOrdinal) {
        status = U_ZERO_ERROR;
        int32_t targetDay = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_MONTH, &status);
        int32_t currentDay = targetDay;
        do {
            udate = __cficu_ucal_getMillis(calendar->_cal, &status);
            __cficu_ucal_add(calendar->_cal, UCAL_SECOND, -1, &status);
            currentDay = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_MONTH, &status);
        } while (targetDay == currentDay);
        __cficu_ucal_setMillis(calendar->_cal, udate, &status);
    }
    
    CFAbsoluteTime start, t;
    CFTimeInterval length;
    udate = __cficu_ucal_getMillis(calendar->_cal, &status);
    start = udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970;
    if (__CFCalendarGetTimeRangeOfTimeZoneTransition(calendar, start, &t, &length)) {
        udate = (start - length + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        __cficu_ucal_setMillis(calendar->_cal, udate, &status);
    }
}

// assume field is within millisecond to hour
static double __CFCalendarTotalSecondsInSmallUnits(CFCalendarRef calendar, UCalendarDateFields field, UErrorCode *status) {
    double totalSecond = 0;
    if (field == UCAL_MILLISECOND || field == UCAL_MILLISECONDS_IN_DAY) {
        return totalSecond;
    }
    int32_t value = __cficu_ucal_get(calendar->_cal, UCAL_MILLISECOND, status);
    totalSecond += (double)value/1000.0;
    if (field == UCAL_SECOND) {
        return totalSecond;
    }
    value = __cficu_ucal_get(calendar->_cal, UCAL_SECOND, status);
    totalSecond += (double)value;
    if (field == UCAL_MINUTE) {
        return totalSecond;
    }
    value = __cficu_ucal_get(calendar->_cal, UCAL_MINUTE, status);
    totalSecond += (double)value*60.0;
    return totalSecond;
}

// assume the ICU calendar has been initialized
// we rely on ICU to add and roll units which are larger than or equal to DAYs
// we have an assumption which is we assume that there is no time zone with a backward repeated day
// at the time of writing this code, there is only one instance of DST that forwards a day
static UDate __CFCalendarAdd(CFCalendarRef calendar, UCalendarDateFields field, int32_t amount, CFOptionFlags options, UErrorCode *status){
    UDate result;
    CFOptionFlags roll = options & kCFCalendarComponentsWrap;
    if (field == UCAL_MILLISECOND || field == UCAL_SECOND || field == UCAL_MINUTE || field == UCAL_HOUR_OF_DAY ||
        field == UCAL_HOUR || field == UCAL_MILLISECONDS_IN_DAY || field == UCAL_AM_PM) {
        
        double unitLength = 0.0;
        BOOL keepHourInvariant = NO;
        switch (field) {
            case UCAL_MILLISECOND:
            case UCAL_MILLISECONDS_IN_DAY:
                unitLength = 1.0;
                break;
            case UCAL_MINUTE:
                unitLength = 60000.0;
                break;
            case UCAL_SECOND:
                unitLength = 1000.0;
                break;
            case UCAL_HOUR:
            case UCAL_HOUR_OF_DAY:
                unitLength = 3600000.0;
                break;
            case UCAL_AM_PM:
                unitLength = 3600000.0 * 12.0;
                keepHourInvariant = YES;
                break;
            default:
                break;
        }
        
        double leftoverTime = 0.0;
        if (roll) {
            int32_t min = __cficu_ucal_getLimit(calendar->_cal, field, UCAL_ACTUAL_MINIMUM, status);
            int32_t max = __cficu_ucal_getLimit(calendar->_cal, field, UCAL_ACTUAL_MAXIMUM, status);
            int32_t gap = max - min + 1;
            int32_t originalValue = __cficu_ucal_get(calendar->_cal, field, status);
            int32_t finalValue = originalValue + amount;
            finalValue = (finalValue - min) % gap;
            if (finalValue < 0) {
                finalValue += gap;
            }
            finalValue += min;
            if (finalValue < originalValue && amount > 0) {
                amount = finalValue;
                CFAbsoluteTime at = __cficu_ucal_getMillis(calendar->_cal, status) / 1000.0 - kCFAbsoluteTimeIntervalSince1970;
                CFCalendarUnit largeField;
                switch (field) {
                    case UCAL_MILLISECOND:
                    case UCAL_MILLISECONDS_IN_DAY:
                        largeField = kCFCalendarUnitSecond;
                        break;
                    case UCAL_SECOND:
                        largeField = kCFCalendarUnitMinute;
                        break;
                    case UCAL_MINUTE:
                        largeField = kCFCalendarUnitHour;
                        break;
                    case UCAL_HOUR_OF_DAY:
                    case UCAL_HOUR:
                        largeField = kCFCalendarUnitDay;
                        break;
                    default:
                        largeField = kCFCalendarUnitSecond; // Just pick some value
                        break;
                }
                leftoverTime = __CFCalendarTotalSecondsInSmallUnits(calendar, field, status);
                __CFCalendarSetToFirstInstant(calendar, largeField, at);
            } else {
                amount = finalValue - originalValue;
            }
        }
        
        int32_t dst = 0;
        int32_t hour = 0;
        if (keepHourInvariant) {
            dst = __cficu_ucal_get(calendar->_cal, UCAL_DST_OFFSET, status) + __cficu_ucal_get(calendar->_cal, UCAL_ZONE_OFFSET, status);
            hour = __cficu_ucal_get(calendar->_cal, UCAL_HOUR_OF_DAY, status);
        }
        
        result = __cficu_ucal_getMillis(calendar->_cal, status);
        result += (double)amount * unitLength;
        result += leftoverTime * 1000.0;
        __cficu_ucal_setMillis(calendar->_cal, result, status);
        
        if (keepHourInvariant) {
            dst -= __cficu_ucal_get(calendar->_cal, UCAL_DST_OFFSET, status) + __cficu_ucal_get(calendar->_cal, UCAL_ZONE_OFFSET, status);
            if (dst != 0) {
                result = __cficu_ucal_getMillis(calendar->_cal, status) + dst;
                __cficu_ucal_setMillis(calendar->_cal, result, status);
                if (__cficu_ucal_get(calendar->_cal, UCAL_HOUR_OF_DAY, status) != hour) {
                    result -= dst;
                    __cficu_ucal_setMillis(calendar->_cal, result, status);
                }
            }
        }
    } else {
        if (roll) {
            __cficu_ucal_roll(calendar->_cal, field, amount, status);
        } else {
            __cficu_ucal_add(calendar->_cal, field, amount, status);
        }
        
        CFAbsoluteTime t, start;
        CFTimeInterval length;
        result = __cficu_ucal_getMillis(calendar->_cal, status);
        start = result / 1000.0 - kCFAbsoluteTimeIntervalSince1970;
        if (amount > 0 && __CFCalendarGetTimeRangeOfTimeZoneTransition(calendar, start, &t, &length)) {
            result = (start - length + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, result, status);
        }
    }
    return result;
}

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
    if (calendar->_identifier) {
        CFRelease(calendar->_identifier);
    }
    if (calendar->_locale) {
        CFRelease(calendar->_locale);
    }
    if (calendar->_tz) {
        CFRelease(calendar->_tz);
    }
    if (calendar->_gregorianStart) {
        CFRelease(calendar->_gregorianStart);
    }
    if (calendar->_cal) {
        __cficu_ucal_close(calendar->_cal);
    }
}

const CFRuntimeClass __CFCalendarClass = {
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
    return _kCFRuntimeIDCFCalendar;
}

CF_PRIVATE UCalendar *__CFCalendarCreateUCalendar(CFStringRef calendarID, CFStringRef localeID, CFTimeZoneRef tz) {
    if (calendarID) {
        CFDictionaryRef components = CFLocaleCreateComponentsFromLocaleIdentifier(kCFAllocatorSystemDefault, localeID);
        CFMutableDictionaryRef mcomponents = CFDictionaryCreateMutableCopy(kCFAllocatorSystemDefault, 0, components);
        CFDictionarySetValue(mcomponents, kCFLocaleCalendarIdentifierKey, calendarID);
        localeID = CFLocaleCreateLocaleIdentifierFromComponents(kCFAllocatorSystemDefault, mcomponents);
        CFRelease(mcomponents);
        CFRelease(components);
    }

    const size_t BUFFER_SIZE = 512;
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
    UCalendar *cal = __cficu_ucal_open(ubuffer, cnt, cstr, UCAL_DEFAULT, &status);
    if (calendarID) CFRelease(localeID);
    return cal;
}

CF_PRIVATE void __CFCalendarSetupCal(CFCalendarRef calendar) {
    ICU_LOG("                // __CFCalendarSetupCal enter\n");
    calendar->_cal = __CFCalendarCreateUCalendar(calendar->_identifier, CFLocaleGetIdentifier(calendar->_locale), calendar->_tz);
    __cficu_ucal_setAttribute(calendar->_cal, UCAL_FIRST_DAY_OF_WEEK, calendar->_firstWeekday);
    __cficu_ucal_setAttribute(calendar->_cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, calendar->_minDaysInFirstWeek);
    ICU_LOG("    ucal_setAttribute(cal, UCAL_FIRST_DAY_OF_WEEK, %ld);\n", calendar->_firstWeekday);
    ICU_LOG("    ucal_setAttribute(cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, %ld);\n", calendar->_minDaysInFirstWeek);
    if (calendar->_gregorianStart) {
        CFAbsoluteTime at = CFDateGetAbsoluteTime(calendar->_gregorianStart);
        UDate udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        UErrorCode status = U_ZERO_ERROR;
        __cficu_ucal_setGregorianChange(calendar->_cal, udate, &status);
        ICU_LOG("    UErrorCode status = U_ZERO_ERROR;\n");
        ICU_LOG("    UDate udate = %.06f;\n", udate);
        ICU_LOG("    ucal_setGregorianChange(cal, udate, &status);\n");
    }
    ICU_LOG("                // __CFCalendarSetupCal exit\n");
}

CF_PRIVATE Boolean _CFCalendarIsDateInWeekend(CFCalendarRef calendar, CFDateRef date) {
    if (calendar->_cal == NULL) {
        __CFCalendarSetupCal(calendar);
    }
    CFAbsoluteTime at = CFDateGetAbsoluteTime(date);
    UDate udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
    UErrorCode status = U_ZERO_ERROR;
    UBool result = __cficu_ucal_isWeekend(calendar->_cal, udate, &status);
    return result;
}

CF_PRIVATE Boolean _CFCalendarGetNextWeekend(CFCalendarRef calendar, _CFCalendarWeekendRange *range) {
    // TODO: Double check this logic vs that in _NSCFCalendar
    CFIndex weekdaysIndex[7];
    memset(weekdaysIndex, '\0', sizeof(CFIndex)*7);
    CFIndex firstWeekday = CFCalendarGetFirstWeekday(calendar);
    weekdaysIndex[0] = firstWeekday;
    for (CFIndex i = 1; i < 7; i++) {
        weekdaysIndex[i] = (weekdaysIndex[i-1] % 7) + 1;
    }
    UCalendarWeekdayType weekdayTypes[7];
    CFIndex onset = kCFNotFound;
    CFIndex cease = kCFNotFound;
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (!calendar->_cal) {
        return false;
    }
    for (CFIndex i = 0; i < 7; i++) {
        UErrorCode status = U_ZERO_ERROR;
        weekdayTypes[i] = ucal_getDayOfWeekType(calendar->_cal, (UCalendarDaysOfWeek)weekdaysIndex[i], &status);
        if (weekdayTypes[i] == UCAL_WEEKEND_ONSET) {
            onset = weekdaysIndex[i];
        } else if (weekdayTypes[i] == UCAL_WEEKEND_CEASE) {
            cease = weekdaysIndex[i];
        }
    }
    BOOL hasWeekend = NO;
    for (CFIndex i = 0; i < 7; i++) {
        if (weekdayTypes[i] == UCAL_WEEKEND || weekdayTypes[i] == UCAL_WEEKEND_ONSET || weekdayTypes[i] == UCAL_WEEKEND_CEASE) {
            hasWeekend = YES;
            break;
        }
    }
    if (!hasWeekend) {
        return false;
    }
    int32_t onsetTime = 0;
    int32_t ceaseTime = 0;
    if (onset != kCFNotFound) {
        UErrorCode status = U_ZERO_ERROR;
        onsetTime = ucal_getWeekendTransition(calendar->_cal, (UCalendarDaysOfWeek)onset, &status);
    }
    if (cease != kCFNotFound) {
        UErrorCode status = U_ZERO_ERROR;
        ceaseTime = ucal_getWeekendTransition(calendar->_cal, (UCalendarDaysOfWeek)cease, &status);
    }
    CFIndex weekendStart = kCFNotFound;
    CFIndex weekendEnd = kCFNotFound;
    if (onset != kCFNotFound) {
        weekendStart = onset;
    } else {
        if (weekdayTypes[0] == UCAL_WEEKEND && weekdayTypes[6] == UCAL_WEEKEND) {
            for (CFIndex i = 5; i >= 0; i--) {
                if (weekdayTypes[i] != UCAL_WEEKEND) {
                    weekendStart = weekdaysIndex[i + 1];
                    break;
                }
            }
        } else {
            for (CFIndex i = 0; i < 7; i++) {
                if (weekdayTypes[i] == UCAL_WEEKEND) {
                    weekendStart = weekdaysIndex[i];
                    break;
                }
            }
        }
    }
    if (cease != kCFNotFound) {
        weekendEnd = cease;
    } else {
        if (weekdayTypes[0] == UCAL_WEEKEND && weekdayTypes[6] == UCAL_WEEKEND) {
            for (CFIndex i = 1; i < 7; i++) {
                if (weekdayTypes[i] != UCAL_WEEKEND) {
                    weekendEnd = weekdaysIndex[i - 1];
                    break;
                }
            }
        } else {
            for (CFIndex i = 6; i >= 0; i--) {
                if (weekdayTypes[i] == UCAL_WEEKEND) {
                    weekendEnd = weekdaysIndex[i];
                    break;
                }
            }
        }
    }
    
    range->onsetTime = onsetTime / 1000.0;
    range->ceaseTime = ceaseTime / 1000.0;
    range->start = weekendStart;
    range->end = weekendEnd;
    
    // TODO: NSCalendar.m has additional logic here but it depends on stuff not yet ported to this C implementation
    return true;
}

CF_PRIVATE void __CFCalendarZapCal(CFCalendarRef calendar) {
    if (calendar->_cal) __cficu_ucal_close(calendar->_cal);
    calendar->_cal = NULL;
    ICU_LOG("    if (cal) ucal_close(cal);\n");
}

// Applies user-editable settings (first weekday, minimum days in first week) from the given locale onto the given calendar without applying the locale onto the calendar.
static void __CFCalendarApplyUserSettingsFromLocale(CFCalendarRef calendar, CFLocaleRef locale) {
    CFDictionaryRef prefs = __CFLocaleGetPrefs(locale);
    if (prefs && !calendar->_userSet_firstWeekday) {
        CFPropertyListRef metapref = (CFPropertyListRef)CFDictionaryGetValue(prefs, CFSTR("AppleFirstWeekday"));
        if (NULL != metapref && CFGetTypeID(metapref) == CFDictionaryGetTypeID()) {
            metapref = (CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)metapref, calendar->_identifier);
        }
        if (NULL != metapref && CFGetTypeID(metapref) == CFNumberGetTypeID()) {
            CFIndex wkdy;
            if (CFNumberGetValue((CFNumberRef)metapref, kCFNumberCFIndexType, &wkdy)) {
                calendar->_firstWeekday = wkdy;
                if (calendar->_cal) {
                    __cficu_ucal_setAttribute(calendar->_cal, UCAL_FIRST_DAY_OF_WEEK, wkdy);
                    ICU_LOG("    ucal_setAttribute(cal, UCAL_FIRST_DAY_OF_WEEK, %ld);\n", wkdy);
                }
            }
        }
    }
    
    if (prefs && !calendar->_userSet_minDaysInFirstWeek) {
        CFPropertyListRef metapref = (CFPropertyListRef)CFDictionaryGetValue(prefs, CFSTR("AppleMinDaysInFirstWeek"));
        if (NULL != metapref && CFGetTypeID(metapref) == CFDictionaryGetTypeID()) {
            metapref = (CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)metapref, calendar->_identifier);
        }
        if (NULL != metapref && CFGetTypeID(metapref) == CFNumberGetTypeID()) {
            CFIndex mwd;
            if (CFNumberGetValue((CFNumberRef)metapref, kCFNumberCFIndexType, &mwd)) {
                calendar->_minDaysInFirstWeek = mwd;
                if (calendar->_cal) {
                    __cficu_ucal_setAttribute(calendar->_cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, mwd);
                    ICU_LOG("    ucal_setAttribute(cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, %ld);\n", mwd);
                }
            }
        }
    }
}

static bool _CFCalendarInitialize(CFCalendarRef calendar, CFAllocatorRef allocator, CFStringRef identifier, CFTimeZoneRef tz, CFLocaleRef locale, CFIndex firstDayOfWeek, CFIndex minDaysInFirstWeek, CFDateRef gregorianStartDate) {
    ICU_LOG("                // CFCalendarCreateWithIdentifier enter\n");
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    __CFGenericValidateType(identifier, CFStringGetTypeID());
    
    CFStringRef canonicalIdent = NULL;
    if (CFEqual(kCFCalendarIdentifierGregorian, identifier)) canonicalIdent = kCFCalendarIdentifierGregorian;
    else if (CFEqual(kCFCalendarIdentifierJapanese, identifier)) canonicalIdent = kCFCalendarIdentifierJapanese;
    else if (CFEqual(kCFCalendarIdentifierBuddhist, identifier)) canonicalIdent = kCFCalendarIdentifierBuddhist;
    else if (CFEqual(kCFCalendarIdentifierIslamic, identifier)) canonicalIdent = kCFCalendarIdentifierIslamic;
    else if (CFEqual(kCFCalendarIdentifierIslamicCivil, identifier)) canonicalIdent = kCFCalendarIdentifierIslamicCivil;
    else if (CFEqual(kCFCalendarIdentifierHebrew, identifier)) canonicalIdent = kCFCalendarIdentifierHebrew;
    else if (CFEqual(kCFCalendarIdentifierRepublicOfChina, identifier)) canonicalIdent = kCFCalendarIdentifierRepublicOfChina;
    else if (CFEqual(kCFCalendarIdentifierPersian, identifier)) canonicalIdent = kCFCalendarIdentifierPersian;
    else if (CFEqual(kCFCalendarIdentifierIndian, identifier)) canonicalIdent = kCFCalendarIdentifierIndian;
    else if (CFEqual(kCFCalendarIdentifierCoptic, identifier)) canonicalIdent = kCFCalendarIdentifierCoptic;
    else if (CFEqual(kCFCalendarIdentifierEthiopicAmeteMihret, identifier)) canonicalIdent = kCFCalendarIdentifierEthiopicAmeteMihret;
    else if (CFEqual(kCFCalendarIdentifierEthiopicAmeteAlem, identifier)) canonicalIdent = kCFCalendarIdentifierEthiopicAmeteAlem;
    else if (CFEqual(kCFCalendarIdentifierChinese, identifier)) canonicalIdent = kCFCalendarIdentifierChinese;
    else if (CFEqual(kCFCalendarIdentifierISO8601, identifier)) canonicalIdent = kCFCalendarIdentifierISO8601;
    else if (CFEqual(kCFCalendarIdentifierIslamicTabular, identifier)) canonicalIdent = kCFCalendarIdentifierIslamicTabular;
    else if (CFEqual(kCFCalendarIdentifierIslamicUmmAlQura, identifier)) canonicalIdent = kCFCalendarIdentifierIslamicUmmAlQura;
    if (!canonicalIdent) ICU_LOG("                // CFCalendarCreateWithIdentifier exit NULL 1\n");
    if (!canonicalIdent) return false;
    
    calendar->_identifier = (CFStringRef)CFRetain(canonicalIdent);
    calendar->_locale = locale ? CFLocaleCreateCopy(allocator, locale) : CFRetain(CFLocaleGetSystem());
    calendar->_tz = tz ? CFRetain(tz) : CFTimeZoneCopyDefault();
    calendar->_cal = __CFCalendarCreateUCalendar(calendar->_identifier, CFLocaleGetIdentifier(calendar->_locale), calendar->_tz);
    if (!calendar->_cal) {
        ICU_LOG("                // CFCalendarCreateWithIdentifier exit NULL 3\n");
        return false;
    }
    calendar->_firstWeekday = firstDayOfWeek == kCFNotFound ? __cficu_ucal_getAttribute(calendar->_cal, UCAL_FIRST_DAY_OF_WEEK) : firstDayOfWeek;
    calendar->_minDaysInFirstWeek = minDaysInFirstWeek == kCFNotFound ? __cficu_ucal_getAttribute(calendar->_cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK) : minDaysInFirstWeek;
    ICU_LOG("    int32_t firstWeekday = ucal_getAttribute(cal, UCAL_FIRST_DAY_OF_WEEK);\n");
    ICU_LOG("    int32_t minDaysInFirstWeek = ucal_getAttribute(cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK);\n");
    if (kCFCalendarIdentifierGregorian == calendar->_identifier) {
        ICU_LOG("    UErrorCode status = U_ZERO_ERROR;\n");
        ICU_LOG("    UDate udate = ucal_getGregorianChange(cal, &status);\n");
        ICU_LOG("    CFAbsoluteTime gregorianStart = U_SUCCESS(status) ? (udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970) : -13197600000.0;\n");
        CFAbsoluteTime at = 0;
        if (gregorianStartDate) {
            at = CFDateGetAbsoluteTime(gregorianStartDate);
            calendar->_gregorianStart = CFRetain(gregorianStartDate);
        } else {
            UErrorCode status = U_ZERO_ERROR;
            UDate udate = __cficu_ucal_getGregorianChange(calendar->_cal, &status);
            at = U_SUCCESS(status) ? (udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970) : -13197600000.0; // Oct 15, 1582
            calendar->_gregorianStart = CFDateCreate(CFGetAllocator(calendar), at);
        }
        UErrorCode status = U_ZERO_ERROR;
        UDate udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        __cficu_ucal_setGregorianChange(calendar->_cal, udate, &status);
    }
    calendar->_userSet_firstWeekday = false;
    calendar->_userSet_minDaysInFirstWeek = false;
    calendar->_userSet_gregorianStart = false;
    ICU_LOG("                // CFCalendarCreateWithIdentifier exit\n");
    return true;
}

static CFCalendarRef _CFCalendarCreate(CFAllocatorRef allocator, CFStringRef identifier, CFTimeZoneRef tz, CFLocaleRef locale, CFIndex firstDayOfWeek, CFIndex minDaysInFirstWeek, CFDateRef gregorianStartDate) {
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());

    struct __CFCalendar *calendar = NULL;
    size_t size = sizeof(struct __CFCalendar) - sizeof(CFRuntimeBase);
    calendar = (struct __CFCalendar *)_CFRuntimeCreateInstance(allocator, CFCalendarGetTypeID(), size, NULL);
    if (NULL == calendar) {
        ICU_LOG("                // CFCalendarCreateWithIdentifier exit NULL 2\n");
        return NULL;
    }
    
    if (!_CFCalendarInitialize(calendar, allocator, identifier, tz, locale, firstDayOfWeek, minDaysInFirstWeek, gregorianStartDate)) {
        // _CFCalendarInitialize will have already logged which exit path was taken.
        CFRelease(calendar);
        return NULL;
    }
    
    return calendar;
}

CFCalendarRef CFCalendarCopyCurrent(void) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef calID = (CFStringRef)CFLocaleGetValue(locale, kCFLocaleCalendarIdentifierKey);
    if (calID) {
        CFCalendarRef calendar = _CFCalendarCreate(kCFAllocatorSystemDefault, calID, NULL, locale, kCFNotFound, kCFNotFound, NULL);
        
        // `calendar` has the default first weekday and other locale settings set on it.
        // We need to explicitly apply the settings from the locale without creating a new copy of it (via `CFCalendarSetLocale`).
        __CFCalendarApplyUserSettingsFromLocale(calendar, locale);
        
        CFRelease(locale);
        return calendar;
    } else if(locale) {
        CFRelease(locale);
    }
    return NULL;
}

CFCalendarRef CFCalendarCreateWithIdentifier(CFAllocatorRef allocator, CFStringRef identifier) {
    return _CFCalendarCreate(allocator, identifier, NULL, NULL, kCFNotFound, kCFNotFound, NULL);
}

CF_CROSS_PLATFORM_EXPORT Boolean _CFCalendarInitWithIdentifier(CFCalendarRef calendar, CFStringRef identifier) {
    return _CFCalendarInitialize(calendar, kCFAllocatorSystemDefault, identifier, NULL, NULL, kCFNotFound, kCFNotFound, NULL) ? true : false;
}

CFCalendarRef _CFCalendarCreateCopy(CFAllocatorRef allocator, CFCalendarRef calendar) {
    //We should probably just conditionally call -copyWithZone: here but I'm concerned that it could expose incorrect third party subclasses that have just happened to get away with it until now
    Boolean isObjC = CF_IS_OBJC(_kCFRuntimeIDCFCalendar, calendar);
    CFCalendarRef result = NULL;
    if (isObjC) {
        CFTimeZoneRef tz = CFCalendarCopyTimeZone(calendar);
        CFLocaleRef locale = CFCalendarCopyLocale(calendar);
        CFDateRef gsd = CFCalendarCopyGregorianStartDate(calendar);
        CFIndex firstWeekday = CFCalendarGetFirstWeekday(calendar);
        CFIndex minDaysInFirstWeek = CFCalendarGetMinimumDaysInFirstWeek(calendar);
        //Do not attempt to refactor _CFCalendarCreate to take fewer arguments with the idea of using the setter functions instead, the setters also set the _userSet_* flags
        result = _CFCalendarCreate(allocator, CFCalendarGetIdentifier(calendar), tz, locale, firstWeekday, minDaysInFirstWeek, gsd);
        if (tz) CFRelease(tz);
        if (locale) CFRelease(locale);
        if (gsd) CFRelease(gsd);
    } else {
        result = _CFCalendarCreate(allocator, calendar->_identifier, calendar->_tz, calendar->_locale, calendar->_firstWeekday, calendar->_minDaysInFirstWeek, calendar->_gregorianStart);
    }
    
    return result;
}

CFStringRef CFCalendarGetIdentifier(CFCalendarRef calendar) {
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFStringRef, calendar, NSCalendar.calendarIdentifier);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFStringRef, (NSCalendar *)calendar, calendarIdentifier);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return calendar->_identifier;
}

CFLocaleRef CFCalendarCopyLocale(CFCalendarRef calendar) {
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFLocaleRef, calendar, NSCalendar.copyLocale);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFLocaleRef, (NSCalendar *)calendar, _copyLocale);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return CFLocaleCreateCopy(CFGetAllocator(calendar->_locale), calendar->_locale);
}

void CFCalendarSetLocale(CFCalendarRef calendar, CFLocaleRef locale) {
    ICU_LOG("                // CFCalendarSetLocale enter\n");
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, void, calendar, NSCalendar.setLocale, locale);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, void, (NSCalendar *)calendar, setLocale:(NSLocale *)locale);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    __CFGenericValidateType(locale, CFLocaleGetTypeID());
    if (locale && (locale != calendar->_locale)) {
        CFRelease(calendar->_locale);
        calendar->_locale = CFLocaleCreateCopy(CFGetAllocator(calendar), locale);

        if (calendar->_cal) __cficu_ucal_close(calendar->_cal);
        ICU_LOG("    if (cal) ucal_close(cal);\n");
        // do NOT use __CFCalendarSetupCal here
        calendar->_cal = __CFCalendarCreateUCalendar(calendar->_identifier, CFLocaleGetIdentifier(calendar->_locale), calendar->_tz);
        if (!calendar->_cal) HALT;

        if (!calendar->_userSet_firstWeekday) {
            calendar->_firstWeekday = __cficu_ucal_getAttribute(calendar->_cal, UCAL_FIRST_DAY_OF_WEEK);
            ICU_LOG("    int32_t firstWeekday = ucal_getAttribute(cal, UCAL_FIRST_DAY_OF_WEEK);\n");
            __cficu_ucal_setAttribute(calendar->_cal, UCAL_FIRST_DAY_OF_WEEK, calendar->_firstWeekday);
        } else {
            __cficu_ucal_setAttribute(calendar->_cal, UCAL_FIRST_DAY_OF_WEEK, calendar->_firstWeekday);
            ICU_LOG("    ucal_setAttribute(cal, UCAL_FIRST_DAY_OF_WEEK, %ld);\n", calendar->_firstWeekday);
        }
        if (!calendar->_userSet_minDaysInFirstWeek) {
            calendar->_minDaysInFirstWeek = __cficu_ucal_getAttribute(calendar->_cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK);
            ICU_LOG("    int32_t minDaysInFirstWeek = ucal_getAttribute(cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK);\n");
            __cficu_ucal_setAttribute(calendar->_cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, calendar->_minDaysInFirstWeek);
        } else {
            __cficu_ucal_setAttribute(calendar->_cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, calendar->_minDaysInFirstWeek);
            ICU_LOG("    ucal_setAttribute(cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, %ld);\n", calendar->_minDaysInFirstWeek);
        }
        if (!calendar->_userSet_gregorianStart && calendar->_gregorianStart) {
            CFRelease(calendar->_gregorianStart);
            UErrorCode status = U_ZERO_ERROR;
            UDate udate = __cficu_ucal_getGregorianChange(calendar->_cal, &status);
            CFAbsoluteTime at = U_SUCCESS(status) ? (udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970) : -13197600000.0; // Oct 15, 1582
            calendar->_gregorianStart = CFDateCreate(CFGetAllocator(calendar), at);
            udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            status = U_ZERO_ERROR;
            __cficu_ucal_setGregorianChange(calendar->_cal, udate, &status);
            ICU_LOG("    UErrorCode status = U_ZERO_ERROR;\n");
            ICU_LOG("    UDate udate = ucal_getGregorianChange(cal, &status);\n");
            ICU_LOG("    CFAbsoluteTime gregorianStart = U_SUCCESS(status) ? (udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970) : -13197600000.0;\n");
        } else if (calendar->_gregorianStart) {
            CFAbsoluteTime at = CFDateGetAbsoluteTime(calendar->_gregorianStart);
            UDate udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            UErrorCode status = U_ZERO_ERROR;
            if (calendar->_cal) __cficu_ucal_setGregorianChange(calendar->_cal, udate, &status);
            ICU_LOG("    CFAbsoluteTime at = %.06f;\n", at);
            ICU_LOG("    UDate udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0;\n");
            ICU_LOG("    UErrorCode status = U_ZERO_ERROR;\n");
            ICU_LOG("    if (cal) ucal_setGregorianChange(cal, udate, &status);\n");
        }
        
        __CFCalendarApplyUserSettingsFromLocale(calendar, locale);
    }
    ICU_LOG("                // CFCalendarSetLocale exit\n");
}

CFTimeZoneRef CFCalendarCopyTimeZone(CFCalendarRef calendar) {
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFTimeZoneRef, calendar, NSCalendar.copyTimeZone);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFTimeZoneRef, (NSCalendar *)calendar, _copyTimeZone);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return (CFTimeZoneRef)CFRetain(calendar->_tz);
}

void CFCalendarSetTimeZone(CFCalendarRef calendar, CFTimeZoneRef tz) {
    ICU_LOG("                // CFCalendarSetTimeZone enter\n");
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, void, calendar, NSCalendar.setTimeZone, tz);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, void, (NSCalendar *)calendar, setTimeZone:(NSTimeZone *)tz);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (tz) __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    if (tz != calendar->_tz) {
        if (calendar->_tz) CFRelease(calendar->_tz);
        calendar->_tz = tz ? (CFTimeZoneRef)CFRetain(tz) : CFTimeZoneCopyDefault();
        if (calendar->_cal) __CFCalendarZapCal(calendar);
    }
    ICU_LOG("                // CFCalendarSetTimeZone exit\n");
}

CFIndex CFCalendarGetFirstWeekday(CFCalendarRef calendar) {
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFIndex, calendar, NSCalendar.firstWeekday);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFIndex, (NSCalendar *)calendar, firstWeekday);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return calendar->_firstWeekday;
}

void CFCalendarSetFirstWeekday(CFCalendarRef calendar, CFIndex wkdy) {
    ICU_LOG("                // CFCalendarSetFirstWeekday enter\n");
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, void, calendar, NSCalendar.setFirstWeekday, wkdy);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, void, (NSCalendar *)calendar, setFirstWeekday:(NSUInteger)wkdy);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    calendar->_firstWeekday = wkdy;
    if (calendar->_cal) {
        __cficu_ucal_setAttribute(calendar->_cal, UCAL_FIRST_DAY_OF_WEEK, wkdy);
        ICU_LOG("    ucal_setAttribute(cal, UCAL_FIRST_DAY_OF_WEEK, %ld);\n", wkdy);
    }
    calendar->_userSet_firstWeekday = true;
    ICU_LOG("                // CFCalendarSetFirstWeekday exit\n");
}

CFIndex CFCalendarGetMinimumDaysInFirstWeek(CFCalendarRef calendar) {
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFIndex, calendar, NSCalendar.minimumDaysInFirstWeek);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFIndex, (NSCalendar *)calendar, minimumDaysInFirstWeek);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return calendar->_minDaysInFirstWeek;
}

void CFCalendarSetMinimumDaysInFirstWeek(CFCalendarRef calendar, CFIndex mwd) {
    ICU_LOG("                // CFCalendarSetMinimumDaysInFirstWeek enter\n");
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, void, calendar, NSCalendar.setMinimumDaysInFirstWeek, mwd);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, void, (NSCalendar *)calendar, setMinimumDaysInFirstWeek:(NSUInteger)mwd);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    calendar->_minDaysInFirstWeek = mwd;
    if (calendar->_cal) {
        __cficu_ucal_setAttribute(calendar->_cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, mwd);
        ICU_LOG("    ucal_setAttribute(cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, %ld);\n", mwd);
    }
    calendar->_userSet_minDaysInFirstWeek = true;
    ICU_LOG("                // CFCalendarSetMinimumDaysInFirstWeek exit\n");
}

CFDateRef CFCalendarCopyGregorianStartDate(CFCalendarRef calendar) {
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFDateRef, calendar, NSCalendar.copyGregorianStartDate);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFDateRef, (NSCalendar *)calendar, _copyGregorianStartDate);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return calendar->_gregorianStart ? (CFDateRef)CFRetain(calendar->_gregorianStart) : NULL;
}

void CFCalendarSetGregorianStartDate(CFCalendarRef calendar, CFDateRef _Nullable date) {
    ICU_LOG("                // CFCalendarSetGregorianStartDate enter\n");
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, void, calendar, NSCalendar.setGregorianStartDate, date);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, void, (NSCalendar *)calendar, _setGregorianStartDate:(NSDate *)date);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    if (date) __CFGenericValidateType(date, CFDateGetTypeID());
    if (calendar->_gregorianStart) CFRelease(calendar->_gregorianStart);
    calendar->_gregorianStart = NULL;
    if (!date && (kCFCalendarIdentifierGregorian == calendar->_identifier)) {
        UErrorCode status = U_ZERO_ERROR;
        ICU_LOG("    UErrorCode status = U_ZERO_ERROR;\n");
        UCalendar *cal = __CFCalendarCreateUCalendar(calendar->_identifier, CFLocaleGetIdentifier(calendar->_locale), calendar->_tz);
        UDate udate = cal ? __cficu_ucal_getGregorianChange(cal, &status) : 0;
        CFAbsoluteTime at;
        if (cal && U_SUCCESS(status)) {
            at = udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970;
        } else {
            at = -13197600000.0; // Oct 15, 1582
            udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        }
        ICU_LOG("    UDate udate = cal ? ucal_getGregorianChange(cal, &status) : 0;\n");
        ICU_LOG("    CFAbsoluteTime gregorianStart = (cal && U_SUCCESS(status)) ? udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970 : -13197600000.0;\n");
        ICU_LOG("    if (!(cal && U_SUCCESS(status))) { udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0; }\n");
        calendar->_gregorianStart = CFDateCreate(CFGetAllocator(calendar), at);
        status = U_ZERO_ERROR;
        if (calendar->_cal) __cficu_ucal_setGregorianChange(calendar->_cal, udate, &status);
        if (cal) __cficu_ucal_close(cal);
        ICU_LOG("    status = U_ZERO_ERROR;\n");
        ICU_LOG("    if (cal) ucal_setGregorianChange(cal, udate, &status);\n");
        ICU_LOG("    if (cal) ucal_close(cal);\n");
        calendar->_userSet_gregorianStart = false;
    } else if (kCFCalendarIdentifierGregorian == calendar->_identifier) {
        calendar->_gregorianStart = (CFDateRef)CFRetain(date);
        CFAbsoluteTime at = CFDateGetAbsoluteTime(date);
        UDate udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        ICU_LOG("    CFAbsoluteTime at = %.06f;\n", at);
        ICU_LOG("    UDate udate = (at + kCFAbsoluteTimeIntervalSince1970) * 1000.0;\n");
        UErrorCode status = U_ZERO_ERROR;
        if (calendar->_cal) __cficu_ucal_setGregorianChange(calendar->_cal, udate, &status);
        ICU_LOG("    UErrorCode status = U_ZERO_ERROR;\n");
        ICU_LOG("    if (cal) ucal_setGregorianChange(cal, udate, &status);\n");
        calendar->_userSet_gregorianStart = true;
    }
    ICU_LOG("                // CFCalendarSetGregorianStartDate exit\n");
}

// #warning workaround for 5718919
#define UCAL_QUARTER 4444

#define UCAL_FIELD_ERROR 9999

static UCalendarDateFields __CFCalendarGetICUFieldCode(CFCalendarUnit unit) {
    switch (unit) {
    case kCFCalendarUnitEra: return UCAL_ERA;
    case kCFCalendarUnitYear: return UCAL_YEAR;
    case kCFCalendarUnitQuarter: return (UCalendarDateFields)UCAL_QUARTER;
    case kCFCalendarUnitMonth: return UCAL_MONTH;
    case kCFCalendarUnitDay: return UCAL_DAY_OF_MONTH;
    case kCFCalendarUnitHour: return UCAL_HOUR_OF_DAY;
    case kCFCalendarUnitMinute: return UCAL_MINUTE;
    case kCFCalendarUnitSecond: return UCAL_SECOND;
    case kCFCalendarUnitWeek_Deprecated: return UCAL_WEEK_OF_YEAR;
    case kCFCalendarUnitWeekOfYear: return UCAL_WEEK_OF_YEAR;
    case kCFCalendarUnitWeekOfMonth: return UCAL_WEEK_OF_MONTH;
    case kCFCalendarUnitYearForWeekOfYear: return UCAL_YEAR_WOY;
    case kCFCalendarUnitWeekday: return UCAL_DAY_OF_WEEK;
    case kCFCalendarUnitWeekdayOrdinal: return UCAL_DAY_OF_WEEK_IN_MONTH;
    }
    return (UCalendarDateFields)UCAL_FIELD_ERROR;
}

static CFCalendarUnit __CFCalendarUnitFromICUDateFields(UCalendarDateFields field) {
    switch (field) {
        case UCAL_ERA: return kCFCalendarUnitEra;
        case UCAL_YEAR: return kCFCalendarUnitYear;
        case UCAL_MONTH: return kCFCalendarUnitMonth;
        case UCAL_WEEK_OF_YEAR: return kCFCalendarUnitWeekOfYear;
        case UCAL_WEEK_OF_MONTH: return kCFCalendarUnitWeekOfMonth;
        case UCAL_DATE: return kCFCalendarUnitDay; // same value as UCAL_DAY_OF_MONTH
        case UCAL_DAY_OF_YEAR: return kCFCalendarUnitDay;
        case UCAL_DAY_OF_WEEK: return kCFCalendarUnitWeekday;
        case UCAL_DAY_OF_WEEK_IN_MONTH: return kCFCalendarUnitWeekdayOrdinal;
        case UCAL_HOUR: return kCFCalendarUnitHour;
        case UCAL_HOUR_OF_DAY: return kCFCalendarUnitHour;
        case UCAL_MINUTE: return kCFCalendarUnitMinute;
        case UCAL_SECOND: return kCFCalendarUnitSecond;
        case UCAL_ZONE_OFFSET: return kCFCalendarUnitTimeZone;
        case UCAL_YEAR_WOY: return kCFCalendarUnitYearForWeekOfYear;
        case UCAL_IS_LEAP_MONTH: return kCFCalendarUnitLeapMonth;
        case UCAL_EXTENDED_YEAR: return kCFCalendarUnitYear;
    }
    return 0;
}

static UCalendarDateFields __CFCalendarGetICUFieldCodeFromChar(char ch) {
    switch (ch) {
    case 'G': return UCAL_ERA;
    case 'y': return UCAL_YEAR;
    case 'U': return UCAL_YEAR;
    case 'r': return UCAL_YEAR;
    case 'M': return UCAL_MONTH;
    case 'L': return UCAL_MONTH;
    case 'l': return UCAL_IS_LEAP_MONTH;
    case 'd': return UCAL_DAY_OF_MONTH;
    case 'h': return UCAL_HOUR;
    case 'H': return UCAL_HOUR_OF_DAY;
    case 'm': return UCAL_MINUTE;
    case 's': return UCAL_SECOND;
    case 'S': return UCAL_MILLISECOND;
    case '^': return UCAL_WEEK_OF_YEAR;
    case 'w': return UCAL_WEEK_OF_YEAR;
    case 'W': return UCAL_WEEK_OF_MONTH;
    case 'Y': return UCAL_YEAR_WOY;
    case 'E': return UCAL_DAY_OF_WEEK;
    case 'c': return UCAL_DAY_OF_WEEK;
    case 'D': return UCAL_DAY_OF_YEAR;
    case 'F': return UCAL_DAY_OF_WEEK_IN_MONTH;
    case 'a': return UCAL_AM_PM;
    case 'g': return UCAL_JULIAN_DAY;
    case 'Q': return (UCalendarDateFields)UCAL_QUARTER;
    }
    return (UCalendarDateFields)UCAL_FIELD_ERROR;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static const char *__CFCalendarGetICUFieldCodeName(UCalendarDateFields field) {
    switch (field) {
    case UCAL_ERA: return "UCAL_ERA";
    case UCAL_YEAR: return "UCAL_YEAR";
    case UCAL_MONTH: return "UCAL_MONTH";
    case UCAL_IS_LEAP_MONTH: return "UCAL_IS_LEAP_MONTH";
    case UCAL_DAY_OF_MONTH: return "UCAL_DAY_OF_MONTH";
    case UCAL_HOUR: return "UCAL_HOUR";
    case UCAL_HOUR_OF_DAY: return "UCAL_HOUR_OF_DAY";
    case UCAL_MINUTE: return "UCAL_MINUTE";
    case UCAL_SECOND: return "UCAL_SECOND";
    case UCAL_MILLISECOND: return "UCAL_MILLISECOND";
    case UCAL_WEEK_OF_YEAR: return "UCAL_WEEK_OF_YEAR";
    case UCAL_WEEK_OF_MONTH: return "UCAL_WEEK_OF_MONTH";
    case UCAL_DAY_OF_WEEK: return "UCAL_DAY_OF_WEEK";
    case UCAL_DAY_OF_YEAR: return "UCAL_DAY_OF_YEAR";
    case UCAL_DAY_OF_WEEK_IN_MONTH: return "UCAL_DAY_OF_WEEK_IN_MONTH";
    case UCAL_AM_PM: return "UCAL_AM_PM";
    case UCAL_JULIAN_DAY: return "UCAL_JULIAN_DAY";
    case UCAL_ZONE_OFFSET: return "UCAL_ZONE_OFFSET";
    case UCAL_DST_OFFSET: return "UCAL_DST_OFFSET";
    case UCAL_YEAR_WOY: return "UCAL_YEAR_WOY";
    case UCAL_DOW_LOCAL: return "UCAL_DOW_LOCAL";
    case UCAL_EXTENDED_YEAR: return "UCAL_EXTENDED_YEAR";
    case UCAL_MILLISECONDS_IN_DAY: return "UCAL_MILLISECONDS_IN_DAY";
    case UCAL_FIELD_COUNT: return "???";
    }
    if (field == (UCalendarDateFields)UCAL_QUARTER) return "UCAL_QUARTER";
    return "???";
}

static const char *__CFCalendarGetUnitName(CFCalendarUnit unit) {
    switch (unit) {
    case kCFCalendarUnitEra: return "kCFCalendarUnitEra";
    case kCFCalendarUnitYear: return "kCFCalendarUnitYear";
    case kCFCalendarUnitQuarter: return "kCFCalendarUnitQuarter";
    case kCFCalendarUnitMonth: return "kCFCalendarUnitMonth";
    case kCFCalendarUnitDay: return "kCFCalendarUnitDay";
    case kCFCalendarUnitHour: return "kCFCalendarUnitHour";
    case kCFCalendarUnitMinute: return "kCFCalendarUnitMinute";
    case kCFCalendarUnitSecond: return "kCFCalendarUnitSecond";
    case kCFCalendarUnitWeek_Deprecated: return "kCFCalendarUnitWeek";
    case kCFCalendarUnitWeekOfYear: return "kCFCalendarUnitWeekOfYear";
    case kCFCalendarUnitWeekOfMonth: return "kCFCalendarUnitWeekOfMonth";
    case kCFCalendarUnitYearForWeekOfYear: return "kCFCalendarUnitYearForWeekOfYear";
    case kCFCalendarUnitWeekday: return "kCFCalendarUnitWeekday";
    case kCFCalendarUnitWeekdayOrdinal: return "kCFCalendarUnitWeekdayOrdinal";
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    case kCFCalendarUnitNanosecond: return "kCFCalendarUnitNanosecond";
#pragma GCC diagnostic pop
    }
    return "???";
}

#pragma GCC diagnostic pop

CFRange CFCalendarGetMinimumRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit unit) {
    ICU_LOG("                // CFCalendarGetMinimumRangeOfUnit enter (%s)\n", __CFCalendarGetUnitName(unit));
    // Note: We do not toll-free bridge for Swift
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFCalendar, CFRange, (NSCalendar *)calendar, _minimumRangeOfUnit:(NSCalendarUnit)unit);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    switch (unit) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    case kCFCalendarUnitCalendar: return CFRangeMake(kCFNotFound, kCFNotFound);
    case kCFCalendarUnitTimeZone: return CFRangeMake(kCFNotFound, kCFNotFound);
#pragma GCC diagnostic pop
    case kCFCalendarUnitEra: break;
    case kCFCalendarUnitYear: break;
    case kCFCalendarUnitQuarter: return CFRangeMake(1, 4);
    case kCFCalendarUnitMonth: break;
    case kCFCalendarUnitDay: break;
    case kCFCalendarUnitHour: return CFRangeMake(0, 24);
    case kCFCalendarUnitMinute: return CFRangeMake(0, 60);
    case kCFCalendarUnitSecond: return CFRangeMake(0, 60);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    case kCFCalendarUnitNanosecond: return CFRangeMake(0, 1000 * 1000 * 1000);
#pragma GCC diagnostic pop
    case kCFCalendarUnitWeek_Deprecated: break;
    case kCFCalendarUnitWeekOfYear: break;
    case kCFCalendarUnitWeekOfMonth: break;
    case kCFCalendarUnitYearForWeekOfYear: break;
    case kCFCalendarUnitWeekday: return CFRangeMake(1, 7);
    case kCFCalendarUnitWeekdayOrdinal: break;
    }
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
        ICU_LOG("    ucal_clear(cal);\n");
        __cficu_ucal_clear(calendar->_cal);
        UCalendarDateFields field = __CFCalendarGetICUFieldCode(unit);
        if ((UCalendarDateFields)UCAL_FIELD_ERROR == field) return CFRangeMake(kCFNotFound, kCFNotFound);
        UErrorCode status = U_ZERO_ERROR;
        ICU_LOG("    UErrorCode status = U_ZERO_ERROR;\n");
        CFRange range;
        range.location = __cficu_ucal_getLimit(calendar->_cal, field, UCAL_GREATEST_MINIMUM, &status);
        range.length = __cficu_ucal_getLimit(calendar->_cal, field, UCAL_LEAST_MAXIMUM, &status) - range.location + 1;
        ICU_LOG("    CFRange range;\n");
        ICU_LOG("    range.location = ucal_getLimit(cal, %s, UCAL_GREATEST_MINIMUM, &status);\n", __CFCalendarGetICUFieldCodeName(field));
        ICU_LOG("    range.length = ucal_getLimit(cal, %s, UCAL_LEAST_MAXIMUM, &status) - range.location + 1;\n", __CFCalendarGetICUFieldCodeName(field));
        if (kCFCalendarUnitMonth == unit) range.location++;
        if (kCFCalendarUnitMonth == unit) ICU_LOG("    range.location++; // month\n");
        if (U_SUCCESS(status)) ICU_LOG("                // CFCalendarGetMinimumRangeOfUnit exit {%ld, %ld}\n", range.location, range.length);
        if (U_SUCCESS(status)) return range;
    }
    ICU_LOG("                // CFCalendarGetMinimumRangeOfUnit exit {%ld, %ld}\n", kCFNotFound, kCFNotFound);
    return CFRangeMake(kCFNotFound, kCFNotFound);
}

CFRange CFCalendarGetMaximumRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit unit) {
    ICU_LOG("                // CFCalendarGetMaximumRangeOfUnit enter (%s)\n", __CFCalendarGetUnitName(unit));
    // Note: We do not toll-free bridge for Swift
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFRange, (NSCalendar *)calendar, _maximumRangeOfUnit:(NSCalendarUnit)unit);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    switch (unit) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    case kCFCalendarUnitCalendar: return CFRangeMake(kCFNotFound, kCFNotFound);
    case kCFCalendarUnitTimeZone: return CFRangeMake(kCFNotFound, kCFNotFound);
#pragma GCC diagnostic pop
    case kCFCalendarUnitEra: break;
    case kCFCalendarUnitYear: break;
    case kCFCalendarUnitQuarter: return CFRangeMake(1, 4);
    case kCFCalendarUnitMonth: break;
    case kCFCalendarUnitDay: break;
    case kCFCalendarUnitHour: return CFRangeMake(0, 24);
    case kCFCalendarUnitMinute: return CFRangeMake(0, 60);
    case kCFCalendarUnitSecond: return CFRangeMake(0, 60);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    case kCFCalendarUnitNanosecond: return CFRangeMake(0, 1000 * 1000 * 1000);
#pragma GCC diagnostic pop
    case kCFCalendarUnitWeek_Deprecated: break;
    case kCFCalendarUnitWeekOfYear: break;
    case kCFCalendarUnitWeekOfMonth: break;
    case kCFCalendarUnitYearForWeekOfYear: break;
    case kCFCalendarUnitWeekday: return CFRangeMake(1, 7);
    case kCFCalendarUnitWeekdayOrdinal: break;
    }
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
        ICU_LOG("    ucal_clear(cal);\n");
        __cficu_ucal_clear(calendar->_cal);
        UCalendarDateFields field = __CFCalendarGetICUFieldCode(unit);
        if ((UCalendarDateFields)UCAL_FIELD_ERROR == field) return CFRangeMake(kCFNotFound, kCFNotFound);
        UErrorCode status = U_ZERO_ERROR;
        ICU_LOG("    UErrorCode status = U_ZERO_ERROR;\n");
        CFRange range;
        range.location = __cficu_ucal_getLimit(calendar->_cal, field, UCAL_MINIMUM, &status);
        range.length = __cficu_ucal_getLimit(calendar->_cal, field, UCAL_MAXIMUM, &status) - range.location + 1;
        ICU_LOG("    CFRange range;\n");
        ICU_LOG("    range.location = ucal_getLimit(cal, %s, UCAL_MINIMUM, &status);\n", __CFCalendarGetICUFieldCodeName(field));
        ICU_LOG("    range.length = ucal_getLimit(cal, %s, UCAL_MAXIMUM, &status) - range.location + 1;\n", __CFCalendarGetICUFieldCodeName(field));
        if (kCFCalendarUnitMonth == unit) range.location++;
        if (kCFCalendarUnitMonth == unit) ICU_LOG("    range.location++; // month\n");
        if (U_SUCCESS(status)) ICU_LOG("                // CFCalendarGetMaximumRangeOfUnit exit {%ld, %ld}\n", range.location, range.length);
        if (U_SUCCESS(status)) return range;
    }
    ICU_LOG("                // CFCalendarGetMaximumRangeOfUnit exit {%ld, %ld}\n", kCFNotFound, kCFNotFound);
    return CFRangeMake(kCFNotFound, kCFNotFound);
}

Boolean _CFCalendarComposeAbsoluteTimeV(CFCalendarRef calendar, /* out */ CFAbsoluteTime *atp, const char *componentDesc, int32_t *vector, int32_t count) {
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
        UErrorCode status = U_ZERO_ERROR;
        __cficu_ucal_clear(calendar->_cal);
        __cficu_ucal_set(calendar->_cal, UCAL_YEAR, 1);
        __cficu_ucal_set(calendar->_cal, UCAL_MONTH, 0);
        __cficu_ucal_set(calendar->_cal, UCAL_IS_LEAP_MONTH, 0);
        __cficu_ucal_set(calendar->_cal, UCAL_DAY_OF_MONTH, 1);
        __cficu_ucal_set(calendar->_cal, UCAL_HOUR_OF_DAY, 0);
        __cficu_ucal_set(calendar->_cal, UCAL_MINUTE, 0);
        __cficu_ucal_set(calendar->_cal, UCAL_SECOND, 0);
        __cficu_ucal_set(calendar->_cal, UCAL_MILLISECOND, 0);
        const char *desc = componentDesc;
        Boolean seenMonth = false, seenDay = false, seenWeekOY = false, seenWeekday = false;
        Boolean seenOldWeek = false, seenYear = false, seenYearWOY = false;
        char ch = *desc;
        while (ch) {
            UCalendarDateFields field = __CFCalendarGetICUFieldCodeFromChar(ch);
            if (UCAL_YEAR == field) {
                seenYear = true;
            } else if (UCAL_YEAR_WOY == field) {
                seenYearWOY = true;
            }
            if (UCAL_WEEK_OF_YEAR == field) {
                if ('^' == ch) seenOldWeek = true; else seenWeekOY = true;
            } else if (UCAL_DAY_OF_WEEK == field) {
                seenWeekday = true;
            } else if (UCAL_MONTH == field) {
                seenMonth = true;
            } else if (UCAL_DAY_OF_MONTH == field) {
                seenDay = true;
            }
            desc++;
            ch = *desc;
        }
        int32_t nanosecond = 0;
        desc = componentDesc;
        ch = *desc;
        while (ch) {
            int32_t value = *vector;
            if ('#' == ch) {
                nanosecond = value;
            } else {
                // We support the old behavior of changing a UCAL_YEAR to UCAL_YEAR_WOY
                // behavior for compatibility IFF the developer has not also mixed in
                // either or both of the new WeekOfYear and YearForWeekOfYear values.
                // And of course, the old conditions were that Month and Day use had
                // precedence to keep the Year interpreted as the ordinary Year.
                // So, change to UCAL_YEAR_WOY if either of these is true:
                //   (!(D && M) && (WD && OW) && !(WOY || YWOY))
                //   (!(D || M) && (OW) && !(WOY || YWOY))
                // Note that the old possibility of just have a Weekday (plus Year)
                // present, and not a week number, trigger the change is no longer
                // supported (and unlikely to have been much exercised).
                UCalendarDateFields field = __CFCalendarGetICUFieldCodeFromChar(ch);
                if (UCAL_YEAR == field) {
                    if (!seenWeekOY && !seenYearWOY && seenOldWeek) {
                        if (!(seenDay && seenMonth) && (seenWeekday || (!seenDay && !seenMonth))) {
                            field = UCAL_YEAR_WOY;
                        }
                    }
                }
                if (UCAL_MONTH == field) value--;
// #warning support UCAL_QUARTER
                if ((UCalendarDateFields)UCAL_QUARTER != field && (UCalendarDateFields)UCAL_FIELD_ERROR != field) {
                    __cficu_ucal_set(calendar->_cal, field, value);
                }
            }
            vector++;
            desc++;
            ch = *desc;
        }
        UDate udate = __cficu_ucal_getMillis(calendar->_cal, &status);
        CFAbsoluteTime at = (udate / 1000.0) - kCFAbsoluteTimeIntervalSince1970 + nanosecond * 1.0e-9;
        CFAbsoluteTime t;
        CFTimeInterval length;
        if (__CFCalendarGetTimeRangeOfTimeZoneTransition(calendar, at, &t, &length)) {
            at = at - length;
        }
        if (atp) *atp = at;
        return U_SUCCESS(status) ? true : false;
    }
    return false;
}

Boolean _CFCalendarDecomposeAbsoluteTimeV(CFCalendarRef calendar, CFAbsoluteTime at, const char *componentDesc, int32_t **vector, int32_t count) {
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
        UErrorCode status = U_ZERO_ERROR;
        __cficu_ucal_clear(calendar->_cal);
        UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        __cficu_ucal_setMillis(calendar->_cal, udate, &status);
        char ch = *componentDesc;
        for (int32_t i = 0; i < count && ch != '\0'; i++) {
            if ('#' == ch) {
                *(*vector) = (int32_t)((at - floor(at)) * 1.0e+9);
            } else {
                UCalendarDateFields field = __CFCalendarGetICUFieldCodeFromChar(ch);
                int32_t value = 0;
// #warning support UCAL_QUARTER
                if ((UCalendarDateFields)UCAL_QUARTER != field && (UCalendarDateFields)UCAL_FIELD_ERROR != field) {
                    value = __cficu_ucal_get(calendar->_cal, field, &status);
                }
                if (UCAL_MONTH == field) value++;
                *(*vector) = value;
            }
            vector++;
            componentDesc++;
            ch = *componentDesc;
        }
        return U_SUCCESS(status) ? true : false;
    }
    return false;
}

Boolean _CFCalendarAddComponentsV(CFCalendarRef calendar, /* inout */ CFAbsoluteTime *atp, CFOptionFlags options, const char *componentDesc, int32_t *vector, int32_t count) {
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
        UErrorCode status = U_ZERO_ERROR;
        __cficu_ucal_clear(calendar->_cal);
        double startingInt;
        double startingFrac = modf(*atp, &startingInt);
        if (startingFrac < 0) {
            // `modf` returns negative integral and fractional parts when `*atp` is negative. In this case, we would wrongly turn the time backwards by adding the negative fractional part back after we're done with wrapping in `__CFCalendarAdd` below. To avoid this, ensure that `startingFrac` is always positive: subseconds do not contribute to the wrapping of a second, so they should always be additive to the time ahead.
            startingFrac += 1.0;
            startingInt -= 1.0;
        }
        UDate udate = (startingInt + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        __cficu_ucal_setMillis(calendar->_cal, udate, &status);
        int32_t nanosecond = 0;
        char ch = *componentDesc;
        for (int32_t i = 0; i < count && ch != '\0'; i++) {
            int32_t amount = *vector;
            if ('#' == ch) {
                nanosecond = amount;
            } else {
                UCalendarDateFields field = __CFCalendarGetICUFieldCodeFromChar(ch);
// #warning support UCAL_QUARTER
                if ((UCalendarDateFields)UCAL_QUARTER != field && (UCalendarDateFields)UCAL_FIELD_ERROR != field) {
                    __CFCalendarAdd(calendar, field, amount, options, &status);
                }
            }
            vector++;
            componentDesc++;
            ch = *componentDesc;
        }
        udate = __cficu_ucal_getMillis(calendar->_cal, &status);
        *atp = (udate / 1000.0) - kCFAbsoluteTimeIntervalSince1970 + startingFrac + (nanosecond * 1.0e-9);
        return U_SUCCESS(status) ? true : false;
    }
    return false;
}

#pragma mark -

Boolean _CFCalendarGetComponentDifferenceV(CFCalendarRef calendar, CFAbsoluteTime startingAT, CFAbsoluteTime resultAT, CFOptionFlags options, const char *componentDesc, int32_t **vector, int32_t count) {
    if (!calendar->_cal) __CFCalendarSetupCal(calendar);
    if (calendar->_cal) {
        UErrorCode status = U_ZERO_ERROR;
        __cficu_ucal_clear(calendar->_cal);
        UDate curr = (startingAT + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        UDate currX = floor(curr);
        UDate diff = curr - currX;
        curr = currX;
        UDate goal = (resultAT + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        goal -= diff;
        __cficu_ucal_setMillis(calendar->_cal, curr, &status);
        char ch = *componentDesc;
        for (int32_t i = 0; i < count && ch != '\0'; i++) {
            int32_t result = 0;
            if ('#' == ch) {
                UDate curr0 = __cficu_ucal_getMillis(calendar->_cal, &status);
                double tmp = floor((goal - curr0) * 1.0e+6);
                if (tmp < (double)INT32_MAX) {
                    result = (int32_t)tmp;
                } else {
                    result = INT32_MAX;
                }
                __cficu_ucal_setMillis(calendar->_cal, goal, &status);
            } else {
                UCalendarDateFields field = __CFCalendarGetICUFieldCodeFromChar(ch);
                if (UCAL_ERA == field) {
                    // ICU refuses to do the subtraction, probably because we are
                    // at the limit of UCAL_ERA.  Use alternate strategy.
                    curr = __cficu_ucal_getMillis(calendar->_cal, &status);
                    int32_t currEra = __cficu_ucal_get(calendar->_cal, UCAL_ERA, &status);
                    __cficu_ucal_setMillis(calendar->_cal, goal, &status);
                    int32_t goalEra = __cficu_ucal_get(calendar->_cal, UCAL_ERA, &status);
                    __cficu_ucal_setMillis(calendar->_cal, curr, &status);
                    __cficu_ucal_set(calendar->_cal, UCAL_ERA, goalEra);
                    result = goalEra - currEra;
                } else if ((UCalendarDateFields)UCAL_QUARTER == field) {
// #warning support UCAL_QUARTER
                } else if ((UCalendarDateFields)UCAL_FIELD_ERROR != field) {
                    result = __cficu_ucal_getFieldDifference(calendar->_cal, goal, field, &status);
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
    // Note: We do not toll-free bridge for Swift
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), Boolean, (NSCalendar *)calendar, _composeAbsoluteTime:atp :(const unsigned char *)componentDesc :args);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    int32_t idx, cnt = strlen((char *)componentDesc);
    STACK_BUFFER_DECL(int32_t, vector, cnt);
    for (idx = 0; idx < cnt; idx++) {
        int32_t arg = va_arg(args, int32_t);
        vector[idx] = arg;
    }
    va_end(args);
    return _CFCalendarComposeAbsoluteTimeV(calendar, atp, componentDesc, vector, cnt);
}

Boolean CFCalendarDecomposeAbsoluteTime(CFCalendarRef calendar, CFAbsoluteTime at, const char *componentDesc, ...) {
    __CFCalendarValidateAndCapTimeRange(at);
    va_list args;
    va_start(args, componentDesc);
    // Note: We do not toll-free bridge for Swift
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), Boolean, (NSCalendar *)calendar, _decomposeAbsoluteTime:at :(const unsigned char *)componentDesc :args);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    int32_t idx, cnt = strlen((char *)componentDesc);
    STACK_BUFFER_DECL(int32_t *, vector, cnt);
    for (idx = 0; idx < cnt; idx++) {
        int32_t *arg = va_arg(args, int32_t *);
        vector[idx] = arg;
    }
    va_end(args);
    return _CFCalendarDecomposeAbsoluteTimeV(calendar, at, componentDesc, vector, cnt);
}

Boolean CFCalendarAddComponents(CFCalendarRef calendar, /* inout */ CFAbsoluteTime *atp, CFOptionFlags options, const char *componentDesc, ...) {
    __CFCalendarValidateAndCapTimeRange(*atp);
    va_list args;
    va_start(args, componentDesc);
    // Note: We do not toll-free bridge for Swift
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), Boolean, (NSCalendar *)calendar, _addComponents:atp :options :(const unsigned char *)componentDesc :args);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    int32_t idx, cnt = strlen((char *)componentDesc);
    STACK_BUFFER_DECL(int32_t, vector, cnt);
    for (idx = 0; idx < cnt; idx++) {
        int32_t arg = va_arg(args, int32_t);
        vector[idx] = arg;
    }
    va_end(args);
    return _CFCalendarAddComponentsV(calendar, atp, options, componentDesc, vector, cnt);
}

CF_PRIVATE CFDateRef _CFCalendarCreateDateByAddingValueOfUnitToDate(CFCalendarRef calendar, CFIndex val, CFCalendarUnit unit, CFDateRef date) {
    const char *units;
    // TODO: Combine with other switch on single unit, use a table
    switch (unit) {
        case kCFCalendarUnitEra: units = "G"; break;
        case kCFCalendarUnitYear: units = "y"; break;
        case kCFCalendarUnitQuarter: units = "Q"; break;
        case kCFCalendarUnitMonth: units = "M"; break;
        case kCFCalendarUnitDay: units = "d"; break;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        case kCFCalendarUnitWeek: units = "^"; break; // extra thing understood by private SPI below
#pragma GCC diagnostic pop
        case kCFCalendarUnitWeekOfYear: units = "w"; break;
        case kCFCalendarUnitWeekOfMonth: units = "W"; break;
        case kCFCalendarUnitYearForWeekOfYear: units = "Y"; break;
        case kCFCalendarUnitWeekday: units = "E"; break;
        case kCFCalendarUnitWeekdayOrdinal: units = "F"; break;
        case kCFCalendarUnitHour: units = "H"; break;
        case kCFCalendarUnitMinute: units = "m"; break;
        case kCFCalendarUnitSecond: units = "s"; break;
        case kCFCalendarUnitNanosecond: units = "#"; break;
        default:
            return NULL;
    }
    
    CFAbsoluteTime at = CFDateGetAbsoluteTime(date);
    Boolean result = CFCalendarAddComponents(calendar, &at, 0, units, val);
    if (result) {
        return CFDateCreate(kCFAllocatorSystemDefault, at);
    } else {
        return NULL;
    }
}

Boolean CFCalendarGetComponentDifference(CFCalendarRef calendar, CFAbsoluteTime startingAT, CFAbsoluteTime resultAT, CFOptionFlags options, const char *componentDesc, ...) {
    __CFCalendarValidateAndCapTimeRange(startingAT);
    __CFCalendarValidateAndCapTimeRange(resultAT);

    va_list args;
    va_start(args, componentDesc);
    // Note: We do not toll-free bridge for Swift
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), Boolean, (NSCalendar *)calendar, _diffComponents:startingAT :resultAT :options :(const unsigned char *)componentDesc :args);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    int32_t idx, cnt = strlen((char *)componentDesc);
    STACK_BUFFER_DECL(int32_t *, vector, cnt);
    for (idx = 0; idx < cnt; idx++) {
        int32_t *arg = va_arg(args, int32_t *);
        vector[idx] = arg;
    }
    va_end(args);
    Boolean ret = _CFCalendarGetComponentDifferenceV(calendar, startingAT, resultAT, options, componentDesc, vector, cnt);
    return ret;
}

Boolean CFCalendarGetTimeRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit unit, CFAbsoluteTime at, CFAbsoluteTime *startp, CFTimeInterval *tip) {
    __CFCalendarValidateAndCapTimeRange(at);
    // Note: We do not toll-free bridge for Swift
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), Boolean, (NSCalendar *)calendar, _rangeOfUnit:(NSCalendarUnit)unit startTime:startp interval:tip forAT:at);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());

    const CFTimeInterval inf_ti = 4398046511104.0;
    CFStringRef ident = CFCalendarGetIdentifier(calendar);
    switch (unit) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    case kCFCalendarUnitCalendar: return false;
    case kCFCalendarUnitTimeZone: return false;
#pragma GCC diagnostic pop
    case kCFCalendarUnitEra:;
        if (kCFCalendarIdentifierGregorian == ident || kCFCalendarIdentifierISO8601 == ident) {
            if (at < -63113904000.0) {
                if (startp) *startp = -63113904000.0 - inf_ti;
            } else {
                if (startp) *startp = -63113904000.0;
            }
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierRepublicOfChina == ident) {
            if (at < -2808691200.0) {
                if (startp) *startp = -2808691200.0 - inf_ti;
            } else {
                if (startp) *startp = -2808691200.0;
            }
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierCoptic == ident) {
            if (at < -54162518400.0) {
                if (startp) *startp = -54162518400.0 - inf_ti;
            } else {
                if (startp) *startp = -54162518400.0;
            }
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierBuddhist == ident) {
            if (at < -80249875200.0) {
                return false;
            }
            if (startp) *startp = -80249875200.0;
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierIslamic == ident) {
            if (at < -43499980800.0) {
                return false;
            }
            if (startp) *startp = -43499980800.0;
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierIslamicCivil == ident) {
            if (at < -43499894400.0) {
                return false;
            }
            if (startp) *startp = -43499894400.0;
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierIslamicTabular == ident) {
            if (at < -43499980800.0) {
                return false;
            }
            if (startp) *startp = -43499980800.0;
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierIslamicUmmAlQura == ident) {
            if (at < -43499980800.0) {
                return false;
            }
            if (startp) *startp = -43499980800.0;
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierHebrew == ident) {
            if (at < -181778083200.0) {
                return false;
            }
            if (startp) *startp = -181778083200.0;
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierPersian == ident) {
            if (at < -43510176000.0) {
                return false;
            }
            if (startp) *startp = -43510176000.0;
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierIndian == ident) {
            if (at < -60645542400.0) {
                return false;
            }
            if (startp) *startp = -60645542400.0;
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierEthiopicAmeteAlem == ident) {
            if (at < -236439216000.0) {
                return false;
            }
            if (startp) *startp = -236439216000.0;
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierEthiopicAmeteMihret == ident) {
            if (at < -236439216000.0) {
                return false;
            }
            if (at < -62872416000.0) {
                if (startp) *startp = -236439216000.0;
                if (tip) *tip = -62872416000.0 - -236439216000.0;
                return true;
            }
            if (startp) *startp = -62872416000.0;
            if (tip) *tip = inf_ti;
            return true;
        } else if (kCFCalendarIdentifierJapanese == ident) {
            if (at < -42790982400.0) {
                return false;
            }
            break;
        } else if (kCFCalendarIdentifierChinese == ident) {
            if (at < -146325744000.0) {
                return false;
            }
            break;
        }
        break;
    case kCFCalendarUnitYear: break;
    case kCFCalendarUnitYearForWeekOfYear: break;
    case kCFCalendarUnitQuarter: break;
    case kCFCalendarUnitMonth: break;
    case kCFCalendarUnitDay: break;
    case kCFCalendarUnitHour:
        {
            CFTimeZoneRef timezone = CFCalendarCopyTimeZone(calendar);
            CFTimeInterval ti = CFTimeZoneGetSecondsFromGMT(timezone, at);
            CFRelease(timezone);
#if 1
            CFTimeInterval fixedAT = at + ti; // compute local time
            fixedAT = floor(fixedAT / 3600.0) * 3600.0;
            fixedAT -= ti; // compute GMT
            if (startp) *startp = fixedAT;
#else
            NSTimeInterval secondsToSubtract = fmod(fmod(at + ti, 3600.0) + 3600.0, 3600.0); // amount of seconds into the current local time's hour
            if (startp) *startp = at - secondsToSubtract;
#endif
            if (tip) *tip = 3600.0;
            return true;
        }
    case kCFCalendarUnitMinute:
        if (startp) *startp = floor(at / 60.0) * 60.0;
        if (tip) *tip = 60.0;
        return true;
    case kCFCalendarUnitSecond:
        if (startp) *startp = floor(at);
        if (tip) *tip = 1.0;
        return true;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    case kCFCalendarUnitNanosecond:
        if (startp) *startp = floor(at * 1.0e+9) * 1.0e-9;
        if (tip) *tip = 1.0e-9;
        return true;
#pragma GCC diagnostic pop
    case kCFCalendarUnitWeek_Deprecated: break;
    case kCFCalendarUnitWeekOfMonth: break;
    case kCFCalendarUnitWeekOfYear: break;
    case kCFCalendarUnitWeekdayOrdinal:
    case kCFCalendarUnitWeekday:
        unit = kCFCalendarUnitDay; break;
    }

    if (!calendar->_cal) {
        __CFCalendarSetupCal(calendar);
        if (!calendar->_cal) {
            return false;
        }
    }

    // Set UCalendar to first instant of unit prior to 'at'
    __CFCalendarSetToFirstInstant(calendar, unit, at);

    UErrorCode status = U_ZERO_ERROR;
    UDate end = 0.0, start = __cficu_ucal_getMillis(calendar->_cal, &status);
    CFAbsoluteTime start_at = start / 1000.0 - kCFAbsoluteTimeIntervalSince1970;

    if (tip) {
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (unit) {
        case kCFCalendarUnitEra: {
            __cficu_ucal_add(calendar->_cal, UCAL_ERA, 1, &status);
            UDate newdate = __cficu_ucal_getMillis(calendar->_cal, &status);
            if (newdate == start) {
                // ICU refused to do the addition, probably because we are
                // at the limit of UCAL_ERA.
                if (startp) *startp = start_at;
                if (tip) *tip = inf_ti;
                return true;
            }
            break;
        }
        case kCFCalendarUnitYear:
            __cficu_ucal_add(calendar->_cal, UCAL_YEAR, 1, &status);
            break;
        case kCFCalendarUnitYearForWeekOfYear:
            __cficu_ucal_add(calendar->_cal, UCAL_YEAR_WOY, 1, &status);
            break;
        case kCFCalendarUnitQuarter: {
            // #warning adding 3 months and tacking any 13th month in the last quarter is not right for Hebrew
            __cficu_ucal_add(calendar->_cal, UCAL_MONTH, 3, &status);
            int32_t m = __cficu_ucal_get(calendar->_cal, UCAL_MONTH, &status);
            if (12 == m) { // for calendars with 13 months
                __cficu_ucal_add(calendar->_cal, UCAL_MONTH, 1, &status);
                // workaround ICU bug with Coptic, Ethiopic calendars
                int32_t d = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_MONTH, &status);
                int32_t d1 = __cficu_ucal_getLimit(calendar->_cal, UCAL_DAY_OF_MONTH, UCAL_ACTUAL_MINIMUM, &status);
                if (d != d1) {
                    __cficu_ucal_set(calendar->_cal, UCAL_DAY_OF_MONTH, d1);
                }
            }
            break;
        }
        case kCFCalendarUnitMonth:
            __cficu_ucal_add(calendar->_cal, UCAL_MONTH, 1, &status);
            break;
        case kCFCalendarUnitWeek_Deprecated:
            __cficu_ucal_add(calendar->_cal, UCAL_WEEK_OF_YEAR, 1, &status);
            break;
        case kCFCalendarUnitWeekOfYear:
            __cficu_ucal_add(calendar->_cal, UCAL_WEEK_OF_YEAR, 1, &status);
            break;
        case kCFCalendarUnitWeekOfMonth:
            __cficu_ucal_add(calendar->_cal, UCAL_WEEK_OF_MONTH, 1, &status);
            break;
        case kCFCalendarUnitDay:
            __cficu_ucal_add(calendar->_cal, UCAL_DAY_OF_MONTH, 1, &status);
            break;
        }
#pragma GCC diagnostic pop // See 10693376

        // move back to 0h0m0s, in case the start of the unit wasn't at 0h0m0s
        __cficu_ucal_set(calendar->_cal, UCAL_HOUR_OF_DAY, __cficu_ucal_getLimit(calendar->_cal, UCAL_HOUR_OF_DAY, UCAL_ACTUAL_MINIMUM, &status));
        __cficu_ucal_set(calendar->_cal, UCAL_MINUTE, __cficu_ucal_getLimit(calendar->_cal, UCAL_MINUTE, UCAL_ACTUAL_MINIMUM, &status));
        __cficu_ucal_set(calendar->_cal, UCAL_SECOND, __cficu_ucal_getLimit(calendar->_cal, UCAL_SECOND, UCAL_ACTUAL_MINIMUM, &status));
        __cficu_ucal_set(calendar->_cal, UCAL_MILLISECOND, 0);

        status = U_ZERO_ERROR;
        end = __cficu_ucal_getMillis(calendar->_cal, &status);
    }
    
    CFAbsoluteTime t, end_at;
    CFTimeInterval length;
    end_at = end / 1000.0 - kCFAbsoluteTimeIntervalSince1970;
    if (__CFCalendarGetTimeRangeOfTimeZoneTransition(calendar, end_at, &t, &length)) {
        end = (end_at - length + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
    }
    
    if (startp) *startp = start_at;
    if (tip) *tip = (end - start) / 1000.0;
    return true;
}

CF_PRIVATE Boolean _CFCalendarGetTimeRangeOfUnitForDate(CFCalendarRef calendar, CFCalendarUnit unit, CFDateRef *startDate, CFTimeInterval *tip, CFDateRef date) {
    assert(date != NULL);
    CFAbsoluteTime start = 0.0;
    CFTimeInterval ti = 0.0;
    Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, unit, CFDateGetAbsoluteTime(date), &start, &ti);
    if (b) {
        if (startDate) *startDate = CFDateCreate(kCFAllocatorSystemDefault, start);
        if (tip) *tip = ti;
        return true;
    }
    return false;
}

CF_PRIVATE CFDateRef _CFCalendarCreateStartDateForTimeRangeOfUnitForDate(CFCalendarRef calendar, CFCalendarUnit unit, CFDateRef date, CFTimeInterval * _Nullable tip) {
    assert(date != NULL);
    CFAbsoluteTime start = 0.0;
    CFTimeInterval ti = 0.0;
    Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, unit, CFDateGetAbsoluteTime(date), &start, &ti);
    if (b) {
        if (tip) *tip = ti;
        return CFDateCreate(kCFAllocatorSystemDefault, start);
    }
    return NULL;
}

CF_PRIVATE CFDateIntervalRef _CFCalendarCreateDateInterval(CFAllocatorRef allocator, CFCalendarRef calendar, CFCalendarUnit unit, CFDateRef date) {
    assert(date != NULL);
    CFAbsoluteTime start = 0.0;
    CFTimeInterval ti = 0.0;
    if (CFCalendarGetTimeRangeOfUnit(calendar, unit, CFDateGetAbsoluteTime(date), &start, &ti)) {
        CFDateRef startDate = CFDateCreate(kCFAllocatorSystemDefault, start);
        CFDateIntervalRef result = CFDateIntervalCreate(allocator, startDate, ti);
        CFRelease(startDate);
        return result;
    }
    return NULL;
}

// The recursion in this function assumes the order of the unit is dependent upon the order of the higher unit before it.  For example, the ordinality of the week of the month is dependent upon the ordinality of the month in which it lies, and that month is dependent upon the ordinality of the year in which it lies, etc.
static CFIndex __CFCalendarGetOrdinalityOfUnit3(CFCalendarRef calendar, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime at) {
    if (!calendar->_cal) {
        __CFCalendarSetupCal(calendar);
        if (!calendar->_cal) {
            return kCFNotFound;
        }
    }
    switch (biggerUnit) {
    case kCFCalendarUnitEra:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
        case kCFCalendarUnitYear: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex year = __cficu_ucal_get(calendar->_cal, UCAL_YEAR, &status);
            return year;
        }
        case kCFCalendarUnitYearForWeekOfYear: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex year = __cficu_ucal_get(calendar->_cal, UCAL_YEAR_WOY, &status);
            return year;
        }
        case kCFCalendarUnitQuarter: {
            CFIndex year = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitYear, kCFCalendarUnitEra, at);
            if (kCFNotFound == year) return kCFNotFound;
            CFIndex q = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitQuarter, kCFCalendarUnitYear, at);
            if (kCFNotFound == q) return kCFNotFound;
            CFIndex quarter = 4 * (year - 1) + q;
            return quarter;
        }
        case kCFCalendarUnitMonth: { // do not use this combo for recursion
            CFAbsoluteTime start = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitEra, at, &start, NULL);
            if (!b) return kCFNotFound;

            UErrorCode status = U_ZERO_ERROR;
            UDate at_udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            UDate start_udate = (floor(start) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            UDate test_udate;

            CFIndex month = 0;
            CFRange r = CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitDay);
            if (r.location != kCFNotFound && r.length != kCFNotFound) {
                month = (CFIndex)floor(((at - start) / 86400.0 / (r.length - r.location + 1)) * 0.96875); // low-ball the estimate
                month = (10 < month) ? month - 10 : 0; // low-ball estimate further
            }
            do {
                month++;
                status = U_ZERO_ERROR;
                __cficu_ucal_clear(calendar->_cal);
                __cficu_ucal_setMillis(calendar->_cal, start_udate, &status);
                test_udate = __CFCalendarAdd(calendar, UCAL_MONTH, month, 0, &status);
            } while (test_udate <= at_udate);
            return month;
        }
        case kCFCalendarUnitWeekOfYear: // do not use this combo for recursion
        case kCFCalendarUnitWeekOfMonth: // do not use this combo for recursion
        case kCFCalendarUnitWeek_Deprecated: { // do not use this combo for recursion
            CFAbsoluteTime start = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitEra, at, &start, NULL);
            if (!b) return kCFNotFound;

            UErrorCode status = U_ZERO_ERROR;
            UDate at_udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            UDate start_udate = (floor(start) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            UDate test_udate;
            __cficu_ucal_clear(calendar->_cal);
            __cficu_ucal_setMillis(calendar->_cal, start_udate, &status);
            // move start forward to first day of week if not already there
            CFIndex days_added = 0;
            while (__cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status) != calendar->_firstWeekday) {
                __CFCalendarAdd(calendar, UCAL_DAY_OF_MONTH, 1, 0, &status);
                days_added++;
            }
            start_udate += days_added * 86400.0 * 1000.0;
            if (calendar->_minDaysInFirstWeek <= days_added) {
                start_udate -= 7 * 86400.0 * 1000.0; // previous week chunk was big enough, count it
            }

            CFIndex week = (CFIndex)floor((at - start) / 86400.0 / 7.0);
            week = (10 < week) ? week - 10 : 0; // low-ball estimate
            do {
                week++;
                status = U_ZERO_ERROR;
                __cficu_ucal_clear(calendar->_cal);
                __cficu_ucal_setMillis(calendar->_cal, start_udate, &status);
                test_udate = __CFCalendarAdd(calendar, UCAL_WEEK_OF_YEAR, week, 0, &status);
            } while (test_udate <= at_udate);
            return week;
        }
        case kCFCalendarUnitWeekdayOrdinal:
        case kCFCalendarUnitWeekday: { // do not use this combo for recursion
            CFAbsoluteTime start = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitEra, at, &start, NULL);
            if (!b) return kCFNotFound;

            UErrorCode status = U_ZERO_ERROR;
            UDate at_udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            UDate start_udate = (floor(start) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            UDate test_udate;
            __cficu_ucal_clear(calendar->_cal);
            __cficu_ucal_setMillis(calendar->_cal, at_udate, &status);
            CFIndex target_dow = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status);
            __cficu_ucal_clear(calendar->_cal);
            __cficu_ucal_setMillis(calendar->_cal, start_udate, &status);
            // move start forward to target day of week if not already there
            while (__cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status) != target_dow) {
                __CFCalendarAdd(calendar, UCAL_DAY_OF_MONTH, 1, 0, &status);
                start_udate += 86400.0 * 1000.0;
            }

            CFIndex nth_weekday = (CFIndex)floor((at - start) / 86400.0 / 7.0);
            nth_weekday = (10 < nth_weekday) ? nth_weekday - 10 : 0; // low-ball estimate
            do {
                nth_weekday++;
                status = U_ZERO_ERROR;
                __cficu_ucal_clear(calendar->_cal);
                __cficu_ucal_setMillis(calendar->_cal, start_udate, &status);
                test_udate = __CFCalendarAdd(calendar, UCAL_WEEK_OF_YEAR, nth_weekday, 0, &status);
            } while (test_udate <= at_udate);
            return nth_weekday;
        }
        case kCFCalendarUnitDay: {
            CFAbsoluteTime start = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitEra, at, &start, NULL);
            // must do this to make sure things are set up for recursive calls to __CFCalendarGetOrdinalityOfUnit3
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            if (!b) return kCFNotFound;
            CFIndex day = (CFIndex)floor((at - start) / 86400.0) + 1;
            return day;
        }
        case kCFCalendarUnitHour: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex day = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitDay, kCFCalendarUnitEra, at);
            if (kCFNotFound == day) return kCFNotFound;
            if ((LONG_MAX - 24) / 24 < (day - 1)) return kCFNotFound;
            CFIndex hour = (day - 1) * 24 + __cficu_ucal_get(calendar->_cal, UCAL_HOUR_OF_DAY, &status) + 1;
            return hour;
        }
        case kCFCalendarUnitMinute: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex hour = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitHour, kCFCalendarUnitEra, at);
            if (kCFNotFound == hour) return kCFNotFound;
            if ((LONG_MAX - 60) / 60 < (hour - 1)) return kCFNotFound;
            CFIndex minute = (hour - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_MINUTE, &status) + 1;
            return minute;
        }
        case kCFCalendarUnitSecond: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex minute = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitMinute, kCFCalendarUnitEra, at);
            if (kCFNotFound == minute) return kCFNotFound;
            if ((LONG_MAX - 60) / 60 < (minute - 1)) return kCFNotFound;
            CFIndex second = (minute - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_SECOND, &status) + 1;
            return second;
        }
        }
#pragma GCC diagnostic pop // See 10693376
         break;
    case kCFCalendarUnitYear:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
        case kCFCalendarUnitQuarter: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex quarter = __cficu_ucal_get(calendar->_cal, UCAL_MONTH, &status);
            CFStringRef ident = CFCalendarGetIdentifier(calendar);
            if (kCFCalendarIdentifierHebrew == ident) {
                CFIndex mquarter[] = {3, 3, 3, 4, 4, 4, 4, 1, 1, 1, 2, 2, 2};
                quarter = mquarter[quarter];
            } else {
                CFIndex mquarter[] = {1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4};
                quarter = mquarter[quarter];
            }
            return quarter;
        }
        case kCFCalendarUnitMonth: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex month = __cficu_ucal_get(calendar->_cal, UCAL_MONTH, &status) + 1;
            return month;
        }
        case kCFCalendarUnitWeekOfMonth:
            return kCFNotFound;
        case kCFCalendarUnitWeekOfYear:
        case kCFCalendarUnitWeek_Deprecated: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex doy = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_YEAR, &status);
            __cficu_ucal_set(calendar->_cal, UCAL_DAY_OF_YEAR, 1);
            CFIndex fd_dow = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status);
            CFIndex week = (doy + 7 - calendar->_minDaysInFirstWeek + (fd_dow + calendar->_minDaysInFirstWeek - calendar->_firstWeekday + 6) % 7) / 7;
            status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            return week;
        }
        case kCFCalendarUnitWeekdayOrdinal:
        case kCFCalendarUnitWeekday: { // do not use this combo for recursion
            CFAbsoluteTime start = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitYear, at, &start, NULL);
            if (!b) return kCFNotFound;

            UErrorCode status = U_ZERO_ERROR;
            CFIndex at_week = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitWeek_Deprecated, kCFCalendarUnitYear, at);
            CFIndex target_dow = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status);
            if (kCFNotFound == at_week) return kCFNotFound;

            status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(start) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            // move start forward to target day of week if not already there
            while (__cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status) != target_dow) {
                udate = __CFCalendarAdd(calendar, UCAL_DAY_OF_MONTH, 1, 0, &status);
            }
            start = udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970;

            CFIndex start_week = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitWeek_Deprecated, kCFCalendarUnitYear, start);
            if (kCFNotFound == start_week) return kCFNotFound;
            CFIndex nth_weekday = at_week - start_week + 1;
            return nth_weekday;
        }
        case kCFCalendarUnitDay: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex day = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_YEAR, &status);
            return day;
        }
        case kCFCalendarUnitHour: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex day = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitDay, kCFCalendarUnitYear, at);
            if (kCFNotFound == day) return kCFNotFound;
            CFIndex hour = (day - 1) * 24 + __cficu_ucal_get(calendar->_cal, UCAL_HOUR_OF_DAY, &status) + 1;
            return hour;
        }
        case kCFCalendarUnitMinute: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex hour = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitHour, kCFCalendarUnitYear, at);
            if (kCFNotFound == hour) return kCFNotFound;
            CFIndex minute = (hour - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_MINUTE, &status) + 1;
            return minute;
        }
        case kCFCalendarUnitSecond: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex minute = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitMinute, kCFCalendarUnitYear, at);
            if (kCFNotFound == minute) return kCFNotFound;
            CFIndex second = (minute - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_SECOND, &status) + 1;
            return second;
        }
#if _CF_CALENDAR_NANOSECONDS_AVAILABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
        case kCFCalendarUnitNanosecond: {
            CFIndex second = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitSecond, kCFCalendarUnitYear, at);
            if (kCFNotFound == second) return kCFNotFound;
            double dseconds = (double)(second - 1) + (at - floor(at));
            return (CFIndex)(dseconds * 1.0e+9) + 1;
        }
#pragma GCC diagnostic pop
#endif
        }
#pragma GCC diagnostic pop // See 10693376
         break;
    case kCFCalendarUnitYearForWeekOfYear:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
        case kCFCalendarUnitQuarter:
            return kCFNotFound;
        case kCFCalendarUnitMonth:
            return kCFNotFound;
        case kCFCalendarUnitWeekOfMonth:
        case kCFCalendarUnitWeek_Deprecated:
            return kCFNotFound;
        case kCFCalendarUnitWeekOfYear: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex week = __cficu_ucal_get(calendar->_cal, UCAL_WEEK_OF_YEAR, &status);
            return U_SUCCESS(status) ? week : kCFNotFound;
        }
        case kCFCalendarUnitWeekdayOrdinal:
        case kCFCalendarUnitWeekday: { // do not use this combo for recursion
            CFAbsoluteTime start = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitYearForWeekOfYear, at, &start, NULL);
            if (!b) return kCFNotFound;

            UErrorCode status = U_ZERO_ERROR;
            CFIndex at_week = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitWeekOfYear, kCFCalendarUnitYearForWeekOfYear, at);
            CFIndex target_dow = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status);
            if (kCFNotFound == at_week) return kCFNotFound;

            status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(start) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            // move start forward to target day of week if not already there
            while (__cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status) != target_dow) {
                udate = __CFCalendarAdd(calendar, UCAL_DAY_OF_MONTH, 1, 0, &status);
            }
            start = udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970;

            CFIndex start_week = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitWeekOfYear, kCFCalendarUnitYearForWeekOfYear, start);
            if (kCFNotFound == start_week) return kCFNotFound;
            CFIndex nth_weekday = at_week - start_week + 1;
            return nth_weekday;
        }
        case kCFCalendarUnitDay: {
            CFAbsoluteTime start = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitYearForWeekOfYear, at, &start, NULL);
            if (!b) return kCFNotFound;
            CFIndex day = (CFIndex)floor((at - start) / 86400.0) + 1;
            return day;
        }
        case kCFCalendarUnitHour: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex day = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitDay, kCFCalendarUnitYearForWeekOfYear, at);
            if (kCFNotFound == day) return kCFNotFound;
            CFIndex hour = (day - 1) * 24 + __cficu_ucal_get(calendar->_cal, UCAL_HOUR_OF_DAY, &status) + 1;
            return hour;
        }
        case kCFCalendarUnitMinute: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex hour = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitHour, kCFCalendarUnitYearForWeekOfYear, at);
            if (kCFNotFound == hour) return kCFNotFound;
            CFIndex minute = (hour - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_MINUTE, &status) + 1;
            return minute;
        }
        case kCFCalendarUnitSecond: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex minute = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitMinute, kCFCalendarUnitYearForWeekOfYear, at);
            if (kCFNotFound == minute) return kCFNotFound;
            CFIndex second = (minute - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_SECOND, &status) + 1;
            return second;
        }
#if _CF_CALENDAR_NANOSECONDS_AVAILABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
        case kCFCalendarUnitNanosecond: {
            CFIndex second = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitSecond, kCFCalendarUnitYearForWeekOfYear, at);
            if (kCFNotFound == second) return kCFNotFound;
            double dseconds = (double)(second - 1) + (at - floor(at));
            return (CFIndex)(dseconds * 1.0e+9) + 1;
        }
#pragma GCC diagnostic pop
#endif
        }
#pragma GCC diagnostic pop // See 10693376
         break;
    case kCFCalendarUnitQuarter:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
        case kCFCalendarUnitMonth: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex month = __cficu_ucal_get(calendar->_cal, UCAL_MONTH, &status);
            CFStringRef ident = CFCalendarGetIdentifier(calendar);
            if (kCFCalendarIdentifierHebrew == ident) {
                CFIndex mcount[] = {1, 2, 3, 1, 2, 3, 4, 1, 2, 3, 1, 2, 3};
                month = mcount[month];
            } else {
                CFIndex mcount[] = {1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 4};
                month = mcount[month];
            }
            return month;
        }
        case kCFCalendarUnitWeekOfYear:
        case kCFCalendarUnitWeekOfMonth:
        case kCFCalendarUnitWeek_Deprecated: { // do not use this combo for recursion
            CFAbsoluteTime start = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitQuarter, at, &start, NULL);
            if (!b) return kCFNotFound;

            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(start) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            // move start forward to first day of week if not already there
            CFIndex days_added = 0;
            while (__cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status) != calendar->_firstWeekday) {
                udate = __CFCalendarAdd(calendar, UCAL_DAY_OF_MONTH, 1, 0, &status);
                days_added++;
            }
            start = udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970;

            CFIndex start_week = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitWeek_Deprecated, kCFCalendarUnitYear, start);
            if (kCFNotFound == start_week) return kCFNotFound;
            if (calendar->_minDaysInFirstWeek <= days_added) {
                start_week--; // previous week chunk was big enough, back up
            }

            CFIndex at_week = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitWeek_Deprecated, kCFCalendarUnitYear, at);
            if (kCFNotFound == at_week) return kCFNotFound;
            CFIndex week = at_week - start_week + 1;
            return week;
        }
        case kCFCalendarUnitWeekdayOrdinal:
        case kCFCalendarUnitWeekday: { // do not use this combo for recursion
            CFAbsoluteTime start = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitQuarter, at, &start, NULL);
            if (!b) return kCFNotFound;

            UErrorCode status = U_ZERO_ERROR;
            CFIndex at_week = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitWeek_Deprecated, kCFCalendarUnitYear, at);
            CFIndex target_dow = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status);
            if (kCFNotFound == at_week) return kCFNotFound;

            status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(start) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            // move start forward to target day of week if not already there
            while (__cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status) != target_dow) {
                udate = __CFCalendarAdd(calendar, UCAL_DAY_OF_MONTH, 1, 0, &status);
            }
            start = udate / 1000.0 - kCFAbsoluteTimeIntervalSince1970;

            CFIndex start_week = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitWeek_Deprecated, kCFCalendarUnitYear, start);
            if (kCFNotFound == start_week) return kCFNotFound;
            CFIndex nth_weekday = at_week - start_week + 1;
            return nth_weekday;
        }
        case kCFCalendarUnitDay: {
            CFAbsoluteTime start = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitQuarter, at, &start, NULL);
            // must do this to make sure things are set up for recursive calls to __CFCalendarGetOrdinalityOfUnit3
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            if (!b) return kCFNotFound;
            CFIndex day = (CFIndex)floor((at - start) / 86400.0) + 1;
            return day;
        }
        case kCFCalendarUnitHour: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex day = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitDay, kCFCalendarUnitQuarter, at);
            if (kCFNotFound == day) return kCFNotFound;
            CFIndex hour = (day - 1) * 24 + __cficu_ucal_get(calendar->_cal, UCAL_HOUR_OF_DAY, &status) + 1;
            return hour;
        }
        case kCFCalendarUnitMinute: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex hour = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitHour, kCFCalendarUnitQuarter, at);
            if (kCFNotFound == hour) return kCFNotFound;
            CFIndex minute = (hour - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_MINUTE, &status) + 1;
            return minute;
        }
        case kCFCalendarUnitSecond: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex minute = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitMinute, kCFCalendarUnitQuarter, at);
            if (kCFNotFound == minute) return kCFNotFound;
            CFIndex second = (minute - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_SECOND, &status) + 1;
            return second;
        }
#if _CF_CALENDAR_NANOSECONDS_AVAILABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
        case kCFCalendarUnitNanosecond: {
            CFIndex second = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitSecond, kCFCalendarUnitQuarter, at);
            if (kCFNotFound == second) return kCFNotFound;
            double dseconds = (double)(second - 1) + (at - floor(at));
            return (CFIndex)(dseconds * 1.0e+9) + 1;
        }
#pragma GCC diagnostic pop
#endif
        }
#pragma GCC diagnostic pop // See 10693376
         break;
    case kCFCalendarUnitMonth:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
        case kCFCalendarUnitWeekOfYear:
            return kCFNotFound;
        case kCFCalendarUnitWeekOfMonth:
        case kCFCalendarUnitWeek_Deprecated: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex week = __cficu_ucal_get(calendar->_cal, UCAL_WEEK_OF_MONTH, &status);
            return week;
        }
        case kCFCalendarUnitDay: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex day = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_MONTH, &status);
            return day;
        }
        case kCFCalendarUnitWeekdayOrdinal:
        case kCFCalendarUnitWeekday: {
            CFIndex day = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitDay, kCFCalendarUnitMonth, at);
            if (kCFNotFound == day) return kCFNotFound;
            CFIndex nth_weekday = (day + 6) / 7;
            return nth_weekday;
        }
        case kCFCalendarUnitHour: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex day = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitDay, kCFCalendarUnitMonth, at);
            if (kCFNotFound == day) return kCFNotFound;
            CFIndex hour = (day - 1) * 24 + __cficu_ucal_get(calendar->_cal, UCAL_HOUR_OF_DAY, &status) + 1;
            return hour;
        }
        case kCFCalendarUnitMinute: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex hour = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitHour, kCFCalendarUnitMonth, at);
            if (kCFNotFound == hour) return kCFNotFound;
            CFIndex minute = (hour - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_MINUTE, &status) + 1;
            return minute;
        }
        case kCFCalendarUnitSecond: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex minute = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitMinute, kCFCalendarUnitMonth, at);
            if (kCFNotFound == minute) return kCFNotFound;
            CFIndex second = (minute - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_SECOND, &status) + 1;
            return second;
        }
#if _CF_CALENDAR_NANOSECONDS_AVAILABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
        case kCFCalendarUnitNanosecond: {
            CFIndex second = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitSecond, kCFCalendarUnitMonth, at);
            if (kCFNotFound == second) return kCFNotFound;
            double dseconds = (double)(second - 1) + (at - floor(at));
            return (CFIndex)(dseconds * 1.0e+9) + 1;
        }
#pragma GCC diagnostic pop
#endif
        }
#pragma GCC diagnostic pop // See 10693376
         break;
    case kCFCalendarUnitWeekOfYear:
    case kCFCalendarUnitWeekOfMonth:
    case kCFCalendarUnitWeek_Deprecated:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
        case kCFCalendarUnitDay:
        case kCFCalendarUnitWeekday: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex day = __cficu_ucal_get(calendar->_cal, UCAL_DAY_OF_WEEK, &status) + 1 - calendar->_firstWeekday;
            if (day <= 0) day += 7;
            return day;
        }
        case kCFCalendarUnitHour: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex day = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitDay, kCFCalendarUnitWeek_Deprecated, at);
            if (kCFNotFound == day) return kCFNotFound;
            CFIndex hour = (day - 1) * 24 + __cficu_ucal_get(calendar->_cal, UCAL_HOUR_OF_DAY, &status) + 1;
            return hour;
        }
        case kCFCalendarUnitMinute: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex hour = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitHour, kCFCalendarUnitWeek_Deprecated, at);
            if (kCFNotFound == hour) return kCFNotFound;
            CFIndex minute = (hour - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_MINUTE, &status) + 1;
            return minute;
        }
        case kCFCalendarUnitSecond: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex minute = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitMinute, kCFCalendarUnitWeek_Deprecated, at);
            if (kCFNotFound == minute) return kCFNotFound;
            CFIndex second = (minute - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_SECOND, &status) + 1;
            return second;
        }
#if _CF_CALENDAR_NANOSECONDS_AVAILABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
        case kCFCalendarUnitNanosecond: {
            CFIndex second = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitSecond, kCFCalendarUnitWeek_Deprecated, at);
            if (kCFNotFound == second) return kCFNotFound;
            double dseconds = (double)(second - 1) + (at - floor(at));
            return (CFIndex)(dseconds * 1.0e+9) + 1;
        }
#pragma GCC diagnostic pop
#endif
        }
#pragma GCC diagnostic pop // See 10693376
         break;
    case kCFCalendarUnitWeekday:
    case kCFCalendarUnitDay:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
        case kCFCalendarUnitHour: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex hour = __cficu_ucal_get(calendar->_cal, UCAL_HOUR_OF_DAY, &status) + 1;
            return hour;
        }
        case kCFCalendarUnitMinute: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex hour = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitHour, kCFCalendarUnitDay, at);
            if (kCFNotFound == hour) return kCFNotFound;
            CFIndex minute = (hour - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_MINUTE, &status) + 1;
            return minute;
        }
        case kCFCalendarUnitSecond: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex minute = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitMinute, kCFCalendarUnitDay, at);
            if (kCFNotFound == minute) return kCFNotFound;
            CFIndex second = (minute - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_SECOND, &status) + 1;
            return second;
        }
#if _CF_CALENDAR_NANOSECONDS_AVAILABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
        case kCFCalendarUnitNanosecond: {
            CFIndex second = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitSecond, kCFCalendarUnitDay, at);
            if (kCFNotFound == second) return kCFNotFound;
            double dseconds = (double)(second - 1) + (at - floor(at));
            return (CFIndex)(dseconds * 1.0e+9) + 1;
        }
#pragma GCC diagnostic pop
#endif
        }
#pragma GCC diagnostic pop // See 10693376
        break;
    case kCFCalendarUnitHour:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
        case kCFCalendarUnitMinute: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex minute = __cficu_ucal_get(calendar->_cal, UCAL_MINUTE, &status) + 1;
            return minute;
        }
        case kCFCalendarUnitSecond: {
            UErrorCode status = U_ZERO_ERROR;
            CFIndex minute = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitMinute, kCFCalendarUnitHour, at);
            if (kCFNotFound == minute) return kCFNotFound;
            CFIndex second = (minute - 1) * 60 + __cficu_ucal_get(calendar->_cal, UCAL_SECOND, &status) + 1;
            return second;
        }
#if _CF_CALENDAR_NANOSECONDS_AVAILABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
        case kCFCalendarUnitNanosecond: {
            CFIndex second = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitSecond, kCFCalendarUnitHour, at);
            if (kCFNotFound == second) return kCFNotFound;
            double dseconds = (double)(second - 1) + (at - floor(at));
            return (CFIndex)(dseconds * 1.0e+9) + 1;
        }
#pragma GCC diagnostic pop
#endif
        }
#pragma GCC diagnostic pop // See 10693376
        break;
    case kCFCalendarUnitMinute:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
        case kCFCalendarUnitSecond: {
            UErrorCode status = U_ZERO_ERROR;
            __cficu_ucal_clear(calendar->_cal);
            UDate udate = (floor(at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
            __cficu_ucal_setMillis(calendar->_cal, udate, &status);
            CFIndex second = __cficu_ucal_get(calendar->_cal, UCAL_SECOND, &status) + 1;
            return second;
        }
#if _CF_CALENDAR_NANOSECONDS_AVAILABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
        case kCFCalendarUnitNanosecond: {
            CFIndex second = __CFCalendarGetOrdinalityOfUnit3(calendar, kCFCalendarUnitSecond, kCFCalendarUnitMinute, at);
            if (kCFNotFound == second) return kCFNotFound;
            double dseconds = (double)(second - 1) + (at - floor(at));
            return (CFIndex)(dseconds * 1.0e+9) + 1;
        }
#pragma GCC diagnostic pop
#endif
        }
#pragma GCC diagnostic pop // See 10693376
        break;
    case kCFCalendarUnitSecond:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
        case kCFCalendarUnitNanosecond:
#pragma GCC diagnostic pop
            return (CFIndex)((at - floor(at)) * 1.0e+9) + 1;
        }
#pragma GCC diagnostic pop // See 10693376
        break;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    case kCFCalendarUnitNanosecond:
        break;
#pragma GCC diagnostic pop
    case kCFCalendarUnitWeekdayOrdinal:
        break;
    }
    return kCFNotFound;
}

// By default, unless special exception is made,
//   Week means Week-of-Year
//   Day means Day-of-Month
//   WeekdayOrdinal means WeekdayOrdinal-of-Month

static CFRange __CFCalendarGetRangeOfUnit3(CFCalendarRef calendar, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime at) {
    if (!calendar->_cal) {
        __CFCalendarSetupCal(calendar);
        if (!calendar->_cal) {
            return CFRangeMake(kCFNotFound, kCFNotFound);
        }
    }

#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (biggerUnit) {
        case kCFCalendarUnitCalendar:
        case kCFCalendarUnitTimeZone:
        case kCFCalendarUnitWeekdayOrdinal:
        case kCFCalendarUnitNanosecond:
            return CFRangeMake(kCFNotFound, kCFNotFound);
    }
#pragma GCC diagnostic pop // See 10693376

#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (smallerUnit) {
    case kCFCalendarUnitWeekday:
        switch (biggerUnit) {
        case kCFCalendarUnitSecond:
        case kCFCalendarUnitMinute:
        case kCFCalendarUnitHour:
        case kCFCalendarUnitDay:
        case kCFCalendarUnitWeekday:
            return CFRangeMake(kCFNotFound, kCFNotFound);
        }
        return CFCalendarGetMaximumRangeOfUnit(calendar, smallerUnit);
    case kCFCalendarUnitHour:
        switch (biggerUnit) {
        case kCFCalendarUnitSecond:
        case kCFCalendarUnitMinute:
        case kCFCalendarUnitHour:
            return CFRangeMake(kCFNotFound, kCFNotFound);
        }
        return CFCalendarGetMaximumRangeOfUnit(calendar, smallerUnit);
    case kCFCalendarUnitMinute:
        switch (biggerUnit) {
        case kCFCalendarUnitSecond:
        case kCFCalendarUnitMinute:
            return CFRangeMake(kCFNotFound, kCFNotFound);
        }
        return CFCalendarGetMaximumRangeOfUnit(calendar, smallerUnit);
    case kCFCalendarUnitSecond:
        switch (biggerUnit) {
        case kCFCalendarUnitSecond:
            return CFRangeMake(kCFNotFound, kCFNotFound);
        }
        return CFCalendarGetMaximumRangeOfUnit(calendar, smallerUnit);
    case kCFCalendarUnitNanosecond:
        return CFCalendarGetMaximumRangeOfUnit(calendar, smallerUnit);
    }
#pragma GCC diagnostic pop // See 10693376

#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (biggerUnit) {
    case kCFCalendarUnitEra:
        // assume it cycles through every possible combination in an era
        // at least once; this is a little dodgy for the Japanese calendar
        // but this calculation isn't terribly useful either
        switch (smallerUnit) {
        case kCFCalendarUnitYear: return CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitYear);
        case kCFCalendarUnitQuarter: return CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitQuarter);
        case kCFCalendarUnitMonth: return CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitMonth);
        case kCFCalendarUnitWeek_Deprecated: return CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitWeek_Deprecated);
        case kCFCalendarUnitWeekOfYear: return CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitWeekOfYear);
        case kCFCalendarUnitWeekOfMonth: return CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitWeekOfMonth);
        case kCFCalendarUnitDay: return CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitDay);
        case kCFCalendarUnitWeekdayOrdinal: {
            CFRange r = CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitDay);
            return CFRangeMake(1, (r.location + r.length - 1 + 6) / 7);
        }
        }
        break;
    case kCFCalendarUnitYear:
        switch (smallerUnit) {
        case kCFCalendarUnitQuarter:
        case kCFCalendarUnitMonth:
        case kCFCalendarUnitWeekOfYear:
        case kCFCalendarUnitWeek_Deprecated: algorithmA: {
            CFRange result = CFRangeMake(kCFNotFound, kCFNotFound);
            CFAbsoluteTime start = 0.0;
            CFTimeInterval ti = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, biggerUnit, at, &start, &ti);
            if (!b) return result;
            CFIndex ord1 = CFCalendarGetOrdinalityOfUnit(calendar, smallerUnit, biggerUnit, start + 0.1);
            if (kCFNotFound == ord1) return result;
            CFIndex ord2 = CFCalendarGetOrdinalityOfUnit(calendar, smallerUnit, biggerUnit, start + ti - 0.1);
            if (kCFNotFound == ord2) return result;
            return CFRangeMake(ord1, ord2 + 1 - ord1);
        }
        case kCFCalendarUnitWeekOfMonth:
        case kCFCalendarUnitDay:
        case kCFCalendarUnitWeekdayOrdinal: algorithmB: {
            CFRange result = CFRangeMake(kCFNotFound, kCFNotFound);
            CFAbsoluteTime start = 0.0;
            CFTimeInterval ti = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, biggerUnit, at, &start, &ti);
            if (!b) return result;
            CFIndex counter = 15; // stopgap in case something goes wrong
            CFAbsoluteTime end = start + ti - 1.0;
            CFAbsoluteTime current = start + 1.0;
            do {
                CFAbsoluteTime startM = 0.0;
                CFTimeInterval tiM = 0.0;
                b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitMonth, current, &startM, &tiM);
                if (!b) return result;
                CFIndex ord1 = CFCalendarGetOrdinalityOfUnit(calendar, smallerUnit, kCFCalendarUnitMonth, startM + 0.1);
                if (kCFNotFound == ord1) return result;
                CFIndex ord2 = CFCalendarGetOrdinalityOfUnit(calendar, smallerUnit, kCFCalendarUnitMonth, startM + tiM - 0.1);
                if (kCFNotFound == ord2) return result;
                if ((kCFNotFound == result.location) || (kCFNotFound == result.length)) {
                    result = CFRangeMake(ord1, ord2 + 1 - ord1);
                } else {
                     CFIndex mn = result.location;
                     if (ord1 < mn) mn = ord1;
                     result.location = mn;
                     result.length += ord2;
                }
                counter--;
                current = startM + tiM + 1.0;
            } while (current < end && 0 < counter);
            return result;
        }
        }
        break;
    case kCFCalendarUnitYearForWeekOfYear:
        switch (smallerUnit) {
        case kCFCalendarUnitQuarter:
        case kCFCalendarUnitMonth:
        case kCFCalendarUnitWeekOfYear:
        case kCFCalendarUnitWeek_Deprecated:
            goto algorithmA; // some of these are going to return (kCFNotFound, kCFNotFound)
        case kCFCalendarUnitWeekOfMonth:
            break;
        case kCFCalendarUnitDay:
        case kCFCalendarUnitWeekdayOrdinal:
            goto algorithmB; // this question is odd to ask
        }
        break;
    case kCFCalendarUnitQuarter:
        switch (smallerUnit) {
        case kCFCalendarUnitMonth:
        case kCFCalendarUnitWeekOfYear:
        case kCFCalendarUnitWeek_Deprecated: algorithmC: {
            CFRange result = CFRangeMake(kCFNotFound, kCFNotFound);
            CFAbsoluteTime start = 0.0;
            CFTimeInterval ti = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, biggerUnit, at, &start, &ti);
            if (!b) return result;
            CFIndex ord1 = CFCalendarGetOrdinalityOfUnit(calendar, smallerUnit, kCFCalendarUnitYear, start + 0.1);
            if (kCFNotFound == ord1) return result;
            CFIndex ord2 = CFCalendarGetOrdinalityOfUnit(calendar, smallerUnit, kCFCalendarUnitYear, start + ti - 0.1);
            if (kCFNotFound == ord2) return result;
            return CFRangeMake(ord1, ord2 + 1 - ord1);
        }
        case kCFCalendarUnitWeekOfMonth:
        case kCFCalendarUnitDay:
        case kCFCalendarUnitWeekdayOrdinal:
            goto algorithmB;
        }
        break;
    case kCFCalendarUnitMonth:
        switch (smallerUnit) {
        case kCFCalendarUnitWeekOfYear:
        case kCFCalendarUnitWeek_Deprecated:
            goto algorithmC;
        case kCFCalendarUnitWeekOfMonth:
        case kCFCalendarUnitDay:
        case kCFCalendarUnitWeekdayOrdinal:
            goto algorithmA;
        }
        break;
    case kCFCalendarUnitWeekOfYear:
        break;
    case kCFCalendarUnitWeekOfMonth:
    case kCFCalendarUnitWeek_Deprecated:
#pragma GCC diagnostic push // See 10693376
#pragma GCC diagnostic ignored "-Wswitch-enum"
        switch (smallerUnit) {
        case kCFCalendarUnitDay: {
            CFRange result = CFRangeMake(kCFNotFound, kCFNotFound);
            CFAbsoluteTime startW = 0.0, startM = 0.0;
            CFTimeInterval tiW = 0.0, tiM = 0.0;
            Boolean b = CFCalendarGetTimeRangeOfUnit(calendar, biggerUnit, at, &startW, &tiW);
            if (!b) return result;
            b = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitMonth, at, &startM, &tiM);
            if (!b) return result;
            CFAbsoluteTime start = (startW < startM) ? (startM) : (startW);
            CFAbsoluteTime end = (startW + tiW < startM + tiM) ? (startW + tiW) : (startM + tiM);
            CFIndex ord1 = CFCalendarGetOrdinalityOfUnit(calendar, smallerUnit, kCFCalendarUnitMonth, start + 0.1);
            if (kCFNotFound == ord1) return result;
            CFIndex ord2 = CFCalendarGetOrdinalityOfUnit(calendar, smallerUnit, kCFCalendarUnitMonth, end - 0.1);
            if (kCFNotFound == ord2) return result;
            return CFRangeMake(ord1, ord2 + 1 - ord1);
        }
        }
#pragma GCC diagnostic pop // See 10693376
        break;
    }
#pragma GCC diagnostic pop // See 10693376
    return CFRangeMake(kCFNotFound, kCFNotFound);
}

#pragma mark -

#define CONVERT(M, CH) do {          \
    CFIndex _value = dateComp->M;    \
    if (CFDateComponentUndefined != _value) {compDesc[compCount] = CH; vector[compCount] = (int32_t)_value; compCount++;} \
} while (0);

CFDateRef CFCalendarCreateDateFromComponents(CFAllocatorRef allocator, CFCalendarRef calendar, CFDateComponentsRef dateComp) {
    CFAbsoluteTime at;
    int32_t vector[20];
    char compDesc[20 + 1];
    int32_t compCount = 0;

    CONVERT(_era, 'G')
    CONVERT(_year, 'y')
    CONVERT(_quarter, 'Q')
    if (CFDateComponentUndefined != dateComp->_weekOfYear) {
        CONVERT(_weekOfYear, 'w')
    } else {
        CONVERT(_week, '^') // extra thing understood by private SPI below
    }
    CONVERT(_weekOfMonth, 'W')
    CONVERT(_yearForWeekOfYear, 'Y')
    CONVERT(_weekday, 'E')
    CONVERT(_weekdayOrdinal, 'F')
    CONVERT(_month, 'M')
    CONVERT(_leapMonth, 'l')
    CONVERT(_day, 'd')
    CONVERT(_hour, 'H')
    CONVERT(_minute, 'm')
    CONVERT(_second, 's')
    CONVERT(_nanosecond, '#')
    compDesc[compCount] = '\0';

    CFTimeZoneRef tempTZ = dateComp->_timeZone, oldTZ = NULL;
    if (tempTZ) {
        oldTZ = CFCalendarCopyTimeZone(calendar);
        CFCalendarSetTimeZone(calendar, tempTZ);
    }
    Boolean b = _CFCalendarComposeAbsoluteTimeV(calendar, &at, compDesc, vector, compCount);
    if (tempTZ) {
        CFCalendarSetTimeZone(calendar, oldTZ);
        CFRelease(oldTZ);
    }
    if (b) {
        return CFDateCreate(CFAllocatorGetDefault(), at);
    }
    return NULL;
}

CFIndex CFCalendarGetComponentFromDate(CFCalendarRef calendar, CFCalendarUnit unit, CFDateRef date) {
    
    // One character for most, two characters for month (incl leap month), plus a terminating null
    char compDesc[3];
    switch (unit) {
        case kCFCalendarUnitEra: compDesc[0] = 'G'; break;
        case kCFCalendarUnitYear: compDesc[0] = 'y'; break;
        case kCFCalendarUnitQuarter: compDesc[0] = 'Q'; break;
        case kCFCalendarUnitMonth: compDesc[0] = 'M'; break;
        case kCFCalendarUnitDay: compDesc[0] = 'd'; break;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        case kCFCalendarUnitWeek: compDesc[0] = '^'; break; // extra thing understood by private SPI below
#pragma GCC diagnostic pop
        case kCFCalendarUnitWeekOfYear: compDesc[0] = 'w'; break;
        case kCFCalendarUnitWeekOfMonth: compDesc[0] = 'W'; break;
        case kCFCalendarUnitYearForWeekOfYear: compDesc[0] = 'Y'; break;
        case kCFCalendarUnitWeekday: compDesc[0] = 'E'; break;
        case kCFCalendarUnitWeekdayOrdinal: compDesc[0] = 'F'; break;
        case kCFCalendarUnitHour: compDesc[0] = 'H'; break;
        case kCFCalendarUnitMinute: compDesc[0] = 'm'; break;
        case kCFCalendarUnitSecond: compDesc[0] = 's'; break;
        case kCFCalendarUnitNanosecond: compDesc[0] = '#'; break;
        default:
            return CFDateComponentUndefined;
    }
    
    int32_t compCount = 0;
    
    // Check for special case leap month
    // TODO: This is probably not required, but left in place to more accurately duplicate previous behavior for time being.
    if (unit == kCFCalendarUnitMonth) {
        compDesc[1] = 'l';
        compDesc[2] = '\0';
        compCount = 2;
    } else {
        compDesc[1] = '\0';
        compCount = 1;
    }
    
    CFAbsoluteTime at = CFDateGetAbsoluteTime(date);
    int32_t ints[3];
    int32_t *vector[3] = {&ints[0], &ints[1], &ints[2]};
    
    if (_CFCalendarDecomposeAbsoluteTimeV(calendar, at, compDesc, vector, compCount)) {
        CFIndex result = (CFIndex)ints[0];
        return result;
    } else {
        return CFDateComponentUndefined;
    }
}

#define SETUPDESC(U, CH) {if (units & (U)) {compDesc[compCount] = CH; compCount++;}}
#define SETCOMP(U, M) do {                \
    if (units & (U)) {                \
        dateComp->M = ints[compCount];    \
        compCount++;                      \
    }} while (0);

CFDateComponentsRef CFCalendarCreateDateComponentsFromDate(CFAllocatorRef allocator, CFCalendarRef calendar, CFCalendarUnit units, CFDateRef date) {
    CFAbsoluteTime at = CFDateGetAbsoluteTime(date);
    int32_t ints[20];
    int32_t *vector[20] = {&ints[0], &ints[1], &ints[2], &ints[3], &ints[4], &ints[5], &ints[6], &ints[7], &ints[8], &ints[9], &ints[10], &ints[11], &ints[12], &ints[13], &ints[14], &ints[15], &ints[16], &ints[17], &ints[18], &ints[19]};
    char compDesc[20 + 1];
    int32_t compCount = 0;

    SETUPDESC(kCFCalendarUnitEra, 'G')
    SETUPDESC(kCFCalendarUnitYear, 'y')
    SETUPDESC(kCFCalendarUnitQuarter, 'Q')
    SETUPDESC(kCFCalendarUnitMonth, 'M')
    SETUPDESC(kCFCalendarUnitMonth, 'l')
    SETUPDESC(kCFCalendarUnitDay, 'd')
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    SETUPDESC(kCFCalendarUnitWeek, '^') // extra thing understood by private SPI below
#pragma GCC diagnostic pop
    SETUPDESC(kCFCalendarUnitWeekOfYear, 'w')
    SETUPDESC(kCFCalendarUnitWeekOfMonth, 'W')
    SETUPDESC(kCFCalendarUnitYearForWeekOfYear, 'Y')
    SETUPDESC(kCFCalendarUnitWeekday, 'E')
    SETUPDESC(kCFCalendarUnitWeekdayOrdinal, 'F')
    SETUPDESC(kCFCalendarUnitHour, 'H')
    SETUPDESC(kCFCalendarUnitMinute, 'm')
    SETUPDESC(kCFCalendarUnitSecond, 's')
    SETUPDESC(kCFCalendarUnitNanosecond, '#')
    compDesc[compCount] = '\0';

    if (_CFCalendarDecomposeAbsoluteTimeV(calendar, at, compDesc, vector, compCount)) {
        CFDateComponentsRef dateComp = CFDateComponentsCreate(allocator);
        compCount = 0;
        SETCOMP(kCFCalendarUnitEra, _era)
        SETCOMP(kCFCalendarUnitYear, _year)
        SETCOMP(kCFCalendarUnitQuarter, _quarter)
        SETCOMP(kCFCalendarUnitMonth, _month)
        SETCOMP(kCFCalendarUnitMonth, _leapMonth)
        SETCOMP(kCFCalendarUnitDay, _day)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        SETCOMP(kCFCalendarUnitWeek, _week)
#pragma GCC diagnostic pop
        SETCOMP(kCFCalendarUnitWeekOfYear, _weekOfYear)
        SETCOMP(kCFCalendarUnitWeekOfMonth, _weekOfMonth)
        SETCOMP(kCFCalendarUnitYearForWeekOfYear, _yearForWeekOfYear)
        SETCOMP(kCFCalendarUnitWeekday, _weekday)
        SETCOMP(kCFCalendarUnitWeekdayOrdinal, _weekdayOrdinal)
        SETCOMP(kCFCalendarUnitHour, _hour)
        SETCOMP(kCFCalendarUnitMinute, _minute)
        SETCOMP(kCFCalendarUnitSecond, _second)
        SETCOMP(kCFCalendarUnitNanosecond, _nanosecond)
        if (units & kCFCalendarUnitCalendar) {
            CFDateComponentsSetCalendar(dateComp, calendar);
        }
        if (units & kCFCalendarUnitTimeZone) {
            CFTimeZoneRef calTZ = CFCalendarCopyTimeZone(calendar);
            CFDateComponentsSetTimeZone(dateComp, calTZ);
            CFRelease(calTZ);
        }
        return dateComp;
    } else {
        // This function is non-null, so return a placeholder.
        return CFDateComponentsCreate(allocator);
    }
}

CF_PRIVATE CFDateRef _CFCalendarCreateDateByAddingDateComponentsToDate(CFAllocatorRef allocator, CFCalendarRef calendar, CFDateComponentsRef dateComp, CFDateRef date, CFOptionFlags opts) {
    assert(calendar != NULL);
    assert(dateComp != NULL);
    assert(date != NULL);
    
    CFAbsoluteTime at = CFDateGetAbsoluteTime(date);
    int32_t vector[20];
    char compDesc[20 + 1];
    int32_t compCount = 0;
    
    CONVERT(_era, 'G')
    CONVERT(_year, 'y')
    CONVERT(_yearForWeekOfYear, 'Y')
    CONVERT(_quarter, 'Q')
    CONVERT(_month, 'M')
    // No leap month support needed here, since these are quantities, not values
    if (CFDateComponentUndefined != dateComp->_weekOfYear) {
        CONVERT(_weekOfYear, 'w')
    } else {
        CONVERT(_week, '^') // extra thing understood by private SPI below
    }
    CONVERT(_weekOfMonth, 'W')
    CONVERT(_day, 'd')
    CONVERT(_weekday, 'E')
    CONVERT(_weekdayOrdinal, 'F')
    CONVERT(_hour, 'H')
    CONVERT(_minute, 'm')
    CONVERT(_second, 's')
    CONVERT(_nanosecond, '#')
    compDesc[compCount] = '\0';
    
    Boolean b = _CFCalendarAddComponentsV((CFCalendarRef)calendar, &at, opts, compDesc, vector, compCount);
    if (b) {
        return CFDateCreate(allocator, at);
    }
    return NULL;
}

CFCalendarUnit _CFCalendarGetUnitsFromDateFormat(CFStringRef format) {
    CFCalendarUnit units = 0;
    CFIndex cnt = CFStringGetLength(format);
    for (CFIndex i = 0; i < cnt; ++i) {
        UniChar c = CFStringGetCharacterAtIndex(format, i);
#if __HAS_APPLE_ICU__
        UDateFormatField fmt_field = udat_patternCharToDateFormatField(c);
        UCalendarDateFields ucal_field = udat_toCalendarDateField(fmt_field);
#else
        UCalendarDateFields ucal_field = __CFCalendarGetICUFieldCodeFromChar(c);
#endif
        CFCalendarUnit unit = __CFCalendarUnitFromICUDateFields(ucal_field);
        if (unit) {
            units |= unit;
        }
    }

    return units;
}

#pragma mark -

#if 0
// Allowed parameters
						 Bigger
	E	Y	Q	M	W	D	H	M	S	N	WD	WDO
Smllr
E	-	-	-	-	-	-	-	-	-	-	-	-
Y	ABCZYX	-	-	-	-	-	-	-	-	-	-	-
Q	CX	CX	-	-	-	-	-	-	-	-	-	-
M	ABCZYX	ABCZYX	CX	-	-	-	-	-	-	-	-	-
W	ABCZYX	ABCZYX	CX	ABCZYX	-	-	-	-	-	-	-	-
D	ABCZYX	ABCZYX	CX	ABCZYX	ABCZYX	-	-	-	-	-	-	-
H	ABCZYX	ABCZYX	CX	ABCZYX	ABCZYX	ABCZYX	-	-	-	-	BCX	-
M	CBX+	ABCZYX	CX	ABCZYX	ABCZYX	ABCZYX	ABCZYX	-	-	-	BCX	-
S	CBX+	ABCZYX	CX	ABCZYX	ABCZYX	ABCZYX	ABCZYX	ABCZYX	-	-	BCX	-
N	C	CX*	CX*	CX*	CX*	CX*	CX*	CX*	CX	-	CX*	-
WD	BCX	BCX	CX	BCX	ABCZYX	-	-	-	-	-	-	-
WDO	ABCZYX	ABCZYX	CX	ABCZYX	-	-	-	-	-	-	-	-

Z: for ordinal, allowed in 10.4
Y: for ordinal, allowed in 10.5
X: for ordinal, allowed in 10.6, * => in 64-bit only, + => unless the result would overflow
A: for range, allowed in 10.4
B: for range, allowed in 10.5
C: for range, allowed in 10.6

#endif

CFRange CFCalendarGetRangeOfUnit(CFCalendarRef calendar, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime at) {
    __CFCalendarValidateAndCapTimeRange(at);
    // Note: We do not toll-free bridge for Swift
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFRange, (NSCalendar *)calendar, _rangeOfUnit:(NSCalendarUnit)smallerUnit inUnit:(NSCalendarUnit)biggerUnit forAT:at);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return __CFCalendarGetRangeOfUnit3(calendar, smallerUnit, biggerUnit, at);
}

CFIndex CFCalendarGetOrdinalityOfUnit(CFCalendarRef calendar, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime at) {
    __CFCalendarValidateAndCapTimeRange(at);
    // Note: We do not toll-free bridge for Swift
    CF_OBJC_FUNCDISPATCHV(CFCalendarGetTypeID(), CFIndex, (NSCalendar *)calendar, _ordinalityOfUnit:(NSCalendarUnit)smallerUnit inUnit:(NSCalendarUnit)biggerUnit forAT:at);
    __CFGenericValidateType(calendar, CFCalendarGetTypeID());
    return __CFCalendarGetOrdinalityOfUnit3(calendar, smallerUnit, biggerUnit, at);
}

CF_PRIVATE CFCalendarRef _CFCalendarCreateCoWWithIdentifier(CFStringRef identifier) {
    return CFCalendarCreateWithIdentifier(kCFAllocatorSystemDefault, identifier);
}

