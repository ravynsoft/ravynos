/*	CFNumberFormatter.h
	Copyright (c) 2003-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFNUMBERFORMATTER__)
#define __COREFOUNDATION_CFNUMBERFORMATTER__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFLocale.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef CFStringRef CFNumberFormatterKey CF_STRING_ENUM;

typedef struct CF_BRIDGED_MUTABLE_TYPE(id) __CFNumberFormatter *CFNumberFormatterRef;

// CFNumberFormatters are not thread-safe.  Do not use one from multiple threads!

CF_EXPORT
CFTypeID CFNumberFormatterGetTypeID(void);

typedef CF_ENUM(CFIndex, CFNumberFormatterStyle) {	// number format styles
	kCFNumberFormatterNoStyle = 0,
	kCFNumberFormatterDecimalStyle = 1,
	kCFNumberFormatterCurrencyStyle = 2,
	kCFNumberFormatterPercentStyle = 3,
	kCFNumberFormatterScientificStyle = 4,
	kCFNumberFormatterSpellOutStyle = 5,
	kCFNumberFormatterOrdinalStyle API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0)) = 6,
	kCFNumberFormatterCurrencyISOCodeStyle API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0)) = 8,
	kCFNumberFormatterCurrencyPluralStyle API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0)) = 9,
	kCFNumberFormatterCurrencyAccountingStyle API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0)) = 10,
};


CF_EXPORT
CFNumberFormatterRef CFNumberFormatterCreate(CFAllocatorRef allocator, CFLocaleRef locale, CFNumberFormatterStyle style);
	// Returns a CFNumberFormatter, localized to the given locale, which
	// will format numbers to the given style.

CF_EXPORT
CFLocaleRef CFNumberFormatterGetLocale(CFNumberFormatterRef formatter);

CF_EXPORT
CFNumberFormatterStyle CFNumberFormatterGetStyle(CFNumberFormatterRef formatter);
	// Get the properties with which the number formatter was created.

CF_EXPORT
CFStringRef CFNumberFormatterGetFormat(CFNumberFormatterRef formatter);

CF_EXPORT
void CFNumberFormatterSetFormat(CFNumberFormatterRef formatter, CFStringRef formatString);
	// Set the format description string of the number formatter.  This
	// overrides the style settings.  The format of the format string
	// is as defined by the ICU library, and is similar to that found
	// in Microsoft Excel and NSNumberFormatter.
	// The number formatter starts with a default format string defined
	// by the style argument with which it was created.


CF_EXPORT
CFStringRef CFNumberFormatterCreateStringWithNumber(CFAllocatorRef allocator, CFNumberFormatterRef formatter, CFNumberRef number);

CF_EXPORT
CFStringRef CFNumberFormatterCreateStringWithValue(CFAllocatorRef allocator, CFNumberFormatterRef formatter, CFNumberType numberType, const void *valuePtr);
	// Create a string representation of the given number or value
	// using the current state of the number formatter.


typedef CF_OPTIONS(CFOptionFlags, CFNumberFormatterOptionFlags) {
    kCFNumberFormatterParseIntegersOnly = 1	/* only parse integers */
};

CF_EXPORT
CFNumberRef CFNumberFormatterCreateNumberFromString(CFAllocatorRef allocator, CFNumberFormatterRef formatter, CFStringRef string, CFRange *rangep, CFOptionFlags options);

CF_EXPORT
Boolean CFNumberFormatterGetValueFromString(CFNumberFormatterRef formatter, CFStringRef string, CFRange *rangep, CFNumberType numberType, void *valuePtr);
	// Parse a string representation of a number using the current state
	// of the number formatter.  The range parameter specifies the range
	// of the string in which the parsing should occur in input, and on
	// output indicates the extent that was used; this parameter can
	// be NULL, in which case the whole string may be used.  The
	// return value indicates whether some number was computed and
	// (if valuePtr is not NULL) stored at the location specified by
	// valuePtr.  The numberType indicates the type of value pointed
	// to by valuePtr.


CF_EXPORT
void CFNumberFormatterSetProperty(CFNumberFormatterRef formatter, CFNumberFormatterKey key, CFTypeRef value);

CF_EXPORT
CFTypeRef CFNumberFormatterCopyProperty(CFNumberFormatterRef formatter, CFNumberFormatterKey key);
	// Set and get various properties of the number formatter, the set of
	// which may be expanded in the future.

CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterCurrencyCode;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterDecimalSeparator;	// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterCurrencyDecimalSeparator; // CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterAlwaysShowDecimalSeparator; // CFBoolean
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterGroupingSeparator;	// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterUseGroupingSeparator;	// CFBoolean
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterPercentSymbol;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterZeroSymbol;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterNaNSymbol;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterInfinitySymbol;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterMinusSign;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterPlusSign;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterCurrencySymbol;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterExponentSymbol;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterMinIntegerDigits;	// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterMaxIntegerDigits;	// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterMinFractionDigits;	// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterMaxFractionDigits;	// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterGroupingSize;		// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterSecondaryGroupingSize;	// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterRoundingMode;		// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterRoundingIncrement;	// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterFormatWidth;		// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterPaddingPosition;	// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterPaddingCharacter;	// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterDefaultFormat;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterMultiplier;		// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterPositivePrefix;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterPositiveSuffix;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterNegativePrefix;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterNegativeSuffix;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterPerMillSymbol;		// CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterInternationalCurrencySymbol; // CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterCurrencyGroupingSeparator API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0)); // CFString
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterIsLenient API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));		// CFBoolean
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterUseSignificantDigits API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));	// CFBoolean
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterMinSignificantDigits API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));	// CFNumber
CF_EXPORT const CFNumberFormatterKey kCFNumberFormatterMaxSignificantDigits API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));	// CFNumber

typedef CF_ENUM(CFIndex, CFNumberFormatterRoundingMode) {
    kCFNumberFormatterRoundCeiling = 0,
    kCFNumberFormatterRoundFloor = 1,
    kCFNumberFormatterRoundDown = 2,
    kCFNumberFormatterRoundUp = 3,
    kCFNumberFormatterRoundHalfEven = 4,
    kCFNumberFormatterRoundHalfDown = 5,
    kCFNumberFormatterRoundHalfUp = 6
};

typedef CF_ENUM(CFIndex, CFNumberFormatterPadPosition) {
    kCFNumberFormatterPadBeforePrefix = 0,
    kCFNumberFormatterPadAfterPrefix = 1,
    kCFNumberFormatterPadBeforeSuffix = 2,
    kCFNumberFormatterPadAfterSuffix = 3
};


CF_EXPORT
Boolean CFNumberFormatterGetDecimalInfoForCurrencyCode(CFStringRef currencyCode, int32_t *defaultFractionDigits, double *roundingIncrement);
	// Returns the number of fraction digits that should be displayed, and
	// the rounding increment (or 0.0 if no rounding is done by the currency)
	// for the given currency.  Returns false if the currency code is unknown
	// or the information is not available.
	// Not localized because these are properties of the currency.

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFNUMBERFORMATTER__ */

