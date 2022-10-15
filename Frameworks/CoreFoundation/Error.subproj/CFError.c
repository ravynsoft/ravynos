/*	CFError.c
	Copyright (c) 2006-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Ali Ozer
*/

#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFError_Private.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/ForFoundationOnly.h>
#if TARGET_OS_MAC
#include <mach/mach_error.h>
#endif




#if DEPLOYMENT_RUNTIME_SWIFT
// We don't have the right memory management semantics to handle storing Swift blocks right now
#define USES_CALLBACK_BLOCKS 0
#else
#define USES_CALLBACK_BLOCKS 1
#endif

/* Pre-defined userInfo keys
*/
CONST_STRING_DECL(kCFErrorLocalizedDescriptionKey,          "NSLocalizedDescription");
CONST_STRING_DECL(kCFErrorLocalizedFailureKey,              "NSLocalizedFailure");
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
static os_unfair_lock _CFErrorLock = OS_UNFAIR_LOCK_INIT;




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

/* This is the description you get for %@. Note that this used to return the shorter description prior to 10.11/iOS9; moving forward we just generate the same result.  In general this function is rarely invoked, so shouldn't matter much.
*/
static CFStringRef __CFErrorCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions) {
    if (
        true
        ) {
        return _CFErrorCreateDebugDescription((CFErrorRef)cf);
    } else {
        return CFErrorCopyDescription((CFErrorRef)cf);
    }
}

static void __CFErrorDeallocate(CFTypeRef cf) {
    CFErrorRef err = (CFErrorRef)cf;
    CFRelease(err->domain);
    CFRelease(err->userInfo);
}

const CFRuntimeClass __CFErrorClass = {
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
    return _kCFRuntimeIDCFError;
}




/**** CFError support functions ****/

/* Returns a shared empty dictionary (unless the allocator is not kCFAllocatorSystemDefault, in which case returns a newly allocated one).
*/
static CFDictionaryRef _CFErrorCreateEmptyDictionary(CFAllocatorRef allocator) {
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
#if DEPLOYMENT_TARGET_OBJC
    if (_CFAllocatorIsSystemDefault(allocator)) {
        return (CFDictionaryRef)CFRetain(__NSDictionary0__);
    } else {
        return CFDictionaryCreate(allocator, NULL, NULL, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
#endif
    
    return CFDictionaryCreate(allocator, NULL, NULL, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
}

/* A non-retained accessor for the userInfo. Might return NULL in some cases, if the subclass of NSError returned nil for some reason. It works with a CF or NSError.
*/
static CFDictionaryRef _CFErrorGetUserInfo(CFErrorRef err) {
    CF_OBJC_FUNCDISPATCHV(CFErrorGetTypeID(), CFDictionaryRef, (NSError *)err, userInfo);
    __CFAssertIsError(err);
    return err->userInfo;
}

/* Retrieve the value for the error key from the userInfo dictionary only
 */
static CFStringRef _CFErrorCopyUserInfoKeyFromUserInfo(CFErrorRef err, CFStringRef key) {
    CFStringRef result = NULL;
    CFDictionaryRef userInfo = _CFErrorGetUserInfo(err);
    if (userInfo) {
        result = (CFStringRef)CFDictionaryGetValue(userInfo, key);
        if (result) CFRetain(result);
    }
    return result;
}

/* Retrieve the value for the error key from the callback only
 */
static CFStringRef _CFErrorCopyUserInfoKeyFromCallBack(CFErrorRef err, CFStringRef key) {
    CFStringRef result = NULL;
#if USES_CALLBACK_BLOCKS
    CFErrorUserInfoKeyCallBackBlock callBackBlock = CFErrorCopyCallBackBlockForDomain(CFErrorGetDomain(err));
    if (callBackBlock) {
        result = (CFStringRef)callBackBlock(err, key);
        if (result) CFRetain(result);
        CFRelease(callBackBlock);
    }
#endif
    return result;
}

/* This function retrieves the value of the specified key from the userInfo, or from the callback. It works with a CF or NSError.
*/
static CFStringRef _CFErrorCopyUserInfoKey(CFErrorRef err, CFStringRef key) {
    CFStringRef result = _CFErrorCopyUserInfoKeyFromUserInfo(err, key);
    if (!result) result = _CFErrorCopyUserInfoKeyFromCallBack(err, key);
    return result;
}


/* The real guts of the localized description creation functionality. See the header file comments for -localizedDescription for the steps this function goes through to compute the description. This function can take a CF or NSError. It's called by NSError for the localizedDescription computation.
*/
CFStringRef _CFErrorCreateLocalizedDescription(CFErrorRef err) {
    // We first do two passes for kCFErrorLocalizedDescriptionKey and kCFErrorLocalizedFailureKey, first looking in userInfo, then callback.
    // This two-pass approach allows an entry in the userInfo for either one to override the (presumably more generic) entry from the callback.
    for (int pass = 0; pass < 2; pass++) {
        CFStringRef (*keyLookupFunc)(CFErrorRef, CFStringRef) = ((pass == 0) ? _CFErrorCopyUserInfoKeyFromUserInfo : _CFErrorCopyUserInfoKeyFromCallBack);
        
        // First look for kCFErrorLocalizedDescriptionKey; if non-NULL, return that as-is
        CFStringRef localizedDesc = (keyLookupFunc)(err, kCFErrorLocalizedDescriptionKey);
        if (localizedDesc) return localizedDesc;
    
        // If a we have kCFErrorLocalizedFailureKey, use it, in conjunction with kCFErrorLocalizedFailureReasonKey if we have that, otherwise by itself
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"
        
        CFStringRef failure = (keyLookupFunc)(err, kCFErrorLocalizedFailureKey);
        
#pragma clang diagnostic pop
        
        if (failure) {
            CFStringRef reason = _CFErrorCopyUserInfoKey(err, kCFErrorLocalizedFailureReasonKey);    // This can come from userInfo or callback
            if (reason) {
                CFStringRef const backstopComboString = CFSTR("%@ %@");
                CFStringRef comboString = NULL;
#if TARGET_OS_MAC || TARGET_OS_WIN32
                CFBundleRef cfBundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CoreFoundation"));
                if (cfBundle) comboString = CFCopyLocalizedStringFromTableInBundle(CFSTR("%@ %@"), CFSTR("Error"), cfBundle, "Used for presenting the 'what failed' and 'why it failed' sections of an error message, where each one is one or more complete sentences. The first %@ corresponds to the 'what failed' (For instance 'The file could not be saved.') and the second one 'why it failed' (For instance 'The volume is out of space.')");
#endif
                CFStringRef result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, comboString ?: backstopComboString, failure, reason);
                if (comboString) CFRelease(comboString);
                CFRelease(failure);
                CFRelease(reason);
                return result;
            } else {
                return failure;
            }
        }
    }
    
#if TARGET_OS_MAC || TARGET_OS_WIN32
    // Cache the CF bundle since we will be using it for localized strings. Note that for platforms without bundle support we also go this non-localized route.
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
#if TARGET_OS_MAC || TARGET_OS_WIN32
    }
    
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


/* _CFErrorCreateDebugDescription provides the "debug" description, used by CFCopyDescription, -[NSObject description], and %@ formatting. _CFErrorFormatDebugDescriptionAux() and userInfoKeyValueShow() are helpers.
*/
typedef struct {
    CFMutableStringRef result;
    CFIndex depth;
} errorFormattingContext;

static void _CFErrorFormatDebugDescriptionAux(CFErrorRef err, errorFormattingContext *context);

static void userInfoKeyValueShow(const void *key, const void *value, void *ctxt) {
    errorFormattingContext *context = ctxt;
    if (context == NULL) {
        HALT_MSG("*** userInfoKeyValueShow() called with NULL context ***");
    }
    if (CFEqual(key, kCFErrorUnderlyingErrorKey) && value != NULL && CFGetTypeID(value) == CFErrorGetTypeID()) {
        CFStringAppendFormat(context->result, NULL, CFSTR("%@=%p {"), key, value);
        _CFErrorFormatDebugDescriptionAux((CFErrorRef)value, context);
        CFStringAppend(context->result, CFSTR("}, "));
    }
    else {
	CFStringAppendFormat(context->result, NULL, CFSTR("%@=%@, "), key, value);
    }
}

static void _CFErrorFormatDebugDescriptionAux(CFErrorRef err, errorFormattingContext *context) {
    // Attempt to do cheaper lookups before resorting to localized
    CFStringRef debugDesc = _CFErrorCopyUserInfoKey(err, kCFErrorDebugDescriptionKey);
    CFStringRef desc = _CFErrorCopyUserInfoKey(err, kCFErrorDescriptionKey);
    if (!desc) {
        desc = debugDesc;
        debugDesc = NULL;   // Since we don't want to show the same string twice; this transfers ownership from debugDesc to desc
        if (!desc) {
            desc = _CFErrorCopyUserInfoKey(err, kCFErrorLocalizedDescriptionKey);
            if (!desc) {
                desc = _CFErrorCopyUserInfoKey(err, kCFErrorLocalizedFailureReasonKey);
            }
        }
    }
    
    CFDictionaryRef userInfo = _CFErrorGetUserInfo(err);
    CFStringAppendFormat(context->result, NULL, CFSTR("Error Domain=%@ Code=%ld"), CFErrorGetDomain(err), (long)CFErrorGetCode(err));
    CFStringAppendFormat(context->result, NULL, CFSTR(" \"%@\""), desc);
    if (desc && debugDesc && (CFStringGetLength(debugDesc) > 0) && !CFEqual(desc, debugDesc)) CFStringAppendFormat(context->result, NULL, CFSTR(" (%@)"), debugDesc);
    if (userInfo && CFDictionaryGetCount(userInfo)) {
        if (context->depth < 3) {   // We only show underlying errors 3 deep by default
            context->depth++;
            CFStringAppend(context->result, CFSTR(" UserInfo={"));
            CFDictionaryApplyFunction(userInfo, userInfoKeyValueShow, context);
            CFIndex commaLength = (CFStringHasSuffix(context->result, CFSTR(", "))) ? 2 : 0;
            CFStringReplace(context->result, CFRangeMake(CFStringGetLength(context->result)-commaLength, commaLength), CFSTR("}"));
            context->depth--;
        } else {
            CFStringAppendFormat(context->result, NULL, CFSTR(" UserInfo=%p (not displayed)"), userInfo);
        }
    }
    if (debugDesc) CFRelease(debugDesc);
    if (desc) CFRelease(desc);
}

CFStringRef _CFErrorCreateDebugDescription(CFErrorRef err) {
    errorFormattingContext context;
    context.result = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    context.depth = 0;
    _CFErrorFormatDebugDescriptionAux(err, &context);
    return context.result;
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
static CFTypeRef _CFErrorPOSIXCallBack(CFErrorRef err, CFStringRef key) CF_RETURNS_RETAINED {
    if (!CFEqual(key, kCFErrorDescriptionKey) && !CFEqual(key, kCFErrorLocalizedFailureReasonKey)) return NULL;
    
    const char *errCStr = strerror(CFErrorGetCode(err));
    CFStringRef errStr = (errCStr && strlen(errCStr)) ? CFStringCreateWithCString(kCFAllocatorSystemDefault, errCStr, kCFStringEncodingUTF8) : NULL;
    
    if (!errStr) return NULL;
    if (CFEqual(key, kCFErrorDescriptionKey)) return errStr;	// If all we wanted was the non-localized description, we're done
    
#if TARGET_OS_MAC
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
                        CFStringRef notFoundPlaceholder = CFSTR("%%NOTFOUND%%");
                        CFStringRef localizedErrStr = CFBundleCopyLocalizedString(bundle, errStr, notFoundPlaceholder, CFSTR("ErrnoErrors"));
                        if (localizedErrStr == notFoundPlaceholder) {
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

#if TARGET_OS_MAC
/* Built-in callback for Mach domain.
*/
static CFTypeRef _CFErrorMachCallBack(CFErrorRef err, CFStringRef key) CF_RETURNS_RETAINED {
    if (CFEqual(key, kCFErrorDescriptionKey)) {
        const char *errStr = mach_error_string(CFErrorGetCode(err));
        if (errStr && strlen(errStr)) return CFStringCreateWithCString(kCFAllocatorSystemDefault, errStr, kCFStringEncodingUTF8);
    }
    return NULL;
}
#endif


static const void *blockCopyValueCallBack(CFAllocatorRef allocator, const void *value) {return _Block_copy(value);}
static void blockReleaseValueCallBack(CFAllocatorRef allocator, const void *value) {_Block_release(value);}
static void __CFErrorSetCallBackForDomainNoLock(CFStringRef domainName, CFErrorUserInfoKeyCallBack callBack);

/* This initialize function is meant to be called lazily, the first time a callback is registered or requested. It creates the table and registers the built-in callbacks. Clearly doing this non-lazily in _CFErrorInitialize() would be simpler, but this is a fine example of something that should not have to happen at launch time.
*/
static void _CFErrorInitializeCallBackTable(void) {
    // Create the table outside the lock
    CFDictionaryValueCallBacks dictionaryValueCallBacks = kCFTypeDictionaryValueCallBacks;
    dictionaryValueCallBacks.retain = blockCopyValueCallBack;
    dictionaryValueCallBacks.release = blockReleaseValueCallBack;
    CFMutableDictionaryRef table = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &dictionaryValueCallBacks);
    
    os_unfair_lock_lock_with_options(&_CFErrorLock, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
    if (!_CFErrorCallBackTable) {
        _CFErrorCallBackTable = table;
        // Register the known providers, going thru a special variant of the function that doesn't lock
        __CFErrorSetCallBackForDomainNoLock(kCFErrorDomainPOSIX, _CFErrorPOSIXCallBack);
#if TARGET_OS_MAC
        __CFErrorSetCallBackForDomainNoLock(kCFErrorDomainMach, _CFErrorMachCallBack);
#endif
        os_unfair_lock_unlock(&_CFErrorLock);
    } else {
        os_unfair_lock_unlock(&_CFErrorLock);
        CFRelease(table);
    }
}

void CFErrorSetCallBackBlockForDomain(CFStringRef domainName, CFErrorUserInfoKeyCallBackBlock block) {
    if (!_CFErrorCallBackTable) _CFErrorInitializeCallBackTable();
    os_unfair_lock_lock_with_options(&_CFErrorLock, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
    if (block) {
        CFDictionarySetValue(_CFErrorCallBackTable, domainName, (void *)block);
    } else {
        CFDictionaryRemoveValue(_CFErrorCallBackTable, domainName);
    }
    os_unfair_lock_unlock(&_CFErrorLock);
}

CFErrorUserInfoKeyCallBackBlock CFErrorGetCallBackBlockForDomain(CFStringRef domainName) {
    if (!_CFErrorCallBackTable) _CFErrorInitializeCallBackTable();
    os_unfair_lock_lock_with_options(&_CFErrorLock, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
    CFErrorUserInfoKeyCallBackBlock callBack = _CFErrorCallBackTable ? (CFErrorUserInfoKeyCallBackBlock)CFDictionaryGetValue(_CFErrorCallBackTable, domainName) : NULL;
    os_unfair_lock_unlock(&_CFErrorLock);
    return callBack;
}

CFErrorUserInfoKeyCallBackBlock CFErrorCopyCallBackBlockForDomain(CFStringRef domainName) {
    if (!_CFErrorCallBackTable) _CFErrorInitializeCallBackTable();
    os_unfair_lock_lock_with_options(&_CFErrorLock, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
    CFErrorUserInfoKeyCallBackBlock callBack = _CFErrorCallBackTable ? (CFErrorUserInfoKeyCallBackBlock)CFDictionaryGetValue(_CFErrorCallBackTable, domainName) : NULL;
    if (callBack) CFRetain(callBack);
    os_unfair_lock_unlock(&_CFErrorLock);
    return callBack;
}

// This is meant to be called only with _CFErrorSpinlock taken and with _CFErrorCallBackTable already created
static void __CFErrorSetCallBackForDomainNoLock(CFStringRef domainName, CFErrorUserInfoKeyCallBack callBack) {
    // Since we have replaced the callback functionality with a callback block functionality, we register callback function embedded in a block which autoreleases the result
    if (callBack) {
        CFErrorUserInfoKeyCallBackBlock block = ^(CFErrorRef err, CFStringRef key){
            CFTypeRef result = callBack(err, key);
#if !DEPLOYMENT_RUNTIME_SWIFT
            if (result) CFAutorelease(result);
#endif
            return result;
        };
        CFDictionarySetValue(_CFErrorCallBackTable, domainName, (void *)block);
    } else {
        CFDictionaryRemoveValue(_CFErrorCallBackTable, domainName);
    }
}

// !!! This function can go away after 10.13/iOS 11
void CFErrorSetCallBackForDomain(CFStringRef domainName, CFErrorUserInfoKeyCallBack callBack) {
    // Since we have replaced the callback functionality with a callback block functionality, we now register (legacy) callback functions embedded in a block which autoreleases the result
    CFErrorUserInfoKeyCallBackBlock block = (!callBack) ? NULL : ^(CFErrorRef err, CFStringRef key){
        CFTypeRef result = callBack(err, key);
#if !DEPLOYMENT_RUNTIME_SWIFT
        if (result) CFAutorelease(result);
#endif
        return result;
    };
    CFErrorSetCallBackBlockForDomain(domainName, block);
}

// !!! This function can go away after 10.13/iOS 11
CFErrorUserInfoKeyCallBack CFErrorGetCallBackForDomain(CFStringRef domainName) {
    // Since there were no callers other than CF, removed as of 10.11/iOS9
    // Otherwise would have had to have separate tables for callback functions and blocks
    return NULL;
}




