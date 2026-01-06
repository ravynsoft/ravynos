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

#include <TargetConditionals.h>

#if TARGET_OS_OSX

#include "IOPlatformSupportPrivate.h"
#include <IOKit/IOKitLib.h>
#include <IOKit/pwr_mgt/IOPMLibPrivate.h>
#include <sys/sysctl.h>
#include <sys/syslog.h>
#include <notify.h>

// find what files to include
#if 0
#define DLOG(format)                    syslog(LOG_DEBUG, "IONTSLib: " format)
#define DLOG1(format, args ...)         syslog(LOG_DEBUG, "IONTSLib: " format, args)
#define DLOG_ERROR(format)              syslog(LOG_ERR, "IONTSLib: " format)
#define DLOG_ERROR1(format, args ...)   syslog(LOG_ERR, "IONTSLib: " format, args)
#else
#define DLOG(format)
#define DLOG1(format, args ...)
#define DLOG_ERROR(format)
#define DLOG_ERROR1(format, args ...)
#endif


/* IOCopyModel
 *
 * Argument:
 *	char** model: returns the model name
 *  uint32_t *majorRev: returns the major revision number
 *  uint32_t *minorRev: returns the minor revision number
 * Return value:
 * 	true on success
 * Note: In case of success, IOCopyModel allocates memory for *model,
 *       it is the caller's responsibility to free *model.
 * Note: model format is expected to be %s%u,%u
 *       If the revision number is partial, the value is assumed to be %u,0 or 0,0
 */

IOReturn IOCopyModel(char** model, uint32_t *majorRev, uint32_t *minorRev)
{
    int mib[2];
    char *machineModel = NULL; // must free
    char *revStr;
    int count;
    unsigned long modelLen;
    size_t length = 1024;
    IOReturn rc = kIOReturnError;
    
    if (!model || !majorRev || !minorRev) {
        DLOG_ERROR("IOCopyModel: Passing NULL arguments\n");
        rc =  kIOReturnBadArgument;
        goto exit;
    }

    machineModel = malloc(length);
    mib[0] = CTL_HW;
    mib[1] = HW_MODEL;
    if (sysctl(mib, 2, machineModel, &length, NULL, 0)) {
        DLOG_ERROR1("IOCopyModel: sysctl (error %d)\n", errno);
        goto exit;
    }
    
    modelLen = strcspn(machineModel, "0123456789");
    if (modelLen == 0) {
        DLOG_ERROR("IOCopyModel: Could not find machine model name\n");
        goto exit;
    }
    *model = strndup(machineModel, modelLen);
    if (*model == NULL) {
        DLOG_ERROR("IOCopyModel: Could not find machine model name\n");
        goto exit;
    }
    
    *majorRev = 0;
    *minorRev = 0;
    revStr = strpbrk(machineModel, "0123456789");
    if (!revStr) {
        DLOG_ERROR("IOCopyModel: Could not find machine version number, inferred value is 0,0\n");
        goto exit;
    }
    
    count = sscanf(revStr, "%d,%d", majorRev, minorRev);
    if (count < 2) {
        DLOG_ERROR("IOCopyModel: Could not find machine version number\n");
        if (count<1) {
            *majorRev = 0;
        }
        *minorRev = 0;
        goto exit;
    }
    rc = kIOReturnSuccess;

exit:
    if (machineModel) free(machineModel);
    if (rc != kIOReturnSuccess) {
        if(model && (*model)) {
            free (*model);
            *model = NULL;
        }
        *majorRev = 0;
        *minorRev = 0;
    }
    return rc;
} // IOCopyModel

/* IOSMCKeyProxyPresent
 *
 * Return value:
 * 	true if system has SMC Key Proxy
 */
Boolean IOSMCKeyProxyPresent()
{
    char* model = NULL; // must free
    uint32_t majorRev;
    uint32_t minorRev;
    Boolean success = false;
    
    IOReturn modelFound = IOCopyModel(&model, &majorRev, &minorRev);
    if (modelFound != kIOReturnSuccess) {
        DLOG_ERROR("IOSMCKeyProxySupported: Could not find machine model\n");
        return false;
    }
    if (!strncmp(model, "MacBookPro", 10) && (majorRev <=7))
        goto exit;
    if (!strncmp(model, "MacBookAir", 10) && (majorRev <=2))
        goto exit;
    if (!strncmp(model, "MacBook", 10))
        goto exit;
    if (!strncmp(model, "iMac", 10)       && (majorRev <=11))
        goto exit;
    if (!strncmp(model, "Macmini", 10)    && (majorRev <=3))
        goto exit;
    if (!strncmp(model, "MacPro", 10)     && (majorRev <=5))
        goto exit;
    if (!strncmp(model, "Xserve", 10))
        goto exit;
    
    success = true;
    DLOG("Machine appears to have SMC Key Proxy\n");
    
exit:
    if (model) free(model);
    return success;
} // IOSMCKeyProxyPresent

/* IONoteToSelfSupported
 *
 * Return value:
 * 	true if system supports Note To Self
 */
Boolean IONoteToSelfSupported()
{
    char* model = NULL; // must free
    uint32_t majorRev;
    uint32_t minorRev;
    Boolean success = false;
    
    IOReturn modelFound = IOCopyModel(&model, &majorRev, &minorRev);
    if (modelFound != kIOReturnSuccess) {
        DLOG_ERROR("IONoteToSelfSupported: Could not find machine model\n");
        return false;
    }
    if (!strncmp(model, "MacBookPro", 10) && (majorRev < 5 ||
                                              (majorRev == 5 && minorRev <= 2)))
        goto exit;
    if (!strncmp(model, "MacBookAir", 10) && (majorRev <=2))
        goto exit;
    if (!strncmp(model, "MacBook", 10)    && (majorRev <=5))
        goto exit;
    if (!strncmp(model, "iMac", 10)       && (majorRev <=9))
        goto exit;
    if (!strncmp(model, "Macmini", 10)    && (majorRev <=3))
        goto exit;
    if (!strncmp(model, "MacPro", 10))
        goto exit;
    if (!strncmp(model, "Xserve", 10))
        goto exit;
    
    success = true;
    DLOG("Machine appears to be capable of Note To Self\n");
    
exit:
    if (model) free(model);
    return success;
} // IONoteToSelfSupported

/* IOAuthenticatedRestartSupported
 *
 * Return value:
 * 	true if system supports Authenticated Restart, using Note To Self or SMC Key Proxy
 */
Boolean IOAuthenticatedRestartSupported()
{
    return IONoteToSelfSupported() || IOSMCKeyProxyPresent();
} // IOAuthenticatedRestartSupported

static CFTypeRef _copyRootDomainProperty(CFStringRef key)
{
    io_registry_entry_t     platformReg = IO_OBJECT_NULL;
    CFTypeRef               obj = NULL;

    platformReg = IORegistryEntryFromPath( kIOMasterPortDefault,
                                          kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");
    if (IO_OBJECT_NULL != platformReg)
    {
        obj = IORegistryEntryCreateCFProperty(
                               platformReg,
                               key,
                               kCFAllocatorDefault,
                               kNilOptions);
    }
    IOObjectRelease(platformReg);
    
    return obj;
}

/*
 *
 * Defaults
 *
 */
IOReturn IOPlatformCopyFeatureDefault(
    CFStringRef   platformSettingKey,
    CFTypeRef     *outValue)
{
    /* IOPPF should publish the platform defaults dictionary under IOPMrootDomain
     * under the key 'IOPlatformFeatureDefaults'
     */
    CFDictionaryRef         platformFeatures = NULL;
    CFTypeRef               returnObj = NULL;
    
    if (!platformSettingKey || !outValue) {
        return kIOReturnBadArgument;
    }

    platformFeatures = _copyRootDomainProperty(CFSTR("IOPlatformFeatureDefaults"));
    
    if (platformFeatures) {
        returnObj = CFDictionaryGetValue(platformFeatures, platformSettingKey);
        if (returnObj) {
            *outValue = CFRetain(returnObj);
        }
        CFRelease(platformFeatures);
    }
    
    if (*outValue) {
        return kIOReturnSuccess;
    } else {
        return kIOReturnUnsupported;
    }
}

/*
 *
 * Active
 *
 */
IOReturn IOPlatformCopyFeatureActive(
    CFStringRef   platformSettingKey,
    CFTypeRef     *outValue)
{
    
    if (!platformSettingKey || !outValue) {
        return kIOReturnBadArgument;
    }
    
    if (CFEqual(platformSettingKey, kIOPlatformTCPKeepAliveDuringSleep))
    {
        int keepAliveIsValid = 0;
        keepAliveIsValid = IOPMGetValueInt(kIOPMTCPKeepAliveIsActive);
        
        if (keepAliveIsValid) {
            *outValue = kCFBooleanTrue;
        } else {
            *outValue = kCFBooleanFalse;
        }
        return kIOReturnSuccess;
    } 
    else if (CFEqual(platformSettingKey, CFSTR(kIOPMWakeOnLANKey))) {
        if (IOPMGetValueInt(kIOPMWakeOnLanIsActive)) {
            *outValue = kCFBooleanTrue;
        } else {
            *outValue = kCFBooleanFalse;
        }
        return kIOReturnSuccess;
    } 
    else {
        return IOPlatformCopyFeatureDefault(platformSettingKey, outValue);
    }
}

// from syscfg_DClr_RGB
typedef union __attribute__((packed)) {
    uint32_t rgb;
    struct {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t reserved;
    } component_v1;
    struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t reserved;
    } component_v2;
} IOPlatformDeviceColorRGB;

// from syscfg_DClr
typedef struct __attribute__((packed)) {
    uint8_t minor_version;
    uint8_t major_version;
    uint8_t reserved_2;
    uint8_t reserved_3;
    IOPlatformDeviceColorRGB device_enclosure;
    IOPlatformDeviceColorRGB cover_glass;
    uint8_t reserved_C;
    uint8_t reserved_D;
    uint8_t reserved_E;
    uint8_t reserved_F;
} IOPlatformDeviceColors;

IOReturn IOPlatformGetDeviceColor(
    CFStringRef whichColor,
    uint8_t * red, uint8_t * green, uint8_t * blue )
{
    IOReturn ret = kIOReturnNotFound;

    if (!whichColor)
        return kIOReturnBadArgument;

    if (CFEqual(whichColor, kIOPlatformDeviceEnclosureColorKey))
    {
        io_registry_entry_t platform = IO_OBJECT_NULL;
        CFTypeRef           prop = NULL;

        platform = IORegistryEntryFromPath( kIOMasterPortDefault,
                                            kIODeviceTreePlane ":/efi/platform");
        if (IO_OBJECT_NULL != platform)
        {
            prop = IORegistryEntryCreateCFProperty(
                               platform,
                               CFSTR("device-colors"),
                               kCFAllocatorDefault,
                               kNilOptions);

            IOObjectRelease(platform);
        }

        if (prop && (CFGetTypeID(prop) == CFDataGetTypeID()))
        {
            CFDataRef data = (typeof(data)) prop;
            const IOPlatformDeviceColors * colors = (typeof(colors)) CFDataGetBytePtr(data);

            if ((CFDataGetLength(data) == sizeof(IOPlatformDeviceColors)) &&
                colors && (colors->major_version == 2))
            {
                if (red) *red = colors->device_enclosure.component_v2.red;
                if (green) *green = colors->device_enclosure.component_v2.green;
                if (blue) *blue = colors->device_enclosure.component_v2.blue;
                ret = kIOReturnSuccess;
            }
        }

        if (prop) CFRelease(prop);
    }

    return ret;
}

#endif /* TARGET_OS_OSX */
