/*
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2012 Apple Computer, Inc.  All Rights Reserved.
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

#include <AssertMacros.h>
#include <pthread.h>
#include <dispatch/dispatch.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFBase.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFSerialize.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDResourceUserClient.h>
#include <IOKit/IODataQueueClient.h>
#include "IOHIDUserDevice.h"
#include "IOHIDDebugTrace.h"
#include <IOKit/IOKitLibPrivate.h>
#include <os/assumes.h>
#include <dispatch/private.h>
#include <os/state_private.h>
#include <mach/mach_time.h>

#include <IOKit/hid/IOHIDAnalytics.h>

#define IOHIDUDLogError(fmt, ...)   os_log_error(_IOHIDLogCategory(kIOHIDLogCategoryUserDevice), "0x%llx: " fmt, device->regID, ##__VA_ARGS__)
#define IOHIDUDLog(fmt, ...)       os_log(_IOHIDLogCategory(kIOHIDLogCategoryUserDevice), "0x%llx: " fmt, device->regID, ##__VA_ARGS__)
#define IOHIDUDLogInfo(fmt, ...)    os_log_info(_IOHIDLogCategory(kIOHIDLogCategoryUserDevice), "0x%llx: " fmt, device->regID, ##__VA_ARGS__)
#define IOHIDUDLogDebug(fmt, ...)   os_log_debug(_IOHIDLogCategory(kIOHIDLogCategoryUserDevice), "0x%llx: " fmt, device->regID, ##__VA_ARGS__)

static IOHIDUserDeviceRef   __IOHIDUserDeviceCreate(
                                    CFAllocatorRef          allocator, 
                                    CFAllocatorContext *    context __unused,
                                    IOOptionBits            options);
static void                 __IOHIDUserDeviceExtRelease( CFTypeRef object );
static void                 __IOHIDUserDeviceIntRelease( CFTypeRef object );
static void                 __IOHIDUserDeviceRegister(void);
static void                 __IOHIDUserDeviceQueueCallback(CFMachPortRef port, void *msg, CFIndex size, void *info);
static void                 __IOHIDUserDeviceHandleReportAsyncCallback(void *refcon, IOReturn result);
static Boolean              __IOHIDUserDeviceSetupAsyncSupport(IOHIDUserDeviceRef device);
static IOReturn             __IOHIDUserDeviceStartDevice(IOHIDUserDeviceRef device, IOOptionBits options);
static void                 __IOHIDUserDeviceDestroyDevice(IOHIDUserDeviceRef device);
static CFStringRef          __IOHIDUserDeviceCopyDebugDescription(CFTypeRef object);
static bool                 __IOHIDUserDeviceSetupAnalytics(IOHIDUserDeviceRef device);
static void                 __IOHIDUserDeviceUpdateUsageAnalytics(IOHIDUserDeviceRef device);

typedef struct __IOHIDUserDevice
{
    IOHIDObjectBase                 hidBase;

    io_service_t                    service;
    io_connect_t                    connect;
    io_service_t                    userDevice;
    CFDictionaryRef                 properties;
    IOOptionBits                    options;
    os_state_handle_t               stateHandler;
    dispatch_queue_t                stateQueue;
    uint64_t                        queueCallbackTS;
    uint64_t                        dequeueTS;
    uint64_t                        regID;
    
    CFRunLoopRef                    runLoop;
    CFStringRef                     runLoopMode;
    
    _Atomic uint32_t                dispatchStateMask;
    
    dispatch_queue_t                dispatchQueue;
    dispatch_mach_t                 dispatchMach;
    dispatch_block_t                cancelHandler;
    
    struct {
        CFMachPortRef               port;
        CFRunLoopSourceRef          source;
        IODataQueueMemory *         data;
        uint64_t                    size;
        uint32_t                    lastTail;
        CFTypeRef                   usageAnalytics;
    } queue;
    
    struct {
        IONotificationPortRef       port;
        CFRunLoopSourceRef          source;
        IODataQueueMemory *         data;
    } async;
    
    struct {
        IOHIDUserDeviceReportCallback   callback;
        void *                          refcon;
    } setReport, getReport;

    struct {
        IOHIDUserDeviceReportWithReturnLengthCallback   callback;
        void *                                          refcon;
    } getReportWithReturnLength;
    
    IOHIDUserDeviceSetReportBlock setReportBlock;
    IOHIDUserDeviceGetReportBlock getReportBlock;
    
    struct {
        uint32_t                    setreport;
        uint32_t                    getreport;
        uint32_t                    handlereport;
    } statistics;
    
} __IOHIDUserDevice, *__IOHIDUserDeviceRef;

static const IOHIDObjectClass __IOHIDUserDeviceClass = {
    {
        _kCFRuntimeCustomRefCount,      // version
        "IOHIDUserDevice",              // className
        NULL,                           // init
        NULL,                           // copy
        __IOHIDUserDeviceExtRelease,    // finalize
        NULL,                           // equal
        NULL,                           // hash
        NULL,                           // copyFormattingDesc
        __IOHIDUserDeviceCopyDebugDescription,  // copyDebugDesc
        NULL,                           // reclaim
        _IOHIDObjectExtRetainCount,     // refcount
        NULL                            // requiredAlignment
    },
    _IOHIDObjectIntRetainCount,
    __IOHIDUserDeviceIntRelease
};

static pthread_once_t   __deviceTypeInit            = PTHREAD_ONCE_INIT;
static CFTypeID         __kIOHIDUserDeviceTypeID    = _kCFRuntimeNotATypeID;
static mach_port_t      __masterPort                = MACH_PORT_NULL;


typedef struct __IOHIDDeviceHandleReportAsyncContext {
    IOHIDUserDeviceHandleReportAsyncCallback   callback;
    void *                          refcon;
} IOHIDDeviceHandleReportAsyncContext;


//------------------------------------------------------------------------------
// __IOHIDNotificationCopyDebugDescription
//------------------------------------------------------------------------------
CFStringRef __IOHIDUserDeviceCopyDebugDescription(CFTypeRef object)
{
    IOHIDUserDeviceRef device = (IOHIDUserDeviceRef) object;
    
    return CFStringCreateWithFormat(CFGetAllocator(object), NULL, CFSTR("<IOHIDUserDeviceRef ref:%d/%d id:0x%llx stats:%d,%d,%d>"),
                                    (int)device->hidBase.ref,
                                    (int)device->hidBase.xref,
                                    device->regID,
                                    device->statistics.setreport,
                                    device->statistics.getreport,
                                    device->statistics.handlereport
                                    );
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceRegister
//------------------------------------------------------------------------------
void __IOHIDUserDeviceRegister(void)
{
    IOMasterPort(bootstrap_port, &__masterPort);
    __kIOHIDUserDeviceTypeID = _CFRuntimeRegisterClass(&__IOHIDUserDeviceClass.cfClass);
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceCreate
//------------------------------------------------------------------------------
IOHIDUserDeviceRef __IOHIDUserDeviceCreate(   
                                CFAllocatorRef              allocator, 
                                CFAllocatorContext *        context __unused,
                                IOOptionBits                options)
{
    IOHIDUserDeviceRef  device = NULL;
    uint32_t            size;
    
    /* allocate service */
    size  = sizeof(__IOHIDUserDevice) - sizeof(CFRuntimeBase);
    device = (IOHIDUserDeviceRef)_IOHIDObjectCreateInstance(allocator, IOHIDUserDeviceGetTypeID(), size, NULL);
    
    if (!device)
        return NULL;
    
    device->options = options;
    
    HIDDEBUGTRACE(kHID_UserDev_Create, device, 0, 0, 0);
    
    return device;
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceFinalizeStateHandler
//------------------------------------------------------------------------------
void __IOHIDUserDeviceFinalizeStateHandler(void *context)
{
    IOHIDUserDeviceRef device = (IOHIDUserDeviceRef)context;
    _IOHIDObjectInternalRelease(device);
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceDestroyDevice
//------------------------------------------------------------------------------
void __IOHIDUserDeviceDestroyDevice(IOHIDUserDeviceRef device)
{
    // Severs all ties with our kernel service. This will terminate the
    // underlying IOHIDUserDevice.
    IOHIDUDLog("Destroy: %@", device);
    
    if (device->async.port) {
        IONotificationPortDestroy(device->async.port);
        device->async.port = NULL;
    }
    
    if (device->queue.data)
    {
#if !__LP64__
        vm_address_t        mappedMem = (vm_address_t)device->queue.data;
#else
        mach_vm_address_t   mappedMem = (mach_vm_address_t)device->queue.data;
#endif
        IOConnectUnmapMemory (  device->connect,
                              0,
                              mach_task_self(),
                              mappedMem);
        device->queue.data = NULL;
    }
    
    if (device->connect) {
        IOServiceClose(device->connect);
        device->connect = 0;
    }
    
    if (device->service) {
        IOObjectRelease(device->service);
        device->service = 0;
    }
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceExtRelease
//------------------------------------------------------------------------------
void __IOHIDUserDeviceExtRelease( CFTypeRef object )
{
    IOHIDUserDeviceRef device = (IOHIDUserDeviceRef)object;
    
    HIDDEBUGTRACE(kHID_UserDev_Release, object, 0, 0, 0);
    
    if (device->dispatchQueue) {
        // enforce the call to activate/cancel
        os_assert(device->dispatchStateMask == (kIOHIDDispatchStateActive | kIOHIDDispatchStateCancelled),
                  "Invalid dispatch state: 0x%x", device->dispatchStateMask);
    }
    
    if (device->stateHandler) {
        os_state_remove_handler(device->stateHandler);
    }
    
    if (device->stateQueue) {
        dispatch_set_context(device->stateQueue, device);
        dispatch_set_finalizer_f(device->stateQueue, __IOHIDUserDeviceFinalizeStateHandler);
        _IOHIDObjectInternalRetain(device);
        dispatch_release(device->stateQueue);
    }
    
    if (device->queue.port) {
        CFMachPortInvalidate(device->queue.port);
    }
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceIntRelease
//------------------------------------------------------------------------------
void __IOHIDUserDeviceIntRelease( CFTypeRef object )
{
    IOHIDUserDeviceRef device = (IOHIDUserDeviceRef)object;
    
    HIDDEBUGTRACE(kHID_UserDev_Release, object, 0, 0, 0);
    
    __IOHIDUserDeviceDestroyDevice(device);
    
    if (device->queue.source) {
        CFRelease(device->queue.source);
        device->queue.source = NULL;
    }
    
    if (device->queue.port) {
        mach_port_mod_refs(mach_task_self(),
                           CFMachPortGetPort(device->queue.port),
                           MACH_PORT_RIGHT_RECEIVE,
                           -1);
        
        CFRelease(device->queue.port);
        device->queue.port = NULL;
    }
    
    if (device->properties) {
        CFRelease(device->properties);
        device->properties = NULL;
    }
    
    if (device->userDevice) {
        IOObjectRelease(device->userDevice);
        device->userDevice = 0;
    }

    if (device->queue.usageAnalytics) {
        IOHIDAnalyticsEventCancel(device->queue.usageAnalytics);
        CFRelease(device->queue.usageAnalytics);
        device->queue.usageAnalytics = NULL;
    }
    
    if (device->setReportBlock) {
        Block_release(device->setReportBlock);
    }
    
    if (device->getReportBlock) {
        Block_release(device->getReportBlock);
    }
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceCopyService
//------------------------------------------------------------------------------
io_service_t IOHIDUserDeviceCopyService(IOHIDUserDeviceRef device)
{
    io_service_t service = IO_OBJECT_NULL;
    
    if (device->userDevice) {
        service = device->userDevice;
        IOObjectRetain(service);
    }
    
    return service;
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceGetTypeID
//------------------------------------------------------------------------------
CFTypeID IOHIDUserDeviceGetTypeID(void) 
{
    if ( _kCFRuntimeNotATypeID == __kIOHIDUserDeviceTypeID )
        pthread_once(&__deviceTypeInit, __IOHIDUserDeviceRegister);
        
    return __kIOHIDUserDeviceTypeID;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDUserDeviceStartDevice
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IOReturn __IOHIDUserDeviceStartDevice(IOHIDUserDeviceRef device, IOOptionBits options)
{
    CFDataRef   data = NULL;
    IOReturn    kr;
    uint64_t    input = options;
    
    data = IOCFSerialize(device->properties, 0);
    require_action(data, error, kr=kIOReturnNoMemory);
    
    kr = IOConnectCallMethod(device->connect, kIOHIDResourceDeviceUserClientMethodCreate, &input, 1, CFDataGetBytePtr(data), CFDataGetLength(data), NULL, NULL, NULL, NULL);
    require_noerr(kr, error);
    
    kr = IOConnectGetService(device->connect, &device->userDevice);
    require_noerr(kr, error);
    
    IORegistryEntryGetRegistryEntryID(device->service, &device->regID);
    
    IOHIDUDLog("Start: %@", device);
    
    HIDDEBUGTRACE(kHID_UserDev_Start, device, options, device->regID, 0);

error:
    if ( data ) {
        CFRelease(data);
    }
    
    if (kr) {
        IOHIDUDLogError("IOHIDUserDevice start failed:0x%x properties:%@", kr, device->properties);
    }
    return kr;
    
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceSerializeState
//------------------------------------------------------------------------------
CFMutableDictionaryRef __IOHIDUserDeviceSerializeState(IOHIDUserDeviceRef device)
{
    io_service_t service = IO_OBJECT_NULL;
    uint64_t regID = 0;
    CFMutableDictionaryRef state = NULL;
    
    state = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                      0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);
    require(state, exit);
    
    service = IOHIDUserDeviceCopyService(device);
    if (service) {
        IORegistryEntryGetRegistryEntryID(service, &regID);
    }
    
    CFDictionarySetValue(state, CFSTR("DispatchQueue"), device->dispatchQueue ? kCFBooleanTrue : kCFBooleanFalse);
    CFDictionarySetValue(state, CFSTR("RunLoop"), device->runLoop ? kCFBooleanTrue : kCFBooleanFalse);
    CFDictionarySetValue(state, CFSTR("Queue"), device->queue.data ? kCFBooleanTrue : kCFBooleanFalse);
    CFDictionarySetValue(state, CFSTR("SetReportCallback"), device->setReport.callback ? kCFBooleanTrue : kCFBooleanFalse);
    CFDictionarySetValue(state, CFSTR("GetReportCallback"), (device->getReport.callback || device->getReportWithReturnLength.callback) ? kCFBooleanTrue : kCFBooleanFalse);
    
    _IOHIDDictionaryAddSInt64(state, CFSTR("RegistryID"), regID);
    _IOHIDDictionaryAddSInt64(state, CFSTR("QueueCallbackTimestamp"), device->queueCallbackTS);
    _IOHIDDictionaryAddSInt64(state, CFSTR("DequeueTimestamp"), device->dequeueTS);
    _IOHIDDictionaryAddSInt64(state, CFSTR("SetReportCnt"), device->statistics.setreport);
    _IOHIDDictionaryAddSInt64(state, CFSTR("GetReportCnt"), device->statistics.getreport);
    _IOHIDDictionaryAddSInt64(state, CFSTR("HandleReportCnt"), device->statistics.handlereport);

exit:
    
    if (service) {
        IOObjectRelease(service);
    }
    
    return state;
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceStateHandler
//------------------------------------------------------------------------------
os_state_data_t __IOHIDUserDeviceStateHandler(IOHIDUserDeviceRef device,
                                              os_state_hints_t hints)
{
    os_state_data_t stateData = NULL;
    CFMutableDictionaryRef deviceState = NULL;
    CFDataRef serializedDeviceState = NULL;
    
    if (hints->osh_api != OS_STATE_API_FAULT &&
        hints->osh_api != OS_STATE_API_REQUEST) {
        return NULL;
    }
    
    deviceState = __IOHIDUserDeviceSerializeState(device);
    require(deviceState, exit);
    
    serializedDeviceState = CFPropertyListCreateData(kCFAllocatorDefault, deviceState, kCFPropertyListBinaryFormat_v1_0, 0, NULL);
    require(serializedDeviceState, exit);
    
    uint32_t serializedDeviceStateSize = (uint32_t)CFDataGetLength(serializedDeviceState);
    stateData = calloc(1, OS_STATE_DATA_SIZE_NEEDED(serializedDeviceStateSize));
    require(stateData, exit);
    
    strlcpy(stateData->osd_title, "IOHIDUserDevice State", sizeof(stateData->osd_title));
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
// IOHIDUserDeviceCreate
//------------------------------------------------------------------------------
IOHIDUserDeviceRef IOHIDUserDeviceCreate(
                                CFAllocatorRef                  allocator, 
                                CFDictionaryRef                 properties)
{
    return IOHIDUserDeviceCreateWithOptions(allocator, properties, 0);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceCreateWithProperties
//------------------------------------------------------------------------------
IOHIDUserDeviceRef IOHIDUserDeviceCreateWithProperties(CFAllocatorRef allocator,
                                                       CFDictionaryRef properties,
                                                       IOOptionBits options)
{
    return IOHIDUserDeviceCreateWithOptions(allocator, properties, options);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceCreateWithOptions
//------------------------------------------------------------------------------
IOHIDUserDeviceRef IOHIDUserDeviceCreateWithOptions(CFAllocatorRef allocator, CFDictionaryRef properties, IOOptionBits options)
{
    IOHIDUserDeviceRef  device = NULL;
    IOHIDUserDeviceRef  result = NULL;
    kern_return_t       kr;


    require(properties, error);
        
    device = __IOHIDUserDeviceCreate(allocator, NULL, options);
    require(device, error);
    
    device->properties = CFDictionaryCreateCopy(allocator, properties);
    require(device->properties, error);

    device->service = IOServiceGetMatchingService(__masterPort, IOServiceMatching("IOHIDResource"));
    require_action (device->service, error, IOHIDUDLogError("IOHIDResource not found"));

    kr = IOServiceOpen(device->service, mach_task_self(), kIOHIDResourceUserClientTypeDevice, &device->connect);
    require_noerr_action(kr, error, IOHIDUDLogError("IOServiceOpen:0x%x", kr));
    
    kr = __IOHIDUserDeviceStartDevice(device, device->options);
    require_noerr_action(kr, error, IOHIDUDLogError("__IOHIDUserDeviceStartDevice:0x%x", kr));
    
    device->stateQueue = dispatch_queue_create("IOHIDUserDeviceStateQueue", DISPATCH_QUEUE_SERIAL);
    require(device->stateQueue, error);
    
    device->stateHandler = os_state_add_handler(device->stateQueue,
                                                ^os_state_data_t(os_state_hints_t hints) {
        return __IOHIDUserDeviceStateHandler(device, hints);
    });

    require(__IOHIDUserDeviceSetupAnalytics(device), error);

    result = device;
    CFRetain(result);


error:

    if (device) {
        CFRelease(device);
    }
    return result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDUserDeviceSetupAsyncSupport
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Boolean __IOHIDUserDeviceSetupAsyncSupport(IOHIDUserDeviceRef device)
{
    Boolean result = false;
    
    // we've already been scheduled
    os_assert(!device->runLoop && !device->dispatchQueue, "Device already scheduled");
    
    if ( !device->queue.data ) {
        IOReturn ret;
    #if !__LP64__
        vm_address_t        address = 0;
        vm_size_t           size    = 0;
    #else
        mach_vm_address_t   address = 0;
        mach_vm_size_t      size    = 0;
    #endif
        
        ret = IOConnectMapMemory(device->connect, 0, mach_task_self(), &address, &size, kIOMapAnywhere);
        require_noerr_action(ret, exit, result=false; IOHIDUDLogError("IOConnectMapMemory:0x%x", ret));
        
        device->queue.data =(IODataQueueMemory * )address;
        device->queue.size = (uint64_t)size;
    }

    if ( !device->queue.port ) {
        mach_port_t port = IODataQueueAllocateNotificationPort();
        
        if ( port != MACH_PORT_NULL ) {
            CFMachPortContext context = {0, device, NULL, NULL, NULL};
            
            device->queue.port = CFMachPortCreateWithPort(CFGetAllocator(device), port, __IOHIDUserDeviceQueueCallback, &context, FALSE);
        }
    }
    require_action(device->queue.port, exit, result=false);

    if ( !device->async.port ) {
        device->async.port = IONotificationPortCreate(kIOMasterPortDefault);
    }

    require_action(device->async.port, exit, result=false);
    
    result = true;
    
exit:
    
    HIDDEBUGTRACE(kHID_UserDev_AsyncSupport, device, result, 0, 0);
    
    return result;
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceScheduleWithRunLoop
//------------------------------------------------------------------------------
void IOHIDUserDeviceScheduleWithRunLoop(IOHIDUserDeviceRef device, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
    os_assert(__IOHIDUserDeviceSetupAsyncSupport(device));
    
    device->runLoop = runLoop;
    device->runLoopMode = runLoopMode;
    
    if ( !device->queue.source ) {
        device->queue.source = CFMachPortCreateRunLoopSource(CFGetAllocator(device), device->queue.port, 0);
        if ( !device->queue.source )
            return;
    }
    
    if ( !device->async.source ) {
        device->async.source = IONotificationPortGetRunLoopSource(device->async.port);
        if ( !device->async.source )
            return;
    }
    
    CFRunLoopAddSource(runLoop, device->async.source, runLoopMode);
    CFRunLoopAddSource(runLoop, device->queue.source, runLoopMode);
    
    if (device->setReport.callback ||
        device->getReport.callback ||
        device->getReportWithReturnLength.callback) {
        IOConnectSetNotificationPort(device->connect,
                                     0,
                                     CFMachPortGetPort(device->queue.port),
                                     (uintptr_t)NULL);
    }
    
    if (device->options & kIOHIDUserDeviceCreateOptionStartWhenScheduled) {
        IOConnectCallMethod(device->connect,
                            kIOHIDResourceDeviceUserClientMethodRegisterService,
                            NULL, 0, NULL, 0, NULL, NULL, NULL, NULL);
    }
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceUnscheduleFromRunLoop
//------------------------------------------------------------------------------
void IOHIDUserDeviceUnscheduleFromRunLoop(IOHIDUserDeviceRef device, CFRunLoopRef runLoop __unused, CFStringRef runLoopMode __unused)
{
    HIDDEBUGTRACE(kHID_UserDev_Unschedule, device, 0, 0, 0);
    
    os_assert(device->runLoop && !device->dispatchQueue,
              "Unschedule failed queue: %p runLoop: %p", device->dispatchQueue, device->runLoop);
    
    IOConnectSetNotificationPort(device->connect, 0, MACH_PORT_NULL, (uintptr_t)NULL);
    CFRunLoopRemoveSource(device->runLoop, device->queue.source, device->runLoopMode);
    CFRunLoopRemoveSource(device->runLoop, device->async.source, device->runLoopMode);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceScheduleWithDispatchQueue
//------------------------------------------------------------------------------
void IOHIDUserDeviceScheduleWithDispatchQueue(IOHIDUserDeviceRef device,
                                              dispatch_queue_t queue)
{
    IOHIDUserDeviceSetDispatchQueue(device, queue);
    IOHIDUserDeviceActivate(device);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceUnscheduleFromDispatchQueue
//------------------------------------------------------------------------------
void IOHIDUserDeviceUnscheduleFromDispatchQueue(IOHIDUserDeviceRef device,
                                                dispatch_queue_t queue __unused)
{
    IOHIDUserDeviceCancel(device);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceSetDispatchQueue
//------------------------------------------------------------------------------
void IOHIDUserDeviceSetDispatchQueue(IOHIDUserDeviceRef device,
                                     dispatch_queue_t queue)
{
    HIDDEBUGTRACE(kHID_UserDev_ScheduleDispatch, device, 0, 0, 0);
    
    os_assert(__IOHIDUserDeviceSetupAsyncSupport(device));
    
    device->dispatchQueue = dispatch_queue_create_with_target("IOHIDUserDeviceDispatchQueue", DISPATCH_QUEUE_SERIAL, queue);
    require(device->dispatchQueue, exit);
    
    _IOHIDObjectInternalRetain(device);
    device->dispatchMach = dispatch_mach_create("IOHIDDeviceDispatchMach", device->dispatchQueue,
                                                ^(dispatch_mach_reason_t reason,
                                                  dispatch_mach_msg_t message,
                                                  mach_error_t error __unused) {
        switch (reason) {
            case DISPATCH_MACH_MESSAGE_RECEIVED: {
                size_t size = 0;
                mach_msg_header_t *header = dispatch_mach_msg_get_msg(message, &size);
                
                __IOHIDUserDeviceQueueCallback(device->queue.port, header, size, device);
                break;
            }
            case DISPATCH_MACH_CANCELED: {
                dispatch_release(device->dispatchMach);
                device->dispatchMach = NULL;
                
                __IOHIDUserDeviceDestroyDevice(device);
                
                if (device->cancelHandler) {
                    (device->cancelHandler)();
                    Block_release(device->cancelHandler);
                    device->cancelHandler = NULL;
                }
                
                dispatch_release(device->dispatchQueue);
                _IOHIDObjectInternalRelease(device);
                break;
            }
            default:
                break;
        }
    });
    
    require_action(device->dispatchMach, exit, _IOHIDObjectInternalRelease(device));
    
exit:
    return;
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceSetCancelHandler
//------------------------------------------------------------------------------
void IOHIDUserDeviceSetCancelHandler(IOHIDUserDeviceRef device, dispatch_block_t handler)
{
    os_assert(!device->cancelHandler && handler);
    
    device->cancelHandler = Block_copy(handler);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceActivate
//------------------------------------------------------------------------------
void IOHIDUserDeviceActivate(IOHIDUserDeviceRef device)
{
    os_assert(device->dispatchQueue && !device->runLoop,
              "Activate failed queue: %p runLoop: %p", device->dispatchQueue, device->runLoop);
    
    if (atomic_fetch_or(&device->dispatchStateMask, kIOHIDDispatchStateActive) & kIOHIDDispatchStateActive) {
        return;
    }
    
    IONotificationPortSetDispatchQueue(device->async.port, device->dispatchQueue);
    
    if (device->setReport.callback ||
        device->getReport.callback ||
        device->getReportWithReturnLength.callback ||
        device->setReportBlock ||
        device->getReportBlock) {
        IOConnectSetNotificationPort(device->connect,
                                     0,
                                     CFMachPortGetPort(device->queue.port),
                                     (uintptr_t)NULL);
    }
    
    dispatch_mach_connect(device->dispatchMach, CFMachPortGetPort(device->queue.port), MACH_PORT_NULL, 0);
    
    if (device->options & kIOHIDUserDeviceCreateOptionStartWhenScheduled) {
        IOConnectCallMethod(device->connect,
                            kIOHIDResourceDeviceUserClientMethodRegisterService,
                            NULL, 0, NULL, 0, NULL, NULL, NULL, NULL);
    }
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceCancel
//------------------------------------------------------------------------------
void IOHIDUserDeviceCancel(IOHIDUserDeviceRef device)
{
    HIDDEBUGTRACE(kHID_UserDev_UnscheduleDispatch, device, 0, 0, 0);
    
    if (atomic_fetch_or(&device->dispatchStateMask, kIOHIDDispatchStateCancelled) & kIOHIDDispatchStateCancelled) {
        return;
    }
    
    os_assert(device->dispatchQueue && !device->runLoop,
              "Unschedule failed queue: %p runLoop: %p", device->dispatchQueue, device->runLoop);
    
    IOConnectSetNotificationPort(device->connect,
                                 0,
                                 MACH_PORT_NULL,
                                 (uintptr_t)NULL);
    
    dispatch_mach_cancel(device->dispatchMach);
}

CFTypeRef IOHIDUserDeviceCopyProperty(IOHIDUserDeviceRef device,
                                      CFStringRef key)
{
    return IORegistryEntryCreateCFProperty(device->userDevice, key, kCFAllocatorDefault, 0);
}

Boolean IOHIDUserDeviceSetProperty(IOHIDUserDeviceRef device,
                                   CFStringRef key,
                                   CFTypeRef property)
{
    return (IOConnectSetCFProperty(device->connect, key, property) == kIOReturnSuccess);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceRegisterGetReportCallback
//------------------------------------------------------------------------------
void IOHIDUserDeviceRegisterGetReportCallback(IOHIDUserDeviceRef device, IOHIDUserDeviceReportCallback callback, void * refcon)
{
    device->getReport.callback  = callback;
    device->getReport.refcon    = refcon;
    
    // if someone scheduled before setting callback..
    if (device->getReport.callback && device->queue.port) {
        IOConnectSetNotificationPort(device->connect,
                                     0,
                                     CFMachPortGetPort(device->queue.port),
                                     (uintptr_t)NULL);
    }
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceRegisterGetReportCallback
//------------------------------------------------------------------------------
void IOHIDUserDeviceRegisterGetReportWithReturnLengthCallback(IOHIDUserDeviceRef device, IOHIDUserDeviceReportWithReturnLengthCallback callback, void * refcon)
{
    device->getReportWithReturnLength.callback  = callback;
    device->getReportWithReturnLength.refcon    = refcon;
    
    // if someone scheduled before setting callback..
    if (device->getReportWithReturnLength.callback && device->queue.port) {
        IOConnectSetNotificationPort(device->connect,
                                     0,
                                     CFMachPortGetPort(device->queue.port),
                                     (uintptr_t)NULL);
    }
}


//------------------------------------------------------------------------------
// IOHIDUserDeviceRegisterSetReportCallback
//------------------------------------------------------------------------------
void IOHIDUserDeviceRegisterSetReportCallback(IOHIDUserDeviceRef device, IOHIDUserDeviceReportCallback callback, void * refcon)
{
    device->setReport.callback  = callback;
    device->setReport.refcon    = refcon;
    
    // if someone scheduled before setting callback..
    if (device->setReport.callback && device->queue.port) {
        IOConnectSetNotificationPort(device->connect,
                                     0,
                                     CFMachPortGetPort(device->queue.port),
                                     (uintptr_t)NULL);
    }
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceRegisterGetReportBlock
//------------------------------------------------------------------------------
void IOHIDUserDeviceRegisterGetReportBlock(IOHIDUserDeviceRef device,
                                           IOHIDUserDeviceGetReportBlock block)
{
    os_assert(!device->getReportBlock, "Get report block already set");
    device->getReportBlock = Block_copy(block);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceRegisterSetReportBlock
//------------------------------------------------------------------------------
void IOHIDUserDeviceRegisterSetReportBlock(IOHIDUserDeviceRef device,
                                           IOHIDUserDeviceSetReportBlock block)
{
    os_assert(!device->setReportBlock, "Set report block already set");
    device->setReportBlock = Block_copy(block);
}

#ifndef min
#define min(a, b) \
    ((a < b) ? a:b)
#endif

//------------------------------------------------------------------------------
// __IOHIDUserDeviceQueueCallback
//------------------------------------------------------------------------------
void __IOHIDUserDeviceQueueCallback(CFMachPortRef port __unused, void *msg __unused, CFIndex size __unused, void *info)
{
    IOHIDUserDeviceRef device = (IOHIDUserDeviceRef)info;
    kern_return_t kr;
    
    HIDDEBUGTRACE(kHID_UserDev_QueueCallback, device, 0, 0, 0);
    
    device->queueCallbackTS = mach_continuous_time();

    if (!device->queue.data) {
        return;
    }

    __IOHIDUserDeviceUpdateUsageAnalytics(device);

    // check entry size
    IODataQueueEntry *  nextEntry;
    uint32_t            dataSize;

    // if queue empty, then stop
    while ((nextEntry = IODataQueuePeek(device->queue.data))) {
    
        IOHIDResourceDataQueueHeader *  header                                                  = (IOHIDResourceDataQueueHeader*)&(nextEntry->data);
        uint64_t                        response[kIOHIDResourceUserClientResponseIndexCount]    = {kIOReturnUnsupported,header->token};
        uint8_t *                       responseReport  = NULL;
        CFIndex                         responseLength  = 0;
        
        // set report
        if ( header->direction == kIOHIDResourceReportDirectionOut ) {
            CFIndex     reportLength    = min(header->length, (nextEntry->size - sizeof(IOHIDResourceDataQueueHeader)));
            uint8_t *   report          = ((uint8_t*)header)+sizeof(IOHIDResourceDataQueueHeader);
            
            ++(device->statistics.setreport);
            
            if ( device->setReport.callback ) {
                HIDDEBUGTRACE(kHID_UserDev_SetReportCallback, device, 0, 0, 0);
                response[kIOHIDResourceUserClientResponseIndexResult] = (*device->setReport.callback)(device->setReport.refcon, header->type, header->reportID, report, reportLength);
            } else if (device->setReportBlock) {
                response[kIOHIDResourceUserClientResponseIndexResult] = (device->setReportBlock)(header->type,
                                                                                                 header->reportID,
                                                                                                 report,
                                                                                                 reportLength);
            } else {
                IOHIDUDLogInfo("set report not handled");
            }
            
        }
        else if ( header->direction == kIOHIDResourceReportDirectionIn ) {
            // RY: malloc our own data that we'll send back to the kernel.
            // I thought about mapping the mem dec from the caller in kernel,  
            // but given the typical usage, it is so not worth it
            responseReport = (uint8_t *)malloc(header->length);
            responseLength = header->length;
            ++(device->statistics.getreport);
            
            if ( device->getReport.callback ) {
                response[kIOHIDResourceUserClientResponseIndexResult] = (*device->getReport.callback)(device->getReport.refcon, header->type, header->reportID, responseReport, responseLength);
            }
            
            if ( device->getReportWithReturnLength.callback ) {
                response[kIOHIDResourceUserClientResponseIndexResult] = (*device->getReportWithReturnLength.callback)(device->getReportWithReturnLength.refcon, header->type, header->reportID, responseReport, &responseLength);
            } else if (device->getReportBlock) {
                response[kIOHIDResourceUserClientResponseIndexResult] = (device->getReportBlock)(header->type,
                                                                                                 header->reportID,
                                                                                                 responseReport,
                                                                                                 &responseLength);
            }
            
            if (!device->getReportWithReturnLength.callback &&
                !device->getReport.callback &&
                !device->getReportBlock) {
                IOHIDUDLogInfo("get report not handled");
            }
        }

        // post the response
        kr = IOConnectCallMethod(device->connect, kIOHIDResourceDeviceUserClientMethodPostReportResponse, response, sizeof(response)/sizeof(uint64_t), responseReport, responseLength, NULL, NULL, NULL, NULL);
        if (kr) {
            IOHIDUDLogError("kIOHIDResourceDeviceUserClientMethodPostReportResponse:%x", kr);
        }
        
        if ( responseReport )
            free(responseReport);
    
        // dequeue the item
        dataSize = 0;
        device->dequeueTS = mach_continuous_time();
        IODataQueueDequeue(device->queue.data, NULL, &dataSize);
    }
}


//------------------------------------------------------------------------------
// __IOHIDUserDeviceHandleReportAsyncCallback
//------------------------------------------------------------------------------
void __IOHIDUserDeviceHandleReportAsyncCallback(void *refcon, IOReturn result)
{
    IOHIDDeviceHandleReportAsyncContext *pContext = (IOHIDDeviceHandleReportAsyncContext *)refcon;
    
    HIDDEBUGTRACE(kHID_UserDev_HandleReportCallback, pContext, 0, 0, 0);
    
    if (pContext->callback)
        pContext->callback(pContext->refcon, result);

    free(pContext);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceHandleReportAsync
//------------------------------------------------------------------------------
IOReturn IOHIDUserDeviceHandleReportAsyncWithTimeStamp(IOHIDUserDeviceRef device, uint64_t timestamp, const uint8_t *report, CFIndex reportLength, IOHIDUserDeviceHandleReportAsyncCallback callback, void * refcon)
{
    IOHIDDeviceHandleReportAsyncContext *pContext = malloc(sizeof(IOHIDDeviceHandleReportAsyncContext));
    
    if (!pContext)
        return kIOReturnNoMemory;

    pContext->callback = callback;
    pContext->refcon = refcon;
    
    mach_port_t wakePort = MACH_PORT_NULL;
    uint64_t asyncRef[kOSAsyncRef64Count];
    
    wakePort = IONotificationPortGetMachPort(device->async.port);
    
    asyncRef[kIOAsyncCalloutFuncIndex] = (uint64_t)(uintptr_t)__IOHIDUserDeviceHandleReportAsyncCallback;
    asyncRef[kIOAsyncCalloutRefconIndex] = (uint64_t)(uintptr_t)pContext;

    return IOConnectCallAsyncMethod(device->connect, kIOHIDResourceDeviceUserClientMethodHandleReport, wakePort, asyncRef, kOSAsyncRef64Count, &timestamp, 1, report, reportLength, NULL, NULL, NULL, NULL);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceHandleReportWithTimeStamp
//------------------------------------------------------------------------------
IOReturn IOHIDUserDeviceHandleReportWithTimeStamp(IOHIDUserDeviceRef device, uint64_t timestamp, const uint8_t * report, CFIndex reportLength)
{
    IOReturn kr;
    HIDDEBUGTRACE(kHID_UserDev_HandleReport, timestamp, device, reportLength, 0);
    ++(device->statistics.handlereport);
    kr = IOConnectCallMethod(device->connect, kIOHIDResourceDeviceUserClientMethodHandleReport, &timestamp, 1, report, reportLength, NULL, NULL, NULL, NULL);
    if (kr) {
        IOHIDUDLogError("kIOHIDResourceDeviceUserClientMethodHandleReport:%x", kr);
    }
    return kr;
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceHandleReport
//------------------------------------------------------------------------------
IOReturn IOHIDUserDeviceHandleReport(IOHIDUserDeviceRef device, const uint8_t * report, CFIndex reportLength)
{
    return IOHIDUserDeviceHandleReportWithTimeStamp(device, mach_absolute_time(), report, reportLength);
}

//------------------------------------------------------------------------------
// IOHIDUserDeviceHandleReportAsync
//------------------------------------------------------------------------------
IOReturn IOHIDUserDeviceHandleReportAsync(IOHIDUserDeviceRef device, const uint8_t * report, CFIndex reportLength, IOHIDUserDeviceHandleReportAsyncCallback callback, void * refcon)
{
    return IOHIDUserDeviceHandleReportAsyncWithTimeStamp(device, mach_absolute_time(), report, reportLength, callback, refcon);
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceSetupAnalytics
//------------------------------------------------------------------------------
bool __IOHIDUserDeviceSetupAnalytics(IOHIDUserDeviceRef device)
{
    bool                result = false;
    CFDictionaryRef     eventDesc = NULL;
    static CFStringRef  keys[] = { CFSTR("staticSize"), CFSTR("queueType") };
    CFTypeRef           values[sizeof(keys)/sizeof(keys[0])] = {0};
    IOHIDAnalyticsHistogramSegmentConfig analyticsConfig = {
        .bucket_count       = 8,
        .bucket_width       = 13,
        .bucket_base        = 0,
        .value_normalizer   = 1,
    };

    values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &device->queue.size);
    values[1] = CFSTR("userDeviceQueue");

    eventDesc = CFDictionaryCreate(kCFAllocatorDefault,
                                   (const void **)keys,
                                   (const void **)values,
                                   sizeof(values)/sizeof(values[0]),
                                   &kCFTypeDictionaryKeyCallBacks,
                                   &kCFTypeDictionaryValueCallBacks);
    require_action(eventDesc, exit, IOHIDUDLogError("Unable to create analytics description"));

    device->queue.usageAnalytics = IOHIDAnalyticsHistogramEventCreate(CFSTR("com.apple.hid.queueUsage"), eventDesc, CFSTR("UsagePercent"), &analyticsConfig, 1);
    require_action(device->queue.usageAnalytics, exit, IOHIDUDLogError("Unable to create queue analytics"));

    IOHIDAnalyticsEventActivate(device->queue.usageAnalytics);

    result = true;

exit:
    if (eventDesc) {
        CFRelease(eventDesc);
    }
    for (size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        if (values[i]) {
            CFRelease(values[i]);
        }
    }
    return result;
}

//------------------------------------------------------------------------------
// __IOHIDUserDeviceUpdateUsageAnalytics
//------------------------------------------------------------------------------
void __IOHIDUserDeviceUpdateUsageAnalytics(IOHIDUserDeviceRef device)
{

    uint32_t head;
    uint32_t tail;
    uint64_t queueUsage;

    require(device->queue.data, exit);
    require(device->queue.usageAnalytics, exit);

    head = (uint32_t)device->queue.data->head;
    tail = (uint32_t)device->queue.data->tail;

    // Submit queue usage at local maximum queue size.
    // (first call to dequeue in a series w/o enqueue)
    if (tail == device->queue.lastTail) {
        return;
    }

    if (head < tail) {
        queueUsage = tail - head;
    }
    else {
        queueUsage = device->queue.size - (head - tail);
    }
    queueUsage = (queueUsage * 100) / device->queue.size;

    IOHIDAnalyticsHistogramEventSetIntegerValue(device->queue.usageAnalytics, queueUsage);

    device->queue.lastTail = tail;

exit:
    return;
}

