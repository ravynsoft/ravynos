/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#include <mach/mach.h>
#include <TargetConditionals.h>
#include <mach/kmod.h>
#include <sys/param.h>
#include <mach/mach_error.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include <syslog.h>
#include <stdarg.h>

#include "OSKext.h"
#include "misc_util.h"
#include "KextManager.h"
#include "KextManagerPriv.h"
#include "kextmanager_mig.h"

#define ARRAY_COUNT(array) (sizeof((array)) / sizeof((array[0])))

/*********************************************************************
* IMPORTANT: All calls in this module should be simple RPCs to kextd
* or use the OSKext library *without* creating any OSKext objects.
*********************************************************************/

static kern_return_t get_kextd_port(mach_port_t *kextd_port); // internal convenience function

/*********************************************************************
*********************************************************************/
CFURLRef KextManagerCreateURLForBundleIdentifier(
    CFAllocatorRef allocator,
    CFStringRef    bundleIdentifier)
{
    CFURLRef bundleURL = NULL;  // returned

    kern_return_t kern_result = kOSReturnError;
    char bundle_id[KMOD_MAX_NAME] = "";

    mach_port_t   kextd_port = MACH_PORT_NULL;

    char bundle_path[MAXPATHLEN] = "";
    CFStringRef bundlePath = NULL;  // must free
    OSReturn kext_result = kOSReturnError;
    kext_result_t tmpRes;

    if (!bundleIdentifier) {
        goto finish;
    }

    if (!CFStringGetCString(bundleIdentifier,
        bundle_id, sizeof(bundle_id) - 1, kCFStringEncodingUTF8)) {
        goto finish;
    }

    kern_result = get_kextd_port(&kextd_port);
    if (kern_result != kOSReturnSuccess) {
        goto finish;
    }

    kern_result = kextmanager_path_for_bundle_id(
        kextd_port, bundle_id, bundle_path, &tmpRes);
    kext_result = tmpRes;
    if (kern_result != kOSReturnSuccess) {
        goto finish;
    }

    if (kext_result != kOSReturnSuccess) {
        goto finish;
    }

    bundlePath = CFStringCreateWithCString(kCFAllocatorDefault,
        bundle_path, kCFStringEncodingUTF8);
    if (!bundlePath) {
        goto finish;
    }

    bundleURL = CFURLCreateWithFileSystemPath(allocator,
        bundlePath, kCFURLPOSIXPathStyle, true);

finish:

    if (bundlePath)  CFRelease(bundlePath);

    return bundleURL;
}

/*********************************************************************
* Internal function for use by KextManagerLoadKextWithIdentifier()
* and KextManagerLoadKextWithURL().
*********************************************************************/
OSReturn __KextManagerSendLoadKextRequest(
    CFMutableDictionaryRef  requestDict,
    CFArrayRef              dependencyKextAndFolderURLs)
{
    OSReturn           result           = kOSReturnError;
    mach_port_t        kextd_port       = MACH_PORT_NULL;
    CFDataRef          requestData      = NULL;  // must release
    CFMutableArrayRef  dependencyPaths  = NULL;  // must release
    CFURLRef           dependencyAbsURL = NULL;  // must release
    CFStringRef        dependencyPath   = NULL;  // must release
    CFErrorRef         err              = NULL;  // must release

    if (!requestDict) {
        result = kOSKextReturnInvalidArgument;
        goto finish;
    }

    result = get_kextd_port(&kextd_port);
    if (result != kOSReturnSuccess) {
        goto finish;
    }

    if (dependencyKextAndFolderURLs &&
        CFArrayGetCount(dependencyKextAndFolderURLs)) {

        CFIndex count, index;

        dependencyPaths = CFArrayCreateMutable(kCFAllocatorDefault,
            /* capacity */ 0, &kCFTypeArrayCallBacks);
        if (!dependencyPaths) {
            result = kOSKextReturnNoMemory;
            goto finish;
        }
        CFDictionarySetValue(requestDict, kKextLoadDependenciesKey,
            dependencyPaths);

        count = CFArrayGetCount(dependencyKextAndFolderURLs);
        for (index = 0; index < count; index++) {
            CFURLRef thisURL = (CFURLRef)CFArrayGetValueAtIndex(
                dependencyKextAndFolderURLs, index);

            SAFE_RELEASE_NULL(dependencyPath);
            SAFE_RELEASE_NULL(dependencyAbsURL);

            dependencyAbsURL = CFURLCopyAbsoluteURL(thisURL);
            if (!dependencyAbsURL) {
                result = kOSKextReturnNoMemory;
                goto finish;
            }
            dependencyPath = CFURLCopyFileSystemPath(dependencyAbsURL,
                kCFURLPOSIXPathStyle);
            if (!dependencyPath) {
                result = kOSKextReturnNoMemory;
                goto finish;
            }

            CFArrayAppendValue(dependencyPaths, dependencyPath);
        }
    }

    requestData = CFPropertyListCreateData(kCFAllocatorDefault,
         requestDict, kCFPropertyListBinaryFormat_v1_0,
         /* options */ 0,
         &err);
    if (!requestData) {
        // any point in logging error reason here? nothing caller can do....
        result = kOSKextReturnSerialization;
        goto finish;
    }

    result = kextmanager_load_kext(kextd_port,
        (char *)CFDataGetBytePtr(requestData),
        CFDataGetLength(requestData));

finish:
    SAFE_RELEASE(requestData);
    SAFE_RELEASE(dependencyPaths);
    SAFE_RELEASE(dependencyPath);
    SAFE_RELEASE(dependencyAbsURL);
    SAFE_RELEASE(err);

    return result;
}

/*********************************************************************
*********************************************************************/
OSReturn KextManagerLoadKextWithIdentifier(
    CFStringRef    kextIdentifier,
    CFArrayRef     dependencyKextAndFolderURLs)
{
    OSReturn               result      = kOSReturnError;
    CFMutableDictionaryRef requestDict = NULL;  // must release

    if (!kextIdentifier) {
        result = kOSKextReturnInvalidArgument;
        goto finish;
    }

    requestDict = CFDictionaryCreateMutable(kCFAllocatorDefault,
        /* capacity */ 0, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!requestDict) {
        result = kOSKextReturnNoMemory;
        goto finish;
    }

    CFDictionarySetValue(requestDict, kKextLoadIdentifierKey,
        kextIdentifier);

    result = __KextManagerSendLoadKextRequest(requestDict,
        dependencyKextAndFolderURLs);

finish:
    SAFE_RELEASE(requestDict);
    return result;
}

/*********************************************************************
*********************************************************************/
OSReturn KextManagerLoadKextWithURL(
    CFURLRef       kextURL,
    CFArrayRef     dependencyKextAndFolderURLs)
{
    OSReturn               result      = kOSReturnError;
    CFMutableDictionaryRef requestDict = NULL;  // must release
    CFURLRef               absURL      = NULL;  // must release
    CFStringRef            kextPath    = NULL;  // must release

    if (!kextURL) {
        result = kOSKextReturnInvalidArgument;
        goto finish;
    }

    requestDict = CFDictionaryCreateMutable(kCFAllocatorDefault,
        /* capacity */ 0, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!requestDict) {
        result = kOSKextReturnNoMemory;
        goto finish;
    }

    absURL = CFURLCopyAbsoluteURL(kextURL);
    if (!absURL) {
        result = kOSKextReturnNoMemory;
        goto finish;
    }

    kextPath = CFURLCopyFileSystemPath(absURL, kCFURLPOSIXPathStyle);
    if (!kextPath) {
        result = kOSKextReturnSerialization;
        goto finish;
    }
    CFDictionarySetValue(requestDict, kKextLoadPathKey, kextPath);

    result = __KextManagerSendLoadKextRequest(requestDict,
        dependencyKextAndFolderURLs);

finish:
    SAFE_RELEASE(requestDict);
    SAFE_RELEASE(absURL);
    SAFE_RELEASE(kextPath);
    return result;
}

/*********************************************************************
*********************************************************************/
OSReturn KextManagerUnloadKextWithIdentifier(
    CFStringRef kextIdentifier)
{
    OSReturn      result           = kOSReturnError;
    OSKextLogSpec oldUserLogSpec   = OSKextGetLogFilter(/* kernel? */ false);
    OSKextLogSpec oldKernelLogSpec = OSKextGetLogFilter(/* kernel? */ true);

    if (!kextIdentifier) {
        result = kOSKextReturnInvalidArgument;
        goto finish;
    }

    OSKextSetLogFilter(kOSKextLogSilentFilter, /* kernelFlag */ false);
    OSKextSetLogFilter(kOSKextLogSilentFilter, /* kernelFlag */ true);

    result = OSKextUnloadKextWithIdentifier(kextIdentifier,
        /* terminateServicesAndRemovePersonalities */ TRUE);

finish:

    OSKextSetLogFilter(oldUserLogSpec, /* kernelFlag */ false);
    OSKextSetLogFilter(oldKernelLogSpec, /* kernelFlag */ true);
    return result;
}

/*********************************************************************
* Use this applier function to strip out any info from the kernel
* we don't want to expose in public API.
*********************************************************************/
void _removePrivateKextInfo(
    const void * vKey __unused,
    const void * vValue,
          void * vContext __unused)
{
    CFMutableDictionaryRef kextInfo = (CFMutableDictionaryRef)vValue;
    CFDictionaryRemoveValue(kextInfo, CFSTR("OSBundleMachOHeaders"));
    CFDictionaryRemoveValue(kextInfo, CFSTR("OSBundleLogStrings"));
    return;
}

/*********************************************************************
*********************************************************************/
CFDictionaryRef KextManagerCopyLoadedKextInfo(
    CFArrayRef  kextIdentifiers,
    CFArrayRef  infoKeys)
{
    CFMutableDictionaryRef result     = NULL;
    CFDictionaryRef        kextResult = NULL;  // must release

    kextResult = OSKextCopyLoadedKextInfo(kextIdentifiers, infoKeys);
    if (!kextResult) {
        goto finish;
    }

   /* Copy and remove properties we don't want people to use.
    */
    result = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, kextResult);
    CFDictionaryApplyFunction(result, &_removePrivateKextInfo, /* context */ NULL);

finish:
    SAFE_RELEASE(kextResult);
    return (CFDictionaryRef)result;
}

/*********************************************************************
*********************************************************************/
CFArrayRef _KextManagerCreatePropertyValueArray(
    CFAllocatorRef allocator __unused,
    CFStringRef    propertyKey)
{
    CFMutableArrayRef valueArray = NULL;  // returned
    CFDataRef         xmlData = NULL;  // must release
    CFTypeRef	      cfObj;

    kern_return_t kern_result = kOSReturnError;
    property_key_t property_key = "";  // matches prop_key_t in .defs file

    mach_port_t   kextd_port = MACH_PORT_NULL;

    char      * xml_data = NULL;  // must vm_deallocate()
    natural_t   xml_data_length = 0;

    CFErrorRef error = NULL;  // must release

    if (!propertyKey || PROPERTYKEY_LEN <
	    (CFStringGetMaximumSizeForEncoding(CFStringGetLength(propertyKey),
	    kCFStringEncodingUTF8))) {
        goto finish;
    }

    if (!CFStringGetCString(propertyKey,
        property_key, sizeof(property_key) - 1, kCFStringEncodingUTF8)) {
        goto finish;
    }

    kern_result = get_kextd_port(&kextd_port);
    if (kern_result != kOSReturnSuccess) {
        goto finish;
    }

    kern_result = kextmanager_create_property_value_array(kextd_port,
        property_key, &xml_data, &xml_data_length);
    if (kern_result != kOSReturnSuccess) {
        goto finish;
    }

    xmlData = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (UInt8 *)xml_data,
        xml_data_length, kCFAllocatorNull);
    if (!xmlData) {
        goto finish;
    }

    cfObj = CFPropertyListCreateWithData(kCFAllocatorDefault,
        xmlData, /* options */ 0, /* format */ NULL, &error);
    if (!cfObj) {
        // any point in logging error reason here? nothing caller can do....
        goto finish;
    }

    if (CFGetTypeID(cfObj) != CFArrayGetTypeID()) {
        CFRelease(cfObj);
        goto finish;
    }
    valueArray = (CFMutableArrayRef) cfObj;

finish:

    SAFE_RELEASE(error);
    SAFE_RELEASE(xmlData);

    if (xml_data) {
        vm_deallocate(mach_task_self(), (vm_address_t)xml_data,
            xml_data_length);
    }
    return valueArray;
}

/**********************************************************************
* The following functions are called by sysextd within system extension
* management routines and require including "KextManagerPriv.h".
**********************************************************************/

OSReturn _KextManagerValidateExtension(CFStringRef extPath)
{
    OSReturn        result      = kOSReturnError;
    mach_port_t     kextd_port  = MACH_PORT_NULL;
    CFDataRef       requestData = NULL;  // must release
    CFDictionaryRef requestDict = NULL;  // must release
    CFErrorRef      err         = NULL;  // must release

    result = get_kextd_port(&kextd_port);
    if (result != kOSReturnSuccess) {
        goto error;
    }

    const void *keys[] = { kExtPathKey };
    const void *vals[] = { extPath };
    requestDict = CFDictionaryCreate(
        kCFAllocatorDefault,
        &keys[0],
        &vals[0],
        ARRAY_COUNT(keys),
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );
    if (!requestDict) {
        result = kOSKextReturnNoMemory;
        goto error;
    }

    requestData = CFPropertyListCreateData(kCFAllocatorDefault,
         requestDict, kCFPropertyListBinaryFormat_v1_0,
         /* options */ 0,
         &err);
    if (!requestData) {
        goto error;
    }

    result = kextmanager_validate_ext(kextd_port,
        (char *)CFDataGetBytePtr(requestData),
        CFDataGetLength(requestData));
    if (result != kOSReturnSuccess) {
        goto error;
    }

error:
    SAFE_RELEASE(requestDict);
    SAFE_RELEASE(requestData);
    SAFE_RELEASE(err);
    return result;
}

OSReturn _KextManagerUpdateExtension(CFStringRef extPath, bool extIsEnabled)
{
    OSReturn        result      = kOSReturnError;
    mach_port_t     kextd_port  = MACH_PORT_NULL;
    CFDataRef       requestData = NULL;  // must release
    CFDictionaryRef requestDict = NULL;  // must release
    CFErrorRef      err         = NULL;  // must release

    result = get_kextd_port(&kextd_port);
    if (result != kOSReturnSuccess) {
        goto error;
    }

    const void *keys[] = { kExtPathKey, kExtEnabledKey };
    const void *vals[] = { extPath, extIsEnabled ? kCFBooleanTrue : kCFBooleanFalse };
    requestDict = CFDictionaryCreate(
        kCFAllocatorDefault,
        &keys[0],
        &vals[0],
        ARRAY_COUNT(keys),
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );
    if (!requestDict) {
        result = kOSKextReturnNoMemory;
        goto error;
    }

    requestData = CFPropertyListCreateData(kCFAllocatorDefault,
         requestDict, kCFPropertyListBinaryFormat_v1_0,
         /* options */ 0,
         &err);
    if (!requestData) {
        goto error;
    }

    result = kextmanager_update_ext(kextd_port,
        (char *)CFDataGetBytePtr(requestData),
        CFDataGetLength(requestData));
    if (result != kOSReturnSuccess) {
        goto error;
    }

error:
    SAFE_RELEASE(requestDict);
    SAFE_RELEASE(requestData);
    SAFE_RELEASE(err);
    return result;
}

OSReturn _KextManagerStopExtension(CFStringRef extPath)
{
    OSReturn        result      = kOSReturnError;
    mach_port_t     kextd_port  = MACH_PORT_NULL;
    CFDataRef       requestData = NULL;  // must release
    CFDictionaryRef requestDict = NULL;  // must release
    CFErrorRef      err         = NULL;  // must release

    result = get_kextd_port(&kextd_port);
    if (result != kOSReturnSuccess) {
        goto error;
    }

    const void *keys[] = { kExtPathKey };
    const void *vals[] = { extPath };
    requestDict = CFDictionaryCreate(
        kCFAllocatorDefault,
        &keys[0],
        &vals[0],
        ARRAY_COUNT(keys),
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );
    if (!requestDict) {
        result = kOSKextReturnNoMemory;
        goto error;
    }

    requestData = CFPropertyListCreateData(kCFAllocatorDefault,
         requestDict, kCFPropertyListBinaryFormat_v1_0,
         /* options */ 0,
         &err);
    if (!requestData) {
        goto error;
    }

    result = kextmanager_validate_ext(kextd_port,
        (char *)CFDataGetBytePtr(requestData),
        CFDataGetLength(requestData));
    if (result != kOSReturnSuccess) {
        goto error;
    }

error:
    SAFE_RELEASE(requestDict);
    SAFE_RELEASE(requestData);
    SAFE_RELEASE(err);
    return result;
}

/*********************************************************************
*********************************************************************/
static kern_return_t get_kextd_port(mach_port_t * kextd_port)
{
    kern_return_t kern_result = kOSReturnError;
    mach_port_t   bootstrap_port = MACH_PORT_NULL;

    kern_result = task_get_bootstrap_port(mach_task_self(), &bootstrap_port);
    if (kern_result == kOSReturnSuccess) {
        kern_result = bootstrap_look_up2(bootstrap_port,
                                         (char *)KEXTD_SERVER_NAME,
                                         kextd_port,
                                         0,
                                         BOOTSTRAP_PRIVILEGED_SERVER);
    }

    return kern_result;
}

