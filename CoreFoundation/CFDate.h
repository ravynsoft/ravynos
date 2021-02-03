/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>

typedef struct __NSDate *CFDateRef;

typedef double CFTimeInterval;
typedef CFTimeInterval CFAbsoluteTime;

#import <CoreFoundation/CFTimeZone.h>

typedef struct {
    SInt32 year;
    SInt8 month;
    SInt8 day;
    SInt8 hour;
    SInt8 minute;
    double second;
} CFGregorianDate;

typedef struct {
    SInt32 years;
    SInt32 months;
    SInt32 days;
    SInt32 hours;
    SInt32 minutes;
    double seconds;
} CFGregorianUnits;

typedef enum {
    kCFGregorianUnitsYears = 0x01,
    kCFGregorianUnitsMonths = 0x02,
    kCFGregorianUnitsDays = 0x04,
    kCFGregorianUnitsHours = 0x08,
    kCFGregorianUnitsMinutes = 0x10,
    kCFGregorianUnitsSeconds = 0x20,
    kCFGregorianAllUnits = 0x00FFFFFF
} CFGregorianUnitFlags;

COREFOUNDATION_EXPORT const CFTimeInterval kCFAbsoluteTimeIntervalSince1970;
COREFOUNDATION_EXPORT const CFTimeInterval kCFAbsoluteTimeIntervalSince1904;

COREFOUNDATION_EXPORT CFTypeID CFDateGetTypeID(void);

COREFOUNDATION_EXPORT CFDateRef CFDateCreate(CFAllocatorRef allocator, CFAbsoluteTime absoluteTime);

COREFOUNDATION_EXPORT CFComparisonResult CFDateCompare(CFDateRef self, CFDateRef other, void *context);
COREFOUNDATION_EXPORT CFAbsoluteTime CFDateGetAbsoluteTime(CFDateRef self);
COREFOUNDATION_EXPORT CFTimeInterval CFDateGetTimeIntervalSinceDate(CFDateRef self, CFDateRef other);

COREFOUNDATION_EXPORT CFAbsoluteTime CFAbsoluteTimeGetCurrent();

COREFOUNDATION_EXPORT SInt32 CFAbsoluteTimeGetWeekOfYear(CFAbsoluteTime absoluteTime, CFTimeZoneRef timeZone);
COREFOUNDATION_EXPORT SInt32 CFAbsoluteTimeGetDayOfWeek(CFAbsoluteTime absoluteTime, CFTimeZoneRef timeZone);
COREFOUNDATION_EXPORT SInt32 CFAbsoluteTimeGetDayOfYear(CFAbsoluteTime absoluteTime, CFTimeZoneRef timeZone);

COREFOUNDATION_EXPORT CFAbsoluteTime CFAbsoluteTimeAddGregorianUnits(CFAbsoluteTime absoluteTime, CFTimeZoneRef timeZone, CFGregorianUnits units);
COREFOUNDATION_EXPORT CFGregorianUnits CFAbsoluteTimeGetDifferenceAsGregorianUnits(CFAbsoluteTime absoluteTime, CFAbsoluteTime other, CFTimeZoneRef timeZone, CFOptionFlags unitFlags);
COREFOUNDATION_EXPORT CFGregorianDate CFAbsoluteTimeGetGregorianDate(CFAbsoluteTime absoluteTime, CFTimeZoneRef timeZone);

COREFOUNDATION_EXPORT CFAbsoluteTime CFGregorianDateGetAbsoluteTime(CFGregorianDate gregorianDate, CFTimeZoneRef timeZone);

COREFOUNDATION_EXPORT Boolean CFGregorianDateIsValid(CFGregorianDate gregorianDate, CFOptionFlags unitFlags);
