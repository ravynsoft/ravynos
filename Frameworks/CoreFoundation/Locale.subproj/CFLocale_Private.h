/*	CFLocale_Private.h
	Copyright (c) 2016-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors 
 */

#if !defined(__COREFOUNDATION_CFLOCALE_PRIVATE__)
#define __COREFOUNDATION_CFLOCALE_PRIVATE__ 1

#include <CoreFoundation/CoreFoundation.h>

/// Returns the user’s preferred locale as-is, without attempting to match the locale’s language to the main bundle, unlike `CFLocaleCopyCurrent`.
CF_EXPORT CFLocaleRef _CFLocaleCopyPreferred(void) API_AVAILABLE(macosx(10.13), ios(11.0), watchos(4.0), tvos(11.0));

typedef CF_ENUM(CFIndex, _CFLocaleCalendarDirection) {
    _kCFLocaleCalendarDirectionLeftToRight = 0,
    _kCFLocaleCalendarDirectionRightToLeft = 1
} API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT _CFLocaleCalendarDirection _CFLocaleGetCalendarDirection(void) API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));

CF_EXPORT const CFLocaleKey kCFLocaleMeasurementSystem API_AVAILABLE(macosx(10.13), ios(11.0), watchos(4.0), tvos(11.0));
CF_EXPORT const CFStringRef kCFLocaleMeasurementSystemMetric API_AVAILABLE(macosx(10.13), ios(11.0), watchos(4.0), tvos(11.0));
CF_EXPORT const CFStringRef kCFLocaleMeasurementSystemUS API_AVAILABLE(macosx(10.13), ios(11.0), watchos(4.0), tvos(11.0));
CF_EXPORT const CFStringRef kCFLocaleMeasurementSystemUK API_AVAILABLE(macosx(10.13), ios(11.0), watchos(4.0), tvos(11.0));

CF_EXPORT const CFLocaleKey kCFLocaleTemperatureUnit API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT const CFStringRef kCFLocaleTemperatureUnitCelsius API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT const CFStringRef kCFLocaleTemperatureUnitFahrenheit API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));


#endif /* __COREFOUNDATION_CFLOCALE_PRIVATE__ */
