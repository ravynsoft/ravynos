/*    CFDateComponents.h
      Copyright (c) 2004-2019, Apple Inc. and the Swift project authors

      Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
      Licensed under Apache License v2.0 with Runtime Library Exception
      See http://swift.org/LICENSE.txt for license information
      See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#if !defined(__COREFOUNDATION_CFDATECOMPONENTS__)
#define __COREFOUNDATION_CFDATECOMPONENTS__ 1

#include <CoreFoundation/CFBase.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN
CF_ASSUME_NONNULL_BEGIN

// Must match NSDateComponentUndefined
CF_ENUM(CFIndex) {
#if TARGET_OS_WIN32
    CFDateComponentUndefined = LLONG_MAX
#else
    CFDateComponentUndefined = __LONG_MAX__
#endif
};

enum {
    kCFCalendarUnitNanosecond = (1 << 15), // Must match NSCalendarUnitNanosecond
    kCFCalendarUnitCalendar = (1 << 20), // Must match NSCalendarUnitCalendar
    kCFCalendarUnitTimeZone = (1 << 21), // Must match NSCalendarUnitTimeZone
    kCFCalendarUnitLeapMonth = (1 << 30)
};

typedef struct CF_BRIDGED_TYPE(id) __CFDateComponents * CFDateComponentsRef;

CF_EXPORT
CFDateComponentsRef _Nullable CFDateComponentsCreate(CFAllocatorRef _Nullable allocator) CF_RETURNS_RETAINED;

CF_EXPORT
CFIndex CFDateComponentsGetValue(CFDateComponentsRef dateComp, CFCalendarUnit unit);

CF_EXPORT
void CFDateComponentsSetValue(CFDateComponentsRef dateComp, CFCalendarUnit unit, CFIndex value);

CF_EXPORT
CFCalendarRef _Nullable CFDateComponentsCopyCalendar(CFDateComponentsRef dateComp);

CF_EXPORT
void CFDateComponentsSetCalendar(CFDateComponentsRef dateComp, CFCalendarRef _Nullable calendar);

CF_EXPORT
CFTimeZoneRef _Nullable CFDateComponentsCopyTimeZone(CFDateComponentsRef dateComp);

CF_EXPORT
void CFDateComponentsSetTimeZone(CFDateComponentsRef dateComp, CFTimeZoneRef _Nullable timeZone);

CF_EXPORT
Boolean CFDateComponentsIsLeapMonthSet(CFDateComponentsRef dc);

CF_EXPORT
Boolean CFDateComponentsIsLeapMonth(CFDateComponentsRef dc);

CF_EXPORT
Boolean CFDateComponentsIsValidDate(CFDateComponentsRef dateComp);

CF_EXPORT
Boolean CFDateComponentsIsValidDateInCalendar(CFDateComponentsRef dateComp, CFCalendarRef _Nullable calendar);

// Calendar API

CF_EXPORT
Boolean CFDateComponentsDateMatchesComponents(CFDateComponentsRef dateComp, CFCalendarRef calendar, CFDateRef date);

CF_EXPORT
CFDateRef _Nullable CFCalendarCreateDateFromComponents(CFAllocatorRef _Nullable allocator, CFCalendarRef calendar, CFDateComponentsRef dateComp);

CF_EXPORT
CFIndex CFCalendarGetComponentFromDate(CFCalendarRef calendar, CFCalendarUnit unit, CFDateRef date);

CF_EXPORT
CFDateComponentsRef CFCalendarCreateDateComponentsFromDate(CFAllocatorRef _Nullable allocator, CFCalendarRef calendar, CFCalendarUnit units, CFDateRef date);

CF_EXPORT
CFDateComponentsRef CFDateComponentsCreateCopy(CFAllocatorRef allocator, CFDateComponentsRef dc);
    
CF_ASSUME_NONNULL_END
CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED


#endif /* ! __COREFOUNDATION_CFDATECOMPONENTS__ */
