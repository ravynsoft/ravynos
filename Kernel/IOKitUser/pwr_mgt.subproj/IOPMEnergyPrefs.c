/*
 * Copyright (c) 2001 Apple Computer, Inc. All rights reserved.
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

#include <sys/cdefs.h>
#include <TargetConditionals.h>

#include "IOSystemConfiguration.h"
#include <CoreFoundation/CoreFoundation.h>
#if !TARGET_OS_IPHONE
#include <IOKit/platform/IOPlatformSupportPrivate.h>
#endif
#include <IOKit/pwr_mgt/IOPM.h>
#include <IOKit/pwr_mgt/IOPMPrivate.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPowerSourcesPrivate.h>
#include <IOKit/pwr_mgt/IOPMUPSPrivate.h>
#include <IOKit/IOCFSerialize.h>
#include <IOKit/IOHibernatePrivate.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include <sys/syslog.h>
#include "IOPMLib.h"
#include "IOPMLibPrivate.h"
#include "powermanagement.h"

#include <asl.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>

#include <mach/mach.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <notify.h>

#include <os/log.h>

typedef struct {
    const char *keyName;
    uint32_t    defaultValueAC;
    uint32_t    defaultValueBattery;
    uint32_t    defaultValueUPS;
} PMSettingDescriptorStruct;

PMSettingDescriptorStruct defaultSettings[] =
{   /* Setting Name                                 AC - Battery - UPS */
    {kIOPMAutoPowerOffDelayKey,                    259200,   0,  0},
    {kIOPMAutoPowerOffEnabledKey,                       0,   0,  0},
    {kIOPMDarkWakeBackgroundTaskKey,                    1,   0,  0},
    {kIOPMDeepSleepEnabledKey,                          0,   0,  0},
    {kIOPMDeepSleepDelayKey,                            0,   0,  0},
    {kIOPMDiskSleepKey,                                 10, 10, 10},
    {kIOPMDisplaySleepKey,                              10,  2,  2},
    {kIOPMDisplaySleepUsesDimKey,                       1,   1,  1},
    {kIOPMDynamicPowerStepKey,                          1,   1,  0},
    {kIOPMGPUSwitchKey,                                 2,   2,  2},
  /* kIOHibernateFileKey is added manually in copyDefaultPreferences */
    {kIOHibernateModeKey,                               0,   0,  0},
    {kIOPMMobileMotionModuleKey,                        1,   1,  1},
    {kIOPMPrioritizeNetworkReachabilityOverSleepKey,    0,   0,  0},
    {kIOPMReduceBrightnessKey,                          0,   1,  1},
    {kIOPMReduceSpeedKey,                               0,   0,  1},
    {kIOPMRestartOnPowerLossKey,                        0,   0,  0},
    {kIOPMSleepOnPowerButtonKey,                        1,   1,  1},
    {kIOPMSystemSleepKey,                               10, 10, 10},
    {kIOPMTTYSPreventSleepKey,                          1,   1,  1},
    {kIOPMWakeOnACChangeKey,                            0,   0,  0},
    {kIOPMWakeOnClamshellKey,                           1,   1,  1},
    {kIOPMWakeOnLANKey,                                 1,   0,  0},
    {kIOPMWakeOnRingKey,                                1,   0,  0},
    {kIOPMTCPKeepAlivePrefKey,                          1,   1,  1},
    {kIOPMProximityDarkWakeKey,                         1,   0,  0},
    {kIOPMProModeKey,                                   0,   0,  0},
};

static const int kPMSettingsCount = sizeof(defaultSettings)/sizeof(PMSettingDescriptorStruct);

static bool overridesSet = false;
/* com.apple.PowerManagement.plist general keys
 */

#define kIOHibernateDefaultFile                         "/var/vm/sleepimage"

/* IOPMRootDomain property keys for default settings
 */
#define kIOPMSystemDefaultOverrideKey                   "SystemPowerProfileOverrideDict"


// Supported Feature bitfields for IOPMrootDomain Supported Features
enum {
    kIOPMSupportedOnAC              = 1<<0,
    kIOPMSupportedOnBatt            = 1<<1,
    kIOPMSupportedOnUPS             = 1<<2
};

/* Power sources
 *
 */
#define kPowerSourcesCount          3


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
// Forwards

static void updatePrefsDict(
                CFMutableDictionaryRef prefs,
                CFStringRef pwr_src,
                CFStringRef key,
                CFTypeRef value);
__private_extern__ io_registry_entry_t getPMRootDomainRef(void);
static CFStringRef supportedNameForPMName( CFStringRef pm_name );
static bool featureSupportsPowerSource(
                CFTypeRef                       featureDetails,
                CFStringRef                     power_source);

#define  kOverWriteDuplicates       true
#define  kKeepOriginalValues        false
static void mergeDictIntoMutable(
                CFMutableDictionaryRef          target,
                CFDictionaryRef                 overrides,
                bool                            overwrite);
static CFDictionaryRef getSystemProvidedPreferences(void);
static CFDictionaryRef copyDefaultPreferences(void);
static CFMutableDictionaryRef copyActivePreferences();

IOReturn _pm_connect(mach_port_t *newConnection);
IOReturn _pm_disconnect(mach_port_t connection);
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/**************************************************
*
* Energy Saver Preferences
*
**************************************************/

/**************************************************/

typedef struct {
    int dtoken;
} _PrefsChangeNotification;

IOPMNotificationHandle IOPMRegisterPrefsChangeNotification(dispatch_queue_t queue, void (^block)())
{
    _PrefsChangeNotification *_prefschange = NULL;
    uint32_t                 r = 0;

    _prefschange = calloc(1, sizeof(_PrefsChangeNotification));

    if (_prefschange)
    {
        r = notify_register_dispatch(kIOPMPrefsChangeNotify,
                                     &_prefschange->dtoken,
                                     queue,
                                     block);
        if (NOTIFY_STATUS_OK != r) {
            free(_prefschange);
            _prefschange = NULL;
        }
    }

    return _prefschange;
}

void IOPMUnregisterPrefsChangeNotification(IOPMNotificationHandle handle)
{
    _PrefsChangeNotification *_prefschange = (_PrefsChangeNotification *)handle;

    if (_prefschange)
    {
        if (_prefschange->dtoken) {
            notify_cancel(_prefschange->dtoken);
        }
        bzero(_prefschange, sizeof(*_prefschange));
        free(_prefschange);
    }
}



/*
 * Energy settings that are safe to migrate between different systems/models
 * are saved in default prefs path as returned by getGenericPrefsPath().
 *
 * Settings that are not safe to migrate between systems/models are saved in
 * host specific path as returned by getHostPrefsPath().
 *
 * We want to save all settings when user is upgrading the system. But, its
 * not safe to migrate hibernatemode from an iMac to MBPro. Ideally, we should be
 * able to migrate all settings betwen same models. But, that need some support
 * from CF and migration components.
 */

CFStringRef getGenericPrefsPath( )
{
    return CFSTR(kIOPMCFPrefsPath);
}

CFStringRef getHostPrefsPath()
{
    uuid_t uuid;
    static CFStringRef uuidStr = NULL;
    struct timespec ts = {1, 0};
    int rc;
    char cstr[100];

    if (isA_CFString(uuidStr)) {
        return uuidStr;
    }

    rc = gethostuuid(uuid, &ts);
    if (rc == 0) {
        snprintf(cstr, sizeof(cstr), "%s.%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                kIOPMCFPrefsPath,
                uuid[0],uuid[1],uuid[2],uuid[3],uuid[4],
                uuid[5],uuid[6],uuid[7],uuid[8],uuid[9],
                uuid[10],uuid[11],uuid[12],uuid[13],uuid[14],uuid[15]);

        uuidStr = CFStringCreateWithCString(kCFAllocatorDefault, cstr, kCFStringEncodingMacRoman);
    }
    else {
        os_log(OS_LOG_DEFAULT, "Failed to get UUID. rc=%d\n", rc);
    }
    return uuidStr;
}

bool isA_GenericPref(CFStringRef key)
{
    CFStringRef genericSettings[] = {
        CFSTR(kIOPMDarkWakeBackgroundTaskKey),
        CFSTR(kIOPMDiskSleepKey),
        CFSTR(kIOPMDisplaySleepKey),
        CFSTR(kIOPMDisplaySleepUsesDimKey),
        CFSTR(kIOPMGPUSwitchKey),
        CFSTR(kIOPMReduceBrightnessKey),
        CFSTR(kIOPMRestartOnPowerLossKey),
        CFSTR(kIOPMSystemSleepKey),
        CFSTR(kIOPMWakeOnLANKey),
        CFSTR(kIOPMProModeKey),
        CFSTR(kIOPMCarrierMode),
        CFSTR(kIOPMCarrierModeVh),
        CFSTR(kIOPMCarrierModeVl),
        CFSTR(kIOPMVact),
    };

    static CFSetRef genericSet = NULL;

    if (genericSet == NULL) {
        genericSet = CFSetCreate(0, (const void **)genericSettings,
                sizeof(genericSettings)/sizeof(genericSettings[0]), &kCFTypeSetCallBacks);
        if (genericSet == NULL) {
            return NULL;
        }
    }

    if (CFSetContainsValue(genericSet, key)) {
        return true;
    }
    else {
        return false;
    }

}

/*
 * copyPreferencesForSrc - Copies prefs for the specified power source from
 * both generic and host specific settings and then returns the merged dictionary
 */
CFMutableDictionaryRef copyPreferencesForSrc(CFStringRef power_source)
{
    CFStringRef         hostPrefsPath = getHostPrefsPath();
    CFStringRef         genericPrefsPath = getGenericPrefsPath();
    CFDictionaryRef     genericSettings = NULL;
    CFDictionaryRef     hostSettings = NULL;

    CFMutableDictionaryRef settings = NULL;

    genericSettings = IOPMCopyFromPrefs(genericPrefsPath, power_source);
    if (isA_CFDictionary(genericSettings)) {
        settings = CFDictionaryCreateMutableCopy(0, 0, genericSettings);
    }

    hostSettings = IOPMCopyFromPrefs(hostPrefsPath, power_source);
    if (isA_CFDictionary(hostSettings)) {
        if (settings) {
            mergeDictIntoMutable(settings, hostSettings, kKeepOriginalValues);
        }
        else {
            settings = CFDictionaryCreateMutableCopy(0, 0, hostSettings);
        }
    }

    if (genericSettings) {
        CFRelease(genericSettings);
    }

    if (hostSettings) {
        CFRelease(hostSettings);
    }

    return settings;
}

/*
 * setPreferencesForSrc - Separates the 'prefs' into generic and host specific
 * settings and writes those dictionaries to two separate prefs paths
 *
 * Returns if synchronize is successfull.
 */
bool setPreferencesForSrc(CFStringRef pwrSrc, CFDictionaryRef prefs, bool synchronize)
{

    CFIndex count;
    const CFStringRef         *keys = NULL;
    const CFTypeRef           *objs = NULL;
    bool  ret = false;

    CFStringRef                 hostPrefsPath = getHostPrefsPath();
    CFStringRef                 genericPrefsPath = getGenericPrefsPath();
    CFMutableDictionaryRef      genericSettings = NULL;
    CFMutableDictionaryRef      hostSettings = NULL;

    if (prefs == NULL) {
        goto exit;
    }
    count = CFDictionaryGetCount(prefs);
    if(0 == count) {
        goto exit;
    }

    keys = (CFStringRef *)malloc(sizeof(CFStringRef) * count);
    objs = (CFTypeRef *)malloc(sizeof(CFTypeRef) * count);
    if(!keys || !objs) {
        goto exit;
    }

    CFDictionaryGetKeysAndValues(prefs, (const void **)keys,  (const void **)objs);

    for(int i=0; i<count; i++) {

        if (isA_GenericPref(keys[i])) {

            if (genericSettings == NULL) {
                CFDictionaryRef tmp = IOPMCopyFromPrefs(genericPrefsPath, pwrSrc);
                if (tmp) {
                    genericSettings = CFDictionaryCreateMutableCopy(NULL, 0, tmp);
                    CFRelease(tmp);
                }
                else {
                    genericSettings = CFDictionaryCreateMutable(0, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                }
            }

            if (genericSettings) {
                CFDictionarySetValue(genericSettings, keys[i], objs[i]);
            }
        }
        else {
            if (hostSettings == NULL) {
                CFDictionaryRef tmp = IOPMCopyFromPrefs(hostPrefsPath, pwrSrc);
                if (tmp) {
                    hostSettings = CFDictionaryCreateMutableCopy(NULL, 0, tmp);
                    CFRelease(tmp);
                }
                else {
                    hostSettings = CFDictionaryCreateMutable(0, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                }
            }

            if (hostSettings) {
                CFDictionarySetValue(hostSettings, keys[i], objs[i]);
            }
        }
    }

exit:
    if (genericPrefsPath) {
        if (genericSettings) {
            CFPreferencesSetValue(pwrSrc, genericSettings, genericPrefsPath,
                    kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
        }
        else if (prefs == NULL) {
            CFPreferencesSetValue(pwrSrc, NULL, genericPrefsPath,
                    kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
        }
        if (synchronize) {
            ret = CFPreferencesSynchronize(genericPrefsPath,
                                              kCFPreferencesAnyUser,
                                              kCFPreferencesCurrentHost);
        }
    }

    if (hostPrefsPath) {
        if (hostSettings) {
            CFPreferencesSetValue(pwrSrc, hostSettings, hostPrefsPath,
                    kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
        }
        else if (prefs == NULL) {
            CFPreferencesSetValue(pwrSrc, NULL, hostPrefsPath,
                    kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
        }
        if (synchronize) {
            ret &= CFPreferencesSynchronize(hostPrefsPath,
                                              kCFPreferencesAnyUser,
                                              kCFPreferencesCurrentHost);
        }
    }

    if (keys) {
        free((void *)keys);
    }
    if (objs) {
        free((void *)objs);
    }
    if (genericSettings) {
        CFRelease(genericSettings);
    }
    if (hostSettings) {
        CFRelease(hostSettings);
    }
    return ret;

}

// INTERNAL ONLY --- Returns settings that are saved to disk. This will have
// all settings modified by user and may have some settings at default values
//
CFMutableDictionaryRef IOPMCopyPreferencesOnFile(void)
{

    CFMutableDictionaryRef prefs        = NULL;
    unsigned int    i;

    CFStringRef pwrSrc[] = {CFSTR(kIOPMACPowerKey), CFSTR(kIOPMBatteryPowerKey), CFSTR(kIOPMUPSPowerKey)};

    prefs = CFDictionaryCreateMutable(0, kPowerSourcesCount,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);

    if (!prefs) {
        return NULL;
    }

    for (i = 0; i < sizeof(pwrSrc)/sizeof(pwrSrc[0]); i++) {
        // Attempt to read custom settings
        CFMutableDictionaryRef dict = copyPreferencesForSrc(pwrSrc[i]);
        if (isA_CFDictionary(dict)) {
            CFDictionarySetValue(prefs, pwrSrc[i], dict);
            CFRelease(dict);
        }
    }

    return prefs;
}


/**************************************************/
CFMutableDictionaryRef IOPMCopyPMPreferences(void)
{
    return copyActivePreferences();
}

/**************************************************/

/*
 * Same as IOPMCopyPMPreferences, but we have to keep it
 * around since it is used by multiple projects already.
 */
CFDictionaryRef IOPMCopyActivePMPreferences(void)
{
    return IOPMCopyPMPreferences();
}

/**************************************************/

CFDictionaryRef IOPMCopyDefaultPreferences(void)
{
    return getSystemProvidedPreferences();
}

/**************************************************/
#if !TARGET_OS_IPHONE

IOReturn IOPMCopyPMSetting(
    CFStringRef key,
    CFStringRef power_source,
    CFTypeRef *outValue)
{
    CFDictionaryRef ActiveSettings  = NULL;
    CFDictionaryRef perPS           = NULL;
    CFStringRef     usePowerSource  = power_source;
    CFTypeRef       psblob          = NULL;
    bool            supported       = false;

    IOReturn        ret;

    if (!key || !outValue) {
        ret = kIOReturnBadArgument;
        goto exit;
    }

    *outValue = 0;

    if (!usePowerSource) {
        IOPSPowerSourceIndex activeps;
        IOPSGetSupportedPowerSources(&activeps, NULL, NULL);
        if (kIOPSProvidedByExternalBattery == activeps) {
            usePowerSource = CFSTR(kIOPMUPSPowerKey);
        } else if (kIOPSProvidedByBattery == activeps) {
            usePowerSource = CFSTR(kIOPMBatteryPowerKey);
        } else {
            usePowerSource = CFSTR(kIOPMACPowerKey);
        }
    }

    supported = IOPMFeatureIsAvailable(key, usePowerSource);

    if (!supported) {
        ret = kIOReturnUnsupported;
        goto exit;
    }

    ActiveSettings = IOPMCopyPMPreferences();
    if (ActiveSettings) {
        perPS = (CFDictionaryRef)CFDictionaryGetValue(
                          ActiveSettings, usePowerSource);

        if (perPS) {
            *outValue = CFDictionaryGetValue(perPS, key);
        }
    }

    if (*outValue) {
        CFRetain(*outValue);
        ret = kIOReturnSuccess;
    } else {
        ret = kIOReturnNotFound;
    }

    if (ActiveSettings) {
        CFRelease(ActiveSettings);
    }

exit:
    if (psblob) {
        CFRelease(psblob);
    }
    return ret;
}

#endif

/**************************************************/

bool comparePrefsToDefaults(CFDictionaryRef prefs, CFStringRef pwr_src)
{
    CFIndex count;
    bool  matches = true;
    CFTypeRef   *defaultValue = NULL;
    CFStringRef *keys = NULL;
    CFTypeRef *prefsValues = NULL;
    CFDictionaryRef defaults = NULL;

    if (!prefs || ((count = CFDictionaryGetCount(prefs)) == 0)) {
        return true;
    }

    defaults = getSystemProvidedPreferences();


    CFDictionaryRef defaultsForSrc = CFDictionaryGetValue(defaults, pwr_src);
    if (!defaultsForSrc || (CFDictionaryGetCount(defaultsForSrc) == 0)) {
        matches = false;
        goto exit;
    }

    keys = (CFStringRef *)malloc(sizeof(CFStringRef) * count);
    prefsValues = (CFTypeRef *)malloc(sizeof(CFTypeRef) * count);

    if(!keys || !prefsValues) {
        matches = false;
        goto exit;
    }
    CFDictionaryGetKeysAndValues(prefs, (const void **)keys,  (const void **)prefsValues);

    for (CFIndex idx = 0; idx < count; idx++) {

        defaultValue = NULL;
        if (!CFDictionaryGetValueIfPresent(defaultsForSrc, keys[idx], (const void **)&defaultValue)) {
            matches = false;
            break;
        }

        if (!CFEqual(defaultValue, prefsValues[idx])) {
            matches = false;
            break;
        }

    }


exit:
   if (keys) {
        free((void *)keys);
    }
    if (prefsValues) {
        free((void *)prefsValues);
    }
    if (defaults) {
        CFRelease(defaults);
    }

    return matches;
}

bool IOPMUsingDefaultPreferences(CFStringRef pwr_src)
{
    bool ret = false;

    if (isA_CFString(pwr_src)) {
        CFDictionaryRef prefs = copyPreferencesForSrc(pwr_src);

        ret = comparePrefsToDefaults(prefs, pwr_src);
        if (prefs) CFRelease(prefs);

    } else {
        // Check for all sources
        CFDictionaryRef acPrefs = copyPreferencesForSrc(CFSTR(kIOPMACPowerKey));
        CFDictionaryRef battPrefs = copyPreferencesForSrc(CFSTR(kIOPMBatteryPowerKey));
        CFDictionaryRef upsPrefs = copyPreferencesForSrc(CFSTR(kIOPMUPSPowerKey));

        ret = true;
        ret &= comparePrefsToDefaults(acPrefs, CFSTR(kIOPMACPowerKey));
        if (ret) {
            ret &= comparePrefsToDefaults(battPrefs, CFSTR(kIOPMBatteryPowerKey));
        }
        if (ret) {
            ret &= comparePrefsToDefaults(upsPrefs, CFSTR(kIOPMUPSPowerKey));
        }

        if (acPrefs)    CFRelease(acPrefs);
        if (battPrefs)  CFRelease(battPrefs);
        if (upsPrefs)   CFRelease(upsPrefs);
    }

    return ret;
}

/**************************************************/

IOReturn IOPMRevertPMPreferences(CFArrayRef keys_arr)
{
    IOReturn                ret     = kIOReturnInternalError;
    int                     count   = 0;
    CFStringRef             setting = NULL;
    CFMutableDictionaryRef  prefs   = IOPMCopyPreferencesOnFile();

    if (!prefs) {
        goto exit;
    }

    // Check if settings are already defaults
    if (IOPMUsingDefaultPreferences(NULL)) {
        ret = kIOReturnSuccess;
        goto exit;
    }

    // Revert all preferences
    if (!keys_arr || (0 == (count = CFArrayGetCount(keys_arr)))) {
        ret = IOPMSetPMPreferences(NULL);
        goto exit;
    }

    // Remove each key in the array
    for (int i = 0; i < count; i++) {
        setting = CFArrayGetValueAtIndex(keys_arr, i);
        if (isA_CFString(setting)) {
            updatePrefsDict(prefs, CFSTR(kIOPMACPowerKey), setting, NULL);
            updatePrefsDict(prefs, CFSTR(kIOPMBatteryPowerKey), setting, NULL);
            updatePrefsDict(prefs, CFSTR(kIOPMUPSPowerKey), setting, NULL);
        }
    }

    ret = IOPMSetPMPreferences(prefs);

exit:
    if (prefs) CFRelease(prefs);
    return ret;
}

/**************************************************/

static void updatePrefsDict(CFMutableDictionaryRef prefs,
                            CFStringRef pwr_src,
                            CFStringRef key,
                            CFTypeRef value)
{
    CFMutableDictionaryRef dictRef = NULL;
    CFDictionaryRef        pwrDict = NULL;

    pwrDict = CFDictionaryGetValue(prefs, pwr_src);
    if (!pwrDict) {
        return;
    }

    dictRef = CFDictionaryCreateMutableCopy(0, CFDictionaryGetCount(pwrDict), pwrDict);
    if (dictRef) {
        if (value) {
            CFDictionarySetValue(dictRef, key, value);
        } else {
            CFDictionaryRemoveValue(dictRef, key);
        }

        CFDictionarySetValue(prefs, pwr_src, dictRef);
        CFRelease(dictRef);
    }

    return;
}

IOReturn IOPMSetPMPreference(CFStringRef key,
                             CFTypeRef value,
                             CFStringRef pwr_src)
{
    CFMutableDictionaryRef prefs   = IOPMCopyPreferencesOnFile();
    IOReturn               ret     = kIOReturnError;

    if (!prefs) {
        goto exit;
    }

    // Make sure power source is supported
    if (pwr_src) {
        if (!CFDictionaryContainsKey(prefs, pwr_src)) {
            ret = kIOReturnBadArgument;
            goto exit;
        }
    }

    // Revert all keys
    if (key == NULL) {
        if (pwr_src) {
            // Revert a single power source's preferences
            CFDictionaryRemoveValue(prefs, pwr_src);
        } else {
            // Pass in null to IOPMSetPMPreferences to revert all preferences
            CFRelease(prefs);
            prefs = NULL;
        }
    }
    // Single key set/revert
    else {
        if (pwr_src) {
            updatePrefsDict(prefs, pwr_src, key, value);
        } else {
            updatePrefsDict(prefs, CFSTR(kIOPMACPowerKey), key, value);
            updatePrefsDict(prefs, CFSTR(kIOPMBatteryPowerKey), key, value);
            updatePrefsDict(prefs, CFSTR(kIOPMUPSPowerKey), key, value);
        }
    }

    ret = IOPMSetPMPreferences(prefs);

exit:
    if (prefs) CFRelease(prefs);
    return ret;
}

/**************************************************/

// Sets (and activates) the preferences
IOReturn IOPMSetPMPreferences(CFDictionaryRef ESPrefs)
{
    bool result = false;
    CFDictionaryRef tmp = NULL;

    if ((getuid() != 0) && (geteuid() != 0)) {
        return kIOReturnNotPrivileged;
    }
    if (ESPrefs == NULL) {
        // Revert all preferences
        setPreferencesForSrc(CFSTR(kIOPMACPowerKey), NULL, false);
        setPreferencesForSrc(CFSTR(kIOPMBatteryPowerKey), NULL, false);
        result = setPreferencesForSrc(CFSTR(kIOPMUPSPowerKey), NULL, true);

    } else {
        // Set each power source dictionary
        tmp = CFDictionaryGetValue(ESPrefs, CFSTR(kIOPMACPowerKey));
        setPreferencesForSrc(CFSTR(kIOPMACPowerKey), tmp, false);

        tmp = CFDictionaryGetValue(ESPrefs, CFSTR(kIOPMBatteryPowerKey));
        setPreferencesForSrc(CFSTR(kIOPMBatteryPowerKey), tmp, false);

        tmp = CFDictionaryGetValue(ESPrefs, CFSTR(kIOPMUPSPowerKey));
        result = setPreferencesForSrc(CFSTR(kIOPMUPSPowerKey), tmp, true);
    }

    if (result) {
        notify_post(kIOPMPrefsChangeNotify);
    }

    return (result ? kIOReturnSuccess : kIOReturnError);
}

/*!
 * @function          IOPMWriteToPrefs
 * @abstract          Writes the specified key/value pair to appropriate PM Prefs based on the key
 * @param             key, value - key/value pair
 * @param             synchronize - should set to true if CFPreferencesSynchronize should be called.
 * @param             broadcast - should set to true if this write need to be notified to all listeners
 */
IOReturn IOPMWriteToPrefs(CFStringRef key, CFTypeRef value,
                          bool synchronize, bool broadcast)
{
    bool result = true;
    CFStringRef prefsPath = NULL;

    if ((getuid() != 0) && (geteuid() != 0)) {
        return kIOReturnNotPrivileged;
    }
    if (isA_GenericPref(key)) {
        prefsPath = getGenericPrefsPath();
    }
    else {
        prefsPath = getHostPrefsPath();
    }

    CFPreferencesSetValue(key, value,
                          prefsPath,
                          kCFPreferencesAnyUser,
                          kCFPreferencesCurrentHost);

    if (synchronize) {
        result = CFPreferencesSynchronize(prefsPath,
                                          kCFPreferencesAnyUser,
                                          kCFPreferencesCurrentHost);

        if (result && broadcast) {
            notify_post(kIOPMPrefsChangeNotify);
        }
    }

    return (result ? kIOReturnSuccess : kIOReturnError);
}



/*!
 @function          IOPMCopyFromPrefs
 @abstract          Copies the value from the appropriate prefs path(based on the key) and returns it
 * @param           prefsPath - The path to preferences file.
 *                              If set to NULL, function will find the appropriate prefs path
 * @param           key - Key  to read
 */

CFTypeRef IOPMCopyFromPrefs(CFStringRef prefsPath, CFStringRef key)
{

    CFTypeRef value = NULL;

    if (prefsPath == NULL) {
        if (isA_GenericPref(key)) {
            prefsPath = getGenericPrefsPath();
        }
        else {
            prefsPath = getHostPrefsPath();
        }
    }

    if (!isA_CFString(prefsPath)) {
        return NULL;
    }

    value  = CFPreferencesCopyValue(key,
                                    prefsPath,
                                    kCFPreferencesAnyUser,
                                    kCFPreferencesCurrentHost);

    return value;
}

/**************************************************/

/*
 * IOPMFeatureIsAvailable
 (
 * @param PMFeature - Name of a PM feature (like "WakeOnRing" or "Reduce Processor Speed")
 * @param power_source - The current power source (like "AC Power" or "Battery Power")
 * @result true if the given PM feature is supported on the given power source,
 *      false if the feature is unsupported.
 */
bool IOPMFeatureIsAvailable(CFStringRef PMFeature, CFStringRef power_source)
{
    CFDictionaryRef             supportedFeatures = NULL;
    io_registry_entry_t         registry_entry = MACH_PORT_NULL;
    int                         return_this_value = 0;

    if(!(registry_entry = getPMRootDomainRef()))
        return false;

    supportedFeatures = IORegistryEntryCreateCFProperty(registry_entry, CFSTR("Supported Features"),
                                                        kCFAllocatorDefault, kNilOptions);

    if (!supportedFeatures)
        return false;

    if( CFEqual(PMFeature, CFSTR(kIOPMDarkWakeBackgroundTaskKey)) )
    {

#if TARGET_OS_IPHONE
        goto exit;
#else
        return_this_value = 0;

        if (!power_source)
            power_source = CFSTR(kIOPMACPowerKey);

        // On new machines (late 2012 and beyond), IOPPF publishes PowerNap
        // support using the kIOPMPowerNapSupportedKey
        CFTypeRef pnDetails = CFDictionaryGetValue( supportedFeatures,
                                CFSTR(kIOPMPowerNapSupportedKey));
        if(featureSupportsPowerSource(pnDetails, power_source))
        {
            return_this_value = 1;
            goto exit;
        }


        CFTypeRef btdetails = CFDictionaryGetValue( supportedFeatures,
                                CFSTR(kIOPMDarkWakeBackgroundTaskKey));
        CFTypeRef ssdetails = CFDictionaryGetValue( supportedFeatures,
                                CFSTR(kIOPMSleepServicesKey));
        if(featureSupportsPowerSource(btdetails, power_source) ||
           featureSupportsPowerSource(ssdetails, power_source))
        {
            return_this_value = 1;
        }
    }
    else if (CFEqual(PMFeature, CFSTR(kIOPMTCPKeepAlivePrefKey)))
    {
        CFTypeRef tcpka = kCFBooleanFalse;
        IOPlatformCopyFeatureDefault(kIOPlatformTCPKeepAliveDuringSleep, &tcpka);
        if (tcpka == kCFBooleanTrue)
            return_this_value = 1;
        else
            return_this_value = 0;
#endif
    }
    else
    {
       return_this_value = IOPMFeatureIsAvailableWithSupportedTable(
                               PMFeature,
                               power_source,
                               supportedFeatures);
    }

exit:
    CFRelease(supportedFeatures);

    return (return_this_value ? true:false);
}


bool IOPMFeatureIsAvailableWithSupportedTable(
     CFStringRef                PMFeature,
     CFStringRef                power_source,
     CFDictionaryRef            supportedFeatures)
{
    CFStringRef                 supportedString = NULL;
    CFTypeRef                   featureDetails = NULL;
    bool                        ret = false;

    if (!power_source)
        power_source = CFSTR(kIOPMACPowerKey);

    if (!supportedFeatures)
        return false;

    /* Basic sleep timer settings are always available *
     * TTY connection ability to prevent sleep is always available */
    if (CFEqual(PMFeature, CFSTR(kIOPMDisplaySleepKey))
        || CFEqual(PMFeature, CFSTR(kIOPMSystemSleepKey))
        || CFEqual(PMFeature, CFSTR(kIOPMDiskSleepKey))
        || CFEqual(PMFeature, CFSTR(kIOPMTTYSPreventSleepKey))
        || CFEqual(PMFeature, kIOPMSleepDisabledKey)
        || CFEqual(PMFeature, CFSTR(kIOPMDestroyFVKeyOnStandbyKey)))
    {
        ret = true;
        goto exit;
    }

    /* deprecated - kIOPMRestartOnKernelPanicKey
     * 11195840 Remove "restart automatically if computer freezes" check-box
     */
    if (CFEqual(PMFeature, CFSTR(kIOPMRestartOnKernelPanicKey))) {
        ret = false;
        goto exit;
    }

    // *********************************
    // Special case for PowerButtonSleep

    if(CFEqual(PMFeature, CFSTR(kIOPMSleepOnPowerButtonKey)))
    {
#if TARGET_OS_IPHONE
        ret = false;
#else
        CFArrayRef  tmp_array = NULL;
        // Pressing the power button only causes sleep on desktop PowerMacs,
        // cubes, and iMacs.
        // Therefore this feature is not supported on portables.
        // We'll use the presence of a battery (or the capability for a battery)
        // as evidence whether this is a portable or not.
        IOReturn r = IOPMCopyBatteryInfo(kIOMasterPortDefault, &tmp_array);
        if((r == kIOReturnSuccess) && tmp_array)
        {
            CFRelease(tmp_array);
            ret = false;
        } else ret = true;
#endif
        goto exit;
    }

    // *********************************
    // Special case for ReduceBrightness

    if ( CFEqual(PMFeature, CFSTR(kIOPMReduceBrightnessKey)) )
    {
        // ReduceBrightness feature is only supported on laptops
        // and on desktops with UPS with brightness-adjustable LCD displays.
        // These machines report a "DisplayDims" property in the
        // supportedFeatures dictionary.
        // ReduceBrightness is never supported on AC Power.
        bool hasBatt = false;
        bool hasUPS = false;
        IOPSGetSupportedPowerSources(NULL, &hasBatt, &hasUPS);

        if (( hasBatt || hasUPS )
            && supportedFeatures
            && CFDictionaryGetValue(supportedFeatures, CFSTR("DisplayDims"))
            && !CFEqual(power_source, CFSTR(kIOPMACPowerKey)) )
        {
            ret = true;
        } else {
            ret = false;
        }

        goto exit;
    }

#if TARGET_OS_IOS || TARGET_OS_WATCH
    if (CFEqual(PMFeature, CFSTR(kIOPMCarrierMode))
        || CFEqual(PMFeature, CFSTR(kIOPMCarrierModeVl))
        || CFEqual(PMFeature, CFSTR(kIOPMCarrierModeVh)))
    {
        ret = true;
        goto exit;
    }
#endif // TARGET_OS_IOS || TARGET_OS_WATCH

    // ***********************************
    // Generic code for all other settings

    supportedString = supportedNameForPMName( PMFeature );
    if(!supportedString) {
        ret = false;
        goto exit;
    }

    /*
     * Special casing for hibernation: Hibernation feature support is published
     * before SystemPowerProfileOverrideDict is set with model specific hibernatemode.
     * This results in picking default mode 0.
     *  Allow support to hibernation only after overrides are set
     */
    if (CFEqual(supportedString, CFSTR(kIOHibernateFeatureKey)) && !overridesSet) {
        ret = false;
        goto exit;
    }
    featureDetails = CFDictionaryGetValue(supportedFeatures, supportedString);
    if(!featureDetails) {
        ret = false;
        goto exit;
    }

    if(featureSupportsPowerSource(featureDetails, power_source))
    {
        ret = true;
    }


exit:
    return ret;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
// Internals

/* getPMRootDomainRef
 *
 * Caller should not release the returned io_registry_entry_t
 */
__private_extern__ io_registry_entry_t  getPMRootDomainRef(void)
{
    static io_registry_entry_t cached_root_domain = MACH_PORT_NULL;

    if( MACH_PORT_NULL == cached_root_domain ) {
        cached_root_domain = IORegistryEntryFromPath( kIOMasterPortDefault,
                        kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");

    }
    return cached_root_domain;
}


/* Maps a PowerManagement string constant
 *   -> to its corresponding Supported Feature in IOPMrootDomain
 */
static CFStringRef
supportedNameForPMName( CFStringRef pm_name )
{
#if TARGET_OS_IPHONE
    if( CFEqual(pm_name, CFSTR(kIOPMReduceBrightnessKey))
        || CFEqual(pm_name, CFSTR(kIOPMDisplaySleepUsesDimKey)) )
#else
    if(CFEqual(pm_name, CFSTR(kIOPMDisplaySleepUsesDimKey)))
#endif /* TARGET_OS_IPHONE */
    {
        return CFSTR("DisplayDims");
    }

    if(CFEqual(pm_name, CFSTR(kIOPMWakeOnLANKey))
       || CFEqual(pm_name, CFSTR(kIOPMPrioritizeNetworkReachabilityOverSleepKey)))
    {
        return CFSTR("WakeOnMagicPacket");
    }

    if(CFEqual(pm_name, CFSTR(kIOPMMobileMotionModuleKey)))
    {
        return CFSTR("MobileMotionModule");
    }

    if( CFEqual(pm_name, CFSTR(kIOHibernateModeKey))
        || CFEqual(pm_name, CFSTR(kIOHibernateFreeRatioKey))
        || CFEqual(pm_name, CFSTR(kIOHibernateFreeTimeKey))
        || CFEqual(pm_name, CFSTR(kIOHibernateFileKey))
        || CFEqual(pm_name, CFSTR(kIOHibernateFeatureKey)))
    {
        return CFSTR(kIOHibernateFeatureKey);
    }

    if (CFEqual(pm_name, CFSTR(kIOPMDeepSleepEnabledKey))
        || CFEqual(pm_name, CFSTR(kIOPMDeepSleepDelayKey))
        || CFEqual(pm_name, CFSTR(kIOPMDeepSleepDelayHighKey))
        || CFEqual(pm_name, CFSTR(kIOPMStandbyBatteryThresholdKey)))
    {
        return CFSTR("DeepSleep");
    }

    if (CFEqual(pm_name, CFSTR(kIOPMAutoPowerOffEnabledKey))
        || CFEqual(pm_name, CFSTR(kIOPMAutoPowerOffDelayKey)))
    {
        return CFSTR("AutoPowerOff");
    }

    if (CFEqual(pm_name, CFSTR(kIOPMProximityDarkWakeKey))) {
        return CFSTR("ProximityWake");
    }

#if TARGET_OS_IOS || TARGET_OS_WATCH
    if (CFEqual(pm_name, CFSTR(kIOPMCarrierMode))
        || CFEqual(pm_name, CFSTR(kIOPMCarrierModeVh))
        || CFEqual(pm_name, CFSTR(kIOPMCarrierModeVl)))
    {
        return CFSTR("CarrierChargingMode");
    }
#endif // TARGET_OS_IPHONE || TARGET_OS_WATCH
#if TARGET_OS_OSX
    if (CFEqual(pm_name, CFSTR(kIOPMVact)))
    {
        return CFSTR("VAC-T");
    }
#endif // TARGET_OS_OSX

    return pm_name;
}

// Helper for IOPMFeatureIsAvailable
static bool
featureSupportsPowerSource(CFTypeRef featureDetails, CFStringRef power_source)
{
    CFNumberRef         featureNum   = NULL;
    CFNumberRef         tempNum      = NULL;
    CFArrayRef          featureArr   = NULL;
    uint32_t            ps_support   = 0;
    uint32_t            tmp;
    unsigned int        i;

    if( (featureNum = isA_CFNumber(featureDetails)) )
    {
        CFNumberGetValue(featureNum, kCFNumberSInt32Type, &ps_support);
    } else if( (featureArr = isA_CFArray(featureDetails)) )
    {
        // If several entitites are asserting a given feature, we OR
        // together their supported power sources.

        unsigned int arrayCount = CFArrayGetCount(featureArr);
        for(i = 0; i<arrayCount; i++)
        {
            tempNum = isA_CFNumber(CFArrayGetValueAtIndex(featureArr, i));
            if(tempNum) {
                CFNumberGetValue(tempNum, kCFNumberSInt32Type, &tmp);
                ps_support |= tmp;
            }
        }
    }

    if(!power_source) {

        // Lack of a defined power source just gets a "true" return
        // if the setting is supported on ANY power source.

        return (ps_support ? true : false);
    }

    if(CFEqual(CFSTR(kIOPMACPowerKey), power_source) )
    {
        return (ps_support & kIOPMSupportedOnAC) ? true : false;
    } else if(CFEqual(CFSTR(kIOPMBatteryPowerKey), power_source) )
    {
        return (ps_support & kIOPMSupportedOnBatt) ? true : false;
    } else if(CFEqual(CFSTR(kIOPMUPSPowerKey), power_source) )
    {
        return (ps_support & kIOPMSupportedOnUPS) ? true : false;
    } else {
        // unexpected power source argument
        return false;
    }

}

static bool checkPowerSourceSupported(CFStringRef str)
{
    bool hasBatt;
    bool hasUPS;

    if (CFEqual(str, CFSTR(kIOPMACPowerKey))) {
        return true;
    }

    IOPSGetSupportedPowerSources(NULL, &hasBatt, &hasUPS);

    if (CFEqual(str, CFSTR(kIOPMBatteryPowerKey))) {
        return hasBatt;
    }

    if (CFEqual(str, CFSTR(kIOPMUPSPowerKey))) {
        return hasUPS;
    }
    return false;
}

/***
 * IOPMRemoveIrrelevantProperties
 *
 * Prunes unsupported properties from the energy dictionary.
 * e.g. If your machine doesn't have a modem, this removes the Wake On Ring property.
 ***/
void IOPMRemoveIrrelevantProperties(CFMutableDictionaryRef energyPrefs)
{
    int                         profile_count = 0;
    int                         dict_count    = 0;
    CFStringRef                 *profile_keys = NULL;
    CFDictionaryRef             *profile_vals = NULL;
    CFStringRef                 *dict_keys    = NULL;
    CFDictionaryRef             *dict_vals    = NULL;
    CFMutableDictionaryRef      this_profile  = NULL;
    CFDictionaryRef                _supportedCached = NULL;
    io_registry_entry_t         _rootDomain   = IO_OBJECT_NULL;
    bool                        hasBatt       = false;
    bool                        hasUPS        = false;


    // Grab a copy of RootDomain's supported energy saver settings
    _rootDomain = getPMRootDomainRef();
    if (IO_OBJECT_NULL != _rootDomain)
    {
        _supportedCached = IORegistryEntryCreateCFProperty(_rootDomain,
                                                           CFSTR("Supported Features"),
                                                           kCFAllocatorDefault, kNilOptions);
    }

    /*
     * Remove features when not supported -
     *      Wake On Administrative Access, Dynamic Speed Step, etc.
     */
    profile_count = CFDictionaryGetCount(energyPrefs);
    profile_keys = (CFStringRef *)malloc(sizeof(CFStringRef) * profile_count);
    profile_vals = (CFDictionaryRef *)malloc(sizeof(CFDictionaryRef) * profile_count);
    if (!profile_keys || !profile_vals)
        goto exit;

    hasBatt = checkPowerSourceSupported(CFSTR(kIOPMBatteryPowerKey));
    hasUPS = checkPowerSourceSupported(CFSTR(kIOPMUPSPowerKey));

    CFDictionaryGetKeysAndValues(energyPrefs, (const void **)profile_keys,
                                 (const void **)profile_vals);
    // For each CFDictionary at the top level (battery, AC)
    while(--profile_count >= 0)
    {
        if ((CFEqual(profile_keys[profile_count], CFSTR(kIOPMBatteryPowerKey)) && !hasBatt) ||
            (CFEqual(profile_keys[profile_count], CFSTR(kIOPMUPSPowerKey)) && !hasUPS))
        {
            // Remove dictionary if the whole power source isn't supported on this machine.
            CFDictionaryRemoveValue(energyPrefs, profile_keys[profile_count]);
        } else {

            // Make a mutable copy of the prefs dictionary

            this_profile = (CFMutableDictionaryRef)isA_CFDictionary(
                                                                    CFDictionaryGetValue(energyPrefs, profile_keys[profile_count]));
            if(!this_profile)
                continue;

            this_profile = CFDictionaryCreateMutableCopy(NULL, 0, this_profile);
            if(!this_profile)
                continue;

            CFDictionarySetValue(energyPrefs, profile_keys[profile_count], this_profile);
            CFRelease(this_profile);

            // And prune unneeded settings from our new mutable property

            dict_count = CFDictionaryGetCount(this_profile);
            dict_keys = (CFStringRef *)malloc(sizeof(CFStringRef) * dict_count);
            dict_vals = (CFDictionaryRef *)malloc(sizeof(CFDictionaryRef) * dict_count);
            if (!dict_keys || !dict_vals)
                continue;
            CFDictionaryGetKeysAndValues(this_profile,
                                         (const void **)dict_keys, (const void **)dict_vals);
            // For each specific property within each dictionary
            while(--dict_count >= 0)
            {
                if( CFEqual((CFStringRef)dict_keys[dict_count], CFSTR(kIOPMDarkWakeBackgroundTaskKey)) )
                {
#if !TARGET_OS_IPHONE
                    // We conditionalize PowerNap support on kIOPMPowerNapSupportedKey for all
                    // machines late 2012 and beyond. The presence of this key is a sufficient
                    // condition to support PowerNap
                    bool supportsNewPNKey = false;
                    if(IOPMFeatureIsAvailableWithSupportedTable( CFSTR(kIOPMPowerNapSupportedKey),
                           (CFStringRef)profile_keys[profile_count], _supportedCached)) {
                            supportsNewPNKey = true;
                    }

                    // For legacy machines, we look for either the kIOPMDarkWakeBackgroundTaskKey or the
                    // kIOPMSleepServicesKey
                    if ( ((!IOPMFeatureIsAvailableWithSupportedTable( CFSTR(kIOPMDarkWakeBackgroundTaskKey),
                            (CFStringRef)profile_keys[profile_count], _supportedCached)  &&
                        !IOPMFeatureIsAvailableWithSupportedTable( CFSTR(kIOPMSleepServicesKey),
                            (CFStringRef)profile_keys[profile_count], _supportedCached))
                         && !supportsNewPNKey
                      )
                   {
                       CFDictionaryRemoveValue(this_profile, (CFStringRef)dict_keys[dict_count]);
                   }
                }
                else if (CFEqual((CFStringRef)dict_keys[dict_count], CFSTR(kIOPMTCPKeepAlivePrefKey)))
                {
                    CFTypeRef tcpka = kCFBooleanFalse;
                    IOPlatformCopyFeatureDefault(kIOPlatformTCPKeepAliveDuringSleep, &tcpka);
                    if (tcpka == kCFBooleanFalse) {
                        CFDictionaryRemoveValue(this_profile,
                                                (CFStringRef)dict_keys[dict_count]);
                    }
#endif /* TARGET_OS_OSX */
                }
                else if( !IOPMFeatureIsAvailableWithSupportedTable((CFStringRef)dict_keys[dict_count],
                                    (CFStringRef)profile_keys[profile_count], _supportedCached) )
                {
                    // If the property isn't supported, remove it
                    CFDictionaryRemoveValue(this_profile,
                                            (CFStringRef)dict_keys[dict_count]);
                }
            }
            free(dict_keys); dict_keys = NULL;
            free(dict_vals); dict_vals = NULL;
        }
    }

exit:
    if (dict_keys) {
        free(dict_keys);
    }
    if (dict_vals) {
        free(dict_vals);
    }
    if (profile_keys)
        free(profile_keys);
    if (profile_vals)
        free(profile_vals);
    if (_supportedCached)
        CFRelease(_supportedCached);
    return;
}

/**************************************************
*
* System Power Settings
*
**************************************************/



IOReturn IOPMActivateSystemPowerSettings( void )
{

    return kIOReturnUnsupported;
}

CFDictionaryRef IOPMCopySystemPowerSettings(void)
{
    CFDictionaryRef         settings                = NULL;

    settings = CFPreferencesCopyValue(CFSTR(kIOPMSystemPowerSettingsKey),
                                      getGenericPrefsPath(),
                                      kCFPreferencesAnyUser,
                                      kCFPreferencesCurrentHost);

    // No custom settings, so these keys will not be present
    if (isA_CFDictionary(settings)) {
        return settings;
    }
    else {
        return NULL;
    }
}

IOReturn IOPMSetSystemPowerSetting(CFStringRef key, CFTypeRef value)
{
    IOReturn                rc = kIOReturnError;
#if !TARGET_OS_IPHONE
    bool                    result = false;
    CFDictionaryRef         settings = NULL;
    CFMutableDictionaryRef  mutableSettings = NULL;

    if ((getuid() != 0) && (geteuid() != 0)) {
        return kIOReturnNotPrivileged;
    }

    settings = IOPMCopySystemPowerSettings();
    if (!settings) {
        mutableSettings = CFDictionaryCreateMutable(0, 0,
                                &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks);
    }
    else {
        mutableSettings = CFDictionaryCreateMutableCopy(0, 0, settings);
    }

    if (!mutableSettings) {
        rc = kIOReturnError;
        goto exit;
    }

    CFDictionarySetValue(mutableSettings, key, value);
    CFPreferencesSetValue(CFSTR(kIOPMSystemPowerSettingsKey),
                          mutableSettings, getGenericPrefsPath(),
                          kCFPreferencesAnyUser, kCFPreferencesCurrentHost);

    result = CFPreferencesSynchronize(getGenericPrefsPath(),
                                      kCFPreferencesAnyUser,
                                      kCFPreferencesCurrentHost);

    if (result) {
        notify_post(kIOPMPrefsChangeNotify);
    }

    rc = (result ? kIOReturnSuccess : kIOReturnError);

exit:
    if (settings) {
        CFRelease(settings);
    }
    if (mutableSettings) {
        CFRelease(mutableSettings);
    }
#else
    (void)key;
    (void)value;
#endif
    return rc;
}

/**************************************************
*
* Power Profiles
*
*
**************************************************/

static void mergeDictIntoMutable(
    CFMutableDictionaryRef  target,
    CFDictionaryRef         overrides,
    bool                    overwrite)
{
    const CFStringRef         *keys = NULL;
    const CFTypeRef           *objs = NULL;
    int                 count;
    int                 i;

    count = CFDictionaryGetCount(overrides);
    if(0 == count) return;

    keys = (CFStringRef *)malloc(sizeof(CFStringRef) * count);
    objs = (CFTypeRef *)malloc(sizeof(CFTypeRef) * count);
    if(!keys || !objs) {
        goto exit;
    }

    CFDictionaryGetKeysAndValues(overrides,
                    (const void **)keys, (const void **)objs);
    for(i=0; i<count; i++)
    {
        if(overwrite) {
            CFDictionarySetValue(target, keys[i], objs[i]);
        } else {
            // no value added if key already present
            CFDictionaryAddValue(target, keys[i], objs[i]);
        }
    }

exit:

    if (keys) {
        free((void *)keys);
    }
    if (objs) {
        free((void *)objs);
    }
}

/* getSystemProvidedPreferences()
 *
 * The IOPlatformPluginFamily kext on the system may conditionally override the Energy
 * Saver's profiles. Only IOPlatformPluginFamily should be setting these properties.
 * We check for kIOPMSystemDefaultOverrideKey (a partial settings defaults substitution).
 *
 * Overrides are a single dictionary of PM settings merged into the
 * default PM settings defined in the defaultSettings array.
 *
 * Alternatively, Overrides are a 3 dictionary set, each dictionary
 * being a proper PM settings dictionary. The 3 keys must be
 * "AC Power", "Battery Power" and "UPS Power" respectively. Each
 * dictionary under those keys should contain only PM settings.
 *
 */
static CFDictionaryRef getSystemProvidedPreferences(void)
{
    CFMutableDictionaryRef  systemPrefs     = NULL;
    CFDictionaryRef         overridePrefs   = NULL;
    CFDictionaryRef         acOverride      = NULL;
    CFDictionaryRef         battOverride    = NULL;
    CFDictionaryRef         upsOverride     = NULL;
    CFDictionaryRef         tmp             = NULL;
    CFMutableDictionaryRef  acDict          = NULL;
    CFMutableDictionaryRef  battDict        = NULL;
    CFMutableDictionaryRef  upsDict         = NULL;
    CFDictionaryRef         defaultPrefs    = NULL;
    io_registry_entry_t     regEntry        = MACH_PORT_NULL;
    bool                    overrides       = false;
    bool                    overrideCopied  = false;

    regEntry = getPMRootDomainRef();
    if (regEntry == MACH_PORT_NULL) {
        goto exit;
    }

    overridePrefs = IORegistryEntryCreateCFProperty(regEntry,
                        CFSTR(kIOPMSystemDefaultOverrideKey),
                        kCFAllocatorDefault, 0);

    overrides = isA_CFDictionary(overridePrefs);

    if (overrides) {
        acOverride = CFDictionaryGetValue(overridePrefs, CFSTR(kIOPMACPowerKey));
        battOverride = CFDictionaryGetValue(overridePrefs, CFSTR(kIOPMBatteryPowerKey));
        upsOverride = CFDictionaryGetValue(overridePrefs, CFSTR(kIOPMUPSPowerKey));

        if(!acOverride && !battOverride && !upsOverride)
        {
            // The dictionary didn't specify any per-power source overrides, which
            // means that it's a flat dictionary strictly of PM settings.
            // We duplicate it 3 ways, as each overridden setting in this dictionary
            // will be applied to each power source's settings.
            acOverride = CFDictionaryCreateCopy(0,overridePrefs);
            battOverride = CFDictionaryCreateCopy(0,overridePrefs);
            upsOverride = CFDictionaryCreateCopy(0,overridePrefs);
            overrideCopied = true;
        } else if (!acOverride || !battOverride || !upsOverride) {
            goto exit;
        }
        overridesSet = true;
    }

    defaultPrefs = copyDefaultPreferences();
    if(!defaultPrefs) {
        goto exit;
    }

    tmp = CFDictionaryGetValue(defaultPrefs, CFSTR(kIOPMACPowerKey));
    if (tmp) {
        acDict = CFDictionaryCreateMutableCopy(0, CFDictionaryGetCount(tmp), tmp);
        if (!acDict) {
            goto exit;
        }
    }
    tmp = CFDictionaryGetValue(defaultPrefs, CFSTR(kIOPMBatteryPowerKey));
    if (tmp) {
        battDict = CFDictionaryCreateMutableCopy(0, CFDictionaryGetCount(tmp), tmp);
        if (!battDict) {
            goto exit;
        }
    }
    tmp = CFDictionaryGetValue(defaultPrefs, CFSTR(kIOPMUPSPowerKey));
    if (tmp) {
        upsDict = CFDictionaryCreateMutableCopy(0, CFDictionaryGetCount(tmp), tmp);
        if (!upsDict) {
            goto exit;
        }
    }

    if (overrides) {
        mergeDictIntoMutable(acDict, acOverride, kOverWriteDuplicates);
        mergeDictIntoMutable(battDict, battOverride, kOverWriteDuplicates);
        mergeDictIntoMutable(upsDict, upsOverride, kOverWriteDuplicates);
    }

    systemPrefs = CFDictionaryCreateMutable(0,
                                kPowerSourcesCount,
                                &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks);
    if (!systemPrefs) {
        goto exit;
    }
    CFDictionaryAddValue(systemPrefs, CFSTR(kIOPMACPowerKey), acDict);
    CFDictionaryAddValue(systemPrefs, CFSTR(kIOPMBatteryPowerKey), battDict);
    CFDictionaryAddValue(systemPrefs, CFSTR(kIOPMUPSPowerKey), upsDict);

exit:
    if (overridePrefs) CFRelease(overridePrefs);
    if (acDict)        CFRelease(acDict);
    if (battDict)      CFRelease(battDict);
    if (upsDict)       CFRelease(upsDict);
    if (defaultPrefs)  CFRelease(defaultPrefs);
    if (overrideCopied) {
        if (acOverride) {
            CFRelease(acOverride);
        }
        if (battOverride) {
            CFRelease(battOverride);
        }
        if (upsOverride) {
            CFRelease(upsOverride);
        }
    }

    return systemPrefs;
}

static CFDictionaryRef copyDefaultPreferences(void)
{
    CFMutableDictionaryRef  batt    = NULL;
    CFMutableDictionaryRef  ac      = NULL;
    CFMutableDictionaryRef  ups     = NULL;
    CFMutableDictionaryRef  prefs   = NULL;
    int                     i;
    CFNumberRef             val;
    CFStringRef             key;

    batt = CFDictionaryCreateMutable(0, 0,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
    ac = CFDictionaryCreateMutable(0, 0,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
    ups = CFDictionaryCreateMutable(0, 0,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);

    /*
     * Populate default battery dictionary
     */

    for (i=0; i<kPMSettingsCount; i++) {

        key = CFStringCreateWithCString(0, defaultSettings[i].keyName, kCFStringEncodingMacRoman);

        if (batt) {
            val = CFNumberCreate(0, kCFNumberSInt32Type, &defaultSettings[i].defaultValueBattery);
            CFDictionarySetValue(batt, key, val);
            CFRelease(val);
        }

        if (ac) {
            val = CFNumberCreate(0, kCFNumberSInt32Type, &defaultSettings[i].defaultValueAC);
            CFDictionarySetValue(ac, key, val);
            CFRelease(val);
        }
        if (ups) {
            val = CFNumberCreate(0, kCFNumberSInt32Type, &defaultSettings[i].defaultValueUPS);
            CFDictionarySetValue(ups, key, val);
            CFRelease(val);
        }
        CFRelease(key);
    }

    if (batt) {
        CFDictionarySetValue(batt, CFSTR(kIOHibernateFileKey), CFSTR(kIOHibernateDefaultFile));
    }
    if (ac) {
        CFDictionarySetValue(ac, CFSTR(kIOHibernateFileKey), CFSTR(kIOHibernateDefaultFile));
    }
    if (ups) {
        CFDictionarySetValue(ups, CFSTR(kIOHibernateFileKey), CFSTR(kIOHibernateDefaultFile));
    }

    prefs = CFDictionaryCreateMutable(0, kPowerSourcesCount,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
    if (prefs) {
        CFDictionaryAddValue(prefs, CFSTR(kIOPMACPowerKey), ac);
        CFDictionaryAddValue(prefs, CFSTR(kIOPMBatteryPowerKey), batt);
        CFDictionaryAddValue(prefs, CFSTR(kIOPMUPSPowerKey), ups);
    }

    if (ac) {
        CFRelease(ac);
    }
    if (batt) {
        CFRelease(batt);
    }
    if (ups) {
        CFRelease(ups);
    }

    return prefs;
}

CFMutableDictionaryRef copyActivePreferences()
{
    CFMutableDictionaryRef prefs        = NULL;
    CFMutableDictionaryRef tmp          = NULL;
    CFDictionaryRef        acDict       = NULL;
    CFDictionaryRef        battDict     = NULL;
    CFDictionaryRef        upsDict      = NULL;
    CFDictionaryRef        acDef        = NULL;
    CFDictionaryRef        battDef      = NULL;
    CFDictionaryRef        upsDef       = NULL;
    CFDictionaryRef        defaultPrefs = NULL;

    // Attempt to read custom settings
    acDict = copyPreferencesForSrc(CFSTR(kIOPMACPowerKey));
    battDict = copyPreferencesForSrc(CFSTR(kIOPMBatteryPowerKey));
    upsDict = copyPreferencesForSrc(CFSTR(kIOPMUPSPowerKey));

    prefs = CFDictionaryCreateMutable(0, kPowerSourcesCount,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);

    if (!prefs) {
        goto exit;
    }

    // Get default preferences to merge with our custom preferences
    defaultPrefs = getSystemProvidedPreferences();
    if (!defaultPrefs) {
        goto exit;
    }
    acDef = CFDictionaryGetValue(defaultPrefs, CFSTR(kIOPMACPowerKey));
    if (!acDef) {
        goto exit;
    }
    battDef = CFDictionaryGetValue(defaultPrefs, CFSTR(kIOPMBatteryPowerKey));
    if (!battDef) {
        goto exit;
    }
    upsDef = CFDictionaryGetValue(defaultPrefs, CFSTR(kIOPMBatteryPowerKey));
    if (!upsDef) {
        goto exit;
    }

    // Copy any custom settings into our dictionary
    if (acDict) {
        tmp = CFDictionaryCreateMutableCopy(0, CFDictionaryGetCount(acDict), acDict);
    } else {
        tmp = CFDictionaryCreateMutable(0, 0,
                                        &kCFTypeDictionaryKeyCallBacks,
                                        &kCFTypeDictionaryValueCallBacks);
    }
    // Merge in defaults and store in the prefs dictionary
    if (tmp) {
        mergeDictIntoMutable(tmp, acDef, kKeepOriginalValues);
        CFDictionarySetValue(prefs, CFSTR(kIOPMACPowerKey), tmp);
        CFRelease(tmp);
    } else {
        goto exit;
    }

    if (battDict) {
        tmp = CFDictionaryCreateMutableCopy(0, CFDictionaryGetCount(battDict), battDict);
    } else {
        tmp = CFDictionaryCreateMutable(0, 0,
                                        &kCFTypeDictionaryKeyCallBacks,
                                        &kCFTypeDictionaryValueCallBacks);
    }
    if (tmp) {
        mergeDictIntoMutable(tmp, battDef, kKeepOriginalValues);
        CFDictionarySetValue(prefs, CFSTR(kIOPMBatteryPowerKey), tmp);
        CFRelease(tmp);
    } else {
        goto exit;
    }

    if (upsDict) {
        tmp = CFDictionaryCreateMutableCopy(0, CFDictionaryGetCount(upsDict), upsDict);
    } else {
        tmp = CFDictionaryCreateMutable(0, 0,
                                        &kCFTypeDictionaryKeyCallBacks,
                                        &kCFTypeDictionaryValueCallBacks);
    }
    if (tmp) {
        mergeDictIntoMutable(tmp, upsDef, kKeepOriginalValues);
        CFDictionarySetValue(prefs, CFSTR(kIOPMUPSPowerKey), tmp);
        CFRelease(tmp);
    } else {
        goto exit;
    }

    // Remove unsupported keys/power sources
    IOPMRemoveIrrelevantProperties(prefs);

exit:
    if (defaultPrefs)   CFRelease(defaultPrefs);
    if (acDict)         CFRelease(acDict);
    if (battDict)       CFRelease(battDict);
    if (upsDict)        CFRelease(upsDict);

    return prefs;
}
