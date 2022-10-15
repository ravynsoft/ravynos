/*	CFLocale.h
	Copyright (c) 2002-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFLOCALE__)
#define __COREFOUNDATION_CFLOCALE__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFNotificationCenter.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef CFStringRef CFLocaleIdentifier CF_EXTENSIBLE_STRING_ENUM;
typedef CFStringRef CFLocaleKey CF_STRING_ENUM;

typedef const struct CF_BRIDGED_TYPE(NSLocale) __CFLocale *CFLocaleRef;

CF_EXPORT
CFTypeID CFLocaleGetTypeID(void);

CF_EXPORT
CFLocaleRef CFLocaleGetSystem(void);
	// Returns the "root", canonical locale.  Contains fixed "backstop" settings.

CF_EXPORT
CFLocaleRef CFLocaleCopyCurrent(void);
	// Returns the logical "user" locale for the current user.
	// [This is Copy in the sense that you get a retain you have to release,
	// but we may return the same cached object over and over.]  Settings
	// you get from this locale do not change under you as CFPreferences
	// are changed (for safety and correctness).  Generally you would not
	// grab this and hold onto it forever, but use it to do the operations
	// you need to do at the moment, then throw it away.  (The non-changing
	// ensures that all the results of your operations are consistent.)

CF_EXPORT
CFArrayRef CFLocaleCopyAvailableLocaleIdentifiers(void);
	// Returns an array of CFStrings that represents all locales for
	// which locale data is available.

CF_EXPORT
CFArrayRef CFLocaleCopyISOLanguageCodes(void);
	// Returns an array of CFStrings that represents all known legal ISO
	// language codes.  Note: many of these will not have any supporting
	// locale data in Mac OS X.

CF_EXPORT
CFArrayRef CFLocaleCopyISOCountryCodes(void);
	// Returns an array of CFStrings that represents all known legal ISO
	// country codes.  Note: many of these will not have any supporting
	// locale data in Mac OS X.

CF_EXPORT
CFArrayRef CFLocaleCopyISOCurrencyCodes(void);
	// Returns an array of CFStrings that represents all known legal ISO
	// currency codes.  Note: some of these currencies may be obsolete, or
	// represent other financial instruments.

CF_EXPORT
CFArrayRef CFLocaleCopyCommonISOCurrencyCodes(void) API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
	// Returns an array of CFStrings that represents ISO currency codes for
	// currencies in common use.

CF_EXPORT
CFArrayRef CFLocaleCopyPreferredLanguages(void) API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
	// Returns the array of canonicalized CFString locale IDs that the user prefers.

CF_EXPORT
CFLocaleIdentifier CFLocaleCreateCanonicalLanguageIdentifierFromString(CFAllocatorRef allocator, CFStringRef localeIdentifier);
	// Map an arbitrary language identification string (something close at
	// least) to a canonical language identifier.

CF_EXPORT
CFLocaleIdentifier CFLocaleCreateCanonicalLocaleIdentifierFromString(CFAllocatorRef allocator, CFStringRef localeIdentifier);
	// Map an arbitrary locale identification string (something close at
	// least) to the canonical identifier.

CF_EXPORT
CFLocaleIdentifier CFLocaleCreateCanonicalLocaleIdentifierFromScriptManagerCodes(CFAllocatorRef allocator, LangCode lcode, RegionCode rcode);
	// Map a Mac OS LangCode and RegionCode to the canonical locale identifier.

CF_EXPORT
CFLocaleIdentifier CFLocaleCreateLocaleIdentifierFromWindowsLocaleCode(CFAllocatorRef allocator, uint32_t lcid) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
	// Map a Windows LCID to the canonical locale identifier.

CF_EXPORT
uint32_t CFLocaleGetWindowsLocaleCodeFromLocaleIdentifier(CFLocaleIdentifier localeIdentifier) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
	// Map a locale identifier to a Windows LCID.

typedef CF_ENUM(CFIndex, CFLocaleLanguageDirection) {
    kCFLocaleLanguageDirectionUnknown = 0,
    kCFLocaleLanguageDirectionLeftToRight = 1,
    kCFLocaleLanguageDirectionRightToLeft = 2,
    kCFLocaleLanguageDirectionTopToBottom = 3,
    kCFLocaleLanguageDirectionBottomToTop = 4
};

CF_EXPORT
CFLocaleLanguageDirection CFLocaleGetLanguageCharacterDirection(CFStringRef isoLangCode) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));

CF_EXPORT
CFLocaleLanguageDirection CFLocaleGetLanguageLineDirection(CFStringRef isoLangCode) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));

CF_EXPORT
CFDictionaryRef CFLocaleCreateComponentsFromLocaleIdentifier(CFAllocatorRef allocator, CFLocaleIdentifier localeID);
	// Parses a locale ID consisting of language, script, country, variant,
	// and keyword/value pairs into a dictionary. The keys are the constant
	// CFStrings corresponding to the locale ID components, and the values
	// will correspond to constants where available.
	// Example: "en_US@calendar=japanese" yields a dictionary with three
	// entries: kCFLocaleLanguageCode=en, kCFLocaleCountryCode=US, and
	// kCFLocaleCalendarIdentifier=kCFJapaneseCalendar.

CF_EXPORT
CFLocaleIdentifier CFLocaleCreateLocaleIdentifierFromComponents(CFAllocatorRef allocator, CFDictionaryRef dictionary);
	// Reverses the actions of CFLocaleCreateDictionaryFromLocaleIdentifier,
	// creating a single string from the data in the dictionary. The
	// dictionary {kCFLocaleLanguageCode=en, kCFLocaleCountryCode=US,
	// kCFLocaleCalendarIdentifier=kCFJapaneseCalendar} becomes
	// "en_US@calendar=japanese".

CF_EXPORT
CFLocaleRef CFLocaleCreate(CFAllocatorRef allocator, CFLocaleIdentifier localeIdentifier);
	// Returns a CFLocaleRef for the locale named by the "arbitrary" locale identifier.

CF_EXPORT
CFLocaleRef CFLocaleCreateCopy(CFAllocatorRef allocator, CFLocaleRef locale);
	// Having gotten a CFLocale from somebody, code should make a copy
	// if it is going to use it for several operations
	// or hold onto it.  In the future, there may be mutable locales.

CF_EXPORT
CFLocaleIdentifier CFLocaleGetIdentifier(CFLocaleRef locale);
	// Returns the locale's identifier.  This may not be the same string
	// that the locale was created with (CFLocale may canonicalize it).

CF_EXPORT
CFTypeRef CFLocaleGetValue(CFLocaleRef locale, CFLocaleKey key);
	// Returns the value for the given key.  This is how settings and state
	// are accessed via a CFLocale.  Values might be of any CF type.

CF_EXPORT
CFStringRef CFLocaleCopyDisplayNameForPropertyValue(CFLocaleRef displayLocale, CFLocaleKey key, CFStringRef value);
	// Returns the display name for the given value.  The key tells what
	// the value is, and is one of the usual locale property keys, though
	// not all locale property keys have values with display name values.


CF_EXPORT const CFNotificationName kCFLocaleCurrentLocaleDidChangeNotification API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));


// Locale Keys
CF_EXPORT const CFLocaleKey kCFLocaleIdentifier;
CF_EXPORT const CFLocaleKey kCFLocaleLanguageCode;
CF_EXPORT const CFLocaleKey kCFLocaleCountryCode;
CF_EXPORT const CFLocaleKey kCFLocaleScriptCode;
CF_EXPORT const CFLocaleKey kCFLocaleVariantCode;

CF_EXPORT const CFLocaleKey kCFLocaleExemplarCharacterSet;
CF_EXPORT const CFLocaleKey kCFLocaleCalendarIdentifier;
CF_EXPORT const CFLocaleKey kCFLocaleCalendar;
CF_EXPORT const CFLocaleKey kCFLocaleCollationIdentifier;
CF_EXPORT const CFLocaleKey kCFLocaleUsesMetricSystem;
CF_EXPORT const CFLocaleKey kCFLocaleMeasurementSystem; // "Metric", "U.S." or "U.K."
CF_EXPORT const CFLocaleKey kCFLocaleDecimalSeparator;
CF_EXPORT const CFLocaleKey kCFLocaleGroupingSeparator;
CF_EXPORT const CFLocaleKey kCFLocaleCurrencySymbol;
CF_EXPORT const CFLocaleKey kCFLocaleCurrencyCode; // ISO 3-letter currency code
CF_EXPORT const CFLocaleKey kCFLocaleCollatorIdentifier API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFLocaleKey kCFLocaleQuotationBeginDelimiterKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFLocaleKey kCFLocaleQuotationEndDelimiterKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFLocaleKey kCFLocaleAlternateQuotationBeginDelimiterKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFLocaleKey kCFLocaleAlternateQuotationEndDelimiterKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));

// Values for kCFLocaleCalendarIdentifier
typedef CFStringRef CFCalendarIdentifier CF_STRING_ENUM;

CF_EXPORT const CFCalendarIdentifier kCFGregorianCalendar;
CF_EXPORT const CFCalendarIdentifier kCFBuddhistCalendar;
CF_EXPORT const CFCalendarIdentifier kCFChineseCalendar;
CF_EXPORT const CFCalendarIdentifier kCFHebrewCalendar;
CF_EXPORT const CFCalendarIdentifier kCFIslamicCalendar;
CF_EXPORT const CFCalendarIdentifier kCFIslamicCivilCalendar;
CF_EXPORT const CFCalendarIdentifier kCFJapaneseCalendar;
CF_EXPORT const CFCalendarIdentifier kCFRepublicOfChinaCalendar API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFCalendarIdentifier kCFPersianCalendar API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFCalendarIdentifier kCFIndianCalendar API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFCalendarIdentifier kCFISO8601Calendar API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFCalendarIdentifier kCFIslamicTabularCalendar API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFCalendarIdentifier kCFIslamicUmmAlQuraCalendar API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFLOCALE__ */

