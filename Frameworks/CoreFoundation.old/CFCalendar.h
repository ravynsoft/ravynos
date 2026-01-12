/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFDate.h>
#import <CoreFoundation/CFTimeZone.h>

typedef struct __NSCalendar *CFCalendarRef;

typedef enum {
    kCFCalendarUnitEra = (1 << 1),
    kCFCalendarUnitYear = (1 << 2),
    kCFCalendarUnitMonth = (1 << 3),
    kCFCalendarUnitDay = (1 << 4),
    kCFCalendarUnitHour = (1 << 5),
    kCFCalendarUnitMinute = (1 << 6),
    kCFCalendarUnitSecond = (1 << 7),
    kCFCalendarUnitWeek = (1 << 8),
    kCFCalendarUnitWeekday = (1 << 9),
    kCFCalendarUnitWeekdayOrdinal = (1 << 10),
} CFCalendarUnit;

COREFOUNDATION_EXPORT CFTypeID CFCalendarGetTypeID(void);

COREFOUNDATION_EXPORT CFCalendarRef CFCalendarCopyCurrent(void);

COREFOUNDATION_EXPORT Boolean CFCalendarAddComponents(CFCalendarRef self, CFAbsoluteTime *absoluteTime, CFOptionFlags options, const unsigned char *componentDesc, ...);
COREFOUNDATION_EXPORT Boolean CFCalendarComposeAbsoluteTime(CFCalendarRef self, CFAbsoluteTime *absoluteTime, const unsigned char *componentDesc, ...);
COREFOUNDATION_EXPORT CFLocaleRef CFCalendarCopyLocale(CFCalendarRef self);
COREFOUNDATION_EXPORT CFTimeZoneRef CFCalendarCopyTimeZone(CFCalendarRef self);
COREFOUNDATION_EXPORT CFCalendarRef CFCalendarCreateWithIdentifier(CFAllocatorRef allocator, CFStringRef identifier);
COREFOUNDATION_EXPORT Boolean CFCalendarDecomposeAbsoluteTime(CFCalendarRef self, CFAbsoluteTime absoluteTime, const unsigned char *componentDesc, ...);
COREFOUNDATION_EXPORT Boolean CFCalendarGetComponentDifference(CFCalendarRef self, CFAbsoluteTime startingAT, CFAbsoluteTime resultAT, CFOptionFlags options, const unsigned char *componentDesc, ...);
COREFOUNDATION_EXPORT CFIndex CFCalendarGetFirstWeekday(CFCalendarRef self);
COREFOUNDATION_EXPORT CFStringRef CFCalendarGetIdentifier(CFCalendarRef self);
COREFOUNDATION_EXPORT CFRange CFCalendarGetMaximumRangeOfUnit(CFCalendarRef self, CFCalendarUnit unit);
COREFOUNDATION_EXPORT CFIndex CFCalendarGetMinimumDaysInFirstWeek(CFCalendarRef self);
COREFOUNDATION_EXPORT CFRange CFCalendarGetMinimumRangeOfUnit(CFCalendarRef self, CFCalendarUnit unit);
COREFOUNDATION_EXPORT CFIndex CFCalendarGetOrdinalityOfUnit(CFCalendarRef self, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime absoluteTime);
COREFOUNDATION_EXPORT CFRange CFCalendarGetRangeOfUnit(CFCalendarRef self, CFCalendarUnit smallerUnit, CFCalendarUnit biggerUnit, CFAbsoluteTime absoluteTime);
COREFOUNDATION_EXPORT Boolean CFCalendarGetTimeRangeOfUnit(CFCalendarRef self, CFCalendarUnit unit, CFAbsoluteTime absoluteTime, CFAbsoluteTime *start, CFTimeInterval *end);
COREFOUNDATION_EXPORT void CFCalendarSetFirstWeekday(CFCalendarRef self, CFIndex firstWeekday);

COREFOUNDATION_EXPORT void CFCalendarSetLocale(CFCalendarRef self, CFLocaleRef locale);
COREFOUNDATION_EXPORT void CFCalendarSetMinimumDaysInFirstWeek(CFCalendarRef self, CFIndex value);
COREFOUNDATION_EXPORT void CFCalendarSetTimeZone(CFCalendarRef self, CFTimeZoneRef timeZone);
