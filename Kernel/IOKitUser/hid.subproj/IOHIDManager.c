/*
 * Copyright (c) 1999-2008 Apple Computer, Inc.  All Rights Reserved.
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
#include <pthread.h>
#include <CoreFoundation/CFRuntime.h>
#include "IOHIDManager.h"
#include <IOKit/IOKitLib.h>
#include "IOHIDLibPrivate.h"
#include "IOHIDDevicePrivate.h"
#include "IOHIDLib.h"
#include "IOHIDManagerPersistentProperties.h"
#include <os/assumes.h>
#include <AssertMacros.h>
#include <os/lock.h>
#include <os/state_private.h>

static IOHIDManagerRef  __IOHIDManagerCreate(
                                    CFAllocatorRef          allocator, 
                                    CFAllocatorContext *    context __unused);
static void             __IOHIDManagerExtRelease( CFTypeRef object );
static void             __IOHIDManagerIntRelease( CFTypeRef object );
static void             __IOHIDManagerSetDeviceMatching(
                                IOHIDManagerRef                 manager,
                                CFDictionaryRef                 matching);
static void             __IOHIDManagerDeviceAdded(
                                    void *                      context,
                                    io_iterator_t               iterator);
static void             __IOHIDManagerDeviceRemoved(void *refcon,
                                                    io_service_t service,
                                                    uint32_t messageType,
                                                    void *messageArgument);
static IOReturn         __ApplyToDevices(IOHIDManagerRef manager,
                                         IOOptionBits options);
static void             __IOHIDManagerDeviceApplier(
                                    const void *                value,
                                    void *                      context);
static void             __IOHIDManagerInitialEnumCallback(
                                    void *                      info);
static void             __IOHIDManagerMergeDictionaries(
                                    CFDictionaryRef             srcDict, 
                                    CFMutableDictionaryRef      dstDict);
static  void __IOHIDManagerFinalizeStateHandler (void *context);
static  os_state_data_t __IOHIDManagerStateHandler (IOHIDManagerRef device, os_state_hints_t hints);
static  CFMutableDictionaryRef __IOHIDManagerSerializeState(IOHIDManagerRef device);

enum {
    kDeviceApplierOpen                      = 1 << 0,
    kDeviceApplierClose                     = 1 << 1,
    kDeviceApplierInitEnumCallback          = 1 << 2,
    kDeviceApplierSetInputMatching          = 1 << 3,
    kDeviceApplierSetInputCallback          = 1 << 4,
    kDeviceApplierSetInputReportCallback    = 1 << 5, 
    kDeviceApplierScheduleRunLoop           = 1 << 6,
    kDeviceApplierUnscheduleRunLoop         = 1 << 7,
    kDeviceApplierSetDispatchQueue          = 1 << 8,
    kDeviceApplierSetCancelHandler          = 1 << 9,
    kDeviceApplierActivate                  = 1 << 10,
    kDeviceApplierCancel                    = 1 << 11,
    kDeviceApplierSetInputTSReportCallback  = 1 << 12,
};

typedef struct __DeviceApplierArgs {
    IOHIDManagerRef     manager;
    IOOptionBits        options;
    IOReturn            retVal;
}   DeviceApplierArgs;

typedef struct __IOHIDManager
{
    IOHIDObjectBase                 hidBase;
    
    CFMutableSetRef                 devices;
    os_unfair_lock                  deviceLock;
    
    CFMutableSetRef                 iterators;
    CFMutableDictionaryRef          removalNotifiers;
    
    CFMutableDictionaryRef          properties;
    CFMutableDictionaryRef          deviceInputBuffers;

    IONotificationPortRef           notifyPort;
    CFRunLoopRef                    runLoop;
    CFStringRef                     runLoopMode;
    
    dispatch_queue_t                dispatchQueue;
    dispatch_block_t                cancelHandler;
    uint32_t                        cancelCount;
    
    _Atomic uint32_t                dispatchStateMask;
    
    CFRunLoopSourceRef              initEnumRunLoopSource;
    CFMutableDictionaryRef          initRetVals;
    
    Boolean                         isOpen;
    IOOptionBits                    openOptions;
    IOOptionBits                    createOptions;
    
    void *                          inputContext;
    IOHIDValueCallback              inputCallback;
    
    void *                              reportContext;
    IOHIDReportCallback                 reportCallback;
    IOHIDReportWithTimeStampCallback    reportTimestampCallback;
    
    void *                          matchContext;
    IOHIDDeviceCallback             matchCallback;
    
    void *                          removalContext;
    IOHIDDeviceCallback             removalCallback;
    
    CFArrayRef                      inputMatchingMultiple;
    Boolean                         isDirty;

    os_state_handle_t               stateHandler;
    dispatch_queue_t                stateQueue;

} __IOHIDManager, *__IOHIDManagerRef;

static const IOHIDObjectClass __IOHIDManagerClass = {
    {
        _kCFRuntimeCustomRefCount,  // version
        "IOHIDManager",             // className
        NULL,                       // init
        NULL,                       // copy
        __IOHIDManagerExtRelease,   // finalize
        NULL,                       // equal
        NULL,                       // hash
        NULL,                       // copyFormattingDesc
        NULL,                       // copyDebugDesc
        NULL,                       // reclaim
        _IOHIDObjectExtRetainCount, // refcount
        NULL                        // requiredAlignment
    },
    _IOHIDObjectIntRetainCount,
    __IOHIDManagerIntRelease
};

static pthread_once_t __sessionTypeInit = PTHREAD_ONCE_INIT;
static CFTypeID __kIOHIDManagerTypeID = _kCFRuntimeNotATypeID;

//------------------------------------------------------------------------------
// __IOHIDManagerRegister
//------------------------------------------------------------------------------
void __IOHIDManagerRegister(void)
{
    __kIOHIDManagerTypeID = _CFRuntimeRegisterClass(&__IOHIDManagerClass.cfClass);
}

//------------------------------------------------------------------------------
// __IOHIDManagerCreate
//------------------------------------------------------------------------------
IOHIDManagerRef __IOHIDManagerCreate(   
                                CFAllocatorRef              allocator, 
                                CFAllocatorContext *        context __unused)
{
    uint32_t        size;
    
    /* allocate service */
    size = sizeof(__IOHIDManager) - sizeof(CFRuntimeBase);
    
    return (IOHIDManagerRef)_IOHIDObjectCreateInstance(allocator, IOHIDManagerGetTypeID(), size, NULL);
}

//------------------------------------------------------------------------------
// __IOHIDManagerExtRelease
//------------------------------------------------------------------------------
void __IOHIDManagerExtRelease( CFTypeRef object )
{
    IOHIDManagerRef manager = (IOHIDManagerRef)object;
    
    if (manager->dispatchQueue) {
        // enforce the call to activate/cancel
        os_assert(manager->dispatchStateMask == (kIOHIDDispatchStateActive | kIOHIDDispatchStateCancelled),
                  "Invalid dispatch state: 0x%x", manager->dispatchStateMask);
    }
    
    if ( (manager->createOptions & kIOHIDManagerOptionUsePersistentProperties) && 
        !(manager->createOptions & kIOHIDManagerOptionDoNotSaveProperties)) {
        __IOHIDManagerSaveProperties(manager, NULL);
    }
    
    if ( manager->isOpen ) {
        // This will unschedule the manager if it was opened
        IOHIDManagerClose(manager, manager->openOptions);
    }
    
    if ( manager->runLoop ) {
        // This will unschedule the manager if it wasn't
        IOHIDManagerUnscheduleFromRunLoop(manager, manager->runLoop, manager->runLoopMode);
    }
        
    // Remove the notification
    if (manager->notifyPort) {
        CFRunLoopSourceRef source = IONotificationPortGetRunLoopSource(manager->notifyPort);
        if (source) {
            CFRunLoopSourceInvalidate(source);
            if (manager->runLoop)
                CFRunLoopRemoveSource(manager->runLoop,
                                      source,
                                      manager->runLoopMode);
        }
    }
    
    if (manager->initEnumRunLoopSource) {
        CFRunLoopSourceInvalidate(manager->initEnumRunLoopSource);
        if (manager->runLoop)
            CFRunLoopRemoveSource(manager->runLoop,
                                  manager->initEnumRunLoopSource,
                                  manager->runLoopMode);
    }
    
    if (manager->stateHandler) {
        os_state_remove_handler(manager->stateHandler);
    }
    
    if (manager->stateQueue) {
        dispatch_set_context(manager->stateQueue, manager);
        dispatch_set_finalizer_f(manager->stateQueue, __IOHIDManagerFinalizeStateHandler);
        _IOHIDObjectInternalRetain(manager);
        dispatch_release(manager->stateQueue);
    }

}

//------------------------------------------------------------------------------
// __IOHIDManagerIntRelease
//------------------------------------------------------------------------------
void __IOHIDManagerIntRelease( CFTypeRef object )
{
    IOHIDManagerRef manager = (IOHIDManagerRef)object;
    
    // Destroy the notification
    if ( manager->notifyPort ) {
        IONotificationPortDestroy(manager->notifyPort);
        manager->notifyPort = NULL;
    }
    
    if ( manager->initEnumRunLoopSource ) {
        CFRelease(manager->initEnumRunLoopSource);
        manager->initEnumRunLoopSource = NULL;
    }
    
    if ( manager->devices ) {
        CFRelease(manager->devices);
        manager->devices = NULL;
    }
    
    if ( manager->deviceInputBuffers ) {
        CFRelease(manager->deviceInputBuffers);
        manager->deviceInputBuffers = NULL;
    }
    
    if ( manager->iterators ) {
        CFRelease(manager->iterators);
        manager->iterators = NULL;
    }
    
    if ( manager->properties ) {
        CFRelease(manager->properties);
        manager->properties = NULL;
    }
    
    if ( manager->initRetVals ) {
        CFRelease(manager->initRetVals);
        manager->initRetVals = NULL;
    }
    
    if ( manager->inputMatchingMultiple ) {
        CFRelease(manager->inputMatchingMultiple);
        manager->inputMatchingMultiple = NULL;
    }
    
    if (manager->dispatchQueue) {
        dispatch_release(manager->dispatchQueue);
    }
    
    if (manager->removalNotifiers) {
        CFRelease(manager->removalNotifiers);
    }
}

//------------------------------------------------------------------------------
// __IOHIDManagerSetDeviceMatching
//------------------------------------------------------------------------------
void __IOHIDManagerSetDeviceMatching(
                                IOHIDManagerRef                 manager,
                                CFDictionaryRef                 matching)
{
    CFMutableDictionaryRef  matchingDict = NULL;
    io_iterator_t           iterator;
    IOReturn                kr;
    
    if (!manager->notifyPort) {
        manager->notifyPort = IONotificationPortCreate(kIOMasterPortDefault);
        
        if (manager->runLoop)  {
            CFRunLoopSourceRef source = IONotificationPortGetRunLoopSource(manager->notifyPort);
            if (source) {
                CFRunLoopAddSource(
                            manager->runLoop,
                            source,
                            manager->runLoopMode);
            }
        }
    }

    matchingDict = IOServiceMatching(kIOHIDDeviceKey);
    
    if ( !matchingDict )
        return;
        
    __IOHIDManagerMergeDictionaries(matching, matchingDict);
    
    // Now set up a notification to be called when a device is first 
    // matched by I/O Kit.  Note that this will not catch any devices that were 
    // already plugged in so we take care of those later.
    kr = IOServiceAddMatchingNotification(manager->notifyPort,
                                          kIOFirstMatchNotification,
                                          matchingDict,
                                          __IOHIDManagerDeviceAdded,
                                          manager,
                                          &iterator
                                          );

    if (kr != kIOReturnSuccess) {
        IOHIDLogError("IOServiceAddMatchingNotification:0x%x", kr);
        return;
    }
                
    // Add iterator to set for later destruction
    if ( !manager->iterators ) {
        CFSetCallBacks callbacks;
        
        bzero(&callbacks, sizeof(CFSetCallBacks));
        
        callbacks.retain    = _IOObjectCFRetain;
        callbacks.release   = _IOObjectCFRelease;
        
        manager->iterators  = CFSetCreateMutable(
                                            CFGetAllocator(manager),
                                            0, 
                                            &callbacks);
        if ( !manager->iterators )
            return;
    }
    
    intptr_t temp = iterator;
    os_unfair_lock_lock(&manager->deviceLock);
    CFSetAddValue(manager->iterators, (void *)temp);
    os_unfair_lock_unlock(&manager->deviceLock);
    IOObjectRelease(iterator);

    __IOHIDManagerDeviceAdded(manager, iterator);
}

//------------------------------------------------------------------------------
// __IOHIDManagerDeviceAdded
//------------------------------------------------------------------------------
void __IOHIDManagerDeviceAdded(     void *                      refcon,
                                    io_iterator_t               iterator)
{
    IOHIDManagerRef manager = (IOHIDManagerRef)refcon;
    IOHIDDeviceRef  device;
    IOReturn        retVal;
    io_service_t    service;
    Boolean         initial = FALSE;
    io_object_t     notification;
    
    while (( service = IOIteratorNext(iterator) )) {
        
        device = IOHIDDeviceCreate(CFGetAllocator(manager), service);
        
        if ( device ) {
            if ( !manager->devices ) {                
                manager->devices = CFSetCreateMutable(
                                                        CFGetAllocator(manager),
                                                        0, 
                                                        &kCFTypeSetCallBacks);
                initial = TRUE;
                
                if ( manager->isOpen )
                    manager->initRetVals = CFDictionaryCreateMutable(
                                            CFGetAllocator(manager),
                                            0,
                                            &kCFTypeDictionaryKeyCallBacks,
                                            NULL);
            }
            
            DeviceApplierArgs args;

            args.manager = manager;
            args.options = 0;
            
            retVal = IOServiceAddInterestNotification(manager->notifyPort,
                                                      service,
                                                      kIOGeneralInterest,
                                                      __IOHIDManagerDeviceRemoved,
                                                      manager,
                                                      &notification);
            
            if (!manager->removalNotifiers) {
                CFDictionaryValueCallBacks cb = kCFTypeDictionaryValueCallBacks;
                
                cb.retain = _IOObjectCFRetain;
                cb.release = _IOObjectCFRelease;
                
                manager->removalNotifiers = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                                      0,
                                                                      &kCFTypeDictionaryKeyCallBacks,
                                                                      &cb);
            }
            
            if (retVal == kIOReturnSuccess) {
                os_unfair_lock_lock(&manager->deviceLock);
                CFDictionarySetValue(manager->removalNotifiers,
                                     device,
                                     (void *)(intptr_t)notification);
                os_unfair_lock_unlock(&manager->deviceLock);
                
                IOObjectRelease(notification);
            } else {
                IOHIDLogError("IOServiceAddInterestNotification: 0x%x", retVal);
                CFRelease(device);
                IOObjectRelease(service);
                continue;
            }
            
            if ( manager->devices ) {
                os_unfair_lock_lock(&manager->deviceLock);
                CFSetAddValue(manager->devices, device);
                os_unfair_lock_unlock(&manager->deviceLock);
            }
            
            CFRelease(device);
            
            retVal = kIOReturnSuccess;
            
            if ( manager->isOpen )
                args.options |= kDeviceApplierOpen;
 
            if ( manager->inputMatchingMultiple )
                args.options |= kDeviceApplierSetInputMatching;
            
            if ( manager->inputCallback )
                args.options |= kDeviceApplierSetInputCallback;

            if ( manager->reportCallback )
                args.options |= kDeviceApplierSetInputReportCallback;
            
            if (manager->reportTimestampCallback) {
                args.options |= kDeviceApplierSetInputTSReportCallback;
            }
                                                       
            if ( manager->runLoop ) {
                args.options |= kDeviceApplierScheduleRunLoop;

                // If this this is called using the iterator returned in 
                // IOServiceAddMatchingNotification, pend performing the
                // callback on the runLoop
                if ( !initial && manager->matchCallback )
                    args.options |= kDeviceApplierInitEnumCallback;
            }
            
            if (manager->dispatchQueue) {
                args.options |= kDeviceApplierSetDispatchQueue;
                
                if (manager->matchCallback &&
                    manager->dispatchStateMask == kIOHIDDispatchStateActive)
                    args.options |= kDeviceApplierInitEnumCallback;
            }
            
            if (manager->cancelHandler) {
                args.options |= kDeviceApplierSetCancelHandler;
            }
            
            if (manager->dispatchStateMask & kIOHIDDispatchStateActive) {
                args.options |= kDeviceApplierActivate;
            }
            
            __IOHIDManagerDeviceApplier((const void *)device, &args);

            if ( (manager->createOptions & kIOHIDManagerOptionUsePersistentProperties) && 
                !(manager->createOptions & kIOHIDManagerOptionDoNotLoadProperties)) {
                __IOHIDDeviceLoadProperties(device);
            }
            if (manager->properties) {
                CFDictionaryApplyFunction(manager->properties, 
                                          __IOHIDApplyPropertiesToDeviceFromDictionary, 
                                          device);
            }
        }
        
        IOObjectRelease(service);
    }
    
    // Dispatch initial enumeration callback on runLoop
    if ( initial ) {
        
        CFRunLoopSourceContext  context;
        
        bzero(&context, sizeof(CFRunLoopSourceContext));
        
        context.info    = manager;
        context.perform = __IOHIDManagerInitialEnumCallback;
        
        manager->initEnumRunLoopSource = CFRunLoopSourceCreate( 
                                                        CFGetAllocator(manager),
                                                        0, 
                                                        &context);
                                                        
        if ( manager->runLoop && manager->initEnumRunLoopSource ) {
            CFRunLoopAddSource(
                        manager->runLoop, 
                        manager->initEnumRunLoopSource, 
                        manager->runLoopMode);
                        
            CFRunLoopSourceSignal(manager->initEnumRunLoopSource);
        }
    }
}

//------------------------------------------------------------------------------
// __IOHIDManagerDeviceRemoved
//------------------------------------------------------------------------------
void __IOHIDManagerDeviceRemoved(void *refcon,
                                 io_service_t service,
                                 uint32_t messageType,
                                 void *messageArgument __unused)
{
    IOHIDManagerRef manager = (IOHIDManagerRef)refcon;
    __block IOHIDDeviceRef sender = NULL;
    uint64_t regID = 0;
    
    IORegistryEntryGetRegistryEntryID(service, &regID);
    
    if (messageType != kIOMessageServiceIsTerminated) {
        return;
    }
    
    os_unfair_lock_lock(&manager->deviceLock);
    _IOHIDCFSetApplyBlock(manager->devices, ^(CFTypeRef value) {
        IOHIDDeviceRef device = (IOHIDDeviceRef)value;
        
        if (!sender) {
            uint64_t deviceID = IOHIDDeviceGetRegistryEntryID(device);
            
            if (regID == deviceID) {
                sender = device;
            }
        }
    });
    os_unfair_lock_unlock(&manager->deviceLock);
    
    if (!sender) {
        return;
    }
    
    os_unfair_lock_lock(&manager->deviceLock);
    CFDictionaryRemoveValue(manager->removalNotifiers, sender);
    os_unfair_lock_unlock(&manager->deviceLock);
    
    if ( manager->deviceInputBuffers )
        CFDictionaryRemoveValue(manager->deviceInputBuffers, sender);
        
    if ( (manager->createOptions & kIOHIDManagerOptionUsePersistentProperties) && 
        !(manager->createOptions & kIOHIDManagerOptionDoNotSaveProperties)) {
        if (CFSetContainsValue(manager->devices, sender))
            __IOHIDDeviceSaveProperties((IOHIDDeviceRef)sender, NULL);
    }
    
    if (!(manager->createOptions &kIOHIDManagerOptionIndependentDevices) &&
        manager->dispatchQueue) {
        IOHIDDeviceCancel(sender);
        IOHIDDeviceActivate(sender);
    }
    
    if (manager->removalCallback &&
        (manager->runLoop || manager->dispatchStateMask & kIOHIDDispatchStateActive)) {
        (*manager->removalCallback)(manager->removalContext,
                                    kIOReturnSuccess,
                                    manager,
                                    sender);
    }
    
    os_unfair_lock_lock(&manager->deviceLock);
    CFSetRemoveValue(manager->devices, sender);
    os_unfair_lock_unlock(&manager->deviceLock);
}

//------------------------------------------------------------------------------
// __IOHIDManagerInitialEnumCallback
//------------------------------------------------------------------------------
void __IOHIDManagerInitialEnumCallback(void * info)
{
    IOHIDManagerRef manager = (IOHIDManagerRef)info;
    
    require(manager->matchCallback && manager->devices, exit);
    
    __ApplyToDevices(manager, kDeviceApplierInitEnumCallback);
    
exit:
    // After we have dispatched all of the enum callbacks, kill the source
    if (manager->runLoop) {
        CFRunLoopRemoveSource(manager->runLoop,
                              manager->initEnumRunLoopSource, 
                              manager->runLoopMode);
    }

    CFRelease(manager->initEnumRunLoopSource);
    manager->initEnumRunLoopSource = NULL;
    
    if (manager->initRetVals) {
        CFRelease(manager->initRetVals);
        manager->initRetVals = NULL;
    }
}

//------------------------------------------------------------------------------
// __ApplyToDevices
//------------------------------------------------------------------------------
IOReturn __ApplyToDevices(IOHIDManagerRef manager, IOOptionBits options)
{
    CFSetRef devices = NULL;
    IOReturn ret = kIOReturnError;
    DeviceApplierArgs args = { manager, options, kIOReturnSuccess };
    
    require_action(manager->devices, exit, ret = kIOReturnNoDevice);
    
    os_unfair_lock_lock(&manager->deviceLock);
    devices = CFSetCreateCopy(CFGetAllocator(manager), manager->devices);
    os_unfair_lock_unlock(&manager->deviceLock);
    
    require(devices, exit);
    
    CFSetApplyFunction(devices, __IOHIDManagerDeviceApplier, &args);
    ret = args.retVal;
    
    CFRelease(devices);
    
exit:
    return ret;
}

//------------------------------------------------------------------------------
// __IOHIDManagerDeviceApplier
//------------------------------------------------------------------------------
void __IOHIDManagerDeviceApplier(
                                    const void *                value,
                                    void *                      context)
{
    DeviceApplierArgs * args    = (DeviceApplierArgs*)context;
    IOHIDDeviceRef      device  = (IOHIDDeviceRef)value;
    IOHIDManagerRef     manager = args->manager;
    intptr_t            retVal  = kIOReturnSuccess;
    
    require_quiet((manager->createOptions & kIOHIDManagerOptionIndependentDevices) == 0, exit);
    
    if ( args->options & kDeviceApplierOpen ) {
        retVal = IOHIDDeviceOpen(           device,
                                            args->manager->openOptions);
        if ( args->manager->initRetVals )
            CFDictionarySetValue(args->manager->initRetVals, device, (void*)retVal);
    }

    if ( args->options & kDeviceApplierClose )
        retVal = IOHIDDeviceClose(          device,
                                            args->manager->openOptions);

    if ( args->options & kDeviceApplierSetInputMatching )
        IOHIDDeviceSetInputValueMatchingMultiple( 
                                            device,
                                            args->manager->inputMatchingMultiple);
    
    if ( args->options & kDeviceApplierSetInputCallback )
        IOHIDDeviceRegisterInputValueCallback( 
                                            device,
                                            args->manager->inputCallback,
                                            args->manager->inputContext);

    if ( args->options & kDeviceApplierSetInputReportCallback ||
         args->options & kDeviceApplierSetInputTSReportCallback ) {
        CFMutableDataRef dataRef = NULL;
        if ( !args->manager->deviceInputBuffers ) {
            args->manager->deviceInputBuffers = CFDictionaryCreateMutable(CFGetAllocator(args->manager), 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        }

        dataRef = (CFMutableDataRef)CFDictionaryGetValue(args->manager->deviceInputBuffers, device);
        if ( !dataRef ) {
            CFNumberRef number = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDMaxInputReportSizeKey));
            CFIndex     length = 64;
            
            if ( number ) {
                CFNumberGetValue(number, kCFNumberCFIndexType, &length);
            }
            
            dataRef = CFDataCreateMutable(CFGetAllocator(args->manager), length);
            if ( dataRef ) {
                CFDataSetLength(dataRef, length);
                CFDictionarySetValue(args->manager->deviceInputBuffers, device, dataRef);
                CFRelease(dataRef);
            }
        }
        
        if (args->options & kDeviceApplierSetInputReportCallback) {
            IOHIDDeviceRegisterInputReportCallback(device,
                                                   (uint8_t *)CFDataGetMutableBytePtr(dataRef),
                                                   CFDataGetLength(dataRef),
                                                   args->manager->reportCallback,
                                                   args->manager->reportContext);
        } else {
            IOHIDDeviceRegisterInputReportWithTimeStampCallback(device,
                                                                (uint8_t *)CFDataGetMutableBytePtr(dataRef),
                                                                CFDataGetLength(dataRef),
                                                                args->manager->reportTimestampCallback,
                                                                args->manager->reportContext);
        }
    }
    
    if ( args->options & kDeviceApplierScheduleRunLoop )
        IOHIDDeviceScheduleWithRunLoop(     device,
                                            args->manager->runLoop,
                                            args->manager->runLoopMode);

    if ( args->options & kDeviceApplierUnscheduleRunLoop )
        IOHIDDeviceUnscheduleFromRunLoop(   device,
                                            args->manager->runLoop,
                                            args->manager->runLoopMode);
    
    if (args->options & kDeviceApplierSetDispatchQueue) {
        IOHIDDeviceSetDispatchQueue(device, manager->dispatchQueue);
    }
    
    if (args->options & kDeviceApplierSetCancelHandler) {
        manager->cancelCount++;
        
        IOHIDDeviceSetCancelHandler(device, ^{
            manager->cancelCount--;
            
            // once all of our devices have been cancelled
            // we call our manager's cancel handler
            if ((manager->dispatchStateMask & kIOHIDDispatchStateCancelled)
                && !manager->cancelCount) {
                if (manager->cancelHandler) {
                    (manager->cancelHandler)();
                    Block_release(manager->cancelHandler);
                    manager->cancelHandler = NULL;
                    _IOHIDObjectInternalRelease(manager);
                }
            }
        });
    }
    
    if (args->options & kDeviceApplierActivate) {
        IOHIDDeviceActivate(device);
    }
    
    if (args->options & kDeviceApplierCancel) {
        IOHIDDeviceCancel(device);
    }
    
    if ( args->retVal == kIOReturnSuccess && retVal != kIOReturnSuccess )
        args->retVal = retVal;
    
exit:
    if (args->options & kDeviceApplierInitEnumCallback) {
        if (args->manager->initRetVals) {
            retVal = (intptr_t)CFDictionaryGetValue(args->manager->initRetVals,
                                                    device);
        }
        
        (*args->manager->matchCallback)(args->manager->matchContext,
                                        retVal,
                                        args->manager,
                                        device);
    }
}

//------------------------------------------------------------------------------
// IOHIDManagerGetTypeID
//------------------------------------------------------------------------------
CFTypeID IOHIDManagerGetTypeID(void)
{
    if ( _kCFRuntimeNotATypeID == __kIOHIDManagerTypeID )
        pthread_once(&__sessionTypeInit, __IOHIDManagerRegister);
        
    return __kIOHIDManagerTypeID;
}

//------------------------------------------------------------------------------
// IOHIDManagerCreate
//------------------------------------------------------------------------------
IOHIDManagerRef IOHIDManagerCreate(     
                                    CFAllocatorRef          allocator,
                                    IOOptionBits            options)
{    
    IOHIDManagerRef manager;
    
    manager = __IOHIDManagerCreate(allocator, NULL);
    
    if (!manager)
        return (_Nonnull IOHIDManagerRef)NULL;
    
    manager->deviceLock = OS_UNFAIR_LOCK_INIT;
    manager->createOptions = options;
    
    if ( (manager->createOptions & kIOHIDManagerOptionUsePersistentProperties) && 
        !(manager->createOptions & kIOHIDManagerOptionDoNotLoadProperties))
    {
        __IOHIDManagerLoadProperties(manager);
    }
    
    manager->stateQueue = dispatch_queue_create("IOHIDManagerStateQueue", DISPATCH_QUEUE_SERIAL);
    require(manager->stateQueue, error);
    
    manager->stateHandler = os_state_add_handler(manager->stateQueue,
                                                ^os_state_data_t(os_state_hints_t hints) {
                                                    return __IOHIDManagerStateHandler(manager, hints);
                                                });
error:
    return manager;
}

//------------------------------------------------------------------------------
// IOHIDManagerOpen
//------------------------------------------------------------------------------
IOReturn IOHIDManagerOpen(
                                IOHIDManagerRef                 manager,
                                IOOptionBits                    options)
{
    IOReturn ret = kIOReturnSuccess;
    
    require(!manager->isOpen, exit);
    
    manager->isOpen = true;
    manager->openOptions = options;
    
    if (manager->devices) {
        IOOptionBits deviceOptions = kDeviceApplierOpen;
        
        if (manager->inputMatchingMultiple) {
            deviceOptions |= kDeviceApplierSetInputMatching;
        }
        
        if (manager->inputCallback) {
            deviceOptions |= kDeviceApplierSetInputCallback;
        }
        
        if (manager->reportCallback) {
            deviceOptions |= kDeviceApplierSetInputReportCallback;
        }
        
        if (manager->reportTimestampCallback) {
            deviceOptions |= kDeviceApplierSetInputTSReportCallback;
        }
        
        ret = __ApplyToDevices(manager, deviceOptions);
    }
    
exit:
    return ret;
}
                                
//------------------------------------------------------------------------------
// IOHIDManagerClose
//------------------------------------------------------------------------------
IOReturn IOHIDManagerClose(
                                IOHIDManagerRef                 manager,
                                IOOptionBits                    options)
{
    IOReturn ret = kIOReturnError;
    
    if (manager->runLoop) {
        IOHIDManagerUnscheduleFromRunLoop(manager, manager->runLoop, manager->runLoopMode);
    }
    
    require_action(manager->isOpen, exit, ret = kIOReturnNotOpen);
    
    manager->isOpen = false;
    manager->openOptions = options;
    
    if (manager->devices) {
        ret = __ApplyToDevices(manager, kDeviceApplierClose);
    } else {
        ret = kIOReturnSuccess;
    }
    
exit:
    if ( (manager->createOptions & kIOHIDManagerOptionUsePersistentProperties) && 
        !(manager->createOptions & kIOHIDManagerOptionDoNotSaveProperties)) {
        __IOHIDManagerSaveProperties(manager, NULL);
    }
    
    return ret;
}

//------------------------------------------------------------------------------
// IOHIDManagerGetProperty
//------------------------------------------------------------------------------
CFTypeRef IOHIDManagerGetProperty(
                                IOHIDManagerRef                 manager,
                                CFStringRef                     key)
{
    if ( !manager->properties )
        return NULL;
        
    return CFDictionaryGetValue(manager->properties, key);
}
                                
//------------------------------------------------------------------------------
// IOHIDManagerSetProperty
//------------------------------------------------------------------------------
Boolean IOHIDManagerSetProperty(
                                IOHIDManagerRef                 manager,
                                CFStringRef                     key,
                                CFTypeRef                       value)
{
    CFSetRef devices = NULL;
    bool result = false;
    __IOHIDApplyPropertyToSetContext context = { key, value };
    
    if (!manager->properties) {
        manager->properties = CFDictionaryCreateMutable(CFGetAllocator(manager),
                                                        0,
                                                        &kCFTypeDictionaryKeyCallBacks,
                                                        &kCFTypeDictionaryValueCallBacks);
        require(manager->properties, exit);
    }
    
    manager->isDirty = TRUE;
    CFDictionarySetValue(manager->properties, key, value);
    
    require_action(manager->devices, exit, result = true);
    
    os_unfair_lock_lock(&manager->deviceLock);
    devices = CFSetCreateCopy(CFGetAllocator(manager), manager->devices);
    os_unfair_lock_unlock(&manager->deviceLock);
    
    require(devices, exit);
    
    CFSetApplyFunction(devices, __IOHIDApplyPropertyToDeviceSet, &context);
    CFRelease(devices);
    
    result = true;
    
exit:
    return result;
}
                                        
//------------------------------------------------------------------------------
// IOHIDManagerSetDeviceMatching
//------------------------------------------------------------------------------
void IOHIDManagerSetDeviceMatching(
                                IOHIDManagerRef                 manager,
                                CFDictionaryRef                 matching)
{
    CFArrayRef multiple = NULL;
    
    os_assert(manager->dispatchStateMask == kIOHIDDispatchStateInactive, "Manager has already been activated/cancelled.");
      
    if ( matching ) 
        multiple = CFArrayCreate(CFGetAllocator(manager),
                                (const void **)&matching,
                                1,
                                &kCFTypeArrayCallBacks);

    IOHIDManagerSetDeviceMatchingMultiple(manager, multiple);
                                        
    if ( multiple )
        CFRelease(multiple);
}

//------------------------------------------------------------------------------
// IOHIDManagerSetDeviceMatchingMultiple
//------------------------------------------------------------------------------
void IOHIDManagerSetDeviceMatchingMultiple(
                                IOHIDManagerRef                 manager,
                                CFArrayRef                      multiple)
{
    CFIndex         i, count;
    CFTypeRef       value;
    
    os_assert(manager->dispatchStateMask == kIOHIDDispatchStateInactive, "Manager has already been activated/cancelled.");
    
    if ( manager->devices ) {
        if ( (manager->createOptions & kIOHIDManagerOptionUsePersistentProperties) && 
            !(manager->createOptions & kIOHIDManagerOptionDoNotSaveProperties)) {
            __IOHIDManagerSaveProperties(manager, NULL);
        }
        
        os_unfair_lock_lock(&manager->deviceLock);
        CFSetRemoveAllValues(manager->devices);
        os_unfair_lock_unlock(&manager->deviceLock);
    }
    
    if (manager->removalNotifiers) {
        os_unfair_lock_lock(&manager->deviceLock);
        CFDictionaryRemoveAllValues(manager->removalNotifiers);
        os_unfair_lock_unlock(&manager->deviceLock);
    }
        
    if (manager->iterators) {
        os_unfair_lock_lock(&manager->deviceLock);
        CFSetRemoveAllValues(manager->iterators);
        os_unfair_lock_unlock(&manager->deviceLock);
    }

    if ( manager->deviceInputBuffers )
        CFDictionaryRemoveAllValues(manager->deviceInputBuffers);
    
    if ( multiple ) {
        count = CFArrayGetCount(multiple);
        for ( i=0; i<count; i++ ) {
            value = CFArrayGetValueAtIndex(multiple, i);
            if (CFDictionaryGetTypeID() == CFGetTypeID(value))
                __IOHIDManagerSetDeviceMatching(manager, (CFDictionaryRef)value);
        }
    }
    else 
        __IOHIDManagerSetDeviceMatching(manager, NULL);

}

//------------------------------------------------------------------------------
// IOHIDManagerCopyDevices
//------------------------------------------------------------------------------
CFSetRef IOHIDManagerCopyDevices(
                                IOHIDManagerRef                 manager)
{
    CFSetRef devices = NULL;
    
    require(manager->devices, exit);
    
    os_unfair_lock_lock(&manager->deviceLock);
    devices = CFSetCreateCopy(CFGetAllocator(manager), manager->devices);
    os_unfair_lock_unlock(&manager->deviceLock);
    
exit:
    return devices;
}

//------------------------------------------------------------------------------
// IOHIDManagerRegisterDeviceMatchingCallback
//------------------------------------------------------------------------------
void IOHIDManagerRegisterDeviceMatchingCallback(
                                IOHIDManagerRef                 manager,
                                IOHIDDeviceCallback             callback,
                                void *                          context)
{
    os_assert(manager->dispatchStateMask == kIOHIDDispatchStateInactive, "Manager has already been activated/cancelled.");
    
    manager->matchCallback  = callback;
    manager->matchContext   = context;
}

//------------------------------------------------------------------------------
// IOHIDManagerRegisterDeviceRemovalCallback
//------------------------------------------------------------------------------
void IOHIDManagerRegisterDeviceRemovalCallback(
                                IOHIDManagerRef                 manager,
                                IOHIDDeviceCallback             callback,
                                void *                          context)
{
    os_assert(manager->dispatchStateMask == kIOHIDDispatchStateInactive, "Manager has already been activated/cancelled.");
    
    manager->removalCallback    = callback;
    manager->removalContext     = context;

}
                                        
//------------------------------------------------------------------------------
// IOHIDManagerRegisterInputReportCallback
//------------------------------------------------------------------------------
void IOHIDManagerRegisterInputReportCallback( 
                                    IOHIDManagerRef             manager,
                                    IOHIDReportCallback         callback,
                                    void *                      context)
{
    os_assert(manager->dispatchStateMask == kIOHIDDispatchStateInactive, "Manager has already been activated/cancelled.");
    
    manager->reportCallback  = callback;
    manager->reportContext   = context;
    
    require(manager->isOpen && manager->devices, exit);
    
    __ApplyToDevices(manager, kDeviceApplierSetInputReportCallback);
    
exit:
    return;
}

//------------------------------------------------------------------------------
// IOHIDManagerRegisterInputReportWithTimeStampCallback
//------------------------------------------------------------------------------
void IOHIDManagerRegisterInputReportWithTimeStampCallback(
                                    IOHIDManagerRef                    manager,
                                    IOHIDReportWithTimeStampCallback   callback,
                                    void *                             context)
{
    os_assert(manager->dispatchStateMask == kIOHIDDispatchStateInactive, "Manager has already been activated/cancelled.");
    
    manager->reportTimestampCallback = callback;
    manager->reportContext = context;
    
    require(manager->isOpen && manager->devices, exit);
    
    __ApplyToDevices(manager, kDeviceApplierSetInputTSReportCallback);
    
exit:
    return;
}

//------------------------------------------------------------------------------
// IOHIDManagerRegisterInputValueCallback
//------------------------------------------------------------------------------
void IOHIDManagerRegisterInputValueCallback( 
                                    IOHIDManagerRef             manager,
                                    IOHIDValueCallback          callback,
                                    void *                      context)
{
    os_assert(manager->dispatchStateMask == kIOHIDDispatchStateInactive, "Manager has already been activated/cancelled.");
    
    manager->inputCallback  = callback;
    manager->inputContext   = context;
    
    require(manager->isOpen && manager->devices, exit);
    
    __ApplyToDevices(manager, kDeviceApplierSetInputCallback);
    
exit:
    return;
}

//------------------------------------------------------------------------------
// IOHIDManagerSetInputValueMatching
//------------------------------------------------------------------------------
void IOHIDManagerSetInputValueMatching( 
                                    IOHIDManagerRef             manager,
                                    CFDictionaryRef             matching)
{
    os_assert(manager->dispatchStateMask == kIOHIDDispatchStateInactive, "Manager has already been activated/cancelled.");
    
    if ( matching ) {
        CFArrayRef multiple = CFArrayCreate(CFGetAllocator(manager), (const void **)&matching, 1, &kCFTypeArrayCallBacks);
        
        IOHIDManagerSetInputValueMatchingMultiple(manager, multiple);
        
        CFRelease(multiple);
    } else {
        IOHIDManagerSetInputValueMatchingMultiple(manager, NULL);
    }
}

//------------------------------------------------------------------------------
// IOHIDManagerSetInputValueMatchingMultiple
//------------------------------------------------------------------------------
void IOHIDManagerSetInputValueMatchingMultiple( 
                                    IOHIDManagerRef             manager,
                                    CFArrayRef                  multiple)
{
    os_assert(manager->dispatchStateMask == kIOHIDDispatchStateInactive, "Manager has already been activated/cancelled.");
    
    if ( manager->inputMatchingMultiple )
        CFRelease(manager->inputMatchingMultiple);
        
    if ( multiple )
        CFRetain(multiple);
        
    manager->inputMatchingMultiple = multiple;
    
    require(manager->isOpen && manager->devices, exit);
    
    __ApplyToDevices(manager, kDeviceApplierSetInputMatching);
    
exit:
    return;
}

//------------------------------------------------------------------------------
// IOHIDManagerScheduleWithRunLoop
//------------------------------------------------------------------------------
void IOHIDManagerScheduleWithRunLoop(
                                        IOHIDManagerRef         manager,
                                        CFRunLoopRef            runLoop, 
                                        CFStringRef             runLoopMode)
{
    os_assert(!manager->runLoop && !manager->dispatchQueue,
              "Schedule failed queue: %p runLoop: %p", manager->dispatchQueue, manager->runLoop);
    
    manager->runLoop        = runLoop;
    manager->runLoopMode    = runLoopMode;

    if ( manager->runLoop ) {
        // Schedule the notifyPort
        if (manager->notifyPort) {
            CFRunLoopSourceRef source = IONotificationPortGetRunLoopSource(manager->notifyPort);
            if (source) {
                CFRunLoopAddSource(
                            manager->runLoop,
                            source,
                            manager->runLoopMode);
            }
        }

        // schedule the initial enumeration routine
        if ( manager->initEnumRunLoopSource ) {
            CFRunLoopAddSource(
                        manager->runLoop, 
                        manager->initEnumRunLoopSource, 
                        manager->runLoopMode);

            CFRunLoopSourceSignal(manager->initEnumRunLoopSource);
        }
        
        // Schedule the devices
        __ApplyToDevices(manager, kDeviceApplierScheduleRunLoop);
    }
}

//------------------------------------------------------------------------------
// IOHIDManagerUnscheduleFromRunLoop
//------------------------------------------------------------------------------
void IOHIDManagerUnscheduleFromRunLoop(
                                        IOHIDManagerRef         manager,
                                        CFRunLoopRef            runLoop, 
                                        CFStringRef             runLoopMode)
{
    if (!manager->runLoop)
        return;
    
    if (!CFEqual(manager->runLoop, runLoop) || 
        !CFEqual(manager->runLoopMode, runLoopMode)) 
        return;
    
    // Unschedule the devices
    __ApplyToDevices(manager, kDeviceApplierUnscheduleRunLoop);
    
    // Unschedule the initial enumeration routine
    if (manager->initEnumRunLoopSource) {
        CFRunLoopSourceInvalidate(manager->initEnumRunLoopSource);
        CFRunLoopRemoveSource(manager->runLoop,
                              manager->initEnumRunLoopSource,
                              manager->runLoopMode);
        CFRelease(manager->initEnumRunLoopSource);
        manager->initEnumRunLoopSource = NULL;
    }
    
    // stop receiving device notifications
    if (manager->iterators) {
        os_unfair_lock_lock(&manager->deviceLock);
        CFSetRemoveAllValues(manager->iterators);
        os_unfair_lock_unlock(&manager->deviceLock);
    }
    
    if (manager->removalNotifiers) {
        os_unfair_lock_lock(&manager->deviceLock);
        CFDictionaryRemoveAllValues(manager->removalNotifiers);
        os_unfair_lock_unlock(&manager->deviceLock);
    }
    
    manager->runLoop        = NULL;
    manager->runLoopMode    = NULL;
}

//------------------------------------------------------------------------------
// IOHIDManagerSetDispatchQueue
//------------------------------------------------------------------------------

void IOHIDManagerSetDispatchQueue(IOHIDManagerRef manager, dispatch_queue_t queue)
{
    os_assert(!manager->runLoop && !manager->dispatchQueue);
    
    manager->dispatchQueue = dispatch_queue_create_with_target("IOHIDManagerDispatchQueue", DISPATCH_QUEUE_SERIAL, queue);
    require(manager->dispatchQueue, exit);
    
    __ApplyToDevices(manager, kDeviceApplierSetDispatchQueue);
    
exit:
    return;
}

//------------------------------------------------------------------------------
// IOHIDManagerSetCancelHandler
//------------------------------------------------------------------------------
void IOHIDManagerSetCancelHandler(IOHIDManagerRef manager, dispatch_block_t handler)
{
    os_assert(!manager->cancelHandler && handler);
    
    _IOHIDObjectInternalRetain(manager);
    manager->cancelHandler = Block_copy(handler);
    
    __ApplyToDevices(manager, kDeviceApplierSetCancelHandler);
}

//------------------------------------------------------------------------------
// IOHIDManagerActivate
//------------------------------------------------------------------------------
void IOHIDManagerActivate(IOHIDManagerRef manager)
{
    IOOptionBits deviceOptions = 0;
    
    os_assert(manager->dispatchQueue && !manager->runLoop,
              "Activate failed queue: %p runLoop: %p", manager->dispatchQueue, manager->runLoop);
    
    if (atomic_fetch_or(&manager->dispatchStateMask, kIOHIDDispatchStateActive) & kIOHIDDispatchStateActive) {
        return;
    }
    
    if (manager->notifyPort) {
        IONotificationPortSetDispatchQueue(manager->notifyPort, manager->dispatchQueue);
    }
    
    deviceOptions = kDeviceApplierActivate;
    
    if (manager->matchCallback) {
        deviceOptions |= kDeviceApplierInitEnumCallback;
    }
    
    __ApplyToDevices(manager, deviceOptions);
}

//------------------------------------------------------------------------------
// IOHIDManagerCancel
//------------------------------------------------------------------------------
void IOHIDManagerCancel(IOHIDManagerRef manager)
{
    os_assert(manager->dispatchQueue && !manager->runLoop,
              "Cancel failed queue: %p runLoop: %p", manager->dispatchQueue, manager->runLoop);
    
    if (atomic_fetch_or(&manager->dispatchStateMask, kIOHIDDispatchStateCancelled) & kIOHIDDispatchStateCancelled) {
        return;
    }
    
    if (manager->notifyPort) {
        IONotificationPortDestroy(manager->notifyPort);
        manager->notifyPort = NULL;
    }
    
    // stop receiving device notifications
    if (manager->iterators) {
        os_unfair_lock_lock(&manager->deviceLock);
        CFSetRemoveAllValues(manager->iterators);
        os_unfair_lock_unlock(&manager->deviceLock);
    }
    
    if (manager->removalNotifiers) {
        os_unfair_lock_lock(&manager->deviceLock);
        CFDictionaryRemoveAllValues(manager->removalNotifiers);
        os_unfair_lock_unlock(&manager->deviceLock);
    }
    
    if (manager->devices &&
        CFSetGetCount(manager->devices) &&
        manager->cancelCount) {
        __ApplyToDevices(manager, kDeviceApplierCancel);
    } else if (manager->cancelHandler) {
        (manager->cancelHandler)();
        Block_release(manager->cancelHandler);
        manager->cancelHandler = NULL;
        _IOHIDObjectInternalRelease(manager);
    }
}

//------------------------------------------------------------------------------
// IOHIDManagerSaveToPropertyDomain
//------------------------------------------------------------------------------
void IOHIDManagerSaveToPropertyDomain(IOHIDManagerRef                 manager,
                                      CFStringRef                     applicationID,
                                      CFStringRef                     userName,
                                      CFStringRef                     hostName,
                                      IOOptionBits                    options)
{
    __IOHIDPropertyContext context = {
        applicationID, userName, hostName, options
    };
    
    if (manager && applicationID && userName && hostName) {
        __IOHIDManagerSaveProperties(manager, &context);        
    }
    else {
        // LOG AN ERROR?
    }
}

//------------------------------------------------------------------------------
CFStringRef __IOHIDManagerGetRootKey() 
{
    return CFSTR(kIOHIDManagerKey);
}

//------------------------------------------------------------------------------
void __IOHIDManagerSaveProperties(IOHIDManagerRef manager, __IOHIDPropertyContext *context)
{
    if (manager->isDirty && manager->properties) {
        __IOHIDPropertySaveToKeyWithSpecialKeys(manager->properties, 
                                               __IOHIDManagerGetRootKey(),
                                               NULL,
                                               context);
        manager->isDirty = FALSE;
    }
    
    if (manager->devices) {
        CFSetRef devices = NULL;
        
        os_unfair_lock_lock(&manager->deviceLock);
        devices = CFSetCreateCopy(CFGetAllocator(manager), manager->devices);
        os_unfair_lock_unlock(&manager->deviceLock);
        
        require(devices, exit);
        
        CFSetApplyFunction(devices, __IOHIDSaveDeviceSet, context);
        CFRelease(devices);
    }
    
exit:
    return;
}

//------------------------------------------------------------------------------
void __IOHIDManagerLoadProperties(IOHIDManagerRef manager)
{
    // Convert to __IOHIDPropertyLoadFromKeyWithSpecialKeys if we identify special keys
    CFMutableDictionaryRef properties = __IOHIDPropertyLoadDictionaryFromKey(__IOHIDManagerGetRootKey());
    
    if (properties) {
        CFRELEASE_IF_NOT_NULL(manager->properties);
        manager->properties = properties;
        manager->isDirty = FALSE;
    }
    
    // We do not load device properties here, since the devices are not present when this is called.
}

//------------------------------------------------------------------------------
void __IOHIDPropertySaveWithContext(CFStringRef key, CFPropertyListRef value, __IOHIDPropertyContext *context)
{
    if (key && value) {
        if (context && context->applicationID && context->userName && context->hostName) {
            CFPreferencesSetValue(key, value, context->applicationID, context->userName, context->hostName);
        }
        else {
            CFPreferencesSetAppValue(key, value, kCFPreferencesCurrentApplication);
        }
    }
}

//------------------------------------------------------------------------------
void __IOHIDPropertySaveToKeyWithSpecialKeys(CFDictionaryRef dictionary, CFStringRef key, CFStringRef *specialKeys, __IOHIDPropertyContext *context)
{
    CFMutableDictionaryRef temp = CFDictionaryCreateMutableCopy(NULL, 0, dictionary);
    
    if (specialKeys) {
        while (*specialKeys) {
            CFTypeRef value = CFDictionaryGetValue(temp, *specialKeys);
            if (value) {
                CFStringRef subKey = CFStringCreateWithFormat(NULL, NULL, 
                                                              CFSTR("%@#%@"), 
                                                              key,
                                                              *specialKeys);
                __IOHIDPropertySaveWithContext(subKey, value, context);
                CFDictionaryRemoveValue(temp, *specialKeys);
                CFRelease(subKey);
            }
            specialKeys++;
        }
    }
    
    CFAbsoluteTime time = CFAbsoluteTimeGetCurrent();
    CFNumberRef timeNum = CFNumberCreate(NULL, kCFNumberDoubleType, &time);
    CFDictionaryAddValue(temp, CFSTR("time of last save"), timeNum);
    CFRelease(timeNum);
    __IOHIDPropertySaveWithContext(key, temp, context);
    CFRelease(temp);
}

//------------------------------------------------------------------------------
CFMutableDictionaryRef __IOHIDPropertyLoadDictionaryFromKey(CFStringRef key)
{
    CFMutableDictionaryRef result = NULL;
    CFDictionaryRef baseProperties = CFPreferencesCopyAppValue(key, kCFPreferencesCurrentApplication);
    if (baseProperties && (CFGetTypeID(baseProperties) == CFDictionaryGetTypeID())) {
        result = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        
        CFDictionaryRef properties = CFPreferencesCopyValue(key, kCFPreferencesAnyApplication, kCFPreferencesAnyUser, kCFPreferencesAnyHost);
        if (properties && (CFGetTypeID(properties) == CFDictionaryGetTypeID()))
            __IOHIDManagerMergeDictionaries(properties, result);
        if (properties)
            CFRelease(properties);
        
        properties = CFPreferencesCopyValue(key, kCFPreferencesAnyApplication, kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
        if (properties && (CFGetTypeID(properties) == CFDictionaryGetTypeID()))
            __IOHIDManagerMergeDictionaries(properties, result);
        if (properties)
            CFRelease(properties);
        
        properties = CFPreferencesCopyValue(key, kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
        if (properties && (CFGetTypeID(properties) == CFDictionaryGetTypeID()))
            __IOHIDManagerMergeDictionaries(properties, result);
        if (properties)
            CFRelease(properties);
        
        properties = CFPreferencesCopyValue(key, kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
        if (properties && (CFGetTypeID(properties) == CFDictionaryGetTypeID()))
            __IOHIDManagerMergeDictionaries(properties, result);
        if (properties)
            CFRelease(properties);
        
        properties = CFPreferencesCopyValue(key, kCFPreferencesCurrentApplication, kCFPreferencesAnyUser, kCFPreferencesAnyHost);
        if (properties && (CFGetTypeID(properties) == CFDictionaryGetTypeID()))
            __IOHIDManagerMergeDictionaries(properties, result);
        if (properties)
            CFRelease(properties);
        
        properties = CFPreferencesCopyValue(key, kCFPreferencesCurrentApplication, kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
        if (properties && (CFGetTypeID(properties) == CFDictionaryGetTypeID()))
            __IOHIDManagerMergeDictionaries(properties, result);
        if (properties)
            CFRelease(properties);
        
        properties = CFPreferencesCopyValue(key, kCFPreferencesCurrentApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
        if (properties && (CFGetTypeID(properties) == CFDictionaryGetTypeID()))
            __IOHIDManagerMergeDictionaries(properties, result);
        if (properties)
            CFRelease(properties);
        
        properties = CFPreferencesCopyValue(key, kCFPreferencesCurrentApplication, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
        if (properties && (CFGetTypeID(properties) == CFDictionaryGetTypeID()))
            __IOHIDManagerMergeDictionaries(properties, result);
        if (properties)
            CFRelease(properties);
        
        __IOHIDManagerMergeDictionaries(baseProperties, result);
    }
    if (baseProperties)
        CFRelease(baseProperties);
    return result;
}

//------------------------------------------------------------------------------
CFMutableDictionaryRef __IOHIDPropertyLoadFromKeyWithSpecialKeys(CFStringRef key, CFStringRef *specialKeys)
{
    CFMutableDictionaryRef result = __IOHIDPropertyLoadDictionaryFromKey(key);
    
    if (!result)
        result = CFDictionaryCreateMutable(NULL, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    while (*specialKeys) {
        CFStringRef subKey = CFStringCreateWithFormat(NULL, NULL, 
                                                      CFSTR("%@#%@"), 
                                                      key,
                                                      *specialKeys);
        CFPropertyListRef value = CFPreferencesCopyAppValue(subKey, kCFPreferencesCurrentApplication);
        if (value) {
            CFDictionarySetValue(result, *specialKeys, value);
            CFRelease(value);
        }
        
        CFRelease(subKey);
        specialKeys++;
    }
    
    return result;
}

//===========================================================================
// Static Helper Definitions
//===========================================================================
//------------------------------------------------------------------------------
// __IOHIDManagerMergeDictionaries
//------------------------------------------------------------------------------
void __IOHIDManagerMergeDictionaries(CFDictionaryRef        srcDict, 
                                     CFMutableDictionaryRef dstDict)
{
    uint32_t        count;
    CFTypeRef *     values;
    CFStringRef *   keys;
    
    if ( !dstDict || !srcDict || !(count = CFDictionaryGetCount(srcDict)))
        return;
        
    values  = (CFTypeRef *)malloc(sizeof(CFTypeRef) * count);
    keys    = (CFStringRef *)malloc(sizeof(CFStringRef) * count);
    
    if ( values && keys ) {
        CFDictionaryGetKeysAndValues(srcDict, (const void **)keys, (const void **)values);
        
        for ( uint32_t i=0; i<count; i++) 
            CFDictionarySetValue(dstDict, keys[i], values[i]);
    }
        
    if ( values )
        free(values);
        
    if ( keys )
        free(keys);
}

//------------------------------------------------------------------------------
// __IOHIDManagerFinalizeStateHandler
//------------------------------------------------------------------------------
void __IOHIDManagerFinalizeStateHandler (void *context)
{
    IOHIDManagerRef manager = (IOHIDManagerRef)context;
    _IOHIDObjectInternalRelease(manager);
}

//------------------------------------------------------------------------------
// __IOHIDManagerStateHandler
//------------------------------------------------------------------------------
os_state_data_t __IOHIDManagerStateHandler (IOHIDManagerRef device, os_state_hints_t hints)
{
    os_state_data_t stateData = NULL;
    CFMutableDictionaryRef deviceState = NULL;
    CFDataRef serializedDeviceState = NULL;
    
    if (hints->osh_api != OS_STATE_API_FAULT &&
        hints->osh_api != OS_STATE_API_REQUEST) {
        return NULL;
    }
    
    deviceState = __IOHIDManagerSerializeState(device);
    require(deviceState, exit);
    
    serializedDeviceState = CFPropertyListCreateData(kCFAllocatorDefault, deviceState, kCFPropertyListBinaryFormat_v1_0, 0, NULL);
    require(serializedDeviceState, exit);
    
    uint32_t serializedDeviceStateSize = (uint32_t)CFDataGetLength(serializedDeviceState);
    stateData = calloc(1, OS_STATE_DATA_SIZE_NEEDED(serializedDeviceStateSize));
    require(stateData, exit);
    
    strlcpy(stateData->osd_title, "IOHIDManager State", sizeof(stateData->osd_title));
    stateData->osd_type = OS_STATE_DATA_SERIALIZED_NSCF_OBJECT;
    stateData->osd_data_size = serializedDeviceStateSize;
    CFDataGetBytes(serializedDeviceState, CFRangeMake(0, serializedDeviceStateSize), stateData->osd_data);
    
exit:
    if (deviceState) {
        CFRelease(deviceState);
    }
    
    if (serializedDeviceState) {
        CFRelease(serializedDeviceState);
    }
    
    return stateData;
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceSerializeState
//------------------------------------------------------------------------------
CFMutableDictionaryRef __IOHIDManagerSerializeState(IOHIDManagerRef manager)
{
    CFMutableDictionaryRef state = NULL;
    CFSetRef devices = NULL;
    state = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                      0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);
    require(state, exit);
    
    
    CFDictionarySetValue(state, CFSTR("DispatchQueue"), manager->dispatchQueue ? kCFBooleanTrue : kCFBooleanFalse);
    CFDictionarySetValue(state, CFSTR("RunLoop"), manager->runLoop ? kCFBooleanTrue : kCFBooleanFalse);
    _IOHIDDictionaryAddSInt32(state, CFSTR("openOptions"), manager->openOptions);
    _IOHIDDictionaryAddSInt32(state, CFSTR("createOptions"), manager->createOptions);
    CFDictionarySetValue(state, CFSTR("isOpen"), manager->isOpen ? kCFBooleanTrue : kCFBooleanFalse);
    
    os_unfair_lock_lock(&manager->deviceLock);
    if (manager->devices) {
        devices = CFSetCreateCopy(CFGetAllocator(manager->devices), manager->devices);
    }
    os_unfair_lock_unlock(&manager->deviceLock);
    
    if (devices) {
        CFMutableArrayRef deviceList = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
        if (deviceList) {
            _IOHIDCFSetApplyBlock (devices, ^(CFTypeRef value) {
                IOHIDDeviceRef device =  (IOHIDDeviceRef) value;
                uint64_t regID  = 0;
                io_service_t service = IOHIDDeviceGetService (device);
                if (service) {
                    IORegistryEntryGetRegistryEntryID(service, &regID);
                    _IOHIDArrayAppendSInt64 (deviceList, regID);
                }
            });
            CFDictionarySetValue(state, CFSTR("devices"), deviceList);
            CFRelease(deviceList);
        }
        CFRelease(devices);
    }

exit:
    
    return state;
}
