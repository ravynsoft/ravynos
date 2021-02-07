/* NSLocale.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray, Richard Frith-Macdonald
   Date: June, 2010
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef __NSLocale_h_GNUSTEP_BASE_INCLUDE
#define __NSLocale_h_GNUSTEP_BASE_INCLUDE

#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

#import <Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSMutableDictionary;
@class NSString;
@class NSCharacterSet;

typedef NSUInteger NSLocaleLanguageDirection;
enum
{
  NSLocaleLanguageDirectionUnknown = 0,
  NSLocaleLanguageDirectionLeftToRight = 1,
  NSLocaleLanguageDirectionRightToLeft = 2,
  NSLocaleLanguageDirectionTopToBottom = 3,
  NSLocaleLanguageDirectionBottomToTop = 4
};

GS_EXPORT NSString * const NSCurrentLocaleDidChangeNotification;

//
// NSLocale Component Keys
//
GS_EXPORT NSString * const NSLocaleIdentifier;
GS_EXPORT NSString * const NSLocaleLanguageCode;
GS_EXPORT NSString * const NSLocaleCountryCode;
GS_EXPORT NSString * const NSLocaleScriptCode;
GS_EXPORT NSString * const NSLocaleVariantCode;
GS_EXPORT NSString * const NSLocaleExemplarCharacterSet;
GS_EXPORT NSString * const NSLocaleCalendarIdentifier;
GS_EXPORT NSString * const NSLocaleCalendar;
GS_EXPORT NSString * const NSLocaleCollationIdentifier;
GS_EXPORT NSString * const NSLocaleUsesMetricSystem;
GS_EXPORT NSString * const NSLocaleMeasurementSystem;
GS_EXPORT NSString * const NSLocaleDecimalSeparator;
GS_EXPORT NSString * const NSLocaleGroupingSeparator;
GS_EXPORT NSString * const NSLocaleCurrencySymbol;
GS_EXPORT NSString * const NSLocaleCurrencyCode;
GS_EXPORT NSString * const NSLocaleCollatorIdentifier;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
GS_EXPORT NSString * const NSLocaleQuotationBeginDelimiterKey;
GS_EXPORT NSString * const NSLocaleQuotationEndDelimiterKey;
GS_EXPORT NSString * const NSLocaleAlternateQuotationBeginDelimiterKey;
GS_EXPORT NSString * const NSLocaleAlternateQuotationEndDelimiterKey;
#endif

//
// NSLocale Calendar Keys
//
GS_EXPORT NSString * const NSGregorianCalendar;
GS_EXPORT NSString * const NSBuddhistCalendar;
GS_EXPORT NSString * const NSChineseCalendar;
GS_EXPORT NSString * const NSHebrewCalendar;
GS_EXPORT NSString * const NSIslamicCalendar;
GS_EXPORT NSString * const NSIslamicCivilCalendar;
GS_EXPORT NSString * const NSJapaneseCalendar;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
GS_EXPORT NSString * const NSRepublicOfChinaCalendar;
GS_EXPORT NSString * const NSPersianCalendar;
GS_EXPORT NSString * const NSIndianCalendar;
GS_EXPORT NSString * const NSISO8601Calendar;
#endif

/**
 * Provides information describing language, date and time, and currency
 * information.
 */
GS_EXPORT_CLASS
@interface NSLocale : NSObject <NSCoding, NSCopying>
{
#if	GS_EXPOSE(NSLocale)
@private
  NSString		*_localeId;
  NSMutableDictionary	*_components;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
/** Returns a version of the current locale which automatically updates
 *  when locale settngs are changed.
 */
+ (id) autoupdatingCurrentLocale;
#endif

/** Returns an array of NSStrings with all the available locale identifiers.
 */
+ (NSArray *) availableLocaleIdentifiers;

/** Returns the caoninical identifier for a language represented by
 * the supplied string.
 */
+ (NSString *) canonicalLanguageIdentifierFromString: (NSString *)string;

/** Returns the canonical identifier for a locale represented by the
 * supplied string.
 */
+ (NSString *) canonicalLocaleIdentifierFromString: (NSString *)string;

/** Returns the direction in which the language is written.
 */
+ (NSLocaleLanguageDirection) characterDirectionForLanguage:
  (NSString *)isoLangCode;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
/** Returns a list of available ISO currency code strings.
 */
+ (NSArray *) commonISOCurrencyCodes;
#endif

/** Parses the supplied locale identifier and returns a dictionary containing
 * its components.<br />
 * Components are NSLocaleLanguageCode, NSLocaleCountryCode, NSLocaleCalendar.
 */
+ (NSDictionary *) componentsFromLocaleIdentifier: (NSString *)string;

/** Returns the current locale information.
 */
+ (instancetype) currentLocale;

/** Returns an array of NSString representing all known country codes.
 */
+ (NSArray *) ISOCountryCodes;

/** Returns an array of NSString representing all known currency codes.
 */
+ (NSArray *) ISOCurrencyCodes;

/** Returns an array of NSString representing all known language codes.
 */
+ (NSArray *) ISOLanguageCodes;

/** Returns the direction in which lines of  text in the specified
 * language are written.
 */
+ (NSLocaleLanguageDirection) lineDirectionForLanguage: (NSString*)isoLangCode;

/** Builds and returns a locale idntifier from the individual components
 * supplied in dict.<br />
 * Components are NSLocaleLanguageCode, NSLocaleCountryCode, NSLocaleCalendar.
 */
+ (NSString *) localeIdentifierFromComponents: (NSDictionary*)dict;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
/** Returns the standard locale identifier for the windows locale code.
 */
+ (NSString *) localeIdentifierFromWindowsLocaleCode: (uint32_t)lcid;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
/** Returns a locale initialised with the given locale identifier.
 */
+ (instancetype) localeWithLocaleIdentifier:(NSString *)string;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
/** Returns an array of preferred languages.  Sorted from most preferred to
 *  leave preferred.
 */
+ (NSArray *) preferredLanguages;
#endif

/** Returns the the system locale.
 */
+ (instancetype) systemLocale;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
/** Returns the windows locale code corresponding to the staqndard locale
 * identifier.
 */
+ (uint32_t) windowsLocaleCodeFromLocaleIdentifier:
  (NSString *)localeIdentifier;
#endif

/** Returns the localised representation of the supplied value converted
 * on the basis that it represents information whose type is specified by
 * the key.
 */
- (NSString *) displayNameForKey: (NSString *)key value: (id)value;

/** Initialises the receiver to be the locale specified by the identifier.
 * This may result in replacement of the receiver by an existing locale.
 */
- (instancetype) initWithLocaleIdentifier: (NSString *)string;

/** Returns the canonical identifier for the receiver (which
 * may differ from the identifgier used to create the receiver
 * since different identifiers may map to the same locale).
 */
- (NSString *) localeIdentifier;

/** Returns the named object from the receiver locale.
 */
- (id) objectForKey: (id)key;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_12, GS_API_LATEST)
- (NSString *) languageCode;
- (NSString *) countryCode;
- (NSString *) scriptCode;
- (NSString *) variantCode;
- (NSCharacterSet *) exemplarCharacterSet;
- (NSString *) collationIdentifier;
- (NSString *) collatorIdentifier;
#endif

@end

#if	defined(__cplusplus)
}
#endif

#endif /* OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)	*/

#endif /* __NSLocale_h_GNUSTEPBASE_INCLUDE */
