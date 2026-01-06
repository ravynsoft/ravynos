/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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

#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/vm_map.h>
#include <servers/bootstrap.h>


#include "IOSystemConfiguration.h"
#include <CoreFoundation/CoreFoundation.h> 
#include <IOKit/IOKitLib.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/pwr_mgt/IOPMLibPrivate.h>
#include "powermanagement_mig.h"
#include "powermanagement.h"

__private_extern__ IOReturn _copyPMServerObject(int selector, int assertionID,
                                                CFTypeRef selectorData, CFTypeRef *objectOut);

/*
 * SCPreferences file format
 *     com.apple.AutoWake.xml
 *
 * - CFSTR(kIOPMRepeatingPowerOnKey)
 *      - CFSTR(kIOPMPowerEventTimeKey) = CFNumberRef (kCFNumberIntType)
 *      - CFSTR(kIOPMDaysOfWeekKey) = CFNumberRef (kCFNumberIntType)
 *      - CFSTR(kIOPMPowerEventTypeKey) = CFStringRef (kIOPMAutoSleep, kIOPMAutoShutdown, kIOPMAutoPowerOn, kIOPMAutoWake)
 * - CFSTR(kIOPMRepeatingPowerOffKey)
 *      - CFSTR(kIOPMPowerEventTimeKey) = CFNumberRef (kCFNumberIntType)
 *      - CFSTR(kIOPMDaysOfWeekKey) = CFNumberRef (kCFNumberIntType)
 *      - CFSTR(kIOPMPowerEventTypeKey) = CFStringRef (kIOPMAutoSleep, kIOPMAutoShutdown, kIOPMAutoPowerOn, kIOPMAutoWake)
 */
 
IOReturn _pm_connect(mach_port_t *newConnection);
IOReturn _pm_disconnect(mach_port_t connection);

IOReturn IOPMScheduleRepeatingPowerEvent(CFDictionaryRef events)
{
    IOReturn                    ret = kIOReturnError;
    CFDataRef                   flatPackage = NULL;
    kern_return_t               rc = KERN_SUCCESS;
    mach_port_t                 pm_server = MACH_PORT_NULL;
    
    // Validate our inputs
    if(!isA_CFDictionary(events)) return kIOReturnBadArgument;

    
    if(kIOReturnSuccess != _pm_connect(&pm_server)) {
        ret = kIOReturnInternalError;
        goto exit;
    }

    flatPackage = CFPropertyListCreateData(0, events,
                          kCFPropertyListBinaryFormat_v1_0, 0, NULL );

    if ( !flatPackage ) {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    rc = io_pm_schedule_repeat_event(pm_server, (vm_offset_t)CFDataGetBytePtr(flatPackage),
            CFDataGetLength(flatPackage), 1, &ret);

    if (rc != KERN_SUCCESS)
        ret = kIOReturnInternalError;
 
exit:

    if (MACH_PORT_NULL != pm_server) {
        _pm_disconnect(pm_server);
    }
    if(flatPackage) CFRelease(flatPackage);

    return ret;
}


CFDictionaryRef IOPMCopyRepeatingPowerEvents(void)
{
    CFMutableDictionaryRef      return_dict = NULL;

    _copyPMServerObject(kIOPMPowerEventsMIGCopyRepeatEvents, 0, NULL, (CFTypeRef *)&return_dict);
    return return_dict;
}

IOReturn IOPMCancelAllRepeatingPowerEvents(void)
{    
    IOReturn                    ret = kIOReturnError;
    kern_return_t               rc = KERN_SUCCESS;
    mach_port_t                 pm_server = MACH_PORT_NULL;
    
    if(kIOReturnSuccess != _pm_connect(&pm_server)) {
        ret = kIOReturnInternalError;
        goto exit;
    }

    rc = io_pm_cancel_repeat_events(pm_server, &ret);

    if (rc != KERN_SUCCESS)
        ret = kIOReturnInternalError;


    if (MACH_PORT_NULL != pm_server) {
        _pm_disconnect(pm_server);
    }

exit:
    return ret;
}
