/*
 * Copyright (c) 2003, 2012 Apple Computer, Inc. All rights reserved.
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
#include <IOKit/pwr_mgt/IOPMLibPrivate.h>
#include <IOKit/IOCFSerialize.h>
#include <mach/mach_port.h>
#include <mach/vm_map.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include <notify.h>
#include <asl.h>

#include "IOSystemConfiguration.h"
#include "IOPowerSources.h"
#include "IOPowerSourcesPrivate.h"
#include "IOPSKeys.h"
#include "powermanagement.h"


#define kSmartBattRequestUpdateIndex        4
IOReturn IOPSRequestBatteryUpdate(int type)
{
    io_registry_entry_t     battery_reg = IO_OBJECT_NULL;
    io_connect_t            battery_connect = IO_OBJECT_NULL;
    uint64_t                utype = (uint64_t)type;
    IOReturn                ret = kIOReturnSuccess;
    
    battery_reg = IOServiceGetMatchingService(kIOMasterPortDefault,
                                              IOServiceMatching("AppleSmartBatteryManager"));
    if (IO_OBJECT_NULL != battery_reg)
    {
        ret = IOServiceOpen(battery_reg, mach_task_self(), 0, &battery_connect);
        if (kIOReturnSuccess == ret) {
            IOConnectCallMethod(battery_connect, kSmartBattRequestUpdateIndex, &utype, 1,
                                NULL, 0, NULL, NULL, NULL, NULL);
            IOServiceClose(battery_connect);
        }
        IOObjectRelease(battery_reg);
    } else {
        return kIOReturnNotFound;
    }
    return ret;
}


/* IOPSCopyInternalBatteriesArray
 *
 * Argument: 
 *	CFTypeRef power_sources: The return value from IOPSCoyPowerSourcesInfo()
 * Return value:
 * 	CFArrayRef: all the batteries we found
 *			NULL if none are found
 */
CFArrayRef
IOPSCopyInternalBatteriesArray(CFTypeRef power_sources)
{
    CFArrayRef			array = IOPSCopyPowerSourcesList(power_sources);
    CFMutableArrayRef   ret_arr;
    CFTypeRef			name = NULL;
    CFDictionaryRef		ps;
    CFStringRef			transport_type;
    int				    i, count;

    if(array) {
        if(!isA_CFArray(array)) {
            CFRelease(array);
            return NULL;
        }
    }
    count = CFArrayGetCount(array);
    name = NULL;

    ret_arr = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if(!ret_arr) goto exit;

    for(i=0; i<count; i++) {
        name = CFArrayGetValueAtIndex(array, i);
        ps = isA_CFDictionary(IOPSGetPowerSourceDescription(power_sources, name));
        if(ps) {
            transport_type = isA_CFString(CFDictionaryGetValue(ps, CFSTR(kIOPSTransportTypeKey)));
            if(transport_type && CFEqual(transport_type, CFSTR(kIOPSInternalType)))
            {
                CFArrayAppendValue(ret_arr, name);
            }
        }
    }

    if(0 == CFArrayGetCount(ret_arr)) {
        CFRelease(ret_arr);
        ret_arr = NULL;
    }

exit:
    if(array) CFRelease(array);
    return ret_arr;
}


/* IOPSCopyUPSArray
 *
 * Argument: 
 *	CFTypeRef power_sources: The return value from IOPSCoyPowerSourcesInfo()
 * Return value:
 * 	CFArrayRef: all the UPS's we found
 *			NULL if none are found
 */
 CFArrayRef
IOPSCopyUPSArray(CFTypeRef power_sources)
{
    CFArrayRef			array = IOPSCopyPowerSourcesList(power_sources);
    CFMutableArrayRef   ret_arr;
    CFTypeRef			name = NULL;
    CFDictionaryRef		ps;
    CFStringRef			transport_type;
    int				    i, count;

    if(array) {
        if(!isA_CFArray(array)) {
            CFRelease(array);
            return NULL;
        }
    }
    count = CFArrayGetCount(array);
    name = NULL;

    ret_arr = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if(!ret_arr) goto exit;

    // Iterate through power_sources
    for(i=0; i<count; i++) {
        name = CFArrayGetValueAtIndex(array, i);
        ps = isA_CFDictionary(IOPSGetPowerSourceDescription(power_sources, name));
        if(ps) {
            transport_type = isA_CFString(CFDictionaryGetValue(ps, CFSTR(kIOPSTransportTypeKey)));
            if(transport_type && ( CFEqual(transport_type, CFSTR(kIOPSSerialTransportType)) ||
                CFEqual(transport_type, CFSTR(kIOPSUSBTransportType)) ||
                CFEqual(transport_type, CFSTR(kIOPSNetworkTransportType)) ) )
            {
                CFArrayAppendValue(ret_arr, name);
            }
        }
    }

    if(0 == CFArrayGetCount(ret_arr)) {
        CFRelease(ret_arr);
        ret_arr = NULL;
    }

exit:
    if(array) CFRelease(array);
    return ret_arr;
}

/* IOPSGetActiveUPS
 *
 * Argument: 
 *	CFTypeRef power_sources: The return value from IOPSCoyPowerSourcesInfo()
 * Return value:
 * 	CFTypeRef (actually a CFStringRef): the UPS we're paying attention to, since we only
 *              support one UPS at a time.
 *			NULL if none are found
 */
CFTypeRef  
IOPSGetActiveUPS(CFTypeRef ps_blob)
{
    CFTypeRef       ret_ups;
    CFArrayRef      ups_arr;
    
    ups_arr = IOPSCopyUPSArray(ps_blob);
    if(!ups_arr)
    {
        return NULL;    
    }

    ret_ups = CFArrayGetValueAtIndex(ups_arr, 0);
    
    CFRelease(ups_arr);

    return ret_ups;
}

/* IOPSGetActiveBattery
 *
 * Argument: 
 *	CFTypeRef power_sources: The return value from IOPSCoyPowerSourcesInfo()
 * Return value:
 * 	CFTypeRef (actually a CFStringRef): the UPS we're paying attention to, since we only
 *              support one UPS at a time.
 *			NULL if none are found
 */
CFTypeRef  
IOPSGetActiveBattery(CFTypeRef ps_blob)
{
    CFTypeRef       ret_ups;
    CFArrayRef      ups_arr;
    
    ups_arr = IOPSCopyInternalBatteriesArray(ps_blob);
    if(!ups_arr)
    {
        return NULL;    
    }

    ret_ups = CFArrayGetValueAtIndex(ups_arr, 0);
    
    CFRelease(ups_arr);

    return ret_ups;
}

/***
 * int powerSourceSupported(CFStringRef)
 * takes: CFSTR of kIOPMACPowerKey, kIOPMBatteryPowerKey, kIOPMUPSPowerKey
 * returns true if this machine supports (has) that power type.
 */
CFBooleanRef IOPSPowerSourceSupported(CFTypeRef ps_blob, CFStringRef ps_type)
{

    if(!isA_CFString(ps_type)) 
    {
        return kCFBooleanFalse;
    } 
    
    if(CFEqual(ps_type, CFSTR(kIOPMACPowerKey))) 
    {
        return kCFBooleanTrue;
    }
    
#if defined (__i386__) || defined (__x86_64__) 
    if (CFEqual(ps_type, CFSTR(kIOPMBatteryPowerKey))) 
    {
        CFBooleanRef            ret = kCFBooleanFalse;
        io_registry_entry_t     platform = IO_OBJECT_NULL;
        CFDataRef               systemTypeData = NULL;
        int                     *systemType = 0;
        
        platform = IORegistryEntryFromPath(kIOMasterPortDefault, 
                                    kIODeviceTreePlane ":/");
        
        if (IO_OBJECT_NULL == platform) {
            return kCFBooleanFalse;
        }
        
        systemTypeData = (CFDataRef)IORegistryEntryCreateCFProperty(
                                platform, CFSTR("system-type"),
                                kCFAllocatorDefault, kNilOptions);
        if (systemTypeData 
            && (CFDataGetLength(systemTypeData) > 0)
            && (systemType = (int *)CFDataGetBytePtr(systemTypeData))
            && (2 == *systemType))
        {
            ret = kCFBooleanTrue;        
        } else {
            ret = kCFBooleanFalse;
        }
        if (systemTypeData)
            CFRelease(systemTypeData);
        IOObjectRelease(platform);
        return ret;
    }
#else
    if (ps_blob 
       && CFEqual(ps_type, CFSTR(kIOPMBatteryPowerKey))
       && IOPSGetActiveBattery(ps_blob))
    {
        return kCFBooleanTrue;
    }
#endif
    
    if (ps_blob 
       && CFEqual(ps_type, CFSTR(kIOPMUPSPowerKey))
       && IOPSGetActiveUPS(ps_blob))
    {
        return kCFBooleanTrue;
    }

    return kCFBooleanFalse;
}

/******************************************************************************

    IOPSPowerSourceID - publishing a power source from user space

 ******************************************************************************/

/* _pm_connect
 *  implemented in IOPMLibPrivate.c
 */
IOReturn _pm_connect(mach_port_t *newConnection);
IOReturn _pm_disconnect(mach_port_t connection);

dispatch_queue_t _getPSDispatchQueue(void)
{
    static dispatch_queue_t psQ = NULL;
    
    if (!psQ) {
        psQ = dispatch_queue_create("Power Management power source queue", NULL);
    }
    
    return psQ;
}

/* OpaqueIOPSPowerSourceID
 * As typecast in the header:
 * typedef struct OpaqueIOPSPowerSourceID *IOPSPowerSourceID;
 */
struct OpaqueIOPSPowerSourceID {
    CFDictionaryRef resyncCopy;
    int             psid;
    int             resyncToken;
};

#define kMaxPSTypeLength        25
#define kMaxSCDSKeyLength       1024

static IOReturn createPowerSource(struct OpaqueIOPSPowerSourceID *newPS)
{
    mach_port_t                 pm_server = MACH_PORT_NULL;
    IOReturn                    return_code = kIOReturnSuccess;
    kern_return_t               kr = KERN_SUCCESS;

    return_code = _pm_connect(&pm_server);
    if(kIOReturnSuccess != return_code) {
        return_code = kIOReturnNotOpen;
        goto fail;
    }
    
    kr = io_ps_new_pspowersource(
            pm_server, 
            &newPS->psid,   // out: integer psid
            &return_code);  // out: Return code

    if(KERN_SUCCESS != kr) {
        return_code = kIOReturnNotResponding;
    }

fail:
    if (IO_OBJECT_NULL != pm_server) {
        _pm_disconnect(pm_server);
    }

    return return_code;

}

static IOReturn setPowerSourceDetails(
    IOPSPowerSourceID whichPS,
    CFDictionaryRef details)
{
    IOReturn        ret = kIOReturnSuccess;
    CFDataRef       flatDetails;
    mach_port_t     pm_server = MACH_PORT_NULL;
    
    if (!whichPS || !isA_CFDictionary(details) || whichPS->psid == 0)
        return kIOReturnBadArgument;
    
    flatDetails = IOCFSerialize(details, 0);
    if (!flatDetails) {
        ret = kIOReturnBadArgument;
        goto exit;
    }
    
    ret = _pm_connect(&pm_server);
    if(kIOReturnSuccess != ret) {
        ret = kIOReturnNotOpen;
        goto exit;
    }
    
    // Pass the details off to powerd
    io_ps_update_pspowersource(pm_server,
                               whichPS->psid,
                               (vm_offset_t) CFDataGetBytePtr(flatDetails),
                               CFDataGetLength(flatDetails),
                               (int *)&ret);
    
    if (ret == kIOReturnSuccess) {
        CFMutableDictionaryRef  resyncCopy = NULL;
        resyncCopy = CFDictionaryCreateMutableCopy(NULL, 0, details);
        if (isA_CFDictionary(whichPS->resyncCopy))
            CFRelease(whichPS->resyncCopy);
        whichPS->resyncCopy = resyncCopy;
    }
    _pm_disconnect(pm_server);
    
exit:
    if (flatDetails)
        CFRelease(flatDetails);
    return ret;
}

IOReturn IOPSCreatePowerSource(
    IOPSPowerSourceID *outPS)
{
    __block IOReturn return_code = kIOReturnSuccess;
    
    dispatch_sync(_getPSDispatchQueue(), ^
    {
        IOPSPowerSourceID newPS = NULL;
        
        if (!outPS) {
            return_code = kIOReturnBadArgument;
            return;
        }
        
        // newPS - This tracking structure must be freed by IOPSReleasePowerSource()
        newPS = calloc(1, sizeof(struct OpaqueIOPSPowerSourceID));
        
        if (!newPS) {
            return_code = kIOReturnVMError;
            return;
        }
        
        return_code = createPowerSource(newPS);
        
        if (kIOReturnSuccess == return_code) {
            *outPS = newPS;
            notify_register_dispatch( kIOUserAssertionReSync,
                                     &newPS->resyncToken, _getPSDispatchQueue(),
                                     ^(int t __unused) {
                                         IOReturn ret;
                                         if ((ret = createPowerSource(newPS)) != kIOReturnSuccess) {
                                             asl_log(0,0,ASL_LEVEL_ERR, "createPowerSource returned 0x%x\n",ret);
                                         }
                                         if ((ret = setPowerSourceDetails(newPS, newPS->resyncCopy)) != kIOReturnSuccess) {
                                             asl_log(0,0,ASL_LEVEL_ERR, "setPowerSourceDetails returned 0x%x\n",ret);
                                         }
                                     });
        } else {
            *outPS = NULL;
            if (newPS) {
                free(newPS);
            }
        }
    });
    
    return return_code;
}

IOReturn IOPSSetPowerSourceDetails(
    IOPSPowerSourceID whichPS,
    CFDictionaryRef details)
{
    __block IOReturn ret = kIOReturnSuccess;
    
    dispatch_sync(_getPSDispatchQueue(), ^
    {
        ret = setPowerSourceDetails(whichPS, details);
    });

    return ret;
}

IOReturn IOPSReleasePowerSource(
    IOPSPowerSourceID whichPS)
{
    __block IOReturn ret = kIOReturnSuccess;
    
    dispatch_sync(_getPSDispatchQueue(), ^
    {
        mach_port_t pm_server = MACH_PORT_NULL;
        
        if (!whichPS) {
            ret = kIOReturnBadArgument;
            return;
        }
        
        if (kIOReturnSuccess == _pm_connect(&pm_server)) {
            // Pass the details off to powerd
            io_ps_release_pspowersource(pm_server,
                                        whichPS->psid);
            
            _pm_disconnect(pm_server);
        }
        
        notify_cancel(whichPS->resyncToken);
        if (isA_CFDictionary(whichPS->resyncCopy))
            CFRelease(whichPS->resyncCopy);
        
        whichPS->resyncCopy = NULL;
        whichPS->psid = 0;
        free(whichPS);
    });
    
    return ret;
}


IOReturn IOPSCopyChargeLog(CFAbsoluteTime sinceTime, CFDictionaryRef *chargeLog)
{

    IOReturn                rc = kIOReturnInternalError;
    CFDataRef               unfolder = NULL;
    vm_offset_t             logPtr = NULL;
    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           kern_result;
    mach_msg_type_number_t  logSize = 0;

    *chargeLog = NULL;
    _pm_connect(&pm_server);

    if(pm_server == MACH_PORT_NULL)
      return kIOReturnNotOpen;

    kern_result = io_ps_copy_chargelog(pm_server, sinceTime,
                                               &logPtr, &logSize, &rc);

    if ((KERN_SUCCESS != kern_result) || (rc != kIOReturnSuccess)) {
        goto exit;
    }

    if (logSize == 0) {
        rc = kIOReturnNotFound;
        goto exit;
    }

    unfolder = CFDataCreateWithBytesNoCopy(0, (const UInt8 *)logPtr, (CFIndex)logSize, kCFAllocatorNull);
    if (unfolder)
    {
        *chargeLog = CFPropertyListCreateWithData(0, unfolder, 
                                                   kCFPropertyListMutableContainersAndLeaves, 
                                                   NULL, NULL);
        CFRelease(unfolder);
    }


exit:

    if (logPtr && logSize)  {
        vm_deallocate(mach_task_self(), logPtr, logSize);
    }

    if (MACH_PORT_NULL != pm_server) 
        _pm_disconnect(pm_server);

    return rc;


}
