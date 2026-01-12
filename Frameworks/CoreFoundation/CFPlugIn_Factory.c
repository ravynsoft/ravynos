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

/*	CFPlugIn_Factory.c
	Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
        Responsibility: Tony Parker
*/

#include "CFBundle_Internal.h"
#include "CFInternal.h"

static CFTypeID __kCFPFactoryTypeID = _kCFRuntimeNotATypeID;

struct __CFPFactory {
    CFRuntimeBase _base;
        
    CFUUIDRef _uuid;
    Boolean _enabled;
    char _padding[3];
    
    CFPlugInFactoryFunction _func;
    
    CFPlugInRef _plugIn;
    CFStringRef _funcName;
    
    CFMutableArrayRef _types;
    CFLock_t _lock;
};

static void _CFPFactoryDeallocate(CFTypeRef factory);

static const CFRuntimeClass __CFPFactoryClass = {
    0,
    "_CFPFactory",
    NULL,	// init
    NULL,	// copy
    _CFPFactoryDeallocate,
    NULL,	// equal
    NULL,	// hash
    NULL,       // formatting desc
    NULL,       // debug desc
};

CF_PRIVATE void __CFPFactoryInitialize(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFPFactoryTypeID = _CFRuntimeRegisterClass(&__CFPFactoryClass); });
}

static CFTypeID _CFPFactoryGetTypeID(void) {
    return __kCFPFactoryTypeID;
}

static CFLock_t CFPlugInGlobalDataLock = CFLockInit;
static CFMutableDictionaryRef _factoriesByFactoryID = NULL; /* Value is _CFPFactoryRef */
static CFMutableDictionaryRef _factoriesByTypeID = NULL; /* Value is array of _CFPFactoryRef */

static void _CFPFactoryAddToTable(_CFPFactoryRef factory) {
    __CFLock(&factory->_lock);
    CFUUIDRef uuid = (CFUUIDRef)CFRetain(factory->_uuid);
    CFRetain(factory);
    __CFUnlock(&factory->_lock);
    
    __CFLock(&CFPlugInGlobalDataLock);    
    if (!_factoriesByFactoryID) {
        CFDictionaryValueCallBacks _factoryDictValueCallbacks = {0, NULL, NULL, NULL, NULL};
        _factoriesByFactoryID = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &_factoryDictValueCallbacks);
    }
    CFDictionarySetValue(_factoriesByFactoryID, uuid, factory);
    __CFUnlock(&CFPlugInGlobalDataLock);
    
    if (uuid) CFRelease(uuid);
    CFRelease(factory);
}

static void _CFPFactoryRemoveFromTable(_CFPFactoryRef factory) {
    __CFLock(&factory->_lock);
    CFUUIDRef uuid = factory->_uuid;
    if (uuid) CFRetain(uuid);
    __CFUnlock(&factory->_lock);
    
    __CFLock(&CFPlugInGlobalDataLock);
    if (uuid && _factoriesByTypeID) CFDictionaryRemoveValue(_factoriesByFactoryID, uuid);
    __CFUnlock(&CFPlugInGlobalDataLock);
    
    if (uuid) CFRelease(uuid);
}

CF_PRIVATE _CFPFactoryRef _CFPFactoryFind(CFUUIDRef factoryID, Boolean enabled) {
    _CFPFactoryRef result = NULL;
    
    __CFLock(&CFPlugInGlobalDataLock);
    if (_factoriesByFactoryID) {
        result = (_CFPFactoryRef )CFDictionaryGetValue(_factoriesByFactoryID, factoryID);
        if (result && result->_enabled != enabled) result = NULL;
    }
    __CFUnlock(&CFPlugInGlobalDataLock);
    return result;
}

static void _CFPFactoryDeallocate(CFTypeRef ty) {
    SInt32 c;
    _CFPFactoryRef factory = (_CFPFactoryRef)ty;
    
    _CFPFactoryRemoveFromTable(factory);

    if (factory->_plugIn) {
        _CFPlugInRemoveFactory(factory->_plugIn, factory);
        CFRelease(factory->_plugIn);
    }
    
    /* Remove all types for this factory. */
    c = CFArrayGetCount(factory->_types);
    while (c-- > 0) _CFPFactoryRemoveType(factory, (CFUUIDRef)CFArrayGetValueAtIndex(factory->_types, c));
    CFRelease(factory->_types);

    if (factory->_funcName) CFRelease(factory->_funcName);
    if (factory->_uuid) CFRelease(factory->_uuid);
}

static _CFPFactoryRef _CFPFactoryCommonCreate(CFAllocatorRef allocator, CFUUIDRef factoryID) {
    _CFPFactoryRef factory;
    uint32_t size;
    size = sizeof(struct __CFPFactory) - sizeof(CFRuntimeBase);
    factory = (_CFPFactoryRef)_CFRuntimeCreateInstance(allocator, _CFPFactoryGetTypeID(), size, NULL);
    if (!factory) return NULL;

    factory->_uuid = (CFUUIDRef)CFRetain(factoryID);
    factory->_enabled = true;
    factory->_types = CFArrayCreateMutable(allocator, 0, &kCFTypeArrayCallBacks);
    factory->_lock = CFLockInit; // WARNING: grab global lock before this lock
        
    _CFPFactoryAddToTable(factory);

    return factory;
}

CF_PRIVATE _CFPFactoryRef _CFPFactoryCreate(CFAllocatorRef allocator, CFUUIDRef factoryID, CFPlugInFactoryFunction func) {
    _CFPFactoryRef factory = _CFPFactoryCommonCreate(allocator, factoryID);

    __CFLock(&factory->_lock);    
    factory->_func = func;
    factory->_plugIn = NULL;
    factory->_funcName = NULL;
    __CFUnlock(&factory->_lock);

    return factory;
}

CF_PRIVATE _CFPFactoryRef _CFPFactoryCreateByName(CFAllocatorRef allocator, CFUUIDRef factoryID, CFPlugInRef plugIn, CFStringRef funcName) {
    _CFPFactoryRef factory = _CFPFactoryCommonCreate(allocator, factoryID);

    __CFLock(&factory->_lock);    
    factory->_func = NULL;
    factory->_plugIn = (CFPlugInRef)CFRetain(plugIn);
    if (plugIn) _CFPlugInAddFactory(plugIn, factory);
    factory->_funcName = (funcName ? (CFStringRef)CFStringCreateCopy(allocator, funcName) : NULL);
    __CFUnlock(&factory->_lock);

    return factory;
}

CF_PRIVATE CFUUIDRef _CFPFactoryCopyFactoryID(_CFPFactoryRef factory) {
    __CFLock(&factory->_lock);
    CFUUIDRef uuid = factory->_uuid;
    if (uuid) CFRetain(uuid);
    __CFUnlock(&factory->_lock);
    return uuid;
}

CF_PRIVATE CFPlugInRef _CFPFactoryCopyPlugIn(_CFPFactoryRef factory) {
    __CFLock(&factory->_lock);
    CFPlugInRef result = factory->_plugIn;
    if (result) CFRetain(result);
    __CFUnlock(&factory->_lock);
    return result;
}

CF_PRIVATE void *_CFPFactoryCreateInstance(CFAllocatorRef allocator, _CFPFactoryRef factory, CFUUIDRef typeID) {
    void *result = NULL;

    __CFLock(&factory->_lock);
    if (factory->_enabled) {
        if (!factory->_func) {
            factory->_func = (CFPlugInFactoryFunction)CFBundleGetFunctionPointerForName(factory->_plugIn, factory->_funcName);
            if (!factory->_func) CFLog(__kCFLogPlugIn, CFSTR("Cannot find function pointer %@ for factory %@ in %@"), factory->_funcName, factory->_uuid, factory->_plugIn);
        }
        if (factory->_func) {
            // UPPGOOP
            CFPlugInFactoryFunction f = factory->_func;
            __CFUnlock(&factory->_lock);
            FAULT_CALLBACK((void **)&(f));
            result = (void *)INVOKE_CALLBACK2(f, allocator, typeID);
            __CFLock(&factory->_lock);
        }
    } else {
        CFLog(__kCFLogPlugIn, CFSTR("Factory %@ is disabled"), factory->_uuid);
    }    
    __CFUnlock(&factory->_lock);

    return result;
}

CF_PRIVATE void _CFPFactoryDisable(_CFPFactoryRef factory) {
    __CFLock(&factory->_lock);    
    factory->_enabled = false;
    __CFUnlock(&factory->_lock);
    CFRelease(factory);
}

CF_PRIVATE void _CFPFactoryFlushFunctionCache(_CFPFactoryRef factory) {
    /* MF:!!! Assert that this factory belongs to a plugIn. */
    /* This is called by the factory's plugIn when the plugIn unloads its code. */
    __CFLock(&factory->_lock);
    factory->_func = NULL;
    __CFUnlock(&factory->_lock);
}

CF_PRIVATE void _CFPFactoryAddType(_CFPFactoryRef factory, CFUUIDRef typeID) {
    /* Add the factory to the type's array of factories */
    __CFLock(&factory->_lock);
    /* Add the type to the factory's type list */
    CFArrayAppendValue(factory->_types, typeID);
    __CFUnlock(&factory->_lock);

    __CFLock(&CFPlugInGlobalDataLock);
    if (!_factoriesByTypeID) _factoriesByTypeID = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFMutableArrayRef array = (CFMutableArrayRef)CFDictionaryGetValue(_factoriesByTypeID, typeID);
    if (!array) {
        CFArrayCallBacks _factoryArrayCallbacks = {0, NULL, NULL, NULL, NULL};
        // Create this from default allocator
        array = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &_factoryArrayCallbacks);
        CFDictionarySetValue(_factoriesByTypeID, typeID, array);
        CFRelease(array);
    }
    CFArrayAppendValue(array, factory);
    __CFUnlock(&CFPlugInGlobalDataLock);
}

CF_PRIVATE void _CFPFactoryRemoveType(_CFPFactoryRef factory, CFUUIDRef typeID) {
    /* Remove it from the factory's type list */
    SInt32 idx;

    __CFLock(&factory->_lock);
    idx = CFArrayGetFirstIndexOfValue(factory->_types, CFRangeMake(0, CFArrayGetCount(factory->_types)), typeID);
    if (idx >= 0) CFArrayRemoveValueAtIndex(factory->_types, idx);
    __CFUnlock(&factory->_lock);

    /* Remove the factory from the type's list of factories */
    __CFLock(&CFPlugInGlobalDataLock);
    if (_factoriesByTypeID) {
        CFMutableArrayRef array = (CFMutableArrayRef)CFDictionaryGetValue(_factoriesByTypeID, typeID);
        if (array) {
            idx = CFArrayGetFirstIndexOfValue(array, CFRangeMake(0, CFArrayGetCount(array)), factory);
            if (idx >= 0) {
                CFArrayRemoveValueAtIndex(array, idx);
                if (CFArrayGetCount(array) == 0) CFDictionaryRemoveValue(_factoriesByTypeID, typeID);
            }
        }
    }
    __CFUnlock(&CFPlugInGlobalDataLock);
}

CF_PRIVATE Boolean _CFPFactorySupportsType(_CFPFactoryRef factory, CFUUIDRef typeID) {
    SInt32 idx;

    __CFLock(&factory->_lock);
    idx = CFArrayGetFirstIndexOfValue(factory->_types, CFRangeMake(0, CFArrayGetCount(factory->_types)), typeID);
    __CFUnlock(&factory->_lock);
    
    return (idx >= 0 ? true : false);
}

CF_PRIVATE CFArrayRef _CFPFactoryFindCopyForType(CFUUIDRef typeID) {
    CFArrayRef result = NULL;
    __CFLock(&CFPlugInGlobalDataLock);
    if (_factoriesByTypeID) {
        result = (CFArrayRef)CFDictionaryGetValue(_factoriesByTypeID, typeID);
        if (result) CFRetain(result);
    }
    __CFUnlock(&CFPlugInGlobalDataLock);

    return result;
}

/* These methods are called by CFPlugInInstance when an instance is created or destroyed.  If a factory's instance count goes to 0 and the factory has been disabled, the factory is destroyed. */
CF_PRIVATE void _CFPFactoryAddInstance(_CFPFactoryRef factory) {
    /* MF:!!! Assert that factory is enabled. */
    CFRetain(factory);
    __CFLock(&factory->_lock);
    CFPlugInRef plugin = factory->_plugIn;
    if (plugin) CFRetain(plugin);
    __CFUnlock(&factory->_lock);
    if (plugin) {
        _CFPlugInAddPlugInInstance(plugin);
        CFRelease(plugin);
    }    
}

CF_PRIVATE void _CFPFactoryRemoveInstance(_CFPFactoryRef factory) {
    __CFLock(&factory->_lock);
    CFPlugInRef plugin = factory->_plugIn;
    if (plugin) CFRetain(plugin);
    __CFUnlock(&factory->_lock);
    if (plugin) {
        _CFPlugInRemovePlugInInstance(factory->_plugIn);
        CFRelease(plugin);
    }
    CFRelease(factory);
}
