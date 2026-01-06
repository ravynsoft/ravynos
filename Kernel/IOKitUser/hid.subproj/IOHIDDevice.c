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

#define DEBUG_ASSERT_COMPONENT_NAME_STRING IOHIDDevice
#if 0
#warning ### REMOVE THIS ###
#define DEBUG_ASSERT_PRODUCTION_CODE 0
#endif

#include <pthread.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFBase.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <asl.h>
#include <AssertMacros.h>
#include "IOHIDDevicePlugIn.h"
#include "IOHIDDevicePrivate.h"
#include "IOHIDQueue.h"
#include "IOHIDElement.h"
#include "IOHIDTransaction.h"
#include "IOHIDLibPrivate.h"
#include "IOHIDManagerPersistentProperties.h"
#include <os/assumes.h>
#include <dispatch/private.h>
#include "HIDDeviceIvar.h"

typedef struct  __IOHIDDevice {
    struct objc_object base;
    struct {
    HIDDeviceIvar
    };
} __IOHIDDevice;

//------------------------------------------------------------------------------
static CFArrayRef       __IOHIDDeviceCopyMatchingInputElements(
                                    IOHIDDeviceRef          device, 
                                    CFArrayRef              multiple);
static void             __IOHIDDeviceRegisterMatchingInputElements(
                                    IOHIDDeviceRef          device, 
                                    IOHIDQueueRef           queue,
                                    CFArrayRef              mutlipleMatch);
static void             __IOHIDDeviceInputElementValueCallback(
                                    void *                  context,
                                    IOReturn                result, 
                                    void *                  sender);
static void             __IOHIDDeviceNotification(
                                    IOHIDDeviceRef          service,
                                    io_service_t            ioservice,
                                    natural_t               messageType,
                                    void *                  messageArgument );
static void             __IOHIDDeviceValueCallback(
                                    void *                  context, 
                                    IOReturn                result, 
                                    void *                  sender, 
                                    IOHIDValueRef           value);
static void             __IOHIDDeviceReportCallbackOnce(
                                    void *                  context, 
                                    IOReturn                result, 
                                    void *                  sender, 
                                    IOHIDReportType         type, 
                                    uint32_t                reportID, 
                                    uint8_t *               report, 
                                    CFIndex                 reportLength);
static void             __IOHIDDeviceInputReportCallback(
                                    void *                  context, 
                                    IOReturn                result, 
                                    void *                  sender, 
                                    IOHIDReportType         type, 
                                    uint32_t                reportID, 
                                    uint8_t *               report, 
                                    CFIndex                 reportLength);
static void             __IOHIDDeviceInputReportWithTimeStampCallback(
                                    void *                  context, 
                                    IOReturn                result, 
                                    void *                  sender, 
                                    IOHIDReportType         type, 
                                    uint32_t                reportID, 
                                    uint8_t *               report, 
                                    CFIndex                 reportLength,
                                    uint64_t                timeStamp);
static void             __IOHIDDeviceInputReportApplier(
                                    CFDataRef               value, 
                                    void                    *voidContext);
static void             __IOHIDDeviceRegisterInputReportCallback(
                                    IOHIDDeviceRef                    device,
                                    uint8_t *                         report,
                                    CFIndex                           reportLength,
                                    IOHIDReportCallback               callback,
                                    IOHIDReportWithTimeStampCallback  callbackWithTimeStamp,
                                    void *                            context);
static void             __IOHIDDeviceTransactionCallback(
                                    void *                  context, 
                                    IOReturn                result, 
                                    void *                  sender);

static Boolean          __IOHIDDeviceCallbackBaseDataIsEqual(
                                    const void *            value1,
                                    const void *            value2);
static Boolean          __IOHIDDeviceSetupAsyncSupport(IOHIDDeviceRef device);

//------------------------------------------------------------------------------
typedef struct {
    void            *context;
    IOReturn        result;
    void            *sender;
    IOHIDReportType type;
    uint32_t        reportID;
    uint8_t         *report;
    CFIndex         reportLength;
    uint64_t        timeStamp;
} IOHIDDeviceInputReportApplierContext;

typedef struct {
    void *          context;
    void *          callback;
} IOHIDDeviceCallbackBaseInfo;

typedef struct {
    void *          context;
    void *          callback;
    IOHIDDeviceRef  device;
} IOHIDDeviceCallbackInfo;

typedef struct {
    void *          context;
    void *          callback;
    IOHIDDeviceRef  device;
    CFArrayRef      elements;
} IOHIDDeviceTransactionCallbackInfo;

typedef struct {
    void *              context;
    IOHIDValueCallback  callback;
} IOHIDDeviceInputElementValueCallbackInfo;

typedef struct {
    void *          context;
    IOHIDCallback   callback;
} IOHIDDeviceRemovalCallbackInfo;

typedef struct {
    void *                              context;
    IOHIDReportCallback                 callback;
    IOHIDReportWithTimeStampCallback    callbackWithTimeStamp;
    IOHIDDeviceRef                      device;
} IOHIDDeviceReportCallbackInfo;

static dispatch_once_t  __deviceInit = 0;
static  CFSetCallBacks  __callbackBaseSetCallbacks  = {};

static CFStringRef __debugKeys[] = {CFSTR(kIOHIDTransportKey), CFSTR(kIOHIDVendorIDKey), CFSTR(kIOHIDVendorIDSourceKey), CFSTR(kIOHIDProductIDKey), CFSTR(kIOHIDManufacturerKey), CFSTR(kIOHIDProductKey), CFSTR(kIOHIDPrimaryUsagePageKey), CFSTR(kIOHIDPrimaryUsageKey), CFSTR(kIOHIDCategoryKey), CFSTR(kIOHIDReportIntervalKey), CFSTR(kIOHIDSampleIntervalKey), CFSTR(kIOHIDBatchIntervalKey)};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IOHIDDeviceCopyDescription
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CFStringRef IOHIDDeviceCopyDescription(IOHIDDeviceRef device)
{
    CFMutableStringRef  description     = NULL;
    CFStringRef         subDescription  = NULL;
    io_name_t           name            = "";
    io_registry_entry_t regEntry        = MACH_PORT_NULL;
    
    regEntry = device->service;
    if ( regEntry && (IOObjectRetain(regEntry) != KERN_SUCCESS)) {
        regEntry = MACH_PORT_NULL;
    }
    
    description = CFStringCreateMutable(CFGetAllocator(device), 0);
    require(description, exit);
    
    IORegistryEntryGetName(regEntry, name);
    
    if (strlen(name) == 0) {
        IOObjectGetClass(regEntry, name);
    }
    
    subDescription = CFStringCreateWithFormat(CFGetAllocator(device), NULL, CFSTR("<IOHIDDevice %p [%p]  'ClassName=%s'"),
                                              device,
                                              CFGetAllocator(device),
                                              name);
    if ( subDescription ) {
        CFStringAppend(description, subDescription);
        CFRelease(subDescription);
    }
    
    for ( uint32_t index=0; index<(sizeof(__debugKeys)/sizeof(CFStringRef)); index++ ) {
        CFTypeRef   object;
        
        object = IOHIDDeviceGetProperty(device, __debugKeys[index]);
        if ( !object )
            continue;
        
        if ( CFGetTypeID(object) == CFNumberGetTypeID() ) {
            CFNumberRef number = (CFNumberRef)object;
            uint32_t    value;
            
            CFNumberGetValue(number, kCFNumberSInt32Type, &value);
            
            subDescription = CFStringCreateWithFormat(CFGetAllocator(device), NULL, CFSTR(" %@=%d"), __debugKeys[index], value);
        }
        else if ( CFGetTypeID(object) == CFStringGetTypeID() ) {
            
            subDescription = CFStringCreateWithFormat(CFGetAllocator(device), NULL, CFSTR(" %@=%@"), __debugKeys[index], object);
        }
        
        if ( !subDescription )
            continue;
        
        CFStringAppend(description, subDescription);
        CFRelease(subDescription);
    }
    
    CFStringAppend(description, CFSTR(">"));
exit:
    
    if ( regEntry )
        IOObjectRelease(regEntry);
    
    return description;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// _IOHIDDeviceReleasePrivate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void _IOHIDDeviceReleasePrivate(IOHIDDeviceRef device)
{
    if (device->dispatchQueue) {
        // enforce the call to activate/cancel
        os_assert(device->dispatchStateMask == (kIOHIDDispatchStateActive | kIOHIDDispatchStateCancelled),
                  "Invalid dispatch state: 0x%x", device->dispatchStateMask);
    }
    
    if (device->queue) {
        CFRelease(device->queue);
    }
    
    if ( device->inputMatchingMultiple ) {
        CFRelease(device->inputMatchingMultiple);
        device->inputMatchingMultiple = NULL;
    }
    
    CFRELEASE_IF_NOT_NULL(device->properties);
    CFRELEASE_IF_NOT_NULL(device->elements);
    CFRELEASE_IF_NOT_NULL(device->rootKey);
    
    CFRELEASE_IF_NOT_NULL(device->removalCallbackSet);
    CFRELEASE_IF_NOT_NULL(device->inputValueCallbackSet);
    CFRELEASE_IF_NOT_NULL(device->inputReportCallbackSet);
    
    if ( device->deviceInterface ) {
        (*device->deviceInterface)->Release(device->deviceInterface);
        device->deviceInterface = NULL;
    }
    
    if ( device->deviceTimeStampedInterface ) {
        (*device->deviceTimeStampedInterface)->Release(device->deviceTimeStampedInterface);
        device->deviceTimeStampedInterface = NULL;
    }

    
    if ( device->service ) {
        IOObjectRelease(device->service);
        device->service = 0;
    }
    
    if ( device->notification ) {
        IOObjectRelease(device->notification);
        device->notification = 0;
    }
    
    if (device->notificationPort) {
        IONotificationPortDestroy(device->notificationPort);
        device->notificationPort = NULL;
    }
    
    if (device->plugInInterface) {
        IODestroyPlugInInterface(device->plugInInterface);
        device->plugInInterface = NULL;
    }
    
    if (device->elementHandler) {
        Block_release(device->elementHandler);
    }
    
    if (device->removalHandler) {
        Block_release(device->removalHandler);
    }
    
    if (device->inputReportHandler) {
        Block_release(device->inputReportHandler);
    }
    
    if (device->reportBuffer) {
        CFRelease(device->reportBuffer);
    }
    
    if (device->transaction) {
        CFRelease(device->transaction);
    }
    
    if (device->batchElements) {
        CFRelease(device->batchElements);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceNotification
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceNotification(
                            IOHIDDeviceRef          device,
                            io_service_t            ioservice __unused,
                            natural_t               messageType,
                            void *                  messageArgument __unused)
{
    CFIndex index, count = 0;
    if ( !device || (messageType != kIOMessageServiceIsTerminated) || !device->removalCallbackSet || !(count=CFSetGetCount(device->removalCallbackSet)))
        return;
    
    CFRetain(device);
    
    CFDataRef dataValues[count];
    
    bzero(dataValues, sizeof(CFDataRef) * count);
    
    CFSetGetValues(device->removalCallbackSet, (const void **)dataValues);
    
    for ( index=0; index<count; index++ ) {
        IOHIDDeviceRemovalCallbackInfo *    info;
        CFDataRef                           infoRef;
        
        infoRef = (CFDataRef)dataValues[index];
        if ( !infoRef )
            continue;
        
        info = (IOHIDDeviceRemovalCallbackInfo *)CFDataGetBytePtr(infoRef);
        if ( !info )
            continue;
        
        if (!info->callback)
            continue;
        
        info->callback(info->context, kIOReturnSuccess, device);
    }

    CFRelease(device);
}

//------------------------------------------------------------------------------
// _IOHIDDeviceGetIOCFPlugInInterface
//------------------------------------------------------------------------------
IOCFPlugInInterface ** _IOHIDDeviceGetIOCFPlugInInterface( 
                                IOHIDDeviceRef                  device)
{
    return device->plugInInterface;
}

//------------------------------------------------------------------------------
// IOHIDDeviceCreate
//------------------------------------------------------------------------------
IOHIDDeviceRef IOHIDDeviceCreate(
                                CFAllocatorRef                  allocator, 
                                io_service_t                    service)
{
    IOCFPlugInInterface **          plugInInterface     = NULL;
    IOHIDDeviceDeviceInterface **   deviceInterface     = NULL;
    IOHIDDeviceTimeStampedDeviceInterface **   deviceTimeStampedInterface     = NULL;
    IOHIDDeviceRef                  device              = NULL;
    SInt32                          score               = 0;
    HRESULT                         result;
    
    dispatch_once(&__deviceInit, ^{
        __callbackBaseSetCallbacks = kCFTypeSetCallBacks;
        __callbackBaseSetCallbacks.equal = __IOHIDDeviceCallbackBaseDataIsEqual;
    });

    require_noerr(IOObjectRetain(service), retain_fail);
    require_noerr(IOCreatePlugInInterfaceForService(service, kIOHIDDeviceTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score), plugin_fail);
    require_noerr((*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOHIDDeviceDeviceInterfaceID), (LPVOID)&deviceInterface), query_fail);
    result = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOHIDDeviceDeviceInterfaceID2), (LPVOID)&deviceTimeStampedInterface);
    if (result != S_OK) {
        deviceTimeStampedInterface = NULL;
    }
    device = _IOHIDDeviceCreatePrivate(allocator);
    require(device, create_fail);
        
    device->plugInInterface = plugInInterface;
    device->deviceInterface = deviceInterface;
    device->deviceTimeStampedInterface = deviceTimeStampedInterface;
    device->service         = service;
    
    IORegistryEntryGetRegistryEntryID(service, &device->regID);

    return device;
    
create_fail:
    if (deviceInterface)
        (*deviceInterface)->Release(deviceInterface);
    if (deviceTimeStampedInterface)
        (*deviceTimeStampedInterface)->Release(deviceInterface);
query_fail:
    IODestroyPlugInInterface(plugInInterface);
plugin_fail:
    IOObjectRelease(service);
retain_fail:
    return NULL;
}

//------------------------------------------------------------------------------
// IOHIDDeviceGetService
//------------------------------------------------------------------------------
io_service_t IOHIDDeviceGetService(
                                IOHIDDeviceRef                  device)
{
    return device->service;
}


//------------------------------------------------------------------------------
// IOHIDDeviceOpen
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceOpen(          
                                IOHIDDeviceRef                  device, 
                                IOOptionBits                    options)
{
    return (*device->deviceInterface)->open(device->deviceInterface, options);
}

//------------------------------------------------------------------------------
// IOHIDDeviceClose
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceClose(
                                IOHIDDeviceRef                  device, 
                                IOOptionBits                    options)
{
    if ( device->inputValueCallbackSet )
        CFSetRemoveAllValues(device->inputValueCallbackSet);
    if ( device->inputReportCallbackSet )
        CFSetRemoveAllValues(device->inputReportCallbackSet);
    
    return (*device->deviceInterface)->close(device->deviceInterface, options);
}

//------------------------------------------------------------------------------
// IOHIDDeviceConformsTo
//------------------------------------------------------------------------------
Boolean IOHIDDeviceConformsTo(          
                                IOHIDDeviceRef                  device, 
                                uint32_t                        usagePage,
                                uint32_t                        usage)
{
    Boolean                 doesConform = FALSE;
    CFMutableDictionaryRef  matching    = NULL;
    
    matching = CFDictionaryCreateMutable(CFGetAllocator(device), 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (matching) {
        CFArrayRef  elements    = NULL;
        CFNumberRef number      = NULL;
        uint32_t    value;
    
        value   = kIOHIDElementTypeCollection;
        number  = CFNumberCreate(CFGetAllocator(device), kCFNumberIntType, &value);
        CFDictionarySetValue(matching, CFSTR(kIOHIDElementTypeKey), number);
        CFRelease(number);
        
        value   = usagePage;
        number  = CFNumberCreate(CFGetAllocator(device), kCFNumberIntType, &value);
        CFDictionarySetValue(matching, CFSTR(kIOHIDElementUsagePageKey), number);
        CFRelease(number);

        value   = usage;
        number  = CFNumberCreate(CFGetAllocator(device), kCFNumberIntType, &value);
        CFDictionarySetValue(matching, CFSTR(kIOHIDElementUsageKey), number);
        CFRelease(number);
        
        elements = IOHIDDeviceCopyMatchingElements(device, matching, 0);
        if ( elements ) {
            CFIndex         count, index;
            IOHIDElementRef element;
            
            count = CFArrayGetCount(elements);
            for (index=0; index<count; index++) {
                element = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, index);
                
                if ( IOHIDElementGetCollectionType(element) == kIOHIDElementCollectionTypePhysical ||
                     IOHIDElementGetCollectionType(element) == kIOHIDElementCollectionTypeApplication ) {
                     
                     doesConform = TRUE;
                     break;
                }
            
            }
            CFRelease(elements);
        }
        CFRelease(matching);
    }
    
    return doesConform;
}

//------------------------------------------------------------------------------
// IOHIDDeviceGetProperty
//------------------------------------------------------------------------------
CFTypeRef IOHIDDeviceGetProperty(
                                IOHIDDeviceRef                  device, 
                                CFStringRef                     key)
{
    CFTypeRef   property = NULL;
    IOReturn    ret;
    
    ret = (*device->deviceInterface)->getProperty(
                                            device->deviceInterface,
                                            key, 
                                            &property);
    
    if ((ret != kIOReturnSuccess) || !property) {
        if (device->properties) {
            property = CFDictionaryGetValue(device->properties, key);
        }
        else {
            property = NULL;
        }
    }

    return property;
}
                                
//------------------------------------------------------------------------------
// IOHIDDeviceSetProperty
//------------------------------------------------------------------------------
Boolean IOHIDDeviceSetProperty(
                                IOHIDDeviceRef                  device,
                                CFStringRef                     key,
                                CFTypeRef                       property)
{
    if (!device->properties) {
        device->properties = CFDictionaryCreateMutable(CFGetAllocator(device),
                                                        0,
                                                        &kCFTypeDictionaryKeyCallBacks,
                                                        &kCFTypeDictionaryValueCallBacks);
        if (!device->properties)
            return FALSE;
    }
    
    device->isDirty = TRUE;
    CFDictionarySetValue(device->properties, key, property);
    
    if (CFEqual(key, CFSTR(kIOHIDDeviceSuspendKey)) && CFGetTypeID(property) == CFBooleanGetTypeID()) {
        require(device->queue, exit);
        
        if (property == kCFBooleanTrue) {
            IOHIDQueueStop(device->queue);
        } else {
            IOHIDQueueStart(device->queue);
        }
    }
    
exit:
    return (*device->deviceInterface)->setProperty(device->deviceInterface, 
                                                   key, 
                                                   property) == kIOReturnSuccess;
}

//------------------------------------------------------------------------------
// IOHIDDeviceCopyMatchingElements
//------------------------------------------------------------------------------
CFArrayRef IOHIDDeviceCopyMatchingElements(
                                IOHIDDeviceRef                  device, 
                                CFDictionaryRef                 matching, 
                                IOOptionBits                    options)
{
    CFArrayRef  elements = NULL;
    IOReturn    ret;
    
    ret = (*device->deviceInterface)->copyMatchingElements(
                                        device->deviceInterface, 
                                        matching,
                                        &elements,
                                        options);
                                        
    if ( ret != kIOReturnSuccess && elements ) {
        CFRelease(elements);
        elements = NULL;
    }
    
    if ( elements ) {
        CFIndex         count, index;
        IOHIDElementRef element;
        
        count = CFArrayGetCount(elements);
        for (index=0; index<count; index++) {
            element = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, index);
            _IOHIDElementSetDevice(element, device);
            
            if (device->elements ||
                (device->elements = CFSetCreateMutable(CFGetAllocator(device),
                                                        0,
                                                        &kCFTypeSetCallBacks))) {
                if (!CFSetContainsValue(device->elements, element)) {
                    CFSetSetValue(device->elements, element);
                    if (device->loadProperties) {
                        __IOHIDElementLoadProperties(element);
                    }
                }
            }
        }
    }
    
    return elements;
}

//------------------------------------------------------------------------------
// __IOHIDDeviceSetupAsyncSupport
//------------------------------------------------------------------------------
Boolean __IOHIDDeviceSetupAsyncSupport(IOHIDDeviceRef device)
{
    IOReturn                    ret             = kIOReturnError;
    CFTypeRef                   asyncSource     = NULL;
    Boolean                     result          = false;
    CFRunLoopSourceContext1     sourceContext   = { 1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    
    // we've already been scheduled
    os_assert(!device->runLoop && !device->dispatchQueue, "Device already scheduled");
    
    require_action(!device->asyncEventSource, exit, result = true);
    
    ret = (*device->deviceInterface)->getAsyncEventSource(device->deviceInterface, &asyncSource);
    
    os_assert(ret == kIOReturnSuccess && asyncSource, "Failed to get async event source: 0x%x %p", ret, asyncSource);
    device->asyncEventSource = (CFRunLoopSourceRef)asyncSource;
    
    CFRunLoopSourceGetContext(device->asyncEventSource, (CFRunLoopSourceContext *)&sourceContext);
    device->sourceContext = sourceContext;
    
    device->queuePort = (CFMachPortRef)device->sourceContext.info;
    os_assert(device->queuePort, "Failed to get queue port %p %p", device->queuePort, &device->sourceContext);
    
    if (!device->notificationPort) {
        device->notificationPort = IONotificationPortCreate(kIOMasterPortDefault);
    }
    os_assert(device->notificationPort, "Failed to create notification port %p", device->notificationPort);
    
    result = true;
    
exit:
    return result;
}

//------------------------------------------------------------------------------
// IOHIDDeviceScheduleWithRunLoop
//------------------------------------------------------------------------------
void IOHIDDeviceScheduleWithRunLoop(
                                IOHIDDeviceRef                  device, 
                                CFRunLoopRef                    runLoop, 
                                CFStringRef                     runLoopMode)
{
    if (device->runLoop) {
        IOHIDDeviceUnscheduleFromRunLoop(device, device->runLoop, device->runLoopMode);
    }
    
    os_assert(__IOHIDDeviceSetupAsyncSupport(device));
    
    device->runLoop     = runLoop;
    device->runLoopMode = runLoopMode;
    
    CFRunLoopAddSource(device->runLoop, device->asyncEventSource, device->runLoopMode);
    
    CFRunLoopSourceRef source = IONotificationPortGetRunLoopSource(device->notificationPort);
    if (source) {
        CFRunLoopAddSource(device->runLoop, source, device->runLoopMode);
    }
    
    // Default queue has already been created, so go ahead and schedule it
    if (device->queue) {
        IOHIDQueueScheduleWithRunLoop(device->queue, device->runLoop, device->runLoopMode);
        IOHIDQueueStart(device->queue);
    }
}

//------------------------------------------------------------------------------
// IOHIDDeviceUnscheduleFromRunLoop
//------------------------------------------------------------------------------
void IOHIDDeviceUnscheduleFromRunLoop(  
                                IOHIDDeviceRef                  device, 
                                CFRunLoopRef                    runLoop __unused,
                                CFStringRef                     runLoopMode __unused)
{
    if (!device->runLoop) {
        return;
    }
    
    if (device->queue) {
      IOHIDQueueStop(device->queue);
      IOHIDQueueUnscheduleFromRunLoop(device->queue, device->runLoop, device->runLoopMode);
    }
    
    if (device->notificationPort) {
        CFRunLoopSourceRef source = IONotificationPortGetRunLoopSource(device->notificationPort);
        if (source) {
            CFRunLoopRemoveSource(device->runLoop, source, device->runLoopMode);
        }
    }
    
    if (device->asyncEventSource) {
        CFRunLoopRemoveSource(device->runLoop, device->asyncEventSource, device->runLoopMode);
    }
    
    device->runLoop = NULL;
    device->runLoopMode = NULL;
}

//------------------------------------------------------------------------------
// IOHIDDeviceSetDispatchQueue
//------------------------------------------------------------------------------
void IOHIDDeviceSetDispatchQueue(IOHIDDeviceRef device, dispatch_queue_t queue)
{
    os_assert(__IOHIDDeviceSetupAsyncSupport(device));
    
    device->dispatchQueue = dispatch_queue_create_with_target("IOHIDDeviceDispatchQueue", DISPATCH_QUEUE_SERIAL, queue);
    require(device->dispatchQueue, exit);
    
    CFRetain(device);
    device->dispatchMach = dispatch_mach_create("IOHIDDeviceDispatchMach", device->dispatchQueue,
                                                ^(dispatch_mach_reason_t reason,
                                                  dispatch_mach_msg_t message,
                                                  mach_error_t error __unused) {
        switch (reason) {
            case DISPATCH_MACH_MESSAGE_RECEIVED: {
                size_t size = 0;
                mach_msg_header_t *header = dispatch_mach_msg_get_msg(message, &size);
                
                // this will call into _cfmachPortCallback in IOHIDDeviceClass
                ((CFRunLoopSourceContext1*)&device->sourceContext)->perform(header, size, NULL, device->queuePort);
                break;
            }
            case DISPATCH_MACH_CANCELED: {
                dispatch_release(device->dispatchMach);
                device->dispatchMach = NULL;
                
                if (device->cancelHandler && !device->queue) {
                    (device->cancelHandler)();
                    Block_release(device->cancelHandler);
                    device->cancelHandler = NULL;
                }
                
                 dispatch_release(device->dispatchQueue);
                CFRelease(device);
                break;
            }
            default:
            break;
        }
    });
    
    require_action(device->dispatchMach, exit, CFRelease(device));
    
    if (device->queue) {
        IOHIDQueueSetDispatchQueue(device->queue, device->dispatchQueue);
        
        CFRetain(device);
        IOHIDQueueSetCancelHandler(device->queue, ^{
            CFRelease(device->queue);
            device->queue = NULL;
            
            if (device->cancelHandler && !device->dispatchMach) {
                (device->cancelHandler)();
                Block_release(device->cancelHandler);
                device->cancelHandler = NULL;
            }
            CFRelease(device);
        });
    }
    
exit:
    return;
}

//------------------------------------------------------------------------------
// IOHIDDeviceSetCancelHandler
//------------------------------------------------------------------------------
void IOHIDDeviceSetCancelHandler(IOHIDDeviceRef device, dispatch_block_t handler)
{
    os_assert(!device->cancelHandler && handler);
    
    device->cancelHandler = Block_copy(handler);
}

//------------------------------------------------------------------------------
// IOHIDDeviceActivate
//------------------------------------------------------------------------------
void IOHIDDeviceActivate(IOHIDDeviceRef device)
{
    os_assert(device->dispatchQueue && !device->runLoop,
              "Activate failed queue: %p runLoop: %p", device->dispatchQueue, device->runLoop);
    
    if (atomic_fetch_or(&device->dispatchStateMask, kIOHIDDispatchStateActive) & kIOHIDDispatchStateActive) {
        return;
    }
    
    dispatch_mach_connect(device->dispatchMach, CFMachPortGetPort(device->queuePort), MACH_PORT_NULL, 0);
    
    IONotificationPortSetDispatchQueue(device->notificationPort, device->dispatchQueue);
    
    if (device->queue) {
        IOHIDQueueActivate(device->queue);
    }
}

//------------------------------------------------------------------------------
// IOHIDDeviceCancel
//------------------------------------------------------------------------------
void IOHIDDeviceCancel(IOHIDDeviceRef device)
{
    os_assert(device->dispatchQueue && !device->runLoop,
              "Cancel failed queue: %p runLoop: %p", device->dispatchQueue, device->runLoop);
    
    if (atomic_fetch_or(&device->dispatchStateMask, kIOHIDDispatchStateCancelled) & kIOHIDDispatchStateCancelled) {
        return;
    }
    
    dispatch_mach_cancel(device->dispatchMach);
    
    if (device->notificationPort) {
        IONotificationPortDestroy(device->notificationPort);
        device->notificationPort = NULL;
    }
    
    if (device->queue) {
        IOHIDQueueCancel(device->queue);
    }
}

//------------------------------------------------------------------------------
// IOHIDDeviceRegisterRemovalCallback
//------------------------------------------------------------------------------
void IOHIDDeviceRegisterRemovalCallback( 
                                IOHIDDeviceRef                  device, 
                                IOHIDCallback                   callback, 
                                void *                          context)
{
    IOHIDDeviceRemovalCallbackInfo  info    = { context, callback };
    CFDataRef                       infoRef = NULL;
    
    os_assert(device->dispatchStateMask == kIOHIDDispatchStateInactive, "Device has already been activated/cancelled.");
    
    if (!device->removalCallbackSet) {
        device->removalCallbackSet = CFSetCreateMutable(NULL, 0, &__callbackBaseSetCallbacks);
    }
    require(device->removalCallbackSet, cleanup);
    
    // adding a callback
    infoRef = CFDataCreate(CFGetAllocator(device), (const UInt8 *) &info, sizeof(info));
    require(infoRef, cleanup);

    if (callback) {
        kern_return_t kret = 0;
        
        CFSetAddValue(device->removalCallbackSet, infoRef);
        
        if (!device->notificationPort) {
            device->notificationPort = IONotificationPortCreate(kIOMasterPortDefault);
            require(device->notificationPort, cleanup);
            require(device->service, cleanup);
        }
        
        if (device->runLoop) {
            CFRunLoopSourceRef source = IONotificationPortGetRunLoopSource(device->notificationPort);
            if (source) {
                CFRunLoopAddSource(device->runLoop, source, device->runLoopMode);
            }
        }
        
        kret = IOServiceAddInterestNotification(device->notificationPort,   // notifyPort
                                                device->service,            // service
                                                kIOGeneralInterest,         // interestType
                                                (IOServiceInterestCallback)__IOHIDDeviceNotification, // callback
                                                device,                     // refCon
                                                &(device->notification)     // notification
                                                );
        require_noerr(kret, cleanup);
    } else {
        CFSetRemoveValue(device->removalCallbackSet, infoRef);
    }
    
cleanup:
    CFRELEASE_IF_NOT_NULL(infoRef);
}

//******************************************************************************
// HID ELEMENT SUPPORT
//******************************************************************************

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceCopyMatchingInputElements
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CFArrayRef __IOHIDDeviceCopyMatchingInputElements(IOHIDDeviceRef device, CFArrayRef multiple)
{
    CFMutableArrayRef       inputElements   = NULL;
    CFArrayRef              elements        = NULL;
    CFIndex                 index, count;
    
    // Grab the matching multiple.  If one has not already been specified,
    // fallback to the default
    if ( multiple ) {
        CFRetain(multiple);
    } else {
        uint32_t inputTypes[] = { kIOHIDElementTypeInput_Misc, kIOHIDElementTypeInput_Button, kIOHIDElementTypeInput_Axis, kIOHIDElementTypeInput_ScanCodes};
        
        count = sizeof(inputTypes) / sizeof(uint32_t);
        
        CFDictionaryRef matching[count];
        
        bzero(matching, sizeof(CFDictionaryRef) * count);
        
        CFStringRef key = CFSTR(kIOHIDElementTypeKey);
        for ( index=0; index<count; index++ ) {            
            CFNumberRef number = CFNumberCreate(CFGetAllocator(device), kCFNumberIntType, &inputTypes[index]);
            matching[index] = CFDictionaryCreate(CFGetAllocator(device),
                                                 (const void **)&key,
                                                 (const void **)&number,
                                                 1, 
                                                 &kCFCopyStringDictionaryKeyCallBacks, 
                                                 &kCFTypeDictionaryValueCallBacks);
            CFRelease(number);
        }
        
        multiple = CFArrayCreate(CFGetAllocator(device), (const void **)matching, count, &kCFTypeArrayCallBacks);
        
        for ( index=0; index<count; index++ )
            CFRelease(matching[index]);
    }
    
    if ( !multiple )
        return NULL;
        
    count = CFArrayGetCount( multiple );
   
    for ( index=0; index<count; index++) {
        elements = IOHIDDeviceCopyMatchingElements( device,
                                                    CFArrayGetValueAtIndex(multiple, index),
                                                    0);
        if ( !elements ) 
            continue;
            
        if ( !inputElements )
            inputElements = CFArrayCreateMutableCopy(   CFGetAllocator(device),
                                                        0, 
                                                        elements);
        else
            CFArrayAppendArray( inputElements, 
                                elements, 
                                CFRangeMake(0, CFArrayGetCount(elements)));
     
        CFRelease(elements);
    }
   
    CFRelease(multiple);
    
    return inputElements;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceRegisterMatchingInputElements
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceRegisterMatchingInputElements(
                                IOHIDDeviceRef                  device, 
                                IOHIDQueueRef                   queue,
                                CFArrayRef                      mutlipleMatch)
{
    CFArrayRef elements = __IOHIDDeviceCopyMatchingInputElements(device, mutlipleMatch);
    
    if ( !elements )
        return;
        
    CFIndex         i, count;
    IOHIDElementRef element;
    
    count = CFArrayGetCount(elements);
    
    for (i=0; i<count; i++) {
        element = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
        
        if ( !element  ) {
            continue;
        }
        IOHIDQueueAddElement(queue, element);
    }
    
    CFRelease(elements);
}

//------------------------------------------------------------------------------
// IOHIDDeviceRegisterInputValueCallback
//------------------------------------------------------------------------------
void IOHIDDeviceRegisterInputValueCallback(
                                IOHIDDeviceRef                  device, 
                                IOHIDValueCallback              callback, 
                                void *                          context)
{
    CFDataRef                                   infoRef = NULL;
    IOHIDDeviceInputElementValueCallbackInfo    info    = {context, callback};
    
    os_assert(device->dispatchStateMask == kIOHIDDispatchStateInactive, "Device has already been activated/cancelled.");
    
    CFRetain(device);
    
    if (!device->inputValueCallbackSet) {
        device->inputValueCallbackSet = CFSetCreateMutable(NULL, 0, &__callbackBaseSetCallbacks);
    }
    require(device->inputValueCallbackSet, cleanup);

    infoRef = CFDataCreate(CFGetAllocator(device), (const UInt8 *) &info, sizeof(info));
    require(infoRef, cleanup);

    if (callback) {
        // adding a callback
        if ( !device->queue ) {
            device->queue = IOHIDQueueCreate(CFGetAllocator(device), device, 20, 0);
            require(device->queue, cleanup);
            
            __IOHIDDeviceRegisterMatchingInputElements(device, device->queue, device->inputMatchingMultiple);
            
            // If a run loop has been already set, go ahead and schedule the queues
            if ( device->runLoop ) {
                IOHIDQueueScheduleWithRunLoop(device->queue,
                                              device->runLoop, 
                                              device->runLoopMode);
                
                IOHIDQueueStart(device->queue);
            }
            
            if (device->dispatchQueue) {
                IOHIDQueueSetDispatchQueue(device->queue, device->dispatchQueue);
                
                CFRetain(device);
                IOHIDQueueSetCancelHandler(device->queue, ^{
                    CFRelease(device->queue);
                    device->queue = NULL;
                    
                    if (device->cancelHandler && !device->dispatchMach) {
                        (device->cancelHandler)();
                        Block_release(device->cancelHandler);
                        device->cancelHandler = NULL;
                    }
                    CFRelease(device);
                });
            }
        }
        CFSetAddValue(device->inputValueCallbackSet, infoRef);
    }
    else {
        // removing a callback
        CFSetRemoveValue(device->inputValueCallbackSet, infoRef);
    }
    
    if (device && device->queue)
        IOHIDQueueRegisterValueAvailableCallback(device->queue, 
                                                 __IOHIDDeviceInputElementValueCallback, 
                                                 device);

cleanup:
    CFRELEASE_IF_NOT_NULL(infoRef);
    CFRelease(device);
}

//------------------------------------------------------------------------------
// IOHIDDeviceSetInputValueMatching
//------------------------------------------------------------------------------
void IOHIDDeviceSetInputValueMatching(
                                IOHIDDeviceRef                  device, 
                                CFDictionaryRef                 matching)
{
    os_assert(device->dispatchStateMask == kIOHIDDispatchStateInactive, "Device has already been activated/cancelled.");
    
    if ( matching ) {
        CFArrayRef multiple = CFArrayCreate(CFGetAllocator(device), (const void **)&matching, 1, &kCFTypeArrayCallBacks);
        
        IOHIDDeviceSetInputValueMatchingMultiple(device, multiple);
        
        CFRelease(multiple);
    } else {
        IOHIDDeviceSetInputValueMatchingMultiple(device, NULL);
    }
}

//------------------------------------------------------------------------------
// IOHIDDeviceSetInputValueMatchingMultiple
//------------------------------------------------------------------------------
void IOHIDDeviceSetInputValueMatchingMultiple(
                                IOHIDDeviceRef                  device, 
                                CFArrayRef                      multiple)
{
    os_assert(device->dispatchStateMask == kIOHIDDispatchStateInactive, "Device has already been activated/cancelled.");
    
    if ( device->queue ) {
        IOHIDValueRef   value;
        CFArrayRef      elements;
        
        // stop the queue
        IOHIDQueueStop(device->queue);
        
        // drain the queue
        while ( (value = IOHIDQueueCopyNextValue(device->queue)) )
            CFRelease(value);

        // clear the exising elements from the queue            
        elements = _IOHIDQueueCopyElements(device->queue);
        if ( elements ) {
            CFIndex index, count;
        
            count = CFArrayGetCount(elements);
            for (index=0; index<count; index++)
                IOHIDQueueRemoveElement(device->queue, (IOHIDElementRef)CFArrayGetValueAtIndex(elements, index));
                
            CFRelease(elements);
        }
        
        // reload
        __IOHIDDeviceRegisterMatchingInputElements(device, device->queue, multiple);
        
        // start it back up
        IOHIDQueueStart(device->queue);
    }

    if ( device->inputMatchingMultiple ) 
        CFRelease(device->inputMatchingMultiple);
    
    device->inputMatchingMultiple = multiple ? (CFArrayRef)CFRetain(multiple) : NULL;
    
}

//------------------------------------------------------------------------------
// IOHIDDeviceSetValue
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceSetValue(
                                IOHIDDeviceRef                  device, 
                                IOHIDElementRef                 element, 
                                IOHIDValueRef                   value)
{
    return (*device->deviceInterface)->setValue(
                                                device->deviceInterface,
                                                element,
                                                value,
                                                0,
                                                NULL,
                                                NULL,
                                                0);
}

//------------------------------------------------------------------------------
// IOHIDDeviceSetValueMultiple
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceSetValueMultiple(
                                IOHIDDeviceRef                  device, 
                                CFDictionaryRef                 multiple)
{
    return IOHIDDeviceSetValueMultipleWithCallback(device, multiple, 0, NULL, NULL);
}

//------------------------------------------------------------------------------
// IOHIDDeviceSetValueWithCallback
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceSetValueWithCallback(
                                IOHIDDeviceRef                  device, 
                                IOHIDElementRef                 element, 
                                IOHIDValueRef                   value, 
                                CFTimeInterval                  timeout,
                                IOHIDValueCallback              callback, 
                                void *                          context)
{
    IOHIDDeviceCallbackInfo * elementInfo = 
            (IOHIDDeviceCallbackInfo *)malloc(sizeof(IOHIDDeviceCallbackInfo));
    
    if ( !elementInfo )
        return kIOReturnNoMemory;

    elementInfo->device     = device;
    elementInfo->callback   = callback;
    elementInfo->context    = context;

    uint32_t timeoutMS = timeout * 1000;
    
    IOReturn ret = (*device->deviceInterface)->setValue(
                                                device->deviceInterface,
                                                element,
                                                value,
                                                timeoutMS,
                                                __IOHIDDeviceValueCallback,
                                                elementInfo,
                                                0);
                                                
    if ( ret != kIOReturnSuccess )
        free(elementInfo);
    
    return ret;
}

//------------------------------------------------------------------------------
// IOHIDDeviceSetValueMultipleWithCallback
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceSetValueMultipleWithCallback(
                                IOHIDDeviceRef                  device, 
                                CFDictionaryRef                 multiple,
                                CFTimeInterval                  timeout,
                                IOHIDValueMultipleCallback      callback, 
                                void *                          context)
{
    IOReturn            ret             = kIOReturnError;
    IOHIDElementRef *   elements        = NULL;
    IOHIDValueRef *     values          = NULL;
    IOHIDTransactionRef transaction     = NULL;
    CFIndex             index, count;
    
    
    if ( !multiple || ((count = CFDictionaryGetCount(multiple)) == 0))
        return kIOReturnBadArgument;
           
    do {
        transaction = IOHIDTransactionCreate(   CFGetAllocator(device),
                                                device,
                                                kIOHIDTransactionDirectionTypeOutput,
                                                0);
        if ( !transaction ) {
            ret = kIOReturnNoMemory;
            break;
         }

        elements = (IOHIDElementRef *)malloc(count * sizeof(IOHIDElementRef));
        if ( !elements ) { 
            ret = kIOReturnNoMemory;
            break;
        }
            
        values  = (IOHIDValueRef *)malloc(count * sizeof(IOHIDValueRef));
        if ( !values ) { 
            ret = kIOReturnNoMemory;
            break;
        }
        
        CFDictionaryGetKeysAndValues(multiple, (const void **)elements, (const void **)values);
        
        for ( index=0; index<count; index++ ) {
            IOHIDTransactionAddElement(transaction, elements[index]);
            IOHIDTransactionSetValue(transaction, elements[index], values[index], 0);
        }

        if ( callback ) {  // Async
            IOHIDDeviceTransactionCallbackInfo * elementInfo = 
                    (IOHIDDeviceTransactionCallbackInfo *)malloc(sizeof(IOHIDDeviceTransactionCallbackInfo));
            
            if ( !elementInfo ) { 
                ret = kIOReturnNoMemory;
                break;
            }

            elementInfo->device     = device;
            elementInfo->callback   = callback;
            elementInfo->context    = context;
            elementInfo->elements   = CFArrayCreate(CFGetAllocator(device), (const void **)elements, count, &kCFTypeArrayCallBacks);
        
            ret = IOHIDTransactionCommitWithCallback(transaction, timeout, __IOHIDDeviceTransactionCallback, elementInfo);
            
            if ( ret != kIOReturnSuccess )
                free(elementInfo);
                
        } else { // Sync
            ret = IOHIDTransactionCommit(transaction);
        }
        
    } while ( FALSE );
    
    if ( transaction )
        CFRelease(transaction);
        
    if ( elements )
        free(elements);

    if ( values )
        free(values);
    
    return ret;
}

//------------------------------------------------------------------------------
// IOHIDDeviceGetValue
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceGetValue(
                                IOHIDDeviceRef                  device, 
                                IOHIDElementRef                 element, 
                                IOHIDValueRef *                 pValue)
{
    return (*device->deviceInterface)->getValue(
                                                device->deviceInterface,
                                                element,
                                                pValue,
                                                0,
                                                NULL,
                                                NULL,
                                                0);
}

//------------------------------------------------------------------------------
// IOHIDDeviceGetValueWithOptions
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceGetValueWithOptions (
                             IOHIDDeviceRef                  device,
                             IOHIDElementRef                 element,
                             IOHIDValueRef *                 pValue,
                             uint32_t                        options)
{
    return (*device->deviceInterface)->getValue(
                                                device->deviceInterface,
                                                element,
                                                pValue,
                                                0,
                                                NULL,
                                                NULL,
                                                options);
}

//------------------------------------------------------------------------------
// IOHIDDeviceCopyValueMultiple
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceCopyValueMultiple(
                                IOHIDDeviceRef                  device, 
                                CFArrayRef                      elements, 
                                CFDictionaryRef *               pMultiple)
{
    return IOHIDDeviceCopyValueMultipleWithCallback(device,
                                                    elements,
                                                    pMultiple,
                                                    0,
                                                    NULL,
                                                    NULL);
}

//------------------------------------------------------------------------------
// IOHIDDeviceGetValueWithCallback
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceGetValueWithCallback(
                                IOHIDDeviceRef                  device, 
                                IOHIDElementRef                 element, 
                                IOHIDValueRef *                 pValue,
                                CFTimeInterval                  timeout,
                                IOHIDValueCallback              callback, 
                                void *                          context)
{
    IOHIDDeviceCallbackInfo * elementInfo = 
            (IOHIDDeviceCallbackInfo *)malloc(sizeof(IOHIDDeviceCallbackInfo));
    
    if ( !elementInfo )
        return kIOReturnNoMemory;

    elementInfo->device     = device;
    elementInfo->callback   = callback;
    elementInfo->context    = context;

    uint32_t timeoutMS = timeout * 1000;
    
    IOReturn ret = (*device->deviceInterface)->getValue(
                                                device->deviceInterface,
                                                element,
                                                pValue,
                                                timeoutMS,
                                                __IOHIDDeviceValueCallback,
                                                elementInfo,
                                                0);
                                                
    if ( ret != kIOReturnSuccess )
        free(elementInfo);
        
    return ret;
}

//------------------------------------------------------------------------------
// IOHIDDeviceCopyValueMultipleWithCallback
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceCopyValueMultipleWithCallback(
                                IOHIDDeviceRef                  device, 
                                CFArrayRef                      elements, 
                                CFDictionaryRef *               pMultiple,
                                CFTimeInterval                  timeout,
                                IOHIDValueMultipleCallback      callback, 
                                void *                          context)
{
    IOReturn            ret             = kIOReturnError;
    IOHIDTransactionRef transaction     = NULL;
    IOHIDValueRef       value           = NULL;
    CFIndex             index, count;
    
    
    if ( !pMultiple || !elements || ((count = CFArrayGetCount(elements)) == 0))
        return kIOReturnBadArgument;
           
    do {
        transaction = IOHIDTransactionCreate(   CFGetAllocator(device),
                                                device,
                                                kIOHIDTransactionDirectionTypeInput,
                                                0);
        if ( !transaction ) {
            ret = kIOReturnNoMemory;
            break;
         }

        // Add the elements to the transaction
        for ( index=0; index<count; index++ )
            IOHIDTransactionAddElement(transaction, (IOHIDElementRef)CFArrayGetValueAtIndex(elements, index));

        if ( callback ) {  // Async
            IOHIDDeviceTransactionCallbackInfo * elementInfo = 
                    (IOHIDDeviceTransactionCallbackInfo *)malloc(sizeof(IOHIDDeviceTransactionCallbackInfo));
            
            if ( !elementInfo ) { 
                ret = kIOReturnNoMemory;
                break;
            }

            elementInfo->device     = device;
            elementInfo->callback   = callback;
            elementInfo->context    = context;
            elementInfo->elements   = CFArrayCreateCopy(CFGetAllocator(device), elements);
        
            ret = IOHIDTransactionCommitWithCallback(transaction, timeout, __IOHIDDeviceTransactionCallback, elementInfo);
            
            if ( ret != kIOReturnSuccess )
                free(elementInfo);
                
        } else { // Sync
        
            ret = IOHIDTransactionCommit(transaction);
            
            if ( ret != kIOReturnSuccess )
                break;
                
            CFMutableDictionaryRef multiple = CFDictionaryCreateMutable(
                                                    CFGetAllocator(device),
                                                    count, 
                                                    &kCFCopyStringDictionaryKeyCallBacks, 
                                                    &kCFTypeDictionaryValueCallBacks);
            
            if ( !multiple ) {
                ret = kIOReturnNoMemory;
                break;
            }
            
            for ( index = 0; index<count; index++ ) {
                IOHIDElementRef element = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, index);
                value = IOHIDTransactionGetValue(transaction, element, 0);
                if ( !value )
                    continue;
                     
                CFDictionarySetValue(multiple, element, value); 
            }
            
            if ( CFDictionaryGetCount(multiple) == 0 ) {
                CFRelease(multiple);
                multiple = NULL;
            }
            
            *pMultiple = multiple;
        }
        
    } while ( FALSE );
    
    if ( transaction )
        CFRelease(transaction);
                
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceValueCallback
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceValueCallback(
                                    void *                  context, 
                                    IOReturn                result, 
                                    void *                  sender __unused, 
                                    IOHIDValueRef           value)
{
    IOHIDDeviceCallbackInfo *   elementInfo = (IOHIDDeviceCallbackInfo *)context;
    IOHIDDeviceRef              device      = elementInfo->device;
    
    if ( elementInfo->callback ) {
        IOHIDValueCallback callback = elementInfo->callback;
        
        (*callback)(elementInfo->context,
                    result, 
                    device,
                    value);
    }
    
    free(elementInfo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceInputElementValueCallback
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceInputElementValueCallback(
                                    void *                  context,
                                    IOReturn                result, 
                                    void *                  sender __unused)
{
    IOHIDDeviceRef  device  = (IOHIDDeviceRef)context;
    IOHIDQueueRef   queue   = (IOHIDQueueRef)sender;
    IOHIDValueRef   value   = NULL;
    
    if ( (queue != device->queue) || (kIOReturnSuccess != result) )
        return;
    
    CFRetain(device);

    CFIndex count = CFSetGetCount(device->inputValueCallbackSet);
    
    if ( count ) {
        CFDataRef   dataValues[count];
        CFIndex     index = 0;
        
        bzero(dataValues, sizeof(CFDataRef) * count);
        
        CFSetGetValues(device->inputValueCallbackSet, (const void **)dataValues);
        
        // Drain the queue and dispatch the values
        while ( (value = IOHIDQueueCopyNextValue(queue)) ) {
            for ( index=0; index<count; index++ ) {
                CFDataRef infoRef = (CFDataRef)dataValues[index];
                IOHIDDeviceInputElementValueCallbackInfo *info;
                
                if ( !infoRef )
                    continue;
                info = (IOHIDDeviceInputElementValueCallbackInfo *)CFDataGetBytePtr(infoRef);
                if (info->callback)
                    info->callback(info->context, kIOReturnSuccess, device, value);
            }
            CFRelease(value);
        }
    }

    CFRelease(device);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceTransactionCallback
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceTransactionCallback(
                                    void *                  context,
                                    IOReturn                result, 
                                    void *                  sender)
{
    IOHIDTransactionRef                 transaction     = (IOHIDTransactionRef)sender;
    IOHIDDeviceTransactionCallbackInfo  *elementInfo    = (IOHIDDeviceTransactionCallbackInfo *)context;
    IOHIDDeviceRef                      device          = elementInfo->device;
    IOHIDValueMultipleCallback          callback        = elementInfo->callback;
    CFMutableDictionaryRef              values          = NULL;
    
    do {
        if ( !transaction )
            break;
            
        if ( !callback )
            break;
            
        if ( result == kIOReturnSuccess ) {
            
            if ( !elementInfo->elements )
                break;
                
            CFIndex         count = CFArrayGetCount(elementInfo->elements);
            CFIndex         index = 0;
            IOHIDElementRef element = NULL;
            IOHIDValueRef   value = NULL;
            
            if ( count )
                values = CFDictionaryCreateMutable(
                                        CFGetAllocator(device), 
                                        count, 
                                        &kCFCopyStringDictionaryKeyCallBacks, 
                                        &kCFTypeDictionaryValueCallBacks);
            
            if ( !values )
                break;
            
            for ( index = 0; index<count; index++ ) {
                element = (IOHIDElementRef)CFArrayGetValueAtIndex(elementInfo->elements, index);
                if ( !element )
                    continue;

                /* FIX ME: provide an option to get transaction value before it is cleared */
                value = IOHIDTransactionGetValue(transaction, element, 0);
                if ( !value )
                    continue;
                    
                CFDictionarySetValue(values, element, value); 
            }

            if ( CFDictionaryGetCount(values) == 0 ) {
                CFRelease(values);
                values = NULL;
            }
        }

        (*callback)(elementInfo->context, result, device, values);
            
    } while ( FALSE );
    
    if ( values )
        CFRelease(values);

    if ( elementInfo->elements )
        CFRelease(elementInfo->elements);
        
    free(elementInfo);
}

//******************************************************************************
// HID REPORT SUPPORT
//******************************************************************************
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceReportCallbackOnce
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceReportCallbackOnce(void *                  context,
                                     IOReturn                result, 
                                     void *                  sender, 
                                     IOHIDReportType         type, 
                                     uint32_t                reportID, 
                                     uint8_t *               report, 
                                     CFIndex                 reportLength)
{
    IOHIDDeviceReportCallbackInfo   *info   = (IOHIDDeviceReportCallbackInfo *)context;
    IOHIDDeviceRef                  device      = info->device;
    
    CFRetain(device);
    if ( info->callback && sender == device->deviceInterface ) {
        info->callback(info->context,
                       result,
                       device,
                       type,
                       reportID,
                       report,
                       reportLength);
    }
    else if ( info->callbackWithTimeStamp && sender == device->deviceTimeStampedInterface ) {
        info->callbackWithTimeStamp(info->context,
                                    result,
                                    device,
                                    type,
                                    reportID,
                                    report,
                                    reportLength,
                                    0);
    }
    
    free(context);
    CFRelease(device);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceInputReportApplier
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceInputReportApplier(CFDataRef value, void *voidContext)
{
    if (value && voidContext) {
        IOHIDDeviceReportCallbackInfo *info = (IOHIDDeviceReportCallbackInfo *)CFDataGetBytePtr(value);
        IOHIDDeviceInputReportApplierContext *context = (IOHIDDeviceInputReportApplierContext*)voidContext;
        if ( info ) {
            if (info->callback) {
                info->callback(info->context,
                               context->result,
                               context->sender,
                               context->type,
                               context->reportID,
                               context->report,
                               context->reportLength);
            }
            if (info->callbackWithTimeStamp) {
                info->callbackWithTimeStamp(info->context,
                                            context->result,
                                            context->sender,
                                            context->type,
                                            context->reportID,
                                            context->report,
                                            context->reportLength,
                                            context->timeStamp);
            }
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceInputReportCallback
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceInputReportCallback(  void *                  context,
                                        IOReturn                result,
                                        void *                  sender __unused,
                                        IOHIDReportType         type,
                                        uint32_t                reportID,
                                        uint8_t *               report,
                                        CFIndex                 reportLength)
{
    IOHIDDeviceRef  device  = (IOHIDDeviceRef)context;
    CFIndex         count;
    
    if (device && device->inputReportCallbackSet && (count = CFSetGetCount(device->inputReportCallbackSet)) ) {
        
        IOHIDDeviceInputReportApplierContext applierContext = {
            context, result, device, type, reportID, report, reportLength, 0
        };
        CFRetain(device);
        
        CFSetApplyFunction(device->inputReportCallbackSet,
                           (CFSetApplierFunction)__IOHIDDeviceInputReportApplier,
                           &applierContext);
        
        CFRelease(device);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceInputReportWithTimeStampCallback
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceInputReportWithTimeStampCallback(void *                  context,
                                                   IOReturn                result,
                                                   void *                  sender __unused,
                                                   IOHIDReportType         type,
                                                   uint32_t                reportID,
                                                   uint8_t *               report,
                                                   CFIndex                 reportLength,
                                                   uint64_t                timeStamp)
{
    IOHIDDeviceRef  device = (IOHIDDeviceRef)context;
    
    if (device && device->inputReportCallbackSet && CFSetGetCount(device->inputReportCallbackSet) ) {
        IOHIDDeviceInputReportApplierContext applierContext = {
            context, result, device, type, reportID, report, reportLength, timeStamp
        };
        CFRetain(device);
        
        CFSetApplyFunction(device->inputReportCallbackSet,
                           (CFSetApplierFunction)__IOHIDDeviceInputReportApplier,
                           &applierContext);
        
        CFRelease(device);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceCallbackBaseDataIsEqual
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Boolean __IOHIDDeviceCallbackBaseDataIsEqual(const void * value1, const void * value2)
{
    IOHIDDeviceCallbackBaseInfo * info1, * info2;
    CFDataRef data1     = (CFDataRef)value1;
    CFDataRef data2     = (CFDataRef)value2;
    Boolean   result    = false;
    
    require_action_quiet(data1!=data2, exit, result=true);

    require_action_quiet(data1&&data2, exit, result=false);
    
    info1 = (IOHIDDeviceCallbackBaseInfo*)CFDataGetBytePtr(data1);
    info2 = (IOHIDDeviceCallbackBaseInfo*)CFDataGetBytePtr(data2);
    
    require_action_quiet(info1 && info2, exit, result=false);
    
    result = info1->context == info2->context;
    
exit:
    return result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceRegisterInputReportCallback
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceRegisterInputReportCallback(IOHIDDeviceRef                    device,
                                              uint8_t *                         report,
                                              CFIndex                           reportLength,
                                              IOHIDReportCallback               callback,
                                              IOHIDReportWithTimeStampCallback  callbackWithTimeStamp,
                                              void *                            context)
{
    IOHIDDeviceReportCallbackInfo   info    = {context, callback, callbackWithTimeStamp, device};
    CFDataRef                       infoRef = NULL;
    
    CFRetain(device);
    
    if (!device->inputReportCallbackSet) {
        device->inputReportCallbackSet = CFSetCreateMutable(NULL, 0, &__callbackBaseSetCallbacks);
    }
    require(device->inputReportCallbackSet, cleanup);
    
    // adding or modifying a callback
    infoRef = CFDataCreate(CFGetAllocator(device), (const UInt8 *) &info, sizeof(info));
    require(infoRef, cleanup);
    
    if (callback || callbackWithTimeStamp) {
        CFSetAddValue(device->inputReportCallbackSet, infoRef);
        
        if (device->deviceTimeStampedInterface) {
            (*device->deviceTimeStampedInterface)->
                    setInputReportWithTimeStampCallback(device->deviceInterface,
                                                        report,
                                                        reportLength,
                                                        __IOHIDDeviceInputReportWithTimeStampCallback,
                                                        device,
                                                        0);
        }
        else {
            (*device->deviceInterface)->setInputReportCallback(device->deviceInterface,
                                                               report,
                                                               reportLength,
                                                               __IOHIDDeviceInputReportCallback,
                                                               device,
                                                               0);
        }
    }
    else {
        CFSetRemoveValue(device->inputReportCallbackSet, infoRef);
    }
    
cleanup:
    CFRELEASE_IF_NOT_NULL(infoRef);
    CFRelease(device);
}

//------------------------------------------------------------------------------
// IOHIDDeviceRegisterInputReportCallback
//------------------------------------------------------------------------------
void IOHIDDeviceRegisterInputReportCallback(IOHIDDeviceRef                  device,
                                            uint8_t *                       report, 
                                            CFIndex                         reportLength,
                                            IOHIDReportCallback             callback, 
                                            void *                          context)
{
    os_assert(device->dispatchStateMask == kIOHIDDispatchStateInactive, "Device has already been activated/cancelled.");
    
    __IOHIDDeviceRegisterInputReportCallback(device, report, reportLength, callback, NULL, context);
}

//------------------------------------------------------------------------------
// IOHIDDeviceRegisterInputReportWithTimeStampCallback
//------------------------------------------------------------------------------
void IOHIDDeviceRegisterInputReportWithTimeStampCallback(IOHIDDeviceRef                      device,
                                                         uint8_t *                           report,
                                                         CFIndex                             reportLength,
                                                         IOHIDReportWithTimeStampCallback    callback,
                                                         void *                              context)
{
    os_assert(device->dispatchStateMask == kIOHIDDispatchStateInactive, "Device has already been activated/cancelled.");
    
    __IOHIDDeviceRegisterInputReportCallback(device, report, reportLength, NULL, callback, context);
}

//------------------------------------------------------------------------------
// IOHIDDeviceSetReport
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceSetReport(
                                IOHIDDeviceRef                  device,
                                IOHIDReportType                 reportType,
                                CFIndex                         reportID,
                                const uint8_t *                 report,
                                CFIndex                         reportLength)
{
    return (*device->deviceInterface)->setReport(
                                                device->deviceInterface,
                                                reportType,
                                                reportID,
                                                report,
                                                reportLength,
                                                0,
                                                NULL,
                                                NULL,
                                                0);
}
                                
//------------------------------------------------------------------------------
// IOHIDDeviceSetReportWithCallback
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceSetReportWithCallback(
                                IOHIDDeviceRef                  device,
                                IOHIDReportType                 reportType,
                                CFIndex                         reportID,
                                const uint8_t *                 report,
                                CFIndex                         reportLength,
                                CFTimeInterval                  timeout,
                                IOHIDReportCallback             callback,
                                void *                          context)
{
    IOHIDDeviceReportCallbackInfo *info_ptr = (IOHIDDeviceReportCallbackInfo*)malloc(sizeof(IOHIDDeviceReportCallbackInfo));

    if ( !info_ptr )
        return kIOReturnNoMemory;

    info_ptr->context = context;
    info_ptr->callback = callback;
    info_ptr->device = device;
    
    uint32_t timeoutMS = timeout * 1000;
    
    IOReturn result = (*device->deviceInterface)->setReport(device->deviceInterface,
                                                             reportType,
                                                             reportID,
                                                             report,
                                                             reportLength,
                                                             timeoutMS,
                                                             __IOHIDDeviceReportCallbackOnce,
                                                             info_ptr,
                                                             0);
    if (result)
        free(info_ptr);
    return result;
}

//------------------------------------------------------------------------------
// IOHIDDeviceGetReport
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceGetReport(
                                IOHIDDeviceRef                  device,
                                IOHIDReportType                 reportType,
                                CFIndex                         reportID,
                                uint8_t *                       report,
                                CFIndex *                       pReportLength)
{
    return (*device->deviceInterface)->getReport(
                                                device->deviceInterface,
                                                reportType,
                                                reportID,
                                                report,
                                                pReportLength,
                                                0,
                                                NULL,
                                                NULL,
                                                0);
}

//------------------------------------------------------------------------------
// IOHIDDeviceGetReportWithCallback
//------------------------------------------------------------------------------
IOReturn IOHIDDeviceGetReportWithCallback(
                                IOHIDDeviceRef                  device,
                                IOHIDReportType                 reportType,
                                CFIndex                         reportID,
                                uint8_t *                       report,
                                CFIndex *                       pReportLength,
                                CFTimeInterval                  timeout,
                                IOHIDReportCallback             callback,
                                void *                          context)
{
    IOHIDDeviceReportCallbackInfo info = {
        context,
        callback,
        NULL,
        device
    };
    void *info_ptr = malloc(sizeof(info));
    
    if ( !info_ptr )
        return kIOReturnNoMemory;
    
    memcpy(info_ptr, &info, sizeof(info));
    
    uint32_t timeoutMS = timeout * 1000;
    
    IOReturn result = (*device->deviceInterface)->getReport(device->deviceInterface,
                                                            reportType,
                                                            reportID,
                                                            report,
                                                            pReportLength,
                                                            timeoutMS,
                                                            __IOHIDDeviceReportCallbackOnce,
                                                            (void *) info_ptr,
                                                            0);
    if (result)
        free(info_ptr);
    return result;
}

//------------------------------------------------------------------------------
// IOHIDDeviceGetRegistryEntryID
//------------------------------------------------------------------------------
uint64_t IOHIDDeviceGetRegistryEntryID(IOHIDDeviceRef device)
{
    return device->regID;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceGetRootKey
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CFStringRef __IOHIDDeviceGetRootKey(IOHIDDeviceRef device)
{
    if (!device->rootKey) {
        // Device Root Key
        // All *required* matching information
        CFStringRef manager = __IOHIDManagerGetRootKey();
        CFStringRef transport = (CFStringRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDTransportKey));
        CFNumberRef vendor = (CFNumberRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey));
        CFNumberRef product = (CFNumberRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));
        CFStringRef serial = (CFStringRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDSerialNumberKey));
        if (!transport || (CFGetTypeID(transport) != CFStringGetTypeID())) {
            transport = CFSTR("unknown");
        }
        if (!vendor || (CFGetTypeID(vendor) != CFNumberGetTypeID())) {
            vendor = kCFNumberNaN;
        }
        if (!product || (CFGetTypeID(product) != CFNumberGetTypeID())) {
            product = kCFNumberNaN;
        }
        if (!serial || (CFGetTypeID(serial) != CFStringGetTypeID())) {
            serial = CFSTR("none");
        }
        
        device->rootKey = CFStringCreateWithFormat(NULL, NULL, 
                                                   CFSTR("%@#%@#%@#%@#%@"), 
                                                   manager,
                                                   transport,
                                                   vendor,
                                                   product,
                                                   serial);
    }
    
    return device->rootKey;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceGetUUIDString
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CFStringRef __IOHIDDeviceGetUUIDString(IOHIDDeviceRef device)
{
    CFStringRef uuidStr = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManagerUUIDKey));
    if (!uuidStr || (CFGetTypeID(uuidStr) != CFStringGetTypeID())) {
        CFUUIDRef uuid = CFUUIDCreate(NULL);
        uuidStr = CFUUIDCreateString(NULL, uuid);
        IOHIDDeviceSetProperty(device, CFSTR(kIOHIDManagerUUIDKey), uuidStr);
        CFRelease(uuid);
        CFRelease(uuidStr);
    }
    return uuidStr;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceGetUUIDKey
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CFStringRef __IOHIDDeviceGetUUIDKey(IOHIDDeviceRef device)
{
    if (!device->UUIDKey) {
        CFStringRef manager = __IOHIDManagerGetRootKey();
        CFStringRef uuidStr = __IOHIDDeviceGetUUIDString(device);

        device->UUIDKey = CFStringCreateWithFormat(NULL, NULL, 
                                                   CFSTR("%@#%@"), 
                                                   manager,
                                                   uuidStr);
    }
    
    return device->UUIDKey;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceSaveProperties
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceSaveProperties(IOHIDDeviceRef device, __IOHIDPropertyContext *context)
{
    if (device->isDirty && device->properties) {
        CFStringRef uuidStr = __IOHIDDeviceGetUUIDString(device);
        CFArrayRef uuids = (CFArrayRef)CFPreferencesCopyAppValue(__IOHIDDeviceGetRootKey(device), kCFPreferencesCurrentApplication);
        CFArrayRef uuidsToWrite = NULL;
        if (uuids && (CFGetTypeID(uuids) == CFArrayGetTypeID())) {
            CFRange range = { 0, CFArrayGetCount(uuids) };
            if (!CFArrayContainsValue(uuids, range, uuidStr)) {
                uuidsToWrite = CFArrayCreateMutableCopy(NULL, CFArrayGetCount(uuids) + 1, uuids);
                CFArrayAppendValue((CFMutableArrayRef)uuidsToWrite, uuidStr);
            }
        }
        else {
            uuidsToWrite = CFArrayCreate(NULL, (const void**)&uuidStr, 1, &kCFTypeArrayCallBacks);
        }
        if (uuidsToWrite) {
            __IOHIDPropertySaveWithContext(__IOHIDDeviceGetRootKey(device), uuidsToWrite, context);
        }
        
        CFRELEASE_IF_NOT_NULL(uuids);
        CFRELEASE_IF_NOT_NULL(uuidsToWrite);
        
        __IOHIDPropertySaveToKeyWithSpecialKeys(device->properties, 
                                               __IOHIDDeviceGetUUIDKey(device), 
                                               NULL, 
                                               context);
        device->isDirty = FALSE;
    }

    if (device->elements)
        CFSetApplyFunction(device->elements, __IOHIDSaveElementSet, context);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDDeviceLoadProperties
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDDeviceLoadProperties(IOHIDDeviceRef device)
{
    CFStringRef uuidStr = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManagerUUIDKey));
    device->loadProperties = TRUE;
    
    // Have we already generated a key?
    if (!device->UUIDKey) {
        // Is there a UUID in the device properties?
        if (!uuidStr || (CFGetTypeID(uuidStr) != CFStringGetTypeID())) {
            // Are there any UUIDs for this device?
            CFArrayRef uuids = (CFArrayRef)CFPreferencesCopyAppValue(__IOHIDDeviceGetRootKey(device), kCFPreferencesCurrentApplication);
            if (uuids && (CFGetTypeID(uuids) == CFArrayGetTypeID()) && CFArrayGetCount(uuids)) {
                // VTN3 ¥ TODO: Add optional matching based on location ID and anything else you can think of
                uuidStr = (CFStringRef)CFArrayGetValueAtIndex(uuids, 0);
            }
        }
        if (!uuidStr || (CFGetTypeID(uuidStr) != CFStringGetTypeID())) {
            CFUUIDRef uuid = CFUUIDCreate(NULL);
            uuidStr = CFUUIDCreateString(NULL, uuid);
            CFRelease(uuid);
        }
        IOHIDDeviceSetProperty(device, CFSTR(kIOHIDManagerUUIDKey), uuidStr);
    }
    
    // Convert to __IOHIDPropertyLoadFromKeyWithSpecialKeys if we identify special keys
    CFMutableDictionaryRef properties = __IOHIDPropertyLoadDictionaryFromKey(__IOHIDDeviceGetUUIDKey(device));
    
    if (properties) {
        CFRELEASE_IF_NOT_NULL(device->properties);
        device->properties = properties;
        device->isDirty = FALSE;
    }
    
    IOHIDDeviceSetProperty(device, CFSTR(kIOHIDManagerUUIDKey), uuidStr);
    
    // A device does not apply its properties to its elements.
    if (device->elements)
        CFSetApplyFunction(device->elements, __IOHIDLoadElementSet, NULL);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDApplyPropertyToDeviceSet
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDApplyPropertyToDeviceSet(const void *value, void *context) {
    IOHIDDeviceRef device = (IOHIDDeviceRef)value;
    __IOHIDApplyPropertyToSetContext *data = (__IOHIDApplyPropertyToSetContext*)context;
    if (device && data) {
        IOHIDDeviceSetProperty(device, data->key, data->property);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDApplyPropertiesToDeviceFromDictionary
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDApplyPropertiesToDeviceFromDictionary(const void *key, const void *value, void *context) {
    IOHIDDeviceRef device = (IOHIDDeviceRef)context;
    if (device && key && value) {
        IOHIDDeviceSetProperty(device, (CFStringRef)key, (CFTypeRef)value);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDSaveDeviceSet
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void __IOHIDSaveDeviceSet(const void *value, void *context) {
    IOHIDDeviceRef device = (IOHIDDeviceRef)value;
    if (device)
        __IOHIDDeviceSaveProperties(device, (__IOHIDPropertyContext*)context);
}

//------------------------------------------------------------------------------
