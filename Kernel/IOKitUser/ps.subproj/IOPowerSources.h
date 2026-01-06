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
/*
 * HISTORY
 *
 */

#ifndef _IOKIT_IOPOWERSOURCES_H
#define _IOKIT_IOPOWERSOURCES_H

#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h> /* CFTimeInterval, CFArrayRef, CFDictionaryRef, CFRunLoopSourceRef */
__BEGIN_DECLS


/*! 
 * @header IOPowerSources.h
 *
 * @discussion  IOPowerSources provides uniform access to the state of power sources attached to the system.
 *              You can receive a change notification when any power source data changes.
 *              "Power sources" currently include batteries and UPS devices.
 *
 *              The header follows CF semantics in that it is the caller's responsibility to 
 *              CFRelease() anything returned by a "Copy" function, and the caller should not 
 *              CFRelease() anything returned by a "Get" function.
 */

/*!
 *  @functiongroup Low Power Warnings
 */

/*! @constant   kIOPSNotifyLowBattery
 *
 *  @abstract   Notify(3) key. IOKit posts kIOPSNotifyLowBattery notifications when the
 *              system is drawing from limited battery power, and the battery time 
 *              remaining drops into a warnable level.
 * 
 *              See also kIOPSNotifyPowerSource, and kIOPSNotifyTimeRemaining
 */
#define kIOPSNotifyLowBattery   "com.apple.system.powersources.lowbattery"

/*!
 * @enum       IOPSLowBatteryWarningLevel
 *
 * @discussion Possible return values from <code>@link IOPSGetBatteryWarningLevel@/link</code>
 */
typedef enum {
/*! @constant   kIOPSLowBatteryWarningNone
 *
 *  @abstract   The system is not in a low battery situation, or is on drawing from an external power source.
 *
 *  @discussion The system displays no low power warnings; neither should application clients of this
 *              API.
 */
    kIOPSLowBatteryWarningNone  = 1,

/*! @constant   kIOPSLowBatteryWarningEarly
 *
 *  @abstract   The battery can provide no more than 20 minutes of runtime.
 *
 *  @discussion OS X makes no guarantees that the system shall remain in Early Warning for 20 minutes.
 *              Batteries are frequently calibrated differently and may provide runtime
 *              for more, or less, than the estimated 20 minutes.
 *              OS X alerts the user by changing the color of BatteryMonitor to red.
 *              Warning the user is optional for full screen apps.
 */
    kIOPSLowBatteryWarningEarly = 2,

/*! @constant   kIOPSLowBatteryWarningFinal
 *
 *  @abstract   The battery can provide no more than 10 minutes of runtime.
 *
 *  @discussion OS X makes no guarantees that the system shall remain in Final Warning for 10 minutes.
 *              Batteries are frequently calibrated differently and may provide runtime 
 *              for more, or less, than the estimated 10 minutes.
 */
    kIOPSLowBatteryWarningFinal = 3
} IOPSLowBatteryWarningLevel;

/*! @function   IOPSGetBatteryWarningLevel
 *
 *  @abstract   Indicates whether the system is at a low battery warning level.
 *
 *  @discussion If your app runs in full screen mode and occludes OS X's battery monitor's low
 *              battery warnings, you should alert the user at least when the system 
 *              is in kIOPSLowBatteryWarnFinal.
 */
IOPSLowBatteryWarningLevel IOPSGetBatteryWarningLevel(void);

/*!
 *  @functiongroup Quick Power Source Info
 */

/*! 
 * @define      kIOPSNotifyTimeRemaining
 *              C-string key for a notification of changes to any power source's time remaining estimate.
 *              IOKit also posts this notification  when the active power source changes between AC, Battery, and UPS.
 *
 *              If you only need to detect when the power source changes between AC, Battery, or UPS, please use
 *              <code>@link kIOPSNotifyPowerSource @/link</code>; your code will run less often and conserve battery life.
 *
 *              See API <code>@link IOPSGetTimeRemainingEstimate @/link</code> to determine whether the active power source is
 *              limited or unlimited; and to determine the estimated time remaining until empty.
 *
 *              Use notify(3) API to register for notifications.
 */
#define kIOPSNotifyTimeRemaining                "com.apple.system.powersources.timeremaining"
#define kIOPSTimeRemainingNotificationKey       kIOPSNotifyTimeRemaining

/*!
 * @define      kIOPSNotifyPowerSource
 * @abstract    C-string key for a notification of changes to the active power source.
 * @discussion  Use this notification to discover when the active power source changes from AC power (unlimited/wall power),
 *              to Battery Power or UPS Power (limited power). IOKit will not deliver this notification when a battery's
 *              time remaining changes, only when the active power source changes. This makes it a more efficient
 *              choice for clients only interested in differentiating AC vs Battery.
 *
 *              See API <code>@link IOPSGetTimeRemainingEstimate @/link</code> to determine whether the active power source is
 *              limited or unlimited.
 *
 *              Example: IOKit posts kIOPSNotifyPowerSource upon connecting or disconnecting AC power to a laptop.
 *                       IOKit posts kIOPSNotifyPowerSource upon a UPS losing AC Power; as the system switches to a limited
 *                       UPS battery power source.
 *
 *              Use notify(3) API to register for notifications.
 * */
#define kIOPSNotifyPowerSource                  "com.apple.system.powersources.source"

/*!
 * @define      kIOPSNotifyAttach
 * @abstract    C-string key for a notification when a power source is attached or detached.
 * @discussion  Example: IOKit posts kIOPSNotifyAttach upon detection of internal battery.
 *                       IOKit posts kIOPSNotifyAttach when a user attaches or detaches an external UPS.
 *
 *              Note that IOKit doesn't deliver kIOPSNotifyAttach upon plugging or unplugging AC Power to a laptop; see
 *              <code>@link kIOPSNotifyPowerSource@/link</code> for changes to the active power source.
 *
 *              IOKit may take many seconds to discover a built-in battery at boot time. If your user process runs
 *              at early boot, use kIOPSNotifyAttach to detect an IOPowerSource's appearance.
 *              Use notify(3) API to register for notifications.
 */
#define kIOPSNotifyAttach                       "com.apple.system.powersources.attach"

/*!
 * @define      kIOPSNotifyAnyPowerSource
 * @abstract    C-string key for a notification that of changes to any attribute of any IOPowerSource.
 * @discussion  Use notify(3) API to register for notifications.
 *              IOKit posts this notification more frequently than the other notifications, and thus uses more
 *              energy to run your code. To conserve CPU cycles and battery life, please consider another notification 
 *              that also fits your needs. Please consider these instead:
 *                  <code>@link kIOPSNotifyPowerSource @/link</code>,
 *                  <code>@link kIOPSNotifyTimeRemaining @/link</code>
 *
 */
#define kIOPSNotifyAnyPowerSource               "com.apple.system.powersources"

/*!
 * @constant    kIOPSTimeRemainingUnknown
 *              Possible return value from <code>@link IOPSGetTimeRemainingEstimate@/link</code>
 *              Indicates the system is connected to a limited power source, but the system is still
 *              calculating a time remaining estimate. Check for a valid estimate again when IOKit posts the
 *              notification <code>@link kIOPSPowerSourcesNotificationKey@/link</code>.
 */
#define     kIOPSTimeRemainingUnknown           ((CFTimeInterval)-1.0)

/*!
 * @constant    kIOPSTimeRemainingUnlimited
 *              Possible return value from <code>@link IOPSGetTimeRemainingEstimate@/link</code>
 *              Indicates the system is connected to an external power source, without a time limit.
 */
#define     kIOPSTimeRemainingUnlimited         ((CFTimeInterval)-2.0)

 
/*!
 * @constant    kIOPMUPSPowerKey
 *              Possible return value from <code>@link IOPSGetProvidingPowerSourceType@/link</code>
 *              Indicates that the system is connected to an external power source, that identifies itself as
 *              as an UPS.
 */

#define kIOPMUPSPowerKey                                "UPS Power"

/*!
 * @constant    kIOPMBatteryPowerKey
 *              Possible return value from <code>@link IOPSGetProvidingPowerSourceType@/link</code>
 *              Indicates that the system is connected to internal battery power source.
 */
#define kIOPMBatteryPowerKey                            "Battery Power"

/*!
 * @constant    kIOPMACPowerKey
 *              Possible return value from <code>@link IOPSGetProvidingPowerSourceType@/link</code>
 *              Indicates that the system is connected to an external unlimited power source.
 */
#define kIOPMACPowerKey                                 "AC Power"


/*! 
 * @function    IOPSGetTimeRemainingEstimate
 *
 * @abstract    Returns the estimated minutes remaining until all power sources 
 *              (battery and/or UPS's) are empty, or returns <code>@link kIOPSTimeRemainingUnlimited@/link </code>
 *              if attached to an unlimited power source.
 *
 * @discussion  
 *              If attached to an "Unlimited" power source, like AC power or any external source, the 
 *              return value is <code>@link kIOPSTimeRemainingUnlimited@/link </code>
 *
 *              If the system is on "Limited" power, like a battery or UPS,
 *              but is still calculating the time remaining, which may 
 *              take several seconds after each system power event 
 *              (e.g. waking from sleep, or unplugging AC Power), the return value is 
 *              <code>@link kIOPSTimeRemainingUnknown@/link </code>
 *
 *              Otherwise, if the system is on "Limited" power and the system has an accurate time
 *              remaining estimate, the system returns a CFTimeInterval estimate of the time
 *              remaining until the system is out of battery power.
 *
 *              If you require more detailed battery information, use 
 *              <code>@link IOPSCopyPowerSourcesInfo @/link></code>
 *              and <code>@link IOPSGetPowerSourceDescription @/link></code>.
 *
 * @result      
 *              Returns <code>@link kIOPSTimeRemainingUnknown@/link</code> if the 
 *              OS cannot determine the time remaining. 
 *              
 *              Returns <code>@link kIOPSTimeRemainingUnlimited@/link</code> if the 
 *              system has an unlimited power source.
 *
 *              Otherwise returns a positive number of type CFTimeInterval, indicating the time
 *              remaining in seconds until all power sources are depleted.
 */
CFTimeInterval IOPSGetTimeRemainingEstimate(void);


/*! 
 *  @functiongroup Power Source Descriptions
 */

typedef void  (*IOPowerSourceCallbackType)(void *context);

/*! @function   IOPSCopyPowerSourcesInfo
 *
 *  @abstract   Returns a blob of Power Source information in an opaque CFTypeRef. 
 *
 *  @discussion Clients should not directly access data in the returned CFTypeRef - 
 *              they should use the accessor functions IOPSCopyPowerSourcesList and 
 *              IOPSGetPowerSourceDescription, instead.
 *
 *  @result     NULL if errors were encountered, a CFTypeRef otherwise.
 *              Caller must CFRelease() the return value when done accessing it.
 */
CFTypeRef IOPSCopyPowerSourcesInfo(void);

/*! @function   IOPSCopyPowerSourcesList
 *
 *  @abstract   Returns a CFArray of Power Source handles, each of type CFTypeRef.
 *
 *  @discussion The caller shouldn't directly access the CFTypeRefs, but should use
 *              IOPSGetPowerSourceDescription on each member of the CFArrayRef.
 *
 *  @param      blob Takes the CFTypeRef returned by IOPSCopyPowerSourcesInfo()
 *
 *  @result     Returns NULL if errors were encountered, otherwise a CFArray of CFTypeRefs.
 *              Caller must CFRelease() the returned CFArrayRef.
 */
CFArrayRef IOPSCopyPowerSourcesList(CFTypeRef blob);

/*! @function   IOPSGetPowerSourceDescription
 *  
 *  @abstract Returns a CFDictionary with readable information about the specific power source.
 *
 *  @discussion See the C-strings defined in IOPSKeys.h for specific keys into the dictionary.
 *              Don't expect all keys to be present in any dictionary. Some power sources, for example,
 *              may not support the "Time Remaining To Empty" key and it will not be present in their dictionaries.
 *
 *  @param      blob The CFTypeRef returned by IOPSCopyPowerSourcesInfo()
 *
 *  @param      ps One of the CFTypeRefs in the CFArray returned by IOPSCopyPowerSourcesList()
 *
 *  @result     Returns NULL if an error was encountered, otherwise a CFDictionary. Caller should 
 *              NOT release the returned CFDictionary - it will be released as part of the CFTypeRef returned by
 *              IOPSCopyPowerSourcesInfo().
 */
CFDictionaryRef IOPSGetPowerSourceDescription(CFTypeRef blob, CFTypeRef ps);

/*! @function    IOPSGetProvidingPowerSourceType
 *
 *  @abstract    Indicates the power source the computer is currently drawing from.
 *
 *  @discussion  Determines which power source is providing power.
 *
 *  @param       snapshot The CFTypeRef returned by IOPSCopyPowerSourcesInfo()
 *
 *  @result      One of: CFSTR(kIOPMACPowerKey), CFSTR(kIOPMBatteryPowerKey), CFSTR(kIOPMUPSPowerKey)
 */
CFStringRef     IOPSGetProvidingPowerSourceType(CFTypeRef snapshot);

/*! @function   IOPSNotificationCreateRunLoopSource
 *  
 *  @abstract   Returns a CFRunLoopSourceRef that notifies the caller when power source
 *              information changes.
 *
 *  @discussion Returns a CFRunLoopSourceRef for scheduling with your CFRunLoop. 
 *              If your project does not use a CFRunLoop, you can alternatively
 *              receive notifications via mach port, dispatch, or signal, via <code>notify.h</code>
 *              using the name <code>@link kIOPSTimeRemainingNotificationKey @/link</code>.
 *
 *              IOKit delivers this notification when percent remaining or time remaining changes.
 *              Thus it fires fairly frequently while discharging or charging the battery; 
 *              please consider using:
 *              <code>@link IOPSCreateLimitedPowerNotification @/link</code> if you only require
 *              notifications when the power source type changes from limited to unlimited.
 *
 *  @param      callback A function to be called whenever any power source is added, removed, or changes.
 *
 *  @param      context Any user-defined pointer, passed to the IOPowerSource callback.
 *
 *  @result     Returns NULL if an error was encountered, otherwise a CFRunLoopSource. Caller must
 *              release the CFRunLoopSource.
 */
CFRunLoopSourceRef IOPSNotificationCreateRunLoopSource(IOPowerSourceCallbackType callback, void *context);

/*! @function   IOPSCreateLimitedPowerNotification
 *
 *  @abstract   Returns a CFRunLoopSourceRef that notifies the caller when power source
 *              changes from an unlimited power source (like attached to wall, car, or airplane power), to a limited
 *              power source (like a battery or UPS).
 *
 *  @discussion Returns a CFRunLoopSourceRef for scheduling with your CFRunLoop.
 *              If your project does not use a CFRunLoop, you can alternatively
 *              receive this notification via <code>notify.h</code>
 *              using the name <code>@link kIOPSNotifyPowerSource @/link</code>
 *
 *  @param      callback A function to be called whenever the power source changes from AC to DC..
 *
 *  @param      context Any user-defined pointer, passed to the IOPowerSource callback.
 *
 *  @result     Returns NULL if an error was encountered, otherwise a CFRunLoopSource. Caller must
 *              release the CFRunLoopSource.
 */
CFRunLoopSourceRef IOPSCreateLimitedPowerNotification(IOPowerSourceCallbackType callback, void *context) __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0);

/*! @function   IOPSCopyExternalPowerAdapterDetails
 *
 *  @abstract   Returns a CFDictionary that describes the attached (AC) external
 *              power adapter (if any external power adapter is attached.
 *
 *  @discussion Use the kIOPSPowerAdapter... keys described in IOPSKeys.h
 *              to interpret the returned CFDictionary.
 *
 *  @result     Returns a CFDictionary on success. Caller must release the returned
 *              dictionary. If no adapter is attached, or if there's an error,  returns NULL.
 */
CFDictionaryRef IOPSCopyExternalPowerAdapterDetails(void);

__END_DECLS

#endif /* _IOKIT_IOPOWERSOURCES_H */
