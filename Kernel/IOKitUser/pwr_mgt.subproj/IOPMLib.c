/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#include <mach/mach.h>
#include <mach/mach_init.h>
#include <notify.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFSerialize.h>
#include <IOKit/pwr_mgt/IOPM.h>
#include "IOSystemConfiguration.h"
#include "IOPMLib.h"

#define arrayCnt(var) (sizeof(var) / sizeof(var[0]))

io_connect_t IOPMFindPowerManagement( mach_port_t master_device_port )
{
    io_connect_t    fb;
    kern_return_t    kr;
    io_service_t    obj = MACH_PORT_NULL;

    obj = IORegistryEntryFromPath( master_device_port, 
                        kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");
    if( obj ) {
        kr = IOServiceOpen( obj,mach_task_self(), 0, &fb);
        if ( kr == kIOReturnSuccess ) {
            IOObjectRelease(obj);
            return fb;
        }
        IOObjectRelease(obj);
    }
    return 0;
}


IOReturn IOPMGetAggressiveness ( 
    io_connect_t fb, 
    unsigned long type, 
    unsigned long * lAggressiveness )
{

    uint64_t inData = type;
    uint64_t aggressiveness = 0;
    uint32_t len = 1;
    kern_return_t err = IOConnectCallScalarMethod(fb, kPMGetAggressiveness,
                &inData, 1, &aggressiveness, &len);
    *lAggressiveness = aggressiveness;

    if (err)
        return kIOReturnError;
    else
        return err;
}


IOReturn IOPMSetAggressiveness ( 
    io_connect_t fb, 
    unsigned long type, 
    unsigned long aggressiveness )
{
    uint64_t inData[] = { type, aggressiveness };
    uint64_t rtn = 0;
    uint32_t len = 1;
    kern_return_t err = IOConnectCallScalarMethod(fb, kPMSetAggressiveness,
               inData, arrayCnt(inData), &rtn, &len);

    if (err)
        return kIOReturnError;
    else
        return (IOReturn) rtn;
}


IOReturn IOPMSleepSystem ( io_connect_t fb )
{
    uint64_t rtn = 0;
    uint32_t len = 1;
    kern_return_t err = IOConnectCallScalarMethod(fb, kPMSleepSystem,
                NULL, 0, &rtn, &len);

    if (err)
    return kIOReturnError;
    else
    return (IOReturn) rtn;
}

/* Private call for Apple Internal use only */
IOReturn IOPMSleepSystemWithOptions ( io_connect_t fb, CFDictionaryRef options )
{
    uint64_t rtn = 0;
    size_t len = sizeof(uint32_t);
    kern_return_t err;
    CFDataRef serializedOptions = NULL;
    
    if( !options ) {
        return IOPMSleepSystem( fb );
    }
    
    serializedOptions = IOCFSerialize( options, 0 );

    if (!serializedOptions) 
    {
        return kIOReturnInternalError;
    }
    
    /* kPMSleepSystemOptions
     * in: serialized CFDictionary of options
     * out: IOReturn code returned from sleepSystem
     */
    err = IOConnectCallStructMethod(
                fb, 
                kPMSleepSystemOptions,
                CFDataGetBytePtr(serializedOptions), /* inputStruct */
                CFDataGetLength(serializedOptions), /* inputStructCnt */
                &rtn, /* outputStruct */
                &len); /* outputStructCnt */

    CFRelease(serializedOptions);
                
    if (kIOReturnSuccess != err)
        return err;
    else
        return (IOReturn) rtn;
}


IOReturn IOPMCopyBatteryInfo( mach_port_t masterPort, CFArrayRef * oInfo )
{
    io_registry_entry_t         root_domain;
    IOReturn                    kr = kIOReturnUnsupported;
    
    *oInfo = NULL;
    
    // ********************************************************************
    // For PPC machines (with PMU), battery location is published under 
    // IOPMrootDomain
    root_domain = IORegistryEntryFromPath( masterPort, 
                        kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");
    if(!root_domain) return kIOReturnUnsupported;
    *oInfo = IORegistryEntryCreateCFProperty( 
                            root_domain, CFSTR(kIOBatteryInfoKey),
                            kCFAllocatorDefault, kNilOptions);
    IOObjectRelease(root_domain);
    
    if(*oInfo) {
        // Successfully read battery info from IOPMrootDomain
        return kIOReturnSuccess;
    } 
    
    
    // ********************************************************************
    // For non-PMU based batteries with IOPMPowerSource conforming classes
    // Scan IORegistry for IOPMPowerSource nodes with IOLegacyBatteryInfo
    // - Toss all IOLegacyBatteryInfo dictionaries into an OSArray
    int                     batt_count = 0;
    io_registry_entry_t     battery;
    io_iterator_t           ioreg_batteries;
    CFMutableArrayRef       legacyArray = CFArrayCreateMutable( 
                                kCFAllocatorDefault, 1, &kCFTypeArrayCallBacks);

    if(!legacyArray) return kIOReturnNoMemory;
    
    kr = IOServiceGetMatchingServices( 
                    MACH_PORT_NULL, 
                    IOServiceMatching("IOPMPowerSource"), 
                    &ioreg_batteries);
    if(KERN_SUCCESS != kr) {
        CFRelease(legacyArray);
        return kIOReturnError;
    }
    
    while( (battery = (io_registry_entry_t)IOIteratorNext(ioreg_batteries)) )
    {
        CFDictionaryRef     legacyDict;
        
        legacyDict = IORegistryEntryCreateCFProperty( battery, 
                                CFSTR(kIOPMPSLegacyBatteryInfoKey),
                                kCFAllocatorDefault,
                                0);

        if(!legacyDict) continue;

        batt_count++;
        CFArrayAppendValue(legacyArray, legacyDict);
        CFRelease(legacyDict);        
        IOObjectRelease(battery);
    }
    IOObjectRelease(ioreg_batteries);
    
    if(batt_count > 0) {
        *oInfo = legacyArray;
    } else {
        CFRelease(legacyArray);

        // Returns kIOReturnUnsupported if no batteries found
        return kIOReturnUnsupported;
    }

    return kIOReturnSuccess;
}


io_connect_t IORegisterApp( 
    void * refcon,
    io_service_t theDriver,
    IONotificationPortRef * thePortRef,
    IOServiceInterestCallback callback,
    io_object_t * notifier )
{
    io_connect_t        fb = MACH_PORT_NULL;
    kern_return_t        kr;
    
    *notifier = MACH_PORT_NULL;
    
    if ( theDriver == MACH_PORT_NULL ) goto failure_exit;

    kr = IOServiceOpen(theDriver, mach_task_self(), 0, &fb);

    if ( (kr != kIOReturnSuccess) || (fb == MACH_PORT_NULL) )  {
        goto failure_exit;
    }

    kr = IOServiceAddInterestNotification(
                            *thePortRef, theDriver, kIOAppPowerStateInterest,
                            callback, refcon, notifier);

    if ( kr == KERN_SUCCESS ) {
        // Successful exit case
        return fb;
    }

failure_exit:
    if ( fb != MACH_PORT_NULL ) {
        IOServiceClose(fb);
    }
    if ( *notifier != MACH_PORT_NULL ) {
        IOObjectRelease(*notifier);
    }
    return MACH_PORT_NULL;
}


io_connect_t IORegisterForSystemPower ( void * refcon,
                                        IONotificationPortRef * thePortRef,
                                        IOServiceInterestCallback callback,
                                        io_object_t * root_notifier )
{
    io_connect_t                fb = IO_OBJECT_NULL;
    IONotificationPortRef       notify = NULL;
    kern_return_t               kr;
    io_service_t                obj = IO_OBJECT_NULL;
     
    *root_notifier = IO_OBJECT_NULL;

    notify = IONotificationPortCreate(MACH_PORT_NULL);

    obj = IORegistryEntryFromPath( IO_OBJECT_NULL,
                    kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");

    if( obj == IO_OBJECT_NULL) goto failure_exit;
    
    kr = IOServiceOpen( obj,mach_task_self(), 0, &fb);

    if ( (kr != kIOReturnSuccess) || (fb == IO_OBJECT_NULL) )  {
        goto failure_exit;
    }

    kr = IOServiceAddInterestNotification(
                            notify,obj,kIOAppPowerStateInterest,
                            callback,refcon,root_notifier);

    IOObjectRelease(obj);
    if ( kr == KERN_SUCCESS ) {
        // Successful exit case
        *thePortRef = notify;
        return fb;
    }
    
failure_exit:    
    if ( obj != IO_OBJECT_NULL) {
        IOObjectRelease(obj);
    }
    if ( notify != NULL) {
        IONotificationPortDestroy(notify);
    }
    if ( fb != IO_OBJECT_NULL) {
        IOServiceClose(fb);
    }
    if ( *root_notifier != IO_OBJECT_NULL) {
        IOObjectRelease(*root_notifier);
    }
    
    return IO_OBJECT_NULL;
}


IOReturn IODeregisterApp ( io_object_t * notifier )
{
    if ( *notifier ) {
        IOObjectRelease(*notifier);
        *notifier = MACH_PORT_NULL;
    }
    return kIOReturnSuccess;
}


IOReturn IODeregisterForSystemPower ( io_object_t * root_notifier )
{
    if ( *root_notifier ) {
        IOObjectRelease(*root_notifier);
        *root_notifier = MACH_PORT_NULL;
    }
    return kIOReturnSuccess;
}


IOReturn IOAllowPowerChange(io_connect_t kernelPort, intptr_t notificationID)
{
    uint64_t inData = notificationID;
    kern_return_t err = IOConnectCallScalarMethod(
                                kernelPort, kPMAllowPowerChange,
                                &inData, 1, NULL, NULL);

    if (err) {
        return kIOReturnError;
    } else {
        return err;
    }
}


IOReturn IOCancelPowerChange ( io_connect_t kernelPort, intptr_t notificationID )
{
    uint64_t inData = notificationID;
    kern_return_t err = IOConnectCallScalarMethod(
                                kernelPort, kPMCancelPowerChange,
                                &inData, 1, NULL, NULL);

    if (err) {
        return kIOReturnError;
    } else {
        return err;
    }
}


boolean_t IOPMSleepEnabled ( void ) 
{
    io_registry_entry_t         root;
    boolean_t                   flag = false;
    CFTypeRef                   data = NULL;
    
    root = IORegistryEntryFromPath(MACH_PORT_NULL, 
                    kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");
    if ( !root ) return false;
    
    data = IORegistryEntryCreateCFProperty(
                            root, CFSTR("IOSleepSupported"),
                            kCFAllocatorDefault, kNilOptions);
    if ( data ) {
        flag = true;
        CFRelease(data);
    }

    IOObjectRelease(root);
    return flag;
}

/**************************************************
*
* System Load Advisory
*
* Reads system load state out of SCDynamicStore.
* PM configd plugin maintains state.
*
**************************************************/

#define kSLALevelPath       CFSTR("/IOKit/PowerManagement/SystemLoad")
#define kSLADetailedPath    CFSTR("/IOKit/PowerManagement/SystemLoad/Detailed")

/* IOGetSystemLoadAdvisory
 * In case of error, or inability to find system load advisory level,
 * returns kIOSystemLoadAdvisoryLevelOK.
 */
IOSystemLoadAdvisoryLevel IOGetSystemLoadAdvisory( void )
{
    IOSystemLoadAdvisoryLevel   _gt = kIOSystemLoadAdvisoryLevelOK;
    int                         notifyToken = 0;
    int                         status;
    uint64_t                    newval;

    status = notify_register_check(kIOSystemLoadAdvisoryNotifyName, &notifyToken);
    if (NOTIFY_STATUS_OK == status)
    {
        notify_get_state(notifyToken, &newval);
        notify_cancel(notifyToken);
        _gt = (IOSystemLoadAdvisoryLevel)newval;
    }
    
    return _gt;
}

/* IOCopyLoadAdvisoryLevelDetailed
 * In case of error, or inability to find system load advisory level,
 * returns NULL.
 */
CFDictionaryRef IOCopySystemLoadAdvisoryDetailed(void)
{
    CFDictionaryRef     gtDetailed = NULL;
    SCDynamicStoreRef   storage = NULL;
    CFStringRef         gtDetailedKey = SCDynamicStoreKeyCreate(
                            kCFAllocatorDefault, 
                            CFSTR("%@%@"),
                            _io_kSCDynamicStoreDomainState, 
                            kSLADetailedPath);

    storage = SCDynamicStoreCreate(
                            kCFAllocatorDefault,
                            CFSTR("IOKit IOGetSystemLoadAdvisoryDetailed"),
                            NULL,
                            NULL);

    if (!storage || !gtDetailedKey) {
        goto exit;
    }
    gtDetailed = SCDynamicStoreCopyValue(storage, gtDetailedKey);
    if(gtDetailed)
    {
        if(!isA_CFDictionary(gtDetailed)) {
            CFRelease(gtDetailed);
            gtDetailed = NULL;
        }
    }
exit:    
    if (gtDetailedKey) CFRelease(gtDetailedKey);
    if (storage) CFRelease(storage);    
    return gtDetailed;
}

