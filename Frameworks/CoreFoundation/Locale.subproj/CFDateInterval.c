/*    CFDateInterval.c
      Copyright (c) 2017-2019, Apple Inc. and the Swift project authors

      Portions Copyright (c) 2017-2019, Apple Inc. and the Swift project authors
      Licensed under Apache License v2.0 with Runtime Library Exception
      See http://swift.org/LICENSE.txt for license information
      See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFString.h>
#include "CFInternal.h"
#include <CoreFoundation/CFDateInterval.h>

/* Runtime setup */
static CFTypeID __kCFDateIntervalTypeID = _kCFRuntimeNotATypeID;

struct __CFDateInterval {
    CFRuntimeBase _base;
    CFDateRef _start;
    CFTimeInterval _duration;
};

static Boolean __CFDateIntervalEqual(CFTypeRef cf1, CFTypeRef cf2) {
    if (cf1 == cf2) return true;
    
    CFDateIntervalRef di1 = (CFDateIntervalRef)cf1;
    CFDateIntervalRef di2 = (CFDateIntervalRef)cf2;
    return di1->_duration == di2->_duration && CFEqual(di1->_start, di2->_start);
}

static CFHashCode __CFDateIntervalHash(CFTypeRef cf) {
    CFDateIntervalRef di = (CFDateIntervalRef)cf;
    CFAbsoluteTime start = CFDateGetAbsoluteTime(di->_start);
    CFAbsoluteTime end = start + di->_duration;
    CFAbsoluteTime buf[] = {start, end};
    return CFHashBytes((uint8_t *)buf, (sizeof(buf) / sizeof(buf[0])) * sizeof(CFAbsoluteTime));
}

static void __CFDateIntervalDeallocate(CFTypeRef cf) {
    CFDateIntervalRef di = (CFDateIntervalRef)cf;
    CFRelease(di->_start);
}

static CFStringRef __CFDateIntervalCopyDescription(CFTypeRef cf) {
    CFDateIntervalRef di = (CFDateIntervalRef)cf;

    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFDateInterval %p [%p]> %@ %f"), di, CFGetAllocator(di), di->_start, di->_duration);
}

static const CFRuntimeClass __CFDateIntervalClass = {
    0,
    "CFDateInterval",
    NULL,   // init
    NULL,   // copy
    __CFDateIntervalDeallocate,
    __CFDateIntervalEqual,
    __CFDateIntervalHash,
    NULL,   //
    __CFDateIntervalCopyDescription
};

CF_PRIVATE CFTypeID CFDateIntervalGetTypeID(void) {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        __kCFDateIntervalTypeID = _CFRuntimeRegisterClass(&__CFDateIntervalClass);
    });
    return __kCFDateIntervalTypeID;
}

CF_EXPORT CFDateIntervalRef CFDateIntervalCreate(CFAllocatorRef _Nullable allocator, CFDateRef startDate, CFTimeInterval duration) {
    if (!allocator) allocator = CFAllocatorGetDefault();
    struct __CFDateInterval *di = NULL;
    uint32_t size = sizeof(struct __CFDateInterval) - sizeof(CFRuntimeBase);
    di = (struct __CFDateInterval *)_CFRuntimeCreateInstance(allocator, CFDateIntervalGetTypeID(), size, NULL);
    if (NULL == di) HALT;
    
    di->_start = CFRetain(startDate);
    di->_duration = duration;
    return di;
}

CF_EXPORT CFDateIntervalRef CFDateIntervalCreateWithEndDate(CFAllocatorRef _Nullable allocator, CFDateRef startDate, CFDateRef endDate) {
    return CFDateIntervalCreate(allocator, startDate, CFDateGetAbsoluteTime(endDate) - CFDateGetAbsoluteTime(startDate));
}

CF_EXPORT CFTimeInterval CFDateIntervalGetDuration(CFDateIntervalRef interval) {
    return interval->_duration;
}

CF_EXPORT CFDateRef CFDateIntervalCopyStartDate(CFDateIntervalRef interval) {
    return CFRetain(interval->_start);
}

CF_EXPORT CFDateRef CFDateIntervalCopyEndDate(CFDateIntervalRef interval) {
    return CFDateCreate(kCFAllocatorSystemDefault, CFDateGetAbsoluteTime(interval->_start) + interval->_duration);
}

CF_EXPORT CFComparisonResult CFDateIntervalCompare(CFDateIntervalRef interval1, CFDateIntervalRef interval2) {
    CFComparisonResult result = CFDateCompare(interval1->_start, interval2->_start, NULL);
    if (result == kCFCompareEqualTo) {
        if (interval1->_duration < interval2->_duration) return kCFCompareLessThan;
        if (interval1->_duration > interval2->_duration) return kCFCompareGreaterThan;
        return kCFCompareEqualTo;
    }
    return result;
}

CF_EXPORT Boolean CFDateIntervalIntersectsDateInterval(CFDateIntervalRef interval, CFDateIntervalRef intervalToIntersect) {
    CFDateRef otherEndDate = CFDateIntervalCopyEndDate(intervalToIntersect);
    CFDateRef selfEndDate = CFDateIntervalCopyEndDate(interval);
    Boolean result =
        CFDateIntervalContainsDate(interval, intervalToIntersect->_start) ||
        CFDateIntervalContainsDate(interval, otherEndDate) ||
        CFDateIntervalContainsDate(intervalToIntersect, interval->_start) ||
        CFDateIntervalContainsDate(intervalToIntersect, selfEndDate);
    
    CFRelease(otherEndDate);
    CFRelease(selfEndDate);
    return result;
}

CF_EXPORT CFDateIntervalRef _Nullable CFDateIntervalCreateIntersectionWithDateInterval(CFAllocatorRef allocator, CFDateIntervalRef interval, CFDateIntervalRef intervalToIntersect) {
    if (CFEqual(interval, intervalToIntersect)) {
        return (CFDateIntervalRef)CFRetain(interval);
    }
    
    if (!CFDateIntervalIntersectsDateInterval(interval, intervalToIntersect)) {
        return NULL;
    }
    
    CFAbsoluteTime selfStart = CFDateGetAbsoluteTime(interval->_start);
    CFAbsoluteTime selfEnd = selfStart + interval->_duration;
    CFAbsoluteTime otherStart = CFDateGetAbsoluteTime(intervalToIntersect->_start);
    CFAbsoluteTime otherEnd = otherStart + intervalToIntersect->_duration;
    
    CFDateRef resultStartDate = NULL;
    CFAbsoluteTime resultEndTime = 0;
    if (otherStart >= selfStart) {
        resultStartDate = intervalToIntersect->_start;
    } else {
        // interval starts after intervalToIntersect
        resultStartDate = interval->_start;
    }
    
    if (otherEnd >= selfEnd) {
        resultEndTime = selfEnd;
    } else {
        // intervalToIntersect ends before interval
        resultEndTime = otherEnd;
    }
    
    CFTimeInterval resultDuration = resultEndTime - CFDateGetAbsoluteTime(resultStartDate);
    
    return CFDateIntervalCreate(allocator, resultStartDate, resultDuration);
}

CF_EXPORT Boolean CFDateIntervalContainsDate(CFDateIntervalRef interval, CFDateRef date) {
    CFAbsoluteTime time = CFDateGetAbsoluteTime(date);
    CFAbsoluteTime start = CFDateGetAbsoluteTime(interval->_start);
    CFAbsoluteTime end = start + interval->_duration;
    return ((time >= start) && (time <= end));
}
