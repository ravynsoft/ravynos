/*
 * Copyright (c) 1998-2005 Apple Computer, Inc. All rights reserved.
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

#include <CoreFoundation/CFArray.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/pwr_mgt/IOPMLibDefs.h>
#include <IOKit/pwr_mgt/IOPMKeys.h>
#include <Availability.h>

#ifndef _IOKIT_PWRMGT_IOPMLIB_
#define _IOKIT_PWRMGT_IOPMLIB_

#ifdef __cplusplus
extern "C" {
#endif

/*!
@header IOPMLib.h
    IOPMLib provides access to common power management facilities, like initiating 
    system sleep, getting current idle timer values, registering for sleep/wake notifications, 
    and preventing system sleep.
*/

/*! @function IOPMFindPowerManagement
    @abstract Finds the Root Power Domain IOService.
    @param master_device_port  Just pass in MACH_PORT_NULL for master device port.
    @result Returns a io_connect_t handle on the root domain. Must be released with IOServiceClose() when done.
 */
io_connect_t IOPMFindPowerManagement( mach_port_t master_device_port )
                            AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;
    
/*! @function IOPMSetAggressiveness
    @abstract Sets one of the aggressiveness factors in IOKit Power Management.
    @param fb  Representation of the Root Power Domain from IOPMFindPowerManagement.
    @param type Specifies which aggressiveness factor is being set.
    @param aggressiveness New value of the aggressiveness factor.
    @result Returns kIOReturnSuccess or an error condition if request failed.
*/
IOReturn IOPMSetAggressiveness (io_connect_t fb, unsigned long type, unsigned long aggressiveness )
                            AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;
                            
/*! @function IOPMGetAggressiveness
    @abstract Retrieves the current value of one of the aggressiveness factors in IOKit Power Management.
    @param fb  Representation of the Root Power Domain from IOPMFindPowerManagement.
    @param type Specifies which aggressiveness factor is being retrieved.
    @param aggressiveness Points to where to store the retrieved value of the aggressiveness factor.
    @result Returns kIOReturnSuccess or an error condition if request failed. 
 */
IOReturn IOPMGetAggressiveness ( io_connect_t fb, unsigned long type, unsigned long * aggressiveness )
                            AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*! @function IOPMSleepEnabled
    @abstract Tells whether the system supports full sleep, or just doze
    @result Returns true if the system supports sleep, false if some hardware prevents full sleep.
 */
boolean_t IOPMSleepEnabled ( void ) AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*! @function IOPMSleepSystem
    @abstract Request that the system initiate sleep.
    @discussion For security purposes, caller must be root or the console user.
    @param fb  Port used to communicate to the kernel,  from IOPMFindPowerManagement.
    @result Returns kIOReturnSuccess or an error condition if request failed.
 */
IOReturn IOPMSleepSystem ( io_connect_t fb ) AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*! @function IOPMCopyBatteryInfo
     @abstract Request battery data from the system.
     @discussion This API is supported, but not recommended. Developers should prefer to use
     the IOPowerSources API. IOPowerSources provides more battery data, and notifications
     when battery state changes)
     @param masterPort The master port obtained from IOMasterPort(). Just pass MACH_PORT_NULL.
     @param info A CFArray of CFDictionaries containing raw battery data. Use these keys, defined in IOPM.h
            to read from the dictionary:
         <pre>
         <code>@link kIOBatteryInfoKey @/link</code>
         <code>@link kIOBatteryCurrentChargeKey @/link</code>
         <code>@link kIOBatteryCapacityKey @/link</code>
         <code>@link kIOBatteryFlagsKey @/link</code>
         <code>@link kIOBatteryVoltageKey @/link</code>
         <code>@link kIOBatteryAmperageKey @/link</code>
         <code>@link kIOBatteryCycleCountKey @/link</code>
         </pre>
     @result Returns kIOReturnSuccess or an error condition if request failed.
 */
IOReturn IOPMCopyBatteryInfo( mach_port_t masterPort, CFArrayRef * info )
                            AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;



/*!
    @functiongroup Notifications
*/    


/*! @function IORegisterApp
    @abstract DEPRECATED - An obsolete method for interacting with driver power state changes.
    @discussion This function is obsolete and deprecated. To receive notifications of driver power state changes,
        			Please use IOServiceAddInterestNotification with interest type gIOGeneralInterest instead.
 */
io_connect_t IORegisterApp( void * refcon,
                            io_service_t theDriver,
                            IONotificationPortRef * thePortRef,
                            IOServiceInterestCallback callback,
                            io_object_t * notifier )
                            __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_9, __IPHONE_NA, __IPHONE_NA);
    
/*! @function       IORegisterForSystemPower
    @abstract       Connects the caller to the Root Power Domain IOService for the purpose of receiving sleep & wake notifications for the system.
				    Does not provide system shutdown and restart notifications.
    @discussion     Provides sleep/wake notifications to applications. Requires that applications acknowledge
                    some, but not all notifications. Register for sleep/wake notifications will deliver these messages
                    over the sleep/wake lifecycle:
            <ul>
            <li>kIOMessageSystemWillSleep is delivered at the point the system is initiating a 
                non-abortable sleep.
                Callers MUST acknowledge this event by calling @link IOAllowPowerChange @/link.
                If a caller does not acknowledge the sleep notification, the sleep will continue anyway after
                a 30 second timeout (resulting in bad user experience). 
                Delivered before any hardware is powered off.

            <li>kIOMessageSystemWillPowerOn is delivered at early wakeup time, before most hardware has been
                powered on. Be aware that any attempts to access disk, network, the display, etc. may result
                in errors or blocking your process until those resources become available.
                Caller must NOT acknowledge kIOMessageSystemWillPowerOn; the caller must simply return from its handler.
            
            <li>kIOMessageSystemHasPoweredOn is delivered at wakeup completion time, after all device drivers and
                hardware have handled the wakeup event. Expect this event 1-5 or more seconds after initiating
                system wakeup.
                Caller must NOT acknowledge kIOMessageSystemHasPoweredOn; the caller must simply return from its handler.

            <li>kIOMessageCanSystemSleep indicates the system is pondering an idle sleep, but gives apps the
                chance to veto that sleep attempt. 
                Caller must acknowledge kIOMessageCanSystemSleep by calling @link IOAllowPowerChange @/link
                or @link IOCancelPowerChange @/link. Calling IOAllowPowerChange will not veto the sleep; any
                app that calls IOCancelPowerChange will veto the idle sleep. A kIOMessageCanSystemSleep 
                notification will be followed up to 30 seconds later by a kIOMessageSystemWillSleep message.
                or a kIOMessageSystemWillNotSleep message.

            <li>kIOMessageSystemWillNotSleep is delivered when some app client has vetoed an idle sleep
                request. kIOMessageSystemWillNotSleep may follow a kIOMessageCanSystemSleep notification,
                but will not otherwise be sent.
                Caller must NOT acknowledge kIOMessageSystemWillNotSleep; the caller must simply return from its handler.
            </ul>
        To deregister for sleep/wake notifications, the caller must make two calls, in this order:
            <ol><li> Call IODeregisterForSystemPower with the 'notifier' argument returned here.
            <li> Then call IONotificationPortDestroy passing the 'thePortRef' argument
                returned here.
            </ol>  
    @param refcon       Caller may provide data to receive as an argument to 'callback' on power state changes.
    @param thePortRef   On return, thePortRef is a pointer to an IONotificationPortRef, which will deliver the power notifications. 
                        The port is allocated by this function and must be later released by the caller (after calling <code>@link IODeregisterForSystemPower @/link</code>). 
                        The caller should also enable IONotificationPortRef by calling <code>@link IONotificationPortGetRunLoopSource @/link</code>, or 
                        <code>@link IONotificationPortGetMachPort @/link</code>, or <code>@link IONotificationPortSetDispatchQueue @/link</code>.
    @param callback     A c-function which is called during the notification.
    @param notifier     On success, returns a pointer to a unique notifier which caller must keep and pass to a subsequent call to IODeregisterForSystemPower.
    @result             Returns a io_connect_t session for the IOPMrootDomain or IO_OBJECT_NULL if request failed.
                        Caller must close return value via IOServiceClose() after calling IODeregisterForSystemPower on the notifier argument.
 */
io_connect_t IORegisterForSystemPower ( void * refcon,
                                        IONotificationPortRef * thePortRef,
                                        IOServiceInterestCallback callback,
                                        io_object_t * notifier )
                                        AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;                                        
/*! @function           IODeregisterApp
    @abstract           Disconnects the caller from an IOService after receiving power state change notifications from the IOService. (Caller must also release IORegisterApp's return io_connect_t and returned IONotificationPortRef for complete clean-up).
    @param notifier     An object from IORegisterApp.
    @result             Returns kIOReturnSuccess or an error condition if request failed.
 */
IOReturn IODeregisterApp ( io_object_t * notifier ) AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*! @function           IODeregisterForSystemPower
    @abstract           Disconnects the caller from the Root Power Domain IOService after receiving system power state change notifications. (Caller must also destroy the IONotificationPortRef returned from IORegisterForSystemPower.)
    @param notifier     The object returned from IORegisterForSystemPower.
    @result             Returns kIOReturnSuccess or an error condition if request failed.
*/
IOReturn IODeregisterForSystemPower ( io_object_t * notifier )
                            AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;
                            
/*! @function           IOAllowPowerChange
    @abstract           The caller acknowledges notification of a power state change on a device it has registered for notifications for via IORegisterForSystemPower or IORegisterApp.
    @discussion         Must be used when handling kIOMessageCanSystemSleep and kIOMessageSystemWillSleep messages from IOPMrootDomain system power. The caller should not call IOAllowPowerChange in response to any messages 
                        except for these two.
    @param kernelPort   Port used to communicate to the kernel,  from IORegisterApp or IORegisterForSystemPower.
    @param notificationID A copy of the notification ID which came as part of the power state change notification being acknowledged.
    @result             Returns kIOReturnSuccess or an error condition if request failed.
*/
IOReturn IOAllowPowerChange ( io_connect_t kernelPort, intptr_t notificationID )
                            AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*! @function IOCancelPowerChange
    @abstract The caller denies an idle system sleep power state change.
    @discussion Should only called in response to kIOMessageCanSystemSleep messages from IOPMrootDomain. IOCancelPowerChange has no meaning for responding to kIOMessageSystemWillSleep (which is non-abortable) or any other messages. 
    
    When an app responds to a kIOMessageCanSystemSleep message by calling IOCancelPowerChange, the app
    vetoes the idle sleep request. The system will stay awake. 
    The idle timer will elapse again after a period of inactivity, and the system will
    send out the same kIOMessageCanSystemSleep message, and interested applications will respond gain.
 
    @param kernelPort  Port used to communicate to the kernel,  from IORegisterApp or IORegisterForSystemPower.
    @param notificationID A copy of the notification ID which came as part of the power state change notification being acknowledged.
    @result Returns kIOReturnSuccess or an error condition if request failed.
 */
IOReturn IOCancelPowerChange ( io_connect_t kernelPort, intptr_t notificationID )
                            AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;

/*!
    @functiongroup ScheduledEvents
*/    

/*! @function IOPMSchedulePowerEvent
    @abstract Schedule the machine to wake from sleep, power on, go to sleep, or shutdown. 
    @discussion This event will be added to the system's queue of power events and stored persistently on disk. The sleep and shutdown events present a graphical warning and allow a console user to cancel the event. Must be called as root.
    @param time_to_wake Date and time that the system will power on/off.
    @param my_id A CFStringRef identifying the calling app by CFBundleIdentifier. May be NULL.
    @param type The type of power on you desire, either wake from sleep or power on. Choose from:
        <ul>
                <li>CFSTR(<code>kIOPMAutoWake</code>) == wake machine, 
                <li>CFSTR(<code>kIOPMAutoPowerOn</code>) == power on machine, 
                <li>CFSTR(<code>kIOPMAutoWakeOrPowerOn</code>) == wake or power on,
                <li>CFSTR(<code>kIOPMAutoSleep</code>) == sleep machine, 
                <li>CFSTR(<code>kIOPMAutoShutdown</code>) == power off machine,
                <li>CFSTR(<code>kIOPMAutoRestart</code>) == restart the machine.
        </ul>
    @result kIOReturnSuccess on success, otherwise on failure
*/
IOReturn IOPMSchedulePowerEvent(CFDateRef time_to_wake, CFStringRef my_id, CFStringRef type);

/*! @function IOPMCancelScheduledPowerEvent
    @abstract Cancel a previously scheduled power event.
    @discussion Arguments mirror those to IOPMSchedulePowerEvent. All arguments must match the original arguments from when the power on was scheduled. Must be called as root.
    @param time_to_wake Cancel entry with this date and time.
    @param my_id Cancel entry with this name.
    @param type Type to cancel
    @result kIOReturnSuccess on success, otherwise on failure
*/
IOReturn IOPMCancelScheduledPowerEvent(CFDateRef time_to_wake, CFStringRef my_id, CFStringRef type);

/*! @function IOPMCopyScheduledPowerEvents
    @abstract List all scheduled system power events
    @discussion Returns a CFArray of CFDictionaries of power events. Each CFDictionary  contains keys for CFSTR(kIOPMPowerEventTimeKey), CFSTR(kIOPMPowerEventAppNameKey), and CFSTR(kIOPMPowerEventTypeKey).
    @result A CFArray of CFDictionaries of power events. The CFArray must be released by the caller. NULL if there are no scheduled events.
*/
CFArrayRef IOPMCopyScheduledPowerEvents(void); 

    
#pragma mark IOKit Power Assertions
/*!
 *
 * @functiongroup   IOPMAssertions
 *
 */
    
        
/*!
 * @define          kIOPMAssertPreventUserIdleSystemSleep
 *
 * @abstract        Prevents the system from sleeping automatically due to a lack of user activity.
 *
 * @discussion      When asserted and set to level <code>@link kIOPMAssertionLevelOn@/link</code>,
 *                  will prevent the system from sleeping due to a period of idle user activity.
 *                  Create this assertion with
 *                      <code>@link IOPMAssertionCreateWithName @/link</code>
 *                      or <code>@link IOPMAssertionCreateWithDescription @/link</code>
 *
 *                  The display may dim and idle sleep while <code>kIOPMAssertPreventUserIdleSystemSleep</code> is
 *                  enabled, but the system may not idle sleep. The system may still sleep for lid close, 
 *                  Apple menu, low battery, or other sleep reasons.
 *
 *                  This assertion has no effect if the system is in Dark Wake.
 *
 */
#define kIOPMAssertPreventUserIdleSystemSleep               CFSTR("PreventUserIdleSystemSleep")

/*!
 * @define          kIOPMAssertPreventUserIdleDisplaySleep
 *
 * @abstract        Prevents the display from dimming automatically.
 *
 * @discussion      Create this assertion with
 *                      <code>@link IOPMAssertionCreateWithName @/link</code>
 *                      or <code>@link IOPMAssertionCreateWithDescription @/link</code>.
 * 
 *                  When asserted and set to level <code>@link kIOPMAssertionLevelOn@/link</code>,
 *                  will prevent the display from turning off due to a period of idle user activity.
 *                  Note that the display may still sleep for other reasons, like a user closing a
 *                  portable's lid or the machine sleeping. If the display is already off, this
 *                  assertion does not light up the display. If display needs to be turned on, then
 *                  consider calling function <code>@link IOPMAssertionDeclareUserActivity@/link</code>.
 *
 *                  While the display is prevented from dimming, the system cannot go into idle sleep.
 *
 *                  This assertion has no effect if the system is in Dark Wake.
 */
#define kIOPMAssertPreventUserIdleDisplaySleep              CFSTR("PreventUserIdleDisplaySleep")

/*!
 * @define          kIOPMAssertPreventDiskIdle
 *
 * @abstract        Prevent attached disks from idling into lower power states.
 *
 * @discussion      When asserted and set to level <code>@link kIOPMAssertionLevelOn@/link</code>,
 *                  will prevent attached disks and optical media from idling into lower power states.
 *                  Create this assertion with
 *                      <code>@link IOPMAssertionCreateWithName @/link</code>
 *                      or <code>@link IOPMAssertionCreateWithDescription @/link</code>.
 *
 *                  Apps who rely on real-time access to disks should create this assertion to avoid
 *                  latencies caused by disks changing power states. For example, audio and video performance
 *                  or recording apps may benefit from this assertion. Most Apps should not take this assertion;
 *                  preventing disk idle consumes battery life, and most apps don't require the low latency
 *                  disk access that this provides.
 *
 *                  This assertion doesn't increase a disk's power state (it just prevents that device from idling).
 *                  After creating this assertion, the caller should perform disk I/O on the necessary drives to
 *                  ensure that they're in a usable power state.
 *
 *                  The system may still sleep while this assertion is active.
 *                  Callers should also take <code>@link kIOPMAssertPreventUserIdleSystemSleep@/link</code>
 *                  if necessary, to prevent idle system sleep.
 */
#define kIOPMAssertPreventDiskIdle                          CFSTR("PreventDiskIdle")

/*!
 * @define          kIOPMAssertNetworkClientActive
 *
 * @abstract        Keeps the system awake while OS X serves active network clients.
 *
 * @discussion      When asserted and set to level <code>@link kIOPMAssertionLevelOn@/link</code>,
 *                  will keep the computer awake. Create this assertion with 
 *                      <code>@link IOPMAssertionCreateWithName @/link</code>
 *                      or <code>@link IOPMAssertionCreateWithDescription @/link</code>.
 *
 *                  Instead of taking this assertion, most callers should instead use: 
 *                  <code>@link IOPMDeclareNetworkClientActivity @/link</code>
 *                  it takes the assertion, but with a built-in timeout.
 *
 *                  This assertion keeps the system awake in dark or full wake,
 *                  as long as the system is on AC power. On battery, this assertion can 
 *                  prevent system from going into idle sleep.
 *                  IOKit power assertions are suggestions and OS X may not honor them under
 *                  battery, thermal, or user  circumstances.
 *
 *                  This assertion provides CPU, disk, and network connectivity. 
 *                  If the network is no longer available, this assertion may stop working 
 *                  and allow the system to go to sleep.
 *
 *                  Callers should take this assertion when they have remote clients connected and active.
 *                  Please <code>@link IOPMAssertionRelease @/link></code> this assertion if remote clients become 
 *                  inactive, idle, or disconnected. 
 *                  If your process already manages user timeouts and tracks activity,
 *                  you can take this assertion directly with 
 *                  <code>@link IOPMAssertionCreateWithProperties @/link</code>. 
 *
 *                  IOKit can manage remote client idleness for you if you call 
 *                  <code>@link IOPMDeclareNetworkClientActivity @/link</code> upon every remote access,
 *
 *                  This assertion is a suggestion; Mac OS X may need to sleep the system even if
 *                  this assertion is active.
 */
#define kIOPMAssertNetworkClientActive                             CFSTR("NetworkClientActive")
    

    
/*! @/defineblock   IOPMAssertionDictionary Keys*/

/*!
 * @typedef         IOPMAssertionID
 *
 * @abstract        Type for AssertionID arguments to <code>@link IOPMAssertionCreateWithProperties@/link</code>
 *                  and <code>@link IOPMAssertionRelease@/link</code>
 */
typedef uint32_t IOPMAssertionID;


/*!
 * @enum            kIOPMNullAssertionID
 *
 * @abstract        This value represents a non-initialized assertion ID
 *
 * @constant        kIOPMNullAssertionID
 *                  This value represents a non-initialized assertion ID.
 *
 */
enum {
    kIOPMNullAssertionID = 0
};

/*!
 * @typedef         IOPMAssertionLevel
 *
 * @abstract        Type for AssertionLevel argument to IOPMAssertionCreate
 *
 * @discussion      Possible values for <code>IOPMAssertionLevel</code> are
 *                  <code>@link kIOPMAssertionLevelOff@/link</code>
 *                  and <code>@link kIOPMAssertionLevelOn@/link</code>
 */
typedef uint32_t IOPMAssertionLevel;

/*!
 * @enum            Assertion Levels
 */
enum {
    /*!
     * @constant    kIOPMAssertionLevelOff
     * @abstract    Level for a disabled assertion, passed as an argument to IOPMAssertionCreate.
    */
    kIOPMAssertionLevelOff = 0,

    /*!
     * @constant    kIOPMAssertionLevelOn
     * @abstract    Level for an enabled assertion, passed as an argument to IOPMAssertionCreate.
    */
    kIOPMAssertionLevelOn  = 255
 };


typedef enum {
    kIOPMUserActiveLocal,   /* User is local on the system */
    kIOPMUserActiveRemote   /* Remote User connected to the system */
} IOPMUserActiveType;

    
/*! @function           IOPMAssertionCreateWithDescription
 *
 * @abstract            The preferred API to create a power assertion.
 *
 * @description         Creates an IOPMAssertion. This is the preferred API to call to create an assertion.
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
 *	                    Caller may pass NULL, but it helps power users and administrators identify the
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
 * @param AssertionID   (Output) On successful return, contains a unique reference to a PM assertion.
 *
 * @result              kIOReturnSuccess, or another IOKit return code on error.
 */
IOReturn	IOPMAssertionCreateWithDescription(
                                           CFStringRef  AssertionType,
                                           CFStringRef  Name,
                                           CFStringRef  Details,
                                           CFStringRef  HumanReadableReason,
                                           CFStringRef  LocalizationBundlePath,
                                           CFTimeInterval   Timeout,
                                           CFStringRef  TimeoutAction,
                                           IOPMAssertionID  *AssertionID)
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);



/*!
 * @function                    IOPMAssertionCreateWithProperties
 *
 * @abstract                    Creates an IOPMAssertion with more flexibility than <code>@link IOPMAssertionCreateWithDescription @/link</code>.
 * @param AssertionProperties   Dictionary providing the properties of the assertion that need to be created.
 * @param AssertionID           (Output) On successful return, contains a unique reference to a PM assertion.
 *
 * @discussion
 *      Create a new PM assertion - the caller must specify the type of assertion, initial level, and its
 *      properties as @link IOPMAssertionDictionaryKeys@/link keys in the <code>AssertionProperties</code> dictionary.
 *      The following keys are recommend and/or required to be specified in the AssertionProperties
 *      dictionary argument.
 *      <ul>
 *          <li> REQUIRED: <code>kIOPMAssertionTypeKey</code> define the assertion type.
 *
 *          <li> REQUIRED: <code>kIOPMAssertionNameKey</code> Caller must describe the name for the activity that
 *               requires the change in behavior provided by the assertion.
 *
 *          <li> OPTIONAL: <code>kIOPMAssertionLevelKey</code> define an inital value. If not set, assertion is
 *               turned on after creation.
 *
 *          <li> OPTIONAL: <code>kIOPMAssertionDetailsKey</code> Caller may describe context-specific data about the
 *               assertion.
 *
 *          <li> OPTIONAL: <code>kIOPMAssertionHumanReadableReasonKey</code> Caller may describe the reason for creating the assertion
 *              in a localizable CFString. This should be a human readable phrase that describes the actions the
 *              calling process is taking while the assertion is held, like "Downloading TV episodes", or "Compiling Projects"
 *
 *          <li> OPTIONAL: <code>kIOPMAssertionLocalizationBundlePathKey</code> Caller may provide its bundle's path, where OS X
 *              can localize for GUI display the CFString specified by <code>@link kIOPMAssertionHumanReadableReasonKey@/link</code>.
 *
 *          <li> OPTIONAL: <code>kIOPMAssertionPlugInIDKey</code> if the caller is a plugin with a different identity than the process
 *              it's loaded in.
 *
 *          <li> OPTIONAL: <code>kIOPMAssertionFrameworkIDKey</code> if the caller is a framework acting on behalf of a process.
 *
 *          <li> OPTIONAL: <code>kIOPMAssertionTimeoutKey</code> The caller may specify a timeout.
 *      </ul>
 */
IOReturn IOPMAssertionCreateWithProperties(
                             CFDictionaryRef      AssertionProperties,
                             IOPMAssertionID      *AssertionID)
                            __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_3_2);

/*!
 * @function            IOPMAssertionDeclareUserActivity
 *
 * @abstract            Declares that the user is active on the system.
 *
 * @discussion          This causes the display to power on and postpone display sleep,
 *                      up to the user's display sleep Energy Saver settings.
 *
 *                      If you need to hold the display awake for a longer period and you know
 *                      how long you'd like to hold it, consider taking assertion
 *                      <code>@link kIOPMAssertPreventUserIdleDisplaySleep @/link</code> using
 *                      <code>@link IOPMAssertionCreateWithDescription @/link</code> API instead.
 *
 *                      No special privileges are necessary to make this call - any process may
 *                      call this API. Caller must specify an AssertionName - NULL is not
 *                      a valid input.
 *
 * @param AssertionName     A string that describes the name of the caller and the activity being
 *                          handled by this assertion (e.g. "Mail Compacting Mailboxes"). Name may be no longer
 *                          than 128 characters.
 *
 * @param   userType        This parameter specifies if the active user is located locally in front of the
 *                          system or connected to the system over the network. Various components of the system
 *                          are maintained at different power levels depending on user location.
 *
 * @param AssertionID       On Success, unique id will be returned in this parameter. Caller
 *                          may call this function again with the unique id retured previously to report continous
 *                          user activity. The unique id returned by this function may change on each call depending
 *                          on how frequently this function call is repeated and the current display sleep timer value.
 *                          If you make this call more than once, track the returned value for
 *                          assertionID, and pass it in as an argument on each call.
 *
 * @result                  Returns kIOReturnSuccess on success, any other return indicates
 *                          PM could not successfully activate the specified assertion.
 */
IOReturn IOPMAssertionDeclareUserActivity(
                                          CFStringRef          AssertionName,
                                          IOPMUserActiveType   userType,
                                          IOPMAssertionID      *AssertionID)
AVAILABLE_MAC_OS_X_VERSION_10_7_AND_LATER;   /* This API is introduced in 10.7.3 */
    

/*!
 * @function            IOPMDeclareNetworkClientActivity
 *
 * @abstract            A convenience function for handling remote network clients; this is a wrapper for
 *                      holding <code>@link kIOPMAssertNetworkClientActive @/link </code>
 *
 * @discussion          Call this whenever you detect activity from your remote network clients.
 *                      This call generates an IPC call, and may block.
 *
 *                      On the first invocation, this will populate parameter
 *                      <code>AssertionID</code> with a new assertion ID.
 *                      You should pass in this returned assertion ID on every access.
 *                      
 *                      When system is on AC power, every call to <code>IOPMDeclareNetworkClientActivity</code> 
 *                      prevents system from idle sleeping and from demand sleeping for the duration of
 *                      system sleep timer. When system is on Battery power, every call to 
 *                      <code>IOPMDeclareNetworkClientActivity</code> prevents system from idle sleeping for the 
 *                      duration of system sleep timer.
 *
 *                      Assertion created by this interface is valid only for the duration of system sleep timer
 *                      from the last call. IOKit will disable <code>AssertionID</code> after that duration.
 *
 *                      If you detect that your remote client is no longer active, please immediately call
 *                      <code>@link IOPMAssertionRelease@/link</code. Do not wait for the timeout.
 *
 *                      If your process can detect when remote clients are active and idle, you can skip
 *                      this API and directly create <code>@link kIOPMAssertNetworkClientActive @/link</code> yourself.
 *
 *                      If your remote clients require access to the framebuffer or the GPU, then this
 *                      isn't the appropriate call for you. Please see 
 *                      <code>@link IOPMAssertionDeclareUserActivity @/link</code> and pass in argument
 *                      <code>@link kIOPMUserActiveRemote @/link</code>.
 *
 * @param AssertionName     A string that describes the name of the caller and the activity being
 *                          handled by this assertion (e.g. "Serving a podcast"). The name must be less than
 *                          128 characters.
 * *
 * @param AssertionID       On Success, unique id will be returned in this parameter. Caller
 *                          may call this function again with the unique id retured previously to report additional
 *                          user activity. The unique id returned by this function may change on each call depending
 *                          on how frequently this function call is repeated and the current system sleep timer value.
 *                          If you make this call more than once, track the returned value for
 *                          assertionID, and pass it in as an argument on each call.
 *
 * @result                  Returns kIOReturnSuccess on success, any other return indicates
 *                          PM could not successfully activate the specified assertion.
 */
IOReturn IOPMDeclareNetworkClientActivity(
                                          CFStringRef          AssertionName,
                                          IOPMAssertionID      *AssertionID)
    __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

    
/*!
 * @function            IOPMAssertionRetain
 *
 * @abstract            Increments the assertion's retain count.
 * @discussion          Increments the retain count according to CoreFoundation style retain/release semantics.
 *                      Retain count can be inspected in the assertion's info dictionary at
 *                      key <code>@link kIOPMAssertionRetainCountKey@/link</code>
 * @param theAssertion  The assertion ID to retain.
 */    
void IOPMAssertionRetain(IOPMAssertionID theAssertion)
                            __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_3_2);

/*!
 * @function            IOPMAssertionRelease
 *
 * @abstract            Decrements the assertion's retain count.
 *
 * @discussion          If the retain count becomes zero, then this also frees and deactivates
 *                      the assertion referred to by <code>assertionID</code>
 *
 *                      Calls to <code>@link IOPMAssertionCreate@/link</code> and <code>@link IOPMAssertionRetain@/link</code>
 *                      must each be paired with calls to IOPMAssertionRelease.
 *
 * @param               AssertionID The assertion_id, returned from IOPMAssertionCreate, to cancel.
 *
 * @result              Returns kIOReturnSuccess on success.
 */
IOReturn IOPMAssertionRelease(IOPMAssertionID AssertionID)
                            __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_3_2);
    

/*!
 * @function            IOPMAssertionCopyProperties
 * @abstract            Copies details about an <code>IOPMAssertion</code>.
 * @discussion          Returns a dictionary describing an IOPMAssertion's specifications and current state.
 * @param theAssertion  The assertion ID to copy info about.
 * @result              A dictionary describing the assertion with keys specified in See @link IOPMAssertionDictionaryKeys@/link. 
 *                      It's the caller's responsibility to release this dictionary.
 */
CFDictionaryRef IOPMAssertionCopyProperties(IOPMAssertionID theAssertion)
                            __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_3_2);

/*!
 * @function            IOPMAssertionSetProperty
 * @abstract            Sets a property in the assertion.
 * @discussion          Only the process that created an assertion may change its properties.
 * @param theAssertion  The <code>@link IOPMAssertionID@/link</code> of the assertion to modify.
 * @param theProperty   The CFString key, from <code>@link IOPMAssertionDictionaryKeys@/link</code> to modify.
 * @param theValue      The property to set. The value must match the CF type expected for the specified key.
 * @result              Returns <code>@link kIOReturnNotPriviliged@/link</code> if the caller doesn't 
 *                          have permission to modify this assertion.
 *                      Returns <code>@link kIOReturnNotFound@/link</code> if PM can't locate this assertion.
 *                      Returns <code>@link kIOReturnError@/link</code> upon an unidentified error.
 *                      Returns <code>@link kIOReturnSuccess@/link</code> otherwise.
 */
IOReturn IOPMAssertionSetProperty(IOPMAssertionID theAssertion, CFStringRef theProperty, CFTypeRef theValue)
                            __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_3_2);

/*!
 * @function            IOPMCopyAssertionsByProcess
 *
 * @abstract            Returns a dictionary listing all assertions, grouped by their owning process.
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
IOReturn IOPMCopyAssertionsByProcess(CFDictionaryRef *AssertionsByPID)
                            __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_3_2);
    
/*!
 * @function            IOPMCopyAssertionsStatus
 *
 * @abstract            Returns a list of available assertions and their system-wide levels.
 *
 * @discussion          The system-wide level is the maximum of all individual assertions' levels.
 *
 * @param AssertionsStatus On success, this returns a CFDictionary of all assertions currently available.
 *                      The keys in the dictionary are the assertion types, and the value of each is a CFNumber that
 *                      represents the aggregate level for that assertion.  Caller must CFRelease() this dictionary when done.
 *
 * @result Returns      kIOReturnSuccess on success.
 */
IOReturn IOPMCopyAssertionsStatus(CFDictionaryRef *AssertionsStatus)
                            __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_3_2);

    
    

/*!
 * @function            IOPMAssertionCreate
 *
 * @abstract            This is a deprecated call to create a power assertion.
 *
 * @deprecated          IOPMAssertionCreate is deprecated in favor of <code>@link IOPMAssertionCreateWithProperties@/link</code>.
 *                      Please use that version of this API instead.
 *
 * @discussion          No special privileges necessary to make this call - any process may
 *                      activate a power assertion.
 *
 * @param AssertionType     The CFString assertion type to request from the PM system.
 * @param AssertionLevel    Pass kIOPMAssertionLevelOn or kIOPMAssertionLevelOff.
 * @param AssertionID       On success, a unique id will be returned in this parameter.
 *
 * @result              Returns kIOReturnSuccess on success, any other return indicates
 *                          PM could not successfully activate the specified assertion.
 */
IOReturn IOPMAssertionCreate(CFStringRef        AssertionType, 
                           IOPMAssertionLevel   AssertionLevel,
                           IOPMAssertionID      *AssertionID)
                            __OSX_AVAILABLE_BUT_DEPRECATED
                            (__MAC_10_5,__MAC_10_6,__IPHONE_2_0, __IPHONE_2_1);

/*!
 * @function            IOPMAssertionCreateWithName
 *
 * @abstract            The simplest API to create a power assertion.
 *
 * @discussion          No special privileges are necessary to make this call - any process may
 *                      activate a power assertion. Caller must specify an AssertionName - NULL is not
 *                      a valid input.
 *
 * @param AssertionType     The CFString assertion type to request from the PM system.
 * @param AssertionLevel    Pass kIOPMAssertionLevelOn or kIOPMAssertionLevelOff.
 * @param AssertionName     A string that describes the name of the caller and the activity being
 *                          handled by this assertion (e.g. "Mail Compacting Mailboxes"). Name may be no longer 
 *                          than 128 characters.
 *
 * @param AssertionID       On success, a unique id will be returned in this parameter.
 *
 * @result                  Returns kIOReturnSuccess on success, any other return indicates
 *                          PM could not successfully activate the specified assertion.
 */
IOReturn IOPMAssertionCreateWithName(
                        CFStringRef          AssertionType, 
                        IOPMAssertionLevel   AssertionLevel,
                        CFStringRef          AssertionName,
                        IOPMAssertionID      *AssertionID)
                        AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER;                           
                           
/*!
 * @define          kIOPMAssertionTimeoutKey
 * @abstract        kIOPMAssertionTimeoutKey specifies an outer bound, in seconds, that this assertion should be asserted.
 *
 * @discussion      If your application hangs, or is unable to complete its assertion task in a reasonable amount of time,
 *                  specifying a timeout allows PM to disable your assertion so the system can resume normal activity.
 *                  Once a timeout with the <code>@link kIOPMAssertionTimeoutActionTurnOff@/link</code> assertion fires, the level
 *                  will be set to <code>@link kIOPMAssertionTimeoutActionTurnOff@/link</code>. The assertion may be re-armed by calling
 *                  <code>@link IOPMAssertionSetProperty@/link</code> and setting a new value for for 
 *                  <code>kIOPMAssertionTimeoutKey</code>.
 *
 *                  This key may be specified in the dictionary passed to <code>@link IOPMAssertionCreateWithProperties@/link</code>.
 *
 *                  This key may be present in the dictionary returned from <code>@link IOPMAssertionCopyProperties@/link</code>.
 */
#define kIOPMAssertionTimeoutKey                CFSTR("TimeoutSeconds")


/*!
 * @define          kIOPMAssertionTimeoutActionKey
 *
 * @abstract        Specifies the action to take upon timeout expiration.
 *
 * @discussion      Specifying the timeout action only has meaning if you also specify an <code>@link kIOPMAssertionTimeoutKey@/link</code>
 *                  If the caller does not specify a timeout action, the default action is <code>@link kIOPMAssertionTimeoutActionTurnOff@/link</code>
 *
 *                  This key may be specified in the dictionary passed to <code>@link IOPMAssertionCreateWithProperties@/link</code>.
 *
 *                  This key may be present in the dictionary returned from <code>@link IOPMAssertionCopyProperties@/link</code>.
 */
#define kIOPMAssertionTimeoutActionKey          CFSTR("TimeoutAction")

/*!
 * @define          kIOPMAssertionTimeoutActionLog
 *
 * @abstract        A potential value for <code>@link kIOPMAssertionTimeoutActionKey@/link</code>
 *
 * @discussion      When this timeout action is specified, PM will log the timeout event
 *                  but will not turn off or affect the setting of the assertion in any way.
 *
 */
#define kIOPMAssertionTimeoutActionLog          CFSTR("TimeoutActionLog")

/*!
 * @define          kIOPMAssertionTimeoutActionTurnOff
 *
 * @discussion      When a timeout expires with this action, Power Management will log the timeout event,
 *                  and will set the assertion's level to <code>@link kIOPMAssertionLevelOff@/link</code>.
 */
#define kIOPMAssertionTimeoutActionTurnOff      CFSTR("TimeoutActionTurnOff")

/*!
 * @define          kIOPMAssertionTimeoutActionRelease
 *
 * @discussion      When a timeout expires with this action, Power Management will log the timeout event,
 *                  and will release the assertion.
 */
#define kIOPMAssertionTimeoutActionRelease      CFSTR("TimeoutActionRelease")

/*!
 * @define          kIOPMAssertionRetainCountKey
 *
 * @discussion      kIOPMAssertionRetainCountKey reflects the CoreFoundation-style retain count on this assertion.
 *                  Creating or retaining an assertion increments its retain count.
 *                  Release an assertion decrements its retain count.
 *                  When the retain count decrements to zero, the OS will destroy the object.
 *
 *                  This key can be found in the dictionary returned from <code>@link IOPMAssertionCopyProperties@/link</code>.
 */
#define kIOPMAssertionRetainCountKey            CFSTR("RetainCount")

/*!
 * @define          kIOPMAssertionNameKey
 *
 * @abstract        The CFDictionary key for assertion name. Setting this key is required when you're creating an assertion.
 *
 * @discussion      <code>kIOPMAssertionNameKey</code> describes the the activity the assertion is protecting. The creator should
 *                  specify a CFString value for this key in the dictionary passed to <code>@link IOPMAssertionCreateWithProperties@/link</code>
 *
 *                  The assertion name is separate from the assertion type's behavior - specify a CFString
 *                  like "Checking mail" or "Compiling" that describes the task that this assertion protects.
 *
 *                  The CFString you associate with this key does not have to be localizable (OS X will not attempt to localize it.)
 *
 *                  Describe your assertion as thoroughly as possible. See these other keys that can you can also set to add explanation
 *                  to an assertion:
 *                  <ul>
 *                      <li> OPTIONAL <code>@link kIOPMAssertionDetailsKey@/link</code>
 *                      <li> OPTIONAL <code>@link kIOPMAssertionHumanReadableReasonKey @/link</code>
 *                      <li> OPTIONAL <code>@link kIOPMAssertionLocalizationBundlePathKey@/link</code>
 *                  </ul>
 */
#define kIOPMAssertionNameKey                  CFSTR("AssertName")

/*!
 * @define          kIOPMAssertionDetailsKey
 *
 * @abstract        You may provide extra, contextual information about an assertion for admins and for debugging
 *                  in this key. Setting this key in an assertion dictionary is optional.
 *
 * @discussion      Please name your assertion something unique with <code>@link kIOPMAssertionNameKey@/link</code> first.
 *                  If you have more data to describe this assertion, put it here as a CFString.
 *
 *  '               EXAMPLE: OS X creates an assertion named <code>com.apple.powermanagement.tty</code> to prevent sleep for
 *                  remote-logged in users. To identify the cause for these assertions, OS X sets <code>kIOPMAssertionDetailsKey</code>
 *                  to the CFString device path of the active remote session(s), e.g. "/dev/ttys000 /dev/ttys004"
 *
 *                  The CFString you associate with this key does not have to be localizable (OS X will not attempt to localize it.)
 *
 *                  Describe your assertion as thoroughly as possible. See these other keys that can you can set to add explanation
 *                  to an assertion:
 *                  <ul>
 *                      <li>REQUIRED <code>@link kIOPMAssertionNameKey@/link</code>
 *                      <li>OPTIONAL <code>@link kIOPMAssertionHumanReadableReasonKey @/link</code>
 *                      <li>OPTIONAL <code>@link kIOPMAssertionLocalizationBundlePathKey@/link</code>
 *                  </ul>
 */
#define kIOPMAssertionDetailsKey             CFSTR("Details")

/*!
 * @define          kIOPMAssertionHumanReadableReasonKey
 *
 * @abstract        Optional key that provides a localizable string for OS X to display PM Assertions in the GUI.
 *
 * @discussion      The caller should specify this string in <code>@link IOPMAssertionCreateWithProperties@/link</code>.
 *                  If present, OS X may display this string, localized to the user's language, to explain changes in system
 *                  behavior caused by the assertion.
 *
 *                  If set, the caller must also specify a bundle path for the key
 *                      <code>@link kIOPMAssertionLocalizationBundlePathKey@/link</code>
 *                  The bundle at that path should contain localization info for the specified string.
 *
 *                  This key may be specified in the dictionary passed to <code>@link IOPMAssertionCreateWithProperties@/link</code>.
 *
 *                  This key may be present in the dictionary returned from <code>@link IOPMAssertionCopyProperties@/link</code>.
 *
 *                  Describe your assertion as thoroughly as possible. See these other keys that can you can set to add explanation
 *                  to an assertion:
 *                  <ul>
 *                      <li>REQUIRED <code>@link kIOPMAssertionNameKey@/link</code>
 *                      <li>OPTIONAL <code>@link kIOPMAssertionDetailsKey @/link</code>
 *                  </ul>
 */
#define kIOPMAssertionHumanReadableReasonKey    CFSTR("HumanReadableReason")

/*!
 * @define          kIOPMAssertionLocalizationBundlePathKey
 *
 * @abstract        Refers to a CFURL, as a CFString, identifying the path to the caller's
 *                  bundle, which contains localization info.
 *
 * @discussion      The bundle must contain localizations for
 *                  <code>@link kIOPMAssertionHumanReadableReasonKey@/link</code>
 *
 *                  This key may be specified in the dictionary passed to <code>@link IOPMAssertionCreateWithProperties@/link</code>.
 *
 *                  This key may be present in the dictionary returned from <code>@link IOPMAssertionCopyProperties@/link</code>.
 */
#define kIOPMAssertionLocalizationBundlePathKey   CFSTR("BundlePath")

/*!
 * @define          kIOPMAssertionFrameworkIDKey
 *
 * @abstract        Specify if the assertion creator is a framework.
 *
 * @discussion      If the code that creates the assertion resides in a framework or library, the caller
 *                  should specify a CFBundleIdentifier, as a CFString, identifying that bundle here.
 *                  This info helps developers and administrators determine the source of an assertion.
 *
 *                  This key may be specified in the dictionary passed to <code>@link IOPMAssertionCreateWithProperties@/link</code>.
 *
 *                  This key may be present in the dictionary returned from <code>@link IOPMAssertionCopyProperties@/link</code>.
 */
#define kIOPMAssertionFrameworkIDKey            CFSTR("FrameworkBundleID")

/*!
 * @define          kIOPMAssertionPlugInIDKey
 *
 * @abstract        Specify if the assertion creator is a plugin.
 *
 * @discussion      If the code that creates the assertion resides in a plugin, the caller
 *                  should specify a CFBundleIdentifier, as a CFString, identifying the plugin's bundle here.
 *                  This info helps developers and administrators determine the source of an assertion.
 *
 *                  This key may be specified in the dictionary passed to <code>@link IOPMAssertionCreateWithProperties@/link</code>.
 *
 *                  This key may be present in the dictionary returned from <code>@link IOPMAssertionCopyProperties@/link</code>.
 */
#define kIOPMAssertionPlugInIDKey               CFSTR("PlugInBundleID")

/*!
 * @define          kIOPMAssertionTypeKey
 *
 * @abstract        The CFDictionary key for assertion type in an assertion info dictionary.
 *
 * @discussion      The value for this key will be a CFStringRef, with the value of the assertion
 *                  type specified at creation time.
 *                  Note that OS X may substitute a support assertion type string if the caller
 *                  specifies a deprecated assertion type; in that case the value for this key could
 *                  differ from the caller-provided assertion type.
 */
#define kIOPMAssertionTypeKey                   CFSTR("AssertType")

/*!
 * @define          kIOPMAssertionLevelKey
 *
 * @abstract        The CFDictionary key for assertion level in an assertion info dictionary.
 *
 * @discussion      The value for this key will be a CFNumber, kCFNumberIntType with value
 *                  <code>kIOPMAssertionLevelOff</code> or <code>kIOPMAssertionLevelOn</code>.
 *                  The level reflects the assertion's level set at creation, or adjusted via
 *                  <code>@link IOPMAssertionSetProperty@/link</code>
 */
#define kIOPMAssertionLevelKey                  CFSTR("AssertLevel")


/*!
 * @define          kIOPMAssertionTypePreventUserIdleSystemSleep
 * @abstract        This assertion type is identical to <code>@link kIOPMAssertPreventUserIdleSystemSleep @/link</code>
 *                  Please use that instead.
 */
#define kIOPMAssertionTypePreventUserIdleSystemSleep        kIOPMAssertPreventUserIdleSystemSleep
    
/*!
 * @define          kIOPMAssertionTypePreventUserIdleDisplaySleep
 * @abstract        This assertion type is identical to <code>@link kIOPMAssertPreventUserIdleDisplaySleep @/link</code>
 *                  Please use that instead.
 */
#define kIOPMAssertionTypePreventUserIdleDisplaySleep       kIOPMAssertPreventUserIdleDisplaySleep
    
/*!
 * @define          kIOPMAssertionTypePreventSystemSleep
 * @deprecated      Deprecated in 10.9. This assertion is not supported in any OS X releases.
 * @abstract        This assertion is deprecated. Do not use it.
 * @discussion      Please consider using either assertion type for system activities:
 *                      <ul>
 *                          <li><code>@link kIOPMAssertRemoteAccess @/link</code></li>
 *                          <li><code>@link kIOPMAssertPreventUserIdleSystemSleep @/link</code></li>
 *                      </ul>
 */
#define kIOPMAssertionTypePreventSystemSleep                CFSTR("PreventSystemSleep")

/*!
 * @define          kIOPMAssertionTypeNoIdleSleep
 * @deprecated      Deprecated in 10.7.
 * @abstract        Please use assertion type <code>@link kIOPMAssertPreventUserIdleSystemSleep@/link</code> instead.
 */
#define kIOPMAssertionTypeNoIdleSleep                       CFSTR("NoIdleSleepAssertion")

/*!
 * @define          kIOPMAssertionTypeNoDisplaySleep
 * @deprecated      Deprecated in 10.7.
 * @abstract        Please use assertion type <code>@link kIOPMAssertPreventUserIdleDisplaySleep@/link</code> instead.
 */
#define kIOPMAssertionTypeNoDisplaySleep                    CFSTR("NoDisplaySleepAssertion")

    
#pragma mark IOSystemLoadAdvisory
/*!
 * @functiongroup IOSystemLoadAdvisory
 */

/*! @define kIOSystemLoadAdvisoryNotifyName
   @abstract The notification by this name fires when system "SystemLoadAdvisory" status changes.
   @discussion Pass this string as an argument to register via notify(3).
            You can query SystemLoadAdvisory state via notify_get_state() when this notification
            fires - this is more efficient than calling IOGetSystemLoadAdvisory(), and returns
            an identical combined SystemLoadAdvisory value.
 */
#define kIOSystemLoadAdvisoryNotifyName   "com.apple.system.powermanagement.SystemLoadAdvisory"

/*! @typedef IOSystemLoadAdvisoryLevel
   @abstract Return type for IOGetSystemLoadAdvisory
   @discussion Value is one of kIOSystemLoadAdvisoryLevelGreat, kIOSystemLoadAdvisoryLevelOK,
       or kIOSystemLoadAdvisoryLevelBad.
 */
typedef int IOSystemLoadAdvisoryLevel;

enum {
    kIOSystemLoadAdvisoryLevelBad       = 1,
    kIOSystemLoadAdvisoryLevelOK        = 2,
    kIOSystemLoadAdvisoryLevelGreat     = 3
};

/*! @define kIOSystemLoadAdvisoryUserLevelKey
   @abstract Key for dictionary returned by IOCopySystemLoadAdvisoryDetailed
   @discussion Indicates user activity constraints on the current SystemLoadAdvisory level.
 */
#define kIOSystemLoadAdvisoryUserLevelKey             CFSTR("UserLevel")

/*! @define kIOSystemLoadAdvisoryBatteryLevelKey
   @abstract Key for dictionary returned by IOCopySystemLoadAdvisoryDetailed
   @discussion Indicates battery constraints on the current SystemLoadAdvisory level.
 */
#define kIOSystemLoadAdvisoryBatteryLevelKey          CFSTR("BatteryLevel")

/*! @define kIOSystemLoadAdvisoryThermalLevelKey
   @abstract Key for dictionary returned by IOCopySystemLoadAdvisoryDetailed
   @discussion Indicates thermal constraints on the current SystemLoadAdvisory level.
 */
#define kIOSystemLoadAdvisoryThermalLevelKey          CFSTR("ThermalLevel")

/*! @define kIOSystemLoadAdvisoryCombinedLevelKey
   @abstract Key for dictionary returned by IOCopySystemLoadAdvisoryDetailed
   @discussion Provides a combined level based on UserLevel, BatteryLevel,
        and ThermalLevels; the combined level is the minimum of these levels. 
        In the future, this combined level may represent new levels as well.
        The combined level is identical to the value returned by IOGetSystemLoadAdvisory().
 */
#define kIOSystemLoadAdvisoryCombinedLevelKey         CFSTR("CombinedLevel")

/*! @function IOGetSystemLoadAdvisory
   @abstract Returns a hint about whether now would be a good time to perform time-insensitive 
        work.
   @discussion Based on user and system load, IOGetSystemLoadAdvisory determines "better" and "worse"
   times to run optional or time-insensitive CPU or disk work.
    
   Applications may use this result to avoid degrading the user experience. If it is a 
   "Bad" or "OK" time to perform work, applications should slow down and perform work
   less aggressively.
   
   There is no guarantee that the system will ever be in "Great" condition to perform work -
   all essential work must still be performed even in "Bad", or "OK" times. 
   Completely optional work, such as updating caches, may be postponed indefinitely.
   
   Note: You may more efficiently read the SystemLoadAdvisory level using notify_get_state() instead
        of IOGetSystemLoadAdvisory. The results are identical. notify_get_state() requires that you
        pass the token argument received by registering for SystemLoadAdvisory notifications.
   
   @return IOSystemLoadAdvisoryLevel - one of:
   <ul>
        <li>kIOSystemLoadAdvisoryLevelGreat - A Good time to perform time-insensitive work.
        <li>kIOSystemLoadAdvisoryLevelOK - An OK time to perform time-insensitive work.
        <li>kIOSystemLoadAdvisoryLevelBad - A Bad time to perform time-insensitive work.
    </ul>
 */
   
IOSystemLoadAdvisoryLevel IOGetSystemLoadAdvisory( void );

/*! @function IOCopySystemLoadAdvisoryDetailed
   @abstract Indicates how user activity, battery level, and thermal level each 
        contribute to the overall "SystemLoadAdvisory" level. In the future, 
        this combined level may represent new levels as well.
   @discussion See dictionary keys defined above.
   @return Returns a CFDictionaryRef, or NULL on error. Caller must release the
        returned dictionary.
 */
CFDictionaryRef IOCopySystemLoadAdvisoryDetailed(void);


/*!
 * @functiongroup CPU Power & Thermal
 */
/*! 
 * @define      kIOPMCPUPowerNotificationKey
 * @abstract    Key to register for BSD style notifications on CPU power or thermal change.
 */

#define kIOPMCPUPowerNotificationKey            "com.apple.system.power.CPU"
/*! 
 * @define      kIOPMThermalWarningNotificationKey
 * @abstract    Key to register for BSD style notifications on system thermal warnings.
 */
#define kIOPMThermalWarningNotificationKey      "com.apple.system.power.thermal_warning"
    
/*!
 * @function    IOPMCopyCPUPowerStatus
 * @abstract    Copy status of all current CPU power levels.
 * @discussion  The returned dictionary may define some of these keys, 
 *              as defined in IOPM.h:
 *              <ul>   
 *                  <li>kIOPMCPUPowerLimitProcessorSpeedKey
 *                  <li>kIOPMCPUPowerLimitProcessorCountKey
 *                  <li>kIOPMCPUPowerLimitSchedulerTimeKey        
 *              </ul>
 * @param       cpuPowerStatus Upon success, a pointer to a dictionary defining CPU power; 
 *              otherwise NULL. Pointer will be populated with a newly created dictionary 
 *              upon successful return. Caller must release dictionary.
 * @result      kIOReturnSuccess, or other error report. Returns kIOReturnNotFound if 
 *              CPU PowerStatus has not been published.
 */
IOReturn IOPMCopyCPUPowerStatus(CFDictionaryRef *cpuPowerStatus);
    
/*! 
 * @function    IOPMGetThermalWarningLevel
 * @abstract    Get thermal warning level of the system.
 * @result      An integer pointer declaring the power warning level of the system. 
 *              The value of the integer is one of (defined in IOPM.h):
 *              <ul>
 *                  <li>kIOPMThermalWarningLevelNormal
 *                  <li>kIOPMThermalWarningLevelDanger
 *                  <li>kIOPMThermalWarningLevelCrisis
 *              </ul>
 *              Upon success the thermal level value will be found at the 
 *              pointer argument.
 * @result      kIOReturnSuccess, or other error report. Returns kIOReturnNotFound if 
 *              thermal warning level has not been published.
 */
IOReturn IOPMGetThermalWarningLevel(uint32_t *thermalLevel);
    


#ifdef __cplusplus
}
#endif

#endif
