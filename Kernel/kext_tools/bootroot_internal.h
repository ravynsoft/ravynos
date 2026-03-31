/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
/*
 * FILE: bootroot_internal.h
 * AUTH: Soren Spies (sspies)
 * DATE: 8 June 2006 (as update_boot.h)
 * DESC: routines for implementing 'kextcache -u' functionality (4252674)
 *       in which bootcaches.plist files get copied to any Apple_Boots
 */

#ifndef _BOOTROOT_INTERNAL_H_
#define _BOOTROOT_INTERNAL_H_

#include <CoreFoundation/CoreFoundation.h>

#include "bootroot.h"

// internal options for "update" operations
typedef enum {
    // BROptsNone          = 0x0,       // in bootroot.h

    // command-line options
    kBRUForceUpdateHelpers = 1 << 0,    // -f: ignore bootstamps, update helpers

    kBRUCachesOnly         = 1 << 1,    // -caches-only: don't update helpers
    kBRUHelpersOptional    = 1 << 2,    // -Installer: helper updates !req'd
    // kBRUExpectUpToDate     = 1 << 3,    // -U: successful updates -> EX_OSFILE (in bootroot.h)
    kBRUEarlyBoot          = 1 << 4,    // -Boot: launch* calling us

    kBRUInvalidateKextcache = 1 << 5,   // -i: mimic sudo touch /S/L/Extensions

    // needUpdates() opt (default is all caches, default-bootable)
    kBRUCachesAnyRoot       = 1 << 6,   // non-default B!=R configs okay

    kBRUImmutableKernel     = 1 << 7,   // -X: build-immutable-kernel

    // copy files opts

    // kBRAnyBootStamps = 0x10000 (1<<16) // in bootroot.h
    // kBRUseStagingDir = 0x20000 (1<<17) // in bootroot.h
} BRUpdateOpts_t;

// in update_boot.c

/*
 * Update all caches and any helper partitions (kextcache -u).
 * Except when kForceUpdateHelpers is specified, unrecognized
 * bootcaches.plist causes immediate success.
 */
int checkUpdateCachesAndBoots(CFURLRef volumeURL, BRUpdateOpts_t flags);

// "put" and "take" let routines decide if a lock is needed (e.g. if no kextd)
// Only used by volume lockers (kextcache, libBootRoot clients, !kextd)
int takeVolumeForPath(const char *volPath);
int putVolumeForPath(const char *path, int status);

#endif  // _BOOTROOT_INTERNAL_H_
