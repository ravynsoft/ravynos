/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#include <TargetConditionals.h>
#include <libc.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include  "IOSystemConfiguration.h"


#define SYSTEM_FRAMEWORK_DIR "/System/Library/Frameworks"
#define SYSTEM_CONFIGURATION "SystemConfiguration.framework/SystemConfiguration"
#define SC_FRAMEWORK	     SYSTEM_FRAMEWORK_DIR "/" SYSTEM_CONFIGURATION

/* IOKIT_SC_SYMBOL 
 *
 * - Tells the linker to pretend that _SYMBOL doesn't exist in OS X 10.7 and later. 
 * - Exports _SYMBOL as extern for OS X
 * - Hides the _SYMBOL as __private_extern__ for iOS builds
 *
 */
#if TARGET_OS_IPHONE
#define IOKIT_SC_SYMBOL(_RETURN, _SYMBOL)   __private_extern__ _RETURN _io_##_SYMBOL
#else
#define IOKIT_SC_SYMBOL(_RETURN, _SYMBOL)   extern const char _SYMBOL##_tmp7 __asm("$ld$hide$os10.7$_" #_SYMBOL ); \
                                            __attribute__ ((visibility("default"))) const char _SYMBOL##_tmp7 = 0; \
                                            _RETURN _SYMBOL
#endif

const CFStringRef _io_kSCCompAnyRegex = CFSTR("[^/]+");
const CFStringRef _io_kSCDynamicStoreDomainState = CFSTR("State:");

static void * symAddrInSC(const char *name)
{
    static void * handle = NULL;
    
    if (!handle) {
        void            *locHandle;
        const char      *framework = SC_FRAMEWORK;
        struct stat     statbuf;
        const char      *suffix = getenv("DYLD_IMAGE_SUFFIX");
        char            path[MAXPATHLEN];

        strlcpy(path, framework, sizeof(path));
        
        if (suffix) {
            strlcat(path, suffix, sizeof(path));
        }

        if (0 <= stat(path, &statbuf)) {
            locHandle = dlopen(path,      RTLD_LAZY);
        } else {
            locHandle = dlopen(framework, RTLD_LAZY);
        }
            
        if (locHandle) {
            handle = locHandle;
        }
    }

    if (handle)
        return dlsym(handle, name);
    else
        return NULL;
}

IOKIT_SC_SYMBOL(Boolean, SCDynamicStoreAddWatchedKey)(
        SCDynamicStoreRef		store,
	 CFStringRef			key,
	 Boolean				isRegex)
{
    static typeof (SCDynamicStoreAddWatchedKey) *dyfunc;
    if (!dyfunc) 
        dyfunc = symAddrInSC("SCDynamicStoreAddWatchedKey");

    if (dyfunc)
        return (*dyfunc)(store, key, isRegex);
    else
        return false;
}


IOKIT_SC_SYMBOL(int, SCError)()
{
    static typeof (SCError) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCError");
    if (dyfunc)
	return (*dyfunc)();
    else
	return kSCStatusFailed;
}



IOKIT_SC_SYMBOL(CFDictionaryRef, SCDynamicStoreCopyMultiple)(
	SCDynamicStoreRef		store,
    CFArrayRef			keys,
    CFArrayRef			patterns
)
{
    static typeof (SCDynamicStoreCopyMultiple) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCDynamicStoreCopyMultiple");
    if (dyfunc)
	return (*dyfunc)(store, keys, patterns);
    else
	return NULL;
}


IOKIT_SC_SYMBOL(CFTypeRef, SCDynamicStoreCopyValue)(
	SCDynamicStoreRef		store,
	CFStringRef			key
)
{
    static typeof (SCDynamicStoreCopyValue) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCDynamicStoreCopyValue");
    if (dyfunc)
	return (*dyfunc)(store, key);
    else
	return NULL;
}


IOKIT_SC_SYMBOL(SCDynamicStoreRef, SCDynamicStoreCreate)(
	CFAllocatorRef		allocator,
	CFStringRef			name,
	SCDynamicStoreCallBack	callout,
	SCDynamicStoreContext	*context
)
{
    static typeof (SCDynamicStoreCreate) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCDynamicStoreCreate");
    if (dyfunc)
	return (*dyfunc)(allocator, name, callout, context);
    else
	return NULL;
}


IOKIT_SC_SYMBOL(CFRunLoopSourceRef, SCDynamicStoreCreateRunLoopSource)(
    CFAllocatorRef		allocator,
	SCDynamicStoreRef		store,
	CFIndex			order
)
{
    static typeof (SCDynamicStoreCreateRunLoopSource) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCDynamicStoreCreateRunLoopSource");
    if (dyfunc)
	return (*dyfunc)(allocator, store, order);
    else
	return NULL;
}


IOKIT_SC_SYMBOL(CFStringRef, SCDynamicStoreKeyCreate)(
    CFAllocatorRef		allocator,
	CFStringRef			fmt,
	...
)
{
    // Local implementation of a SCDynamicStore wrapper function
    va_list val;

    va_start(val, fmt);
    CFStringRef key =
	CFStringCreateWithFormatAndArguments(allocator, NULL, fmt, val);
    va_end(val);

    return key;
}


IOKIT_SC_SYMBOL(CFStringRef, SCDynamicStoreKeyCreatePreferences)(
	CFAllocatorRef		allocator,
	CFStringRef			prefsID,
	SCPreferencesKeyType	keyType
)
{
    static typeof (SCDynamicStoreKeyCreatePreferences) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCDynamicStoreKeyCreatePreferences");
    if (dyfunc)
	return (*dyfunc)(allocator, prefsID, keyType);
    else
	return NULL;
}


IOKIT_SC_SYMBOL(Boolean, SCDynamicStoreSetNotificationKeys)(
	SCDynamicStoreRef		store,
	CFArrayRef			keys,
	CFArrayRef			patterns
)
{
    static typeof (SCDynamicStoreSetNotificationKeys) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCDynamicStoreSetNotificationKeys");
    if (dyfunc)
	return (*dyfunc)(store, keys, patterns);
    else
	return false;
}


IOKIT_SC_SYMBOL(Boolean, SCDynamicStoreNotifyValue)(
	SCDynamicStoreRef		store,
	CFStringRef			key
)
{
    static typeof (SCDynamicStoreNotifyValue) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCDynamicStoreNotifyValue");
    if (dyfunc)
	return (*dyfunc)(store, key);
    else
	return false;
}

IOKIT_SC_SYMBOL(Boolean, SCDynamicStoreSetValue)(
    SCDynamicStoreRef		store,
	CFStringRef			key,
	CFPropertyListRef		value
)
{
    static typeof (SCDynamicStoreSetValue) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCDynamicStoreSetValue");
    if (dyfunc)
	return (*dyfunc)(store, key, value);
    else
	return false;
}


IOKIT_SC_SYMBOL(Boolean, SCPreferencesApplyChanges)(
	SCPreferencesRef		prefs
)
{
    static typeof (SCPreferencesApplyChanges) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCPreferencesApplyChanges");
    if (dyfunc)
	return (*dyfunc)(prefs);
    else
	return false;
}


IOKIT_SC_SYMBOL(Boolean, SCPreferencesCommitChanges)(
	SCPreferencesRef		prefs
)
{
    static typeof (SCPreferencesCommitChanges) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCPreferencesCommitChanges");
    if (dyfunc)
	return (*dyfunc)(prefs);
    else
	return false;
}


IOKIT_SC_SYMBOL(SCPreferencesRef, SCPreferencesCreate)(
	CFAllocatorRef		allocator,
	CFStringRef			name,
	CFStringRef			prefsID
)
{
    static typeof (SCPreferencesCreate) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCPreferencesCreate");
    if (dyfunc)
	return (*dyfunc)(allocator, name, prefsID);
    else
	return NULL;
}


IOKIT_SC_SYMBOL(SCPreferencesRef, SCPreferencesCreateWithAuthorization)(
	CFAllocatorRef		allocator,
	CFStringRef		name,
	CFStringRef		prefsID,
	AuthorizationRef	authorization
)
{
    static typeof (SCPreferencesCreateWithAuthorization) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCPreferencesCreateWithAuthorization");
    if (dyfunc)
	return (*dyfunc)(allocator, name, prefsID, authorization);
    else
	return NULL;
}


IOKIT_SC_SYMBOL(CFPropertyListRef, SCPreferencesGetValue)(
	SCPreferencesRef		prefs,
	CFStringRef			key
)
{
    static typeof (SCPreferencesGetValue) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCPreferencesGetValue");
    if (dyfunc)
	return (*dyfunc)(prefs, key);
    else
	return NULL;
}


IOKIT_SC_SYMBOL(Boolean, SCPreferencesLock)(
	SCPreferencesRef		prefs,
	Boolean			wait
)
{
    static typeof (SCPreferencesLock) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCPreferencesLock");
    if (dyfunc)
	return (*dyfunc)(prefs, wait);
    else
	return false;
}


IOKIT_SC_SYMBOL(Boolean, SCPreferencesRemoveValue)(
	SCPreferencesRef		prefs,
	CFStringRef			key
)
{
    static typeof (SCPreferencesRemoveValue) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCPreferencesRemoveValue");
    if (dyfunc)
	return (*dyfunc)(prefs, key);
    else
	return false;
}


IOKIT_SC_SYMBOL(Boolean, SCPreferencesSetValue)(
	SCPreferencesRef		prefs,
	CFStringRef				key,
	CFPropertyListRef		value
)
{
    static typeof (SCPreferencesSetValue) *dyfunc;
    if (!dyfunc) 
		dyfunc = symAddrInSC("SCPreferencesSetValue");
    if (dyfunc)
		return (*dyfunc)(prefs, key, value);
    else
		return false;
}


IOKIT_SC_SYMBOL(Boolean, SCPreferencesUnlock)(
	SCPreferencesRef		prefs
) 
{
    static typeof (SCPreferencesUnlock) *dyfunc;
    if (!dyfunc) 
	dyfunc = symAddrInSC("SCPreferencesUnlock");
    if (dyfunc)
	return (*dyfunc)(prefs);
    else
	return false;
}
