/* Definition of class NSNumberFormatter
   Copyright (C) 1999 Free Software Foundation, Inc.
   
   Written by: 	Fred Kiefer <FredKiefer@gmx.de>
   Date: 	July 2000
   Updated by: Richard Frith-Macdonald <rfm@gnu.org> Sept 2001
   
   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSNumberFormatter_h_GNUSTEP_BASE_INCLUDE
#define _NSNumberFormatter_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

#import	<Foundation/NSObject.h>
#import	<Foundation/NSFormatter.h>
#import	<Foundation/NSDecimalNumber.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class	NSString, NSAttributedString, NSDictionary,
        NSError, NSLocale, NSNumber;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
enum
{
  NSNumberFormatterNoStyle = 0,
  NSNumberFormatterDecimalStyle = 1,
  NSNumberFormatterCurrencyStyle = 2,
  NSNumberFormatterPercentStyle = 3,
  NSNumberFormatterScientificStyle = 4,
  NSNumberFormatterSpellOutStyle = 5
};
typedef NSUInteger NSNumberFormatterStyle;

enum
{
  NSNumberFormatterBehaviorDefault = 0,
  NSNumberFormatterBehavior10_0 = 1000,
  NSNumberFormatterBehavior10_4 = 1040
};
typedef NSUInteger NSNumberFormatterBehavior;

enum
{
  NSNumberFormatterPadBeforePrefix = 0,
  NSNumberFormatterPadAfterPrefix = 1,
  NSNumberFormatterPadBeforeSuffix = 2,
  NSNumberFormatterPadAfterSuffix = 3
};
typedef NSUInteger NSNumberFormatterPadPosition;

enum
{
  NSNumberFormatterRoundCeiling = 0,
  NSNumberFormatterRoundFloor = 1,
  NSNumberFormatterRoundDown = 2,
  NSNumberFormatterRoundUp = 3,
  NSNumberFormatterRoundHalfEven = 4,
  NSNumberFormatterRoundHalfDown = 5,
  NSNumberFormatterRoundHalfUp = 6
};
typedef NSUInteger NSNumberFormatterRoundingMode;
#endif

/**
 * <p><em><strong>This class is currently not fully implemented!  All set
 * methods will work, but stringForObject: will ignore the format completely.
 * The documentation below describes what the behavior SHOULD
 * be...</strong></em></p>
 *
 * <p>A specialization of the [NSFormatter] class for generating string
 * representations of numbers ([NSNumber] and [NSDecimalNumber] instances) and
 * for parsing numeric values in strings.</p>
 *
 *  <p>See the [NSFormatter] documentation for description of the basic methods
 *  for formatting and parsing that are available.</p>
 *
 * <p>There are no convenience initializers or constructors for this class.
 *  Instead, to obtain an instance, call alloc init and then -setFormat: .</p>
 *
 *  <p>The basic format of a format string uses "#" signs to represent digits,
 *  and other characters to represent themselves, in a context-dependent way.
 *  Thus, for example, <code>@"#,###.00"</code> means to print the number
 *  ending in .00 if it has no decimal part, otherwise print two decimal
 *  places, and to print one comma if it is greater than 1000.  Thus, 1000
 *  prints as "1,000.00", and 1444555.979 prints as "1444,555.98" (see
 *  -setRoundingBehavior:).</p>
 *
 * <p>After setting the format, you may change the thousands separator and
 * decimal point using set methods, or by calling -setLocalizesFormat: .</p>
 *
 * <p>You may set separate formats to be used for positive numbers, negative
 * numbers, and zero independently.</p>
 *
 * <p>In addition, this class supports attributed strings (see
 * [NSAttributedString]), so that you can specify font and color attributes,
 * among others, to display aspects of a number.  You can assign specific sets
 * of attributes for positive and negative numbers, and for specific cases
 * including 0, NaN, and nil... </p>
 */
GS_EXPORT_CLASS
@interface NSNumberFormatter : NSFormatter
{
#if	GS_EXPOSE(NSNumberFormatter)
@private
  BOOL _hasThousandSeparators;
  BOOL _allowsFloats;
  BOOL _localizesFormat;
  unichar _thousandSeparator;
  unichar _decimalSeparator;
  NSDecimalNumberHandler *_roundingBehavior;
  NSDecimalNumber *_maximum;
  NSDecimalNumber *_minimum;
  NSAttributedString *_attributedStringForNil;
  NSAttributedString *_attributedStringForNotANumber;
  NSAttributedString *_attributedStringForZero;
  NSString *_negativeFormat;
  NSString *_positiveFormat;
  NSDictionary *_attributesForPositiveValues;
  NSDictionary *_attributesForNegativeValues;
#endif
#if     GS_NONFRAGILE
#  if	defined(GS_NSNumberFormatter_IVARS)
@public
GS_NSNumberFormatter_IVARS;
#  endif
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

/**
 * Returns the format string this instance was initialized with.
 */
- (NSString*) format;

/**
 * Sets format string.  See class description for more information.
 */
- (void) setFormat: (NSString*)aFormat;

/**
 * Returns whether this format should defer to the locale in determining
 * thousands separator and decimal point.  The default is to NOT localize.
 */
- (BOOL) localizesFormat;

/**
 * Set whether this format should defer to the locale in determining thousands
 * separator and decimal point.  The default is to NOT localize.
 */
- (void) setLocalizesFormat: (BOOL)flag;

/**
 * Returns format used for negative numbers.
 */
- (NSString*) negativeFormat;

/**
 * Sets format used for negative numbers.  See class description for more
 * information.
 */
- (void) setNegativeFormat: (NSString*)aFormat;

/**
 * Returns format used for positive numbers.
 */
- (NSString*) positiveFormat;

/**
 * Sets format used for positive numbers.  See class description for more
 * information.
 */
- (void) setPositiveFormat: (NSString*)aFormat;


// Attributed Strings
/**
 *  Returns the exact attributed string used for nil values.  By default this
 *  is an empty string.
 */
- (NSAttributedString*) attributedStringForNil;

/**
 *  Sets the exact attributed string used for nil values.  By default this
 *  is an empty string.
 */
- (void) setAttributedStringForNil: (NSAttributedString*)newAttributedString;

/**
 *  Returns the exact attributed string used for NaN values.  By default this
 *  is the string "NaN" with no attributes.
 */
- (NSAttributedString*) attributedStringForNotANumber;

/**
 *  Sets the exact attributed string used for NaN values.  By default this
 *  is the string "NaN" with no attributes.
 */
- (void) setAttributedStringForNotANumber: (NSAttributedString*)newAttributedString;

/**
 *  Returns the exact attributed string used for zero values.  By default this
 *  is based on the format for zero values, if set, or the format for positive
 *  values otherwise.
 */
- (NSAttributedString*) attributedStringForZero;

/**
 *  Sets the exact attributed string used for zero values.  By default this
 *  is based on the format for zero values, if set, or the format for positive
 *  values otherwise.
 */
- (void) setAttributedStringForZero: (NSAttributedString*)newAttributedString;

/**
 * Returns the attributes to apply to negative values (whole string), when
 * -attributedStringForObjectValue:withDefaultAttributes: is called.  Default
 * is none.
 */
- (NSDictionary*) textAttributesForNegativeValues;

/**
 * Sets the attributes to apply to negative values (whole string), when
 * -attributedStringForObjectValue:withDefaultAttributes: is called.  Default
 * is none.
 */
- (void) setTextAttributesForNegativeValues: (NSDictionary*)newAttributes;

/**
 * Returns the attributes to apply to positive values (whole string), when
 * -attributedStringForObjectValue:withDefaultAttributes: is called.  Default
 * is none.
 */
- (NSDictionary*) textAttributesForPositiveValues;

/**
 * Sets the attributes to apply to positive values (whole string), when
 * -attributedStringForObjectValue:withDefaultAttributes: is called.  Default
 * is none.
 */
- (void) setTextAttributesForPositiveValues: (NSDictionary*)newAttributes;


// Rounding.. this should be communicated as id<NSDecimalNumberBehaviors>,
// not NSDecimalNumberHandler, but this is the way OpenStep and OS X do it..

/**
 * Returns object specifying the rounding behavior used when truncating
 * decimal digits in formats.  Default is
 * [NSDecimalNumberHandler+defaultDecimalNumberHandler].
 */
- (NSDecimalNumberHandler*) roundingBehavior;

/**
 * Sets object specifying the rounding behavior used when truncating
 * decimal digits in formats.  Default is
 * [NSDecimalNumberHandler+defaultDecimalNumberHandler].
 */
- (void) setRoundingBehavior: (NSDecimalNumberHandler*)newRoundingBehavior;

// Separators

/**
 * Returns whether thousands separator should be used, regardless of whether
 * it is set in format.  (Default is YES if explicitly set in format.)
 */
- (BOOL) hasThousandSeparators;

/**
 * Sets whether thousands separator should be used, regardless of whether
 * it is set in format.  (Default is YES if explicitly set in format.)
 */
- (void) setHasThousandSeparators: (BOOL)flag;


/**
 * Returns thousands separator used; default is ','.
 */
- (NSString*) thousandSeparator;

/**
 * Sets thousands separator used; default is ','.
 */
- (void) setThousandSeparator: (NSString*)newSeparator;

/**
 * Returns whether number parsing will accept floating point values or generate
 * an exception (only int values are valid).  Default is YES.
 */
- (BOOL) allowsFloats;

/**
 * Sets whether number parsing will accept floating point values or generate
 * an exception (only int values are valid).  Default is YES.
 */
- (void) setAllowsFloats: (BOOL)flag;

/**
 * Returns thousands separator used; default is '.'.
 */
- (NSString*) decimalSeparator;

/**
 * Sets thousands separator used; default is '.'.
 */
- (void) setDecimalSeparator: (NSString*)newSeparator;

// Maximum/minimum

/**
 * Returns maximum value that will be accepted as valid in number parsing.
 * Default is none.
 */
- (NSDecimalNumber*) maximum;

/**
 * Sets maximum value that will be accepted as valid in number parsing.
 * Default is none.
 */
- (void) setMaximum: (NSDecimalNumber*)aMaximum;

/**
 * Returns minimum value that will be accepted as valid in number parsing.
 * Default is none.
 */
- (NSDecimalNumber*) minimum;

/**
 * Sets minimum value that will be accepted as valid in number parsing.
 * Default is none.
 */
- (void) setMinimum: (NSDecimalNumber*)aMinimum;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
/** Sets the behavior of the formatter.<br />
 * NB. If GNUstep has been built without the ICU library,
 * NSNumberFormatterBehavior10_0 is currently used irrespective of
 * this setting.
 */
- (void) setFormatterBehavior: (NSNumberFormatterBehavior) behavior;

/** Returns the behavior of the receiver, either the default behavior
 * set for number formatters, or the behavior specified by an earlier
 * call to the -setFormatterBehavior: method.
 */
- (NSNumberFormatterBehavior) formatterBehavior;

/** Sets the default behavior of number formatters.<br />
 * NB. If GNUstep has been built without the ICU library,
 * NSNumberFormatterBehavior10_0 is currently used irrespective of
 * this setting.
 */
+ (void) setDefaultFormatterBehavior: (NSNumberFormatterBehavior) behavior;

/** Returns the formatter behavior previously set as the default
 * using the +setDefaultFormatterBehavior: method.
 */
+ (NSNumberFormatterBehavior) defaultFormatterBehavior;

- (void) setNumberStyle: (NSNumberFormatterStyle) style;
- (NSNumberFormatterStyle) numberStyle;
- (void) setGeneratesDecimalNumbers: (BOOL) flag;
- (BOOL) generatesDecimalNumbers;

- (void) setLocale: (NSLocale *) locale;
- (NSLocale *) locale;

- (void) setRoundingIncrement: (NSNumber *) number;
- (NSNumber *) roundingIncrement;
- (void) setRoundingMode: (NSNumberFormatterRoundingMode) mode;
- (NSNumberFormatterRoundingMode) roundingMode;

- (void) setFormatWidth: (NSUInteger) number;
- (NSUInteger) formatWidth;
- (void) setMultiplier: (NSNumber *) number;
- (NSNumber *) multiplier;

- (void) setPercentSymbol: (NSString *) string;
- (NSString *) percentSymbol;
- (void) setPerMillSymbol: (NSString *) string;
- (NSString *) perMillSymbol;
- (void) setMinusSign: (NSString *) string;
- (NSString *) minusSign;
- (void) setPlusSign: (NSString *) string;
- (NSString *) plusSign;
- (void) setExponentSymbol: (NSString *) string;
- (NSString *) exponentSymbol;
- (void) setZeroSymbol: (NSString *) string;
- (NSString *) zeroSymbol;
- (void) setNilSymbol: (NSString *) string;
- (NSString *) nilSymbol;
- (void) setNotANumberSymbol: (NSString *) string;
- (NSString *) notANumberSymbol;
- (void) setNegativeInfinitySymbol: (NSString *) string;
- (NSString *) negativeInfinitySymbol;
- (void) setPositiveInfinitySymbol: (NSString *) string;
- (NSString *) positiveInfinitySymbol;

- (void) setCurrencySymbol: (NSString *) string;
- (NSString *) currencySymbol;
- (void) setCurrencyCode: (NSString *) string;
- (NSString *) currencyCode;
- (void) setInternationalCurrencySymbol: (NSString *) string;
- (NSString *) internationalCurrencySymbol;

- (void) setPositivePrefix: (NSString *) string;
- (NSString *) positivePrefix;
- (void) setPositiveSuffix: (NSString *) string;
- (NSString *) positiveSuffix;
- (void) setNegativePrefix: (NSString *) string;
- (NSString *) negativePrefix;
- (void) setNegativeSuffix: (NSString *) string;
- (NSString *) negativeSuffix;

- (void) setTextAttributesForZero: (NSDictionary *) newAttributes;
- (NSDictionary *) textAttributesForZero;
- (void) setTextAttributesForNil: (NSDictionary *) newAttributes;
- (NSDictionary *) textAttributesForNil;
- (void) setTextAttributesForNotANumber: (NSDictionary *) newAttributes;
- (NSDictionary *) textAttributesForNotANumber;
- (void) setTextAttributesForPositiveInfinity: (NSDictionary *) newAttributes;
- (NSDictionary *) textAttributesForPositiveInfinity;
- (void) setTextAttributesForNegativeInfinity: (NSDictionary *) newAttributes;
- (NSDictionary *) textAttributesForNegativeInfinity;

- (void) setGroupingSeparator: (NSString *) string;
- (NSString *) groupingSeparator;
- (void) setUsesGroupingSeparator: (BOOL) flag;
- (BOOL) usesGroupingSeparator;
- (void) setAlwaysShowsDecimalSeparator: (BOOL) flag;
- (BOOL) alwaysShowsDecimalSeparator;
- (void) setCurrencyDecimalSeparator: (NSString *) string;
- (NSString *) currencyDecimalSeparator;
- (void) setGroupingSize: (NSUInteger) number;
- (NSUInteger) groupingSize;
- (void) setSecondaryGroupingSize: (NSUInteger) number;
- (NSUInteger) secondaryGroupingSize;

- (void) setPaddingCharacter: (NSString *) string;
- (NSString *) paddingCharacter;
- (void) setPaddingPosition: (NSNumberFormatterPadPosition) position;
- (NSNumberFormatterPadPosition) paddingPosition;

- (void) setMinimumIntegerDigits: (NSUInteger) number;
- (NSUInteger) minimumIntegerDigits;
- (void) setMinimumFractionDigits: (NSUInteger) number;
- (NSUInteger) minimumFractionDigits;
- (void) setMaximumIntegerDigits: (NSUInteger) number;
- (NSUInteger) maximumIntegerDigits;
- (void) setMaximumFractionDigits: (NSUInteger) number;
- (NSUInteger) maximumFractionDigits;

/**
 * Returns the number for this string.
 */
- (NSNumber *) numberFromString: (NSString *) string;
/**
 * Returns the string version of this number based on the format
 * specified.
 */
- (NSString *) stringFromNumber: (NSNumber *) number;

- (BOOL) getObjectValue: (out id *) anObject
              forString: (NSString *) aString
                  range: (NSRange *) rangep
                  error: (out NSError **) error;

#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (void) setUsesSignificantDigits: (BOOL) flag;
- (BOOL) usesSignificantDigits;
- (void) setMinimumSignificantDigits: (NSUInteger) number;
- (NSUInteger) minimumSignificantDigits;
- (void) setMaximumSignificantDigits: (NSUInteger) number;
- (NSUInteger) maximumSignificantDigits;

- (void) setCurrencyGroupingSeparator: (NSString *) string;
- (NSString *) currencyGroupingSeparator;

- (void) setLenient: (BOOL) flag;
- (BOOL) isLenient;

- (void) setPartialStringValidationEnabled: (BOOL) enabled;
- (BOOL) isPartialStringValidationEnabled;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
+ (NSString *) localizedStringFromNumber: (NSNumber *) num
    numberStyle: (NSNumberFormatterStyle) localizationStyle;
#endif

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSNumberFormatter_h_GNUSTEP_BASE_INCLUDE */

