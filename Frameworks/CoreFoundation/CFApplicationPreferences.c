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

/*	CFApplicationPreferences.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: David Smith
*/

#include <CoreFoundation/CFPreferences.h>
#include "CFInternal.h"
#include <CoreFoundation/CFUniChar.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFNumberFormatter.h>
#include <CoreFoundation/CFDateFormatter.h>
#include <sys/types.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#include <unistd.h>
#endif

static Boolean _CFApplicationPreferencesSynchronizeNoLock(_CFApplicationPreferences *self);
void _CFPreferencesDomainSetMultiple(CFPreferencesDomainRef domain, CFDictionaryRef dict);
static void updateDictRep(_CFApplicationPreferences *self);
static void _CFApplicationPreferencesSetSearchList(_CFApplicationPreferences *self, CFArrayRef newSearchList);
Boolean _CFApplicationPreferencesContainsDomainNoLock(_CFApplicationPreferences *self, CFPreferencesDomainRef domain);
static CFTypeRef _CFApplicationPreferencesCreateValueForKey2(_CFApplicationPreferences *self, CFStringRef defaultName);

// Right now, nothing is getting destroyed pretty much ever.  We probably don't want this to be the case, but it's very tricky - domains live in the cache as well as a given application's preferences, and since they're not CFTypes, there's no reference count.  Also, it's not clear we ever want domains destroyed.  When they're synchronized, they clear their internal state (to force reading from the disk again), so they're not very big.... REW, 12/17/98

CFPropertyListRef CFPreferencesCopyAppValue(CFStringRef key, CFStringRef appName) {
    _CFApplicationPreferences *standardPrefs;
    CFAssert1(appName != NULL, __kCFLogAssertion, "%s(): Cannot access application preferences with a NULL application name", __PRETTY_FUNCTION__);
    CFAssert1(key != NULL, __kCFLogAssertion, "%s(): Cannot access preferences with a NULL key", __PRETTY_FUNCTION__);
    
    standardPrefs = _CFStandardApplicationPreferences(appName);
    return standardPrefs ? _CFApplicationPreferencesCreateValueForKey2(standardPrefs, key) : NULL;
}

CF_EXPORT Boolean CFPreferencesAppBooleanValue(CFStringRef key, CFStringRef appName, Boolean *keyExistsAndHasValidFormat) {
    CFPropertyListRef value;
    Boolean result, valid;
    CFTypeID typeID = 0;
    CFAssert1(appName != NULL, __kCFLogAssertion, "%s(): Cannot access application preferences with a NULL application name", __PRETTY_FUNCTION__);
    CFAssert1(key != NULL, __kCFLogAssertion, "%s(): Cannot access preferences with a NULL key", __PRETTY_FUNCTION__);

    if (!keyExistsAndHasValidFormat) {
        keyExistsAndHasValidFormat = &valid;
    }
    value = CFPreferencesCopyAppValue(key, appName);
    if (!value) {
        *keyExistsAndHasValidFormat = false;
        return false;
    }
    typeID = CFGetTypeID(value);
    if (typeID == CFStringGetTypeID()) {
        if (CFStringCompare((CFStringRef)value, CFSTR("true"), kCFCompareCaseInsensitive) == kCFCompareEqualTo || CFStringCompare((CFStringRef)value, CFSTR("YES"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            *keyExistsAndHasValidFormat = true;
            result = true;
        } else if (CFStringCompare((CFStringRef)value, CFSTR("false"), kCFCompareCaseInsensitive) == kCFCompareEqualTo || CFStringCompare((CFStringRef)value, CFSTR("NO"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            *keyExistsAndHasValidFormat = true;
            result = false;
        } else {
            *keyExistsAndHasValidFormat = false;
            result = false;
        }
    } else if (typeID == CFNumberGetTypeID()) {
        if (CFNumberIsFloatType((CFNumberRef)value)) {
            *keyExistsAndHasValidFormat = false;
            result = false;
        } else {
            int i;
            *keyExistsAndHasValidFormat = true;
            CFNumberGetValue((CFNumberRef)value, kCFNumberIntType, &i);
            result = (i == 0) ? false : true;
        }
    } else if (typeID == CFBooleanGetTypeID()) {
        result = (value == kCFBooleanTrue);
        *keyExistsAndHasValidFormat = true;
    } else {
        // Unknown type
        result = false;
        *keyExistsAndHasValidFormat = false;
    }
    CFRelease(value);
    return result;
}

CF_PRIVATE CFIndex CFPreferencesAppIntegerValue(CFStringRef key, CFStringRef appName, Boolean *keyExistsAndHasValidFormat) {
    CFPropertyListRef value;
    CFIndex result;
    CFTypeID typeID = 0;
    Boolean valid;
    CFAssert1(appName != NULL, __kCFLogAssertion, "%s(): Cannot access application preferences with a NULL application name", __PRETTY_FUNCTION__);
    CFAssert1(key != NULL, __kCFLogAssertion, "%s(): Cannot access preferences with a NULL key", __PRETTY_FUNCTION__);

    value = CFPreferencesCopyAppValue(key, appName);
    if (!keyExistsAndHasValidFormat) {
        keyExistsAndHasValidFormat = &valid;
    }
    if (!value) {
        *keyExistsAndHasValidFormat = false;
        return 0;
    }
    typeID = CFGetTypeID(value);
    if (typeID == CFStringGetTypeID()) {
        SInt32 charIndex = 0;
        SInt32 intVal;
        CFStringInlineBuffer buf;
        Boolean success;
        CFStringInitInlineBuffer((CFStringRef)value, &buf, CFRangeMake(0, CFStringGetLength((CFStringRef)value)));
        success = __CFStringScanInteger(&buf, NULL, &charIndex, false, &intVal);
        *keyExistsAndHasValidFormat = (success && charIndex == CFStringGetLength((CFStringRef)value));
        result = (*keyExistsAndHasValidFormat) ? intVal : 0;
    } else if (typeID == CFNumberGetTypeID()) {
        *keyExistsAndHasValidFormat = !CFNumberIsFloatType((CFNumberRef)value);
        if (*keyExistsAndHasValidFormat) {
            CFNumberGetValue((CFNumberRef)value, kCFNumberCFIndexType, &result);
        } else {
            result = 0;
        }
    } else {
        // Unknown type
        result = 0;
        *keyExistsAndHasValidFormat = false;
    }
    CFRelease(value);
    return result;
}

Boolean CFPreferencesGetAppBooleanValue(CFStringRef key, CFStringRef appName, Boolean *keyExistsAndHasValidFormat) {
    CFAssert1(appName != NULL, __kCFLogAssertion, "%s(): Cannot access application preferences with a NULL application name", __PRETTY_FUNCTION__);
    CFAssert1(key != NULL, __kCFLogAssertion, "%s(): Cannot access preferences with a NULL key", __PRETTY_FUNCTION__);
    return CFPreferencesAppBooleanValue(key, appName, keyExistsAndHasValidFormat);
}

CFIndex CFPreferencesGetAppIntegerValue(CFStringRef key, CFStringRef appName, Boolean *keyExistsAndHasValidFormat) {
    CFAssert1(appName != NULL, __kCFLogAssertion, "%s(): Cannot access application preferences with a NULL application name", __PRETTY_FUNCTION__);
    CFAssert1(key != NULL, __kCFLogAssertion, "%s(): Cannot access preferences with a NULL key", __PRETTY_FUNCTION__);
    return CFPreferencesAppIntegerValue(key, appName, keyExistsAndHasValidFormat);
}

void CFPreferencesSetAppValue(CFStringRef key, CFTypeRef value, CFStringRef appName) {
    _CFApplicationPreferences *standardPrefs;
    CFAssert1(appName != NULL, __kCFLogAssertion, "%s(): Cannot access application preferences with a NULL application name", __PRETTY_FUNCTION__);
    CFAssert1(key != NULL, __kCFLogAssertion, "%s(): Cannot access preferences with a NULL key", __PRETTY_FUNCTION__);

    standardPrefs = _CFStandardApplicationPreferences(appName);
    if (standardPrefs) {
        if (value) _CFApplicationPreferencesSet(standardPrefs, key, value);
        else _CFApplicationPreferencesRemove(standardPrefs, key);
    }
}


static CFLock_t __CFApplicationPreferencesLock = CFLockInit; // Locks access to __CFStandardUserPreferences
static CFMutableDictionaryRef __CFStandardUserPreferences = NULL; // Mutable dictionary; keys are app names, values are _CFApplicationPreferences 

Boolean CFPreferencesAppSynchronize(CFStringRef appName) {
    _CFApplicationPreferences *standardPrefs;
    Boolean result;
    CFAssert1(appName != NULL, __kCFLogAssertion, "%s(): Cannot access application preferences with a NULL application name", __PRETTY_FUNCTION__);
    
    // Do not call _CFStandardApplicationPreferences(), as we do not want to create the preferences only to synchronize
    __CFLock(&__CFApplicationPreferencesLock);
    if (__CFStandardUserPreferences)  {
        standardPrefs = (_CFApplicationPreferences *)CFDictionaryGetValue(__CFStandardUserPreferences, appName);
    } else {
        standardPrefs = NULL;
    }

    result = standardPrefs ? _CFApplicationPreferencesSynchronizeNoLock(standardPrefs) : _CFSynchronizeDomainCache();
    __CFUnlock(&__CFApplicationPreferencesLock);
    return result;
}

void CFPreferencesFlushCaches(void) {
    CFAllocatorRef alloc = __CFPreferencesAllocator();
    __CFLock(&__CFApplicationPreferencesLock);
    if (__CFStandardUserPreferences)  {
        _CFApplicationPreferences **prefsArray, *prefsBuf[32];
        CFIndex idx, count = CFDictionaryGetCount(__CFStandardUserPreferences);
        if (count < 32) {
            prefsArray = prefsBuf;
        } else {
            prefsArray = (_CFApplicationPreferences **)CFAllocatorAllocate(alloc, count * sizeof(_CFApplicationPreferences *), 0);
        }
        CFDictionaryGetKeysAndValues(__CFStandardUserPreferences, NULL, (const void **)prefsArray);

        __CFUnlock(&__CFApplicationPreferencesLock);
        // DeallocateApplicationPreferences needs the lock
        for (idx = 0; idx < count; idx ++) {
            _CFApplicationPreferences *appPrefs = prefsArray[idx];
            _CFApplicationPreferencesSynchronize(appPrefs);
            _CFDeallocateApplicationPreferences(appPrefs);
        }
        __CFLock(&__CFApplicationPreferencesLock);

        CFRelease(__CFStandardUserPreferences);
        __CFStandardUserPreferences = NULL;
        if(prefsArray != prefsBuf) CFAllocatorDeallocate(alloc, prefsArray);
    }
    __CFUnlock(&__CFApplicationPreferencesLock);
    _CFPreferencesPurgeDomainCache();
}

// quick message to indicate that the given domain has changed, and we should go through and invalidate any dictReps that involve this domain.
void _CFApplicationPreferencesDomainHasChanged(CFPreferencesDomainRef changedDomain) {
    CFAllocatorRef alloc = __CFPreferencesAllocator();
    __CFLock(&__CFApplicationPreferencesLock);
    if(__CFStandardUserPreferences) {  // only grovel over the prefs if there's something there to grovel
        _CFApplicationPreferences **prefsArray, *prefsBuf[32];
        CFIndex idx, count = CFDictionaryGetCount(__CFStandardUserPreferences);
        if(count < 32) {
            prefsArray = prefsBuf;
        } else {
            prefsArray = (_CFApplicationPreferences **)CFAllocatorAllocate(alloc, count * sizeof(_CFApplicationPreferences *), 0);
        }
        CFDictionaryGetKeysAndValues(__CFStandardUserPreferences, NULL, (const void **)prefsArray);
        // For this operation, giving up the lock is the last thing we want to do, so use the modified flavor of _CFApplicationPreferencesContainsDomain
        for(idx = 0; idx < count; idx++) {
            _CFApplicationPreferences *appPrefs = prefsArray[idx];
            if(_CFApplicationPreferencesContainsDomainNoLock(appPrefs, changedDomain)) {
                updateDictRep(appPrefs);
            }
        }
        if(prefsArray != prefsBuf) _CFAllocatorDeallocateGC(alloc, prefsArray);
    }
    __CFUnlock(&__CFApplicationPreferencesLock);
}


// Begin ported code from NSUserDefaults.m


static void updateDictRep(_CFApplicationPreferences *self) {
    if (self->_dictRep) {
        CFRelease(self->_dictRep);
        self->_dictRep = NULL;
    }
}

static void __addKeysAndValues(const void *key, const void *value, void *context) {
    CFDictionarySetValue((CFMutableDictionaryRef)context, key, value);
}

static CFMutableDictionaryRef computeDictRep(_CFApplicationPreferences *self, Boolean skipC0C0A) {
    CFAllocatorRef alloc = __CFPreferencesAllocator();
    CFMutableArrayRef searchList = self->_search;
    CFIndex idx;
    CFIndex cnt = CFArrayGetCount(searchList);
    CFDictionaryRef subdomainDict;
    CFMutableDictionaryRef dictRep;
    
    dictRep = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, & kCFTypeDictionaryValueCallBacks);
    _CFDictionarySetCapacity(dictRep, 260);	// avoid lots of rehashing
    
    // For each subdomain triplet in the domain, iterate over dictionaries, adding them if necessary to the dictRep
    for (idx = cnt; idx--;) {
        CFPreferencesDomainRef domain = (CFPreferencesDomainRef)CFArrayGetValueAtIndex(searchList, idx);

        if (!domain) continue;

        subdomainDict = _CFPreferencesDomainDeepCopyDictionary(domain);
        if (subdomainDict) {
            CFDictionaryApplyFunction(subdomainDict, __addKeysAndValues, dictRep);
            CFRelease(subdomainDict);
        }
    }
    return dictRep;
}

CFTypeRef _CFApplicationPreferencesSearchDownToDomain(_CFApplicationPreferences *self, CFPreferencesDomainRef stopper, CFStringRef key) {
    return NULL;
}


void _CFApplicationPreferencesUpdate(_CFApplicationPreferences *self) {
    __CFLock(&__CFApplicationPreferencesLock);
    updateDictRep(self);
    __CFUnlock(&__CFApplicationPreferencesLock);
}

CF_EXPORT CFDictionaryRef _CFApplicationPreferencesCopyRepresentation(_CFApplicationPreferences *self);

CF_PRIVATE CFDictionaryRef __CFApplicationPreferencesCopyCurrentState(void) {
    _CFApplicationPreferences *self = _CFStandardApplicationPreferences(kCFPreferencesCurrentApplication);
    CFDictionaryRef result = _CFApplicationPreferencesCopyRepresentation(self);
    return result;
}

// CACHING here - we will only return a value as current as the last time computeDictRep() was called
static CFTypeRef _CFApplicationPreferencesCreateValueForKey2(_CFApplicationPreferences *self, CFStringRef defaultName) {
    CFTypeRef result;
    __CFLock(&__CFApplicationPreferencesLock);
    if (!self->_dictRep) {
        self->_dictRep = computeDictRep(self, true);
    }
    result = (self->_dictRep) ? (CFTypeRef )CFDictionaryGetValue(self->_dictRep, defaultName) : NULL;
    if (result) {
        CFRetain(result);
    }
    __CFUnlock(&__CFApplicationPreferencesLock);
    return result;
}


void _CFApplicationPreferencesSet(_CFApplicationPreferences *self, CFStringRef defaultName, CFTypeRef value) {
    CFPreferencesDomainRef applicationDomain;

    __CFLock(&__CFApplicationPreferencesLock);
    applicationDomain = _CFPreferencesStandardDomain(self->_appName, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    if(applicationDomain) {
        _CFPreferencesDomainSet(applicationDomain, defaultName, value);
        if (CFArrayContainsValue(self->_search, CFRangeMake(0, CFArrayGetCount(self->_search)), applicationDomain)) {
            // Expensive; can't we just check the relevant value throughout the appropriate sets of domains? -- REW, 7/19/99
            updateDictRep(self);
        }
    }
    __CFUnlock(&__CFApplicationPreferencesLock);
}

void _CFApplicationPreferencesRemove(_CFApplicationPreferences *self, CFStringRef defaultName) {
    CFPreferencesDomainRef appDomain;

    __CFLock(&__CFApplicationPreferencesLock);
    appDomain = _CFPreferencesStandardDomain(self->_appName, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    if(appDomain) {
        _CFPreferencesDomainSet(appDomain, defaultName, NULL);
        if (CFArrayContainsValue(self->_search, CFRangeMake(0, CFArrayGetCount(self->_search)), appDomain)) {
            // If key exists, it will be in the _dictRep (but possibly overridden)
            updateDictRep(self);
        }
    }
    __CFUnlock(&__CFApplicationPreferencesLock);
}

static Boolean _CFApplicationPreferencesSynchronizeNoLock(_CFApplicationPreferences *self) {
    Boolean success = _CFSynchronizeDomainCache();
    updateDictRep(self);
    return success;
}

Boolean _CFApplicationPreferencesSynchronize(_CFApplicationPreferences *self) {
    Boolean result;
    __CFLock(&__CFApplicationPreferencesLock);
    result = _CFApplicationPreferencesSynchronizeNoLock(self);
    __CFUnlock(&__CFApplicationPreferencesLock);
    return result;
}

// appName should not be kCFPreferencesCurrentApplication going in to this call
_CFApplicationPreferences *_CFApplicationPreferencesCreateWithUser(CFStringRef userName, CFStringRef appName) {
    CFAllocatorRef alloc = __CFPreferencesAllocator();
    _CFApplicationPreferences *self = (_CFApplicationPreferences*)CFAllocatorAllocate(alloc, sizeof(_CFApplicationPreferences), 0);
    if (self) {
        self->_dictRep = NULL;
        self->_appName = (CFStringRef)CFRetain(appName);
        self->_search = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
        if (!self->_search) {
            CFAllocatorDeallocate(alloc, self);
            CFRelease(appName);
            self = NULL;
        }
    }
    return self;
}

// Do NOT release the domain after adding it to the array; domain_expression should not return a retained object  -- REW, 8/26/99 
#define ADD_DOMAIN(domain_expression) domain = domain_expression; if (domain) {CFArrayAppendValue(search, domain);}
void _CFApplicationPreferencesSetStandardSearchList(_CFApplicationPreferences *appPreferences) {
    /* Here is how the domains end up in priority order in a search list.  Only a subset of these are setup by default.
	argument domain
	this app, this user, managed
	this app, any user, managed
        this app, this user, this host
        this app, this user, any host (AppDomain)
	suiteN, this user, this host
	suiteN, this user, any host
        ...
	suite0, this user, this host
	suite0, this user, any host
        any app, this user, this host
        any app, this user, any host (GlobalDomain)
        NSUserDefaults backwards-compat ICU domain
        this app, any user, this host
        this app, any user, any host
	suiteN, any user, this host
	suiteN, any user, any host
        ...
	suite0, any user, this host
	suite0, any user, any host
        any app, any user, this host
        any app, any user, any host
	registration domain
    */
    CFPreferencesDomainRef domain;
    CFMutableArrayRef search = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    if (!search) {
        // couldn't allocate memory!
        return;
    }

    ADD_DOMAIN(_CFPreferencesStandardDomain(appPreferences->_appName, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost));
    ADD_DOMAIN(_CFPreferencesStandardDomain(appPreferences->_appName, kCFPreferencesCurrentUser, kCFPreferencesAnyHost));
    ADD_DOMAIN(_CFPreferencesStandardDomain(kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost));
    ADD_DOMAIN(_CFPreferencesStandardDomain(kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost));
    ADD_DOMAIN(_CFPreferencesStandardDomain(appPreferences->_appName, kCFPreferencesAnyUser, kCFPreferencesCurrentHost));
    ADD_DOMAIN(_CFPreferencesStandardDomain(appPreferences->_appName, kCFPreferencesAnyUser, kCFPreferencesAnyHost));
    ADD_DOMAIN(_CFPreferencesStandardDomain(kCFPreferencesAnyApplication, kCFPreferencesAnyUser, kCFPreferencesCurrentHost));
    ADD_DOMAIN(_CFPreferencesStandardDomain(kCFPreferencesAnyApplication, kCFPreferencesAnyUser, kCFPreferencesAnyHost));

    _CFApplicationPreferencesSetSearchList(appPreferences, search);
    CFRelease(search);
}
#undef ADD_DOMAIN


CF_PRIVATE _CFApplicationPreferences *_CFStandardApplicationPreferences(CFStringRef appName) {
    _CFApplicationPreferences *appPreferences;
//    CFAssert(appName != kCFPreferencesAnyApplication, __kCFLogAssertion, "Cannot use any of the CFPreferences...App... functions with an appName of kCFPreferencesAnyApplication");
    __CFLock(&__CFApplicationPreferencesLock);
    if (!__CFStandardUserPreferences)  {
        __CFStandardUserPreferences = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, & kCFTypeDictionaryKeyCallBacks, NULL);
    }
    if (!__CFStandardUserPreferences) {
        // Couldn't create
        __CFUnlock(&__CFApplicationPreferencesLock);
        return NULL;
    }
    if ((appPreferences = (_CFApplicationPreferences *)CFDictionaryGetValue(__CFStandardUserPreferences, appName)) == NULL ) {
        appPreferences = _CFApplicationPreferencesCreateWithUser(kCFPreferencesCurrentUser, appName);
        CFDictionarySetValue(__CFStandardUserPreferences, appName, appPreferences);
        __CFUnlock(&__CFApplicationPreferencesLock);
        _CFApplicationPreferencesSetStandardSearchList(appPreferences);
    } else {
        __CFUnlock(&__CFApplicationPreferencesLock);
    }
    return appPreferences;
}

// Exclusively for Foundation's use
void _CFApplicationPreferencesSetCacheForApp(_CFApplicationPreferences *appPrefs, CFStringRef appName) {
    __CFLock(&__CFApplicationPreferencesLock);
    if (!__CFStandardUserPreferences) {
        __CFStandardUserPreferences = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, NULL);
        CFDictionarySetValue(__CFStandardUserPreferences, appName, appPrefs);
        __CFUnlock(&__CFApplicationPreferencesLock);
    } else {
        _CFApplicationPreferences *oldPrefs = (_CFApplicationPreferences *)CFDictionaryGetValue(__CFStandardUserPreferences, appName);
        CFDictionarySetValue(__CFStandardUserPreferences, appName, appPrefs);
        __CFUnlock(&__CFApplicationPreferencesLock);
        if (oldPrefs) {
            _CFDeallocateApplicationPreferences(oldPrefs);
        }
    }
}


void _CFDeallocateApplicationPreferences(_CFApplicationPreferences *self) {
    CFAllocatorRef alloc = __CFPreferencesAllocator();
    _CFApplicationPreferences *cachedPrefs = NULL;
    __CFLock(&__CFApplicationPreferencesLock);

    // Get us out of the cache before destroying!
    if (__CFStandardUserPreferences)  {
        cachedPrefs = (_CFApplicationPreferences *)CFDictionaryGetValue(__CFStandardUserPreferences, self->_appName);
    }
    if (cachedPrefs == self) {
        CFDictionaryRemoveValue(__CFStandardUserPreferences, self->_appName);
    }
    
    if (self->_dictRep) CFRelease(self->_dictRep);
    CFRelease(self->_search);
    CFRelease(self->_appName);
    CFAllocatorDeallocate(alloc, self);
    __CFUnlock(&__CFApplicationPreferencesLock);
}


CF_EXPORT
CFDictionaryRef _CFApplicationPreferencesCopyRepresentation(_CFApplicationPreferences *self) {
    CFDictionaryRef dict;
    __CFLock(&__CFApplicationPreferencesLock);
    if (!self->_dictRep) {
        self->_dictRep = computeDictRep(self, true);
    }
    if (self->_dictRep) {
        CFRetain(self->_dictRep);
    }
    dict = self->_dictRep;
    __CFUnlock(&__CFApplicationPreferencesLock);
    return dict;
}

static void _CFApplicationPreferencesSetSearchList(_CFApplicationPreferences *self, CFArrayRef newSearchList) {
    CFIndex idx, count;
    __CFLock(&__CFApplicationPreferencesLock);
    CFArrayRemoveAllValues(self->_search);
    count = CFArrayGetCount(newSearchList);
    for (idx = 0; idx < count; idx ++) {
        CFArrayAppendValue(self->_search, CFArrayGetValueAtIndex(newSearchList, idx));
    }
    updateDictRep(self);
    __CFUnlock(&__CFApplicationPreferencesLock);
}

void CFPreferencesAddSuitePreferencesToApp(CFStringRef appName, CFStringRef suiteName) {
    _CFApplicationPreferences *appPrefs;
    
    appPrefs = _CFStandardApplicationPreferences(appName);
    _CFApplicationPreferencesAddSuitePreferences(appPrefs, suiteName);
}

void _CFApplicationPreferencesAddSuitePreferences(_CFApplicationPreferences *appPrefs, CFStringRef suiteName) {
    CFPreferencesDomainRef domain; 
    CFIndex idx;
    CFRange range;

    // Find where to insert the new suite
    __CFLock(&__CFApplicationPreferencesLock);
    domain = _CFPreferencesStandardDomain(appPrefs->_appName, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    range.location = 0;
    range.length = CFArrayGetCount(appPrefs->_search);
    idx = domain ? CFArrayGetFirstIndexOfValue(appPrefs->_search, range, domain) : kCFNotFound;
    __CFUnlock(&__CFApplicationPreferencesLock);
    idx ++; // We want just below the app domain.  Coincidentally, this gives us the top of the list if the app domain has been removed.
    domain = _CFPreferencesStandardDomain(suiteName, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    if (domain) {
        __CFLock(&__CFApplicationPreferencesLock);
        CFArrayInsertValueAtIndex(appPrefs->_search, idx, domain);
        __CFUnlock(&__CFApplicationPreferencesLock);
        range.length ++;
    }
    domain = _CFPreferencesStandardDomain(suiteName, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
    if (domain) {
        __CFLock(&__CFApplicationPreferencesLock);
        CFArrayInsertValueAtIndex(appPrefs->_search, idx, domain);
        __CFUnlock(&__CFApplicationPreferencesLock);
        range.length ++;
    }

    // Now the AnyUser domains
    domain = _CFPreferencesStandardDomain(appPrefs->_appName, kCFPreferencesAnyUser, kCFPreferencesAnyHost);
    idx = domain ? CFArrayGetFirstIndexOfValue(appPrefs->_search, range, domain) : kCFNotFound;
    if (idx == kCFNotFound) {
        // Someone blew away the app domain. For the any user case, we look for right below the global domain
        domain = _CFPreferencesStandardDomain(kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
        idx = domain ? CFArrayGetFirstIndexOfValue(appPrefs->_search, range, domain) : kCFNotFound;
        if (idx == kCFNotFound) {
            // Try the "any host" choice
            domain = _CFPreferencesStandardDomain(kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
            idx = domain ? CFArrayGetFirstIndexOfValue(appPrefs->_search, range, domain) : kCFNotFound;
            if (idx == kCFNotFound) {
                // We give up; put the new domains at the bottom
                idx = CFArrayGetCount(appPrefs->_search) - 1;
            }
        }
    }
    idx ++;
    domain = _CFPreferencesStandardDomain(suiteName, kCFPreferencesAnyUser, kCFPreferencesAnyHost);
    if (domain) {
        __CFLock(&__CFApplicationPreferencesLock);
        CFArrayInsertValueAtIndex(appPrefs->_search, idx, domain);
        __CFUnlock(&__CFApplicationPreferencesLock);
    }
    domain = _CFPreferencesStandardDomain(suiteName, kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    if (domain) {
        __CFLock(&__CFApplicationPreferencesLock);
        CFArrayInsertValueAtIndex(appPrefs->_search, idx, domain);
        __CFUnlock(&__CFApplicationPreferencesLock);
    }
    __CFLock(&__CFApplicationPreferencesLock);
    updateDictRep(appPrefs);
    __CFUnlock(&__CFApplicationPreferencesLock);
}

void CFPreferencesRemoveSuitePreferencesFromApp(CFStringRef appName, CFStringRef suiteName) {
    _CFApplicationPreferences *appPrefs;

    appPrefs = _CFStandardApplicationPreferences(appName);
    
    _CFApplicationPreferencesRemoveSuitePreferences(appPrefs, suiteName);
}

void _CFApplicationPreferencesRemoveSuitePreferences(_CFApplicationPreferences *appPrefs, CFStringRef suiteName) {
    CFPreferencesDomainRef domain;

    __CFLock(&__CFApplicationPreferencesLock);
    domain = _CFPreferencesStandardDomain(suiteName, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    __CFUnlock(&__CFApplicationPreferencesLock);
    if (domain) _CFApplicationPreferencesRemoveDomain(appPrefs, domain);

    __CFLock(&__CFApplicationPreferencesLock);
    domain = _CFPreferencesStandardDomain(suiteName, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
    __CFUnlock(&__CFApplicationPreferencesLock);
    if (domain) _CFApplicationPreferencesRemoveDomain(appPrefs, domain);

    __CFLock(&__CFApplicationPreferencesLock);
    domain = _CFPreferencesStandardDomain(suiteName, kCFPreferencesAnyUser, kCFPreferencesAnyHost);
    __CFUnlock(&__CFApplicationPreferencesLock);
    if (domain) _CFApplicationPreferencesRemoveDomain(appPrefs, domain);

    __CFLock(&__CFApplicationPreferencesLock);
    domain = _CFPreferencesStandardDomain(suiteName, kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    __CFUnlock(&__CFApplicationPreferencesLock);
    if (domain) _CFApplicationPreferencesRemoveDomain(appPrefs, domain);
}

void _CFApplicationPreferencesAddDomain(_CFApplicationPreferences *self, CFPreferencesDomainRef domain, Boolean addAtTop) {
    __CFLock(&__CFApplicationPreferencesLock);
    if (addAtTop) {
        CFArrayInsertValueAtIndex(self->_search, 0, domain);
    } else {
        CFArrayAppendValue(self->_search, domain);
    }
    updateDictRep(self);
    __CFUnlock(&__CFApplicationPreferencesLock);
}

Boolean _CFApplicationPreferencesContainsDomain(_CFApplicationPreferences *self, CFPreferencesDomainRef domain) {
    Boolean result;

    if (!domain) {
        return false;
    }

    __CFLock(&__CFApplicationPreferencesLock);
    result = CFArrayContainsValue(self->_search, CFRangeMake(0, CFArrayGetCount(self->_search)), domain);
    __CFUnlock(&__CFApplicationPreferencesLock);
    return result;
}

Boolean _CFApplicationPreferencesContainsDomainNoLock(_CFApplicationPreferences *self, CFPreferencesDomainRef domain) {
    Boolean result;
    result = CFArrayContainsValue(self->_search, CFRangeMake(0, CFArrayGetCount(self->_search)), domain);
    return result;
}

void _CFApplicationPreferencesRemoveDomain(_CFApplicationPreferences *self, CFPreferencesDomainRef domain) {
    CFIndex idx;
    CFRange range;
    __CFLock(&__CFApplicationPreferencesLock);
    range.location = 0;
    range.length = CFArrayGetCount(self->_search);
    while ((idx = CFArrayGetFirstIndexOfValue(self->_search, range, domain)) != kCFNotFound) {
        CFArrayRemoveValueAtIndex(self->_search, idx);
        range.location = idx;
        range.length  = range.length - idx - 1;
    }
    updateDictRep(self);
    __CFUnlock(&__CFApplicationPreferencesLock);
}
