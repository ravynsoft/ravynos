/*
 * Copyright (c) 2004, 2012 Apple Computer, Inc. All rights reserved.
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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFXPCBridge.h>
#include <TargetConditionals.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/pwr_mgt/IOPMPrivate.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/vm_map.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <notify.h>
#include <os/log.h>
#include "IOSystemConfiguration.h"
#include "IOPMLibPrivate.h"
#include "powermanagement.h"
#include <asl.h>
#include <os/lock_private.h>
#include <xpc/xpc.h>

#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>


#define POWERD_XPC_ID   "com.apple.iokit.powerdxpc"
#define pwrLogDirName "/System/Library/PowerEvents"

static const int kMaxNameLength = 128;
static mach_port_t powerd_connection = MACH_PORT_NULL;
static os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;


static void _reset_connection( )
{
  powerd_connection = MACH_PORT_NULL;
  lock = OS_UNFAIR_LOCK_INIT;
}   

IOReturn _pm_connect(mach_port_t *newConnection)
{
    kern_return_t       kern_result = KERN_SUCCESS;
    IOReturn            ret = kIOReturnSuccess;
    
    if(!newConnection) return kIOReturnBadArgument;
    if (powerd_connection != MACH_PORT_NULL) {
        *newConnection = powerd_connection;

        return kIOReturnSuccess;
    }

    os_unfair_lock_lock(&lock);

    // Check again and see if the port is created by another thread 
    if (powerd_connection != MACH_PORT_NULL) {
        *newConnection = powerd_connection;
        goto exit;
    }
    // open reference to powerd
    kern_result = bootstrap_look_up2(bootstrap_port, 
                                     kIOPMServerBootstrapName, 
                                     &powerd_connection, 
                                     0, 
                                     BOOTSTRAP_PRIVILEGED_SERVER);    
    if(KERN_SUCCESS != kern_result) {
        *newConnection = powerd_connection =  MACH_PORT_NULL;
        asl_log(NULL, NULL, ASL_LEVEL_ERR, 
                "bootstrap_look_up2 failed with 0x%x\n", kern_result);
        ret = kIOReturnError;
        goto exit;
    }
    *newConnection = powerd_connection;
    if (pthread_atfork(NULL, NULL, _reset_connection) != 0)
       powerd_connection = MACH_PORT_NULL;

exit:
    os_unfair_lock_unlock(&lock);

    return ret;
}

IOReturn _pm_disconnect(mach_port_t connection __unused)
{
    // Do nothing. We re-use the mach port
    return kIOReturnSuccess;
}

dispatch_queue_t  getPMQueue()
{
    static dispatch_once_t pmQueue_pred;
    static dispatch_queue_t pmQueue = NULL;

    if (!pmQueue) {
        dispatch_once(&pmQueue_pred, ^{
                pmQueue = dispatch_queue_create("PM Notifications", NULL);
                });
        if (!pmQueue) {
            os_log_error(OS_LOG_DEFAULT, "Failed to create PM queue\n");
            return NULL;
        }
    }

    return pmQueue;
}

bool IOPMUserIsActive(void)
{
    io_service_t                service = IO_OBJECT_NULL;
    CFBooleanRef                userIsActiveBool = NULL;
    bool                        ret_val = false;
    
    service = IORegistryEntryFromPath(kIOMasterPortDefault, 
            kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");

    if (IO_OBJECT_NULL != service) {
        userIsActiveBool = IORegistryEntryCreateCFProperty(
                       service,
                       CFSTR(kIOPMUserIsActiveKey),
                       kCFAllocatorDefault, 0);
        IOObjectRelease(service);
    }
    
    ret_val = (kCFBooleanTrue == userIsActiveBool);

    if (userIsActiveBool) {
        CFRelease(userIsActiveBool);
    }
    return ret_val;
}

typedef struct {

    dispatch_queue_t    clientQueue;
// UserActive
    void (^callblock_bool)(bool);
    IONotificationPortRef           notify;
    io_object_t                     letItGo;

// UserActivityLevel
    void (^callblock_activity)(uint64_t, uint64_t);
    int             dtoken;
    xpc_object_t    connection;
    uint32_t        idleTimeout;
    uint64_t            levels;
} _UserActiveNotification;

void IOPMUserDidChangeCallback(
    void *refcon __unused,
    io_service_t service __unused,
    uint32_t messageType,
    void *messageArgument __unused)
{
    _UserActiveNotification *_useractive = (_UserActiveNotification *)refcon;
    
    if (_useractive && (messageType == kIOPMMessageUserIsActiveChanged))
    {
        dispatch_async(_useractive->clientQueue, ^{
            _useractive->callblock_bool( IOPMUserIsActive() );
        });
    }
}


static void unregisterNotification(IOPMNotificationHandle handle)
{
    _UserActiveNotification *_useractive = (_UserActiveNotification *)handle;
    if (!_useractive)
    {
        return;
    }
    if (_useractive->callblock_bool) {
        Block_release(_useractive->callblock_bool);
    }
    if (_useractive->callblock_activity) {
        Block_release(_useractive->callblock_activity);
    }
    if (_useractive->notify) {
        IONotificationPortDestroy(_useractive->notify);
    }
    if (IO_OBJECT_NULL != _useractive->letItGo) {
        IOObjectRelease(_useractive->letItGo);
    }
    if (_useractive->dtoken) {
        notify_cancel(_useractive->dtoken);
    }
    if (_useractive->connection) {
        xpc_release(_useractive->connection);
    }
    if (_useractive->clientQueue) {
        dispatch_release(_useractive->clientQueue);
    }
    bzero(_useractive, sizeof(*_useractive));
    free(_useractive);
}

IOPMNotificationHandle IOPMScheduleUserActiveChangedNotification(dispatch_queue_t queue, void (^block)(bool))
{
    _UserActiveNotification *_useractive = NULL;
    io_registry_entry_t     service = IO_OBJECT_NULL;
    kern_return_t           kr = KERN_INVALID_VALUE;
    
    _useractive = calloc(1, sizeof(_UserActiveNotification));
    
    if (_useractive)
    {
        _useractive->callblock_bool = Block_copy(block);
        _useractive->clientQueue = queue;
        dispatch_retain(queue);
        
        _useractive->notify = IONotificationPortCreate(MACH_PORT_NULL);
        if (_useractive->notify) {
            IONotificationPortSetDispatchQueue(_useractive->notify, getPMQueue());
        }
        
        service = IORegistryEntryFromPath(kIOMasterPortDefault,
                kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");

        kr = IOServiceAddInterestNotification(
                _useractive->notify, service, kIOGeneralInterest,
                IOPMUserDidChangeCallback, (void *)_useractive, &_useractive->letItGo);
        
        IOObjectRelease(service);
    }
    
    if (kIOReturnSuccess != kr) {
        unregisterNotification((IOPMNotificationHandle)_useractive);
        _useractive = NULL;
    }
    
    return _useractive;
}


void IOPMUnregisterNotification(IOPMNotificationHandle handle)
{
    dispatch_sync(getPMQueue(), ^{
        _UserActiveNotification *_useractive = (_UserActiveNotification *)handle;
        if (_useractive && _useractive->connection) {
            xpc_connection_cancel(_useractive->connection);
        }
    });
}

static void decodeUserActivityLevel64(uint64_t in,
                                      uint64_t *outActive,
                                      uint64_t *outMSA)
{
    if (outActive) {
        *outActive = in;
    }
    if (outMSA) {
        *outMSA = in ? (in & (1 << (ffsl(in)-1))) : 0;
    }
}


IOReturn IOPMGetUserActivityLevel(uint64_t *outUserActive,
                                  uint64_t *mostSignificantActivity)
{
    int         token = 0;
    uint64_t    payload = 0;
    uint32_t    r = 0;

    r = notify_register_check("com.apple.system.powermanagement.useractivity2",
                              &token);

    if (NOTIFY_STATUS_OK == r)
    {
        notify_get_state(token, &payload);
        notify_cancel(token);
    }

    decodeUserActivityLevel64(payload,
                              outUserActive,
                              mostSignificantActivity);


    return kIOReturnSuccess;
}

bool sendUserActivityMsg(_UserActiveNotification *_useractive, char *key)
{
    xpc_object_t            desc = NULL;
    xpc_object_t            msg = NULL;


    dispatch_assert_queue(getPMQueue());
    desc = xpc_dictionary_create(NULL, NULL, 0);
    msg = xpc_dictionary_create(NULL, NULL, 0);
    if (desc && msg) {
        xpc_dictionary_set_uint64(desc, kUserActivityTimeoutKey, _useractive->idleTimeout);
        xpc_dictionary_set_value(msg, key, desc);

        xpc_connection_send_message(_useractive->connection, msg);
        xpc_release(msg);
        xpc_release(desc);
        return true;
    }
    else {
        os_log_error(OS_LOG_DEFAULT, "Failed to create xpc objects to send userActivityRegister message\n");
        return false;
    }

}

void processUserActivityMsg(_UserActiveNotification *_useractive, xpc_object_t msg)
{
    xpc_type_t type = xpc_get_type(msg);

    if (type == XPC_TYPE_DICTIONARY) {
        _useractive->levels = xpc_dictionary_get_uint64(msg, kUserActivityLevels);
        uint64_t significant = _useractive->levels ? (1 << (ffsl(_useractive->levels)-1)) : 0;
        void (^block)(uint64_t, uint64_t) = Block_copy(_useractive->callblock_activity);
        uint64_t levels = _useractive->levels;

        dispatch_async(_useractive->clientQueue, ^{
            block(levels, significant);
            Block_release(block);
        });
    }
    else if (type == XPC_TYPE_ERROR) {
        if (msg == XPC_ERROR_CONNECTION_INTERRUPTED) {
            dispatch_async(getPMQueue(), ^{sendUserActivityMsg(_useractive, kUserActivityRegister);});
        }
        else {
            unregisterNotification(_useractive);
        }
    }
}

IOPMNotificationHandle IOPMScheduleUserActivityLevelNotificationWithTimeout(
                               dispatch_queue_t queue,
                               uint32_t timeout,
                               void (^inblock)(uint64_t, uint64_t))
{
#if TARGET_OS_SIMULATOR
    return NULL;
#else
    xpc_object_t connection;
    _UserActiveNotification *_useractive = NULL;
    _useractive = calloc(1, sizeof(_UserActiveNotification));
    __block bool msgSent = false;

    if (!_useractive) {
        return NULL;
    }

    connection = xpc_connection_create_mach_service(POWERD_XPC_ID, dispatch_get_main_queue(), 0);
    if (!connection) {
        free(_useractive);
        return NULL;
    }
    xpc_connection_set_target_queue(connection, getPMQueue());
    xpc_connection_set_event_handler(connection,
            ^(xpc_object_t msg ) {processUserActivityMsg(_useractive, msg); });

    _useractive->connection = connection;
    dispatch_retain(queue);
    _useractive->clientQueue = queue;

    _useractive->callblock_activity = Block_copy(inblock);
    _useractive->idleTimeout = timeout;
    xpc_connection_resume(connection);
    dispatch_sync(getPMQueue(), ^{
        msgSent = sendUserActivityMsg(_useractive, kUserActivityRegister);
    });
    if (msgSent == false) {
        IOPMUnregisterNotification(_useractive);
        return NULL;
    }

    return _useractive;
#endif // TARGET_OS_SIMULATOR
}

IOReturn IOPMSetUserActivityIdleTimeout(IOPMNotificationHandle handle, uint32_t timeout)
{
    _UserActiveNotification *_useractive = (_UserActiveNotification *)handle;
    __block IOReturn rc = kIOReturnInternalError;

    dispatch_sync(getPMQueue(), ^{
        if (!_useractive || !_useractive->connection) {
            rc =  kIOReturnBadArgument;
        }
        else {
            _useractive->idleTimeout = timeout;
            if (sendUserActivityMsg(_useractive, kUserActivityTimeoutUpdate)) {
                rc = kIOReturnSuccess;
            }
        }
    });

    return rc;
}

IOPMNotificationHandle IOPMScheduleUserActivityLevelNotification(dispatch_queue_t queue,
                                                                 void (^inblock)(uint64_t, uint64_t))
{
    _UserActiveNotification *_useractive = NULL;
    uint32_t                r = 0;

    _useractive = calloc(1, sizeof(_UserActiveNotification));

    if (!_useractive) {
        return NULL;
    }

    notify_handler_t calloutBlock = ^(int token)
    {
        uint64_t data = 0;
        uint64_t active = 0;
        uint64_t most = 0;
        uint32_t r2;

        r2 = notify_get_state(token, &data);
        if (NOTIFY_STATUS_OK == r2)
        {
            decodeUserActivityLevel64(data, &active, &most);

            if (_useractive)
                inblock(active, most);
        }
    };

    r = notify_register_dispatch("com.apple.system.powermanagement.useractivity2",
                                 &_useractive->dtoken,
                                 queue,
                                 calloutBlock);
    if (NOTIFY_STATUS_OK != r) {
        free(_useractive);
        _useractive = NULL;
    }

    return _useractive;
}



CFStringRef IOPMCopyUserActivityLevelDescription(uint64_t userActive)
{
    int                prev = 0;
    CFMutableStringRef  result = CFStringCreateMutable(0, 0);

    if (0==userActive) {
        CFStringAppend(result, CFSTR("Inactive"));
        goto exit;
    }

    if (kIOPMUserPresentActive & userActive) {
        if (prev++) CFStringAppend(result,CFSTR(" "));
        CFStringAppend(result, CFSTR("PresentActive"));
    }
    if (kIOPMUserPresentPassive & userActive) {
        if (prev++) CFStringAppend(result,CFSTR(" "));
        CFStringAppend(result, CFSTR("PresentPassive"));
    }
    if (kIOPMUserRemoteClientActive & userActive) {
        if (prev++) CFStringAppend(result,CFSTR(" "));
        CFStringAppend(result, CFSTR("RemoteActive"));
    }
    if (kIOPMUserNotificationActive  & userActive) {
        if (prev++) CFStringAppend(result,CFSTR(" "));
        CFStringAppend(result, CFSTR("NotificationActive"));
    }
exit:
    return result;
}

typedef struct {
    void (^callblock)(IOMessage msgType, void *msgData);
    io_object_t             letItGo;
    IONotificationPortRef   port;
    io_connect_t            svc;
    dispatch_queue_t        clientQueue;
    uint32_t                pendingMessage;
    void                    *messageArgument;
} _powerNotification;

IOReturn IOPMAllowRemotePowerChange(IOPMNotificationHandle ref, IOMessage  message)
{
    __block IOReturn ret = kIOReturnSuccess;
    _powerNotification *handle = (_powerNotification *)ref;
    if (!handle) {
        return kIOReturnBadArgument;
    }
    dispatch_sync(getPMQueue(), ^{
        uint64_t  inData = (uint64_t)handle->messageArgument;
        if (message != handle->pendingMessage) {
            ret = kIOReturnBadArgument;
            return;
        }

        ret = IOConnectCallScalarMethod(
                        handle->svc, kPMAllowPowerChange,
                        &inData, 1, NULL, NULL);

    });

    return ret;
}

void _systemPowerCallback(
    void *refcon __unused,
    io_service_t service __unused,
    uint32_t messageType,
    void *messageArgument)
{
    // This function gets called on pmQueue
    _powerNotification *handle = (_powerNotification *)refcon;

    handle->pendingMessage = messageType;
    handle->messageArgument = messageArgument;
    dispatch_async(handle->clientQueue, ^{
        handle->callblock(messageType, NULL);
    });
}


IOPMNotificationHandle IOPMRegisterForRemoteSystemPower(
                            dispatch_queue_t queue,
                            void (^inblock)(IOMessage msgType, void *msgData))
{

    _powerNotification     *handle = NULL;
    io_service_t            object = IO_OBJECT_NULL;
    kern_return_t           kr = KERN_INVALID_VALUE;

    if (!queue) {
        return NULL;
    }
    handle = calloc(1, sizeof(_powerNotification));

    if (!handle) {
        goto failure_exit;
    }
    dispatch_retain(queue);
    handle->clientQueue = queue;

    handle->callblock= Block_copy(inblock);

    handle->port = IONotificationPortCreate(MACH_PORT_NULL);
    if (handle->port) {
        IONotificationPortSetDispatchQueue(handle->port, getPMQueue());
    }

    object = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleEmbeddedSleepWakeHandler"));
    if (!object) {
        goto failure_exit;
    }
    kr = IOServiceOpen( object, mach_task_self(), 0, &handle->svc);

    if ( (kr != kIOReturnSuccess) || (handle->svc == IO_OBJECT_NULL) )  {
        goto failure_exit;
    }

    kr = IOServiceAddInterestNotification(
            handle->port, object, kIOGeneralInterest,
            _systemPowerCallback, (void *)handle, &handle->letItGo);

    IOObjectRelease(object);
    object = IO_OBJECT_NULL;

    if (kIOReturnSuccess != kr) {
        goto failure_exit;
    }

    return handle;

failure_exit:
    if (object) {
        IOObjectRelease(object);
    }
    if (handle) {
        IODeregisterForRemoteSystemPower((IOPMNotificationHandle)handle);
    }
    return NULL;

}

void IODeregisterForRemoteSystemPower(IOPMNotificationHandle ref)
{
    _powerNotification *handle = (_powerNotification *)ref;
    if (!handle) {
        return;
    }

    dispatch_sync(getPMQueue(), ^{
        if (handle->clientQueue) {
            dispatch_release(handle->clientQueue);
        }
        if (handle->callblock) {
            Block_release(handle->callblock);
        }
        if (handle->port) {
            IONotificationPortDestroy(handle->port);
        }
        if (IO_OBJECT_NULL != handle->letItGo) {
            IOObjectRelease(handle->letItGo);
        }
        if (handle->svc) {
            IOServiceClose(handle->svc);
        }
        bzero(handle, sizeof(*handle));
        free(handle);
    });

}

/*****************************************************************************/
/*****************************************************************************/

/******************************************************************************
 * IOPMCopyHIDPostEventHistory
 *
 ******************************************************************************/

IOReturn IOPMCopyHIDPostEventHistory(CFArrayRef *outArray)
{
    CFDataRef               serializedData = NULL;
    vm_address_t            outBuffer = 0;
    vm_size_t               outSize = 0;
    mach_port_t             pmserverport = MACH_PORT_NULL;
    IOReturn                ret = kIOReturnError;
    int                     history_return = 0;
    
    if (kIOReturnSuccess != _pm_connect(&pmserverport))
        goto exit;

    if (KERN_SUCCESS != io_pm_hid_event_copy_history(pmserverport, 
                                &outBuffer, (mach_msg_type_number_t *) &outSize, &history_return))
    {
        goto exit;
    }

    serializedData = CFDataCreate(0, (const UInt8 *)outBuffer, outSize);
    if (serializedData) {
        *outArray = (CFArrayRef)CFPropertyListCreateWithData(0, serializedData, 0, NULL, NULL);
        CFRelease(serializedData);
    }
 
    if (*outArray)
        ret = kIOReturnSuccess;
 
    vm_deallocate(mach_task_self(), outBuffer, outSize);
    _pm_disconnect(pmserverport);
exit:
    return ret;
}

/******************************************************************************
 * IOPMGetLastWakeTime
 *
 ******************************************************************************/

IOReturn IOPMGetLastWakeTime(
    CFAbsoluteTime      *lastWakeTimeOut,
    CFTimeInterval      *adjustedForPhysicalWakeOut)
{
    IOReturn            ret;
    CFTimeInterval      lastSMCS3S0WakeInterval = 0.0;
    CFAbsoluteTime      lastWakeTime;
    struct timeval      rawLastWakeTime;
    size_t              rawLastWakeTimeSize = sizeof(rawLastWakeTime);

    if (!lastWakeTimeOut || !adjustedForPhysicalWakeOut) {
        return kIOReturnBadArgument;
    }
    
    *lastWakeTimeOut = 0.0;
    *adjustedForPhysicalWakeOut = 0.0;

    ret = sysctlbyname("kern.waketime", &rawLastWakeTime, &rawLastWakeTimeSize, NULL, 0);
    if (ret || !rawLastWakeTime.tv_sec) {
        return kIOReturnNotReady;
    }

    // Convert the timeval, which is in UNIX time, to a CFAbsoluteTime
    lastWakeTime = rawLastWakeTime.tv_sec + (rawLastWakeTime.tv_usec / 1000000.0);
    lastWakeTime -= kCFAbsoluteTimeIntervalSince1970;


    *lastWakeTimeOut = lastWakeTime;
    *adjustedForPhysicalWakeOut = lastSMCS3S0WakeInterval;

    return kIOReturnSuccess;
}


#pragma mark -
#pragma mark API

/******************************************************************************
 * IOPMCopyPowerHistory
 *
 ******************************************************************************/
IOReturn IOPMCopyPowerHistory(CFArrayRef *outArray)
{
  DIR *dp;
  struct dirent *ep;
  
  CFMutableArrayRef logs = CFArrayCreateMutable(0, 0, &kCFTypeArrayCallBacks);

  dp = opendir(pwrLogDirName);
  if(dp == NULL)
    return kIOReturnError;
  
  CFMutableDictionaryRef uuid_details;
  CFStringRef uuid;
  CFStringRef timestamp;

  char *tok;
  char *d_name;
  char *copy = NULL;
  int fileCount = 0;

  while ((ep = readdir(dp))) {
    if(!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, ".."))
      continue;
    else
     fileCount++;

    uuid_details = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                             0,
                                             &kCFTypeDictionaryKeyCallBacks,
                                             &kCFTypeDictionaryValueCallBacks);
    copy = d_name = strdup(ep->d_name);

    // Parse filename for metadata
    int part = 1;
    while((tok = strsep(&d_name, "_")) != NULL) {
      
      if(part == 1) {
        timestamp = CFStringCreateWithCString(kCFAllocatorDefault,
                                              tok,
                                              kCFStringEncodingUTF8);
        CFDictionarySetValue(uuid_details, 
                             CFSTR(kIOPMPowerHistoryTimestampKey), 
                             timestamp);
        CFRelease(timestamp);
      }
      else if(part == 2) {
        uuid = CFStringCreateWithCString(kCFAllocatorDefault, 
                                         tok, 
                                         kCFStringEncodingUTF8);
        CFDictionarySetValue(uuid_details, 
                             CFSTR(kIOPMPowerHistoryUUIDKey), 
                             uuid);
        CFRelease(uuid);
      }
      else if(part == 3) {
        // We don't want the extension .plog to be part of the
        // timestamp
        tok = strsep(&tok, ".");
        timestamp = CFStringCreateWithCString(kCFAllocatorDefault,
                                              tok,
                                              kCFStringEncodingUTF8);
        CFDictionarySetValue(uuid_details, 
                             CFSTR(kIOPMPowerHistoryTimestampCompletedKey), 
                             timestamp);
        CFRelease(timestamp);
      }
      
      part++;
    }
    CFArrayAppendValue(logs, uuid_details);
    CFRelease(uuid_details);
    free(copy);
  }
  
  closedir(dp);

  if(fileCount == 0) {
    *outArray = NULL;
    return kIOReturnNotFound;
  }
  else
    *outArray = logs;

  return kIOReturnSuccess;
} 


/******************************************************************************
 * IOPMCopyPowerHistoryDetailed
 *
 ******************************************************************************/
IOReturn IOPMCopyPowerHistoryDetailed(CFStringRef UUID, CFDictionaryRef *details)//CFArrayRef *outArray)
{
    IOReturn                return_code = kIOReturnBadArgument;
    CFDataRef               serializedData = NULL;
    char                    uuid_cstr[kMaxNameLength];
    
    CFURLRef                fileURL = NULL;
    CFMutableStringRef      fileName = CFStringCreateMutable(
                                                    kCFAllocatorDefault,
                                                    255);

    CFStringAppend(fileName, CFSTR(pwrLogDirName));
    CFStringAppend(fileName, CFSTR("/"));

    if (NULL == details || NULL == UUID) {
        goto exit;
    }

    *details = NULL;

    if (!CFStringGetCString(UUID, uuid_cstr, sizeof(uuid_cstr), kCFStringEncodingMacRoman)) {
        goto exit;
    }
    
    DIR *dp;
    struct dirent *ep;

    dp = opendir(pwrLogDirName);

    if(dp == NULL) {
      return_code = kIOReturnError;
      goto exit;
    }

    while ((ep = readdir(dp))) {
      if(strstr(ep->d_name, uuid_cstr)) {
        CFStringRef uuid_file = CFStringCreateWithCString(
                                                    kCFAllocatorDefault,
                                                    ep->d_name,
                                                    kCFStringEncodingUTF8);
        CFStringAppend(fileName, uuid_file);
        if(fileURL) CFRelease(fileURL);
        fileURL = CFURLCreateWithFileSystemPath(
                                               kCFAllocatorDefault,
                                               fileName,
                                               kCFURLPOSIXPathStyle,
                                               false); 

        CFRelease(uuid_file);
      }
    }

    closedir(dp);

    if(!fileURL) {
      return_code = kIOReturnError;
      goto exit;
    }
    SInt32 errorCode;
    Boolean status = CFURLCreateDataAndPropertiesFromResource(
                                                kCFAllocatorDefault,
                                                fileURL,
                                                &serializedData,
                                                NULL,
                                                NULL,
                                                &errorCode);

    if (serializedData && status) {
        *details = (CFDictionaryRef)CFPropertyListCreateWithData(0, serializedData, 0, NULL, NULL);
        CFRelease(serializedData);
    }

    if (NULL == details) {
        return_code = kIOReturnError;
    } else {
        return_code = kIOReturnSuccess;
    }

exit:
    if(fileURL) CFRelease(fileURL);
    CFRelease(fileName);
    return return_code;
}

/******************************************************************************
 * IOPMSetSleepServiceCapTimeout
 *
 ******************************************************************************/
IOReturn IOPMSetSleepServicesWakeTimeCap(CFTimeInterval cap)
{
   mach_port_t             pm_server       = MACH_PORT_NULL;
   kern_return_t           return_code     = KERN_SUCCESS;
   int                     pm_mig_return = -1;

   return_code = _pm_connect(&pm_server);

   if(kIOReturnSuccess != return_code) {
      return return_code;
   }
 
   return_code = io_pm_set_sleepservice_wake_time_cap(pm_server, (int)cap, &pm_mig_return);

   _pm_disconnect(pm_server);
   if (pm_mig_return == kIOReturnSuccess)
      return return_code;
   else
      return pm_mig_return;
}

/******************************************************************************
 * IOPMSleepWakeSetUUID
 *
 ******************************************************************************/
IOReturn IOPMSleepWakeSetUUID(CFStringRef newUUID)
{
    IOReturn        ret = kIOReturnSuccess;
    io_service_t    service = IO_OBJECT_NULL;
    CFTypeRef       setObject = NULL;
    
    if (!newUUID) {
        // Clear active UUID
        setObject = kCFBooleanFalse;
    } else {
        // cache the upcoming UUID
        setObject = newUUID;
    }
    
    service = IORegistryEntryFromPath(kIOMasterPortDefault, 
            kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");

    if (IO_OBJECT_NULL != service) {
        ret = IORegistryEntrySetCFProperty(
                    service, CFSTR(kIOPMSleepWakeUUIDKey),
                    setObject);
        IOObjectRelease(service);
    }
    return ret;
}
 
/******************************************************************************
 * IOPMSleepWakeCopyUUID
 *
 ******************************************************************************/
CFStringRef IOPMSleepWakeCopyUUID(void)
{
    CFStringRef     uuidString = NULL;
    io_service_t    service = IO_OBJECT_NULL;
    
    service = IORegistryEntryFromPath(kIOMasterPortDefault, 
            kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");

    if (IO_OBJECT_NULL != service) {
        uuidString = IORegistryEntryCreateCFProperty(
                       service,
                       CFSTR(kIOPMSleepWakeUUIDKey),
                       kCFAllocatorDefault, 0);
        IOObjectRelease(service);
    }
    
    // Caller must release uuidString, if non-NULL
    return uuidString;
}


bool IOPMGetUUID(int whichUUID, char *putTheUUIDHere, int sizeOfBuffer)
{
    bool return_bool_value = false;
    
    if (kIOPMSleepWakeUUID == whichUUID)
    {
        CFStringRef bs = IOPMSleepWakeCopyUUID();
        if (bs) {
            Boolean bool_result;
            bool_result = CFStringGetCString(bs, putTheUUIDHere, sizeOfBuffer, kCFStringEncodingUTF8);
            CFRelease(bs);
            return bool_result;
        }        
        return false;
    }
    else if (kIOPMSleepServicesUUID == whichUUID) 
    {
        mach_port_t             pm_server       = MACH_PORT_NULL;
        kern_return_t           return_code     = KERN_SUCCESS;
        char                    strPtr[kPMMIGStringLength];
        int                     pm_mig_return = -1;

        return_code = _pm_connect(&pm_server);

        if(kIOReturnSuccess != return_code) {
            return false;
        }
        
        bzero(strPtr, sizeof(strPtr));
        return_code = io_pm_get_uuid(pm_server, kIOPMSleepServicesUUID, strPtr, &pm_mig_return);

        if ((KERN_SUCCESS == return_code) && (KERN_SUCCESS == pm_mig_return))
        {
            bzero(putTheUUIDHere, sizeOfBuffer);
            
            strncpy(putTheUUIDHere, strPtr, sizeOfBuffer-1);
 
            return_bool_value = true;
        }

        _pm_disconnect(pm_server);
    }

    return return_bool_value;    
}

/******************************************************************************
 * IOPMLogWakeProgress
 *
 ******************************************************************************/
IOReturn IOPMLogWakeProgress(uint32_t component, uint32_t progress_data)
{
    io_registry_entry_t gRoot = IO_OBJECT_NULL;
    CFNumberRef         setNum = NULL;
    IOReturn            ret = kIOReturnError;
    CFStringRef         key = NULL;
    
    os_log_debug(OS_LOG_DEFAULT, "Wake progress from %d. data: 0x%x\n", component, progress_data);

    switch (component) {
        case kIOPMLoginWindowProgress:
            key = CFSTR(kIOPMLoginWindowProgressKey);
            break;
        case kIOPMCoreDisplayProgress:
            key = CFSTR(kIOPMCoreDisplayProgressKey);
            break;
        case kIOPMCoreGraphicsProgress:
            key = CFSTR(kIOPMCoreGraphicsProgressKey);
            break;
        default:
            return kIOReturnBadArgument;
    }
    
    gRoot = IORegistryEntryFromPath( kIOMasterPortDefault,
        kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");
    
    if (IO_OBJECT_NULL == gRoot)
        return kIOReturnError;
    
    setNum = CFNumberCreate(0, kCFNumberIntType, &progress_data);
    
    if (!setNum)
        goto exit;
    
    ret = IORegistryEntrySetCFProperty(gRoot, key, setNum);
    
exit:
    if (setNum)
        CFRelease(setNum);
    if (gRoot)
        IOObjectRelease(gRoot);
    return ret;
}

/******************************************************************************
 * IOPMCopySleepWakeFailure
 *
 ******************************************************************************/
CFDictionaryRef IOPMCopySleepWakeFailure(void)
{
    CFStringRef         scFailureKey = NULL;
    CFDictionaryRef     scFailureDictionary = NULL;
    SCDynamicStoreRef   scDynStore = NULL;

    scDynStore = SCDynamicStoreCreate(0, CFSTR("IOPMSleepFailure"), NULL, NULL);
    if (!scDynStore)
        goto exit;

    scFailureKey = SCDynamicStoreKeyCreate(
                            kCFAllocatorDefault, 
                            CFSTR("%@%@/%@"),
                            kSCDynamicStoreDomainState, 
                            CFSTR("PowerManagement"), 
                            CFSTR(kIOPMDynamicStoreSleepFailureKey));
    if (!scFailureKey)
        goto exit;

    scFailureDictionary = SCDynamicStoreCopyValue(scDynStore, scFailureKey);
    if(scFailureDictionary)
    {
        if(!isA_CFDictionary(scFailureDictionary)) {
            CFRelease(scFailureDictionary);
            scFailureDictionary = NULL;
        }
    }
exit:
    if (scDynStore)
        CFRelease(scDynStore);    
    if (scFailureKey)
        CFRelease(scFailureKey);
    return scFailureDictionary;
}



void IOPMClaimSystemWakeEvent(
    CFStringRef         identity,
    CFStringRef         reason,
    CFDictionaryRef     description)
{
    xpc_connection_t        connection = NULL;
    xpc_object_t            sendClaim = NULL;
    xpc_object_t            msg = NULL;
    xpc_object_t            desc = NULL;
    char                    str[255];

    connection = xpc_connection_create_mach_service(POWERD_XPC_ID,
                            dispatch_get_global_queue(DISPATCH_QUEUE_CONCURRENT, 0), 0);
    
    if (!connection) {
        goto exit;
    }
    
    xpc_connection_set_target_queue(connection,
                            dispatch_get_global_queue(DISPATCH_QUEUE_CONCURRENT, 0));
    
    xpc_connection_set_event_handler(connection,
                                     ^(xpc_object_t e __unused) { });
    
    sendClaim = xpc_dictionary_create(NULL, NULL, 0);
    if (sendClaim) {
        if (identity) {
            CFStringGetCString(identity, str, sizeof(str), kCFStringEncodingUTF8);
            xpc_dictionary_set_string(sendClaim, "identity", str);
        }
        if (reason) {
            CFStringGetCString(reason, str, sizeof(str), kCFStringEncodingUTF8);
            xpc_dictionary_set_string(sendClaim, "reason", str);
        }
        
        if (description) {
            desc = _CFXPCCreateXPCObjectFromCFObject(description);
            if (desc) {
                xpc_dictionary_set_value(sendClaim, "description", desc);
                xpc_release(desc);
            }
        }
    }
    
    msg = xpc_dictionary_create(NULL, NULL, 0);
    if (msg) {
        xpc_dictionary_set_value(msg, "claimSystemWakeEvent", sendClaim);
        xpc_connection_resume(connection);
        xpc_connection_send_message(connection, msg);
        xpc_release(msg);
    }
exit:
    if (sendClaim) {
        xpc_release(sendClaim);
    }
    if (connection) {
        xpc_release(connection);
    }
}

/*****************************************************************************/
/*****************************************************************************/
/*
 * IOPMConnection is the connection created by clients to receive sleep/wake notifications
 * from powerd. Below interfaces are provided to manage IOPMConnecion. Complete documentation
 * of these interfaces is available in the header file.
 *
 * IOPMConnectionCreate() opens an IOPMConnection.
 * IOPMConnectionSetNotification() associates a notificiation handler with an IOPMConnection.
 * IOPMConnectionScheduleWithRunLoop() schedules a notification on the specified runloop.
 * IOPMConnectionUnscheduleFromRunLoop() removes a previously scheduled run loop source.
 *
 * IOPMConnectionSetDispatchQueue() schedules IOPMConnection callbacks on a dispatch queue,
 * instead of run loop source. This interace can also be used un-schedule IOPMConnection callbacks.
 *
 * IOPMConnectionRelease() cleans up state and notifications for the specified IOPMConnection.
 *
 * IOPMConnectionCreate() registers for notifications on 'kIOUserAssertionReSync'.  powerd posts a
 * notification on this string when process starts and completes its initialization. IOPMConnection
 * is re-established with powerd when that notification is observed.
 */

/* __IOPMConnection is the IOKit-tracking struct to keep track of
 * open connections.
 */
typedef struct {
    uint32_t                id;
    int                     pid;
    CFStringRef             name;
    IOPMCapabilityBits      interests;
    int                     restoreToken;
    
    /* for dispatch-style notifications */
    mach_port_t             mach_port;
    dispatch_source_t       dispatchDelivery;

    /* for CFRunLoop-style notifications */
    CFMachPortRef           localCFMachPort;
    CFRunLoopSourceRef      localCFMachPortRLS;

    IOPMEventHandlerType    userCallout;
    void                    *userParam;
    int                     runLoopCount;
} __IOPMConnection;


/*****************************************************************************/
/*****************************************************************************/

/* iopm_mach_port_callback
 * The caller installed this callback on a runloop of their choice.
 * Note that this code is running in the caller's runloop context; we don't have
 * it serialized.
 */
#define kMsgPayloadCount    2

typedef struct {
    mach_msg_header_t   header;
    mach_msg_body_t     body;
    uint32_t            payload[kMsgPayloadCount];
} IOPMMessageStructure;

static void iopm_mach_port_callback(
    CFMachPortRef port __unused, 
    void *msg, 
    CFIndex size __unused, 
    void *info)
{
    IOPMMessageStructure *m  = (IOPMMessageStructure *)msg;
    
    __IOPMConnection        *connection = (__IOPMConnection *)info;

    if (!connection || !connection->userCallout) {
        return;
    }

    (*(connection->userCallout))(
                    connection->userParam,
                    (IOPMConnection)connection,
                    m->payload[1],      // messageToken argument
                    m->payload[0]);     // event DATA
    
    return;
}

/*****************************************************************************/
/*****************************************************************************/

static kern_return_t _conveyMachPortToPowerd(
                                             __IOPMConnection *connection, 
                                             mach_port_t the_port,
                                             bool enable)
{
    mach_port_t             pm_server       = MACH_PORT_NULL;
    kern_return_t           return_code     = KERN_SUCCESS;
    IOReturn                rc = kIOReturnSuccess;
    
    return_code = _pm_connect(&pm_server);
    
    if(kIOReturnSuccess != return_code) {
        goto exit;
    }
    
    return_code = io_pm_connection_schedule_notification(pm_server, connection->id, the_port, enable ? 0:1, &rc);
    if (return_code != KERN_SUCCESS) {
        os_log_error(OS_LOG_DEFAULT, "Failed to send mach port for IOPMConnection id 0x%x. rc:0x%x\n", connection->id, return_code);
        rc = kIOReturnInternalError;
    }
    else if (rc != kIOReturnSuccess) {
        os_log_error(OS_LOG_DEFAULT, "Failed to update machport for IOPMConnection id 0x%x. rc:0x%x\n", connection->id, rc);
    }

    _pm_disconnect(pm_server);
    
exit:
    return rc;
}

/*****************************************************************************/
/*****************************************************************************/
IOReturn IOPMSetActivePushConnectionState(bool exists)
{

    return IOPMSetValueInt(kIOPMPushConnectionActive, exists ? 1:0);
}

IOReturn IOPMGetActivePushConnectionState(bool *exists)
{
    int value = 0;

    if (exists == NULL)
        return kIOReturnBadArgument;

    value = IOPMGetValueInt(kIOPMPushConnectionActive);
    *exists = value ? true:false;

    return kIOReturnSuccess;
}

/*****************************************************************************/
/*****************************************************************************/
#pragma mark -
#pragma mark IOPMConnection

IOReturn IOPMConnectionSetNotification(
    IOPMConnection myConnection, 
    void *param, 
    IOPMEventHandlerType handler)
{
    __IOPMConnection *connection = (__IOPMConnection *)myConnection;

    if (!connection || !handler)
        return kIOReturnBadArgument;
    
    connection->userParam = param;
    connection->userCallout = handler;

    return kIOReturnSuccess;
}

/*****************************************************************************/
/*****************************************************************************/

IOReturn IOPMConnectionScheduleWithRunLoop(
    IOPMConnection myConnection, 
    CFRunLoopRef theRunLoop,
    CFStringRef runLoopMode)
{
    __IOPMConnection *connection = (__IOPMConnection *)myConnection;

    __block IOReturn        return_code = kIOReturnError;

    if (!connection || !theRunLoop || !runLoopMode)
        return kIOReturnBadArgument;

    dispatch_sync(getPMQueue(), ^{
        CFMachPortContext       mpContext = { 1, (void *)connection, NULL, NULL, NULL };

        if (NULL == connection->localCFMachPort)
        {
            // Create the mach port on which we'll receive mach messages
            // from PM configd.
            connection->localCFMachPort = CFMachPortCreate(
                                               kCFAllocatorDefault,
                                               iopm_mach_port_callback,
                                               &mpContext, NULL);

            if (connection->localCFMachPort) {
                connection->localCFMachPortRLS = CFMachPortCreateRunLoopSource(
                                                           kCFAllocatorDefault,
                                                           connection->localCFMachPort,
                                                           0);
            }
        }

        if (!connection->localCFMachPortRLS) {
            return_code = kIOReturnInternalError;
            return;
        }

        // Record our new run loop.
        connection->runLoopCount++;

        CFRunLoopAddSource(theRunLoop, connection->localCFMachPortRLS, runLoopMode);

        // We have a mapping of one mach_port connected to PM configd to as many
        // CFRunLoopSources that the caller originates.
        if (1 == connection->runLoopCount)
        {
            mach_port_t notify_mach_port = MACH_PORT_NULL;

            if (connection->localCFMachPort) {
                notify_mach_port = CFMachPortGetPort(connection->localCFMachPort);
            }

            return_code =_conveyMachPortToPowerd(connection, notify_mach_port, true);

        }
    });
    
    return return_code;
}

/*****************************************************************************/
/*****************************************************************************/

IOReturn IOPMConnectionUnscheduleFromRunLoop(
                                             IOPMConnection myConnection, 
                                             CFRunLoopRef theRunLoop,
                                             CFStringRef runLoopMode)
{
    __IOPMConnection    *connection = (__IOPMConnection *)myConnection;
    __block IOReturn            return_code = kIOReturnSuccess;
    
    if (!connection || !theRunLoop || !runLoopMode)
        return kIOReturnBadArgument;
    
    dispatch_sync(getPMQueue(), ^{
        if (connection->localCFMachPort) {
            CFRunLoopRemoveSource(theRunLoop, connection->localCFMachPortRLS, runLoopMode);
        }

        connection->runLoopCount--;

        if (0 == connection->runLoopCount)
        {
            mach_port_t notify_mach_port = MACH_PORT_NULL;

            if (connection->localCFMachPortRLS) {
                CFRelease(connection->localCFMachPortRLS);
            }
            if (connection->localCFMachPort) {
                notify_mach_port = CFMachPortGetPort(connection->localCFMachPort);

                return_code = _conveyMachPortToPowerd(connection, notify_mach_port, false);
                CFMachPortInvalidate(connection->localCFMachPort);
                CFRelease(connection->localCFMachPort);
            }

        }
    });

    return return_code;
}

/*
 * IOPMConnectionSetDispatchQueue - Associates the specified dispatch queue with connection and posts sleep/wake notifications
 * on that dispatch queue.
 * Setting myQueue parameter to null value disassociates the previously set dispatch queue from the connection.
 */
void setDispatchQueue( __IOPMConnection *connection, dispatch_queue_t myQueue)
{
    if (!myQueue) {
        /* Clean up a previously scheduled dispatch. */
        if (connection->dispatchDelivery)
        {
            _conveyMachPortToPowerd(connection, connection->mach_port, false);
            dispatch_source_cancel(connection->dispatchDelivery);
            connection->dispatchDelivery = 0;
        }
        return;
    }

    if (KERN_SUCCESS != mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &connection->mach_port))
    {
        // Error allocating mach port
        return;
    }

    mach_port_insert_right(mach_task_self(), connection->mach_port, connection->mach_port,
                           MACH_MSG_TYPE_MAKE_SEND);

    if (!(connection->dispatchDelivery
          = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, connection->mach_port, 0, myQueue)))
    {
        // Error creating dispatch_source
        mach_port_deallocate(mach_task_self(), connection->mach_port);
        mach_port_mod_refs(mach_task_self(), connection->mach_port, MACH_PORT_RIGHT_RECEIVE, -1);
        connection->mach_port = MACH_PORT_NULL;
        return;
    }

    mach_port_t  mach_port = connection->mach_port;
    dispatch_source_t  dispatchDelivery = connection->dispatchDelivery;

    dispatch_source_set_cancel_handler(connection->dispatchDelivery,
                                       ^{
                                           dispatch_release(dispatchDelivery);

                                           mach_port_mod_refs(mach_task_self(), mach_port, MACH_PORT_RIGHT_RECEIVE, -1);
                                           mach_port_deallocate(mach_task_self(), mach_port);
                                       });

    dispatch_source_set_event_handler(connection->dispatchDelivery,
                                      ^{
                                          struct {
                                              IOPMMessageStructure    m;
                                              mach_msg_trailer_t      trailer;
                                          } msg;

                                          bzero(&msg, sizeof(msg));
                                          kern_return_t status = mach_msg( ( void * )&msg,
                                                                          MACH_RCV_MSG | MACH_RCV_TIMEOUT,
                                                                          0,
                                                                          sizeof( msg ),
                                                                          connection->mach_port,
                                                                          0,
                                                                          MACH_PORT_NULL );

                                          if (!connection || !connection->userCallout
                                              || (KERN_SUCCESS != status))
                                          {
                                              return;
                                          }

                                          (*(connection->userCallout))(connection->userParam,
                                                                       (IOPMConnection)connection,
                                                                       msg.m.payload[1], msg.m.payload[0]);
                                      });

    dispatch_resume(connection->dispatchDelivery);

    _conveyMachPortToPowerd(connection, connection->mach_port, true);
    return;
}

void IOPMConnectionSetDispatchQueue(
    IOPMConnection myConnection, 
    dispatch_queue_t myQueue)
{
    __IOPMConnection *connection = (__IOPMConnection *)myConnection;

    if (!connection)
        return;

    setDispatchQueue(connection, myQueue);
    return;
}

/*****************************************************************************/
/*****************************************************************************/
static IOReturn _connectionCreate(__IOPMConnection *connection)
{
    mach_port_t     pm_server = MACH_PORT_NULL;
    IOReturn        return_code = kIOReturnSuccess;
    kern_return_t   kern_result;
    char            arg_name_str[kMaxNameLength];
    uint32_t        new_connection_id = 0;

    return_code = _pm_connect(&pm_server);
    if (return_code != kIOReturnSuccess) {
        goto exit;
    }

    CFStringGetCString(connection->name, arg_name_str,
                        sizeof(arg_name_str), kCFStringEncodingMacRoman);

    kern_result = io_pm_connection_create(
                                    pm_server,
                                    arg_name_str,
                                    connection->interests,
                                    &new_connection_id,
                                    &return_code);
    if (kern_result != KERN_SUCCESS) {
        os_log_error(OS_LOG_DEFAULT, "Failed to send IOPMConnection request. rc:0x%x\n", kern_result);
        return_code = kIOReturnInternalError;
        goto exit;
    }
    else if (return_code != kIOReturnSuccess) {
        os_log_error(OS_LOG_DEFAULT, "Failed to create IOPMConnection. rc:0x%x\n", return_code);
        goto exit;
    }

    connection->id = (int)new_connection_id;
    os_log_info(OS_LOG_DEFAULT, "Created IOPMConnection with id 0x%x\n", connection->id);
exit:
    return return_code;
}

static void restoreConnection(__IOPMConnection *connection)
{
    mach_port_t notify_mach_port = MACH_PORT_NULL;

    os_log_info(OS_LOG_DEFAULT, "Recreating IOPMConnection id 0x%x\n", connection->id);
    _connectionCreate(connection);

    if (connection->mach_port != MACH_PORT_NULL) {
        notify_mach_port = connection->mach_port;
    }
    else if (connection->localCFMachPort) {
        notify_mach_port = CFMachPortGetPort(connection->localCFMachPort);
    }
    if (notify_mach_port != MACH_PORT_NULL) {
        _conveyMachPortToPowerd(connection, notify_mach_port, true);
    }
}


IOReturn IOPMConnectionCreate(
    CFStringRef myName,
    IOPMCapabilityBits interests,
    IOPMConnection *newConnection)
{
    __block IOReturn   return_code = kIOReturnError;

    // * vet argument newConnection
    if (!newConnection)
        return kIOReturnBadArgument;

    // * vet argument 'interests'
    // A caller specifying 0 interests would get no notifications
    if (0 == interests) {
        return kIOReturnBadArgument;
    }

    // * vet argument 'myName'
    if (!myName || (kMaxNameLength < CFStringGetLength(myName))) {
        return kIOReturnBadArgument;
    }

    dispatch_sync(getPMQueue(), ^{
        __IOPMConnection        *connection = NULL;
        int status;

        *newConnection = NULL;
        connection = calloc(1, sizeof(__IOPMConnection));
        if (!connection) {
            return_code = kIOReturnInternalError;
            return;
        }
        connection->restoreToken = -1;

        CFRetain(myName);
        connection->name = myName;
        connection->interests = interests;

        return_code = _connectionCreate(connection);
        if (return_code != kIOReturnSuccess) {
            CFRelease(myName);
            free(connection);
            return;
        }

        // Register a handler to re-establish connection on powerd's restart
        status = notify_register_dispatch(kIOUserAssertionReSync, &connection->restoreToken, getPMQueue(),
                                 ^(int token __unused){ restoreConnection(connection); });
        if (status != NOTIFY_STATUS_OK) {
            os_log_error(OS_LOG_DEFAULT, "Failed to register for reconnecting with powerd. rc:0x%x", status);
        }

        *newConnection = (void *)connection;
        return_code = kIOReturnSuccess;
    });
    return return_code;

}

/*****************************************************************************/
/*****************************************************************************/

IOReturn IOPMConnectionRelease(IOPMConnection connection)
{
    __block IOReturn            return_code = kIOReturnError;

    dispatch_sync(getPMQueue(), ^{
        __IOPMConnection    *connection_private = (__IOPMConnection *)connection;
        mach_port_t         pm_server = MACH_PORT_NULL;
        kern_return_t       kern_result;
        IOReturn            err;

        if (notify_is_valid_token(connection_private->restoreToken)) {
            notify_cancel(connection_private->restoreToken);
        }
        if (connection_private->name) {
            CFRelease(connection_private->name);
        }

        err = _pm_connect(&pm_server);
        if(kIOReturnSuccess != err) {
            return_code = kIOReturnInternalError;
            goto exit;
        }

#if !TARGET_OS_IPHONE
        if (connection_private->dispatchDelivery) {
            setDispatchQueue(connection_private, NULL);
        }
#endif 

        kern_result = io_pm_connection_release(pm_server,
                                               connection_private->id,
                                               &return_code);
        if (kern_result != KERN_SUCCESS) {
            return_code = kern_result;
        }
        bzero(connection_private, sizeof(*connection_private));
        free(connection_private);
    exit:
        if (MACH_PORT_NULL != pm_server) {
            _pm_disconnect(pm_server);
        }
    });

    return return_code;
}

/*****************************************************************************/
/*****************************************************************************/

IOReturn IOPMConnectionAcknowledgeEvent(
    IOPMConnection connect, 
    IOPMConnectionMessageToken token)
{
#if TARGET_OS_IPHONE
    (void)connect;
    (void)token;
    
    return kIOReturnUnsupported;
#else
    return IOPMConnectionAcknowledgeEventWithOptions(
                           connect, token, NULL);
#endif /* TARGET_OS_IPHONE */
}


/*****************************************************************************/
/*****************************************************************************/
IOReturn IOPMConnectionAcknowledgeEventWithOptions(
    IOPMConnection myConnection, 
    IOPMConnectionMessageToken token, 
    CFDictionaryRef options)
{
#if TARGET_OS_IPHONE
    (void)myConnection;
    (void)token;
    (void)options;
    
    return kIOReturnUnsupported;
#else
    __block IOReturn   return_code = kIOReturnError;

    dispatch_sync(getPMQueue(), ^{

        __IOPMConnection    *connection = (__IOPMConnection *)myConnection;

        mach_port_t         pm_server = MACH_PORT_NULL;
        kern_return_t       kern_result;
        IOReturn            err;

        CFDataRef           serializedData = NULL;
        vm_offset_t         buffer_ptr = 0;
        size_t              buffer_size = 0;

        // No response is expected when token is 0
        if (token == 0) {
            return_code = kIOReturnSuccess;
            goto exit;
        }

        err = _pm_connect(&pm_server);
        if(kIOReturnSuccess != err) {
            return_code = kIOReturnInternalError;
            goto exit;
        }

        if (options)
        {
            serializedData = CFPropertyListCreateData(
                                                      kCFAllocatorDefault,
                                                      (CFPropertyListRef)options,
                                                      kCFPropertyListBinaryFormat_v1_0, 0, NULL);

            if (serializedData)
            {
                buffer_ptr = (vm_offset_t)CFDataGetBytePtr(serializedData);
                buffer_size = (size_t)CFDataGetLength(serializedData);

            }
        }

        kern_result = io_pm_connection_acknowledge_event(pm_server,
                                                         (uint32_t)connection->id,
                                                         (uint32_t)token,
                                                         buffer_ptr,
                                                         buffer_size,
                                                         &return_code);

        if (kern_result != KERN_SUCCESS) {
            return_code = kern_result;
        }
    exit:
        if (MACH_PORT_NULL != pm_server) {
            _pm_disconnect(pm_server);
        }

        if (serializedData) CFRelease(serializedData);
    });

    return return_code;
#endif /* TARGET_OS_IPHONE */
}

/*****************************************************************************/
/*****************************************************************************/

IOReturn IOPMCopyConnectionStatus(int statusSelector, CFTypeRef *output)
{
    int                 return_code = kIOReturnError;
    mach_port_t         pm_server = MACH_PORT_NULL;
    IOReturn            err;
    
    CFDictionaryRef     *dictionaryOutput = (CFDictionaryRef *)output;
 
    err = _pm_connect(&pm_server);
    if(kIOReturnSuccess != err) {
        return_code = kIOReturnInternalError;
        goto exit;
    }
    
    *dictionaryOutput = NULL;
    statusSelector = 0;
    return_code = kIOReturnSuccess;
exit:
    if (MACH_PORT_NULL != pm_server) {
        _pm_disconnect(pm_server);
    }

    return return_code;
}

/*****************************************************************************/
/*****************************************************************************/

#define SYSTEM_ON_CAPABILITIES (kIOPMCapabilityCPU | kIOPMCapabilityVideo | kIOPMCapabilityAudio \
                                | kIOPMCapabilityNetwork | kIOPMCapabilityDisk)

IOPMCapabilityBits IOPMConnectionGetSystemCapabilities(void)
{

    mach_port_t             pm_server = MACH_PORT_NULL;
    IOPMCapabilityBits      ret_cap = SYSTEM_ON_CAPABILITIES;
    IOReturn                return_code = kIOReturnError;

    return_code = _pm_connect(&pm_server);

    if(pm_server == MACH_PORT_NULL)
      return ret_cap; 

    io_pm_get_capability_bits(pm_server, &ret_cap, &return_code);

    _pm_disconnect(pm_server);

    return ret_cap;


}


bool IOPMIsADarkWake(IOPMCapabilityBits c)
{
    return ((c & kIOPMCapabilityCPU) && !(c & kIOPMCapabilityVideo));
}

bool IOPMAllowsBackgroundTask(IOPMCapabilityBits c)
{
    return (0 != (c & kIOPMCapabilityBackgroundTask));
}

bool IOPMAllowsPushServiceTask(IOPMCapabilityBits c)
{
    return (0 != (c & kIOPMCapabilityPushServiceTask));
}

bool IOPMIsASilentWake(IOPMCapabilityBits c)
{
    return (0 != (c & kIOPMCapabilitySilentRunning));
}

bool IOPMIsAUserWake(IOPMCapabilityBits c)
{
    return (0 != (c & kIOPMCapabilityVideo));
}

bool IOPMIsASleep(IOPMCapabilityBits c)
{
    return (0 == (c & kIOPMCapabilityCPU));
}

bool IOPMGetCapabilitiesDescription(char *buf, int buf_size, IOPMCapabilityBits in_caps)
{
    uint64_t caps = (uint64_t)in_caps;
    int printed_total = 0;
    char *on_sleep_dark = "";
    
    if (IOPMIsASleep(caps))
    {
        on_sleep_dark = "Sleep";
    } else if (IOPMIsADarkWake(caps))
    {
        on_sleep_dark = "DarkWake";
    } else if (IOPMIsAUserWake(caps))
    {
        on_sleep_dark = "FullWake";
    } else
    {
        on_sleep_dark = "Unknown";
    }
    
    printed_total = snprintf(buf, buf_size, "%s:%s%s%s%s%s%s%s",
                             on_sleep_dark,
                             (caps & kIOPMCapabilityCPU) ? "cpu ":"<off> ",
                             (caps & kIOPMCapabilityDisk) ? "disk ":"",
                             (caps & kIOPMCapabilityNetwork) ? "net ":"",
                             (caps & kIOPMCapabilityAudio) ? "aud ":"",
                             (caps & kIOPMCapabilityVideo) ? "vid ":"",
                             (caps & kIOPMCapabilityPushServiceTask) ? "push ":"",
                             (caps & kIOPMCapabilityBackgroundTask) ? "bg ":"");
    
    return (printed_total <= buf_size);
}


/*****************************************************************************/
/*****************************************************************************/
#pragma mark -
#pragma mark Talking about DarkWake

bool IOPMGetSleepServicesActive(void)
{
    int         token = 0;
    uint64_t    payload = 0;

    if (NOTIFY_STATUS_OK == notify_register_check(kIOPMSleepServiceActiveNotifyName, &token)) 
    {
        notify_get_state(token, &payload);
        notify_cancel(token);
    }    
    
    return ((payload &  kIOPMSleepServiceActiveNotifyBit) ? true : false);
}


int IOPMGetDarkWakeThermalEmergencyCount(void)
{
    return IOPMGetValueInt(kIOPMDarkWakeThermalEventCount);
}

/*****************************************************************************/
/*****************************************************************************/
#pragma mark -
#pragma mark Power Internals

/*!
 * @function        IOPMGetValueInt
 * @abstract        For IOKit use only.
 */
int IOPMGetValueInt(int selector) {
    mach_port_t             pm_server = MACH_PORT_NULL;
    int                     valint = 0;
    kern_return_t           kern_result;
    
    if (kIOReturnSuccess == _pm_connect(&pm_server))
    {
        kern_result = io_pm_get_value_int(
                                          pm_server,
                                          selector,
                                          &valint);
        if (KERN_SUCCESS != kern_result) {
            valint = 0;
        }
        _pm_disconnect(pm_server);
    }
    return valint;
}

/*!
 * @function        IOPMSetValueInt
 * @abstract        For IOKit use only.
 */
IOReturn IOPMSetValueInt(int selector, int value) {
    mach_port_t             pm_server = MACH_PORT_NULL;
    IOReturn                rc = kIOReturnError;
    
    if (kIOReturnSuccess == _pm_connect(&pm_server))
    {
        io_pm_set_value_int(
                            pm_server,
                            selector,
                            value, &rc);
        _pm_disconnect(pm_server);
    }
    else {
        return kIOReturnNotReady;
    }

    return rc;
}


IOReturn IOPMSetDebugFlags(uint32_t newFlags, uint32_t *oldFlags)
{

    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result;
    uint32_t                flags;
    IOReturn                return_code = kIOReturnError;

    return_code = _pm_connect(&pm_server);

    if(pm_server == MACH_PORT_NULL)
      return kIOReturnNotReady; 

    kern_result = io_pm_set_debug_flags(pm_server, newFlags, kIOPMDebugFlagsSetValue, &flags, &return_code);

    _pm_disconnect(pm_server);

    if ( kern_result == KERN_SUCCESS && return_code == kIOReturnSuccess) {
        if (oldFlags) 
            *oldFlags = flags;
        return kIOReturnSuccess;
    }
    else 
        return return_code;
}

#if TARGET_OS_IPHONE
IOReturn IOPMSetBTWakeInterval(uint32_t newInterval __unused, 
      uint32_t *oldInterval __unused)
{
#else
IOReturn IOPMSetBTWakeInterval(uint32_t newInterval, uint32_t *oldInterval)
{

    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result;
    uint32_t                interval = 0;
    IOReturn                return_code = kIOReturnError;

    return_code = _pm_connect(&pm_server);

    if(pm_server == MACH_PORT_NULL)
      return kIOReturnNotReady; 

    kern_result = io_pm_set_bt_wake_interval(pm_server, newInterval, &interval, &return_code);

    _pm_disconnect(pm_server);

    if ( kern_result == KERN_SUCCESS && return_code == kIOReturnSuccess) {
        if (oldInterval) 
            *oldInterval = interval;
        return kIOReturnSuccess;
    }
    else 
        return return_code;

#endif
    return kIOReturnSuccess;

}

#if TARGET_OS_IPHONE
IOReturn IOPMSetDWLingerInterval(uint32_t newInterval __unused)
{
#else
IOReturn IOPMSetDWLingerInterval(uint32_t newInterval)
{

    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result;
    IOReturn                return_code = kIOReturnError;

    return_code = _pm_connect(&pm_server);

    if(pm_server == MACH_PORT_NULL)
      return kIOReturnNotReady;

    kern_result = io_pm_set_dw_linger_interval(pm_server, newInterval, &return_code);

    _pm_disconnect(pm_server);

    if ( kern_result == KERN_SUCCESS && return_code == kIOReturnSuccess) {
        return kIOReturnSuccess;
    }
    else
        return return_code;

#endif
    return kIOReturnSuccess;

}

IOReturn IOPMChangeSystemActivityAssertionBehavior(uint32_t newFlags, uint32_t *oldFlags)
{
    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result;
    uint32_t                flags = 0;
    IOReturn                return_code = kIOReturnError;

    return_code = _pm_connect(&pm_server);

    if(pm_server == MACH_PORT_NULL)
      return kIOReturnNotReady;

    kern_result = io_pm_change_sa_assertion_behavior(pm_server, newFlags, &flags, &return_code);

    _pm_disconnect(pm_server);

    if ( kern_result == KERN_SUCCESS && return_code == kIOReturnSuccess)
    {
        if(flags)
            *oldFlags = flags;
        return kIOReturnSuccess;
    }
    else
        return return_code;
}

/*****************************************************************************/
/*****************************************************************************/
IOReturn IOPMCtlAssertionType(char *type, int op)
{
    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result;
    IOReturn                return_code = kIOReturnError;

    return_code = _pm_connect(&pm_server);

    if(pm_server == MACH_PORT_NULL)
      return kIOReturnNotReady; 

    kern_result = io_pm_ctl_assertion_type(pm_server, type, 
            op, &return_code);

    _pm_disconnect(pm_server);

    if ( kern_result == KERN_SUCCESS && return_code == kIOReturnSuccess) {
        return kIOReturnSuccess;
    }
    else 
        return return_code;
}

/*****************************************************************************/
/*****************************************************************************/
#define kPMReportPowerOn       0x01 
#define kPMReportDeviceUsable  0x02
#define kPMReportLowPower      0x04      
CFDictionaryRef  IOPMCopyPowerStateInfo(uint64_t state_id)
{
    CFMutableDictionaryRef dict = NULL;
    CFTypeRef objRef = NULL;
    uint32_t val = 0;

    dict  = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                 0,
                                 &kCFTypeDictionaryKeyCallBacks,
                                 &kCFTypeDictionaryValueCallBacks);

    if (!dict)
        return kIOReturnNoMemory;

    val = state_id & 0xf;
    objRef = CFNumberCreate(0, kCFNumberIntType, &val);
    if (objRef) {
        CFDictionarySetValue(dict, kIOPMNodeCurrentState, objRef);
        CFRelease(objRef);
    }

    val = ((state_id >> 4) & 0xf);
    objRef = CFNumberCreate(0, kCFNumberIntType, &val);
    if (objRef) {
        CFDictionarySetValue(dict, kIOPMNodeMaxState, objRef);
        CFRelease(objRef); objRef = NULL;
    }

    if ( (state_id >> 8) & kPMReportPowerOn)
        CFDictionarySetValue(dict, kIOPMNodeIsPowerOn, kCFBooleanTrue);
    else
        CFDictionarySetValue(dict, kIOPMNodeIsPowerOn, kCFBooleanFalse);

    if ( (state_id >> 8) & kPMReportDeviceUsable)
        CFDictionarySetValue(dict, kIOPMNodeIsDeviceUsable, kCFBooleanTrue);
    else
        CFDictionarySetValue(dict, kIOPMNodeIsDeviceUsable, kCFBooleanFalse);


    if ( (state_id >> 8) & kPMReportLowPower)
        CFDictionarySetValue(dict, kIOPMNodeIsLowPower, kCFBooleanTrue);
    else
        CFDictionarySetValue(dict, kIOPMNodeIsLowPower, kCFBooleanFalse);

    return dict;
}
/*****************************************************************************/
/*****************************************************************************/

IOReturn IOPMAssertionNotify(char *name, int req_type)
{
    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result;
    IOReturn                return_code = kIOReturnError;

    return_code = _pm_connect(&pm_server);

    if(pm_server == MACH_PORT_NULL)
      return kIOReturnNotReady; 

    kern_result = io_pm_assertion_notify(pm_server, name, 
            req_type, &return_code);

    _pm_disconnect(pm_server);

    if ( kern_result == KERN_SUCCESS && return_code == kIOReturnSuccess) {
        return kIOReturnSuccess;
    }
    else 
        return return_code;
}

/*****************************************************************************/
IOReturn _copySleepPreventersList(CFStringRef key, CFArrayRef *outArray)
{
    io_service_t            service = IO_OBJECT_NULL;
    CFArrayRef                  obj;
    service = IORegistryEntryFromPath(kIOMasterPortDefault,
            kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");

    if (IO_OBJECT_NULL == service)
    {
        return kIOReturnInternalError;
    }
    obj  = IORegistryEntryCreateCFProperty( service, key, kCFAllocatorDefault, 0);
    IOObjectRelease(service);

    *outArray = obj;
    return kIOReturnSuccess;

}
/*****************************************************************************/
IOReturn  IOPMCopySleepPreventersList(int preventerType, CFArrayRef *outArray)
{
    CFStringRef                 key;

    if (outArray == NULL)
    {
        return kIOReturnBadArgument;
    }
    if (preventerType == kIOPMIdleSleepPreventers)
    {
        key = CFSTR(kIOPMIdleSleepPreventersKey);
    }
    else if (preventerType == kIOPMSystemSleepPreventers)
    {
        key = CFSTR(kIOPMSystemSleepPreventersKey);
    }
    else
    {
        return kIOReturnBadArgument;
    }
    return _copySleepPreventersList(key, outArray);

}

/*****************************************************************************/
IOReturn  IOPMCopySleepPreventersListWithID(int preventerType, CFArrayRef *outArray)
{
    CFStringRef                 key;

    if (outArray == NULL)
    {
        return kIOReturnBadArgument;
    }
    if (preventerType == kIOPMIdleSleepPreventers)
    {
        key = CFSTR(kIOPMIdleSleepPreventersWithIDKey);
    }
    else if (preventerType == kIOPMSystemSleepPreventers)
    {
        key = CFSTR(kIOPMSystemSleepPreventersWithIDKey);
    }
    else
    {
        return kIOReturnBadArgument;
    }

    return _copySleepPreventersList(key, outArray);

}

/*****************************************************************************/


