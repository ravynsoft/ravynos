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

/*	CFError.c
	Copyright (c) 2006-2014, Apple Inc. All rights reserved.
	Responsibility: Ali Ozer
*/

#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFError_Private.h>
#include "CFInternal.h"
#include <CoreFoundation/CFPriv.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#include <mach/mach_error.h>
#endif


/* Pre-defined userInfo keys
*/
CONST_STRING_DECL(kCFErrorLocalizedDescriptionKey,          "NSLocalizedDescription");
CONST_STRING_DECL(kCFErrorLocalizedFailureReasonKey,        "NSLocalizedFailureReason");
CONST_STRING_DECL(kCFErrorLocalizedRecoverySuggestionKey,   "NSLocalizedRecoverySuggestion");
CONST_STRING_DECL(kCFErrorDescriptionKey,                   "NSDescription");
CONST_STRING_DECL(kCFErrorDebugDescriptionKey,              "NSDebugDescription");
CONST_STRING_DECL(kCFErrorUnderlyingErrorKey,               "NSUnderlyingError");
CONST_STRING_DECL(kCFErrorURLKey,               	    "NSURL");
CONST_STRING_DECL(kCFErrorFilePathKey,                      "NSFilePath");

/* Pre-defined error domains
*/
CONST_STRING_DECL(kCFErrorDomainPOSIX,              "NSPOSIXErrorDomain");
CONST_STRING_DECL(kCFErrorDomainOSStatus,           "NSOSStatusErrorDomain");
CONST_STRING_DECL(kCFErrorDomainMach,               "NSMachErrorDomain");
CONST_STRING_DECL(kCFErrorDomainCocoa,              "NSCocoaErrorDomain");

/* We put the localized names of domain names here so genstrings can pick them out.  Any additional domains that are added should be listed here if we'd like them localized.

CFCopyLocalizedStringWithDefaultValue(CFSTR("NSMachErrorDomain"), CFSTR("Error"), NULL, CFSTR("Mach"), "Name of the 'Mach' error domain when showing to user. This probably will not get localized, unless there is a generally recognized phrase for 'Mach' in the language.")
CFCopyLocalizedStringWithDefaultValue(CFSTR("NSCoreFoundationErrorDomain"), CFSTR("Error"), NULL, CFSTR("Core Foundation"), "Name of the 'Core Foundation' error domain when showing to user. Very likely this will not get localized differently in other languages.")
CFCopyLocalizedStringWithDefaultValue(CFSTR("NSPOSIXErrorDomain"), CFSTR("Error"), NULL, CFSTR("POSIX"), "Name of the 'POSIX' error domain when showing to user. This probably will not get localized, unless there is a generally recognized phrase for 'POSIX' in the language.")
CFCopyLocalizedStringWithDefaultValue(CFSTR("NSOSStatusErrorDomain"), CFSTR("Error"), NULL, CFSTR("OSStatus"), "Name of the 'OSStatus' error domain when showing to user. Very likely this will not get localized.")
CFCopyLocalizedStringWithDefaultValue(CFSTR("NSCocoaErrorDomain"), CFSTR("Error"), NULL, CFSTR("Cocoa"), "Name of the 'Cocoa' error domain when showing to user. Very likely this will not get localized.")
*/



/* Forward declarations
*/
static CFDictionaryRef _CFErrorGetUserInfo(CFErrorRef err);
static CFStringRef _CFErrorCopyUserInfoKey(CFErrorRef err, CFStringRef key);
static CFDictionaryRef _CFErrorCreateEmptyDictionary(CFAllocatorRef allocator);

/* Assertions and other macros/inlines
*/
#define __CFAssertIsError(cf) __CFGenericValidateType(cf, CFErrorGetTypeID())

/* This lock is used in the few places in CFError where we create and access shared static objects. Should only be around tiny snippets of code; no recursion
*/
static CFLock_t _CFErrorSpinlock = CFLockInit;




/**** CFError CF runtime stuff ****/

struct __CFError {		// Potentially interesting to keep layout same as NSError (but currently not a requirement)
    CFRuntimeBase _base;
    CFIndex code;
    CFStringRef domain;		// !!! Could compress well-known domains down to few bits, but probably not worth its weight in code since CFErrors are rare
    CFDictionaryRef userInfo;	// !!! Could avoid allocating this slot if userInfo is NULL, but probably not worth its weight in code since CFErrors are rare
};

/* CFError equal checks for equality of domain, code, and userInfo. 
*/
static Boolean __CFErrorEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFErrorRef err1 = (CFErrorRef)cf1;
    CFErrorRef err2 = (CFErrorRef)cf2;
    
    // First do quick checks of code and domain (in that order for performance)
    if (CFErrorGetCode(err1) != CFErrorGetCode(err2)) return false;
    if (!CFEqual(CFErrorGetDomain(err1), CFErrorGetDomain(err2))) return false;

    // If those are equal, then check the dictionaries
    CFDictionaryRef dict1 = CFErrorCopyUserInfo(err1);
    CFDictionaryRef dict2 = CFErrorCopyUserInfo(err2);

    Boolean result = false;
    
    if (dict1 == dict2) {
        result = true;
    } else if (dict1 && dict2 && CFEqual(dict1, dict2)) {
        result = true;
    }
    
    if (dict1) CFRelease(dict1);
    if (dict2) CFRelease(dict2);
    
    return result;
}

/* CFError hash code is hash(domain) + code
*/
static CFHashCode __CFErrorHash(CFTypeRef cf) {
    CFErrorRef err = (CFErrorRef)cf;
    /* !!! We do not need an assertion here, as this is called by the CFBase runtime only */
    return CFHash(err->domain) + err->code;
}

/* This is the full debug description. Shows the description (possibly localized), plus the domain, code, and userInfo explicitly. If there is a debug description, shows that as well. 
*/
static CFStringRef __CFErrorCopyDescription(CFTypeRef cf) {
    return _CFErrorCreateDebugDescription((CFErrorRef)cf);
}

/* This is the description you get for %@; we tone it down a bit from what you get in __CFErrorCopyDescription().
*/
static CFStringRef __CFErrorCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions) {
    CFErrorRef err = (CFErrorRef)cf;
    return CFErrorCopyDescription(err);     // No need to release, since we are returning from a Copy function
}

static void __CFErrorDeallocate(CFTypeRef cf) {
    CFErrorRef err = (CFErrorRef)cf;
    CFRelease(err->domain);
    CFRelease(err->userInfo);
}


static CFTypeID __kCFErrorTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFErrorClass = {
    0,
    "CFError",
    NULL,      // init
    NULL,      // copy
    __CFErrorDeallocate,
    __CFErrorEqual,
    __CFErrorHash,
    __CFErrorCopyFormattingDescription,
    __CFErrorCopyDescription
};

CFTypeID CFErrorGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFErrorTypeID = _CFRuntimeRegisterClass(&__CFErrorClass); });
    return __kCFErrorTypeID;
}




/**** CFError support functions ****/

/* Returns a shared empty dictionary (unless the allocator is not kCFAllocatorSystemDefault, in which case returns a newly allocated one).
*/
static CFDictionaryRef _CFErrorCreateEmptyDictionary(CFAllocatorRef allocator) {
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    if (_CFAllocatorIsSystemDefault(allocator)) {
        static CFDictionaryRef emptyErrorDictionary = NULL;
        if (emptyErrorDictionary == NULL) {
            CFDictionaryRef tmp = CFDictionaryCreate(allocator, NULL, NULL, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            __CFLock(&_CFErrorSpinlock);
            if (emptyErrorDictionary == NULL) {
                emptyErrorDictionary = tmp;
                __CFUnlock(&_CFErrorSpinlock);
            } else {
                __CFUnlock(&_CFErrorSpinlock);
                CFRelease(tmp);
            }
        }
        return (CFDictionaryRef)CFRetain(emptyErrorDictionary);
    } else {
        return CFDictionaryCreate(allocator, NULL, NULL, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
}

/* A non-retained accessor for the userInfo. Might return NULL in some cases, if the subclass of NSError returned nil for some reason. It works with a CF or NSError.
*/
static CFDictionaryRef _CFErrorGetUserInfo(CFErrorRef err) {
    CF_OBJC_FUNCDISPATCHV(CFErrorGetTypeID(), CFDictionaryRef, (NSError *)err, userInfo);
    __CFAssertIsError(err);
    return err->userInfo;
}

/* This function retrieves the value of the specified key from the userInfo, or from the callback. It works with a CF or NSError.
*/
static CFStringRef _CFErrorCopyUserInfoKey(CFErrorRef err, CFStringRef key) {
    CFStringRef result = NULL;
    // First consult the userInfo dictionary   
    CFDictionaryRef userInfo = _CFErrorGetUserInfo(err);
    if (userInfo) result = (CFStringRef)CFDictionaryGetValue(userInfo, key);
    // If that doesn't work, consult the callback
    if (result) {
        CFRetain(result);
    } else {
        CFErrorUserInfoKeyCallBack callBack = CFErrorGetCallBackForDomain(CFErrorGetDomain(err));
        if (callBack) result = (CFStringRef)callBack(err, key);
    }
    return result;
}

/* The real guts of the description creation functionality. See the header file for the steps this function goes through to compute the description. This function can take a CF or NSError. It's called by NSError for the localizedDescription computation.
*/
CFStringRef _CFErrorCreateLocalizedDescription(CFErrorRef err) {
    // First look for kCFErrorLocalizedDescriptionKey; if non-NULL, return that as-is.
    CFStringRef localizedDesc = _CFErrorCopyUserInfoKey(err, kCFErrorLocalizedDescriptionKey);
    if (localizedDesc) return localizedDesc;


#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
    // Cache the CF bundle since we will be using it for localized strings.
    CFBundleRef cfBundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CoreFoundation"));
    
    if (!cfBundle) {	// This should be rare, but has been observed in the wild, due to running out of file descriptors. Normally we might not go to such extremes, but since we want to be able to present reasonable errors even in the case of errors such as running out of file descriptors, why not. This is CFError after all. !!! Be sure to have the same logic here as below for going through various options for fetching the strings.
#endif
    
	CFStringRef result = NULL, reasonOrDesc;

	if ((reasonOrDesc = _CFErrorCopyUserInfoKey(err, kCFErrorLocalizedFailureReasonKey))) {	    // First look for kCFErrorLocalizedFailureReasonKey
	    result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("The operation couldn\\U2019t be completed. %@"), reasonOrDesc);
	} else if ((reasonOrDesc = _CFErrorCopyUserInfoKey(err, kCFErrorDescriptionKey))) {	    // Then try kCFErrorDescriptionKey
	    result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("The operation couldn\\U2019t be completed. (%@ error %ld - %@)"), CFErrorGetDomain(err), (long)CFErrorGetCode(err), reasonOrDesc);
	} else {	// Last resort, just the domain and code
	    result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("The operation couldn\\U2019t be completed. (%@ error %ld.)"), CFErrorGetDomain(err), (long)CFErrorGetCode(err));
	}
	if (reasonOrDesc) CFRelease(reasonOrDesc);
	return result;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
    }
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
    // Then look for kCFErrorLocalizedFailureReasonKey; if there, create a full sentence from that.
    CFStringRef reason = _CFErrorCopyUserInfoKey(err, kCFErrorLocalizedFailureReasonKey);
    if (reason) {
	CFStringRef operationFailedStr = CFCopyLocalizedStringFromTableInBundle(CFSTR("The operation couldn\\U2019t be completed. %@"), CFSTR("Error"), cfBundle, "A generic error string indicating there was a problem. The %@ will be replaced by a second sentence which indicates why the operation failed.");
        CFStringRef result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, operationFailedStr, reason);
	CFRelease(operationFailedStr);
        CFRelease(reason);
	return result;
    }

    // Otherwise, generate a semi-user presentable string from the domain, code, and if available, the presumably non-localized kCFErrorDescriptionKey.
    CFStringRef result;
    CFStringRef desc = _CFErrorCopyUserInfoKey(err, kCFErrorDescriptionKey);
    CFStringRef localizedDomain = CFCopyLocalizedStringFromTableInBundle(CFErrorGetDomain(err), CFSTR("Error"), cfBundle, "These are localized in the comment above");
    if (desc) {     // We have kCFErrorDescriptionKey, so include that with the error domain and code
	CFStringRef operationFailedStr = CFCopyLocalizedStringFromTableInBundle(CFSTR("The operation couldn\\U2019t be completed. (%@ error %ld - %@)"), CFSTR("Error"), cfBundle, "A generic error string indicating there was a problem, followed by a parenthetical sentence which indicates error domain, code, and a description when there is no other way to present an error to the user. The first %@ indicates the error domain, %ld indicates the error code, and the second %@ indicates the description; so this might become '(Mach error 42 - Server error.)' for instance.");
	result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, operationFailedStr, localizedDomain, (long)CFErrorGetCode(err), desc);
	CFRelease(operationFailedStr);
        CFRelease(desc);
    } else {        // We don't have kCFErrorDescriptionKey, so just use error domain and code
	CFStringRef operationFailedStr = CFCopyLocalizedStringFromTableInBundle(CFSTR("The operation couldn\\U2019t be completed. (%@ error %ld.)"), CFSTR("Error"), cfBundle, "A generic error string indicating there was a problem, followed by a parenthetical sentence which indicates error domain and code when there is no other way to present an error to the user. The %@ indicates the error domain while %ld indicates the error code; so this might become '(Mach error 42.)' for instance.");
	result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, operationFailedStr, localizedDomain, (long)CFErrorGetCode(err));
	CFRelease(operationFailedStr);
    }
    CFRelease(localizedDomain);
    return result;
#endif
}

/* The real guts of the failure reason creation functionality. This function can take a CF or NSError. It's called by NSError for the localizedFailureReason computation.
*/
CFStringRef _CFErrorCreateLocalizedFailureReason(CFErrorRef err) {
    // We simply return the value of kCFErrorLocalizedFailureReasonKey; no other searching takes place
    return _CFErrorCopyUserInfoKey(err, kCFErrorLocalizedFailureReasonKey);
}

/* The real guts of the recovery suggestion functionality. This function can take a CF or NSError. It's called by NSError for the localizedRecoverySuggestion computation.
*/
CFStringRef _CFErrorCreateLocalizedRecoverySuggestion(CFErrorRef err) {
    // We simply return the value of kCFErrorLocalizedRecoverySuggestionKey; no other searching takes place
    return _CFErrorCopyUserInfoKey(err, kCFErrorLocalizedRecoverySuggestionKey);
}

/* The "debug" description, used by CFCopyDescription and -[NSObject description].
*/
static void userInfoKeyValueShow(const void *key, const void *value, void *context) {
    CFStringRef desc;
    if (CFEqual(key, kCFErrorUnderlyingErrorKey) && (desc = CFErrorCopyDescription((CFErrorRef)value))) {	// We check desc, see <rdar://problem/8415727>
	CFStringAppendFormat((CFMutableStringRef)context, NULL, CFSTR("%@=%p \"%@\", "), key, value, desc); 
	CFRelease(desc);
    } else {
	CFStringAppendFormat((CFMutableStringRef)context, NULL, CFSTR("%@=%@, "), key, value); 
    }
}

CFStringRef _CFErrorCreateDebugDescription(CFErrorRef err) {
    CFStringRef desc = CFErrorCopyDescription(err);
    CFStringRef debugDesc = _CFErrorCopyUserInfoKey(err, kCFErrorDebugDescriptionKey);
    CFDictionaryRef userInfo = _CFErrorGetUserInfo(err);
    CFMutableStringRef result = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFStringAppendFormat(result, NULL, CFSTR("Error Domain=%@ Code=%ld"), CFErrorGetDomain(err), (long)CFErrorGetCode(err));
    CFStringAppendFormat(result, NULL, CFSTR(" \"%@\""), desc);
    if (debugDesc && CFStringGetLength(debugDesc) > 0) CFStringAppendFormat(result, NULL, CFSTR(" (%@)"), debugDesc);
    if (userInfo && CFDictionaryGetCount(userInfo)) {
        CFStringAppendFormat(result, NULL, CFSTR(" UserInfo=%p {"), userInfo);
	CFDictionaryApplyFunction(userInfo, userInfoKeyValueShow, (void *)result);
	CFIndex commaLength = (CFStringHasSuffix(result, CFSTR(", "))) ? 2 : 0;
	CFStringReplace(result, CFRangeMake(CFStringGetLength(result)-commaLength, commaLength), CFSTR("}"));
    }
    if (debugDesc) CFRelease(debugDesc);
    if (desc) CFRelease(desc);
    return result;
}




/**** CFError API/SPI ****/

/* Note that there are two entry points for creating CFErrors. This one does it with a presupplied userInfo dictionary.
*/
CFErrorRef CFErrorCreate(CFAllocatorRef allocator, CFStringRef domain, CFIndex code, CFDictionaryRef userInfo) {
    __CFGenericValidateType(domain, CFStringGetTypeID());
    if (userInfo) __CFGenericValidateType(userInfo, CFDictionaryGetTypeID());

    CFErrorRef err = (CFErrorRef)_CFRuntimeCreateInstance(allocator, CFErrorGetTypeID(), sizeof(struct __CFError) - sizeof(CFRuntimeBase), NULL);
    if (NULL == err) return NULL;

    ((struct __CFError *)err)->domain = CFStringCreateCopy(allocator, domain);
    ((struct __CFError *)err)->code = code;
    ((struct __CFError *)err)->userInfo = userInfo ? CFDictionaryCreateCopy(allocator, userInfo) : _CFErrorCreateEmptyDictionary(allocator);

    return err;
}

/* Note that there are two entry points for creating CFErrors. This one does it with individual keys and values which are used to create the userInfo dictionary.
*/
CFErrorRef CFErrorCreateWithUserInfoKeysAndValues(CFAllocatorRef allocator, CFStringRef domain, CFIndex code, const void *const *userInfoKeys, const void *const *userInfoValues, CFIndex numUserInfoValues) {
    __CFGenericValidateType(domain, CFStringGetTypeID());

    CFErrorRef err = (CFErrorRef)_CFRuntimeCreateInstance(allocator, CFErrorGetTypeID(), sizeof(struct __CFError) - sizeof(CFRuntimeBase), NULL);
    if (NULL == err) return NULL;

    ((struct __CFError *)err)->domain = CFStringCreateCopy(allocator, domain);
    ((struct __CFError *)err)->code = code;
    ((struct __CFError *)err)->userInfo = CFDictionaryCreate(allocator, (const void **)userInfoKeys, (const void **)userInfoValues, numUserInfoValues, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    return err;
}

CFStringRef CFErrorGetDomain(CFErrorRef err) {
    CF_OBJC_FUNCDISPATCHV(CFErrorGetTypeID(), CFStringRef, (NSError *)err, domain);
    __CFAssertIsError(err);
    return err->domain;
}

CFIndex CFErrorGetCode(CFErrorRef err) {
    CF_OBJC_FUNCDISPATCHV(CFErrorGetTypeID(), CFIndex, (NSError *)err, code);
    __CFAssertIsError(err);
    return err->code;
}

/* This accessor never returns NULL. For usage inside this file, consider __CFErrorGetUserInfo().
*/
CFDictionaryRef CFErrorCopyUserInfo(CFErrorRef err) {
    CFDictionaryRef userInfo = _CFErrorGetUserInfo(err);
    return userInfo ? (CFDictionaryRef)CFRetain(userInfo) : _CFErrorCreateEmptyDictionary(CFGetAllocator(err));
}

CFStringRef CFErrorCopyDescription(CFErrorRef err) {
    if (CF_IS_OBJC(CFErrorGetTypeID(), err)) {  // Since we have to return a retained result, we need to treat the toll-free bridging specially
        CFStringRef desc = (CFStringRef) CF_OBJC_CALLV((NSError *)err, localizedDescription);
        return desc ? (CFStringRef)CFRetain(desc) : NULL;    // !!! It really should never return nil.
    }
    __CFAssertIsError(err);
    return _CFErrorCreateLocalizedDescription(err);
}

CFStringRef CFErrorCopyFailureReason(CFErrorRef err) {
    if (CF_IS_OBJC(CFErrorGetTypeID(), err)) {  // Since we have to return a retained result, we need to treat the toll-free bridging specially
        CFStringRef str = (CFStringRef) CF_OBJC_CALLV((NSError *)err, localizedFailureReason);
        return str ? (CFStringRef)CFRetain(str) : NULL;    // It's possible for localizedFailureReason to return nil
    }
    __CFAssertIsError(err);
    return _CFErrorCreateLocalizedFailureReason(err);
}

CFStringRef CFErrorCopyRecoverySuggestion(CFErrorRef err) {
    if (CF_IS_OBJC(CFErrorGetTypeID(), err)) {  // Since we have to return a retained result, we need to treat the toll-free bridging specially
        CFStringRef str = (CFStringRef) CF_OBJC_CALLV((NSError *)err, localizedRecoverySuggestion);
        return str ? (CFStringRef)CFRetain(str) : NULL;    // It's possible for localizedRecoverySuggestion to return nil
    }
    __CFAssertIsError(err);
    return _CFErrorCreateLocalizedRecoverySuggestion(err);
}


/**** CFError CallBack management ****/

/* Domain-to-callback mapping dictionary
*/
static CFMutableDictionaryRef _CFErrorCallBackTable = NULL;


/* Built-in callback for POSIX domain. Note that we will pick up localizations from ErrnoErrors.strings in /System/Library/CoreServices/CoreTypes.bundle, if the file happens to be there.
*/
static CFTypeRef _CFErrorPOSIXCallBack(CFErrorRef err, CFStringRef key) {
    if (!CFEqual(key, kCFErrorDescriptionKey) && !CFEqual(key, kCFErrorLocalizedFailureReasonKey)) return NULL;
    
    const char *errCStr = strerror(CFErrorGetCode(err));
    CFStringRef errStr = (errCStr && strlen(errCStr)) ? CFStringCreateWithCString(kCFAllocatorSystemDefault, errCStr, kCFStringEncodingUTF8) : NULL;
    
    if (!errStr) return NULL;
    if (CFEqual(key, kCFErrorDescriptionKey)) return errStr;	// If all we wanted was the non-localized description, we're done
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
    // We need a kCFErrorLocalizedFailureReasonKey, so look up a possible localization for the error message
    // Look for the bundle in /System/Library/CoreServices/CoreTypes.bundle
    CFArrayRef paths = CFCopySearchPathForDirectoriesInDomains(kCFLibraryDirectory, kCFSystemDomainMask, false);
    if (paths) {
	if (CFArrayGetCount(paths) > 0) {
            CFStringRef fileSystemPath = CFURLCopyFileSystemPath((CFURLRef)CFArrayGetValueAtIndex(paths, 0), kCFURLPOSIXPathStyle);
            if (fileSystemPath) {
                CFStringRef path = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@/CoreServices/CoreTypes.bundle"), fileSystemPath);
                CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, path, kCFURLPOSIXPathStyle, false /* not a directory */);
                CFRelease(fileSystemPath);
                if (url) {
                    CFBundleRef bundle = CFBundleCreate(kCFAllocatorSystemDefault, url);
                    if (bundle) {
                        // We only want to return a result if there was a localization
                        CFStringRef localizedErrStr = CFBundleCopyLocalizedString(bundle, errStr, errStr, CFSTR("ErrnoErrors"));
                        if (localizedErrStr == errStr) {
                            CFRelease(localizedErrStr);
                            CFRelease(errStr);
                            errStr = NULL;
                        } else {
                            CFRelease(errStr);
                            errStr = localizedErrStr;
                        }
                        CFRelease(bundle);
                    }
                    CFRelease(url);
                }
                CFRelease(path);
            }
	}
	CFRelease(paths);
    }
#endif
    
    return errStr;
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
/* Built-in callback for Mach domain.
*/
static CFTypeRef _CFErrorMachCallBack(CFErrorRef err, CFStringRef key) {
    if (CFEqual(key, kCFErrorDescriptionKey)) {
        const char *errStr = mach_error_string(CFErrorGetCode(err));
        if (errStr && strlen(errStr)) return CFStringCreateWithCString(kCFAllocatorSystemDefault, errStr, kCFStringEncodingUTF8);
    }
    return NULL;
}
#endif


/* This initialize function is meant to be called lazily, the first time a callback is registered or requested. It creates the table and registers the built-in callbacks. Clearly doing this non-lazily in _CFErrorInitialize() would be simpler, but this is a fine example of something that should not have to happen at launch time.
*/
static void _CFErrorInitializeCallBackTable(void) {
    // Create the table outside the lock
    CFMutableDictionaryRef table = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, NULL);
    __CFLock(&_CFErrorSpinlock);
    if (!_CFErrorCallBackTable) {
        _CFErrorCallBackTable = table;
        __CFUnlock(&_CFErrorSpinlock);
    } else {
        __CFUnlock(&_CFErrorSpinlock);
        CFRelease(table);
        // Note, even though the table looks like it was initialized, we go on to register the items on this thread as well, since otherwise we might consult the table before the items are actually registered.
    }
    CFErrorSetCallBackForDomain(kCFErrorDomainPOSIX, _CFErrorPOSIXCallBack);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED    
    CFErrorSetCallBackForDomain(kCFErrorDomainMach, _CFErrorMachCallBack);
#endif
}

void CFErrorSetCallBackForDomain(CFStringRef domainName, CFErrorUserInfoKeyCallBack callBack) {
    if (!_CFErrorCallBackTable) _CFErrorInitializeCallBackTable();
    __CFLock(&_CFErrorSpinlock);
    if (callBack) {
        CFDictionarySetValue(_CFErrorCallBackTable, domainName, (void *)callBack);
    } else {
        CFDictionaryRemoveValue(_CFErrorCallBackTable, domainName);
    }
    __CFUnlock(&_CFErrorSpinlock);
}

CFErrorUserInfoKeyCallBack CFErrorGetCallBackForDomain(CFStringRef domainName) {
    if (!_CFErrorCallBackTable) _CFErrorInitializeCallBackTable();
    __CFLock(&_CFErrorSpinlock);
    CFErrorUserInfoKeyCallBack callBack = (CFErrorUserInfoKeyCallBack)CFDictionaryGetValue(_CFErrorCallBackTable, domainName);
    __CFUnlock(&_CFErrorSpinlock);
    return callBack;
}



