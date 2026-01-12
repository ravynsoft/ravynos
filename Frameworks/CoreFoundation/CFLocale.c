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

/*  CFLocale.c
    Copyright (c) 2002-2014, Apple Inc. All rights reserved.
    Responsibility: David Smith
*/

// Note the header file is in the OpenSource set (stripped to almost nothing), but not the .c file

#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFPreferences.h>
#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFNumber.h>
#include "CFInternal.h"
#include "CFLocaleInternal.h"
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
#include <unicode/uloc.h>           // ICU locales
#include <unicode/ulocdata.h>       // ICU locale data
#include <unicode/ucal.h>
#include <unicode/ucurr.h>          // ICU currency functions
#include <unicode/uset.h>           // ICU Unicode sets
#include <unicode/putil.h>          // ICU low-level utilities
#include <unicode/umsg.h>           // ICU message formatting
#include <unicode/ucol.h>
#endif
#include <CoreFoundation/CFNumberFormatter.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if DEPLOYMENT_TARGET_EMBEDDED_MINI
// Some compatability definitions
#define ULOC_FULLNAME_CAPACITY 157
#define ULOC_KEYWORD_AND_VALUES_CAPACITY 100

//typedef long UErrorCode;
//#define U_BUFFER_OVERFLOW_ERROR 15
//#define U_ZERO_ERROR 0
//
//typedef uint16_t UChar;
#endif


CONST_STRING_DECL(kCFLocaleCurrentLocaleDidChangeNotification, "kCFLocaleCurrentLocaleDidChangeNotification")

static const char *kCalendarKeyword = "calendar";
static const char *kCollationKeyword = "collation";
#define kMaxICUNameSize 1024

typedef struct __CFLocale *CFMutableLocaleRef;

PE_CONST_STRING_DECL(__kCFLocaleCollatorID, "locale:collator id")


enum {
    __kCFLocaleKeyTableCount = 21
};

struct key_table {
    CFStringRef key;
    bool (*get)(CFLocaleRef, bool user, CFTypeRef *, CFStringRef context);  // returns an immutable copy & reference
    bool (*set)(CFMutableLocaleRef, CFTypeRef, CFStringRef context);
    bool (*name)(const char *, const char *, CFStringRef *); 
    CFStringRef context;
};


// Must forward decl. these functions:
static bool __CFLocaleCopyLocaleID(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleSetNOP(CFMutableLocaleRef locale, CFTypeRef cf, CFStringRef context);
static bool __CFLocaleFullName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleCopyCodes(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCountryName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleScriptName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleLanguageName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleCurrencyShortName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleCopyExemplarCharSet(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleVariantName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleNoName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleCopyCalendarID(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCalendarName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleCollationName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleCopyUsesMetric(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCopyCalendar(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCopyCollationID(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCopyMeasurementSystem(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCopyNumberFormat(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCopyNumberFormat2(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCurrencyFullName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleCopyCollatorID(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCopyDelimiter(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);

// Note string members start with an extra &, and are fixed up at init time
static struct key_table __CFLocaleKeyTable[__kCFLocaleKeyTableCount] = {
    {(CFStringRef)&kCFLocaleIdentifierKey, __CFLocaleCopyLocaleID, __CFLocaleSetNOP, __CFLocaleFullName, NULL},
    {(CFStringRef)&kCFLocaleLanguageCodeKey, __CFLocaleCopyCodes, __CFLocaleSetNOP, __CFLocaleLanguageName, (CFStringRef)&kCFLocaleLanguageCodeKey},
    {(CFStringRef)&kCFLocaleCountryCodeKey, __CFLocaleCopyCodes, __CFLocaleSetNOP, __CFLocaleCountryName, (CFStringRef)&kCFLocaleCountryCodeKey},
    {(CFStringRef)&kCFLocaleScriptCodeKey, __CFLocaleCopyCodes, __CFLocaleSetNOP, __CFLocaleScriptName, (CFStringRef)&kCFLocaleScriptCodeKey},
    {(CFStringRef)&kCFLocaleVariantCodeKey, __CFLocaleCopyCodes, __CFLocaleSetNOP, __CFLocaleVariantName, (CFStringRef)&kCFLocaleVariantCodeKey},
    {(CFStringRef)&kCFLocaleExemplarCharacterSetKey, __CFLocaleCopyExemplarCharSet, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {(CFStringRef)&kCFLocaleCalendarIdentifierKey, __CFLocaleCopyCalendarID, __CFLocaleSetNOP, __CFLocaleCalendarName, NULL},
    {(CFStringRef)&kCFLocaleCalendarKey, __CFLocaleCopyCalendar, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {(CFStringRef)&kCFLocaleCollationIdentifierKey, __CFLocaleCopyCollationID, __CFLocaleSetNOP, __CFLocaleCollationName, NULL},
    {(CFStringRef)&kCFLocaleUsesMetricSystemKey, __CFLocaleCopyUsesMetric, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {(CFStringRef)&kCFLocaleMeasurementSystemKey, __CFLocaleCopyMeasurementSystem, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {(CFStringRef)&kCFLocaleDecimalSeparatorKey, __CFLocaleCopyNumberFormat, __CFLocaleSetNOP, __CFLocaleNoName, (CFStringRef)&kCFNumberFormatterDecimalSeparatorKey},
    {(CFStringRef)&kCFLocaleGroupingSeparatorKey, __CFLocaleCopyNumberFormat, __CFLocaleSetNOP, __CFLocaleNoName, (CFStringRef)&kCFNumberFormatterGroupingSeparatorKey},
    {(CFStringRef)&kCFLocaleCurrencySymbolKey, __CFLocaleCopyNumberFormat2, __CFLocaleSetNOP, __CFLocaleCurrencyShortName, (CFStringRef)&kCFNumberFormatterCurrencySymbolKey},
    {(CFStringRef)&kCFLocaleCurrencyCodeKey, __CFLocaleCopyNumberFormat2, __CFLocaleSetNOP, __CFLocaleCurrencyFullName, (CFStringRef)&kCFNumberFormatterCurrencyCodeKey},
    {(CFStringRef)&kCFLocaleCollatorIdentifierKey, __CFLocaleCopyCollatorID, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {(CFStringRef)&__kCFLocaleCollatorID, __CFLocaleCopyCollatorID, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {(CFStringRef)&kCFLocaleQuotationBeginDelimiterKey, __CFLocaleCopyDelimiter, __CFLocaleSetNOP, __CFLocaleNoName, (CFStringRef)&kCFLocaleQuotationBeginDelimiterKey},
    {(CFStringRef)&kCFLocaleQuotationEndDelimiterKey, __CFLocaleCopyDelimiter, __CFLocaleSetNOP, __CFLocaleNoName, (CFStringRef)&kCFLocaleQuotationEndDelimiterKey},
    {(CFStringRef)&kCFLocaleAlternateQuotationBeginDelimiterKey, __CFLocaleCopyDelimiter, __CFLocaleSetNOP, __CFLocaleNoName, (CFStringRef)&kCFLocaleAlternateQuotationBeginDelimiterKey},
    {(CFStringRef)&kCFLocaleAlternateQuotationEndDelimiterKey, __CFLocaleCopyDelimiter, __CFLocaleSetNOP, __CFLocaleNoName, (CFStringRef)&kCFLocaleAlternateQuotationEndDelimiterKey},
};


static CFLocaleRef __CFLocaleSystem = NULL;
static CFMutableDictionaryRef __CFLocaleCache = NULL;
static CFLock_t __CFLocaleGlobalLock = CFLockInit;

struct __CFLocale {
    CFRuntimeBase _base;
    CFStringRef _identifier;    // canonical identifier, never NULL
    CFMutableDictionaryRef _cache;
    CFMutableDictionaryRef _overrides;
    CFDictionaryRef _prefs;
    CFLock_t _lock;
    Boolean _nullLocale;
};
 
CF_PRIVATE Boolean __CFLocaleGetNullLocale(struct __CFLocale *locale) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), Boolean, (NSLocale *)locale, _nullLocale);
    return locale->_nullLocale;
}

CF_PRIVATE void __CFLocaleSetNullLocale(struct __CFLocale *locale) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), void, (NSLocale *)locale, _setNullLocale);
    locale->_nullLocale = true;
}

/* Flag bits */
enum {      /* Bits 0-1 */
    __kCFLocaleOrdinary = 0,
    __kCFLocaleSystem = 1,
    __kCFLocaleUser = 2,
    __kCFLocaleCustom = 3
};

CF_INLINE CFIndex __CFLocaleGetType(CFLocaleRef locale) {
    return __CFBitfieldGetValue(((const CFRuntimeBase *)locale)->_cfinfo[CF_INFO_BITS], 1, 0);
}

CF_INLINE void __CFLocaleSetType(CFLocaleRef locale, CFIndex type) {
    __CFBitfieldSetValue(((CFRuntimeBase *)locale)->_cfinfo[CF_INFO_BITS], 1, 0, (uint8_t)type);
}

CF_INLINE void __CFLocaleLockGlobal(void) {
    __CFLock(&__CFLocaleGlobalLock);
}

CF_INLINE void __CFLocaleUnlockGlobal(void) {
    __CFUnlock(&__CFLocaleGlobalLock);
}

CF_INLINE void __CFLocaleLock(CFLocaleRef locale) {
    __CFLock(&((struct __CFLocale *)locale)->_lock);
}

CF_INLINE void __CFLocaleUnlock(CFLocaleRef locale) {
    __CFUnlock(&((struct __CFLocale *)locale)->_lock);
}


static Boolean __CFLocaleEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFLocaleRef locale1 = (CFLocaleRef)cf1;
    CFLocaleRef locale2 = (CFLocaleRef)cf2;
    // a user locale and a locale created with an ident are not the same even if their contents are
    if (__CFLocaleGetType(locale1) != __CFLocaleGetType(locale2)) return false;
    if (!CFEqual(locale1->_identifier, locale2->_identifier)) return false;
    if (NULL == locale1->_overrides && NULL != locale2->_overrides) return false;
    if (NULL != locale1->_overrides && NULL == locale2->_overrides) return false;
    if (NULL != locale1->_overrides && !CFEqual(locale1->_overrides, locale2->_overrides)) return false;
    if (__kCFLocaleUser == __CFLocaleGetType(locale1)) {
        return CFEqual(locale1->_prefs, locale2->_prefs);
    }
    return true;
}

static CFHashCode __CFLocaleHash(CFTypeRef cf) {
    CFLocaleRef locale = (CFLocaleRef)cf;
    return CFHash(locale->_identifier);
}

static CFStringRef __CFLocaleCopyDescription(CFTypeRef cf) {
    CFLocaleRef locale = (CFLocaleRef)cf;
    const char *type = NULL;
    switch (__CFLocaleGetType(locale)) {
    case __kCFLocaleOrdinary: type = "ordinary"; break;
    case __kCFLocaleSystem: type = "system"; break;
    case __kCFLocaleUser: type = "user"; break;
    case __kCFLocaleCustom: type = "custom"; break;
    }
    return CFStringCreateWithFormat(CFGetAllocator(locale), NULL, CFSTR("<CFLocale %p [%p]>{type = %s, identifier = '%@'}"), cf, CFGetAllocator(locale), type, locale->_identifier);
}

static void __CFLocaleDeallocate(CFTypeRef cf) {
    CFLocaleRef locale = (CFLocaleRef)cf;
    CFRelease(locale->_identifier);
    if (NULL != locale->_cache) CFRelease(locale->_cache);
    if (NULL != locale->_overrides) CFRelease(locale->_overrides);
    if (NULL != locale->_prefs) CFRelease(locale->_prefs);
}

static CFTypeID __kCFLocaleTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFLocaleClass = {
    0,
    "CFLocale",
    NULL,   // init
    NULL,   // copy
    __CFLocaleDeallocate,
    __CFLocaleEqual,
    __CFLocaleHash,
    NULL,   // 
    __CFLocaleCopyDescription
};

CFTypeID CFLocaleGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{
        __kCFLocaleTypeID = _CFRuntimeRegisterClass(&__CFLocaleClass); // initOnce covered
        for (CFIndex idx = 0; idx < __kCFLocaleKeyTableCount; idx++) {
            // table fixup to workaround compiler/language limitations
            __CFLocaleKeyTable[idx].key = *((CFStringRef *)__CFLocaleKeyTable[idx].key);
            if (NULL != __CFLocaleKeyTable[idx].context) {
                __CFLocaleKeyTable[idx].context = *((CFStringRef *)__CFLocaleKeyTable[idx].context);
            }
        }
    });
    return __kCFLocaleTypeID;
}

CFLocaleRef CFLocaleGetSystem(void) {
    CFLocaleRef locale;
    __CFLocaleLockGlobal();
    if (NULL == __CFLocaleSystem) {
	__CFLocaleUnlockGlobal();
	locale = CFLocaleCreate(kCFAllocatorSystemDefault, CFSTR(""));
	if (!locale) return NULL;
	__CFLocaleSetType(locale, __kCFLocaleSystem);
	__CFLocaleLockGlobal();
	if (NULL == __CFLocaleSystem) {
	    __CFLocaleSystem = locale;
	} else {
	    if (locale) CFRelease(locale);
	}
    }
    locale = __CFLocaleSystem ? (CFLocaleRef)CFRetain(__CFLocaleSystem) : NULL;
    __CFLocaleUnlockGlobal();
    return locale;
}

extern CFDictionaryRef __CFXPreferencesCopyCurrentApplicationState(void);

static CFLocaleRef __CFLocaleCurrent = NULL;


#if DEPLOYMENT_TARGET_MACOSX
#define FALLBACK_LOCALE_NAME CFSTR("")
#elif DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#define FALLBACK_LOCALE_NAME CFSTR("en_US")
#elif DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
#define FALLBACK_LOCALE_NAME CFSTR("en_US")
#endif

static CFLocaleRef _CFLocaleCopyCurrentGuts(CFStringRef name, Boolean useCache, CFDictionaryRef overridePrefs) {
    
    CFStringRef ident = NULL;
    // We cannot be helpful here, because it causes performance problems,
    // even though the preference lookup is relatively quick, as there are
    // things which call this function thousands or millions of times in
    // a short period.
    if (!name) {
#if 0 // DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
        name = (CFStringRef)CFPreferencesCopyAppValue(CFSTR("AppleLocale"), kCFPreferencesCurrentApplication);
#endif
    } else {
        CFRetain(name);
    }
    if (name && (CFStringGetTypeID() == CFGetTypeID(name))) {
        ident = CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorSystemDefault, name);
    }
    if (name) CFRelease(name);
    
    if (useCache) {
        CFLocaleRef oldLocale = NULL;
        __CFLocaleLockGlobal();
        if (__CFLocaleCurrent) {
            if (ident && !CFEqual(__CFLocaleCurrent->_identifier, ident)) {
                oldLocale = __CFLocaleCurrent;
                __CFLocaleCurrent = NULL;
            } else {
                CFLocaleRef res = __CFLocaleCurrent;
                CFRetain(res);
                __CFLocaleUnlockGlobal();
                if (ident) CFRelease(ident);
                return res;
            }
        }
        __CFLocaleUnlockGlobal();
        if (oldLocale) CFRelease(oldLocale);
    }
    
    CFDictionaryRef prefs = NULL;
    
    struct __CFLocale *locale;
    uint32_t size = sizeof(struct __CFLocale) - sizeof(CFRuntimeBase);
    locale = (struct __CFLocale *)_CFRuntimeCreateInstance(kCFAllocatorSystemDefault, CFLocaleGetTypeID(), size, NULL);
    if (NULL == locale) {
	if (prefs) CFRelease(prefs);
	if (ident) CFRelease(ident);
	return NULL;
    }
    __CFLocaleSetType(locale, __kCFLocaleUser);
    if (NULL == ident) ident = (CFStringRef)CFRetain(FALLBACK_LOCALE_NAME);
    locale->_identifier = ident;
    locale->_cache = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, NULL, &kCFTypeDictionaryValueCallBacks);
    locale->_overrides = NULL;
    locale->_prefs = prefs;
    locale->_lock = CFLockInit;
    locale->_nullLocale = false;
    
    if (useCache) {
        __CFLocaleLockGlobal();
        if (NULL == __CFLocaleCurrent) {
            __CFLocaleCurrent = locale;
        } else {
            CFRelease(locale);
        }
        locale = (struct __CFLocale *)CFRetain(__CFLocaleCurrent);
        __CFLocaleUnlockGlobal();
    }
    return locale;
}

/*
 <rdar://problem/13834276> NSDateFormatter: Cannot specify force12HourTime/force24HourTime
 This returns an instance of CFLocale that's set up exactly like it would be if the user changed the current locale to that identifier, then called CFLocaleCopyCurrent()
 */
CFLocaleRef _CFLocaleCopyAsIfCurrent(CFStringRef name) {
    return _CFLocaleCopyCurrentGuts(name, false, NULL);
}

/*
 <rdar://problem/14032388> Need the ability to initialize a CFLocaleRef from a preferences dictionary
 This returns an instance of CFLocale that's set up exactly like it would be if the user changed the current locale to that identifier, set the preferences keys in the overrides dictionary, then called CFLocaleCopyCurrent()
 */
CFLocaleRef _CFLocaleCopyAsIfCurrentWithOverrides(CFStringRef name, CFDictionaryRef overrides) {
    return _CFLocaleCopyCurrentGuts(name, false, overrides);
}

CFLocaleRef CFLocaleCopyCurrent(void) {
    return _CFLocaleCopyCurrentGuts(NULL, true, NULL);
}

CF_PRIVATE CFDictionaryRef __CFLocaleGetPrefs(CFLocaleRef locale) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), CFDictionaryRef, (NSLocale *)locale, _prefs);
    return locale->_prefs;
}

CFLocaleRef CFLocaleCreate(CFAllocatorRef allocator, CFStringRef identifier) {
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    __CFGenericValidateType(identifier, CFStringGetTypeID());
    CFStringRef localeIdentifier = NULL;
    if (identifier) {
	localeIdentifier = CFLocaleCreateCanonicalLocaleIdentifierFromString(allocator, identifier);
    }
    if (NULL == localeIdentifier) return NULL;
    CFStringRef old = localeIdentifier;
    localeIdentifier = (CFStringRef)CFStringCreateCopy(allocator, localeIdentifier);
    CFRelease(old);
    __CFLocaleLockGlobal();
    // Look for cases where we can return a cached instance.
    // We only use cached objects if the allocator is the system
    // default allocator.
    if (!allocator) allocator = __CFGetDefaultAllocator();
    Boolean canCache = _CFAllocatorIsSystemDefault(allocator);
    if (canCache && __CFLocaleCache) {
	CFLocaleRef locale = (CFLocaleRef)CFDictionaryGetValue(__CFLocaleCache, localeIdentifier);
	if (locale) {
	    CFRetain(locale);
	    __CFLocaleUnlockGlobal();
	    CFRelease(localeIdentifier);
	    return locale;
	}
    }
    struct __CFLocale *locale = NULL;
    uint32_t size = sizeof(struct __CFLocale) - sizeof(CFRuntimeBase);
    locale = (struct __CFLocale *)_CFRuntimeCreateInstance(allocator, CFLocaleGetTypeID(), size, NULL);
    if (NULL == locale) {
	return NULL;
    }
    __CFLocaleSetType(locale, __kCFLocaleOrdinary);
    locale->_identifier = localeIdentifier;
    locale->_cache = CFDictionaryCreateMutable(allocator, 0, NULL, &kCFTypeDictionaryValueCallBacks);
    locale->_overrides = NULL;
    locale->_prefs = NULL;
    locale->_lock = CFLockInit;
    if (canCache) {
	if (NULL == __CFLocaleCache) {
	    __CFLocaleCache = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	}
        CFDictionarySetValue(__CFLocaleCache, localeIdentifier, locale);
    }
    __CFLocaleUnlockGlobal();
    return (CFLocaleRef)locale;
}

CFLocaleRef CFLocaleCreateCopy(CFAllocatorRef allocator, CFLocaleRef locale) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), CFLocaleRef, (NSLocale *)locale, copy);
    return (CFLocaleRef)CFRetain(locale);
}

CFStringRef CFLocaleGetIdentifier(CFLocaleRef locale) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), CFStringRef, (NSLocale *)locale, localeIdentifier);
    return locale->_identifier;
}

CFTypeRef CFLocaleGetValue(CFLocaleRef locale, CFStringRef key) {
#if DEPLOYMENT_TARGET_MACOSX
    if (!_CFExecutableLinkedOnOrAfter(CFSystemVersionSnowLeopard)) {
	// Hack for Opera, which is using the hard-coded string value below instead of
        // the perfectly good public kCFLocaleCountryCode constant, for whatever reason.
	if (key && CFEqual(key, CFSTR("locale:country code"))) {
	    key = kCFLocaleCountryCodeKey;
	}
    }
#endif
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), CFTypeRef, (NSLocale *)locale, objectForKey:(id)key);
    CFIndex idx, slot = -1;
    for (idx = 0; idx < __kCFLocaleKeyTableCount; idx++) {
	if (__CFLocaleKeyTable[idx].key == key) {
	    slot = idx;
	    break;
	}
    }
    if (-1 == slot && NULL != key) {
	for (idx = 0; idx < __kCFLocaleKeyTableCount; idx++) {
	    if (CFEqual(__CFLocaleKeyTable[idx].key, key)) {
		slot = idx;
		break;
	    }
	}
    }
    if (-1 == slot) {
	return NULL;
    }
    CFTypeRef value;
    if (NULL != locale->_overrides && CFDictionaryGetValueIfPresent(locale->_overrides, __CFLocaleKeyTable[slot].key, &value)) {
	return value;
    }
    __CFLocaleLock(locale);
    if (CFDictionaryGetValueIfPresent(locale->_cache, __CFLocaleKeyTable[slot].key, &value)) {
	__CFLocaleUnlock(locale);
	return value;
    }
    if (__kCFLocaleUser == __CFLocaleGetType(locale) && __CFLocaleKeyTable[slot].get(locale, true, &value, __CFLocaleKeyTable[slot].context)) {
	if (value) CFDictionarySetValue(locale->_cache, __CFLocaleKeyTable[idx].key, value);
	if (value) CFRelease(value);
	__CFLocaleUnlock(locale);
	return value;
    }
    if (__CFLocaleKeyTable[slot].get(locale, false, &value, __CFLocaleKeyTable[slot].context)) {
	if (value) CFDictionarySetValue(locale->_cache, __CFLocaleKeyTable[idx].key, value);
	if (value) CFRelease(value);
	__CFLocaleUnlock(locale);
	return value;
    }
    __CFLocaleUnlock(locale);
    return NULL;
}

CFStringRef CFLocaleCopyDisplayNameForPropertyValue(CFLocaleRef displayLocale, CFStringRef key, CFStringRef value) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), CFStringRef, (NSLocale *)displayLocale, _copyDisplayNameForKey:(id)key value:(id)value);
    CFIndex idx, slot = -1;
    for (idx = 0; idx < __kCFLocaleKeyTableCount; idx++) {
	if (__CFLocaleKeyTable[idx].key == key) {
	    slot = idx;
	    break;
	}
    }
    if (-1 == slot && NULL != key) {
	for (idx = 0; idx < __kCFLocaleKeyTableCount; idx++) {
	    if (CFEqual(__CFLocaleKeyTable[idx].key, key)) {
		slot = idx;
		break;
	    }
	}
    }
    if (-1 == slot || !value) {
	return NULL;
    }
    // Get the locale ID as a C string
    char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    char cValue[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    if (CFStringGetCString(displayLocale->_identifier, localeID, sizeof(localeID)/sizeof(localeID[0]), kCFStringEncodingASCII) && CFStringGetCString(value, cValue, sizeof(cValue)/sizeof(char), kCFStringEncodingASCII)) {
        CFStringRef result;
        if ((NULL == displayLocale->_prefs) && __CFLocaleKeyTable[slot].name(localeID, cValue, &result)) {
            return result;
        }

        // We could not find a result using the requested language. Fall back through all preferred languages.
        CFArrayRef langPref = NULL;
	if (displayLocale->_prefs) {
	    langPref = (CFArrayRef)CFDictionaryGetValue(displayLocale->_prefs, CFSTR("AppleLanguages"));
	    if (langPref) CFRetain(langPref);
	} else {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
	    langPref = (CFArrayRef)CFPreferencesCopyAppValue(CFSTR("AppleLanguages"), kCFPreferencesCurrentApplication);
#endif
	}
        if (langPref != NULL) {
            CFIndex count = CFArrayGetCount(langPref);
            CFIndex i;
            bool success = false;
            for (i = 0; i < count && !success; ++i) {
                CFStringRef language = (CFStringRef)CFArrayGetValueAtIndex(langPref, i);
                CFStringRef cleanLanguage = CFLocaleCreateCanonicalLanguageIdentifierFromString(kCFAllocatorSystemDefault, language);
                if (CFStringGetCString(cleanLanguage, localeID, sizeof(localeID)/sizeof(localeID[0]), kCFStringEncodingASCII)) {
                    success = __CFLocaleKeyTable[slot].name(localeID, cValue, &result);
		}
                CFRelease(cleanLanguage);
            }
	    CFRelease(langPref);
            if (success)
                return result;
        }
    }
    return NULL;
}

CFArrayRef CFLocaleCopyAvailableLocaleIdentifiers(void) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    int32_t locale, localeCount = uloc_countAvailable();
    CFMutableSetRef working = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
    for (locale = 0; locale < localeCount; ++locale) {
        const char *localeID = uloc_getAvailable(locale);
        CFStringRef string1 = CFStringCreateWithCString(kCFAllocatorSystemDefault, localeID, kCFStringEncodingASCII);
	// do not include canonicalized version as IntlFormats cannot cope with that in its popup
	CFSetAddValue(working, string1);
        CFRelease(string1);
    }
    CFIndex cnt = CFSetGetCount(working);
    STACK_BUFFER_DECL(const void *, buffer, cnt);
    CFSetGetValues(working, buffer);
    CFArrayRef result = CFArrayCreate(kCFAllocatorSystemDefault, buffer, cnt, &kCFTypeArrayCallBacks);
    CFRelease(working);
    return result;
#else
    return CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
#endif
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
static CFArrayRef __CFLocaleCopyCStringsAsArray(const char* const* p) {
    CFMutableArrayRef working = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    for (; *p; ++p) {
        CFStringRef string = CFStringCreateWithCString(kCFAllocatorSystemDefault, *p, kCFStringEncodingASCII);
        CFArrayAppendValue(working, string);
        CFRelease(string);
    }
    CFArrayRef result = CFArrayCreateCopy(kCFAllocatorSystemDefault, working);
    CFRelease(working);
    return result;
}

static CFArrayRef __CFLocaleCopyUEnumerationAsArray(UEnumeration *enumer, UErrorCode *icuErr) {
    const UChar *next = NULL;
    int32_t len = 0;
    CFMutableArrayRef working = NULL;
    if (U_SUCCESS(*icuErr)) {
        working = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    }
    while ((next = uenum_unext(enumer, &len, icuErr)) && U_SUCCESS(*icuErr)) {
        CFStringRef string = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (const UniChar *)next, (CFIndex) len);
        CFArrayAppendValue(working, string);
        CFRelease(string);
    }
    if (*icuErr == U_INDEX_OUTOFBOUNDS_ERROR) {
        *icuErr = U_ZERO_ERROR;      // Temp: Work around bug (ICU 5220) in ucurr enumerator
    }
    CFArrayRef result = NULL;
    if (U_SUCCESS(*icuErr)) {
        result = CFArrayCreateCopy(kCFAllocatorSystemDefault, working);
    }
    if (working != NULL) {
        CFRelease(working);
    }
    return result;
}
#endif

CFArrayRef CFLocaleCopyISOLanguageCodes(void) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    const char* const* p = uloc_getISOLanguages();
    return __CFLocaleCopyCStringsAsArray(p);
#else
    return CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
#endif
}

CFArrayRef CFLocaleCopyISOCountryCodes(void) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    const char* const* p = uloc_getISOCountries();
    return __CFLocaleCopyCStringsAsArray(p);
#else
    return CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
#endif
}

CFArrayRef CFLocaleCopyISOCurrencyCodes(void) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    UErrorCode icuStatus = U_ZERO_ERROR;
    UEnumeration *enumer = ucurr_openISOCurrencies(UCURR_ALL, &icuStatus);
    CFArrayRef result = __CFLocaleCopyUEnumerationAsArray(enumer, &icuStatus);
    uenum_close(enumer);
#else
    CFArrayRef result = CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
#endif
    return result;
}

CFArrayRef CFLocaleCopyCommonISOCurrencyCodes(void) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    UErrorCode icuStatus = U_ZERO_ERROR;
    UEnumeration *enumer = ucurr_openISOCurrencies(UCURR_COMMON|UCURR_NON_DEPRECATED, &icuStatus);
    CFArrayRef result = __CFLocaleCopyUEnumerationAsArray(enumer, &icuStatus);
    uenum_close(enumer);
#else
    CFArrayRef result = CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
#endif
    return result;
}

CFStringRef CFLocaleCreateLocaleIdentifierFromWindowsLocaleCode(CFAllocatorRef allocator, uint32_t lcid) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    char buffer[kMaxICUNameSize];
    UErrorCode status = U_ZERO_ERROR;
    int32_t ret = uloc_getLocaleForLCID(lcid, buffer, kMaxICUNameSize, &status);
    if (U_FAILURE(status) || kMaxICUNameSize <= ret) return NULL;
    CFStringRef str = CFStringCreateWithCString(kCFAllocatorSystemDefault, buffer, kCFStringEncodingASCII);
    CFStringRef ident = CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorSystemDefault, str);
    CFRelease(str);
    return ident;
#else
    return CFSTR("");
#endif
}

uint32_t CFLocaleGetWindowsLocaleCodeFromLocaleIdentifier(CFStringRef localeIdentifier) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    CFStringRef ident = CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorSystemDefault, localeIdentifier);
    char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    Boolean b = ident ? CFStringGetCString(ident, localeID, sizeof(localeID)/sizeof(char), kCFStringEncodingASCII) : false;
    if (ident) CFRelease(ident);
    return b ? uloc_getLCID(localeID) : 0;
#else
    return 0;
#endif
}

CFLocaleLanguageDirection CFLocaleGetLanguageCharacterDirection(CFStringRef isoLangCode) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    Boolean b = isoLangCode ? CFStringGetCString(isoLangCode, localeID, sizeof(localeID)/sizeof(char), kCFStringEncodingASCII) : false;
    CFLocaleLanguageDirection dir;
    UErrorCode status = U_ZERO_ERROR;
    ULayoutType idir = b ? uloc_getCharacterOrientation(localeID, &status) : ULOC_LAYOUT_UNKNOWN;
    switch (idir) {
    case ULOC_LAYOUT_LTR: dir = kCFLocaleLanguageDirectionLeftToRight; break;
    case ULOC_LAYOUT_RTL: dir = kCFLocaleLanguageDirectionRightToLeft; break;
    case ULOC_LAYOUT_TTB: dir = kCFLocaleLanguageDirectionTopToBottom; break;
    case ULOC_LAYOUT_BTT: dir = kCFLocaleLanguageDirectionBottomToTop; break;
    default: dir = kCFLocaleLanguageDirectionUnknown; break;
    }
    return dir;
#else
    return kCFLocaleLanguageDirectionLeftToRight;
#endif
}

CFLocaleLanguageDirection CFLocaleGetLanguageLineDirection(CFStringRef isoLangCode) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    Boolean b = isoLangCode ? CFStringGetCString(isoLangCode, localeID, sizeof(localeID)/sizeof(char), kCFStringEncodingASCII) : false;
    CFLocaleLanguageDirection dir;
    UErrorCode status = U_ZERO_ERROR;
    ULayoutType idir = b ? uloc_getLineOrientation(localeID, &status) : ULOC_LAYOUT_UNKNOWN;
    switch (idir) {
    case ULOC_LAYOUT_LTR: dir = kCFLocaleLanguageDirectionLeftToRight; break;
    case ULOC_LAYOUT_RTL: dir = kCFLocaleLanguageDirectionRightToLeft; break;
    case ULOC_LAYOUT_TTB: dir = kCFLocaleLanguageDirectionTopToBottom; break;
    case ULOC_LAYOUT_BTT: dir = kCFLocaleLanguageDirectionBottomToTop; break;
    default: dir = kCFLocaleLanguageDirectionUnknown; break;
    }
    return dir;
#else
    return kCFLocaleLanguageDirectionLeftToRight;
#endif
}

CFArrayRef CFLocaleCopyPreferredLanguages(void) {
    CFMutableArrayRef newArray = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
    CFArrayRef languagesArray = (CFArrayRef)CFPreferencesCopyAppValue(CFSTR("AppleLanguages"), kCFPreferencesCurrentApplication);
    if (languagesArray && (CFArrayGetTypeID() == CFGetTypeID(languagesArray))) {
	for (CFIndex idx = 0, cnt = CFArrayGetCount(languagesArray); idx < cnt; idx++) {
            CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(languagesArray, idx);
	    if (str && (CFStringGetTypeID() == CFGetTypeID(str))) {
                CFStringRef ident = CFLocaleCreateCanonicalLanguageIdentifierFromString(kCFAllocatorSystemDefault, str);
		CFArrayAppendValue(newArray, ident);
		CFRelease(ident);
	    }
	}
    }
    if (languagesArray)	CFRelease(languagesArray);
#endif
    return newArray;
}

// -------- -------- -------- -------- -------- --------

// These functions return true or false depending on the success or failure of the function.
// In the Copy case, this is failure to fill the *cf out parameter, and that out parameter is
// returned by reference WITH a retain on it.
static bool __CFLocaleSetNOP(CFMutableLocaleRef locale, CFTypeRef cf, CFStringRef context) {
    return false;
}

static bool __CFLocaleCopyLocaleID(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
    *cf = CFRetain(locale->_identifier);
    return true;
}


static bool __CFLocaleCopyCodes(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
    CFDictionaryRef codes = NULL;
    // this access of _cache is protected by the lock in CFLocaleGetValue()
    if (!CFDictionaryGetValueIfPresent(locale->_cache, CFSTR("__kCFLocaleCodes"), (const void **)&codes)) {
        codes = CFLocaleCreateComponentsFromLocaleIdentifier(kCFAllocatorSystemDefault, locale->_identifier);
	if (codes) CFDictionarySetValue(locale->_cache, CFSTR("__kCFLocaleCodes"), codes);
	if (codes) CFRelease(codes);
    }
    if (codes) {
	CFStringRef value = (CFStringRef)CFDictionaryGetValue(codes, context); // context is one of kCFLocale*Code constants
	if (value) CFRetain(value);
	*cf = value;
	return true;
    }
    return false;
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
CFCharacterSetRef _CFCreateCharacterSetFromUSet(USet *set) {
    UErrorCode icuErr = U_ZERO_ERROR;
    CFMutableCharacterSetRef working = CFCharacterSetCreateMutable(NULL);
    UChar   buffer[2048];   // Suitable for most small sets
    int32_t stringLen;

    if (working == NULL)
        return NULL;

    int32_t itemCount = uset_getItemCount(set);
    int32_t i;
    for (i = 0; i < itemCount; ++i)
    {
        UChar32   start, end;
        UChar * string;

        string = buffer;
        stringLen = uset_getItem(set, i, &start, &end, buffer, sizeof(buffer)/sizeof(UChar), &icuErr);
        if (icuErr == U_BUFFER_OVERFLOW_ERROR)
        {
            string = (UChar *) malloc(sizeof(UChar)*(stringLen+1));
            if (!string)
            {
                CFRelease(working);
                return NULL;
            }
            icuErr = U_ZERO_ERROR;
            (void) uset_getItem(set, i, &start, &end, string, stringLen+1, &icuErr);
        }
        if (U_FAILURE(icuErr))
        {
            if (string != buffer)
                free(string);
            CFRelease(working);
            return NULL;
        }
        if (stringLen <= 0)
            CFCharacterSetAddCharactersInRange(working, CFRangeMake(start, end-start+1));
        else
        {
            CFStringRef cfString = CFStringCreateWithCharactersNoCopy(kCFAllocatorSystemDefault, (UniChar *)string, stringLen, kCFAllocatorNull);
            CFCharacterSetAddCharactersInString(working, cfString);
            CFRelease(cfString);
        }
        if (string != buffer)
            free(string);
    }
    
    CFCharacterSetRef   result = CFCharacterSetCreateCopy(kCFAllocatorSystemDefault, working);
    CFRelease(working);
    return result;
}
#endif

static bool __CFLocaleCopyExemplarCharSet(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    if (CFStringGetCString(locale->_identifier, localeID, sizeof(localeID)/sizeof(char), kCFStringEncodingASCII)) {
        UErrorCode icuStatus = U_ZERO_ERROR;
	ULocaleData* uld = ulocdata_open(localeID, &icuStatus);
        USet *set = ulocdata_getExemplarSet(uld, NULL, USET_ADD_CASE_MAPPINGS, ULOCDATA_ES_STANDARD, &icuStatus);
	ulocdata_close(uld);
        if (U_FAILURE(icuStatus))
            return false;
        if (icuStatus == U_USING_DEFAULT_WARNING)   // If default locale used, force to empty set
            uset_clear(set);
        *cf = (CFTypeRef) _CFCreateCharacterSetFromUSet(set);
        uset_close(set);
        return (*cf != NULL);
    }
#endif
    return false;
}

static bool __CFLocaleCopyICUKeyword(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context, const char *keyword)
{
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    if (CFStringGetCString(locale->_identifier, localeID, sizeof(localeID)/sizeof(char), kCFStringEncodingASCII))
    {
        char value[ULOC_KEYWORD_AND_VALUES_CAPACITY];
        UErrorCode icuStatus = U_ZERO_ERROR;
        if (uloc_getKeywordValue(localeID, keyword, value, sizeof(value)/sizeof(char), &icuStatus) > 0 && U_SUCCESS(icuStatus))
        {
            *cf = (CFTypeRef) CFStringCreateWithCString(kCFAllocatorSystemDefault, value, kCFStringEncodingASCII);
            return true;
        }
    }
#endif
    *cf = NULL;
    return false;
}

static bool __CFLocaleCopyICUCalendarID(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context, const char *keyword) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    if (CFStringGetCString(locale->_identifier, localeID, sizeof(localeID)/sizeof(char), kCFStringEncodingASCII)) {
        UErrorCode icuStatus = U_ZERO_ERROR;
	UEnumeration *en = ucal_getKeywordValuesForLocale(keyword, localeID, TRUE, &icuStatus);
	int32_t len;
	const char *value = uenum_next(en, &len, &icuStatus);
	if (U_SUCCESS(icuStatus)) {
            *cf = (CFTypeRef) CFStringCreateWithCString(kCFAllocatorSystemDefault, value, kCFStringEncodingASCII);
	    uenum_close(en);
            return true;
        }
	uenum_close(en);
    }
#endif
    *cf = NULL;
    return false;
}

static bool __CFLocaleCopyCalendarID(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
    bool succeeded = __CFLocaleCopyICUKeyword(locale, user, cf, context, kCalendarKeyword);
    if (!succeeded) {
	succeeded = __CFLocaleCopyICUCalendarID(locale, user, cf, context, kCalendarKeyword);
    }
    if (succeeded) {
	if (CFEqual(*cf, kCFCalendarIdentifierGregorian)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierGregorian);
	} else if (CFEqual(*cf, kCFCalendarIdentifierBuddhist)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierBuddhist);
	} else if (CFEqual(*cf, kCFCalendarIdentifierJapanese)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierJapanese);
	} else if (CFEqual(*cf, kCFCalendarIdentifierIslamic)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierIslamic);
	} else if (CFEqual(*cf, kCFCalendarIdentifierIslamicCivil)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierIslamicCivil);
	} else if (CFEqual(*cf, kCFCalendarIdentifierHebrew)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierHebrew);
	} else if (CFEqual(*cf, kCFCalendarIdentifierChinese)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierChinese);
	} else if (CFEqual(*cf, kCFCalendarIdentifierRepublicOfChina)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierRepublicOfChina);
	} else if (CFEqual(*cf, kCFCalendarIdentifierPersian)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierPersian);
	} else if (CFEqual(*cf, kCFCalendarIdentifierIndian)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierIndian);
	} else if (CFEqual(*cf, kCFCalendarIdentifierISO8601)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierISO8601);
	} else if (CFEqual(*cf, kCFCalendarIdentifierCoptic)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierCoptic);
	} else if (CFEqual(*cf, kCFCalendarIdentifierEthiopicAmeteMihret)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierEthiopicAmeteMihret);
	} else if (CFEqual(*cf, kCFCalendarIdentifierEthiopicAmeteAlem)) {
	    CFRelease(*cf);
	    *cf = CFRetain(kCFCalendarIdentifierEthiopicAmeteAlem);
        } else if (CFEqual(*cf, kCFCalendarIdentifierIslamicTabular)) {
            CFRelease(*cf);
            *cf = CFRetain(kCFCalendarIdentifierIslamicTabular);
        } else if (CFEqual(*cf, kCFCalendarIdentifierIslamicUmmAlQura)) {
            CFRelease(*cf);
            *cf = CFRetain(kCFCalendarIdentifierIslamicUmmAlQura);
        } else {
	    CFRelease(*cf);
	    *cf = NULL;
	    return false;
	}
    } else {
	*cf = CFRetain(kCFCalendarIdentifierGregorian);
    }
    return true;
}

static bool __CFLocaleCopyCalendar(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    if (__CFLocaleCopyCalendarID(locale, user, cf, context)) {
	CFCalendarRef calendar = CFCalendarCreateWithIdentifier(kCFAllocatorSystemDefault, (CFStringRef)*cf);
	CFCalendarSetLocale(calendar, locale);
        CFDictionaryRef prefs = __CFLocaleGetPrefs(locale);
        CFPropertyListRef metapref = prefs ? CFDictionaryGetValue(prefs, CFSTR("AppleFirstWeekday")) : NULL;
        if (NULL != metapref && CFGetTypeID(metapref) == CFDictionaryGetTypeID()) {
            metapref = (CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)metapref, *cf);
        }
        if (NULL != metapref && CFGetTypeID(metapref) == CFNumberGetTypeID()) {
            CFIndex wkdy;
            if (CFNumberGetValue((CFNumberRef)metapref, kCFNumberCFIndexType, &wkdy)) {
                CFCalendarSetFirstWeekday(calendar, wkdy);
            }
        }
        metapref = prefs ? CFDictionaryGetValue(prefs, CFSTR("AppleMinDaysInFirstWeek")) : NULL;
        if (NULL != metapref && CFGetTypeID(metapref) == CFDictionaryGetTypeID()) {
            metapref = (CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)metapref, *cf);
        }
        if (NULL != metapref && CFGetTypeID(metapref) == CFNumberGetTypeID()) {
            CFIndex mwd;
            if (CFNumberGetValue((CFNumberRef)metapref, kCFNumberCFIndexType, &mwd)) {
                CFCalendarSetMinimumDaysInFirstWeek(calendar, mwd);
            }
        }
	CFRelease(*cf);
	*cf = calendar;
	return true;
    }
#endif
    return false;
}

static bool __CFLocaleCopyDelimiter(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    ULocaleDataDelimiterType type = (ULocaleDataDelimiterType)0;
    if (context == kCFLocaleQuotationBeginDelimiterKey) {
	type = ULOCDATA_QUOTATION_START;
    } else if (context == kCFLocaleQuotationEndDelimiterKey) {
	type = ULOCDATA_QUOTATION_END;
    } else if (context == kCFLocaleAlternateQuotationBeginDelimiterKey) {
	type = ULOCDATA_ALT_QUOTATION_START;
    } else if (context == kCFLocaleAlternateQuotationEndDelimiterKey) {
	type = ULOCDATA_ALT_QUOTATION_END;
    } else {
	return false;
    }

    char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    if (!CFStringGetCString(locale->_identifier, localeID, sizeof(localeID)/sizeof(char), kCFStringEncodingASCII)) {
	return false;
    }

    UChar buffer[130];
    UErrorCode status = U_ZERO_ERROR;
    ULocaleData *uld = ulocdata_open(localeID, &status);
    int32_t len = ulocdata_getDelimiter(uld, type, buffer, sizeof(buffer) / sizeof(buffer[0]), &status);
    ulocdata_close(uld);
    if (U_FAILURE(status) || sizeof(buffer) / sizeof(buffer[0]) < len) {
        return false;
    }

    *cf = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (UniChar *)buffer, len);
    return (*cf != NULL);
#else
    if (context == kCFLocaleQuotationBeginDelimiterKey || context == kCFLocaleQuotationEndDelimiterKey || context == kCFLocaleAlternateQuotationBeginDelimiterKey || context == kCFLocaleAlternateQuotationEndDelimiterKey) {
	*cf = CFRetain(CFSTR("\""));
        return true;
    } else {
        return false;
    }
#endif
}

static bool __CFLocaleCopyCollationID(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
    return __CFLocaleCopyICUKeyword(locale, user, cf, context, kCollationKeyword);
}

static bool __CFLocaleCopyCollatorID(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
    CFStringRef canonLocaleCFStr = NULL;
    if (user && locale->_prefs) {
	CFStringRef pref = (CFStringRef)CFDictionaryGetValue(locale->_prefs, CFSTR("AppleCollationOrder"));
	if (pref) {
	    // Canonicalize pref string in case it's not in the canonical format.
	    canonLocaleCFStr = CFLocaleCreateCanonicalLanguageIdentifierFromString(kCFAllocatorSystemDefault, pref);
	} else {
	    CFArrayRef languagesArray = (CFArrayRef)CFDictionaryGetValue(locale->_prefs, CFSTR("AppleLanguages"));
	    if (languagesArray && (CFArrayGetTypeID() == CFGetTypeID(languagesArray))) {
		if (0 < CFArrayGetCount(languagesArray)) {
		    CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(languagesArray, 0);
		    if (str && (CFStringGetTypeID() == CFGetTypeID(str))) {
			canonLocaleCFStr = CFLocaleCreateCanonicalLanguageIdentifierFromString(kCFAllocatorSystemDefault, str);
		    }
		}
	    }
	}
    }
    if (!canonLocaleCFStr) {
	canonLocaleCFStr = CFLocaleGetIdentifier(locale);
	CFRetain(canonLocaleCFStr);
    }
    *cf = canonLocaleCFStr;
    return canonLocaleCFStr ? true : false;
}

static bool __CFLocaleCopyUsesMetric(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    bool us = false;    // Default is Metric
    bool done = false;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    if (user) {
	CFTypeRef pref = CFDictionaryGetValue(locale->_prefs, CFSTR("AppleMetricUnits"));
	if (pref) {
	    us = (kCFBooleanFalse == pref);
	    done = true;
	} else {
	    pref = CFDictionaryGetValue(locale->_prefs, CFSTR("AppleMeasurementUnits"));
	    if (pref) {
		us = CFEqual(pref, CFSTR("Inches"));
		done = true;
	    }
	}
    }
#endif
    if (!done) {
        char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
        if (CFStringGetCString(locale->_identifier, localeID, sizeof(localeID)/sizeof(char), kCFStringEncodingASCII)) {
            UErrorCode  icuStatus = U_ZERO_ERROR;
            UMeasurementSystem ms = UMS_SI;
            ms = ulocdata_getMeasurementSystem(localeID, &icuStatus);
            if (U_SUCCESS(icuStatus)) {
                us = (ms == UMS_US);
                done = true;
            }
        }
    }
    if (!done)
        us = false;
    *cf = us ? CFRetain(kCFBooleanFalse) : CFRetain(kCFBooleanTrue);
    return true;
#else
    *cf = CFRetain(kCFBooleanFalse);
    return true;
#endif
}

static bool __CFLocaleCopyMeasurementSystem(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
    if (__CFLocaleCopyUsesMetric(locale, user, cf, context)) {
	bool us = (*cf == kCFBooleanFalse);
	CFRelease(*cf);
	*cf = us ? CFRetain(CFSTR("U.S.")) : CFRetain(CFSTR("Metric"));
	return true;
    }
    return false;
}

static bool __CFLocaleCopyNumberFormat(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
    CFStringRef str = NULL;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    CFNumberFormatterRef nf = CFNumberFormatterCreate(kCFAllocatorSystemDefault, locale, kCFNumberFormatterDecimalStyle);
    str = nf ? (CFStringRef)CFNumberFormatterCopyProperty(nf, context) : NULL;
    if (nf) CFRelease(nf);
#endif
    if (str) {
	*cf = str;
	return true;
    }
    return false;
}

// ICU does not reliably set up currency info for other than Currency-type formatters,
// so we have to have another routine here which creates a Currency number formatter.
static bool __CFLocaleCopyNumberFormat2(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
    CFStringRef str = NULL;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    CFNumberFormatterRef nf = CFNumberFormatterCreate(kCFAllocatorSystemDefault, locale, kCFNumberFormatterCurrencyStyle);
    str = nf ? (CFStringRef)CFNumberFormatterCopyProperty(nf, context) : NULL;
    if (nf) CFRelease(nf);
#endif
    if (str) {
	*cf = str;
	return true;
    }
    return false;
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
typedef int32_t (*__CFICUFunction)(const char *, const char *, UChar *, int32_t, UErrorCode *);

static bool __CFLocaleICUName(const char *locale, const char *valLocale, CFStringRef *out, __CFICUFunction icu) {
    UErrorCode icuStatus = U_ZERO_ERROR;
    int32_t size;
    UChar name[kMaxICUNameSize];

    size = (*icu)(valLocale, locale, name, kMaxICUNameSize, &icuStatus);
    if (U_SUCCESS(icuStatus) && size > 0 && icuStatus != U_USING_DEFAULT_WARNING) {
        *out = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (UniChar *)name, size);
        return (*out != NULL);
    }
    return false;
}

static bool __CFLocaleICUKeywordValueName(const char *locale, const char *value, const char *keyword, CFStringRef *out) {
    UErrorCode icuStatus = U_ZERO_ERROR;
    int32_t size = 0;
    UChar name[kMaxICUNameSize];
    // Need to make a fake locale ID
    char lid[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    if (strlen(value) < ULOC_KEYWORD_AND_VALUES_CAPACITY) {
	strlcpy(lid, "en_US@", sizeof(lid));
	strlcat(lid, keyword, sizeof(lid));
	strlcat(lid, "=", sizeof(lid));
	strlcat(lid, value, sizeof(lid));
        size = uloc_getDisplayKeywordValue(lid, keyword, locale, name, kMaxICUNameSize, &icuStatus);
        if (U_SUCCESS(icuStatus) && size > 0 && icuStatus != U_USING_DEFAULT_WARNING) {
            *out = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (UniChar *)name, size);
            return (*out != NULL);
        }
    }
    return false;
}

static bool __CFLocaleICUCurrencyName(const char *locale, const char *value, UCurrNameStyle style, CFStringRef *out) {
    int valLen = strlen(value);
    if (valLen != 3) // not a valid ISO code
        return false;
    UChar curr[4];
    UBool isChoice = FALSE;
    int32_t size = 0;
    UErrorCode icuStatus = U_ZERO_ERROR;
    u_charsToUChars(value, curr, valLen);
    curr[valLen] = '\0';
    const UChar *name;
    name = ucurr_getName(curr, locale, style, &isChoice, &size, &icuStatus);
    if (U_FAILURE(icuStatus) || icuStatus == U_USING_DEFAULT_WARNING)
        return false;
    UChar result[kMaxICUNameSize];
    if (isChoice)
    {
        UChar pattern[kMaxICUNameSize];
        CFStringRef patternRef = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("{0,choice,%S}"), name);
        CFIndex pattlen = CFStringGetLength(patternRef);
        CFStringGetCharacters(patternRef, CFRangeMake(0, pattlen), (UniChar *)pattern);
        CFRelease(patternRef);
        pattern[pattlen] = '\0';        // null terminate the pattern
        // Format the message assuming a large amount of the currency
        size = u_formatMessage("en_US", pattern, pattlen, result, kMaxICUNameSize, &icuStatus, 10.0);
        if (U_FAILURE(icuStatus))
            return false;
        name = result;
        
    }
    *out = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (UniChar *)name, size);
    return (*out != NULL);
}
#endif

static bool __CFLocaleFullName(const char *locale, const char *value, CFStringRef *out) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    UErrorCode icuStatus = U_ZERO_ERROR;
    int32_t size;
    UChar name[kMaxICUNameSize];
    
    // First, try to get the full locale.
    size = uloc_getDisplayName(value, locale, name, kMaxICUNameSize, &icuStatus);
    if (U_FAILURE(icuStatus) || size <= 0)
        return false;

    // Did we wind up using a default somewhere?
    if (icuStatus == U_USING_DEFAULT_WARNING) {
        // For some locale IDs, there may be no language which has a translation for every
        // piece. Rather than return nothing, see if we can at least handle
        // the language part of the locale.
        UErrorCode localStatus = U_ZERO_ERROR;
        int32_t localSize;
        UChar localName[kMaxICUNameSize];
        localSize = uloc_getDisplayLanguage(value, locale, localName, kMaxICUNameSize, &localStatus);
        if (U_FAILURE(localStatus) || size <= 0 || localStatus == U_USING_DEFAULT_WARNING)
            return false;
    }

    // This locale is OK, so use the result.
    *out = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (UniChar *)name, size);
    return (*out != NULL);
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleLanguageName(const char *locale, const char *value, CFStringRef *out) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    return __CFLocaleICUName(locale, value, out, uloc_getDisplayLanguage);
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleCountryName(const char *locale, const char *value, CFStringRef *out) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    // Need to make a fake locale ID
    char lid[ULOC_FULLNAME_CAPACITY];
    if (strlen(value) < sizeof(lid) - 3) {
	strlcpy(lid, "en_", sizeof(lid));
	strlcat(lid, value, sizeof(lid));
        return __CFLocaleICUName(locale, lid, out, uloc_getDisplayCountry);
    }
    return false;
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleScriptName(const char *locale, const char *value, CFStringRef *out) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    // Need to make a fake locale ID
    char lid[ULOC_FULLNAME_CAPACITY];
    if (strlen(value) == 4) {
	strlcpy(lid, "en_", sizeof(lid));
	strlcat(lid, value, sizeof(lid));
	strlcat(lid, "_US", sizeof(lid));
        return __CFLocaleICUName(locale, lid, out, uloc_getDisplayScript);
    }
    return false;
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleVariantName(const char *locale, const char *value, CFStringRef *out) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    // Need to make a fake locale ID
    char lid[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    if (strlen(value) < sizeof(lid) - 6) {
	strlcpy(lid, "en_US_", sizeof(lid));
	strlcat(lid, value, sizeof(lid));
        return __CFLocaleICUName(locale, lid, out, uloc_getDisplayVariant);
    }
    return false;
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleCalendarName(const char *locale, const char *value, CFStringRef *out) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    return __CFLocaleICUKeywordValueName(locale, value, kCalendarKeyword, out);
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleCollationName(const char *locale, const char *value, CFStringRef *out) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    return __CFLocaleICUKeywordValueName(locale, value, kCollationKeyword, out);
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleCurrencyShortName(const char *locale, const char *value, CFStringRef *out) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    return __CFLocaleICUCurrencyName(locale, value, UCURR_SYMBOL_NAME, out);
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleCurrencyFullName(const char *locale, const char *value, CFStringRef *out) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    return __CFLocaleICUCurrencyName(locale, value, UCURR_LONG_NAME, out);
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleNoName(const char *locale, const char *value, CFStringRef *out) {
    return false;
}

#undef kMaxICUNameSize

