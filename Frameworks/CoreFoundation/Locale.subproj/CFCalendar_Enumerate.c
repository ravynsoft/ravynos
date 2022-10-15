/*	CFCalendar_Enumerate.c
	Copyright (c) 2004-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/


#include <CoreFoundation/CFCalendar.h>
#include "CFCalendar_Internal.h"
#include "CFDateComponents.h"
#include "CFLocaleInternal.h"
#include "CFInternal.h"

CF_PRIVATE CFDateRef _CFDateCreateWithTimeIntervalSinceDate(CFAllocatorRef allocator, CFTimeInterval ti, CFDateRef date) {
    CFTimeInterval ti2 = CFDateGetAbsoluteTime(date);
    return CFDateCreate(allocator, ti + ti2);
}

#pragma mark -

#define NUM_CALENDAR_UNITS (14)
static CFCalendarUnit const calendarUnits[NUM_CALENDAR_UNITS] = {kCFCalendarUnitEra, kCFCalendarUnitYear, kCFCalendarUnitQuarter, kCFCalendarUnitMonth, kCFCalendarUnitDay, kCFCalendarUnitHour, kCFCalendarUnitMinute, kCFCalendarUnitSecond, kCFCalendarUnitWeekday, kCFCalendarUnitWeekdayOrdinal, kCFCalendarUnitWeekOfMonth, kCFCalendarUnitWeekOfYear, kCFCalendarUnitYearForWeekOfYear, kCFCalendarUnitNanosecond};

static CFDateRef _CFCalendarCreateDateIfEraHasYear(CFCalendarRef calendar, CFIndex era, CFIndex year) {
    CFDateComponentsRef tempComp = CFDateComponentsCreate(kCFAllocatorSystemDefault);
    CFDateComponentsSetValue(tempComp, kCFCalendarUnitEra, era);
    CFDateComponentsSetValue(tempComp, kCFCalendarUnitYear, year);
    CFDateRef date = CFCalendarCreateDateFromComponents(kCFAllocatorSystemDefault, calendar, tempComp);
    CFRelease(tempComp);
    
    CFDateComponentsRef comp = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitEra|kCFCalendarUnitYear, date); // This is only needed for the comparison below
    if (year == 1) {
        CFDateComponentsRef addingUnit = CFDateComponentsCreate(kCFAllocatorSystemDefault);
        CFDateComponentsSetValue(addingUnit, kCFCalendarUnitDay, 1);
        
        // this is needed for Japanese calendar (and maybe other calenders with more than a few eras too)
        while (CFDateComponentsGetValue(comp, kCFCalendarUnitEra) < era) {
            CFDateRef newDate = _CFCalendarCreateDateByAddingDateComponentsToDate(kCFAllocatorSystemDefault, calendar, addingUnit, date, 0);
            CFRelease(date);
            date = newDate;
            CFRelease(comp);
            comp = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitEra, date);
        }
        CFRelease(addingUnit);
        CFRelease(comp);
        comp = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitEra|kCFCalendarUnitYear, date); // Because comp may have changed in the loop
    }
    CFIndex compEra = CFDateComponentsGetValue(comp, kCFCalendarUnitEra);
    CFIndex compYear = CFDateComponentsGetValue(comp, kCFCalendarUnitYear);
    CFRelease(comp);
    if (compEra == era && compYear == year) {  // For Gregorian calendar at least, era and year should always match up so date should always be assigned to result.
        return date;
    } else {
        CFRelease(date);
    }

    return NULL;
}

static CFDateRef _CFCalendarCreateDateIfEraHasYearForWeekOfYear(CFCalendarRef calendar, CFIndex era, CFIndex yearForWeekOfYear, CFTimeInterval *resultInv) {
    CFDateRef yearBegin = _CFCalendarCreateDateIfEraHasYear(calendar, era, yearForWeekOfYear);
    if (yearBegin) {
        // find the beginning of the year for week of year
        CFDateRef result = NULL;
        _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitYearForWeekOfYear, &result, resultInv, yearBegin);
        CFRelease(yearBegin);
        return result;
    }
    return NULL;
}

static Boolean _CFCalendarCheckDateContainsMatchingComponents(CFCalendarRef calendar, CFDateRef date, CFDateComponentsRef compsToMatch, CFCalendarUnit *mismatchedUnits) {
    Boolean dateMatchesComps = true;
    
    CFCalendarRef cal = CFDateComponentsCopyCalendar(compsToMatch);
    CFTimeZoneRef tz = CFDateComponentsCopyTimeZone(compsToMatch);
    Boolean leapMonth = CFDateComponentsIsLeapMonth(compsToMatch);
    Boolean isLeapMonthSet = CFDateComponentsIsLeapMonthSet(compsToMatch);
    CFIndex era = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitEra);
    CFIndex year = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitYear);
    CFIndex quarter = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitQuarter);
    CFIndex month = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitMonth);
    CFIndex day = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitDay);
    CFIndex hour = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitHour);
    CFIndex minute = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitMinute);
    CFIndex second = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitSecond);
    CFIndex weekday = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitWeekday);
    CFIndex weekdayordinal = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitWeekdayOrdinal);
    CFIndex weekOfMonth = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitWeekOfMonth);
    CFIndex weekOfYear = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitWeekOfYear);
    CFIndex yearForWeekOfYear = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitYearForWeekOfYear);
    CFIndex nanosecond = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitNanosecond);
    CFIndex compsToMatchValues[NUM_CALENDAR_UNITS] = {era, year, quarter, month, day, hour, minute, second, weekday, weekdayordinal, weekOfMonth, weekOfYear, yearForWeekOfYear, nanosecond};

    CFCalendarUnit setUnits = 0;
    for (CFIndex i = 0; i < NUM_CALENDAR_UNITS; i++) {
        if (compsToMatchValues[i] != CFDateComponentUndefined) {
            setUnits |= calendarUnits[i];
        }
    }
    CFDateComponentsRef compsFromDate = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, setUnits, date);

    if (cal) {
        CFDateComponentsSetCalendar(compsFromDate, cal);
        CFRelease(cal);
    }

    if (tz) {
        CFDateComponentsSetTimeZone(compsFromDate, tz);
        CFRelease(tz);
    }

    if (!CFEqual(compsFromDate, compsToMatch)) {
        dateMatchesComps = false;

        // Need to make them both arrays so we can cycle through them and find the mismatched units
        CFIndex dateEra = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitEra);
        CFIndex dateYear = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitYear);
        CFIndex dateQuarter = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitQuarter);
        CFIndex dateMonth = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitMonth);
        CFIndex dateDay = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitDay);
        CFIndex dateHour = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitHour);
        CFIndex dateMinute = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitMinute);
        CFIndex dateSecond = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitSecond);
        CFIndex dateWeekday = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitWeekday);
        CFIndex dateWeekdayordinal = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitWeekdayOrdinal);
        CFIndex dateWeekOfMonth = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitWeekOfMonth);
        CFIndex dateWeekOfYear = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitWeekOfYear);
        CFIndex dateYearForWeekOfYear = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitYearForWeekOfYear);
        CFIndex dateNanosecond = CFDateComponentsGetValue(compsFromDate, kCFCalendarUnitNanosecond);
        CFIndex compsFromDateValues[NUM_CALENDAR_UNITS] = {dateEra, dateYear, dateQuarter, dateMonth, dateDay, dateHour, dateMinute, dateSecond, dateWeekday, dateWeekdayordinal, dateWeekOfMonth, dateWeekOfYear, dateYearForWeekOfYear, dateNanosecond};

        if (mismatchedUnits) {
            for (CFIndex i = 0; i < NUM_CALENDAR_UNITS; i++) {
                if (compsToMatchValues[i] != compsFromDateValues[i]) {
                    *mismatchedUnits |= calendarUnits[i];
                }
            }
            if (isLeapMonthSet) {
                if (leapMonth != CFDateComponentsIsLeapMonth(compsFromDate)) {
                    *mismatchedUnits |= kCFCalendarUnitLeapMonth;
                }
            }
        }
    }

    CFRelease(compsFromDate);
    return dateMatchesComps;
}

static CFCalendarUnit _CFCalendarNextHigherUnit(CFCalendarUnit unit) {
    CFCalendarUnit higherUnit = kCFNotFound;
    switch (unit) {
        case kCFCalendarUnitEra:
            break;
        case kCFCalendarUnitYear:
        case kCFCalendarUnitYearForWeekOfYear:
            higherUnit = kCFCalendarUnitEra;
            break;
        case kCFCalendarUnitWeekOfYear:
            higherUnit = kCFCalendarUnitYearForWeekOfYear;
            break;
        case kCFCalendarUnitQuarter:
        case kCFCalendarUnitLeapMonth:
        case kCFCalendarUnitMonth:
            higherUnit = kCFCalendarUnitYear;
            break;
        case kCFCalendarUnitDay:
        case kCFCalendarUnitWeekOfMonth:
        case kCFCalendarUnitWeekdayOrdinal:
            higherUnit = kCFCalendarUnitMonth;
            break;
        case kCFCalendarUnitWeekday:
            higherUnit = kCFCalendarUnitWeekOfMonth;
            break;
        case kCFCalendarUnitHour:
            higherUnit = kCFCalendarUnitDay;
            break;
        case kCFCalendarUnitMinute:
            higherUnit = kCFCalendarUnitHour;
            break;
        case kCFCalendarUnitSecond:
            higherUnit = kCFCalendarUnitMinute;
            break;
        case kCFCalendarUnitNanosecond:
            higherUnit = kCFCalendarUnitSecond;
        default:
            break;
    }
    return higherUnit;
}

static Boolean _CFCalendarCheckIfLeapMonthHack(CFCalendarRef calendar, CFDateRef date) {
    // This is a hack.  kCFCalendarUnitLeapMonth is SPI and components:fromDate: doesn't use it so we can't use it here (even though it's what we actually want), but whenever you create an NSDateComponents object, it sets leapMonthSet by default so we benefit.
    // TODO: Fact check above statement
    CFDateComponentsRef tempComps = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitMonth, date);
    Boolean isLeapMonth = CFDateComponentsIsLeapMonth(tempComps);
    Boolean isLeapMonthSet = CFDateComponentsIsLeapMonthSet(tempComps);
    CFRelease(tempComps);
    return (isLeapMonthSet && isLeapMonth);
}

static CFIndex _CFCalendarSetUnitCount(CFDateComponentsRef comps) {
    CFIndex totalSetUnits = 0;
    CFIndex era = CFDateComponentsGetValue(comps, kCFCalendarUnitEra);
    CFIndex year = CFDateComponentsGetValue(comps, kCFCalendarUnitYear);
    CFIndex quarter = CFDateComponentsGetValue(comps, kCFCalendarUnitQuarter);
    CFIndex month = CFDateComponentsGetValue(comps, kCFCalendarUnitMonth);
    CFIndex day = CFDateComponentsGetValue(comps, kCFCalendarUnitDay);
    CFIndex hour = CFDateComponentsGetValue(comps, kCFCalendarUnitHour);
    CFIndex minute = CFDateComponentsGetValue(comps, kCFCalendarUnitMinute);
    CFIndex second = CFDateComponentsGetValue(comps, kCFCalendarUnitSecond);
    CFIndex weekday = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekday);
    CFIndex weekdayordinal = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekdayOrdinal);
    CFIndex weekOfMonth = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekOfMonth);
    CFIndex weekOfYear = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekOfYear);
    CFIndex yearForWeekOfYear = CFDateComponentsGetValue(comps, kCFCalendarUnitYearForWeekOfYear);
    CFIndex nanosecond = CFDateComponentsGetValue(comps, kCFCalendarUnitNanosecond);
    CFIndex compsValues[NUM_CALENDAR_UNITS] = {era, year, quarter, month, day, hour, minute, second, weekday, weekdayordinal, weekOfMonth, weekOfYear, yearForWeekOfYear, nanosecond};

    for (CFIndex i = 0; i < NUM_CALENDAR_UNITS; i++) {
        if (compsValues[i] != CFDateComponentUndefined) totalSetUnits++;
    }
    return totalSetUnits;
}

#pragma mark -
#pragma mark Create Next Helpers

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingEra(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards, Boolean *foundEra) {
    const CFIndex era = CFDateComponentsGetValue(comps, kCFCalendarUnitEra);
    if (era == CFDateComponentUndefined) return NULL;

    CFIndex dateEra = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitEra, startDate);
    if (era == dateEra) return NULL;

    CFDateRef result = NULL;

    if ((goBackwards && era <= dateEra) || (!goBackwards && era >= dateEra)) {
        CFDateComponentsRef datecomp = CFDateComponentsCreate(kCFAllocatorSystemDefault);
        CFDateComponentsSetValue(datecomp, kCFCalendarUnitEra, era);
        CFDateComponentsSetValue(datecomp, kCFCalendarUnitYear, 1);
        CFDateComponentsSetValue(datecomp, kCFCalendarUnitMonth, 1);
        CFDateComponentsSetValue(datecomp, kCFCalendarUnitDay, 1);
        CFDateComponentsSetValue(datecomp, kCFCalendarUnitHour, 0);
        CFDateComponentsSetValue(datecomp, kCFCalendarUnitMinute, 0);
        CFDateComponentsSetValue(datecomp, kCFCalendarUnitSecond, 0);
        CFDateComponentsSetValue(datecomp, kCFCalendarUnitNanosecond, 0);
        result = CFCalendarCreateDateFromComponents(kCFAllocatorSystemDefault, calendar, datecomp);
        CFRelease(datecomp);
        CFIndex dateCompEra = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitEra, result);
        if (dateCompEra != era) {
            *foundEra = false; // cannot find the matching era
        }
    } else {
        *foundEra = false; // cannot find the matching era
    }
    
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingYear(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards) {
    const CFIndex year = CFDateComponentsGetValue(comps, kCFCalendarUnitYear);
    if (year == CFDateComponentUndefined) return NULL;
    
    CFDateRef result = NULL;
    CFDateComponentsRef dateComp = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitEra|kCFCalendarUnitYear, startDate);
    if (year != CFDateComponentsGetValue(dateComp, kCFCalendarUnitYear)) {
        CFDateRef yearBegin = _CFCalendarCreateDateIfEraHasYear(calendar, CFDateComponentsGetValue(dateComp, kCFCalendarUnitEra), year);
        if (yearBegin) {
            // We set searchStartDate to the end of the year ONLY if we know we will be trying to match anything else beyond just the year and it'll be a backwards search; otherwise, we set searchStartDate to the start of the year.
            const CFIndex totalSetUnits = _CFCalendarSetUnitCount(comps);
            if (goBackwards && (totalSetUnits > 1)) {
                CFTimeInterval yearEndInv = 0.0;
                Boolean foundRange = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitYear, CFDateGetAbsoluteTime(yearBegin), NULL, &yearEndInv);
                if (foundRange) {
                    result = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, yearEndInv-1, yearBegin);
                }
                CFRelease(yearBegin);
            } else {
                result = yearBegin;
            }
        }
    }
    CFRelease(dateComp);
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingYearForWeekOfYear(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards) {
    const CFIndex yearForWeekOfYear = CFDateComponentsGetValue(comps, kCFCalendarUnitYearForWeekOfYear);
    if (yearForWeekOfYear == CFDateComponentUndefined) return NULL;
    
    CFDateRef result = NULL;
    CFDateComponentsRef dateComp = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitEra|kCFCalendarUnitYearForWeekOfYear, startDate);
    if (yearForWeekOfYear != CFDateComponentsGetValue(dateComp, kCFCalendarUnitYearForWeekOfYear)) {
        CFTimeInterval yearInv = 0.0;
        CFDateRef yearBegin = _CFCalendarCreateDateIfEraHasYearForWeekOfYear(calendar, CFDateComponentsGetValue(dateComp, kCFCalendarUnitEra), yearForWeekOfYear, &yearInv);
        if (yearBegin) {
            if (goBackwards) {
                // We need to set searchStartDate to the end of the year
                CFTimeInterval yearEndInv = 0.0;
                Boolean foundRange = CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitYearForWeekOfYear, CFDateGetAbsoluteTime(yearBegin), NULL, &yearEndInv);
                if (foundRange) {
                    result = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, yearEndInv-1, yearBegin);
                }
                CFRelease(yearBegin);
            } else {
                result = yearBegin;
            }
        }
    }
    CFRelease(dateComp);
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingQuarter(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards) {
    const CFIndex quarter = CFDateComponentsGetValue(comps, kCFCalendarUnitQuarter);
    if (quarter == CFDateComponentUndefined) return NULL;

    CFDateRef result = NULL;
    // So this is a thing  -- <rdar://problem/30229506> NSCalendar -component:fromDate: always returns 0 for kCFCalendarUnitQuarter
    CFDateRef yearBegin = NULL;
    CFTimeInterval yearInv = 0.0;
    Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitYear, &yearBegin, &yearInv, startDate); // Get the beginning of the year we need
    if (foundRange) {
        CFIndex count;
        CFDateRef quarterBegin = NULL;
        CFTimeInterval quarterInv = 0.0;
        if (goBackwards) {
            quarterBegin = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, yearInv-1, yearBegin);
            CFRelease(yearBegin);
            count = 4;
            while ((count != quarter) && (count > 0)) {
                CFDateRef tempDate = NULL;
                foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitQuarter, &tempDate, &quarterInv, quarterBegin);
                CFRelease(quarterBegin);
                quarterBegin = tempDate;
                
                quarterInv *= -1;
                tempDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, quarterInv, quarterBegin);
                CFRelease(quarterBegin);
                quarterBegin = tempDate;
                
                count--;
            }
        } else {
            count = 1;
            quarterBegin = yearBegin;
            while ((count != quarter) && (count < 5)) {
                CFDateRef tempDate = NULL;
                foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitQuarter, &tempDate, &quarterInv, quarterBegin);
                CFRelease(quarterBegin);
                quarterBegin = tempDate;
                
                tempDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, quarterInv, quarterBegin);
                CFRelease(quarterBegin);
                quarterBegin = tempDate;
                
                count++;
            }
        }
        result = quarterBegin;
    }
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingWeekOfYear(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards) {
    const CFIndex weekOfYear = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekOfYear);
    if (weekOfYear == CFDateComponentUndefined) return NULL;
    
    CFIndex dateWeekOfYear = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitWeekOfYear, startDate);
    if (weekOfYear == dateWeekOfYear) {
        // Already matches
        return NULL;
    }
    
    // After this point, the result is at least the start date
    CFDateRef result = CFRetain(startDate);
    
    CFDateRef woyBegin = NULL;
    CFTimeInterval woyInv = 0.0;
    do {
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitWeekOfYear, &woyBegin, &woyInv, result);
        if (foundRange) {
            if (goBackwards) {
                // If yearForWeekOfYear is set, at this point we'd already be at the end of the right year, so we can just iterate backward to find the week we need
                woyInv *= -1;  // So we can go backwards in time
            }
            CFDateRef tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, woyInv, woyBegin);
            dateWeekOfYear = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitWeekOfYear, tempSearchDate);
            CFRelease(result);
            result = tempSearchDate;
            CFRelease(woyBegin);
        }
    } while (weekOfYear != dateWeekOfYear);
    
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingMonth(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards, Boolean isStrictMatching) {
    
    const CFIndex month = CFDateComponentsGetValue(comps, kCFCalendarUnitMonth);
    if (month == CFDateComponentUndefined) return NULL;

    Boolean isLeapMonthDesired = CFDateComponentsIsLeapMonth(comps);
    Boolean isLeapMonthSetDesired = CFDateComponentsIsLeapMonthSet(comps);
    const Boolean isChineseCalendar = CFEqual(CFCalendarGetIdentifier(calendar), kCFCalendarIdentifierChinese);
    // isLeapMonth only works for Chinese calendar
    if (!isChineseCalendar) {
        isLeapMonthSetDesired = false;
        isLeapMonthDesired = false;
    }

    // After this point, result is at least startDate
    CFDateRef result = CFRetain(startDate);
    CFIndex dateMonth = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, result);
    if (month != dateMonth) {
        CFDateRef monthBegin = NULL;
        CFTimeInterval mInv = 0.0;
        do {
            Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitMonth, &monthBegin, &mInv, result);
            if (foundRange) {
                // If year is set, at this point we'd already be at the start of the right year, so we can just iterate forward to find the month we need
                if (goBackwards) {
                    CFIndex numMonth = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, monthBegin);
                    if ((numMonth == 3) && CFEqual(CFCalendarGetIdentifier(calendar), kCFCalendarIdentifierGregorian)) {
                        mInv -= (86400*3); // Take it back 3 days so we land in february.  That is, March has 31 days, and Feb can have 28 or 29, so to ensure we get to either Feb 1 or 2, we need to take it back 3 days.
                    } else {
                        mInv -= 86400; // Take it back a day
                    }
                    mInv *= -1;  // So we can go backwards in time
                }
                CFDateRef tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, mInv, monthBegin);
                dateMonth = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, tempSearchDate);
                CFRelease(result);
                result = tempSearchDate;
                CFRelease(monthBegin);
            }
        } while (month != dateMonth);
    }
    
    // As far as we know, this is only relevant for the Chinese calendar.  In that calendar, the leap month has the same month number as the preceding month.
    // If we're searching forwards in time looking for a leap month, we need to skip the first occurrence we found of that month number because the first occurrence would not be the leap month; however, we only do this is if we are matching strictly. If we don't care about strict matching, we can skip this and let the caller handle it so it can deal with the approximations if necessary.
    if (isLeapMonthSetDesired && isLeapMonthDesired && isStrictMatching) {
        CFDateRef leapMonthBegin = NULL;
        CFTimeInterval leapMonthInv = 0.0;
        Boolean leapMonthFound = false;
        
        // We should check if we're already at a leap month!
        if (!_CFCalendarCheckIfLeapMonthHack(calendar, result)) {
            CFDateRef tempSearchDate = CFRetain(result);
            do {
                Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitMonth, &leapMonthBegin, &leapMonthInv, tempSearchDate);
                if (foundRange) {
                    if (goBackwards) {
                        if (isChineseCalendar) {
                            // Months in the Chinese calendar can be either 29 days ("short month") or 30 days ("long month").  We need to account for this when moving backwards in time so we don't end up accidentally skipping months.  If leapMonthBegin is 30 days long, we need to subtract from that 30 so we don't potentially skip over the previous short month.
                            // Also note that some days aren't exactly 24hrs long, so we can end up with lengthOfMonth being something like 29.958333333332, for example.  This is a (albeit hacky) way of getting around that.
                            double lengthOfMonth = leapMonthInv / 86400;
                            if (lengthOfMonth > 30) {
                                leapMonthInv -= 86400*2;
                            } else if (lengthOfMonth > 28) {
                                leapMonthInv -= 86400;
                            }
                        }
                        leapMonthInv *= -1;
                    }
                    CFDateRef tempPossibleLeapMonth = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, leapMonthInv, leapMonthBegin);
                    CFDateComponentsRef monthComps = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitMonth|kCFCalendarUnitLeapMonth, tempPossibleLeapMonth);
                    dateMonth = CFDateComponentsGetValue(monthComps, kCFCalendarUnitMonth);
                    Boolean tempLeapMonth = false;
                    if (CFDateComponentsIsLeapMonthSet(monthComps)) {
                        tempLeapMonth = CFDateComponentsIsLeapMonth(monthComps);
                    }
                    CFRelease(monthComps);
                    if ((dateMonth == month) && tempLeapMonth) {
                        CFRelease(result);
                        result = tempPossibleLeapMonth;
                        leapMonthFound = true;
                    } else {
                        CFRelease(tempSearchDate);
                        tempSearchDate = tempPossibleLeapMonth;
                    }
                    CFRelease(leapMonthBegin);
                }
            } while (!leapMonthFound);
            CFRelease(tempSearchDate);
        }
    }
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingWeekOfMonth(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards) {
    
    const CFIndex weekOfMonth = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekOfMonth);
    if (weekOfMonth == CFDateComponentUndefined) return NULL;
    
    CFIndex dateWeekOfMonth = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitWeekOfMonth, startDate);
    if (weekOfMonth == dateWeekOfMonth) {
        // Already matches
        return NULL;
    }

    // After this point, result is at least startDate
    CFDateRef result = CFRetain(startDate);
    
    CFDateRef womBegin = NULL;
    CFTimeInterval womInv = 0.0;
    do {
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitWeekOfMonth, &womBegin, &womInv, result);
        if (foundRange) {
            // We need to advance or rewind to the next week.
            // This is simple when we can jump by a whole week interval, but there are complications around WoM == 1 because it can start on any day of the week. Jumping forward/backward by a whole week can miss it.
            //
            // A week 1 which starts on any day but Sunday contains days from week 5 of the previous month, e.g.
            //
            //        June 2018
            //   Su Mo Tu We Th Fr Sa
            //                   1  2
            //    3  4  5  6  7  8  9
            //   10 11 12 13 14 15 16
            //   17 18 19 20 21 22 23
            //   24 25 26 27 28 29 30
            //
            // Week 1 of June 2018 starts on Friday; any day before that is week 5 of May.
            // We can jump by a week interval if we're not looking for WoM == 2 or we're not close.
            bool advanceDaily = (weekOfMonth == 1) /* we're looking for WoM == 1 */;
            if (goBackwards) {
                // Last week/earlier this week is week 1.
                advanceDaily &= dateWeekOfMonth <= 2;
            } else {
                // We need to be careful if it's the last week of the month. We can't assume what number week that would be, so figure it out.
                CFRange range = CFCalendarGetRangeOfUnit(calendar, kCFCalendarUnitWeekOfMonth, kCFCalendarUnitMonth, CFDateGetAbsoluteTime(result));
                advanceDaily &= dateWeekOfMonth == range.length;
            }
            
            CFDateRef tempSearchDate = NULL;
            if (!advanceDaily) {
                // We can jump directly to next/last week. There's just one further wrinkle here when doing so backwards: due to DST, it's possible that this week is longer/shorter than last week.
                // That means that if we rewind by womInv (the length of this week), we could completely skip last week, or end up not at its first instant.
                //
                // We can avoid this by not rewinding by womInv, but by going directly to the start.
                if (goBackwards) {
                    // Any instant before womBegin is last week.
                    CFDateRef lateLastWeek = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, -1, womBegin);
                    if (!_CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitWeekOfMonth, &tempSearchDate, NULL, lateLastWeek)) {
                        // It shouldn't be possible to hit this case, but if we somehow got here, we can fall back to searching day by day.
                        advanceDaily = YES;
                    }
                    
                    CFRelease(lateLastWeek);
                } else {
                    // Skipping forward doesn't have these DST concerns, since womInv already represents the length of this week.
                    tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, womInv, womBegin);
                }
            }
            
            // This is a separate condition because it represents a "possible" fallthrough from above.
            if (advanceDaily) {
                // The start of week 1 of any month is just day 1 of the month.
                CFDateRef today = CFRetain(womBegin);
                while (CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitDay, today) != 1) {
                    CFDateRef next = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, (goBackwards ? -1 : 1), kCFCalendarUnitDay, today);
                    CFRelease(today);
                    today = next;
                }
                
                tempSearchDate = today;
            }
            
            CFRelease(womBegin);
            dateWeekOfMonth = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitWeekOfMonth, tempSearchDate);
            CFRelease(result);
            result = tempSearchDate;
        }
    } while (weekOfMonth != dateWeekOfMonth);
    
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingWeekdayOrdinal(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards) {
    
    const CFIndex weekdayordinal = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekdayOrdinal);
    if (weekdayordinal == CFDateComponentUndefined) return NULL;
    
    CFIndex dateWeekdayOrd = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitWeekdayOrdinal, startDate);
    if (weekdayordinal == dateWeekdayOrd) {
        // Already matches
        return NULL;
    }
    
    // After this point, result is at least startDate
    CFDateRef result = CFRetain(startDate);

    CFDateRef wdBegin = NULL;
    CFTimeInterval wdInv = 0.0;
    do {
        // TODO: Consider jumping ahead by week here instead of day.
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitWeekdayOrdinal, &wdBegin, &wdInv, result);
        if (foundRange) {
            if (goBackwards) {
                wdInv *= -1;  // So we can go backwards in time
            }
            CFDateRef tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, wdInv, wdBegin);
            CFRelease(wdBegin);
            dateWeekdayOrd = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitWeekdayOrdinal, tempSearchDate);
            CFRelease(result);
            result = tempSearchDate;
        }
    } while (weekdayordinal != dateWeekdayOrd);
    
    // NOTE: In order for an ordinal weekday to not be ambiguous, it needs both
    //  - the ordinality (e.g. 1st)
    //  - the weekday (e.g. Tuesday)
    // If the weekday is not set, we assume the client just wants the first time in a month where the number of occurrences of a day matches the weekdayOrdinal value (e.g. for weekdayOrdinal = 4, this means the first time a weekday is the 4th of that month. So if the start date is 2017-06-01, then the first time we hit a day that is the 4th occurrence of a weekday would be 2017-06-22. I recommend looking at the month in its entirety on a calendar to see what I'm talking about.).  This is an odd request, but we will return that result to the client while silently judging them.
    // For a non-ambiguous ordinal weekday (i.e. the ordinality and the weekday have both been set), we need to ensure that we get the exact ordinal day that we are looking for. Hence the below weekday check.
    
    const CFIndex weekday = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekday);
    if (weekday == CFDateComponentUndefined) {
        // Skip weekday
        return result;
    }
    
    // Once we're here, it means we found a day with the correct ordinality, but it may not be the specific weekday we're also looking for (e.g. we found the 2nd Thursday of the month when we're looking for the 2nd Friday).
    CFIndex dateWeekday = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitWeekday, result);
    if (weekday == dateWeekday) {
        // Already matches
        return result;
    }
    
    // Start result over
    CFRelease(result);
    result = NULL;

    if (dateWeekday > weekday) {
        // We're past the weekday we want. Go to the beginning of the week
        // We use startDate again here, not result
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitWeekOfMonth, &result, NULL, startDate);
        if (foundRange) {
            CFDateComponentsRef startingDayWeekdayComps = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitWeekday|kCFCalendarUnitWeekdayOrdinal, result);
            dateWeekday = CFDateComponentsGetValue(startingDayWeekdayComps, kCFCalendarUnitWeekday);
            dateWeekdayOrd = CFDateComponentsGetValue(startingDayWeekdayComps, kCFCalendarUnitWeekdayOrdinal);
            CFRelease(startingDayWeekdayComps);
        }
    }
    
    if (!result) {
        // We need to have a value here - use the start date
        result = CFRetain(startDate);
    }
    
    while ((weekday != dateWeekday) || (weekdayordinal != dateWeekdayOrd)) {
        // Now iterate through each day of the week until we find the specific weekday we're looking for.
        CFDateRef currWeekday = NULL;
        CFTimeInterval currWeekdayInv = 0.0;
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitDay, &currWeekday, &currWeekdayInv, result);
        if (foundRange) {
            CFDateRef nextDay = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, currWeekdayInv, currWeekday);
            CFDateComponentsRef nextDayWeekdayComps = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitWeekday|kCFCalendarUnitWeekdayOrdinal, nextDay);
            dateWeekday = CFDateComponentsGetValue(nextDayWeekdayComps, kCFCalendarUnitWeekday);
            dateWeekdayOrd = CFDateComponentsGetValue(nextDayWeekdayComps, kCFCalendarUnitWeekdayOrdinal);
            CFRelease(nextDayWeekdayComps);
            CFRelease(result);
            result = nextDay;
            CFRelease(currWeekday);
        }
    }

    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingWeekday(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards) {

    // NOTE: This differs from the weekday check in weekdayOrdinal because weekday is meant to be ambiguous and can be set without setting the ordinality.
    // e.g. inquiries like "find the next tuesday after 2017-06-01" or "find every wednesday before 2012-12-25"
    
    const CFIndex weekday = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekday);
    if (weekday == CFDateComponentUndefined) return NULL;
    
    CFIndex dateWeekday = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitWeekday, startDate);
    if (weekday == dateWeekday) {
        // Already matches
        return NULL;
    }

    // After this point, result is at least startDate
    CFDateRef result = CFRetain(startDate);

    CFDateRef wdBegin = NULL;
    CFTimeInterval wdInv = 0.0;
    do {
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitWeekday, &wdBegin, &wdInv, result);
        if (foundRange) {
            // We need to either advance or rewind by a day.
            // * Advancing to tomorrow is relatively simple: get the start of today and get the length of that day — then, advance by that length
            // * Rewinding to the start of yesterday is more complicated: the length of today is not necessarily the length of yesterday if DST transitions are involved:
            //   * Today can have 25 hours: if we rewind 25 hours from the start of today, we'll skip yesterday altogether
            //   * Today can have 24 hours: if we rewind 24 hours from the start of today, we might skip yesterday if it had 23 hours, or end up at the wrong time if it had 25
            //   * Today can have 23 hours: if we rewind 23 hours from the start of today, we'll end up at the wrong time yesterday
            //
            // We need to account for DST by ensuring we rewind to exactly the time we want.
            CFDateRef tempSearchDate = NULL;
            if (goBackwards) {
                // Any time prior to dayBegin is yesterday. Since we want to rewind to the start of yesterday, do that directly.
                CFDateRef lateYesterday = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, -1, wdBegin);
                CFDateRef yesterdayBegin = NULL;
                
                // Now we can get the exact moment that yesterday began on.
                // It shouldn't be possible to fail to find this interval, but if that somehow happens, we can try to fall back to the simple but wrong method.
                foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitDay, &yesterdayBegin, NULL, lateYesterday);
                if (foundRange) {
                    tempSearchDate = yesterdayBegin;
                } else {
                    // This fallback is only really correct when today and yesterday have the same length.
                    // Again, it shouldn't be possible to hit this case.
                    tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, -wdInv, wdBegin);
                }
                
                CFRelease(lateYesterday);
            } else {
                // This is always correct to do since we are using today's length on today — there can't be a mismatch.
                tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, wdInv, wdBegin);
            }

            CFRelease(wdBegin);
            dateWeekday = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitWeekday, tempSearchDate);
            CFRelease(result);
            result = tempSearchDate;
        }
    } while (weekday != dateWeekday);
    
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingDay(CFCalendarRef calendar, CFDateRef startDate, CFDateRef originalStartDate, CFDateComponentsRef comps, Boolean goBackwards) {
    
    const CFIndex day = CFDateComponentsGetValue(comps, kCFCalendarUnitDay);
    if (day == CFDateComponentUndefined) return NULL;
    
    // After this point, result is at least startDate
    CFDateRef result = CFRetain(startDate);

    CFIndex dateDay = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitDay, result);
    const CFIndex month = CFDateComponentsGetValue(comps, kCFCalendarUnitMonth);

    if ((month != CFDateComponentUndefined) && goBackwards) {
        // Are we in the right month already?  If we are and goBackwards is set, we should move to the beginning of the last day of the month and work backwards.
        CFDateRef monthBegin = NULL;
        CFTimeInterval monthInv = 0.0;
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitMonth, &monthBegin, &monthInv, result);
        if (foundRange) {
            CFDateRef tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, monthInv-1, monthBegin);
            // Check the order to make sure we didn't jump ahead of the start date
            CFComparisonResult order = CFDateCompare(tempSearchDate, originalStartDate, NULL);
            if (order == kCFCompareGreaterThan) {
                // We went too far ahead.  Just go back to using the start date as our upper bound.
                CFRelease(result);
                result = CFRetain(originalStartDate);
            } else {
                CFDateRef dayBegin = NULL;
                CFTimeInterval dInv = 0.0;
                foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitDay, &dayBegin, &dInv, tempSearchDate);
                if (foundRange) {
                    CFRelease(result);
                    result = dayBegin;
                    dateDay = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitDay, result);
                }
            }
            CFRelease(tempSearchDate);
            CFRelease(monthBegin);
        }
    }
    
    CFDateRef dayBegin = NULL;
    CFTimeInterval dInv = 0.0;
    if (day != dateDay) {
        // The condition below keeps us from blowing past a month day by day to find a day which does not exist.
        // e.g. trying to find the 30th of February starting in January would go to March 30th if we don't stop here
        CFIndex const originalMonth = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, result);
        Boolean advancedPastWholeMonth = false;
        do {
            Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitDay, &dayBegin, &dInv, result);
            if (foundRange) {
                // We need to either advance or rewind by a day.
                // * Advancing to tomorrow is relatively simple: get the start of today and get the length of that day — then, advance by that length
                // * Rewinding to the start of yesterday is more complicated: the length of today is not necessarily the length of yesterday if DST transitions are involved:
                //   * Today can have 25 hours: if we rewind 25 hours from the start of today, we'll skip yesterday altogether
                //   * Today can have 24 hours: if we rewind 24 hours from the start of today, we might skip yesterday if it had 23 hours, or end up at the wrong time if it had 25
                //   * Today can have 23 hours: if we rewind 23 hours from the start of today, we'll end up at the wrong time yesterday
                //
                // We need to account for DST by ensuring we rewind to exactly the time we want.
                CFDateRef tempSearchDate = NULL;
                if (goBackwards) {
                    // Any time prior to dayBegin is yesterday. Since we want to rewind to the start of yesterday, do that directly.
                    CFDateRef lateYesterday = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, -1, dayBegin);
                    CFDateRef yesterdayBegin = NULL;
                    
                    // Now we can get the exact moment that yesterday began on.
                    // It shouldn't be possible to fail to find this interval, but if that somehow happens, we can try to fall back to the simple but wrong method.
                    foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitDay, &yesterdayBegin, NULL, lateYesterday);
                    if (foundRange) {
                        tempSearchDate = yesterdayBegin;
                    } else {
                        // This fallback is only really correct when today and yesterday have the same length.
                        // Again, it shouldn't be possible to hit this case.
                        tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, -dInv, dayBegin);
                    }
                    
                    CFRelease(lateYesterday);
                } else {
                    // This is always correct to do since we are using today's length on today -- there can't be a mismatch.
                    tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, dInv, dayBegin);
                }
                
                CFRelease(dayBegin);
                dateDay = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitDay, tempSearchDate);
                CFIndex const dateMonth = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, tempSearchDate);
                CFRelease(result);
                result = tempSearchDate;

                if (llabs(dateMonth - originalMonth) >= 2) {
                    advancedPastWholeMonth = true;
                    break;
                }
            }
        } while (day != dateDay);

        // If we blew past a month in its entirety, roll back by a day to the very end of the month.
        if (advancedPastWholeMonth) {
            CFDateRef const tempSearchDate = result;
            result = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, -dInv, tempSearchDate);
            CFRelease(tempSearchDate);
        }
    } else {
        // When the search date matches the day we're looking for, we still need to clear the lower components in case they are not part of the components we're looking for.
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitDay, &dayBegin, &dInv, result);
        if (foundRange) {
            CFRelease(result);
            result = dayBegin;
        }
    }
    
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingHour(CFCalendarRef calendar, CFDateRef startDate, CFDateRef originalStartDate, CFDateComponentsRef comps, Boolean goBackwards, Boolean findLastMatch, Boolean isStrictMatching, CFOptionFlags options) {
    const CFIndex hour = CFDateComponentsGetValue(comps, kCFCalendarUnitHour);
    if (hour == CFDateComponentUndefined) return NULL;

    // After this point, result is at least startDate
    CFDateRef result = CFRetain(startDate);

    Boolean adjustedSearchStartDate = false;
    
    CFIndex dateHour = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, result);
    
    // The loop below here takes care of advancing forward in the case of an hour mismatch, taking DST into account.
    // However, it does not take into account a unique circumstance: searching for hour 0 of a day on a day that has no hour 0 due to DST.
    //
    // America/Sao_Paulo, for instance, is a time zone which has DST at midnight -- an instant after 11:59:59 PM can become 1:00 AM, which is the start of the new day:
    //
    //            2018-11-03                      2018-11-04
    //    ┌─────11:00 PM (GMT-3)─────┐ │ ┌ ─ ─ 12:00 AM (GMT-3)─ ─ ─┐ ┌─────1:00 AM (GMT-2) ─────┐
    //    │                          │ │ |                          │ │                          │
    //    └──────────────────────────┘ │ └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘ └▲─────────────────────────┘
    //                                            Nonexistent          └── Start of Day
    //
    // The issue with this specifically is that parts of the rewinding algorithm that handle overshooting rewind to the start of the day to search again (or alternatively, adjusting higher components tends to send us to the start of the day).
    // This doesn't work when the day starts past the time we're looking for if we're looking for hour 0.
    //
    // If we're not matching strictly, we need to check whether we're already a non-strict match and not an overshoot.
    if (hour == 0 /* searching for hour 0 */ && !isStrictMatching) {
        CFDateRef dayBegin = NULL;
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitDay, &dayBegin, NULL, result);
        if (foundRange) {
            CFIndex firstHourOfTheDay = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, dayBegin);
            if (firstHourOfTheDay != 0 && dateHour == firstHourOfTheDay) {
                // We're at the start of the day; it's just not hour 0.
                // We have a candidate match. We can modify that match based on the actual options we need to set.
                if (options & kCFCalendarMatchNextTime) {
                    // We don't need to preserve the smallest components. We can wipe them out.
                    // Note that we rewind to the start of the hour by rewinding to the start of the day -- normally we'd want to rewind to the start of _this_ hour in case there were a difference in a first/last scenario (repeated hour DST transition), but we can't both be missing hour 0 _and_ be the second hour in a repeated transition.
                    result = CFRetain(dayBegin);
                } else if (options & kCFCalendarMatchNextTimePreservingSmallerUnits || options & kCFCalendarMatchPreviousTimePreservingSmallerUnits) {
                    // We want to preserve any currently set smaller units (hour and minute), so don't do anything.
                    // If we need to match the previous time (i.e. go back an hour), that adjustment will be made elsewhere, in the generalized isForwardDST adjustment in the main loop.
                }
                
                // Avoid making any further adjustments again.
                adjustedSearchStartDate = YES;
            }
            
            CFRelease(dayBegin);
        }
    }
    
    // This is a real mismatch and not due to hour 0 being missing.
    // NOTE: The behavior of generalized isForwardDST checking depends on the behavior of this loop!
    //       Right now, in the general case, this loop stops iteration _before_ a forward DST transition. If that changes, please take a look at the isForwardDST code for when `beforeTransition = NO` and adjust as necessary.
    if (hour != dateHour && !adjustedSearchStartDate) {
        CFDateRef hourBegin = NULL;
        CFTimeInterval hInv = 0.0;
        do {
            Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitHour, &hourBegin, &hInv, result);
            if (foundRange) {
                CFIndex prevDateHour = dateHour;
                CFDateRef tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, hInv, hourBegin);
                dateHour = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, tempSearchDate);
                
                // Sometimes we can get into a position where the next hour is also equal to hour (as in we hit a backwards DST change). In this case, we could be at the first time this hour occurs. If we want the next time the hour is technically the same (as in we need to go to the second time this hour occurs), we check to see if we hit a backwards DST change.
                CFDateRef possibleBackwardDSTDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, hInv*2, hourBegin);
                CFIndex secondDateHour = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, possibleBackwardDSTDate);
                
                if (((dateHour - prevDateHour) == 2) || (prevDateHour == 23 && dateHour == 1)) {
                    // We've hit a forward DST transition.
                    dateHour -= 1;
                    CFRelease(possibleBackwardDSTDate);
                    CFRelease(tempSearchDate);
                    
                    CFRelease(result);
                    result = hourBegin;
                } else if ((secondDateHour == dateHour) && findLastMatch) {  // If we're not trying to find the last match, just pass on the match we already found.
                    // We've hit a backwards DST transition.
                    CFRelease(tempSearchDate);
                    CFRelease(hourBegin);
                    
                    CFRelease(result);
                    result = possibleBackwardDSTDate;
                } else {
                    CFRelease(possibleBackwardDSTDate);
                    CFRelease(hourBegin);

                    CFRelease(result);
                    result = tempSearchDate;
                }
                
                adjustedSearchStartDate = true;
            }
        } while (hour != dateHour);
        
        CFComparisonResult order = CFDateCompare(originalStartDate, result, NULL);
        if (goBackwards && (order == kCFCompareLessThan)) { // We've gone into the future when we were supposed to go into the past.  We're ahead by a day.
            CFDateRef tempResult = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, -1, kCFCalendarUnitDay, result);
            CFRelease(result);
            result = tempResult;
            
            // Check hours again to see if they match (they may not because of DST change already being handled implicitly by dateByAddingUnit:)
            dateHour = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, result);
            if ((dateHour - hour) == 1) {  // Detecting a DST transition
                // We have moved an hour ahead of where we want to be so we go back 1 hour to readjust.
                CFDateRef tempResult = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, -1, kCFCalendarUnitHour, result);
                CFRelease(result);
                result = tempResult;
            } else if ((hour - dateHour) == 1) {
                // <rdar://problem/31051045> NSCalendar enumerateDatesStartingAfterDate: returns nil for the day before a forward DST change when searching backwards
                // This is a weird special edge case that only gets hit when you're searching backwards and move past a forward (skip an hour) DST transition.
                // We're not at a DST transition but the hour of our date got moved because the previous day had a DST transition.
                // So we're an hour before where we want to be. We move an hour ahead to correct and get back to where we need to be.
                CFDateRef tempResult = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, 1, kCFCalendarUnitHour, result);
                CFRelease(result);
                result = tempResult;
            }
        }
    }
    
    if (findLastMatch) {
        CFDateRef hourBegin = NULL;
        CFTimeInterval hInv = 0.0;
        
        // This does the same as above for detecting backwards DST changes except that it covers the case where dateHour is already equal to hour. In this case, we could be at the first time we hit an hour (if it falls in a DST transition). Note: the check here tends to be for time zones like Brazil/East where the transition happens at midnight.
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitHour, &hourBegin, &hInv, result);
        if (foundRange) {
            // Rewind forward/back hour-by-hour until we get to a different hour. A loop here is necessary because not all DST transitions are only an hour long.
            CFDateRef next = CFRetain(hourBegin);
            CFIndex nextHour = hour;
            while (nextHour == hour) {
                CFRelease(result);
                result = next;
                
                next = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, (goBackwards ? -1 : 1), kCFCalendarUnitHour, next);
                nextHour = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, next);
            }
            
            CFRelease(next);
            CFRelease(hourBegin);
            adjustedSearchStartDate = YES;
        }
    }
    
    if (!adjustedSearchStartDate) {
        // This applies if we didn't hit the above cases to adjust the search start date, i.e. the hour already matches the start hour and either:
        // 1) We're not looking to match the "last" (repeated) hour in a DST transition (regardless of whether we're in a DST transition), or
        // 2) We are looking to match that hour, but we're not in that DST transition.
        //
        // In either case, we need to clear the lower components in case they are not part of the components we're looking for.
        CFDateRef hourBegin = NULL;
        CFTimeInterval hInv = 0.0;
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitHour, &hourBegin, &hInv, result);
        if (foundRange) {
            CFRelease(result);
            result = hourBegin;
            adjustedSearchStartDate = true;
        }
    }
    
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingMinute(CFCalendarRef calendar, CFDateRef startDate, CFDateRef originalStartDate, CFDateComponentsRef comps, Boolean goBackwards) {
    const CFIndex minute = CFDateComponentsGetValue(comps, kCFCalendarUnitMinute);
    if (minute == CFDateComponentUndefined) return NULL;

    // After this point, result is at least startDate
    CFDateRef result = CFRetain(startDate);

    CFIndex dateMinute = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMinute, result);
    CFDateRef minuteBegin = NULL;
    CFTimeInterval minInv = 0.0;
    if (minute != dateMinute) {
        do {
            Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitMinute, &minuteBegin, &minInv, result);
            if (foundRange) {
                CFDateRef tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, minInv, minuteBegin);
                CFRelease(minuteBegin);
                dateMinute = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMinute, tempSearchDate);
                CFRelease(result);
                result = tempSearchDate;
            }
        } while (minute != dateMinute);
    } else {
        // When the search date matches the minute we're looking for, we need to clear the lower components in case they are not part of the components we're looking for.
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitMinute, &minuteBegin, &minInv, result);
        if (foundRange) {
            CFRelease(result);
            result = minuteBegin;
        }
    }
    
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingSecond(CFCalendarRef calendar, CFDateRef startDate, CFDateRef originalStartDate, CFDateComponentsRef comps, Boolean goBackwards) {
    const CFIndex second = CFDateComponentsGetValue(comps, kCFCalendarUnitSecond);
    if (second == CFDateComponentUndefined) return NULL;

    // After this point, result is at least startDate
    CFDateRef result = CFRetain(startDate);

    CFIndex dateSecond = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitSecond, result);
    CFDateRef secondBegin = NULL;
    CFTimeInterval secInv = 0.0;
    if (second != dateSecond) {
        do {
            Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitSecond, &secondBegin, &secInv, result);
            if (foundRange) {
                CFDateRef tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, secInv, secondBegin);
                CFRelease(secondBegin);
                dateSecond = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitSecond, tempSearchDate);
                CFRelease(result);
                result = tempSearchDate;
            }
        } while (second != dateSecond);
        
        CFComparisonResult order = CFDateCompare(originalStartDate, result, NULL);
        if (order == kCFCompareLessThan /* originalStartDate < result */) {
            if (goBackwards) {
                // We've gone into the future when we were supposed to go into the past.
                // There are multiple times a day where the seconds repeat.  Need to take that into account.
                CFIndex originalStartSecond = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitSecond, originalStartDate);
                if (dateSecond > originalStartSecond) {
                    CFDateRef tempResult = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, -1, kCFCalendarUnitMinute, result);
                    CFRelease(result);
                    result = tempResult;
                }
            } else {
                // <rdar://problem/31098131> NSCalendar returning nil for nextDateAfterDate when I would expect it to pass -- UserNotifications unit tests are failing
                // See the corresponding unit test for this^^ radar to see the case in action.  This handles the case where dateSecond started ahead of second, so doing the above landed us in the next minute.  If minute is not set, we are fine.  But if minute is set, then we are now in the wrong minute and we have to readjust.
                CFIndex searchStartMin = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMinute, result);
                const CFIndex minute = CFDateComponentsGetValue(comps, kCFCalendarUnitMinute);
                if (minute != CFDateComponentUndefined) {
                    if (searchStartMin > minute) {  // We've gone ahead of where we needed to be
                        do { // Reset to beginning of minute
                            CFDateRef minBegin = NULL;
                            CFTimeInterval minBeginInv = 0.0;
                            Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitMinute, &minBegin, &minBeginInv, result);
                            if (foundRange) {
                                CFDateRef tempSearchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, minBeginInv*-1, minBegin);
                                CFRelease(minBegin);
                                searchStartMin = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMinute, tempSearchDate);
                                CFRelease(result);
                                result = tempSearchDate;
                            }
                        } while (searchStartMin > minute);
                    }
                }
            }
        }
    } else {
        // When the search date matches the second we're looking for, we need to clear the lower components in case they are not part of the components we're looking for.
        Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitSecond, &secondBegin, &secInv, result);
        if (foundRange) {
            CFRelease(result);
            result = secondBegin;
            /* Now searchStartDate <= startDate */
        }
    }
    
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateDateAfterDateMatchingNanosecond(CFCalendarRef calendar, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards) {
    const CFIndex nanosecond = CFDateComponentsGetValue(comps, kCFCalendarUnitNanosecond);
    if (nanosecond == CFDateComponentUndefined) return NULL;

    // This taken directly from the old algorithm.  We don't have great support for nanoseconds in general and trying to treat them like seconds causes a hang. :-/
    // Also see <rdar://problem/30229247> NSCalendar -dateFromComponents: doesn't correctly set nanoseconds
    CFDateComponentsRef dateComp = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitEra|kCFCalendarUnitYear|kCFCalendarUnitMonth|kCFCalendarUnitDay|kCFCalendarUnitHour|kCFCalendarUnitMinute|kCFCalendarUnitSecond, startDate);
    CFDateComponentsSetValue(dateComp, kCFCalendarUnitNanosecond, nanosecond);
    CFDateRef result = CFCalendarCreateDateFromComponents(kCFAllocatorSystemDefault, calendar, dateComp);
    CFRelease(dateComp);
    return result;
}

static CFDateRef _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(CFCalendarRef calendar, Boolean *success, CFDateRef startDate, CFDateComponentsRef comps, Boolean goBackwards, Boolean findLastMatch, CFOptionFlags options) {

    const Boolean isStrictMatching = (options & kCFCalendarMatchStrictly) == kCFCalendarMatchStrictly;

    // Default answer for success is yes
    *success = true;
    
    // This is updated as we do our search
    CFDateRef searchStartDate = CFRetain(startDate);
    CFDateRef tempDate = NULL;

    tempDate = _CFCalendarCreateDateAfterDateMatchingEra(calendar, searchStartDate, comps, goBackwards, success);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }
    
    tempDate = _CFCalendarCreateDateAfterDateMatchingYear(calendar, searchStartDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }
    
    tempDate = _CFCalendarCreateDateAfterDateMatchingYearForWeekOfYear(calendar, searchStartDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }
    
    tempDate = _CFCalendarCreateDateAfterDateMatchingQuarter(calendar, searchStartDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }

    tempDate = _CFCalendarCreateDateAfterDateMatchingWeekOfYear(calendar, searchStartDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }

    tempDate = _CFCalendarCreateDateAfterDateMatchingMonth(calendar, searchStartDate, comps, goBackwards, isStrictMatching);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }

    tempDate = _CFCalendarCreateDateAfterDateMatchingWeekOfMonth(calendar, searchStartDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }

    tempDate = _CFCalendarCreateDateAfterDateMatchingWeekdayOrdinal(calendar, searchStartDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }
    
    tempDate = _CFCalendarCreateDateAfterDateMatchingWeekday(calendar, searchStartDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }
    
    tempDate = _CFCalendarCreateDateAfterDateMatchingDay(calendar, searchStartDate, startDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }

    tempDate = _CFCalendarCreateDateAfterDateMatchingHour(calendar, searchStartDate, startDate, comps, goBackwards, findLastMatch, isStrictMatching, options);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }

    tempDate = _CFCalendarCreateDateAfterDateMatchingMinute(calendar, searchStartDate, startDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }

    tempDate = _CFCalendarCreateDateAfterDateMatchingSecond(calendar, searchStartDate, startDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }

    tempDate = _CFCalendarCreateDateAfterDateMatchingNanosecond(calendar, searchStartDate, comps, goBackwards);
    if (tempDate) { CFRelease(searchStartDate); searchStartDate = tempDate; }
    
    *success = true;
    return searchStartDate;
}

#pragma mark -

static CFCalendarUnit _CFCalendarFindHighestSetUnitInDateComponents(CFDateComponentsRef comps) {
    CFIndex era = CFDateComponentsGetValue(comps, kCFCalendarUnitEra);
    CFIndex year = CFDateComponentsGetValue(comps, kCFCalendarUnitYear);
    CFIndex quarter = CFDateComponentsGetValue(comps, kCFCalendarUnitQuarter);
    CFIndex month = CFDateComponentsGetValue(comps, kCFCalendarUnitMonth);
    CFIndex day = CFDateComponentsGetValue(comps, kCFCalendarUnitDay);
    CFIndex hour = CFDateComponentsGetValue(comps, kCFCalendarUnitHour);
    CFIndex minute = CFDateComponentsGetValue(comps, kCFCalendarUnitMinute);
    CFIndex second = CFDateComponentsGetValue(comps, kCFCalendarUnitSecond);
    CFIndex weekday = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekday);
    CFIndex weekdayordinal = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekdayOrdinal);
    CFIndex weekOfMonth = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekOfMonth);
    CFIndex weekOfYear = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekOfYear);
    CFIndex yearForWeekOfYear = CFDateComponentsGetValue(comps, kCFCalendarUnitYearForWeekOfYear);
    CFIndex nanosecond = CFDateComponentsGetValue(comps, kCFCalendarUnitNanosecond);
    CFIndex compsValues[NUM_CALENDAR_UNITS] = {era, year, quarter, month, day, hour, minute, second, weekday, weekdayordinal, weekOfMonth, weekOfYear, yearForWeekOfYear, nanosecond};

    CFCalendarUnit highestMatchUnit = kCFNotFound;
    for (CFIndex i = 0; i < NUM_CALENDAR_UNITS; i++) {
        if (compsValues[i] != CFDateComponentUndefined) {
            highestMatchUnit = calendarUnits[i];
            break;
        }
    }

    return highestMatchUnit;
}

static CFCalendarUnit _CFCalendarFindLowestSetUnitInDateComponents(CFDateComponentsRef comps) {
    CFIndex era = CFDateComponentsGetValue(comps, kCFCalendarUnitEra);
    CFIndex year = CFDateComponentsGetValue(comps, kCFCalendarUnitYear);
    CFIndex quarter = CFDateComponentsGetValue(comps, kCFCalendarUnitQuarter);
    CFIndex month = CFDateComponentsGetValue(comps, kCFCalendarUnitMonth);
    CFIndex day = CFDateComponentsGetValue(comps, kCFCalendarUnitDay);
    CFIndex hour = CFDateComponentsGetValue(comps, kCFCalendarUnitHour);
    CFIndex minute = CFDateComponentsGetValue(comps, kCFCalendarUnitMinute);
    CFIndex second = CFDateComponentsGetValue(comps, kCFCalendarUnitSecond);
    CFIndex weekday = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekday);
    CFIndex weekdayordinal = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekdayOrdinal);
    CFIndex weekOfMonth = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekOfMonth);
    CFIndex weekOfYear = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekOfYear);
    CFIndex yearForWeekOfYear = CFDateComponentsGetValue(comps, kCFCalendarUnitYearForWeekOfYear);
    CFIndex nanosecond = CFDateComponentsGetValue(comps, kCFCalendarUnitNanosecond);
    CFIndex compsValues[NUM_CALENDAR_UNITS] = {era, year, quarter, month, day, hour, minute, second, weekday, weekdayordinal, weekOfMonth, weekOfYear, yearForWeekOfYear, nanosecond};

    CFCalendarUnit lowestMatchUnit = kCFNotFound;
    for (CFIndex i = NUM_CALENDAR_UNITS - 1; i >= 0; i--) {
        if (compsValues[i] != CFDateComponentUndefined) {
            lowestMatchUnit = calendarUnits[i];
            break;
        }
    }

    return lowestMatchUnit;
}

static Boolean _CFCalendarVerifyCalendarOptions(CFOptionFlags options) {
    Boolean optionsAreValid = true;
    CFOptionFlags matchStrictly = options & kCFCalendarMatchStrictly;
    CFOptionFlags matchPrevious = options & kCFCalendarMatchPreviousTimePreservingSmallerUnits;
    CFOptionFlags matchNextKeepSmaller = options & kCFCalendarMatchNextTimePreservingSmallerUnits;
    CFOptionFlags matchNext = options & kCFCalendarMatchNextTime;
    CFOptionFlags matchFirst = options & kCFCalendarMatchFirst;
    CFOptionFlags matchLast = options & kCFCalendarMatchLast;

    if (matchStrictly && (matchPrevious | matchNextKeepSmaller | matchNext)) {
        // We can't throw here because we've never thrown on this case before, even though it is technically an invalid case.  The next best thing is to return.
        optionsAreValid = false;
    }

    if (!matchStrictly) {
        if ((matchPrevious && matchNext) || (matchPrevious && matchNextKeepSmaller) || (matchNext && matchNextKeepSmaller) || (!matchPrevious && !matchNext && !matchNextKeepSmaller)) {
            optionsAreValid = false;
        }
    }

    if (matchFirst && matchLast) {
        optionsAreValid = false;
    }

    return optionsAreValid;
}

static Boolean _CFCalendarVerifyCFDateComponentsValues(CFCalendarRef calendar, CFDateComponentsRef comps) {
    Boolean dcValuesAreValid = true;
    CFIndex era = CFDateComponentsGetValue(comps, kCFCalendarUnitEra);
    CFIndex year = CFDateComponentsGetValue(comps, kCFCalendarUnitYear);
    CFIndex quarter = CFDateComponentsGetValue(comps, kCFCalendarUnitQuarter);
    CFIndex month = CFDateComponentsGetValue(comps, kCFCalendarUnitMonth);
    CFIndex day = CFDateComponentsGetValue(comps, kCFCalendarUnitDay);
    CFIndex hour = CFDateComponentsGetValue(comps, kCFCalendarUnitHour);
    CFIndex minute = CFDateComponentsGetValue(comps, kCFCalendarUnitMinute);
    CFIndex second = CFDateComponentsGetValue(comps, kCFCalendarUnitSecond);
    CFIndex weekday = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekday);
    CFIndex weekdayordinal = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekdayOrdinal);
    CFIndex weekOfMonth = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekOfMonth);
    CFIndex weekOfYear = CFDateComponentsGetValue(comps, kCFCalendarUnitWeekOfYear);
    CFIndex yearForWeekOfYear = CFDateComponentsGetValue(comps, kCFCalendarUnitYearForWeekOfYear);
    CFIndex nanosecond = CFDateComponentsGetValue(comps, kCFCalendarUnitNanosecond);
    CFIndex compsValues[NUM_CALENDAR_UNITS] = {era, year, quarter, month, day, hour, minute, second, weekday, weekdayordinal, weekOfMonth, weekOfYear, yearForWeekOfYear, nanosecond};
    
    // Verify the values fall in the correct range for their corresponding type
    CFRange max, min;
    Boolean compHasAtLeastOneFieldSet = false;
    CFStringRef calID = CFCalendarGetIdentifier(calendar);
    Boolean calIDIsHebrewIndianOrPersian =
        CFEqual(calID, kCFCalendarIdentifierHebrew) ||
        CFEqual(calID, kCFCalendarIdentifierIndian) ||
        CFEqual(calID, kCFCalendarIdentifierPersian);
    for (CFIndex i = 0; i < NUM_CALENDAR_UNITS; i++) {
        CFIndex currentCompVal = compsValues[i];
        if (currentCompVal != CFDateComponentUndefined) {
            compHasAtLeastOneFieldSet = true;
            CFCalendarUnit currentUnit = calendarUnits[i];
            if (currentUnit == kCFCalendarUnitWeekdayOrdinal) {
                max.length = 6;
                max.location = 1;
                min.length = 4;
                min.location = 1;
            } else if ((currentUnit == kCFCalendarUnitYear || currentUnit == kCFCalendarUnitYearForWeekOfYear) && calIDIsHebrewIndianOrPersian) {
                max = CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitYear);
                min = CFCalendarGetMinimumRangeOfUnit(calendar, kCFCalendarUnitYear);
                max.location = min.location = 1;
            } else if (currentUnit == kCFCalendarUnitYearForWeekOfYear) {
                max = CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitYear);
                min = CFCalendarGetMinimumRangeOfUnit(calendar, kCFCalendarUnitYear);
            } else {
                max = CFCalendarGetMaximumRangeOfUnit(calendar, currentUnit);
                min = CFCalendarGetMinimumRangeOfUnit(calendar, currentUnit);
            }
            if (currentCompVal > max.location+max.length-1 || currentCompVal < min.location) {
                dcValuesAreValid = false;
                break;
            }
        }
    }
    
    if ((false == compHasAtLeastOneFieldSet) && !CFDateComponentsIsLeapMonth(comps)) {
        dcValuesAreValid = false;
    }

    return dcValuesAreValid;
}

static CFDateRef _CFCalendarCreateMatchingDateAfterStartDateMatchingComponentsInNextHighestUnitRange(CFCalendarRef calendar, Boolean *foundEra, CFDateComponentsRef comps, CFCalendarUnit nextHighestUnit, CFDateRef startDate, Boolean goBackwards, Boolean findLast, CFOptionFlags options) {
    CFDateRef startOfCurrentRangeForUnit = NULL;
    CFTimeInterval invOfCurrentRangeForUnit = 0.0;
    Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, nextHighestUnit, &startOfCurrentRangeForUnit, &invOfCurrentRangeForUnit, startDate);
    if (foundRange) {
        CFDateRef nextSearchDate = NULL;
        if (goBackwards) {
            if (nextHighestUnit == kCFCalendarUnitDay) {
                /*
                 If nextHighestUnit is kCFCalendarUnitDay, it's a safe assumption that the highest actual set unit is the hour.
                 There are cases where we're looking for a minute and/or second within the first hour of the day. If we start just at the top of the day and go backwards, we could end up missing the minute/second we're looking for.
                 E.g.
                 We're looking for { hour: 0, minute: 30, second: 0 } in the day before the start date 2017-05-26 07:19:50 UTC. At this point, startOfCurrentRangeForUnit would be 2017-05-26 07:00:00 UTC.
                 In this case, the algorithm would do the following:
                     start at 2017-05-26 07:00:00 UTC, see that the hour is already set to what we want, jump to minute.
                     when checking for minute, it will cycle forward to 2017-05-26 07:30:00 +0000 but then compare to the start and see that that date is incorrect because it's in the future. Then it will cycle the date back to 2017-05-26 06:30:00 +0000.
                     the __findMatchingDate: call below will exit with 2017-05-26 06:30:00 UTC and the algorithm will see that date is incorrect and reset the new search date go back a day to 2017-05-25 07:19:50 UTC. Then we get back here to this method and move the start to 2017-05-25 07:00:00 UTC and the call to __findMatchingDate: below will return 2017-05-25 06:30:00 UTC, which skips what we want (2017-05-25 07:30:00 UTC) and the algorithm eventually keeps moving further and further into the past until it exhausts itself and returns nil.
                 To adjust for this scenario, we add this line below that sets nextSearchDate to the last minute of the previous day (using the above example, 2017-05-26 06:59:59 UTC), which causes the algorithm to not skip the minutes/seconds within the first hour of the previous day.
                 Radar that revealed this bug: // <rdar://problem/32609242> NSCalendar nextDateAfterDate:matchingHour:minute:second:options: unexpectedly returning nil when searching backwards
                 */
                nextSearchDate = CFDateCreate(kCFAllocatorSystemDefault, CFDateGetAbsoluteTime(startOfCurrentRangeForUnit) - 1);

                /*
                 One caveat: if we are looking for a date within the first hour of the day (i.e. between 12 and 1 am), we want to ensure we go forwards in time to hit the exact minute and/or second we're looking for since nextSearchDate is now in the previous day.
                 Associated radar: <rdar://problem/33944890> iOS 11 beta 5 and beta 6: Wrong backward search with -[NSCalendar nextDateAfterDate:matchingHour:minute:second:options:]
                 */
                if (0 == CFDateComponentsGetValue(comps, kCFCalendarUnitHour)) {
                    goBackwards = false;
                }
            } else {
                nextSearchDate = CFRetain(startOfCurrentRangeForUnit);
            }
        } else {
            nextSearchDate = CFDateCreate(kCFAllocatorSystemDefault, CFDateGetAbsoluteTime(startOfCurrentRangeForUnit) + invOfCurrentRangeForUnit);
        }
        CFRelease(startOfCurrentRangeForUnit);
        CFDateRef result = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, foundEra, nextSearchDate, comps, goBackwards, findLast, options);
        CFRelease(nextSearchDate);
        return result;
    }
    
    *foundEra = true;
    return NULL;
}

static CFDateComponentsRef _CFCalendarCreateAdjustedComponents(CFCalendarRef calendar, CFDateComponentsRef comps, CFDateRef date, Boolean goBackwards) {
    // formerly known as "_CFCalendarEnsureThoroughEnumerationByAdjustingComponents"
    // This method ensures that the algorithm enumerates through each year or month if they are not explicitly set in the NSDateComponents object passed into enumerateDatesStartingAfterDate:matchingComponents:options:usingBlock:.  This only applies to cases where the highest set unit is month or day (at least for now).  For full in context explanation, see where it gets called in enumerateDatesStartingAfterDate:matchingComponents:options:usingBlock:.
    CFCalendarUnit highestSetUnit = _CFCalendarFindHighestSetUnitInDateComponents(comps);
    switch (highestSetUnit) {
        case kCFCalendarUnitMonth: {
            CFDateComponentsRef adjustedComps = CFDateComponentsCreateCopy(kCFAllocatorSystemDefault, comps);
            CFDateComponentsSetValue(adjustedComps, kCFCalendarUnitYear, CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitYear, date));
            CFDateRef adjustedDate = CFCalendarCreateDateFromComponents(kCFAllocatorSystemDefault, calendar, adjustedComps);
            if (adjustedDate) {
                CFComparisonResult order = CFDateCompare(date, adjustedDate, NULL);
                if (!goBackwards && (order == kCFCompareGreaterThan)) {
                    CFDateComponentsSetValue(adjustedComps, kCFCalendarUnitYear, CFDateComponentsGetValue(adjustedComps, kCFCalendarUnitYear) + 1);
                } else if (goBackwards && (order == kCFCompareLessThan)) {
                    CFDateComponentsSetValue(adjustedComps, kCFCalendarUnitYear, CFDateComponentsGetValue(adjustedComps, kCFCalendarUnitYear) - 1);
                }
                CFRelease(adjustedDate);
            }
            return adjustedComps;
        }
        case kCFCalendarUnitDay: {
            CFDateComponentsRef adjustedComps = CFDateComponentsCreateCopy(kCFAllocatorSystemDefault, comps);
            if (goBackwards) {
                // We need to make sure we don't surpass the day we want
                CFIndex dateDay = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitDay, date);
                if (CFDateComponentsGetValue(comps, kCFCalendarUnitDay) >= dateDay) {
                    CFDateRef tempDate = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, -1, kCFCalendarUnitMonth, date);
                    CFDateComponentsSetValue(adjustedComps, kCFCalendarUnitMonth, CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, tempDate));
                    CFRelease(tempDate);
                } else {
                    // adjustedComps here are the date components we're trying to match against; dateDay is the current day of the current search date.
                    // See the comment in enumerateDatesStartingAfterDate:matchingComponents:options:usingBlock: for the justification for adding the month to the components here.
                    //
                    // However, we can't unconditionally add the current month to these components. If the current search date is on month M and day D, and the components we're trying to match have day D' set, the resultant date components to match against are {day=D', month=M}.
                    // This is only correct sometimes:
                    //
                    //  * If D' > D (e.g. we're on Nov 05, and trying to find the next 15th of the month), then it's okay to try to match Nov 15.
                    //  * However, if D' <= D (e.g. we're on Nov 05, and are trying to find the next 2nd of the month), then it's not okay to try to match Nov 02.
                    //
                    // We can only adjust the month if it won't cause us to search "backwards" in time (causing us to elsewhere end up skipping the first [correct] match we find).
                    // These same changes apply to the goBackwards case above.
                    CFIndex dateDay = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, date);
                    CFDateComponentsSetValue(adjustedComps, kCFCalendarUnitMonth, dateDay);
                }
            } else {
                CFIndex dateDay = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitDay, date);
                if (CFDateComponentsGetValue(comps, kCFCalendarUnitDay) > dateDay) {
                    CFDateComponentsSetValue(adjustedComps, kCFCalendarUnitMonth, CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, date));
                }
            }
            return adjustedComps;
        }
        default:
            return (CFDateComponentsRef)CFRetain(comps);
    }
}

static CFDateRef _Nullable _CFCalendarCreateBumpedDateUpToNextHigherUnitInComponents(CFCalendarRef calendar, CFDateRef searchingDate, CFDateComponentsRef comps, Boolean goBackwards, CFDateRef matchDate) {
    CFCalendarUnit highestSetUnit = _CFCalendarFindHighestSetUnitInDateComponents(comps);
    CFCalendarUnit nextUnitAboveHighestSet = _CFCalendarNextHigherUnit(highestSetUnit);
    if (highestSetUnit == kCFCalendarUnitEra) {
        nextUnitAboveHighestSet = kCFCalendarUnitYear;
    } else if (highestSetUnit == kCFCalendarUnitYear || highestSetUnit == kCFCalendarUnitYearForWeekOfYear) {
        nextUnitAboveHighestSet = highestSetUnit;
    }
    
    if (nextUnitAboveHighestSet == kCFNotFound) {
        // _CFCalendarNextHigherUnit() can return kCFNotFound if bailedUnit was somehow a deprecated unit or something
        return NULL;
    }

    // Advance to the start or end of the next highest unit. Old code here used to add `±1 nextUnitAboveHighestSet` to searchingDate and manually adjust afterwards, but this is incorrect in many cases.
    // For instance, this is wrong when searching forward looking for a specific Week of Month. Take for example, searching for WoM == 1:
    //
    //           January 2018           February 2018
    //       Su Mo Tu We Th Fr Sa    Su Mo Tu We Th Fr Sa
    //  W1       1  2  3  4  5  6                 1  2  3
    //  W2    7  8  9 10 11 12 13     4  5  6  7  8  9 10
    //  W3   14 15 16 17 18 19 20    11 12 13 14 15 16 17
    //  W4   21 22 23 24 25 26 27    18 19 20 21 22 23 24
    //  W5   28 29 30 31             25 26 27 28
    //
    // Consider searching for `WoM == 1` when searchingDate is *in* W1 of January. Because we're looking to advance to next month, we could simply add a month, right?
    // Adding a month from Monday, January 1st lands us on Thursday, February 1st; from Tuesday, January 2nd we get Friday, February 2nd, etc. Note though that for January 4th, 5th, and 6th, adding a month lands us in **W2** of February!
    // This means that if we continue searching forward from there, we'll have completely skipped W1 of February as a candidate week, and search forward until we hit W1 of March. This is incorrect.
    //
    // What we really want is to skip to the _start_ of February and search from there -- if we undershoot, we can always keep looking.
    // Searching backwards is similar: we can overshoot if we were subtracting a month, so instead we want to jump back to the very end of the previous month.
    // In general, this translates to jumping to the very beginning of the next period of the next highest unit when searching forward, or jumping to the very end of the last period when searching backward.
    CFDateRef result = NULL;
    CFTimeInterval period = 0.0;
    Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, nextUnitAboveHighestSet, &result, &period, searchingDate);
    if (foundRange) {
        // Skip to the start of the next period (the beginning of this period + its length), or go to the end of the last one (the beginning of this period -1 instant).
        CFDateRef adjusted = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, (goBackwards ? -1 : period), result);
        CFRelease(result);
        result = adjusted;
    }

    if (matchDate) {
        CFComparisonResult ordering = CFDateCompare(matchDate, result, NULL);
        if (((ordering != kCFCompareLessThan) && !goBackwards) || ((ordering != kCFCompareGreaterThan) && goBackwards)) {
            // We need to advance searchingDate so that it starts just after matchDate
            CFCalendarUnit lowestSetUnit = _CFCalendarFindLowestSetUnitInDateComponents(comps);
            CFRelease(result);
            result = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, (goBackwards ? -1 : 1), lowestSetUnit, matchDate);
        }
    }
    
    return result;
}

static void _CFCalendarPreserveSmallerUnits(CFCalendarRef calendar, CFDateRef date, CFDateComponentsRef compsToMatch, CFDateComponentsRef compsToModify) {
    CFDateComponentsRef smallerUnits = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitHour|kCFCalendarUnitMinute|kCFCalendarUnitSecond, date);
    // Either preserve the units we're trying to match if they are explicitly defined or preserve the hour/min/sec in the date.
    
    CFDateComponentsSetValue(compsToModify, kCFCalendarUnitHour, CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitHour) != CFDateComponentUndefined ? CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitHour) : CFDateComponentsGetValue(smallerUnits, kCFCalendarUnitHour));
    CFDateComponentsSetValue(compsToModify, kCFCalendarUnitMinute, CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitMinute) != CFDateComponentUndefined ? CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitMinute) : CFDateComponentsGetValue(smallerUnits, kCFCalendarUnitMinute));
    CFDateComponentsSetValue(compsToModify, kCFCalendarUnitSecond, CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitSecond) != CFDateComponentUndefined ? CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitSecond) : CFDateComponentsGetValue(smallerUnits, kCFCalendarUnitSecond));
    
    CFRelease(smallerUnits);
}

#pragma mark -

// This function adjusts a mismatched data in the case where it is the chinese calendar and we have detected a leap month mismatch.
// It will return NULL in the case where we could not find an appropriate adjustment. In that case, the algorithm should keep iterating.
static CFDateRef _Nullable _CFCalendarCreateAdjustedDateForMismatchedChineseLeapMonth(CFCalendarRef calendar, CFDateRef start, CFDateRef searchingDate, CFDateRef matchDate, CFDateComponentsRef matchingComponents, CFDateComponentsRef compsToMatch, CFOptionFlags opts, Boolean *exactMatch, Boolean *isLeapDay) {

    const Boolean goBackwards = (opts & kCFCalendarSearchBackwards) == kCFCalendarSearchBackwards;
    const Boolean findLast = (opts & kCFCalendarMatchLast) == kCFCalendarMatchLast;
    const Boolean strictMatching = (opts & kCFCalendarMatchStrictly) == kCFCalendarMatchStrictly;
    const Boolean goToNextExistingWithSmallerUnits = (opts & kCFCalendarMatchNextTimePreservingSmallerUnits) == kCFCalendarMatchNextTimePreservingSmallerUnits;
    const Boolean goToNextExisting = (opts & kCFCalendarMatchNextTime) == kCFCalendarMatchNextTime;

    // We are now going to look for the month that precedes the leap month we're looking for.
    CFDateComponentsRef matchDateComps = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitEra|kCFCalendarUnitYear|kCFCalendarUnitMonth|kCFCalendarUnitDay, matchDate);
    const Boolean isMatchLeapMonthSet = CFDateComponentsIsLeapMonthSet(matchDateComps);
    const Boolean isMatchLeapMonth = CFDateComponentsIsLeapMonth(matchDateComps);
    CFRelease(matchDateComps);
    
    const Boolean isDesiredLeapMonthSet = CFDateComponentsIsLeapMonthSet(matchingComponents);
    const Boolean isDesiredLeapMonth = CFDateComponentsIsLeapMonth(matchingComponents);
    if (!(isMatchLeapMonthSet && !isMatchLeapMonth && isDesiredLeapMonthSet && isDesiredLeapMonth)) {
        // Not one of the things we adjust for
        return CFRetain(matchDate);
    }
    
    // Not an exact match after this point
    *exactMatch = false;

    CFDateRef result = CFRetain(matchDate);

    CFDateComponentsRef compsCopy = CFDateComponentsCreateCopy(kCFAllocatorSystemDefault, compsToMatch);
    CFDateComponentsSetValue(compsCopy, kCFCalendarUnitLeapMonth, 0);
    
    // See if tempMatchDate is already the preceding non-leap month.
    CFCalendarUnit mismatchedUnits = 0;
    Boolean dateMatchesComps = _CFCalendarCheckDateContainsMatchingComponents(calendar, result, compsCopy, &mismatchedUnits);
    if (!dateMatchesComps) {
        // tempMatchDate was not the preceding non-leap month so now we try to find it.
        Boolean success;
        CFDateRef nonLeapStart = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, &success, searchingDate, compsCopy, goBackwards, findLast, opts);
        dateMatchesComps = _CFCalendarCheckDateContainsMatchingComponents(calendar, nonLeapStart, compsCopy, &mismatchedUnits);
        if (!dateMatchesComps || !success) {
            // Bail if we can't even find the preceding month.  Returning NULL allows the alg to keep iterating until we either eventually find another match and caller says stop or we hit our max number of iterations and give up.
            CFRelease(result);
            CFRelease(nonLeapStart);
            result = NULL;
        } else {
            CFRelease(result);
            result = nonLeapStart;
        }
    }
    
    if (!dateMatchesComps || !result) {
        CFRelease(compsCopy);
        return result;
    }

    // We have the non-leap month so now we check to see if the month following is a leap month.
    _CFReleaseDeferred CFDateRef nonLeapMonthBegin = NULL;
    CFTimeInterval nonLeapInv = 0.0;
    Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitMonth, &nonLeapMonthBegin, &nonLeapInv, result);
    if (!foundRange) {
        CFRelease(compsCopy);
        return result;
    }

    CFDateComponentsSetValue(compsCopy, kCFCalendarUnitLeapMonth, 1);
    CFDateRef beginMonthAfterNonLeap = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, nonLeapInv, nonLeapMonthBegin);
    // Now we see if we find the date we want in what we hope is the leap month.
    Boolean success;
    CFDateRef possibleLeapDateMatch = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, &success, beginMonthAfterNonLeap, compsCopy, goBackwards, findLast, opts);
    dateMatchesComps = _CFCalendarCheckDateContainsMatchingComponents(calendar, possibleLeapDateMatch, compsCopy, &mismatchedUnits);
    if (dateMatchesComps) {
        // Hooray! It was a leap month and we found the date we wanted!
        CFRelease(result);
        CFRelease(compsCopy);
        CFRelease(beginMonthAfterNonLeap);
        return possibleLeapDateMatch;
    }
    
    // Either the month wasn't a leap month OR we couldn't find the date we wanted (e.g. the requested date is a bogus nonexistent one).
    CFRelease(possibleLeapDateMatch);
    
    if (strictMatching) {
        CFRelease(result);
        CFRelease(compsCopy);
        CFRelease(beginMonthAfterNonLeap);
        return NULL; // We give up, we couldn't find what we needed.
    }
    
    // We approximate.
    /*
     Two things we need to test for here. Either
     (a) beginMonthAfterNonLeap is a leap month but the date we're looking for doesn't exist (e.g. looking for the 30th day in a 29-day month) OR
     (b) beginMonthAfterNonLeap is not a leap month OR
     
     The reason we need to test for each separately is because they get handled differently.
     For (a): beginMonthAfterNonLeap IS a leap month BUT we can't find the date we want
     PreviousTime - Last day of this month (beginMonthAfterNonLeap) preserving smaller units
     NextTimePreserving - First day of following month (month after beginMonthAfterNonLeap) preserving smaller units
     Nextime - First day of following month (month after beginMonthAfterNonLeap) at the beginning of the day
     
     For (b): beginMonthAfterNonLeap is NOT a leap month
     PreviousTime - The day we want in the previous month (nonLeapMonthBegin) preserving smaller units
     NextTimePreserving - First day of this month (beginMonthAfterNonLeap) preserving smaller units
     Nextime - First day of this month (beginMonthAfterNonLeap)
     */
    if (_CFCalendarCheckIfLeapMonthHack(calendar, beginMonthAfterNonLeap)) {  // (a)
        if (goToNextExisting) {
            // We want the beginning of the next month
            CFDateRef begOfMonth = NULL;
            CFTimeInterval monthInv = 0.0;
            foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitMonth, &begOfMonth, &monthInv, beginMonthAfterNonLeap);
            if (foundRange) {
                CFRelease(result);
                result = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, monthInv, begOfMonth);
                CFRelease(begOfMonth);
            }
        } else {
            CFDateRef dateToUse = NULL;
            // Make sure we have the same hour/min/sec as the start date to preserve the smaller units
            _CFCalendarPreserveSmallerUnits(calendar, start, compsToMatch, compsCopy);
            if (goToNextExistingWithSmallerUnits) {
                // Make sure this is set to false
                CFDateComponentsSetValue(compsCopy, kCFCalendarUnitLeapMonth, 0);
                CFDateComponentsSetValue(compsCopy, kCFCalendarUnitDay, 1);
                // We want the first day of the next month so we need to advance the date a bit and make sure we're not on day 1 of the current month.
                CFDateRef throwAwayDate = NULL;
                CFTimeInterval dayInv = 0.0;
                foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitDay, &throwAwayDate, &dayInv, beginMonthAfterNonLeap);
                if (foundRange) {
                    CFDateRef nextDay = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, dayInv, throwAwayDate);
                    dateToUse = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, &success, nextDay, compsCopy, false, findLast, opts);
                    CFRelease(nextDay);
                    CFRelease(throwAwayDate);
                }
            } else { // MatchPreviousPreservingSmallerUnits
                CFDateRef begOfMonth = NULL;
                CFTimeInterval monthInv = 0.0;
                foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitMonth, &begOfMonth, &monthInv, beginMonthAfterNonLeap);
                if (foundRange) {
                    CFDateRef lastDayEnd = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, monthInv-1, begOfMonth);
                    CFDateComponentsRef monthDayComps = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitMonth|kCFCalendarUnitDay, lastDayEnd);
                    CFDateComponentsSetValue(compsCopy, kCFCalendarUnitMonth, CFDateComponentsGetValue(monthDayComps, kCFCalendarUnitMonth));
                    CFDateComponentsSetValue(compsCopy, kCFCalendarUnitDay, CFDateComponentsGetValue(monthDayComps, kCFCalendarUnitDay));
                    CFRelease(monthDayComps);
                    // Make sure this is set to true
                    CFDateComponentsSetValue(compsCopy, kCFCalendarUnitLeapMonth, 1);
                    dateToUse = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, &success, lastDayEnd, compsCopy, true, findLast, opts);
                    CFRelease(lastDayEnd);
                    CFRelease(begOfMonth);
                }
            }
            if (dateToUse) {
                // We have to give a date since we can't return NULL so whatever we get back, we go with. Hopefully it's what we want.
                // tempMatchDate was already set to matchDate so it shouldn't be NULL here anyway.
                CFRelease(result);
                result = dateToUse;
            }
        }
    } else {  // (b)
        if (goToNextExisting) {
            // We need first day of this month and we don't care about preserving the smaller units.
            CFRelease(result);
            result = CFRetain(beginMonthAfterNonLeap);
        } else {
            // Make sure this is set to false
            CFDateComponentsSetValue(compsCopy, kCFCalendarUnitLeapMonth, 0);
            // Make sure we have the same hour/min/sec as the start date to preserve the smaller units
            _CFCalendarPreserveSmallerUnits(calendar, start, compsToMatch, compsCopy);
            CFDateRef dateToUse = NULL;
            if (goToNextExistingWithSmallerUnits) {
                // We need first day of this month but we need to preserve the smaller units.
                CFDateComponentsSetValue(compsCopy, kCFCalendarUnitMonth, CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, beginMonthAfterNonLeap));
                CFDateComponentsSetValue(compsCopy, kCFCalendarUnitDay, 1);
                dateToUse = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, &success, beginMonthAfterNonLeap, compsCopy, false, findLast, opts);
            } else {  // MatchPreviousPreservingSmallerUnits
                // compsCopy is already set to what we're looking for, which is the date we want in the previous non-leap month. This also preserves the smaller units.
                dateToUse = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, &success, nonLeapMonthBegin, compsCopy, false, findLast, opts);
            }
            if (dateToUse) {
                // We have to give a date since we can't return NULL so whatever we get back, we go with. Hopefully it's what we want.
                // tempMatchDate was already set to matchDate so it shouldn't be NULL here anyway.
                CFRelease(result);
                result = dateToUse;
            }
        }
    }
    
    // Even though we have an approximate date here, we still count it as a substitute for the leap date we were hoping to find.
    *isLeapDay = true;
    
    CFRelease(beginMonthAfterNonLeap);
    CFRelease(compsCopy);
    return result;
}

// For calendars other than Chinese
static CFDateRef _Nullable _CFCalendarCreateAdjustedDateForMismatchedLeapMonthOrDay(CFCalendarRef calendar, CFDateRef start, CFDateRef searchingDate, CFDateRef matchDate, CFDateComponentsRef matchingComponents, CFDateComponentsRef compsToMatch, CFCalendarUnit nextHighestUnit, CFOptionFlags opts, Boolean *exactMatch, Boolean *isLeapDay) {
    
    const CFDateComponentsRef searchDateComps = CFCalendarCreateDateComponentsFromDate(kCFAllocatorSystemDefault, calendar, kCFCalendarUnitYear|kCFCalendarUnitMonth|kCFCalendarUnitDay, searchingDate);
    const CFIndex searchDateDay = CFDateComponentsGetValue(searchDateComps, kCFCalendarUnitDay);
    const CFIndex searchDateMonth = CFDateComponentsGetValue(searchDateComps, kCFCalendarUnitMonth);
    const CFIndex searchDateYear = CFDateComponentsGetValue(searchDateComps, kCFCalendarUnitYear);
    CFRelease(searchDateComps);
    
    const CFIndex desiredMonth = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitMonth);
    const CFIndex desiredDay = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitDay);

    // if comps aren't equal, it means we jumped to a day that doesn't exist in that year (non-leap year) i.e. we've detected a leap year situation
    Boolean detectedLeapYearSituation = (((desiredDay != CFDateComponentUndefined) && (searchDateDay != desiredDay)) ||
                                         ((desiredMonth != CFDateComponentUndefined) && (searchDateMonth != desiredMonth)));
    if (!detectedLeapYearSituation) {
        // Nothing to do here
        return NULL;
    }
    
    const Boolean goBackwards = (opts & kCFCalendarSearchBackwards) == kCFCalendarSearchBackwards;
    const Boolean findLast = (opts & kCFCalendarMatchLast) == kCFCalendarMatchLast;
    const Boolean strictMatching = (opts & kCFCalendarMatchStrictly) == kCFCalendarMatchStrictly;
    // NSCalendarMatchPreviousTimePreservingSmallerUnits is implied if neither of these two below nor NSCalendarMatchStrictly are set.
    const Boolean goToNextExistingWithSmallerUnits = (opts & kCFCalendarMatchNextTimePreservingSmallerUnits) == kCFCalendarMatchNextTimePreservingSmallerUnits;
    const Boolean goToNextExisting = (opts & kCFCalendarMatchNextTime) == kCFCalendarMatchNextTime;

    Boolean foundGregLeap = false, foundGregLeapMatchesComps = false;
    const Boolean isGregorianCalendar = CFEqual(CFCalendarGetIdentifier(calendar), kCFCalendarIdentifierGregorian);
    
    CFDateRef result = CFRetain(matchDate);
    
    if (isGregorianCalendar) {
        // We've identified a leap year in the Gregorian calendar OR we've identified a day that doesn't exist in a different month
        // We check the original matchingComponents to check the caller's *intent*. If they're looking for February, then they are indeed looking for a leap year. If they didn't ask for February explicitly and we added it to compsToMatch ourselves, then don't force us to the next leap year.
        if (desiredMonth == 2 && CFDateComponentsGetValue(matchingComponents, kCFCalendarUnitMonth) == 2) {  // Check for Gregorian leap year
            CFIndex amountToAdd = 0;
            if (goBackwards) {
                amountToAdd = (searchDateYear % 4) * -1;
                // It's possible that we're in a leap year but before 2/29.  Since we're going backwards, we need to go to the previous leap year.
                if ((amountToAdd == 0) && (searchDateMonth >= desiredMonth)) {
                    amountToAdd = -4;
                }
            } else {
                amountToAdd = 4 - (searchDateYear % 4);
            }
            CFDateRef searchDateInLeapYear = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, amountToAdd, kCFCalendarUnitYear, searchingDate);
            CFDateRef startOfLeapYear = NULL;
            CFTimeInterval startOFLeapYearInv = 0.0;
            Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitYear, &startOfLeapYear, &startOFLeapYearInv, searchDateInLeapYear);
            if (foundRange) {
                CFRelease(result);
                result = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, &foundGregLeap, startOfLeapYear, compsToMatch, false, findLast, opts);
                CFRelease(startOfLeapYear);
                foundGregLeapMatchesComps = _CFCalendarCheckDateContainsMatchingComponents(calendar, result, compsToMatch, NULL);
            }
            CFRelease(searchDateInLeapYear);
        }
    }
    
    if (!foundGregLeap || !foundGregLeapMatchesComps) {
        if (strictMatching) {
            if (isGregorianCalendar) {
                *exactMatch = false;  // We couldn't find what we needed but we found sumthin. Step C will decide whether or not to NULL the date out.
            } else {
                // For other calendars (besides Chinese which is already being handled), go to the top of the next period for the next highest unit of the one that bailed.
                Boolean eraMatch = false;
                CFDateRef tempDate = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponentsInNextHighestUnitRange(calendar, &eraMatch, matchingComponents, nextHighestUnit, searchingDate, goBackwards, findLast, opts);
                if (!eraMatch) {
                    *exactMatch = false;  // We couldn't find what we needed but we found sumthin. Step C will decide whether or not to NULL the date out.
                    CFRelease(tempDate);
                } else {
                    CFRelease(result);
                    result = tempDate;
                }
            }
        } else {
            // Figure out best approximation to give.  The "correct" approximations for these cases depend on the calendar.
            // Note that this also works for the Hebrew calendar - since the preceding month is not numbered the same as the leap month (like in the Chinese calendar) we can treat the non-existent day in the same way that we handle Feb 29 in the Gregorian calendar.
            CFDateComponentsRef compsCopy = CFDateComponentsCreateCopy(kCFAllocatorSystemDefault, compsToMatch);
            CFDateComponentsRef tempComps = CFDateComponentsCreate(kCFAllocatorSystemDefault);
            CFDateComponentsSetValue(tempComps, kCFCalendarUnitYear, searchDateYear);
            CFDateComponentsSetValue(tempComps, kCFCalendarUnitMonth, desiredMonth);
            CFDateComponentsSetValue(tempComps, kCFCalendarUnitDay, 1);
            CFDateRef tempDate = CFCalendarCreateDateFromComponents(kCFAllocatorSystemDefault, calendar, tempComps);
            if (goToNextExisting) {
                // Go to the start of the next day after the desired month and day
                CFIndex compsToMatchYear = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitYear);
                if (compsToMatchYear != CFDateComponentUndefined) {
                    // If we explicitly set the year to match by calling __ensureThoroughEnumerationByAdjustingComponents:, we should use that year instead and not searchDateYear.
                    CFDateComponentsSetValue(compsCopy, kCFCalendarUnitYear, compsToMatchYear > searchDateYear ? compsToMatchYear : searchDateYear);
                } else {
                    CFDateComponentsSetValue(compsCopy, kCFCalendarUnitYear, searchDateYear);
                }
                CFDateRef followingMonthDate = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, 1, kCFCalendarUnitMonth, tempDate);
                CFDateComponentsSetValue(compsCopy, kCFCalendarUnitMonth, CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, followingMonthDate));
                CFRelease(followingMonthDate);
                CFDateComponentsSetValue(compsCopy, kCFCalendarUnitDay, 1);
                
                Boolean success;
                CFRelease(result);
                result = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, &success, start, compsCopy, goBackwards, findLast, opts);
                Boolean dateMatchesComps = _CFCalendarCheckDateContainsMatchingComponents(calendar, result, compsCopy, NULL);
                if (success && dateMatchesComps) {
                    // Need to go to the start of the day.
                    CFDateRef startOfDay = NULL;
                    CFTimeInterval startOfDayInv = 0.0;
                    Boolean foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitDay, &startOfDay, &startOfDayInv, result);
                    if (foundRange) {
                        CFRelease(result);
                        result = startOfDay;
                    } else {
                        CFRelease(startOfDay);
                    }
                } else {
                    CFRelease(result);
                    result = NULL;
                }
            } else {
                // Make sure we have the same hour/min/sec as the start date to preserve the smaller units
                _CFCalendarPreserveSmallerUnits(calendar, start, compsToMatch, compsCopy);
                if (goToNextExistingWithSmallerUnits) {
                    CFIndex compsToMatchYear = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitYear);
                    if (compsToMatchYear != CFDateComponentUndefined) {
                        // If we explicitly set the year to match by calling __ensureThoroughEnumerationByAdjustingComponents:, we should use that year instead and not searchDateYear.
                        CFDateComponentsSetValue(compsCopy, kCFCalendarUnitYear, compsToMatchYear > searchDateYear ? compsToMatchYear : searchDateYear);
                    } else {
                        CFDateComponentsSetValue(compsCopy, kCFCalendarUnitYear, searchDateYear);
                    }
                    CFDateComponentsSetValue(tempComps, kCFCalendarUnitYear, CFDateComponentsGetValue(compsCopy, kCFCalendarUnitYear));
                    CFRelease(tempDate);
                    tempDate = CFCalendarCreateDateFromComponents(kCFAllocatorSystemDefault, calendar, tempComps);
                    CFDateRef followingMonthDate = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, 1, kCFCalendarUnitMonth, tempDate);
                    CFDateComponentsSetValue(compsCopy, kCFCalendarUnitMonth, CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, followingMonthDate));
                    CFRelease(followingMonthDate);
                    CFDateComponentsSetValue(compsCopy, kCFCalendarUnitDay, 1); // We want the beginning of the next month.
                } else { // MatchPreviousPreservingSmallerUnits
                    CFRange foundRange = CFCalendarGetRangeOfUnit(calendar, kCFCalendarUnitDay, kCFCalendarUnitMonth, CFDateGetAbsoluteTime(tempDate));
                    CFIndex lastDayOfTheMonth = foundRange.length;
                    if (desiredDay >= lastDayOfTheMonth) {
                        CFDateComponentsSetValue(compsCopy, kCFCalendarUnitDay, lastDayOfTheMonth);
                    } else {
                        // Go to the prior day before the desired month
                        CFDateComponentsSetValue(compsCopy, kCFCalendarUnitDay, desiredDay - 1);
                    }
                }
                
                Boolean success;
                CFRelease(result);
                result = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, &success, searchingDate, compsCopy, goBackwards, findLast, opts);
                Boolean dateMatchesComps = _CFCalendarCheckDateContainsMatchingComponents(calendar, result, compsCopy, NULL);
                if (!success || !dateMatchesComps) {
                    // Bail if we couldn't even find an approximate match.
                    CFRelease(result);
                    result = NULL;
                }
            }
            CFRelease(tempComps);
            CFRelease(compsCopy);
            CFRelease(tempDate);
            *exactMatch = false;
            *isLeapDay = true;
        }
    }
    
    return result;
}

// This function checks the input (assuming we've detected a mismatch hour), for a DST transition. If we find one, then it returns a new date. Otherwise it returns NULL.
static CFDateRef _Nullable _CFCalendarCreateAdjustedDateForMismatchedHour(CFCalendarRef calendar, CFDateRef matchDate /* the currently proposed match */, CFDateComponentsRef compsToMatch, CFOptionFlags opts, Boolean *exactMatch) {
    
    // It's possible this is a DST time. Let's check.
    CFDateRef startOfHour = NULL;
    CFTimeInterval hourInv = 0.0;
    Boolean found = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitHour, &startOfHour, &hourInv, matchDate);
    if (!found) {
        // Not DST
        return NULL;
    }
    
    // matchDate may not match because of a forward DST transition (e.g. spring forward, hour is lost).
    // matchDate may be before or after this lost hour, so look in both directions.
    CFIndex currentHour = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, startOfHour);
    
    Boolean isForwardDST = false;
    Boolean beforeTransition = true;
    CFDateRef next = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, hourInv, startOfHour);
    CFIndex nextHour = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, next);
    if ((nextHour - currentHour) > 1 || (currentHour == 23 && nextHour > 0)) {
        // We're just before a forward DST transition, e.g., for America/Sao_Paulo:
        //
        //            2018-11-03                      2018-11-04
        //    ┌─────11:00 PM (GMT-3)─────┐ │ ┌ ─ ─ 12:00 AM (GMT-3)─ ─ ─┐ ┌─────1:00 AM (GMT-2) ─────┐
        //    │                          │ │ |                          │ │                          │
        //    └──────▲───────────────────┘ │ └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘ └──────────────────────────┘
        //           └── Here                        Nonexistent
        //
        isForwardDST = true;
    } else {
        // We might be just after such a transition.
        CFDateRef previous = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, -1, startOfHour);
        CFIndex previousHour = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, previous);
        
        if ((currentHour - previousHour) > 1 || (previousHour == 23 && currentHour > 0)) {
            // We're just after a forward DST transition, e.g., for America/Sao_Paulo:
            //
            //            2018-11-03                      2018-11-04
            //    ┌─────11:00 PM (GMT-3)─────┐ │ ┌ ─ ─ 12:00 AM (GMT-3)─ ─ ─┐ ┌─────1:00 AM (GMT-2) ─────┐
            //    │                          │ │ |                          │ │                          │
            //    └──────────────────────────┘ │ └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─┘ └──▲───────────────────────┘
            //                                            Nonexistent            └── Here
            //
            isForwardDST = YES;
            beforeTransition = NO;
        }
        
        CFRelease(previous);
    }
    
    CFDateRef result = NULL;
    if (isForwardDST && !(opts & kCFCalendarMatchStrictly) /* we can only adjust when matches need not be strict */) {
        // We can adjust the time as necessary to make this match close enough.
        // Since we aren't trying to strictly match and are now going to make a best guess approximation, we set exactMatch to NO.
        *exactMatch = false;
        
        if (beforeTransition) {
            if (opts & kCFCalendarMatchNextTimePreservingSmallerUnits) {
                // Adding an hour this way gives us the right candidate while preserving the minute and second (as opposed to `next` which has those wiped out)
                result = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, 1, kCFCalendarUnitHour, matchDate);
            } else if (opts & kCFCalendarMatchNextTime) {
                // `next` is the start of the next hour past `matchDate` (i.e. round `matchDate` up to the next hour)
                result = CFRetain(next);
            } else {
                // No need to check `NSCalendarMatchPreviousTimePreservingSmallerUnits` or `NSCalendarMatchStrictly`:
                // * If we're matching the previous time, `matchDate` is already correct because we're pre-transition
                // * If we're matching strictly, we shouldn't be here (should be guarded by the if-statement condition): we can't adjust a strict match
                result = CFRetain(matchDate);
            }
        } else {
            if (opts & kCFCalendarMatchNextTime) {
                // `startOfHour` is the start of the hour containing `matchDate` (i.e. take `matchDate` but wipe the minute and second)
                result = CFRetain(startOfHour);
            } else if (opts & kCFCalendarMatchPreviousTimePreservingSmallerUnits) {
                // We've arrived here after a mismatch due to a forward DST transition, and specifically, one which produced a candidate matchDate which was _after_ the transition.
                // At the time of writing this (2018-07-11), the only way to hit this case is under the following circumstances:
                //
                //   * DST transition in a time zone which transitions at `hour = 0` (i.e. 11:59:59 -> 01:00:00)
                //   * Components request `hour = 0`
                //   * Components contain a date component higher than hour which advanced us to the start of the day from a prior day
                //
                // If the DST transition is not at midnight, the components request any other hour, or there is no higher date component, we will have fallen into the usual hour-rolling loop.
                // That loop right now takes care to stop looping _before_ the transition.
                //
                // This means that right now, if we attempt to match the previous time while preserving smaller components (i.e. rewinding by an hour), we will no longer match the higher date component which had been requested.
                // For instance, if searching for `weekday = 1` (Sunday) got us here, rewinding by an hour brings us back to Saturday. Similarly, if asking for `month = x` got us here, rewinding by an hour would bring us to `month = x - 1`.
                // These mismatches are not proper candidates and should not be accepted.
                //
                // However, if the conditions of the hour-rolling loop ever change, I am including the code which would be correct to use here: attempt to roll back by an hour, and check whether we've introduced a new mismatch.
                // Right now, candidateMatches is always false, so the check is redundant.
                
                // We don't actually have a match. Claim it's not DST too, to avoid accepting matchDate as-is anyway further on (which is what isForwardDST = YES allows for).
                result = NULL;
     
                // NOTE: The following is intentionally if-def'd out. Please see the comment above for rationale.
#if 0
                // Subtract an hour to keep smaller units the same.
                // The issue here is that this may cause a different component mismatch (e.g. rewinding across a day where the DST transition is at midnight).
                CFDateRef candidate = _CFCalendarCreateDateByAddingValueOfUnitToDate(calendar, -1, kCFCalendarUnitHour, matchDate);
                
                // We know the hour mismatched, and that's fine. Check all other components though.
                CFDateComponentsRef candidateComponents = CFDateComponentsCreateCopy(kCFAllocatorSystemDefault, compsToMatch);
                CFDateComponentsSetValue(candidateComponents, kCFCalendarUnitHour, CFDateComponentUndefined);
                
                Boolean candidateMatches = _CFCalendarCheckDateContainsMatchingComponents(calendar, candidate, candidateComponents, NULL);
                if (candidateMatches) {
                    *exactMatch = true;
                    result = candidate;
                } else {
                    // The new candidate is no better (e.g. we introduced a different mismatch elsewhere).
                    // We'll have to skip ahead.
                    CFRelease(candidate);
                }
                
                CFRelease(candidateComponents);
#endif
            } else {
                // No need to check `NSCalendarMatchNextTimePreservingSmallerUnits` or `NSCalendarMatchStrictly`:
                // * If we're matching the next time, `matchDate` is already correct because we're post-transition
                // * If we're matching strictly, we shouldn't be here (should be guarded by the if-statement condition): we can't adjust a strict match
                result = CFRetain(matchDate);
            }
        }
    }
    
    CFRelease(next);
    CFRelease(startOfHour);
    return result;
}

static CFDateRef _Nullable _CFCalendarCreateAdjustedDateForMismatches(CFCalendarRef calendar, CFDateRef start /* the original search date */, CFDateRef searchingDate /* the date that is adjusted as we loop */, CFDateRef matchDate /* the currently proposed match */, Boolean success /* output of the match function */, CFDateComponentsRef matchingComponents /* aka searchingComponents */, CFDateComponentsRef compsToMatch, CFOptionFlags opts, Boolean *isForwardDST, Boolean *exactMatch, Boolean *isLeapDay) {
    
    // Set up some default answers for the out args
    *isForwardDST = false;
    *exactMatch = true;
    *isLeapDay = false;
    
    // use this to find the units that don't match and then those units become the bailedUnit
    CFCalendarUnit mismatchedUnits = 0;
    Boolean dateMatchesComps = _CFCalendarCheckDateContainsMatchingComponents(calendar, matchDate, compsToMatch, &mismatchedUnits);
    
    // Skip trying to correct nanoseconds or quarters.  See <rdar://problem/30229247> NSCalendar -dateFromComponents: doesn't correctly set nanoseconds and <rdar://problem/30229506> NSCalendar -component:fromDate: always returns 0 for NSCalendarUnitQuarter
    Boolean nanoSecondsMismatch = ((mismatchedUnits & kCFCalendarUnitNanosecond) == kCFCalendarUnitNanosecond);
    Boolean quarterMismatch = ((mismatchedUnits & kCFCalendarUnitQuarter) == kCFCalendarUnitQuarter);
    if (!(!nanoSecondsMismatch && !quarterMismatch)) {
        // Everything else is fine. Just return this date.
        return CFRetain(matchDate);
    }

    if (mismatchedUnits == kCFCalendarUnitHour) {
        CFDateRef resultAdjustedForDST = _CFCalendarCreateAdjustedDateForMismatchedHour(calendar, matchDate, compsToMatch, opts, exactMatch);
        if (resultAdjustedForDST) {
            *isForwardDST = true;
            // Skip the next set of adjustments too
            return resultAdjustedForDST;
        }
    }
    
    if (success && dateMatchesComps) {
        // Everything is already fine. Just return the value.
        return CFRetain(matchDate);
    }
    
    const Boolean goBackwards = (opts & kCFCalendarSearchBackwards) == kCFCalendarSearchBackwards;
    const Boolean findLast = (opts & kCFCalendarMatchLast) == kCFCalendarMatchLast;
    const Boolean isChineseCalendar = CFEqual(CFCalendarGetIdentifier(calendar), kCFCalendarIdentifierChinese);

    CFCalendarUnit bailedUnit = 0;
    // We want to get the highest mismatched unit
    for (CFIndex i = NUM_CALENDAR_UNITS - 1; i > -1; i--) {
        CFCalendarUnit highestSoFar = mismatchedUnits & calendarUnits[i];
        if (highestSoFar == calendarUnits[i]) {
            bailedUnit = highestSoFar;
        }
    }
    const Boolean leapMonthMismatch = ((mismatchedUnits & kCFCalendarUnitLeapMonth) == kCFCalendarUnitLeapMonth);
    CFCalendarUnit nextHighestUnit =_CFCalendarNextHigherUnit(bailedUnit);
    if (nextHighestUnit == kCFNotFound && !leapMonthMismatch) {
        // _CFCalendarNextHigherUnit() can return kCFNotFound if bailedUnit was somehow a deprecated unit or something
        // Just return the original date in this case
        return CFRetain(matchDate);
    }
    
    // corrective measures
    if (bailedUnit == kCFCalendarUnitEra) {
        nextHighestUnit = kCFCalendarUnitYear;
    } else if (bailedUnit == kCFCalendarUnitYear || bailedUnit == kCFCalendarUnitYearForWeekOfYear) {
        nextHighestUnit = bailedUnit;
    }
    
    // We need to check for leap* situations
    Boolean const isGregorianCalendar = CFEqual(CFCalendarGetIdentifier(calendar), kCFGregorianCalendar);
    if (nextHighestUnit == kCFCalendarUnitYear || leapMonthMismatch) {
        const CFIndex desiredMonth = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitMonth);
        const CFIndex desiredDay = CFDateComponentsGetValue(compsToMatch, kCFCalendarUnitDay);
        if (!((desiredMonth != CFDateComponentUndefined) && (desiredDay != CFDateComponentUndefined))) {
            // Just return the original date in this case
            return CFRetain(matchDate);
        }

        if (isChineseCalendar) {
            if (leapMonthMismatch) {
                return _CFCalendarCreateAdjustedDateForMismatchedChineseLeapMonth(calendar, start, searchingDate, matchDate, matchingComponents, compsToMatch, opts, exactMatch, isLeapDay);
            } else {
                // Just return the original date in this case
                return CFRetain(matchDate);
            }
        }

        // Here is where we handle the other leap* situations (e.g. leap years in Gregorian calendar, leap months in Hebrew calendar)
        const Boolean monthMismatched = (mismatchedUnits & kCFCalendarUnitMonth) == kCFCalendarUnitMonth;
        const Boolean dayMismatched = (mismatchedUnits & kCFCalendarUnitDay) == kCFCalendarUnitDay;
        if (monthMismatched || dayMismatched) {
            CFDateRef result = _CFCalendarCreateAdjustedDateForMismatchedLeapMonthOrDay(calendar, start, searchingDate, matchDate, matchingComponents, compsToMatch, nextHighestUnit, opts, exactMatch, isLeapDay);

            // result may be NULL, which indicates a need to keep iterating
            return result;
        }

        // Last opportunity here is just to return the original match date
        return CFRetain(matchDate);
    } else if (nextHighestUnit == kCFCalendarUnitMonth && isGregorianCalendar && CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitMonth, matchDate) == 2) {
        // We've landed here because we couldn't find the date we wanted in February, because it doesn't exist (e.g. Feb 31st or 30th, or 29th on a non-leap-year).
        // matchDate is the end of February, so we need to advance to the beginning of March.
        CFDateRef startOfFebruary = NULL;
        CFTimeInterval monthInv = 0.0;

        Boolean const foundRange = _CFCalendarGetTimeRangeOfUnitForDate(calendar, kCFCalendarUnitMonth, &startOfFebruary, &monthInv, matchDate);
        if (foundRange) {
            CFDateRef adjustedDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorDefault, monthInv, startOfFebruary);
            if (opts & kCFCalendarMatchNextTimePreservingSmallerUnits) {
                // Advancing has caused us to lose all smaller units, so if we're looking to preserve them we need to add them back.
                CFDateComponentsRef const smallerUnits = CFCalendarCreateDateComponentsFromDate(kCFAllocatorDefault, calendar, kCFCalendarUnitHour | kCFCalendarUnitMinute | kCFCalendarUnitSecond, start);
                CFDateRef const tempSearchDate = adjustedDate;
                adjustedDate = _CFCalendarCreateDateByAddingDateComponentsToDate(kCFAllocatorDefault, calendar, smallerUnits, tempSearchDate, opts);
                CFRelease(tempSearchDate);
                CFRelease(smallerUnits);
            }

            // This isn't strictly a leap day, just a day that doesn't exist.
            *isLeapDay = true;
            *exactMatch = false;
            CFRelease(startOfFebruary);
            return adjustedDate;
        }

        return CFRetain(matchDate);
    } else {
        // Go to the top of the next period for the next highest unit of the one that bailed.
        Boolean s = false;
        CFDateRef result = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponentsInNextHighestUnitRange(calendar, &s, matchingComponents, nextHighestUnit, searchingDate, goBackwards, findLast, opts);
        if (!s) {
            *exactMatch = false;
        }

        return result;
    }
}

#pragma mark -
#pragma mark Enumerate Entry Point

CF_CROSS_PLATFORM_EXPORT void _CFCalendarEnumerateDates(CFCalendarRef calendar, CFDateRef start, CFDateComponentsRef matchingComponents, CFOptionFlags opts, void (^block)(CFDateRef _Nullable, Boolean, Boolean*)) {
    if (!start || !_CFCalendarVerifyCalendarOptions(opts) || !_CFCalendarVerifyCFDateComponentsValues(calendar, matchingComponents)) {
        return;
    }
    
    // Set up all of the options we need
    const Boolean goBackwards = (opts & kCFCalendarSearchBackwards) == kCFCalendarSearchBackwards;
    const Boolean findLast = (opts & kCFCalendarMatchLast) == kCFCalendarMatchLast;
    const Boolean strictMatching = (opts & kCFCalendarMatchStrictly) == kCFCalendarMatchStrictly;
    
    CFDateRef searchingDate = CFRetain(start);
    Boolean stop = false;
    
    // We keep track of the previous match date passed to the given block to ensure that we return exclusively increasing or decreasing results.
    // This should only be updated with non-nil values actually passed to the block.
    CFDateRef previouslyReturnedMatchDate = NULL;
    
    /* An important note: There are 2 distinct ways in which the enumeration is controlled.
     1) The caller sets *stop in the block to let us know when to stop.
     2) We keep trying for a certain number of iterations and give up if we don't find an answer within that span.
     Without either of these methods (esp #2), we potentially run into the halting problem.
     */
    CFIndex iterations = -1;
    
#define STOP_EXHAUSTIVE_SEARCH_AFTER_MAX_ITERATIONS 100

    /* This is the loop that controls the iterating. */
    while (stop == false) {
        iterations++;
        Boolean passToBlock = false;
        Boolean exactMatch = true;
        _CFReleaseDeferred CFDateRef resultDate = NULL;
        Boolean isLeapDay = false;
        
        // NOTE: Several comments reference "isForwardDST" as a way to relate areas in forward DST handling.
        //       If you rename this variable, make sure to find all references in comments as well and update accordingly.
        Boolean isForwardDST = false;
        
        /* Step A: Call helper method that does the searching */
        
        /* Note: The reasoning behind this is a bit difficult to immediately grok because it's not obvious but what it does is ensure that the algorithm enumerates through each year or month if they are not explicitly set in the NSDateComponents object passed in by the caller.  This only applies to cases where the highest set unit is month or day (at least for now).
         For ex, let's say comps is set the following way:
         { Day: 31 }
         We want to enumerate through all of the months that have a 31st day.  If NSCalendarMatchStrictly is set, the algorithm automagically skips over the months that don't have a 31st day and we pass the desired results to the block.  However, if any of the approximation options are set, we can't skip the months that don't have a 31st day - we need to provide the appropriate approximate date for them.  Calling this method allows us to see that day is the highest unit set in comps, and sets the month value in compsToMatch (previously unset in comps) to whatever the month is of the date we're using to search.
         
         Ex: searchingDate is '2016-06-10 07:00:00 +0000' so { Day: 31 } becomes { Month: 6, Day: 31 } in compsToMatch
         
         This way, the algorithm from here on out sees that month is now the highest set unit and we ensure that we search for the day we want in each month and provide an approximation when we can't find it, thus getting the results the caller expects.
         
         Ex: { Month: 6, Day: 31 } does not exist so if NSCalendarMatchNextTime is set, we pass '2016-07-01 07:00:00 +0000' to the block.
         */
        _CFReleaseDeferred CFDateComponentsRef compsToMatch = _CFCalendarCreateAdjustedComponents(calendar, matchingComponents, searchingDate, goBackwards);
        
        Boolean success = true;
        _CFReleaseDeferred CFDateRef matchDate = _CFCalendarCreateMatchingDateAfterStartDateMatchingComponents(calendar, &success, searchingDate, compsToMatch, goBackwards, findLast, opts);
        
        /* Step B: Couldn't find matching date with a quick and dirty search in the current era, year, etc.  Now try in the near future/past and make adjustments for leap situations and non-existent dates */
        
        _CFReleaseDeferred CFDateRef tempDate = _CFCalendarCreateAdjustedDateForMismatches(calendar, start, searchingDate, matchDate, success, matchingComponents, compsToMatch, opts, &isForwardDST, &exactMatch, &isLeapDay);
        
        // tempDate may be NULL, which indicates a need to keep iterating
        /* Step C: Validate what we found and then run block. Then prepare the search date for the next round of the loop */
        if (tempDate) {
            CFRelease(matchDate);
            matchDate = CFRetain(tempDate);
            
            // Check the components to see if they match what was desired
            CFCalendarUnit mismatchedUnits = 0;
            Boolean dateMatchesComps = _CFCalendarCheckDateContainsMatchingComponents(calendar, matchDate, matchingComponents, &mismatchedUnits);
            Boolean nanoSecondsMismatch = ((mismatchedUnits & kCFCalendarUnitNanosecond) == kCFCalendarUnitNanosecond);
            Boolean quarterMismatch = ((mismatchedUnits & kCFCalendarUnitQuarter) == kCFCalendarUnitQuarter);
            
            // Since we adjusted the matching components at the start of this loop, we'll check to see if what we found matches the components that were originally passed in.
            if (dateMatchesComps && !exactMatch) {
                exactMatch = true;
            }
            
            // Bump up the next highest unit
            CFDateRef newSearchingDate = _CFCalendarCreateBumpedDateUpToNextHigherUnitInComponents(calendar, searchingDate, matchingComponents, goBackwards, matchDate);
            if (newSearchingDate) {
                CFRelease(searchingDate);
                searchingDate = newSearchingDate;
            }
            
            // Nanosecond and quarter mismatches are not considered inexact.
            Boolean notAnExactMatch = (!dateMatchesComps && !nanoSecondsMismatch && !quarterMismatch);
            if (notAnExactMatch) {
                exactMatch = false;
            }

            CFComparisonResult order = previouslyReturnedMatchDate ? CFDateCompare(previouslyReturnedMatchDate, matchDate, NULL) : CFDateCompare(start, matchDate, NULL);
            if (((goBackwards && (order == kCFCompareLessThan)) || (!goBackwards && (order == kCFCompareGreaterThan))) && !nanoSecondsMismatch) {
                // We've gone ahead when we should have gone backwards or we went in the past when we were supposed to move forwards.
                // Normally, it's sufficient to set matchDate to nil and move on with the existing searching date. However, the searching date has been bumped forward by the next highest date component, which isn't always correct.
                // Specifically, if we're in a type of transition when the highest date component can repeat between now and the next highest date component, then we need to move forward by less.
                //
                // This can happen during a "fall back" DST transition in which an hour is repeated:
                //
                //   ┌─────1:00 PDT─────┐ ┌─────1:00 PST─────┐
                //   │                  │ │                  │
                //   └───────────▲───▲──┘ └───────────▲──────┘
                //               │   │                │
                //               |   |                valid
                //               │   last match/start
                //               │
                //               matchDate
                //
                // Instead of jumping ahead by a whole day, we can jump ahead by an hour to the next appropriate match. `valid` here would be the result found by searching with NSCalendarMatchLast.
                // In this case, before giving up on the current match date, we need to adjust newSearchingDate with this information.
                //
                // Currently, the case we care most about is adjusting for DST, but we might need to expand this to handle repeated months in some calendars.
                if (_CFCalendarFindHighestSetUnitInDateComponents(compsToMatch) == kCFCalendarUnitHour) {
                    CFIndex const matchHour = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, matchDate);

                    CFTimeInterval const hourAdjustment = (goBackwards ? -1 : 1) * 60 * 60;
                    CFDateRef const potentialNextMatchDate = _CFDateCreateWithTimeIntervalSinceDate(kCFAllocatorSystemDefault, hourAdjustment, matchDate);
                    CFIndex const potentialMatchHour = CFCalendarGetComponentFromDate(calendar, kCFCalendarUnitHour, potentialNextMatchDate);

                    if (matchHour == potentialMatchHour) {
                        // We're in a DST transition where the hour repeats. Use this date as the next search date.
                        CFRelease(searchingDate);
                        searchingDate = potentialNextMatchDate;
                    } else {
                        CFRelease(potentialNextMatchDate);
                    }
                }

                // In any case, return nil.
                // <rdar://problem/30681375> NSCalendar enumerateDatesAfterStartDate: should return nil or start date if search criteria is inconsistent?
                // TODO - Determine if we want to return the starting boundary if the approximation options are set.
                CFRelease(matchDate);
                matchDate = NULL;
                
                // Returning NULL is fine if we've exhausted our search.
                if (iterations < STOP_EXHAUSTIVE_SEARCH_AFTER_MAX_ITERATIONS) {
                    continue;
                } else {
                    passToBlock = true;
                }
            }

            // At this point, the date we matched is allowable unless:
            // 1) It's not an exact match AND
            // 2) We require an exact match (strict) OR
            // 3) It's not an exact match but not because we found a DST hour or day that doesn't exist in the month (i.e. it's truly the wrong result)
            Boolean const allowInexactMatchingDueToTimeSkips = isForwardDST || isLeapDay;
            if (matchDate && !exactMatch && (strictMatching || !allowInexactMatchingDueToTimeSkips)) {
                CFRelease(matchDate);
                matchDate = NULL;
            }

            if (order != kCFCompareEqualTo || !matchDate) { // If we get a result that is exactly the same as the start date, skip.
                if (matchDate) {
                    resultDate = CFRetain(matchDate);
                    passToBlock = true;
                } else {
                    if (iterations < STOP_EXHAUSTIVE_SEARCH_AFTER_MAX_ITERATIONS) {
                        continue;
                    } else {
                        passToBlock = true;
                    }
                }
            } else {
                // Returning NULL is fine if we've exhausted our search.
                if (iterations < STOP_EXHAUSTIVE_SEARCH_AFTER_MAX_ITERATIONS) {
                    continue;
                } else {
                    passToBlock = true;
                }
            }
        } else {
            // Bump up the next highest unit
            CFDateRef newSearchingDate = _CFCalendarCreateBumpedDateUpToNextHigherUnitInComponents(calendar, searchingDate, matchingComponents, goBackwards, NULL);
            if (newSearchingDate) {
                CFRelease(searchingDate);
                searchingDate = newSearchingDate;
            }
            
            // Returning NULL is fine if we've exhausted our search.
            if (iterations < STOP_EXHAUSTIVE_SEARCH_AFTER_MAX_ITERATIONS) {
                continue;
            } else {
                passToBlock = true;
            }
        }
        
        if (passToBlock) {
            // Execute the block
            if (resultDate) {
                if (previouslyReturnedMatchDate) CFRelease(previouslyReturnedMatchDate);
                previouslyReturnedMatchDate = CFRetain(resultDate);
            }
            
            block(resultDate, exactMatch, &stop);
        }
    } // End while loop
    
    if (previouslyReturnedMatchDate) CFRelease(previouslyReturnedMatchDate);
    CFRelease(searchingDate);
}

