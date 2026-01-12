/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*	CFNumberFormatter.h
	Copyright (c) 2003-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFNUMBERFORMATTER__)
#define __COREFOUNDATION_CFNUMBERFORMATTER__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFLocale.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct __CFNumberFormatter *CFNumberFormatterRef;

// CFNumberFormatters are not thread-safe.  Do not use one from multiple threads!

CF_EXPORT
CFTypeID CFNumberFormatterGetTypeID(void);

typedef CF_ENUM(CFIndex, CFNumberFormatterStyle) {	// number format styles
	kCFNumberFormatterNoStyle = 0,
	kCFNumberFormatterDecimalStyle = 1,
	kCFNumberFormatterCurrencyStyle = 2,
	kCFNumberFormatterPercentStyle = 3,
	kCFNumberFormatterScientificStyle = 4,
	kCFNumberFormatterSpellOutStyle = 5
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
void CFNumberFormatterSetProperty(CFNumberFormatterRef formatter, CFStringRef key, CFTypeRef value);

CF_EXPORT
CFTypeRef CFNumberFormatterCopyProperty(CFNumberFormatterRef formatter, CFStringRef key);
	// Set and get various properties of the number formatter, the set of
	// which may be expanded in the future.

CF_EXPORT const CFStringRef kCFNumberFormatterCurrencyCode;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterDecimalSeparator;	// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterCurrencyDecimalSeparator; // CFString
CF_EXPORT const CFStringRef kCFNumberFormatterAlwaysShowDecimalSeparator; // CFBoolean
CF_EXPORT const CFStringRef kCFNumberFormatterGroupingSeparator;	// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterUseGroupingSeparator;	// CFBoolean
CF_EXPORT const CFStringRef kCFNumberFormatterPercentSymbol;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterZeroSymbol;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterNaNSymbol;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterInfinitySymbol;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterMinusSign;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterPlusSign;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterCurrencySymbol;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterExponentSymbol;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterMinIntegerDigits;	// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterMaxIntegerDigits;	// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterMinFractionDigits;	// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterMaxFractionDigits;	// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterGroupingSize;		// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterSecondaryGroupingSize;	// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterRoundingMode;		// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterRoundingIncrement;	// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterFormatWidth;		// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterPaddingPosition;	// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterPaddingCharacter;	// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterDefaultFormat;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterMultiplier;		// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterPositivePrefix;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterPositiveSuffix;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterNegativePrefix;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterNegativeSuffix;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterPerMillSymbol;		// CFString
CF_EXPORT const CFStringRef kCFNumberFormatterInternationalCurrencySymbol; // CFString
CF_EXPORT const CFStringRef kCFNumberFormatterCurrencyGroupingSeparator CF_AVAILABLE(10_5, 2_0); // CFString
CF_EXPORT const CFStringRef kCFNumberFormatterIsLenient CF_AVAILABLE(10_5, 2_0);		// CFBoolean
CF_EXPORT const CFStringRef kCFNumberFormatterUseSignificantDigits CF_AVAILABLE(10_5, 2_0);	// CFBoolean
CF_EXPORT const CFStringRef kCFNumberFormatterMinSignificantDigits CF_AVAILABLE(10_5, 2_0);	// CFNumber
CF_EXPORT const CFStringRef kCFNumberFormatterMaxSignificantDigits CF_AVAILABLE(10_5, 2_0);	// CFNumber

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

