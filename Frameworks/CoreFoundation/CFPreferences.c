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

/*	CFPreferences.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: David Smith
*/

#include <CoreFoundation/CFPreferences.h>
#include <CoreFoundation/CFURLAccess.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#include <CoreFoundation/CFUserNotification.h>
#endif
#include <CoreFoundation/CFPropertyList.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
#include <CoreFoundation/CFBundle.h>
#endif
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include <sys/stat.h>
#if DEPLOYMENT_TARGET_MACOSX
#include <unistd.h>
#include <CoreFoundation/CFUUID.h>
#endif

#if DEBUG_PREFERENCES_MEMORY
#include "../Tests/CFCountingAllocator.c"
#endif

static CFURLRef _CFPreferencesURLForStandardDomainWithSafetyLevel(CFStringRef domainName, CFStringRef userName, CFStringRef hostName, unsigned long safeLevel);

struct __CFPreferencesDomain {
    CFRuntimeBase _base;
    /* WARNING - not copying the callbacks; we know they are always static structs */
    const _CFPreferencesDomainCallBacks *_callBacks;
    CFTypeRef _context;
    void *_domain;
};

CONST_STRING_DECL(kCFPreferencesAnyApplication, "kCFPreferencesAnyApplication")
CONST_STRING_DECL(kCFPreferencesAnyHost, "kCFPreferencesAnyHost")
CONST_STRING_DECL(kCFPreferencesAnyUser, "kCFPreferencesAnyUser")
CONST_STRING_DECL(kCFPreferencesCurrentApplication, "kCFPreferencesCurrentApplication")
CONST_STRING_DECL(kCFPreferencesCurrentHost, "kCFPreferencesCurrentHost")
CONST_STRING_DECL(kCFPreferencesCurrentUser, "kCFPreferencesCurrentUser")


static CFAllocatorRef _preferencesAllocator = NULL;
CF_PRIVATE CFAllocatorRef __CFPreferencesAllocator(void) {
    if (!_preferencesAllocator) {
#if DEBUG_PREFERENCES_MEMORY
        _preferencesAllocator = CFCountingAllocatorCreate(NULL);
#else
        _preferencesAllocator = __CFGetDefaultAllocator();
        CFRetain(_preferencesAllocator);
#endif
    }
    return _preferencesAllocator;
}

// declaration for telling the 
void _CFApplicationPreferencesDomainHasChanged(CFPreferencesDomainRef);

#if DEBUG_PREFERENCES_MEMORY
#warning Preferences debugging on
CF_EXPORT void CFPreferencesDumpMem(void) {
    if (_preferencesAllocator) {
//        CFCountingAllocatorPrintSummary(_preferencesAllocator);
        CFCountingAllocatorPrintPointers(_preferencesAllocator);
    }
//    CFCountingAllocatorReset(_preferencesAllocator);
}
#endif

#if DEPLOYMENT_TARGET_MACOSX
#pragma mark -
#pragma mark Determining host UUID
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
// The entry point is in libSystem.B.dylib, but not actually declared
// If this becomes available in a header (<rdar://problem/4943036>), I need to pull this out
int gethostuuid(unsigned char *uuid_buf, const struct timespec *timeoutp);

CF_PRIVATE CFStringRef _CFGetHostUUIDString(void) {
    static CFStringRef __hostUUIDString = NULL;
    
    if (!__hostUUIDString) {
        CFUUIDBytes uuidBytes;
        int getuuidErr = 0;
        struct timespec timeout = {0, 0};   // Infinite timeout for gethostuuid()
        
        getuuidErr = gethostuuid((unsigned char *)&uuidBytes, &timeout);
        if (getuuidErr == -1) {
            // An error has occurred trying to get the host UUID string. There's nothing we can do here, so we should just return NULL.
            CFLog(kCFLogLevelWarning, CFSTR("_CFGetHostUUIDString: unable to determine UUID for host. Error: %d"), errno);
            return NULL;
        }
        
        CFUUIDRef uuidRef = CFUUIDCreateFromUUIDBytes(kCFAllocatorSystemDefault, uuidBytes);
        CFStringRef uuidAsString = CFUUIDCreateString(kCFAllocatorSystemDefault, uuidRef);
        
        if (!OSAtomicCompareAndSwapPtrBarrier(NULL, (void *)uuidAsString, (void *)&__hostUUIDString)) {
            CFRelease(uuidAsString);    // someone else made the assignment, so just release the extra string.
        }
        
        CFRelease(uuidRef);
    }
    
    return __hostUUIDString;
}

CF_PRIVATE CFStringRef _CFPreferencesGetByHostIdentifierString(void) {
    static CFStringRef __byHostIdentifierString = NULL;

    if (!__byHostIdentifierString) {
        CFStringRef hostID = _CFGetHostUUIDString();
        if (hostID) {
            if (CFStringHasPrefix(hostID, CFSTR("00000000-0000-1000-8000-"))) {
                // If the host UUID is prefixed by "00000000-0000-1000-8000-" then the UUID returned is the "compatible" type. The last field of the string will be the MAC address of the primary ethernet interface of the computer. We use this for compatibility with existing by-host preferences.
                CFStringRef lastField = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, hostID, CFRangeMake(24, 12));
                CFMutableStringRef tmpstr = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, lastField);
                CFStringLowercase(tmpstr, NULL);
                CFStringRef downcasedField = CFStringCreateCopy(kCFAllocatorSystemDefault, tmpstr);
                
                if (!OSAtomicCompareAndSwapPtrBarrier(NULL, (void *)downcasedField, (void *)&__byHostIdentifierString)) {
                    CFRelease(downcasedField);
                }
                
                CFRelease(tmpstr);
                CFRelease(lastField);
            } else {
                // The host UUID is a full UUID, and we should just use that. This doesn't involve any additional string creation, so we should just be able to do the assignment.
                __byHostIdentifierString = hostID;
            }
        } else {
            __byHostIdentifierString = CFSTR("UnknownHostID");
        }
    }
    
    return __byHostIdentifierString;
}

#else

CF_PRIVATE CFStringRef _CFPreferencesGetByHostIdentifierString(void) {
    return CFSTR("");
}

#endif


static unsigned long __CFSafeLaunchLevel = 0;

#if DEPLOYMENT_TARGET_WINDOWS
#include <shfolder.h>

#endif

static CFURLRef _preferencesDirectoryForUserHostSafetyLevel(CFStringRef userName, CFStringRef hostName, unsigned long safeLevel) {
    CFAllocatorRef alloc = __CFPreferencesAllocator();
#if DEPLOYMENT_TARGET_WINDOWS

	CFURLRef url = NULL;

	CFMutableStringRef completePath = _CFCreateApplicationRepositoryPath(alloc, CSIDL_APPDATA);
 	if (completePath) {
	    // append "Preferences\" and make the CFURL
	    CFStringAppend(completePath, CFSTR("Preferences\\"));
		url = CFURLCreateWithFileSystemPath(alloc, completePath, kCFURLWindowsPathStyle, true);
		CFRelease(completePath);
	}


	// Can't find a better place?  Home directory then?
	if (url == NULL)
		url = CFCopyHomeDirectoryURLForUser((userName == kCFPreferencesCurrentUser) ? NULL : userName);

	return url;
 
#else
    CFURLRef  home = NULL;
    CFURLRef  url;
    int levels = 0;
    //    if (hostName != kCFPreferencesCurrentHost && hostName != kCFPreferencesAnyHost) return NULL; // Arbitrary host access not permitted
    if (userName == kCFPreferencesAnyUser) {
        if (!home) home = CFURLCreateWithFileSystemPath(alloc, CFSTR("/Library/Preferences/"), kCFURLPOSIXPathStyle, true);
        levels = 1;
        if (hostName == kCFPreferencesCurrentHost) url = home;
        else {
            url = CFURLCreateWithFileSystemPathRelativeToBase(alloc, CFSTR("Network/"), kCFURLPOSIXPathStyle, true, home);
            levels ++;
            CFRelease(home);
        }
    } else {
        home = CFCopyHomeDirectoryURLForUser((userName == kCFPreferencesCurrentUser) ? NULL : userName);
        if (home) {
            url = (safeLevel > 0) ? CFURLCreateWithFileSystemPathRelativeToBase(alloc, CFSTR("Library/Safe Preferences/"), kCFURLPOSIXPathStyle, true, home) :
            CFURLCreateWithFileSystemPathRelativeToBase(alloc, CFSTR("Library/Preferences/"), kCFURLPOSIXPathStyle, true, home);
            levels = 2;
            CFRelease(home);
            if (hostName != kCFPreferencesAnyHost) {
                home = url;
                url = CFURLCreateWithFileSystemPathRelativeToBase(alloc, CFSTR("ByHost/"), kCFURLPOSIXPathStyle, true, home);
                levels ++;
                CFRelease(home);
            }
        } else {
            url = NULL;
        }
    }
    return url;
#endif
}

static CFURLRef  _preferencesDirectoryForUserHost(CFStringRef  userName, CFStringRef  hostName) {
    return _preferencesDirectoryForUserHostSafetyLevel(userName, hostName, __CFSafeLaunchLevel);
}

static Boolean __CFPreferencesWritesXML = true;

Boolean __CFPreferencesShouldWriteXML(void) {
    return __CFPreferencesWritesXML;
}

static CFLock_t domainCacheLock = CFLockInit;
static CFMutableDictionaryRef  domainCache = NULL; // mutable

// Public API

CFTypeRef  CFPreferencesCopyValue(CFStringRef  key, CFStringRef  appName, CFStringRef  user, CFStringRef  host) {
    CFPreferencesDomainRef domain;
    CFAssert1(appName != NULL && user != NULL && host != NULL, __kCFLogAssertion, "%s(): Cannot access preferences for a NULL application name, user, or host", __PRETTY_FUNCTION__);
    CFAssert1(key != NULL, __kCFLogAssertion, "%s(): Cannot access preferences with a NULL key", __PRETTY_FUNCTION__);
    
    domain = _CFPreferencesStandardDomain(appName, user, host);
    if (domain) {
        return _CFPreferencesDomainCreateValueForKey(domain, key);
    } else {
        return NULL;
    }
}

CFDictionaryRef CFPreferencesCopyMultiple(CFArrayRef keysToFetch, CFStringRef appName, CFStringRef user, CFStringRef host) {
    CFPreferencesDomainRef domain;
    CFMutableDictionaryRef result;
    CFIndex idx, count;

    CFAssert1(appName != NULL && user != NULL && host != NULL, __kCFLogAssertion, "%s(): Cannot access preferences for a NULL application name, user, or host", __PRETTY_FUNCTION__);
    __CFGenericValidateType(appName, CFStringGetTypeID());
    __CFGenericValidateType(user, CFStringGetTypeID());
    __CFGenericValidateType(host, CFStringGetTypeID());

    domain = _CFPreferencesStandardDomain(appName, user, host);
    if (!domain) return NULL;
    if (!keysToFetch) {
        return _CFPreferencesDomainDeepCopyDictionary(domain);
    } else {
        __CFGenericValidateType(keysToFetch, CFArrayGetTypeID());
        count = CFArrayGetCount(keysToFetch);
        result = CFDictionaryCreateMutable(CFGetAllocator(domain), count, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        if (!result) return NULL;
        for (idx = 0; idx < count; idx ++) {
            CFStringRef key = (CFStringRef)CFArrayGetValueAtIndex(keysToFetch, idx);
            CFPropertyListRef value;
            __CFGenericValidateType(key, CFStringGetTypeID());
            value = _CFPreferencesDomainCreateValueForKey(domain, key);
            if (value) {
                CFDictionarySetValue(result, key, value);
                CFRelease(value);
            }
        }
    }
    return result;
}

void CFPreferencesSetValue(CFStringRef  key, CFTypeRef  value, CFStringRef  appName, CFStringRef  user, CFStringRef  host) {
    CFPreferencesDomainRef domain;
    CFAssert1(appName != NULL && user != NULL && host != NULL, __kCFLogAssertion, "%s(): Cannot access preferences for a NULL application name, user, or host", __PRETTY_FUNCTION__);
    CFAssert1(key != NULL, __kCFLogAssertion, "%s(): Cannot access preferences with a NULL key", __PRETTY_FUNCTION__);

    domain = _CFPreferencesStandardDomain(appName, user, host);
    if (domain) {
        _CFPreferencesDomainSet(domain, key, value);
        _CFApplicationPreferencesDomainHasChanged(domain);
    }
}


void CFPreferencesSetMultiple(CFDictionaryRef keysToSet, CFArrayRef keysToRemove, CFStringRef appName, CFStringRef user, CFStringRef host) {
    CFPreferencesDomainRef domain;
    CFIndex idx, count;
    CFAssert1(appName != NULL && user != NULL && host != NULL, __kCFLogAssertion, "%s(): Cannot access preferences for a NULL application name, user, or host", __PRETTY_FUNCTION__);
    if (keysToSet) __CFGenericValidateType(keysToSet, CFDictionaryGetTypeID());
    if (keysToRemove) __CFGenericValidateType(keysToRemove, CFArrayGetTypeID());
    __CFGenericValidateType(appName, CFStringGetTypeID());
    __CFGenericValidateType(user, CFStringGetTypeID());
    __CFGenericValidateType(host, CFStringGetTypeID());

    CFTypeRef *keys = NULL;
    CFTypeRef *values;
    CFIndex numOfKeysToSet = 0;
    
    domain = _CFPreferencesStandardDomain(appName, user, host);
    if (!domain) return;

    CFAllocatorRef alloc = CFGetAllocator(domain);
    
    if (keysToSet && (count = CFDictionaryGetCount(keysToSet))) {
        numOfKeysToSet = count;
        keys = (CFTypeRef *)CFAllocatorAllocate(alloc, 2*count*sizeof(CFTypeRef), 0);
        if (keys) {
            values = &(keys[count]);
            CFDictionaryGetKeysAndValues(keysToSet, keys, values);
            for (idx = 0; idx < count; idx ++) {
                _CFPreferencesDomainSet(domain, (CFStringRef)keys[idx], values[idx]);
            }
        }
    }
    if (keysToRemove && (count = CFArrayGetCount(keysToRemove))) {
        for (idx = 0; idx < count; idx ++) {
            CFStringRef removedKey = (CFStringRef)CFArrayGetValueAtIndex(keysToRemove, idx);
            _CFPreferencesDomainSet(domain, removedKey, NULL);
        }
    }


    _CFApplicationPreferencesDomainHasChanged(domain);
    
    if(keys) CFAllocatorDeallocate(alloc, keys);
}

Boolean CFPreferencesSynchronize(CFStringRef  appName, CFStringRef  user, CFStringRef  host) {
    CFPreferencesDomainRef domain;
    CFAssert1(appName != NULL && user != NULL && host != NULL, __kCFLogAssertion, "%s(): Cannot access preferences for a NULL application name, user, or host", __PRETTY_FUNCTION__);

    domain = _CFPreferencesStandardDomain(appName, user, host);
    if(domain) _CFApplicationPreferencesDomainHasChanged(domain);
    
    return domain ? _CFPreferencesDomainSynchronize(domain) : false;
}

CFArrayRef  CFPreferencesCopyApplicationList(CFStringRef  user, CFStringRef  host) {
    CFArrayRef  array;
    CFAssert1(user != NULL && host != NULL, __kCFLogAssertion, "%s(): Cannot access preferences for a NULL user or host", __PRETTY_FUNCTION__);
    array = _CFPreferencesCreateDomainList(user, host);
    return array;
}

CFArrayRef  CFPreferencesCopyKeyList(CFStringRef  appName, CFStringRef  user, CFStringRef  host) {
    CFPreferencesDomainRef domain;
    CFAssert1(appName != NULL && user != NULL && host != NULL, __kCFLogAssertion, "%s(): Cannot access preferences for a NULL application name, user, or host", __PRETTY_FUNCTION__);

    domain = _CFPreferencesStandardDomain(appName, user, host);
    if (!domain) {
        return NULL;
    } else {
        CFArrayRef  result;

	CFAllocatorRef alloc = __CFPreferencesAllocator();
	CFDictionaryRef d = _CFPreferencesDomainDeepCopyDictionary(domain);
	CFIndex count = d ? CFDictionaryGetCount(d) : 0;
	CFTypeRef *keys = (CFTypeRef *)CFAllocatorAllocate(alloc, count * sizeof(CFTypeRef), 0);
	if (d) CFDictionaryGetKeysAndValues(d, keys, NULL);
        if (count == 0) {
            result = NULL;
        } else {
            result = CFArrayCreate(alloc, keys, count, &kCFTypeArrayCallBacks);
        }
	CFAllocatorDeallocate(alloc, keys);
	if (d) CFRelease(d);
        return result;
    }
}


/****************************/
/*  CFPreferencesDomain     */
/****************************/

static CFStringRef __CFPreferencesDomainCopyDescription(CFTypeRef cf) {
    return CFStringCreateWithFormat(__CFPreferencesAllocator(), NULL, CFSTR("<Private CFType %p>\n"), cf);
}

static void __CFPreferencesDomainDeallocate(CFTypeRef cf) {
    const struct __CFPreferencesDomain *domain = (struct __CFPreferencesDomain *)cf;
    CFAllocatorRef alloc = __CFPreferencesAllocator();
    domain->_callBacks->freeDomain(alloc, domain->_context, domain->_domain);
    if (domain->_context) CFRelease(domain->_context);
}

static CFTypeID __kCFPreferencesDomainTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFPreferencesDomainClass = {
    0,
    "CFPreferencesDomain",
    NULL,      // init
    NULL,      // copy
    __CFPreferencesDomainDeallocate,
    NULL,
    NULL,
    NULL,      // 
    __CFPreferencesDomainCopyDescription
};

/* We spend a lot of time constructing these prefixes; we should cache.  REW, 7/19/99 */
static CFStringRef  _CFPreferencesCachePrefixForUserHost(CFStringRef  userName, CFStringRef  hostName) {
    if (userName == kCFPreferencesAnyUser && hostName == kCFPreferencesAnyHost) {
        return (CFStringRef)CFRetain(CFSTR("*/*/"));
    }
    CFMutableStringRef result = CFStringCreateMutable(__CFPreferencesAllocator(), 0);
    if (userName == kCFPreferencesCurrentUser) {
        userName = CFCopyUserName();
        CFStringAppend(result, userName);
        CFRelease(userName);
        CFStringAppend(result, CFSTR("/"));
    } else if (userName == kCFPreferencesAnyUser) {
        CFStringAppend(result, CFSTR("*/"));
    }
    if (hostName == kCFPreferencesCurrentHost) {
        CFStringRef hostID = _CFPreferencesGetByHostIdentifierString();
        CFStringAppend(result, hostID);
        CFStringAppend(result, CFSTR("/"));
    } else if (hostName == kCFPreferencesAnyHost) {
        CFStringAppend(result, CFSTR("*/"));
    }
    return result;
}

// It would be nice if we could remember the key for "well-known" combinations, so we're not constantly allocing more strings....  - REW 2/3/99
static CFStringRef  _CFPreferencesStandardDomainCacheKey(CFStringRef  domainName, CFStringRef  userName, CFStringRef  hostName) {
    CFStringRef  prefix = _CFPreferencesCachePrefixForUserHost(userName, hostName);
    CFStringRef  result = NULL;
    
    if (prefix) {
        result = CFStringCreateWithFormat(__CFPreferencesAllocator(), NULL, CFSTR("%@%@"), prefix, domainName);
        CFRelease(prefix);
    }
    return result;
}

static CFURLRef _CFPreferencesURLForStandardDomainWithSafetyLevel(CFStringRef domainName, CFStringRef userName, CFStringRef hostName, unsigned long safeLevel) {
    CFURLRef theURL = NULL;
    CFAllocatorRef prefAlloc = __CFPreferencesAllocator();
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_WINDOWS
    CFURLRef prefDir = _preferencesDirectoryForUserHostSafetyLevel(userName, hostName, safeLevel);
    CFStringRef  appName;
    CFStringRef  fileName;
    Boolean mustFreeAppName = false;
    
    if (!prefDir) return NULL;
    if (domainName == kCFPreferencesAnyApplication) {
        appName = CFSTR(".GlobalPreferences");
    } else if (domainName == kCFPreferencesCurrentApplication) {
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        appName = mainBundle ? CFBundleGetIdentifier(mainBundle) : NULL;
        if (!appName || CFStringGetLength(appName) == 0) {
            appName = _CFProcessNameString();
        }
    } else {
        appName = domainName;
    }
    if (userName != kCFPreferencesAnyUser) {
        if (hostName == kCFPreferencesAnyHost) {
            fileName = CFStringCreateWithFormat(prefAlloc, NULL, CFSTR("%@.plist"), appName);
        } else if (hostName == kCFPreferencesCurrentHost) {
            CFStringRef hostID = _CFPreferencesGetByHostIdentifierString();
            fileName = CFStringCreateWithFormat(prefAlloc, NULL, CFSTR("%@.%@.plist"), appName, hostID);
        } else {
            fileName = CFStringCreateWithFormat(prefAlloc, NULL, CFSTR("%@.%@.plist"), appName, hostName);      // sketchy - this allows someone to set an arbitrary hostname.
        }
    } else {
        fileName = CFStringCreateWithFormat(prefAlloc, NULL, CFSTR("%@.plist"), appName);
    }
    if (mustFreeAppName) {
	CFRelease(appName);
    }
    if (fileName) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
        theURL = CFURLCreateWithFileSystemPathRelativeToBase(prefAlloc, fileName, kCFURLPOSIXPathStyle, false, prefDir);
#elif DEPLOYMENT_TARGET_WINDOWS
		theURL = CFURLCreateWithFileSystemPathRelativeToBase(prefAlloc, fileName, kCFURLWindowsPathStyle, false, prefDir);
#endif
        if (prefDir) CFRelease(prefDir);
        CFRelease(fileName);
    }
#else
//#error Do not know where to store NSUserDefaults on this platform
#endif
    return theURL;
}

static CFURLRef _CFPreferencesURLForStandardDomain(CFStringRef domainName, CFStringRef userName, CFStringRef hostName) {
    return _CFPreferencesURLForStandardDomainWithSafetyLevel(domainName, userName, hostName, __CFSafeLaunchLevel);
}

CFPreferencesDomainRef _CFPreferencesStandardDomain(CFStringRef  domainName, CFStringRef  userName, CFStringRef  hostName) {
    CFPreferencesDomainRef domain;
    CFStringRef  domainKey;
    Boolean shouldReleaseDomain = true;
     domainKey = _CFPreferencesStandardDomainCacheKey(domainName, userName, hostName);
    __CFLock(&domainCacheLock);
    if (!domainCache) {
        CFAllocatorRef alloc = __CFPreferencesAllocator();
        domainCache = CFDictionaryCreateMutable(alloc, 0, & kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    domain = (CFPreferencesDomainRef)CFDictionaryGetValue(domainCache, domainKey);
    __CFUnlock(&domainCacheLock);
    if (!domain) {
        // Domain's not in the cache; load from permanent storage
		CFURLRef  theURL = _CFPreferencesURLForStandardDomain(domainName, userName, hostName);
        if (theURL) {
			domain = _CFPreferencesDomainCreate(theURL, &__kCFXMLPropertyListDomainCallBacks);

            if (userName == kCFPreferencesAnyUser) {
                _CFPreferencesDomainSetIsWorldReadable(domain, true);
            }
            CFRelease(theURL);
        }
	__CFLock(&domainCacheLock);
        if (domain && domainCache) {
            // We've just synthesized a domain & we're about to throw it in the domain cache. The problem is that someone else might have gotten in here behind our backs, so we can't just blindly set the domain (3021920). We'll need to check to see if this happened, and compensate.
            CFPreferencesDomainRef checkDomain = (CFPreferencesDomainRef)CFDictionaryGetValue(domainCache, domainKey);
            if(checkDomain) {
                // Someone got in here ahead of us, so we shouldn't smash the domain we're given. checkDomain is the current version, we should use that.
                // checkDomain was retrieved with a Get, so we don't want to over-release.
                shouldReleaseDomain = false;
                CFRelease(domain);	// release the domain we synthesized earlier.
                domain = checkDomain;	// repoint it at the domain picked up out of the cache.
            } else {
                // We must not have found the domain in the cache, so it's ok for us to put this in.
                CFDictionarySetValue(domainCache, domainKey, domain);                
            }
            if(shouldReleaseDomain) CFRelease(domain);
        }
	__CFUnlock(&domainCacheLock);
    }
    CFRelease(domainKey);
    return domain;
}

static void __CFPreferencesPerformSynchronize(const void *key, const void *value, void *context) {
    CFPreferencesDomainRef domain = (CFPreferencesDomainRef)value;
    Boolean *cumulativeResult = (Boolean *)context;
    if (!_CFPreferencesDomainSynchronize(domain)) *cumulativeResult = false;
}

CF_PRIVATE Boolean _CFSynchronizeDomainCache(void) {
    Boolean result = true;
    __CFLock(&domainCacheLock);
    if (domainCache) {
        CFDictionaryApplyFunction(domainCache, __CFPreferencesPerformSynchronize, &result);
    }
    __CFUnlock(&domainCacheLock);
    return result;
}

CF_PRIVATE void _CFPreferencesPurgeDomainCache(void) {
    _CFSynchronizeDomainCache();
    __CFLock(&domainCacheLock);
    if (domainCache) {
        CFRelease(domainCache);
        domainCache = NULL;
    }
    __CFUnlock(&domainCacheLock);
}

CF_PRIVATE CFArrayRef  _CFPreferencesCreateDomainList(CFStringRef  userName, CFStringRef  hostName) {
    CFAllocatorRef prefAlloc = __CFPreferencesAllocator();
    CFArrayRef  domains;
    CFMutableArrayRef  marray;
    CFStringRef  *cachedDomainKeys;
    CFPreferencesDomainRef *cachedDomains;
    SInt32 idx, cnt;
    CFStringRef  suffix;
    UInt32 suffixLen;
    CFURLRef prefDir = _preferencesDirectoryForUserHost(userName, hostName);
    
    if (!prefDir) {
        return NULL;
    }
    if (hostName == kCFPreferencesAnyHost) {
        suffix = CFStringCreateWithCString(prefAlloc, ".plist", kCFStringEncodingASCII);
    } else if (hostName == kCFPreferencesCurrentHost) {
        CFStringRef hostID = _CFPreferencesGetByHostIdentifierString();
        suffix = CFStringCreateWithFormat(prefAlloc, NULL, CFSTR(".%@.plist"), hostID);
    } else {
        suffix = CFStringCreateWithFormat(prefAlloc, NULL, CFSTR(".%@.plist"), hostName);   // sketchy - this allows someone to create a domain list for an arbitrary hostname.
    }
    suffixLen = CFStringGetLength(suffix);
    
    domains = (CFArrayRef)CFURLCreatePropertyFromResource(prefAlloc, prefDir, kCFURLFileDirectoryContents, NULL);
    CFRelease(prefDir);
    if (domains){
        marray = CFArrayCreateMutableCopy(prefAlloc, 0, domains);
        CFRelease(domains);
    } else {
        marray = CFArrayCreateMutable(prefAlloc, 0, & kCFTypeArrayCallBacks);
    }
    for (idx = CFArrayGetCount(marray)-1; idx >= 0; idx --) {
        CFURLRef  url = (CFURLRef)CFArrayGetValueAtIndex(marray, idx);
        CFStringRef string = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
        if (!CFStringHasSuffix(string, suffix)) {
            CFArrayRemoveValueAtIndex(marray, idx);
        } else {
            CFStringRef  dom = CFStringCreateWithSubstring(prefAlloc, string, CFRangeMake(0, CFStringGetLength(string) - suffixLen));
            if (CFEqual(dom, CFSTR(".GlobalPreferences"))) {
                CFArraySetValueAtIndex(marray, idx, kCFPreferencesAnyApplication);
            } else {
                CFArraySetValueAtIndex(marray, idx, dom);
            }
            CFRelease(dom);
        }
        CFRelease(string);
    }
    CFRelease(suffix);
    
    // Now add any domains added in the cache; delete any that have been deleted in the cache
    __CFLock(&domainCacheLock);
    if (!domainCache) {
        __CFUnlock(&domainCacheLock);
        return marray;
    }
    cnt = CFDictionaryGetCount(domainCache);
    cachedDomainKeys = (CFStringRef *)CFAllocatorAllocate(prefAlloc, 2 * cnt * sizeof(CFStringRef), 0);
    cachedDomains = (CFPreferencesDomainRef *)(cachedDomainKeys + cnt);
    CFDictionaryGetKeysAndValues(domainCache, (const void **)cachedDomainKeys, (const void **)cachedDomains);
    __CFUnlock(&domainCacheLock);
    suffix = _CFPreferencesCachePrefixForUserHost(userName, hostName);
    suffixLen = CFStringGetLength(suffix);
    
    for (idx = 0; idx < cnt; idx ++) {
        CFStringRef  domainKey = cachedDomainKeys[idx];
        CFPreferencesDomainRef domain = cachedDomains[idx];
        CFStringRef  domainName;
        CFIndex keyCount = 0;
        
        if (!CFStringHasPrefix(domainKey, suffix)) continue;
        domainName = CFStringCreateWithSubstring(prefAlloc, domainKey, CFRangeMake(suffixLen, CFStringGetLength(domainKey) - suffixLen));
        if (CFEqual(domainName, CFSTR("*"))) {
            CFRelease(domainName);
            domainName = (CFStringRef)CFRetain(kCFPreferencesAnyApplication);
        } else if (CFEqual(domainName, kCFPreferencesCurrentApplication)) {
            CFRelease(domainName);
            domainName = (CFStringRef)CFRetain(_CFProcessNameString());
        }
        CFDictionaryRef d = _CFPreferencesDomainDeepCopyDictionary(domain);
        keyCount = d ? CFDictionaryGetCount(d) : 0;
        if (keyCount) CFRelease(d);
        if (keyCount == 0) {
            // Domain was deleted
            SInt32 firstIndexOfValue = CFArrayGetFirstIndexOfValue(marray, CFRangeMake(0, CFArrayGetCount(marray)), domainName);
            if (0 <= firstIndexOfValue) {
                CFArrayRemoveValueAtIndex(marray, firstIndexOfValue);
            }
        } else if (!CFArrayContainsValue(marray, CFRangeMake(0, CFArrayGetCount(marray)), domainName)) {
            CFArrayAppendValue(marray, domainName);
        }
        CFRelease(domainName);
    }
    CFRelease(suffix);
    CFAllocatorDeallocate(prefAlloc, cachedDomainKeys);
    return marray;
}

//
// CFPreferencesDomain functions
//

CFPreferencesDomainRef _CFPreferencesDomainCreate(CFTypeRef  context, const _CFPreferencesDomainCallBacks *callBacks) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFPreferencesDomainTypeID = _CFRuntimeRegisterClass(&__CFPreferencesDomainClass); });
    CFAllocatorRef alloc = __CFPreferencesAllocator();
    CFPreferencesDomainRef newDomain;
    CFAssert(callBacks != NULL && callBacks->createDomain != NULL && callBacks->freeDomain != NULL && callBacks->fetchValue != NULL && callBacks->writeValue != NULL, __kCFLogAssertion, "Cannot create a domain with NULL callbacks");
    newDomain = (CFPreferencesDomainRef)_CFRuntimeCreateInstance(alloc, __kCFPreferencesDomainTypeID, sizeof(struct __CFPreferencesDomain) - sizeof(CFRuntimeBase), NULL);
    if (newDomain) {
        newDomain->_callBacks = callBacks;
        if (context) CFRetain(context);
        newDomain->_context = context;
        newDomain->_domain = callBacks->createDomain(alloc, context);
    }
    return newDomain;
}

CFTypeRef  _CFPreferencesDomainCreateValueForKey(CFPreferencesDomainRef domain, CFStringRef key) {
    return domain->_callBacks->fetchValue(domain->_context, domain->_domain, key);
}

void _CFPreferencesDomainSet(CFPreferencesDomainRef domain, CFStringRef  key, CFTypeRef  value) {
    domain->_callBacks->writeValue(domain->_context, domain->_domain, key, value);
}

CF_PRIVATE Boolean _CFPreferencesDomainSynchronize(CFPreferencesDomainRef domain) {
    return domain->_callBacks->synchronize(domain->_context, domain->_domain);
}

CF_PRIVATE void _CFPreferencesDomainSetIsWorldReadable(CFPreferencesDomainRef domain, Boolean isWorldReadable) {
    if (domain->_callBacks->setIsWorldReadable) {
        domain->_callBacks->setIsWorldReadable(domain->_context, domain->_domain, isWorldReadable);
    }
}

CF_PRIVATE void *_CFPreferencesDomainCopyDictFunc(CFPreferencesDomainRef domain) {
    return domain->_callBacks->copyDomainDictionary;
}

void _CFPreferencesDomainSetDictionary(CFPreferencesDomainRef domain, CFDictionaryRef dict) {
    CFAllocatorRef alloc = __CFPreferencesAllocator();
    CFDictionaryRef d = _CFPreferencesDomainDeepCopyDictionary(domain);
    CFIndex idx, count = d ? CFDictionaryGetCount(d) : 0;

    CFTypeRef *keys = (CFTypeRef *)CFAllocatorAllocate(alloc, count * sizeof(CFTypeRef), 0);
    if (d) CFDictionaryGetKeysAndValues(d, keys, NULL);
    for (idx = 0; idx < count; idx ++) {
        _CFPreferencesDomainSet(domain, (CFStringRef)keys[idx], NULL);
    }
    CFAllocatorDeallocate(alloc, keys);
    if (d) CFRelease(d);

    if (dict && (count = CFDictionaryGetCount(dict)) != 0) {
        CFStringRef *newKeys = (CFStringRef *)CFAllocatorAllocate(alloc, count * sizeof(CFStringRef), 0);
        CFDictionaryGetKeysAndValues(dict, (const void **)newKeys, NULL);
        for (idx = 0; idx < count; idx ++) {
            CFStringRef key = newKeys[idx];
            _CFPreferencesDomainSet(domain, key, (CFTypeRef)CFDictionaryGetValue(dict, key));
        }
            CFAllocatorDeallocate(alloc, newKeys);
    }
}

CFDictionaryRef _CFPreferencesDomainDeepCopyDictionary(CFPreferencesDomainRef domain) {
    CFDictionaryRef result = domain->_callBacks->copyDomainDictionary(domain->_context, domain->_domain);
    if(result && CFDictionaryGetCount(result) == 0) {
        CFRelease(result);
        result = NULL;
    }
    return result;
}

Boolean _CFPreferencesDomainExists(CFStringRef domainName, CFStringRef userName, CFStringRef hostName) {
    CFPreferencesDomainRef domain;
    domain = _CFPreferencesStandardDomain(domainName, userName, hostName);
    if (domain) {
		CFDictionaryRef d = _CFPreferencesDomainDeepCopyDictionary(domain);
		if (d) CFRelease(d);
		return d != NULL;
    } else {
        return false;
    }
}

/* Volatile domains - context is ignored; domain is a CFDictionary (mutable) */
static void *createVolatileDomain(CFAllocatorRef allocator, CFTypeRef  context) {
    return CFDictionaryCreateMutable(allocator, 0, & kCFTypeDictionaryKeyCallBacks, & kCFTypeDictionaryValueCallBacks);
}

static void freeVolatileDomain(CFAllocatorRef allocator, CFTypeRef  context, void *domain) {
    CFRelease((CFTypeRef)domain);
}

static CFTypeRef  fetchVolatileValue(CFTypeRef  context, void *domain, CFStringRef  key) {
    CFTypeRef  result = CFDictionaryGetValue((CFMutableDictionaryRef  )domain, key);
    if (result) CFRetain(result);
    return result;
}

static void writeVolatileValue(CFTypeRef  context, void *domain, CFStringRef  key, CFTypeRef  value) {
    if (value)
        CFDictionarySetValue((CFMutableDictionaryRef  )domain, key, value);
    else
        CFDictionaryRemoveValue((CFMutableDictionaryRef  )domain, key);
}

static Boolean synchronizeVolatileDomain(CFTypeRef  context, void *domain) {
    return true;
}

static void getVolatileKeysAndValues(CFAllocatorRef alloc, CFTypeRef context, void *domain, void **buf[], CFIndex *numKeyValuePairs) {
    CFMutableDictionaryRef dict = (CFMutableDictionaryRef)domain;
    CFIndex count = CFDictionaryGetCount(dict);

    if (buf) {
        void **values;
        if ( count < *numKeyValuePairs ) {
            values = *buf + count;
            CFDictionaryGetKeysAndValues(dict, (const void **)*buf, (const void **)values);
        } else if (alloc != kCFAllocatorNull) {
            if (*buf) {
                *buf = (void **)CFAllocatorReallocate(alloc, *buf, count * 2 * sizeof(void *), 0);
            } else {
                *buf = (void **)CFAllocatorAllocate(alloc, count*2*sizeof(void *), 0);
            }
            if (*buf) {
                values = *buf + count;
                CFDictionaryGetKeysAndValues(dict, (const void **)*buf, (const void **)values);
            }
        }
    }
    *numKeyValuePairs = count;
}

static CFDictionaryRef copyVolatileDomainDictionary(CFTypeRef context, void *volatileDomain) {
    CFMutableDictionaryRef dict = (CFMutableDictionaryRef)volatileDomain;
    
    CFDictionaryRef result = (CFDictionaryRef)CFPropertyListCreateDeepCopy(__CFPreferencesAllocator(), dict, kCFPropertyListImmutable);
    return result;
}

const _CFPreferencesDomainCallBacks __kCFVolatileDomainCallBacks = {createVolatileDomain, freeVolatileDomain, fetchVolatileValue, writeVolatileValue, synchronizeVolatileDomain, getVolatileKeysAndValues, copyVolatileDomainDictionary, NULL};
