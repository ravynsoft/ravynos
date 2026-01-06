
/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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


#ifndef IOKitUser_IOPLATFORMSUPPORTPRIVATE_h
#define IOKitUser_IOPLATFORMSUPPORTPRIVATE_h

#include <TargetConditionals.h>

#if TARGET_OS_OSX
#include <IOKit/IOReturn.h>
#include <CoreFoundation/CoreFoundation.h>

/*! @define     kIOPlatformTCPKeepAliveDuringSleep
 *
 *              Pass this key to <code>IOPlatformCopyFeatureDefault</code>
 *              to determine whether this platform supports TCPKeepAliveDuringSleep.
 *
 *              Pass this key to <code>IOPlatformCopyFeatureActive</code> 
 *              to determine whether the current state of the system (Lid open/closed, 
 *              external displays, Do Not Disturb, Power Nap, TCP KeepAlive expiration, etc.)
 *              dictates that TCPKeepAlive should be on.
 */
#define kIOPlatformTCPKeepAliveDuringSleep      CFSTR("TCPKeepAliveDuringSleep")

/*! @define     kIOPlatformDeviceEnclosureColorKey
 *              Pass this key to <code>IOPlatformGetDeviceColor</code>
 *              to request the device enclosure color.
 */
#define kIOPlatformDeviceEnclosureColorKey      CFSTR("DeviceEnclosureColor")

/*!
 * @function    IOPlatformCopyFeatureDefault
 * @abstract    Indicates whether a feature is supported, and what its default
 *              setting is.
 * @discussion  This is an SPI front end for IOKit platform drivers.
 *              This provides a conduit for settings specified by IOPPF (IOPlatformPluginFamily).
 *
 *              If the IOPPF driver hasn't matched & started yet, this function will block
 *              up to 10 seconds for it to do so. You should only anticipate this happening
 *              at boot time.
 *
 * @param       platformSettingKey A CFStringRef describing a platform feature.
 * @param       outValue Upon success, this function will place a CF object at *outValue.
 *              IOPPF defines the type and value of *outValue.
 *              It's the caller's responsibility to confirm the object's CF type before dereferencing it.
 *              It's the caller's responsibility to CFRelease(outValue)
 *
 * @result      kIOReturnSuccess on success.
 *              kIOReturnNotReady if the IOPPF hasn't matched and started yet (and we waited 10s for it to happen).
 *              kIOReturnUnsupported if the IOPPF doesn't support this feature.
 */
IOReturn IOPlatformCopyFeatureDefault(
                                      CFStringRef   platformSettingKey,
                                      CFTypeRef     *outValue);

/*!
 * @function    IOPlatformCopyFeatureActive
 * @abstract    Indicates a feature's current value.
 *
 * @discussion  Some platform features can dynamically change value. This function encapsulates 
 *              the conditions that affect the feature's value.
 *
 * @param       platformSettingKey A CFStringRef describing a platform feature.
 * @param       outValue Upon success, this function will place a CF object at *outValue.
 *              IOPPF defines the type and value of *outValue.
 *              It's the caller's responsibility to confirm the object's CF type before dereferencing it.
 *              It's the caller's responsibility to CFRelease(outValue)
 *
 * @result      kIOReturnSuccess on success.
 *              kIOReturnNotReady if the IOPPF hasn't matched and started yet (and we waited 10s for it to happen).
 *              kIOReturnUnsupported if the IOPPF doesn't support this feature.
 */
IOReturn IOPlatformCopyFeatureActive(
                                      CFStringRef   platformSettingKey,
                                      CFTypeRef     *outValue);

/*!
 * @function    IOSMCKeyProxyPresent
 * @abstract    Indicates whether this system has SMC Key Proxy.
 * @discussion  Assumes that all systems have SMC Key Proxy except for
                those on a blacklist of older systems.
 * @result      true if system has SMC Key Proxy, false otherwise.
 */
Boolean IOSMCKeyProxyPresent(void);

/*!
 * @function    IONoteToSelfSupported
 * @abstract    Indicates whether Note To Self is supported on this system.
 * @discussion  Assumes that all systems support Note To Self except for
                those on a blacklist of non-capable systems.
 * @result      true if Note To Self can be used, false otherwise.
 */
Boolean IONoteToSelfSupported(void);

/*!
 * @function    IOAuthenticatedRestartSupported
 * @abstract    Indicates whether Authenticated Restart is supported on this system.
 * @discussion  Call this function before using Authenticated Restart. 
                Assumes that Authenticated Restart can be used only if the system
                has SMC Key Proxy or supports Note To Self.
 * @result      true if Authenticated Restart can be used, false otherwise.
 */
Boolean IOAuthenticatedRestartSupported(void);

/*!
 * @function    IOPlatformGetDeviceColor
 * @abstract    Get the color for an area of a device.
 * @discussion  This function retrieves the requested device color from a
 *              platform specific source and reports the color's RGB values.
 * @param       whichColor A CFStringRef indicating the requested color.
 * @param       red Return the value of the red color component.
 * @param       green Return the value of the green color component.
 * @param       blue Return the value of the blue color component.
 * @result      kIOReturnSuccess on success.
 *              kIOReturnNotFound if the requested color was not found on the platform.
 *              kIOReturnBadArgument if the function exited due to an invalid argument.
 */
IOReturn IOPlatformGetDeviceColor(  CFStringRef whichColor,
                                    uint8_t * red, uint8_t * green, uint8_t * blue );

/*!
 * @function    IOCopyModel
 * @abstract    Copy the model name, major and minor revision numbers
 * @discussion  This function returns the model name of the sysem.
 * @param       char ** model - Allocates and returns the model name.
 * @param       uint32_t *majorRev - Major revision number
 * @param       uint32_t *minorRev - Minor revision number
 * @result      kIOReturnSuccess on success. Caller is expected to release memory for *model.
 */
IOReturn  IOCopyModel(char** model, uint32_t *majorRev, uint32_t *minorRev);


#endif /* TARGET_OS_OSX */
#endif /* IOKitUser_IOPLATFORMSUPPORTPRIVATE_h */
