/*    CFDateInterval.h
      Copyright (c) 2004-2019, Apple Inc. and the Swift project authors

      Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
      Licensed under Apache License v2.0 with Runtime Library Exception
      See http://swift.org/LICENSE.txt for license information
      See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#if !defined(__COREFOUNDATION_CFDATEINTERVAL__)
#define __COREFOUNDATION_CFDATEINTERVAL__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDate.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN
CF_ASSUME_NONNULL_BEGIN

typedef struct __CFDateInterval * CFDateIntervalRef;

CF_EXPORT CFDateIntervalRef CFDateIntervalCreate(CFAllocatorRef _Nullable allocator, CFDateRef startDate, CFTimeInterval duration) API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

CF_EXPORT CFDateIntervalRef CFDateIntervalCreateWithEndDate(CFAllocatorRef _Nullable allocator, CFDateRef startDate, CFDateRef endDate) API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

CF_EXPORT CFTimeInterval CFDateIntervalGetDuration(CFDateIntervalRef interval) API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

CF_EXPORT CFDateRef CFDateIntervalCopyStartDate(CFDateIntervalRef interval) API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

CF_EXPORT CFDateRef CFDateIntervalCopyEndDate(CFDateIntervalRef interval) API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

/*
 Comparison prioritizes ordering by start date. If the start dates are equal, then it will order by duration.
 e.g.
    Given intervals a and b
        a.   |-----|
        b.      |-----|
 CFDateIntervalCompare(a, b) would return kCFCompareLessThan because a's startDate is earlier in time than b's start date.

 In the event that the start dates are equal, the compare method will attempt to order by duration.
 e.g.
    Given intervals c and d
        c.  |-----|
        d.  |---|
 CFDateIntervalCompare(c, d)would result in kCFCompareGreaterThan because c is longer than d.

 If both the start dates and the durations are equal, then the intervals are considered equal and kCFCompareEqualTo is returned as the result.
 */
CF_EXPORT CFComparisonResult CFDateIntervalCompare(CFDateIntervalRef interval1, CFDateIntervalRef interval2) API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

CF_EXPORT Boolean CFDateIntervalIntersectsDateInterval(CFDateIntervalRef interval, CFDateIntervalRef intervalToIntersect) API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

CF_EXPORT CFDateIntervalRef _Nullable CFDateIntervalCreateIntersectionWithDateInterval(CFAllocatorRef _Nullable allocator, CFDateIntervalRef interval, CFDateIntervalRef intervalToIntersect) API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

CF_EXPORT Boolean CFDateIntervalContainsDate(CFDateIntervalRef interval, CFDateRef date) API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

CF_ASSUME_NONNULL_END
CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif

