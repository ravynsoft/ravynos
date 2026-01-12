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

/*	CFPlugIn_Instance.c
	Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
        Responsibility: Tony Parker
*/

#include "CFBundle_Internal.h"
#include "CFInternal.h"

static CFTypeID __kCFPlugInInstanceTypeID = _kCFRuntimeNotATypeID;

struct __CFPlugInInstance {
    CFRuntimeBase _base;
    
    _CFPFactoryRef factory;
    
    CFPlugInInstanceGetInterfaceFunction getInterfaceFunction;
    CFPlugInInstanceDeallocateInstanceDataFunction deallocateInstanceDataFunction;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4200)
#endif //_MSC_VER
    uint8_t _instanceData[0];
#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER
};

static CFStringRef __CFPlugInInstanceCopyDescription(CFTypeRef cf) {
    /* MF:!!! Implement me */
    return CFSTR("Some CFPlugInInstance");
}

static void __CFPlugInInstanceDeallocate(CFTypeRef cf) {
    CFPlugInInstanceRef instance = (CFPlugInInstanceRef)cf;
    
    __CFGenericValidateType(cf, CFPlugInInstanceGetTypeID());

    if (instance->deallocateInstanceDataFunction) {
        FAULT_CALLBACK((void **)&(instance->deallocateInstanceDataFunction));
        (void)INVOKE_CALLBACK1(instance->deallocateInstanceDataFunction, (void *)(&instance->_instanceData[0]));
    }

    if (instance->factory) _CFPFactoryRemoveInstance(instance->factory);
}

static const CFRuntimeClass __CFPlugInInstanceClass = {
    0,
    "CFPlugInInstance",
    NULL,      // init
    NULL,      // copy
    __CFPlugInInstanceDeallocate,
    NULL,      // equal
    NULL,      // hash
    NULL,      // 
    __CFPlugInInstanceCopyDescription
};

CFTypeID CFPlugInInstanceGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFPlugInInstanceTypeID = _CFRuntimeRegisterClass(&__CFPlugInInstanceClass); });
    return __kCFPlugInInstanceTypeID;
}

CF_EXPORT CFPlugInInstanceRef CFPlugInInstanceCreateWithInstanceDataSize(CFAllocatorRef allocator, CFIndex instanceDataSize, CFPlugInInstanceDeallocateInstanceDataFunction deallocateInstanceFunction, CFStringRef factoryName, CFPlugInInstanceGetInterfaceFunction getInterfaceFunction) {
    CFPlugInInstanceRef instance;
    UInt32 size;
    size = sizeof(struct __CFPlugInInstance) + instanceDataSize - sizeof(CFRuntimeBase);
    instance = (CFPlugInInstanceRef)_CFRuntimeCreateInstance(allocator, CFPlugInInstanceGetTypeID(), size, NULL);
    if (!instance) return NULL;

    instance->factory = _CFPFactoryFind((CFUUIDRef)factoryName, true);
    if (instance->factory) _CFPFactoryAddInstance(instance->factory);
    instance->getInterfaceFunction = getInterfaceFunction;
    instance->deallocateInstanceDataFunction = deallocateInstanceFunction;

    return instance;
}

CF_EXPORT Boolean CFPlugInInstanceGetInterfaceFunctionTable(CFPlugInInstanceRef instance, CFStringRef interfaceName, void **ftbl) {
    void *myFtbl;
    Boolean result = false;

    if (instance->getInterfaceFunction) {
        FAULT_CALLBACK((void **)&(instance->getInterfaceFunction));
        result = INVOKE_CALLBACK3(instance->getInterfaceFunction, instance, interfaceName, &myFtbl) ? true : false;
    }
    if (ftbl) *ftbl = (result ? myFtbl : NULL);
    return result;
}

CF_EXPORT CFStringRef CFPlugInInstanceGetFactoryName(CFPlugInInstanceRef instance) {
    // This function leaks, but it's the only safe way to access the factory name (on 10.8 or later).
    // On 10.9 we added the CF_RETURNS_RETAINED annotation to the header.
    CFUUIDRef factoryId = _CFPFactoryCopyFactoryID(instance->factory);
    return (CFStringRef)factoryId;
}

CF_EXPORT void *CFPlugInInstanceGetInstanceData(CFPlugInInstanceRef instance) {
    return (void *)(&instance->_instanceData[0]);
}

