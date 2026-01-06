/*
 * Copyright (c) 2010 Apple Computer, Inc. All rights reserved.
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
#include <CoreFoundation/CFPriv.h>
#include <mach/mach.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/pwr_mgt/IOPMLibPrivate.h>
#include "IOSystemConfiguration.h"
#include <sys/time.h>
#include <notify.h>
#include <execinfo.h>
#include <asl.h>
#include <mach/mach_time.h>
#if !TARGET_OS_SIMULATOR
#include <energytrace.h>
#endif
#include <os/log.h>
#include <xpc/xpc.h>
#define POWERD_XPC_ID   "com.apple.iokit.powerdxpc"

#include "powermanagement_mig.h"
#include "powermanagement.h"

#include <servers/bootstrap.h>


#define kAssertionsArraySize        5
#define NUM_BT_FRAMES               8

#define DEBUG 1
#if DEBUG
#define DEBUG_LOG(fmt, args...) \
{ \
    os_log_debug(OS_LOG_DEFAULT, fmt, ##args); \
}
#else
#define DEBUG_LOG(fmt, args...) {}
#endif

#define ERROR_LOG(fmt, args...) \
{  \
    os_log_error(OS_LOG_DEFAULT, fmt, ##args); \
}

static uint64_t  collectBackTrace = 0;

IOReturn _pm_connect(mach_port_t *newConnection);
IOReturn _pm_disconnect(mach_port_t connection);
__private_extern__ IOReturn _copyPMServerObject(int selector, int assertionID,
                                                CFTypeRef selectorData, CFTypeRef *objectOut);
__private_extern__ io_registry_entry_t getPMRootDomainRef(void);
dispatch_queue_t  getPMQueue();

static IOReturn pm_connect_init(mach_port_t *newConnection)
{
#if !TARGET_OS_IPHONE
    static int              disableAppSleepToken = 0;
    static int              enableAppSleepToken = 0;
#endif
    static int              collectBackTraceToken = 0;

#if !TARGET_OS_IPHONE
    if ( !disableAppSleepToken ) {
        char notify_str[128];

        snprintf(notify_str, sizeof(notify_str), "%s.%d", 
                 kIOPMDisableAppSleepPrefix, getpid());

        notify_register_dispatch(
                                 notify_str,
                                 &disableAppSleepToken,
                                 dispatch_get_main_queue(),
                                 ^(int t __unused){
                                 __CFRunLoopSetOptionsReason(__CFRunLoopOptionsTakeAssertion, 
                                                             CFSTR("App is holding power assertion."));
                                 });
    }

    if ( !enableAppSleepToken ) {
        char notify_str[128];

        snprintf(notify_str, sizeof(notify_str), "%s.%d", 
                 kIOPMEnableAppSleepPrefix, getpid());

        notify_register_dispatch(
                                 notify_str,
                                 &enableAppSleepToken,
                                 dispatch_get_main_queue(),
                                 ^(int t __unused){
                                 __CFRunLoopSetOptionsReason(__CFRunLoopOptionsDropAssertion, 
                                                             CFSTR("App released all power assertions."));
                                 });
    }
#endif

    if (!collectBackTraceToken) {
        notify_register_dispatch(
                                 kIOPMAssertionsCollectBTString,
                                 &collectBackTraceToken,
                                 dispatch_get_main_queue(),
                                 ^(int t){
                                     notify_get_state(t, &collectBackTrace);
                                 });
        notify_get_state(collectBackTraceToken, &collectBackTrace);

    }

    return _pm_connect(newConnection);
}

static IOReturn pm_connect_close(mach_port_t connection)
{
    return _pm_disconnect(connection);
}

static inline void saveBackTrace(CFMutableDictionaryRef props)
{

    void *              bt[NUM_BT_FRAMES];
    char                **syms = NULL;
    int                 i;
    CFStringRef         frame_cf = NULL;
    CFMutableArrayRef   syms_cf = NULL;


    int nframes = backtrace((void**)(&bt), NUM_BT_FRAMES);

    syms = backtrace_symbols(bt, nframes);
    syms_cf = CFArrayCreateMutable(0, nframes, &kCFTypeArrayCallBacks);   
    if (syms && syms_cf) {
        for (i = 0; i < nframes; i++) {
            frame_cf = NULL;
            frame_cf = CFStringCreateWithCString(NULL, syms[i], 
                                                 kCFStringEncodingMacRoman);
            if (frame_cf) {
                CFArrayInsertValueAtIndex(syms_cf, i, frame_cf);
                CFRelease(frame_cf);
            }
            else {
                CFArrayInsertValueAtIndex(syms_cf, i, CFSTR(" "));
            }
        }
        CFDictionarySetValue(props, kIOPMAssertionCreatorBacktrace, syms_cf);
    }

    if (syms_cf) CFRelease(syms_cf);
    if (syms) free(syms);
}





bool   gAsyncMode = false;
IOReturn IOPMEnableAsyncAssertions()
{
    gAsyncMode = true;

    return kIOReturnSuccess;
}

IOReturn IOPMDisableAsyncAssertions()
{
    gAsyncMode = false;

    return kIOReturnSuccess;
}

#define kMaxAsyncAssertions         128 // Max number of assertions at a time per process
#define kDefaultOffloadDelay        0   // Delay (in secs) after which assertions are offloaded to powerd

// Async assertions uses upper 16-bits to store the idx.
// Sync assertions uses lower-16 bits to store idx.
#define ASYNC_ID_FROM_IDX(idx)  (((idx & 0x7fff) | 0x8000) << 16)
#define ASYNC_IDX_FROM_ID(id)   (((id) >> 16) & 0x7fff)
#define IS_ASYNC_ID(id) ((id) & (0xffff << 16))


static dispatch_source_t             offloader = 0;
static CFMutableDictionaryRef       gAssertionsDict = NULL;
static uint64_t  nextOffload_ts;
static xpc_connection_t assertionConnection;



uint64_t getMonotonicTime( )
{
    static mach_timebase_info_data_t    timebaseInfo;

    if (timebaseInfo.denom == 0)
        mach_timebase_info(&timebaseInfo);

    return ( (mach_absolute_time( ) * timebaseInfo.numer) / (timebaseInfo.denom * NSEC_PER_SEC));
}

//
// Send a create or property update message
// Returns remote assertion id for create request
// Returns null for property update message
//
IOPMAssertionID sendAsyncAssertionMsg(bool create, CFDictionaryRef dict, IOReturn *rc)
{
    xpc_object_t            desc = NULL;
    xpc_object_t            msg = NULL;
    xpc_object_t            resp = NULL;
    IOPMAssertionID         remoteID = kIOPMNullAssertionID;


    msg = xpc_dictionary_create(NULL, NULL, 0);
    if (!msg) {
        ERROR_LOG("Failed to create xpc msg object\n");
        return remoteID;
    }

    desc = _CFXPCCreateXPCMessageWithCFObject(dict);
    if (!desc) {
        ERROR_LOG("Failed to convert CF dictionary to xpc object\n");
        return remoteID;
    }

    if (create) {
        xpc_dictionary_set_value(msg, kAssertionCreateMsg, desc);

        resp = xpc_connection_send_message_with_reply_sync(assertionConnection, msg);
        remoteID = xpc_dictionary_get_uint64(resp, kAssertionIdKey);
        if (rc) {
            *rc = xpc_dictionary_get_uint64(resp, kMsgReturnCode);
        }

    }
    else {
        xpc_dictionary_set_value(msg, kAssertionPropertiesMsg, desc);
        xpc_connection_send_message(assertionConnection, msg);
    }

    xpc_release(msg);
    xpc_release(desc);

    return remoteID;
}

void sendAsyncReleaseMsg(IOPMAssertionID remoteID)
{
    xpc_object_t            msg = NULL;


    msg = xpc_dictionary_create(NULL, NULL, 0);
    if (!msg) {
        ERROR_LOG("Failed to create xpc msg object\n");
        return;
    }

    DEBUG_LOG("Sending Assertion release message for assertion Id 0x%x\n", remoteID);
    xpc_dictionary_set_uint64(msg, kAssertionReleaseMsg, remoteID);

    xpc_connection_send_message(assertionConnection, msg);
    xpc_release(msg);

    return;
}

void offloadAssertions(bool clearOldOnes)
{
    int idx;
    IOPMAssertionID remoteID;
    CFNumberRef remoteIDCf;
    CFTypeRef   value;
    IOReturn rc;
    bool released;

    for (idx=0; idx < kMaxAsyncAssertions; idx++) {

        IOPMAssertionID id = ASYNC_ID_FROM_IDX(idx);
        if (!CFDictionaryGetValueIfPresent(gAssertionsDict, (uintptr_t)idx, (const void **)&value)) {
            continue;
        }

        if (isA_CFNumber(value)) {
            // This is the remoteID for previously offloaded assertion
            if (clearOldOnes) {
                CFDictionaryRemoveValue(gAssertionsDict, (uintptr_t)idx);
            }
            continue;
        }
        if (!isA_CFDictionary(value)) {
            ERROR_LOG("Unexpected type in the assertions dictionary with id 0x%x\n", id);
            continue;
        }

        released = CFDictionaryContainsKey(value, kIOPMAssertionReleaseDateKey);

        if (!released) {
            // Not sending the released assertion. But, we may want to send it in future
            // for logging purposes.
            rc = kIOReturnSuccess;
            remoteID = sendAsyncAssertionMsg(true, value, &rc);
            if (rc != kIOReturnSuccess) {
                ERROR_LOG("powerd returned err 0x%x to create assertion with id 0x%x. Dropping the assertion\n",
                          rc, id);

                CFDictionaryRemoveValue(gAssertionsDict, (uintptr_t)idx);
                continue;
            }
            if (remoteID == kIOPMNullAssertionID) {
                ERROR_LOG("Failed to send the assertion with id 0x%x to remote end\n", id);
                continue;
            }

            remoteIDCf = CFNumberCreate(0, kCFNumberIntType, &remoteID);
            if (!remoteIDCf) {
                ERROR_LOG("Failed to create the remoteID to CF for id 0x%x\n", id);
                continue;
            }
            DEBUG_LOG("powerd returned assertion id 0x%x for async id 0x%x\n",
                      remoteID, id);

            CFDictionaryReplaceValue(gAssertionsDict, (uintptr_t)idx, remoteIDCf);
            CFRelease(remoteIDCf);
        }
        else {
            DEBUG_LOG("Notified powerd about already released async id 0x%x\n", id);
            CFDictionaryRemoveValue(gAssertionsDict, (uintptr_t)idx);
        }

    }
    nextOffload_ts = 0;
    dispatch_suspend(offloader);

}

void processCheckAssertionsMsg(xpc_object_t msg)
{
    uint32_t blockers = 0;
    CFTypeRef value;
    CFDictionaryRef props;
    uint64_t    token = 0;
    int idx;

    for (idx=0; idx < kMaxAsyncAssertions; idx++) {
        IOPMAssertionID id = ASYNC_ID_FROM_IDX(idx);

        if (!CFDictionaryGetValueIfPresent(gAssertionsDict, (uintptr_t)idx, (const void **)&value)) {
            continue;
        }

        if (isA_CFNumber(value)) {
            // This is the remoteID for previously offloaded assertion
            IOPMAssertionID remoteId = kIOPMNullAssertionID;
            CFNumberGetValue(value, kCFNumberDoubleType, &remoteId);
            DEBUG_LOG("Received assertion check when offloaded assertion exists(0x%x)\n", remoteId);
            continue;
        }
        if (!isA_CFDictionary(value)) {
            ERROR_LOG("Unexpected type in the assertions dictionary with id 0x%x\n", id);
            continue;
        }
        props = value;

        CFNumberRef numRef = CFDictionaryGetValue(props, kIOPMAssertionLevelKey);
        int level = kIOPMAssertionLevelOn;
        if (isA_CFNumber(numRef)) {
            CFNumberGetValue(numRef, kCFNumberIntType, &level);
        }
        if (level != kIOPMAssertionLevelOn) {
            blockers++;
        }
    }

    xpc_object_t reply = xpc_dictionary_create_reply(msg);
    if (!reply) {
        ERROR_LOG("Failed to create xpc reply object\n");
        return;
    }

    // Taken the token from incoming message and put it in reply
    token = xpc_dictionary_get_uint64(msg, kAssertionCheckTokenKey);
    xpc_dictionary_set_uint64(reply, kAssertionCheckTokenKey, token);
    xpc_dictionary_set_uint64(reply, kAssertionCheckCountKey, blockers);

    DEBUG_LOG("Replying to assertion check message with count %d token:%llu\n", blockers, token);
    xpc_connection_t remote = xpc_dictionary_get_remote_connection(msg);
    xpc_connection_send_message(remote, reply);
    xpc_release(reply);

}

void processAssertionTimeout(xpc_object_t msg)
{
    IOPMAssertionID id;
    int idx;

    id = xpc_dictionary_get_uint64(msg, kAssertionTimeoutMsg);
    idx = ASYNC_IDX_FROM_ID(id);
    CFDictionaryRemoveValue(gAssertionsDict, (uintptr_t)idx);

    DEBUG_LOG("Received assertion timeout message for asyncId 0x%x\n", id);
}

void processRemoteMsg(xpc_object_t msg)
{
    xpc_type_t type = xpc_get_type(msg);

    if (type == XPC_TYPE_DICTIONARY) {
        if ((xpc_dictionary_get_value(msg, kAssertionCheckMsg))) {
            processCheckAssertionsMsg(msg);
        }
        else if ((xpc_dictionary_get_value(msg, kAssertionTimeoutMsg))) {
            processAssertionTimeout(msg);
        }
        else {
            ERROR_LOG("Unexpected message from async assertions connections\n");
        }
    }
    else if (type == XPC_TYPE_ERROR) {
        if (msg == XPC_ERROR_CONNECTION_INTERRUPTED) {
            // powerd must have crashed
            dispatch_async(getPMQueue(), ^{offloadAssertions(true);});
        }
        else {
            ERROR_LOG("Irrecoverable error for assertion creation\n");
            IOPMDisableAsyncAssertions();
        }
    }
}

void initialSetup( )
{
    if (gAssertionsDict) {
        return;
    }
    // Keys into this array integers and doesn't need retain/release
    // Values are either CFDictionary or CFNumber type objects
    gAssertionsDict = CFDictionaryCreateMutable(NULL, kMaxAsyncAssertions,
            NULL, &kCFTypeDictionaryValueCallBacks);
    if (!gAssertionsDict) {
        ERROR_LOG("Failed to create gAssertionsDict\n");
        goto error_exit;
    }

    offloader = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, getPMQueue());
    if (offloader) {

        dispatch_source_set_event_handler(offloader, ^{ offloadAssertions(false); });
        dispatch_source_set_cancel_handler(offloader, ^{
                dispatch_release(offloader);
                offloader = NULL;
                });

        dispatch_source_set_timer(offloader, dispatch_time(DISPATCH_TIME_NOW, 0), kDefaultOffloadDelay, 0);
    }

    assertionConnection = xpc_connection_create_mach_service(POWERD_XPC_ID, dispatch_get_main_queue(), 0);
    if (!assertionConnection) {
        ERROR_LOG("Failed to create assertionConnection\n");
        goto error_exit;
    }

    xpc_connection_set_target_queue(assertionConnection, getPMQueue());
    xpc_connection_set_event_handler(assertionConnection,
            ^(xpc_object_t msg ) {processRemoteMsg(msg); });

    xpc_connection_resume(assertionConnection);

    return;

error_exit:
    if (gAssertionsDict) {
        CFRelease(gAssertionsDict);
        gAssertionsDict = NULL;
    }
    if (offloader) {
        dispatch_resume(offloader);
        dispatch_cancel(offloader);
    }

    if (assertionConnection) {
        xpc_connection_cancel(assertionConnection);
    }
}


bool createAsyncAssertion(CFDictionaryRef AssertionProperties, IOPMAssertionID *id)
{
    __block bool created = true;
    static uint32_t gNextAssertionIdx = 0;
    CFStringRef     assertionTypeString;

    if (!gAsyncMode) {
        return false;
    }
    assertionTypeString = CFDictionaryGetValue(AssertionProperties, kIOPMAssertionTypeKey);

    if (!isA_CFString(assertionTypeString) ||
            !(CFEqual(assertionTypeString, kIOPMAssertionTypePreventUserIdleSystemSleep) ||
              CFEqual(assertionTypeString, kIOPMAssertionTypeNoIdleSleep)) ) {
        return false;
    }
    dispatch_queue_t pmQ = getPMQueue();
    if (!pmQ) {
        return false;
    }

    dispatch_sync(pmQ, ^(void){

        uint32_t  idx;
        CFTimeInterval      timeout_secs = kDefaultOffloadDelay;
        uint64_t            timeout_ts;

        CFMutableDictionaryRef  mutableProps;
        CFNumberRef numRef;

        // Initial setup
        initialSetup();

        // Generate an id
        for (idx=gNextAssertionIdx; CFDictionaryContainsKey(gAssertionsDict, (uintptr_t)idx) == true;) {
            idx = (idx+1) % kMaxAsyncAssertions;
            if (idx == gNextAssertionIdx) break;
        }

        if (CFDictionaryContainsKey(gAssertionsDict, (uintptr_t)idx) == true) {
            created = false;
            return;
        }

        CFDateRef start_date = CFDateCreate(0, CFAbsoluteTimeGetCurrent());
        if (!start_date) {
            created = false;
            return;
        }

        mutableProps = CFDictionaryCreateMutableCopy(NULL, 0, AssertionProperties);
        if (!mutableProps) {
            created = false;
            CFRelease(start_date);
            return;
        }

        CFDictionarySetValue(mutableProps, kIOPMAssertionCreateDateKey, start_date);
        CFRelease(start_date);
        CFDictionarySetValue(gAssertionsDict, (uintptr_t)idx, (const void *)mutableProps);
        gNextAssertionIdx = (idx+1) % kMaxAsyncAssertions;

        *id = ASYNC_ID_FROM_IDX(idx);
    
        numRef = CFNumberCreate(0, kCFNumberSInt32Type, id);
        if (numRef) {
            CFDictionarySetValue(mutableProps, kIOPMAsyncClientAssertionIdKey, numRef);
            CFRelease(numRef);
        }

        numRef = CFDictionaryGetValue(mutableProps, kIOPMAssertionTimeoutKey);
        if (isA_CFNumber(numRef))  {
            CFNumberGetValue(numRef, kCFNumberDoubleType, &timeout_secs);
        }
        if (!timeout_secs || (timeout_secs > kDefaultOffloadDelay)) {
            timeout_secs = kDefaultOffloadDelay;
        }
        timeout_ts = timeout_secs + getMonotonicTime();

        if (!nextOffload_ts || (timeout_ts < nextOffload_ts)) {
            dispatch_source_set_timer(offloader,
                      dispatch_time(DISPATCH_TIME_NOW, timeout_secs*NSEC_PER_SEC), DISPATCH_TIME_FOREVER, 0);
            if (!nextOffload_ts) {
                dispatch_resume(offloader);
            }
            nextOffload_ts = timeout_ts;
        }

        CFRelease(mutableProps);

    });

    return created;
}

void releaseAsyncAssertion(IOPMAssertionID AssertionID)
{

    dispatch_queue_t pmQ = getPMQueue();
    if (!pmQ) {
        return;
    }

    dispatch_sync(pmQ, ^{
        int idx = ASYNC_IDX_FROM_ID(AssertionID);
        IOPMAssertionID remoteID;
        CFTypeRef   value;

        if (!CFDictionaryGetValueIfPresent(gAssertionsDict, (uintptr_t)idx, (const void **)&value)) {
            ERROR_LOG("Failed to get the assertion details for id:0x%x\n", AssertionID);
            return;
        }

        if (isA_CFNumber(value)) {
            // Send assertion release message to powerd
            CFNumberGetValue(value, kCFNumberIntType, &remoteID);
            sendAsyncReleaseMsg(remoteID);

            CFDictionaryRemoveValue(gAssertionsDict, (uintptr_t)idx);
            DEBUG_LOG("Releasing assertion id 0x%x asyncId 0x%x\n", remoteID, AssertionID);
            return;
        }
        else if (isA_CFDictionary(value)) {

            CFDateRef release_date = CFDateCreate(0, CFAbsoluteTimeGetCurrent());
            if (release_date) {
                CFDictionarySetValue((CFMutableDictionaryRef)value, kIOPMAssertionReleaseDateKey, release_date);
                CFRelease(release_date);
            }

            DEBUG_LOG("Releasing assertion asyncId 0x%x\n", AssertionID);
        }
        else {
            ERROR_LOG("Unexpected data type in gAssertionsDict for id:0x%x\n", AssertionID);
            return;
        }
    });

}

static void updateProperty(const void *key, const void *value, void *context __unused)
{
    CFDictionarySetValue(gAssertionsDict, key, value);
}


void setAsyncAssertionProperties(CFMutableDictionaryRef properties, IOPMAssertionID AssertionID)
{

    dispatch_queue_t pmQ = getPMQueue();
    if (!pmQ) {
        return;
    }

    dispatch_sync(pmQ, ^{
        int idx = ASYNC_IDX_FROM_ID(AssertionID);
        CFTypeRef   value;
        IOReturn rc = kIOReturnSuccess;

        if (!CFDictionaryGetValueIfPresent(gAssertionsDict, (uintptr_t)idx, (const void **)&value)) {
            ERROR_LOG("Failed to get the assertion deatils for id:0x%x\n", AssertionID);
            return;
        }

        if (isA_CFNumber(value)) {
            // Send assertion properties update  message to powerd
            CFDictionarySetValue(properties, kIOPMAssertionIdKey, value);
            sendAsyncAssertionMsg(false, properties, &rc);
            if (rc != kIOReturnSuccess) {
                ERROR_LOG("powerd returned err 0x%x for properties update on id:0x%x\n",
                             rc, AssertionID);
            }

            return;
        }
        else if (isA_CFDictionary(value)) {

            CFDictionaryApplyFunction(properties, updateProperty, NULL);
        }
        else {
            ERROR_LOG("Unexpected data type in gAssertionsDict for id:0x%x\n", AssertionID);
            return;
        }
    });

}

/******************************************************************************
 * IOPMAssertionCreate
 *
 * Deprecated but still supported wrapper for IOPMAssertionCreateWithProperties
 ******************************************************************************/
IOReturn IOPMAssertionCreate(
                             CFStringRef             AssertionType,
                             IOPMAssertionLevel      AssertionLevel,
                             IOPMAssertionID         *AssertionID)
{
    return IOPMAssertionCreateWithName(AssertionType, AssertionLevel,
                                       CFSTR("Nameless (via IOPMAssertionCreate)"), AssertionID);
}

/******************************************************************************
 * IOPMAssertionCreateWithName
 *
 * Deprecated but still supported wrapper for IOPMAssertionCreateWithProperties
 ******************************************************************************/
IOReturn IOPMAssertionCreateWithName(
                                     CFStringRef          AssertionType, 
                                     IOPMAssertionLevel   AssertionLevel,
                                     CFStringRef          AssertionName,
                                     IOPMAssertionID      *AssertionID)
{
    CFMutableDictionaryRef      properties = NULL;
    IOReturn                    result = kIOReturnError;
    CFNumberRef                 numRef;

    if (!AssertionName || !AssertionID || !AssertionType)
        return kIOReturnBadArgument;

    numRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &AssertionLevel);
    if (!isA_CFNumber(numRef)) {
        return result;
    }
    properties = CFDictionaryCreateMutable(0, 3, &kCFTypeDictionaryKeyCallBacks, 
                                           &kCFTypeDictionaryValueCallBacks);

    if (properties)
    {

        CFDictionarySetValue(properties, kIOPMAssertionTypeKey, AssertionType);

        CFDictionarySetValue(properties, kIOPMAssertionNameKey, AssertionName);

        CFDictionarySetValue(properties, kIOPMAssertionLevelKey, numRef);
        CFRelease(numRef);

        result = IOPMAssertionCreateWithProperties(properties, AssertionID);

        CFRelease(properties);
    }    

    return result;
}

/******************************************************************************
 * IOPMAssertionCreateWithDescription
 *
 ******************************************************************************/

static CFMutableDictionaryRef createAssertionDescription(
                                       CFStringRef  AssertionType,
                                       CFStringRef  Name,
                                       CFStringRef  Details,
                                       CFStringRef  HumanReadableReason,
                                       CFStringRef  LocalizationBundlePath,
                                       CFTimeInterval   Timeout,
                                       CFStringRef  TimeoutAction)
{
    CFMutableDictionaryRef  descriptor = NULL;

    descriptor = CFDictionaryCreateMutable(0, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (!descriptor) {
        goto exit;
    }

    CFDictionarySetValue(descriptor, kIOPMAssertionNameKey, Name);

    CFDictionarySetValue(descriptor, kIOPMAssertionTypeKey, AssertionType);

    if (Details) {
        CFDictionarySetValue(descriptor, kIOPMAssertionDetailsKey, Details);
    }
    if (HumanReadableReason) {
        CFDictionarySetValue(descriptor, kIOPMAssertionHumanReadableReasonKey, HumanReadableReason);
    }
    if (LocalizationBundlePath) {
        CFDictionarySetValue(descriptor, kIOPMAssertionLocalizationBundlePathKey, LocalizationBundlePath);
    }
    if (Timeout) {
        CFNumberRef Timeout_num = CFNumberCreate(0, kCFNumberDoubleType, &Timeout);
        CFDictionarySetValue(descriptor, kIOPMAssertionTimeoutKey, Timeout_num);
        CFRelease(Timeout_num);
    }
    if (TimeoutAction) {
        CFDictionarySetValue(descriptor, kIOPMAssertionTimeoutActionKey, TimeoutAction);
    }

exit:
    return descriptor;

}

IOReturn    IOPMAssertionCreateWithDescription(
                                               CFStringRef  AssertionType,
                                               CFStringRef  Name,
                                               CFStringRef  Details,
                                               CFStringRef  HumanReadableReason,
                                               CFStringRef  LocalizationBundlePath,
                                               CFTimeInterval   Timeout,
                                               CFStringRef  TimeoutAction,
                                               IOPMAssertionID  *AssertionID)
{
    CFMutableDictionaryRef  descriptor = NULL;
    IOReturn ret = kIOReturnError;

    if (!AssertionType || !Name || !AssertionID) {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    descriptor = createAssertionDescription(AssertionType, Name, Details, HumanReadableReason,
                                            LocalizationBundlePath, Timeout, TimeoutAction);
    if (!descriptor) {
        goto exit;
    }

    ret = IOPMAssertionCreateWithProperties(descriptor, AssertionID);

    CFRelease(descriptor);

exit:
    return ret;
}


IOReturn    IOPMAssertionCreateWithAutoTimeout(
                                               CFStringRef  AssertionType,
                                               CFStringRef  Name,
                                               CFStringRef  Details,
                                               CFStringRef  HumanReadableReason,
                                               CFStringRef  LocalizationBundlePath,
                                               CFTimeInterval   Timeout,
                                               CFStringRef  TimeoutAction,
                                               IOPMAssertionID  *AssertionID)
{
    CFMutableDictionaryRef  descriptor = NULL;
    IOReturn ret = kIOReturnError;

    if (!AssertionType || !Name || !AssertionID) {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    descriptor = createAssertionDescription(AssertionType, Name, Details, HumanReadableReason,
                                            LocalizationBundlePath, Timeout, TimeoutAction);
    if (!descriptor) {
        goto exit;
    }

    CFDictionarySetValue(descriptor, kIOPMAssertionAutoTimesout, kCFBooleanTrue);

    ret = IOPMAssertionCreateWithProperties(descriptor, AssertionID);

    CFRelease(descriptor);

exit:
    return ret;
}

IOReturn    IOPMAssertionCreateWithResourceList(
                                               CFStringRef  AssertionType,
                                               CFStringRef  Name,
                                               CFStringRef  Details,
                                               CFStringRef  HumanReadableReason,
                                               CFStringRef  LocalizationBundlePath,
                                               CFTimeInterval   Timeout,
                                               CFStringRef  TimeoutAction,
                                               CFArrayRef   resources,
                                               IOPMAssertionID  *AssertionID)
{
    CFMutableDictionaryRef  descriptor = NULL;
    IOReturn ret = kIOReturnError;

    if (!AssertionType || !Name || !AssertionID || !isA_CFArray(resources)) {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    descriptor = createAssertionDescription(AssertionType, Name, Details, HumanReadableReason,
                                            LocalizationBundlePath, Timeout, TimeoutAction);
    if (!descriptor) {
        goto exit;
    }

    CFDictionarySetValue(descriptor, kIOPMAssertionResourcesUsed, resources);
    ret = IOPMAssertionCreateWithProperties(descriptor, AssertionID);

    CFRelease(descriptor);

exit:
    return ret;
}

/******************************************************************************
 * IOPMAssertionCreateWithProperties
 *
 ******************************************************************************/
IOReturn IOPMAssertionCreateWithProperties(
                                           CFDictionaryRef         AssertionProperties,
                                           IOPMAssertionID         *AssertionID)
{
    IOReturn                return_code     = kIOReturnError;
    kern_return_t           kern_result     = KERN_SUCCESS;
    mach_port_t             pm_server       = MACH_PORT_NULL;
    IOReturn                err;
    CFDataRef               flattenedProps  = NULL;
    CFStringRef             assertionTypeString = NULL;
    CFStringRef             name = NULL;
    CFMutableDictionaryRef  mutableProps = NULL;
    int                     disableAppSleep = 0;
    int                     enTrIntensity = -1;
    bool                    assertionEnabled = true;
    bool                    asyncMode = false;
#if TARGET_OS_IPHONE
    static int              resyncToken = 0;
    static CFMutableDictionaryRef  resyncCopy = NULL;
#endif

    if (!AssertionProperties || !AssertionID) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }
    assertionTypeString = CFDictionaryGetValue(AssertionProperties, kIOPMAssertionTypeKey);
    name = CFDictionaryGetValue(AssertionProperties, kIOPMAssertionNameKey);

    if (!isA_CFString(assertionTypeString) || !isA_CFString(name)) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }
    err = pm_connect_init(&pm_server);
    if(kIOReturnSuccess != err) {
        return_code = kIOReturnInternalError;
        goto exit;
    }




#if TARGET_OS_IPHONE

    if (isA_CFString(assertionTypeString) && 
        CFEqual(assertionTypeString, kIOPMAssertionTypeEnableIdleSleep) && !resyncToken) {

        resyncCopy = CFDictionaryCreateMutableCopy(NULL, 0, AssertionProperties);
        notify_register_dispatch( kIOUserAssertionReSync, 
                                  &resyncToken, dispatch_get_main_queue(),
                                  ^(int t __unused) {
                                  IOPMAssertionID id;
                                  IOPMAssertionCreateWithProperties(resyncCopy, &id);
                                  });
    }
#endif    

    CFNumberRef numRef = CFDictionaryGetValue(AssertionProperties, kIOPMAssertionLevelKey);
    if (isA_CFNumber(numRef)) {
        int level = 0;
        CFNumberGetValue(numRef, kCFNumberIntType, &level);
        if (level == kIOPMAssertionLevelOff) {
            assertionEnabled = false;
        }
    }

    if (collectBackTrace && assertionEnabled) {
        if (!mutableProps) {
            mutableProps = CFDictionaryCreateMutableCopy(NULL, 0, AssertionProperties);
            if (!mutableProps) {
                return_code = kIOReturnInternalError;
                goto exit;
            }
        }
        saveBackTrace(mutableProps);
    }

    flattenedProps = CFPropertyListCreateData(0, (mutableProps != NULL) ? mutableProps : AssertionProperties, 
                                              kCFPropertyListBinaryFormat_v1_0, 0, NULL /* error */);
    if (!flattenedProps) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }


    asyncMode = createAsyncAssertion(
            AssertionProperties, (IOPMAssertionID *)AssertionID);
    if (!asyncMode) {
        kern_result = io_pm_assertion_create( pm_server,
                                              (vm_offset_t)CFDataGetBytePtr(flattenedProps),
                                              CFDataGetLength(flattenedProps),
                                              (int *)AssertionID,
                                              &disableAppSleep,
                                              &enTrIntensity,
                                              &return_code);


        if(KERN_SUCCESS != kern_result) {
            return_code = kIOReturnInternalError;
            goto exit;
        }
    }
    else {
            return_code = kIOReturnSuccess;
#if !TARGET_OS_SIMULATOR
            enTrIntensity = kEnTrQualSPKeepSystemAwake;
#endif
    }

#if !TARGET_OS_IPHONE
    if (disableAppSleep) {
        CFStringRef assertionName = NULL;
        CFStringRef appSleepString = NULL;

        assertionName = CFDictionaryGetValue(AssertionProperties, kIOPMAssertionNameKey);
        appSleepString = CFStringCreateWithFormat(NULL, NULL, 
                                                  CFSTR("App is holding power assertion %u with name \'%@\' "),
                                                  *AssertionID, assertionName);

        __CFRunLoopSetOptionsReason(__CFRunLoopOptionsTakeAssertion,  appSleepString);

        CFRelease(appSleepString);
    }
#endif


#if !TARGET_OS_SIMULATOR
        if ((enTrIntensity != kEnTrQualNone) && assertionEnabled) {
            entr_act_begin(kEnTrCompSysPower, kEnTrActSPPMAssertion, *AssertionID,
                                    enTrIntensity, kEnTrValNone);
        }
#endif

exit:
    if (flattenedProps) {
        CFRelease(flattenedProps);
    }

    if (MACH_PORT_NULL != pm_server) {
        pm_connect_close(pm_server);
    }

    // Release mutableProps if allocated in this function
    if (mutableProps)
        CFRelease(mutableProps);

    return return_code;
}

/******************************************************************************
 * IOPMPerformBlockWithAssertion
 *
 ******************************************************************************/
IOReturn IOPMPerformBlockWithAssertion(
                                       CFDictionaryRef assertion_properties,
                                       dispatch_block_t the_block)
{
    IOPMAssertionID _id = kIOPMNullAssertionID;
    IOReturn rc;
    
    if (!assertion_properties || !the_block) {
        return kIOReturnBadArgument;
    }
    
    rc = IOPMAssertionCreateWithProperties(assertion_properties, &_id);
    if (rc != kIOReturnSuccess) {
        return rc;
    }
    
    the_block();
    
    if (kIOPMNullAssertionID != _id) {
        IOPMAssertionRelease(_id);
    }
    
    return kIOReturnSuccess;
}

/******************************************************************************
 * IOPMAssertionsRetain
 *
 ******************************************************************************/
void IOPMAssertionRetain(IOPMAssertionID theAssertion)
{
    IOReturn                return_code     = kIOReturnError;
    kern_return_t           kern_result     = KERN_SUCCESS;
    mach_port_t             pm_server       = MACH_PORT_NULL;
    IOReturn                err;
    int                     disableAppSleep = 0;
    int                     enableAppSleep = 0;
    int                     retainCnt = 0;

    if (!theAssertion) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    err = pm_connect_init(&pm_server);
    if(kIOReturnSuccess != err) {
        return_code = kIOReturnInternalError;
        goto exit;
    }

    if (gAssertionsDict && IS_ASYNC_ID(theAssertion)) {
        ERROR_LOG("Assertion retain not supported in async mode\n");
        return_code = kIOReturnInternalError;
        goto exit;
    }

    kern_result = io_pm_assertion_retain_release( pm_server, 
                                                  (int)theAssertion,
                                                  kIOPMAssertionMIGDoRetain,
                                                  &retainCnt,
                                                  &disableAppSleep,
                                                  &enableAppSleep,
                                                  &return_code);

    if(KERN_SUCCESS != kern_result) {
        return_code = kIOReturnInternalError;
    } else {
#if !TARGET_OS_IPHONE
        if (disableAppSleep) {
            CFStringRef appSleepString = NULL;

            appSleepString = CFStringCreateWithFormat(NULL, NULL, CFSTR("App is holding power assertion %u"),
                                                      theAssertion);
            __CFRunLoopSetOptionsReason(__CFRunLoopOptionsTakeAssertion,  appSleepString);

            CFRelease(appSleepString);
        }
#endif
#if !TARGET_OS_SIMULATOR
        entr_act_modify(kEnTrCompSysPower, kEnTrActSPPMAssertion,
                                (int)theAssertion, kEnTrModSPRetain, kEnTrValNone); 
#endif
    }


exit:
    if (MACH_PORT_NULL != pm_server) {
        pm_connect_close(pm_server);
    }
    return;
}


/******************************************************************************
 * IOPMAssertionsRelease
 *
 ******************************************************************************/
IOReturn IOPMAssertionRelease(IOPMAssertionID AssertionID)
{
    IOReturn                return_code = kIOReturnError;
    kern_return_t           kern_result = KERN_SUCCESS;
    mach_port_t             pm_server = MACH_PORT_NULL;
    IOReturn                err;
    int                     disableAppSleep = 0;
    int                     enableAppSleep = 0;
    int                     retainCnt = 0;
    bool                    asyncMode = false;

    if (!AssertionID) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    err = pm_connect_init(&pm_server);
    if(kIOReturnSuccess != err) {
        return_code = kIOReturnInternalError;
        goto exit;
    }

    if (gAssertionsDict && IS_ASYNC_ID(AssertionID)) {
        asyncMode = true;
        releaseAsyncAssertion(AssertionID);
    }

    if (!asyncMode) {
            kern_result = io_pm_assertion_retain_release( pm_server,
                                                          (int)AssertionID,
                                                          kIOPMAssertionMIGDoRelease,
                                                          &retainCnt,
                                                          &disableAppSleep,
                                                          &enableAppSleep,
                                                          &return_code);

        if(KERN_SUCCESS != kern_result) {
            return_code = kIOReturnInternalError;
        }
    }
    else {
        return_code = kIOReturnSuccess;
    }
#if !TARGET_OS_IPHONE
    if (enableAppSleep) {
        CFStringRef appSleepString = NULL;

        appSleepString = CFStringCreateWithFormat(NULL, NULL, CFSTR("App released its last power assertion %u"),
                                                  AssertionID);
        __CFRunLoopSetOptionsReason(__CFRunLoopOptionsDropAssertion, appSleepString);

        CFRelease(appSleepString);
    }
#endif

#if !TARGET_OS_SIMULATOR
    if (retainCnt)
        entr_act_modify(kEnTrCompSysPower, kEnTrActSPPMAssertion,
                                (int)AssertionID, kEnTrModSPRelease, kEnTrValNone);
    else
        entr_act_end(kEnTrCompSysPower, kEnTrActSPPMAssertion,
                                (int)AssertionID, kEnTrQualNone, kEnTrValNone);
#endif

    pm_connect_close(pm_server);
exit:
    return return_code;
}


/******************************************************************************
 * IOPMAssertionSetProperty
 *
 ******************************************************************************/
IOReturn IOPMAssertionSetProperty(IOPMAssertionID theAssertion, CFStringRef theProperty, CFTypeRef theValue)
{
    IOReturn                return_code     = kIOReturnError;
    kern_return_t           kern_result     = KERN_SUCCESS;
    mach_port_t             pm_server       = MACH_PORT_NULL;
    CFDataRef               sendData        = NULL;
    CFMutableDictionaryRef  sendDict        = NULL;
    int                     disableAppSleep = 0;
    int                     enableAppSleep = 0;
    int                     enTrIntensity = -1;
    bool                    assertionEnabled = false;
    bool                    assertionDisabled = false;
    bool                    asyncMode = false;

    if (!theAssertion) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    return_code = pm_connect_init(&pm_server);

    if(kIOReturnSuccess != return_code) {
        goto exit;
    }

    sendDict = CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    if (!sendDict) {
        return_code = kIOReturnNoMemory;
        goto exit;
    }

    CFDictionarySetValue(sendDict, theProperty, theValue);
    if ((CFStringCompare(theProperty, kIOPMAssertionLevelKey, 0) == kCFCompareEqualTo) && (isA_CFNumber(theValue))) {
        int level = 0;
        CFNumberGetValue(theValue, kCFNumberIntType, &level);
        if (level == kIOPMAssertionLevelOff) {
            assertionDisabled = true;
        }
        else if (level == kIOPMAssertionLevelOn) {
            assertionEnabled = true;
        }
    }

    if (collectBackTrace && assertionEnabled) {
        saveBackTrace(sendDict);
    }

    sendData = CFPropertyListCreateData(0, sendDict, kCFPropertyListBinaryFormat_v1_0, 0, NULL /* error */);
    CFRelease(sendDict);

    if (gAssertionsDict && IS_ASYNC_ID(theAssertion)) {
        asyncMode = true;
        setAsyncAssertionProperties(sendDict, theAssertion);
    }



    if (!asyncMode) {
        kern_result = io_pm_assertion_set_properties(pm_server,
                                                     (int)theAssertion,
                                                     (vm_offset_t)CFDataGetBytePtr(sendData),
                                                     CFDataGetLength(sendData),
                                                     &disableAppSleep,
                                                     &enableAppSleep,
                                                     &enTrIntensity,
                                                     (int *)&return_code);

        if(KERN_SUCCESS != kern_result) {
            return_code = kIOReturnInternalError;
            goto exit;
        }
    }
    else {
        return_code = kIOReturnSuccess;
    }

#if !TARGET_OS_IPHONE
    if (disableAppSleep) {
        CFStringRef appSleepString = NULL;

        appSleepString = CFStringCreateWithFormat(NULL, NULL, CFSTR("App is holding power assertion %u"),
                                                  theAssertion);
        __CFRunLoopSetOptionsReason(__CFRunLoopOptionsTakeAssertion,  appSleepString);

        CFRelease(appSleepString);
    }
    else if (enableAppSleep) {
        CFStringRef appSleepString = NULL;

        appSleepString = CFStringCreateWithFormat(NULL, NULL, CFSTR("App released its last power assertion %u"),
                                                  theAssertion);
        __CFRunLoopSetOptionsReason(__CFRunLoopOptionsDropAssertion, appSleepString);

        CFRelease(appSleepString);
    }
#endif


#if !TARGET_OS_SIMULATOR
    if (enTrIntensity != kEnTrQualNone) {
        if (assertionEnabled) {
            entr_act_begin(kEnTrCompSysPower, kEnTrActSPPMAssertion, theAssertion,
                           enTrIntensity, kEnTrValNone);
        }
        else if (assertionDisabled) {
            entr_act_end(kEnTrCompSysPower, kEnTrActSPPMAssertion, theAssertion,
                         kEnTrQualNone, kEnTrValNone);
        }
    }
#endif


exit:
    if (sendData)
        CFRelease(sendData);

    if (MACH_PORT_NULL != pm_server) {
        pm_connect_close(pm_server);
    }
    return return_code;    
}
/******************************************************************************
 * IOPMAssertionSetProcessState
 *
 ******************************************************************************/
IOReturn IOPMAssertionSetProcessState(pid_t pid, IOPMAssertionProcessStateType state)
{
    xpc_connection_t        connection = NULL;
    xpc_object_t            msg = NULL;

    connection = xpc_connection_create_mach_service(POWERD_XPC_ID,
                                                    dispatch_get_global_queue(DISPATCH_QUEUE_CONCURRENT, 0), 0);

    if (!connection) {
        return kIOReturnError;
    }

    xpc_connection_set_target_queue(connection,
                                    dispatch_get_global_queue(DISPATCH_QUEUE_CONCURRENT, 0));

    xpc_connection_set_event_handler(connection,
                                     ^(xpc_object_t e __unused) { });

    msg = xpc_dictionary_create(NULL, NULL, 0);
    if(!msg) {
        xpc_release(connection);
        os_log_error(OS_LOG_DEFAULT, "Failed to create xpc msg object\n");
        return kIOReturnError;
    }

    os_log_debug(OS_LOG_DEFAULT, "Setting Assertion State for PID %d to %d\n", pid, state);

    xpc_dictionary_set_uint64(msg, "pid", pid);
    xpc_dictionary_set_uint64(msg, kAssertionSetStateMsg, state);
    xpc_connection_resume(connection);
    xpc_connection_send_message(connection, msg);

    xpc_release(msg);
    xpc_release(connection);
    return kIOReturnSuccess;
}

/******************************************************************************
 * IOPMAssertionSetTimeout
 *
 ******************************************************************************/
IOReturn IOPMAssertionSetTimeout(IOPMAssertionID whichAssertion, 
                                 CFTimeInterval timeoutInterval)
{
    IOReturn            return_code = kIOReturnError;
    CFNumberRef         intervalNum = NULL;
    int                 timeoutSecs = (int)timeoutInterval;

    intervalNum = CFNumberCreate(0, kCFNumberIntType, &timeoutSecs);

    if (intervalNum) 
    {
        return_code = IOPMAssertionSetProperty(whichAssertion, kIOPMAssertionTimeoutKey, intervalNum);

        CFRelease(intervalNum);
    }

    return return_code;
}

/******************************************************************************
 * IOPMAssertionDeclareNotificationEvent
 *
 ******************************************************************************/
 IOReturn IOPMAssertionDeclareNotificationEvent(
                        CFStringRef          notificationName,
                        CFTimeInterval       secondsToDisplay,
                        IOPMAssertionID      *AssertionID)
{
    IOPMAssertionID     id = kIOPMNullAssertionID;
    IOReturn            ret = kIOReturnSuccess;
    io_registry_entry_t rootdomain = getPMRootDomainRef();
    CFBooleanRef        lidIsClosed = NULL;
    CFBooleanRef        desktopMode = NULL;

    if (rootdomain == MACH_PORT_NULL) 
        return kIOReturnInternalError;

    desktopMode = IORegistryEntryCreateCFProperty(rootdomain, 
                                                  CFSTR("DesktopMode"), kCFAllocatorDefault, 0);
    lidIsClosed = IORegistryEntryCreateCFProperty(rootdomain, 
                                                  CFSTR(kAppleClamshellStateKey), kCFAllocatorDefault, 0);

    if ((kCFBooleanTrue == lidIsClosed) && (kCFBooleanFalse == desktopMode)) {
        ret = kIOReturnNotReady;
        goto exit;
    }

    ret = IOPMAssertionCreateWithDescription(
                                             kIOPMAssertDisplayWake,
                                             notificationName, NULL, NULL, NULL,
                                             secondsToDisplay, kIOPMAssertionTimeoutActionRelease,
                                             &id);
    if (AssertionID)
        *AssertionID = id;

exit:
    if (lidIsClosed) CFRelease(lidIsClosed);
    if (desktopMode) CFRelease(desktopMode);

    return ret;
}

/******************************************************************************
 * IOPMAssertionDeclareSystemActivityWithProperties
 *
 ******************************************************************************/
IOReturn IOPMAssertionDeclareSystemActivityWithProperties(
    CFMutableDictionaryRef         assertionProperties,
    IOPMAssertionID                *AssertionID,
    IOPMSystemState                *SystemState)
{
    IOReturn        err;
    IOReturn        return_code = kIOReturnError;
    mach_port_t     pm_server   = MACH_PORT_NULL;
    kern_return_t   kern_result = KERN_SUCCESS;

    CFDataRef   flattenedProps  = NULL;
    CFStringRef assertionTypeString = NULL;
    CFStringRef name = NULL;

    if (!assertionProperties || !AssertionID || !SystemState) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    err = pm_connect_init(&pm_server);
    if(kIOReturnSuccess != err) {
        return_code = kIOReturnInternalError;
        goto exit;
    }

    name = CFDictionaryGetValue(assertionProperties, kIOPMAssertionNameKey);
    if (!isA_CFString(name)) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    assertionTypeString = CFDictionaryGetValue(assertionProperties, kIOPMAssertionTypeKey);
    /* Caller is not allowed to specify the AssertionType */
    if (isA_CFString(assertionTypeString)) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    if (collectBackTrace) {
        saveBackTrace(assertionProperties);
    }

    flattenedProps = CFPropertyListCreateData(0, assertionProperties,
                                              kCFPropertyListBinaryFormat_v1_0, 0, NULL /* error */);
    if (!flattenedProps) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    kern_result = io_pm_declare_system_active(
                                              pm_server,
                                              (int *)SystemState,
                                              (vm_offset_t)CFDataGetBytePtr(flattenedProps),
                                              CFDataGetLength(flattenedProps),
                                              (int *)AssertionID,
                                              &return_code);

    if(KERN_SUCCESS != kern_result) {
        return_code = kIOReturnInternalError;
        goto exit;
    }
exit:
    if (flattenedProps)
        CFRelease(flattenedProps);

    if (MACH_PORT_NULL != pm_server) {
        pm_connect_close(pm_server);
    }

    return return_code;
}

/******************************************************************************
 * IOPMAssertionDeclareSystemActivity
 *
 ******************************************************************************/
IOReturn IOPMAssertionDeclareSystemActivity(
                                            CFStringRef             AssertionName,
                                            IOPMAssertionID         *AssertionID,
                                            IOPMSystemState         *SystemState)
{
    IOReturn                return_code = kIOReturnError;
    CFMutableDictionaryRef  properties  = NULL;

    if (!AssertionName || !AssertionID || !SystemState) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    properties = CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(properties, kIOPMAssertionNameKey, AssertionName);

    return_code = IOPMAssertionDeclareSystemActivityWithProperties(properties,
                                                                   AssertionID,
                                                                   SystemState);

exit:
    if (properties)
        CFRelease(properties);

    return return_code;

}
/******************************************************************************
 * IOPMAssertionDeclareUserActivity
 *
 ******************************************************************************/
IOReturn IOPMAssertionDeclareUserActivity(
                                          CFStringRef          AssertionName,
                                          IOPMUserActiveType   userType,
                                          IOPMAssertionID      *AssertionID)
{

    IOReturn        return_code = kIOReturnError;
    mach_port_t     pm_server = MACH_PORT_NULL;
    kern_return_t   kern_result = KERN_SUCCESS;
    IOReturn        err;
    int             disableAppSleep = 0;


    CFMutableDictionaryRef  properties = NULL;
    CFDataRef               flattenedProps  = NULL;

    if (!AssertionName || !AssertionID) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    
    err = pm_connect_init(&pm_server);
    if(kIOReturnSuccess != err) {
        return_code = kIOReturnInternalError;
        goto exit;
    }

    properties = CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, 
                                           &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(properties, kIOPMAssertionNameKey, AssertionName);

    if (collectBackTrace) {
        saveBackTrace(properties);
    }

    flattenedProps = CFPropertyListCreateData(0, properties, 
                                              kCFPropertyListBinaryFormat_v1_0, 0, NULL /* error */);
    if (!flattenedProps) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    kern_result = io_pm_declare_user_active( 
                                            pm_server, 
                                            userType,
                                            (vm_offset_t)CFDataGetBytePtr(flattenedProps),
                                            CFDataGetLength(flattenedProps),
                                            (int *)AssertionID,
                                            &disableAppSleep,
                                            &return_code);


    if(KERN_SUCCESS != kern_result) {
        return_code = kIOReturnInternalError;
        goto exit;
    }
#if !TARGET_OS_IPHONE
    else if (disableAppSleep) {
        __CFRunLoopSetOptionsReason(__CFRunLoopOptionsTakeAssertion,
                                    CFSTR("App is holding 'DeclareUserActivity' power assertion"));
    }
#endif


exit:
    if (flattenedProps)
        CFRelease(flattenedProps);

    if (properties)
        CFRelease(properties);

    if (MACH_PORT_NULL != pm_server) {
        pm_connect_close(pm_server);
    }

    return return_code;
}


/******************************************************************************
 * IOPMDeclareNetworkClientActivity
 *
 ******************************************************************************/
IOReturn IOPMDeclareNetworkClientActivity(
                                          CFStringRef          AssertionName,
                                          IOPMAssertionID      *AssertionID)
{

    IOReturn        return_code = kIOReturnError;
    mach_port_t     pm_server = MACH_PORT_NULL;
    kern_return_t   kern_result = KERN_SUCCESS;
    IOReturn        err;
    int             disableAppSleep = 0;


    CFMutableDictionaryRef  properties = NULL;
    CFDataRef               flattenedProps  = NULL;

    if (!AssertionName || !AssertionID) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    err = pm_connect_init(&pm_server);
    if(kIOReturnSuccess != err) {
        return_code = kIOReturnInternalError;
        goto exit;
    }

    properties = CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, 
                                           &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(properties, kIOPMAssertionNameKey, AssertionName);

    if (collectBackTrace) {
        saveBackTrace(properties);
    }

    flattenedProps = CFPropertyListCreateData(0, properties, 
                                              kCFPropertyListBinaryFormat_v1_0, 0, NULL /* error */);
    if (!flattenedProps) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    kern_result = io_pm_declare_network_client_active(
                                                      pm_server, 
                                                      (vm_offset_t)CFDataGetBytePtr(flattenedProps),
                                                      CFDataGetLength(flattenedProps),
                                                      (int *)AssertionID,
                                                      &disableAppSleep,
                                                      &return_code);


    if(KERN_SUCCESS != kern_result) {
        return_code = kIOReturnInternalError;
        goto exit;
    }
#if !TARGET_OS_IPHONE
    else if (disableAppSleep) {
        __CFRunLoopSetOptionsReason(__CFRunLoopOptionsTakeAssertion,
                                    CFSTR("App is holding 'DeclareNetworkClientActivity' power assertion"));
    }
#endif


exit:
    if (flattenedProps)
        CFRelease(flattenedProps);

    if (properties)
        CFRelease(properties);

    if (MACH_PORT_NULL != pm_server) {
        pm_connect_close(pm_server);
    }

    return return_code;
}

/*****************************************************************************/
IOReturn IOPMSetReservePowerMode(bool enable)
{

    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result = KERN_SUCCESS;
    IOReturn                return_code;
    int                     rc = kIOReturnSuccess;

    return_code = _pm_connect(&pm_server);
    if (return_code != kIOReturnSuccess)
        return return_code;

    if(pm_server == MACH_PORT_NULL)
        return kIOReturnNotReady; 


    kern_result = io_pm_set_value_int( pm_server, kIOPMSetReservePowerMode, enable ? 1 : 0, &rc); 
    _pm_disconnect(pm_server);

    if (rc != kIOReturnSuccess)
        return rc;

    return kern_result;
}

/******************************************************************************
 * IOPMCopyAssertionsByProcessWithAllocator
 *
 ******************************************************************************/
static IOReturn _copyAssertionsByProcess(int operator, CFDictionaryRef *AssertionsByPid, CFAllocatorRef allocator)
{
    IOReturn                return_code     = kIOReturnError;
    CFArrayRef              flattenedDictionary = NULL;
    int                     flattenedArrayCount = 0;
    CFNumberRef             *newDictKeys = NULL;
    CFArrayRef              *newDictValues = NULL;

    if (!AssertionsByPid)
        return kIOReturnBadArgument;

    if ((operator != kIOPMAssertionMIGCopyAll) && (operator != kIOPMAssertionMIGCopyInactive)) {
        return kIOReturnBadArgument;
    }
    return_code = _copyPMServerObject(operator, 0, NULL, (CFTypeRef *)&flattenedDictionary);

    if (kIOReturnSuccess != return_code)
        goto exit;

    /*
     * This API returns a dictionary whose keys are process ID's.
     * This is perfectly acceptable in CoreFoundation, EXCEPT that you cannot
     * serialize a dictionary with CFNumbers for keys using CF or IOKit
     * serialization.
     *
     * To serialize this dictionary and pass it from configd to the caller's process,
     * we re-formatted it as a "flattened" array of dictionaries in configd,
     * and we will re-constitute with pid's for keys here.
     *
     * Next time around, I will simply not use CFNumberRefs for keys in API.
     */


    if (flattenedDictionary) {
        flattenedArrayCount = CFArrayGetCount(flattenedDictionary);
    }

    if (0 == flattenedArrayCount) {
        goto exit;
    }

    newDictKeys = (CFNumberRef *)malloc(sizeof(CFNumberRef) * flattenedArrayCount);
    newDictValues = (CFArrayRef *)malloc(sizeof(CFArrayRef) * flattenedArrayCount);

    if (!newDictKeys || !newDictValues)
        goto exit;

    for (int i=0; i < flattenedArrayCount; i++)
    {
        CFDictionaryRef dictionaryAtIndex = NULL;

        if ((dictionaryAtIndex = CFArrayGetValueAtIndex(flattenedDictionary, i)))
        {

            newDictKeys[i]      = CFDictionaryGetValue(dictionaryAtIndex, kIOPMAssertionPIDKey);
            newDictValues[i]    = CFDictionaryGetValue(dictionaryAtIndex, CFSTR("PerTaskAssertions"));
        }
    }

    *AssertionsByPid = CFDictionaryCreate(allocator,
                                          (const void **)newDictKeys, (const void **)newDictValues, flattenedArrayCount,
                                          &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    return_code = kIOReturnSuccess;

exit:
    if (newDictKeys)
        free(newDictKeys);
    if (newDictValues)
        free(newDictValues);
    if (flattenedDictionary)
        CFRelease(flattenedDictionary);
    return return_code;

}
IOReturn IOPMCopyAssertionsByProcessWithAllocator(CFDictionaryRef *AssertionsByPid, CFAllocatorRef allocator)
{
    return _copyAssertionsByProcess(kIOPMAssertionMIGCopyAll, AssertionsByPid, allocator);
}

/******************************************************************************
 * IOPMCopyAssertionsByProcess
 *
 ******************************************************************************/
IOReturn IOPMCopyAssertionsByProcess(CFDictionaryRef         *AssertionsByPid)
{
    return IOPMCopyAssertionsByProcessWithAllocator(AssertionsByPid, kCFAllocatorDefault);
}

/******************************************************************************
 * IOPMCopyAssertionsByType
 *
 ******************************************************************************/
IOReturn IOPMCopyAssertionsByType(CFStringRef type, CFArrayRef *assertionsArray)
{
    if (!assertionsArray) {
        return kIOReturnBadArgument;
    }

    return _copyPMServerObject(kIOPMAssertionMIGCopyByType, 0, type, (CFTypeRef *)assertionsArray);
}

/******************************************************************************
 * IOPMCopyAssertionsByType
 *
 ******************************************************************************/
IOReturn IOPMCopyInactiveAssertionsByProcess(CFDictionaryRef  *inactiveAssertions)
{
    return _copyAssertionsByProcess(kIOPMAssertionMIGCopyInactive, inactiveAssertions, kCFAllocatorDefault);
}

/******************************************************************************
 * IOPMAssertionCopyProperties
 *
 ******************************************************************************/
CFDictionaryRef IOPMAssertionCopyProperties(IOPMAssertionID theAssertion)
{
    CFDictionaryRef         theResult       = NULL;    

    _copyPMServerObject(kIOPMAssertionMIGCopyOneAssertionProperties, theAssertion, NULL, (CFTypeRef *)&theResult);

    return theResult;
}

/******************************************************************************
 * IOPMCopyAssertionsStatus
 *
 ******************************************************************************/
IOReturn IOPMCopyAssertionsStatus(CFDictionaryRef *AssertionsStatus)
{
    if (!AssertionsStatus)
        return kIOReturnBadArgument;

    return _copyPMServerObject(kIOPMAssertionMIGCopyStatus, 0, NULL, (CFTypeRef *)AssertionsStatus);
}

/******************************************************************************
 * IOPMCopyDeviceRestartPreventers
 *
 ******************************************************************************/
IOReturn IOPMCopyDeviceRestartPreventers(CFArrayRef *preventers)
{
    IOReturn            ret             = kIOReturnError;
    CFMutableArrayRef   assertionArr    = NULL;
    CFStringRef         types[]         = { kIOPMAssertPreventUserIdleSystemSleep,
                                            kIOPMAssertionTypeSystemIsActive };
    int                 numTypes        = sizeof(types)/sizeof(types[0]);
    
    for (int i = 0; i < numTypes; i++) {
        CFArrayRef array = NULL;
        
        ret = IOPMCopyAssertionsByType(types[i], &array);
        if (ret != kIOReturnSuccess) {
            goto exit;
        }
        
        if (array) {
            for (CFIndex j = 0; j < CFArrayGetCount(array); j++) {
                CFDictionaryRef assertion       = CFArrayGetValueAtIndex(array, j);
                CFBooleanRef    allowsRestart   = NULL;
                
                /* skip assertions that allow restart */
                allowsRestart = CFDictionaryGetValue(assertion, kIOPMAssertionAllowsDeviceRestart);
                if (allowsRestart && CFBooleanGetValue(allowsRestart)) {
                    continue;
                }
                
                if (!assertionArr) {
                    assertionArr = CFArrayCreateMutable(0, 0, &kCFTypeArrayCallBacks);
                    if (!assertionArr) {
                        CFRelease(array);
                        goto exit;
                    }
                }
                
                CFArrayAppendValue(assertionArr, assertion);
            }
            CFRelease(array);
        }
    }
    
exit:
    *preventers = assertionArr;
    return ret;
}

/******************************************************************************
 * _copyPMServerObject
 *
 ******************************************************************************/
__private_extern__ IOReturn _copyPMServerObject(int selector, int assertionID,
                                                CFTypeRef selectorData, CFTypeRef *objectOut)
{
    IOReturn                return_code     = kIOReturnError;
    kern_return_t           kern_result     = KERN_SUCCESS;
    mach_port_t             pm_server       = MACH_PORT_NULL;
    vm_offset_t             theResultsPtr   = 0;
    mach_msg_type_number_t  theResultsCnt   = 0;
    CFDataRef               theResultData   = NULL;
    CFDataRef               inData          = NULL;
    vm_offset_t             inDataPtr       = NULL;
    CFIndex                 inDataLen       = 0;

    *objectOut = NULL;

    if(kIOReturnSuccess != (return_code = pm_connect_init(&pm_server))) {
        return kIOReturnNotFound;
    }

    if (selectorData) {
        inData = CFPropertyListCreateData(0, selectorData, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
        if (inData) {
            inDataPtr = (vm_offset_t)CFDataGetBytePtr(inData);
            inDataLen = CFDataGetLength(inData);
        }
    }
    kern_result = io_pm_assertion_copy_details(pm_server, assertionID, selector,
                                               inDataPtr, inDataLen,
                                               &theResultsPtr, &theResultsCnt, &return_code);

    if(KERN_SUCCESS != kern_result) {
        goto exit;
    }

    if (return_code != kIOReturnSuccess) {
        goto exit;
    }

    if ((theResultData = CFDataCreate(0, (const UInt8 *)theResultsPtr, (CFIndex)theResultsCnt)))
    {
        *objectOut = CFPropertyListCreateWithData(0, theResultData, kCFPropertyListImmutable, NULL, NULL);
        CFRelease(theResultData);
    }

    if (theResultsPtr && 0 != theResultsCnt) {
        vm_deallocate(mach_task_self(), theResultsPtr, theResultsCnt);
    }

exit:
    if (inData) {
        CFRelease(inData);
    }

    if (MACH_PORT_NULL != pm_server) {
        pm_connect_close(pm_server);
    }

    return kIOReturnSuccess;
}

/******************************************************************************
 * IOPMCopyAssertionActivityLog
 *
 ******************************************************************************/
IOReturn IOPMCopyAssertionActivityLogWithAllocator(CFArrayRef *activityLog, bool *overflow, CFAllocatorRef allocator)
{
    static uint32_t refCnt = UINT_MAX;
    
    return IOPMCopyAssertionActivityUpdateWithAllocator(activityLog, overflow, &refCnt, allocator);
    
}

/******************************************************************************
 * IOPMCopyAssertionActivityLog
 *
 ******************************************************************************/
IOReturn IOPMCopyAssertionActivityLog(CFArrayRef *activityLog, bool *overflow)
{
    static uint32_t refCnt = UINT_MAX;

    return IOPMCopyAssertionActivityUpdate(activityLog, overflow, &refCnt);

}

IOReturn IOPMCopyAssertionActivityUpdateWithAllocator(CFArrayRef *logUpdates, bool *overflow, uint32_t *refCnt, CFAllocatorRef allocator)
{
    uint32_t                of;
    IOReturn                rc = kIOReturnInternalError;
    CFDataRef               unfolder = NULL;
    vm_offset_t             logPtr = NULL;
    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result;
    mach_msg_type_number_t  logSize = 0;

    *logUpdates = NULL;
    _pm_connect(&pm_server);

    if(pm_server == MACH_PORT_NULL)
        return NULL;

    kern_result = io_pm_assertion_activity_log(pm_server,
                                               &logPtr, &logSize,
                                               refCnt, &of, &rc);

    if ((KERN_SUCCESS != kern_result) || (rc != kIOReturnSuccess)) {
        goto exit;
    }

    if (logSize == 0) {
        goto exit;
    }

    unfolder = CFDataCreateWithBytesNoCopy(0, (const UInt8 *)logPtr, (CFIndex)logSize, kCFAllocatorNull);
    if (unfolder)
    {
        *logUpdates = CFPropertyListCreateWithData(allocator, unfolder,
                                                   kCFPropertyListMutableContainers,
                                                   NULL, NULL);
        CFRelease(unfolder);
    }

    if (overflow) {
        *overflow = of ? true : false;
    }

exit:

    if (logPtr && logSize)  {
        vm_deallocate(mach_task_self(), logPtr, logSize);
    }

    if (MACH_PORT_NULL != pm_server)
        pm_connect_close(pm_server);
    
    return rc;
    
}
IOReturn IOPMCopyAssertionActivityUpdate(CFArrayRef *logUpdates, bool *overflow, uint32_t *refCnt)
{
    return IOPMCopyAssertionActivityUpdateWithAllocator(logUpdates, overflow,  refCnt, kCFAllocatorDefault);
}

/*****************************************************************************/
IOReturn IOPMSetAssertionActivityLog(bool enable)
{

    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result = KERN_SUCCESS;
    IOReturn                return_code;
    int                     rc = kIOReturnSuccess;

    return_code = _pm_connect(&pm_server);
    if(return_code != kIOReturnSuccess) {
        return return_code;
    }

    if(pm_server == MACH_PORT_NULL)
        return kIOReturnNotReady; 


    kern_result = io_pm_set_value_int( pm_server, kIOPMSetAssertionActivityLog, enable ? 1 : 0, &rc); 
    _pm_disconnect(pm_server);

    return kern_result;
}
/*****************************************************************************/
IOReturn IOPMSetAssertionActivityAggregate(bool enable)
{

    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result = KERN_SUCCESS;
    IOReturn                return_code;
    int                     rc = kIOReturnSuccess;

    return_code = _pm_connect(&pm_server);
    if(return_code != kIOReturnSuccess) {
        return return_code;
    }

    if(pm_server == MACH_PORT_NULL)
        return kIOReturnNotReady; 


    kern_result = io_pm_set_value_int( pm_server, kIOPMSetAssertionActivityAggregate, enable ? 1 : 0, &rc); 
    _pm_disconnect(pm_server);

    return kern_result;
}


/*****************************************************************************/
CFDictionaryRef IOPMCopyAssertionActivityAggregateWithAllocator(CFAllocatorRef allocator)
{
    IOReturn                rc = kIOReturnInternalError;
    CFDataRef               unfolder = NULL;
    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result;
    vm_offset_t             addr = NULL;
    mach_msg_type_number_t  size = 0;
    CFDictionaryRef  statsData = NULL;


    _pm_connect(&pm_server);

    if(pm_server == MACH_PORT_NULL)
        return NULL;

    kern_result = io_pm_assertion_activity_aggregate(pm_server,
                                                     &addr,  &size,
                                                     &rc);

    if ((KERN_SUCCESS != kern_result) || (rc != kIOReturnSuccess)) {
        goto exit;
    }

    unfolder = CFDataCreateWithBytesNoCopy(0, (const UInt8 *)addr, (CFIndex)size, kCFAllocatorNull);
    if (unfolder)
    {
        statsData = CFPropertyListCreateWithData(allocator, unfolder,
                                                 kCFPropertyListMutableContainers, NULL, NULL);
        CFRelease(unfolder);
    }

exit:

    if (addr && size)
        vm_deallocate(mach_task_self(), addr, size);


    if (MACH_PORT_NULL != pm_server)
        pm_connect_close(pm_server);

    return statsData;
}

CFDictionaryRef IOPMCopyAssertionActivityAggregate( )
{
    return IOPMCopyAssertionActivityAggregateWithAllocator(kCFAllocatorDefault);
}
/*****************************************************************************/
void IOPMAssertionSetBTCollection(bool enable)
{
    int collectBackTraceToken = 0;

    notify_register_check(kIOPMAssertionsCollectBTString, &collectBackTraceToken);
    notify_set_state(collectBackTraceToken, enable ? 1 : 0);
    notify_post(kIOPMAssertionsCollectBTString);
    notify_cancel(collectBackTraceToken);
}

/*****************************************************************************/
IOReturn IOPMSetAssertionExceptionLimits(CFDictionaryRef procLimits)
{
    IOReturn                return_code     = kIOReturnError;
    kern_return_t           kern_result     = KERN_SUCCESS;
    mach_port_t             pm_server       = MACH_PORT_NULL;
    IOReturn                err;
    CFDataRef               flattenedData  = NULL;

    if (!isA_CFDictionary(procLimits)) {
        return kIOReturnBadArgument;
    }

    err = pm_connect_init(&pm_server);
    if(kIOReturnSuccess != err) {
        return_code = kIOReturnInternalError;
        goto exit;
    }

    flattenedData = CFPropertyListCreateData(0, procLimits, kCFPropertyListBinaryFormat_v1_0, 0, NULL);
    if (!flattenedData) {
        return_code = kIOReturnBadArgument;
        goto exit;
    }

    kern_result = io_pm_set_exception_limits(pm_server,
                                             (vm_offset_t)CFDataGetBytePtr(flattenedData),
                                             CFDataGetLength(flattenedData),
                                             &return_code);
    if(KERN_SUCCESS != kern_result) {
        return_code = kIOReturnInternalError;
        goto exit;
    }

exit:
    if (flattenedData) {
        CFRelease(flattenedData);
    }
    if (MACH_PORT_NULL != pm_server) {
        pm_connect_close(pm_server);
    }
    return return_code;
}
/*****************************************************************************/
typedef struct {
    int     token;
} _IOPMAssertionExceptionHandle;

IOPMNotificationHandle IOPMScheduleAssertionExceptionNotification(dispatch_queue_t queue,
                                                                  void (^excHandler)(IOPMAssertionException, pid_t))
{
    _IOPMAssertionExceptionHandle *hdl = NULL;
    uint32_t status;

    hdl = calloc(1, sizeof(_IOPMAssertionExceptionHandle));
    if (!hdl) {
        return NULL;
    }

    notify_handler_t calloutBlock = ^(int token) {
        uint32_t sts;
        uint64_t data;
        IOPMAssertionException exc;
        pid_t pid;

        sts = notify_get_state(token, &data);
        if (sts == NOTIFY_STATUS_OK) {
            pid = data & 0xffffffff;
            exc = (data >> 32);

            excHandler(exc, pid);
        }
    };
    status = notify_register_dispatch(kIOPMAssertionExceptionNotifyName,
                                      &hdl->token, queue, calloutBlock);

    if (status != NOTIFY_STATUS_OK) {
        free(hdl);
        hdl = NULL;
    }

    return hdl;
}

void IOPMUnregisterExceptionNotification(IOPMNotificationHandle handle)
{
    _IOPMAssertionExceptionHandle *excHandle = (_IOPMAssertionExceptionHandle *)handle;
    if (excHandle) {
        if (excHandle->token) {
            notify_cancel(excHandle->token);
        }
        bzero(excHandle, sizeof(*excHandle));
        free(excHandle);
    }
}
