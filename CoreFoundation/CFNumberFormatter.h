/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFNumber.h>

typedef struct __NSNumberFormatter *CFNumberFormatterRef;

enum {
    kCFNumberFormatterNoStyle = 0,
    kCFNumberFormatterDecimalStyle = 1,
    kCFNumberFormatterCurrencyStyle = 2,
    kCFNumberFormatterPercentStyle = 3,
    kCFNumberFormatterScientificStyle = 4,
    kCFNumberFormatterSpellOutStyle = 5,
};
typedef CFIndex CFNumberFormatterStyle;

enum {
    kCFNumberFormatterParseIntegersOnly = 1
};

typedef enum {
    kCFNumberFormatterRoundCeiling = 0,
    kCFNumberFormatterRoundFloor = 1,
    kCFNumberFormatterRoundDown = 2,
    kCFNumberFormatterRoundUp = 3,
    kCFNumberFormatterRoundHalfEven = 4,
    kCFNumberFormatterRoundHalfDown = 5,
    kCFNumberFormatterRoundHalfUp = 6
} CFNumberFormatterRoundingMode;

enum {
    kCFNumberFormatterPadBeforePrefix = 0,
    kCFNumberFormatterPadAfterPrefix = 1,
    kCFNumberFormatterPadBeforeSuffix = 2,
    kCFNumberFormatterPadAfterSuffix = 3
};

COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterCurrencyCode;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterDecimalSeparator;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterCurrencyDecimalSeparator;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterAlwaysShowDecimalSeparator;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterGroupingSeparator;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterUseGroupingSeparator;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterPercentSymbol;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterZeroSymbol;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterNaNSymbol;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterInfinitySymbol;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterMinusSign;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterPlusSign;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterCurrencySymbol;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterExponentSymbol;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterMinIntegerDigits;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterMaxIntegerDigits;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterMinFractionDigits;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterMaxFractionDigits;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterGroupingSize;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterSecondaryGroupingSize;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterRoundingMode;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterRoundingIncrement;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterFormatWidth;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterPaddingPosition;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterPaddingCharacter;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterDefaultFormat;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterMultiplier;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterPositivePrefix;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterPositiveSuffix;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterNegativePrefix;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterNegativeSuffix;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterPerMillSymbol;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterInternationalCurrencySymbol;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterCurrencyGroupingSeparator;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterIsLenient;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterUseSignificantDigits;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterMinSignificantDigits;
COREFOUNDATION_EXPORT const CFStringRef kCFNumberFormatterMaxSignificantDigits;

COREFOUNDATION_EXPORT CFTypeID CFNumberFormatterGetTypeID(void);

COREFOUNDATION_EXPORT CFNumberFormatterRef CFNumberFormatterCreate(CFAllocatorRef allocator, CFLocaleRef locale, CFNumberFormatterStyle style);

COREFOUNDATION_EXPORT CFLocaleRef CFNumberFormatterGetLocale(CFNumberFormatterRef self);
COREFOUNDATION_EXPORT CFNumberFormatterStyle CFNumberFormatterGetStyle(CFNumberFormatterRef self);
COREFOUNDATION_EXPORT CFStringRef CFNumberFormatterGetFormat(CFNumberFormatterRef self);

COREFOUNDATION_EXPORT void CFNumberFormatterSetFormat(CFNumberFormatterRef self, CFStringRef format);
COREFOUNDATION_EXPORT void CFNumberFormatterSetProperty(CFNumberFormatterRef self, CFStringRef key, CFTypeRef value);

COREFOUNDATION_EXPORT CFTypeRef CFNumberFormatterCopyProperty(CFNumberFormatterRef self, CFStringRef key);
COREFOUNDATION_EXPORT CFNumberRef CFNumberFormatterCreateNumberFromString(CFAllocatorRef allocator, CFNumberFormatterRef self, CFStringRef string, CFRange *rangep, CFOptionFlags options);
COREFOUNDATION_EXPORT CFStringRef CFNumberFormatterCreateStringWithNumber(CFAllocatorRef allocator, CFNumberFormatterRef self, CFNumberRef number);
COREFOUNDATION_EXPORT CFStringRef CFNumberFormatterCreateStringWithValue(CFAllocatorRef allocator, CFNumberFormatterRef self, CFNumberType numberType, const void *valuep);
COREFOUNDATION_EXPORT Boolean CFNumberFormatterGetDecimalInfoForCurrencyCode(CFStringRef currencyCode, int32_t *defaultFractionDigits, double *roundingIncrement);
COREFOUNDATION_EXPORT Boolean CFNumberFormatterGetValueFromString(CFNumberFormatterRef self, CFStringRef string, CFRange *rangep, CFNumberType numberType, void *valuep);
