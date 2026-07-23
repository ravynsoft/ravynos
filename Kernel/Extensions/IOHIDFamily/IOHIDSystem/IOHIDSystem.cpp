/*
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

#define DEBUG_ASSERT_COMPONENT_NAME_STRING "IOHIDSystem"
//#define DEBUG_ASSERT_PRODUCTION_CODE 0


#include <AssertMacros.h>
#include "IOHIDKeys.h"

#include "IOHIDEvent.h"
#include <IOKit/system.h>
#include <IOKit/assert.h>
#include <IOKit/IOReporter.h>

#include <libkern/c++/OSContainers.h>
#include <libkern/c++/OSCollectionIterator.h>

#include <kern/queue.h>

#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/hidsystem/IOHIDevice.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <IOKit/pwr_mgt/RootDomain.h>
#include <kern/clock.h>
#include "IOHIDShared.h"
#include "IOHIDSystem.h"
//#include "IOHIDEventService.h"
//#include "IOHIDPointing.h"
//#include "IOHIDKeyboard.h"
//#include "IOHIDConsumer.h"
#include "IOHIDevicePrivateKeys.h"
#include "IOHITablet.h"
#include "IOHIDPointingDevice.h"
#include "IOHIDKeyboardDevice.h"
#include "IOHIDPrivate.h"
#include "IOHIDPrivateKeys.h"
//#include "IOHIDEventServiceQueue.h"
#include "IOLLEvent.h"
#include "IOHIDPointingEventDevice.h"
#include "IOHIDKeyboardEventDevice.h"

#include "ev_private.h"
#include "IOHIDUserClient.h"
#include "AppleHIDUsageTables.h"
#include "IOHIDKeyboard.h"
#include "IOHIDFamilyTrace.h"
#include "IOHIDWorkLoop.h"
#include "IOHIDSystemCursorHelper.h"
#include "IOHIDDebug.h"

#include <sys/kdebug.h>
#include <sys/proc.h>
#include <string.h>
#include <libkern/libkern.h>

#ifdef __cplusplus
    extern "C"
    {
        #include <UserNotification/KUNCUserNotifications.h>
        void cons_cinput( char c);
    }
#endif

extern "C" {
    #include <sys/kauth.h>

    #define CONFIG_MACF 1
    #include <security/mac_framework.h>
    #undef CONFIG_MACF
};

const char * IOHIDSystem::Diags::cursorStrings[] = {
    "ShowCursor",
    "HideCursor",
    "MoveCursor"
};

bool displayWranglerUp( OSObject *, void *, IOService * );

#if 0
#define PROFILE_TRACE(X)     IOHID_DEBUG(kIOHIDDebugCode_Profiling, X, __LINE__, 0, 0)
#else
#define PROFILE_TRACE(X)
#endif

static IOHIDSystem * evInstance = 0;
MasterAudioFunctions *masterAudioFunctions = 0;

#define xpr_ev_cursor(x, a, b, c, d, e)

#ifndef kIOFBWaitCursorFramesKey
#define kIOFBWaitCursorFramesKey    "IOFBWaitCursorFrames"
#endif
#ifndef kIOFBWaitCursorPeriodKey
#define kIOFBWaitCursorPeriodKey    "IOFBWaitCursorPeriod"
#endif


#ifndef kIOUserClientCrossEndianCompatibleKey
#define kIOUserClientCrossEndianCompatibleKey "IOUserClientCrossEndianCompatible"
#endif

#ifndef abs
#define abs(_a) ((_a >= 0) ? _a : -_a)
#endif

#define NORMAL_MODIFIER_MASK (NX_COMMANDMASK | NX_CONTROLMASK | NX_SHIFTMASK | NX_ALTERNATEMASK)

#define POINTING_EVENT_MASK (NX_LMOUSEDOWNMASK | \
    NX_LMOUSEDOWNMASK |\
    NX_LMOUSEUPMASK |\
    NX_RMOUSEDOWNMASK |\
    NX_RMOUSEUPMASK |\
    NX_MOUSEMOVEDMASK |\
    NX_LMOUSEDRAGGEDMASK |\
    NX_RMOUSEDRAGGEDMASK |\
    NX_TABLETPOINTERMASK |\
    NX_TABLETPROXIMITY |\
    NX_MOUSEENTEREDMASK |\
    NX_MOUSEEXITEDMASK |\
    NX_SCROLLWHEELMOVEDMASK)

#define KEYBOARD_EVENT_MASK (NX_KEYDOWNMASK | NX_FLAGSCHANGEDMASK)

#define EV_MAX_SCREENS 64

#define IDLE_HID_ACTIVITY_NSECS ((uint64_t)(5*60*NSEC_PER_SEC))

static UInt8 ScalePressure(unsigned pressure)
{
    return ((pressure * (unsigned long long)EV_MAXPRESSURE) / (unsigned)(65535LL));
}

#define EV_NS_TO_TICK(ns)   AbsoluteTimeToTick(ns)
#define EV_TICK_TO_NS(tick,ns)  TickToAbsoluteTime(tick,ns)


/* COMMAND GATE COMPATIBILITY TYPE DEFS */
typedef struct _IOHIDCmdGateActionArgs {
    void*   arg0;
    void*   arg1;
    void*   arg2;
    void*   arg3;
    void*   arg4;
    void*   arg5;
    void*   arg6;
    void*   arg7;
    void*   arg8;
    void*   arg9;
    void*   arg10;
    void*   arg11;
} IOHIDCmdGateActionArgs;
/* END COMMAND GATE COMPATIBILITY TYPE DEFS */


/* END HID SYSTEM EVENT LOCK OUT SUPPORT */

// RY: Consume any keyboard events that come in before the
// deadline after the system wakes up or if the keySwitch is locked
#define kHIDConsumeCauseNone    0x00
#define kHIDConsumeCauseKeyLock 0x01
#define kHIDConsumeCauseDeadline 0x02

#define WAKE_DISPLAY_ON_MOVEMENT (NX_WAKEMASK & MOVEDEVENTMASK)

#define DISPLAY_IS_ENABLED (displayState & IOPMDeviceUsable)

#ifdef NEW_HID
#define TICKLE_DISPLAY(event) \
{ \
    if (!evStateChanging && displayManager) { \
        IOHID_DEBUG(kIOHIDDebugCode_DisplayTickle, event, __LINE__, 0, 0); \
        if (!DISPLAY_IS_ENABLED) { \
            kprintf("IOHIDSystem tickle when screen off for event %d at line %d\n", (int)event, __LINE__); \
            HIDLogInfo("IOHIDSystem tickle when screen off for event %d", (int)event); \
        } \
        displayManager->activityTickle(IOHID_DEBUG_CODE(event)); \
    } \
    updateHidActivity(); \
}
#else
#define TICKLE_DISPLAY(x) /**/
#endif

enum {
    // Options for IOHIDPostEvent()
    kIOHIDSetGlobalEventFlags       = 0x00000001,
    kIOHIDSetCursorPosition         = 0x00000002,
    kIOHIDSetRelativeCursorPosition = 0x00000004,
    kIOHIDPostHIDManagerEvent       = 0x00000008
};

#define kIOHIDPowerOnThresholdNS            (500ULL * kMillisecondScale)    // 1/2 second
#define kIOHIDRelativeTickleThresholdNS     (50ULL * kMillisecondScale)     // 1/20 second
#define kIOHIDRelativeTickleThresholdPixel  3
#define kIOHIDDispaySleepAbortThresholdNS   (5ULL * kSecondScale)           // 5 seconds
#define kIOHIDChattyMouseSuppressionDelayNS kSecondScale                    // 1 second
#define kIOHIDSystenDistantFuture           INT64_MAX

static AbsoluteTime gIOHIDPowerOnThresoldAbsoluteTime;
static AbsoluteTime gIOHIDRelativeTickleThresholdAbsoluteTime;
// can be aborted by a mouse/scroll motion
static AbsoluteTime gIOHIDDisplaySleepAbortThresholdAbsoluteTime;
static AbsoluteTime gIOHIDZeroAbsoluteTime;


//************************************************************
// keyboardEventQueue support
//************************************************************
static queue_head_t     gKeyboardEQ;
static IOLock *         gKeyboardEQLock = 0;

typedef IOReturn (*KeyboardEQAction)(IOHIDSystem * self, void *args);

typedef struct _KeyboardEQElement {
    queue_chain_t   link;

    KeyboardEQAction    action;
    OSObject *          sender;
    AbsoluteTime        ts;

    union {
        struct {
            unsigned        eventType;
            unsigned        flags;
            unsigned        key;
            unsigned        charCode;
            unsigned        charSet;
            unsigned        origCharCode;
            unsigned        origCharSet;
            unsigned        keyboardType;
            bool            repeat;
        } keyboard;
        struct {
            unsigned        eventType;
            unsigned        flags;
            unsigned        key;
            unsigned        flavor;
            UInt64          guid;
            bool            repeat;
        } keyboardSpecial;
        struct {
            unsigned        flags;
        } flagsChanged;
    } event;
} KeyboardEQElement;

#define KEYBOARD_EQ_LOCK    if (gKeyboardEQLock) IOLockLock(gKeyboardEQLock);
#define KEYBOARD_EQ_UNLOCK  if (gKeyboardEQLock) IOLockUnlock(gKeyboardEQLock);

/* This function exists because IOHIDSystem uses OSArrays of OSNumbers. Since
 * OSArray::containsObject() compares by pointer equality instead of
 * OSObject::isEqualTo().  OSNumbers cannot be found by value with
 * containsObject(), so we must use this instead.
 */
#define kObjectNotFound ((unsigned int) -1)
//static unsigned int
//getArrayIndexForObject(OSArray *array, OSObject* object)
//{
//    OSObject *tmp;
//    u_int i;
//
//    for (i = 0; i < array->getCount(); ++i) {
//        tmp = array->getObject(i);
//        if (tmp->isEqualTo(object)) {
//            return i;
//        }
//    }
//
//    return kObjectNotFound;
//}

static void
hidActivityThread_cb(thread_call_param_t us, thread_call_param_t )
{
    ((IOHIDSystem *)us)->hidActivityChecker();
}

typedef struct {
    IOCommandGate::Action   handler;
    IOService               *newService;
}
IOHIDSystem_notificationData;

#define super IOService
OSDefineMetaClassAndStructors(IOHIDSystem, IOService);

struct IOHIDSystem::ExpansionData
{
    // The goal of the cursorHelper is to isolate all of the cursor
    // calculations and updates into one class.
    IOHIDSystemCursorHelper cursorHelper;
    UInt64                  periodicEventLast;
    UInt64                  periodicEventNext;
    UInt64                  cursorEventLast;
    UInt64                  cursorMoveLast;
    UInt64                  cursorMoveDelta;
    UInt64                  cursorWaitLast;
    UInt64                  cursorWaitDelta;

    IONotifier *            displayWranglerMatching;
    IONotifier *            graphicsDeviceMatching;

    bool                    continuousCursor;
    bool                    hidActivityIdle; // Is HID activity idle for more than IDLE_HID_ACTIVITY_NSECS ?
    bool                    forceIdle;
    AbsoluteTime            lastTickleTime;
    thread_call_t           hidActivityThread;

    // async delayed notifications
    IOLock                  *delayedNotificationLock;
    OSArray                 *delayedNotificationArray;
    thread_call_t           delayedNotificationThread;
    
    UInt32                  onScreenPinMask;
    IOGBounds               onScreenBounds[EV_MAX_SCREENS];
    
};

#define _cursorHelper               (_privateData->cursorHelper)
#define _periodicEventLast          (_privateData->periodicEventLast)
#define _periodicEventNext          (_privateData->periodicEventNext)
#define _cursorEventLast            (_privateData->cursorEventLast)
#define _cursorMoveLast             (_privateData->cursorMoveLast)
#define _cursorMoveDelta            (_privateData->cursorMoveDelta)
#define _cursorWaitLast             (_privateData->cursorWaitLast)
#define _cursorWaitDelta            (_privateData->cursorWaitDelta)

#define _displayWranglerMatching    (_privateData->displayWranglerMatching)
#define _graphicsDeviceMatching     (_privateData->graphicsDeviceMatching)

#define _continuousCursor            (_privateData->continuousCursor)
#define _hidActivityIdle            (_privateData->hidActivityIdle)
#define _lastTickleTime             (_privateData->lastTickleTime)
#define _forceIdle                  (_privateData->forceIdle)
#define _hidActivityThread          (_privateData->hidActivityThread)

#define _delayedNotificationLock    (_privateData->delayedNotificationLock)
#define _delayedNotificationArray   (_privateData->delayedNotificationArray)
#define _delayedNotificationThread  (_privateData->delayedNotificationThread)


#define _onScreenBounds             (_privateData->onScreenBounds)
#define _onScreenPinMask            (_privateData->onScreenPinMask)


enum {
    kScrollDirectionInvalid = 0,
    kScrollDirectionXPositive,
    kScrollDirectionXNegative,
    kScrollDirectionYPositive,
    kScrollDirectionYNegative,
    kScrollDirectionZPositive,
    kScrollDirectionZNegative,
};

#define _cursorLog(ts)      do { \
                                if (evg != 0) \
                                    if (evg->logCursorUpdates) { \
                                        _cursorHelper.logPosition(__func__, ts); \
                                    } \
                            } \
                            while(false)

#define _cursorLogTimed()   do { \
                                if (evg != 0) \
                                    if (evg->logCursorUpdates) { \
                                        AbsoluteTime ts; \
                                        clock_get_uptime(&ts); \
                                        _cursorHelper.logPosition(__func__, AbsoluteTime_to_scalar(&ts)); \
                                    } \
                            } \
                            while(false)

/* Return the current instance of the EventDriver, or 0 if none. */
IOHIDSystem * IOHIDSystem::instance()
{
  return evInstance;
}

#define kDefaultMinimumDelta    0x1ffffffffULL

bool IOHIDSystem::init(OSDictionary * properties)
{
    IOByteCount size;
    
    if (!super::init(properties))
        return false;

    _privateData = (ExpansionData*)IOMalloc(sizeof(ExpansionData));
    if (!_privateData)
        return false;
    bzero(_privateData, sizeof(ExpansionData));
    _periodicEventNext = kIOHIDSystenDistantFuture;

    if (!_cursorHelper.init())
        return false;
    _cursorLogTimed();


    /*
     * Initialize minimal state.
     */
    evScreen          = NULL;
    periodicES        = 0;
    keyboardEQES      = 0;
    cmdGate           = 0;
    workLoop          = 0;
    displayState = IOPMDeviceUsable;
    AbsoluteTime_to_scalar(&displayStateChangeDeadline) = 0;
    AbsoluteTime_to_scalar(&displaySleepWakeupDeadline) = 0;
    AbsoluteTime_to_scalar(&gIOHIDZeroAbsoluteTime) = 0;
    powerState        = 0;

    nanoseconds_to_absolutetime(kIOHIDPowerOnThresholdNS, &gIOHIDPowerOnThresoldAbsoluteTime);
    nanoseconds_to_absolutetime(kIOHIDDispaySleepAbortThresholdNS, &gIOHIDDisplaySleepAbortThresholdAbsoluteTime);

    queue_init(&gKeyboardEQ);
    gKeyboardEQLock = IOLockAlloc();
    
    size = sizeof(EvOffsets) + sizeof(EvGlobals);
    globalMemory = IOBufferMemoryDescriptor::withOptions( kIODirectionNone | kIOMemoryKernelUserShared, size );
    
    if (!globalMemory)
        return false;
    
    shmem_addr = (uintptr_t) globalMemory->getBytesNoCopy();
    shmem_size = size;
    
    initShmem(true);
    
    return true;
}

IOHIDSystem * IOHIDSystem::probe(IOService *    provider,
                 SInt32 *   score)
{
  if (!super::probe(provider,score))  return 0;

  return this;
}

/*
 * Perform reusable initialization actions here.
 */
IOWorkLoop * IOHIDSystem::getWorkLoop() const
{
    return workLoop;
}


bool IOHIDSystem::start(IOService * provider)
{
    bool            iWasStarted = false;
    OSObject        *obj = NULL;
    OSNumber        *number = NULL;
    OSDictionary    *matchingDevice = serviceMatching("IOHIDevice");
    OSDictionary    *matchingWrangler = serviceMatching("IODisplayWrangler");
    OSDictionary    *matchingGraphicsDevice = serviceMatching("IOGraphicsDevice");
    IOServiceMatchingNotificationHandler iohidNotificationHandler = OSMemberFunctionCast(IOServiceMatchingNotificationHandler, this, &IOHIDSystem::genericNotificationHandler);
    
    require(super::start(provider), exit_early);

    _setScrollCountParameters();
    
    evInstance = this;
    
    /* A few details to be set up... */
    _cursorHelper.desktopLocation().fromIntFloor(INIT_CURSOR_X, INIT_CURSOR_Y);
    _cursorHelper.desktopLocationDelta().fromIntFloor(0, 0);
    _cursorHelper.updateScreenLocation(NULL, NULL);
    _cursorLogTimed();
    _delayedNotificationLock = IOLockAlloc();
    _delayedNotificationArray = OSArray::withCapacity(2);
    
    evScreenSize = sizeof(EvScreen) * EV_MAX_SCREENS;
    evScreen = (void *) IOMalloc(evScreenSize);
    savedParameters = OSDictionary::withCapacity(4);
    
    require(evScreen && savedParameters && _delayedNotificationLock && _delayedNotificationArray, exit_early);
    
    bzero(evScreen, evScreenSize);
    firstWaitCursorFrame = EV_WAITCURSOR;
    maxWaitCursorFrame   = EV_MAXCURSOR;
    createParameters();
    
    
    // Let's go ahead and cache our registry name.
    // This was added to remove a call to getName while
    // we are disabling preemption
    obj = copyProperty(kIOHIDPowerOnDelayNSKey, gIOServicePlane);
    if (obj != NULL) {
        number = OSDynamicCast(OSNumber, obj);
        if (number != NULL) {
            UInt64 value = number->unsigned64BitValue();
            if (value < kMillisecondScale) {
                // logging not yet available
            }
            else if (value > (10ULL * kSecondScale)) {
                // logging not yet available
            }
            else {
                setProperty(kIOHIDPowerOnDelayNSKey, number);
                gIOHIDPowerOnThresoldAbsoluteTime = value;
            }
        }
        obj->release();
    }

    /*
     * Start up the work loop
     */
    workLoop = IOHIDWorkLoop::workLoop();
    cmdGate = IOCommandGate::commandGate(this);
    periodicES = IOTimerEventSource::timerEventSource(this, (IOTimerEventSource::Action) &_periodicEvents);
    keyboardEQES = IOInterruptEventSource::interruptEventSource(this, (IOInterruptEventSource::Action) &doProcessKeyboardEQ);
    
    //require(workLoop && cmdGate && periodicES && eventConsumerES && keyboardEQES, exit_early);
    require(workLoop && cmdGate && periodicES && keyboardEQES, exit_early);
    
    require_noerr(workLoop->addEventSource(cmdGate), exit_early);
    require_noerr(workLoop->addEventSource(periodicES), exit_early);
    require_noerr(workLoop->addEventSource(keyboardEQES), exit_early);
  
    _delayedNotificationThread = thread_call_allocate( OSMemberFunctionCast(thread_call_func_t, this, &IOHIDSystem::doProcessNotifications), this);
    require(_delayedNotificationThread, exit_early);
    
    publishNotify = addMatchingNotification(gIOPublishNotification,
                                            matchingDevice,
                                            iohidNotificationHandler,
                                            this,
                                            (void *)&IOHIDSystem::handlePublishNotification );
    require(publishNotify, exit_early);
    
    // RY: Listen to the root domain
    rootDomain = (IOService *)getPMRootDomain();
    
    if (rootDomain)
        rootDomain->registerInterestedDriver(this);
    
    registerPrioritySleepWakeInterest(powerStateHandler, this, 0);
        
    _displayWranglerMatching = addMatchingNotification(gIOPublishNotification,
                                                       matchingWrangler,
                                                       iohidNotificationHandler,
                                                       this,
                                                       (void *)&IOHIDSystem::handlePublishNotification);
    require(_displayWranglerMatching, exit_early);
    
    _graphicsDeviceMatching = addMatchingNotification(gIOTerminatedNotification,
                                                      matchingGraphicsDevice,
                                                      iohidNotificationHandler,
                                                      this,
                                                      (void *)&IOHIDSystem::handleTerminationNotification);
    require(_graphicsDeviceMatching, exit_early);

    {
        IOReturn ret;
        IOHistogramSegmentConfig configs[] = {
            {   // Segment for expected latencies, 0us to 1,000us (1ms)
                .base_bucket_width = 100,
                .scale_flag = 0,
                .segment_idx = 0,
                .segment_bucket_count = 10

            },
            {   // Segment for high latencies, 1,000us to 100,000us (100ms)
                .base_bucket_width = 20000,
                .scale_flag = 0,
                .segment_idx = 1,
                .segment_bucket_count = 5
            }
        };

        _diags.cursorTotalHistReporter = IOHistogramReporter::with(this,
                                                                   kIOReportCategoryPeripheral | kIOReportCategoryPerformance,
                                                                   IOREPORT_MAKEID('C','u','r','s','o','r','T','o'),
                                                                   "Cursor Total Latency",
                                                                   kIOReportUnit_us,
                                                                   sizeof(configs)/sizeof(configs[0]),
                                                                   configs);

        if (_diags.cursorTotalHistReporter) {
            ret = IOReportLegend::addReporterLegend(this, _diags.cursorTotalHistReporter, "Cursor", "Total");
            require(ret == kIOReturnSuccess, exit_early);
        }

        _diags.cursorGraphicsHistReporter = IOHistogramReporter::with(this,
                                                              kIOReportCategoryPeripheral | kIOReportCategoryPerformance,
                                                              IOREPORT_MAKEID('C','u','r','s','o','r','G','r'),
                                                              "Cursor Graphics Latency",
                                                              kIOReportUnit_us,
                                                              sizeof(configs)/sizeof(configs[0]),
                                                              configs);

        if (_diags.cursorGraphicsHistReporter) {
            ret = IOReportLegend::addReporterLegend(this, _diags.cursorGraphicsHistReporter, "Cursor", "Graphics");
            require(ret == kIOReturnSuccess, exit_early);
        }
    }
    
    /*
     * IOHIDSystem serves both as a service and a nub (we lead a double
     * life).  Register ourselves as a nub to kick off matching.
     */
    
#if TARGET_OS_OSX
    _hidActivityThread = thread_call_allocate(hidActivityThread_cb, (thread_call_param_t)this);
    _hidActivityIdle = true;
    require(_hidActivityThread, exit_early);
#endif

    registerService();
    iWasStarted = true;
    IOLog("IOHIDSystem started\n");

exit_early:
    OSSafeReleaseNULL(matchingDevice);
    OSSafeReleaseNULL(matchingWrangler);
    OSSafeReleaseNULL(matchingGraphicsDevice);
    
    if (!iWasStarted)
        evInstance = 0;
    
    return iWasStarted;
}

void IOHIDSystem::updatePowerState(UInt32 messageType)
{
    powerState = messageType;
}

IOReturn IOHIDSystem::powerStateHandler( void *target, void *refCon __unused,
                        UInt32 messageType, IOService *service __unused, void *messageArgs __unused, vm_size_t argSize __unused)
{
    IOHIDSystem*  myThis = OSDynamicCast( IOHIDSystem, (OSObject*)target );
    
    if (messageType != kIOMessageSystemCapabilityChange) {
        myThis->updatePowerState(messageType);
    }
    
    return kIOReturnSuccess;
}

// powerStateDidChangeTo
//
// The display wrangler has changed state, so the displays have changed
// state, too.  We save the new state.

IOReturn IOHIDSystem::powerStateDidChangeTo( IOPMPowerFlags theFlags, unsigned long state, IOService * service)
{
    IOHID_DEBUG(kIOHIDDebugCode_PowerStateChangeEvent, service, state, theFlags, 0);
    if (service == displayManager)
    {
        displayState = theFlags;
        if (theFlags & IOPMDeviceUsable) {
            clock_get_uptime(&displayStateChangeDeadline);
            ADD_ABSOLUTETIME(&displayStateChangeDeadline,
                             &gIOHIDRelativeTickleThresholdAbsoluteTime);
            AbsoluteTime_to_scalar(&displaySleepWakeupDeadline) = 0;

           // Reset all flags. Flag key release events may not get delivered as they
           // come late in system sleep or display sleep process.
           updateEventFlags(0);
        }
        else {
             // If the display has transitioned from usable to unusable state
             // because of display sleep  then set the deadline before which
             // pointer movement can bring the display to usable state.
             // Also make sure that this display sleep is not driven by IOPM as part of system sleep

           if ( !CMP_ABSOLUTETIME(&displaySleepWakeupDeadline, &gIOHIDZeroAbsoluteTime) && 
                   (kOSBooleanFalse == rootDomain->copyProperty("DisplayIdleForDemandSleep"))) {
             clock_get_uptime(&displaySleepWakeupDeadline);
             ADD_ABSOLUTETIME(&displaySleepWakeupDeadline, &gIOHIDDisplaySleepAbortThresholdAbsoluteTime);
           }
           //displaySleepDrivenByPM = false; // Reset flag for next use

           // Force set HID activity to idle
           _lastTickleTime = 0;
           _forceIdle = true;
           thread_call_enter(_hidActivityThread);
        }

    }
    return IOPMNoErr;
}

bool IOHIDSystem::genericNotificationHandler(void * handler,
                                             IOService * newService,
                                             IONotifier * /* notifier */)
{
    bool result = false;

    if (handler && newService) {
        IOHIDSystem_notificationData    rawData = {(IOCommandGate::Action)handler, newService};
        OSData                          *data = OSData::withBytes(&rawData, sizeof(rawData));

        if (data) {
            newService->retain();
            IOLockLock(_delayedNotificationLock);
            _delayedNotificationArray->setObject(data);
            IOLockUnlock(_delayedNotificationLock);
            data->release();

            thread_call_enter (_delayedNotificationThread);

            result = true;
        }
    }

    return result;
}

void IOHIDSystem::doProcessNotifications() {
    while (_delayedNotificationArray->getCount() > 0) {
        // retrieve the first item from the queue
        IOLockLock(_delayedNotificationLock);
        OSData *notificationData = OSDynamicCast(OSData, _delayedNotificationArray->getObject(0));
        if (notificationData) {
            notificationData->retain();
        }
        _delayedNotificationArray->removeObject(0);
        IOLockUnlock(_delayedNotificationLock);
        
        // process the notification
        if (notificationData) {
            const IOHIDSystem_notificationData *data = (const IOHIDSystem_notificationData *)notificationData->getBytesNoCopy();
            data->handler(this, data->newService, NULL, NULL, NULL);
            data->newService->release();
            notificationData->release();
        }
    }
}



bool IOHIDSystem::handlePublishNotification(
            void * target,
            IOService * newService )
{
    IOHIDSystem * self = (IOHIDSystem *) target;

    if (!self || !newService || newService->isInactive()) {
        // device went away before we could add it. ignore.
        return true;
    }
    
    // avoiding OSDynamicCast & dependency on graphics family
    if( newService->metaCast("IODisplayWrangler")) {
        if( !self->displayManager) {
            self->displayState = newService->registerInterestedDriver(self);
            self->displayManager = newService;
        }
        return true;
    }
#ifdef NEW_HID
    if(OSDynamicCast(IOHIPointing, newService) && !OSDynamicCast(IOHIDPointing, newService)) {
      IOHIDPointingEventDevice * shim = IOHIDPointingEventDevice::newPointingDeviceAndStart(newService);
      if (shim) {
        shim->release();
      }
    }
    if(OSDynamicCast(IOHIKeyboard, newService) && !OSDynamicCast(IOHIDKeyboard, newService) && !OSDynamicCast(IOHIDConsumer, newService)) {
      IOHIDKeyboardEventDevice * shim = IOHIDKeyboardEventDevice::newKeyboardDeviceAndStart(newService);
      if (shim) {
        shim->release();
      }
    }
#endif
  
    
    if (self->attach( newService ) == false) {
        return true;
    }
  
    self->registerEventSource( newService );
    return true;
}

bool IOHIDSystem::handleTerminationNotification(void *target, IOService *newService)
{
    bool                result          = false;
    IOHIDSystem         *self           = (IOHIDSystem *)target;
    IOGraphicsDevice    *graphicsDevice = (IOGraphicsDevice *)newService->metaCast("IOGraphicsDevice");
    
    require(self, exit);
    
    if (graphicsDevice) {
        for (int i = 0; i < EV_MAX_SCREENS; i++) {
            EvScreen *screen_ptr = &((EvScreen*)self->evScreen)[i];
            if (screen_ptr->instance == graphicsDevice) {
                self->unregisterScreen(i+SCREENTOKEN);
                break;
            }
        }
    }
    
    result = true;
    
exit:
    return result;
}


/*
 * Free locally allocated resources, and then ourselves.
 */
void IOHIDSystem::free()
{
    if (cmdGate) {
        evClose();
    }

    // we are going away. stop the workloop.
    if (workLoop) {
        workLoop->disableAllEventSources();
    }

    if (periodicES) {
        periodicES->cancelTimeout();
        
        if ( workLoop )
            workLoop->removeEventSource( periodicES );
        
        periodicES->release();
        periodicES = 0;
    }

    if (keyboardEQES) {
        keyboardEQES->disable();
        
        if ( workLoop )
            workLoop->removeEventSource( keyboardEQES );
        keyboardEQES->release();
        keyboardEQES = 0;
    }
    
    if (evScreen) IOFree( (void *)evScreen, evScreenSize );
    evScreen = (void *)0;
    evScreenSize = 0;

    if (publishNotify) {
        publishNotify->remove();
        publishNotify = 0;
    }

    if (_privateData) {
      
        if (_displayWranglerMatching) {
            _displayWranglerMatching->remove();
            _displayWranglerMatching = 0;
        }
        
        if (_graphicsDeviceMatching) {
            _graphicsDeviceMatching->remove();
            _graphicsDeviceMatching = 0;
        }

        if (_delayedNotificationThread) {
            thread_call_cancel_wait(_delayedNotificationThread);
            thread_call_free (_delayedNotificationThread);
        }

        if (_delayedNotificationLock) {
            IOLockFree(_delayedNotificationLock);
            _delayedNotificationLock = 0;
        }

        OSSafeReleaseNULL(_delayedNotificationArray);
    }
    OSSafeReleaseNULL(cmdGate); // gate is already closed
    OSSafeReleaseNULL(workLoop);

    if ( gKeyboardEQLock ) {
        IOLock * lock = gKeyboardEQLock;
        IOLockLock(lock);
        gKeyboardEQLock = 0;
        IOLockUnlock(lock);
        IOLockFree(lock);
    }

    OSSafeReleaseNULL(_hidKeyboardDevice);
    OSSafeReleaseNULL(_hidPointingDevice);
    
    if (_privateData) {
        _cursorHelper.finalize();
        IOFree((void*)_privateData, sizeof(ExpansionData));
        _privateData = NULL;
    }
    
    OSSafeReleaseNULL(globalMemory);

    OSSafeReleaseNULL(_diags.cursorTotalHistReporter);
    OSSafeReleaseNULL(_diags.cursorGraphicsHistReporter);
    
    super::free();
}


IOReturn IOHIDSystem::configureReport(IOReportChannelList *channels,
                                      IOReportConfigureAction action,
                                      void *result,
                                      void *destination)
{
    IOReturn ret;

    if (_diags.cursorTotalHistReporter) {
        ret = _diags.cursorTotalHistReporter->configureReport(channels, action, result, destination);
        require(ret == kIOReturnSuccess, exit);
    }

    if (_diags.cursorGraphicsHistReporter) {
        ret = _diags.cursorGraphicsHistReporter->configureReport(channels, action, result, destination);
        require(ret == kIOReturnSuccess, exit);
    }

    ret = super::configureReport(channels, action, result, destination);
    require(ret == kIOReturnSuccess, exit);

exit:
    return ret;
}

IOReturn IOHIDSystem::updateReport(IOReportChannelList *channels,
                                   IOReportConfigureAction action,
                                   void *result,
                                   void *destination)
{
    IOReturn ret;

    if (_diags.cursorTotalHistReporter) {
        ret = _diags.cursorTotalHistReporter->updateReport(channels, action, result, destination);
        require(ret == kIOReturnSuccess, exit);
    }

    if (_diags.cursorGraphicsHistReporter) {
        ret = _diags.cursorGraphicsHistReporter->updateReport(channels, action, result, destination);
        require(ret == kIOReturnSuccess, exit);
    }

    ret = super::updateReport(channels, action, result, destination);
    require(ret == kIOReturnSuccess, exit);

exit:
    return ret;
}



/*
 * Open the driver for business.  This call must be made before
 * any other calls to the Event driver.  We can only be opened by
 * one user at a time.
 */
IOReturn IOHIDSystem::evOpen(void)
{
    IOReturn r = kIOReturnSuccess;

    if ( evOpenCalled == true )
    {
        r = kIOReturnBusy;
        goto done;
    }
    evOpenCalled = true;

    if (!evInitialized)
    {
        evInitialized = true;
        // Put code here that is to run on the first open ONLY.
    }

done:
    return r;
}

IOReturn IOHIDSystem::evClose(void){
    return cmdGate->runAction((IOCommandGate::Action)doEvClose);
}

IOReturn IOHIDSystem::doEvClose(IOHIDSystem *self)
                        /* IOCommandGate::Action */
{
    return self->evCloseGated();
}

IOReturn IOHIDSystem::evCloseGated(void)
{
    if ( evOpenCalled == false )
        return kIOReturnBadArgument;

    evStateChanging = true;

    // Early close actions here
    if( cursorEnabled)
        hideCursor();
    cursorStarted = false;
    cursorEnabled = false;

    // Clear screens registry and related data
    if ( evScreen != (void *)0 )
    {
        screens = 0;
    }
    // Remove port notification for the eventPort and clear the port out
    //setEventPortGated(MACH_PORT_NULL);

    // Clear local state to shutdown
    evStateChanging = false;
    evOpenCalled = false;
    eventsOpen = false;

    return kIOReturnSuccess;
}

//
// Dispatch state to screens registered with the Event Driver
// Pending state changes for a device may be coalesced.
//
//
// This should be run from a command gate action.
//
void IOHIDSystem::evDispatch(
               /* command */ EvCmd evcmd)
{
    if( !eventsOpen || (evcmd == EVLEVEL) || (evcmd == EVNOP))
        return;

    for( int i = 0; i < EV_MAX_SCREENS; i++ ) {
        bool onscreen = (0 != (cursorScreens & (1 << i)));

        if (onscreen) {
            EvScreen *esp = &((EvScreen*)evScreen)[i];

            if ( esp->instance && !esp->instance->isInactive() ) {
                IOGPoint p;
                p.x = evg->screenCursorFixed.x / 256;   // Copy from shmem.
                p.y = evg->screenCursorFixed.y / 256;
                
                switch ( evcmd )
                {
                    case EVMOVE:
                        esp->instance->moveCursor(&p, evg->frame);
                        break;

                    case EVSHOW:
                        esp->instance->showCursor(&p, evg->frame);
                        break;

                    case EVHIDE:
                        esp->instance->hideCursor();
                        break;

                    default:
                        // should never happen
                        break;
                }
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////
//#define LOG_SCREEN_REGISTRATION
#ifdef LOG_SCREEN_REGISTRATION
#warning LOG_SCREEN_REGISTRATION is defined
#define log_screen_reg(fmt, args...)  kprintf(">>> " fmt, args)
#else
#define log_screen_reg(fmt, args...)
#endif

////////////////////////////////////////////////////////////////////////////
int
IOHIDSystem::registerScreen(IOGraphicsDevice * io_gd,
                            IOGBounds * boundsPtr,
                            IOGBounds * virtualBoundsPtr)
{
    int result = -1;

    // If we are not open for business, fail silently
    if (eventsOpen) {
        // for this version of the call, these must all be supplied
        if (!io_gd || !boundsPtr || !virtualBoundsPtr) {
            HIDLogError("invalid call %p %p %p", io_gd, boundsPtr, virtualBoundsPtr);
        }
        else {
            UInt32 index;
            IOReturn ret = workLoop->runAction((IOWorkLoop::Action)&IOHIDSystem::doRegisterScreen,
                                               this, io_gd, boundsPtr, virtualBoundsPtr, &index);
            if (ret == kIOReturnSuccess) {
                result = SCREENTOKEN + index;
            }
            else {
                HIDLogError("failed %08x", ret);
            }
        }
    }
    log_screen_reg("%s: registered token %d\n", __PRETTY_FUNCTION__, result);
    return result;
}

////////////////////////////////////////////////////////////////////////////
IOReturn
IOHIDSystem::extRegisterVirtualDisplay(void* token_ptr,void*,void*,void*,void*,void*)
{
    IOReturn result = kIOReturnBadArgument;
    if (token_ptr) {
        SInt32 index;
        UInt64 *token = (UInt64 *)token_ptr;
        result = workLoop->runAction((IOWorkLoop::Action)&IOHIDSystem::doRegisterScreen,
                                     this, NULL, NULL, NULL, &index);
        if ((index >= 0) && (index < EV_MAX_SCREENS)) {
            *token = SCREENTOKEN + index;
        }
        else {
            *token = 0;
            if (result == kIOReturnSuccess) {
                result = kIOReturnInternalError;
                HIDLogError("IOHIDSystem tried to return an invalid token with no error");
            }
        }
        log_screen_reg("%s: registered token %lld on %x\n", __PRETTY_FUNCTION__, *token, result);
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////
IOReturn
IOHIDSystem::doRegisterScreen(IOHIDSystem *self,
                              IOGraphicsDevice *io_gd,
                              IOGBounds *bounds,
                              IOGBounds *virtualBounds,
                              void *index)
{
    return self->registerScreenGated(io_gd, bounds, virtualBounds, (SInt32*)index);
}

////////////////////////////////////////////////////////////////////////////
//#define LOG_SCROLL_STATE
#ifdef LOG_SCROLL_STATE
#   define log_scroll_state(s, ...)     kprintf(">>> %s:%d " s, "IOHIDSystem", __LINE__, __VA_ARGS__)
#   define log_scroll_state_b(s, ...)   kprintf(">>> %s:%d " s, "IOHIDSystem", __LINE__, __VA_ARGS__)
#else
#   define log_scroll_state(s, ...)
#   define log_scroll_state_b(s, ...)
#endif

////////////////////////////////////////////////////////////////////////////
IOReturn
IOHIDSystem::registerScreenGated(IOGraphicsDevice *io_gd,
                                 IOGBounds *boundsPtr,
                                 IOGBounds *virtualBoundsPtr,
                                 SInt32 *index)
{
    EvScreen *screen_ptr = NULL;
    OSNumber *num = NULL;
    IOReturn result = kIOReturnSuccess;
    *index = -1;

    /* shmemSize and bounds already set */
    log_screen_reg("%s %p %p %p\n", __func__, io_gd, boundsPtr, virtualBoundsPtr);

    // locate next available screen
    for (int i = 0; (i < EV_MAX_SCREENS) && (screen_ptr == NULL); i++) {
        screen_ptr = &((EvScreen*)evScreen)[i];

        if (io_gd && (screen_ptr->instance == io_gd)) {
            // Empty slot.
            log_screen_reg("%s refound at index %d\n", __func__, i);
            *index = i;
        }
        else if (screen_ptr->creator_pid) {
            proc_t isStillAlive = proc_find(screen_ptr->creator_pid);
            if (isStillAlive) {
                // Still in use.
                screen_ptr = NULL;
                proc_rele(isStillAlive);
            }
            else {
                // Dead head.
                log_screen_reg("Screen %d recycled from pid %d", i, screen_ptr->creator_pid);
                *index = i;
                screen_ptr->creator_pid = 0;
                screen_ptr->displayBounds = NULL;
                screen_ptr->desktopBounds = NULL;
            }
        }
        else if (screen_ptr->instance) {
            // Still in use.
            screen_ptr = NULL; // try the next one
        }
        else {
            // New slot
            log_screen_reg("%s new index at %d\n", __func__, i);
            *index = i;
        }
    }

    if (!screen_ptr) {
        HIDLogError("No space found for new screen");
        result = kIOReturnNoResources;
    }
    else if (io_gd && boundsPtr && virtualBoundsPtr) {
        // called by video driver. they maintain their own bounds.
        screen_ptr->instance = io_gd;
        screen_ptr->instance->retain();
        screen_ptr->displayBounds = boundsPtr;
        screen_ptr->desktopBounds = virtualBoundsPtr;
        screen_ptr->creator_pid = 0; // kernel made

        // Update our idea of workSpace bounds
        if ( boundsPtr->minx < workSpace.minx )
            workSpace.minx = boundsPtr->minx;
        if ( boundsPtr->miny < workSpace.miny )
            workSpace.miny = boundsPtr->miny;
        if ( boundsPtr->maxx < workSpace.maxx )
            workSpace.maxx = boundsPtr->maxx;
        if ( screen_ptr->displayBounds->maxy < workSpace.maxy )
            workSpace.maxy = boundsPtr->maxy;

        // perform other bookkeeping
        num = (OSNumber*)io_gd->copyProperty(kIOFBWaitCursorFramesKey);
        if(OSDynamicCast(OSNumber, num) &&
           (num->unsigned32BitValue() > maxWaitCursorFrame)) {
            firstWaitCursorFrame = 0;
            maxWaitCursorFrame   = num->unsigned32BitValue();
            evg->lastFrame       = maxWaitCursorFrame;
        }
        OSSafeReleaseNULL(num);
        num = (OSNumber*)io_gd->copyProperty(kIOFBWaitCursorPeriodKey);
        if( OSDynamicCast(OSNumber, num) ) {
            clock_interval_to_absolutetime_interval(num->unsigned32BitValue(), kNanosecondScale, &_cursorWaitDelta);
        }
        OSSafeReleaseNULL(num);
    }
    else if (!io_gd && !boundsPtr && !virtualBoundsPtr) {
        // called by window server. we maintain the bounds.
        screen_ptr->displayBounds = (IOGBounds*)screen_ptr->scratch;
        screen_ptr->desktopBounds = screen_ptr->displayBounds + 1;
        screen_ptr->creator_pid = proc_selfpid();

        // default the bounds to lala land
        screen_ptr->displayBounds->minx = screen_ptr->desktopBounds->minx = screen_ptr->displayBounds->miny = screen_ptr->desktopBounds->miny = -30001;
        screen_ptr->displayBounds->maxx = screen_ptr->desktopBounds->maxx = screen_ptr->displayBounds->maxy = screen_ptr->desktopBounds->maxy = -30000;
    }
    else {
        result = kIOReturnBadArgument;
    }

    if (result == kIOReturnSuccess) {
        log_screen_reg(">>> display %d is for %d\n", *index, screen_ptr->creator_pid);
        if (*index >= screens) {
            screens = 1 + *index;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////
void IOHIDSystem::unregisterScreen(int token) {
    uintptr_t index = token - SCREENTOKEN;
    log_screen_reg("%s: unregistering token %d\n", __PRETTY_FUNCTION__, token);
    if (index < EV_MAX_SCREENS) {
        IOReturn ret = cmdGate->runAction((IOCommandGate::Action)doUnregisterScreen, (void *)index);
        if (ret != kIOReturnSuccess) {
            HIDLogError("recieved %08x for token %d.", ret, token);
        }
    }
    else {
        HIDLogError("called with invalid token %d.", token);
    }
}

////////////////////////////////////////////////////////////////////////////
IOReturn
IOHIDSystem::extUnregisterVirtualDisplay(void* token_ptr,void*,void*,void*,void*,void*)
{
    IOReturn result = kIOReturnSuccess;
    uintptr_t token = (uintptr_t)token_ptr;
    uintptr_t index = token - SCREENTOKEN;
    if (index < EV_MAX_SCREENS) {
        result = cmdGate->runAction((IOCommandGate::Action)doUnregisterScreen, (void *)index);
    }
    else {
        HIDLogError("called with invalid token %d.", (int)token);
        result = kIOReturnBadArgument;
    }
    log_screen_reg("%s: unregistering token %lu on %x\n", __PRETTY_FUNCTION__, token, result);

    return result;
}

////////////////////////////////////////////////////////////////////////////
IOReturn IOHIDSystem::doUnregisterScreen (IOHIDSystem *self, void * arg0)
                        /* IOCommandGate::Action */
{
    uintptr_t index = (uintptr_t) arg0;

    return self->unregisterScreenGated((int)index);
}

////////////////////////////////////////////////////////////////////////////
IOReturn IOHIDSystem::unregisterScreenGated(int index)
{
    IOReturn result = kIOReturnSuccess;
    log_screen_reg("%s %d %d\n", __func__, index, screens);

    if ( eventsOpen == false || index >= EV_MAX_SCREENS ) {
        result = kIOReturnNoResources;
    }
    else {
        EvScreen *screen_ptr = ((EvScreen*)evScreen)+index;

        if (screen_ptr->displayBounds) {
            hideCursor();

            // clear the variables
            OSSafeReleaseNULL(screen_ptr->instance);
            screen_ptr->desktopBounds = NULL;
            screen_ptr->displayBounds = NULL;
            screen_ptr->creator_pid = 0;

            // Put the cursor someplace reasonable if it was on the destroyed screen
            cursorScreens &= ~(1 << index);
            // This will jump the cursor back on screen
            setCursorPosition((IOGPoint *)&evg->cursorLoc, true);
            screens--;
            showCursor();
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////
IOReturn
IOHIDSystem::extSetVirtualDisplayBounds(void* token_ptr,void* minx,void* maxx,void* miny,void* maxy,void*)
{
    IOReturn result = kIOReturnSuccess;
    uintptr_t token = (uintptr_t)token_ptr;
    uintptr_t index = token - SCREENTOKEN;
    log_screen_reg("%s: set bounds on token %lu\n", __PRETTY_FUNCTION__, token);
    if (index < EV_MAX_SCREENS) {
        IOGBounds tempBounds = { (SInt16)(uintptr_t) minx, (SInt16)(uintptr_t) maxx, (SInt16)(uintptr_t) miny, (SInt16)(uintptr_t) maxy };
        result = cmdGate->runAction((IOCommandGate::Action)doSetDisplayBounds, (void*) index, (void*) &tempBounds);
    }
    else {
        HIDLogError("called with invalid token %d.", (int)token);
        result = kIOReturnBadArgument;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////
IOReturn
IOHIDSystem::doSetDisplayBounds (IOHIDSystem *self, void * arg0, void * arg1)
{
    uintptr_t index = (uintptr_t) arg0;
    IOGBounds *tempBounds = (IOGBounds*) arg1;

    return self->setDisplayBoundsGated((UInt32)index, tempBounds);
}

////////////////////////////////////////////////////////////////////////////
IOReturn
IOHIDSystem::setDisplayBoundsGated (UInt32 index, IOGBounds *tempBounds)
{
    IOReturn result = kIOReturnSuccess;
    log_screen_reg("%s ((%d,%d),(%d,%d))\n", __func__, tempBounds->minx, tempBounds->miny, tempBounds->maxx, tempBounds->maxy);

    if ( eventsOpen == false || index >= (UInt32)EV_MAX_SCREENS ) {
        result = kIOReturnNoResources;
    }
    else {
        EvScreen *screen_ptr = ((EvScreen*)evScreen)+index;

        if (screen_ptr->instance) {
            HIDLogError("called on an internal device %d", (int)index);
            result = kIOReturnNotPermitted;
        }
        else if (!screen_ptr->displayBounds || !screen_ptr->desktopBounds) {
            HIDLogError("called with invalid index %d", (int)index);
            result = kIOReturnBadArgument;
        }
        else {
            // looks good
            hideCursor();
            *(screen_ptr->displayBounds) = *(screen_ptr->desktopBounds) = *tempBounds;
            // Put the cursor someplace reasonable if it was on the moved screen
            cursorScreens &= ~(1 << index);
            // This will jump the cursor back on screen
            setCursorPosition((IOGPoint *)&evg->cursorLoc, true);
            showCursor();
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////
/* Member of EventClient protocol
 *
 * Absolute position input devices and some specialized output devices
 * may need to know the bounding rectangle for all attached displays.
 * The following method returns a IOGBounds* for the workspace.  Please note
 * that the bounds are kept as signed values, and that on a multi-display
 * system the minx and miny values may very well be negative.
 */
IOGBounds * IOHIDSystem::workspaceBounds()
{
    return &workSpace;
}

IOReturn IOHIDSystem::registerEventQueue(IOSharedDataQueue * queue)
{
    return cmdGate->runAction((IOCommandGate::Action)doRegisterEventQueue, (void *)queue);
}

IOReturn IOHIDSystem::doRegisterEventQueue (IOHIDSystem *self, void * arg0)
                        /* IOCommandGate::Action */
{
    return self->registerEventQueueGated((IOSharedDataQueue *)arg0);
}

IOReturn IOHIDSystem::registerEventQueueGated(void * p1)
{
    IOSharedDataQueue * queue = (IOSharedDataQueue *)p1;
    if ( !queue )
        return kIOReturnBadArgument;

    if ( !dataQueueSet )
        dataQueueSet = OSSet::withCapacity(4);

    dataQueueSet->setObject(queue);

    return kIOReturnSuccess;
}

IOReturn IOHIDSystem::unregisterEventQueue(IOSharedDataQueue * queue)
{
    return cmdGate->runAction((IOCommandGate::Action)doUnregisterEventQueue, (void *)queue);
}

IOReturn IOHIDSystem::doUnregisterEventQueue (IOHIDSystem *self, void * arg0)
                        /* IOCommandGate::Action */
{
    return self->unregisterEventQueueGated((IOSharedDataQueue *)arg0);
}

IOReturn IOHIDSystem::unregisterEventQueueGated(void * p1)
{
    IOSharedDataQueue * queue = (IOSharedDataQueue *)p1;

    if ( !queue )
        return kIOReturnBadArgument;

    if ( dataQueueSet )
        dataQueueSet->removeObject(queue);

    return kIOReturnSuccess;
}

IOReturn IOHIDSystem::createShmem(void* p1, void*, void*, void*, void*, void*)
{                                                                    // IOMethod
    return cmdGate->runAction((IOCommandGate::Action)doCreateShmem, p1);
}

IOReturn IOHIDSystem::doCreateShmem (IOHIDSystem *self, void * arg0)
                        /* IOCommandGate::Action */
{
    return self->createShmemGated(arg0);
}

IOReturn IOHIDSystem::createShmemGated(void* p1 __unused)
{
    initShmem(false);
    eventsOpen = true;
    return kIOReturnSuccess;
}

// Initialize the shared memory area.
//
// This should be run from a command gate action.
void IOHIDSystem::initShmem(bool clean)
{
    int  i;
    EvOffsets *eop;
    int oldFlags = 0;

    /* top of sharedMem is EvOffsets structure */
    eop = (EvOffsets *) shmem_addr;

    if (!clean) {
        oldFlags = ((EvGlobals *)((char *)shmem_addr + sizeof(EvOffsets)))->eventFlags;
    }

    bzero( (void*)shmem_addr, shmem_size);

    /* fill in EvOffsets structure */
    eop->evGlobalsOffset = sizeof(EvOffsets);

    /* find pointers to start of globals and private shmem region */
    evg = (EvGlobals *)((char *)shmem_addr + sizeof(EvOffsets));

    evg->version = kIOHIDCurrentShmemVersion;
    evg->structSize = sizeof( EvGlobals);

    /* Set default wait cursor parameters */
    evg->waitCursorEnabled = TRUE;
    evg->globalWaitCursorEnabled = TRUE;
    evg->lastFrame = maxWaitCursorFrame;
    evg->waitThreshold = (12 * EV_TICKS_PER_SEC) / 10;

    evg->buttons = 0;
    evg->eNum = INITEVENTNUM;
    evg->eventFlags = oldFlags;

    evg->cursorLoc.x = _cursorHelper.desktopLocation().xValue().as32();
    evg->cursorLoc.y = _cursorHelper.desktopLocation().yValue().as32();
    evg->desktopCursorFixed.x = _cursorHelper.desktopLocation().xValue().asFixed24x8();
    evg->desktopCursorFixed.y = _cursorHelper.desktopLocation().yValue().asFixed24x8();
    evg->screenCursorFixed.x = _cursorHelper.getScreenLocation().xValue().asFixed24x8();
    evg->screenCursorFixed.y = _cursorHelper.getScreenLocation().yValue().asFixed24x8();

    evg->updateCursorPositionFromFixed = 0;
    evg->logCursorUpdates = 0;
    evg->dontCoalesce = 0;
    evg->dontWantCoalesce = 0;
    evg->wantPressure = 0;
    evg->wantPrecision = 0;
    evg->mouseRectValid = 0;
    evg->movedMask = 0;
    evg->cursorSema = OS_SPINLOCK_INIT;
    evg->waitCursorSema = OS_SPINLOCK_INIT;

    /* Set up low-level queues */
    lleqSize = LLEQSIZE;
    for (i=lleqSize; --i != -1; ) {
        evg->lleq[i].event.type = 0;
        AbsoluteTime_to_scalar(&evg->lleq[i].event.time) = 0;
        evg->lleq[i].event.flags = 0;
        evg->lleq[i].sema = OS_SPINLOCK_INIT;
        evg->lleq[i].next = i+1;
    }
    evg->LLELast = 0;
    evg->lleq[lleqSize-1].next = 0;
    evg->LLEHead = 1;
    evg->LLETail = 1;

    _cursorLogTimed();
}


UInt32 IOHIDSystem::eventFlags()
{
    return evg ? (evg->eventFlags) : 0;
}

void IOHIDSystem::sleepDisplayTickle()
{
    if (powerState == kIOMessageSystemWillSleep) {
        TICKLE_DISPLAY(kIOHIDEventTypeKeyboard);
    }
}

void IOHIDSystem::dispatchEvent(IOHIDEvent *event, IOOptionBits options __unused)
{
    if ( !event || !dataQueueSet)
        return;

    OSCollectionIterator *      iterator    = OSCollectionIterator::withCollection(dataQueueSet);
#ifdef NEW_HID
    IOHIDEventServiceQueue *    dataQueue   = NULL;

    if ( !iterator )
        return;

    while ((dataQueue = OSDynamicCast(IOHIDEventServiceQueue, iterator->getNextObject()))) {
        dataQueue->enqueueEvent(event);
    }
#endif

    iterator->release();

}

void IOHIDSystem::updateHidActivity()
{
#if TARGET_OS_OSX
    clock_get_uptime(&_lastTickleTime);
    _forceIdle = false;
    if (_hidActivityIdle)
        thread_call_enter(_hidActivityThread);
#endif
}

void IOHIDSystem::hidActivityChecker( )
{
    cmdGate->runAction((IOCommandGate::Action)reportUserHidActivity, NULL);
}

void IOHIDSystem::reportUserHidActivity(IOHIDSystem *self, void *args )
{
    self->reportUserHidActivityGated(args);
}

void IOHIDSystem::reportUserHidActivityGated(void *args __unused)
{
    AbsoluteTime deadline = 0;
    AbsoluteTime ts;
    static AbsoluteTime  idleHidActivity = 0;

    if (!idleHidActivity)
        nanoseconds_to_absolutetime(IDLE_HID_ACTIVITY_NSECS, &idleHidActivity);


    clock_get_uptime(&ts);
    if (!_forceIdle && ((ts-_lastTickleTime)  < idleHidActivity)) {
        if (_hidActivityIdle) {
            _hidActivityIdle = false;
            messageClients(kIOHIDSystemUserHidActivity, (void *)_hidActivityIdle );
        }
        clock_absolutetime_interval_to_deadline((idleHidActivity+_lastTickleTime-ts), &deadline);

        thread_call_enter_delayed(_hidActivityThread, deadline);
    }
    else if ( !_hidActivityIdle ) {
        _hidActivityIdle = true;
        messageClients(kIOHIDSystemUserHidActivity, (void *)_hidActivityIdle );
    }
}

IOReturn IOHIDSystem::extGetUserHidActivityState(void *arg0,void*,void*,void*,void*,void*)
{
    return cmdGate->runAction((IOCommandGate::Action)getUserHidActivityState, arg0);
}

IOReturn IOHIDSystem::getUserHidActivityState(IOHIDSystem *self, void *arg0)
{
    return self->getUserHidActivityStateGated(arg0);
}

IOReturn IOHIDSystem::getUserHidActivityStateGated(void *state)
{
    if (state) {
        *((uint64_t*)state) = _hidActivityIdle ? 1 : 0;
        return kIOReturnSuccess;
    }
    return kIOReturnBadArgument;
}

//
// Helper functions for postEvent
//
//static inline int myAbs(int a) { return(a > 0 ? a : -a); }


// postEvent
//
// This routine actually places events in the event queue which is in
// the EvGlobals structure.  It is called from all parts of the ev
// driver.
//
// This should be run from a command gate action.
//

void IOHIDSystem::postEvent(int           what,
             /* at */       IOFixedPoint64 *location,
             /* atTime */   AbsoluteTime  ts,
             /* withData */ NXEventData * myData,
             /* sender */   OSObject *    sender,
             /* extPID */   UInt32        extPID,
             /* processKEQ*/bool          processKEQ,
             /* options*/   UInt32        options
                            )
{
    // Clear out the keyboard queue up until this TS.  This should keep
    // the events in order.
    IOHIDEvent * event = NULL;
    
    PROFILE_TRACE(7);
    
    if ( processKEQ ) {
        processKeyboardEQ(this, &ts);
    }
    
    NXEventExt nxEvent;
    memset(&nxEvent, 0, sizeof(nxEvent));
    
    uint64_t   ns;
    nxEvent.payload.type         = what;
    if (sender) {
        IORegistryEntry *entry = OSDynamicCast(IORegistryEntry, sender);
        if (entry) {
            nxEvent.payload.service_id = (uintptr_t)entry->getRegistryEntryID();
        } else {
            nxEvent.payload.service_id = getRegistryEntryID();
        }
    }
    else {
        nxEvent.payload.service_id = getRegistryEntryID();
    }
    
    nxEvent.extension.flags = options;
    
    nxEvent.payload.ext_pid      = extPID;
    nxEvent.payload.location.x   = location->xValue().as32();
    nxEvent.payload.location.y   = location->yValue().as32();
    nxEvent.payload.flags        = eventFlags();

   
    proc_t process =  proc_self();
    if (process) {
        kauth_cred_t cred = kauth_cred_proc_ref(process);
        if (cred) {
            nxEvent.extension.audit.val[0] = cred->cr_audit.as_aia_p->ai_auid;
            nxEvent.extension.audit.val[1] = cred->cr_posix.cr_uid;
            nxEvent.extension.audit.val[2] = cred->cr_posix.cr_groups[0];
            nxEvent.extension.audit.val[3] = cred->cr_posix.cr_ruid;
            nxEvent.extension.audit.val[4] = cred->cr_posix.cr_rgid;
            nxEvent.extension.audit.val[5] = proc_pid(process);
            nxEvent.extension.audit.val[6] = cred->cr_audit.as_aia_p->ai_asid;
            nxEvent.extension.audit.val[7] = proc_pidversion(process);
            nxEvent.extension.flags |= NX_EVENT_EXTENSION_AUDIT_TOKEN;
            kauth_cred_unref(&cred);
        }
        proc_rele(process);
    }
    
    require_action (nxEvent.extension.flags & NX_EVENT_EXTENSION_AUDIT_TOKEN, exit, HIDLogError("Unable to get audit token for event"));
    
    
    absolutetime_to_nanoseconds(ts, &ns);
    nxEvent.payload.time = ns;


    if (myData != NULL) {
        nxEvent.payload.data = *myData;
    }

    event = IOHIDEvent::vendorDefinedEvent(
                                  ts,
                                  kHIDPage_AppleVendor,
                                  kHIDUsage_AppleVendor_NXEvent,
                                  0,
                                  (uint8_t*)&nxEvent,
                                  sizeof(nxEvent),
                                  0);
    if ( event ) {
        dispatchEvent(event);
        event->release();
    }

exit:
    PROFILE_TRACE(8);
}




void IOHIDSystem::scheduleNextPeriodicEvent()
{
    if ( !eventsOpen ) {
        // If eventsOpen is false, then the driver shmem is
        // no longer valid, and it is in the process of shutting down.
        // We should give up without rescheduling.
        IOHID_DEBUG(kIOHIDDebugCode_Scheduling, 0, 0, 0, 0);
    }
    else {
        uint64_t            scheduledEvent = _periodicEventNext;
        uint64_t            now = 0;
        static uint64_t     kOneMS = 0;
        if (!kOneMS) {
            clock_interval_to_absolutetime_interval(1, kMillisecondScale, &kOneMS);
        }
        clock_get_uptime(&now);
        
        if ((scheduledEvent > _periodicEventLast) && (scheduledEvent < (now + kOneMS))) {
            // we have an event scheduled in the next MS. Do not reschedule.
        }
        else {
            if (scheduledEvent <= _periodicEventLast) {
                // scheduledEvent is old. do not reuse.
                scheduledEvent = kIOHIDSystenDistantFuture;
            }
            
            if (screens && (kIOPMDeviceUsable & displayState)) {
                // displays are on and nothing is scheduled.
                // calculate deltas
                uint64_t nextMove = _cursorMoveLast + _cursorMoveDelta;
                uint64_t nextWait = _cursorWaitLast + _cursorWaitDelta;
                
                if (_cursorMoveDelta) {
                    if (_cursorEventLast > _cursorMoveLast) {
                        scheduledEvent = nextMove;
                    }
                }
                
                bool waitCursorShouldAlreadyBeUp =  evg->waitCursorEnabled &&
                                                    evg->globalWaitCursorEnabled &&
                                                    evg->ctxtTimedOut &&
                                                    _cursorWaitDelta;
                if (waitCursorShouldAlreadyBeUp) {
                    if (evg->waitCursorUp) {
                        if (scheduledEvent > nextWait) {
                            // update the wait cursor
                            scheduledEvent = nextWait;
                        }
                    }
                    else {
                        // *show* the wait cursor immediately
                        scheduledEvent = now + kOneMS;
                    }
                    
                }
                else if (evg->waitCursorUp) {
                    // *hide* the wait cursor immediately
                    scheduledEvent = now + kOneMS;
                }
#if 0
                kprintf(" %d%d%d%d %lld : %lld %lld %lld %llu\n",
                        evg->waitCursorEnabled, evg->globalWaitCursorEnabled, evg->ctxtTimedOut, evg->waitCursorUp,
                        now,
                        scheduledEvent, nextMove, nextWait, _cursorWaitDelta);
#endif
                IOHID_DEBUG(kIOHIDDebugCode_Scheduling, scheduledEvent, nextMove, nextWait, 0);
            }
            else {
                // We have something to do, but no one is "listening".
                // Try again in 50 ms.
                scheduledEvent = now + (50 * kOneMS);
                IOHID_DEBUG(kIOHIDDebugCode_Scheduling, scheduledEvent, 0, 0, 0);
            }
            
            if (kIOHIDSystenDistantFuture == scheduledEvent) {
                // no periodic events. cancel any pending periodic timer.
                periodicES->cancelTimeout();
            }
            else {
                _periodicEventNext = scheduledEvent;
                if (now + kOneMS > _periodicEventNext) {
                    // do not schedule the event too soon.
                    _periodicEventNext = now + kOneMS;
                }
                if (now + (100 * kOneMS) < _periodicEventNext) {
                    // after 100 ms, we really should check again. *something* is happening.
                    _periodicEventNext = now + (100 * kOneMS);
                }
                IOReturn err = periodicES->wakeAtTime(_periodicEventNext);
                if (err) {
                    HIDLogError("wakeAtTime failed for %lld: %08x (%s)", _periodicEventNext, err, stringFromReturn(err));
                }
            }
        }
    }
}

// Periodic events are driven from this method.
// After taking care of all pending work, the method
// calls scheduleNextPeriodicEvent to compute and set the
// next callout.

// Modified this method to call a command Gate action.
// This will hopefully insure that this is serialized
// with other actions associated with the command gate.

void IOHIDSystem::_periodicEvents(IOHIDSystem * self,
                                  IOTimerEventSource *timer)
{
    self->periodicEvents(timer);
}

void IOHIDSystem::periodicEvents(IOTimerEventSource * timer __unused)
{
    // If eventsOpen is false, then the driver shmem is
    // no longer valid, and it is in the process of shutting down.
    // We should give up without rescheduling.
    if ( !eventsOpen )
        return;

    clock_get_uptime(&_periodicEventLast);          // update last event
    _periodicEventNext = kIOHIDSystenDistantFuture; // currently no next event

    // Update cursor position if needed
    if (_periodicEventLast >= _cursorMoveLast + _cursorMoveDelta) {
        _cursorHelper.startPosting();
        _cursorHelper.applyPostingDelta();
        _setCursorPosition(false, false, lastSender);
        OSSafeReleaseNULL(lastSender);
        _cursorMoveLast = _periodicEventLast;
    }

    // WAITCURSOR ACTION
    if ( OSSpinLockTry(&evg->waitCursorSema) )
    {
        if ( OSSpinLockTry(&evg->cursorSema) )
        {
            // If wait cursor enabled and context timed out, do waitcursor
            if (evg->waitCursorEnabled && evg->globalWaitCursorEnabled)
            {
                /* WAIT CURSOR SHOULD BE ON */
                if (!evg->waitCursorUp) {
                    showWaitCursor();
                    _cursorWaitLast = _periodicEventLast;
                }
            }
            else {
                /* WAIT CURSOR SHOULD BE OFF */
                if (evg->waitCursorUp) {
                    hideWaitCursor();
                    _cursorWaitLast = _periodicEventLast;
                }
            }
            /* Animate cursor */
            if (evg->waitCursorUp && ((_cursorWaitLast + _cursorWaitDelta) < _periodicEventLast)) {
                animateWaitCursor();
                _cursorWaitLast = _periodicEventLast;
            }
            OSSpinLockUnlock(&evg->cursorSema);
        }
        OSSpinLockUnlock(&evg->waitCursorSema);
    }

    scheduleNextPeriodicEvent();

    return;
}

//
// Start the cursor system running.
//
// At this point, the WindowServer is up, running, and ready to process events.
// We will attach the keyboard and mouse, if none are available yet.
//
bool IOHIDSystem::resetCursor()
{
    volatile IOGPoint * p;
    UInt32 newScreens = 0;
    SInt32 candidate = 0;
    SInt32 pinScreen = -1L;

    p = &evg->cursorLoc;

    /* Get mask of screens on which the cursor is present */
    EvScreen *screen = (EvScreen *)evScreen;
    for (int i = 0; i < EV_MAX_SCREENS; i++ ) {
        if (!screen[i].desktopBounds)
            continue;
        if ((screen[i].desktopBounds->maxx - screen[i].desktopBounds->minx) < 128)
            continue;
        candidate = i;
        if (_cursorHelper.desktopLocation().inRect(*screen[i].desktopBounds)) {
            pinScreen = i;
            newScreens |= (1 << i);
        }
    }

    if (newScreens == 0)
        pinScreen = candidate;

    if (!cursorPinned) {
        // reset pin rect
        if (((EvScreen*)evScreen)[pinScreen].desktopBounds) {
            cursorPin = *(((EvScreen*)evScreen)[pinScreen].desktopBounds);
            cursorPin.maxx--;   /* Make half-open rectangle */
            cursorPin.maxy--;
            cursorPinScreen = pinScreen;
        }
        else {
            // How do you pin when there are no screens with bounds...
        }
    }

    if (newScreens == 0) {
        /* Pin new cursor position to cursorPin rect */
        p->x = (p->x < cursorPin.minx) ?
            cursorPin.minx : ((p->x > cursorPin.maxx) ?
            cursorPin.maxx : p->x);
        p->y = (p->y < cursorPin.miny) ?
            cursorPin.miny : ((p->y > cursorPin.maxy) ?
            cursorPin.maxy : p->y);

        /* regenerate mask for new position */
        for (int i = 0; i < EV_MAX_SCREENS; i++ ) {
            if (!screen[i].desktopBounds)
                continue;
            if ((screen[i].desktopBounds->maxx - screen[i].desktopBounds->minx) < 128)
                continue;
            if (_cursorHelper.desktopLocation().inRect(*screen[i].desktopBounds)){
                pinScreen = i;
                newScreens |= (1 << i);
            }
        }
    }

    cursorScreens = newScreens;
    IOFixedPoint64 tempLoc;
    if (evg->updateCursorPositionFromFixed) {
        tempLoc.fromFixed24x8(evg->desktopCursorFixed.x, evg->desktopCursorFixed.y);
    }
    else {
        tempLoc.fromIntFloor(evg->cursorLoc.x, evg->cursorLoc.y);
    }
    _cursorHelper.desktopLocationDelta() += tempLoc - _cursorHelper.desktopLocation();
    _cursorHelper.desktopLocation() = tempLoc;
    if (pinScreen >= 0) {
        _cursorHelper.updateScreenLocation(screen[pinScreen].desktopBounds, screen[pinScreen].displayBounds);
    }
    else {
        _cursorHelper.updateScreenLocation(NULL, NULL);
    }

    _cursorMoveDelta = _cursorWaitDelta = 0;
    _cursorHelper.desktopLocationPosting().fromIntFloor(0, 0);
    _cursorHelper.clearEventCounts();

    _cursorLogTimed();

    scheduleNextPeriodicEvent();

    return( true );
}

bool IOHIDSystem::startCursor()
{
    if (0 == screens) {
        // no screens, no cursor
    }
    else {
        cursorPinned = false;
        resetCursor();
        showCursor();

        // Start the cursor control callouts
        scheduleNextPeriodicEvent();

        cursorStarted = true;
    }

    return( cursorStarted );
}

//
// Wait Cursor machinery.  These methods should be run from a command
// gate action and the shared memory area must be set up.
//
void IOHIDSystem::showWaitCursor()
{
    xpr_ev_cursor("showWaitCursor\n",1,2,3,4,5);
    evg->waitCursorUp = true;
    hideCursor();
    evg->frame = EV_WAITCURSOR;
    showCursor();
}

void IOHIDSystem::hideWaitCursor()
{
    xpr_ev_cursor("hideWaitCursor\n",1,2,3,4,5);
    evg->waitCursorUp = false;
    hideCursor();
    evg->frame = EV_STD_CURSOR;
    showCursor();
}

void IOHIDSystem::animateWaitCursor()
{
    xpr_ev_cursor("animateWaitCursor\n",1,2,3,4,5);
    changeCursor(evg->frame + 1);
}

void IOHIDSystem::changeCursor(int frame)
{
    evg->frame = ((frame > (int)maxWaitCursorFrame) || (frame > evg->lastFrame)) ? firstWaitCursorFrame : frame;
    xpr_ev_cursor("changeCursor %d\n",evg->frame,2,3,4,5);
    moveCursor();
}

//
// Return the screen number in which point p lies.  Return -1 if the point
// lies outside of all registered screens.
//
int IOHIDSystem::pointToScreen(IOGPoint * p)
{
    int i;
    EvScreen *screen = (EvScreen *)evScreen;
    for (i=EV_MAX_SCREENS-1; --i != -1; ) {
        if ((screen[i].desktopBounds != 0)
            && (p->x >= screen[i].desktopBounds->minx)
            && (p->x < screen[i].desktopBounds->maxx)
            && (p->y >= screen[i].desktopBounds->miny)
            && (p->y < screen[i].desktopBounds->maxy))
        return i;
    }
    return(-1); /* Cursor outside of known screen boundary */
}

//
// API used to drive event state out to attached screens
//
// These should be run from a command gate action.
//
inline void IOHIDSystem::showCursor()
{
    _diags.lastCursorActionsMask |=  (1 << Diags::kCursorActionShow);
    _diags.lastActionTimes[Diags::kCursorActionShow] = mach_absolute_time();

    evDispatch(/* command */ EVSHOW);
}
inline void IOHIDSystem::hideCursor()
{
    _diags.lastCursorActionsMask |=  (1 << Diags::kCursorActionHide);
    _diags.lastActionTimes[Diags::kCursorActionHide] = mach_absolute_time();

    evDispatch(/* command */ EVHIDE);
}

inline void IOHIDSystem::moveCursor()
{
    _diags.lastCursorActionsMask |=  (1 << Diags::kCursorActionMove);
    _diags.lastActionTimes[Diags::kCursorActionMove] = mach_absolute_time();

    evDispatch(/* command */ EVMOVE);
}

IOReturn IOHIDSystem::updateParamPropertiesGated(IOService * source) {

  OSDictionary * newParams = OSDictionary::withDictionary( savedParameters );
  if( newParams) {

      // update with user settings
      if ( OSDynamicCast(IOHIDevice, source) )
          ((IOHIDevice *)source)->setParamProperties( newParams );
#ifdef NEW_HID
      else if ( OSDynamicCast(IOHIDEventService, source) )
          ((IOHIDEventService *)source)->setSystemProperties( newParams );
#endif

      setProperty( kIOHIDParametersKey, newParams );
      newParams->release();
      savedParameters = newParams;
  }

  return kIOReturnSuccess;
}

//
// EventSrcClient implementation
//

//
// A new device instance desires to be added to our list.
// Try to get ownership of the device. If we get it, add it to
// the list.
//
bool IOHIDSystem::registerEventSource(IOService * source)
{
    bool success = true;
    if (!source)
        return true;

    if ( OSDynamicCast(IOHIKeyboard, source)) {
      success = ((IOHIKeyboard*)source)->open(this, kIOServiceSeize,0,
                    (KeyboardEventCallback)        _keyboardEvent,
                    (KeyboardSpecialEventCallback) _keyboardSpecialEvent,
                    (UpdateEventFlagsCallback)     _updateEventFlags);
    }

    if ( success )
    {
        //Yet another gate call to protect savedParameters on registerEventSource call from handlePublishNotification
        cmdGate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &IOHIDSystem::updateParamPropertiesGated),source);
    }
    else
    {
        HIDLogError("%s: Seize of %s failed.", getName() , source->getName());
    }
    return true;
}

IOReturn IOHIDSystem::message(UInt32 type, IOService * provider,
                void * argument)
{
    IOReturn     status = kIOReturnSuccess;

    switch (type)
    {
        case kIOMessageServiceIsTerminated:
            if (provider) {
                provider->close( this );
            }
            break;
            
        case kIOMessageServiceWasClosed:
            break;

        case kIOHIDSystemActivityTickle: {
            intptr_t nxEvent = (intptr_t) argument;
            if ((nxEvent >= 0) && (nxEvent <= NX_LASTEVENT)) {
                if (!evStateChanging && displayManager) {
                    IOHID_DEBUG(kIOHIDDebugCode_DisplayTickle, nxEvent, __LINE__, displayState,
                                provider ? provider->getRegistryEntryID() : 0);
                    AbsoluteTime ts;
                    clock_get_uptime (&ts);
                    if (DISPLAY_IS_ENABLED ||
                       (NX_WAKEMASK & EventCodeMask(nxEvent)) ||
                       (NX_UNDIMMASK & EventCodeMask(nxEvent) && CMP_ABSOLUTETIME(&ts, &displaySleepWakeupDeadline) <=0)) {
                        if (!DISPLAY_IS_ENABLED) {
                            kprintf("IOHIDSystem tickle when screen off for event %ld\n", nxEvent);
                        }
                        displayManager->activityTickle(IOHID_DEBUG_CODE(nxEvent));
                    }
                }
            }
            else if (nxEvent == NX_HARDWARE_TICKLE) {
                if (!evStateChanging && displayManager) {
                    IOHID_DEBUG(kIOHIDDebugCode_DisplayTickle, nxEvent, __LINE__, displayState,
                                provider ? provider->getRegistryEntryID() : 0);
                    if (!DISPLAY_IS_ENABLED) {
                        kprintf("IOHIDSystem tickle when screen off for hardware event from %08llx\n",
                                provider ? provider->getRegistryEntryID() : 0);
                    }
                    displayManager->activityTickle(IOHID_DEBUG_CODE(nxEvent));
                }
            }
            else {
                HIDLogError("kIOHIDSystemActivityTickle message for unsupported event %ld sent from %08llx",
                      nxEvent, provider ? provider->getRegistryEntryID() : 0);
            }
            break;
        }

        default:
            status = super::message(type, provider, argument);
            break;
    }

    return status;
}


void IOHIDSystem::doProcessKeyboardEQ(IOHIDSystem * self)
{
    processKeyboardEQ(self);
}

void IOHIDSystem::processKeyboardEQ(IOHIDSystem * self, AbsoluteTime * deadline)
{
    KeyboardEQElement * keyboardEQElement;

    KEYBOARD_EQ_LOCK;

    while ( ((keyboardEQElement = (KeyboardEQElement *)dequeue_head(&gKeyboardEQ)) != NULL)
           && !(deadline && (CMP_ABSOLUTETIME(&(keyboardEQElement->ts), deadline) > 0)))
    {
        KEYBOARD_EQ_UNLOCK;

        if (keyboardEQElement->action)
            (*(keyboardEQElement->action))(self, keyboardEQElement);
        OSSafeReleaseNULL(keyboardEQElement->sender); // NOTE: This is the matching release

        IOFree(keyboardEQElement, sizeof(KeyboardEQElement));

        KEYBOARD_EQ_LOCK;
    }

    KEYBOARD_EQ_UNLOCK;
}


//
// Process a keyboard state change.
//
void IOHIDSystem::_keyboardEvent(IOHIDSystem * self,
                                unsigned   eventType,
         /* flags */            unsigned   flags,
         /* keyCode */          unsigned   key,
         /* charCode */         unsigned   charCode,
         /* charSet */          unsigned   charSet,
         /* originalCharCode */ unsigned   origCharCode,
         /* originalCharSet */  unsigned   origCharSet,
         /* keyboardType */     unsigned   keyboardType,
         /* repeat */           bool       repeat,
         /* atTime */           AbsoluteTime ts,
                                OSObject * sender,
                                void *     refcon __unused)
{
        self->keyboardEvent(eventType, flags , key, charCode, charSet,
                        origCharCode, origCharSet, keyboardType, repeat, ts, sender);
}

void IOHIDSystem::keyboardEvent(unsigned   eventType,
         /* flags */            unsigned   flags,
         /* keyCode */          unsigned   key,
         /* charCode */         unsigned   charCode,
         /* charSet */          unsigned   charSet,
         /* originalCharCode */ unsigned   origCharCode,
         /* originalCharSet */  unsigned   origCharSet,
         /* keyboardType */     unsigned   keyboardType,
         /* repeat */           bool       repeat,
         /* atTime */           AbsoluteTime ts)
{
    keyboardEvent(eventType, flags, key, charCode, charSet,
                origCharCode, origCharSet, keyboardType, repeat, ts, 0);
}

void IOHIDSystem::keyboardEvent(unsigned   eventType,
         /* flags */            unsigned   flags,
         /* keyCode */          unsigned   key,
         /* charCode */         unsigned   charCode,
         /* charSet */          unsigned   charSet,
         /* originalCharCode */ unsigned   origCharCode,
         /* originalCharSet */  unsigned   origCharSet,
         /* keyboardType */     unsigned   keyboardType,
         /* repeat */           bool       repeat,
         /* atTime */           AbsoluteTime ts,
         /* sender */       OSObject * sender)
{
    KeyboardEQElement * keyboardEQElement = (KeyboardEQElement *)IOMalloc(sizeof(KeyboardEQElement));

    if ( !keyboardEQElement )
        return;

    bzero(keyboardEQElement, sizeof(KeyboardEQElement));

    keyboardEQElement->action   = IOHIDSystem::doKeyboardEvent;
    keyboardEQElement->ts       = ts;
    keyboardEQElement->sender   = sender;
    if (sender) sender->retain(); // NOTE: Matching release is in IOHIDSystem::processKeyboardEQ


    keyboardEQElement->event.keyboard.eventType     = eventType;
    keyboardEQElement->event.keyboard.flags         = flags;
    keyboardEQElement->event.keyboard.key           = key;
    keyboardEQElement->event.keyboard.charCode      = charCode;
    keyboardEQElement->event.keyboard.charSet       = charSet;
    keyboardEQElement->event.keyboard.origCharCode  = origCharCode;
    keyboardEQElement->event.keyboard.origCharSet   = origCharSet;
    keyboardEQElement->event.keyboard.keyboardType  = keyboardType;
    keyboardEQElement->event.keyboard.repeat        = repeat;

    KEYBOARD_EQ_LOCK;
    enqueue_tail(&gKeyboardEQ, (queue_entry_t)keyboardEQElement);
    KEYBOARD_EQ_UNLOCK;

    keyboardEQES->interruptOccurred(0, 0, 0);
}

IOReturn IOHIDSystem::doKeyboardEvent(IOHIDSystem *self, void * args)
                        /* IOCommandGate::Action */
{
    KeyboardEQElement * keyboardEQElement = (KeyboardEQElement *)args;

    AbsoluteTime ts         = keyboardEQElement->ts;
    OSObject * sender       = keyboardEQElement->sender;

    unsigned   eventType    = keyboardEQElement->event.keyboard.eventType;
    unsigned   flags        = keyboardEQElement->event.keyboard.flags;
    unsigned   key          = keyboardEQElement->event.keyboard.key;
    unsigned   charCode     = keyboardEQElement->event.keyboard.charCode;
    unsigned   charSet      = keyboardEQElement->event.keyboard.charSet;
    unsigned   origCharCode = keyboardEQElement->event.keyboard.origCharCode;
    unsigned   origCharSet  = keyboardEQElement->event.keyboard.origCharSet;
    unsigned   keyboardType = keyboardEQElement->event.keyboard.keyboardType;
    bool       repeat       = keyboardEQElement->event.keyboard.repeat;

    self->keyboardEventGated(eventType, flags, key, charCode, charSet,
                origCharCode, origCharSet, keyboardType, repeat, ts, sender);

    return kIOReturnSuccess;
}

void IOHIDSystem::keyboardEventGated(unsigned   eventType,
                                /* flags */            unsigned   flags,
                                /* keyCode */          unsigned   key,
                                /* charCode */         unsigned   charCode,
                                /* charSet */          unsigned   charSet,
                                /* originalCharCode */ unsigned   origCharCode,
                                /* originalCharSet */  unsigned   origCharSet,
                                /* keyboardType */  unsigned   keyboardType,
                                /* repeat */           bool       repeat,
                                /* atTime */           AbsoluteTime ts,
                                /* sender */           OSObject * sender)
{

    if (eventType == NX_KEYDOWN) {
        TICKLE_DISPLAY(NX_KEYDOWN);
    }
    // Take on BSD console duties and dispatch the keyboardEvents.
    if (eventsOpen ) {
        NXEventData outData;
        UInt32 usage = 0;
        UInt32 usagePage = 0;
        if ((flags & NX_HIGHCODE_ENCODING_MASK) == NX_HIGHCODE_ENCODING_MASK) {
            usage = (origCharCode & 0xffff0000) >> 16;
            usagePage = (origCharSet & 0xffff0000) >> 16;
            origCharCode &= 0xffff;
            origCharSet &= 0xffff;
        }

        outData.key.repeat = repeat;
        outData.key.keyCode = key;
        outData.key.charSet = charSet;
        outData.key.charCode = charCode;
        outData.key.origCharSet = origCharSet;
        outData.key.origCharCode = origCharCode;
        outData.key.keyboardType = keyboardType;
        outData.key.reserved2 = usage;
        outData.key.reserved3 = usagePage;

        evg->eventFlags = (evg->eventFlags & ~KEYBOARD_FLAGSMASK)
                | (flags & KEYBOARD_FLAGSMASK);

        postEvent(         eventType,
            /* at */       &_cursorHelper.desktopLocation(),
            /* atTime */   ts,
            /* withData */ &outData,
            /* sender */   sender,
            /* extPID */   0,
            /* processKEQ*/false
            );
    } else {
        static const char cursorCodes[] = { 'D', 'A', 'C', 'B' };
        if( (eventType == NX_KEYDOWN) && ((flags & NX_ALTERNATEMASK) != NX_ALTERNATEMASK)) {
            if( (charSet == NX_SYMBOLSET)
                && (charCode >= 0xac) && (charCode <= 0xaf)) {
                cons_cinput( '\033');
                cons_cinput( 'O');
                charCode = cursorCodes[ charCode - 0xac ];
            }
            cons_cinput( charCode );
        }
    }
}

void IOHIDSystem::_keyboardSpecialEvent(  IOHIDSystem * self,
                      unsigned   eventType,
                       /* flags */        unsigned   flags,
                       /* keyCode  */     unsigned   key,
                       /* specialty */    unsigned   flavor,
                       /* guid */         UInt64     guid,
                       /* repeat */       bool       repeat,
                       /* atTime */       AbsoluteTime ts,
                      OSObject * sender,
                      void *     refcon __unused)
{
    self->keyboardSpecialEvent(eventType, flags, key, flavor, guid, repeat, ts, sender);
}

void IOHIDSystem::keyboardSpecialEvent(   unsigned   eventType,
                       /* flags */        unsigned   flags,
                       /* keyCode  */     unsigned   key,
                       /* specialty */    unsigned   flavor,
                       /* guid */         UInt64     guid,
                       /* repeat */       bool       repeat,
                       /* atTime */       AbsoluteTime ts)
{
    keyboardSpecialEvent(eventType, flags, key, flavor, guid, repeat, ts, 0);
}

void IOHIDSystem::keyboardSpecialEvent(   unsigned   eventType,
                       /* flags */        unsigned   flags,
                       /* keyCode  */     unsigned   key,
                       /* specialty */    unsigned   flavor,
                       /* guid */         UInt64     guid,
                       /* repeat */       bool       repeat,
                       /* atTime */       AbsoluteTime ts,
                       /* sender */       OSObject * sender)
{
    KeyboardEQElement * keyboardEQElement = NULL;

    keyboardEQElement = (KeyboardEQElement *)IOMalloc(sizeof(KeyboardEQElement));

    if ( !keyboardEQElement )
        return;

    bzero(keyboardEQElement, sizeof(KeyboardEQElement));

    keyboardEQElement->action   = IOHIDSystem::doKeyboardSpecialEvent;
    keyboardEQElement->ts       = ts;
    keyboardEQElement->sender   = sender;
    if (sender) sender->retain(); // NOTE: Matching release is in IOHIDSystem::processKeyboardEQ

    keyboardEQElement->event.keyboardSpecial.eventType  = eventType;
    keyboardEQElement->event.keyboardSpecial.flags      = flags;
    keyboardEQElement->event.keyboardSpecial.key        = key;
    keyboardEQElement->event.keyboardSpecial.flavor     = flavor;
    keyboardEQElement->event.keyboardSpecial.guid       = guid;
    keyboardEQElement->event.keyboardSpecial.repeat     = repeat;

    KEYBOARD_EQ_LOCK;
    enqueue_tail(&gKeyboardEQ, (queue_entry_t)keyboardEQElement);
    KEYBOARD_EQ_UNLOCK;

    keyboardEQES->interruptOccurred(0, 0, 0);
}


IOReturn IOHIDSystem::doKeyboardSpecialEvent(IOHIDSystem *self, void * args)
                        /* IOCommandGate::Action */
{
    KeyboardEQElement * keyboardEQElement = (KeyboardEQElement *)args;

    AbsoluteTime    ts          = keyboardEQElement->ts;
    OSObject *      sender      = keyboardEQElement->sender;

    unsigned        eventType   = keyboardEQElement->event.keyboardSpecial.eventType;
    unsigned        flags       = keyboardEQElement->event.keyboardSpecial.flags;
    unsigned        key         = keyboardEQElement->event.keyboardSpecial.key;
    unsigned        flavor      = keyboardEQElement->event.keyboardSpecial.flavor;
    UInt64          guid        = keyboardEQElement->event.keyboardSpecial.guid;
    bool            repeat      = keyboardEQElement->event.keyboardSpecial.repeat;

    self->keyboardSpecialEventGated(eventType, flags, key, flavor, guid, repeat, ts, sender);

    return kIOReturnSuccess;
}

void IOHIDSystem::keyboardSpecialEventGated(
                                /* event */     unsigned   eventType,
                                /* flags */     unsigned   flags,
                                /* keyCode  */  unsigned   key __unused,
                                /* specialty */ unsigned   flavor,
                                /* guid */      UInt64     guid __unused,
                                /* repeat */    bool       repeat,
                                /* atTime */    AbsoluteTime ts,
                                /* sender */    OSObject * sender)
{
    NXEventData outData;
 
    // Since the HIDSystem will now take on BSD Console duty,
    // we need to make sure to process the programmer key info
    // prior to doing the eventsOpen check
    if ( eventType == NX_KEYDOWN && flavor == NX_POWER_KEY && !repeat) {
        if ( (flags & (NORMAL_MODIFIER_MASK | NX_DEVICELCMDKEYMASK | NX_DEVICERCMDKEYMASK)) ==
              (NX_COMMANDMASK | NX_DEVICELCMDKEYMASK | NX_DEVICERCMDKEYMASK) )
            PE_enter_debugger("Programmer Key");
        else if ( (flags & NORMAL_MODIFIER_MASK) == ( NX_COMMANDMASK | NX_CONTROLMASK ) )
            PEHaltRestart(kPERestartCPU);
    }

    if ( !eventsOpen )
        return;
    
    bzero( (void *)&outData, sizeof outData );

    if (  flavor == NX_POWER_KEY) {
        keyboardEventGated(eventType,
                           flags,
                           NX_POWER_KEY,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           ts,
                           sender);
        if (eventType == NX_KEYDOWN) {
          outData.compound.subType = NX_SUBTYPE_POWER_KEY;
          postEvent(         NX_SYSDEFINED,
              /* at */       &_cursorHelper.desktopLocation(),
              /* atTime */   ts,
              /* withData */ &outData,
              /* sender */   sender,
              /* extPID */   0,
              /* processKEQ*/false);
        }
    }
}

/*
 * Update current event flags.  Restricted to keyboard flags only, this
 * method is used to silently update the flags state for keys which both
 * generate characters and flag changes.  The specs say we don't generate
 * a flags-changed event for such keys.  This method is also used to clear
 * the keyboard flags on a keyboard subsystem reset.
 */
void IOHIDSystem::_updateEventFlags(IOHIDSystem * self,
                                    unsigned      flags,
                                    OSObject *    sender,
                                    void *        refcon __unused)
{
    self->updateEventFlags(flags, sender);
}

void IOHIDSystem::updateEventFlags(unsigned flags)
{
    updateEventFlags(flags, 0);
}

void IOHIDSystem::updateEventFlags(unsigned flags, OSObject * sender)
{
    KeyboardEQElement * keyboardEQElement = (KeyboardEQElement *)IOMalloc(sizeof(KeyboardEQElement));

    if ( !keyboardEQElement )
        return;

    bzero(keyboardEQElement, sizeof(KeyboardEQElement));

    keyboardEQElement->action = IOHIDSystem::doUpdateEventFlags;
    keyboardEQElement->sender = sender;
    if (sender) sender->retain(); // NOTE: Matching release is in IOHIDSystem::processKeyboardEQ

    keyboardEQElement->event.flagsChanged.flags = flags;

    KEYBOARD_EQ_LOCK;
    enqueue_tail(&gKeyboardEQ, (queue_entry_t)keyboardEQElement);
    KEYBOARD_EQ_UNLOCK;

    keyboardEQES->interruptOccurred(0, 0, 0);
}

IOReturn IOHIDSystem::doUpdateEventFlags(IOHIDSystem *self, void * args)
                        /* IOCommandGate::Action */
{
    KeyboardEQElement * keyboardEQElement = (KeyboardEQElement *)args;

    OSObject * sender   = keyboardEQElement->sender;
    unsigned   flags    = keyboardEQElement->event.flagsChanged.flags;

    self->updateEventFlagsGated(flags, sender);

    return kIOReturnSuccess;
}

void IOHIDSystem::updateEventFlagsGated(unsigned flags, OSObject * sender __unused)
{
    if ( eventsOpen ) {
        evg->eventFlags = (evg->eventFlags & ~KEYBOARD_FLAGSMASK)
                | (flags & KEYBOARD_FLAGSMASK);
        }
}

//
//  Sets the cursor position (evg->cursorLoc) to the new
//  location.  The location is clipped against the cursor pin rectangle,
//  mouse moved/dragged events are generated using the given event mask,
//  and a mouse-exited event may be generated. The cursor image is
//  moved.
//  This should be run from a command gate action.
//
void IOHIDSystem::setCursorPosition(IOGPoint * newLoc, bool external, OSObject * sender)
{
    if ( eventsOpen == true )
    {
        clock_get_uptime(&_cursorEventLast);
        _cursorHelper.desktopLocationDelta().xValue() += (newLoc->x - _cursorHelper.desktopLocation().xValue());
        _cursorHelper.desktopLocationDelta().yValue() += (newLoc->y - _cursorHelper.desktopLocation().yValue());
        _cursorHelper.desktopLocation().fromIntFloor(newLoc->x, newLoc->y);
        _setCursorPosition(external, false, sender);
        _cursorMoveLast = _cursorEventLast;
        scheduleNextPeriodicEvent();
    }
}

//
// This mechanism is used to update the cursor position, possibly generating
// messages to registered frame buffer devices and posting drag, tracking, and
// mouse motion events.
//
// This should be run from a command gate action.
// This can be called from setCursorPosition:(IOGPoint *)newLoc to set the
// position by a _IOSetParameterFromIntArray() call, directly from the absolute or
// relative pointer device routines, or on a timed event callback.
//

void IOHIDSystem::enableContinuousCursor()
{
    kprintf("%s called\n", __func__);
    
    _continuousCursor = true;
}

void IOHIDSystem::disableContinuousCursor()
{
    kprintf("%s called\n", __func__);
    
    _continuousCursor = false;
}

void IOHIDSystem::_onScreenCursorPin()
{
    IOGPoint p      = _cursorHelper.desktopLocation();
    int screen      = pointToScreen(&p);
    
    // check if current desktop location is in a screen with clipping bounds
    // and apply it
    
    require_quiet(screen != -1, exit);
    require_quiet((((1 << screen) & _onScreenPinMask) != 0), exit);

    _cursorHelper.desktopLocation().clipToRect(_onScreenBounds[screen]);
    
exit:
    return;
}

void IOHIDSystem::_setCursorPosition(bool external, bool proximityChange, OSObject * sender)
{
    bool cursorMoved = true;
    
    IOHID_DEBUG(kIOHIDDebugCode_SetCursorPosition, sender,
                _cursorHelper.desktopLocation().xValue().as64(),
                _cursorHelper.desktopLocation().yValue().as64(), 0);
    IOHID_DEBUG(kIOHIDDebugCode_SetCursorPosition, sender, proximityChange, external, 1);
    PROFILE_TRACE(9);

    if (!screens) {
        return;
    }

    if( OSSpinLockTry(&evg->cursorSema) == 0 ) { // host using shmem
        // try again later
        return;
    }

    // Past here we hold the cursorSema lock.  Make sure the lock is
    // cleared before returning or the system will be wedged.
    if (cursorCoupled || external)
    {
        UInt32 newScreens = 0;
        SInt32 pinScreen = -1L;
        EvScreen *screen = (EvScreen *)evScreen;

        bool foundFirstScreen = false;      // T => found first valid screen
        bool foundSecondScreen = false;     // T => found second valid screen
        int leftScreen = 0;
        int rightScreen = 0;

        if (cursorPinned) {
            _cursorHelper.desktopLocation().clipToRect(cursorPin);
        }
        
        else if (_onScreenPinMask != 0) {
            // If the cursor is on a screen with active on-screen pin bounds
            // clip it within those bounds
            _onScreenCursorPin();
        }
        
        else {
            /* Get mask of screens on which the cursor is present */
            for (int i = 0; i < EV_MAX_SCREENS; i++ ) {
                if (!screen[i].desktopBounds)
                    continue;
                if ((screen[i].desktopBounds->maxx - screen[i].desktopBounds->minx) < 128)
                    continue;
                if (_continuousCursor) {
                    if( !foundFirstScreen ) {
                        foundFirstScreen = true;
                        leftScreen = i;
                        rightScreen = i;
                    }
                    else if( !foundSecondScreen ) {
                        // I need this if I care about vertical stacks of displays
    //                    foundSecondScreen =
    //                            (screen[leftScreen].desktopBounds->minx != screen[i].desktopBounds->minx) ||
    //                            (screen[leftScreen].desktopBounds->maxx != screen[i].desktopBounds->maxx) ||
    //                            (screen[leftScreen].desktopBounds->miny != screen[i].desktopBounds->miny) ||
    //                            (screen[leftScreen].desktopBounds->maxy != screen[i].desktopBounds->maxy);
                    }

    //                if (screen[i].desktopBounds->minx < screen[leftScreen].desktopBounds->minx) {     // overlaps can torroid
                    if (screen[i].desktopBounds->maxx <= screen[leftScreen].desktopBounds->minx) {      // non-overlaps torroid
                        leftScreen = i;
                    }
    //                if (screen[rightScreen].desktopBounds->maxx < screen[i].desktopBounds->maxx )     // overlaps can torroid
                    if (screen[rightScreen].desktopBounds->maxx <= screen[i].desktopBounds->minx )  // non-overlaps torroid
                    {
                        rightScreen = i;
                    }
                }
                if (_cursorHelper.desktopLocation().inRect(*screen[i].desktopBounds)) {
                    pinScreen = i;
                    newScreens |= (1 << i);
                }
            }
        }

        if (newScreens == 0) {
            /* At this point cursor has gone off all screens,
               clip it to the closest screen. */
            IOFixedPoint64  aimLoc = _cursorHelper.desktopLocation();
            int64_t     dx;
            int64_t     dy;
            uint64_t    distance;
            uint64_t    bestDistance = -1ULL;
            
            const int64_t       kHackMiniumWrapDistance = 0; // 22 * 22;
            const int64_t       kHackUpperNoWrap = 40;       // Amount of space at the top that does not wrap
            const int64_t       kHackLowerNoWrap = 40;       // Amount of space at the bottom that does not wrap
                                                            // TODO: decide if menubar is special for wrap (and how)
            for (int i = 0; i < EV_MAX_SCREENS; i++ ) {
                if (!screen[i].desktopBounds)
                    continue;
                if ((screen[i].desktopBounds->maxx - screen[i].desktopBounds->minx) < 128)
                    continue;
                if (_continuousCursor && (screen[i].desktopBounds->maxy - screen[i].desktopBounds->miny) < 128)
                    continue;

                IOFixedPoint64  pinnedLoc = aimLoc;
                pinnedLoc.clipToRect(*screen[i].desktopBounds);
                dx = (pinnedLoc.xValue() - aimLoc.xValue()).as64();
                dy = (pinnedLoc.yValue() - aimLoc.yValue()).as64();
                distance = dx * dx + dy * dy;
                
                if (distance <= bestDistance) {
                    bestDistance = distance;
                    
                    if (_continuousCursor &&
                        leftScreen != rightScreen &&
                        screen[i].desktopBounds->miny + kHackUpperNoWrap < pinnedLoc.yValue().as64()  &&
                        pinnedLoc.yValue().as64() < screen[i].desktopBounds->maxy - kHackLowerNoWrap  &&
                        kHackMiniumWrapDistance < bestDistance ) {
                        
                        IOFixed64   targetX;
                        IOFixed64   targetY;
                        int         targetDisplay = i;
                        bool        shouldWrap = true;      // Assume we're wrapping (makes code below shorter)
                        
                        if (aimLoc.xValue().as64() < screen[leftScreen].desktopBounds->minx ) {
                            // Wrap to right side of right screen
                            targetX.fromIntFloor(screen[rightScreen].desktopBounds->maxx);
                            targetY.fromIntFloor(screen[rightScreen].desktopBounds->maxy - screen[rightScreen].desktopBounds->miny);
                            targetDisplay = rightScreen;
                            kprintf("IOHIDSystem::_setCursorPosition Wrap to right side of right screen\n");
                       }
                        else if (screen[rightScreen].desktopBounds->maxx < aimLoc.xValue().as64()) {
                            // Wrap to left side of left screen
                            targetX.fromIntFloor(screen[leftScreen].desktopBounds->minx);
                            targetY.fromIntFloor(screen[leftScreen].desktopBounds->maxy - screen[leftScreen].desktopBounds->miny);
                            targetDisplay = leftScreen;
                            kprintf("IOHIDSystem::_setCursorPosition Wrap to left side of left screen\n");
                       }
                        else {
                            shouldWrap = false;
                            kprintf("IOHIDSystem::_setCursorPosition don't wrap\n");
                        }

                        if (shouldWrap)
                        {
                            IOFixed64   closestYFraction;
                            closestYFraction.fromIntFloor(pinnedLoc.yValue().as64() - screen[i].desktopBounds->miny);
                            closestYFraction /= (screen[i].desktopBounds->maxy - screen[i].desktopBounds->miny);                            
                            targetY *= closestYFraction;
                            
                            pinnedLoc.fromFixed64(targetX, targetY);
                            pinnedLoc.clipToRect(*screen[targetDisplay].desktopBounds); // SHOULD NOT NEED - makes SURE we hit a screen

                        }
                    }
                    
                    _cursorHelper.desktopLocation() = pinnedLoc;
                }
            }
            IOHID_DEBUG(kIOHIDDebugCode_SetCursorPosition, sender,
                        _cursorHelper.desktopLocation().xValue().as64(),
                        _cursorHelper.desktopLocation().yValue().as64(), 2);

            /* regenerate mask for new position */
            for (int i = 0; i < EV_MAX_SCREENS; i++ ) {
                if (!screen[i].desktopBounds)
                    continue;
                if ((screen[i].desktopBounds->maxx - screen[i].desktopBounds->minx) < 128)
                    continue;
                if (_cursorHelper.desktopLocation().inRect(*screen[i].desktopBounds)) {
                    pinScreen = i;
                    newScreens |= (1 << i);
                }
            }
        }
        
        if ((newScreens != 0) && (_onScreenPinMask != 0)) {
            // We are on a new screen now, check if there an on-screen bounds to clip to
            _onScreenCursorPin();
        }
        
        /* Catch the no-move case */
        if ((_cursorHelper.desktopLocation().xValue().asFixed24x8() == evg->desktopCursorFixed.x) &&
            (_cursorHelper.desktopLocation().yValue().asFixed24x8() == evg->desktopCursorFixed.y) &&
            (proximityChange == 0) && (!_cursorHelper.desktopLocationDelta())) {
            cursorMoved = false;    // mouse moved, but cursor didn't
        }
        else {
            evg->cursorLoc.x = _cursorHelper.desktopLocation().xValue().as32();
            evg->cursorLoc.y = _cursorHelper.desktopLocation().yValue().as32();
            evg->desktopCursorFixed.x = _cursorHelper.desktopLocation().xValue().asFixed24x8();
            evg->desktopCursorFixed.y = _cursorHelper.desktopLocation().yValue().asFixed24x8();
            if (pinScreen >= 0) {
                _cursorHelper.updateScreenLocation(screen[pinScreen].desktopBounds, screen[pinScreen].displayBounds);
            }
            else {
                _cursorHelper.updateScreenLocation(NULL, NULL);
            }
            evg->screenCursorFixed.x = _cursorHelper.getScreenLocation().xValue().asFixed24x8();
            evg->screenCursorFixed.y = _cursorHelper.getScreenLocation().yValue().asFixed24x8();

            /* If cursor changed screens */
            if (newScreens != cursorScreens) {
                hideCursor();   /* hide cursor on old screens */
                cursorScreens = newScreens;
                if (pinScreen >= 0) {
                    cursorPin = *(((EvScreen*)evScreen)[pinScreen].desktopBounds);
                    cursorPinScreen = pinScreen;
                    showCursor();
                }
            } else {
                /* cursor moved on same screens */
                moveCursor();
            }
        }
    }
    else {
        /* cursor uncoupled */
        _cursorHelper.desktopLocation().xValue().fromFixed24x8(evg->desktopCursorFixed.x);
        _cursorHelper.desktopLocation().yValue().fromFixed24x8(evg->desktopCursorFixed.y);
    }

    AbsoluteTime    uptime;
    clock_get_uptime(&uptime);

    _cursorLog(uptime);

    /* See if anybody wants the mouse moved or dragged events */
    // Note: extPostEvent clears evg->movedMask as a hack to prevent these events
    // so any change here should check to make sure it does not break that hack
//    if (evg->movedMask) {
//        if      ((evg->movedMask & NX_LMOUSEDRAGGEDMASK) && (evg->buttons & EV_LB)) {
//            _postMouseMoveEvent(NX_LMOUSEDRAGGED, uptime, sender);
//        }
//        else if ((evg->movedMask & NX_RMOUSEDRAGGEDMASK) && (evg->buttons & EV_RB)) {
//            _postMouseMoveEvent(NX_RMOUSEDRAGGED, uptime, sender);
//        }
//        else if  (evg->movedMask & NX_MOUSEMOVEDMASK) {
//            _postMouseMoveEvent(NX_MOUSEMOVED, uptime, sender);
//        }
//    }

    /* check new cursor position for leaving evg->mouseRect */
    if (cursorMoved && evg->mouseRectValid && _cursorHelper.desktopLocation().inRect(evg->mouseRect))
    {
        if (evg->mouseRectValid)
        {
            postEvent(/* what */     NX_MOUSEEXITED,
                      /* at */       &_cursorHelper.desktopLocation(),
                      /* atTime */   uptime,
                      /* withData */ NULL,
                      /* sender */   sender);
            evg->mouseRectValid = 0;
        }
    }
    OSSpinLockUnlock(&evg->cursorSema);
    PROFILE_TRACE(10);
}



/**
 ** IOUserClient methods
 **/

IOReturn IOHIDSystem::newUserClient(task_t         owningTask,
                    /* withToken */ void *         security_id,
                    /* ofType */    UInt32         type,
                    /* withProps*/  OSDictionary *  properties,
                    /* client */    IOUserClient ** handler)
{
    IOHIDCmdGateActionArgs args;

    args.arg0 = &owningTask;
    args.arg1 = security_id;
    args.arg2 = &type;
    args.arg3 = properties;
    args.arg4 = handler;

    return cmdGate->runAction((IOCommandGate::Action)doNewUserClient, &args);
}

IOReturn IOHIDSystem::doNewUserClient(IOHIDSystem *self, void * args)
                        /* IOCommandGate::Action */
{
    task_t         owningTask   = *(task_t *) ((IOHIDCmdGateActionArgs *)args)->arg0;
    void *         security_id  = ((IOHIDCmdGateActionArgs *)args)->arg1;
    UInt32         type         = *(UInt32 *) ((IOHIDCmdGateActionArgs *)args)->arg2;
    OSDictionary * properties   = (OSDictionary *) ((IOHIDCmdGateActionArgs *)args)->arg3;
    IOUserClient ** handler     = (IOUserClient **) ((IOHIDCmdGateActionArgs *)args)->arg4;

    return self->newUserClientGated(owningTask, security_id, type, properties, handler);
}

IOReturn IOHIDSystem::newUserClientGated(task_t    owningTask,
                    /* withToken */ void *         security_id,
                    /* ofType */    UInt32         type,
                    /* withProps*/  OSDictionary *  properties,
                    /* client */    IOUserClient ** handler)
{
    IOUserClient * newConnect = 0;
    IOReturn  err = kIOReturnNoMemory;

    do {
        if (type == kIOHIDParamConnectType) {
            if (eventsOpen) {
                newConnect = new IOHIDParamUserClient;
            } else {
                err = kIOReturnNotOpen;
                break;
            }
        }
        else if ( type == kIOHIDServerConnectType) {
            newConnect = new IOHIDUserClient;
        }
//        else if ( type == kIOHIDStackShotConnectType ) {
//            newConnect = new IOHIDStackShotUserClient;
//        }
        else if ( type == kIOHIDEventSystemConnectType ) {
            newConnect = new IOHIDEventSystemUserClient;
        }
        else {
            err = kIOReturnUnsupported;
        }

        if ( !newConnect) {
            break;
        }

        // initialization is getting out of hand

        if ((false == newConnect->initWithTask(owningTask, security_id, type, properties))
            || (false == newConnect->setProperty(kIOUserClientCrossEndianCompatibleKey, kOSBooleanTrue))
            || (false == newConnect->attach( this ))
            || (false == newConnect->start( this ))
            || ((type == kIOHIDServerConnectType)
                && (err = evOpen()))) {
            newConnect->detach( this );
            newConnect->release();
            newConnect = 0;
            break;
        }

        err = kIOReturnSuccess;

    }
    while( false );

#ifdef DEBUG
    int             pid = -1;
    proc_t          p = (proc_t)get_bsdtask_info(owningTask);
    pid = proc_pid(p);
    HIDLog("(%d) %s returned %p", pid,
          type == kIOHIDParamConnectType ? "IOHIDParamUserClient" :
          type == kIOHIDServerConnectType ? "IOHIDUserClient" :
          //type == kIOHIDStackShotConnectType ? "IOHIDStackShotUserClient" :
          type == kIOHIDEventSystemConnectType ? "IOHIDEventSystemUserClient" :
          "kIOReturnUnsupported",
          newConnect);
#endif

    IOHID_DEBUG(kIOHIDDebugCode_NewUserClient, type, err, newConnect, 0);
    *handler = newConnect;
    return err;
}


IOReturn IOHIDSystem::setEventsEnable(void*p1 __unused,void*,void*,void*,void*,void*)
{                                                                    // IOMethod
    IOReturn ret = kIOReturnSuccess;

    if (mac_iokit_check_hid_control(kauth_cred_get()))
        return kIOReturnNotPermitted;

#if 0
    ret = cmdGate->runAction((IOCommandGate::Action)doSetEventsEnablePre, p1);
    if ( ret == kIOReturnSuccess ) {
        // reset outside gated context
        _resetMouseParameters();
    }
    ret = cmdGate->runAction((IOCommandGate::Action)doSetEventsEnablePost, p1);
#endif

    return ret;
}

#if 0
IOReturn IOHIDSystem::doSetEventsEnablePre(IOHIDSystem *self, void *p1)
                        /* IOCommandGate::Action */
{
    return self->setEventsEnablePreGated(p1);
}

IOReturn IOHIDSystem::setEventsEnablePreGated(void*p1)
{
    bool enable = (bool)p1;

    if( enable) {
        while ( evStateChanging )
            cmdGate->commandSleep(&evStateChanging);

        evStateChanging = true;
        //attachDefaultEventSources();
    }
    return( kIOReturnSuccess);
}

IOReturn IOHIDSystem::doSetEventsEnablePost(IOHIDSystem *self, void *p1)
                        /* IOCommandGate::Action */
{
    return self->setEventsEnablePostGated(p1);
}

IOReturn IOHIDSystem::setEventsEnablePostGated(void*p1)
{
    bool enable = (bool)p1;

    if( enable) {
        evStateChanging = false;
        cmdGate->commandWakeup(&evStateChanging);
    }
    return( kIOReturnSuccess);
}
#endif

IOReturn IOHIDSystem::setCursorEnable(void*p1,void*,void*,void*,void*,void*)
{                                                                    // IOMethod
    IOReturn ret;

    if (mac_iokit_check_hid_control(kauth_cred_get()))
        return kIOReturnNotPermitted;

    ret = cmdGate->runAction((IOCommandGate::Action)doSetCursorEnable, p1);

    return ret;
}

IOReturn IOHIDSystem::doSetCursorEnable(IOHIDSystem *self, void * arg0)
                        /* IOCommandGate::Action */
{
    return self->setCursorEnableGated(arg0);
}

IOReturn IOHIDSystem::setCursorEnableGated(void* p1)
{
    bool        enable = (bool)(intptr_t)p1;

    if ( !eventsOpen ) {
        return kIOReturnNotOpen;
    }

    if( !screens) {
        return kIOReturnNoDevice;
    }

    if( enable ) {
        if( cursorStarted) {
            hideCursor();
            cursorEnabled = resetCursor();
            showCursor();
        }
        else {
            cursorEnabled = startCursor();
        }
    }
    else {
        cursorEnabled = enable;
    }

    if (cursorCoupled != cursorEnabled) {
        _periodicEventNext = kIOHIDSystenDistantFuture;
        _cursorMoveDelta = 0;
        _cursorHelper.desktopLocationPosting().fromIntFloor(0, 0);
        _cursorHelper.clearEventCounts();
        _cursorLogTimed();
        scheduleNextPeriodicEvent();
        cursorCoupled = cursorEnabled;
    }

    return kIOReturnSuccess;
}

IOReturn IOHIDSystem::setContinuousCursorEnable(void*p1,void*,void*,void*,void*,void*)
{                                                                    // IOMethod
    kprintf("%s called\n", __func__);
    
    if (mac_iokit_check_hid_control(kauth_cred_get()))
        return kIOReturnNotPermitted;

    return cmdGate->runAction((IOCommandGate::Action)doSetContinuousCursorEnable, p1);

}

IOReturn IOHIDSystem::doSetContinuousCursorEnable(IOHIDSystem *self, void * arg0)
                        /* IOCommandGate::Action */
{
    kprintf("%s called\n", __func__);
    
    return self->setContinuousCursorEnableGated(arg0);
}

IOReturn IOHIDSystem::setContinuousCursorEnableGated(void* p1)
{
    bool        enable = (bool)(intptr_t)p1;

    kprintf("%s called\n", __func__);
    
    if ( !eventsOpen ) {
        return kIOReturnNotOpen;
    }

    if( !screens) {
        return kIOReturnNoDevice;
    }

    if( enable )
        enableContinuousCursor();
    else
        disableContinuousCursor();

    return kIOReturnSuccess;
}

IOReturn IOHIDSystem::setBounds( IOGBounds * bounds, IOGPoint * screenPoint, bool onScreen )
{
    int         screen          = -1;
    IOReturn    status          = kIOReturnBadArgument;
    UInt32      screenMask      = 0;
    
    if (mac_iokit_check_hid_control(kauth_cred_get()))
        return kIOReturnNotPermitted;
    
    if (onScreen) {
        require(bounds, exit);
        require(screenPoint, exit);
  
        screen = pointToScreen(screenPoint);
        require(screen != -1, exit);
        
        screenMask = (1 << screen);

        if( bounds->minx != bounds->maxx) {
            _onScreenBounds[screen] = *bounds;
            _onScreenPinMask |= screenMask;
        } else {
            _onScreenBounds[screen].maxx = 0;
            _onScreenBounds[screen].maxy = 0;
            _onScreenPinMask &= ~screenMask;
        }
    }
    else {
        if( bounds->minx != bounds->maxx) {
            cursorPin = *bounds;
            cursorPinned = true;
        } else {
            cursorPinned = false;
        }
    }
    
    status = kIOReturnSuccess;
    
exit:
    return( status );
}

IOReturn IOHIDSystem::extSetBounds(void* param,void*,void*,void*,void*,void*)
{
    IOGBounds * bounds = (IOGBounds *) param;
    return setBounds(bounds, NULL, false);
}

IOReturn IOHIDSystem::extSetOnScreenBounds(void * param, void*, void*, void*, void*, void*)
{
    u_int16_t * data = (u_int16_t *)param;
    
    IOGPoint  point  = {.x = (SInt16)data[0], .y = (SInt16)data[1]};
    IOGBounds bounds = {.minx = (SInt16)data[2], .miny = (SInt16)data[3], .maxx = (SInt16)data[4], .maxy = (SInt16)data[5]};
    
    return setBounds(&bounds, &point, true);
}

IOReturn IOHIDSystem::extPostEvent(void*p1,void*p2,void*,void*,void*,void*)
{                                                                    // IOMethod
    AbsoluteTime    ts;

    clock_get_uptime(&ts);

    return cmdGate->runAction((IOCommandGate::Action)doExtPostEvent, p1, p2, &ts);
}

IOReturn IOHIDSystem::doExtPostEvent(IOHIDSystem *self, void * arg0, void * arg1, void * arg2, void * arg3 __unused)
                        /* IOCommandGate::Action */
{
    return self->extPostEventGated(arg0, arg1, arg2);
}

IOReturn IOHIDSystem::extPostEventGated(void *p1,void *p2 __unused, void *p3)
{
    struct evioLLEvent * event      = (struct evioLLEvent *)p1;
//    bool        isMoveOrDragEvent   = false;
    bool        isSeized            = false;
//    int         oldMovedMask        = 0;
    UInt32      buttonState         = 0;
    UInt32      newFlags            = 0;
    AbsoluteTime ts                 = *(AbsoluteTime *)p3;
    //CachedMouseEventStruct  *cachedMouseEvent = NULL;
    UInt32      typeMask            = EventCodeMask(event->type);
    UInt32      options             = 0;
    // rdar://problem/8689199
    int         extPID              = proc_selfpid();

    IOHID_DEBUG(kIOHIDDebugCode_ExtPostEvent, event->type, *(UInt32*)&(event->location), event->setFlags, event->flags);


    if (event->type != NX_NULLEVENT && mac_iokit_check_hid_control(kauth_cred_get()))
        return kIOReturnNotPermitted;

    if ( eventsOpen == false )
        return kIOReturnNotOpen;

#if 0
    if (ShouldConsumeHIDEvent(ts, rootDomainStateChangeDeadline, false)) {
        if (typeMask & NX_WAKEMASK) {
            TICKLE_DISPLAY(event->type);
        }
        return kIOReturnSuccess;
    }
#endif

    if (!DISPLAY_IS_ENABLED) {
#if !WAKE_DISPLAY_ON_MOVEMENT
        if ( (typeMask & NX_WAKEMASK) ||
           ((typeMask & MOVEDEVENTMASK) && (CMP_ABSOLUTETIME(&ts, &displaySleepWakeupDeadline) <= 0)) )
#endif
        {
            TICKLE_DISPLAY(event->type);
        }
        return kIOReturnSuccess;
    }

    TICKLE_DISPLAY(event->type);

    // used in set cursor below
    if (typeMask & MOVEDEVENTMASK)
    {
        //isMoveOrDragEvent = true;

        // We have mouse move event without a specified pressure value and an embedded tablet event
        // We need to scale the tablet pressure to fit in mouseMove pressure
        if ((event->data.mouseMove.subType == NX_SUBTYPE_TABLET_POINT) && (event->data.mouseMove.reserved1 == 0))
        {
            event->data.mouseMove.reserved1 = ScalePressure(event->data.mouseMove.tablet.point.pressure);
        }
    }
    // We have mouse event without a specified pressure value and an embedded tablet event
    // We need to scale the tablet pressure to fit in mouse pressure
    else if ((typeMask & MOUSEEVENTMASK) &&
            (event->data.mouse.subType == NX_SUBTYPE_TABLET_POINT) && (event->data.mouse.pressure == 0))
    {
        event->data.mouse.pressure = ScalePressure(event->data.mouse.tablet.point.pressure);
    }
   
    if( !(event->setCursor & kIOHIDSetCursorPosition)) {
        options = NX_EVENT_EXTENSION_LOCATION_INVALID;
    }

    if( event->setFlags & kIOHIDSetGlobalEventFlags)
    {
        newFlags = evg->eventFlags = (evg->eventFlags & ~KEYBOARD_FLAGSMASK)
                        | (event->flags & KEYBOARD_FLAGSMASK);
    }

    if ( event->setFlags & kIOHIDPostHIDManagerEvent )
    {
#ifdef NEW_HID
        if ((typeMask & (MOUSEEVENTMASK | MOVEDEVENTMASK | NX_SCROLLWHEELMOVEDMASK)) &&
            (_hidPointingDevice || (_hidPointingDevice = IOHIDPointingDevice::newPointingDeviceAndStart(this, 8, 400, true, 2))))
        {
            SInt32  dx = 0;
            SInt32  dy = 0;
            SInt32  wheel = 0;
            buttonState = 0;

            if (typeMask & MOVEDEVENTMASK)
            {
                dx = event->data.mouseMove.dx;
                dy = event->data.mouseMove.dy;
            }
            else if ( event->type == NX_SCROLLWHEELMOVED )
            {
                wheel = event->data.scrollWheel.deltaAxis1;
            }

            _hidPointingDevice->postMouseEvent(buttonState, dx, dy, wheel);
            isSeized |= _hidPointingDevice->isSeized();
        }

        if ((typeMask & (NX_KEYDOWNMASK | NX_KEYUPMASK | NX_FLAGSCHANGEDMASK)) &&
            (_hidKeyboardDevice || (_hidKeyboardDevice = IOHIDKeyboardDevice::newKeyboardDeviceAndStart(this, 1))))
        {
            _hidKeyboardDevice->postFlagKeyboardEvent(newFlags & KEYBOARD_FLAGSMASK);

            if ((event->type != NX_FLAGSCHANGED) && (event->data.key.repeat == 0))
            {
                _hidKeyboardDevice->postKeyboardEvent(event->data.key.keyCode, (event->type == NX_KEYDOWN));
            }

            isSeized |= _hidKeyboardDevice->isSeized();
        }
#endif // NEW_HID
    }

    if ( !isSeized )
    {
        IOFixedPoint64 location;
        location = location.fromIntFloor(event->location.x, event->location.y);
        postEvent(             event->type,
                /* at */       &location,
                /* atTime */   ts,
                /* withData */ &event->data,
                /* sender */   0,
                /* extPID */   extPID,
                /*processKEQ*/ true,
                /* options*/   options);
    }

    scheduleNextPeriodicEvent();

    return kIOReturnSuccess;
}


IOReturn IOHIDSystem::extSetMouseLocation(void*p1,void*p2,void*,void*,void*,void*)
{                                                                    // IOMethod
    IOReturn ret;

    if (mac_iokit_check_hid_control(kauth_cred_get()))
        return kIOReturnNotPermitted;
    if (sizeof(SetFixedMouseLocData) != (intptr_t)p2) {
        HIDLogError("called with inappropriate data size: %d", (int)(intptr_t)p2);
        return kIOReturnBadArgument;
    }

    ret = cmdGate->runAction((IOCommandGate::Action)doExtSetMouseLocation, p1);

    return ret;
}

IOReturn IOHIDSystem::doExtSetMouseLocation(IOHIDSystem *self, void * arg0)
                        /* IOCommandGate::Action */
{
    return self->extSetMouseLocationGated(arg0);
}

IOReturn IOHIDSystem::extSetMouseLocationGated(void *p1)
{
    SetFixedMouseLocData    *data = (SetFixedMouseLocData *)p1;
    IOFixedPoint32          loc;

    require(data, exit);

    _diags.cursorWorkloopTime = mach_absolute_time();
    _diags.lastCursorActionsMask = 0;

    loc.x = data->x;
    loc.y = data->y;

    IOHID_DEBUG(kIOHIDDebugCode_ExtSetLocation, loc.x, loc.y, &loc, 0);

    if ( eventsOpen == true )
    {
        _cursorHelper.desktopLocationDelta() += loc;
        _cursorHelper.desktopLocationDelta() -= _cursorHelper.desktopLocation();
        _cursorHelper.desktopLocation() = loc;
        _setCursorPosition(true);
    }

    if (data->origTs && data->callTs) {
        _recordCursorAction(data->origTs, data->callTs);
    }

exit:
    return kIOReturnSuccess;
}

IOReturn IOHIDSystem::extGetStateForSelector(void*p1,void*p2,void*,void*,void*,void*)
{                                                                    // IOMethod
    return cmdGate->runAction((IOCommandGate::Action)doExtGetStateForSelector, p1, p2);
}

IOReturn IOHIDSystem::extSetStateForSelector(void*p1,void*p2,void*,void*,void*,void*)
{                                                                    // IOMethod
    if (mac_iokit_check_hid_control(kauth_cred_get()))
        return kIOReturnNotPermitted;

    return cmdGate->runAction((IOCommandGate::Action)doExtSetStateForSelector, p1, p2);
}

IOReturn IOHIDSystem::doExtGetStateForSelector(IOHIDSystem *self, void *p1, void *p2)
/* IOCommandGate::Action */
{
    IOReturn result = kIOReturnSuccess;
    unsigned int selector = (unsigned int)(uintptr_t)p1;
    unsigned int *state_O = (unsigned int*)p2;
    switch (selector) {
        case kIOHIDActivityUserIdle:
            *state_O = self->_privateData->hidActivityIdle ? 1 : 0;
            break;
            
        case kIOHIDActivityDisplayOn:
            *state_O = self->displayState & IOPMDeviceUsable ? 1 : 0;
            break;
        case kIOHIDCapsLockState:
            *state_O = (self->eventFlags() & NX_ALPHASHIFTMASK) ? 1 : 0;
            break;
        case kIOHIDNumLockState:
            break;
        default:
            HIDLogError("recieved unexpected selector: %d", selector);
            result = kIOReturnBadArgument;
            break;
    }
    return result;
}

IOReturn IOHIDSystem::doExtSetStateForSelector(IOHIDSystem *self __unused, void *p1, void *p2 __unused)
/* IOCommandGate::Action */
{
    IOReturn result = kIOReturnSuccess;
    unsigned int selector = (unsigned int)(uintptr_t)p1;
    
    switch (selector) {
        case kIOHIDActivityDisplayOn:   // not settable
        default:
            HIDLogError("recieved unexpected selector: %d", selector);
            result = kIOReturnBadArgument;
            break;
            
    }
    return result;
}


IOReturn IOHIDSystem::extGetButtonEventNum(void*p1,void*p2,void*,void*,void*,void*)
{                                                                   // IOMethod
    return cmdGate->runAction((IOCommandGate::Action)doExtGetButtonEventNum, p1, p2);
}

IOReturn IOHIDSystem::doExtGetButtonEventNum(IOHIDSystem *self, void * arg0, void * arg1)
                        /* IOCommandGate::Action */
{
    return self->extGetButtonEventNumGated(arg0, arg1);
}

IOReturn IOHIDSystem::extGetButtonEventNumGated(void *p1 __unused, void* p2 __unused)
{

    return kIOReturnUnsupported;
}

void IOHIDSystem::makeNumberParamProperty( OSDictionary * dict, const char * key,
                            unsigned long long number, unsigned int bits )
{
    OSNumber *  numberRef;
    numberRef = OSNumber::withNumber(number, bits);

    if( numberRef) {
        dict->setObject( key, numberRef);
        numberRef->release();
    }
}

void IOHIDSystem::makeInt32ArrayParamProperty( OSDictionary * dict, const char * key,
                            UInt32 * intArray, unsigned int count )
{
    OSArray *   array;
    OSNumber *  number;

    array = OSArray::withCapacity(count);

    if ( !array )
        return;

    for (unsigned i=0; i<count; i++)
    {
        number = OSNumber::withNumber(intArray[i], sizeof(UInt32) << 3);
        if (number)
        {
            array->setObject(number);
            number->release();
        }
    }

    dict->setObject( key, array);
    array->release();
}

void IOHIDSystem::createParameters( void )
{
    UInt64  nano;
    IOFixed fixed;
    UInt32  int32;

    savedParameters->setObject(kIOHIDDefaultParametersKey, kOSBooleanTrue);

    nano = EV_DCLICKTIME;
    makeNumberParamProperty( savedParameters, kIOHIDClickTimeKey,
                nano, 64 );

    UInt32  tempClickSpace[] = {(UInt32)clickSpaceThresh.x, (UInt32)clickSpaceThresh.y};
    makeInt32ArrayParamProperty( savedParameters, kIOHIDClickSpaceKey,
                tempClickSpace, sizeof(tempClickSpace)/sizeof(UInt32) );

    nano = EV_DEFAULTKEYREPEAT;
    makeNumberParamProperty( savedParameters, kIOHIDKeyRepeatKey,
                nano, 64 );
    nano = EV_DEFAULTINITIALREPEAT;
    makeNumberParamProperty( savedParameters, kIOHIDInitialKeyRepeatKey,
                nano, 64 );

    fixed = EV_DEFAULTPOINTERACCELLEVEL;
    makeNumberParamProperty( savedParameters, kIOHIDPointerAccelerationKey,
                fixed, sizeof(fixed) << 3);
    
    makeNumberParamProperty( savedParameters, kIOHIDMouseAccelerationType,
                fixed, sizeof(fixed) << 3);
    
    makeNumberParamProperty( savedParameters, kIOHIDTrackpadAccelerationType,
                fixed, sizeof(fixed) << 3);

    fixed = EV_DEFAULTSCROLLACCELLEVEL;
    makeNumberParamProperty( savedParameters, kIOHIDScrollAccelerationKey,
                fixed, sizeof(fixed) << 3);
    
    makeNumberParamProperty( savedParameters, kIOHIDMouseScrollAccelerationKey,
                fixed, sizeof(fixed) << 3);
    
    makeNumberParamProperty( savedParameters, kIOHIDTrackpadScrollAccelerationKey,
                fixed, sizeof(fixed) << 3);

    fixed = kIOHIDButtonMode_EnableRightClick;
    makeNumberParamProperty( savedParameters, kIOHIDPointerButtonMode,
                fixed, sizeof(fixed) << 3);

    // set eject delay properties
    int32 = kEjectF12DelayMS;
    makeNumberParamProperty( savedParameters, kIOHIDF12EjectDelayKey,
                            int32, 32 );
    int32 = kEjectKeyDelayMS;
    makeNumberParamProperty( savedParameters, kIOHIDKeyboardEjectDelay,
                            int32, 32 );

    // set slow keys delay property
    int32 = 0;
    makeNumberParamProperty( savedParameters, kIOHIDSlowKeysDelayKey,
                int32, 32 );

    // set disabled property
    int32 = 0; // not disabled
    makeNumberParamProperty( savedParameters, kIOHIDStickyKeysDisabledKey,
                int32, 32 );

    // set on/off property
    int32 = 0; // off
    makeNumberParamProperty( savedParameters, kIOHIDStickyKeysOnKey,
                int32, 32 );

    // set shift toggles property
    int32 = 0; // off, shift does not toggle
    makeNumberParamProperty( savedParameters, kIOHIDStickyKeysShiftTogglesKey,
                int32, 32 );

    // set option toggles property
    int32 = 0; // off, option does not toggle
    makeNumberParamProperty( savedParameters, kIOHIDMouseKeysOptionTogglesKey,
                int32, 32 );

    int32 = 0;
    makeNumberParamProperty( savedParameters, kIOHIDFKeyModeKey,
                            int32, 32 );

    setProperty( kIOHIDParametersKey, savedParameters );
    savedParameters->release();

    // RY: Set up idleTimeSerializer.  This should generate the
    // current idle time when requested.
    OSSerializer * idleTimeSerializer = OSSerializer::forTarget(this, IOHIDSystem::_idleTimeSerializerCallback);
    
    if (idleTimeSerializer)
    {
        setProperty( kIOHIDIdleTimeKey, idleTimeSerializer);
        idleTimeSerializer->release();
    }
    
    OSSerializer * cursorStateSerializer = OSSerializer::forTarget(this, IOHIDSystem::_cursorStateSerializerCallback);
    
    if (cursorStateSerializer)
    {
        setProperty("CursorState", cursorStateSerializer);
        cursorStateSerializer->release();
    }

#if 0
    OSSerializer * displaySerializer = OSSerializer::forTarget(this, IOHIDSystem::_displaySerializerCallback);

    if (displaySerializer)
    {
        setProperty("DisplayState", displaySerializer);
        displaySerializer->release();
    }
#endif
}

bool IOHIDSystem::_idleTimeSerializerCallback(void * target, void * ref __unused, OSSerialize *s)
{
    IOHIDSystem *   self = (IOHIDSystem *) target;
    AbsoluteTime    currentTime;
    OSNumber *      number;
    UInt64          idleTimeNano = 0;
    bool            retValue = false;
    
    if( self->eventsOpen )
    {
        clock_get_uptime( &currentTime);
        SUB_ABSOLUTETIME( &currentTime, &(self->lastUndimEvent));
        absolutetime_to_nanoseconds( currentTime, &idleTimeNano);
    }
    
    number = OSNumber::withNumber(idleTimeNano, 64);
    if (number)
    {
        retValue = number->serialize( s );
        number->release();
    }
    
    return retValue;
}

static inline OSString * __newTimeDiffString(uint64_t absFirst, uint64_t absSecond)
{
    char        valBuf[128];
    uint64_t    ns;
    uint64_t    ns2;

    if (absFirst == 0 || absSecond == 0) {
        return NULL;
    }

    absolutetime_to_nanoseconds(absFirst, &ns);
    absolutetime_to_nanoseconds(absSecond, &ns2);

    ns = ns2 - ns;
    snprintf(valBuf, sizeof(valBuf), "%u.%06u", (unsigned int)(ns/NSEC_PER_SEC), (unsigned int)((ns/NSEC_PER_USEC)%USEC_PER_SEC));

    return OSString::withCString(valBuf);
}

bool IOHIDSystem::_cursorStateSerializerCallback(void * target, void * ref __unused, OSSerialize *s)
{
    IOHIDSystem *   self = (IOHIDSystem *) target;
    OSDictionary *  cursorDict = OSDictionary::withCapacity(3);
    bool            retValue = false;
    
    require(cursorDict, exit);

    for (size_t i = 0; i < Diags::kCursorActionCount; i++) {
        OSString *  string;
        char        keyBuf[128];
        uint64_t    absNow = mach_absolute_time();

        string = __newTimeDiffString(self->_diags.lastActionTimes[i], absNow);
        if (string) {
            strlcpy(keyBuf, "Last", sizeof(keyBuf));
            strlcat(keyBuf, Diags::cursorStrings[i], sizeof(keyBuf));
            strlcat(keyBuf, " (Seconds ago)", sizeof(keyBuf));
            cursorDict->setObject(keyBuf, string);
            OSSafeReleaseNULL(string);
        }
    }
    
    retValue = cursorDict->serialize(s);

exit:
    OSSafeReleaseNULL(cursorDict);
    return retValue;
}

bool IOHIDSystem::_displaySerializerCallback(void * target, void * ref __unused, OSSerialize *s)
{
    IOHIDSystem     *self = (IOHIDSystem *) target;
    bool            retValue = false;
    OSDictionary    *mainDict = OSDictionary::withCapacity(4);
    require(mainDict, exit_early);

#define IfNotNullAddNumToDictWithKey(x,y, w,z) \
    if (x) { \
        OSNumber *num = NULL; \
        num = OSNumber::withNumber(y, 8*sizeof(y)); \
        if (num) { \
            w->setObject(z, num); \
            num->release(); \
        } \
    }

    for(int i = 0; i < self->screens; i++) {
        EvScreen &esp = ((EvScreen*)(self->evScreen))[i];
        OSDictionary    *thisDisplay = OSDictionary::withCapacity(4);
        char            key[256];

        require(thisDisplay, next_display);
        snprintf(key, sizeof(key), "%d", i);
        mainDict->setObject(key, thisDisplay);

        IfNotNullAddNumToDictWithKey(esp.instance, esp.instance->getRegistryEntryID(), thisDisplay, "io_fb_id");
        IfNotNullAddNumToDictWithKey(esp.displayBounds, esp.displayBounds->minx, thisDisplay, "disp_min_x");
        IfNotNullAddNumToDictWithKey(esp.displayBounds, esp.displayBounds->maxx, thisDisplay, "disp_max_x");
        IfNotNullAddNumToDictWithKey(esp.displayBounds, esp.displayBounds->miny, thisDisplay, "disp_min_y");
        IfNotNullAddNumToDictWithKey(esp.displayBounds, esp.displayBounds->maxy, thisDisplay, "disp_max_y");
        IfNotNullAddNumToDictWithKey(esp.desktopBounds, esp.desktopBounds->minx, thisDisplay, "desk_min_x");
        IfNotNullAddNumToDictWithKey(esp.desktopBounds, esp.desktopBounds->maxx, thisDisplay, "desk_max_x");
        IfNotNullAddNumToDictWithKey(esp.desktopBounds, esp.desktopBounds->miny, thisDisplay, "desk_min_y");
        IfNotNullAddNumToDictWithKey(esp.desktopBounds, esp.desktopBounds->maxy, thisDisplay, "desk_max_y");
        IfNotNullAddNumToDictWithKey(esp.creator_pid, esp.creator_pid, thisDisplay, "creator_pid");

    next_display:
        OSSafeReleaseNULL(thisDisplay);
    }

    {   // safety scoping
        OSDictionary    *workSpaceDict = OSDictionary::withCapacity(4);
        IfNotNullAddNumToDictWithKey(workSpaceDict, self->workSpace.minx, workSpaceDict, "minx");
        IfNotNullAddNumToDictWithKey(workSpaceDict, self->workSpace.miny, workSpaceDict, "miny");
        IfNotNullAddNumToDictWithKey(workSpaceDict, self->workSpace.maxx, workSpaceDict, "maxx");
        IfNotNullAddNumToDictWithKey(workSpaceDict, self->workSpace.maxy, workSpaceDict, "maxy");
        mainDict->setObject("workspace", workSpaceDict ? (OSObject*)workSpaceDict : (OSObject*)kOSBooleanFalse);
        OSSafeReleaseNULL(workSpaceDict);
    }

    {   // safety scoping
        OSDictionary    *cursorPinDict = OSDictionary::withCapacity(4);
        IfNotNullAddNumToDictWithKey(cursorPinDict, self->cursorPin.minx, cursorPinDict, "minx");
        IfNotNullAddNumToDictWithKey(cursorPinDict, self->cursorPin.miny, cursorPinDict, "miny");
        IfNotNullAddNumToDictWithKey(cursorPinDict, self->cursorPin.maxx, cursorPinDict, "maxx");
        IfNotNullAddNumToDictWithKey(cursorPinDict, self->cursorPin.maxy, cursorPinDict, "maxy");
        mainDict->setObject("cursorPin", cursorPinDict ? (OSObject*)cursorPinDict : (OSObject*)kOSBooleanFalse);
        OSSafeReleaseNULL(cursorPinDict);
    }

    retValue = mainDict->serialize( s );

exit_early:
    OSSafeReleaseNULL(mainDict);
    return retValue;
}

IOReturn IOHIDSystem::_recordCursorAction(uint64_t origTs, uint64_t callTs)
{
    char actionBuf[128] = {0};
    uint64_t nowNs;
    uint64_t workloopNs;
    // HID/WS latency
    unsigned long long callDelta;
    // Latency to get on IOHIDSystem workloop
    unsigned long long workloopDelta;
    // latency up to cursor actions
    unsigned long long actionDeltas[Diags::kCursorActionCount] = {0};
    // total latency
    unsigned long long totalDelta;
    IOReturn ret = kIOReturnBadArgument;


    require(origTs && callTs > origTs, exit);

    absolutetime_to_nanoseconds(origTs, &origTs);
    absolutetime_to_nanoseconds(_diags.cursorWorkloopTime, &workloopNs);
    absolutetime_to_nanoseconds(callTs, &callTs);
    absolutetime_to_nanoseconds(mach_absolute_time(), &nowNs);

    callDelta = (callTs - origTs) / NSEC_PER_USEC;
    workloopDelta = (workloopNs - origTs) / NSEC_PER_USEC;
    totalDelta = (nowNs - origTs) / NSEC_PER_USEC;

    if (_diags.cursorTotalHistReporter)
        require(_diags.cursorTotalHistReporter->tallyValue((int64_t)totalDelta) != -1, exit);

    for (size_t i = 0; i < Diags::kCursorActionCount; i++) {
        if (_diags.lastCursorActionsMask & (1 << i)) {
            int n;
            uint64_t actionNs;
            absolutetime_to_nanoseconds(_diags.lastActionTimes[i], &actionNs);
            require(actionNs > origTs, exit);

            actionDeltas[i] = (actionNs - origTs) / NSEC_PER_USEC;

            if (_diags.cursorGraphicsHistReporter)
                require(_diags.cursorGraphicsHistReporter->tallyValue((int64_t)actionDeltas[i]) != -1, exit);

            n = snprintf(actionBuf+strlen(actionBuf), sizeof(actionBuf)-strlen(actionBuf), "%s(us) %llu ", Diags::cursorStrings[i], actionDeltas[i]);
            require_action(n > 0 && n < sizeof(actionBuf)-strlen(actionBuf), exit, ret = kIOReturnOverrun);
        }
    }

    //HIDLogInfo("Cursor latency to call(us) %llu workloop(us) %llu %stotal(us) %llu", callDelta, workloopDelta, actionBuf, totalDelta);

    ret = kIOReturnSuccess;

exit:
    return ret;
}

IOReturn IOHIDSystem::setProperties( OSObject * properties )
{
    OSDictionary *  dict;
    IOReturn        ret = kIOReturnSuccess;
  
    dict = OSDynamicCast( OSDictionary, properties );
    if( dict) {
        
        OSObject *userIdleState = dict->getObject(kIOHIDActivityUserIdleKey);
        if (userIdleState) {
            OSNumber *  num;
            uint32_t    state_I = IOHID_DEBUG_CODE(0);
            
            if ((num = OSDynamicCast( OSNumber, userIdleState))) {
                state_I = num->unsigned32BitValue();
                if (state_I == 0) { //  activity report
                    clock_get_uptime(&lastUndimEvent);
                    _privateData->hidActivityIdle = false;
                } else if (state_I == 1) {
                    _privateData->hidActivityIdle = true;
                } else {
                    ret = kIOReturnBadArgument;
                }
            } else {
                ret = kIOReturnBadArgument;
            }
            return ret;
        }

        OSObject *tickleType =  dict->getObject("DisplayTickle");
        if (tickleType) {
            OSNumber *  num;
            uint32_t    type = IOHID_DEBUG_CODE(0);
            if ((num = OSDynamicCast( OSNumber, tickleType))) {
                type = num->unsigned32BitValue();
            }
            if (displayManager) {
                displayManager->activityTickle(type);
            }
            return ret;
        }
        OSNumber *modifiersValue =  OSDynamicCast( OSNumber, dict->getObject(kIOHIDKeyboardGlobalModifiersKey));
        if (modifiersValue) {
            updateEventFlags (modifiersValue->unsigned32BitValue());
            return ret;
        }
        OSDictionary *paramDict = OSDynamicCast( OSDictionary, dict->getObject(kIOHIDParametersKey));
        if (paramDict) {
          dict = paramDict;
        }
        ret = setParamProperties( dict );
    
    } else
    {
        ret = kIOReturnBadArgument;
    }
    return ret;
}

IOReturn IOHIDSystem::setParamProperties( OSDictionary * dict )
{
    OSIterator *    iter    = NULL;
    IOReturn        ret     = kIOReturnSuccess;
    IOReturn        err     = kIOReturnSuccess;

    // Tip off devices that these are default parameters
    dict->setObject(kIOHIDDefaultParametersKey, kOSBooleanTrue);

    ret = cmdGate->runAction((IOCommandGate::Action)doSetParamPropertiesPre, dict, &iter);

    if ( ret == kIOReturnSuccess ) {

        // Do the following down calls outside of the gate
        if( iter) {
            IOService *     eventSrc;
            OSDictionary *  validParameters;
            while( (eventSrc = (IOService *) iter->getNextObject())) {

#if NEW_HID
                if ( OSDynamicCast(IOHIDKeyboard, eventSrc) || OSDynamicCast(IOHIDPointing, eventSrc) || OSDynamicCast(IOHIDConsumer, eventSrc)) {
                  ((IOHIDevice *)eventSrc)->setParamProperties( dict);
                    continue;
                }
#endif

                // Use valid parameters per device.  Basically if the IOHIDevice has a given property
                // in its registery we should NOT push it down via setParamProperties as properties
                // generated via the global IOHIDSystem::setParamProperties are defaults.
                validParameters = createFilteredParamPropertiesForService(eventSrc, dict);
                if ( validParameters ) {

                    if ( OSDynamicCast(IOHIDevice, eventSrc) )
                        ret = ((IOHIDevice *)eventSrc)->setParamProperties( validParameters );
#ifdef NEW_HID
                    else if ( OSDynamicCast( IOHIDEventService, eventSrc ) )
                        ret = ((IOHIDEventService *)eventSrc)->setSystemProperties( validParameters );
#endif
                    if( (ret != kIOReturnSuccess) && (ret != kIOReturnBadArgument))
                        err = ret;

                    dict->merge(validParameters);

                    validParameters->release();
                }
            }
            iter->release();
        }

        // Grab the gate again.
        cmdGate->runAction((IOCommandGate::Action)doSetParamPropertiesPost, dict);
    }

    return err;
}

OSDictionary * IOHIDSystem::createFilteredParamPropertiesForService(IOService * service, OSDictionary * dict)
{
    OSDictionary * validParameters = OSDictionary::withCapacity(4);

    if ( !validParameters )
        return NULL;

    OSDictionary * deviceParameters = NULL;

    if ( OSDynamicCast(IOHIDevice, service) ) {
        deviceParameters = (OSDictionary*)service->copyProperty(kIOHIDParametersKey);
    }

    if (!OSDynamicCast(OSDictionary, deviceParameters))
        OSSafeReleaseNULL(deviceParameters);

    OSCollectionIterator * iterator = OSCollectionIterator::withCollection(dict);
    if ( iterator ) {
        bool done = false;
        while (!done) {
            OSSymbol * key = NULL;
            while ((key = (OSSymbol*)iterator->getNextObject()) != NULL) {
                if ( !deviceParameters || !deviceParameters->getObject(key) ) {
                    validParameters->setObject(key, dict->getObject(key));
                }
            }
            if (iterator->isValid()) {
                done = true;
            }
            else {
                iterator->reset();
                validParameters->flushCollection();
            }
        }
        iterator->release();
    }

    if ( validParameters->getCount() == 0 ) {
        validParameters->release();
        validParameters = NULL;
    }
    else {
        validParameters->setObject(kIOHIDDefaultParametersKey, kOSBooleanTrue);
    }

    return validParameters;
}


IOReturn IOHIDSystem::doSetParamPropertiesPre(IOHIDSystem *self, void * arg0, void * arg1)
                        /* IOCommandGate::Action */
{
    return self->setParamPropertiesPreGated((OSDictionary *)arg0, (OSIterator**)arg1);
}

IOReturn IOHIDSystem::setParamPropertiesPreGated( OSDictionary * dict, OSIterator ** pOpenIter)
{
    OSNumber *  number;

    // check for null
    if (dict == NULL)
        return kIOReturnError;

    // adding a pending flag here because we will be momentarily openning the gate
    // to make down calls into the client, before closing back up again to merge
    // the properties.
    while ( setParamPropertiesInProgress )
        cmdGate->commandSleep(&setParamPropertiesInProgress);

    setParamPropertiesInProgress = true;

    // check the reset before setting the other parameters
    if (dict->getObject(kIOHIDScrollCountResetKey)) {
        _setScrollCountParameters();
    }

    _setScrollCountParameters(dict);


    if( (number = OSDynamicCast( OSNumber, dict->getObject(kIOHIDWaitCursorFrameIntervalKey)))) {
        uint32_t value = number->unsigned32BitValue();
        _cursorWaitDelta = value;
        if (_cursorWaitDelta < kTickScale) {
            _cursorWaitDelta = kTickScale;
        }
        scheduleNextPeriodicEvent();
    }

    // save all params for new devices
    if ( dict->getObject(kIOHIDResetKeyboardKey) ) {
        UInt64 nano = EV_DEFAULTKEYREPEAT;
        makeNumberParamProperty( dict, kIOHIDKeyRepeatKey, nano, 64 );

        nano = EV_DEFAULTINITIALREPEAT;
        makeNumberParamProperty( dict, kIOHIDInitialKeyRepeatKey, nano, 64 );
    }

    if ( dict->getObject(kIOHIDResetPointerKey) ) {
        IOFixed fixed = EV_DEFAULTPOINTERACCELLEVEL;
        makeNumberParamProperty( dict, kIOHIDPointerAccelerationKey, fixed, sizeof(fixed) << 3);

        fixed = kIOHIDButtonMode_EnableRightClick;
        makeNumberParamProperty( dict, kIOHIDPointerButtonMode, fixed, sizeof(fixed) << 3);

        UInt64 nano = EV_DCLICKTIME;
        makeNumberParamProperty( dict, kIOHIDClickTimeKey, nano, 64 );
    }

    if ( dict->getObject(kIOHIDScrollResetKey) ) {
        IOFixed fixed = EV_DEFAULTSCROLLACCELLEVEL;
        makeNumberParamProperty( dict, kIOHIDScrollAccelerationKey, fixed, sizeof(fixed) << 3);
    }

    // update connected input devices
    if ( pOpenIter )
        *pOpenIter = getProviderIterator(); // getOpenProviderIterator();

    return kIOReturnSuccess;
}

void IOHIDSystem::_setScrollCountParameters(OSDictionary *newSettings)
{
    if (!newSettings) {
        newSettings = (OSDictionary*)copyProperty(kIOHIDScrollCountBootDefaultKey);
        if (!OSDynamicCast(OSDictionary, newSettings)) {
            if (newSettings)
                newSettings->release();
            newSettings = NULL;
        }
    }
    else {
        newSettings->retain();
    }

    if (newSettings) {
        OSNumber *number = NULL;
        OSBoolean *boolean = NULL;
        if((number = OSDynamicCast(OSNumber, newSettings->getObject(kIOHIDScrollCountMinDeltaToStartKey))))
        {
            setProperty(kIOHIDScrollCountMinDeltaToStartKey, number);
        }

        if((number = OSDynamicCast(OSNumber, newSettings->getObject(kIOHIDScrollCountMinDeltaToSustainKey))))
        {
            setProperty(kIOHIDScrollCountMinDeltaToSustainKey, number);
        }

        if((number = OSDynamicCast(OSNumber, newSettings->getObject(kIOHIDScrollCountMaxTimeDeltaBetweenKey))))
        {
            setProperty(kIOHIDScrollCountMaxTimeDeltaBetweenKey, number);
        }

        if((number = OSDynamicCast(OSNumber, newSettings->getObject(kIOHIDScrollCountMaxTimeDeltaToSustainKey))))
        {
            setProperty(kIOHIDScrollCountMaxTimeDeltaToSustainKey, number);
        }

        if((boolean = OSDynamicCast(OSBoolean, newSettings->getObject(kIOHIDScrollCountIgnoreMomentumScrollsKey))))
        {
            //_scIgnoreMomentum = (boolean == kOSBooleanTrue);
            setProperty(kIOHIDScrollCountIgnoreMomentumScrollsKey, boolean);
        }

        if((boolean = OSDynamicCast(OSBoolean, newSettings->getObject(kIOHIDScrollCountMouseCanResetKey))))
        {
            //_scMouseCanReset = (boolean == kOSBooleanTrue);
            setProperty(kIOHIDScrollCountMouseCanResetKey, boolean);
        }

        if((number = OSDynamicCast(OSNumber, newSettings->getObject(kIOHIDScrollCountMaxKey))))
        {
            setProperty(kIOHIDScrollCountMaxKey, number);
        }

        if((number = OSDynamicCast(OSNumber, newSettings->getObject(kIOHIDScrollCountAccelerationFactorKey))))
        {
            if (number->unsigned32BitValue() > 0) {
                setProperty(kIOHIDScrollCountAccelerationFactorKey, number);
            }
        }

        newSettings->release();
    }
}

IOReturn IOHIDSystem::doSetParamPropertiesPost(IOHIDSystem *self, void * arg0)
                        /* IOCommandGate::Action */
{
    return self->setParamPropertiesPostGated((OSDictionary *)arg0);
}



IOReturn IOHIDSystem::setParamPropertiesPostGated( OSDictionary * dict)
{
    if ( dict->getObject(kIOHIDTemporaryParametersKey) == NULL ) {
        bool resetKeyboard  = dict->getObject(kIOHIDResetKeyboardKey) != NULL;
        bool resetPointer   = dict->getObject(kIOHIDResetPointerKey) != NULL;
        bool resetScroll    = dict->getObject(kIOHIDScrollResetKey) != NULL;

        OSDictionary * newParams = OSDictionary::withDictionary( savedParameters );
        if( newParams) {
            if ( resetKeyboard ) {
                dict->removeObject(kIOHIDResetKeyboardKey);
            }
            if ( resetPointer ) {
                dict->removeObject(kIOHIDResetPointerKey);
                newParams->removeObject(kIOHIDTrackpadAccelerationType);
                newParams->removeObject(kIOHIDMouseAccelerationType);
            }
            if ( resetScroll ) {
                dict->removeObject(kIOHIDScrollResetKey);
                newParams->removeObject(kIOHIDTrackpadScrollAccelerationKey);
                newParams->removeObject(kIOHIDMouseScrollAccelerationKey);
            }

            newParams->merge( dict );
            setProperty( kIOHIDParametersKey, newParams );
            newParams->release();
            savedParameters = newParams;
        }

        // Send anevent notification that the properties changed.
        struct evioLLEvent      event;
        AbsoluteTime        ts;
        // clear event record
        bzero( (void *)&event, sizeof event);

        event.data.compound.subType = NX_SUBTYPE_HIDPARAMETER_MODIFIED;
        clock_get_uptime(&ts);
        postEvent(NX_SYSDEFINED,
                  &_cursorHelper.desktopLocation(),
                  ts,
                  &(event.data),
                  0,
                  0,
                  true,
                  NX_EVENT_EXTENSION_LOCATION_INVALID);
    }

    // Wake any pending setParamProperties commands.  They
    // still won't do much until we return out.
    setParamPropertiesInProgress = false;
    cmdGate->commandWakeup(&setParamPropertiesInProgress);

    return kIOReturnSuccess;
}


bool IOHIDSystem::attach( IOService * provider )
{
    return super::attach(provider);
}

void IOHIDSystem::detach( IOService * provider )
{
    super::detach(provider);
}

