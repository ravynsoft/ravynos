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

/*	CFNumberFormatter.c
	Copyright (c) 2002-2014, Apple Inc. All rights reserved.
	Responsibility: David Smith
*/

#include <CoreFoundation/CFNumberFormatter.h>
#include <CoreFoundation/ForFoundationOnly.h>
#include <CoreFoundation/CFBigNumber.h>
#include "CFInternal.h"
#include "CFLocaleInternal.h"
#include "CFICULogging.h"
#include <math.h>
#include <float.h>


static void __CFNumberFormatterCustomize(CFNumberFormatterRef formatter);
static CFStringRef __CFNumberFormatterCreateCompressedString(CFStringRef inString, Boolean isFormat, CFRange *rangep);
static UErrorCode __CFNumberFormatterApplyPattern(CFNumberFormatterRef formatter, CFStringRef pattern);
static CONST_STRING_DECL(kCFNumberFormatterFormattingContextKey, "kCFNumberFormatterFormattingContextKey");

#define BUFFER_SIZE 768

struct __CFNumberFormatter {
    CFRuntimeBase _base;
    UNumberFormat *_nf;
    CFLocaleRef _locale;
    CFNumberFormatterStyle _style;
    CFStringRef _format;	// NULL for RBNFs
    CFStringRef _defformat;
    CFStringRef _compformat;
    CFNumberRef _multiplier;
    CFStringRef _zeroSym;
    Boolean _isLenient;
    Boolean _userSetMultiplier;
    Boolean _usesCharacterDirection;
};

static CFStringRef __CFNumberFormatterCopyDescription(CFTypeRef cf) {
    CFNumberFormatterRef formatter = (CFNumberFormatterRef)cf;
    return CFStringCreateWithFormat(CFGetAllocator(formatter), NULL, CFSTR("<CFNumberFormatter %p [%p]>"), cf, CFGetAllocator(formatter));
}

static void __CFNumberFormatterDeallocate(CFTypeRef cf) {
    CFNumberFormatterRef formatter = (CFNumberFormatterRef)cf;
    if (formatter->_nf) __cficu_unum_close(formatter->_nf);
    if (formatter->_locale) CFRelease(formatter->_locale);
    if (formatter->_format) CFRelease(formatter->_format);
    if (formatter->_defformat) CFRelease(formatter->_defformat);
    if (formatter->_compformat) CFRelease(formatter->_compformat);
    if (formatter->_multiplier) CFRelease(formatter->_multiplier);
    if (formatter->_zeroSym) CFRelease(formatter->_zeroSym);
}

static CFTypeID __kCFNumberFormatterTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFNumberFormatterClass = {
    0,
    "CFNumberFormatter",
    NULL,	// init
    NULL,	// copy
    __CFNumberFormatterDeallocate,
    NULL,
    NULL,
    NULL,	// 
    __CFNumberFormatterCopyDescription
};

CFTypeID CFNumberFormatterGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFNumberFormatterTypeID = _CFRuntimeRegisterClass(&__CFNumberFormatterClass); });
    return __kCFNumberFormatterTypeID;
}

CFNumberFormatterRef CFNumberFormatterCreate(CFAllocatorRef allocator, CFLocaleRef locale, CFNumberFormatterStyle style) {
    struct __CFNumberFormatter *memory;
    uint32_t size = sizeof(struct __CFNumberFormatter) - sizeof(CFRuntimeBase);
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    __CFGenericValidateType(locale, CFLocaleGetTypeID());
    memory = (struct __CFNumberFormatter *)_CFRuntimeCreateInstance(allocator, CFNumberFormatterGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
    memory->_nf = NULL;
    memory->_locale = NULL;
    memory->_format = NULL;
    memory->_defformat = NULL;
    memory->_compformat = NULL;
    memory->_multiplier = NULL;
    memory->_zeroSym = NULL;
    memory->_isLenient = false;
    memory->_userSetMultiplier = false;
    memory->_usesCharacterDirection = false;
    if (NULL == locale) locale = CFLocaleGetSystem();
    memory->_style = style;
    uint32_t ustyle;
    switch (style) {
    case kCFNumberFormatterNoStyle: ustyle = UNUM_IGNORE; break;
    case kCFNumberFormatterDecimalStyle: ustyle = UNUM_DECIMAL; break;
    case kCFNumberFormatterCurrencyStyle: ustyle = UNUM_CURRENCY; break;
    case kCFNumberFormatterPercentStyle: ustyle = UNUM_PERCENT; break;
    case kCFNumberFormatterScientificStyle: ustyle = UNUM_SCIENTIFIC; break;
    case kCFNumberFormatterSpellOutStyle: ustyle = UNUM_SPELLOUT; break;
    case kCFNumberFormatterOrdinalStyle: ustyle = UNUM_ORDINAL; break;
    case kCFNumberFormatterDurationStyle: ustyle = UNUM_DURATION; break;
    default:
	CFAssert2(0, __kCFLogAssertion, "%s(): unknown style %d", __PRETTY_FUNCTION__, style);
	ustyle = UNUM_DECIMAL;
	memory->_style = kCFNumberFormatterDecimalStyle;
	break;
    }
    CFStringRef localeName = locale ? CFLocaleGetIdentifier(locale) : CFSTR("");
    char buffer[BUFFER_SIZE];
    const char *cstr = CFStringGetCStringPtr(localeName, kCFStringEncodingASCII);
    if (NULL == cstr) {
	if (CFStringGetCString(localeName, buffer, BUFFER_SIZE, kCFStringEncodingASCII)) cstr = buffer;
    }
    if (NULL == cstr) {
	CFRelease(memory);
	return NULL;
    }
    UErrorCode status = U_ZERO_ERROR;
    memory->_nf = __cficu_unum_open((UNumberFormatStyle)ustyle, NULL, 0, cstr, NULL, &status);
    CFAssert2(memory->_nf, __kCFLogAssertion, "%s(): error (%d) creating number formatter", __PRETTY_FUNCTION__, status);
    if (NULL == memory->_nf) {
	CFRelease(memory);
	return NULL;
    }
    UChar ubuff[4];
    if (kCFNumberFormatterNoStyle == style) {
        status = U_ZERO_ERROR;
	ubuff[0] = '#'; ubuff[1] = ';'; ubuff[2] = '#';
        __cficu_unum_applyPattern(memory->_nf, false, ubuff, 3, NULL, &status);
	__cficu_unum_setAttribute(memory->_nf, UNUM_MAX_INTEGER_DIGITS, 42);
	__cficu_unum_setAttribute(memory->_nf, UNUM_MAX_FRACTION_DIGITS, 0);
    }
    memory->_locale = locale ? CFLocaleCreateCopy(allocator, locale) : CFLocaleGetSystem();
    __CFNumberFormatterCustomize(memory);
    if (kCFNumberFormatterSpellOutStyle != memory->_style && kCFNumberFormatterOrdinalStyle != memory->_style && kCFNumberFormatterDurationStyle != memory->_style) {
	UChar ubuffer[BUFFER_SIZE];
	status = U_ZERO_ERROR;
	int32_t ret = __cficu_unum_toPattern(memory->_nf, false, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && ret <= BUFFER_SIZE) {
	    memory->_format = CFStringCreateWithCharacters(allocator, (const UniChar *)ubuffer, ret);
	}
    }
    memory->_defformat = memory->_format ? (CFStringRef)CFRetain(memory->_format) : NULL;
    memory->_compformat = memory->_format ? __CFNumberFormatterCreateCompressedString(memory->_format, true, NULL) : NULL;
    if (kCFNumberFormatterSpellOutStyle != memory->_style && kCFNumberFormatterOrdinalStyle != memory->_style && kCFNumberFormatterDurationStyle != memory->_style) {
	int32_t n = __cficu_unum_getAttribute(memory->_nf, UNUM_MULTIPLIER);
	if (1 != n) {
	    memory->_multiplier = CFNumberCreate(allocator, kCFNumberSInt32Type, &n);
	    __cficu_unum_setAttribute(memory->_nf, UNUM_MULTIPLIER, 1);
	}
    }
    __cficu_unum_setAttribute(memory->_nf, UNUM_LENIENT_PARSE, 0);
    __cficu_unum_setContext(memory->_nf, UDISPCTX_CAPITALIZATION_NONE, &status);
    return (CFNumberFormatterRef)memory;
}

extern CFDictionaryRef __CFLocaleGetPrefs(CFLocaleRef locale);

static void __substituteFormatStringFromPrefsNF(CFNumberFormatterRef formatter) {
    CFIndex formatStyle = formatter->_style;
    if (kCFNumberFormatterSpellOutStyle == formatStyle) return;
    if (kCFNumberFormatterOrdinalStyle == formatStyle) return;
    if (kCFNumberFormatterDurationStyle == formatStyle) return;
    CFStringRef prefName = CFSTR("AppleICUNumberFormatStrings");
    if (kCFNumberFormatterNoStyle != formatStyle) {
	CFStringRef pref = NULL;
	CFDictionaryRef prefs = __CFLocaleGetPrefs(formatter->_locale);
	CFPropertyListRef metapref = prefs ? CFDictionaryGetValue(prefs, prefName) : NULL;
	if (NULL != metapref && CFGetTypeID(metapref) == CFDictionaryGetTypeID()) {
	    CFStringRef key;
	    switch (formatStyle) {
	    case kCFNumberFormatterDecimalStyle: key = CFSTR("1"); break;
	    case kCFNumberFormatterCurrencyStyle: key = CFSTR("2"); break;
	    case kCFNumberFormatterPercentStyle: key = CFSTR("3"); break;
	    case kCFNumberFormatterScientificStyle: key = CFSTR("4"); break;
	    case kCFNumberFormatterSpellOutStyle: key = CFSTR("5"); break;
	    case kCFNumberFormatterOrdinalStyle: key = CFSTR("6"); break;
	    case kCFNumberFormatterDurationStyle: key = CFSTR("7"); break;
	    default: key = CFSTR("0"); break;
	    }
	    pref = (CFStringRef)CFDictionaryGetValue((CFDictionaryRef)metapref, key);
	}
	if (NULL != pref && CFGetTypeID(pref) == CFStringGetTypeID()) {
	    int32_t icustyle = UNUM_IGNORE;
	    switch (formatStyle) {
	    case kCFNumberFormatterDecimalStyle: icustyle = UNUM_DECIMAL; break;
	    case kCFNumberFormatterCurrencyStyle: icustyle = UNUM_CURRENCY; break;
	    case kCFNumberFormatterPercentStyle: icustyle = UNUM_PERCENT; break;
	    case kCFNumberFormatterScientificStyle: icustyle = UNUM_SCIENTIFIC; break;
	    case kCFNumberFormatterSpellOutStyle: icustyle = UNUM_SPELLOUT; break;
	    case kCFNumberFormatterOrdinalStyle: icustyle = UNUM_ORDINAL; break;
	    case kCFNumberFormatterDurationStyle: icustyle = UNUM_DURATION; break;
	    }
	    CFStringRef localeName = CFLocaleGetIdentifier(formatter->_locale);
	    char buffer[BUFFER_SIZE];
	    const char *cstr = CFStringGetCStringPtr(localeName, kCFStringEncodingASCII);
	    if (NULL == cstr) {
		if (CFStringGetCString(localeName, buffer, BUFFER_SIZE, kCFStringEncodingASCII)) cstr = buffer;
	    }
	    UErrorCode status = U_ZERO_ERROR;
	    UNumberFormat *nf = __cficu_unum_open((UNumberFormatStyle)icustyle, NULL, 0, cstr, NULL, &status);
	    if (NULL != nf) {
		UChar ubuffer[BUFFER_SIZE];
		status = U_ZERO_ERROR;
		int32_t number_len = __cficu_unum_toPattern(nf, false, ubuffer, BUFFER_SIZE, &status);
		if (U_SUCCESS(status) && number_len <= BUFFER_SIZE) {
		    CFStringRef numberString = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (const UniChar *)ubuffer, number_len);
		    status = U_ZERO_ERROR;
		    int32_t formatter_len = __cficu_unum_toPattern(formatter->_nf, false, ubuffer, BUFFER_SIZE, &status);
		    if (U_SUCCESS(status) && formatter_len <= BUFFER_SIZE) {
			CFMutableStringRef formatString = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
			CFStringAppendCharacters(formatString, (const UniChar *)ubuffer, formatter_len);
			// find numberString inside formatString, substitute the pref in that range
			CFRange result;
			if (CFStringFindWithOptions(formatString, numberString, CFRangeMake(0, formatter_len), 0, &result)) {
			    CFStringReplace(formatString, result, pref);
			    __CFNumberFormatterApplyPattern(formatter, formatString);
			}
			CFRelease(formatString);
		    }
		    CFRelease(numberString);
		}
		__cficu_unum_close(nf);
	    }
	}
    }
}

static UniChar __CFNumberFormatterNormalizeCharacter(UniChar c) {
    if (CFCharacterSetIsCharacterMember(CFCharacterSetGetPredefined(kCFCharacterSetWhitespace), c)) {
	return ' ';
    } else {
	return c;
    }
}

/* Attempt to match the unimplemented lenient parsing behavior described at http://www.unicode.org/reports/tr35/#Lenient_Parsing -- specifically any whitespace is ignored that does not exist between two letters or two numbers, and no-break spaces map to spaces. */
static CFStringRef __CFNumberFormatterCreateCompressedString(CFStringRef inString, Boolean isFormat, CFRange *rangep) {
    if (!inString) return NULL;
    CFRange range = { 0, 0 };
    if (rangep) {
	range = *rangep;
    } else {
	range.length = CFStringGetLength(inString);
    }
    CFMutableStringRef outString = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFCharacterSetRef letters = CFCharacterSetGetPredefined(kCFCharacterSetLetter);
    CFCharacterSetRef numbers = CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit);
    UniChar prevCh = 0, nextCh = 0;
    Boolean inQuote = false;
    for (CFIndex in_idx = range.location; in_idx < range.location + range.length; in_idx++) {
        UniChar ch = __CFNumberFormatterNormalizeCharacter(CFStringGetCharacterAtIndex(inString, in_idx));
	nextCh = (in_idx+1 < range.length) ? CFStringGetCharacterAtIndex(inString, in_idx+1) : 0;
	if (isFormat && ch == '\'') inQuote = !inQuote;
	if (inQuote || ch != ' ' || (CFCharacterSetIsCharacterMember(letters, prevCh) && CFCharacterSetIsCharacterMember(letters, nextCh)) || (CFCharacterSetIsCharacterMember(numbers, prevCh) && CFCharacterSetIsCharacterMember(numbers, nextCh))) {
	    CFStringAppendCharacters(outString, &ch, 1);
	    prevCh = ch;
	}
    }
    return outString;
}

// Should not be called for rule-based ICU formatters; not supported
static void __CFNumberFormatterApplySymbolPrefs(const void *key, const void *value, void *context) {
    if (CFGetTypeID(key) == CFStringGetTypeID() && CFGetTypeID(value) == CFStringGetTypeID()) {
	CFNumberFormatterRef formatter = (CFNumberFormatterRef)context;
	UNumberFormatSymbol sym = (UNumberFormatSymbol)CFStringGetIntValue((CFStringRef)key);
	CFStringRef item = (CFStringRef)value;
	CFIndex item_cnt = CFStringGetLength(item);
	STACK_BUFFER_DECL(UChar, item_buffer, item_cnt);
	UChar *item_ustr = (UChar *)CFStringGetCharactersPtr(item);
	if (NULL == item_ustr) {
	    CFStringGetCharacters(item, CFRangeMake(0, __CFMin(BUFFER_SIZE, item_cnt)), (UniChar *)item_buffer);
	    item_ustr = item_buffer;
	}
	UErrorCode status = U_ZERO_ERROR;
	__cficu_unum_setSymbol(formatter->_nf, sym, item_ustr, item_cnt, &status);
   }
}

// Should not be called for rule-based ICU formatters
static UErrorCode __CFNumberFormatterApplyPattern(CFNumberFormatterRef formatter, CFStringRef pattern) {
    if (kCFNumberFormatterSpellOutStyle == formatter->_style) return U_UNSUPPORTED_ERROR;
    if (kCFNumberFormatterOrdinalStyle == formatter->_style) return U_UNSUPPORTED_ERROR;
    if (kCFNumberFormatterDurationStyle == formatter->_style) return U_UNSUPPORTED_ERROR;
    CFIndex cnt = CFStringGetLength(pattern);
    STACK_BUFFER_DECL(UChar, ubuffer, cnt);
    const UChar *ustr = (const UChar *)CFStringGetCharactersPtr(pattern);
    if (NULL == ustr) {
	CFStringGetCharacters(pattern, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	ustr = ubuffer;
    }
    UErrorCode status = U_ZERO_ERROR;
    __cficu_unum_applyPattern(formatter->_nf, false, ustr, cnt, NULL, &status);

    // __cficu_unum_applyPattern() may have magically changed other attributes based on
    // the contents of the format string; we simply expose that ICU behavior, except
    // for UNUM_MULTIPLIER, which we re-read and reset, like we did at initialization
    // time though any user-set multiplier state takes precedence.
    if (formatter->_userSetMultiplier) {
	__cficu_unum_setAttribute(formatter->_nf, UNUM_MULTIPLIER, 1);
    } else {
	if (formatter->_multiplier) CFRelease(formatter->_multiplier);
        formatter->_multiplier = NULL;
        int32_t n = __cficu_unum_getAttribute(formatter->_nf, UNUM_MULTIPLIER);
        if (1 != n) {
	    formatter->_multiplier = CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	    __cficu_unum_setAttribute(formatter->_nf, UNUM_MULTIPLIER, 1);
        }
    }
    return status;
}

static void __CFNumberFormatterCustomize(CFNumberFormatterRef formatter) {
    __substituteFormatStringFromPrefsNF(formatter);
    CFDictionaryRef prefs = __CFLocaleGetPrefs(formatter->_locale);
    CFPropertyListRef metapref = prefs ? CFDictionaryGetValue(prefs, CFSTR("AppleICUNumberSymbols")) : NULL;
    if (NULL != metapref && CFGetTypeID(metapref) == CFDictionaryGetTypeID()) {
	CFDictionaryApplyFunction((CFDictionaryRef)metapref, __CFNumberFormatterApplySymbolPrefs, formatter);
    }
}

CFLocaleRef CFNumberFormatterGetLocale(CFNumberFormatterRef formatter) {
    __CFGenericValidateType(formatter, CFNumberFormatterGetTypeID());
    return formatter->_locale;
}

CFNumberFormatterStyle CFNumberFormatterGetStyle(CFNumberFormatterRef formatter) {
    __CFGenericValidateType(formatter, CFNumberFormatterGetTypeID());
    return formatter->_style;
}

CFStringRef CFNumberFormatterGetFormat(CFNumberFormatterRef formatter) {
    __CFGenericValidateType(formatter, CFNumberFormatterGetTypeID());
    if (kCFNumberFormatterSpellOutStyle == formatter->_style) return NULL;
    if (kCFNumberFormatterOrdinalStyle == formatter->_style) return NULL;
    if (kCFNumberFormatterDurationStyle == formatter->_style) return NULL;
    UChar ubuffer[BUFFER_SIZE];
    CFStringRef newString = NULL;
    UErrorCode status = U_ZERO_ERROR;
    int32_t ret = __cficu_unum_toPattern(formatter->_nf, false, ubuffer, BUFFER_SIZE, &status);
    if (U_SUCCESS(status) && ret <= BUFFER_SIZE) {
	newString = CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, ret);
    }
    if (newString && !formatter->_format) {
	formatter->_format = newString;
	if (formatter->_compformat) CFRelease(formatter->_compformat);
	formatter->_compformat = __CFNumberFormatterCreateCompressedString(formatter->_format, true, NULL);
    } else if (newString && !CFEqual(newString, formatter->_format)) {
	CFRelease(formatter->_format);
	formatter->_format = newString;
	if (formatter->_compformat) CFRelease(formatter->_compformat);
	formatter->_compformat = __CFNumberFormatterCreateCompressedString(formatter->_format, true, NULL);
    } else if (newString) {
	CFRelease(newString);
    }
    return formatter->_format;
}

void CFNumberFormatterSetFormat(CFNumberFormatterRef formatter, CFStringRef formatString) {
    __CFGenericValidateType(formatter, CFNumberFormatterGetTypeID());
    __CFGenericValidateType(formatString, CFStringGetTypeID());
    if (kCFNumberFormatterSpellOutStyle == formatter->_style) return;
    if (kCFNumberFormatterOrdinalStyle == formatter->_style) return;
    if (kCFNumberFormatterDurationStyle == formatter->_style) return;
    CFIndex cnt = CFStringGetLength(formatString);
    CFAssert1(cnt <= 1024, __kCFLogAssertion, "%s(): format string too long", __PRETTY_FUNCTION__);
    if ((!formatter->_format || !CFEqual(formatter->_format, formatString)) && cnt <= 1024) {
	UErrorCode status = __CFNumberFormatterApplyPattern(formatter, formatString);
	if (U_SUCCESS(status)) {
	    UChar ubuffer2[BUFFER_SIZE];
	    status = U_ZERO_ERROR;
	    int32_t ret = __cficu_unum_toPattern(formatter->_nf, false, ubuffer2, BUFFER_SIZE, &status);
	    if (U_SUCCESS(status) && ret <= BUFFER_SIZE) {
	        if (formatter->_format) CFRelease(formatter->_format);
		formatter->_format = CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer2, ret);
		if (formatter->_compformat) CFRelease(formatter->_compformat);
		formatter->_compformat = __CFNumberFormatterCreateCompressedString(formatter->_format, true, NULL);
	    }
	}
    }
}

#define GET_MULTIPLIER                      \
        double multiplier = 1.0;            \
        double dummy = 0.0;                 \
        if (formatter->_multiplier) {       \
            if (!CFNumberGetValue(formatter->_multiplier, kCFNumberFloat64Type, &multiplier)) { \
                multiplier = 1.0;           \
            }                               \
        }                                   \
        if (modf(multiplier, &dummy) < FLT_EPSILON) {   \
            multiplier = floor(multiplier);             \
        }

CFStringRef CFNumberFormatterCreateStringWithNumber(CFAllocatorRef allocator, CFNumberFormatterRef formatter, CFNumberRef number) {
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    __CFGenericValidateType(formatter, CFNumberFormatterGetTypeID());
    __CFGenericValidateType(number, CFNumberGetTypeID());
    // The values of CFNumbers with large unsigned 64-bit ints don't survive well through this
    CFNumberType type = CFNumberGetType(number);
    char buffer[64];
    CFNumberGetValue(number, type, buffer);
    return CFNumberFormatterCreateStringWithValue(allocator, formatter, type, buffer);
}

#define FORMAT_FLT(T, FUNC)							\
	T value = *(T *)valuePtr;					\
	if (0 == value && formatter->_zeroSym) { return (CFStringRef)CFRetain(formatter->_zeroSym); }	\
	if (1.0 != multiplier) {					\
		value = (T)(value * multiplier);                     \
	}								\
	status = U_ZERO_ERROR;						\
	used = FUNC(formatter->_nf, value, ubuffer + 1, cnt, NULL, &status); \
	if (status == U_BUFFER_OVERFLOW_ERROR || cnt < used) {		\
	    cnt = used + 1 + 1;						\
	    ustr = (UChar *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(UChar) * cnt, 0); \
	    status = U_ZERO_ERROR;					\
	    used = FUNC(formatter->_nf, value, ustr + 1, cnt, NULL, &status); \
	}

#define FORMAT_INT(T, FUN)                                                   \
        T value = *(T *)valuePtr;					\
        if (0 == value && formatter->_zeroSym) { return (CFStringRef)CFRetain(formatter->_zeroSym); }	\
        if (1.0 != multiplier) {					\
            value = (T)(value * multiplier);                        \
        }                                                           \
        _CFBigNum bignum;                                           \
        FUN(&bignum, value);                                        \
        char buffer[BUFFER_SIZE + 1];                                           \
        _CFBigNumToCString(&bignum, false, true, buffer, BUFFER_SIZE);      \
        status = U_ZERO_ERROR;                                      \
        used = __cficu_unum_formatDecimal(formatter->_nf, buffer, strlen(buffer), ubuffer + 1, BUFFER_SIZE, NULL, &status);     \
        if (status == U_BUFFER_OVERFLOW_ERROR || cnt < used) {      \
            cnt = used + 1 + 1;                                         \
            ustr = (UChar *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(UChar) * cnt, 0);                 \
            status = U_ZERO_ERROR;                                  \
            used = __cficu_unum_formatDecimal(formatter->_nf, buffer, strlen(buffer), ustr + 1, cnt, NULL, &status);            \
        }                                                           \

CFStringRef CFNumberFormatterCreateStringWithValue(CFAllocatorRef allocator, CFNumberFormatterRef formatter, CFNumberType numberType, const void *valuePtr) {
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    __CFGenericValidateType(formatter, CFNumberFormatterGetTypeID());
    GET_MULTIPLIER;
    UChar *ustr = NULL, ubuffer[BUFFER_SIZE + 1];
    UErrorCode status = U_ZERO_ERROR;
    CFIndex used, cnt = BUFFER_SIZE;
    if (numberType == kCFNumberFloat64Type || numberType == kCFNumberDoubleType) {
	FORMAT_FLT(double, __cficu_unum_formatDouble)
    } else if (numberType == kCFNumberFloat32Type || numberType == kCFNumberFloatType) {
	FORMAT_FLT(float, __cficu_unum_formatDouble)
    } else if (numberType == kCFNumberSInt64Type || numberType == kCFNumberLongLongType) {
	FORMAT_INT(int64_t, _CFBigNumInitWithInt64)
    } else if (numberType == kCFNumberLongType || numberType == kCFNumberCFIndexType) {
#if __LP64__
	FORMAT_INT(int64_t, _CFBigNumInitWithInt64)
#else
	FORMAT_INT(int32_t, _CFBigNumInitWithInt32)
#endif
    } else if (numberType == kCFNumberSInt32Type || numberType == kCFNumberIntType) {
	FORMAT_INT(int32_t, _CFBigNumInitWithInt32)
    } else if (numberType == kCFNumberSInt16Type || numberType == kCFNumberShortType) {
	FORMAT_INT(int16_t, _CFBigNumInitWithInt16)
    } else if (numberType == kCFNumberSInt8Type || numberType == kCFNumberCharType) {
	FORMAT_INT(int8_t, _CFBigNumInitWithInt8)
    } else {
	CFAssert2(0, __kCFLogAssertion, "%s(): unknown CFNumberType (%d)", __PRETTY_FUNCTION__, numberType);
	return NULL;
    }
    CFStringRef string = NULL;
    if (U_SUCCESS(status)) {
        UniChar *bufferToUse = ustr ? (UniChar *)ustr : (UniChar *)ubuffer;
        if (formatter->_usesCharacterDirection && CFLocaleGetLanguageCharacterDirection(CFLocaleGetIdentifier(formatter->_locale)) == kCFLocaleLanguageDirectionRightToLeft) {
            // Insert Unicode RTL marker
            bufferToUse[0] = 0x200F;
            used++;
        } else {
            // Move past direction marker
            bufferToUse++;
        }
        string = CFStringCreateWithCharacters(allocator, bufferToUse, used);
    }
    if (ustr) CFAllocatorDeallocate(kCFAllocatorSystemDefault, ustr);
    return string;
}

#undef FORMAT_FLT
#undef FORMAT_INT
#undef GET_MULTIPLIER

CFNumberRef CFNumberFormatterCreateNumberFromString(CFAllocatorRef allocator, CFNumberFormatterRef formatter, CFStringRef string, CFRange *rangep, CFOptionFlags options) {
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    __CFGenericValidateType(formatter, CFNumberFormatterGetTypeID());
    __CFGenericValidateType(string, CFStringGetTypeID());
    char buffer[16] __attribute__ ((aligned (8)));
    CFRange r = rangep ? *rangep : CFRangeMake(0, CFStringGetLength(string));
    CFNumberRef multiplierRef = formatter->_multiplier;
    formatter->_multiplier = NULL;
    Boolean b = CFNumberFormatterGetValueFromString(formatter, string, &r, kCFNumberSInt64Type, buffer);
    formatter->_multiplier = multiplierRef;
    if (b) {
        Boolean passedMultiplier = true;
        // We handle the multiplier case here manually; the final
        // result is supposed to be (parsed value) / (multiplier), but
        // the int case here should only succeed if the parsed value
        // is an exact multiple of the multiplier.
        if (multiplierRef) {
            int64_t tmp = *(int64_t *)buffer;
            double multiplier = 1.0;
            if (!CFNumberGetValue(multiplierRef, kCFNumberFloat64Type, &multiplier)) {
                multiplier = 1.0;
            }
            double dummy;
            if (llabs(tmp) < fabs(multiplier)) {
                passedMultiplier = false;
            } else if (fabs(multiplier) < 1.0) { // We can't handle this math yet
                passedMultiplier = false;
            } else if (modf(multiplier, &dummy) == 0.0) { // multiplier is an integer
                int64_t imult = (int64_t)multiplier;
                int64_t rem = tmp % imult;
                if (rem != 0) passedMultiplier = false;
                if (passedMultiplier) {
                    tmp = tmp / imult;
                    *(int64_t *)buffer = tmp;
                }
            } else if (multiplier == -1.0) { // simple
                tmp = tmp * -1;
                *(int64_t *)buffer = tmp;
            } else if (multiplier != 1.0) {
                // First, throw away integer multiples of the multiplier to
                // bring the value down to less than 2^53, so that we can
                // cast it to double without losing any precision, important
                // for the "remainder is zero" test.
                // Find power of two which, when multiplier is multiplied by it,
                // results in an integer value. pwr will be <= 52 since multiplier
                // is at least 1.
                int pwr = 0;
                double intgrl;
                while (modf(scalbn(multiplier, pwr), &intgrl) != 0.0) pwr++;
                int64_t i2 = (int64_t)intgrl;
                // scale pwr and i2 up to a reasonably large value so the next loop doesn't take forever
                while (llabs(i2) < (1LL << 50)) { i2 *= 2; pwr++; }
                int64_t cnt = 0;
                while ((1LL << 53) <= llabs(tmp)) {
		    // subtract (multiplier * 2^pwr) each time
                    tmp -= i2; // move tmp toward zero
                    cnt += (1LL << pwr); // remember how many 2^pwr we subtracted
                }
                // If the integer is less than 2^53, there is no loss
                // in converting it to double, so we can just do the
                // direct operation now.
                double rem = fmod((double)tmp, multiplier);
                if (rem != 0.0) passedMultiplier = false;
                if (passedMultiplier) {
		    // The original tmp, which we need to divide by multiplier, is at this point:
                    //   tmp + k * 2^n * multiplier, where k is the number of loop iterations
                    // That original value needs to be divided by multiplier and put back in the
                    // buffer.  Noting that k * 2^n == cnt, and after some algebra, we have:
                    tmp = (int64_t)((double)tmp / multiplier) + cnt;
                    *(int64_t *)buffer = tmp;
                }
            }
        }
        if (passedMultiplier && ((r.length == CFStringGetLength(string)) || (options & kCFNumberFormatterParseIntegersOnly))) {
            if (rangep) *rangep = r;
	    return CFNumberCreate(allocator, kCFNumberSInt64Type, buffer);
        }
    }
    if (options & kCFNumberFormatterParseIntegersOnly) return NULL;
    if (CFNumberFormatterGetValueFromString(formatter, string, rangep, kCFNumberFloat64Type, buffer)) {
	return CFNumberCreate(allocator, kCFNumberFloat64Type, buffer);
    }
    return NULL;
}

Boolean CFNumberFormatterGetValueFromString(CFNumberFormatterRef formatter, CFStringRef string, CFRange *rangep, CFNumberType numberType, void *valuePtr) {
    __CFGenericValidateType(formatter, CFNumberFormatterGetTypeID());
    __CFGenericValidateType(string, CFStringGetTypeID());
    CFStringRef stringToParse = formatter->_isLenient ? __CFNumberFormatterCreateCompressedString(string, false, rangep) : (CFStringRef)CFRetain(string);
    CFRange range = {0, 0};
    if(formatter->_isLenient) {
        range.length = CFStringGetLength(stringToParse);
    } else {
        if (rangep) {
            range = *rangep;
        } else {
            range.length = CFStringGetLength(stringToParse);
        }
        // __cficu_unum_parse chokes on leading whitespace
        CFCharacterSetRef whitespace = CFCharacterSetGetPredefined(kCFCharacterSetWhitespace);
        while(range.length > 0 && CFCharacterSetIsCharacterMember(whitespace, CFStringGetCharacterAtIndex(stringToParse, range.location))) {
            range.location++;
            range.length--;
        }
    }
    Boolean isZero = false;
    if (formatter->_zeroSym) {
	CFStringRef zeroSym = formatter->_isLenient ? __CFNumberFormatterCreateCompressedString(formatter->_zeroSym, false, NULL) : (CFStringRef)CFRetain(formatter->_zeroSym);
	if (kCFCompareEqualTo == CFStringCompare(stringToParse, zeroSym, 0)) {
	    isZero = true;
	}
	CFRelease(zeroSym);
    }
    if (1024 < range.length) range.length = 1024;
    const UChar *ustr = (const UChar *)CFStringGetCharactersPtr(stringToParse);
    STACK_BUFFER_DECL(UChar, ubuffer, (NULL == ustr) ? range.length : 1);
    if (NULL == ustr) {
	CFStringGetCharacters(stringToParse, range, (UniChar *)ubuffer);
	ustr = ubuffer;
    } else if (!formatter->_isLenient) {
	ustr += range.location;
    }
    CFNumberRef multiplierRef = formatter->_multiplier;
    formatter->_multiplier = NULL;
    if (formatter->_isLenient) {
        __CFNumberFormatterApplyPattern(formatter, formatter->_compformat);
        if (formatter->_multiplier) CFRelease(formatter->_multiplier);
        formatter->_multiplier = NULL;
    }
    Boolean integerOnly = 1;
    switch (numberType) {
    case kCFNumberSInt8Type: case kCFNumberCharType:
    case kCFNumberSInt16Type: case kCFNumberShortType:
    case kCFNumberSInt32Type: case kCFNumberIntType:
    case kCFNumberLongType: case kCFNumberCFIndexType:
    case kCFNumberSInt64Type: case kCFNumberLongLongType:
	__cficu_unum_setAttribute(formatter->_nf, UNUM_PARSE_INT_ONLY, 1); // ignored by ICU for rule-based formatters
	break;
    default:
	__cficu_unum_setAttribute(formatter->_nf, UNUM_PARSE_INT_ONLY, 0); // ignored by ICU for rule-based formatters
	integerOnly = 0;
	break;
    }
    int32_t dpos = 0;
    UErrorCode status = U_ZERO_ERROR;
    int64_t dreti = 0;
    double dretd = 0.0;
    if (isZero) {
	dpos = rangep ? rangep->length : 0;
    } else {
	char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
	int32_t len = __cficu_unum_parseDecimal(formatter->_nf, ustr, range.length, &dpos, buffer, sizeof(buffer), &status);
        if (!U_FAILURE(status) && 0 < len && integerOnly) {
	    char *endptr = NULL;
	    errno = 0;
	    dreti = strtoll_l(buffer, &endptr, 10, NULL);
	    if (!(errno == 0 && *endptr == '\0')) status = U_INVALID_FORMAT_ERROR;
	}
	if (!U_FAILURE(status) && 0 < len) {
	    char *endptr = NULL;
	    errno = 0;
	    dretd = strtod_l(buffer, &endptr, NULL);
	    if (!(errno == 0 && *endptr == '\0')) status = U_INVALID_FORMAT_ERROR;
	}
    }
    if (formatter->_isLenient) {
	if (rangep) {
	    CFIndex uncompEnd = rangep->location + rangep->length;
	    CFIndex uncompIdx = rangep->location;
	    for (CFIndex compIdx = 0; compIdx < dpos && uncompIdx < uncompEnd; compIdx++, uncompIdx++) {
		while (uncompIdx < uncompEnd && ustr[compIdx] != __CFNumberFormatterNormalizeCharacter(CFStringGetCharacterAtIndex(string, uncompIdx))) uncompIdx++;
	    }
	    rangep->length = uncompIdx - rangep->location;
	}
	__CFNumberFormatterApplyPattern(formatter, formatter->_format);
        if (formatter->_multiplier) CFRelease(formatter->_multiplier);
        formatter->_multiplier = NULL;
    } else if (rangep) {
        rangep->length = dpos + (range.location - rangep->location);
    }
    formatter->_multiplier = multiplierRef;
    CFRelease(stringToParse);
    if (U_FAILURE(status)) {
	return false;
    }
    if (formatter->_multiplier) {
        double multiplier = 1.0;
        if (!CFNumberGetValue(formatter->_multiplier, kCFNumberFloat64Type, &multiplier)) {
            multiplier = 1.0;
        }
        dreti = (int64_t)((double)dreti / multiplier); // integer truncation, plus double cast can be lossy for dreti > 2^53
        dretd = dretd / multiplier;
    }
    switch (numberType) {
    case kCFNumberSInt8Type: case kCFNumberCharType:
	if (INT8_MIN <= dreti && dreti <= INT8_MAX) {
	    *(int8_t *)valuePtr = (int8_t)dreti;
	    return true;
	}
	break;
    case kCFNumberSInt16Type: case kCFNumberShortType:
	if (INT16_MIN <= dreti && dreti <= INT16_MAX) {
	    *(int16_t *)valuePtr = (int16_t)dreti;
	    return true;
	}
	break;
    case kCFNumberSInt32Type: case kCFNumberIntType:
#if !__LP64__
    case kCFNumberLongType: case kCFNumberCFIndexType:
#endif
	if (INT32_MIN <= dreti && dreti <= INT32_MAX) {
	    *(int32_t *)valuePtr = (int32_t)dreti;
	    return true;
	}
	break;
    case kCFNumberSInt64Type: case kCFNumberLongLongType:
#if __LP64__
    case kCFNumberLongType: case kCFNumberCFIndexType:
#endif
	if (INT64_MIN <= dreti && dreti <= INT64_MAX) {
	    *(int64_t *)valuePtr = (int64_t)dreti;
	    return true;
	}
	break;
    case kCFNumberFloat32Type: case kCFNumberFloatType:
	if (-FLT_MAX <= dretd && dretd <= FLT_MAX) {
	    *(float *)valuePtr = (float)dretd;
	    return true;
	}
	break;
    case kCFNumberFloat64Type: case kCFNumberDoubleType:
	if (-DBL_MAX <= dretd && dretd <= DBL_MAX) {
	    *(double *)valuePtr = (double)dretd;
	    return true;
	}
	break;
    }
    return false;
}

void CFNumberFormatterSetProperty(CFNumberFormatterRef formatter, CFStringRef key, CFTypeRef value) {
    int32_t n;
    double d;
    UErrorCode status = U_ZERO_ERROR;
    UChar ubuffer[BUFFER_SIZE];
    CFIndex cnt;
    __CFGenericValidateType(formatter, CFNumberFormatterGetTypeID());
    __CFGenericValidateType(key, CFStringGetTypeID());
    // rule-based formatters don't do attributes and symbols, except for one
    if (CFEqual(kCFNumberFormatterFormattingContextKey, key)) {
        __CFGenericValidateType(value, CFNumberGetTypeID());
        CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
        __cficu_unum_setContext(formatter->_nf, n, &status);
    }
    if (kCFNumberFormatterSpellOutStyle == formatter->_style && kCFNumberFormatterIsLenientKey != key) return;
    if (kCFNumberFormatterOrdinalStyle == formatter->_style && kCFNumberFormatterIsLenientKey != key) return;
    if (kCFNumberFormatterDurationStyle == formatter->_style && kCFNumberFormatterIsLenientKey != key) return;
    if (kCFNumberFormatterCurrencyCodeKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setTextAttribute(formatter->_nf, UNUM_CURRENCY_CODE, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterDecimalSeparatorKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setSymbol(formatter->_nf, UNUM_DECIMAL_SEPARATOR_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterCurrencyDecimalSeparatorKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setSymbol(formatter->_nf, UNUM_MONETARY_SEPARATOR_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterAlwaysShowDecimalSeparatorKey == key) {
	__CFGenericValidateType(value, CFBooleanGetTypeID());
	__cficu_unum_setAttribute(formatter->_nf, UNUM_DECIMAL_ALWAYS_SHOWN, (kCFBooleanTrue == value));
    } else if (kCFNumberFormatterGroupingSeparatorKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setSymbol(formatter->_nf, UNUM_GROUPING_SEPARATOR_SYMBOL, (const UChar *)ubuffer, cnt, &status);
    } else if (kCFNumberFormatterUseGroupingSeparatorKey == key) {
	__CFGenericValidateType(value, CFBooleanGetTypeID());
	__cficu_unum_setAttribute(formatter->_nf, UNUM_GROUPING_USED, (kCFBooleanTrue == value));
    } else if (kCFNumberFormatterPercentSymbolKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setSymbol(formatter->_nf, UNUM_PERCENT_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterZeroSymbolKey == key) {
        __CFGenericValidateType(value, CFStringGetTypeID());
        CFStringRef old = formatter->_zeroSym;
        formatter->_zeroSym = value ? (CFStringRef)CFRetain(value) : NULL;
        if (old) CFRelease(old);
    } else if (kCFNumberFormatterNaNSymbolKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setSymbol(formatter->_nf, UNUM_NAN_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterInfinitySymbolKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setSymbol(formatter->_nf, UNUM_INFINITY_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterMinusSignKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setSymbol(formatter->_nf, UNUM_MINUS_SIGN_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterPlusSignKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setSymbol(formatter->_nf, UNUM_PLUS_SIGN_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterCurrencySymbolKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setSymbol(formatter->_nf, UNUM_CURRENCY_SYMBOL, (const UChar *)ubuffer, cnt, &status);
    } else if (kCFNumberFormatterExponentSymbolKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setSymbol(formatter->_nf, UNUM_EXPONENTIAL_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterMinIntegerDigitsKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_MIN_INTEGER_DIGITS, n);
    } else if (kCFNumberFormatterMaxIntegerDigitsKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_MAX_INTEGER_DIGITS, n);
    } else if (kCFNumberFormatterMinFractionDigitsKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_MIN_FRACTION_DIGITS, n);
    } else if (kCFNumberFormatterMaxFractionDigitsKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_MAX_FRACTION_DIGITS, n);
    } else if (kCFNumberFormatterGroupingSizeKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_GROUPING_SIZE, n);
    } else if (kCFNumberFormatterSecondaryGroupingSizeKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_SECONDARY_GROUPING_SIZE, n);
    } else if (kCFNumberFormatterRoundingModeKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_ROUNDING_MODE, n);
    } else if (kCFNumberFormatterRoundingIncrementKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberDoubleType, &d);
	__cficu_unum_setDoubleAttribute(formatter->_nf, UNUM_ROUNDING_INCREMENT, d);
    } else if (kCFNumberFormatterFormatWidthKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_FORMAT_WIDTH, n);
    } else if (kCFNumberFormatterPaddingPositionKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_PADDING_POSITION, n);
    } else if (kCFNumberFormatterPaddingCharacterKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setTextAttribute(formatter->_nf, UNUM_PADDING_CHARACTER, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterDefaultFormatKey == key) {
	// read-only attribute
    } else if (kCFNumberFormatterMultiplierKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
        CFNumberRef old = formatter->_multiplier;
        formatter->_multiplier = value ? (CFNumberRef)CFRetain(value) : NULL;
	formatter->_userSetMultiplier = value ? true : false;
        if (old) CFRelease(old);
    } else if (kCFNumberFormatterPositivePrefixKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setTextAttribute(formatter->_nf, UNUM_POSITIVE_PREFIX, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterPositiveSuffixKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setTextAttribute(formatter->_nf, UNUM_POSITIVE_SUFFIX, (const UChar *)ubuffer, cnt, &status);
    } else if (kCFNumberFormatterNegativePrefixKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setTextAttribute(formatter->_nf, UNUM_NEGATIVE_PREFIX, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterNegativeSuffixKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
	cnt = CFStringGetLength((CFStringRef)value);
	if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
	CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
	__cficu_unum_setTextAttribute(formatter->_nf, UNUM_NEGATIVE_SUFFIX, (const UChar *)ubuffer, cnt, &status);
    } else if (kCFNumberFormatterPerMillSymbolKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
        cnt = CFStringGetLength((CFStringRef)value);
        if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
        CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
        __cficu_unum_setSymbol(formatter->_nf, UNUM_PERMILL_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterInternationalCurrencySymbolKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
        cnt = CFStringGetLength((CFStringRef)value);
        if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
        CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
        __cficu_unum_setSymbol(formatter->_nf, UNUM_INTL_CURRENCY_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterCurrencyGroupingSeparatorKey == key) {
	__CFGenericValidateType(value, CFStringGetTypeID());
        cnt = CFStringGetLength((CFStringRef)value);
        if (BUFFER_SIZE < cnt) cnt = BUFFER_SIZE;
        CFStringGetCharacters((CFStringRef)value, CFRangeMake(0, cnt), (UniChar *)ubuffer);
        __cficu_unum_setSymbol(formatter->_nf, UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL, ubuffer, cnt, &status);
    } else if (kCFNumberFormatterIsLenientKey == key) {
	__CFGenericValidateType(value, CFBooleanGetTypeID());
	formatter->_isLenient = (kCFBooleanTrue == value);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_LENIENT_PARSE, (kCFBooleanTrue == value));
    } else if (kCFNumberFormatterUseSignificantDigitsKey == key) {
	__CFGenericValidateType(value, CFBooleanGetTypeID());
	__cficu_unum_setAttribute(formatter->_nf, UNUM_SIGNIFICANT_DIGITS_USED, (kCFBooleanTrue == value));
    } else if (kCFNumberFormatterMinSignificantDigitsKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_MIN_SIGNIFICANT_DIGITS, n);
    } else if (kCFNumberFormatterMaxSignificantDigitsKey == key) {
	__CFGenericValidateType(value, CFNumberGetTypeID());
	CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &n);
	__cficu_unum_setAttribute(formatter->_nf, UNUM_MAX_SIGNIFICANT_DIGITS, n);
    } else if (kCFNumberFormatterUsesCharacterDirectionKey == key) {
        __CFGenericValidateType(value, CFBooleanGetTypeID());
        formatter->_usesCharacterDirection = value == kCFBooleanTrue;
    } else {
	CFAssert3(0, __kCFLogAssertion, "%s(): unknown key %p (%@)", __PRETTY_FUNCTION__, key, key);
    }
    if (_CFExecutableLinkedOnOrAfter(CFSystemVersionSnowLeopard)) {
        // do a dummy call to CFNumberFormatterGetFormat() after changing an attribute because
        // ICU sometimes changes the pattern due to a property change, and we need to poke
        // __cficu_unum_toPattern() and also update our own variables
        CFNumberFormatterGetFormat(formatter);
    }
}

CFTypeRef CFNumberFormatterCopyProperty(CFNumberFormatterRef formatter, CFStringRef key) {
    int32_t n;
    double d;
    UErrorCode status = U_ZERO_ERROR;
    UChar ubuffer[BUFFER_SIZE];
    CFIndex cnt;
    __CFGenericValidateType(formatter, CFNumberFormatterGetTypeID());
    __CFGenericValidateType(key, CFStringGetTypeID());
    // rule-based formatters don't do attributes and symbols, except for one
    if (CFEqual(kCFNumberFormatterFormattingContextKey, key)) {
        n = __cficu_unum_getContext(formatter->_nf, UDISPCTX_TYPE_CAPITALIZATION, &status);
        if (1) {
            return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
        }
    }
    if (kCFNumberFormatterSpellOutStyle == formatter->_style && kCFNumberFormatterIsLenientKey != key) return NULL;
    if (kCFNumberFormatterOrdinalStyle == formatter->_style && kCFNumberFormatterIsLenientKey != key) return NULL;
    if (kCFNumberFormatterDurationStyle == formatter->_style && kCFNumberFormatterIsLenientKey != key) return NULL;
    if (kCFNumberFormatterCurrencyCodeKey == key) {
	cnt = __cficu_unum_getTextAttribute(formatter->_nf, UNUM_CURRENCY_CODE, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt == 0) {
	    CFStringRef localeName = CFLocaleGetIdentifier(formatter->_locale);
	    char buffer[BUFFER_SIZE];
	    const char *cstr = CFStringGetCStringPtr(localeName, kCFStringEncodingASCII);
	    if (NULL == cstr) {
		if (CFStringGetCString(localeName, buffer, BUFFER_SIZE, kCFStringEncodingASCII)) cstr = buffer;
	    }
	    if (NULL == cstr) {
	        return NULL;
	    }
	    UErrorCode status = U_ZERO_ERROR;
	    UNumberFormat *nf = __cficu_unum_open(UNUM_CURRENCY, NULL, 0, cstr, NULL, &status);
	    if (NULL != nf) {
		cnt = __cficu_unum_getTextAttribute(nf, UNUM_CURRENCY_CODE, ubuffer, BUFFER_SIZE, &status);
		__cficu_unum_close(nf);
	    }
	}
	if (U_SUCCESS(status) && 0 < cnt && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterDecimalSeparatorKey == key) {
	cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_DECIMAL_SEPARATOR_SYMBOL, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterCurrencyDecimalSeparatorKey == key) {
	cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_MONETARY_SEPARATOR_SYMBOL, (UChar *)ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterAlwaysShowDecimalSeparatorKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_DECIMAL_ALWAYS_SHOWN);
	if (1) {
	    return CFRetain(n ? kCFBooleanTrue : kCFBooleanFalse);
	}
    } else if (kCFNumberFormatterGroupingSeparatorKey == key) {
	cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_GROUPING_SEPARATOR_SYMBOL, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterUseGroupingSeparatorKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_GROUPING_USED);
	if (1) {
	    return CFRetain(n ? kCFBooleanTrue : kCFBooleanFalse);
	}
   } else if (kCFNumberFormatterPercentSymbolKey == key) {
	cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_PERCENT_SYMBOL, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterZeroSymbolKey == key) {
        return formatter->_zeroSym ? CFRetain(formatter->_zeroSym) : NULL;
    } else if (kCFNumberFormatterNaNSymbolKey == key) {
	cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_NAN_SYMBOL, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterInfinitySymbolKey == key) {
	cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_INFINITY_SYMBOL, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterMinusSignKey == key) {
	cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_MINUS_SIGN_SYMBOL, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterPlusSignKey == key) {
	cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_PLUS_SIGN_SYMBOL, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterCurrencySymbolKey == key) {
	cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_CURRENCY_SYMBOL, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterExponentSymbolKey == key) {
	cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_EXPONENTIAL_SYMBOL, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterMinIntegerDigitsKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_MIN_INTEGER_DIGITS);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else if (kCFNumberFormatterMaxIntegerDigitsKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_MAX_INTEGER_DIGITS);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else if (kCFNumberFormatterMinFractionDigitsKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_MIN_FRACTION_DIGITS);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else if (kCFNumberFormatterMaxFractionDigitsKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_MAX_FRACTION_DIGITS);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else if (kCFNumberFormatterGroupingSizeKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_GROUPING_SIZE);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else if (kCFNumberFormatterSecondaryGroupingSizeKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_SECONDARY_GROUPING_SIZE);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else if (kCFNumberFormatterRoundingModeKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_ROUNDING_MODE);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else if (kCFNumberFormatterRoundingIncrementKey == key) {
	d = __cficu_unum_getDoubleAttribute(formatter->_nf, UNUM_ROUNDING_INCREMENT);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberDoubleType, &d);
	}
    } else if (kCFNumberFormatterFormatWidthKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_FORMAT_WIDTH);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else if (kCFNumberFormatterPaddingPositionKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_PADDING_POSITION);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else if (kCFNumberFormatterPaddingCharacterKey == key) {
	cnt = __cficu_unum_getTextAttribute(formatter->_nf, UNUM_PADDING_CHARACTER, ubuffer, BUFFER_SIZE, &status);
	if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
	    return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
	}
    } else if (kCFNumberFormatterDefaultFormatKey == key) {
	return formatter->_defformat ? CFRetain(formatter->_defformat) : NULL;
    } else if (kCFNumberFormatterMultiplierKey == key) {
        return formatter->_multiplier ? CFRetain(formatter->_multiplier) : NULL;
    } else if (kCFNumberFormatterPositivePrefixKey == key) {
        cnt = __cficu_unum_getTextAttribute(formatter->_nf, UNUM_POSITIVE_PREFIX, ubuffer, BUFFER_SIZE, &status);
        if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
            return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
        }
    } else if (kCFNumberFormatterPositiveSuffixKey == key) {
        cnt = __cficu_unum_getTextAttribute(formatter->_nf, UNUM_POSITIVE_SUFFIX, ubuffer, BUFFER_SIZE, &status);
        if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
            return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
        }
    } else if (kCFNumberFormatterNegativePrefixKey == key) {
        cnt = __cficu_unum_getTextAttribute(formatter->_nf, UNUM_NEGATIVE_PREFIX, ubuffer, BUFFER_SIZE, &status);
        if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
            return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
        }
    } else if (kCFNumberFormatterNegativeSuffixKey == key) {
        cnt = __cficu_unum_getTextAttribute(formatter->_nf, UNUM_NEGATIVE_SUFFIX, ubuffer, BUFFER_SIZE, &status);
        if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
            return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
        }
    } else if (kCFNumberFormatterPerMillSymbolKey == key) {
        cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_PERMILL_SYMBOL, ubuffer, BUFFER_SIZE, &status);
        if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
            return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
        }
    } else if (kCFNumberFormatterInternationalCurrencySymbolKey == key) {
        cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_INTL_CURRENCY_SYMBOL, ubuffer, BUFFER_SIZE, &status);
        if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
            return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
        }
    } else if (kCFNumberFormatterCurrencyGroupingSeparatorKey == key) {
        cnt = __cficu_unum_getSymbol(formatter->_nf, UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL, ubuffer, BUFFER_SIZE, &status);
        if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
            return CFStringCreateWithCharacters(CFGetAllocator(formatter), (const UniChar *)ubuffer, cnt);
        }
    } else if (kCFNumberFormatterIsLenientKey == key) {
	// __cficu_unum_getAttribute(, UNUM_LENIENT_PARSE) is undefined.
	return CFRetain(formatter->_isLenient ? kCFBooleanTrue : kCFBooleanFalse);
    } else if (kCFNumberFormatterUseSignificantDigitsKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_SIGNIFICANT_DIGITS_USED);
	if (1) {
	    return CFRetain(n ? kCFBooleanTrue : kCFBooleanFalse);
	}
    } else if (kCFNumberFormatterMinSignificantDigitsKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_MIN_SIGNIFICANT_DIGITS);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else if (kCFNumberFormatterMaxSignificantDigitsKey == key) {
	n = __cficu_unum_getAttribute(formatter->_nf, UNUM_MAX_SIGNIFICANT_DIGITS);
	if (1) {
	    return CFNumberCreate(CFGetAllocator(formatter), kCFNumberSInt32Type, &n);
	}
    } else {
	CFAssert3(0, __kCFLogAssertion, "%s(): unknown key %p (%@)", __PRETTY_FUNCTION__, key, key);
    }
    return NULL;
}


Boolean CFNumberFormatterGetDecimalInfoForCurrencyCode(CFStringRef currencyCode, int32_t *defaultFractionDigits, double *roundingIncrement) {
    UChar ubuffer[4];
    __CFGenericValidateType(currencyCode, CFStringGetTypeID());
    CFAssert1(3 == CFStringGetLength(currencyCode), __kCFLogAssertion, "%s(): currencyCode is not 3 characters", __PRETTY_FUNCTION__);
    CFStringGetCharacters(currencyCode, CFRangeMake(0, 3), (UniChar *)ubuffer);
    ubuffer[3] = 0;
    UErrorCode icuStatus = U_ZERO_ERROR;
    if (defaultFractionDigits) *defaultFractionDigits = __cficu_ucurr_getDefaultFractionDigits(ubuffer, &icuStatus);
    if (roundingIncrement) *roundingIncrement = __cficu_ucurr_getRoundingIncrement(ubuffer, &icuStatus);
    if (U_FAILURE(icuStatus))
        return false;
    return (!defaultFractionDigits || 0 <= *defaultFractionDigits) && (!roundingIncrement || 0.0 <= *roundingIncrement);
}

// This is for NSNumberFormatter use only!
void *_CFNumberFormatterGetFormatter(CFNumberFormatterRef formatter) {
    return (void *)formatter->_nf;
}

#undef BUFFER_SIZE

