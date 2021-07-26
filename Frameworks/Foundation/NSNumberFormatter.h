/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSFormatter.h>
#import <CoreFoundation/CFNumberFormatter.h>

@class NSLocale, NSDecimalNumberHandler;

enum {
    NSNumberFormatterBehaviorDefault = 0,
    NSNumberFormatterBehavior10_0 = 1000,
    NSNumberFormatterBehavior10_4 = 1040,
};

typedef NSUInteger NSNumberFormatterBehavior;

enum {
    NSNumberFormatterNoStyle = kCFNumberFormatterNoStyle,
    NSNumberFormatterDecimalStyle = kCFNumberFormatterDecimalStyle,
    NSNumberFormatterCurrencyStyle = kCFNumberFormatterCurrencyStyle,
    NSNumberFormatterPercentStyle = kCFNumberFormatterPercentStyle,
    NSNumberFormatterScientificStyle = kCFNumberFormatterScientificStyle,
    NSNumberFormatterSpellOutStyle = kCFNumberFormatterSpellOutStyle,
};
typedef NSUInteger NSNumberFormatterStyle;

enum {
    NSNumberFormatterPadBeforePrefix = kCFNumberFormatterPadBeforePrefix,
    NSNumberFormatterPadAfterPrefix = kCFNumberFormatterPadAfterPrefix,
    NSNumberFormatterPadBeforeSuffix = kCFNumberFormatterPadBeforeSuffix,
    NSNumberFormatterPadAfterSuffix = kCFNumberFormatterPadAfterSuffix,
};
typedef NSUInteger NSNumberFormatterPadPosition;

enum {
    NSNumberFormatterRoundCeiling = kCFNumberFormatterRoundCeiling,
    NSNumberFormatterRoundFloor = kCFNumberFormatterRoundFloor,
    NSNumberFormatterRoundDown = kCFNumberFormatterRoundDown,
    NSNumberFormatterRoundUp = kCFNumberFormatterRoundUp,
    NSNumberFormatterRoundHalfEven = kCFNumberFormatterRoundHalfEven,
    NSNumberFormatterRoundHalfDown = kCFNumberFormatterRoundHalfDown,
    NSNumberFormatterRoundHalfUp = kCFNumberFormatterRoundHalfUp,
};
typedef NSUInteger NSNumberFormatterRoundingMode;

@interface NSNumberFormatter : NSFormatter {
    NSNumberFormatterBehavior _behavior;
    // 10.4
    NSNumberFormatterStyle _numberStyle;
    NSUInteger _formatWidth;
    NSLocale *_locale;
    NSNumber *_multiplier;

    BOOL _allowsFloats;
    BOOL _alwaysShowsDecimalSeparator;
    BOOL _isLenient;
    BOOL _isPartialStringValidationEnabled;
    BOOL _generatesDecimalNumbers;
    BOOL _usesGroupingSeparator;
    BOOL _usesSignificantDigits;

    NSUInteger _minimumIntegerDigits;
    NSUInteger _minimumFractionDigits;
    NSUInteger _minimumSignificantDigits;
    NSUInteger _maximumIntegerDigits;
    BOOL _customMaximumFractionDigits;
    NSUInteger _maximumFractionDigits;
    NSUInteger _maximumSignificantDigits;

    NSNumber *_minimum;
    NSNumber *_maximum;

    NSString *_nilSymbol;
    NSString *_notANumberSymbol;
    NSString *_zeroSymbol;
    NSString *_plusSign;
    NSString *_minusSign;
    NSString *_negativePrefix;
    NSString *_negativeSuffix;
    NSString *_positivePrefix;
    NSString *_positiveSuffix;
    NSString *_negativeInfinitySymbol;
    NSString *_positiveInfinitySymbol;

    NSString *_decimalSeparator;
    NSString *_exponentSymbol;
    NSString *_currencyCode;
    NSString *_currencySymbol;
    NSString *_internationalCurrencySymbol;
    NSString *_currencyDecimalSeparator;
    NSString *_currencyGroupingSeparator;
    NSString *_groupingSeparator;
    NSUInteger _groupingSize;
    NSUInteger _secondaryGroupingSize;
    NSString *_paddingCharacter;
    NSNumberFormatterPadPosition _paddingPosition;
    NSString *_percentSymbol;
    NSString *_perMillSymbol;
    NSNumber *_roundingIncrement;
    NSNumberFormatterRoundingMode _roundingMode;
    NSString *_positiveFormat;
    NSString *_negativeFormat;
    NSDictionary *_textAttributesForPositiveValues;
    NSDictionary *_textAttributesForNegativeValues;
    NSDictionary *_textAttributesForNegativeInfinity;
    NSDictionary *_textAttributesForNil;
    NSDictionary *_textAttributesForNotANumber;
    NSDictionary *_textAttributesForPositiveInfinity;
    NSDictionary *_textAttributesForZero;

    // 10.0
    NSAttributedString *_attributedStringForNil;
    NSAttributedString *_attributedStringForNotANumber;
    NSAttributedString *_attributedStringForZero;
    NSDecimalNumberHandler *_roundingBehavior;
    NSString *_thousandSeparator;

    BOOL _hasThousandSeparators;
    BOOL _localizesFormat;
}

+ (NSNumberFormatterBehavior)defaultFormatterBehavior;
+ (void)setDefaultFormatterBehavior:(NSNumberFormatterBehavior)value;

- (NSNumberFormatterBehavior)formatterBehavior;
- (NSNumberFormatterStyle)numberStyle;

- (NSUInteger)formatWidth;
- (NSLocale *)locale;
- (NSNumber *)multiplier;

- (BOOL)allowsFloats;
- (BOOL)alwaysShowsDecimalSeparator;
- (BOOL)isLenient;
- (BOOL)isPartialStringValidationEnabled;
- (BOOL)generatesDecimalNumbers;
- (BOOL)usesGroupingSeparator;
- (BOOL)usesSignificantDigits;

- (NSUInteger)minimumIntegerDigits;
- (NSUInteger)minimumFractionDigits;
- (NSUInteger)minimumSignificantDigits;

- (NSUInteger)maximumIntegerDigits;
- (NSUInteger)maximumFractionDigits;
- (NSUInteger)maximumSignificantDigits;

- (NSNumber *)minimum;
- (NSNumber *)maximum;

- (void)setMinimum:(NSNumber *)value;
- (void)setMaximum:(NSNumber *)value;

- (NSString *)nilSymbol;
- (NSString *)notANumberSymbol;
- (NSString *)zeroSymbol;
- (NSString *)plusSign;
- (NSString *)minusSign;
- (NSString *)negativePrefix;
- (NSString *)negativeSuffix;
- (NSString *)positivePrefix;
- (NSString *)positiveSuffix;
- (NSString *)negativeInfinitySymbol;
- (NSString *)positiveInfinitySymbol;
- (NSString *)decimalSeparator;
- (NSString *)exponentSymbol;
- (NSString *)currencyCode;
- (NSString *)currencySymbol;
- (NSString *)internationalCurrencySymbol;
- (NSString *)currencyDecimalSeparator;
- (NSString *)currencyGroupingSeparator;
- (NSString *)groupingSeparator;
- (NSUInteger)groupingSize;
- (NSUInteger)secondaryGroupingSize;
- (NSString *)paddingCharacter;
- (NSNumberFormatterPadPosition)paddingPosition;
- (NSString *)percentSymbol;
- (NSString *)perMillSymbol;
- (NSNumber *)roundingIncrement;
- (NSNumberFormatterRoundingMode)roundingMode;

- (NSString *)positiveFormat;
- (NSString *)negativeFormat;
- (NSDictionary *)textAttributesForPositiveValues;
- (NSDictionary *)textAttributesForNegativeValues;
- (NSDictionary *)textAttributesForNegativeInfinity;
- (NSDictionary *)textAttributesForNil;
- (NSDictionary *)textAttributesForNotANumber;
- (NSDictionary *)textAttributesForPositiveInfinity;
- (NSDictionary *)textAttributesForZero;

- (void)setAllowsFloats:(BOOL)flag;

- (void)setCurrencyCode:(NSString *)value;
- (void)setCurrencyDecimalSeparator:(NSString *)value;
- (void)setCurrencyGroupingSeparator:(NSString *)value;
- (void)setCurrencySymbol:(NSString *)value;
- (void)setDecimalSeparator:(NSString *)value;
- (void)setExponentSymbol:(NSString *)value;
- (void)setFormatterBehavior:(NSNumberFormatterBehavior)value;
- (void)setFormatWidth:(NSUInteger)value;
- (void)setGeneratesDecimalNumbers:(BOOL)value;
- (void)setGroupingSeparator:(NSString *)value;
- (void)setGroupingSize:(NSUInteger)value;
- (void)setInternationalCurrencySymbol:(NSString *)value;
- (void)setLenient:(BOOL)value;
- (void)setLocale:(NSLocale *)value;
- (void)setMaximumFractionDigits:(NSUInteger)value;
- (void)setMaximumIntegerDigits:(NSUInteger)value;
- (void)setMaximumSignificantDigits:(NSUInteger)value;
- (void)setMinimumFractionDigits:(NSUInteger)value;
- (void)setMinimumIntegerDigits:(NSUInteger)value;
- (void)setMinimumSignificantDigits:(NSUInteger)value;
- (void)setMinusSign:(NSString *)value;
- (void)setMultiplier:(NSNumber *)value;
- (void)setNegativeInfinitySymbol:(NSString *)value;
- (void)setNegativePrefix:(NSString *)value;
- (void)setNegativeSuffix:(NSString *)value;
- (void)setNilSymbol:(NSString *)value;
- (void)setNotANumberSymbol:(NSString *)value;
- (void)setNumberStyle:(NSNumberFormatterStyle)value;
- (void)setPaddingCharacter:(NSString *)value;
- (void)setPaddingPosition:(NSNumberFormatterPadPosition)value;
- (void)setPartialStringValidationEnabled:(BOOL)value;
- (void)setPercentSymbol:(NSString *)value;
- (void)setPerMillSymbol:(NSString *)value;
- (void)setPlusSign:(NSString *)value;
- (void)setPositiveInfinitySymbol:(NSString *)value;
- (void)setPositivePrefix:(NSString *)value;
- (void)setPositiveSuffix:(NSString *)value;
- (void)setRoundingIncrement:(NSNumber *)value;
- (void)setRoundingMode:(NSNumberFormatterRoundingMode)value;
- (void)setSecondaryGroupingSize:(NSUInteger)value;
- (void)setTextAttributesForNegativeInfinity:(NSDictionary *)value;
- (void)setTextAttributesForNil:(NSDictionary *)value;
- (void)setTextAttributesForNotANumber:(NSDictionary *)value;
- (void)setTextAttributesForPositiveInfinity:(NSDictionary *)value;
- (void)setTextAttributesForZero:(NSDictionary *)value;
- (void)setUsesGroupingSeparator:(BOOL)value;
- (void)setUsesSignificantDigits:(BOOL)value;
- (void)setZeroSymbol:(NSString *)value;

- (void)setAlwaysShowsDecimalSeparator:(BOOL)value;

- (void)setPositiveFormat:(NSString *)format;
- (void)setNegativeFormat:(NSString *)format;

- (void)setTextAttributesForNegativeValues:(NSDictionary *)attributes;
- (void)setTextAttributesForPositiveValues:(NSDictionary *)attributes;

- (NSString *)stringFromNumber:(NSNumber *)number;
- (NSNumber *)numberFromString:(NSString *)string;

- (BOOL)getObjectValue:(id *)valuep forString:(NSString *)string range:(NSRange *)rangep error:(NSError **)errorp;

// 10.0 behavior methods

- (BOOL)hasThousandSeparators;
- (BOOL)localizesFormat;
- (NSString *)thousandSeparator;

- (NSString *)format;
- (NSAttributedString *)attributedStringForZero;
- (NSAttributedString *)attributedStringForNil;
- (NSAttributedString *)attributedStringForNotANumber;
- (NSDecimalNumberHandler *)roundingBehavior;

- (void)setHasThousandSeparators:(BOOL)value;
- (void)setLocalizesFormat:(BOOL)flag;
- (void)setThousandSeparator:(NSString *)value;
- (void)setFormat:(NSString *)format;
- (void)setAttributedStringForZero:(NSAttributedString *)attributedString;
- (void)setAttributedStringForNil:(NSAttributedString *)attributedString;
- (void)setAttributedStringForNotANumber:(NSAttributedString *)attributedString;
- (void)setRoundingBehavior:(NSDecimalNumberHandler *)value;

@end
