/*    CFDateComponents.c
      Copyright (c) 2004-2019, Apple Inc. and the Swift project authors

      Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
      Licensed under Apache License v2.0 with Runtime Library Exception
      See http://swift.org/LICENSE.txt for license information
      See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
      Responsibility: Itai Ferber
 */

#include <assert.h>
#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFString.h>
#include "CFDateComponents.h"
#include "CFInternal.h"
#include "CFCalendar_Internal.h"
#include "CFRuntime_Internal.h"

static Boolean __CFDateComponentsEqual(CFTypeRef cf1, CFTypeRef cf2) {
    assert(NULL != cf1);
    assert(NULL != cf2);
    CFDateComponentsRef dc1 = (CFDateComponentsRef)cf1;
    CFDateComponentsRef dc2 = (CFDateComponentsRef)cf2;
    if (dc1->_era != dc2->_era) return false;
    if (dc1->_year != dc2->_year) return false;
    if (dc1->_quarter != dc2->_quarter) return false;
    if (dc1->_month != dc2->_month) return false;
    if (dc1->_day != dc2->_day) return false;
    if (dc1->_hour != dc2->_hour) return false;
    if (dc1->_minute != dc2->_minute) return false;
    if (dc1->_second != dc2->_second) return false;
    if (dc1->_nanosecond != dc2->_nanosecond) return false;
    if (dc1->_week != dc2->_week) return false;
    if (dc1->_weekOfYear != dc2->_weekOfYear) return false;
    if (dc1->_weekOfMonth != dc2->_weekOfMonth) return false;
    if (dc1->_yearForWeekOfYear != dc2->_yearForWeekOfYear) return false;
    if (dc1->_weekday != dc2->_weekday) return false;
    if (dc1->_weekdayOrdinal != dc2->_weekdayOrdinal) return false;
    // TODO: NSDateComponents would compare leapMonth, not checking isLeapMonthSet first. 'isLeapMonth' returns NO in the case where 'isLeapMonthSet' returns NO.
    // This also manifested as a bug where setting leapMonth -> NO meant that the 'isLeapMonthSet' was false after decoding the archive, because the value was only set on the decoded NSDateComponents if 'leapMonth' was YES.
    // For now, we will use the same logic as before, and look into seeing if we can change that behavior for encoding later.
    if (!((dc1->_leapMonth == 0 && dc2->_leapMonth == CFDateComponentUndefined) ||
          (dc1->_leapMonth == CFDateComponentUndefined && dc2->_leapMonth == 0) ||
          (dc1->_leapMonth == dc2->_leapMonth))) {
        return false;
    }
    if ((dc1->_calendar && !dc2->_calendar) || (!dc1->_calendar && dc2->_calendar)) return false;
    if (dc1->_calendar && dc2->_calendar && !CFEqual(dc1->_calendar, dc2->_calendar)) return false;
    if ((dc1->_timeZone && !dc2->_timeZone) || (!dc1->_timeZone && dc2->_timeZone)) return false;
    if (dc1->_timeZone && dc2->_timeZone && !CFEqual(dc1->_timeZone, dc2->_timeZone)) return false;
    return true;
}

static CFHashCode __CFDateComponentsHash(CFTypeRef cf) {
    assert(NULL != cf);
    CFDateComponentsRef dc = (CFDateComponentsRef)cf;
    CFIndex calHash = dc->_calendar ? CFHash(dc->_calendar) : 0;
    CFIndex tzHash = dc->_timeZone ? CFHash(dc->_timeZone) : 0;
    CFIndex calTzHash = calHash ^ tzHash;
    CFIndex y = dc->_year; if (y == CFDateComponentUndefined) y = 0;
    CFIndex m = dc->_month; if (m == CFDateComponentUndefined) m = 0;
    CFIndex d = dc->_day; if (d == CFDateComponentUndefined) d = 0;
    CFIndex h = dc->_hour; if (h == CFDateComponentUndefined) h = 0;
    CFIndex mm = dc->_minute; if (mm == CFDateComponentUndefined) mm = 0;
    CFIndex s = dc->_second; if (s == CFDateComponentUndefined) s = 0;
    CFIndex yy = dc->_yearForWeekOfYear; if (yy == CFDateComponentUndefined) yy = 0;
    CFIndex hash = calTzHash + (32832013 * (y + yy) + 2678437 * m + 86413 * d + 3607 * h + 61 * mm + s);
    hash = hash + (41 * dc->_weekOfYear + 11 * dc->_weekOfMonth + 7 * dc->_weekday + 3 * dc->_weekdayOrdinal + dc->_quarter) * (1ULL << 5);
    return hash;
}

Boolean CFDateComponentsIsLeapMonthSet(CFDateComponentsRef dc) {
    return dc->_leapMonth != CFDateComponentUndefined;
}

Boolean CFDateComponentsIsLeapMonth(CFDateComponentsRef dc) {
    return (CFDateComponentUndefined != dc->_leapMonth && dc->_leapMonth) ? true : false;
}

CFStringRef _CFDateComponentsCopyDescriptionInner(CFDateComponentsRef dc) {
    CFMutableStringRef mstr = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFStringAppend(mstr, CFSTR("{"));
    CFCalendarRef cal = dc->_calendar;
    if (cal) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Calendar: %@"), cal);
    CFTimeZoneRef tz = dc->_timeZone;
    if (tz) CFStringAppendFormat(mstr, NULL, CFSTR("\n    TimeZone: %@"), tz);
    CFIndex val = dc->_era;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Era: %ld"), (long)val);
    val = dc->_year;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Calendar Year: %ld"), (long)val);
    val = dc->_month;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Month: %ld"), (long)val);
    val = dc->_leapMonth;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Leap Month: %ld"), (long)val);
    val = dc->_day;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Day: %ld"), (long)val);
    val = dc->_hour;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Hour: %ld"), (long)val);
    val = dc->_minute;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Minute: %ld"), (long)val);
    val = dc->_second;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Second: %ld"), (long)val);
    val = dc->_nanosecond;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Nanosecond: %ld"), (long)val);
    val = dc->_quarter;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Quarter: %ld"), (long)val);
    val = dc->_yearForWeekOfYear;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Year for Week of Year: %ld"), (long)val);
    val = dc->_weekOfYear;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Week of Year: %ld"), (long)val);
    val = dc->_weekOfMonth;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Week of Month: %ld"), (long)val);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    val = dc->_week;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Week (obsolete): %ld"), (long)val);
#pragma GCC diagnostic pop
    val = dc->_weekday;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Weekday: %ld"), (long)val);
    val = dc->_weekdayOrdinal;
    if (CFDateComponentUndefined != val) CFStringAppendFormat(mstr, NULL, CFSTR("\n    Weekday Ordinal: %ld"), (long)val);
    return mstr;
}

static CFStringRef __CFDateComponentsCopyDescription(CFTypeRef cf) {
    assert(NULL != cf);
    CFDateComponentsRef dc = (CFDateComponentsRef)cf;
    CFStringRef interiorDescription = _CFDateComponentsCopyDescriptionInner(dc);
    CFStringRef result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFDateComponents %p [%p]>%@"), dc, CFGetAllocator(dc), interiorDescription);
    CFRelease(interiorDescription);
    return result;
}

static void __CFDateComponentsDeallocate(CFTypeRef cf) {
    assert(NULL != cf);
    CFDateComponentsRef dc = (CFDateComponentsRef)cf;
    if (dc->_calendar) CFRelease(dc->_calendar);
    if (dc->_timeZone) CFRelease(dc->_timeZone);
}

const CFRuntimeClass __CFDateComponentsClass = {
    0,
    "CFDateComponents",
    NULL,   // init
    NULL,   // copy
    __CFDateComponentsDeallocate,
    __CFDateComponentsEqual,
    __CFDateComponentsHash,
    NULL,   //
    __CFDateComponentsCopyDescription
};

CFTypeID CFDateComponentsGetTypeID(void) {
    return _kCFRuntimeIDCFDateComponents;
}
/* End Runtime setup */

CFDateComponentsRef CFDateComponentsCreate(CFAllocatorRef allocator) {
    if (!allocator) allocator = CFAllocatorGetDefault();
    struct __CFDateComponents *dc = NULL;
    uint32_t size = sizeof(struct __CFDateComponents) - sizeof(CFRuntimeBase);
    dc = (struct __CFDateComponents *)_CFRuntimeCreateInstance(allocator, CFDateComponentsGetTypeID(), size, NULL);
    if (NULL == dc) return NULL;
    dc->_calendar = NULL;
    dc->_timeZone = NULL;
    dc->_era = CFDateComponentUndefined;
    dc->_year = CFDateComponentUndefined;
    dc->_month = CFDateComponentUndefined;
    dc->_leapMonth = CFDateComponentUndefined;
    dc->_day = CFDateComponentUndefined;
    dc->_hour = CFDateComponentUndefined;
    dc->_minute = CFDateComponentUndefined;
    dc->_second = CFDateComponentUndefined;
    dc->_week = CFDateComponentUndefined;
    dc->_weekday = CFDateComponentUndefined;
    dc->_weekdayOrdinal = CFDateComponentUndefined;
    dc->_quarter = CFDateComponentUndefined;
    dc->_weekOfMonth = CFDateComponentUndefined;
    dc->_weekOfYear = CFDateComponentUndefined;
    dc->_yearForWeekOfYear = CFDateComponentUndefined;
    dc->_nanosecond = CFDateComponentUndefined;
    return dc;
}

CFDateComponentsRef CFDateComponentsCreateCopy(CFAllocatorRef allocator, CFDateComponentsRef dc) {
    CFDateComponentsRef result = CFDateComponentsCreate(allocator);
    if (!result) HALT_MSG("Out of memory");
    
    CFCalendarRef cal = CFDateComponentsCopyCalendar(dc);
    if (cal) {
        CFDateComponentsSetCalendar(result, cal);
        CFRelease(cal);
    }
    CFTimeZoneRef tz = CFDateComponentsCopyTimeZone(dc);
    if (tz) {
        CFDateComponentsSetTimeZone(result, tz);
        CFRelease(tz);
    }
    
    // Just reach in for the rest
    result->_era = dc->_era;
    result->_year = dc->_year;
    result->_month = dc->_month;
    result->_leapMonth = dc->_leapMonth;
    result->_day = dc->_day;
    result->_hour = dc->_hour;
    result->_minute = dc->_minute;
    result->_second = dc->_second;
    result->_week = dc->_week;
    result->_weekday = dc->_weekday;
    result->_weekdayOrdinal = dc->_weekdayOrdinal;
    result->_quarter = dc->_quarter;
    result->_weekOfMonth = dc->_weekOfMonth;
    result->_weekOfYear = dc->_weekOfYear;
    result->_yearForWeekOfYear = dc->_yearForWeekOfYear;
    result->_nanosecond = dc->_nanosecond;

    return result;
}

CFIndex CFDateComponentsGetValue(CFDateComponentsRef dateComp, CFCalendarUnit unit) {
    assert(NULL != dateComp);
    CFIndex val = CFDateComponentUndefined;
    switch (unit) {
        case kCFCalendarUnitEra:
            val = dateComp->_era;
            break;
        case kCFCalendarUnitYear:
            val = dateComp->_year;
            break;
        case kCFCalendarUnitMonth:
            val = dateComp->_month;
            break;
        case kCFCalendarUnitLeapMonth:
            val = dateComp->_leapMonth;
            break;
        case kCFCalendarUnitDay:
            val = dateComp->_day;
            break;
        case kCFCalendarUnitHour:
            val = dateComp->_hour;
            break;
        case kCFCalendarUnitMinute:
            val = dateComp->_minute;
            break;
        case kCFCalendarUnitSecond:
            val = dateComp->_second;
            break;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        case kCFCalendarUnitWeek:
            val = dateComp->_week;
            break;
#pragma GCC diagnostic pop
        case kCFCalendarUnitWeekday:
            val = dateComp->_weekday;
            break;
        case kCFCalendarUnitWeekdayOrdinal:
            val = dateComp->_weekdayOrdinal;
            break;
        case kCFCalendarUnitQuarter:
            val = dateComp->_quarter;
            break;
        case kCFCalendarUnitWeekOfMonth:
            val = dateComp->_weekOfMonth;
            break;
        case kCFCalendarUnitWeekOfYear:
            val = dateComp->_weekOfYear;
            break;
        case kCFCalendarUnitYearForWeekOfYear:
            val = dateComp->_yearForWeekOfYear;
            break;
        case kCFCalendarUnitNanosecond:
            val = dateComp->_nanosecond;
            break;
        default:
            // Unknown units are ignored for forwards compatibility
            break;
    }
    return val;
}

void CFDateComponentsSetValue(CFDateComponentsRef dateComp, CFCalendarUnit unit, CFIndex value) {
    assert(NULL != dateComp);
    switch (unit) {
        case kCFCalendarUnitEra:
            dateComp->_era = value;
            break;
        case kCFCalendarUnitYear:
            dateComp->_year = value;
            break;
        case kCFCalendarUnitMonth:
            dateComp->_month = value;
            break;
        case kCFCalendarUnitLeapMonth:
            dateComp->_leapMonth = value;
            break;
        case kCFCalendarUnitDay:
            dateComp->_day = value;
            break;
        case kCFCalendarUnitHour:
            dateComp->_hour = value;
            break;
        case kCFCalendarUnitMinute:
            dateComp->_minute = value;
            break;
        case kCFCalendarUnitSecond:
            dateComp->_second = value;
            break;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        case kCFCalendarUnitWeek:
            dateComp->_week = value;
            break;
#pragma GCC diagnostic pop
        case kCFCalendarUnitWeekday:
            dateComp->_weekday = value;
            break;
        case kCFCalendarUnitWeekdayOrdinal:
            dateComp->_weekdayOrdinal = value;
            break;
        case kCFCalendarUnitQuarter:
            dateComp->_quarter = value;
            break;
        case kCFCalendarUnitWeekOfMonth:
            dateComp->_weekOfMonth = value;
            break;
        case kCFCalendarUnitWeekOfYear:
            dateComp->_weekOfYear = value;
            break;
        case kCFCalendarUnitYearForWeekOfYear:
            dateComp->_yearForWeekOfYear = value;
            break;
        case kCFCalendarUnitNanosecond:
            dateComp->_nanosecond = value;
            break;
        default:
            // Unknown units are ignored for forwards compatibility
            break;
    }
}

CFCalendarRef CFDateComponentsCopyCalendar(CFDateComponentsRef dateComp) {
    assert(NULL != dateComp);
    if (dateComp->_calendar) {
        return (CFCalendarRef)CFRetain(dateComp->_calendar);
    } else {
        return NULL;
    }
}

void CFDateComponentsSetCalendar(CFDateComponentsRef dateComp, CFCalendarRef calendar) {
    assert(NULL != dateComp);
    CFCalendarRef currCal = dateComp->_calendar;
    if (calendar && currCal) {
        if (CFEqual(currCal, calendar)) return;
    }
    if (currCal) {
        CFRelease(dateComp->_calendar);
        dateComp->_calendar = NULL;
    }
    if (calendar) {
        // We copy the calendar, and set its time zone to match the one in the current date components if required
        CFCalendarRef calCopy = _CFCalendarCreateCopy(kCFAllocatorSystemDefault, calendar);
        
        if (dateComp->_timeZone) {
            CFCalendarSetTimeZone(calCopy, dateComp->_timeZone);
        }
        dateComp->_calendar = calCopy;
    }
}

CFTimeZoneRef CFDateComponentsCopyTimeZone(CFDateComponentsRef dateComp) {
    assert(NULL != dateComp);
    if (dateComp->_timeZone) {
        return (CFTimeZoneRef)CFRetain(dateComp->_timeZone);
    } else {
        return NULL;
    }
}

void CFDateComponentsSetTimeZone(CFDateComponentsRef dateComp, CFTimeZoneRef timeZone) {
    assert(NULL != dateComp);
    CFTimeZoneRef currTZ = dateComp->_timeZone;
    if (timeZone && currTZ) {
        if (CFEqual(currTZ, timeZone)) return;
    }
    if (currTZ) {
        CFRelease(dateComp->_timeZone);
        dateComp->_timeZone = NULL;
    }
    if (timeZone) {
        dateComp->_timeZone = (CFTimeZoneRef)CFRetain(timeZone);
        // Also set the tz on our calendar
        CFCalendarRef cal = dateComp->_calendar;
        if (cal) {
            CFCalendarSetTimeZone(cal, timeZone);
        }
    }
}

Boolean CFDateComponentsIsValidDate(CFDateComponentsRef dateComp) {
    CFCalendarRef cal = dateComp->_calendar;
    if (!cal) {
        return false;
    } else {
        return CFDateComponentsIsValidDateInCalendar(dateComp, cal);
    }
}

Boolean CFDateComponentsIsValidDateInCalendar(CFDateComponentsRef dateComp, CFCalendarRef inCalendar) {
    assert(NULL != dateComp);
    assert(NULL != inCalendar);

    CFIndex ns = dateComp->_nanosecond;
    if (CFDateComponentUndefined != ns && 1000 * 1000 * 1000UL <= ns) {
        return false;
    }
    
    CFCalendarRef calendar = _CFCalendarCreateCopy(kCFAllocatorSystemDefault, inCalendar);
    
    // Clear nanoseconds temporarily
    if (CFDateComponentUndefined != ns && 0 < ns) {
        dateComp->_nanosecond = 0;
    }
    
    CFDateRef date = CFCalendarCreateDateFromComponents(kCFAllocatorSystemDefault, calendar, dateComp);
    
    // Reset nanoseconds
    if (CFDateComponentUndefined != ns && 0 < ns) {
        dateComp->_nanosecond = ns;
    }
    
    Boolean result = true;
    if (date) {
        CFCalendarUnit all = kCFCalendarUnitEra | kCFCalendarUnitYear | kCFCalendarUnitMonth | kCFCalendarUnitDay | kCFCalendarUnitHour | kCFCalendarUnitMinute | kCFCalendarUnitSecond | kCFCalendarUnitWeekday | kCFCalendarUnitWeekdayOrdinal | kCFCalendarUnitQuarter | kCFCalendarUnitWeekOfMonth | kCFCalendarUnitWeekOfYear | kCFCalendarUnitYearForWeekOfYear;
        
        CFDateComponentsRef newComps = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, all, date);
        
        if (result && CFDateComponentUndefined != dateComp->_era) if (newComps->_era != dateComp->_era) result = false;
        if (result && CFDateComponentUndefined != dateComp->_year) if (newComps->_year != dateComp->_year) result = false;
        if (result && CFDateComponentUndefined != dateComp->_month) if (newComps->_month != dateComp->_month) result = false;
        if (result && CFDateComponentUndefined != dateComp->_leapMonth) if (newComps->_leapMonth != dateComp->_leapMonth) result = false;
        if (result && CFDateComponentUndefined != dateComp->_day) if (newComps->_day != dateComp->_day) result = false;
        if (result && CFDateComponentUndefined != dateComp->_hour) if (newComps->_hour != dateComp->_hour) result = false;
        if (result && CFDateComponentUndefined != dateComp->_minute) if (newComps->_minute != dateComp->_minute) result = false;
        if (result && CFDateComponentUndefined != dateComp->_second) if (newComps->_second != dateComp->_second) result = false;
        if (result && CFDateComponentUndefined != dateComp->_weekday) if (newComps->_weekday != dateComp->_weekday) result = false;
        if (result && CFDateComponentUndefined != dateComp->_weekdayOrdinal) if (newComps->_weekdayOrdinal != dateComp->_weekdayOrdinal) result = false;
        if (result && CFDateComponentUndefined != dateComp->_quarter) if (newComps->_quarter != dateComp->_quarter) result = false;
        if (result && CFDateComponentUndefined != dateComp->_weekOfMonth) if (newComps->_weekOfMonth != dateComp->_weekOfMonth) result = false;
        if (result && CFDateComponentUndefined != dateComp->_weekOfYear) if (newComps->_weekOfYear != dateComp->_weekOfYear) result = false;
        if (result && CFDateComponentUndefined != dateComp->_yearForWeekOfYear) if (newComps->_yearForWeekOfYear != dateComp->_yearForWeekOfYear) result = false;
        CFRelease(date);
        CFRelease(newComps);
    }
    
    CFRelease(calendar);
    
    // NSDateComponents has a legacy behavior here to support an unset calendar that means if date is not set we return true
    return result;
}

Boolean CFDateComponentsDateMatchesComponents(CFDateComponentsRef dateComp, CFCalendarRef calendar, CFDateRef date) {
    // TODO
    return false;
}
