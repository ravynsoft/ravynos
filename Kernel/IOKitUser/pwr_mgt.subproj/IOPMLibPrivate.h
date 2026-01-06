/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
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

#ifndef _IOPMLibPrivate_h_
#define _IOPMLibPrivate_h_

#include <TargetConditionals.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CFArray.h>
#include <IOKit/pwr_mgt/IOPMLibDefs.h>
#include <IOKit/pwr_mgt/IOPM.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/ps/IOPowerSources.h>
#include <sys/cdefs.h>

__BEGIN_DECLS
 
/*!
 * @constant    kIOPBundlePath
 * @abstract    c-string path to powerd.bundle
 */
#define kIOPMBundlePath                                     "/System/Library/CoreServices/powerd.bundle"

/*!
 * @constant    kIOPMSystemPowerStateNotify
 * @abstract    Notify(3) string that PM fires every time the system begins a sleep, wake, or a maintenance wake.
 * @discussion  The notification fires at the "system will" notificatin phase; e.g. at the beginning of the sleep or wake.
 *              Unless you have a strong need for this asynchronous sleep/wake notification, you 
 *              should really be using IOPMConnectionCreate().
 */
#define kIOPMSystemPowerStateNotify                         "com.apple.powermanagement.systempowerstate"

/*!
 * @constant    kIOPMSystemPowerCapabilitiesKeySuffix
 * @abstract    SCDynamicStoreKey location where the system state capability can be found.
 * @discussion  This state is always updated immediately prior to when PM delivers 
 *              the notify (3) notification kIOPMSystemPowerStateNotify
 *              The System power capabilities are defined by the 
 *              enum <code>@link IOPMCapabilityBits@/link</code> below.
 */
#define kIOPMSystemPowerCapabilitiesKeySuffix               "/IOKit/SystemPowerCapabilities"

/*! @define kIOUserActivityNotifyName
   @abstract The notification by this name fires when User activity state on the system changes.
   @discussion Pass this string as an argument to register via notify(3).
            You can query User activity  state via notify_get_state() when this notification
            fires. The returned value will be either kIOUserIsIdle or kIOUserIsActive
 */
enum { 
   kIOUserIsIdle = 0, 
   kIOUserIsActive = 1
};

#define kIOUserActivityNotifyName                           "com.apple.system.powermanagement.useractivity"

/*! @define kIOUserAssertionReSync
   @abstract This is the Notify(3) string fired by powerd every time the process is started.
 */
#define kIOUserAssertionReSync                              "com.apple.system.powermanagement.assertionresync"

/*! @define     kIOPMDarkWakeControlEntitlement
 *  @abstract   Apple internal entitlement for processes that may influence
 *              DarkWake policy.
 */
#define kIOPMDarkWakeControlEntitlement     CFSTR("com.apple.private.iokit.darkwake-control")

/*! @define     kIOPMDarkWakeControlEntitlement
 *  @abstract   Apple internal entitlement for processes that receive Interactive Push notifications.
 */
#define kIOPMInteractivePushEntitlement     CFSTR("com.apple.private.iokit.interactive-push")

/*! @define     kIOPMReservePwrCtrlEntitlement
 *  @abstract   Apple internal entitlement for processes that create kIOPMAssertAwakeReservePower assertion 
 *              and call IOPMSetReservePowerMode()
 */
#define kIOPMReservePwrCtrlEntitlement      CFSTR("com.apple.private.iokit.reservepower-control")

/*! @define     kIOPMAssertOnBatteryEntitlement
 *  @abstract   Apple internal entitlement for processes that creates assertion with
 *              kIOPMAssertionAppliesToLimitedPowerKey property set to true.
 */
#define kIOPMAssertOnBatteryEntitlement     CFSTR("com.apple.private.iokit.assertonbattery")

/*! @define     kIOPMAssertOnLidCloseEntitlement
 *  @abstract   Apple internal entitlement for processes that creates assertion with
 *              kIOPMAssertionAppliesOnLidClose property set to true.
 */
#define kIOPMAssertOnLidCloseEntitlement    CFSTR("com.apple.private.iokit.assertonlidclose")

/*! @define     kIOPMWakeRequestEntitlement
 *  @abstract   Apple internal entitlement to allow non-root processes to schedules wakes.
*/
#define kIOPMWakeRequestEntitlement      CFSTR("com.apple.iokit.wakerequest")

/*! @define     kIOPMAssertionSuspendResumeEntitlement
 *  @abstract   Apple internal entitlement for process to suspend and resume assertions on
 *              on behalf of another process
 */
#define kIOPMAssertionSuspendResumeEntitlement     CFSTR("com.apple.private.iokit.assertion-suspendresume")

/*! @define     kIOPMLimitedPowerWakeRequestEntitlement
 *  @abstract   Apple internal entitlement for process, allowing it to program Wake Event while on
 *              Limited Power
 */
#define kIOPMLimitedPowerWakeRequestEntitlement    CFSTR("com.apple.private.iokit.limitedpower-wakerequest")

/*!
 * @constant    kIOPMServerBootstrapName
 * @abstract    Do not use. There's no reason for any code outside of PowerManagement to use this.
 * @discussion  The PM system server registers via this key with launchd.
 */
#define kIOPMServerBootstrapName                            "com.apple.PowerManagement.control"

/*!
 * @group       AutoWake API
 * @abstract    For internal use communicating between IOKitUser and PM configd
 */

/*!
 * @constant    kIOPMAutoWakePresPath
 * @abstract    Filename of the scheduled power events file.
 */
#define kIOPMAutoWakePrefsPath                              "com.apple.AutoWake.xml"

/*
 * @constant    kIOPMAutoWakeScheduleImmediate
 * @abstract    Internal only argument to <code>@link IOPMSchedulePowerEvent@/link</code> 
 *              for use only by debugging tools.
 * @discussion  Once scheduled, these types are not cancellable, nor will they appear in any lists of
 *              scheduled arguments from IOPMCopyScheduledEvents. These are not acceptable
 *              types to pass as Repeating events.
 *
 *              The 'date' argument to IOPMSchedulePowerEvent is an absolute one for 
 *              'AutoWakeScheduleImmediate' and 'AutoPowerScheduleImmediate'. The effect
 *              of these WakeImmediate and PowerOnImmediate types is to schedule the
 *              wake/power event directly with the wake/power controller, ignoring all OS
 *              queueing and management. This will override a previously scheduled wake event
 *              by another application, should one exist. Recommended for testing only. 
 */
#define kIOPMAutoWakeScheduleImmediate                      "WakeImmediate"

/*
 * @constant    kIOPMAutoPowerScheduleImmediate
 * @abstract    Internal only argument to <code>@link IOPMSchedulePowerEvent@/link</code> 
 *              for use only by debugging tools.
 * @discussion  Once scheduled, these types are not cancellable, nor will they appear in any lists of
 *              scheduled arguments from IOPMCopyScheduledEvents. These are not acceptable
 *              types to pass as Repeating events.
 *
 *              The 'date' argument to IOPMSchedulePowerEvent is an absolute one for 
 *              'AutoWakeScheduleImmediate' and 'AutoPowerScheduleImmediate'. The effect
 *              of these WakeImmediate and PowerOnImmediate types is to schedule the
 *              wake/power event directly with the wake/power controller, ignoring all OS
 *              queueing and management. This will override a previously scheduled wake event
 *              by another application, should one exist. Recommended for testing only.
 *
 *
 */
#define kIOPMAutoPowerScheduleImmediate                     "PowerOnImmediate"

/*!
 * @constant    kIOPMSchedulePowerEventNotification
 * @abstract    Notification posted when IOPMSchedulePowerEvent successfully schedules a power event
 * @discussion  i.e. this notify(3) notification fires every time the list of scheduled power events changes.
 */
#define kIOPMSchedulePowerEventNotification                 "com.apple.system.IOPMSchedulePowerEventNotification"

/*!
 * @constant    kIOPMUserVisiblePowerEventNotification
 * @abstract    Notification posted when IOPMSchedulePowerEvent or IOPMCancelScheduledPowerEvent is called
 *              for a UserVisible event
 * @discussion  i.e. this notify(3) notification fires every time the list of scheduled power events changes.
 */
#define kIOPMUserVisiblePowerEventNotification              "com.apple.system.powermanagement.uservisiblepowerevent"



/*
 * @constant    kIOPMAutoWakeRelativeSeconds
 * @abstract    Internal only argument to <code>@link IOPMSchedulePowerEvent@/link</code> 
 *              for use only by debugging tools.
 * @discussion  The 'date' argument to IOPMSchedulePowerEvent is an relative one to "right now," 
 *              when passing type for 'kIOPMSettingDebugWakeRelativeKey' or 'kIOPMAutoPowerRelativeSeconds'
 *
 *              e.g. In this case, we're setting the system to wake from sleep exactly 10
 *              seconds after the system completes going to sleep. We're passing in a date
 *              10 seconds past "right now", but the wakeup controller interprets this as 
 *              relative to sleep time.
 * 
 *              d = CFDateCreate(0, CFAbsoluteGetCurrent() + 10.0);
 *              IOPMSchedulePowerEvent(d, CFSTR("SleepCycler"), CFSTR(kIOPMAutoWakeRelativeSeconds) ); 
 */
#define kIOPMAutoWakeRelativeSeconds                        kIOPMSettingDebugWakeRelativeKey
/*
 * @constant    kIOPMAutoPowerRelativeSeconds
 * @abstract    Internal only argument to <code>@link IOPMSchedulePowerEvent@/link</code> 
 *              for use only by debugging tools.
 * @discussion  The 'date' argument to IOPMSchedulePowerEvent is an relative one to "right now," 
 *              when passing type for 'kIOPMSettingDebugWakeRelativeKey' or 'kIOPMAutoPowerRelativeSeconds'
 *
 *              e.g. In this case, we're setting the system to wake from sleep exactly 10
 *              seconds after the system completes going to sleep. We're passing in a date
 *              10 seconds past "right now", but the wakeup controller interprets this as 
 *              relative to sleep time.
 * 
 *              d = CFDateCreate(0, CFAbsoluteGetCurrent() + 10.0);
 *              IOPMSchedulePowerEvent(d, CFSTR("SleepCycler"), CFSTR(kIOPMAutoWakeRelativeSeconds) ); 
 */
#define kIOPMAutoPowerRelativeSeconds                       kIOPMSettingDebugPowerRelativeKey


/**************************************************
*
* Repeating Sleep/Wake/Shutdown/Restart API
*
**************************************************/
/*!
 * @functiongroup Repeating power events
 */

// Keys to index into CFDictionary returned by IOPSCopyRepeatingPowerEvents()
#define     kIOPMRepeatingPowerOnKey        "RepeatingPowerOn"
#define     kIOPMRepeatingPowerOffKey       "RepeatingPowerOff"

#define     kIOPMAutoSleep                  "sleep"
#define     kIOPMAutoShutdown               "shutdown"

#define     kIOPMPowerEventLeewayKey        "leeway"

#define     kIOPMPowerEventUserVisible      "UserVisible"

// Keys to "days of week" bitfield for IOPMScheduleRepeatingPowerEvent()
enum {
    kIOPMMonday         = 1 << 0,
    kIOPMTuesday        = 1 << 1,
    kIOPMWednesday      = 1 << 2,
    kIOPMThursday       = 1 << 3,
    kIOPMFriday         = 1 << 4,
    kIOPMSaturday       = 1 << 5,
    kIOPMSunday         = 1 << 6
};

// Keys to index into sub-dictionaries of the dictionary returned by IOPSCopyRepeatingPowerEvents
// Absolute time to schedule power on (stored as a CFNumberRef, type = kCFNumberIntType)
//#define kIOPMPowerOnTimeKey                 "time"
// Bitmask of days to schedule a wakeup for (CFNumberRef, type = kCFNumberIntType)
#define kIOPMDaysOfWeekKey                  "weekdays"
// Type of power on event (CFStringRef)
//#define kIOPMTypeOfPowerOnKey               "typeofwake"

/*  @function IOPMScheduleRepeatingPowerEvent
 *  @abstract Schedules a repeating sleep, wake, shutdown, or restart
 *  @discussion Private API to only be used by Energy Saver preferences panel. Note that repeating sleep & wakeup events are valid together, 
 *              and shutdown & power on events are valid together, but you cannot mix sleep & power on, or shutdown & wakeup events.
 *              Every time you call IOPMSchedueRepeatingPowerEvent, we will cancel all previously scheduled repeating events of that type, and any
 *              scheduled repeating events of "incompatible" types, as I just described.
 *  @param  events A CFDictionary containing two CFDictionaries at keys "RepeatingPowerOn" and "RepeatingPowerOff".
                Each of those dictionaries contains keys for the type of sleep, the days_of_week, and the time_of_day. These arguments specify the
                time, days, and type of power events.
 *  @result kIOReturnSuccess on success, kIOReturnError or kIOReturnNotPrivileged otherwise.
 */
IOReturn IOPMScheduleRepeatingPowerEvent(CFDictionaryRef events);

/*  @function IOPMCopyRepeatingPowerEvents
 *  @abstract Gets the system
 *  @discussion Private API to only be used by Energy Saver preferences panel. Copies the system's current repeating power on
                and power off events.
                The returned CFDictionary contains two CFDictionaries at keys "RepeatingPowerOn" and "RepeatingPowerOff".
                Each of those dictionaries contains keys for the type of sleep, the days_of_week, and the time_of_day.
 *  @result NULL on failure, CFDictionary (possibly empty) otherwise.
 */
 CFDictionaryRef IOPMCopyRepeatingPowerEvents(void);

/*  @function IOPMScheduleRepeatingPowerEvent
 *  @abstract Cancels all repeating power events
 *  @result kIOReturnSuccess on success, kIOReturnError or kIOReturnNotPrivileged otherwise.
 */ 
IOReturn IOPMCancelAllRepeatingPowerEvents(void);


/*!
 * @function                    IOPMRequestSysWake
 *
 * @abstract                    Requests the system to wake up from sleep at the specified time
 * @param request               Dictionary describing the wake request
 *
 * @discussion          
 *      Request device to wake up at the specified time.
 *
 *      kIOPMPowerEventAppNameKey   CFStringRef, mandatory. Specifies the purpose of this wake and used for logging and debugging
 *      kIOPMPowerEventTimeKey      CFDateRef, mandatory. Specifies the time when system has to wake up.
 *      kIOPMPowerEventLeewayKey    CFNumber(kCFNumberIntType), optional. Specifies the number of seconds by which wake can be delayed
 *                                  to coalesce with other wake requests. Defaults to 0.
 *      kIOPMPowerEventUserVisible  CFBooleanRef, optional. Specifies if the RTC wake request will be visible to
 *                                  user(Clock alarm, Mail Alert etc)
 */
IOReturn IOPMRequestSysWake(CFDictionaryRef request); 


#pragma mark Private Assertions
/**************************************************
*
* Private assertions
*
**************************************************/
/*!
 * @group Private Assertions
 */

/*! 
 * @constant    kIOPMAssertionTypeApplePushServiceTask
 *
 * @discussion  When active during a SleepServices wake, will keep the system awake, subject to
 *              the sleep services time cap.
 *
 *              Except for running during SleepServices time, while a system is on battery power
 *              a ApplePushServiceTask assertion cannot keep the machine in a higher power state.
 *              e.g. An ApplePushServiceTask cannot prevent sleep if the system is on battery.
 *
 *              Awake Behavior on AC power: this assertion has no effect during full wake. 
 *
 *              DarkWake Behavior on AC power: this assertion will force the system not to sleep;
 *                  e.g. the system will stay in DarkWake as long as this assertion is held.
 *
 *              If the platform supports SilentRunning, the assertion shall implement
 *              this behavior:
 *                   - Keeps the system awake during DarkWake.
 *                   - Valid on AC & Battery
 *                   - Only valid in DarkWake
 *                   - Only valid during global SleepServicesTimeout Window (as determined by powerd)
 *                   - Only valid while SMC is in SilentRunning mode
 *
 *              If the platform doesn't support SilentRunning, the assertion shall 
 *              implement this behavior:
 *                   - Prevents idle sleep
 *                   - Valid on AC & Battery
 *                   - Only valid in FullWake
 *                   - Only valid during global SleepServicesTimeout Window (as determined by powerd)

 *              For SilentRunning support, see:
 *                  - <code>IOPMFeatureIsSupported(CFSTR(kIOPMSilentRunningKey))</code>
 */
#define kIOPMAssertionTypeApplePushServiceTask              CFSTR("ApplePushServiceTask")

/*!
 * @constant    kIOPMAssertInteractivePushServiceTask
 *
 * @abstract    Push Service infrastructure apps must take this while processing a push
 *              and hold it until they can determine whether that push will cause
 *              a user interactive wake or NotificationWake.
 *
 * @discussion  Since pushes might or might not cause a user interactive wake, OS X needs
 *              to wait until the recipient App can decide whether they do. Thus apsd and 
 *              apps must hold this assertion while they process incoming pushes.
 *
 *              The caller must hold the darkwake-control entitlement:
 *                  - kIOPMDarkWakeControlEntitlement
 *
 *              If system supports TCPKeepAlive, and TCPKeepAlive is dynamically enabled,
 *              the assertion shall implement the following behavior:
 *                   - Prevents idle sleep
 *                   - Prevents forced sleep
 *                   - Reverts sleeps in progress.
 *                   - Valid on AC & Battery
 *                   - Valid in DarkWake & FullWake
 *                   - Per-assertion timeout (specified by IOPPF)
 *                   - OS X will prefer to run fanless; but OS X shall allow 
 *                      the fans to come on. That is, OS X prefer that processes run 
 *                      to completion over avoiding fan noise.
 *
 *              If the system does not support TCPKeepAlive, or TCPKeepAlive is dynamically
 *              disabled, then this assertion shall implement 
 *              kIOPMAssertionTypeApplePushServiceTask behavior.
 */
#define kIOPMAssertInteractivePushServiceTask               CFSTR("InteractivePushServiceTask")


/*! 
 * @constant    kIOPMAssertionTypeBackgroundTask
 *
 * @discussion  This assertion should be created and held by applications while performing
 *              system maintenance tasks, e.g. work not initiated by a user.
 *
 *              Only Centralized Task Scheduler Power Nap processes should use this assertion.
 *
 *              Example: Periodic system data backups, spotlight indexing etc.
 *
 *              Behavior of this assertion on Systems that support Silent Running: 
 *               On AC: Prevents sytem sleep. If system is idle or if user requests sleep(by 
 *                      lid close or by selecting sleep from apple menu), system runs with display 
 *                      off in silent running mode. Holding this assertion during darkwake results 
 *                      in extending dark wake in silent running mode.
 *               On Battery: This assertion is not honoured.
 *
 *              Behavior on Systems that don't support Silent Running:
 *               On AC: Prevents system idle sleep. Display may get turned off. If user requests
 *                      sleep(by lid close or apple menu sleep), then system goes into sleep.
 *                      If the assertion is held during dark wake, it is not honored.
 *               On Battery: This assertion is not honoured.
 *
 */
#define kIOPMAssertionTypeBackgroundTask                    CFSTR("BackgroundTask")

/*! 
 * @constant    kIOPMAssertAwakeReservePower
 *
 * @discussion  This assertion should be created to prevent system idle sleep even when system
 *              is in reserve power mode. 
 *
 *              Process creating this assertion must have kIOPMReservePwrCtrlEntitlement 
 *              entitlement.
 */
#define kIOPMAssertAwakeReservePower                       CFSTR("AwakeOnReservePower")




/*! @define         kIOPMAssertionTypeDenySystemSleep
 *  @deprecated     Please use public assertion <code>@link kIOPMAssertionTypePreventSystemSleep@/link</code>,
 *                  instead of this one.
 */
#define kIOPMAssertionTypeDenySystemSleep                   CFSTR("DenySystemSleep")

/*! @define         kIOPMAssertionUserIsActive
 *  @abstract       Powers on the display and prevents display from going dark.
 *  @discussion     This is the backend assertion supporting
 *                  <code>@link IOPMAssertionDeclareUserActivity @/link</code>.
 *                  
 *                  Public API users should call IOPMAssertionDeclareUserActivity;
 *                  internal Apple SPI users may opt to directly create this assertion
 *                  for finer grained control over its timeout.
 */
#define kIOPMAssertionUserIsActive                          CFSTR("UserIsActive")


/*! @define          kIOPMAssertDisplayWake
 *  @abstract        For IOKit use only. 
 *                   This is the acking assertion for:
 *                      <code>@link IOPMAssertionDeclareNotificationEvent@/link</code>
 *  @discussion      Don't create this assertion directly. Always take it by
 *                   calling IOPMAssertionDeclareNotificationEvent.
 *                   Callers must be entitled with com.apple.private.iokit.darkwake-control
 *                   to use this assertion or to call IOPMAssertionDeclareNotificationEVent.
 */
#define kIOPMAssertDisplayWake                              CFSTR("DisplayWake")


/*! @define         kIOPMAssertInternalPreventSleep
 *  @abstract       For powerd use only.
 *  @discussion     Assertion taken by powerd as a proxy temporarily.
 *                  This can be taken either to finish a short-term task(like device enumeration)
 *                  or to keep system awake until wake reason is detected and the apps specific to this
 *                  wake take their own assertion.
 *                  Do not use this assertion outside of powerd.
 */
#define kIOPMAssertInternalPreventSleep                     CFSTR("InternalPreventSleep")

/*! @define         kIOPMAssertiInternalPreventDisplaySleep
 *  @abstract       For powerd use only.
 *  @discussion     Assertion taken by powerd as a proxy temporarily to prevent display sleep.
 *                  Do not use this assertion outside of powerd.
 */
#define kIOPMAssertInternalPreventDisplaySleep              CFSTR("InternalPreventDisplaySleep")

/*! @define         kIOPMAssertRequiresDisplayAudio
 *
 *  @discussion     Prevents display from sleeping even if user is idle, but allows loginwindow
 *                  to start the screen saver. 
 */
#define kIOPMAssertRequiresDisplayAudio                CFSTR("RequiresDisplayAudio")

/*! @define         kIOPMAssertMaintenanceActivity
 *
 * @abstract        Keep the system awake in FullWake or DarkWake for maintenance activity.
 * AC/Batt behavior - Works on AC only
 * Acoustics behavior - This assertion will not trigger system to
 *                      go into non-silent running mode.
 */
#define kIOPMAssertMaintenanceActivity                      CFSTR("MaintenanceWake")

/* @define          kIOPMAssertionTypeSystemIsActive
 * @abstract        Prevents idle sleep, but also reverts a sleep-in-progress.
 *
 * @discussion      Do not create this assertion directly. Create it by calling
 *                  <code>@link IOPMAssertionDeclareSystemActivity()@/link</code>
 *
 * 'kIOPMAssertionTypeSystemIsActive will revert an idle sleep currently in progress
 * where possible. If reverting an idle sleep is not possible, it will try to wake the system
 * up immediately once the system goes to sleep. Apart from this one side-effect, it behaves
 * identically to a PreventUserIdleSystemSleep assertion.
 *
 * Consider calling IOPMAssertionDeclareSystemActivity() if you would like to create an
 * assertion of this type instead of directly creating it using one of the IOPMAssertionCreate
 * APIs.
 *
 */
#define kIOPMAssertionTypeSystemIsActive                    CFSTR("SystemIsActive")


// Disables AC Power Inflow (requires root to initiate)
#define kIOPMAssertionTypeDisableInflow                     CFSTR("DisableInflow")
#define kIOPMInflowDisableAssertion                         kIOPMAssertionTypeDisableInflow

// Disables battery charging (requires root to initiate)
#define kIOPMAssertionTypeInhibitCharging                   CFSTR("ChargeInhibit")
#define kIOPMChargeInhibitAssertion                         kIOPMAssertionTypeInhibitCharging

// Disables low power battery warnings
#define kIOPMAssertionTypeDisableLowBatteryWarnings         CFSTR("DisableLowPowerBatteryWarnings")
#define kIOPMDisableLowBatteryWarningsAssertion             kIOPMAssertionTypeDisableLowBatteryWarnings

// Once initially asserted, the machine may only idle sleep while this assertion
// is asserted. For embedded use only.
#define kIOPMAssertionTypeEnableIdleSleep                   CFSTR("EnableIdleSleep")


// Needs CPU Assertions - DEPRECATED
// Only have meaning on PowerPC machines.
#define kIOPMAssertionTypeNeedsCPU                          CFSTR("CPUBoundAssertion")
#define kIOPMCPUBoundAssertion                              kIOPMAssertionTypeNeedsCPU

// Assertion Actions
#define kPMASLAssertionActionCreate             "Created"
#define kPMASLAssertionActionRetain             "Retain"
#define kPMASLAssertionActionRelease            "Released"
#define kPMASLAssertionActionClientDeath        "ClientDied"
#define kPMASLAssertionActionTimeOut            "TimedOut"
#define kPMASLAssertionActionSummary            "Summary"
#define kPMASLAssertionActionTurnOff            "TurnedOff"
#define kPMASLAssertionActionTurnOn             "TurnedOn"
#define kPMASlAssertionActionCapTimeOut         "CapExpired"
#define kPMASLAssertionActionNameChange         "NameChange"
#define kPMASLAssertionActionSuspend            "Suspended"
#define kPMASLAssertionActionResume             "Resumed"

#pragma mark Private Assertion Dictionary Keys
/*
 * Private Assertion Dictionary Keys
 */

#define kIOPMAssertionUsedDeprecatedCreateAPIKey              CFSTR("UsedDeprecatedCreateAPI")

/*! @constant kIOPMAssertionCreateDateKey
 *  @abstract Records the time at which the assertion was created.
 */
#define kIOPMAssertionCreateDateKey                         CFSTR("AssertStartWhen")

/*! @constant kIOPMAssertionReleaseDateKey
 *  @abstract Records the time that the assertion was released.
 *  @discussion The catch is that we only record the release time for assertions that have
 *  already timed out. In the normal create/release lifecycle of an assertion, we won't record
 *  the release time because we'll destroy the assertion object upon its release.
 */
#define kIOPMAssertionReleaseDateKey                        CFSTR("AssertReleaseWhen")

/*! @constant kIOPMAssertionTimedOutDateKey
 *  @abstract Records the time that an assertion timed out.
 *  @discussion An assertion times out when its specified timeout interval has elapsed.
 *  This value only exists once the assertion has timedout. The presence of this 
 *  key/value pair in a dictionary indicates the assertion has timed-out.
 */
#define kIOPMAssertionTimedOutDateKey                       CFSTR("AssertTimedOutWhen")

/*! @constant kIOPMAssertionTimeOutIntervalKey
 *  @abstract The owner-specified timeout for this assertion.
 */
#define kIOPMAssertionTimeOutIntervalKey                    CFSTR("AssertTimeOutInterval")

/*! @constant kIOPMAssertionTimeoutTimeLeftKey
 *  @abstract Records number of seconds left before this assertion times out
 */
#define kIOPMAssertionTimeoutTimeLeftKey                   CFSTR("AssertTimeoutTimeLeft")

/*! @constant kIOPMAssertionTimeoutUpdateTimeKey
 *  @abstract Records the time at which 'kIOPMAssertionTimeoutTimeLeftKey' is updated
 */
#define kIOPMAssertionTimeoutUpdateTimeKey                 CFSTR("AssertTimeoutUpdateTime")

/*! @constant kIOPMAssertionPIDKey
 *  @abstract The owning process's PID.
 */
#define kIOPMAssertionPIDKey                                CFSTR("AssertPID")

/*! @constant   kIOPMAssertionGlobalIDKey
 *  @abstract   A uint64_t integer that can uniuely identify an assertion system-wide.
 *
 *  @discussion kIOPMAssertionGlobalIDKey differs from the assertion ID (of type IOPMAssertionID) 
 *              returned by IOPMAssertionCreate* API. kIOPMAssertionGlobalIDKey can refer to 
 *              an assertion owned by any process. The 32-bit IOPMAssertionID only refers to 
 *              assertions within the process that created the assertion.
 *              You can use the API <code>@link IOPMCopyAssertionsByProcess@/link</code>
 *              to see assertion data for all processes.
 */
#define kIOPMAssertionGlobalUniqueIDKey                     CFSTR("GlobalUniqueID")

/*!
 * @define          kIOPMAssertionAppliesToLimitedPowerKey
 *
 * @abstract        The CFDictionary key for assertion power limits in an assertion info dictionary.
 *
 * @discussion      The value for this key will be a CFBooleanRef, with value
 *                  <code>kCFBooleanTrue</code> or <code>kCFBooleanFalse</code>. A value of 
 *                  kCFBooleanTrue means that the assertion is applied even when system is running
 *                  on limited power source like battery. A value of kCFBooleanFalse means that
 *                  the assertion is applied only when the system is running on unlimited power
 *                  source like AC. 
 *
 *                  This property is valid only for assertion <code>@link kIOPMAssertionTypePreventSystemSleep @/link</code>. 
 *                  By default, this assertion is applied only when system is running on unlimited
 *                  power source. This behavior can be changed using this property.
 *
 *                  Process creating assertion with this property must have kIOPMAssertOnBatteryEntitlement
 *                  entitlement.
 */

#define kIOPMAssertionAppliesToLimitedPowerKey              CFSTR("AppliesToLimitedPower")
/*!
 * @define          kIOPMAssertionAppliesOnLidClose
 *
 * @abstract        The CFDictionary key in assertion info dictionarty for enabling assertion when lid is closed.
 *
 * @discussion      The value for this key will be a CFBooleanRef, with value
 *                  <code>kCFBooleanTrue</code> or <code>kCFBooleanFalse</code>. A value of 
 *                  kCFBooleanTrue means that the assertion is applied even when lid is closed.
 *                  A value of kCFBooleanFalse means that the assertion is applied only when 
 *                  lid is open. 
 *
 *                  This property is valid only for assertion <code>@link kIOPMAssertionUserIsActive @/link</code>. 
 *                  By default, this assertion is applied only when lid is open and setting this assertion property
 *                  changes that default behavior. This assertion property has no meaning on systems with no lid 
 *                  and it is treated as no-op.
 *
 *                  Process creating assertion with this property must have kIOPMAssertOnLidCloseEntitlement
 *                  entitlement.
 */

#define kIOPMAssertionAppliesOnLidClose                     CFSTR("AppliesOnLidClose")

/*!
 * @define          kIOPMAssertionAutoTimesout
 *
 * @abstract        The CFDictionary key in assertion info dictionary for auto timing out assertion after pre-defined
 *                  number of seconds.
 *
 * @discussion      The value for this key will be a CFBooleanRef, with value <code>kCFBooleanTrue</code> or
 *                  <code>kCFBooleanFalse</code>. A value of kCFBooleanTrue means that the assertion is allowed
 *                  to auto-timeout after pre-defined number of seconds. The auto-timeout duration is a fixed value
 *                  for all assertions with this property.
 *
 *                  A value of kCFBooleanFalse means that the auto-timeout is not allowed.
 *                  This is the default value when this key is not set.
 *
 *                  This optional property can be set at the time of assertion creation and
 *                  cannot be changed later.
 */

#define kIOPMAssertionAutoTimesout                          CFSTR("AutoTimesout")

/*!
 * @define          kIOPMAssertionExitSilentRunning
 *
 * @abstract        The CFDictionary key in assertion info dictionary to trigger exiting silent running mode
 *
 * @discussion      The value for this key will be a CFBooleanRef, with value <code>kCFBooleanTrue</code> or
 *                  <code>kCFBooleanFalse</code>. A value of kCFBooleanTrue triggers system to exit silent
 *                  running mode. Set the property to kCFBooleanFalse has no impact on system state.
 *
 *                  This optional property can be set to kCFBooleanTrue at the time of assertion creation or
 *                  can be set later using <code>@link IOPMAssertionSetProperty @/link</code>. This property
 *                  is not honored on all assertion types.
 */
#define kIOPMAssertionExitSilentRunning                     CFSTR("ExitSilentRunning")

/*!
 * @define          kIOPMAssertionResourcesUsed
 *
 * @abstract        This CFDictionary key in assertion info dictionary lists the resources used
 *                  by the process creating this assertion, or by the process on-behalf of which
 *                  this assertion is created.
 *
 * @discussion      The value for this key will be a CFArray, with each element specifying the
 *                  resource used by the process.
 *
 *                  This optional property can be changed after the property is created.
 */

#define kIOPMAssertionResourcesUsed                         CFSTR("ResourcesUsed")

/*
 * Resource strings that can be used with kIOPMAssertionResourcesUsed key.
 */
#define kIOPMAssertionResourceAudioIn                       CFSTR("audio-in")
#define kIOPMAssertionResourceAudioOut                      CFSTR("audio-out")
#define kIOPMAssertionResourceGPS                           CFSTR("GPS")
#define kIOPMAssertionResourceBaseband                      CFSTR("baseband")
#define kIOPMAssertionResourceBluetooth                     CFSTR("bluetooth")

/*!
 * @define          kIOPMAssertionResourcesName
 *
 * @abstract        This CFDictionary key in assertion info dictionary lists the specific name
 *                  of the resource used. If this key is set, then kIOPMAssertionResourcesUsed
 *                  key should also be set. The resource names may be used to look up to registry.
 *
 * @discussion      The value for this key will be a CFArray, with each element specifying the
 *                  resource names.
 *
 *                  This optional property can be changed after the property is created.
 */
#define kIOPMAssertionResourcesName                         CFSTR("ResourcesName")

/*!
 * @define          kIOPMAssertionActivityBudgeted
 *
 * @abstract        The CFDictionary key in assertion info dictionary specifies if the activity
 *                  for which the assertion is created is budgeted.
 *
 * @discussion      The value for this key will be a CFBooleanRef, with value <code>kCFBooleanTrue</code> or
 *                  <code>kCFBooleanFalse</code>. A value of kCFBooleanTrue means that the activity
 *                  for which the assertions is created is budgeted.
 *
 *                  A value of kCFBooleanFalse means that the acitivity is not budgeted.
 *                  This is the default value when this key is not set.
 *
 *                  This optional property can be set at the time of assertion creation and
 *                  cannot be changed later. Only the activity scheduler is expected to set this property.
 */

#define kIOPMAssertionActivityBudgeted                      CFSTR("ActivityBudgeted")

/*!
 * @define          kIOPMAssertionAllowsDeviceRestart
 *
 * @abstract        The CFDictionary key in assertion info dictionary to specify that it is ok to
 *                  restart the device even when this assertion is active.
 *
 * @discussion      The value for this key will be a CFBooleanRef, with value <code>kCFBooleanTrue</code> or
 *                  <code>kCFBooleanFalse</code>. A value of kCFBooleanTrue means that the system can be
 *                  restarted for system maintenance even when the assertion is active.
 *
 *                  A value of kCFBooleanFalse means that the system should NOT be restarted.
 *                  This is the default value when this key is not set.
 *
 *                  This optional property can be set at the time of assertion creation and
 *                  cannot be changed later.
 */

#define kIOPMAssertionAllowsDeviceRestart                   CFSTR("AllowsDeviceRestart")


/*! 
 * @constant kIOPMAssertionCreatorBacktrace
 * @abstract CFDataRef type. If present, holds the backtrace of the thread creating assertion.
 */
#define kIOPMAssertionCreatorBacktrace                      CFSTR("CreatorBacktrace")

/*!
 * @constant kIOPMAssertionOnBehalfOfPID
 * @abstract CFNumberRef type. If present, specifies the PID on whose behalf the assertion is created.
 *           This can be different from 'kIOPMAssertionPIDKey' as one process can be creating assertion for
 *           other process. This key is purely for accounting & debugging purpose and no action is taken 
 *           when 'kIOPMAssertionOnBehalfOfPID' dies.
 *           This optional property can be set only at the time assertion creation and shouldn't be 
 *           changed after assertion is created.
 */
#define kIOPMAssertionOnBehalfOfPID                         CFSTR("AssertionOnBehalfOfPID")

/*!
 * @constant kIOPMAssertionOnBehalfOfPIDReason
 * @abstarct CFStringRef type. If present, gives a description on why process with PID 'kIOPMAssertionPIDKey' is 
 *           creating assertion on behalf of process with PID 'kIOPMAssertionOnBehalfOfPID'
 *           This optional property can be set only at the time assertion creation and shouldn't be 
 *           changed after assertion is created.
 */
#define kIOPMAssertionOnBehalfOfPIDReason                   CFSTR("AssertionOnBehalfOfPIDReason")

/*!
 * @constant kIOPMAssertionOnBehalfOfBundleID
 * @abstract CFStringRef type. If present, specifies the Bundle ID on whose behalf the assertion is created.
 *           This optional property can be set only at the time assertion creation and shouldn't be
 *           changed after assertion is created.
 */
#define kIOPMAssertionOnBehalfOfBundleID                    CFSTR("AssertionOnBehalfOfBundleID")


/*!
 * @constant kIOPMAssertionTrueTypeKey
 * @abstract Shows the current supported assertion name for the deprecated name
 */
#define kIOPMAssertionTrueTypeKey               CFSTR("AssertionTrueType")

/*!
 * @constant kIOPMAssertionIdKey
 * @abstract Holds the assertionId returned to client on creation
 */
#define kIOPMAssertionIdKey                     CFSTR("AssertionId")

/*!
 * @constant kIOPMAsyncClientAssertionIdKey
 * @abstract Holds the assertionId returned to client on creation
 */
#define kIOPMAsyncClientAssertionIdKey          CFSTR("AsyncClientAssertionId")

/*!
 * @constant        kIOPMAssertionIsStateSuspendedKey
 *
 * @abstract        The CFDictionary key in assertion info dictionary
 *                  specifies whether the assertion is in a suspended state.
 *
 * @discussion      The value of this key will be a CFBooleanRef.
 */
#define kIOPMAssertionIsStateSuspendedKey          CFSTR("AssertionIsStateSuspended")

/*!
 * @constant        kIOPMAssertionTimedOutNotifyString
 * @discussion      Assertion notify(3) string
 *                  Fires when an assertion times out. Caller has to call IOPMAssertionNotify()
 *                  and register with powerd with this string before calling notify_register.
 */
#define kIOPMAssertionTimedOutNotifyString                  "com.apple.system.powermanagement.assertions.timeout"

/*!
 * @constant        kIOPMAssertionsAnyChangedNotifyString
 * @discussion      Assertion notify(3) string
 *                  Fires when any individual assertion is created, released, or modified.
 *                  Caller has to call IOPMAssertionNotify() and register with powerd 
 *                  with this string before calling notify_register.
 *
 */
#define kIOPMAssertionsAnyChangedNotifyString               "com.apple.system.powermanagement.assertions.anychange"


/*!
 * @constant        kIOPMAssertionsChangedNotifyString
 * @discussion      Assertion notify(3) string
 *                  Fires when global assertion levels change. This notification 
 *                  doesn't necessarily fire when any individual assertion is 
 *                  created, released, or modified.
 *                  Caller has to call IOPMAssertionNotify() and register with powerd 
 *                  with this string before calling notify_register.
 */
#define kIOPMAssertionsChangedNotifyString                  "com.apple.system.powermanagement.assertions"

/*!
 * @constant        kIOPMAssertionsCollectBTString
 * @discussion      Assertion notify(3) string
 *                  Processes creating assertions listen on this string to enable/disable
 *                  backtrace collection. When backtrace collected, processes send backtrace
 *                  to powerd along with create request.
 */
#define kIOPMAssertionsCollectBTString                      "com.apple.powermanagement.collectbt"

/*!
 * @constant        kIOPMAssertionsLogBufferHighWM
 * @discussion      Assertion notify(3) string
 *                  Fires when assertion activity log buffer reaches high water mark.
 */
#define kIOPMAssertionsLogBufferHighWM                      "com.apple.powermanagement.assertions.logHighWM"

/*!
 * @constant        kIOPMDisableAppSleepPrefix
 * @discussion      Assertion notify(3) string
 *                  Fires when a process raises an assertion of a type that can prevent app sleep.
 *                  This notification is posted only if power assertions are not currently preventing that app's sleep.
 *              
 *                  Note: This is a prefix only. Actual notification will be appended with PID of the process
 *                  creating the power assertion. eg. com.apple.system.powermanagement.disableappsleep.1234
 *
 */
#define kIOPMDisableAppSleepPrefix                          "com.apple.system.powermanagement.disableappsleep"

/*!
 * @constant        kIOPMEnableAppSleepPrefix
 * @discussion      Assertion notify(3) string
 *                  Fires when there are no more active assertions that can prevent app sleep from a process.
 *                  This notification is posted only if app sleep is currently prevented due to power assertions.
 *              
 *                  Note: This is a prefix only. Actual notification will be appended with PID of the process
 *                  creating the power assertion. eg. com.apple.system.powermanagement.enableappsleep.1234
 *
 */
#define kIOPMEnableAppSleepPrefix                           "com.apple.system.powermanagement.enableappsleep"

/*!
 * @constant        kIOPMLastWakeTimeString
 * @discussion      Assertion notify(3) string
 *                  Last wake time is posted on this string after powerd calculates the value on wake.
 *
 */
#define kIOPMLastWakeTimeString                             "com.apple.powermanagement.lastwaketime"

/*!
 * @constant        kIOPMLastWakeTimeSMCDataString
 * @discussion      Assertion notify(3) string
 *                  Last wake time obtained from SMC is posted on this string after powerd calculates the value on wake.
 *
 */
#define kIOPMLastWakeTimeSMCDataString                      "com.apple.powermanagement.lastwaketimesmcdata"

/*!
 * @constant        kIOPMAssertForUserProximityString
 * @discussion      Assertion notify(3) string
 *                  Notification posted when assertion is created/released for user proximity.
 *                  State is set to 1 when sleep is prevented for user proximity. Otherwise, it is set to 0.
 */
#define kIOPMAssertForUserProximityString                   "com.apple.system.powermanagement.assertuserproximity"


/*! 
 * @define          kIOPMAssertionTimeoutActionKillProcess
 *
 * @discussion      When a timeout expires with this action, Power Management will log the timeout event,
 *                  and will kill the process that created the assertion. Signal SIGTERM is issued to 
 *                  that process.
 */
#define kIOPMAssertionTimeoutActionKillProcess              CFSTR("TimeoutActionKillProcess")


/*! @function       IOPMPerformBlockWithAssertion
 *
 *  @abstract       Runs a caller-specified block while holding a power assertion.
 *
 *  @discussion     This is a convenience API, it can simplify some power assertion uses.
 *
 *  @param          assertion_properties is a CFDictionary describing the assertion to hold while
 *                  executing <code>the_block</code>. See headerdoc comments for 
 *                  <comment>IOPMAssertionCreateWithProperties</code> describing dictionary format.
 *
 *  @param          the_block is a block that doesn't take any arguments. IOPMPerformBlockWithAssertion
 *                  will invoke the_block synchronously on the calling thread, and run until it completes.
 *
 *  @result         Returns kIOReturnSuccess when the block is executed.
 *                  Otherwise, appropriate error code is returned.
 */
IOReturn IOPMPerformBlockWithAssertion(CFDictionaryRef assertion_properties, dispatch_block_t the_block);

/*! @function IOPMAssertionSetTimeout
 *  @abstract Set a timeout for the given assertion.
 *  @discussion When the timeout fires, the assertion will be logged and a general notification
 *  will be delivered to all interested processes. The notification will not identify the
 *  timed-out assertion. The assertion will remain valid & in effect until it's released (if ever).
 *  Timeouts are bad! Timeouts are a hardcore last chance for debugging hard to find assertion
 *  leaks. They are not meant to fire under any normal circumstances.
 */
IOReturn IOPMAssertionSetTimeout(IOPMAssertionID whichAssertion, CFTimeInterval timeoutInterval);


/*! @function           IOPMAssertionCreateWithAutoTimeout
 *
 * @abstract            A variant of IOPMAssertionCreateWithProperties to create assertion with property
 *                      kIOPMAssertionAutoTimesout set to true.
 *
 * @description         Creates an IOPMAssertion with property kIOPMAssertionAutoTimesout set to true.
 *                      It allows the caller to specify the Name, Details, and HumanReadableReason at creation time.
 *                      There are other keys that can further describe an assertion, but most developers don't need
 *                      to use them. Use <code>@link IOPMAssertionSetProperty @/link</code> or
 *                      <code>@link IOPMAssertionCreateWithProperties @/link</code> if you need to specify properties
 *                      that aren't available here.
 *
 * @param AssertionType An assertion type constant.
 *	                    Caller must specify this argument.
 *
 * @param Name          A CFString value to correspond to key <code>@link kIOPMAssertionNameKey@/link</code>.
 *	                    Caller must specify this argument.
 *
 * @param Details 	    A CFString value to correspond to key <code>@link kIOPMAssertionDetailsKey@/link</code>.
 *	                    Caller my pass NULL, but it helps power users and administrators identify the
 *                      reasons for this assertion.
 *
 * @param HumanReadableReason
 *                      A CFString value to correspond to key <code>@link kIOPMAssertionHumanReadableReasonKey@/link</code>.
 *                      Caller may pass NULL, but if it's specified OS X may display it to users
 *                      to describe the active assertions on their system.
 *
 * @param LocalizationBundlePath
 *	                    A CFString value to correspond to key <code>@link kIOPMAssertionLocalizationBundlePathKey@/link</code>.
 *                      This bundle path should include a localization for the string <code>HumanReadableReason</code>
 *                      Caller may pass NULL, but this argument is required if caller specifies <code>HumanReadableReason</code>
 *
 * @param Timeout       Specifies a timeout for this assertion. Pass 0 for no timeout.
 *                      If the timeout value is larger than the auto-timeout value, it may get reduced to auto-timeout value.
 *
 * @param TimeoutAction Specifies a timeout action. Caller my pass NULL. If a timeout is specified but a TimeoutAction is not,
 *                      the default timeout action is <code>kIOPMAssertionTimeoutActionTurnOff</code>
 *
 * @param AssertionID   (Output) On successful return, contains a unique reference to a PM assertion.
 *
 * @result              kIOReturnSuccess, or another IOKit return code on error.
 */
IOReturn	IOPMAssertionCreateWithAutoTimeout(
                                           CFStringRef  AssertionType,
                                           CFStringRef  Name,
                                           CFStringRef  Details,
                                           CFStringRef  HumanReadableReason,
                                           CFStringRef  LocalizationBundlePath,
                                           CFTimeInterval   Timeout,
                                           CFStringRef  TimeoutAction,
                                           IOPMAssertionID  *AssertionID)
__OSX_AVAILABLE_STARTING(__MAC_NA, __IPHONE_10_0);


/*! @function           IOPMAssertionCreateWithResourceList
 *
 * @abstract            A variant of IOPMAssertionCreateWithProperties to create assertion with property
 *                      kIOPMAssertionResourcesUsed set to specified resources.
 *
 * @description         Creates an IOPMAssertion with property kIOPMAssertionResourcesUsed set to specified resources.
 *                      It allows the caller to specify the Name, Details, and HumanReadableReason at creation time.
 *                      There are other keys that can further describe an assertion, but most developers don't need
 *                      to use them. Use <code>@link IOPMAssertionSetProperty @/link</code> or
 *                      <code>@link IOPMAssertionCreateWithProperties @/link</code> if you need to specify properties
 *                      that aren't available here.
 *
 * @param AssertionType An assertion type constant.
 *	                    Caller must specify this argument.
 *
 * @param Name          A CFString value to correspond to key <code>@link kIOPMAssertionNameKey@/link</code>.
 *	                    Caller must specify this argument.
 *
 * @param Details 	    A CFString value to correspond to key <code>@link kIOPMAssertionDetailsKey@/link</code>.
 *	                    Caller my pass NULL, but it helps power users and administrators identify the
 *                      reasons for this assertion.
 *
 * @param HumanReadableReason
 *                      A CFString value to correspond to key <code>@link kIOPMAssertionHumanReadableReasonKey@/link</code>.
 *                      Caller may pass NULL, but if it's specified OS X may display it to users
 *                      to describe the active assertions on their system.
 *
 * @param LocalizationBundlePath
 *	                    A CFString value to correspond to key <code>@link kIOPMAssertionLocalizationBundlePathKey@/link</code>.
 *                      This bundle path should include a localization for the string <code>HumanReadableReason</code>
 *                      Caller may pass NULL, but this argument is required if caller specifies <code>HumanReadableReason</code>
 *
 * @param Timeout       Specifies a timeout for this assertion. Pass 0 for no timeout.
 *
 * @param TimeoutAction Specifies a timeout action. Caller my pass NULL. If a timeout is specified but a TimeoutAction is not,
 *                      the default timeout action is <code>kIOPMAssertionTimeoutActionTurnOff</code>
 *
 * @param Resources     A CFArray with the list of resources used by the process while holding this assertion. Possible values
 *                      are listed with the documentation of <code>kIOPMAssertionResourcesUsed</code>
 *
 * @param AssertionID   (Output) On successful return, contains a unique reference to a PM assertion.
 *
 * @result              kIOReturnSuccess, or another IOKit return code on error.
 */
IOReturn	IOPMAssertionCreateWithResourceList(
                                           CFStringRef  AssertionType,
                                           CFStringRef  Name,
                                           CFStringRef  Details,
                                           CFStringRef  HumanReadableReason,
                                           CFStringRef  LocalizationBundlePath,
                                           CFTimeInterval   Timeout,
                                           CFStringRef  TimeoutAction,
                                           CFArrayRef   Resources,
                                           IOPMAssertionID  *AssertionID)
__OSX_AVAILABLE_STARTING(__MAC_NA, __IPHONE_10_0);



/*
 * CFStrings defined for keys in dictionary returned by IOPMCopyAssertionActivityLog()
 */
#define     kIOPMAssertionActivityTime      CFSTR("ActivityTime")
#define     kIOPMAssertionActivityAction    CFSTR("Action")


/*
 *! @function   IOPMSetAssertionActivityLog
 *  @abstract   Enable/disable assertion activity log by powerd. Activity logging must be enabled explicitly
 *              before any calling IOPMCopyAssertionActivityLog().
 *
 *  @param enable           If true, logging is enabled, otherwise logging is disabled
 *
 *  @result                 Return kIOReturnSuccess on success.
 */
IOReturn IOPMSetAssertionActivityLog(bool enable);

/*
 *!  @function  IOPMCopyAssertionActivityLog
 *   @abstract  Internally calls IOPMCopyAssertionActivityUpdate().
 *   @result                 Return kIOReturnSuccess on success.
 */
IOReturn IOPMCopyAssertionActivityLog(CFArrayRef *assertionLog, bool *overflow);

/*
 *!  @function  IOPMCopyAssertionActivityLogWithAllocator
 *   @abstract  Internally calls IOPMCopyAssertionActivityUpdate().
 *   @result                 Return kIOReturnSuccess on success.
 */
IOReturn IOPMCopyAssertionActivityLogWithAllocator(CFArrayRef *assertionLog, bool *overflow, CFAllocatorRef allocator);

/*
 *!  @function  IOPMCopyAssertionActivityUpdate / IOPMCopyAssertionActivityUpdateWithAllocator
 *   @abstract  Returns all assertions related activity since last time this function is called.
 *              If this is called for first time, all available activity is returned. Depending on the 
 *              frequency of assertion activity and any resets to the activity log, some of the activity 
 *              entries may be lost.
 *
 *   @param  logUpdates     If successful, this field will have a pointer to array of dictionaries, each 
 *                          dictionary with below key/value pairs. Each entry in the array points to one 
 *                          assertion related activity since last read.
 *              CFDictionary
 *              {
 *                  kIOPMAssertionActivityTime          : CFDateRef, Time when this assertion activity occurred
 *                  kIOPMAssertionTypeKey               : CFStringRef, Type of assertion
 *                  kIOPMAssertionNameKey               : CFStringRef, Description string passed to powerd 
 *                                                        when assertion is created
 *                  kIOPMAssertionActivityAction        : CFStringRef, This will be one of 'Create', 
 *                                                        'Retain', 'Release', 'ClientDeath', 'Timeout', 
 *                                                        'Turnoff', 'Turnon'
 *                  kIOPMAssertionPIDKey                : CFNumberRef, PID of the creating process
 *
 *                  kIOPMAssertionRetainCountKey        : CFNumberRef, Retain count of this assertion
 *                  kIOPMAssertionGlobalUniqueIDKey     : CFNumberRef, Unique Id of the assertion
 *                  kIOPMAssertionOnBehalfOfPID         : CFNumberRef, If available, specifies the PID of the process 
 *                                                        on whose behalf this assertion is created
 *                  kIOPMAssertionOnBehalfOfPIDReason   : CFStringRef, If available, description on why assertion 
 *                                                        is created on behalf of PID
 *                  kIOPMAssertionOnBehalfOfBundleID    : CFStringRef, If available, specifies the Bundle ID
 *                                                        on whose behalf this assertion is created
 *                  kIOPMAssertionCreatorBTSymbols      : CFArray of CFStrings giving the backtrace
 *              }
 *
 *  @param  overflow        Set to true if some of the old assertion activity entries are lost between last call
 *                          to this function and this call.
 *
 *  @param  prevRefCnt      Caller should set this to UINT_MAX on first call. Ons subsequent calls, caller should set this
 *                          to value returned in previous call. This field is used by IOKit as reference index of 
 *                          last entry returned on previous call.
 *
 *  @result                 Return kIOReturnSuccess on success.
 */
IOReturn IOPMCopyAssertionActivityUpdate(CFArrayRef *logUpdates, bool *overflow, uint32_t *prevRefCnt);
IOReturn IOPMCopyAssertionActivityUpdateWithAllocator(CFArrayRef *logUpdates, bool *overflow, uint32_t *prevRefCnt, CFAllocatorRef allocator);

/*!
 * @function            IOPMCopyAssertionsByProcessWithAllocator
 *
 * @abstract            Returns a dictionary listing all assertions, grouped by their owning process.
 *                      Custom allocator is passed in as parameter by the client.
 *
 * @discussion          Notes: One process may have multiple assertions. Several processes may
 *                      have asserted the same assertion to different levels.
 *
 * @param AssertionsByPID On success, this returns a dictionary of assertions per process.
 *                      At the top level, keys to the CFDictionary are pids stored as CFNumbers (kCFNumberIntType).
 *                      The value associated with each CFNumber pid is a CFArray of active assertions.
 *                      Each entry in the CFArray is an assertion represented as a CFDictionary. See the keys
 *                      kIOPMAssertionTypeKey and kIOPMAssertionLevelKey.
 *                      Caller must CFRelease() this dictionary when done.
 *
 * @result              Returns kIOReturnSuccess on success.
 */
IOReturn IOPMCopyAssertionsByProcessWithAllocator(CFDictionaryRef *AssertionsByPID, CFAllocatorRef allocator);

/*
 *! @function   IOPMCopyAssertionsByType
 *  @abstract   Copies all active assertions with the specified type.
 *
 *  @param type             Assertion type
 *
 *  @param assertionsArray  On Success, contains the list of assertions for the specified type
 *
 *  @result                 Return kIOReturnSuccess on success.
 */

IOReturn IOPMCopyAssertionsByType(CFStringRef type, CFArrayRef *assertionsArray);

/*
 *! @function   IOPMCopyInactiveAssertionsByProcess
 *
 *  @abstract            Returns a dictionary listing all inactive assertions, grouped by their owning process.
 *
 * @discussion          Notes: One process may have multiple assertions. Several processes may
 *                      have asserted the same assertion to different levels.
 *
 * @param AssertionsByPID On success, this returns a dictionary of inactive assertions per process.
 *                      At the top level, keys to the CFDictionary are pids stored as CFNumbers (kCFNumberIntType).
 *                      The value associated with each CFNumber pid is a CFArray of active assertions.
 *                      Each entry in the CFArray is an assertion represented as a CFDictionary. See the keys
 *                      kIOPMAssertionTypeKey and kIOPMAssertionLevelKey.
 *                      Caller must CFRelease() this dictionary when done.
 *
 * @result              Returns kIOReturnSuccess on success.
 */

IOReturn IOPMCopyInactiveAssertionsByProcess(CFDictionaryRef  *inactiveAssertions);

/*
 *! @function   IOPMCopyDeviceRestartPreventers
 *  @abstract   Copies all active assertions that will prevent device restart.
 *
 *  @param preventers       On Success, contains the list of assertions that prevent device restart.
 *
 *  @result                 Return kIOReturnSuccess on success.
 */
IOReturn IOPMCopyDeviceRestartPreventers(CFArrayRef *preventers);

/*
 *! @function   IOPMSetAssertionActivityAggregate
 *  @abstract   Enable/disable assertion activity duration aggregates by powerd. Activity aggregation must be enabled explicitly
 *              before any calling IOPMCopyAssertionActivityAggregate().
 *
 *  @param enable           If true, logging is enabled, otherwise logging is disabled
 *
 *  @result                 Return kIOReturnSuccess on success.
 */
IOReturn IOPMSetAssertionActivityAggregate(bool enable);


/*
 * ! @function  IOPMCopyAssertionActivityAggregate
 *   @abstract  Returns aggregate duration for which each process held the assertion with in the specified time frame
 *
 *  @result     Returns CFDictionary on success.
 *              The CFDictionary returned by this function is an aggregate of all processes preventing sleep
 *              since the aggregation is enabled by calling IOPMSetAssertionActivityAggregate(). This dictionary
 *              is in the format of Simple Array report and can be interpreted using IOReporting library APIs as
 *              shown below:
 *
 *
 *      IOReportIterate(dict, ^(IOReportChannelRef ch) {
 *              printf(" %llu %lld %lld %lld\n", IOReportChannelGetChannelID(ch),
 *                      IOReportArrayGetValueForIndex(ch, 1), 
 *                      IOReportArrayGetValueForIndex(ch, 2),
 *                      IOReportArrayGetValueForIndex(ch, 3));
 *      });
 *
 *      'dict' is the dictionary returned by IOPMCopyAssertionActivityAggregate(). IOReportChannelGetChannelID()
 *      returns the PID of the process. IOReportArrayGetValueForIndex(ch, 1) returns duration in seconds for which
 *      this process prevented Idle sleep since the first call to IOPMSetAssertionActivityAggregate().
 *      Similarly, value at index 2 returns duration in seconds for which this process prevented demand sleep and
 *      value at index 3 returns duration for which this process prevented display sleep.
 *
 *      Delta of two outputs from IOPMCopyAssertionActivityAggregate() can be obtained by calling IOReporting API 
 *      IOReportCreateSamplesDelta().
 */
CFDictionaryRef IOPMCopyAssertionActivityAggregate( );

/*
 * ! @function  IOPMCopyAssertionActivityAggregateWithAllocator
 *   @abstract  Returns aggregate duration for which each process held the assertion with in the specified time frame with
 *              custom allocator
 *
 *  @param allocator   A custom allocator reference to be used for allocation of the result CFDictionary.
 *
 *
 *  @result     Returns CFDictionary on success.
 *              The CFDictionary returned by this function is an aggregate of all processes preventing sleep
 *              since the aggregation is enabled by calling IOPMSetAssertionActivityAggregate(). This dictionary
 *              is in the format of Simple Array report and can be interpreted using IOReporting library APIs as
 *              shown below:
 *
 *
 *      IOReportIterate(dict, ^(IOReportChannelRef ch) {
 *              printf(" %llu %lld %lld %lld\n", IOReportChannelGetChannelID(ch),
 *                      IOReportArrayGetValueForIndex(ch, 1),
 *                      IOReportArrayGetValueForIndex(ch, 2),
 *                      IOReportArrayGetValueForIndex(ch, 3));
 *      });
 *
 *      'dict' is the dictionary returned by IOPMCopyAssertionActivityAggregate(). IOReportChannelGetChannelID()
 *      returns the PID of the process. IOReportArrayGetValueForIndex(ch, 1) returns duration in seconds for which
 *      this process prevented Idle sleep since the first call to IOPMSetAssertionActivityAggregate().
 *      Similarly, value at index 2 returns duration in seconds for which this process prevented demand sleep and
 *      value at index 3 returns duration for which this process prevented display sleep.
 *
 *      Delta of two outputs from IOPMCopyAssertionActivityAggregate() can be obtained by calling IOReporting API
 *      IOReportCreateSamplesDelta().
 */
CFDictionaryRef IOPMCopyAssertionActivityAggregateWithAllocator(CFAllocatorRef allocator);

/*
 * ! @function  IOPMSetAssertionExceptionLimits
 *   @abstract  Allows caller to set assertion exception limits.
 *
 *  @param procLimits   CFDictionary. Each key is a process name and the value is a dictionary with below keys:
 *                      kIOPMAssertionDurationLimit: CFNumber, Max duration(in seconds) before which assertion should be released
 *                      kIOPMAggregateAssertionLimit: CFNunber, Max duration(in seconds) for which the process can prevent sleep in an
 *                                                    hour of system run time. This is measured across system sleep/wakes.
 *
 *                      If 'procLimits' has a dictionary with key 'kIOPMDefaultLimtsKey', the limits specified in that 
 *                      dictionary will used for all processes that are not explictily specified in 'procLimits'.
 *                      Additionally, 'kIOPMDefaultLimtsKey' can have below keys:
 *                      kIOPMAggregateFrequencyLimit - Frequency at which aggregate assertion limit has to be checked
 *                      kIOPMMinSleepDurationLimit - Minimum expected Sleep duration
 *                      kIOPMMinWakeDurationLimit - Minimum expected Wake duration
 *                      kIOPMFrequentSleepWakeLimit - Number of short sleep/wake cycles to trigger exception
 *
 *
 *  @result     Returns kIOReturnSuccess on success.
 *
 *  This function should be called only by process with appropriate entiltlement. Any previously set
 *  exception limits dictionary will be over-written by this new dictionary.
 */
IOReturn IOPMSetAssertionExceptionLimits(CFDictionaryRef procLimits);

#define kIOPMAssertionDurationLimit     CFSTR("Asssertion Duration Limit")
#define kIOPMAggregateAssertionLimit    CFSTR("Aggregate Assertion Limit")

#define kIOPMDefaultLimtsKey            CFSTR("Default Limits")

// kIOPMAggregateFrequencyLimit - CFNumberRef, frequency(in seconds) at which aggregate assertion duration
// has to be monitored. Defaults to 2 hrs
#define kIOPMAggregateFrequencyLimit    CFSTR("Aggregate Monitor Frequency")

// kIOPMMinSleepDurationLimit - CFNumberRef, sleep duration(in seconds) below which that sleep is considered
// a short sleep
#define kIOPMMinSleepDurationLimit      CFSTR("Minimum Sleep Duration")

// kIOPMMinWakeDurationLimit - CFNumberRef, wake duration(in seconds) below which that wake is considered
// a short wake
#define kIOPMMinWakeDurationLimit       CFSTR("Minimum Wake Duration")

// kIOPMFrequentSleepWakeLimit - CFNumberRef, number of consecutive sleep/wakes that should trigger
// a kIOPMFrequentSleepWakeException exception
#define kIOPMFrequentSleepWakeLimit     CFSTR("Frequent SW Limit")

/* Assertion Exception types */
typedef enum  {
    kIOPMAssertionDurationException = 1,
    kIOPMAssertionAggregateException,
    kIOPMFrequentSleepWakeException,
} IOPMAssertionException;

typedef uintptr_t IOPMNotificationHandle;

/*
 * ! @function    IOPMScheduleAssertionExceptionNotification
 *   @abstract    Register a block to be called on asssertion exceptions.
 *
 *
 * @param       queue The dispatch_queue on which the registered block is called.
 *
 * @param       block The block to execute for assertion exceptions.
 *              The block will receive two arguments:
 *                  IOPMAssertionException  exceptionType
 *                  pid_t                   Process ID causing th exception
 *
 * @result      NULL on failure. Upon success, caller should store the
 *              <code>IOPMNotificationHandle</code> and pass that handle to
 *              <code>IOPMUnregisterExceptionNotification</code> to stop
 *              receiving notifications.
 */
IOPMNotificationHandle IOPMScheduleAssertionExceptionNotification(dispatch_queue_t queue,
                                                                  void (^block)(IOPMAssertionException, pid_t));
/*
 * ! @function    IOPMUnregisterExceptionNotification
 *   @abstract    Unregister notifications for assertion exceptions
 *
 *
 * @param       handle   IOPMNotificationHandle returned with IOPMScheduleAssertionExceptionNotification.
 *
 * @result      None.
 */
void IOPMUnregisterExceptionNotification(IOPMNotificationHandle handle);


#define kIOPMAssertionExceptionNotifyName   "com.apple.powermanagement.assertionexception"

/*
 * Deprecated assertion constants
 */
    // RY: Look's like some embedded clients are still dependent on the following
    #define kIOPMPreventIdleSleepAssertion              kIOPMAssertionTypeNoIdleSleep
    #define kIOPMEnableIdleSleepAssertion               kIOPMAssertionTypeEnableIdleSleep
    enum {
        kIOPMAssertionDisable                           = kIOPMAssertionLevelOff,
        kIOPMAssertionEnable                            = kIOPMAssertionLevelOn,
        kIOPMAssertionIDInvalid                         = kIOPMNullAssertionID
     };
    #define kIOPMAssertionValueKey                      kIOPMAssertionLevelKey

/**************************************************
*
* Energy Saver Preferences - Constants
*
**************************************************/
/*!
 * @group PM Preferences
 */

#define kIOPMDynamicStoreSettingsKey                    "State:/IOKit/PowerManagement/CurrentSettings"

// units - CFNumber in minutes
#define kIOPMDisplaySleepKey                            "Display Sleep Timer"
// units - CFNumber in minutes
#define kIOPMDiskSleepKey                               "Disk Sleep Timer"
// units - CFNumber in minutes
#define kIOPMSystemSleepKey                             "System Sleep Timer"
// units - CFNumber 0/1
#define kIOPMWakeOnLANKey                               "Wake On LAN"
// units - CFNumber 0/1
#define kIOPMWakeOnRingKey                              "Wake On Modem Ring"
// units - CFNumber 0/1
#define kIOPMRestartOnPowerLossKey                      "Automatic Restart On Power Loss"
// units - CFNumber 0/1
#define kIOPMWakeOnACChangeKey                          "Wake On AC Change"
// units - CFNumber 0/1
#define kIOPMSleepOnPowerButtonKey                      "Sleep On Power Button"
// units - CFNumber 0/1
#define kIOPMWakeOnClamshellKey                         "Wake On Clamshell Open"
// units - CFNumber 0/1
#define kIOPMReduceBrightnessKey                        "ReduceBrightness"
// units - CFNumber 0/1
#define kIOPMDisplaySleepUsesDimKey                     "Display Sleep Uses Dim"
// units - CFNumber 0/1
#define kIOPMMobileMotionModuleKey                      "Mobile Motion Module"
// units - CFNumber 0/1
#define kIOPMTTYSPreventSleepKey                        "TTYSPreventSleep"
// units - CFNumber 0/1
#define kIOPMGPUSwitchKey                               "GPUSwitch"
// units - CFNumber 0/1
#define kIOPMDarkWakeBackgroundTaskKey                  "DarkWakeBackgroundTasks"
// units - CFNumber 0/1
#define kIOPMPowerNapSupportedKey                       "PowerNap"
// units - CFNumber 0/1
#define kIOPMSilentRunningKey                           "SilentRunning"
// units - CFNumber 0/1
#define kIOPMSleepServicesKey                           "SleepServices"
// units - CFNumber 0/1
#define kIOPMUnifiedSleepSliderPrefKey                  "UnifiedSleepSliderPref"
// units - CFNumber 0/1
#define kIOPMTCPKeepAlivePrefKey                        "TCPKeepAlivePref"
// units - CFNumber 0/1
#define kIOPMProximityDarkWakeKey                       "ProximityDarkWake"
// units - CFNumber 0/1
#define kIOPMProModeKey                                 "ProMode"

#define kIOPMUpdateDarkWakeBGSettingKey                 "Update DarkWakeBG Setting"
#define kIOPMDarkWakeLingerDurationKey                  "DarkWake Linger Duration"
#define kIOPMAdaptiveDisplaySleepKey                    "Adaptive Display Sleep"
#define kIOPMAdaptiveStandbySleepKey                    "Adaptive Standby Sleep"

#define kIOPMCarrierMode                                "Carrier Mode"
#define kIOPMCarrierModeVh                              "Carrier Mode High Voltage"
#define kIOPMCarrierModeVl                              "Carrier Mode Low Voltage"

#define kIOPMVact                                       "VAC-T"

// Restart on Kernel panic
// Deprecated in 10.8. Do not use.
#define kIOPMRestartOnKernelPanicKey                    "RestartAfterKernelPanic"

/*!
 * @constant    kIOPMSystemProModeEnaged
 * @abstract    Notify(3) string that PM fires every time the system enters ProMode.
 */
#define kIOPMSystemProModeEngaged						"com.apple.system.promode.engaged"

/*!
 * @constant    kIOPMSystemProModeDisengaged
 * @abstract    Notify(3) string that PM fires every time the system exits ProMode
 */
#define kIOPMSystemProModeDisengaged					"com.apple.system.promode.disengaged"


// See xnu/iokit/IOKit/pwr_mgt/IOPM.h for other PM Settings keys:
//  kIOPMDeepSleepEnabledKey 
//  kIOPMDeepSleepDelayKey 

/*!
 * kIOPMPrioritizeNetworkReachabilityOverSleepKey
 *
 * mDNSResponder will transfer responsibility for publishing network services
 * to a Bonjour sleep proxy during sleep. 
 *      
 * This PM Feature has a value of:
 *      kCFBooleanTrue = If no sleep proxy is available, mDNSResponder will 
 *          prevent system sleep with a PM assertion to maintain network connections.
 *
 *      kCFBooleanFalse = If there is no sleep proxy available, the system will 
 *          fall asleep and prevent network service accessibility.
 *
 * mDNSResponder is responsible for acting upon this key.
 * value is false/true as a CFNumber 0/1
 */
#define kIOPMPrioritizeNetworkReachabilityOverSleepKey  "PrioritizeNetworkReachabilityOverSleep"


// kIOPMReduceSpeedKey
// Deprecated; do not use. Only relevant to PowerPC systems.
#define kIOPMReduceSpeedKey                             "Reduce Processor Speed"

// kIOPMDynamicPowerStepKey
// Deprecated; do not use. Only relevant to PowerPC systems.
#define kIOPMDynamicPowerStepKey                        "Dynamic Power Step"

/**************************************************
 *
 * Energy Saver Preferences - Functions
 *
 **************************************************/

#define kIOPMCFPrefsPath                                "com.apple.PowerManagement"

#define kIOPMSystemPowerSettingsKey                     "SystemPowerSettings"
/*!
 * @constant        kIOPMPrefsChangeNotify
 * @discussion      Preferences change notify(3) string
 *                  Fires when any preference change
 *                  occurs via IOPMSetPMPreferences
 */
#define kIOPMPrefsChangeNotify                          "com.apple.system.powermanagement.prefschange"

/*
 * @function    IOPMRegisterPrefsChangeNotification
 *
 * Get a callout when the power management preferences change.
 *
 * @param       queue The dispatch_queue to schedule the callout on.
 *
 * @param       block The block to execute when the state changes.
 *
 * @result      NULL on failure. Upon success, caller should store the
 *              <code>IOPMNotificationHandle</code> and pass that handle to
 *              <code>IOPMUnregisterPrefsChangeNotification</code> to stop
 *              receiving notifications.
 */


IOPMNotificationHandle IOPMRegisterPrefsChangeNotification(dispatch_queue_t queue,
                                                           void (^block)());

/*
 * @function    IOPMUnregisterPrefsChangeNotification
 *
 * @abstract    Cancel and release an <code>IOPMNotificationHandle</code>
 */
void IOPMUnregisterPrefsChangeNotification(IOPMNotificationHandle handle);

/* Structure of a "pm preferences" dictionary, as referenced below.
 *
 * PMPreferencesDictionary = 
 * {
 *      "AC Power" = PowerSourcePreferencesDictionary
 *      "Battery Power" = PowerSourcePreferencesDictionary
 *      "UPS Power" = PowerSourcePreferencesDictionary
 *      "UPS Shutdown levels" = Shutdown levels dictionary
 *      "SystemPowerSettings" = System settings dictionary
 * }
 *
 * PowerSourcePreferencesDictionary = 
 * {
 *      kIOPMDiskSleepKey = 0 and up
 *      kIOPMSystemSleepKey = 0 and up
 *      kIOPMDisplaySleepKey = 0 and up
 *
 *      kIOPMWakeOnLANKey = 0/1
 *      kIOPMWakeOnRingKey = 0/1
 *      kIOPMWakeOnACChangeKey = 0/1
 *      ... etc
 * }
 *
 *
 * Notes: 
 * * If battery power or UPS power are not present on a system, they will
 *   be absent from the PM Preferences dictionary.
 * * If a given setting, like ReduceBrightness is unsupported on a given
 *   system, it will not appear in the PowerSourcePreferencesDictionary.
 */
/*!
@function IOPMCopyPreferencesOnFile
@abstract Returns a dictionary of custom PM preferences saved to disk
          INTERNAL TO POWRED ONLY --- Returns settings that are saved to disk. This will
          have all settings modified by user and may have some settings at default values
@result Returns a CFDictionaryRef or NULL. Caller must CFRelease the dictionary.
*/

CFMutableDictionaryRef IOPMCopyPreferencesOnFile(void);

/*!
@function IOPMCopyPMPreferences
@abstract Returns a dictionary of user-selected custom PM preferences.
@result Returns a CFDictionaryRef or NULL. Caller must CFRelease the dictionary.
*/
CFMutableDictionaryRef IOPMCopyPMPreferences(void);

#if !TARGET_OS_IPHONE

/* 
 * @function IOPMCopyPMSetting
 * @abstract        SPI to discover whether HW Supports a setting, and if so what it's
 *                  currently set to (on each power source).
 * @param key       Please pass in a Energy Settings CFStringRef key from IOPMLibPrivate.h
 * @param power_source	Please specify which power source to query. One of:
 *                      <code>CFSTR(kIOPMACPowerKey)</code>, or <code>CFSTR(kIOPMBatteryPowerKey)</code>,
 *                      <code>CFSTR(kIOPMUPSPowerKey)</code>, or <code>NULL</code> to specify the 
 *                      current power source.
 * @param outValue	Upon success, will return the CFTypeRef value for the setting. Caller must release.
 * @result          kIOReturnSuccess if it can return a valid value.
 *                  kIOReturnUnsupported if the setting isn’t supported on this hardware.
 */
IOReturn IOPMCopyPMSetting(CFStringRef key, CFStringRef power_source, CFTypeRef *outValue);
#endif

/*!
@function IOPMCopyActivePMPreferences
@abstract Same as IOPMCopyPMPreferences
@discussion This is the same as IOPMCopyPMPreferences
 */
CFDictionaryRef     IOPMCopyActivePMPreferences(void);

/*!
 @function IOPMCopyDefaultPreferences
 @abstract Returns the machine's default power management preferences.
 @result Returns a CFDictionaryRef or NULL. Caller must CFRelease the dictionary.
 */
CFDictionaryRef IOPMCopyDefaultPreferences(void);

/*!
 @function      IOPMSetPMPreference
 @abstract      Multi purpose function for setting/reverting PM preferences.
 @discussion    A user can provide a key/value pair for a power management setting.
                Passing in NULL for the key will revert ALL keys for the given power
                source. Passing in NULL for value will revert the preference for the
                given power source. Passing in NULL For pwr_src will apply the key/value
                pair to all supported power sources.
 @param key     An Energy Settings CFStringRef key from IOPMLibPrivate.h or NULL.
 @param value   Setting value, or NULL.
 @param pwr_src CFSTR(kIOPMACPowerKey), CFSTR(kIOPMBatteryPowerKey), 
                CFSTR(kIOPMUPSPowerKey), or NULL for all power sources.
 @result        Returns kIOReturnSuccess or an error condition if request failed.
 */
IOReturn IOPMSetPMPreference(CFStringRef key, CFTypeRef value, CFStringRef pwr_src);

/*!
@function IOPMSetPMPreferences
@abstract Writes a PM preferences dictionary to disk.
    Also activates these preferences and sends notifications to interested
    applications via SystemConfiguration.
@param ESPrefs  Dictionary of Power Management preferences to write out to disk.
@result Returns kIOReturnSuccess or an error condition if request failed.
 */
IOReturn IOPMSetPMPreferences(CFDictionaryRef ESPrefs);

/*!
 @function          IOPMUsingDefaultPreferences
 @abstract          Identifies if the machine is using default preferences 
                    for a given power source. Note: If a user manually restores
                    settings by modifying the plist directly, or by manually setting
                    the value to its default via energy preferences, this SPI may
                    return an incorrect value.
 @param     pwr_src CFSTR(kIOPMACPowerKey), CFSTR(kIOPMBatteryPowerKey),
                    CFSTR(kIOPMUPSPowerKey), or NULL for all power sources.
 @result            True if using defaults, false otherwise.
 */
bool IOPMUsingDefaultPreferences(CFStringRef pwr_src);

/*!
@function IOPMRevertPMPreferences
@abstract Revert the specified settings to their system default values
            across all power sources. Must be called as root.
@param keys_arr  A CFArrayRef of CFStrings. Each string should be a valid IOPM
            setting key.
            Pass NULL to revert all settings to the OS Energy Saver defaults.
@result kIOReturnSuccess on success; error otherwise.
 */
IOReturn IOPMRevertPMPreferences(CFArrayRef keys_arr);

/*!
 * @function        IOPMFeatureIsAvailable
 * @abstract        Identifies whether a PM feature is supported on this platform.
 * @param           feature The CFString feature to check for availability.
 * @param           power_source CFSTR(kIOPMACPowerKey) or CFSTR(kIOPMBatteryPowerKey). Doesn't 
 *                  matter for most features, but a few are power source dependent.
 * @result          Returns true if supported, false otherwise. 
 */
bool IOPMFeatureIsAvailable(CFStringRef feature, CFStringRef power_source);


/*!
 * @function        IOPMFeatureIsAvailableWithSupportedTable
 * @abstract        Like <code>@link IOPMFeatureIsAvailable@/link </code> but lets caller pass in a 
 *                  cached supported-features array.
 * @discussion      Provides an optimized path if you'll be making repeated calls to IOPMFeatureIsAvailable.
 *                  The "Supported features table" does not change often - it may change during boot-up, and it
 *                  may change when hardware is added or removed to the system.
 *
 *                  By providing a cached copy here, IOPMFeatureIsAvailable can avoid re-reading the feature support
 *                  table on every invocation.
 *
 *                  If you are only occasionally invoking IOPMFeatureIsAvailable for 1-2 settings, you don't need
 *                  to bother with IOPMFeatureIsAvailableWithSupportedTable. Look into it for a performance optimization
 *                  if you do make multiple calls.
 *
 * @param           PMFeature The CFString feature to check for availability.
 *
 * @param           power_source CFSTR(kIOPMACPowerKey) or CFSTR(kIOPMBatteryPowerKey). Doesn't
 *                  matter for most features, but a few are power source dependent.
 *
 * @param           supportedFeatures Pass in a CFDictionary describing available features, and which power sources
 *                  each supports. Obtain this dictionary from IOPMrootDomain in the IORegistry with code like this:
 *
 *                  io_registry_entry_t _rootDomain = 
 *                                   IORegistryEntryFromPath( kIOMasterPortDefault, 
 *                                   kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");
 *                  CFDictionaryRef cachedSupportedFeatures =  
 *                                  IORegistryEntryCreateCFProperty(_rootDomain, 
 *                                      CFSTR("Supported Features"), 
 *                                      kCFAllocatorDefault, kNilOptions);
 *
 * @result          Returns true on if supported, false otherwise.
 */
bool IOPMFeatureIsAvailableWithSupportedTable(
     CFStringRef                PMFeature, 
     CFStringRef                power_source,
     CFDictionaryRef            supportedFeatures);

/*!
 @function          IOPMRemoveIrrelevantProperties
 @abstract          Given a dictionary of energy preferences, removes
                    all unsuported key/values as well as power sources
                    associated with that machine. This is used in cases
                    such as migration, when a setting may not be avaiable
                    on the target machine.
 @param             A CFMutableDictionaryRef containing the energy preferences.
 */
void IOPMRemoveIrrelevantProperties(CFMutableDictionaryRef energyPrefs);


/*!
 @function          IOPMWriteToPrefs
 @abstract          Writes the specified key/value pair to PM Prefs
 @param             key, value - key/value pair
 @param             synchronize - should set to true if CFPreferencesSynchronize should be called.
 @param             broadcast - should set to true if this write need to be notified to all listeners
                    NOTE - INTERNAL TO PM. Don't use
 */
IOReturn IOPMWriteToPrefs(CFStringRef key, CFTypeRef value, bool synchronize, bool broadcast);

/*!
 @function          IOPMCopyFromPrefs
 @abstract          Copies the value from the appropriate prefs path(based on the key) and returns it
 * @param           prefsPath - The path to preferences file.
 *                              If set to NULL, function will find the appropriate prefs path
 * @param           key - Key  to read
 *                  NOTE - INTERNAL TO PM. Don't use
 */

CFTypeRef IOPMCopyFromPrefs(CFStringRef prefsPath, CFStringRef key);
/**************************************************/


    /*!
    @function IOPMSleepSystemWithOptions
    @abstract Request that the system initiate sleep.
    @discussion For security purposes, caller must be root or the console user.
    @param userClient  Port used to communicate to the kernel,  from IOPMFindPowerManagement.
    @param sleepOptions a dictionary defining optional arguments to sleep.
    @result Returns kIOReturnSuccess or an error condition if request failed.
     */
IOReturn IOPMSleepSystemWithOptions ( io_connect_t userClient, CFDictionaryRef sleepOptions );

/**************************************************/

/* 
 * kIOPMSystemSleepAvailableAtAll
 *      System Power Setting (not power source specific)
 *      value = true/false
 */
#define	kIOPMSleepDisabledKey	CFSTR("SleepDisabled")

    /*!
@function IOPMCopySystemPowerSettings
@abstract Returns System power settings. 
          System-wide power settings are not power source dependent.
     */
CFDictionaryRef IOPMCopySystemPowerSettings( void );

    /*
@function IOPMSetSystemPowerSetting
@abstract Set a system-wide power management setting
@param key Setting name
@param value Setting value
@result kIOReturnSuccess; or IOReturn error otherwise
     */
IOReturn IOPMSetSystemPowerSetting( CFStringRef key, CFTypeRef value );

    /*!
@function IOPMActivateSystemPowerSettings
@abstract This is not supported.
     */
IOReturn IOPMActivateSystemPowerSettings( void );

/**************************************************
*
* Cancel All Scheduled Power Events
*
**************************************************/

// Indexes for the actions to handle under _io_pm_schedule_power_event.
#define     kIOPMCancelScheduledEvent        0
#define     kIOPMScheduleEvent               1
#define     kIOPMCancelAllScheduledEvents    2

/*  @function IOPMCancellAllScheduledPowerEvent
 *  @abstract Cancels all scheduled power events
 *  @result kIOReturnSuccess on success, kIOReturnError or kIOReturnNotPrivileged otherwise.
 */ 
IOReturn IOPMCancelAllScheduledPowerEvents(void);

/**************************************************
*
* Power API - For userspace power & thermal drivers
*
**************************************************/

/* @function IOPMSystemPowerEventOccurred
    @abstract Called to alert the system of a power or thermal event
    @param eventType A CFStringRef defining the type of event
    @param eventObject The CF payload describing the event
    @result kIOReturnSuccess; kIOReturnNotPrivileged, kIOReturnInternalError, or kIOReturnError on error.
*/
IOReturn IOPMSystemPowerEventOccurred(
        CFStringRef typeString, 
        CFTypeRef eventValue);

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************
*
* Sleep/Wake Bookkeeping API
*
* Returns timing details about system level power events.
*
******************************************************************************/
/*!
 * @functiongroup IOPM UUID Bookkeeping
 */

/*! 
 *  @function   IOPMGetLastWakeTime 
*
 *  @abstract   returns the timestamp of the last system wake.
 *
 *  @discussion If possible, the wakeup time is adjusted backward using hardware
 *              timers, so that whenWake refers to the time of the physical event
 *              that woke the system.
 *
 *  @param      whenWoke On successful return, whenWoke will contain the CFAbsoluteTime,
 *              in the CF time scale, that the system awoke. This time is already adjusted
 *              with the SMC wake delta, when available.
 *
 *  @param      adjustedForPhysicalWake Returns the interval (in CF time units of seconds) that
 *              PM adjusted the wakeup time by. If the system doesn't support the HW timers
 *              required to adjust the time, this value will be zero.
 *
 *  @result     Returns kIOReturnSuccess; all others returns indicate a failure; the returned
 *              time values should not be used upon a failure return value.
 */
IOReturn IOPMGetLastWakeTime(
        CFAbsoluteTime      *whenWoke,
        CFTimeInterval      *adjustedForPhysicalWake);

/*! IOPMSleepWakeSetUUID
 *  Sets the upcoming sleep/wake UUID. The kernel will cache
 *  this and activate it the next time the system goes to sleep.
 *  Pass a CFStringRef to set the upcoming UUID.
 *  Pass NULL to indicate that the current UUID should be cancelled.
 *  Only should be called by PM configd & loginwindow.
 */
IOReturn IOPMSleepWakeSetUUID(CFStringRef newUUID);
 
 
/*!
 * @group IOPMGetUUID types
 *
 * @constant kIOPMSleepWakeUUID
 *              Argument to <code>@link IOPMGetUUID @/link</code>
 *
 * @constant kIOPMSleepServicesUUID
 *              Argument to <code>@link IOPMGetUUID @/link</code>
 *
 */
enum {
    kIOPMSleepWakeUUID      = 1000,
    kIOPMSleepServicesUUID  = 1001
};

/*! 
 *  IOPMGetUUID
 *  
 *  Returns a C string that uniquely describes the "sleep interval"
 *  that the system is in right now.
 *
 *  @param whichUUID      
 *  The argument may be <code>kIOPMSleepWakeUUID</code>, to get the sleep/wake UUID,
 *  or <code>kIOPMSleepServicesUUID</code> to get the sleep/wake UUID.
 *
 *  OS X creates a new SleepWakeUUID when the system goes to sleep. OS X clears
 *  it several minutes after the system wakes from sleep. 
 *  Every time OS X clears the SleepWakeUUID, it creates a new UUID. So there
 *  are always valid SleepWake UUID's defined, even when the machine is awake.
 *  Calls to <code>IOPMGetUUID()</code> for type <code>kIOPMSleepWakeUUID</code> 
 *  should in all non-error cases return true, and return a valid UUID string 
 *  in <code>putTheUUIDHere</code>. 
 *
 *  OS X creates a new SleepServiceUUID at the moment that process 
 *  <code>SleepServiceD</code> decides that a given wakeup shall be bound by 
 *  SleepService cap timers.
 *  OS X clears the SleepServiceUUID when the system exits SleepServices.
 *  At many points during a system's runtime, calling <code>IOPMGetUUID</code>
 *  for <code>kIOPMSleepServicesUUID</code> may result in a return value
 *  of false, and no string being copied into the passed-in buffer.
 *
 *  @param putTheUUIDHere
 *  Caller provides a pointer to a buffer of sufficient size to receive 
 *  a C string UUID. I recommend 100 bytes, because that's about 3x what
 *  you should need. 
 *
 *  @param sizeOfBuffer
 *  Caller must specify the size of the buffer they pass in as argument
 *  <code>putTheUUIDHere</code>
 *
 *  @result
 *  Returns true if successfully copied the UUID into the provided buffer.
 *  Returns false if there was an error, or if there was no buffer.
 *
 *  @discussion
 *  For example:
 *          char my_buffer[100];
 *
 *          if(IOPMGetUUID(kIOPMSleepServicesUUID, my_buffer, sizeof(my_buffer)))
 *          {
 *              printf("my UUID is = %s", my_buffer);
 *           } else {
 *              printf("There is no such UUID defined right now.");
 *          }
 *  
 */

bool IOPMGetUUID(int whichUUID, char *putTheUUIDHere, int sizeOfBuffer);
 
/*! IOPMSleepWakeCopyUUID
 *  Returns the active sleep wake UUID, if it exists.
 *  The returned UUID is only useful for logging events that occurred during
 *  the given sleep/wake.
 *  Returns a CFString containing the UUID. May return NULL if there is no UUID.
 *  Unless the result is NULL, caller must release the result.
 */
CFStringRef IOPMSleepWakeCopyUUID(void);

/*! @group DarkWake */

/*
 * IOPMSetSleepServicesWakeTimeCap
 *
 * Only SleepServicesD should call this SPI. If you are not writing code for
 * SleepServicesD, do not call this SPI.
 *
 * This SPI allows changing the Sleep services wake duration time cap. 
 * On receiving this request, powerd ignores the existing time cap and
 * sets the time cap to specified duration starting from point when
 * the request is received.
 *
 * The initial sleep services time cap still need to be sent to powerd 
 * thru the IOPMConnectionAcknowledgeEventWithOptions() API. This SPI
 * IOPMSetSleepServicesWakeTimeCap() should be used only to change the
 * existing time cap.
 *
 * The request is ignored if issued when there is no existing time cap
 * already set or if issued when system is not in sleep services wake.
 *
 * @param cap
 * This specifies the new cap time in milliseconds.
 *
 *  @result
 *  Returns kIOReturnSuccess if new time cap is set successfully.
 */
IOReturn IOPMSetSleepServicesWakeTimeCap(CFTimeInterval cap);
// Poweranagement's 1-byte status code
#define kIOPMSleepWakeFailureKey            "PMFailurePhase"

// PCI Device failure name
// Will indicate which driver-subtree it occured in 
// If the failure took place during a driver sleep/wake phase,
#define kIOPMSleepWakeFailureDriverTreeKey  "PCIDeviceFailure"

// LoginWindow's 1-byte status code
// Will indicate LoginWindow's role in putting up the security panel 
//  if the failure took place during sleep phase:
//  kIOPMTracePointSystemLoginwindowPhase = 0x30
#define kIOPMSleepWakeFailureLoginKey       "LWFailurePhase"

// The UUID of the sleep that failed
#define kIOPMSleepWakeFailureUUIDKey        "UUID"

// The date of the attempted sleep that resulted in failure.
#define kIOPMSleepWakeFailureDateKey        "Date"

// PowerManagement's 8-byte failure descriptor.
// Do not reference this key, instead references the decoded data
// stored under other keys in this dictionary.
#define kIOPMSleepWakeFailureCodeKey        "PMStatusCode"

#define kIOPMSleepWakeWdogRebootKey         "SWWdogTriggeredRestart"
#define kIOPMSleepWakeWdogLogsValidKey      "SWWdogLogsValid"

/*
 * Allow certain components to log their wake progress with IOPMrootDomain.
 *
 * component - Pass component wake progress value (defined in IOPMPrivate.h)
 * progress_data - Debug data to be stored
 */
IOReturn IOPMLogWakeProgress(uint32_t component, uint32_t progress_data);

/*
 * Returns data describing the sleep failure (if any) that occured prior to the system booting.
 *
 * This routine will return the same CFDictionary over the lifetime of a given boot - it does not return
 * dynamic information after each sleep/wake. It only returns information pertaining to the last
 * failed sleep/wake before booting up.
 * 
 * If NULL, then the last sleep/wake was successful, or we were unable to determine whether
 * there was a problem.
 * If non-NULL, Caller must release the returned dictionary.
 *
 * kIOPMSleepWakeFailureLoginKey points to a CFNumber containing LW's 8-bit code.
 * kIOPMSleepWakeFailureUUIDKey points to a CFStringRef containing the UUID associated with the failed sleep.
 * kIOPMSleepWakeFailureDateKey points to the CFDate that the failed sleep was initiated.
 */
CFDictionaryRef IOPMCopySleepWakeFailure(void);


/**************************************************
*
* SW-generated HID events history
*
* OS PM records software generated HID events by process, for
* later associating HID events that are preventing display sleep
* and system sleep with the processes that created them.
*
**************************************************/

typedef struct {
    CFAbsoluteTime  eventWindowStart;
    uint32_t        hidEventCount;  // MOUSE, KEYBOARD, or other NON-NULL event
    uint32_t        nullEventCount; // NX_NULL_EVENT
} IOPMHIDPostEventActivityWindow;

/*!
 * Key into process HID activity dictionary. 
 * The value is the path to the caller's
 * executable path.
 */
#define kIOPMHIDAppPathKey        CFSTR("AppPathString")

/*!
 * Key into process HID activity dictionary.
 * The value is a CFNumber integer - the caller's pid.
 */
#define kIOPMHIDAppPIDKey       CFSTR("AppPID")

/*!
 * Key into process HID activity dictionary.
 * The value is a CFArray of recent HID activity. The CFArray contains
 * CFData blocks, each with size = sizeof(IOHIDPostEventActivityWindow)
 * Each block summarizing a 5 minute window of HID activity.
 */
#define kIOPMHIDHistoryArrayKey   CFSTR("HIDHistoryArray")

/*!
 * IOPMCopyHIDPostEventHistory
 * The returned CFArray contains one dictionary per HID-posting process.
 *      CFDictionary contains
 *          key = AppPID
 *          CFNumberRef for pid
 *          key = AppPathString
 *          CFStringRef path to pid's executable (program name)
 *          key = HIDHistoryArray
 *          CFArray of CFData
 *              Each CFData represents a "bucket" of IOHPMIDPostEventActivityWindow, 
 *                  representing a 5 minute window of HID activity.
 * @param outDictionary On success, returns a valid CFDictionaryRef. The caller must release this array.
 *      Will return NULL otherwise.
 * @result kIOReturnSuccess on success; kIOReturnNotFound if HID event logging is disabled;
 *      kIOReturnError for all others errors.
 */
IOReturn IOPMCopyHIDPostEventHistory(CFArrayRef *outDictionary);

/**************************************************
*
* IOPM Power Details log
*
* Logs fine-grained details about per-driver sleep/wake activity
* into a in-memory buffer. PowerManagement translates that buffer
* into a CFType-based representation of all driver power activity
* during sleep/wake.
*
* These calls may take upwards of 1 second to execute. Do not call
* any of these "PowerDetails" calls from a UI thread.
*
**************************************************/
#define kIOPMPowerHistoryEventTypeKey                   "EventType"
#define kIOPMPowerHistoryEventReasonKey                 "EventReason"
#define kIOPMPowerHistoryEventResultKey                 "EventResult"
#define kIOPMPowerHistoryDeviceNameKey                  "DeviceName"
#define kIOPMPowerHistoryUUIDKey                        "UUID"
#define kIOPMPowerHistoryInterestedDeviceNameKey        "InterestedDeviceName"
#define kIOPMPowerHistoryTimestampKey                   "Timestamp"
#define kIOPMPowerHistoryDeviceUniqueIDKey              "DevinceUnique"
#define kIOPMPowerHistoryElapsedTimeUSKey               "ElapsedTimeUS"
#define kIOPMPowerHistoryOldStateKey                    "OldPowerState"
#define kIOPMPowerHistoryNewStateKey                    "NewPowerState"
#define kIOPMPowerHistoryTimestampCompletedKey          "ClearTime"
#define kIOPMPowerHistoryEventArrayKey                  "EventArray"

/*
 *  The timestamps that IOPMCopyPowerHistory returns are CFStrings that need
 *  to be interpreted in this Unicode date format pattern. They can be 
 *  converted to CFAbsoluteTime or otherwise, using this format string
 */
#define kIOPMPowerHistoryTimestampFormat             "MM,dd,yy HH:mm:ss zzz"

/*
 * Returns a list of major power events (sleep, wake) in chronological order.
 * On failure, IOReturn will reflect the type of failure.
 * On success, caller must release returned array.
 *
 * Input: A CFArrayRef into which the series of power events is copied into
 * 
 * Output: A filled CFArrayRef with details of all major power events. Caller
 *         is responsible for releasing the filled array at some later point
 *
 * Returns kIOReturnSuccess on success, error code of type IOReturn otherwise
 *
 * NOTE: The CFArray returned consists of individual CFDictionaries. Each
 * CFDictionary within the array has the following structure
 *
 * CFDictionary {
 *   {
 *     key: kIOPMPowerHistoryUUIDKey 
 *     value: UUID of power event cluster (CFString)
 *   }
 *   {
 *     key: kIOPMPowerHistoryTimestampKey
 *     value: Time marking the beginning of this power event cluster (CFString)
 *   }
 *   {
 *     key: kIOPMPowerHistoryTimestampCompletedKey
 *     value: Timae marking the end of this power event cluster (CFString)
 *   }
 * }
 *
 * The timestamps returned as CFStrings can be interpreted/parsed using 
 * kIOPMPowerHistoryTimestampFormat as defined above
 */
IOReturn IOPMCopyPowerHistory(CFArrayRef *outArray);

/*
 * When available, returns details of driver activity that occurred 
 * during a larger power event.
 * 
 * Input: A UUID representing a power event. 
 *        Obtained from IOPMCopyPowerHistory.
 * 
 * Output: A CFDictionaryRef of detailed power events that occurred during 
 *         that UUID. Possibly NULL. 
 *         On success, caller should release the returned array.
 * 
 * Returns: kIOReturnSuccess on success; error code otherwise.
 *
 * NOTE: The CFDictionary returned by this function has the following
 *        structure:
 *  
 * CFDictionary {
 *   {
 *     key: kIOPMPowerHistoryUUIDKey
 *     value: UUID identifier for this cluster of power events (CFString)
 *   }
 *   {
 *     key: kIOPMPowerHistoryTimestampKey
 *     value: Time marking the beginning of the power event cluster 
 *            Note that timestamps are returned as CFAbsoluteTime in this
 *            method (unlike in IOPMCopyPowerHistory) to avoid loss of 
 *            precision (CFAbsoluteTime).
 *   }
 *   {
 *     key: kIOPMPowerHistoryTimestampCompletedKey
 *     value: Time marking the end of the power event cluster
 *            Note that timestamps are returned as CFAbsoluteTime in this
 *            method (unlike in IOPMCopyPowerHistory) to avoid loss of 
 *            precision (CFAbsoluteTime).
 *   }
 *   {
 *     key: kIOPMPowerHistoryEventArrayKey
 *     value: An array containing details of individual power events within
 *            the larger cluster (CFArray). The details of each fine-grained
 *            power event is placed in the form of a CFDictionary inside 
 *            this array. The structure of this CFDictionary is explained 
 *            below
 *   }
 * }
 *
 * 
 * Each CFDictionary (which describes a fine-grained power state change) 
 * within the Power events array is structured as:
 *
 * CFDictionary {
 *   {
 *     key: kIOPMPowerHistoryTimestampKey
 *     value: Time at which the given state change took place (CFAbsoluteTime)
 *   }
 *   {
 *     key: kIOPMPowerHistoryEventReasonKey
 *     value: Reason for the state change, if any (CFNumber)
 *   }
 *   {
 *     key: kIOPMPowerHistoryEventResultKey
 *     value: Result of the given state change, if any
 *   }
 *   {
 *     key: kIOPMPowerHistoryDeviceNameKey
 *     value: Name of the device that's changing state (CFString)
 *   }
 *   {
 *     key: kIOPMPowerHistoryUUIDKey
 *     value: UUID of the larger power event cluster that this state change
 *            belongs to (CFString)
 *   }
 *   {
 *     key: kIOPMPowerHistoryInterestedDeviceNameKey
 *     value: Name of any other device that's interested in watching this 
 *            power state change (CFStringRef)
 *   }
 *   {
 *     key: kIOPMPowerHistoryOldStateKey
 *     value: The old power state from which the device is changing out of
 *            (CFNumber)
 *   }
 *   {
 *     key: kIOPMPowerHistoryNewStateKey
 *     value: The new power state the device is changing into (CFNumber)
 *   }
 *   {
 *     key: kIOPMPowerHistoryTimeElapsedUSKey
 *     value: The total time taken for the power state transition to
 *             complete, measured in microseconds (CFNumber)  
 *   }
 * }
 */
IOReturn IOPMCopyPowerHistoryDetailed(CFStringRef UUID, CFDictionaryRef *outDict);


/**************************************************
*
* IOPMConnection API
*
* IOPMConnection API lets user processes receive
* power management notifications.
*
**************************************************/
/*!
 * @functiongroup IOPMConnection
 */
 
 
/*! 
 * IOPMConnection is the basic connection type between a user process
 * and system power management.
 * IOPMConnections are backed by mach ports, and are automatically cleaned up
 * on process death.
 */
typedef const struct __IOPMConnection * IOPMConnection;

/*! 
 * A unique IOPMConnectionMessageToken accompanies each PM message received from
 * an IOPMConnection notification. This message token should be passed as an argument
 * to IOPMConnectionAcknowledgeEvent().
 */
typedef uint32_t IOPMConnectionMessageToken;


/*****************************************************************************/
/*****************************************************************************/

/*! IOPMClaimSystemWakeEvent
 *
 * Records information about system wake to a persistent log; recoverable with "pmset -g log".
 * The caller must have evidence that activity in the calling process is responsible, or partly
 * responsible for the physical hardware event that woke the system.
 *
 * Correct usage of this SPI:
 *    - apsd detects that the system woke, and that there are incoming push notifications that
 *    have the capability to awaken the system waiting for it. Also, this system must support
 *    TCPKeepAlive, and have been woken by a Network event.
 *    - A USB user space driver detects that the USB device it's responsible for generate the
 *    PME.
 *
 * Incorrect usage of this SPI:
 *    - An app that reconnects to a remote server in response to wakeup.
 *    - An app that resumes video or audio in response to a wakeup.
 *
 * Caller must have kIOPMDarkWakeControlEntitlement - com.apple.private.iokit.darkwake-control.
 *
 * @param identity  Identity of the caller, as in "com.apple.apsd"
 * @param reason    A string describing the unique identity or reason for the wake.
 * @param description [Optional] further information about the wake event.
 */
void IOPMClaimSystemWakeEvent(
        CFStringRef         identity,
        CFStringRef         reason,
        CFDictionaryRef     description);

/*****************************************************************************/

/*
 *! @function   IOPMSetActivePushConnectionState
 *  @abstract   Specify if Active push connections exists on the system.
 *
 *  @param  exists      Set to 'true' when there are active push connections on the system.
 *
 *  @result             Return kIOReturnSuccess on success.
 */
IOReturn IOPMSetActivePushConnectionState(bool exists);

/*
 *! @function   IOPMGetActivePushConnectionState
 *  @abstract   Returns the current state of active push connections
 *
 *  @param  exists      Returns 'true' if there are active push connections on the system.
 *
 *  @result             Return kIOReturnSuccess. All other return values indicate a failure
 *                      in getting the current state; value of 'exists' parameter is not
 *                      valid in that case.
 */
IOReturn IOPMGetActivePushConnectionState(bool *exists);

/*****************************************************************************/

/*! @group IOPMCapabilityBits
 *
 *  Bits define capabilities in IOPMCapabilityBits type.
 *
 *  These bits describe the capabilities of a system power state.
 *  Each bit describes whether a capability is supported; it does not
 *  guarantee that the described feature is accessible. Even if a feature
 *  is accessible, it may be unavailable. Caller should be prepared for an error
 *  when accessing.
 *
 *  Please use these bits to:
 *      - Specify the capabilities of insterest for IOPMConnectionCreate()
 *      - Interpret the power states provided to IOPMEventHandlerType notification.
 */

/*! @constant kIOPMSystemPowerStateCapabilityCPU
 *  If set, indicates that in this power state the CPU is running. If this bit is clear,
 *  then the system is going into a low power state such as sleep or hibernation.
 *
 *  System capability bits (video, audio, network, disk) will only be available
 *  if CPU bit is also available.
 */
#define kIOPMCapabilityCPU                          0x1

/*! @constant kIOPMCapabilityVideo
 *  If set, indicates that in this power state, graphic output to displays are supported.
 */
#define kIOPMCapabilityVideo                        0x2

/*! @constant kIOPMCapabilityAudio
 *  If set, indicates that in this power state, audio output is supported.
 */
#define kIOPMCapabilityAudio                        0x4

/*! @constant kIOPMCapabilityNetwork
 *  If set, indicates that in this power state, network connections are supported.
 */
#define kIOPMCapabilityNetwork                      0x8

/*! @constant kIOPMCapabilityDisk
 *  If set, indicates that in this power state, internal disk and storage device access is supported.
 */
#define kIOPMCapabilityDisk                         0x10

/*! @constant kIOPMCapabilityPushServiceTask
 *  If set, the system may perform PushServiceTasks while in this power state, and
 *  the system will honor PushServiceTask power assertions.
 *  If clear, the system will not honor PushServiceTask power assertions.
 *  Clients should initiate PushServiceTasks or create PushServiceTask assertions if this bit is set.
 */
#define kIOPMCapabilityPushServiceTask              0x20

/*! @constant kIOPMCapabilityBackgroundTask
 *  If set, the system may perform BackgroundTasks while in this power state, and
 *  the system will honor BackgroundTask power assertions.
 *  If clear, the system will not honor BackgroundTask power assertions. If set,
 *  Clients should initiate BackgroundTasks or create BackgroundTask assertions if this bit is set.
 */
#define kIOPMCapabilityBackgroundTask               0x40

/*!
 * @constant kIOPMCapabilitySilentRunning
 * If set, indicates that the system is able to operate in an acoustically favorable environment
 * while performing background tasks
 * If clear, the system is no longer able to maintain this acoustically favorable environment,
 * and maybe prone to making noise (fans/hard drive clicks) while operating. Clients are advised
 * to hold off/end background task work, or risk user annoyance because of machine noise.
 *
 * WARNING: This bit is not for you. Clients should use the PushServiceTask or BackgroundTask bit
 *          to decide when to perform/hold off on intensive background work. This bit is only
 *          provided for the benefit of specific system daemons.
 */
#define kIOPMCapabilitySilentRunning                0x80

/*! @constant kIOPMCapabilityBitsMask
 *  Should be used as a mask to check for states; this value should not be
 *  passed as an an argument to IOPMConnectionCreate.
 *  Passing this as an interest argument will produce undefined behavior.
 *
 *  Note: Do not use constant kIOPMCapabilityMask. That matches a different API,
 *  and should not be used here.
 *  Any PowerStateCapability bits that are not included in this mask are
 *  reserved for future use.
 */
#define kIOPMCapabilityBitsMask                     0xFF

/*! @group IOPMCapability SPI
 */

/*! IOPMCapabilityBits
 *  Should be a bitfield with a subset of the IOPMCapabilityBits.
 */
typedef uint32_t IOPMCapabilityBits;

/*! @constant kIOPMEarlyNotification
 *  If set, indicates that this wake notification is an early notification indicating
 *  that system will wake up with the specified capabilities.
 *  
 *  This notification is issued only if the client has set this bit in 'interests' while
 *  calling IOPMConnectionCreate(). This early notification is issued only as an 
 *  advisory to the clients and the wake process can't be delayed by the client 
 *  receiving this notification.
 */

#define kIOPMEarlyWakeNotification                  0x8000



/*! IOPMIsADarkWake
 *  Returns true if the SystemPowerStateCapabilities represent a DarkWake.
 *  Returns false if the system is asleep, or in full wake, or in any other state.
 */
bool IOPMIsADarkWake(IOPMCapabilityBits);

/*! IOPMIsABackgroundTask
 *  Returns true if the SystemPowerStateCapabilities represent a state where
 *  the system encourages BackgroundTasks to execute.
 */
bool IOPMAllowsBackgroundTask(IOPMCapabilityBits);

/*! IOPMIsAPushServiceTask
 *  Returns true if the SystemPowerStateCapabilities represent a state where
 *  the system encourages PushServiceTasks to execute.
 */
bool IOPMAllowsPushServiceTask(IOPMCapabilityBits);

/*! IOPMIsASilentWake
 * Returns true if the SystemPowerStateCapabilities represent a silent wake.
 * A silent wake is one in which the system is able to sustain an
 * acoustically favorable environment while background tasks execute.
 * Returns false if the machine is no longer able to support this acoustic
 * environment and is likely to be noisy under load.
 *
 * WARNING: This SPI is not for you. Clients should use IOPMAllowsBackgroundTask
 *          and IOPMAllowsPushServiceTask to determine when to perform/not perform
 *          background work. This SPI is provided for the benefit of specific
 *          system daemons
 *          Additionally, the parameters of this SPI are being fine-tuned
 *          and are very likely to change. Use at your own risk/peril/doom.
 */
bool IOPMIsASilentWake(IOPMCapabilityBits);

/*! IOPMIsAUserWake
 *  Returns true if the SystemPowerStateCapabilities represent a full user wake.
 *  This implies that video and audio capability bits are available, though their
 *  hardware is not necessarily powered.
 */
bool IOPMIsAUserWake(IOPMCapabilityBits);

/*! IOPMIsAsleep
 *  Returns true if the capabilities field represents a sleeping state.
 */
bool IOPMIsASleep(IOPMCapabilityBits);


/*! IOPMGetCapabilitiesDescription
 *  Fills the provided buffer with a C string describing the system capabilities.
 *  Example: "FullWake: cpu disk net aud vid" to describe bits for a Full Wake.
 *  Example: "DarkWake: cpu disk net" or "DarkWake: cpu disk net push bg" to describe a DarkWake.
 *  @param  buf A string buffer to be popuplated by this routine. Recommend 50 bytes or larger.
 *  @param buf_size The size of the buffer. IOPMGetCapabilitiesDescription will not write beyond this size.
 *  @param in_caps The capabilities to describe.
 *  @result Returns true on success. Returns false if the provided buffer was too small.
 */
bool IOPMGetCapabilitiesDescription(char *buf, int buf_size, IOPMCapabilityBits in_caps);


/*! @deprecated
 *  These constants are equivalent to the shorter kIOPMCapability constants defined above.
 *  We provide these longer constant strings for source code compatibility.
 *  Please use the kIOPMCapability version of these constants.
 */
typedef uint32_t IOPMSystemPowerStateCapabilities;
#define kIOPMSystemPowerStateCapabilityCPU              kIOPMCapabilityCPU
#define kIOPMSystemPowerStateCapabilityVideo            kIOPMCapabilityVideo
#define kIOPMSystemPowerStateCapabilityAudio            kIOPMCapabilityAudio
#define kIOPMSystemPowerStateCapabilityNetwork          kIOPMCapabilityNetwork
#define kIOPMSystemPowerStateCapabilityDisk             kIOPMCapabilityDisk
#define kIOPMSystemPowerStateCapabilitiesMask           kIOPMCapabilityBitsMask
#define kIOPMSytemPowerStateCapabilitiesMask            kIOPMSystemPowerStateCapabilitiesMask

/*! IOPMEventHandlerType
 *  IOPMEventHandlerType is the generic function type to handle a
 *   notification generated from the power management system. All clients of
 *   IOPMConnection that wish to listen for notifications must provide a handler
 *   when they call IOPMConnectionCreate.
 *  @param param Pointer to user-chosen data.
 *  @param connection The IOPMConnection associated with this notification.
 *  @param token Uniquely identifies this message invocation; should be passed
 *          to IOPMConnectAcknowledgeEvent().
 *  @param eventDescriptor Provides a bitfield describing the new system power state.
 *          See IOPMCapabilityBits below. Please evaluate this value as a bitfield,
 *          not as a constant. Either:
 *               1. Test bits indidivually using bitwise &, 
 *               2. or use the convenient IsADarkWake(), IsASleep(), etc. functions to digest the bitfield.
 */
typedef void (*IOPMEventHandlerType)(
                void *param, 
                IOPMConnection connection, 
                IOPMConnectionMessageToken token, 
                IOPMCapabilityBits eventDescriptor);


/*****************************************************************************/
/*****************************************************************************/

/*! kIOPMAckNetworkMaintenanceWakeDate
 *
 *  This string can be used as a key in the 'options' dictionary
 *  argument to IOPMConnectionAcknowledgeEventWithOptions. Caller
 *  should push a CFDate value to match this key.
 *
 *  System network daemons should populate this option to specify the
 *  next wakeup date.
 *  Passing this "Date" option lets a caller request a maintenance wake
 *  from the system at a later date. If possible, the system will wake
 *  to the requested power state at the requested time.
 *
 *  This acknowledgement option is only valid when the system is transitioning into a
 *  sleep state i.e. entering a state with 0 capabilities. 
 *
 *  This acknowledgement argument may only be successfully used with
 *  the requirements bitfield 
 *      kIOPMCapabilityDisk | kIOPMCapabilityNetwork
 * 
 */
#define kIOPMAckNetworkMaintenanceWakeDate    CFSTR("NetworkMaintenanceWakeDate")

/*! kIOPMAckSUWakeDate
 *
 *  This string can be used as a key in the 'options' dictionary
 *  argument to IOPMConnectionAcknowledgeEventWithOptions. Caller
 *  should push a CFDate value to match this key.
 *
 *  Task scheduling apps should use this key when scheduling Power Nap Software Updates.
 *
 *  This acknowledgement option is only valid when the system is transitioning into a
 *  sleep state i.e. entering a state with 0 capabilities.
 *
 *  This acknowledgement argument may only be successfully used with
 *  the requirements bitfield
 *      kIOPMCapabilityDisk | kIOPMCapabilityNetwork
 *
 */
#define kIOPMAckSUWakeDate    CFSTR("SoftwareUpdate")

/*! kIOPMAckBackgroundTaskWakeDate
 *
 *  This string can be used as a key in the 'options' dictionary
 *  argument to IOPMConnectionAcknowledgeEventWithOptions.
 *  - Caller should set a CFDate value to match this key.
 *  - Caller should set the kIOPMAcknowledgmentOptionSystemCapabilityRequirements key too.
 *
 *  CTS should populate this option to specify the next wakeup
 *  date.
 *
 *  Passing this "Date" option lets a caller request a maintenance wake
 *  from the system at a later date. If possible, the system will wake
 *  to the requested power state at the requested time.
 *
 *  This acknowledgement option is only valid when the system is transitioning into a
 *  sleep state i.e. entering a state with 0 capabilities.
 *
 *  This acknowledgement argument may only be successfully used with
 *  the requirements bitfield
 *      kIOPMCapabilityDisk | kIOPMCapabilityNetwork
 *
 */
#define kIOPMAckBackgroundTaskWakeDate    CFSTR("BackgroundTask")
#define kIOPMAckTimerPluginWakeDate     kIOPMAckBackgroundTaskWakeDate

/*! kIOPMAcknowledgmentOptionDate
 *
 *  Callers should prefer to use a more specific key instead of this key:
 *      kIOPMAckNetworkMaintenanceWakeDate
 *      kIOPMAckSleepTimerWakeDate
 *
 *  This string can be used as a key in the 'options' dictionary
 *  argument to IOPMConnectionAcknowledgeEventWithOptions.
 *
 *  Passing this "Date" option lets a caller request a maintenance wake
 *  from the system at a later date. If possible, the system will wake
 *  to the requested power state at the requested time.
 *
 *  This acknowledgement option is only valid when the system is transitioning into a
 *  sleep state i.e. entering a state with 0 capabilities. 
 *
 *  This acknowledgement argument may only be successfully used with
 *  the requirements bitfield 
 *      kIOPMCapabilityDisk | kIOPMCapabilityNetwork
 *
 */
#define kIOPMAcknowledgmentOptionWakeDate           CFSTR("WakeDate")
#define kIOPMAckWakeDate                            kIOPMAcknowledgmentOptionWakeDate

/*! kIOPMAcknowledgmentOptionSystemCapabilityRequirements
 *
 *  This string can be used as a key in the 'options' dictionary
 *  argument to IOPMConnectionAcknowledgeEventWithOptions.
 *
 *  This string should only be specified if the key
 *  kIOPMAcknowledgementOptionDate is also specified.
 *
 *  In the 'options' acknowledgement dictionary, specify a CFNumber that you
 *  contains a bitfield describing the capabilities your process requires at the next
 *  maintenance wake.
 *
 *  This acknowledgement option is only valid when the system is transitioning into a
 *  sleep state i.e. entering a state with 0 capabilities. 
 *
 *  This acknowledgement argument may only be successfully used with
 *  the requirements bitfield 
 *      kIOPMCapabilityDisk | kIOPMCapabilityNetwork
 * 
 */
#define kIOPMAcknowledgmentOptionSystemCapabilityRequirements   CFSTR("Requirements")
#define kIOPMAckSystemCapabilityRequirements                    kIOPMAcknowledgmentOptionSystemCapabilityRequirements

/*!
 * @constant kIOPMAckAppRefreshWakeDate
 *
 *  This string can be used as a key in the 'options' dictionary
 *  argument to IOPMConnectionAcknowledgeEventWithOptions.
 *
 * Only sleepservicesd should pass this argument. Behavior if passed from any other process
 * is undefined.
 *
 *  Passing this "Date" option lets a caller request a SleepServices wake
 *  from the system at a later date. If possible, the system will wake
 *  and perform SleepServices activity at the specified time.
 *
 *  This acknowledgement option is only valid when the system is transitioning into a
 *  sleep state i.e. entering a state with 0 capabilities.
 *
 */
#define kIOPMAckAppRefreshWakeDate                            CFSTR("AppRefresh")

// Deprecated:
#define kIOPMAcknowledgementOptionSleepServiceDate              kIOPMAckAppRefreshWakeDate
#define kIOPMAckSleepServiceDate                                kIOPMAcknowledgementOptionSleepServiceDate

/*!
 * @constant kIOPMAckSleepServiceCapTimeout
 *
 *  This string can be used as a key in the 'options' dictionary
 *  argument to IOPMConnectionAcknowledgeEventWithOptions.
 *
 * Please pass a CFNumber kCFNumberIntType, with a value in milliseconds. 
 * e.g. Pass 120000 to specify a 2 minute cap.
 *
 * Only sleepservicesd should pass this argument. Behavior if passed from any other process
 * is undefined.
 *
 *  Passing this "cap" option lets a caller specify how long a SleepServices wakeup
 *  may last. PM will enforce that the system returns to sleep after this many milliseconds have elapsed.
 *
 *  This acknowledgement option is only valid when the system is transitioning into a
 *  sleep state i.e. entering a state with 0 capabilities. 
 *
 */
#define kIOPMAcknowledgeOptionSleepServiceCapTimeout            CFSTR("SleepServiceTimeout")
#define kIOPMAckSleepServiceCapTimeout                          kIOPMAcknowledgeOptionSleepServiceCapTimeout

/*!
 * @constant kIOPMAckDHCPRenewWakeDate
 *
 *  This string can be used as a key in the 'options' dictionary
 *  argument to IOPMConnectionAcknowledgeEventWithOptions.
 *
 * Only mDNSResponder should pass this argument. Behavior if passed from any other process
 * is undefined.
 *
 *  Passing this "Date" option lets a caller request a maintenance wake
 *  from the system at a later date. If possible, the system will wake
 *  and perform maintenance activity at the specified time.
 *
 *  This acknowledgement option is only valid when the system is transitioning into a
 *  sleep state i.e. entering a state with 0 capabilities.
 *
 */
#define kIOPMAckDHCPRenewWakeDate                               CFSTR("DHCPRenew")

/*
 * @constant kIOPMAckClientInfo
 *
 * Optional key for IOPMConnectionAcknoweledgeEventWithOptions() dictionary.
 *
 * The caller may set this key to a CFString with a reason or cause for this 
 * DarkWake request.
 *
 * The string should be a reason, process name, pid, or strings that's useful 
 * for debugging. The value will be logged into "pmset -g log" along with
 * the name of the calling process.
 */
#define kIOPMAckClientInfoKey                                   CFSTR("ClientInfo")

/*
 * @constant kIOPMAckClientInfoBGTask
 *
 * Optional key for IOPMConnectionAcknoweledgeEventWithOptions() dictionary.
 *
 * The caller may set this key to a CFString with a reason or cause for this 
 * DarkWake when kIOPMAckBackgroundTaskWakeDate is set in the dictionary.
 *
 * The string should be a reason, process name, pid, or strings that's useful 
 * for debugging. The value will be logged into "pmset -g log" along with
 * the name of the calling process.
 */
#define kIOPMAckClientInfoBGTaskKey                             CFSTR("ClientInfoBGTask")

/*
 * @constant kIOPMAckClientInfoAppRefresh
 *
 * Optional key for IOPMConnectionAcknoweledgeEventWithOptions() dictionary.
 *
 * The caller may set this key to a CFString with a reason or cause for this 
 * DarkWake when kIOPMAckClientInfoAppRefresh is set in the dictionary.
 *
 * The string should be a reason, process name, pid, or strings that's useful 
 * for debugging. The value will be logged into "pmset -g log" along with
 * the name of the calling process.
 */
#define kIOPMAckClientInfoAppRefreshKey                         CFSTR("ClientInfoAppRefresh")

/*!
 * @constant kIOPMAckLimitedPowerWakeDate
 *
 *  This string can be used as a key in the 'options' dictionary
 *  argument to IOPMConnectionAcknowledgeEventWithOptions.
 *
 *  Only a client with a kIOPMLimitedPowerWakeRequestEntitlement is
 *  allowed to use this key.
 *
 *  Passing this "Date" option lets a caller request a maintenance wake
 *  from the system at a later date. If possible, the system will wake
 *  and perform maintenance activity at the specified time.
 *
 *  This acknowledgement option is only valid when the system is transitioning into a
 *  sleep state i.e. entering a state with 0 capabilities.
 *
 */
#define kIOPMAckLimitedPowerWakeDate                            CFSTR("LimitedPowerWakeDate")


/*****************************************************************************/
/*****************************************************************************/


/*!
 * @function        IOPMConnectionGetSystemCapabilities
 *
 * @abstract        Gets the instantaneous system power capabilities.
 *  
 * @discussion      Checking the system capabilities can help distinguish whether the system is in DarkWake,
 *                  UserActive wake, or if it's on the way to sleep.
 *
 * @result          A bitfield describing whether the system is asleep, awake, or in dark wake,
 *                  using kIOPMCapability bits.
 */

IOPMCapabilityBits IOPMConnectionGetSystemCapabilities(void);


/*! @define kIOPMSleepWakeInterest
 *  Pass this as an argument to the <code>interests</code> field of IOPMConnectionCreate() to
 *  receive notifications for Sleep, FullWake, and DarkWake system events.
 */
#define kIOPMSleepWakeInterest     (kIOPMCapabilityCPU | kIOPMCapabilityDisk | kIOPMCapabilityNetwork \
                                           | kIOPMCapabilityVideo | kIOPMCapabilityAudio)

/*!
 * IOPMConnectionCreate() opens an IOPMConnection.
 *
 * IOPMConnections provide access to power management notifications. They are a
 * replacement for the existing IORegisterForSystemPower() API, and provide
 * newer functionality and logging as well.
 *
 * Clients are expected to also call IOPMConnectionSetNotification() and
 * IOPMConnectionScheduleWithRunLoop() in order to receive notifications.
 *
 * @param myName Caller should provide a CFStringRef describing its identity, 
 *      for logging.
 * @param interests A bitfield of IOPMCapabilityBits defining
 *      which capabilites the caller is interested in. Caller will only be notified
 *      of changes to the bits specified here. 
 *      Most callers should pass in the argument kIOPMSleepWakeInterest for the usual
 *          sleep, wake, and darkwake notifications.
 * @param newConnection Upon success this will be populated with a fresh IOPMConnection.
 *      The caller must release this with a call to IOPMReleaseConnection.
 * @result Returns kIOReturnSuccess; otherwise on failure.
 */
IOReturn IOPMConnectionCreate(
            CFStringRef myName, 
            IOPMCapabilityBits interests,
            IOPMConnection *newConnection
            ) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*! 
 * IOPMConnectionSetNotification associates a notificiation handler with an IOPMConnection
 * @discussion Caller must also call IOPMConnectionScheduleWithRunLoop to setup the notification.
 * @param param User-supplied pointer will be passed to all invocations of handler.  
 *      This call does not retain the *param struct; caller must ensure that the pointer is 
 *      retained.
 * @param myConnection The IOPMConnection created by calling <code>@link IOPMConnectionCreate @/link<code>
 * @param handler Ths function pointer will be invoked every time an event of interest
 *      occurs.
 */
IOReturn IOPMConnectionSetNotification(
            IOPMConnection myConnection, 
            void *param, 
            IOPMEventHandlerType handler
            ) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
 * IOPMConnectionScheduleWithRunLoop schedules a notification on the specified runloop.
 * @discussion The connection must be unscheduled by calling 
 *      IOPMConnectionUnscheduleFromRunLoop.
 * @param myConnection The IOPMConnection created by calling <code>@link IOPMConnectionCreate @/link</code>
 * @param theRunLoop A pointer to the run loop that the caller wants the callback 
 *     scheduled on. Invoking IOPMConnectionRelease will remove the notification from the 
 *     notification from the specified runloop. 
 * @result Returns kIOReturnSuccess; otherwise on failure.
 */
IOReturn IOPMConnectionScheduleWithRunLoop(
            IOPMConnection myConnection, 
            CFRunLoopRef theRunLoop,
            CFStringRef runLoopMode
            ) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

 
/*!
 * IOPMConnectionUnscheduleFromRunLoop removes a previously scheduled run loop source.
 * @param myConnection The IOPMConnection created by calling <code>@link IOPMConnectionCreate @/link</code>
 * @param theRunLoop A pointer to the run loop that the caller has previously scheduled
 *      a connection upon. 
 * @result Returns kIOReturnSuccess; otherwise on failure.
 */
IOReturn IOPMConnectionUnscheduleFromRunLoop(
            IOPMConnection myConnection, 
            CFRunLoopRef theRunLoop,
            CFStringRef  runLoopMode
            ) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
 * @function    IOPMConnectionSetDispatchQueue
 * @abstract    Schedule or un-schedule IOPMConnection callbacks on a dispatch queue.
 * @discussion  The caller should set a callback via <code>@link IOPMConnectionSetNotification @/link</code>
 *              before setting the dispatch queue.
 * @param myConnection The IOPMConnection created by a call to <code>@link IOPMConnectionCreate @/link</code>
 * @param myQueue   The dispatch queue to schedule notifications on.
 *                  Pass NULL to cancel dispatch queue notifications.
 *                  If the caller passes in a concurrent dispatch_queue_t, sometimes the notification
 *                  handler can be invoked concurrently (e.g. if the caller fails to respond with
 *                  <code>@link IOPMConnectionAcknowledgeEvent @/link</code> before the notification times out).
 */
void IOPMConnectionSetDispatchQueue(
            IOPMConnection myConnection,
            dispatch_queue_t myQueue)
            __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
 * IOPMConnectionRelease cleans up state and notifications for a given
 *      PM connection. This will not remove any scheduled run loop sources - it is the caller's 
 *      responsibility to remove all scheduled run loop sources.
 * @param connection Connection to release.
 * @result Returns kIOReturnSuccess; otherwise on failure.
 */
IOReturn IOPMConnectionRelease(
            IOPMConnection connection
            ) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);


/*! Acknowledge a power management event.
 * 
 * All IOPMConnection notifications must be acknowledged by calling IOPMConnectAcknowledgeEvent,
 *  or IOPMConnectAcknowledgeEventWithOptions. The caller may invoke IOPMConnectAcknowledgeEvent
 * immediately from within its own notify handler, or the caller may invoke acknowledge
 * the notification from another context.
 *
 * @param connect A valid IOPMConnection object
 * @param token A unique token identifying the message this acknowledgement refers to. This token
 *      should have been received as an argument to the IOPMEventHandlerType handler.
 */
IOReturn IOPMConnectionAcknowledgeEvent(
            IOPMConnection connect, 
            IOPMConnectionMessageToken token
            ) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);


/*! Acknowledge a power management event with additional options.
 *
 * To request a maintenance wake while acknowledging a system sleep event, 
 * the caller should create an 'options' dictionary with values set for keys
 *      kIOPMAcknowledgmentOptionWakeDate
 *      and kIOPMAcknowledgmentOptionSystemCapabilityRequirements
 *
 * Note that requesting a maintenance wake is only meaningful if the system is transitioning
 * into a sleep state; that is, only if the capabilities flags are all 0.
 *
 * @param connect A valid IOPMConnection object
 * @param token A unique token identifying the message this acknowledgement refers to.
 * @param options If the response type requires additonal arguments, they may be placed
 *          as objects in the "options" dictionary using 
 *          any of the kIOPMAcknowledgementOption* strings as dictionary keys.
 *          Passing NULL for options is equivalent to calling 
 *          IOPMConnectionAcknowledgeEvent()
 * @result Returns kIOReturnSuccess; otherwise on failure.
 */
IOReturn IOPMConnectionAcknowledgeEventWithOptions(
            IOPMConnection connect, 
            IOPMConnectionMessageToken token, 
            CFDictionaryRef options
            ) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);


/*!
 * @function        IOPMRegisterForRemoteSystemPower
 * @abstract        Register a block to be called when remote node's power state changes.
 *
 * @param queue     The dispatch_queue on which the registered block is called.
 *
 *  @param block    The block to execute when remote node power state changes.
 *                  The block will receive two arguments:
 *                      message     Message indicating the type of state change.
 *                      msgData     Message data.
 *
 * @result          NULL on failure. Upon success, caller should store the
 *                  <code>IOPMNotificationHandle</code> and pass that handle as a parameter to
 *                  <code>IOPMAllowRemotePowerChange</code> and <code>IODeregisterForRemoteSystemPower</code>
 *                  functions.
 */

IOPMNotificationHandle IOPMRegisterForRemoteSystemPower(
                            dispatch_queue_t queue,
                            void (^inblock)(IOMessage message, void *msgData));

/*!
 * @function        IOPMAllowRemotePowerChange
 * @abstract        Call this function to acknowledge the remote node's state change message.
 *
 *  @param ref      The handle returned by function <code>IOPMRegisterForRemoteSystemPower</code>.
 *
 *  @param message  The message client is acknowledging.
 *
 * @result          kIOReturnSuccess on sucess. Otherwise, appropriate error is returned.
 */
IOReturn IOPMAllowRemotePowerChange(IOPMNotificationHandle ref, IOMessage message);

/*!
 * @function        IODeregisterForRemoteSystemPower
 * @abstract        Call this function stop receiving notifications about remote node's power state
 *                  and release all associated data structures.
 *
 *  @param ref      The handle returned by function <code>IOPMRegisterForRemoteSystemPower</code>.
 */
void IODeregisterForRemoteSystemPower(IOPMNotificationHandle ref);

/*! @group IOPower Assertion SPI
 */


typedef enum {
    kIOPMSystemSleepReverted,
    kIOPMSystemSleepNotReverted
} IOPMSystemState;

/*!
 * @function            IOPMAssertionDeclareSystemActivity
 *
 * @abstract            Declares to the power management policy mechanism that a system process
 *                      is performing a critical activity. If the device is in the process of 
 *                      going to a lower power state, PM policy will attempt to awaken the device 
 *                      to a usable state immediately.
 *
 * @discussion          No special privileges are necessary to make this call - any process may
 *                      call this API. Caller must specify an AssertionName - NULL is not
 *                      a valid input.
 *                      IOPMAssertionDeclareSystemActivity cannot revert from forced sleep in progress. 
 *                      IOPMAssertionDeclareSystemActivity can only revert from the early stages of idle sleeps.
 *
 * @param AssertionName     A string that describes the name of the caller and the activity being
 *                           handled by this assertion (e.g. "Baseband reboot in progress").
 *                           Name may be no longer than 128 characters.
 *
 * @param AssertionID      On Success, unique id will be returned in this parameter. It represents
 *                          an assertion of type <code>@link kIOPMAssertPreventUserIdleSystemSleep 
 *                          @/link</code>. Caller must call <code>@link IOPMAssertionRelease@/link</code> 
 *                          on this unique ID to indicate to PM policy that the critical activity was completed.
 *
 * @param SystemState     Returns either kIOPMSystemSleepReverted, or kIOPMSystemSleepNotReverted.    
 *                         kIOPMSystemSleepReverted -
 *                               Indicates that the system was already awake and usable; or we successfully 
 *                               reverted an in-process sleep transition, bringing the device back to full wake.
 *                               The system will attempt to remain awake in a usable state until
 *                              <code>@link IOPMAssertionRelease@/link</code> gets called on
 *                              the AssertionID. 

 *                         kIOPMSystemSleepNotReverted -
 *                              PM policy was not able to act on this assertion. The system is entering a sleep
 *                              state and it cannot be reverted.
 *                              If this is an idle sleep, PM will schedule device wake immediately 
 *                              following this sleep; the device will be awake and usable then.
 *                              Upon that system wakeup, the policy mechanism will try its
 *                              best to remain awake in a usable state until
 *                              <code>@link IOPMAssertionRelease@/link</code> gets called on the
 *                              AssertionID. In this case, client has to wait until it receives
 *                              kIOMessageSystemWillNotSleep message or kIOMessageSystemHasPoweredOn message
 *                              before proceeding with the work that needs to be done with assertion.
 *                              See documentation for IORegisterForSystemPower for details
 *                              about messages kIOMessageSystemHasPoweredOn and kIOMessageSystemWillNotSleep.
 *                              If the sleep transition is a force sleep, IOKit will not attempt to 
 *                              immediately re-awaken the system. 
 *
 * @result                  Returns kIOReturnSuccess on success, any other return indicates
 *                          PM could not successfully activate the specified assertion.
 */
IOReturn IOPMAssertionDeclareSystemActivity(
                        CFStringRef         AssertionName,
                        IOPMAssertionID     *AssertionID,
                        IOPMSystemState     *SystemState);

/*!
 * @function            IOPMAssertionDeclareSystemActivityWithProperties
 *
 * @abstract            Allows the use of <code>@link IOPMAssertionDeclareSystemActivity()@/link</code>
 *                      while allowing the assertion properties to be set, except for
 *                      <code>kIOPMAssertionTypeKey</code>.
 *
 * @result              Returns kIOReturnSuccess on success, any other return indicates
 *                      PM could not successfully activate the specified assertion.
 */
IOReturn IOPMAssertionDeclareSystemActivityWithProperties(
                        CFMutableDictionaryRef     AssertionProperties,
                        IOPMAssertionID            *AssertionID,
                        IOPMSystemState            *SystemState);

IOReturn IOPMChangeSystemActivityAssertionBehavior(uint32_t newFlags, uint32_t *oldFlags);

/*
 * @function        IOPMSetReservePowerMode
 *
 * @abstract        Enables or disables reserve power mode.
 *
 * @discussion      Caller must have kIOPMReservePwrCtrlEntitlement entitlement
 */
IOReturn IOPMSetReservePowerMode(bool enable);


/*
 * @function        IOPMUserIsActive
 *
 * @abstract        Indicates whether the system is awake for user activity.
 *
 * @discussion      IOPMUserIsActive() distinguishes whether the system is awake because of a user generated event,
 *          or a notification event via <code>IOPMAssertionDeclareNotificationEvent</code>.
 *          <code>IOPMUserIsActive</code> returns true for 5 minutes following a user event, and any time a user active assertion is held.
 *          If the display is asleep, or the system is in DarkWake, IOPMUserIsActive will return false.
 *          User activity events include:
 *          <ul><li>HID events
 *              <li>Lid open
 *              <li>Scheduled RTC wakes
 *              <li>IOPMAssertionDeclareUserActivity()
 *          </ul>
 *
 * @result              true or false.
 */
bool IOPMUserIsActive(void);

/*
 * @function    IOPMScheduleUserActiveNotification
 *
 * @param       queue The dispatch_queue to schedule the callout on.
 *
 * @param       block The block to execute when the "user is active" state changes.
 *
 * @result          NULL on failure. Upon success, caller should store the <code>IOPMNotificationHandle</code> and pass it to
 *         <code>IOPMUnregisterNotification</code> to stop receiving notifications.
 */
IOPMNotificationHandle IOPMScheduleUserActiveChangedNotification(dispatch_queue_t queue, void (^block)(bool));

/*
 * @function    IOPMUnregisterNotification
 *
 * @abstract    Cancel and release an <code>IOPMNotificationHandle</code>
 */
void IOPMUnregisterNotification(IOPMNotificationHandle handle);




/*!
 * @enum IOPMUserActivity States
 * @abstract Describes the user's current active state, available via IOPMGetUserActivity.
 *
 * @constant kIOPMUserPresentActive A local physically present user, or a process representing a user,
 *              is actively using the device. This includes typing, using local input devices,
 *              using a remote control, or opening the lid.
 *              Most user actions that light the display are UserPresent actions.
 *              All non-idle network ScreenShared users are also reported as kIOPMUserPresentActive.
 *
 * @constant kIOPMUserPresentPassive The user is probably paying attention to the system, but
 *              isn't interacting with it. This includes watching and
 *              listening to media.
 *
 * @constant kIOPMUserPresentPassiveWithDisplay The user is probably paying attention to the system,
 *              but isn't interacting with it. Display is on. This includes watching and
 *              listening to media.
 *
 * @constant kIOPMUserPresentPassiveWithoutDisplay The user is probably using the system
 *              but isn't interacting with it. Display is off. This includes listening to media.
 *
 * @constant kIOPMUserRemoteClientActive A remote networked client
 *              is connected and actively using this system's resources.
 *              Including SMB, AFP, ssh, and other network 'Sharing' services.
 *
 * @constant kIOPMUserNotificationActive The system is displaying a notification, as a result of
 *              incoming network activity. A local user may or may not be looking
 *              at the machine; as the machine may have been asleep or unattended.
 *
 */
enum {
    kIOPMUserPresentActive                  = (1<<0),
    kIOPMUserPresentPassive                 = (1<<1),
    kIOPMUserPresentPassiveWithDisplay      = (1<<2),
    kIOPMUserPresentPassiveWithoutDisplay   = (1<<3),
    kIOPMUserRemoteClientActive             = (1<<4),
    kIOPMUserNotificationActive             = (1<<5)
};

#define kIOPMDefaultUserActivityTimeout (5*60)  // 5mins

/*!
 * IOPMGetUserActivityLevel describes the user's current activity level at the system.
 *
 * Reports this user activity level independently of the user's display idle sleep time,
 * and system idle sleep times. This call returns the activity levels with default HID
 * idle timeout. If the caller registered the callback with
 * <code>IOPMScheduleUserActivityLevelNotificationWithTimeout</code>, then the value
 * returned IOPMGetUserActivityLevel may not match the timeout specified by the caller.
 *
 * - Where we measure user activity by the frequency of user action, the user's
 *      level shall remain active for 'idle timeout' duration after last activity.
 * - Where we measure activity through IOKit power assertions, we'll assume the
 *      user remains active as long as a power assertion is held.
 * - When the system goes to sleep, we'll zero out the user activity tracking state.
 * - It is a valid state for the system display to be lit and showing state
 *      (say with display Sleep timer = 20 minutes), but for this API to return
 *      activity with kIOPMUserPresentActive clear, because there's no
 *      user input within 'idle timeout' duration.
 *
 * The default value for 'idle timeout' is 5 mins on OS X and 30secs on iOS.
 * This value can be changed using <code>IOPMSetUserActivityIdleTimeout</code>.
 *
 * @param outUserActive is a bitfield, containing zero or more bits from User
 *              Active States enum.
 *
 * @param mostSignificantActivity  When outUserActive includes one or more bits, this is the
 *              one bit that best describes the user's system activity.
 *              highOrderBit will be equal to one of the IOPMUserActivity constants
 *                  kIOPMUserPresentActive, kIOPMUserPresentPassive,
 *                  kIOPMUserRemoteClientActive, or kIOPMUserNotification
 *
 * @result kIOReturnSuccess on success
 *         kIOReturnNotReady if you're calling this too early in boot, before this data is available.
 *         kIOReturnError, or other IOKit error code otherwise.
 */
IOReturn IOPMGetUserActivityLevel(uint64_t *outUserActive,
                                  uint64_t *mostSignificantActivity);


/*
 * @function    IOPMScheduleUserActivityLevelNotification
 *
 * Get a callout when the user activity level changes.
 *
 * @param       queue The dispatch_queue to schedule the callout on.
 *
 * @param       block The block to execute when the state changes.
 *              The block will receive two arguments -
 *                  uint64_t outUserActive,
 *                  and uint64_t mostSignificantActivity
 *              They'll have the same meaning and behavior as described
 *              in function IOPMGetUserActivityLevel.
 *
 * @result      NULL on failure. Upon success, caller should store the
 *              <code>IOPMNotificationHandle</code> and pass that handle to
 *              <code>IOPMUnregisterNotification</code> to stop
 *              receiving notifications.
 */
IOPMNotificationHandle IOPMScheduleUserActivityLevelNotification(dispatch_queue_t queue,
                                                                 void (^block)(uint64_t, uint64_t));

/*
 * @function    IOPMScheduleUserActivityLevelNotificationWithTimeout
 *
 * @abstract    Get a callout when the user activity level changes.
 *
 * @param       queue The dispatch_queue to schedule the callout on.
 *
 * @paramt      timeout  Number of seconds after last HID activity is user considered
 *                       idle. Display state may overide the HID activity.
 *
 * @param       block The block to execute when the state changes.
 *              The block will receive two arguments -
 *                  uint64_t outUserActive,
 *                  and uint64_t mostSignificantActivity
 *
 * @result      NULL on failure. Upon success, caller should store the
 *              <code>IOPMNotificationHandle</code> and pass that handle to
 *              <code>IOPMUnregisterNotification</code> to stop
 *              receiving notifications.
 */
IOPMNotificationHandle IOPMScheduleUserActivityLevelNotificationWithTimeout(
                                     dispatch_queue_t queue,
                                     uint32_t timeout,
                                     void (^inblock)(uint64_t, uint64_t));
/*
 * @function    IOPMSetUserActivityIdleTimeout
 *
 * @abstract    Change the duration for which user is considered active after
 *              the most recent HID activity. This can be called only if the caller
 *              registered callback with <code>IOPMScheduleUserActivityLevelNotificationWithTimeout</code>
 *
 * @param handle        The handle returned by IOPMScheduleUserActivityLevelNotification.
 *
 * @param idleTimeout   The duration in seconds for which user is considered active
 *                      after the most recent HID activity.
 *
 * @result              'kIOReturnSuccess' on success. Otherwise, appropriate error
 *                      is returned.
 */
IOReturn IOPMSetUserActivityIdleTimeout(IOPMNotificationHandle handle, uint32_t idleTimeout);

/*!
 * IOPMCopyUserActivityLevelDescription
 * Creates a debugging string describing a IOPMUserActivity state.
 */
CFStringRef IOPMCopyUserActivityLevelDescription(uint64_t userActive);


/*!
 * @function    IOPMAssertionDeclareNotificationEvent
 * 
 * @abstract    Temporarily light the display to ensure notifications are visible to users.
 *
 * @discussion  <ul><li>IOPMAssertionDeclareNotificationEvent shall light the display if 
 *              it's not already lit. The display shall remain lit either 
 *                  <ol><li> for the interval specified in secondsToDisplay, 
 *                  <li> or until the caller releases the returned AssertionID with IOPMAssertionRelease()
 *                  <li> or until a user action intervenes and sleeps the display (lid close, system sleep, system shutdown).
 *                  </ol>
 *              <li> IOPMAssertionDeclareNotificationEvent is intended only for NotificationCenter. 
 *              <li> IOPMAssertionDeclareNotificationEvent shall require an entitlement to execute.
 *              <li> When secondsToDisplay timer expires, the display, and Display Sleep idle timers, shall return to their previous state.
 *              <li> If the Mac lit the display to display the notification, the Mac shall dim the display promptly upon completion (unless other system activity would keep the display awake).
 *              <li> If the Mac woke from sleep to display this message, the Mac shall return to sleep promptly upon completion (unless other system activity would keep the Mac awake).
 *              <li> IOPMAssertionDeclareNotificationEvent shall not light the screen if the lid is closed.
 *              <li> IOPMAssertionDeclareNotificationEvent will not modify the user's chosen brightness setting. When the display lights, it will light to the user's previously chosen brightness setting.
 *              <li> IOPMAssertionDeclareNotificationEvent shall not affect the user's HID idleness counters; and it shall not interrupt regular idle display sleep timers.
 *              <li> If Display Sleep == 10 minutes, and the user has been idle for 9 minutes, and NC calls IOPMAssertionDeclareNotificationEvent(secondsToDisplay=5.0) then the display shall still idle sleep at 10 minutes.
 *              </ul>
 *
 * @param       notificationName A human-readable string (it can be reverse-DNS), 
 *              describing the event your App is displaying to the user.
 *              This key is present for admin and debugging use with: 
 *                pmset -g log; pmset -g assertions; pmset -g assertionslog
 * @param       secondsToDisplay CFTimeInterval number of seconds to keep the display lit.
 * @param       AssertionID Optional return value. Upon success, AssertionID will point
 *              to a valid power assertion. To call IOPMAssertionDeclareNotificationEvent,
 *              repeatedly please reuse the returned assertion ID.
 *              You can also call IOPMAssertionRelease(AssertionID) to release the power
 *              assertion (and possibly dim the display) before the secondsToDisplay timer fires.
 * @result      <ul><li>kIOReturnSuccess on success.
 *              <li>kIOReturnNotPrivileged for missing entitlement or privilege.
 *              <li>kIOReturnNotReady if OS X cannot light the display (e.g. lid closed)
 *              <li>kIOReturnError otherwise. </ul>
 */
 IOReturn IOPMAssertionDeclareNotificationEvent(
                        CFStringRef          notificationName,
                        CFTimeInterval       secondsToDisplay,
                        IOPMAssertionID      *AssertionID)
                        __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);


/*!
 * kIOPMIdleSleepPreventersNotifyName
 *
 * Notification name used for BSD notifications. A notification is posted on this name
 * whenever the number of kexts preventing idle sleep changes. This count is saved
 * as the state of this notification.
 */
#define kIOPMIdleSleepPreventersNotifyName  "com.apple.powermanagement.idlesleeppreventers"

/*!
 * kIOPMSystemSleepPreventersNotifyName
 *
 * Notification name used for BSD notifications. A notification is posted on this name
 * whenever the number of kexts preventing system sleep changes. This count is saved
 * as the state of this notification.
 */
#define kIOPMSystemSleepPreventersNotifyName  "com.apple.powermanagement.systemsleeppreventers"

/*!
 * IOPMCopySleepPreventersList
 *
 * Returns an array of kext names that are preventing idle sleep/system sleep
 */
enum
{
    kIOPMIdleSleepPreventers    = 0,
    kIOPMSystemSleepPreventers  = 1
};
IOReturn  IOPMCopySleepPreventersList(int preventerType, CFArrayRef *outArray);

/*!
 * IOPMCopySleepPreventersListWithID
 *
 * Returns an array of dictionary entries with kext names and registryIDs that are 
 * preventing idle sleep/system sleep
 * Dictionary keys -
 * kIOPMDriverAssertionRegistryEntryIDKey : <registry entry id>
 * kIOPMDriverAssertionOwnerStringKey : <kext name>
 */
IOReturn  IOPMCopySleepPreventersListWithID(int preventerType, CFArrayRef *outArray);

/*!
 * kIOPMSleepServiceActiveNotifyName
 *
 * This API is optional; use <code>IOPMGetSleepServicesActive()</code> for thee easiest way
 * to query this information.
 * Listen for changes to the SleepServices state from 0 to 1 and from 1 to 0 on this notify bit.
 * Powerd calls this notification when it enters and exits a SleepService waiting state.
 */
#define kIOPMSleepServiceActiveNotifyName   "com.apple.powermanagement.sleepservices"

/*!
 * kIOPMSleepServiceActiveNotifyBit
 * When set, this bit indicates the system is handling SleepServices work.
 *
 * You should probably use <code>IOPMGetSleepServicesActive</code> instead of this
 * for the easiest way to query this state.
 */
#define kIOPMSleepServiceActiveNotifyBit    (1 << 0)

/*!
 * @function        IOPMGetSleepServicesActive
 *
 * @abstract        Checks whether we're staying awake to do SleepServices work at this instant.
 *
 * @result          Returns true starting after SleepServicesd communicates a SleepService Wakeup Cap time;
 *                  and ending when that cap time expires.
 */

bool IOPMGetSleepServicesActive(void);


/*!
 * @function        IOPMGetDarkWakeThermalEmergencyCount
 *
 * @abstract		Returns the number of times that a Fanless DarkWake slept instead of turning fans on.
 *
 * @discussion      A DarkWake Thermal emergency implies that the processes still held power assertions,
 *                  and were still doing useful work at the time.
 *                  A "DarkWake Thermal emergency" (where the system must sleep rather than engage the fans)
 *                  is different than a system-wide "Thermal emergency" (where the system must sleep because 
 *                  the fans cannot dissipate heat.) This API records the former.
 *
 *                  Use this SPI for statistics gathering only.
 *
 * @result          Zero at boot, increments on every DarkWake Thermal Emergency thereafter.
 */
int IOPMGetDarkWakeThermalEmergencyCount(void);



/*! @group Debug SPI
 */

/*
 * Selctor arguments to IOPMGetValueInt and IOPMSetValueInt
 */
enum {
    kIOPMGetSilentRunningInfo                        = 1,
    kIOPMMT2Bookmark,
    kIOPMSetNoPoll,
    kIOPMDarkWakeThermalEventCount,
    kIOPMTCPKeepAliveExpirationOverride,
    kIOPMTCPKeepAliveIsActive,
    kIOPMSetAssertionActivityLog,
    kIOPMSetAssertionActivityAggregate,
    kIOPMSetReservePowerMode,
    kIOPMWakeOnLanIsActive,
    kIOPMPushConnectionActive,
};

/*!
 * @function        IOPMGetValueInt
 * @abstract        For IOKit internal use only.
 */
int IOPMGetValueInt(int selector);

/*!
 * @function        IOPMSetValueInt
 * @abstract        For IOKit internal use only.
 */
IOReturn IOPMSetValueInt(int selector, int value);

IOReturn IOPMSetDebugFlags(uint32_t newFlags, uint32_t *oldFlags);
IOReturn IOPMSetBTWakeInterval(uint32_t newInterval, uint32_t *oldInterval);
IOReturn IOPMSetDWLingerInterval(uint32_t newInterval);

/* ops for IOPMCtlAssertionType() */
#define kIOPMDisableAssertionType 0x1
#define kIOPMEnableAssertionType 0x2
IOReturn IOPMCtlAssertionType(char *type, int op);

/*!
 * @constant    kIOPMPerformanceWarningNotificationKey
 * @abstract    Notify(3) string that PM fires every time there is performance warning change
 */

#define kIOPMPerformanceWarningNotificationKey      "com.apple.system.power.performance_warning"

/*!
 * @function    IOPMGetPerformanceWarningLevel
 * @abstract    Get performance warning level of the system.
 * @result      An integer pointer declaring the performance warning level of the system.
 *              The value of the integer is one of (defined in IOPMPrivate.h):
 *              <ul>
 *                  <li>kIOPMPerformanceNormal
 *                  <li>kIOPMPerformanceWarning
 *              </ul>
 *              Upon success the performance level value will be found at the
 *              pointer argument
 * @result      kIOReturnSuccess, or other error report. Returns kIOReturnNotFound if
 *              performance warning level has not been published.
 */

IOReturn IOPMGetPerformanceWarningLevel(uint32_t *perfLevel);


/*! @group IOReporting power SPI
 */

/*
 * Keys used in the dictionary returned by IOPMCopyPowerStateInfo()
 */
#define kIOPMNodeCurrentState                 CFSTR("CurrentState")
#define kIOPMNodeMaxState                     CFSTR("MaxState")
#define kIOPMNodeIsPowerOn                    CFSTR("IsPowerOn")
#define kIOPMNodeIsDeviceUsable               CFSTR("IsDeviceUsable")
#define kIOPMNodeIsLowPower                   CFSTR("IsLowPower")
/*!
 * @function        IOPMCopyPowerStateInfo
 *
 * @abstract        Returns a dictionary describing the state_id obtained from 
 *                  IOReporting channels kPMCurrStateChID and kPMPowerStatesChID. 
 *
 * @discussion      On sucess, caller is responsible for releasing the dictionary.
 *
 * @param state_id  Caller passes the state_id value obtained from kPMCurrStateChID 
 *                  and kPMPowerStatesChID channels of IOServicePM.
 *
 * @result          Returns a CFDictionaryRef or NULL. Caller must CFRelease the dictionary.
*/
CFDictionaryRef  IOPMCopyPowerStateInfo(uint64_t state_id);


/* Request type(req_type) for IOPMAssertionNotify() */
enum {
    kIOPMNotifyRegister = 0x1,
    kIOPMNotifyDeRegister = 0x2
};
/* Register/De-Register for assertion change notifications
 * name     - Type of notification. Should be one of kIOPMAssertionsAnyChangedNotifyString, 
 *            kIOPMAssertionsChangedNotifyString and kIOPMAssertionTimedOutNotifyString
 * req_type - Should be either kIOPMNotifyRegister or kIOPMNotifyDeRegister
 */
IOReturn IOPMAssertionNotify(char *name, int req_type);

/* Process Assertion State Types */
typedef enum {
    kIOPMAssertionProcessStateSuspend = 0x1, /* Set State to Suspended */
    kIOPMAssertionProcessStateResume = 0x2   /* Set State to Resume */
} IOPMAssertionProcessStateType;

/*!
 * @function        IOPMAssertionSetProcessState
 *
 * @abstract        Activates or Deactivates the state of assertions held by a process
 *
 * @discussion      Caller must entitlement kIOPMAssertionSuspendResumeEntitlement
 *
 * @param pid       Process id for which assertion state is to be changed
 *
 * @param state     Specifies type of state change as defined by <code>IOPMAssertionProcessStateType</code>.
 *
 * @result          kIOResturnSuccess, or another IOKit return code on error.
 */
IOReturn IOPMAssertionSetProcessState(pid_t pid, IOPMAssertionProcessStateType state);

/*
 * Enable/disable back trace collection at the time of assertion creation.
 * The backtrace collected can be retrieved with IOPMCopyAssertionsByProcess() API.
 */
void IOPMAssertionSetBTCollection(bool enable);


IOReturn IOPMEnableAsyncAssertions();
IOReturn IOPMDisableAsyncAssertions();


__END_DECLS



#endif // _IOPMLibPrivate_h_

