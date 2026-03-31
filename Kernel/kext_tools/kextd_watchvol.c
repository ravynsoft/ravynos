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
 * FILE: watchvol.c
 * AUTH: Soren Spies (sspies)
 * DATE: 5 March 2006
 * DESC: watch volumes as they come, go, or are changed, fire rebuilds
 *       NOTE: everything in this file should happen on the main thread
 *
 * Here's roughly how it all works.
 * 1. sign up for diskarb notifications
 * 2. generate a data structure for each incoming comprehensible OS volume
 * 2a. set up notifications for all relevant paths on said volume
 *     [notifications <-> structures]
 * (2) uses bootcaches.plist to describe what caches a system needs.
 *     All top-level keys are assumed required (which means the mkext could
 *     get fancier in the future if an old-fashioned mkext was still okay).
 *     If keys exist that can't be understood or don't parse correctly,
 *     we bail on watching that volume.
 *
 * 3. intelligently respond to notifications
 * 3a. set up a timer to fire so the system has time to settle
 * 3b. upon lazy firing, rebuild caches OR copy files to Apple_Boot
 * 3c. if someone tries to unmount a BootRoot volume, cancel any timer and check
 * 3d. if a locker unlocks happily, cancel any timer and check  (TODO)
 * 3e. if a locker unlocks unhappily, need to force a check of non-caches? (???)
 * 3f. we don't care if the volume is locked; additional kextcaches wait
 * (3d) has the effect that the first kextcache effectively triggers the
 *      second one which copies caches down.  It also allows us ... to be
 *      smart about things like forcing reboots if we booted from staleness.
 *
 * 4. arbitrate kextcache locks
 * 4a. keep a Mach send right to a receive right in the locker
 * 4b. detect crashes via CFMachPortInvalidaton callback
 * 4c. take success information on unlock
 * 4d. if a lock was lost, force a rebuild (XX)?
 *
 * 5. keep structures up to date
 * 5a. clean up when a volume goes away
 * 5b. disappear/appear whenever there's a change
 *
 * 6. reboot stuff: take a big lock; free it only if locker dies
 *
 * given that we read bootcaches.plist, we don't trust anything in it
 * ... but we push the checking off to kextcache, which ensures
 * (via dev_t/safecalls) that it is only operating on a single volume and
 * not being redirected to other volumes, etc.  We have had Security review.
 *
 * To make sure mkexts have the correct owners, kextd enables owners
 * for the duration of kextcache's locks.
 *
 */

// system includes
#include <asl.h>
#include <notify.h>     // note notify_monitor_file below
#include <string.h>     // strerror()
#include <sysexits.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>   // waitpid(2)
#include <stdlib.h>     // daemon(3)
#include <signal.h>
#include <unistd.h>     // e.g. execv

#include <bless.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <DiskArbitration/DiskArbitrationPrivate.h>
#include <IOKit/kext/kextmanager_types.h>
#include <IOKit/storage/CoreStorage/CoreStorageUserLib.h>   // LVGChanged

#include <IOKit/kext/OSKextPrivate.h>
#ifndef kOSKextLogCacheFlag
    #define kOSKextLogCacheFlag kOSKextLogArchiveFlag
#endif  // no kOSKextLogCacheFlag


// notifyd SPI
extern uint32_t notify_monitor_file(int token, const char *name, int flags);
extern uint32_t notify_get_event(int token, int *ev, char *buf, int *len);

// project includes
#include "kextd_main.h"                 // handleSignal()
#include "kextd_watchvol.h"             // kextd_watch_volumes
#include "kextd_globals.h"              // gClientUID
#include "kextd_usernotification.h"     // kextd_raise_notification
#include "kext_tools_util.h"            // logging helpers
#include "bootcaches.h"                 // struct bootCaches
#include "bootroot.h"                   // BRBLLogFunc (only, for now)
#include "kextmanager_async.h"          // lock_*_reply()
#include "safecalls.h"                  // sdeepunlink
#include "signposts.h"                  // signpost helpers


// constants
#define kAutoUpdateDelay    300
#define kWatchKeyBase       "com.apple.system.kextd.fswatch"
#define kWatchSettleTime    5
#define kMaxUpdateFailures  5   // consecutive failures
#define kMaxUpdateAttempts  25  // reset after failure->success
// XXX after 25 successful updates, touching /S/L/E becomes lame
// 6227955 dictates the failure->success reset metric
// should instead detect 25 back-to-back attempts

// 6775045 / 5350761: basic MessageTracer logging
#define kMessageTracerDomainKey     "com.apple.message.domain"
#define kMessageTracerSignatureKey  "com.apple.message.signature"
#define kMessageTracerResultKey     "com.apple.message.result"
#define kMTCachesDomain             "com.apple.kext.caches"
#define kMTUpdatedCaches            "updatedCaches"
#define kMTBeginShutdownDelay       "beginShutdownDelay"
#define kMTEndShutdownDelay         "endShutdownDelay"


// the type: struct watchedVol's (struct bootCaches in bootcaches.h)
// created/destroyed with volumes coming/going; stored in sFsysWatchDict
// use notify_set_state on our notifications to point to these objects
struct watchedVol {
    // CFUUIDRef volUUID;       // DA id (is the key in sFsysWatchDict)
    CFRunLoopTimerRef delayer;  // non-NULL if something is scheduled
    CFMachPortRef lock;         // send right to locker's port
    CFMutableArrayRef waiters;  // reply ports awaiting this volume
    int updterrs;               // bump when locker reports an error
    int updtattempts;           // # times kextcache launched (w/o err->noerr)
    uint32_t origMntFlags;      // mount flags to restore if owners were off
    Boolean isBootRoot;         // should we try to update helpers?

    CFMutableArrayRef tokens;   // notify(3) tokens
    struct bootCaches *caches;  // parsed version of bootcaches.plist
    CFNumberRef index;          // index into notify_set_state token table

    // XX should track the PID of any launched kextcache (5736801)
};

// module-wide data
static DASessionRef            sDASession = NULL;      // learn about volumes
static DAApprovalSessionRef    sDAApproval = NULL;     // retain volumes
static CFMachPortRef           sFsysChangedPort = NULL; // let us know
static CFRunLoopSourceRef      sFsysChangedSource = NULL; // on the runloop
static CFMutableDictionaryRef  sFsysWatchDict = NULL;  // disk ids -> wstruct*s
static CFMutableDictionaryRef  sVolTable = NULL;  // notify tokens -> wstruct*s
static CFMutableDictionaryRef  sReplyPorts = NULL;  // cfports -> replyPorts
static CFMachPortRef           sRebootLock = NULL;     // sys lock for reboot
static CFMachPortRef           sRebootWaiter = NULL;   // only need one
static CFRunLoopTimerRef       sAutoUpdateDelayer = NULL; // avoid boot / movie
static CFIndex                 sNextVolIndex = 0; // the next token value to use

/*
 * For historical reasons (avoiding first boot movie, old kextcache -U
 * race), kextd waits five minutes before performing automatic updates.
 * Once the timer has expiered, vol_appeared() and fsys_changed() will
 * trigger updates as needed.  6276173 tracks removal of the fixed delay.
 *
 * Updates are always performed at shutdown.
 */



// function declarations (kextd_watch_volumes, _stop in watchvol.h)

// ctor/dtors
static struct watchedVol* create_watchedVol(DADiskRef disk);
static void destroy_watchedVol(struct watchedVol *watched);
static CFMachPortRef createWatchedPort(mach_port_t mport, void *ctx);

// volume state
static void vol_appeared(DADiskRef disk, void *ctx);
static void vol_changed(DADiskRef, CFArrayRef keys, void* ctx);
static void vol_disappeared(DADiskRef, void* ctx);
CF_RETURNS_RETAINED static DADissenterRef is_dadisk_busy(DADiskRef, void *ctx);
static Boolean check_vol_busy(struct watchedVol *watched);

// notification processing delay scheme
static void fsys_changed(CFMachPortRef p, void *msg, CFIndex size, void *info);
static void checkScheduleUpdate(struct watchedVol *watched);
static void check_now(CFRunLoopTimerRef timer, void *ctx);    // notify timer cb

// check and act
static Boolean check_rebuild(struct watchedVol*);   // true if launched

// CFMachPort invalidation callback
static void port_died(CFMachPortRef p, void *info);

// helpers for volume and reboot locking routines
static void checkAutoUpdate(CFRunLoopTimerRef t, void *ctx);
static Boolean reconsiderVolumes(mountpoint_t busyVol);
static Boolean checkAllWatched(mountpoint_t busyVol);   // true => work to do

// logging
static void logMTMessage(char *signature, char *result, char *mfmt, ...);

// additional "local" helpers are declared/defined just before use


// utility macros
#define CFRELEASE(x) if (x) { CFRelease(x); x = NULL; }


#if 0
// for testing
#define twrite(msg) write(STDERR_FILENO, msg, sizeof(msg))
static void debug_chld(int signum) __attribute__((unused))
{
    int olderrno = errno;
    int status;
    pid_t childpid;

    if (signum != SIGCHLD)
        twrite("debug_chld not registered for signal\n");
    else
    if ((childpid = waitpid(-1, &status, WNOHANG)) == -1)
        twrite("DEBUG: SIGCHLD received, but no children available?\n");
    else
    if (!WIFEXITED(status))
        twrite("DEBUG: child quit on signal?\n");
    else
    if (WEXITSTATUS(status))
        twrite("DEBUG: child exited with unhappy status\n");
    else
        twrite("DEBUG: child exited with happy status\n");

    errno = olderrno;
}
#endif

static bool isBaseSystemActive(void)
{
    return getenv("__OSINSTALL_ENVIRONMENT") != NULL;
}

static bool isInstallerActive(void)
{
    return get_bootarg("-rootdmg-ramdisk") != NULL;
}

/******************************************************************************
 * kextd_watch_volumes sets everything up (on the current runloop)
 * cleanup of the file-glabal static objects is done in kextd_stop_volwatch()
 *****************************************************************************/
int kextd_watch_volumes(void)
{
    int rval;
    char *errmsg;
    CFRunLoopRef rl;
    const void *keyobjs[2] = { kDADiskDescriptionMediaContentKey,
                              kDADiskDescriptionVolumePathKey };
    CFArrayRef dakeys = NULL;

    rval = EEXIST;
    errmsg = "kextd_watch_volumes() already initialized";
    if (sFsysWatchDict)     goto finish;

    rval = ELAST + 1;
    // the callbacks will want to go digging in here, so set it up first
    errmsg = "couldn't create data structures";
    // sFsysWatchDict keeps track of watched volumes with UUIDs as keys
    sFsysWatchDict = CFDictionaryCreateMutable(nil, 0,
            &kCFTypeDictionaryKeyCallBacks, NULL);  // storing watchedVol*'s
    if (!sFsysWatchDict)    goto finish;

    // sVolTable keeps track of watched volumes with integer IDs as keys
    sVolTable = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
            &kCFTypeDictionaryKeyCallBacks, NULL);
    if (!sVolTable)         goto finish;

    // We keep two ports for a client; one to for death tracking and one to
    // reply when the time comes.  sReplyPorts maps between the CF wrapper
    // for the death-tracking port and the mach_port_t replyPort.
    sReplyPorts = CFDictionaryCreateMutable(nil, 0,
            &kCFTypeDictionaryKeyCallBacks, NULL);  // storing mach_port_t's
    if (!sReplyPorts)       goto finish;

    errmsg = "error setting up ports and sources";
    rl = CFRunLoopGetCurrent();
    if (!rl)    goto finish;

    // in general, being on the runloop means we could be called ...
    // and we are thus careful about our ordering.  In practice, however,
    // we're adding to the current runloop, which means nothing can happen
    // until this routine exits (we're on the one and only thread).

    /*
     * XX need to set up a better match dictionary
     * kDADiskDescriptionMediaWritableKey = true
     * kDADiskDescriptionVolumeNetworkKey != true
     */

    // make sure we have a chance to block unmounts
    errmsg = "couldn't set up diskarb sessions";
    sDAApproval = DAApprovalSessionCreate(nil);
    if (!sDAApproval)  goto finish;
    DARegisterDiskUnmountApprovalCallback(sDAApproval,
        kDADiskDescriptionMatchVolumeMountable, is_dadisk_busy, NULL);
    DAApprovalSessionScheduleWithRunLoop(sDAApproval, rl,kCFRunLoopDefaultMode);

    // set up the disk arrival session & callbacks
    // XX need to investigate custom match dictionaries
    sDASession = DASessionCreate(nil);
    if (!sDASession)  goto finish;
    DARegisterDiskAppearedCallback(sDASession,
            kDADiskDescriptionMatchVolumeMountable, vol_appeared, NULL);
    if (!(dakeys = CFArrayCreate(nil, keyobjs, 2, &kCFTypeArrayCallBacks)))
        goto finish;
    DARegisterDiskDescriptionChangedCallback(sDASession,
            kDADiskDescriptionMatchVolumeMountable, dakeys, vol_changed, NULL);
    DARegisterDiskDisappearedCallback(sDASession,
            kDADiskDescriptionMatchVolumeMountable, vol_disappeared, NULL);

    // we're ready to rumble!
    DASessionScheduleWithRunLoop(sDASession, rl, kCFRunLoopDefaultMode);

    // 5519500: schedule a timer to re-enable autobuilds and reconsider volumes
    // should sign up for IOSystemLoadAdvisory() once it can avoid the movie
    sAutoUpdateDelayer = CFRunLoopTimerCreate(kCFAllocatorDefault,
                            CFAbsoluteTimeGetCurrent() + kAutoUpdateDelay, 0,
                            0, 0, checkAutoUpdate, NULL);
    if (!sAutoUpdateDelayer) {
        goto finish;
    }
    CFRunLoopAddTimer(rl, sAutoUpdateDelayer, kCFRunLoopDefaultMode);
    CFRelease(sAutoUpdateDelayer);        // later self-invalidation will free

    // if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)  goto finish;
    // errmsg = "couldn't set debug signal handler";
    // if (signal(SIGCHLD, debug_chld) == SIG_ERR)  goto finish;

    errmsg = NULL;
    rval = 0;

    // volume notifications should start coming in shortly

finish:
    if (rval) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "kextd_watch_volumes: %s.", errmsg);
        if (rval != EEXIST) kextd_stop_volwatch();
    }

    if (dakeys)     CFRelease(dakeys);

    return rval;
}

/******************************************************************************
 * 5519500: if appropriate, checkAutoUpdate() enables event-driven updates
 * and checks all watched volumes to see if they need updating.
 * It doesn't yet use IOSystemLoadAdvisory() since that doesn't avoid movies.
 *****************************************************************************/
static void checkAutoUpdate(CFRunLoopTimerRef timer, void *ctx)
{
    // assert(timer == sAutoUpdateDelayer)
    mountpoint_t ignore;

    // one-shot timer self-invalidates, so clear out our reference
    // Also, vol_appeared() won't call check_rebuild() until this is clear.
    sAutoUpdateDelayer = NULL;

    // Enable network access for future security checks.
    enableNetworkAuthentication();

    // and check all of the watched volumes for updates
    (void)checkAllWatched(ignore);
}

/******************************************************************************
 * kextd_stop_volwatch unregisters from everything and cleans up
 * - called from watch_volumes to handle partial cleanup
 *****************************************************************************/
// to help clear out sFsysWatch
static void free_dict_item(const void* key, const void *val, void *c)
{
    destroy_watchedVol((struct watchedVol*)val);
}

// public entry point to this module
void kextd_stop_volwatch()
{
    CFRunLoopRef rl;

    // runloop cleanup
    rl = CFRunLoopGetCurrent();
    if (rl && sDASession)   DASessionUnscheduleFromRunLoop(sDASession, rl,
            kCFRunLoopDefaultMode);
    if (rl && sDAApproval)  DAApprovalSessionUnscheduleFromRunLoop(sDAApproval,
            rl, kCFRunLoopDefaultMode);

    // use CFRELEASE to nullify cfrefs in case watch_volumes called again
    if (sDASession) {
        DAUnregisterCallback(sDASession, vol_disappeared, NULL);
        DAUnregisterCallback(sDASession, vol_changed, NULL);
        DAUnregisterCallback(sDASession, vol_appeared, NULL);
        CFRELEASE(sDASession);
    }

    if (sDAApproval) {
        DAUnregisterApprovalCallback(sDAApproval, is_dadisk_busy, NULL);
        CFRELEASE(sDAApproval);
    }

    if (rl && sFsysChangedSource)  CFRunLoopRemoveSource(rl, sFsysChangedSource,
            kCFRunLoopDefaultMode);
    CFRELEASE(sFsysChangedSource);
    CFRELEASE(sFsysChangedPort);

    if (sFsysWatchDict) {
        CFDictionaryApplyFunction(sFsysWatchDict, free_dict_item, NULL);
        CFRELEASE(sFsysWatchDict);
    }

    return;
}

/******************************************************************************
* destroy_watchedVol unregisters any notification tokens and frees
* pieces created in create_watchedVol
******************************************************************************/
static void destroy_watchedVol(struct watchedVol *watched)
{
    CFIndex ntokens;
    int token;
    int errnum;

    // assert that ->delayer, and ->lock have already been cleaned up
    if (watched->tokens) {
        ntokens = CFArrayGetCount(watched->tokens);
        while(ntokens--) {
            token = (int)(intptr_t)CFArrayGetValueAtIndex(watched->tokens,ntokens);
            // XX should take (hacky) steps to insure token is never zero?
            if (/* !token || */ (errnum = notify_cancel(token))) {
                OSKextLog(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogIPCFlag,
          "destroy_watchedVol: error %d canceling notification on vol '%s'.",
                          errnum, watched->caches->root);
            }
        }
        CFRelease(watched->tokens);
    }
    if (watched->caches) {
        destroyCaches(watched->caches);
    }
    if (watched->index) {
        CFDictionaryRemoveValue(sVolTable, watched->index);
    }
    SAFE_RELEASE(watched->index);
    free(watched);
}

/******************************************************************************
* create_watchedVol calls readCaches and creates watch-specific necessities
******************************************************************************/
static struct watchedVol* create_watchedVol(DADiskRef disk)
{
    struct watchedVol *watched, *rval = NULL;
    char *errmsg;

    errmsg = "allocation error";
    watched = calloc(1, sizeof(*watched));
    if (!watched)   goto finish;

    errmsg = NULL;  // no bootcaches.plist, no problem

    watched->caches = readBootCachesForDADisk(disk);    // logs errors
    if (!watched->caches) {
        goto finish;
    }

    watched->isBootRoot = hasBootRootBoots(watched->caches, NULL, NULL, NULL);

    // get rid of any old bootstamps
    if (!watched->isBootRoot) {
        (void)updateStamps(watched->caches, kBCStampsUnlinkOnly);
    }

    // There will be RPS paths, booters, "misc" paths, and the watchedExts folder.
    // For now, we'll just set the array size to 0 and let it grow.
    errmsg = "allocation error";
    watched->tokens = CFArrayCreateMutable(nil, 0, NULL);
    if (!watched->tokens)   goto finish;

    watched->index = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &sNextVolIndex);
    if (!watched->index) {
        goto finish;
    }
    CFDictionarySetValue(sVolTable, watched->index, watched);
    sNextVolIndex++;

    errmsg = NULL;
    rval = watched;     // success!

finish:
    if (errmsg) {
        if (watched && watched->caches && watched->caches->root[0]) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                "%s: %s.", watched->caches->root, errmsg);
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                "create_watchedVol(): %s.", errmsg);
        }
    }
    if (!rval && watched) {
        destroy_watchedVol(watched);
    }

    return rval;
}


// helper: caller must remove port from other structures (e.g. waiters queue)
// The CFRelease() is releasing a CFMachPort created by createWatchedPort().
// While the volume lock waiters queue is a CFArray (which takes its own
// reference), every element has an extra retain count so that we can
// release the non-CF-contained objects here.  The analyzer doesn't know
// that and thinks we shouldn't be releasing those references in the case
// where *port = (CFMachPortRef)CFArrayGetValueAtIndex(watched->waiters,i);
static int
cleanupPort(CFMachPortRef *port)
{
    mach_port_t lport;

    lport = CFMachPortGetPort(*port);

    /*
     * invalidate the port and cancel the dead name notification
     * NOTE: all these operations are synchronized by the main runloop.
     */
    if (CFMachPortIsValid(*port)) {
        /* don't recursively call port_died()! */
        CFMachPortSetInvalidationCallBack(*port, NULL);
        CFMachPortInvalidate(*port);
    }

    if (sReplyPorts) {
        mach_port_t replyPort;
        // The dictionary value associated with '*port' is actually a send-once
        // right that we need to release on a dead name notification
        replyPort = (mach_port_t)(intptr_t)CFDictionaryGetValue(sReplyPorts, *port);
        if (replyPort != MACH_PORT_NULL) {
            // just consume the right - the client has died anyways.
            mach_port_deallocate(mach_task_self(), replyPort);
        }
        CFDictionaryRemoveValue(sReplyPorts, *port);
    }

    CFRelease(*port);
    *port = NULL;

    return mach_port_deallocate(mach_task_self(), lport);
}

// caller responsibile for setting up the lock and cleaning up the waiter
static int signalWaiter(CFMachPortRef waiter, int status)
{
    int rval = KERN_FAILURE;
    mach_port_t replyPort;

    // extract this client's reply port and reply
    replyPort=(mach_port_t)(intptr_t)CFDictionaryGetValue(sReplyPorts,waiter);
    CFDictionaryRemoveValue(sReplyPorts, waiter);

    if (replyPort != MACH_PORT_NULL) {
        if (waiter == sRebootWaiter) {
            mountpoint_t empty;
            rval = lock_reboot_reply(replyPort, KERN_SUCCESS, empty, status);
        } else {
            rval = lock_volume_reply(replyPort, KERN_SUCCESS, status);
        }
    }

    if (rval) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "signalWaiter failed: %s.",
            safe_mach_error_string(rval));
    }
    return rval;
}

// sRebootWaiter must be set; sRebootLock is released if held
static void handleRebootHandoff()
{
    mountpoint_t busyVol;

    // make sure everything is clean
    // (checkAllWatched() will initiate updates if needed)
    if (checkAllWatched(busyVol) == EBUSY) {
        OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogIPCFlag,
                "%s is still busy, delaying reboot.", busyVol);
        goto finish;
    }

    // signal the waiter
    if (signalWaiter(sRebootWaiter, KERN_SUCCESS) == 0) {
        // on success, make the waiter the locker
        sRebootLock = sRebootWaiter;
        OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogIPCFlag,
                "%s up to date; unblocking reboot.", busyVol);
        logMTMessage(kMTEndShutdownDelay, "success", "unblocking shutdown");
    }
    sRebootWaiter = NULL;

finish:
    return;
}

// cleans up watched->lock, checks for waiters, assigns the lock, and signals
static void handleWatchedHandoff(struct watchedVol *watched)
{
    CFMachPortRef waiter = NULL;

    // release existing lock
    if (watched->lock)
        cleanupPort(&watched->lock);

retry:
    // see if we have any waiters
    if (watched->waiters && CFArrayGetCount(watched->waiters)) {
        waiter = (CFMachPortRef)CFArrayGetValueAtIndex(watched->waiters, 0);

        // move waiter into the pole position and remove from the array
        OSKextLog(NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag,
             "granting lock for %s to waiter %d", watched->caches->root,
             CFMachPortGetPort(waiter));
        watched->lock = waiter;     // context already set to 'watched'
        CFArrayRemoveValueAtIndex(watched->waiters, 0);

        // signal the waiter, cleaning up on failure
        if (signalWaiter(waiter, KERN_SUCCESS)) {
            // loop back to try next waiter if this one failed
            cleanupPort(&watched->lock);    // deallocates former waiter
            goto retry;
        }
    }
}

/******************************************************************************
 * vol_appeared checks whether a volume is interesting
 * (note: the first time we see a volume, it's probably not mounted yet)
 * (we rely on vol_changed to call us when the mountpoint actually appears)
 * - signs up for notifications -> creates new entries in our structures
 * - initiates an initial volume check
 *****************************************************************************/
// set up notifications for a single path
static int watch_path(char *path, struct watchedVol* watched)
{
    int rval = ELAST + 1;   // cheesy
    char key[PATH_MAX];
    int token = 0;
    int errnum;
    uint64_t state;
    CFIndex index;
    mach_port_t fsPort = (sFsysChangedPort != NULL) ?
                            CFMachPortGetPort(sFsysChangedPort) : MACH_PORT_NULL;

    // generate key, register for token, monitor, record pointer in token
    if (strlcpy(key, kWatchKeyBase, PATH_MAX) >= PATH_MAX)  goto finish;
    if (strlcat(key, path, PATH_MAX) >= PATH_MAX)  goto finish;

    /* If this is the first notification we're registering for, then ask notifyd
     * for a port and wrap the port up as a source for the runloop */
    if (!sFsysChangedPort) {
        if (notify_register_mach_port(key, &fsPort, 0, &token) != NOTIFY_STATUS_OK) {
            goto finish;
        }

        CFRunLoopRef rl = CFRunLoopGetCurrent();
        if (!rl)    goto finish;
        // change notifications will eventually come in through this port/source
        sFsysChangedPort = CFMachPortCreateWithPort(nil, fsPort, fsys_changed, NULL, NULL);

        // we have to keep these objects so we can unschedule them later?
        if (!sFsysChangedPort)      goto finish;
        sFsysChangedSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault,
                                sFsysChangedPort, 0);
        if (!sFsysChangedSource)    goto finish;
        CFRunLoopAddSource(rl, sFsysChangedSource, kCFRunLoopDefaultMode);

    /* Otherwise, just re-use the port that notifyd already gave us, since
     * everything has already been set up */
    } else if (notify_register_mach_port(key, &fsPort, NOTIFY_REUSE, &token) != NOTIFY_STATUS_OK) {
            goto finish;
    }

    if (!CFNumberGetValue(watched->index, kCFNumberCFIndexType, &index)) goto finish;
    state = (intptr_t)index;
    if (notify_set_state(token, state))  goto finish;
    if (notify_monitor_file(token, path, /* flags; 0 means "all" */ 0)) goto finish;

#if 0
    OSKextLogCFString(NULL,
                      kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                      CFSTR("%s - watching '%s' "),
                      __func__, path);
#endif

    CFArrayAppendValue(watched->tokens, (void*)(intptr_t)token);

    rval = 0;

finish:
    if (rval && token != -1 && (errnum = notify_cancel(token)))
    OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "watch_path: error %d canceling token.", errnum);

    return rval;
}

#define WATCH(watched, fullp, relpath) do { \
        /* MAKEROOTPATH */ \
        COMPILE_TIME_ASSERT(sizeof(fullp) == PATH_MAX); \
        if (strlcpy(fullp, watched->caches->root, PATH_MAX) >= PATH_MAX) \
            goto finish; \
        if (strlcat(fullp, relpath, PATH_MAX) >= PATH_MAX) \
            goto finish; \
        \
        if (watch_path(fullp, watched)) \
            goto finish; \
    } while(0)
#define kInstallCommitPath "/private/var/run/installd.commit.pid"

#if DEV_KERNEL_SUPPORT
/******************************************************************************
 * watch_kernels gets the parent of the kernel file then enumerates the given
 * directory and calls watch_path for each "valid" kernel file in that
 * directory.  NOTE - "valid" currently means files named "kernel.SUFFIX"
 *****************************************************************************/
static void
watch_kernels(struct watchedVol *  watched,
              const char *         kernelPath)
{
    CFURLRef                kernelURL       = NULL; // must release
    CFURLRef                kernelsDirURL   = NULL; // must release
    CFURLEnumeratorRef      myEnumerator    = NULL; // must release
    CFURLRef                enumURL         = NULL; // do not release
    CFStringRef             tempCFString    = NULL; // must release
    CFArrayRef              resultArray     = NULL; // must release
    char                    tempPath[PATH_MAX];

    if (kernelPath == NULL || watched == NULL || watched->caches == NULL) {
        goto finish;
    }

    /* back up to the parent directory and enumerate from there */
    if (strlcpy(tempPath,
                watched->caches->root,
                sizeof(tempPath)) >= sizeof(tempPath))
        goto finish;
    if (strlcat(tempPath,
                kernelPath,
                sizeof(tempPath)) >= sizeof(tempPath))
        goto finish;

    kernelURL = CFURLCreateFromFileSystemRepresentation(
                                                        NULL,
                                                        (const UInt8 *)tempPath,
                                                        strlen(tempPath),
                                                        true );
    if (kernelURL == NULL) {
        goto finish;
    }

    kernelsDirURL = CFURLCreateCopyDeletingLastPathComponent(NULL, kernelURL );
    if (kernelsDirURL == NULL) {
        goto finish;
    }

    // watch /System/Library/Kernels directory
    if (CFURLGetFileSystemRepresentation(kernelsDirURL,
                                         true /*resolve*/,
                                         (UInt8*)tempPath,
                                         sizeof(tempPath)) ) {
        watch_path(tempPath, watched);
    }

    myEnumerator = CFURLEnumeratorCreateForDirectoryURL(
                                                        NULL,
                                                        kernelsDirURL,
                                                        kCFURLEnumeratorDefaultBehavior,
                                                        NULL );
    if (myEnumerator == NULL) {
        goto finish;
    }

    while (CFURLEnumeratorGetNextURL(myEnumerator,
                                     &enumURL,
                                     NULL) == kCFURLEnumeratorSuccess) {
        // make sure we have a last component and it begins with "kernel."
        // NOTE - we are already watching "/S/L/Kernels/kernel"
        SAFE_RELEASE_NULL(tempCFString);
        tempCFString = CFURLCopyLastPathComponent(enumURL);
        if (tempCFString == NULL ||
            CFStringHasPrefix(tempCFString, CFSTR("kernel.")) == false) {
            continue;
        }

        // skip "dSYM" suffix - 18098771
        if (CFStringHasSuffix(tempCFString,
                              CFSTR("dSYM"))) {
            continue;
        }

        // skip any kernels with more than 1 '.' character.
        // For example: kernel.foo.bar is not a valid kernel name
        SAFE_RELEASE_NULL(resultArray);
        resultArray = CFStringCreateArrayWithFindResults(
                                                         NULL,
                                                         tempCFString,
                                                         CFSTR("."),
                                                         CFRangeMake(0, CFStringGetLength(tempCFString)),
                                                         0);
        if (resultArray && CFArrayGetCount(resultArray) > 1) {
            continue;
        }

        if (CFURLGetFileSystemRepresentation(enumURL,
                                             true /*resolve*/,
                                             (UInt8*)tempPath,
                                             sizeof(tempPath)) ) {
            watch_path(tempPath, watched);
        }
    } // while loop

finish:
    SAFE_RELEASE(kernelURL);
    SAFE_RELEASE(kernelsDirURL);
    SAFE_RELEASE(myEnumerator);
    SAFE_RELEASE(tempCFString);
    SAFE_RELEASE(resultArray);

    return;
}
#endif


static void vol_appeared(DADiskRef disk, void *launchCtx)
{
    int result = 0; // for now, ignore inability to get basic data (4528851)
    CFDictionaryRef ddesc = NULL;
    CFURLRef volURL;
    CFBooleanRef traitVal;
    CFUUIDRef volUUID;
    struct watchedVol *watched = NULL;
    Boolean launched = false;
    os_signpost_id_t spid = generate_signpost_id();

    struct bootCaches *caches;
    int i;
    char path[PATH_MAX];

    os_signpost_interval_begin(get_signpost_log(), spid, SIGNPOST_KEXTD_VOLUME_APPEARED);
    OSKextLogCFString(NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag,
                      CFSTR("%s(%@)"), __FUNCTION__, disk);

    // get description so we can see if the disk is writable, etc
    if (!disk)      goto finish;
    ddesc = DADiskCopyDescription(disk);
    if (!ddesc)     goto finish;

    // volUUID is the key in the dictionary (might we already be watching?)
    volUUID = CFDictionaryGetValue(ddesc, kDADiskDescriptionVolumeUUIDKey);
    if (!volUUID || CFGetTypeID(volUUID) != CFUUIDGetTypeID())      goto finish;
    if ((watched = (void *)CFDictionaryGetValue(sFsysWatchDict, volUUID))) {
        // until we can use a real vol_changed() [4620558] to update
        // 'watched', avoid losing critical data for '/'
        if (0 == strcmp(watched->caches->root, "/")) {
            OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogIPCFlag,
                "'/' re-appeared; refusing to lose existing state (8755513) "
                "at the risk of missing minor changes (4620558)");
            goto finish;
        } else {
            // X vol_changed() has already handled removing it if needed?
            OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogIPCFlag,
                "WARNING: vol_appeared() removing pre-existing '%s' from"
                " watch table.", watched->caches->root);
            vol_disappeared(disk, NULL);
        }
    }

    // XX when DA calls vol_appeared(), the volume will always be unmounted ??
    volURL = CFDictionaryGetValue(ddesc, kDADiskDescriptionVolumePathKey);
    os_signpost_event_emit(get_signpost_log(), spid, SIGNPOST_EVENT_VOLUME_URL, "%@", volURL);
    if (!volURL)        goto finish;    // ignore unmounted volumes

    // 7628429: ignore read-only filesystems (which might be on writable media)
    if (CFURLGetFileSystemRepresentation(volURL, false, (UInt8*)path, PATH_MAX)) {
        struct statfs sfs;
        bool isRoot = (strcmp(path, "/") != 0);
        if (statfs(path, &sfs) == -1)       goto finish;
        if ((!isRoot || (isRoot && isBaseSystemActive())) &&
            sfs.f_flags & MNT_RDONLY)       goto finish;    // ignore ro, non-root
        if (sfs.f_flags & MNT_DONTBROWSE)   goto finish;    // respect nobrowse
    } else {
        // couldn't get a path, we don't care about this one
        goto finish;
    }

    // check description traits (XX need custom match dict - 4528851)
    // with fix for 7628429, not clear we need to check this any more
    traitVal = CFDictionaryGetValue(ddesc, kDADiskDescriptionMediaWritableKey);
    if (!traitVal || CFGetTypeID(traitVal) != CFBooleanGetTypeID()) goto finish;
    if (CFEqual(traitVal, kCFBooleanFalse))     goto finish;

    traitVal = CFDictionaryGetValue(ddesc, kDADiskDescriptionVolumeNetworkKey);
    if (!traitVal || CFGetTypeID(traitVal) != CFBooleanGetTypeID()) goto finish;
    if (CFEqual(traitVal, kCFBooleanTrue))      goto finish;


    // does it have a usable bootcaches.plist? (if not, ignore)
    if (!(watched = create_watchedVol(disk)))   goto finish;

    result = -1;    // anything after this is an error
    caches = watched->caches;

    /* for path in { bootcaches.plist, installd.commit.pid, watchedExts, kernel,
     * locSrcs, rpspaths[], booters, miscpaths[] }
     * rpspaths contains mkext, bootconfig; miscpaths the label file
     * cache paths are relative; WATCH() makes absolute */
    WATCH(watched, path, kBootCachesPath);
    WATCH(watched, path, kInstallCommitPath);

    /* support multiple extensions directories - 11860417 */
    char    *bufptr;
    bufptr = caches->watchedExts;
    for (i = 0; i < caches->nWatchedExts; i++) {
        WATCH(watched, path, bufptr);
        bufptr += (strlen(bufptr) + 1);
    }

    // newer systems kernelpath is /System/Library/Kernels/kernel
    // older systems kernelpath is /mach_kernel
    WATCH(watched, path, caches->kernelpath);
#if DEV_KERNEL_SUPPORT
    // look for other kernels and watch them too.
    if (caches->kernelsCount > 0) {
        watch_kernels(watched, caches->kernelpath);

        // watch any other prelinkedkernel files (prelinkedkernel.SUFFIX)
        if (watched->caches->extraKernelCachePaths) {
            for (i = 0; i < watched->caches->nekcp; i++) {
                WATCH(watched, path, caches->extraKernelCachePaths[i].rpath);
           }
        }
    }
#endif

    WATCH(watched, path, caches->locSource);
    WATCH(watched, path, caches->bgImage);
    // XXX commenting out until 9498428 makes watching this file
    // more efficient (and probably replaces locPref with an array).
    // WATCH(watched, path, caches->locPref);

    // loop over RPS paths
    for (i = 0; i < caches->nrps; i++) {
        WATCH(watched, path, caches->rpspaths[i].rpath);
    }

    if (caches->efibooter.rpath[0]) {
        WATCH(watched, path, caches->efibooter.rpath);
    }
    if (caches->ofbooter.rpath[0]) {
        WATCH(watched, path, caches->ofbooter.rpath);
    }

    // loop over misc paths
    for (i = 0; i < caches->nmisc; i++) {
        WATCH(watched, path, caches->miscpaths[i].rpath);
    }

    // we handled any pre-existing entry for volUUID above
    CFDictionarySetValue(sFsysWatchDict, volUUID, watched);

    // if startup delay hasn't yet passed, skip the usual checks & updates
    if (sAutoUpdateDelayer) {
        OSKextLog(NULL, kOSKextLogDetailLevel | kOSKextLogDirectoryScanFlag,
                "ignoring '%s' until boot is complete",
                watched->caches->root);
    } else {
        launched = check_rebuild(watched);
    }

    // reconsiderVolume() uses launchCtx to get its return value
    if (launchCtx) {
        Boolean *didLaunch = launchCtx;
        *didLaunch = launched;
    }

    result = 0;           // we made it

finish:
    if (ddesc)   CFRelease(ddesc);
    os_signpost_event_emit(get_signpost_log(), spid, SIGNPOST_EVENT_VOLUME_WATCHED, "%d", watched != NULL);
    if (result) {
        if (watched) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                 "Error setting up notifications on '%s'.",
                watched->caches->root);
            destroy_watchedVol(watched);
        }
    }
    os_signpost_interval_end(get_signpost_log(), spid, SIGNPOST_KEXTD_VOLUME_APPEARED);
    return;
}


#undef WATCH

/******************************************************************************
 * vol_changed updates our structures if the mountpoint changed
 * - includes the initial mount after a device appears
 * - thus we only call appeared and disappeared as appropriate
 *
 * Multiple notifications fire multiple times as disks come and go.
 * Depending on how predictable it is, we might be able to get away
 * with just using vol_changed to see when the volume is mounted.
 *****************************************************************************/

static void
check_boots_set_nvram(struct watchedVol *watched)
{
    char *volbsdname = watched->caches->bsdname;
    CFArrayRef dparts, bparts;
    Boolean hasBoots;
    Boolean curSDPreferred = true;      // XXX 8011916
    char *s;
    CFStringRef firstPart;
    char bsdName[DEVMAXPATHSIZE];
    BLContext blctx = { 0, BRBLLogFunc, NULL };

    // logs errors
    hasBoots = hasBootRootBoots(watched->caches, &bparts, &dparts, NULL);
    if (!bparts || !dparts)     goto finish;
    s = (CFArrayGetCount(bparts) == 1) ? "" : "s";

    if (hasBoots) {
        if (!watched->isBootRoot) {
            // gained Apple_Boot partition(s)
            OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                        "%s now has Apple_Boot partition%s", volbsdname, s);
            watched->isBootRoot = true;
            check_rebuild(watched);

            // and if it was the root volume, re-point NVRAM
            if (0 == strcmp(watched->caches->root, "/") && curSDPreferred) {
                firstPart = CFArrayGetValueAtIndex(bparts, 0);
                if (!firstPart)     goto finish;
                if (CFStringGetFileSystemRepresentation(firstPart, bsdName,
                                                        DEVMAXPATHSIZE)) {
                    (void)BLSetEFIBootDevice(&blctx, bsdName);
                }
            }
        }
    } else { // !hasBoots
        if (watched->isBootRoot) {
            // lost Apple_Boot partitions
            OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                        "%s lost its Apple_Boot partition%s", volbsdname, s);

            watched->isBootRoot = false;
            (void)updateStamps(watched->caches, kBCStampsUnlinkOnly);
            // nothing new to rebuild

            // if there was only one data partition, make it the boot partition
            if (0 == strcmp(watched->caches->root, "/") && curSDPreferred &&
                    CFArrayGetCount(dparts) == 1) {
                firstPart = CFArrayGetValueAtIndex(dparts, 0);
                if (!firstPart)     goto finish;
                if (CFStringGetFileSystemRepresentation(firstPart, bsdName,
                                                      DEVMAXPATHSIZE)) {
                    (void)BLSetEFIBootDevice(&blctx, bsdName);
                }
            }
        }
    }

finish:
    if (dparts)      CFRelease(dparts);
    if (bparts)      CFRelease(bparts);
}


/******************************************************************************
* diskarb sends lots of notifications about random stuff?
* All mounts & unmounts appear to come through here so we might be
* able to stop registering vol_appeared() and vol_disappeared() with DA:
* vol_appeared() -> consider_vol(), vol_disappeared() -> vol_unmounted()
*
* <rdar://problem/4620558> Boot != Root hardening: real vol_changed() so we don't lose state across volume change
* tracks *updating* a watch instead of destroying it & recreating it.
*
* XX for unmount, is this called before or after the unmount(2)?
* XX need to investigate custom match dictionary for change notifications
*****************************************************************************/
static void vol_changed(DADiskRef disk, CFArrayRef keys, void* ctx)
{
    CFIndex i = CFArrayGetCount(keys);
    CFDictionaryRef ddesc = NULL;
    CFUUIDRef volUUID;
    CFURLRef volURL = NULL;
    struct watchedVol *watched;
    os_signpost_id_t spid = generate_signpost_id();

    os_signpost_interval_begin(get_signpost_log(), spid, SIGNPOST_KEXTD_VOLUME_CHANGED);
    OSKextLogCFString(NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag,
                      CFSTR("%s(%@, keys[0] = %@)"), __FUNCTION__, disk,
                      CFArrayGetValueAtIndex(keys,0));

    if (!disk)     goto finish;
    ddesc = DADiskCopyDescription(disk);
    if (!ddesc)     goto finish;

    volURL = CFDictionaryGetValue(ddesc, kDADiskDescriptionVolumePathKey);
    os_signpost_event_emit(get_signpost_log(), spid, SIGNPOST_EVENT_VOLUME_URL, "%@", volURL);

    // if it doesn't have a UUID, we can't have or set up a watch for it
    volUUID = CFDictionaryGetValue(ddesc, kDADiskDescriptionVolumeUUIDKey);
    if (!volUUID)  goto finish;

    // only already-mounted OS will be watched
    watched = (void*)CFDictionaryGetValue(sFsysWatchDict, volUUID);

    // walk through the properties that have changed
    while (i--) {
        CFTypeRef key = CFArrayGetValueAtIndex(keys, i);

        // check for mount point changes (coming, going, getting a new name)
        if (CFEqual(key, kDADiskDescriptionVolumePathKey)) {
            // until 4620558 is fixed, mountpoint changes = disappear/appear
            // X we could ignore updates that didn't change the value?
            if (watched)
                vol_disappeared(disk, ctx);  // errors to waiters, etc
            if (CFDictionaryGetValue(ddesc, key))
                vol_appeared(disk, ctx);    // already resists changing '/'
        }

        // check for media changes => Apple_Boot -> NVRAM updates
        else if (CFEqual(key, kDADiskDescriptionMediaContentKey)) {
            if (!watched) {
                // maybe it should be; vol_appeared() checks current state
                vol_appeared(disk, ctx);
            } else {
                // updates isBootRoot, sets NVRAM to a new boot device if needed
                check_boots_set_nvram(watched);
            }
        }

        // something else
        else {
            OSKextLogCFString(NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag,
                CFSTR("vol_changed: %@: uninteresting key"), key);
        }
    }

finish:
    if (ddesc)  CFRelease(ddesc);
    os_signpost_interval_end(get_signpost_log(), spid, SIGNPOST_KEXTD_VOLUME_CHANGED);
}

/******************************************************************************
 * vol_disappeared removes entries from the relevant structures
 * - handles forced removal by invalidating the lock
 * - vol_disappeared() called by vol_changed() for unmounts/changes
 *****************************************************************************/
static void vol_disappeared(DADiskRef disk, void* ctx)
{
    // we used to report errors, but we got weird requests (4528851)
    CFDictionaryRef ddesc = NULL;
    CFUUIDRef volUUID;
    CFURLRef volURL = NULL;
    struct watchedVol *watched;
    os_signpost_id_t spid = generate_signpost_id();

    os_signpost_interval_begin(get_signpost_log(), spid, SIGNPOST_KEXTD_VOLUME_DISAPPEARED);
    OSKextLogCFString(NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag,
                      CFSTR("%s(%@)"), __FUNCTION__, disk);

    // if it's not a disk we're watching, we don't need to stop watching it
    if (!disk)      goto finish;
    ddesc = DADiskCopyDescription(disk);
    if (!ddesc)     goto finish;
    volURL = CFDictionaryGetValue(ddesc, kDADiskDescriptionVolumePathKey);
    os_signpost_event_emit(get_signpost_log(), spid, SIGNPOST_EVENT_VOLUME_URL, "%@", volURL);
    volUUID = CFDictionaryGetValue(ddesc, kDADiskDescriptionVolumeUUIDKey);
    if (!volUUID)   goto finish;
    watched = (void*)CFDictionaryGetValue(sFsysWatchDict, volUUID);
    if (!watched)   goto finish;

    if (0 == strcmp(watched->caches->root, "/")) {
        OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogIPCFlag,
                  "[8755513] vol_disappeared() refusing to stop watching '/'");
        goto finish;
    }

    // if the volume is locked, log a warning
    if (watched->lock) {
        OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogIPCFlag,
                  "WARNING: '%s' is locked; abandoning helper process%s",
                  watched->caches->root, watched->waiters ? "es" : "");
    } else {
        OSKextLog(NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag,
                  "ending volume watch of %s", watched->caches->root);
    }

    // take it off the watch list
    CFDictionaryRemoveValue(sFsysWatchDict, volUUID);

    // and in case some action was in progress
    if (watched->delayer) {
        CFRunLoopTimerInvalidate(watched->delayer);     // refcount->0
        watched->delayer = NULL;
    }
    // see if any lockers are waiting
    // (off the list of watched vols so no new requests can come in)
    if (watched->waiters) {
        CFIndex i = CFArrayGetCount(watched->waiters);
        CFMachPortRef waiter;

        while(i--) {
            waiter = (CFMachPortRef)CFArrayGetValueAtIndex(watched->waiters,i);
            signalWaiter(waiter, ENOENT);
            cleanupPort(&waiter);
        }
        CFRelease(watched->waiters);    // should remove all elements
    }

    // no need to toggle owners since the volume is gone

    destroy_watchedVol(watched);    // cancels notifications

finish:
    if (ddesc)  CFRelease(ddesc);
    os_signpost_interval_end(get_signpost_log(), spid, SIGNPOST_KEXTD_VOLUME_DISAPPEARED);
    return;
}

/******************************************************************************
 * is_dadisk_busy lets diskarb know if we'd rather nothing changed
 * note: dissenter callback is called when root initiates an unmount,
 * but the result is ignored.
 *****************************************************************************/
CF_RETURNS_RETAINED
static DADissenterRef
is_dadisk_busy(DADiskRef disk, void *ctx)
{
    int result = 0;     // ignore weird requests for now (4528851)
    DADissenterRef rval = NULL;
    CFDictionaryRef ddesc = NULL;
    CFUUIDRef volUUID;
    struct watchedVol *watched;

    // XX change to kOSKextLogDetailLevel
    OSKextLogCFString(NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag,
                      CFSTR("%s(%@)"), __FUNCTION__, disk);

    // 37135736 - don't dissent any mounts while in the install environment
    // booted into the BaseSystem
    if (isBaseSystemActive()) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogIPCFlag,
            "is_dadisk_busy won't dissent an unmount in the install environment");
        goto finish;
    }

    if (!disk)      goto finish;
    ddesc = DADiskCopyDescription(disk);
    if (!ddesc)     goto finish;
    volUUID = CFDictionaryGetValue(ddesc, kDADiskDescriptionVolumeUUIDKey);
    if (!volUUID)   goto finish;

    // 34665360 - If a reboot / unmount happens exactly 5 minutes after boot,
    // then kextd's auto update timer could fire and race with the unmount of the volume.
    // The unmount(8)'s unmount(2) syscall blocks the stat(2) in check_rebuild() which
    // is called via the 5 minute runloop timer. This would normally be OK, but the
    // unmount also triggers a kext load which results in a system
    // deadlock. Here, we are really getting a hint that the unmount is imminent, so we
    // re-arm the timer to eliminate the race window (assuming the unmount happens within
    // 5 minutes of this is_dadisk_busy call).
    if (sAutoUpdateDelayer) {
	    // cancel the existing timer (access should be serialized by the runloop)
	    CFRunLoopTimerInvalidate(sAutoUpdateDelayer); // refcount -> 0
	    sAutoUpdateDelayer = CFRunLoopTimerCreate(kCFAllocatorDefault,
	                                              CFAbsoluteTimeGetCurrent() + kAutoUpdateDelay,
	                                              0, 0, 0, checkAutoUpdate, NULL);
	    if (sAutoUpdateDelayer) {
		    CFRunLoopAddTimer(CFRunLoopGetCurrent(),
		                      sAutoUpdateDelayer,
		                      kCFRunLoopDefaultMode);
		    CFRelease(sAutoUpdateDelayer); // later self-invalidation will free
	    }
    }

    result = -1;
    watched = (void*)CFDictionaryGetValue(sFsysWatchDict, volUUID);
    if (!watched) {
        // it might have become worth watching while we weren't :?
        vol_appeared(disk, NULL);
        watched = (void*)CFDictionaryGetValue(sFsysWatchDict, volUUID);
    }

    // 5537105 don't prevent unlocked non-BootRoot volumes from unmounting
    if (watched) {
        // 30381691 - file may have been removed as part of APFS update. Don't prevent unmount in that case.
        struct stat    sb;
        CFURLRef url = NULL;
        char *path = NULL;
        char mntonname[PATH_MAX];

        result = -1;

        url = CFDictionaryGetValue(ddesc, kDADiskDescriptionVolumePathKey);
        if (url && CFURLGetFileSystemRepresentation(url, true, (UInt8 *)mntonname, sizeof(mntonname))) {
            result = asprintf(&path, "%s%s", mntonname, "/usr/standalone/bootcaches.plist");
        }

        if (result != -1) {
            result = stat(path, &sb);
            if(result == -1) {
               result = errno;
            }
        }

        if (path) {
            free (path);
        }

        // Check for ENOENT. We expect this if the installer removes the file as part of APFS conversion.
        if (result == ENOENT) {
            goto finish;
        }

        // once 5736801 is fixed, we'll be able to kill kextcache
        if (watched->lock || (watched->isBootRoot && check_vol_busy(watched))) {
            // since we log this, count it as an error so future successes are logged
            if (watched->updterrs == 0)
                watched->updterrs++;
            if (watched->caches)
                OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogCacheFlag,
                    "%s busy; denying eject (%d tries left)", watched->caches->root,
                    kMaxUpdateAttempts - watched->updtattempts);
            rval = DADissenterCreate(nil, kDAReturnBusy, CFSTR("kextmanager busy"));
            if (!rval)      goto finish;
        }
        // Would it be polite to cancel our watch here?
        // We'd need a chance to start again if the eject is ultimately denied.
        // It would open a window when we weren't watching but it is similar
        // to the window before we start watching?
    }

    result = 0;

finish:
    if (result) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "is_dadisk_busy had trouble answering diskarb.");
    }
    // else OSKextLog(/* kext */ NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag, "returning dissenter %p", rval);
    if (ddesc)  CFRelease(ddesc);

    return rval;    // caller releases dissenter if non-null
}

/******************************************************************************
 * check_vol_busy
 * - busy if locked
 * - check_rebuild to check once more (return code indicates if it did anything)
 *****************************************************************************/
static Boolean check_vol_busy(struct watchedVol *watched)
{
    Boolean busy = (watched->lock != NULL);

    if (busy)    goto finish;

    // if not locked and not over error limits, call check_rebuild
    if (watched->updterrs < kMaxUpdateFailures &&
            watched->updtattempts < kMaxUpdateAttempts) {
        busy = check_rebuild(watched);
    } else {
        // over limits; log an error
        OSKextLog(/* kext */ NULL, kOSKextLogWarningLevel | kOSKextLogIPCFlag,
            "%s: giving up; volume has hit max %s", watched->caches->root,
            watched->updterrs >= kMaxUpdateFailures ? "failures" : "update attempts");
    }

finish:
    return busy;
}


/******************************************************************************
 * fsys_changed gets the mach messages from notifyd
 * - schedule a timer (urgency detected elsewhere calls direct, canceling timer)
 *****************************************************************************/
static void
fsys_changed(CFMachPortRef p, void *m, CFIndex size, void *info)
{
    uint64_t nstate;
    CFNumberRef index = NULL;
    struct watchedVol *watched;
    int notify_status = NOTIFY_STATUS_OK;
    int token;
    mach_msg_empty_rcv_t *msg = (mach_msg_empty_rcv_t*)m;

    // msg_id==token -> notify_get_state() -> watchedVol*
    // XX if (token == 0, perhaps a force-rebuild message?)
    token = msg->header.msgh_id;
    notify_status = notify_get_state(token, &nstate);
    if (NOTIFY_STATUS_OK != notify_status) {
        OSKextLogCFString(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            CFSTR("notify_get_state() failed for token %d: status %d."),
            token, notify_status);
        goto finish;
    }

    if ((CFIndex)nstate > sNextVolIndex) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Index with value %lld rejected: higher than current volume index.",
            nstate);
        goto finish;
    }

    index = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &nstate);
    if (!index) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Failed to create volume index with value %lld",
            nstate);
        goto finish;
    }
    // XX should call notify_get_event() here to consume events?
    // how to know when this notification's events have been consumed?
    // filter out useless (generally spurious) events (esp on unmount :P)
    watched = (struct watchedVol *)CFDictionaryGetValue(sVolTable, index);
    if (!watched) {
        OSKextLogCFString(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            CFSTR("NULL entry for notify token %d."),
            token);
        goto finish;
    }

    checkScheduleUpdate(watched);

finish:
    SAFE_RELEASE(index);
    return;
}

static void
checkScheduleUpdate(struct watchedVol *watched)
{
    char path[PATH_MAX];
    struct stat sb;
    FILE *f;

    // ignore unwatched volumes (notification should have been canceled?)
    if (!CFDictionaryGetCountOfValue(sFsysWatchDict, watched)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Invalid volume: %p.", watched);
        goto finish;
    }

    // The goal is to schedule a timer to do the update later, but if
    // the installer is installing, then we'll ignore this change.
    // Changes to the .pid file will call us again.
    if (strlcpy(path,watched->caches->root,sizeof(path)) >= sizeof(path))
        goto finish;
    if (strlcat(path, kInstallCommitPath, sizeof(path)) >= sizeof(path))
        goto finish;
    if ((f = fopen(path, "r"))) {
        pid_t pid;
        int nmatched = fscanf(f, "%d", &pid);
        fclose(f);
        if (nmatched == 1) {
            if (0 == kill(pid, 0)) {
                OSKextLog(NULL,kOSKextLogDetailLevel|kOSKextLogFileAccessFlag,
                          "%s: installer active, ignoring changes",
                          watched->caches->root);
                goto finish;
            }
        }
    }

	// If we're in the RecoveryOS and RAMDisk booted we should never
	// opportunistically update, or else we can end up disenting unmount on
	// the installer.
	// The OSInstaller cannot rely on kInstallCommitPath as that file lives
	// on the data volume, and not the system volume which is being updated.
	if (isBaseSystemActive() && isInstallerActive()) {
		OSKextLog(NULL, kOSKextLogDetailLevel|kOSKextLogFileAccessFlag,
				  "%s: install environment active, ignoring changes",
				  watched->caches->root);
		goto finish;
	}

    // Check for new bootcaches.plist content
    if (strlcpy(path,watched->caches->root,sizeof(path)) >= sizeof(path))
        goto finish;
    if (strlcat(path, kBootCachesPath, sizeof(path)) >= sizeof(path))
        goto finish;
    if (stat(path, &sb) != 0) {
        if (errno != ENOENT) {
            OSKextLogCFString(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                CFSTR("stat failed for %s: %s."), path, strerror(errno));
        }
        // no longer a bootcaches.plist, bail (w/o canceling watch)
        goto finish;
    }

    if (watched->caches->bcTime.tv_sec != sb.st_mtimespec.tv_sec ||
        watched->caches->bcTime.tv_nsec !=sb.st_mtimespec.tv_nsec) {

        // change is afoot in bootcaches.plist
        char * volRoot = watched->caches->root;
        DADiskRef disk = NULL;

        OSKextLog(/* kext */ NULL,
            kOSKextLogBasicLevel | kOSKextLogFileAccessFlag,
            "%s: bootcaches.plist changed.", volRoot);

        // invalidate current watched information
        disk = createDiskForMount(sDASession, volRoot);
        if (!disk) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag | kOSKextLogIPCFlag,
                 "Unable to create DADisk.");
            goto finish;
        }

        // X: doesn't work for '/', but users' bootcaches.plist shouldn't change
        // except from the InstallOS with their '/' as /Volumes/Mac HD. :)
        vol_disappeared(disk, NULL);    // destroys watched (incl sb)
        vol_appeared(disk, NULL);       // checks if rebuild needed
        CFRelease(disk);

    } else if (sAutoUpdateDelayer == NULL) {
        // check whether the startup delay has passed
        // (XX could let 'touch' work before the startup delay has passed?)
        // and prepare to call check_now in a few seconds
        CFRunLoopTimerContext tc = { 0, watched, NULL, NULL, NULL };
        CFAbsoluteTime firetime = CFAbsoluteTimeGetCurrent() + kWatchSettleTime;

        // cancel any existing timer
        if (watched->delayer) {
            CFRunLoopTimerInvalidate(watched->delayer);
        }

        watched->delayer=CFRunLoopTimerCreate(nil,firetime,0,0,0,check_now,&tc);
        if (!watched->delayer) {
            OSKextLogCFString(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                 CFSTR("fsys_changed() unable to create timer for watched volume."));
            goto finish;
        }

        CFRunLoopAddTimer(CFRunLoopGetCurrent(), watched->delayer,
            kCFRunLoopDefaultMode);
        CFRelease(watched->delayer);  // later auto-invalidation will free
    }

finish:
    return;
}

/******************************************************************************
 * check_now, called after the timer expires, calls check_rebuild()
 * It does not look at updterrs because if something changed, we're willing
 * to look at it again.
 *****************************************************************************/
void check_now(CFRunLoopTimerRef timer, void *info)
{
    struct watchedVol *watched = (struct watchedVol*)info;
    // OSKextLog(/* kext */ NULL, kOSKextLogDebugLevel | kOSKextLogGeneralFlag, "DEBUG: check_now(%p): entry", info);

    // is the volume still being watched?
    if (watched && CFDictionaryGetCountOfValue(sFsysWatchDict, watched)) {
        watched->delayer = NULL;        // timer is no longer pending
        (void)check_rebuild(watched);   // don't care what it did
    } else {
        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
            "%p's timer fired when it should have been Invalid", watched);
    }
}
#include "security.h"

/******************************************************************************
 * check_rebuild uses needUpdates() to stat everything -> rebuilds as necessary
 * - kextcache -u used to do all the work (rebuild mkext, boot's etc)
 *
 * XX if kextcache is broken (e.g. a copy of 'false'), updterrs is never
 * incremented and an an infinite reboot stall could result.  updtattempts
 * caps the total number of automatic update attempts.  The locking mechanism
 * serializes (and effectively throttles) the kextcache processes which should
 * prevent a transient failure condition from preventing multiple meaningful
 * attempts to update the volume.
 *****************************************************************************/
static Boolean check_rebuild(struct watchedVol *watched)
{
    Boolean launched            = false;
    Boolean wantRebuild         = false;
#if DEV_KERNEL_SUPPORT
    char *  suffixPtr           = NULL; // must free
    char *  tmpKernelPath       = NULL; // must free
#endif
    os_signpost_id_t spid       = generate_signpost_id();

    os_signpost_interval_begin(get_signpost_log(), spid, SIGNPOST_KEXTD_CHECK_REBUILD);

    // if we came in some other way and there's a timer pending, cancel it
    if (watched->delayer) {
        CFRunLoopTimerInvalidate(watched->delayer);  // runloop holds last ref
        watched->delayer = NULL;
    }

    // make sure this volume isn't out of control with updates
    if (watched->updtattempts > kMaxUpdateAttempts) {
        OSKextLog(/* kext */ NULL, kOSKextLogWarningLevel | kOSKextLogIPCFlag,
            "%s has failed to update %d times, not trying again",
            watched->caches->root, watched->updtattempts);
        goto finish;
    }

    // This check is also in checkScheduleUpdate(), but _kextmanager_lock_reboot can also
    // result in a call to check_rebuild(), which we don't want in the installer.
    if (isBaseSystemActive() && isInstallerActive()) {
        OSKextLog(NULL, kOSKextLogDetailLevel|kOSKextLogFileAccessFlag,
                  "%s: install environment active, ignoring changes",
                  watched->caches->root);
        goto finish;
    }

    // stat stuff to see if a rebuild is needed
    // (it was 'goto' or ever-deeper nesting): https://www.xkcd.com/292/
    if (check_kext_boot_cache_file(watched->caches,
                                   watched->caches->kext_boot_cache_file->rpath,
                                   watched->caches->kernelpath, NULL)) {
        if (isPrelinkedKernelAutoRebuildDisabled()) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogGeneralFlag | kOSKextLogBasicLevel,
                      "Skipping prelinked kernel auto rebuild; kext-dev-mode setting.");
        }
        else {
            wantRebuild = true;
            goto dorebuild;
        }
    }

#if DEV_KERNEL_SUPPORT
    if (watched->caches->extraKernelCachePaths) {
        int             i;
        cachedPath *    cp;

        tmpKernelPath = malloc(PATH_MAX);
        if (tmpKernelPath == NULL) {
            OSKextLog(NULL, kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                "Could not allocate memory for tmpKernelPath!");
            goto finish;
        }

        for (i = 0; i < watched->caches->nekcp; i++) {

            cp = &watched->caches->extraKernelCachePaths[i];
            SAFE_FREE_NULL(suffixPtr);

            suffixPtr = getPathExtension(cp->rpath);
            if (suffixPtr == NULL)
                continue;
            if (strlcpy(tmpKernelPath, watched->caches->kernelpath, PATH_MAX) >= PATH_MAX)
                continue;
            if (strlcat(tmpKernelPath, ".", PATH_MAX) >= PATH_MAX)
                continue;
            if (strlcat(tmpKernelPath, suffixPtr, PATH_MAX) >= PATH_MAX)
                continue;
            if (check_kext_boot_cache_file(watched->caches,
                                           cp->rpath,
                                           tmpKernelPath, NULL)) {
                if (isPrelinkedKernelAutoRebuildDisabled()) {
                    OSKextLog(/* kext */ NULL,
                              kOSKextLogGeneralFlag | kOSKextLogBasicLevel,
                              "Skipping prelinked kernel auto rebuild; kext-dev-mode setting.");
                }
                else {
                    wantRebuild = true;
                    goto dorebuild;
                }
            }
        }
    }
#endif

    // updates isBootRoot and modifies NVRAM if needed
    check_boots_set_nvram(watched);

    // Boot!=Root-specific caches
    // 16513211 - we no longer call check_loccache here since those resources
    // are not critical, they will get updated on explicit kextcache calls when
    // they are out of date.
    if (watched->isBootRoot) {
        wantRebuild =
        check_csfde(watched->caches) ||
        needUpdates(watched->caches, kBRUCachesAnyRoot,
                    NULL, NULL, NULL,   // use return aggregate
                    kOSKextLogProgressLevel | kOSKextLogFileAccessFlag);
    }

dorebuild:
    if (wantRebuild) {
        if (launch_rebuild_all(watched->caches->root, false, false) > 0) {
            launched = true;
            watched->updtattempts++;
        } else {
            watched->updterrs++;
            OSKextLog(NULL, kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                "Error launching kextcache -u.");
        }
    }

    if (0 == strcmp(watched->caches->root, "/") &&
            plistCachesNeedRebuild(gKernelArchInfo)) {
        rescanExtensions();
    }

finish:
#if DEV_KERNEL_SUPPORT
    SAFE_FREE(tmpKernelPath);
    SAFE_FREE(suffixPtr);
#endif
    os_signpost_interval_end(get_signpost_log(), spid, SIGNPOST_KEXTD_CHECK_REBUILD);
    return launched;
}


// ---- locking services (prototyped via MiG and kextmanager[_mig].defs) ----

/******************************************************************************
 * kextmanager_lock_reboot ensures "all clean" (used by shutdown(8), reboot(8))
 *****************************************************************************/
// iterator helper locking for locked or should-be-locked volumes
static void check_locked(const void *key, const void *val, void *ctx)
{
    struct watchedVol *watched = (struct watchedVol*)val;
    // pointer to the mountpoint_t at the other end
    char *busyVol = ctx;

    // report this one if:
    // it's already locked or if it needs a rebuild
    // check_vol_busy() ensures checks for excessive errors
    if (check_vol_busy(watched)) {
        strlcpy(busyVol, watched->caches->root, sizeof(mountpoint_t));
    }
}

static void watchedPortInvalidate_cb(CFMachPortRef port_ref, void *info)
{
    /*
     * When a dead name notification is received, this callback will happen on
     * the main thread, *not* on the special mach port queue setup by the
     * CFMachPort* APIs. This synchronizes the port_died() operation with
     * another call to cleanupPort() by setting the invalidation callback to
     * NULL in cleanupPort (called from port_died).
     */
    port_died(port_ref, info);
}

// create a CFMachPort with dead name notifications that call port_died
static CFMachPortRef
createWatchedPort(mach_port_t mport, void *ctx)
{
    CFMachPortRef port_ref = NULL;
    int result = ELAST + 1;
    CFMachPortContext mp_ctx = {};

    mp_ctx.info = ctx;
    port_ref = CFMachPortCreateWithPort(nil, mport, NULL, &mp_ctx, false);
    if (!port_ref) {
        goto finish;
    }

    CFMachPortSetInvalidationCallBack(port_ref, watchedPortInvalidate_cb);

    result = 0;

finish:
    if (result && port_ref) {
        CFRelease(port_ref);
        port_ref = NULL;
    }

    return port_ref;
}

static Boolean
checkAllWatched(mountpoint_t busyVol)
{
    int result;

    // if we've contacted diskarb, scan the dictionary for locked items
    busyVol[0] = '\0';
    if (sFsysWatchDict) {
        CFDictionaryApplyFunction(sFsysWatchDict, check_locked, busyVol);
    }
    if (busyVol[0] == '\0') {
        result = 0;     // you got it!
    } else {
        // busyVol (at least) was locked, try again later
        result = EBUSY;
    }

    return result;
}


/******************************************************************************
 * MIG Server Routine
 * _kextmanager_lock_reboot can block reboot if caches are not yet up to date
 *****************************************************************************/
kern_return_t _kextmanager_lock_reboot(mach_port_t p, mach_port_t replyPort,
    mach_port_t client, int waitForLock, mountpoint_t busyVol, int *busyStatus)
{
    kern_return_t rval = KERN_FAILURE;
    int result = ELAST + 1;
    // OSKextLog(/* kext */ NULL, kOSKextLogDebugLevel | kOSKextLogGeneralFlag, "DEBUG: _lock_reboot(..%d/%d..)...", client, replyPort);

    if (!busyStatus) {
        result = EINVAL;
        rval = KERN_SUCCESS;    // for MiG
        goto finish;
    }

    if (gClientUID != 0) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Non-root doesn't need to lock for reboot.");
        result = EPERM;
        rval = KERN_SUCCESS;    // for MiG
        goto finish;
    }

    // shutdown/reboot proceed on result == EALREADY
    if (sRebootLock) {
        result = EALREADY;
        rval = KERN_SUCCESS;    // for MiG
        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogIPCFlag,
                "Warning: Reboot lock request while reboot in progress.");
        goto finish;
    }

    // check all the volumes we are watching and
    // if any new volumes have become eligible
    if (checkAllWatched(busyVol) || reconsiderVolumes(busyVol)) {
        result = EBUSY;
        rval = KERN_SUCCESS;    // for MiG
    } else {
        // great, this guy gets to take the uber reboot lock
        if (!(sRebootLock = createWatchedPort(client, &sRebootLock)))
            goto finish;
        result = 0;             // success
        rval = KERN_SUCCESS;    // for MiG
    }

    // should we reply now or later?
    if (waitForLock && result == EBUSY && sRebootWaiter == NULL) {
        // client will block until we reply with lock or failure
        // [&sRebootLock is context for all interested in the reboot lock]
        if (!(sRebootWaiter = createWatchedPort(client, &sRebootLock)))
            goto finish;

        // stash reply port so we can get it later (X someday dual-use port?)
        CFDictionarySetValue(sReplyPorts, sRebootWaiter, (void*)(intptr_t)replyPort);
        rval = MIG_NO_REPLY;    // for MiG; no result
    } else {
        // we reply to the client if: a. failure other than EBUSY,
        // b. the client won't wait, or c. we've no way to track him.
        rval = KERN_SUCCESS;        // MiG will return to the caller
    }

finish:
    if (rval == KERN_SUCCESS) {
        if (busyStatus) *busyStatus = result;
        if (result != 0) {
            // consume the send-right provided by MIG, but only when we
            // haven't already consumed the reference. We conly consume the
            // reference via createWatchedPort(), and on success those paths
            // set 'result = 0'
            mach_port_deallocate(mach_task_self(), client);
	}
    } else if (rval != MIG_NO_REPLY) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Error %d locking for reboot.", rval);
    }

    // pop up a dialog if reboot is going to stall
    if (result == EBUSY && waitForLock) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogFileAccessFlag | kOSKextLogIPCFlag,
            "'%s' updating, delaying reboot.", busyVol);

        // 6775045 / 5350761: basic MessageTracer logging
        logMTMessage(kMTBeginShutdownDelay, "failure",
                "kext caches need update at shutdown; delaying");
    }

    return rval;
}

/******************************************************************************
 * MIG Server Routine
 * _kextmanager_lock_volume locks volumes for kextcache
 * - vol_uuid is in CFUUIDBytes
 *****************************************************************************/
kern_return_t _kextmanager_lock_volume(mach_port_t p, mach_port_t replyPort,
    mach_port_t client, uuid_t vol_uuid, int waitForLock, int *lockStatus)
{
    kern_return_t rval = KERN_FAILURE;
    int result = EINVAL;
    CFUUIDBytes uuidBytes;
    CFUUIDRef volUUID;
    struct watchedVol *watched = NULL;
    struct statfs sfs;
    Boolean createdLock = false;

    OSKextLog(/* kext */ NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag,
              "%s(..%d..)...", __FUNCTION__, client);

    if (!lockStatus) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "kextmanager_lock_volume requires non-NULL lockStatus.");
        return KERN_INVALID_ARGUMENT;    // just return
    }

    if (gClientUID != 0 /*watched->fsinfo->f_owner ?*/) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Non-root doesn't need to lock or unlock volumes.");
        result = EPERM;
        rval = KERN_SUCCESS;    // for MiG
        goto finish;
    }

    // still initializing, sorry (XX someday could allow client to wait)
    if (!sFsysWatchDict) {
        result = EAGAIN;
        rval = KERN_SUCCESS;    // for MiG
        goto finish;
    }

    // rebooting -> no more update locks!
    if (sRebootLock) {
        OSKextLog(NULL, kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                  "reboot in progress -> denying volume lock request");
        result = ENOLCK;
        rval = KERN_SUCCESS;    // for MiG
        goto finish;
    }

    memcpy(&uuidBytes.byte0, vol_uuid, sizeof(uuid_t));
    volUUID = CFUUIDCreateFromUUIDBytes(nil, uuidBytes);
    if (!volUUID) {
        result = ENOMEM;
        goto finish;
    }
    watched = (void*)CFDictionaryGetValue(sFsysWatchDict, volUUID);
    // would release volUUID here except we want to log it ...
    // if kextd isn't watching, locking is less critical (->Basic)
    if (!watched) {
        OSKextLogCFString(NULL, kOSKextLogBasicLevel | kOSKextLogIPCFlag,
                  CFSTR("not watching %@ -> no volume lock to grant"), volUUID);
        CFRelease(volUUID);
        result = ENOENT;
        rval = KERN_SUCCESS;    // for MiG
        goto finish;
    } else {    // clarify no double release
        CFRelease(volUUID);
    }

    // if not locked, grant the lock
    if (watched->lock == NULL) {
        // create lock
        if (!(watched->lock = createWatchedPort(client, watched))) {
            rval = KERN_FAILURE;        // also the default above
            goto finish;
        }
        createdLock = true;
        // try to enable owners if not currently honored (XX ignores failure)
        if (statfs(watched->caches->root, &sfs) == 0) {
            uint32_t mntgoal = sfs.f_flags;
            mntgoal &= ~(MNT_IGNORE_OWNERSHIP);
            if (sfs.f_flags != mntgoal) {
                (void)updateMount(watched->caches->root, mntgoal);  // logs
                watched->origMntFlags = sfs.f_flags;
            }
        }
        OSKextLog(NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag,
             "granting lock for %s to %d", watched->caches->root,client);
        result = 0;             // success; lock granted
        rval = KERN_SUCCESS;    // for MiG
        // if rval changes after this point, we need to cleanup watched->lock,
        // but we would need to do it carefully because cleanupPort() also
        // deallocates the associated send right, and if we return an error
        // from this function (rval != KERN_SUCCESS) then MIG will also try to
        // deallocate the same send right! Fortunately, right now we don't
        // modify rval after this point.
    } else {
        // lock can't be granted; let the client wait if willing
        if (waitForLock) {
            rval = MIG_NO_REPLY;    // for MiG; no result
        } else {
            result = EBUSY;         // for client
            rval = KERN_SUCCESS;    // for MiG
        }
    }

    // if we're not replying yet, add client to the wait queue
    if (rval == MIG_NO_REPLY) {
        CFMachPortRef waiter;

        // create waiter array (of CFMachPortRefs) if needed
        if (!watched->waiters) {
            watched->waiters=CFArrayCreateMutable(0,1,&kCFTypeArrayCallBacks);
            if (!watched->waiters) {
                rval = KERN_FAILURE;
                goto finish;
            }
        }

        // create waiter and insert into array
        if (!(waiter = createWatchedPort(client, watched))) {
            rval = KERN_FAILURE;
            goto finish;
        }

        // store waiter, replyPort
        // cleanupPort() CFRelease()s all waiter objects
        CFArrayAppendValue(watched->waiters, waiter);
        CFDictionarySetValue(sReplyPorts, waiter, (void*)(intptr_t)replyPort);

        // success: rval remains MIG_NO_REPLY
    }


finish:
    if (rval == KERN_SUCCESS) {
        *lockStatus = result;
        if (result != 0) {
            // consume the send-right provided by MIG, but only when we
            // haven't already consumed the reference. We conly consume the
            // reference via createWatchedPort(), and on success those paths
            // set 'result = 0'
            mach_port_deallocate(mach_task_self(), client);
        }
    } else if (rval == MIG_NO_REPLY) {
        // not replying yet
    } else if (watched) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Error %d locking '%s'.", rval, watched->caches->root);
    } else {
        OSKextLog(NULL, kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Error %d attempting to lock an un-watched volume.", rval);
    }

    return rval;
}

/******************************************************************************
 * MIG Server Routine
 * _kextmanager_unlock_volume unlocks for clients (i.e. kextcache)
 *****************************************************************************/
kern_return_t _kextmanager_unlock_volume(mach_port_t p, mach_port_t client,
    uuid_t vol_uuid, int exitstatus)
{
    kern_return_t rval = KERN_FAILURE;
    CFUUIDRef volUUID = NULL;
    struct watchedVol *watched = NULL;
    CFUUIDBytes uuidBytes;
    // OSKextLog(/* kext */ NULL, kOSKextLogDebugLevel | kOSKextLogGeneralFlag, "DEBUG: _kextmanager_unlock_volume()...");

    if (gClientUID != 0 /*watched->fsinfo->f_owner ?*/) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Non-root doesn't need to lock or unlock volumes.");
        rval = KERN_SUCCESS;
        goto finish;
    }

    // make sure we're set up
    if (!sFsysWatchDict)  {
        goto finish;
    }

    memcpy(&uuidBytes.byte0, vol_uuid, sizeof(uuid_t));
    volUUID = CFUUIDCreateFromUUIDBytes(nil, uuidBytes);
    if (!volUUID) {
        goto finish;
    }
    watched = (void*)CFDictionaryGetValue(sFsysWatchDict, volUUID);
    if (!watched) {
        goto finish;
    }

    if (!watched->lock) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "'%s' isn't locked.", watched->caches->root);
        goto finish;
    }

    if (client != CFMachPortGetPort(watched->lock)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "%d didn't lock '%s'.", client, watched->caches->root);
        goto finish;
    }

    // update error accounting
    if (exitstatus == EX_OK) {
        logMTMessage(kMTUpdatedCaches, "success",
                     /* could be a helper, but this string is reported */
                     "kextcache updated kernel boot caches");
        if (watched->updterrs > 0) {
            // previous error logs -> put a reassuring message in the log
            OSKextLog(NULL, kOSKextLogDebugLevel | kOSKextLogCacheFlag,
                "helper (%d) successfully updated %s",
                client, watched->caches->root);
            watched->updterrs = 0;
            watched->updtattempts = 0;
        }
    } else {
        // not okay
        watched->updterrs++;
        OSKextLog(NULL, kOSKextLogErrorLevel | kOSKextLogCacheFlag,
            "helper error while updating %s (error count: %d)",
            watched->caches->root, watched->updterrs);
    }

    OSKextLog(NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag,
             "%d unlocked %s", client, watched->caches->root);

    // disable owners if we enabled them for the locker (updateMount() logs)
    if (watched->origMntFlags) {
        (void)updateMount(watched->caches->root, watched->origMntFlags);
        watched->origMntFlags = 0;
    }

    // if kextcache failed, handleRebootHandoff() will fire off another
    // but only if updterrs is less than the failure limit
    handleWatchedHandoff(watched);
    if (!sRebootLock && sRebootWaiter)
        handleRebootHandoff();

    // once upon a time, we thought we could save five seconds here
    // instead, we will just call kextcache -u and it will build the mkext

    rval = KERN_SUCCESS;

finish:
    if (volUUID) {
        CFRelease(volUUID);
    }
    if (rval && watched) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Couldn't unlock '%s'.", watched->caches->root);
    }
    if (rval == KERN_SUCCESS) {
        // release the send right given to us by MIG
        // (we didn't do anything with 'client' in this function)
        mach_port_deallocate(mach_task_self(), client);
    }

    return rval;
}

#if 0
{
mach_port_urefs_t refs = 0xff;
OSKextLog(/* kext */ NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag, "DEBUG: mach_port_get_refs(..%d, send..): %x -> %x ref(s).", client,
mach_port_get_refs(mach_task_self(), client, MACH_PORT_RIGHT_SEND, &refs),refs);
}
#endif

/******************************************************************************
* port_died() tells us when a tracked send right goes away. It is called
* from a dispatch event handler for DEAD_NAME notifications. When this
* function is called, it assumes that the dead name notification has added an
* extra send right to the mach port passed in.
*
* We track send rights (on the client ports passed to us) as long as we
* have resources allocated to those clients.  If they die, we get notified
* that the send right went away and then we clean up the associated resource.
*
* NOTE:
* This function should only be called when shutdown/reboot exits before kextd
* or when a kextcache process is terminated against its will. It should also
* only be called in the context of a DEAD_NAME notification. Please do not
* call this function from any other context!
*
* If the client explicitly deallocates its *receive* right / port while we are
* tracking the corresponding send right, port_died() is also called, though
* kextcache should unlock the volume before doing that.
*****************************************************************************/
static void port_died(CFMachPortRef cfport, void *info)
{
    mach_port_t mport = cfport ? CFMachPortGetPort(cfport) : MACH_PORT_NULL;
    struct watchedVol* watched;
    // OSKextLog(/* kext */ NULL, kOSKextLogDebugLevel | kOSKextLogIPCFlag, "DEBUG: port_died(%p/%d): entry.", cfport, mport);

    // all watched-associated ports should have context
    if (!info || !cfport) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "port_died() fatal error: invalid data.");
        goto finish;
    }


    // only means of release for reboot lock
    if (info == &sRebootLock) {
        if (cfport == sRebootLock) {
            // reboot/shutdown happened to exit before kextd
            // XX start timer now or when reboot lock granted?
            cleanupPort(&sRebootLock);
        } else if (cfport == sRebootWaiter) {
            cleanupPort(&sRebootWaiter);        // gave up waiting
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                "Improperly tracked shutdown/reboot process died.");
        }
        goto finish;
    }


    // else ... handle volume lockers and waiters

    watched = (struct watchedVol*)info;

    // vol_disappeared() removes the volume from sFsysWatchDict before
    // cleaning up all the waiters, so unless the runloop somehow jumped
    // over here from the midst of that callout, no ports affiliated
    // with missing watchVol*'s should be dying.  Even multiple
    // reboot waiters would all have the same context and the mach
    // port check above would catch them.
    if (CFDictionaryGetCountOfValue(sFsysWatchDict, watched) == 0) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Warning: missing context for deallocated helper port.");
        cleanupPort(&cfport);
        goto finish;
    }

    // watched points to valid data ... was it the locker?
    if (watched->lock && mport == CFMachPortGetPort(watched->lock)) {
        // try to disable owners if we enabled them for the locker
        if (watched->origMntFlags) {
            (void)updateMount(watched->caches->root, watched->origMntFlags);
            watched->origMntFlags = 0;
        }

        // if locked, is anyone waiting?
        if (watched->lock) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                "helper pid %d exited without unlocking '%s'.",
                CFMachPortGetPort(watched->lock), watched->caches->root);
            watched->updterrs++;
            handleWatchedHandoff(watched);      // cleans up watched->lock
            if (!sRebootLock && sRebootWaiter)
                handleRebootHandoff();
        }
        // if we were storing the worker pid, we might clean it up here
        // see also handleSignalInRunloop() over in kextd_main.c

    } else {    // it must have been a waiter
        CFIndex i;
        if (!watched->waiters) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogWarningLevel | kOSKextLogIPCFlag,
                "Warning: presumed waiter died, but no waiters.");
            goto finish;
        }

        for (i = CFArrayGetCount(watched->waiters); i-- > 0;) {
            CFMachPortRef waiter;

            waiter = (CFMachPortRef)CFArrayGetValueAtIndex(watched->waiters,i);
            if (mport == CFMachPortGetPort(waiter)) {
                cleanupPort(&waiter);       // --retainCount
                // (dropping the "extra" reference left by createWatchedPort() in
                // _kextmanager_lock_volume().  CFArray doesn't require the extra
                // reference, but we also use this for watched->lock, sReboot*, etc.
                CFArrayRemoveValueAtIndex(watched->waiters, i); // release
                goto finish;      // success
            }
        }

        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogIPCFlag,
            "Warning: %s: unknown helper exited.", watched->caches->root);
    }

finish:
    if (mport != MACH_PORT_NULL) {
        /* the DEAD_NAME notification comes with an extra send right */
        mach_port_deallocate(mach_task_self(), mport);
    }

    return;
}


/******************************************************************************
 * reconsiderVolumes() iterates the mount list, checking to see if any
 * local mounts have become interesting.  It uses
 * reconsiderVolume() which calls vol_appeared on any we aren't yet watching.
 * If any newly added one needed an update, busyVol is set to its mountpoint.
 * Used after kAutoUpdateDelay to initialize things and to detect OS copies
 * at shutdown time.
 *****************************************************************************/
static Boolean reconsiderVolume(mountpoint_t volToCheck)
{
    int result;
    Boolean rval = false;
    DADiskRef disk = NULL;
    CFDictionaryRef dadesc = NULL;
    CFUUIDRef volUUID;
    struct watchedVol *watched;

    // if it's unknown to diskarb, we can't do much with it (no error)
    result = 0;
    disk = createDiskForMount(sDASession, volToCheck);
    if (!disk)      goto finish;

    result = -1;
    dadesc = DADiskCopyDescription(disk);
    if (!dadesc)    goto finish;
    volUUID = CFDictionaryGetValue(dadesc, kDADiskDescriptionVolumeUUIDKey);
    if (!volUUID)   goto finish;

    watched = (void*)CFDictionaryGetValue(sFsysWatchDict, volUUID);
    if (!watched) {
        // if not watched, check to see if it now has a known OS
        vol_appeared(disk, &rval);      // handles any updates, etc
    } else {
        // if watched, let vol_changed check on its Apple_Boot status
        const void *objs[1] = { kDADiskDescriptionMediaContentKey };
        CFArrayRef  keys;

        keys = CFArrayCreate(nil,objs,1,&kCFTypeArrayCallBacks);
        if (!keys)      goto finish;

        vol_changed(disk, keys, NULL);
        CFRelease(keys);
    }

    result = 0;     // boolean rval is set by call to vol_appeared()

finish:
    if (disk)       CFRelease(disk);
    if (dadesc)     CFRelease(dadesc);
    if (result) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
            "Error reconsidering volume %s.", volToCheck);
    }

    return rval;
}

static Boolean
reconsiderVolumes(mountpoint_t busyVol)
{
    Boolean rval = false;
    char *errmsg = NULL;
    int nfsys, i;
    size_t bufsz;
    struct statfs *mounts = NULL;

    // if not set up ...
    if (!sDASession)        goto finish;

    errmsg = "Error while getting mount list.";
    if (-1 == (nfsys = getfsstat(NULL, 0, MNT_NOWAIT))) goto finish;
    bufsz = nfsys * sizeof(struct statfs);
    if (!(mounts = malloc(bufsz)))                  goto finish;
    if (-1 == getfsstat(mounts, (int)bufsz, MNT_NOWAIT)) goto finish;

    errmsg = NULL;  // let reconsiderVolume() take it from here
    for (i = 0; i < nfsys; i++) {
        struct statfs *sfs = &mounts[i];

        if (sfs->f_flags & MNT_LOCAL && strcmp(sfs->f_fstypename, "devfs")) {
            if (reconsiderVolume(sfs->f_mntonname) && !rval) {
                // only capture first volume, but check them all
                strlcpy(busyVol, sfs->f_mntonname, sizeof(mountpoint_t));
                rval = true;
            }
        }
    }

    errmsg = NULL;

finish:
    if (errmsg) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
            "%s", errmsg);
    }
    if (mounts)     free(mounts);

    return rval;
}

/*
 * helpers to deal with I/O Kit notifications, bootstamps removal
 */
#define kBRNormalUpdate false
#define kBRInvalidateStamps true        // cf. kBRUForceUpdateHelpers
static void
updateVolForMedia(const void *object, char *mediaDesc, int matchSize,
                  CFStringRef matchingKeys[], CFTypeRef matchingValues[],
                  bool invalidateStamps)
{
    char * errorMessage = NULL;

    CFDictionaryRef matchPropertyDict = NULL;
    CFMutableDictionaryRef matchingDict = NULL;
    io_service_t theMedia = MACH_PORT_NULL;
    DADiskRef dadisk = NULL;
    CFDictionaryRef dadesc = NULL;
    CFUUIDRef volUUID;          // part of dadesc; not released
    struct watchedVol * watched = NULL;  // do not free

    // nothing to do if we're not watching yet
    if (!sFsysWatchDict)    goto finish;

    errorMessage = "change notification missing object.";
    if (!object) {
        goto finish;
    }

    errorMessage = "error creating matching dictionary.";
    matchPropertyDict = CFDictionaryCreate(kCFAllocatorDefault,
        (void*)matchingKeys, matchingValues, matchSize,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!matchPropertyDict) {
        goto finish;
    }

    matchingDict = CFDictionaryCreateMutable(kCFAllocatorDefault,
        0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!matchingDict) {
        goto finish;
    }
    CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey),
                         matchPropertyDict);

    errorMessage = NULL;    // it might have gone away
    theMedia  = IOServiceGetMatchingService(kIOMasterPortDefault,
                                            matchingDict);
    matchingDict = NULL;  // IOServiceGetMatchingService() consumes reference!
    if (!theMedia) {
        goto finish;
    }

    errorMessage = "unable to get DiskArb info.";
    dadisk = DADiskCreateFromIOMedia(nil, sDASession, theMedia);
    if (!dadisk)    goto finish;
    dadesc = DADiskCopyDescription(dadisk);
    if (!dadesc)    goto finish;
    volUUID = CFDictionaryGetValue(dadesc, kDADiskDescriptionVolumeUUIDKey);
    if (!volUUID)   goto finish;

    watched = (void*)CFDictionaryGetValue(sFsysWatchDict, volUUID);
    if (watched) {
        // instead of passing -f, directly invalidate the caches
        if (invalidateStamps) {
            char stampsdir[PATH_MAX];
            errorMessage = "error unlinking bootstamps";
            if (strlcpy(stampsdir,watched->caches->root,PATH_MAX)>=PATH_MAX ||
                    strlcat(stampsdir, kTSCacheDir, PATH_MAX) >= PATH_MAX) {
                goto finish;
            }
            if (sdeepunlink(watched->caches->cachefd, stampsdir)) {
                goto finish;
            }
        }

        // might decide not to do the update right now
        checkScheduleUpdate(watched);
    }

    errorMessage = NULL;

finish:
    if (errorMessage) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "%s: %s", mediaDesc, errorMessage);
    }
    if (dadesc)             CFRelease(dadesc);
    if (dadisk)             CFRelease(dadisk);
    if (theMedia)           IOObjectRelease(theMedia);
    if (matchingDict)       CFRelease(matchingDict);
    if (matchPropertyDict)  CFRelease(matchPropertyDict);

    return;
}

/*******************************************************************************
* updateRAIDSet() -- Something on a RAID set has changed, so we update all
* Apple_Boot partitions.
*******************************************************************************/
#define RAID_MATCH_SIZE   (2)
void updateRAIDSet(
    CFNotificationCenterRef center,
    void * observer,
    CFStringRef name,
    const void * object,
    CFDictionaryRef userInfo)
{
    CFStringRef matchingKeys[RAID_MATCH_SIZE] = {
        CFSTR("RAID"),
        CFSTR("UUID") };
    CFTypeRef matchingValues[RAID_MATCH_SIZE] = {
        (CFTypeRef)kCFBooleanTrue,
        (CFTypeRef)object };

    updateVolForMedia(object, "RAID Volume", RAID_MATCH_SIZE, matchingKeys,
                      matchingValues, kBRInvalidateStamps);
}

/*******************************************************************************
* updateCoreStorageVolume() -- Something on a CoreStorage logical volume has
* changed, so we may need to update its boot partition info.
*******************************************************************************/
#define CSLV_MATCH_SIZE   (2)
void updateCoreStorageVolume(
   CFNotificationCenterRef center,
   void * observer,
   CFStringRef name,
   const void * object,
   CFDictionaryRef userInfo)
{
    CFStringRef matchingKeys[CSLV_MATCH_SIZE] = {
        CFSTR("CoreStorage"),
        CFSTR("UUID") };
    CFTypeRef matchingValues[CSLV_MATCH_SIZE] = {
        (CFTypeRef)kCFBooleanTrue,
        (CFTypeRef)object };
    bool invalidateStamps = false;

    if (CFEqual(name, CFSTR(kCoreStorageNotificationLVGChanged))) {
OSKextLog(NULL, kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
"LVG changed");
        invalidateStamps = true;
    }
    updateVolForMedia(object, "CoreStorage Volume", CSLV_MATCH_SIZE,
                      matchingKeys, matchingValues, invalidateStamps);
}


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations" // needed for asl_*
/*******************************************************************************
* logMTMessage() - log MessageTracer message
*******************************************************************************/
static void logMTMessage(char *signature, char *result, char *mfmt, ...)
{
    va_list ap;
    aslmsg amsg = asl_new(ASL_TYPE_MSG);

    if (!amsg) {
        OSKextLog(NULL, kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "asl_new() failed; MessageTracer message not logged");
        return;
    }

    asl_set(amsg, kMessageTracerDomainKey, kMTCachesDomain);
    asl_set(amsg, kMessageTracerSignatureKey, signature);
    // note: MT 'result' defaults to failure
    asl_set(amsg, kMessageTracerResultKey, result);

    // send it
    va_start(ap, mfmt);
    asl_vlog(NULL /* default handle */, amsg, ASL_LEVEL_NOTICE, mfmt, ap);
    va_end(ap);

    asl_free(amsg);
}
#pragma clang diagnostic pop
