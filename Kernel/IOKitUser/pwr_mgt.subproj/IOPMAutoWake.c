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

#include <TargetConditionals.h>
#include <IOKit/pwr_mgt/IOPMPrivate.h>
#include <IOKit/pwr_mgt/IOPMLibDefs.h>
#include "IOSystemConfiguration.h"
#include "IOPMKeys.h"
#include "IOPMLib.h"
#include "IOPMLibPrivate.h"

#include "powermanagement_mig.h"
#include "powermanagement.h"

#include <servers/bootstrap.h>
#include <notify.h>
#include <asl.h>

enum {
    kIOPMMaxScheduledEntries = 1000
};

#ifndef kIOPMMaintenanceScheduleImmediate
#define kIOPMMaintenanceScheduleImmediate   "MaintenanceImmediate"
#endif

#ifndef kPMSetMaintenanceWakeCalendar
#define kPMSetMaintenanceWakeCalendar 8
#endif

// Forward decls

static IOReturn _IOPMCreatePowerOnDictionary(
    CFDateRef   the_time, 
    CFStringRef the_id, 
    CFStringRef type,
    CFMutableDictionaryRef *d);
static bool inputsValid(
    CFDateRef time_to_wake, 
    CFStringRef my_id, 
    CFStringRef type);
static IOReturn _setRootDomainProperty(
    CFStringRef key,
    CFTypeRef val);
static void tellClockController(
    CFStringRef command,
    CFDateRef power_date);
IOReturn IOPMSchedulePowerEvent(
    CFDateRef time_to_wake, 
    CFStringRef my_id, 
    CFStringRef type);
IOReturn IOPMCancelScheduledPowerEvent(
    CFDateRef time_to_wake, 
    CFStringRef my_id, 
    CFStringRef wake_or_restart);
IOReturn IOPMCancelAllScheduledPowerEvents( void );
CFArrayRef IOPMCopyScheduledPowerEvents( void );

static IOReturn doAMaintenanceWake(CFDateRef earliestRequest, int type);

__private_extern__ IOReturn _copyPMServerObject(int selector, int assertionID,
                                                CFTypeRef selectorData,  CFTypeRef *objectOut);


IOReturn _pm_connect(mach_port_t *newConnection);
IOReturn _pm_disconnect(mach_port_t connection);


static IOReturn 
_IOPMCreatePowerOnDictionary(
    CFDateRef   the_time, 
    CFStringRef the_id, 
    CFStringRef type,
    CFMutableDictionaryRef *dict)
{
    CFMutableDictionaryRef          d;

    // make sure my_id is valid or NULL
    the_id = isA_CFString(the_id);        
    if(!the_id) the_id = CFSTR("");

    if (!isA_CFDate(the_time)) {
        asl_log(0,0,ASL_LEVEL_ERR, "_IOPMCreatePowerOnDictionary received invalid date\n");
        return kIOReturnBadArgument;
    }
 
    d = CFDictionaryCreateMutable(0, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if(!d) return kIOReturnNoMemory;

    CFDictionaryAddValue(d, CFSTR(kIOPMPowerEventTimeKey), the_time);
    CFDictionaryAddValue(d, CFSTR(kIOPMPowerEventAppNameKey), the_id);
    CFDictionaryAddValue(d, CFSTR(kIOPMPowerEventTypeKey), type);

    *dict = d;

    return kIOReturnSuccess;
}

static bool 
inputsValid(
    CFDateRef time_to_wake, 
    CFStringRef my_id __unused, 
    CFStringRef type)
{
    // NULL is an acceptable input for my_id
    
    // NULL is only an accetable input to IOPMSchedulePowerEvent
    // if the type of event being scheduled is Immediate; in which
    // case NULL means "zero out current settings" to the hardware.
    if (!isA_CFDate(time_to_wake)
        && !CFEqual(type, CFSTR(kIOPMAutoWakeScheduleImmediate))
        && !CFEqual(type, CFSTR(kIOPMAutoPowerScheduleImmediate)))
    {
        return false;
    }
    
    if(!isA_CFString(type)) return false;
    if(!(CFEqual(type, CFSTR(kIOPMAutoWake)) || 
        CFEqual(type, CFSTR(kIOPMAutoPowerOn)) ||
        CFEqual(type, CFSTR(kIOPMAutoWakeOrPowerOn)) ||
        CFEqual(type, CFSTR(kIOPMAutoSleep)) ||
        CFEqual(type, CFSTR(kIOPMAutoShutdown)) ||
        CFEqual(type, CFSTR(kIOPMAutoRestart)) ||
        CFEqual(type, CFSTR(kIOPMAutoWakeScheduleImmediate)) ||
        CFEqual(type, CFSTR(kIOPMAutoPowerScheduleImmediate)) ||
        CFEqual(type, CFSTR(kIOPMAutoWakeRelativeSeconds)) ||
        CFEqual(type, CFSTR(kIOPMAutoPowerRelativeSeconds)) ||
        CFEqual(type, CFSTR(kIOPMMaintenanceScheduleImmediate)) ||
        CFEqual(type, CFSTR(kIOPMSleepServiceScheduleImmediate)) ))
    {
        return false;
    }

    return true;
}


static IOReturn 
_setRootDomainProperty(
    CFStringRef                 key, 
    CFTypeRef                   val) 
{
    io_registry_entry_t         root_domain;
    IOReturn                    ret;

    root_domain = IORegistryEntryFromPath( kIOMasterPortDefault, 
                kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");
    if(!root_domain) return kIOReturnError;
 
    ret = IORegistryEntrySetCFProperty(root_domain, key, val);

    IOObjectRelease(root_domain);
    return ret;
}

static IOReturn doAMaintenanceWake(
    CFDateRef                   earliestRequest,
    int                         type)
 {
    CFGregorianDate         maintGregorian;
    IOPMCalendarStruct      pmMaintenanceDate;
    IOReturn                connectReturn = 0;
    size_t                  connectReturnSize = sizeof(IOReturn);
    io_connect_t            root_domain_connect = IO_OBJECT_NULL;
    io_registry_entry_t     root_domain = IO_OBJECT_NULL;
    IOReturn                ret = kIOReturnError;
    kern_return_t           kr = -1;

    // Package maintenance time as a PMCalendarType and pass it into IOPMrootDomain
    CFTimeZoneRef  myTimeZone = NULL;
    
    myTimeZone = CFTimeZoneCreateWithTimeIntervalFromGMT(0, 0.0);
    if (!myTimeZone)
        goto exit;

    maintGregorian = CFAbsoluteTimeGetGregorianDate(CFDateGetAbsoluteTime(earliestRequest), myTimeZone);

    CFRelease(myTimeZone);
    
    // Stuff into PM Calendar struct
    bzero(&pmMaintenanceDate, sizeof(pmMaintenanceDate));
    pmMaintenanceDate.year      = maintGregorian.year;
    pmMaintenanceDate.month     = maintGregorian.month;
    pmMaintenanceDate.day       = maintGregorian.day;
    pmMaintenanceDate.hour      = maintGregorian.hour;
    pmMaintenanceDate.minute    = maintGregorian.minute;
    pmMaintenanceDate.second    = maintGregorian.second;
    pmMaintenanceDate.selector  = type;

    // Open up RootDomain
    
    root_domain = IORegistryEntryFromPath( kIOMasterPortDefault, 
                kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");

    if (root_domain != IO_OBJECT_NULL)
    {
        kr = IOServiceOpen(root_domain, mach_task_self(), 0, &root_domain_connect);        
        
        if (KERN_SUCCESS == kr)
        {
            kr = IOConnectCallStructMethod(
                root_domain_connect, 
                kPMSetMaintenanceWakeCalendar,
                &pmMaintenanceDate, sizeof(pmMaintenanceDate),      // inputs struct
                &connectReturn, &connectReturnSize);                // outputs struct
    
            IOServiceClose(root_domain_connect);
        }
        
        IOObjectRelease(root_domain);
    }

    if ((KERN_SUCCESS != kr) || (kIOReturnSuccess != connectReturn))
    {
        ret = kIOReturnError;
        goto exit;
    }

    ret = kIOReturnSuccess;
exit:
    return ret;
}

static void 
tellClockController(
    CFStringRef command, 
    CFDateRef power_date)
{
    CFAbsoluteTime          now, wake_time;
    CFGregorianDate         gmt_calendar;
    CFTimeZoneRef           gmt_tz = NULL;
    long int                diff_secs;
    IOReturn                ret;
    CFNumberRef             seconds_delta = NULL;
    IOPMCalendarStruct      *cal_date = NULL;
    CFMutableDataRef        date_data = NULL;

    if(!command) goto exit;
    
    // We broadcast the wakeup time both as calendar date struct and as seconds.
    //  * AppleRTC hardware needs the date in a structured calendar format

    // ******************** Calendar struct ************************************
    
    date_data = CFDataCreateMutable(NULL, sizeof(IOPMCalendarStruct));
    CFDataSetLength(date_data, sizeof(IOPMCalendarStruct));
    cal_date = (IOPMCalendarStruct *)CFDataGetBytePtr(date_data);
    bzero(cal_date, sizeof(IOPMCalendarStruct));

    if(!power_date) {
    
        // Zeroed out calendar means "clear wakeup timer"

    } else {

        // A calendar struct stuffed with meaningful date and time
        // schedules a wake or power event for then.

        wake_time = CFDateGetAbsoluteTime(power_date);

        gmt_tz = CFTimeZoneCreateWithTimeIntervalFromGMT(0, 0.0);
        gmt_calendar = CFAbsoluteTimeGetGregorianDate(wake_time, gmt_tz);
        CFRelease(gmt_tz);

        cal_date->second    = lround(gmt_calendar.second);
        if (60 == cal_date->second)
            cal_date->second = 59;
        cal_date->minute    = gmt_calendar.minute;
        cal_date->hour      = gmt_calendar.hour;
        cal_date->day       = gmt_calendar.day;
        cal_date->month     = gmt_calendar.month;
        cal_date->year      = gmt_calendar.year;
    }
    
    if(CFEqual(command, CFSTR(kIOPMAutoWake))) {

        // Set AutoWake calendar property
        ret = _setRootDomainProperty(CFSTR(kIOPMSettingAutoWakeCalendarKey), date_data);
    } else {

        // Set AutoPower calendar property
        ret = _setRootDomainProperty(CFSTR(kIOPMSettingAutoPowerCalendarKey), date_data);    
    }

    if(kIOReturnSuccess != ret) {
        goto exit;
    }


    // *************************** Seconds *************************************
    // ApplePMU/AppleSMU seconds path
    // Machine needs to be told alarm in seconds relative to current time.

    if(!power_date) {
        // NULL dictionary argument, clear wakeup timer
        diff_secs = 0;
    } else {
        // Assume a well-formed entry since we've been doing thorough 
        // type-checking in the find & purge functions
        now = CFAbsoluteTimeGetCurrent();
        wake_time = CFDateGetAbsoluteTime(power_date);
        
        diff_secs = lround(wake_time - now);
        if(diff_secs < 0) goto exit;
    }
    
    // Package diff_secs as a CFNumber
    seconds_delta = CFNumberCreate(0, kCFNumberLongType, &diff_secs);
    if(!seconds_delta) goto exit;
    
    if(CFEqual(command, CFSTR(kIOPMAutoWake))) {

        // Set AutoWake seconds property
        ret = _setRootDomainProperty(CFSTR(kIOPMSettingAutoWakeSecondsKey), seconds_delta);
    } else {

        // Set AutoPower seconds property
        ret = _setRootDomainProperty(CFSTR(kIOPMSettingAutoPowerSecondsKey), seconds_delta);
    }

    if(kIOReturnSuccess != ret) {
        goto exit;
    }

exit:
    if(date_data) CFRelease(date_data);
    if(seconds_delta) CFRelease(seconds_delta);
    return;
}


IOReturn IOPMSchedulePowerEvent(
    CFDateRef time_to_wake, 
    CFStringRef my_id,
    CFStringRef type)
{
    IOReturn                ret = kIOReturnError;
    CFDataRef               flatPackage = NULL;
    kern_return_t           rc = KERN_SUCCESS;
    mach_port_t       		pm_server = MACH_PORT_NULL;
    CFMutableDictionaryRef  package = 0;

    //  verify inputs
    if(!inputsValid(time_to_wake, my_id, type))
    {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    if( CFEqual(type, CFSTR(kIOPMMaintenanceScheduleImmediate)) )
    {
        ret = doAMaintenanceWake(time_to_wake, kPMCalendarTypeMaintenance);
        goto exit;
    } else if (CFEqual(type, CFSTR(kIOPMSleepServiceScheduleImmediate)) )
    {
        ret = doAMaintenanceWake(time_to_wake, kPMCalendarTypeSleepService);
        goto exit;
    } else if( CFEqual(type, CFSTR(kIOPMAutoWakeScheduleImmediate)) )
    {

        // Just send down the wake event immediately
        tellClockController(CFSTR(kIOPMAutoWake), time_to_wake);
        ret = kIOReturnSuccess;
        goto exit;

    } else if( CFEqual(type, CFSTR(kIOPMAutoPowerScheduleImmediate)) )
    {

        // Just send down the power on event immediately
        tellClockController(CFSTR(kIOPMAutoPowerOn), time_to_wake);
        ret = kIOReturnSuccess;
        goto exit;

    } else if( CFEqual( type, CFSTR( kIOPMAutoWakeRelativeSeconds) ) 
            || CFEqual( type, CFSTR( kIOPMAutoPowerRelativeSeconds) ) )
    {

        // Immediately send down a relative seconds argument
        // Seconds are relative to "right now" in CFAbsoluteTime
        CFAbsoluteTime      now_secs;
        CFAbsoluteTime      event_secs;
        CFNumberRef         diff_secs = NULL;
        int                 diff;

        if(time_to_wake)
        {
            now_secs = CFAbsoluteTimeGetCurrent();
            event_secs = CFDateGetAbsoluteTime(time_to_wake);
            diff = (int)event_secs - (int)now_secs;
            if(diff <= 0)
            {
                // Only positive diffs are meaningful
                return kIOReturnIsoTooOld;
            }
        } else {
            diff = 0;
        }
                
        diff_secs = CFNumberCreate(0, kCFNumberIntType, &diff);
        if(!diff_secs) goto exit;
        
        ret = _setRootDomainProperty( type, (CFTypeRef)diff_secs );

        CFRelease(diff_secs);
        goto exit;
    }

    if(kIOReturnSuccess != _pm_connect(&pm_server)) {
        ret = kIOReturnInternalError;
        goto exit;
    }
    
    // Package the event in a CFDictionary
    ret = _IOPMCreatePowerOnDictionary(time_to_wake, my_id, type, &package);
    if (ret != kIOReturnSuccess) {
        goto exit;
    }

    flatPackage = CFPropertyListCreateData(0, package,
                          kCFPropertyListBinaryFormat_v1_0, 0, NULL /* error */);
    if ( !flatPackage ) {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    rc = io_pm_schedule_power_event(pm_server, (vm_offset_t)CFDataGetBytePtr(flatPackage), 
                CFDataGetLength(flatPackage), kIOPMScheduleEvent, &ret);
    if (rc != KERN_SUCCESS)
        ret = kIOReturnInternalError;
    
    notify_post(kIOPMSchedulePowerEventNotification);

exit:
    
    if (MACH_PORT_NULL != pm_server) {
        _pm_disconnect(pm_server);
    }
    if(package) CFRelease(package);
    if(flatPackage) CFRelease(flatPackage);
    return ret;
}


IOReturn IOPMRequestSysWake(CFDictionaryRef req)
{
    IOReturn                ret = kIOReturnSuccess;
    mach_port_t             pm_server = MACH_PORT_NULL;
    IOReturn                err = kIOReturnSuccess;
    CFDateRef               date = NULL;
    CFStringRef             desc = NULL;
    CFNumberRef             leeway = NULL;
    CFDataRef               flatPackage = NULL;
    kern_return_t           kern_result = KERN_SUCCESS;
    CFMutableDictionaryRef  package = 0;
    CFBooleanRef            userVisible = kCFBooleanFalse;
    
    if(!isA_CFDictionary(req)) {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    date = CFDictionaryGetValue(req, CFSTR(kIOPMPowerEventTimeKey));
    desc = CFDictionaryGetValue(req, CFSTR(kIOPMPowerEventAppNameKey));
    leeway = CFDictionaryGetValue(req, CFSTR(kIOPMPowerEventLeewayKey));

    if (!isA_CFDate(date) || !isA_CFString(desc)) {
        ret = kIOReturnBadArgument;
        goto exit;
    }
        
    if ((leeway != NULL) && !isA_CFNumber(leeway)) {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    if (CFDictionaryGetValue(req, CFSTR(kIOPMPowerEventUserVisible)) == kCFBooleanTrue) {
        userVisible = kCFBooleanTrue;
    }

    err = _pm_connect(&pm_server);
    if(kIOReturnSuccess != err) {
        ret = kIOReturnInternalError;
        goto exit;
    }

    ret = _IOPMCreatePowerOnDictionary(date, desc, CFSTR(kIOPMAutoWake), &package);
    if (ret != kIOReturnSuccess) {
        goto exit;
    }
    if (leeway) {
        CFDictionaryAddValue(package, CFSTR(kIOPMPowerEventLeewayKey), leeway);
    }
    CFDictionaryAddValue(package, CFSTR(kIOPMPowerEventUserVisible), userVisible);

    flatPackage = CFPropertyListCreateData(0, package,
                          kCFPropertyListBinaryFormat_v1_0, 0, NULL /* error */);
    if ( !flatPackage ) {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    kern_result = io_pm_schedule_power_event(pm_server, 
            (vm_offset_t)CFDataGetBytePtr(flatPackage), CFDataGetLength(flatPackage), 1, &ret);

    if (kern_result != KERN_SUCCESS) {
        ret = kIOReturnInternalError;
        goto exit;
    }


exit:

    if (pm_server != MACH_PORT_NULL)
        _pm_disconnect(pm_server);
    if(package) CFRelease(package);
    if(flatPackage) CFRelease(flatPackage);
    return ret;
}


IOReturn IOPMCancelScheduledPowerEvent(
    CFDateRef time_to_wake, 
    CFStringRef my_id, 
    CFStringRef wake_or_restart)
{
    IOReturn                ret = kIOReturnSuccess;
    mach_port_t             pm_server = MACH_PORT_NULL;
    IOReturn                err = KERN_SUCCESS;
    CFDataRef               flatPackage = NULL;
    kern_return_t           kern_result = KERN_SUCCESS;
    CFMutableDictionaryRef  package = 0;
    
    if(!inputsValid(time_to_wake, my_id, wake_or_restart)) 
    {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    err = _pm_connect(&pm_server);
    if(kIOReturnSuccess != err) {
        ret = kIOReturnInternalError;
        goto exit;
    }

    ret = _IOPMCreatePowerOnDictionary(time_to_wake, my_id, wake_or_restart, &package);
    if (ret != kIOReturnSuccess) {
        goto exit;
    }
    flatPackage = CFPropertyListCreateData(0, package,
                          kCFPropertyListBinaryFormat_v1_0, 0, NULL /* error */);
    if ( !flatPackage ) {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    kern_result = io_pm_schedule_power_event(pm_server, 
            (vm_offset_t)CFDataGetBytePtr(flatPackage), CFDataGetLength(flatPackage), kIOPMCancelScheduledEvent, &ret);

    if (kern_result != KERN_SUCCESS) {
        ret = kIOReturnInternalError;
        goto exit;
    }


exit:

    if (pm_server != MACH_PORT_NULL)
        _pm_disconnect(pm_server);
    if(package) CFRelease(package);
    if(flatPackage) CFRelease(flatPackage);
    return ret;
}

IOReturn IOPMCancelAllScheduledPowerEvents( void )
{
    IOReturn                ret = kIOReturnInternalError;
    mach_port_t             pm_server = MACH_PORT_NULL;
    kern_return_t           err = KERN_SUCCESS;
    
    err = _pm_connect(&pm_server);
    if(kIOReturnSuccess != err) {
        ret = kIOReturnInternalError;
        goto exit;
    }

    io_pm_schedule_power_event(pm_server, NULL, NULL, kIOPMCancelAllScheduledEvents, &ret);
exit:

    if (pm_server != MACH_PORT_NULL)
        _pm_disconnect(pm_server);
    return ret;
}

CFArrayRef IOPMCopyScheduledPowerEvents(void)
{
    CFMutableArrayRef           new_arr = NULL;

    _copyPMServerObject(kIOPMPowerEventsMIGCopyScheduledEvents, 0, NULL, (CFTypeRef *)&new_arr);

    return (CFArrayRef)new_arr;
}
