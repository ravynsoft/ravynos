/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitServer.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <IOKit/IOCFSerialize.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <libc.h>
#include <zlib.h>

#include "kextd_main.h"
#include "kextd_personalities.h"
#include "kextd_usernotification.h"
#include "kextd_globals.h"
#include "signposts.h"
#include "staging.h"

static OSReturn sendCachedPersonalitiesToKernel(Boolean resetFlag);

/*******************************************************************************
*******************************************************************************/
OSReturn sendSystemKextPersonalitiesToKernel(
    CFArrayRef kexts,
    Boolean    resetFlag)
{
    OSReturn          result         = kOSReturnSuccess;  // optimistic
    CFArrayRef        personalities  = NULL;    // must release
    CFMutableArrayRef authenticKexts = NULL;    // must release
    CFMutableArrayRef nonsecureKexts = NULL;    // must release
    os_signpost_id_t  spid           = 0;
    CFIndex           count, i;

   /* Note that we are going to finish on success here!
    * If we sent personalities we are done.
    * sendCachedPersonalitiesToKernel() logs a msg on failure.
    */
    result = sendCachedPersonalitiesToKernel(resetFlag);
    if (result == kOSReturnSuccess) {
        goto finish;
    }

    spid = generate_signpost_id();
    os_signpost_interval_begin(get_signpost_log(), spid, SIGNPOST_KEXTD_PERSONALITY_SCRAPE);

   /* If we didn't send from cache, send from the kexts. This will cause
    * lots of I/O.
    */
    if (!createCFMutableArray(&authenticKexts, &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        goto finish;
    }

    if (!createCFMutableArray(&nonsecureKexts, &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        goto finish;
    }

    count = CFArrayGetCount(kexts);
    for (i = 0; i < count; i++) {
        OSKextRef aKext = (OSKextRef)CFArrayGetValueAtIndex(kexts, i);
        /* Since only authenticated personalities can be sent to the kernel, and authentication
         * requires them being in a secure location, staging must happen here if necessary.
         * Generally the kextcache rebuild has already taken care of caching these into the
         * personality cache, but we can't depend on the cache.
         */
        OSKextRef stagedKext = createStagedKext(aKext);
        if (!stagedKext) {
            CFArrayAppendValue(nonsecureKexts, aKext);
            OSKextLogCFString(/* kext */ NULL,
                              kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                              CFSTR("Unable to stage kext for iokit matching: %@"),
                              aKext);
            continue;
        }

        if (OSKextIsAuthentic(stagedKext)) {
            CFArrayAppendValue(authenticKexts, stagedKext);
        }
        SAFE_RELEASE(stagedKext);
    }

    if (CFArrayGetCount(nonsecureKexts) > 0) {
        recordNonsecureKexts(nonsecureKexts);
    }

    result = OSKextSendPersonalitiesOfKextsToKernel(authenticKexts,
        resetFlag);
    if (result != kOSReturnSuccess) {
        goto finish;
    }

    personalities = OSKextCopyPersonalitiesOfKexts(authenticKexts);

   /* Now try to write the cache file. Don't save the return value
    * of that function, we're more concerned with whether personalities
    * have actually gone to the kernel.
    */
    _OSKextWriteCache(OSKextGetSystemExtensionsFolderURLs(),
            CFSTR(kIOKitPersonalitiesKey), gKernelArchInfo,
            _kOSKextCacheFormatIOXML, personalities);

finish:
    if (spid) {
        os_signpost_interval_end(get_signpost_log(), spid, SIGNPOST_KEXTD_PERSONALITY_SCRAPE);
    }
    if (result != kOSReturnSuccess) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
           "Error: Couldn't send kext personalities to the IOCatalogue.");
    } else if (personalities) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogProgressLevel | kOSKextLogIPCFlag |
            kOSKextLogKextBookkeepingFlag,
            "Sent %ld kext personalities to the IOCatalogue.",
            CFArrayGetCount(personalities));
    }
    SAFE_RELEASE(nonsecureKexts);
    SAFE_RELEASE(personalities);
    SAFE_RELEASE(authenticKexts);
    return result;
}

/*******************************************************************************
*******************************************************************************/
static OSReturn sendCachedPersonalitiesToKernel(Boolean resetFlag)
{
    OSReturn  result    = kOSReturnError;
    CFDataRef cacheData = NULL;  // must release

    if (!_OSKextReadCache(gRepositoryURLs, CFSTR(kIOKitPersonalitiesKey),
        gKernelArchInfo, _kOSKextCacheFormatIOXML,
        /* parseXML? */ false, (CFPropertyListRef *)&cacheData)) {

        goto finish;
    }

    OSKextLogCFString(/* kext */ NULL,
        kOSKextLogProgressLevel | kOSKextLogIPCFlag |
        kOSKextLogKextBookkeepingFlag,
        CFSTR("%@"), CFSTR("Sending cached kext personalities to IOCatalogue."));

    result = IOCatalogueSendData(kIOMasterPortDefault,
        resetFlag ? kIOCatalogResetDrivers : kIOCatalogAddDrivers,
        (char *)CFDataGetBytePtr(cacheData), (unsigned int)CFDataGetLength(cacheData));
    if (result != kOSReturnSuccess) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
           "error: couldn't send personalities to the kernel.");
        goto finish;
    }

    OSKextLogCFString(/* kext */ NULL,
        kOSKextLogProgressLevel | kOSKextLogIPCFlag |
        kOSKextLogKextBookkeepingFlag,
        CFSTR("%@"), CFSTR("Sent cached kext personalities to the IOCatalogue."));

    result = kOSReturnSuccess;

finish:
    SAFE_RELEASE(cacheData);
    return result;
}

