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

#include "IOSystemConfiguration.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPowerSourcesPrivate.h>
#include <IOKit/ps/IOPSKeys.h>
#include <IOKit/IOReturn.h>

#include "IOPMUPSPrivate.h"
#include "IOPMLibPrivate.h"



static bool 
_validUPSShutdownSettings(CFDictionaryRef prefs)
{
    CFDictionaryRef     d;
    CFNumberRef         val;
    CFBooleanRef        enabled;
    
    if(!prefs) return true;
    if(!isA_CFDictionary(prefs)) return false;

    d = CFDictionaryGetValue(prefs, CFSTR(kIOUPSShutdownAtLevelKey));
    if(d) {
        if(!isA_CFDictionary(d)) return false;
        val = (CFNumberRef)CFDictionaryGetValue(d, CFSTR(kIOUPSShutdownLevelValueKey));
        enabled = (CFBooleanRef)CFDictionaryGetValue(d, CFSTR(kIOUPSShutdownLevelEnabledKey));
        if(!isA_CFNumber(val) || !isA_CFBoolean(enabled)) return false;
    }

    d = CFDictionaryGetValue(prefs, CFSTR(kIOUPSShutdownAfterMinutesOn));
    if(d) {
        if(!isA_CFDictionary(d)) return false;
        val = (CFNumberRef)CFDictionaryGetValue(d, CFSTR(kIOUPSShutdownLevelValueKey));
        enabled = (CFBooleanRef)CFDictionaryGetValue(d, CFSTR(kIOUPSShutdownLevelEnabledKey));
        if(!isA_CFNumber(val) || !isA_CFBoolean(enabled)) return false;
    }

    d = CFDictionaryGetValue(prefs, CFSTR(kIOUPSShutdownAtMinutesLeft));
    if(d) {
        if(!isA_CFDictionary(d)) return false;
        val = (CFNumberRef)CFDictionaryGetValue(d, CFSTR(kIOUPSShutdownLevelValueKey));
        enabled = (CFBooleanRef)CFDictionaryGetValue(d, CFSTR(kIOUPSShutdownLevelEnabledKey));
        if(!isA_CFNumber(val) || !isA_CFBoolean(enabled)) return false;
    }

    return true;
}

static bool
_validUPSIdentifier(CFTypeRef whichUPS)
{
    if(!whichUPS || !isA_CFString(whichUPS)) 
        return false;

    if(!CFEqual(whichUPS, CFSTR(kIOPMDefaultUPSThresholds)))
        return false;   

    return true;
}

static CFDictionaryRef
_createDefaultThresholdDict(void)
{
    int                             zero = 0;
    CFMutableDictionaryRef          ups_setting = NULL;
    CFNumberRef                     cfnum = NULL;

    ups_setting = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &zero);
    
    CFDictionarySetValue(ups_setting, CFSTR(kIOUPSShutdownLevelValueKey), cfnum);
    CFRelease(cfnum);    
    CFDictionarySetValue(ups_setting, CFSTR(kIOUPSShutdownLevelEnabledKey), kCFBooleanFalse);

    return ups_setting;
}

static void
_mergeUnspecifiedUPSThresholds(
    CFTypeRef whichUPS __unused, 
    CFMutableDictionaryRef ret_dict)
{
    int                 i;
    CFDictionaryRef     dict[3];

    for(i=0; i<3; i++)
        dict[i] = _createDefaultThresholdDict();
    
    CFDictionaryAddValue(ret_dict, CFSTR(kIOUPSShutdownAtLevelKey), dict[0]);
    CFDictionaryAddValue(ret_dict, CFSTR(kIOUPSShutdownAfterMinutesOn), dict[1]);
    CFDictionaryAddValue(ret_dict, CFSTR(kIOUPSShutdownAtMinutesLeft), dict[2]);

    for(i=0; i<3; i++)
        CFRelease(dict[i]);

    return;
}


extern IOReturn 
IOPMSetUPSShutdownLevels(CFTypeRef whichUPS, CFDictionaryRef UPSPrefs)
{
    bool    hasUPS = false;
    
    if( (!UPSPrefs || !_validUPSShutdownSettings(UPSPrefs)) || 
        (!whichUPS || !_validUPSIdentifier(whichUPS)) )
    {
        return kIOReturnBadArgument;
    }

    IOPSGetSupportedPowerSources(NULL, NULL, &hasUPS);
    if (!hasUPS) {
        return kIOReturnNoDevice;
    }


    return IOPMWriteToPrefs(whichUPS, UPSPrefs, true, true);

}

extern CFDictionaryRef 
IOPMCopyUPSShutdownLevels(CFTypeRef whichUPS)
{
    CFDictionaryRef             tmp_dict = NULL;
    CFMutableDictionaryRef      ret_dict = NULL;
    bool                        hasUPS = false;

    if( !whichUPS || !_validUPSIdentifier(whichUPS) )
    {
        return NULL;
    }

    IOPSGetSupportedPowerSources(NULL, NULL, &hasUPS);
    if (!hasUPS) {
        return NULL;
    }

    tmp_dict = IOPMCopyFromPrefs(NULL, whichUPS);
    if (!tmp_dict) {
        ret_dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
            &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    else {
        ret_dict = CFDictionaryCreateMutableCopy(0, 0, tmp_dict);
    }
    if(!ret_dict) goto exit;
        
    // Merge in the default values
    _mergeUnspecifiedUPSThresholds(whichUPS, ret_dict);

    // Does this UPS support NOTHING? If so, just return NULL.
    if( (0 == CFDictionaryGetCount(ret_dict)) ||
        !_validUPSShutdownSettings(ret_dict) )
    {
        CFRelease(ret_dict);
        ret_dict = 0;
    }

exit:
    if (tmp_dict)  {
        CFRelease(tmp_dict);
    }
    return ret_dict;
}

