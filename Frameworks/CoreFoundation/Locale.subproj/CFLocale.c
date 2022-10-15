/*      CFLocale.c
	Copyright (c) 2002-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: David Smith
*/

// Note the header file is in the OpenSource set (stripped to almost nothing), but not the .c file

#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFLocale_Private.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFNumber.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#if !TARGET_OS_WASI
#include <CoreFoundation/CFPreferences.h>
#include "CFBundle_Internal.h"
#else
#include "CFBase.h"
#endif
#include "CFLocaleInternal.h"
#include <stdatomic.h>
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
#include <unicode/uloc.h>           // ICU locales
#include <unicode/ulocdata.h>       // ICU locale data
#include <unicode/ucal.h>
#include <unicode/ucurr.h>          // ICU currency functions
#include <unicode/uset.h>           // ICU Unicode sets
#include <unicode/putil.h>          // ICU low-level utilities
#include <unicode/umsg.h>           // ICU message formatting
#include <unicode/ucol.h>
#include <unicode/unumsys.h>        // ICU numbering systems
#include <unicode/uvernum.h>
#if U_ICU_VERSION_MAJOR_NUM > 53 && __has_include(<unicode/uameasureformat.h>)
#include <unicode/uameasureformat.h>

extern int32_t
uameasfmt_getUnitsForUsage( const char*     locale,
                           const char*     category,
                           const char*     usage,
                           UAMeasureUnit*  units,
                           int32_t         unitsCapacity,
                           UErrorCode*     status );

#endif
#endif
#include <CoreFoundation/CFNumberFormatter.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if TARGET_OS_IPHONE
#include <mach-o/dyld_priv.h>
#endif


CF_PRIVATE CFCalendarRef _CFCalendarCreateCoWWithIdentifier(CFStringRef identifier);

CONST_STRING_DECL(kCFLocaleCurrentLocaleDidChangeNotification, "kCFLocaleCurrentLocaleDidChangeNotification")

CF_PRIVATE void __CFLocalePrefsChanged(CFNotificationCenterRef, void *, CFStringRef, const void *, CFDictionaryRef);

static const char * const kCalendarKeyword = "calendar";
static const char * const kCollationKeyword = "collation";
#define kMaxICUNameSize 1024

typedef struct __CFLocale *CFMutableLocaleRef;

CONST_STRING_DECL(__kCFLocaleCollatorID, "locale:collator id")


enum {
    __kCFLocaleKeyTableCount = 22
};

struct key_table {
    CFStringRef const * key;
    bool (*get)(CFLocaleRef, bool user, CFTypeRef *, CFStringRef context);  // returns an immutable copy & reference
    bool (*set)(CFMutableLocaleRef, CFTypeRef, CFStringRef context);
    bool (*name)(const char *, const char *, CFStringRef *); 
    CFStringRef const * context;
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
static bool __CFLocaleCopyTemperatureUnit(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCopyNumberFormat(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCopyNumberFormat2(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCurrencyFullName(const char *locale, const char *value, CFStringRef *out);
static bool __CFLocaleCopyCollatorID(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);
static bool __CFLocaleCopyDelimiter(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context);

static struct key_table const __CFLocaleKeyTable[__kCFLocaleKeyTableCount] = {
    {&kCFLocaleIdentifierKey, __CFLocaleCopyLocaleID, __CFLocaleSetNOP, __CFLocaleFullName, NULL},
    {&kCFLocaleLanguageCodeKey, __CFLocaleCopyCodes, __CFLocaleSetNOP, __CFLocaleLanguageName, &kCFLocaleLanguageCodeKey},
    {&kCFLocaleCountryCodeKey, __CFLocaleCopyCodes, __CFLocaleSetNOP, __CFLocaleCountryName, &kCFLocaleCountryCodeKey},
    {&kCFLocaleScriptCodeKey, __CFLocaleCopyCodes, __CFLocaleSetNOP, __CFLocaleScriptName, &kCFLocaleScriptCodeKey},
    {&kCFLocaleVariantCodeKey, __CFLocaleCopyCodes, __CFLocaleSetNOP, __CFLocaleVariantName, &kCFLocaleVariantCodeKey},
    {&kCFLocaleExemplarCharacterSetKey, __CFLocaleCopyExemplarCharSet, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {&kCFLocaleCalendarIdentifierKey, __CFLocaleCopyCalendarID, __CFLocaleSetNOP, __CFLocaleCalendarName, NULL},
    {&kCFLocaleCalendarKey, __CFLocaleCopyCalendar, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {&kCFLocaleCollationIdentifierKey, __CFLocaleCopyCollationID, __CFLocaleSetNOP, __CFLocaleCollationName, NULL},
    {&kCFLocaleUsesMetricSystemKey, __CFLocaleCopyUsesMetric, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {&kCFLocaleMeasurementSystemKey, __CFLocaleCopyMeasurementSystem, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {&kCFLocaleTemperatureUnitKey, __CFLocaleCopyTemperatureUnit, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {&kCFLocaleDecimalSeparatorKey, __CFLocaleCopyNumberFormat, __CFLocaleSetNOP, __CFLocaleNoName, &kCFNumberFormatterDecimalSeparatorKey},
    {&kCFLocaleGroupingSeparatorKey, __CFLocaleCopyNumberFormat, __CFLocaleSetNOP, __CFLocaleNoName, &kCFNumberFormatterGroupingSeparatorKey},
    {&kCFLocaleCurrencySymbolKey, __CFLocaleCopyNumberFormat2, __CFLocaleSetNOP, __CFLocaleCurrencyShortName, &kCFNumberFormatterCurrencySymbolKey},
    {&kCFLocaleCurrencyCodeKey, __CFLocaleCopyNumberFormat2, __CFLocaleSetNOP, __CFLocaleCurrencyFullName, &kCFNumberFormatterCurrencyCodeKey},
    {&kCFLocaleCollatorIdentifierKey, __CFLocaleCopyCollatorID, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {&__kCFLocaleCollatorID, __CFLocaleCopyCollatorID, __CFLocaleSetNOP, __CFLocaleNoName, NULL},
    {&kCFLocaleQuotationBeginDelimiterKey, __CFLocaleCopyDelimiter, __CFLocaleSetNOP, __CFLocaleNoName, &kCFLocaleQuotationBeginDelimiterKey},
    {&kCFLocaleQuotationEndDelimiterKey, __CFLocaleCopyDelimiter, __CFLocaleSetNOP, __CFLocaleNoName, &kCFLocaleQuotationEndDelimiterKey},
    {&kCFLocaleAlternateQuotationBeginDelimiterKey, __CFLocaleCopyDelimiter, __CFLocaleSetNOP, __CFLocaleNoName, &kCFLocaleAlternateQuotationBeginDelimiterKey},
    {&kCFLocaleAlternateQuotationEndDelimiterKey, __CFLocaleCopyDelimiter, __CFLocaleSetNOP, __CFLocaleNoName, &kCFLocaleAlternateQuotationEndDelimiterKey},
};


static CFLocaleRef __CFLocaleSystem = NULL;
static CFMutableDictionaryRef __CFLocaleCache = NULL;
static CFLock_t __CFLocaleGlobalLock = CFLockInit;

struct __CFLocale {
    CFRuntimeBase _base;
    CFStringRef _identifier;    // canonical identifier, never NULL
    _Atomic(CFMutableDictionaryRef) _cache;
    CFDictionaryRef _prefs;
    CFLock_t _lock;
    // True if this locale is **NOT** one of the "special" languages that
    // requires special handing during case mapping:
    // - "az": Azerbaijani
    // - "lt": Lithuanian
    // - "tr": Turkish
    // - "nl": Dutch
    // - "el": Greek
    // See `CFUniCharMapCaseTo`
    // See https://www.unicode.org/Public/UNIDATA/SpecialCasing.txt
    Boolean _doesNotRequireSpecialCaseHandling;
};
 
CF_PRIVATE Boolean __CFLocaleGetDoesNotRequireSpecialCaseHandling(struct __CFLocale *locale) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), Boolean, (NSLocale *)locale, _doesNotRequireSpecialCaseHandling);
    return locale->_doesNotRequireSpecialCaseHandling;
}

CF_PRIVATE void __CFLocaleSetDoesNotRequireSpecialCaseHandling(struct __CFLocale *locale) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), void, (NSLocale *)locale, _setDoesNotRequireSpecialCaseHandling);
    locale->_doesNotRequireSpecialCaseHandling = true;
}

/* Flag bits */
enum {      /* Bits 0-1 */
    __kCFLocaleOrdinary = 0,
    __kCFLocaleSystem = 1,
    __kCFLocaleUser = 2,
    __kCFLocaleCustom = 3
};

CF_INLINE CFIndex __CFLocaleGetType(CFLocaleRef locale) {
    return __CFRuntimeGetValue(locale, 1, 0);
}

CF_INLINE void __CFLocaleSetType(CFLocaleRef locale, CFIndex type) {
    __CFRuntimeSetValue(locale, 1, 0, (uint8_t)type);
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

CF_INLINE Boolean __CFLocaleCacheGetValueIfPresent(CFLocaleRef locale, CFLocaleKey key, CFTypeRef *value) {
    CFDictionaryRef cache = atomic_load_explicit(&locale->_cache, memory_order_acquire);
    if (!cache) {
        *value = NULL;
        return false;
    }
    
    return CFDictionaryGetValueIfPresent(cache, key, value);
}

CF_INLINE void __CFLocaleCacheSet_alreadyLocked(CFLocaleRef locale, CFLocaleKey key, CFTypeRef value) {
    CFMutableDictionaryRef cache = atomic_load_explicit(&locale->_cache, memory_order_acquire);
    if (!cache) {
        cache = CFDictionaryCreateMutable(CFGetAllocator(locale), 0, NULL, &kCFTypeDictionaryValueCallBacks);
        atomic_store_explicit(&((struct __CFLocale *)locale)->_cache, cache, memory_order_release);
    }
    
    CFDictionarySetValue(cache, key, value);
}

static Boolean __CFLocaleEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFLocaleRef locale1 = (CFLocaleRef)cf1;
    CFLocaleRef locale2 = (CFLocaleRef)cf2;
    // a user locale and a locale created with an ident are not the same even if their contents are
    if (__CFLocaleGetType(locale1) != __CFLocaleGetType(locale2)) return false;
    if (!CFEqual(locale1->_identifier, locale2->_identifier)) return false;
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
    if (NULL != locale->_prefs) CFRelease(locale->_prefs);
}

const CFRuntimeClass __CFLocaleClass = {
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
    return _kCFRuntimeIDCFLocale;
}

CFLocaleRef CFLocaleGetSystem(void) {
    CFLocaleRef locale;
    CFLocaleRef uselessLocale = NULL; //if we lose the race creating the global locale, we need to release the one we created, but we want to do it outside the lock.
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
            uselessLocale = locale;
	}
    }
#if !DEPLOYMENT_RUNTIME_SWIFT
    // This line relies on the fact that outside of Swift, __CFLocaleSystem is immortal.
    locale = __CFLocaleSystem ? (CFLocaleRef)CFRetain(__CFLocaleSystem) : NULL;
#else
    locale = __CFLocaleSystem;
#endif
    __CFLocaleUnlockGlobal();
    if (uselessLocale) CFRelease(uselessLocale);
    return locale;
}

extern CFDictionaryRef __CFXPreferencesCopyCurrentApplicationStateWithDeadlockAvoidance(Boolean * /* _Nonnull */outWouldDeadlock);

static _Atomic(CFLocaleRef) _CFLocaleCurrent_ = NULL;

CF_INLINE CFLocaleRef _cachedCurrentLocale() {
    return atomic_load(&_CFLocaleCurrent_);
}

// Returns true if `newLocale` is made immortal.
static Boolean _setCachedCurrentLocaleAndMakeImmortal(CFLocaleRef newLocale) {
    Boolean success;
    if (newLocale) {
        CFLocaleRef cachedLocale = NULL;
        success = atomic_compare_exchange_strong(&_CFLocaleCurrent_, &cachedLocale, newLocale);
        if (success) {
#if !DEPLOYMENT_RUNTIME_SWIFT
            __CFRuntimeSetRC((CFTypeRef)newLocale, 0);
#else
            // Swift does not support immortal objects yet; add an unbalanced retain instead.
            CFRetain((CFTypeRef)newLocale);
#endif

        }
    } else {
        success = false;
        atomic_store(&_CFLocaleCurrent_, newLocale);
    }

    return success;
}


#if TARGET_OS_OSX && !DEPLOYMENT_RUNTIME_SWIFT
#define FALLBACK_LOCALE_NAME CFSTR("")
#elif TARGET_OS_IPHONE
#define FALLBACK_LOCALE_NAME CFSTR("en_US")
#elif TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD || DEPLOYMENT_RUNTIME_SWIFT
#define FALLBACK_LOCALE_NAME CFSTR("en_US")
#endif

static CFStringRef _CFLocaleCopyLocaleIdentifierByAddingLikelySubtags(CFStringRef localeID)
{
    if (!localeID) {
        return NULL;
    }
    CFStringRef result = NULL;

    char bufLocaleID[ULOC_FULLNAME_CAPACITY];
    const char *cLocaleID = CFStringGetCStringPtr(localeID, kCFStringEncodingUTF8);
    if (NULL == cLocaleID) {
        if (CFStringGetCString(localeID, bufLocaleID, ULOC_FULLNAME_CAPACITY, kCFStringEncodingUTF8)) {
            cLocaleID = bufLocaleID;
        }
    }
    UErrorCode icuStatus = U_ZERO_ERROR;
    char maximizedLocaleID[ULOC_FULLNAME_CAPACITY];
    int32_t bufSize = uloc_addLikelySubtags(cLocaleID, maximizedLocaleID, ULOC_FULLNAME_CAPACITY, &icuStatus);
    if ((bufSize != -1) && U_SUCCESS(icuStatus)) {
        result = CFStringCreateWithCString(NULL, maximizedLocaleID, kCFStringEncodingUTF8);
    }

    return result ? : CFRetain(localeID);
}

// For a given locale (e.g. `en_US`, `zh_CN`, etc.) copies the language identifier with an explicit script code (e.g. `en-Latn`, zh-Hans`, etc.)
static CFStringRef _CFLocaleCopyLanguageIdentifierWithScriptCodeForLocaleIdentifier(CFStringRef localeID)
{
    CFStringRef languageID = NULL;
    if (localeID) {
        CFStringRef maximizedLocaleID = _CFLocaleCopyLocaleIdentifierByAddingLikelySubtags(localeID);
        CFDictionaryRef components = CFLocaleCreateComponentsFromLocaleIdentifier(NULL, maximizedLocaleID);
        CFRelease(maximizedLocaleID);

        CFStringRef languageCode = CFDictionaryGetValue(components, kCFLocaleLanguageCode);
        CFStringRef scriptCode = CFDictionaryGetValue(components, kCFLocaleScriptCode);
        if (languageCode && scriptCode) {
            languageID = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@-%@"), languageCode, scriptCode);
        }
        CFRelease(components);
    }
    return languageID;
}

CFStringRef _CFLocaleCopyNumberingSystemForLocaleIdentifier(CFStringRef localeID)
{
    CFStringRef numberingSystemID = NULL;
    if (localeID) {
        CFDictionaryRef components = CFLocaleCreateComponentsFromLocaleIdentifier(NULL, localeID);
        if (components) {
            // If the locale has an explicitly defined numbering system, that’s our answer!
            numberingSystemID = CFDictionaryGetValue(components, CFSTR("numbers"));
            if (numberingSystemID) {
                CFRetain(numberingSystemID);
            }
            // Otherwise, query ICU for what the default numbering system is.
            else {
                CFMutableDictionaryRef mutableComponents = CFDictionaryCreateMutableCopy(NULL, 0, components);
                if (mutableComponents) {
                    CFDictionarySetValue(mutableComponents, CFSTR("numbers"), CFSTR("default"));
                    CFStringRef localeIDWithDefaultNumbers = CFLocaleCreateLocaleIdentifierFromComponents(NULL, mutableComponents);
                    if (localeIDWithDefaultNumbers) {
                        char bufLocaleIDWithDefaultNumbers[ULOC_FULLNAME_CAPACITY];
                        const char *cLocaleIDWithDefaultNumbers = CFStringGetCStringPtr(localeIDWithDefaultNumbers, kCFStringEncodingUTF8);
                        if (!cLocaleIDWithDefaultNumbers) {
                            if (CFStringGetCString(localeIDWithDefaultNumbers, bufLocaleIDWithDefaultNumbers, ULOC_FULLNAME_CAPACITY, kCFStringEncodingUTF8)) {
                                cLocaleIDWithDefaultNumbers = bufLocaleIDWithDefaultNumbers;
                            }
                        }
                        if (cLocaleIDWithDefaultNumbers) {
                            UErrorCode icuStatus = U_ZERO_ERROR;
                            UNumberingSystem *numberingSystem = unumsys_open(cLocaleIDWithDefaultNumbers, &icuStatus);
                            if (numberingSystem) {
                                const char *cNumberingSystemID = unumsys_getName(numberingSystem);
                                if (cNumberingSystemID) {
                                    numberingSystemID = CFStringCreateWithCString(NULL, cNumberingSystemID, kCFStringEncodingUTF8);
                                }
                                unumsys_close(numberingSystem);
                            }
                        }
                        CFRelease(localeIDWithDefaultNumbers);
                    }
                    CFRelease(mutableComponents);
                }
            }
            CFRelease(components);
        }
    }
    return numberingSystemID;
}

CFArrayRef _CFLocaleCopyValidNumberingSystemsForLocaleIdentifier(CFStringRef localeID)
{
    CFMutableArrayRef numberingSystemIDs = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    if (localeID) {
        CFDictionaryRef components = CFLocaleCreateComponentsFromLocaleIdentifier(NULL, localeID);
        if (components) {
            // 1. If there is an explicitly defined override numbering system, add it first to the list.
            CFStringRef overrideNumberingSystemID = CFDictionaryGetValue(components, CFSTR("numbers"));
            if (overrideNumberingSystemID) {
                CFArrayAppendValue(numberingSystemIDs, overrideNumberingSystemID);
            }
            
            // 2. Query ICU for additional supported numbering systems
            CFStringRef queryList[4] = { CFSTR("default"), NULL, NULL, NULL };
            CFStringRef languageCode = CFDictionaryGetValue(components, kCFLocaleLanguageCode);
            // For Chinese & Thai, although there is a traditional numbering system, it is not one that users will expect to use as a numbering system in the system. (cf. <rdar://problem/19742123&20068835>)
            if (!(CFEqual(languageCode, CFSTR("th")) ||
                  CFEqual(languageCode, CFSTR("zh")) ||
                  CFEqual(languageCode, CFSTR("wuu")) ||
                  CFEqual(languageCode, CFSTR("yue")))) {
                queryList[1] = CFSTR("native");
                queryList[2] = CFSTR("traditional");
                queryList[3] = CFSTR("finance");
            }
            CFMutableDictionaryRef mutableComponents = CFDictionaryCreateMutableCopy(NULL, 0, components);
            if (mutableComponents) {
                for (CFIndex i = 0, count = sizeof(queryList)/sizeof(CFStringRef); i < count; i++) {
                    CFStringRef query = queryList[i];
                    if (query) {
                        CFDictionarySetValue(mutableComponents, CFSTR("numbers"), query);
                        CFStringRef localeIDWithNumbersQuery = CFLocaleCreateLocaleIdentifierFromComponents(NULL, mutableComponents);
                        if (localeIDWithNumbersQuery) {
                            char bufLocaleIDWithNumbersQuery[ULOC_FULLNAME_CAPACITY];
                            const char *cLocaleIDWithNumbersQuery = CFStringGetCStringPtr(localeIDWithNumbersQuery, kCFStringEncodingUTF8);
                            if (!cLocaleIDWithNumbersQuery) {
                                if (CFStringGetCString(localeIDWithNumbersQuery, bufLocaleIDWithNumbersQuery, ULOC_FULLNAME_CAPACITY, kCFStringEncodingUTF8)) {
                                    cLocaleIDWithNumbersQuery = bufLocaleIDWithNumbersQuery;
                                }
                            }
                            if (cLocaleIDWithNumbersQuery) {
                                UNumberingSystem *numberingSystem = NULL;
                                UErrorCode icuStatus = U_ZERO_ERROR;
                                if ((numberingSystem = unumsys_open(cLocaleIDWithNumbersQuery, &icuStatus)) != NULL) {
                                    // There are some really funky numbering systems out there, and we do not support ones that are algorithmic (like the traditional ones for Hebrew, etc.) and ones that are not base 10.
                                    if (!unumsys_isAlgorithmic(numberingSystem) && unumsys_getRadix(numberingSystem) == 10) {
                                        const char *cNumberingSystemID = unumsys_getName(numberingSystem);
                                        if (cNumberingSystemID) {
                                            CFStringRef numberingSystemID = CFStringCreateWithCString(NULL, cNumberingSystemID, kCFStringEncodingUTF8);
                                            if (numberingSystemID) {
                                                if (!CFArrayContainsValue(numberingSystemIDs, CFRangeMake(0, CFArrayGetCount(numberingSystemIDs)), numberingSystemID)) {
                                                    CFArrayAppendValue(numberingSystemIDs, numberingSystemID);
                                                }
                                                CFRelease(numberingSystemID);
                                            }
                                        }
                                    }
                                    unumsys_close(numberingSystem);
                                }
                            }
                            CFRelease(localeIDWithNumbersQuery);
                        }
                    }
                }
                CFRelease(mutableComponents);
            }
            
            // 3. Add `latn`, which we support that for all languages.
            if (!CFArrayContainsValue(numberingSystemIDs, CFRangeMake(0, CFArrayGetCount(numberingSystemIDs)), CFSTR("latn"))) {
                CFArrayAppendValue(numberingSystemIDs, CFSTR("latn"));
            }
            
            CFRelease(components);
        }
    }
    return numberingSystemIDs;
}

CFStringRef _CFLocaleCreateLocaleIdentiferByReplacingLanguageCodeAndScriptCode(CFStringRef localeIDWithDesiredLangCode, CFStringRef localeIDWithDesiredComponents) {
    CFStringRef localeID = NULL;
    if (localeIDWithDesiredLangCode && localeIDWithDesiredComponents) {
        CFStringRef langIDToUse = _CFLocaleCopyLanguageIdentifierWithScriptCodeForLocaleIdentifier(localeIDWithDesiredLangCode);
        if (langIDToUse) {
            CFStringRef maximizedLocaleID = _CFLocaleCopyLocaleIdentifierByAddingLikelySubtags(localeIDWithDesiredComponents);
            if (maximizedLocaleID) {
                CFDictionaryRef localeIDComponents = CFLocaleCreateComponentsFromLocaleIdentifier(NULL, maximizedLocaleID);
                CFRelease(maximizedLocaleID);
                if (localeIDComponents) {
                    CFMutableDictionaryRef mutableComps = CFDictionaryCreateMutableCopy(NULL, CFDictionaryGetCount(localeIDComponents), localeIDComponents);
                    CFRelease(localeIDComponents);
                    if (mutableComps) {
                        CFDictionaryRef languageIDComponents = CFLocaleCreateComponentsFromLocaleIdentifier(NULL, langIDToUse);
                        if (languageIDComponents) {
                            CFStringRef languageCode = CFDictionaryGetValue(languageIDComponents, kCFLocaleLanguageCode);
                            CFStringRef scriptCode = CFDictionaryGetValue(languageIDComponents, kCFLocaleScriptCode);
                            if (languageCode && scriptCode) {
                                // 1. Language & Script
                                // Note that both `languageCode` and `scriptCode` should be overridden in `mutableComps`, even for combinations like `en` + `latn`, because the previous language’s script may not be compatible with the new language. This will produce a “maximized” locale identifier, which we will canonicalize (below) to remove superfluous tags.
                                CFDictionarySetValue(mutableComps, kCFLocaleLanguageCode, languageCode);
                                CFDictionarySetValue(mutableComps, kCFLocaleScriptCode, scriptCode);
                                
                                // 2. Numbering System
                                CFStringRef numberingSystem = _CFLocaleCopyNumberingSystemForLocaleIdentifier(localeIDWithDesiredComponents);
                                if (numberingSystem) {
                                    CFArrayRef validNumberingSystems = _CFLocaleCopyValidNumberingSystemsForLocaleIdentifier(localeIDWithDesiredLangCode);
                                    if (validNumberingSystems) {
                                        CFIndex indexOfNumberingSystem = CFArrayGetFirstIndexOfValue(validNumberingSystems, CFRangeMake(0, CFArrayGetCount(validNumberingSystems)), numberingSystem);
                                        // If the numbering system for `localeIDWithDesiredComponents` is not compatible with the constructed locale’s language, then we should discard it, e.g. `ar_AE@numbers=arab` + `en` should get `en_AE`, not `en_AE@numbers=arab`, since `arab` is not valid for `en`.
                                        if (indexOfNumberingSystem == kCFNotFound || indexOfNumberingSystem == 0) {
                                            CFDictionaryRemoveValue(mutableComps, CFSTR("numbers"));
                                        }
                                        // If the numbering system for `localeIDWithDesiredComponents` is compatible with the constructed locale’s language and is not already the default numbering system (index 0), then set it on the new locale, e.g. `hi_IN@numbers=latn` + `ar` shoudl get `ar_IN@numbers=latn`, since `latn` is valid for `ar`.
                                        else if (indexOfNumberingSystem > 0) {
                                            CFDictionarySetValue(mutableComps, CFSTR("numbers"), numberingSystem);
                                        }
                                        CFRelease(validNumberingSystems);
                                    }
                                    CFRelease(numberingSystem);
                                }
                                
                                // 3. Construct & Canonicalize
                                // The locale constructed from the components will be over-specified for many cases, such as `en_Latn_US`. Before returning it, we should canonicalize it, which will remove any script code that is already implicit in the definition of the locale, yielding `en_US` instead.
                                CFStringRef maximizedLocaleID = CFLocaleCreateLocaleIdentifierFromComponents(NULL, mutableComps);
                                if (maximizedLocaleID) {
                                    localeID = CFLocaleCreateCanonicalLocaleIdentifierFromString(NULL, maximizedLocaleID);
                                    CFRelease(maximizedLocaleID);
                                }
                            }
                            CFRelease(languageIDComponents);
                        }
                        CFRelease(mutableComps);
                    }
                }
            }
            CFRelease(langIDToUse);
        }
    }
    return localeID;
}

static CFArrayRef _CFLocaleCopyPreferredLanguagesFromPrefs(CFArrayRef languagesArray);

/// Creates a new locale identifier by identifying the most preferred localization (using `availableLocalizations` and `preferredLanguages`) and then creating a locale based on the most preferred localization, while retaining any relevant attributes from `preferredLocaleID`, e.g. if `availableLocalizations` is `[ "en", "fr", "de" ]`, `preferredLanguages` is `[ "ar-AE", "en-AE" ]`, `preferredLocaleID` is `ar_AE@numbers=arab;calendar=islamic-civil`, it will return `en_AE@calendar=islamic-civil`, i.e. the language will be matched to `en` since that’s the only available localization that matches, `calendar` will be retained since it’s language-agnostic, but `numbers` will be discarded because the `arab` numbering system is not valid for `en`.
static CFStringRef _CFLocaleCreateLocaleIdentifierForAvailableLocalizations(CFArrayRef availableLocalizations, CFArrayRef preferredLanguages, CFStringRef preferredLocaleID, CFArrayRef *outCanonicalizedPreferredLanguages) {
    CFStringRef result = NULL;
    if (availableLocalizations && CFArrayGetCount(availableLocalizations) > 0 &&
        preferredLanguages && CFArrayGetCount(preferredLanguages) > 0 &&
        preferredLocaleID && CFStringGetLength(preferredLocaleID) > 0)
    {
        CFArrayRef canonicalizedPreferredLanguages = _CFLocaleCopyPreferredLanguagesFromPrefs(preferredLanguages);

        // Combine `availableLocalizations` with `preferredLanguages` to get `preferredLocalizations`, whose #0 object indicates the localization that the app is current launched in.
        CFArrayRef preferredLocalizations = NULL; {
            // Since `availableLocalizations` can contains legacy lproj names such as `English`, `French`, etc. we need to canonicalize these into language identifiers such as `en`, `fr`, etc. Otherwise the logic that later compares these to language identifiers will fail. (<rdar://problem/37141123>)
            CFArrayRef canonicalizedAvailableLocalizations = _CFLocaleCopyPreferredLanguagesFromPrefs(availableLocalizations);
            if (canonicalizedAvailableLocalizations) {
                preferredLocalizations = CFBundleCopyLocalizationsForPreferences(canonicalizedAvailableLocalizations, canonicalizedPreferredLanguages);
                CFRelease(canonicalizedAvailableLocalizations);
            }
        }
        
        if (preferredLocalizations && CFArrayGetCount(preferredLocalizations) > 0) {
            // If we didn't find an overlap, we go with the preferred locale of the bundle.
            CFStringRef preferredLocalization = CFArrayGetValueAtIndex(preferredLocalizations, 0);
            if (preferredLocalization) {
                // The goal here is to preserve all of the overrides present in the value stored in AppleLocale (e.g. "@calendar=buddhist")
                CFStringRef preferredLocaleLanguageID = _CFLocaleCopyLanguageIdentifierWithScriptCodeForLocaleIdentifier(preferredLocaleID);
                CFStringRef preferredLocalizationLanguageID = _CFLocaleCopyLanguageIdentifierWithScriptCodeForLocaleIdentifier(preferredLocalization);
                if (preferredLocaleLanguageID && preferredLocalizationLanguageID) {
                    if (CFEqual(preferredLocaleLanguageID, preferredLocalizationLanguageID)) {
                        result = CFRetain(preferredLocaleID);
                    } else {
                        result = _CFLocaleCreateLocaleIdentiferByReplacingLanguageCodeAndScriptCode(preferredLocalization, preferredLocaleID);
                    }
                }
                if (preferredLocaleLanguageID) { CFRelease(preferredLocaleLanguageID); }
                if (preferredLocalizationLanguageID) { CFRelease(preferredLocalizationLanguageID); }
            }
            
        }
        if (preferredLocalizations) { CFRelease(preferredLocalizations); }
        if (outCanonicalizedPreferredLanguages) {
            *outCanonicalizedPreferredLanguages = canonicalizedPreferredLanguages;
        } else if (canonicalizedPreferredLanguages) {
            CFRelease(canonicalizedPreferredLanguages);
        }
    }
    return result;
}

CFLocaleRef _CFLocaleCreateLikeCurrentWithBundleLocalizations(CFArrayRef availableLocalizations, Boolean allowsMixedLocalizations) {
    CFLocaleRef locale = NULL;
    
    if (allowsMixedLocalizations) {
        locale = _CFLocaleCopyPreferred();
    } else {
        CFArrayRef preferredLanguages = CFLocaleCopyPreferredLanguages();
        CFStringRef preferredLocaleID = CFPreferencesCopyAppValue(CFSTR("AppleLocale"), kCFPreferencesCurrentApplication);
        
        CFStringRef identifier = _CFLocaleCreateLocaleIdentifierForAvailableLocalizations(availableLocalizations, preferredLanguages, preferredLocaleID, NULL);
        if (identifier) {
            locale = CFLocaleCreate(kCFAllocatorSystemDefault, identifier);
        }
        
        if (identifier) {
            CFRelease(identifier);
        }
        if (preferredLocaleID) {
            CFRelease(preferredLocaleID);
        }
        if (preferredLanguages) {
            CFRelease(preferredLanguages);
        }
    }
    
    return locale;
}

static CFLocaleRef _CFLocaleCopyCurrentGuts(CFStringRef name, Boolean useCache, CFDictionaryRef overridePrefs, Boolean disableBundleMatching) {
    /*
     NOTE: calling any CFPreferences function, or any function which calls into a CFPreferences function, *except* for __CFXPreferencesCopyCurrentApplicationStateWithDeadlockAvoidance (and accepting backstop values if its outparam is false), will deadlock. This is because CFPreferences calls os_log_*, which calls -descriptionWithLocale:, which calls CFLocaleCopyCurrent.
     */
    
    CFStringRef ident = NULL;
    // We cannot be helpful here, because it causes performance problems,
    // even though the preference lookup is relatively quick, as there are
    // things which call this function thousands or millions of times in
    // a short period.
    if (!name) {
#if 0 // TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
        name = (CFStringRef)CFPreferencesCopyAppValue(CFSTR("AppleLocale"), kCFPreferencesCurrentApplication);
#endif
    } else {
        CFRetain(name);
    }
    if (name && (CFStringGetTypeID() == CFGetTypeID(name))) {
        ident = CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorSystemDefault, name);
        if (os_log_debug_enabled(_CFOSLog())) {
            CFDictionaryRef const components = CFLocaleCreateComponentsFromLocaleIdentifier(NULL, ident);
            if (components) {
                if (!CFDictionaryGetValue(components, kCFLocaleCountryCode)) {
                    os_log_debug(_CFOSLog(), "CFLocaleCopyCurrent() called with overriding locale identifier '%{public}@' which does not have a country code", ident);
                }
                
                CFRelease(components);
            }
        }
    }
    if (name) CFRelease(name);
    
    // If `disableBundleMatching` is true, caching needs to be turned off, only a single value is cached for the most common case of calling `CFLocaleCopyCurrent`.
    if (disableBundleMatching) {
        useCache = false;
    }
    
    if (useCache) {
        CFLocaleRef cached = _cachedCurrentLocale();
        if (cached && ident) {
            if (CFEqual(cached->_identifier, ident)) {
                // We can just return what's in the cache.
                CFRelease(ident);
                ident = NULL;
            } else {
                // We'll replace what's in the cache with ident below.
                _setCachedCurrentLocaleAndMakeImmortal(NULL);
                cached = NULL;
            }
        }

        if (cached) {
#if DEPLOYMENT_RUNTIME_SWIFT
            CFRetain(cached); // In Swift, this object isn't immortal and needs to be correctly memory-managed.
#endif
            
            return cached;
        }
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
    
    if (!ident) {
        ident = (CFStringRef)CFRetain(FALLBACK_LOCALE_NAME);

        // <rdar://problem/51409572> CFLocaleCopyCurrent() failed to look up current locale -- gpsd dameon is not localized, does not interact directly with users
        // This log was added to try to catch scenarios in which apps fail to look up the current locale thanks to sandboxing issues or CFPreferences issues. It turns out that in its current formulation, this log has a high false positive rate and is very confusing.
        // Disabled for now.
        /*
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            os_log_error(_CFOSLog(), "CFLocaleCopyCurrent() failed to look up current locale via 'AppleLocale' and 'AppleLanguages' in user preferences; falling back to locale identifier '%{public}@' as the default. Consider checking Console for sandbox violations from this process for reading from preferences, or enabling CoreFoundation debug logging for more information. This will only be logged once.", ident);
        });
        */
    }
    
    locale->_identifier = ident;
    locale->_prefs = prefs;
    locale->_lock = CFLockInit;
    locale->_doesNotRequireSpecialCaseHandling = false;
    
    if (useCache) {
        Boolean success = _setCachedCurrentLocaleAndMakeImmortal(locale);
        if (success) {
            // useCache is enabled, the locale is made immortal.
            // The clang analyzer doesn't know about __CFRuntimeSetRC, though, so it sees overwriting locale below as a leak.
            _CLANG_ANALYZER_IGNORE_RETAIN(locale);
        } else {
            // We already have a cached locale. Release the newly created one before overwriting it below.
            CFRelease(locale);
        }
        locale = (struct __CFLocale *)_cachedCurrentLocale();
    }
    return locale;
}

/*
 <rdar://problem/13834276> NSDateFormatter: Cannot specify force12HourTime/force24HourTime
 This returns an instance of CFLocale that's set up exactly like it would be if the user changed the current locale to that identifier, then called CFLocaleCopyCurrent()
 */
CFLocaleRef _CFLocaleCopyAsIfCurrent(CFStringRef name) {
    return _CFLocaleCopyCurrentGuts(name, false, NULL, false);
}

/*
 <rdar://problem/14032388> Need the ability to initialize a CFLocaleRef from a preferences dictionary
 This returns an instance of CFLocale that's set up exactly like it would be if the user changed the current locale to that identifier, set the preferences keys in the overrides dictionary, then called CFLocaleCopyCurrent()
 */
CFLocaleRef _CFLocaleCopyAsIfCurrentWithOverrides(CFStringRef name, CFDictionaryRef overrides) {
    return _CFLocaleCopyCurrentGuts(name, false, overrides, false);
}

CFLocaleRef _CFLocaleCopyPreferred(void) {
    return _CFLocaleCopyCurrentGuts(NULL, true, NULL, true);
}

CFLocaleRef CFLocaleCopyCurrent(void) {
    return _CFLocaleCopyCurrentGuts(NULL, true, NULL, false);
}

CF_PRIVATE CFDictionaryRef __CFLocaleGetPrefs(CFLocaleRef locale) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), CFDictionaryRef, (NSLocale *)locale, _prefs);
    return locale->_prefs;
}

#if DEPLOYMENT_RUNTIME_SWIFT
Boolean _CFLocaleInit(CFLocaleRef locale, CFStringRef identifier) {
    CFStringRef localeIdentifier = NULL;
    if (identifier) {
        localeIdentifier = CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorSystemDefault, identifier);
    }
    if (NULL == localeIdentifier) return false;
    CFStringRef old = localeIdentifier;
    localeIdentifier = (CFStringRef)CFStringCreateCopy(kCFAllocatorSystemDefault, localeIdentifier);
    CFRelease(old);
    
    __CFLocaleSetType(locale, __kCFLocaleOrdinary);
    ((struct __CFLocale *)locale)->_identifier = localeIdentifier;
    ((struct __CFLocale *)locale)->_cache = NULL;
    ((struct __CFLocale *)locale)->_prefs = NULL;
    ((struct __CFLocale *)locale)->_lock = CFLockInit;
    
    return true;
}
#endif

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
    // Look for cases where we can return a cached instance.
    // We only use cached objects if the allocator is the system
    // default allocator.
    if (!allocator) allocator = __CFGetDefaultAllocator();
    Boolean canCache = _CFAllocatorIsSystemDefault(allocator);
    static os_unfair_lock __CFLocaleCacheLock = OS_UNFAIR_LOCK_INIT;
    os_unfair_lock_lock_with_options(&__CFLocaleCacheLock, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
    if (canCache && __CFLocaleCache) {
	CFLocaleRef locale = (CFLocaleRef)CFDictionaryGetValue(__CFLocaleCache, localeIdentifier);
	if (locale) {
	    CFRetain(locale);
            os_unfair_lock_unlock(&__CFLocaleCacheLock);
	    CFRelease(localeIdentifier);
	    return locale;
	}
    }
    struct __CFLocale *locale = NULL;
    uint32_t size = sizeof(struct __CFLocale) - sizeof(CFRuntimeBase);
    locale = (struct __CFLocale *)_CFRuntimeCreateInstance(allocator, CFLocaleGetTypeID(), size, NULL);
    if (NULL == locale) {
        if (localeIdentifier) { CFRelease(localeIdentifier); }
	return NULL;
    }
    __CFLocaleSetType(locale, __kCFLocaleOrdinary);
    locale->_identifier = localeIdentifier;
    locale->_prefs = NULL;
    locale->_lock = CFLockInit;
    if (canCache) {
	if (NULL == __CFLocaleCache) {
	    __CFLocaleCache = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	}
        CFDictionarySetValue(__CFLocaleCache, localeIdentifier, locale);
    }
    os_unfair_lock_unlock(&__CFLocaleCacheLock);
    return (CFLocaleRef)locale;
}

//CFLocaleCreateCopy() always just retained. This caused problems because CFLocaleGetValue(locale, kCFLocaleCalendarKey) would create a calendar, then set its locale to self, leading to a retain cycle
static CFLocaleRef _CFLocaleCreateCopyGuts(CFAllocatorRef allocator, CFLocaleRef locale, CFStringRef calendarIdentifier) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), CFLocaleRef, (NSLocale *)locale, copy);
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    CFStringRef localeIdentifier = CFLocaleGetIdentifier(locale);
    
    if (calendarIdentifier) {
        CFDictionaryRef components = CFLocaleCreateComponentsFromLocaleIdentifier(kCFAllocatorSystemDefault, localeIdentifier);
        CFMutableDictionaryRef mcomponents = CFDictionaryCreateMutableCopy(kCFAllocatorSystemDefault, 0, components);
        CFDictionarySetValue(mcomponents, kCFLocaleCalendarIdentifierKey, calendarIdentifier);
        localeIdentifier = CFLocaleCreateLocaleIdentifierFromComponents(kCFAllocatorSystemDefault, mcomponents);
        CFRelease(mcomponents);
        CFRelease(components);
    } else {
        localeIdentifier = CFStringCreateCopy(allocator, localeIdentifier);
    }

    struct __CFLocale *loc = NULL;
    uint32_t size = sizeof(struct __CFLocale) - sizeof(CFRuntimeBase);
    loc = (struct __CFLocale *)_CFRuntimeCreateInstance(allocator, CFLocaleGetTypeID(), size, NULL);
    if (NULL == loc) {
        if (localeIdentifier) { CFRelease(localeIdentifier); }
        return NULL;
    }
    __CFLocaleSetType(loc, __CFLocaleGetType(locale));
    loc->_identifier = localeIdentifier;
    CFDictionaryRef prefs = __CFLocaleGetPrefs(locale);
    loc->_prefs = prefs ? CFRetain(prefs) : NULL;
    loc->_lock = CFLockInit;
    loc->_doesNotRequireSpecialCaseHandling = locale->_doesNotRequireSpecialCaseHandling;
    return (CFLocaleRef)loc;
}

//CFLocaleCreateCopy() always just retained. This caused problems because CFLocaleGetValue(locale, kCFLocaleCalendarKey) would create a calendar, then set its locale to self, leading to a retain cycle
CFLocaleRef CFLocaleCreateCopy(CFAllocatorRef allocator, CFLocaleRef locale) {
    return _CFLocaleCreateCopyGuts(allocator, locale, NULL);
}

//For CFDateFormatter
CF_PRIVATE CFLocaleRef _CFLocaleCreateCopyWithNewCalendarIdentifier(CFAllocatorRef allocator, CFLocaleRef locale, CFStringRef calendarIdentifier) {
    return _CFLocaleCreateCopyGuts(allocator, locale, calendarIdentifier);
}

CFStringRef CFLocaleGetIdentifier(CFLocaleRef locale) {
    CF_OBJC_FUNCDISPATCHV(CFLocaleGetTypeID(), CFStringRef, (NSLocale *)locale, localeIdentifier);
    return locale->_identifier;
}

CFTypeRef CFLocaleGetValue(CFLocaleRef locale, CFStringRef key) {
#if TARGET_OS_OSX
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
	if (*__CFLocaleKeyTable[idx].key == key) {
	    slot = idx;
	    break;
	}
    }
    if (-1 == slot && NULL != key) {
	for (idx = 0; idx < __kCFLocaleKeyTableCount; idx++) {
	    if (CFEqual(*__CFLocaleKeyTable[idx].key, key)) {
		slot = idx;
		break;
	    }
	}
    }
    if (-1 == slot) {
	return NULL;
    }
    CFTypeRef value;
    __CFLocaleLock(locale);
    struct key_table const entry = __CFLocaleKeyTable[slot];
    if (__CFLocaleCacheGetValueIfPresent(locale, *entry.key, &value)) {
	__CFLocaleUnlock(locale);
	return value;
    }
    CFStringRef const context = entry.context ? *entry.context : NULL;
    if (__kCFLocaleUser == __CFLocaleGetType(locale) && entry.get(locale, true, &value, context)) {
	if (value) __CFLocaleCacheSet_alreadyLocked(locale, *entry.key, value);
	if (value) CFRelease(value);
	__CFLocaleUnlock(locale);
	return value;
    }
    if (entry.get(locale, false, &value, context)) {
	if (value) __CFLocaleCacheSet_alreadyLocked(locale, *entry.key, value);
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
	if (*__CFLocaleKeyTable[idx].key == key) {
	    slot = idx;
	    break;
	}
    }
    if (-1 == slot && NULL != key) {
	for (idx = 0; idx < __kCFLocaleKeyTableCount; idx++) {
	    if (CFEqual(*__CFLocaleKeyTable[idx].key, key)) {
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
        if (__CFLocaleKeyTable[slot].name(localeID, cValue, &result)) {
            return result;
        }

        // We could not find a result using the requested language. Fall back through all preferred languages.
        CFArrayRef langPref = NULL;
	if (displayLocale->_prefs) {
	    langPref = (CFArrayRef)CFDictionaryGetValue(displayLocale->_prefs, CFSTR("AppleLanguages"));
	    if (langPref) CFRetain(langPref);
	} else {
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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

#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
    const char* const* p = uloc_getISOLanguages();
    return __CFLocaleCopyCStringsAsArray(p);
#else
    return CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
#endif
}

CFArrayRef CFLocaleCopyISOCountryCodes(void) {
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
    const char* const* p = uloc_getISOCountries();
    return __CFLocaleCopyCStringsAsArray(p);
#else
    return CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
#endif
}

CFArrayRef CFLocaleCopyISOCurrencyCodes(void) {
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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

_CFLocaleCalendarDirection _CFLocaleGetCalendarDirection(void) {
#if TARGET_OS_MAC
    _CFLocaleCalendarDirection calendarDirection = _kCFLocaleCalendarDirectionLeftToRight;
    Boolean keyExistsAndHasValidFormat = false;
    Boolean calendarIsRightToLeft = CFPreferencesGetAppBooleanValue(CFSTR("NSLocaleCalendarDirectionIsRightToLeft"), kCFPreferencesAnyApplication, &keyExistsAndHasValidFormat);
    if (keyExistsAndHasValidFormat) {
        calendarDirection = calendarIsRightToLeft ? _kCFLocaleCalendarDirectionRightToLeft : _kCFLocaleCalendarDirectionLeftToRight;
    } else {
        // If there was no default set, return the directionality of the effective language,
        // except for Hebrew, where the default should be LTR
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFArrayRef bundleLocalizations = CFBundleCopyBundleLocalizations(mainBundle);

        if (NULL != bundleLocalizations) {
            CFArrayRef effectiveLocalizations = CFBundleCopyPreferredLocalizationsFromArray(bundleLocalizations);
            CFStringRef effectiveLocale = CFArrayGetValueAtIndex(effectiveLocalizations, 0);
            CFDictionaryRef effectiveLocaleComponents = CFLocaleCreateComponentsFromLocaleIdentifier(kCFAllocatorDefault, effectiveLocale);
            CFStringRef effectiveLanguage = CFDictionaryGetValue(effectiveLocaleComponents, kCFLocaleLanguageCodeKey);
            if (NULL != effectiveLanguage) {
                CFLocaleLanguageDirection effectiveLanguageDirection = CFLocaleGetLanguageCharacterDirection(effectiveLanguage);
                calendarDirection = (effectiveLanguageDirection == kCFLocaleLanguageDirectionRightToLeft) ? _kCFLocaleCalendarDirectionRightToLeft : _kCFLocaleCalendarDirectionLeftToRight;
            }
            CFRelease(effectiveLocaleComponents);
            CFRelease(effectiveLocalizations);
            CFRelease(bundleLocalizations);
        }
    }
    return calendarDirection;
#else
    return _kCFLocaleCalendarDirectionLeftToRight;
#endif
}

static CFArrayRef _CFLocaleCopyPreferredLanguagesFromPrefs(CFArrayRef languagesArray) {
    CFMutableArrayRef newArray = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    if (languagesArray && (CFArrayGetTypeID() == CFGetTypeID(languagesArray))) {
        for (CFIndex idx = 0, cnt = CFArrayGetCount(languagesArray); idx < cnt; idx++) {
            CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(languagesArray, idx);
            if (str && (CFStringGetTypeID() == CFGetTypeID(str))) {
                CFStringRef ident = CFLocaleCreateCanonicalLanguageIdentifierFromString(kCFAllocatorSystemDefault, str);
                if (ident) {
                    CFArrayAppendValue(newArray, ident);
                    CFRelease(ident);
                }
            }
        }
    }
    return newArray;
}

static CFArrayRef __CFLocaleCopyPreferredLanguagesForCurrentUser(Boolean forCurrentUser) {
    CFArrayRef languagesArray = NULL;
    if (forCurrentUser) {
        languagesArray = (CFArrayRef)CFPreferencesCopyValue(CFSTR("AppleLanguages"), kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    } else {
        languagesArray = (CFArrayRef)CFPreferencesCopyAppValue(CFSTR("AppleLanguages"), kCFPreferencesCurrentApplication);
    }
    CFArrayRef result = _CFLocaleCopyPreferredLanguagesFromPrefs(languagesArray);
    if (languagesArray) CFRelease(languagesArray);
    return result;
}

CFArrayRef CFLocaleCopyPreferredLanguages(void) {
    return __CFLocaleCopyPreferredLanguagesForCurrentUser(false);
}

CFArrayRef _CFLocaleCopyPreferredLanguagesForCurrentUser(void) {
    return __CFLocaleCopyPreferredLanguagesForCurrentUser(true);
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
    static CFStringRef const kCFLocaleCodesKey = CFSTR("__kCFLocaleCodes");
    
    bool codesWasAllocated = false;
    CFDictionaryRef codes = NULL;
    if (!__CFLocaleCacheGetValueIfPresent(locale, kCFLocaleCodesKey, (const void **)&codes)) {
        codes = CFLocaleCreateComponentsFromLocaleIdentifier(kCFAllocatorSystemDefault, locale->_identifier);
        codesWasAllocated = (codes != NULL);
        
        // This function is only called from a __CFLocaleKeyTable[i].get(...) access, which only happens under a lock.
        // We explicitly assign `NULL` into the cache here to prevent trying to call `CFLocaleCreateComponentsFromLocaleIdentifier` on every access of this — `locale->_identifier` is immutable, so the results would never change.
        __CFLocaleCacheSet_alreadyLocked(locale, kCFLocaleCodesKey, codes);
    }
    
    CFStringRef value = codes ? (CFStringRef)CFDictionaryGetValue(codes, context) : NULL; // context is one of kCFLocale*Code constants
    if (codesWasAllocated) {
        CFRelease(codes);
    }
    
    if (value) {
        *cf = CFRetain(value);
        return true;
    } else {
        return false;
    }
}

#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
    if (__CFLocaleCopyCalendarID(locale, user, cf, context)) {
        CFCalendarRef calendar = _CFCalendarCreateCoWWithIdentifier((CFStringRef)*cf);
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX  || TARGET_OS_BSD
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

#if TARGET_OS_MAC
STATIC_CONST_STRING_DECL(_metricUnitsKey, "AppleMetricUnits");
STATIC_CONST_STRING_DECL(_measurementUnitsKey, "AppleMeasurementUnits");
STATIC_CONST_STRING_DECL(_measurementUnitsCentimeters, "Centimeters");
STATIC_CONST_STRING_DECL(_measurementUnitsInches, "Inches");
STATIC_CONST_STRING_DECL(_temperatureUnitKey, "AppleTemperatureUnit");

static bool __CFLocaleGetMeasurementSystemForPreferences(CFTypeRef metricPref, CFTypeRef measurementPref, UMeasurementSystem *outMeasurementSystem) {
    if (metricPref || measurementPref) {
#if U_ICU_VERSION_MAJOR_NUM >= 55
        if (metricPref == kCFBooleanTrue && measurementPref && CFEqual(measurementPref, _measurementUnitsInches)) {
            *outMeasurementSystem = UMS_UK;
#else
            return false;
#endif
        } else if (metricPref == kCFBooleanFalse) {
            *outMeasurementSystem = UMS_US;
        } else {
            *outMeasurementSystem = UMS_SI;
        }
        return true;
    }
    return false;
}

static void __CFLocaleGetPreferencesForMeasurementSystem(UMeasurementSystem measurementSystem, CFTypeRef *outMetricPref, CFTypeRef *outMeasurementPref) {
    *outMetricPref = measurementSystem != UMS_US? kCFBooleanTrue: kCFBooleanFalse;
    *outMeasurementPref = measurementSystem == UMS_SI? _measurementUnitsCentimeters: _measurementUnitsInches;
}
#endif

#if U_ICU_VERSION_MAJOR_NUM > 54 || (DEPLOYMENT_RUNTIME_OBJC && TARGET_OS_MAC)
static bool _CFLocaleGetTemperatureUnitForPreferences(CFTypeRef temperaturePref, bool *outCelsius) {
    if (temperaturePref) {
        if (CFEqual(temperaturePref, kCFLocaleTemperatureUnitCelsius)) {
            *outCelsius = true;
            return true;
        } else if (CFEqual(temperaturePref, kCFLocaleTemperatureUnitFahrenheit)) {
            *outCelsius = false;
            return true;
        }
    }
    return false;
}
#endif

#if U_ICU_VERSION_MAJOR_NUM > 54 || (DEPLOYMENT_RUNTIME_OBJC && TARGET_OS_MAC)
static CFStringRef _CFLocaleGetTemperatureUnitName(bool celsius) {
    return celsius? kCFLocaleTemperatureUnitCelsius: kCFLocaleTemperatureUnitFahrenheit;
}
#endif

#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

static CFStringRef __CFLocaleGetMeasurementSystemName(UMeasurementSystem measurementSystem) {
    switch (measurementSystem) {
        case UMS_US:
            return kCFLocaleMeasurementSystemUS;
#if U_ICU_VERSION_MAJOR_NUM >= 55
        case UMS_UK:
            return kCFLocaleMeasurementSystemUK;
#endif
        default:
            break;
    }
    return kCFLocaleMeasurementSystemMetric;
}

static  bool __CFLocaleGetMeasurementSystemForName(CFStringRef name, UMeasurementSystem *outMeasurementSystem) {
    if (name) {
        if (CFEqual(name, kCFLocaleMeasurementSystemMetric)) {
            *outMeasurementSystem = UMS_SI;
            return true;
        }
        if (CFEqual(name, kCFLocaleMeasurementSystemUS)) {
            *outMeasurementSystem = UMS_US;
            return true;
        }
#if U_ICU_VERSION_MAJOR_NUM >= 55
        if (CFEqual(name, kCFLocaleMeasurementSystemUK)) {
            *outMeasurementSystem = UMS_UK;
            return true;
        }
#endif
    }
    return false;
}

#pragma clang diagnostic pop

#endif

#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
static void __CFLocaleGetMeasurementSystemGuts(CFLocaleRef locale, bool user, UMeasurementSystem *outMeasurementSystem) {
    UMeasurementSystem output = UMS_SI;    // Default is Metric
    bool done = false;
#if TARGET_OS_MAC
    if (user) {
        CFTypeRef metricPref = CFDictionaryGetValue(locale->_prefs, _metricUnitsKey);
        CFTypeRef measurementPref = CFDictionaryGetValue(locale->_prefs, _measurementUnitsKey);
        done = __CFLocaleGetMeasurementSystemForPreferences(metricPref, measurementPref, &output);
    }
#endif
    if (!done) {
        char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
        if (CFStringGetCString(locale->_identifier, localeID, sizeof(localeID)/sizeof(char), kCFStringEncodingASCII)) {
            UErrorCode  icuStatus = U_ZERO_ERROR;
            output = ulocdata_getMeasurementSystem(localeID, &icuStatus);
            if (U_SUCCESS(icuStatus)) {
                done = true;
            }
        }
    }
    if (!done) {
        output = UMS_SI;
    }
    *outMeasurementSystem = output;
}
#endif


static bool __CFLocaleCopyUsesMetric(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
    UMeasurementSystem system = UMS_SI;
    __CFLocaleGetMeasurementSystemGuts(locale, user, &system);
    *cf = system != UMS_US ? kCFBooleanTrue : kCFBooleanFalse;
    return true;
#else
    *cf = kCFBooleanFalse;  //historical behavior, probably irrelevant in CF Mini
    return true;
#endif
}

static bool __CFLocaleCopyMeasurementSystem(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
    UMeasurementSystem system = UMS_SI;
    __CFLocaleGetMeasurementSystemGuts(locale, user, &system);
    *cf = CFRetain(__CFLocaleGetMeasurementSystemName(system));
    return true;
#else
    *cf = CFRetain(kCFLocaleMeasurementSystemUS); //historical behavior, probably irrelevant in CF Mini
    return true;
#endif
}



static bool __CFLocaleCopyTemperatureUnit(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
#if U_ICU_VERSION_MAJOR_NUM > 54
    bool celsius = true;    // Default is Celsius
    bool done = false;
#if TARGET_OS_MAC
    if (user) {
        CFTypeRef temperatureUnitPref = CFDictionaryGetValue(locale->_prefs, _temperatureUnitKey);
        done = _CFLocaleGetTemperatureUnitForPreferences(temperatureUnitPref, &celsius);
    }
#endif
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
    if (!done) {
        char localeID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
        if (CFStringGetCString(locale->_identifier, localeID, sizeof(localeID)/sizeof(char), kCFStringEncodingASCII)) {
#if U_ICU_VERSION_MAJOR_NUM > 53 && __has_include(<unicode/uameasureformat.h>)
            UErrorCode icuStatus = U_ZERO_ERROR;
            UAMeasureUnit unit;
            int32_t unitCount = uameasfmt_getUnitsForUsage(localeID, "temperature", "weather", &unit, 1, &icuStatus);
            if (U_SUCCESS(icuStatus) && unitCount > 0) {
                if (unit == UAMEASUNIT_TEMPERATURE_FAHRENHEIT) {
                    celsius = false;
                }
                done = true;
            }
#endif
        }
    }
    if (!done) {
        UMeasurementSystem system = UMS_SI;
        __CFLocaleGetMeasurementSystemGuts(locale, user, &system);
        if (system == UMS_US) {
            celsius = false;
        }
        done = true;
    }
#endif
    if (!done) {
        celsius = true;
    }
    *cf = CFRetain(_CFLocaleGetTemperatureUnitName(celsius));
    return true;
#else
    return false;
#endif
}

static bool __CFLocaleCopyNumberFormat(CFLocaleRef locale, bool user, CFTypeRef *cf, CFStringRef context) {
    CFStringRef str = NULL;
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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

#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
    return __CFLocaleICUName(locale, value, out, uloc_getDisplayLanguage);
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleCountryName(const char *locale, const char *value, CFStringRef *out) {
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
    return __CFLocaleICUKeywordValueName(locale, value, kCalendarKeyword, out);
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleCollationName(const char *locale, const char *value, CFStringRef *out) {
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
    return __CFLocaleICUKeywordValueName(locale, value, kCollationKeyword, out);
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleCurrencyShortName(const char *locale, const char *value, CFStringRef *out) {
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
    return __CFLocaleICUCurrencyName(locale, value, UCURR_SYMBOL_NAME, out);
#else
    *out = CFRetain(CFSTR("(none)"));
    return true;
#endif
}

static bool __CFLocaleCurrencyFullName(const char *locale, const char *value, CFStringRef *out) {
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD
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

