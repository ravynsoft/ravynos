/*
 * Copyright (c) 1998-2009 Apple Computer, Inc. All rights reserved.
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
/*
 * Copyright (c) 1998 Apple Computer, Inc.  All rights reserved. 
 *
 * HISTORY
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSByteOrder.h>
#include <bootstrap_priv.h>
#include <mach/mach.h>
#include <mach/mach_time.h>


#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hid/IOHIDLibPrivate.h>
#include <IOKit/pwr_mgt/IOPMLibPrivate.h>
#include <servers/bootstrap.h>
#include "powermanagement.h"
#include <IOKit/hid/IOHIDEventSystemClient.h>
#include <IOKit/hid/IOHIDServiceKeys.h>
#include <os/log.h>
#include <dlfcn.h>
#include "IOHIDLibPrivate.h"
#include <AssertMacros.h>

#if TARGET_OS_OSX
#include <msgtracer_client.h>
#include <msgtracer_keys.h>
#endif //TARGET_OS_OSX

#define IOHID_LOG_CURSOR _IOHIDLogCategory(kIOHIDLogCategoryCursor)

struct SetFixedMouseLocData {
    uint64_t    origTs;
    uint64_t    callTs;
    int32_t     x;
    int32_t     y;
    int32_t     pid;
} __attribute__((packed));

#define TCC_FRAMEWORK  "/System/Library/PrivateFrameworks/TCC.framework/TCC"

typedef enum {
    kTCCAccessPreflightGranted,
    kTCCAccessPreflightDenied,
    kTCCAccessPreflightUnknown
} TCCAccessPreflightResult;

TCCAccessPreflightResult TCCAccessPreflight(CFStringRef service, __unused CFDictionaryRef options);
void TCCAccessRequest(CFStringRef service, CFDictionaryRef options, void (^callback)(Boolean granted));

static typeof (TCCAccessPreflight) *_preflightFunc = NULL;
static typeof (TCCAccessRequest) *_requestFunc = NULL;

static void *__loadTCCFramework()
{
    static void *tccFramework = NULL;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        tccFramework = dlopen(TCC_FRAMEWORK, RTLD_LAZY | RTLD_LOCAL);
        
        if (tccFramework == NULL) {
            IOHIDLogError("Could not load TCC");
            return;
        }
        
        _preflightFunc = dlsym(tccFramework, "TCCAccessPreflight");
        if (_preflightFunc == NULL) {
            IOHIDLogError("Could not find TCC symbol \"TCCAccessPreflight\"");
            return;
        }
        
        _requestFunc = dlsym(tccFramework, "TCCAccessRequest");
        if (_requestFunc == NULL) {
            IOHIDLogError("Could not find TCC symbol \"TCCAccessRequest\"");
            return;
        }
    });
    
    return tccFramework;
}

IOHIDAccessType IOHIDCheckAccess(IOHIDRequestType requestType) {
    TCCAccessPreflightResult tccResult = kTCCAccessPreflightUnknown;
    IOHIDAccessType result = kIOHIDAccessTypeUnknown;
    CFStringRef request = NULL;
    
    require(__loadTCCFramework() && _preflightFunc, exit);
    
    if (requestType == kIOHIDRequestTypePostEvent) {
        request = CFSTR("kTCCServicePostEvent");
    } else if (requestType == kIOHIDRequestTypeListenEvent) {
        request = CFSTR("kTCCServiceListenEvent");
    }
    
    require(request, exit);
    
    tccResult = _preflightFunc(request, NULL);
    
    switch (tccResult) {
        case kTCCAccessPreflightGranted:
            result = kIOHIDAccessTypeGranted;
            break;
        case kTCCAccessPreflightDenied:
            result = kIOHIDAccessTypeDenied;
            break;
        case kTCCAccessPreflightUnknown:
            result = kIOHIDAccessTypeUnknown;
            break;
        default:
            break;
    }
    
exit:
    return result;
}

bool IOHIDRequestAccess(IOHIDRequestType requestType) {
    __block bool result = false;
    __block dispatch_semaphore_t semaphore = 0;
    CFStringRef request = NULL;
    
    require(__loadTCCFramework() && _requestFunc, exit);
    
    if (requestType == kIOHIDRequestTypePostEvent) {
        request = CFSTR("kTCCServicePostEvent");
    } else if (requestType == kIOHIDRequestTypeListenEvent) {
        request = CFSTR("kTCCServiceListenEvent");
    }
    
    require(request, exit);
    
    semaphore = dispatch_semaphore_create(0);
    require(semaphore, exit);
    
    _requestFunc(request, NULL, ^(Boolean granted) {
        result = granted;
        dispatch_semaphore_signal(semaphore);
    });
    
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    dispatch_release(semaphore);
    
exit:
    return result;
}

kern_return_t
IOHIDCreateSharedMemory( io_connect_t connect,
	unsigned int version )
{
    uint64_t inData = version;
    return IOConnectCallMethod( connect, 0,		// Index
			   &inData, 1, NULL, 0,		// Input
			   NULL, NULL, NULL, NULL);	// Output
}

kern_return_t
IOHIDSetEventsEnable( io_connect_t connect,
	boolean_t enable )
{
    uint64_t inData = enable;
    return IOConnectCallMethod( connect, 1,		// Index
			   &inData, 1, NULL, 0,		// Input
			   NULL, NULL, NULL, NULL);	// Output
}

kern_return_t
IOHIDSetCursorEnable( io_connect_t connect,
	boolean_t enable )
{
    uint64_t        inData = enable;
    kern_return_t   ret;

    os_log_info(IOHID_LOG_CURSOR, "Set cursor enable:%s", enable ? "YES" : "NO");

    ret = IOConnectCallMethod(connect, 2,               // Index
                              &inData, 1, NULL, 0,      // Input
                              NULL, NULL, NULL, NULL);	// Output
    if (ret != KERN_SUCCESS) {
        os_log_error(IOHID_LOG_CURSOR, "Set cursor enable failed:0x%x", ret);
    }

    return ret;
}

/* DEPRECATED form of IOHIDPostEvent().
kern_return_t
IOHIDPostEvent( mach_port_t connect,
                int type, IOGPoint location, NXEventData *data,
                boolean_t setCursor, int flags, boolean_t setFlags)
*/

static bool _IOPMReportSoftwareHIDEvent(UInt32 eventType)
{
    mach_port_t         newConnection;
    kern_return_t       kern_result = KERN_SUCCESS;
    int                 allowEvent = true;

    kern_result = bootstrap_look_up2(bootstrap_port, 
                                     kIOPMServerBootstrapName, 
                                     &newConnection, 
                                     0, 
                                     BOOTSTRAP_PRIVILEGED_SERVER);    
    if(KERN_SUCCESS == kern_result) {
        io_pm_hid_event_report_activity(newConnection, eventType, &allowEvent);
        mach_port_deallocate(mach_task_self(), newConnection);
    }
    return allowEvent;
}

#if TARGET_OS_OSX
static void _LOGIOHIDPostEventCaller(void)
{
    // This will return bundle re for executable bundle
    // eg App calling post event
    CFBundleRef appBundleRef = CFBundleGetMainBundle();
    CFStringRef appBundleName = NULL;
    
    if (appBundleRef) {
        appBundleName = CFBundleGetValueForInfoDictionaryKey(appBundleRef, CFSTR("CFBundleIdentifier"));
    }
    
    //Log call to message tracer
    if (!appBundleName) {
        return;
    }
    
    
    msgtracer_log_with_keys("com.apple.iokituser.hid.iohidpostevent", ASL_LEVEL_NOTICE,
                            kMsgTracerKeySignature, CFStringGetCStringPtr(appBundleName, kCFStringEncodingASCII),
                            kMsgTracerKeySummarize, "YES",
                            NULL);
}
#endif //TARGET_OS_OSX

kern_return_t
IOHIDPostEvent( io_connect_t        connect,
                UInt32              eventType,
                IOGPoint            location,
                const NXEventData * eventData,
                UInt32              eventDataVersion,
                IOOptionBits        eventFlags,
                IOOptionBits        options )
{
    int *               eventPid = 0;
    size_t              dataSize = sizeof(struct evioLLEvent) + sizeof(int);
    char                data[dataSize];
    struct evioLLEvent* event;
    UInt32              eventDataSize = sizeof(NXEventData);
    int                 allowEvent = true;
    static dispatch_once_t onceToken;

    bzero(data, dataSize);
    
    event = (struct evioLLEvent*)data;
    
    event->type      = eventType;
    event->location  = location;
    event->flags     = eventFlags;
    event->setFlags  = options;
    event->setCursor = options & (kIOHIDSetCursorPosition | kIOHIDSetRelativeCursorPosition);
    
    eventPid = (int *)(event + 1);
    *eventPid = getpid();

#if TARGET_OS_OSX
    
    //track caller of API
    _LOGIOHIDPostEventCaller();
    
#endif //TARGET_OS_OSX
    
    if ( eventDataVersion < 2 )
    {
        // Support calls from legacy IOHIDPostEvent clients.
        // 1. NXEventData was 32 bytes long.
        // 2. eventDataVersion was (boolean_t) setCursor
        eventDataSize   = 32;
        event->setCursor = eventDataVersion; // 0 or 1
    }

    if ( eventDataSize < sizeof(event->data) )
    {
        bcopy( eventData, &(event->data), eventDataSize );
        bzero( ((UInt8 *)(&(event->data))) + eventDataSize,
               sizeof(event->data) - eventDataSize );
    }
    else
        bcopy( eventData, &event->data, sizeof(event->data) );


    // Let PM log the software HID events
    // also checks if NULL events are allowed in the current system state
    allowEvent  = _IOPMReportSoftwareHIDEvent(event->type);

    if (allowEvent) {
        
        if ((event->setCursor & kIOHIDSetCursorPosition) || event->type != NX_NULLEVENT) {
            dispatch_once(&onceToken, ^{
                IOHIDRequestAccess(kIOHIDRequestTypePostEvent);
            });
        }
        
        return IOConnectCallMethod(connect, 3,		// Index
                   NULL, 0,    data, dataSize,	// Input
                   NULL, NULL, NULL, NULL);	// Output
    }
    else {
        return KERN_SUCCESS;
    }
}

extern kern_return_t
IOHIDSetCursorBounds( io_connect_t connect, const IOGBounds * bounds )
{
    kern_return_t ret;

	if ( !bounds )
		return kIOReturnBadArgument;

    os_log_info(IOHID_LOG_CURSOR, "Set cursor bounds minx:%d miny:%d maxx:%d maxy:%d",
           bounds->minx, bounds->miny, bounds->maxx, bounds->maxy);

	ret = IOConnectCallMethod(connect,    6,                        // Index
                              NULL, 0,    bounds, sizeof(*bounds),  // Input,
                              NULL, NULL, NULL,   NULL);            // Output
    if (ret != KERN_SUCCESS) {
        os_log_error(IOHID_LOG_CURSOR, "Set cursor bounds failed:0x%x", ret);
    }

    return ret;
}

extern kern_return_t
IOHIDSetOnScreenCursorBounds( io_connect_t connect, const IOGPoint * point, const IOGBounds * bounds )
{
    kern_return_t ret;

    if ( !bounds || !point )
        return kIOReturnBadArgument;

    os_log_info(IOHID_LOG_CURSOR, "Set on screen cursor bounds px:%d py:%d minx:%d miny:%d maxx:%d maxy:%d",
           point->x, point->y, bounds->minx, bounds->miny, bounds->maxx, bounds->maxy);
    
    int16_t   data[6] = {point->x, point->y, bounds->minx, bounds->miny, bounds->maxx, bounds->maxy};
    
    ret = IOConnectCallMethod(connect, 12,                      // Index
                              NULL, 0,    data, sizeof(data),   // Input,
                              NULL, NULL, NULL,   NULL);        // Output
    if (ret != KERN_SUCCESS) {
        os_log_error(IOHID_LOG_CURSOR, "Set on screen cursor bounds failed:0x%x", ret);
    }

    return ret;
}

kern_return_t
IOHIDSetMouseLocation( io_connect_t connect, int x, int y )
{
    NXEventData	event;
    IOGPoint   location = {x,y};
    memset(&event, 0, sizeof(event));
    return IOHIDPostEvent(connect, NX_NULLEVENT, location, &event, 2, -1, kIOHIDSetCursorPosition);
}

kern_return_t
_IOHIDSetFixedMouseLocation( io_connect_t connect, struct SetFixedMouseLocData *data)
{
    kern_return_t ret = IOConnectCallMethod(connect, 4,                                             // Index
                                            NULL, 0,    data, sizeof(struct SetFixedMouseLocData),  // Input
                                            NULL, NULL, NULL, NULL);                                // Output

    if (ret != KERN_SUCCESS) {
        os_log_debug(IOHID_LOG_CURSOR, "Set fixed mouse location failed:0x%x", ret);
    }

    return ret;
}

kern_return_t
IOHIDSetFixedMouseLocation( io_connect_t connect, int32_t x, int32_t y )
{
    struct SetFixedMouseLocData data = {0};

    data.callTs = mach_absolute_time();
    data.x = x;
    data.y = y;
    data.pid = getpid();
    
    return _IOHIDSetFixedMouseLocation(connect, &data);
}

kern_return_t
IOHIDSetFixedMouseLocationWithTimeStamp( io_connect_t connect, int32_t x, int32_t y, uint64_t timestamp )
{
    struct SetFixedMouseLocData data = {0};

    data.origTs = timestamp;
    data.callTs = mach_absolute_time();
    data.x = x;
    data.y = y;
    data.pid = getpid();

    return _IOHIDSetFixedMouseLocation(connect, &data);
}

kern_return_t
IOHIDGetButtonEventNum( io_connect_t connect,
	NXMouseButton button, int * eventNum )
{
    kern_return_t	err;

	uint64_t inData = button;
	uint64_t outData;
	uint32_t outSize = 1;
	err = IOConnectCallMethod(connect, 5,						// Index
						  &inData, 1, NULL, 0,				// Input
						  &outData, &outSize, NULL, NULL);	// Output
	*eventNum = (int) outData;
    return( err);
}

kern_return_t
IOHIDGetStateForSelector( io_connect_t handle, int selector, UInt32 *state )
{
    kern_return_t err;
    uint64_t        inData[1] = {selector};
    uint64_t        outData[1] = {0};
    uint32_t        outCount = 1;
    err = IOConnectCallMethod(handle, 5,      // Index
                              inData, 1, NULL, 0,    // Input
                              outData, &outCount, NULL, NULL); // Output
    
    *state = outData[0];
    return err;
}

kern_return_t
IOHIDSetStateForSelector( io_connect_t handle, int selector, UInt32 state )
{
    kern_return_t err;
    uint64_t        inData[2] = {selector, state};
    uint32_t        outCount = 0;
    
    if (selector == kIOHIDActivityUserIdle) {
    
        io_service_t service;
        
        err = IOConnectGetService(handle, &service);
        
        if (err) {
            IOHIDLogError("IOConnectGetService Failed with err : 0x%x", err);
            return err;
        }
        
        CFNumberRef property = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &state);
        
        err = IORegistryEntrySetCFProperty(service, CFSTR(kIOHIDActivityUserIdleKey), property);
        
        if (err) {
            IOHIDLogError("IORegistryEntrySetCFProperty Failed for kIOHIDActivityUserIdleKey with err : 0x%x", err);
        }
       
        if (property) {
            CFRelease(property);
        }
        
        
    } else {
    
        
        err = IOConnectCallMethod(handle, 6,      // Index
                              inData, 2, NULL, 0,    // Input
                              NULL, &outCount, NULL, NULL); // Output
    }
    
    return err;
}


kern_return_t
IOHIDGetModifierLockState( io_connect_t handle, int selector, bool *state )
{
    UInt32 result = 0;
    kern_return_t err = IOHIDGetStateForSelector(handle, selector, &result);
    *state = result ? true : false;
    return err;
}

kern_return_t
IOHIDSetModifierLockState( io_connect_t handle __unused, int selector, bool state )
{
    kern_return_t               err = kIOReturnSuccess;
    IOHIDEventSystemClientRef   client = NULL;
    CFArrayRef                  services = NULL;
  
    if (selector != kIOHIDCapsLockState &&  selector != kIOHIDNumLockState) {
        err = kIOReturnBadArgument;
        goto exit;
    }
    
    client = IOHIDEventSystemClientCreateWithType (kCFAllocatorDefault, kIOHIDEventSystemClientTypePassive, NULL);
    if (client == NULL) {
        IOHIDLogError("Failed to create event system client");
        return kIOReturnError;
    }
    
    services = (CFArrayRef) IOHIDEventSystemClientCopyServices (client);
    if (services == NULL || CFArrayGetCount(services) == 0) {
        err = kIOReturnNoDevice;
        goto exit;
    }
    
    for (CFIndex index = 0; index < CFArrayGetCount(services); index++) {
        IOHIDServiceClientRef service = (IOHIDServiceClientRef) CFArrayGetValueAtIndex(services, index);
        if (service && IOHIDServiceClientConformsTo (service, kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard)) {
            CFStringRef key = (selector == kIOHIDCapsLockState) ?  CFSTR(kIOHIDServiceCapsLockStateKey) : CFSTR(kIOHIDServiceNumLockStateKey);
            IOHIDServiceClientSetProperty(service, key, state ? kCFBooleanTrue : kCFBooleanFalse);
        }
    }
    
exit:
    if (services) {
        CFRelease(services);
    }
    if (client) {
        CFRelease(client);
    }
    return err;
}

kern_return_t
IOHIDRegisterVirtualDisplay( io_connect_t handle, UInt32 *display_token )
{
    kern_return_t err;
    uint64_t        outData[1] = {0};
    uint32_t        outCount = 1;
    
    err = IOConnectCallMethod(handle, 7,      // Index
                              NULL, 0, NULL, 0,    // Input
                              outData, &outCount, NULL, NULL); // Output
    *display_token = outData[0];
    return err;
}

kern_return_t
IOHIDUnregisterVirtualDisplay( io_connect_t handle, UInt32 display_token )
{
    kern_return_t err;
    uint64_t        inData[1] = {display_token};
    uint32_t        outCount = 0;
    
    err = IOConnectCallMethod(handle, 8,      // Index
                              inData, 1, NULL, 0,    // Input
                              NULL, &outCount, NULL, NULL); // Output
    
    return err;
}

kern_return_t
IOHIDSetVirtualDisplayBounds( io_connect_t handle, UInt32 display_token, const IOGBounds * bounds )
{
    kern_return_t err;
    uint64_t        inData[5] = {display_token, bounds->minx, bounds->maxx, bounds->miny, bounds->maxy};
    uint32_t        outCount = 0;
    
    err = IOConnectCallMethod(handle, 9,      // Index
                              inData, 5, NULL, 0,    // Input
                              NULL, &outCount, NULL, NULL); // Output
    
    return err;
}

kern_return_t
IOHIDGetActivityState( io_connect_t handle, bool *hidActivityIdle )
{
    kern_return_t err;
    uint64_t        outData[1] = {0};
    uint32_t        outCount = 1;

    if (!hidActivityIdle) return kIOReturnBadArgument;
    
    err = IOConnectCallMethod(handle, 10,      // Index
                              NULL, 0, NULL, 0,    // Input
                              outData, &outCount, NULL, NULL); // Output
    *hidActivityIdle = outData[0] ? true : false;
    return err;

}
