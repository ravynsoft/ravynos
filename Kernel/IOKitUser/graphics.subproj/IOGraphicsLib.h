/*
 * Copyright (c) 1999-2000 Apple Computer, Inc. All rights reserved.
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

#ifndef _IOKIT_IOGRAPHICSLIB_H
#define _IOKIT_IOGRAPHICSLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <IOKit/IOKitLib.h>
#include <IOKit/graphics/IOFramebufferShared.h>
#include <IOKit/graphics/IOGraphicsInterface.h>

/*! @header IOGraphicsLib
IOGraphicsLib implements non-kernel task access to IOGraphics family object types - IOFramebuffer and IOAccelerator. These functions implement a graphics family specific API.<br>
A connection to a graphics IOService must be made before these functions are called. A connection is made with the IOServiceOpen() function described in IOKitLib.h. An io_connect_t handle is returned by IOServiceOpen(), which must be passed to the IOGraphicsLib functions. The appropriate connection type from IOGraphicsTypes.h must be specified in the call to IOServiceOpen(). All of the IOFramebuffer functions can only be called from a kIOFBServerConnectType connection. Except as specified below, functions whose names begin with IOFB are IOFramebuffer functions. Functions whose names begin with IOPS are IOAccelerator functions and must be called from connections of type kIOFBEngineControllerConnectType or kIOFBEngineConnectType.<br>
The functions in IOGraphicsLib use a number of special types. The display mode is the screen's resolution and refresh rate. The known display modes are referred to by an index of type IODisplayModeID. The display depth is the number of significant color bits used in representing each pixel. Depths are also referred to by an index value that is 0 for 8 bits, 1 for 15 bits, and 2 for 24 bits. A combination of display mode and depth may have a number of supported pixel formats. The pixel aperture is an index of supported pixel formats for a display mode and depth. This index is of type IOPixelAperture. All of these graphics specific types are defined in IOGraphicsTypes.h.
*/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

extern kern_return_t
IOFramebufferOpen(
        io_service_t    service,
        task_port_t     owningTask,
        unsigned int    type,
        io_connect_t  * connect );

/*! @enum IODisplayDictionaryOptions
    @constant kIODisplayMatchingInfo Include only the keys necessary to match two displays with IODisplayMatchDictionaries().
    @constant kIODisplayOnlyPreferredName The kDisplayProductName property includes only the localized names returned by CFBundleCopyPreferredLocalizationsFromArray().
    @constant kIODisplayNoProductName The kDisplayProductName property is not included in the returned dictionary.
*/
enum {
    kIODisplayMatchingInfo      = 0x00000100,
    kIODisplayOnlyPreferredName = 0x00000200,
    kIODisplayNoProductName     = 0x00000400
};

/*! @function IODisplayCreateInfoDictionary
    @abstract Create a CFDictionary with information about display hardware.
    @discussion The CFDictionary created by this function contains information about the display hardware associated with a framebuffer. The keys for the dictionary are listed in IOGraphicsTypes.h.
    @param framebuffer The IOService handle for an IOFramebuffer service.
    @param options Use IODisplayDictionaryOptions to specify which keys to include.
    @result The returned CFDictionary that should be released by the caller with CFRelease(). */

CFDictionaryRef
IODisplayCreateInfoDictionary(
        io_service_t            framebuffer,
        IOOptionBits            options );

/*! @defined IOCreateDisplayInfoDictionary
    @discussion IOCreateDisplayInfoDictionary() was renamed IODisplayCreateInfoDictionary(). IOCreateDisplayInfoDictionary() is now a macro for IODisplayCreateInfoDictionary() for compatibility with older code. */

#define IOCreateDisplayInfoDictionary(f,o)      \
        IODisplayCreateInfoDictionary(f,o)

/*! @function IODisplayMatchDictionaries
    @abstract Match two display information dictionaries to see if they are for the same display.
    @discussion By comparing two CFDictionaries returned from IODisplayCreateInfoDictionary(), this function determines if the displays are the same. The information compared is what is returned by calling IODisplayCreateInfoDictionary() with an option of kIODisplayMatchingInfo. This includes information such as the vendor, product, and serial number.
    @param matching1 A CFDictionary returned from IODisplayCreateInfoDictionary().
    @param matching2 Another CFDictionary returned from IODisplayCreateInfoDictionary().
    @param options No options are currently defined.
    @result Returns FALSE if the two displays are not equivalent or TRUE if they are. */

SInt32
IODisplayMatchDictionaries(
        CFDictionaryRef         matching1,
        CFDictionaryRef         matching2,
        IOOptionBits            options );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

io_service_t
IODisplayForFramebuffer(
        io_service_t    framebuffer,
        IOOptionBits    options );

IOReturn
IODisplaySetParameters(
        io_service_t    service,
        IOOptionBits    options,
        CFDictionaryRef params );

IOReturn
IODisplaySetFloatParameter(
        io_service_t    service,
        IOOptionBits    options,
        CFStringRef     parameterName,
        float           value );
IOReturn
IODisplaySetIntegerParameter(
        io_service_t    service,
        IOOptionBits    options,
        CFStringRef     parameterName,
        SInt32          value );

IOReturn
IODisplayCopyParameters(
        io_service_t      service,
        IOOptionBits      options,
        CFDictionaryRef * params );

IOReturn
IODisplayCopyFloatParameters(
        io_service_t      service,
        IOOptionBits      options,
        CFDictionaryRef * params );

IOReturn
IODisplayGetFloatParameter(
        io_service_t    service,
        IOOptionBits    options,
        CFStringRef     parameterName,
        float *         value );

IOReturn
IODisplayGetIntegerRangeParameter(
        io_service_t    service,
        IOOptionBits    options,
        CFStringRef     parameterName,
        SInt32 *        value,
        SInt32 *        min,
        SInt32 *        max );

IOReturn
IODisplayCommitParameters(
        io_service_t    service,
        IOOptionBits    options );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef __cplusplus
}
#endif

#endif /* ! _IOKIT_IOGRAPHICSLIB_H */

