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

#include <IOKit/kext/kextmanager_types.h>
#include <IOKit/kext/OSKextPrivate.h>

#include "kextd_usernotification.h"
#include "security.h"

//#include <ApplicationServices/ApplicationServices.h>

// xxx - do we want a new log activity flag?

#ifdef NO_CFUserNotification

void kextd_raise_notification(
    CFStringRef alertHeader,
    CFArrayRef  alertMessageArray)
{
    return;
}

#else

SCDynamicStoreRef       sSysConfigDynamicStore            = NULL;
uid_t                   sConsoleUser                      = (uid_t)-1;
CFRunLoopSourceRef      sNotificationQueueRunLoopSource   = NULL;  // must release

CFMutableArrayRef       sPendedNonsecureKextPaths         = NULL;  // must release
CFUserNotificationRef   sNonSecureNotification            = NULL;  // must release
CFRunLoopSourceRef      sNonSecureNotificationRunLoopSource = NULL;  // must release
CFMutableDictionaryRef  sNotifiedNonsecureKextPaths       = NULL;  // must release

CFMutableArrayRef       sPendedRevokedCertKextPaths       = NULL;  // must release
CFUserNotificationRef   sRevokedCertNotification          = NULL;  // must release
CFRunLoopSourceRef      sRevokedCertNotificationRunLoopSource = NULL;  // must release

#if 0 // not yet
CFMutableArrayRef       sPendedUnsignedKextPaths           = NULL;  // must release
CFUserNotificationRef   sUnsignedKextNotification          = NULL;  // must release
CFRunLoopSourceRef      sUnsignedKextNotificationRunLoopSource = NULL;  // must release
#endif

CFMutableArrayRef       sPendedNoLoadKextPaths            = NULL;  // must release
CFUserNotificationRef   sNoLoadKextNotification           = NULL;  // must release
CFRunLoopSourceRef      sNoLoadKextNotificationRunLoopSource = NULL;  // must release

CFMutableArrayRef       sPendedInvalidSignedKextPaths     = NULL;  // must release
CFUserNotificationRef   sInvalidSigNotification           = NULL;  // must release
CFRunLoopSourceRef      sInvalidSigNotificationRunLoopSource = NULL;  // must release


CFMutableArrayRef       sPendedExcludedKextPaths          = NULL;  // must release
CFUserNotificationRef   sExcludedKextNotification         = NULL;  // must release
CFRunLoopSourceRef      sExcludedKextNotificationRunLoopSource = NULL;  // must release

CFDictionaryRef         sKextTranslationsPlist            = NULL;

static void _sessionDidChange(
    SCDynamicStoreRef store,
    CFArrayRef        changedKeys,
    void            * info);

void _checkNotificationQueue(void * info);
void _notificationDismissed(
    CFUserNotificationRef userNotification,
    CFOptionFlags         responseFlags);

static CFStringRef createBundleMappingKey( CFStringRef theBundleID );
static Boolean isInAlertsSentArray(
                                   CFArrayRef          theSentArray,
                                   CFDictionaryRef     theDict,
                                   CFStringRef         theMappingKey );
static CFStringRef getKextAlertMessage(
                                       CFDictionaryRef  theDict,
                                       CFStringRef      theMappingKey );
static Boolean sendKextAlertNotifications(
                                    CFMutableArrayRef * theSentAlertsArrayPtr,
                                    CFArrayRef theKextsArray,
                                    int theAlertType );

static CFStringRef createPathFromAlertType(
                                           CFStringRef theVolRoot,
                                           int theAlertType );
static Boolean doingSystemInstall(void);
static void kextd_raise_nonsecure_notification(
                                               CFStringRef alertHeader,
                                               CFArrayRef  alertMessageArray );
static void kextd_raise_noload_notification(
                                            CFStringRef alertHeader,
                                            CFArrayRef  alertMessageArray );
static void kextd_raise_invalidsig_notification(
                                                CFStringRef alertHeader,
                                                CFArrayRef  alertMessageArray );
static void kextd_raise_revokedcert_notification(
                                                 CFStringRef alertHeader,
                                                 CFArrayRef  alertMessageArray );
static void revealInFinder( CFArrayRef theArray );
#if 0 // not yet
static void kextd_raise_unsignedkext_notification(
                                                  CFStringRef alertHeader,
                                                  CFArrayRef  alertMessageArray );
#endif

static void kextd_raise_excludedkext_notification(
                                                  CFTypeRef alertHeader,
                                                  CFTypeRef alertMessageArray );
static int validateKextsAlertDict( CFDictionaryRef theDict );


/*******************************************************************************
*******************************************************************************/
ExitStatus startMonitoringConsoleUser(
    KextdArgs    * toolArgs)
{
    ExitStatus         result                 = EX_OSERR;
    CFStringRef        consoleUserName        = NULL;  // must release
    CFStringRef        consoleUserKey         = NULL;  // must release
    CFMutableArrayRef  keys                   = NULL;  // must release
    CFRunLoopSourceRef sysConfigRunLoopSource = NULL;  // must release
    CFRunLoopSourceContext sourceContext;

    sSysConfigDynamicStore = SCDynamicStoreCreate(
        kCFAllocatorDefault, CFSTR(KEXTD_SERVER_NAME),
        _sessionDidChange, /* context */ NULL);
    if (!sSysConfigDynamicStore) {
        OSKextLogMemError();
        goto finish;

    }

    consoleUserName = SCDynamicStoreCopyConsoleUser(sSysConfigDynamicStore,
        &sConsoleUser, NULL);
    if (!consoleUserName) {
        sConsoleUser = (uid_t)-1;
        OSKextLog(/* kext */ NULL,
            kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
            "No user logged in at kextd startup.");
    } else {
    OSKextLog(/* kext */ NULL,
        kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
        "User %d logged in at kextd startup.", sConsoleUser);
    }

    consoleUserKey = SCDynamicStoreKeyCreateConsoleUser(kCFAllocatorDefault);
    if (!consoleUserKey) {
        OSKextLogMemError();
        goto finish;
    }
    keys = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if (!keys) {
        OSKextLogMemError();
        goto finish;
    }

    CFArrayAppendValue(keys, consoleUserKey);
    SCDynamicStoreSetNotificationKeys(sSysConfigDynamicStore, keys,
        /* patterns */ NULL);

    sysConfigRunLoopSource = SCDynamicStoreCreateRunLoopSource(
        kCFAllocatorDefault, sSysConfigDynamicStore, 0);
    if (!sysConfigRunLoopSource) {
        OSKextLogMemError();
        goto finish;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(), sysConfigRunLoopSource,
        kCFRunLoopCommonModes);

    bzero(&sourceContext, sizeof(CFRunLoopSourceContext));
    sourceContext.version = 0;
    sourceContext.perform = _checkNotificationQueue;
    sNotificationQueueRunLoopSource = CFRunLoopSourceCreate(kCFAllocatorDefault,
                                        0, &sourceContext);
    if (!sNotificationQueueRunLoopSource) {
       OSKextLog(/* kext */ NULL, kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
           "Failed to create alert run loop source.");
        goto finish;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(), sNotificationQueueRunLoopSource,
        kCFRunLoopDefaultMode);

    if (!createCFMutableArray(&sPendedNonsecureKextPaths,
                              &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        goto finish;
    }

    if (!createCFMutableArray(&sPendedNoLoadKextPaths,
                              &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        goto finish;
    }

    if (!createCFMutableArray(&sPendedRevokedCertKextPaths,
                              &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        goto finish;
    }

#if 0 // not yet
    if (!createCFMutableArray(&sPendedUnsignedKextPaths,
                              &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        goto finish;
    }
#endif

    if (!createCFMutableArray(&sPendedInvalidSignedKextPaths,
                              &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        goto finish;
    }

    if (!createCFMutableArray(&sPendedExcludedKextPaths,
                              &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        goto finish;
    }

    if (!createCFMutableDictionary(&sNotifiedNonsecureKextPaths)) {
        OSKextLogMemError();
        goto finish;
    }

    result = EX_OK;

finish:
    SAFE_RELEASE(consoleUserName);
    SAFE_RELEASE(consoleUserKey);
    SAFE_RELEASE(keys);
    SAFE_RELEASE(sysConfigRunLoopSource);

    return result;
}

/*******************************************************************************
*******************************************************************************/
void stopMonitoringConsoleUser(void)
{
    SAFE_RELEASE(sSysConfigDynamicStore);

    SAFE_RELEASE(sNotificationQueueRunLoopSource);
    SAFE_RELEASE(sPendedNonsecureKextPaths);
    SAFE_RELEASE(sPendedNoLoadKextPaths);
    SAFE_RELEASE(sPendedRevokedCertKextPaths);
    SAFE_RELEASE(sPendedInvalidSignedKextPaths);
    //    SAFE_RELEASE(sPendedUnsignedKextPaths);
    SAFE_RELEASE(sPendedExcludedKextPaths);
    SAFE_RELEASE(sNotifiedNonsecureKextPaths);

    return;
}

/*******************************************************************************
*******************************************************************************/
static void _sessionDidChange(
    SCDynamicStoreRef store,
    CFArrayRef        changedKeys,
    void            * info)
{
    CFStringRef consoleUserName = NULL;  // must release
    uid_t       oldUser         = sConsoleUser;

   /* If any users are logged on via fast user switching, logging out to
    * loginwindow sets the console user to 0 (root). We can't do a reset
    * until all users have fully logged out, in which case
    * SCDynamicStoreCopyConsoleUser() returns NULL.
    */
    consoleUserName = SCDynamicStoreCopyConsoleUser(sSysConfigDynamicStore,
        &sConsoleUser, NULL);
    if (!consoleUserName) {
        if (oldUser != (uid_t)-1) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
                "User %d logged out.", oldUser);
        }
        sConsoleUser = (uid_t)-1;
        resetUserNotifications(/* dismissAlert */ true);
        goto finish;
    }

   /* Sometimes we'll get >1 notification on a user login, so make sure
    * the old & new uid are different.
    */
    if (sConsoleUser != (uid_t)-1 && oldUser != sConsoleUser) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
            "User %d logged in.", sConsoleUser);
        CFRunLoopSourceSignal(sNotificationQueueRunLoopSource);
        CFRunLoopWakeUp(CFRunLoopGetCurrent());
    }

finish:
    SAFE_RELEASE(consoleUserName);

    return;
}

/*******************************************************************************
*******************************************************************************/
void resetUserNotifications(Boolean dismissAlert)
{
    OSKextLog(/* kext */ NULL,
        kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
        "Resetting user notifications.");

    if (dismissAlert) {

        /* Release any reference to any pending user notifications.
         */
        if (sNonSecureNotification) {
            CFUserNotificationCancel(sNonSecureNotification);
            CFRelease(sNonSecureNotification);
            sNonSecureNotification = NULL;
        }
        if (sNonSecureNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sNonSecureNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sNonSecureNotificationRunLoopSource);
            sNonSecureNotificationRunLoopSource = NULL;
        }

        if (sNoLoadKextNotification) {
            CFUserNotificationCancel(sNoLoadKextNotification);
            CFRelease(sNoLoadKextNotification);
            sNoLoadKextNotification = NULL;
        }
        if (sNoLoadKextNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sNoLoadKextNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sNoLoadKextNotificationRunLoopSource);
            sNoLoadKextNotificationRunLoopSource = NULL;
        }

        if (sRevokedCertNotification) {
            CFUserNotificationCancel(sRevokedCertNotification);
            CFRelease(sRevokedCertNotification);
            sRevokedCertNotification = NULL;
        }
        if (sRevokedCertNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sRevokedCertNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sRevokedCertNotificationRunLoopSource);
            sRevokedCertNotificationRunLoopSource = NULL;
        }

#if 0 // not yet
        if (sUnsignedKextNotification) {
            CFUserNotificationCancel(sUnsignedKextNotification);
            CFRelease(sUnsignedKextNotification);
            sUnsignedKextNotification = NULL;
        }
        if (sUnsignedKextNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sUnsignedKextNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sUnsignedKextNotificationRunLoopSource);
            sUnsignedKextNotificationRunLoopSource = NULL;
        }
#endif

        if (sInvalidSigNotification) {
            CFUserNotificationCancel(sInvalidSigNotification);
            CFRelease(sInvalidSigNotification);
            sInvalidSigNotification = NULL;
        }
        if (sInvalidSigNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sInvalidSigNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sInvalidSigNotificationRunLoopSource);
            sInvalidSigNotificationRunLoopSource = NULL;
        }


        if (sExcludedKextNotification) {
            CFUserNotificationCancel(sExcludedKextNotification);
            CFRelease(sExcludedKextNotification);
            sExcludedKextNotification = NULL;
        }
        if (sExcludedKextNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sExcludedKextNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sExcludedKextNotificationRunLoopSource);
            sExcludedKextNotificationRunLoopSource = NULL;
        }
    }

    /* Clear the record of which kexts the user has been told are insecure.
     * If extensions folders have been modified, who knows which kexts are changed?
     * If user is logging out, logging back in will get the same alerts.
     */
    CFArrayRemoveAllValues(sPendedNonsecureKextPaths);
    CFDictionaryRemoveAllValues(sNotifiedNonsecureKextPaths);

    /* clean up pending kext alerts too */
    CFArrayRemoveAllValues(sPendedRevokedCertKextPaths);
    CFArrayRemoveAllValues(sPendedNoLoadKextPaths);
    CFArrayRemoveAllValues(sPendedInvalidSignedKextPaths);
    //    CFArrayRemoveAllValues(sPendedUnsignedKextPaths);
    CFArrayRemoveAllValues(sPendedExcludedKextPaths);

    return;
}

/*******************************************************************************
*******************************************************************************/
void _checkNotificationQueue(void * info __unused)
{
    CFStringRef       kextPath                      = NULL;  // do not release
    CFMutableArrayRef nonsecureAlertMessageArray    = NULL;  // must release
    CFMutableArrayRef noLoadAlertMessageArray       = NULL;  // must release
    CFMutableArrayRef revokedCertAlertMessageArray  = NULL;  // must release
    CFMutableArrayRef invalidSigAlertMessageArray   = NULL;  // must release
    CFMutableArrayRef unsignedKextAlertMessageArray = NULL;  // must release
    CFMutableArrayRef excludedAlertMessageArray     = NULL;  // must release
    CFMutableArrayRef excludedAlertHeaderArray      = NULL;  // must release
    CFIndex     count, i;

    if (sConsoleUser == (uid_t)-1) {
        goto finish;
    }

    /* handle alerts for kexts that do not have the proper privs set
     */
    if (CFArrayGetCount(sPendedNonsecureKextPaths)  &&
        sNonSecureNotificationRunLoopSource == NULL) {
        nonsecureAlertMessageArray = CFArrayCreateMutable(kCFAllocatorDefault,
                                                   0,
                                                   &kCFTypeArrayCallBacks);
        if (nonsecureAlertMessageArray == NULL) {
            goto finish;
        }

        kextPath = (CFStringRef) CFArrayGetValueAtIndex(
                                        sPendedNonsecureKextPaths, 0);
        if (!kextPath) {
            goto finish;
        }

       /* This is the localized format string for the alert message.
        */
        CFArrayAppendValue(nonsecureAlertMessageArray,
            CFSTR("The system extension \""));
        CFArrayAppendValue(nonsecureAlertMessageArray, kextPath);
        CFArrayAppendValue(nonsecureAlertMessageArray,
            CFSTR("\" was installed improperly and cannot be used. "
                  "Please try reinstalling it, or contact the product's vendor "
                  "for an update."));

        CFArrayRemoveValueAtIndex(sPendedNonsecureKextPaths, 0);
        kextd_raise_nonsecure_notification(CFSTR("System extension cannot be used"),
                                           nonsecureAlertMessageArray);
    }
    /* handle alerts for kext signature verification errors that result
     * in a kext not loading (currently only kexts in /Library/Extensions/
     */
    count = CFArrayGetCount(sPendedNoLoadKextPaths);
    if (count > 0 && sNoLoadKextNotificationRunLoopSource == NULL) {
        noLoadAlertMessageArray = CFArrayCreateMutable(
                                                       kCFAllocatorDefault,
                                                       0,
                                                       &kCFTypeArrayCallBacks );
        if (noLoadAlertMessageArray == NULL) {
            goto finish;
        }
        CFArrayAppendValue(
                           noLoadAlertMessageArray,
                           (count > 1 ? CFSTR("The following kernel extensions can't "
                                              "be loaded because they are from "
                                              "unidentified developers.  Extensions "
                                              "loaded from /Library/Extensions must be "
                                              "signed by identified developers. \r")
                            : CFSTR("The kernel extension at \"")) );
        for (i = 0; i < count; i ++) {
            kextPath = (CFStringRef)
            CFArrayGetValueAtIndex( sPendedNoLoadKextPaths, i );
            if (kextPath) {
                if (count > 1) {
                    CFArrayAppendValue(noLoadAlertMessageArray, CFSTR("\r"));
                }
                CFArrayAppendValue(noLoadAlertMessageArray, kextPath);
            }
        }
        if (count == 1) {
            CFArrayAppendValue(noLoadAlertMessageArray,
                               CFSTR("\" can't be loaded because it is from an "
                                     "unidentified developer.  Extensions "
                                     "loaded from /Library/Extensions must be "
                                     "signed by identified developers."));
        }
        CFArrayAppendValue(noLoadAlertMessageArray,
                           CFSTR("\r\rPlease contact the kernel extension "
                                 "vendor for updated software."));

        CFArrayRemoveAllValues(sPendedNoLoadKextPaths);
        kextd_raise_noload_notification(
                                        (count > 1  ? CFSTR("Kernel extensions could not be loaded")
                                         : CFSTR("Kernel extension could not be loaded")),
                                        noLoadAlertMessageArray);
    }

    /* handle alerts for kexts with a revoked cert
     */
    count = CFArrayGetCount(sPendedRevokedCertKextPaths);
    if (count > 0  && sRevokedCertNotificationRunLoopSource == NULL) {
        revokedCertAlertMessageArray = CFArrayCreateMutable(
                                                            kCFAllocatorDefault,
                                                            0,
                                                            &kCFTypeArrayCallBacks );
        if (revokedCertAlertMessageArray == NULL) {
            goto finish;
        }
        CFArrayAppendValue(
                           revokedCertAlertMessageArray,
                           (count > 1 ? CFSTR("The following kernel extensions "
                                              "are damaged and can't be "
                                              "loaded. \r")
                            : CFSTR("The kernel extension at \"")) );
        for (i = 0; i < count; i ++) {
            kextPath = (CFStringRef) CFArrayGetValueAtIndex(
                                            sPendedRevokedCertKextPaths, i );
            if (kextPath) {
                if (count > 1) {
                    CFArrayAppendValue(revokedCertAlertMessageArray, CFSTR("\r"));
                }
                CFArrayAppendValue(revokedCertAlertMessageArray,
                                   kextPath);
            }
        }
        if (count == 1) {
            CFArrayAppendValue(revokedCertAlertMessageArray,
                               CFSTR("\" is damaged and can't be loaded."));
        }
        CFArrayAppendValue(revokedCertAlertMessageArray,
                           (count > 1
                            ? CFSTR("\r\rYou should move them to the Trash.")
                            : CFSTR("\r\rYou should move it to the Trash."))
                           );

        // CFArrayRemoveAllValues(sPendedRevokedCertKextPaths); do not remove here, need to use these to reveal in Finder
        kextd_raise_revokedcert_notification(
                                        (count > 1  ? CFSTR("Kernel extensions could not be loaded")
                                         : CFSTR("Kernel extension could not be loaded")),
                                        revokedCertAlertMessageArray);
    }

#if 0 // not yet
    /* handle alerts for unsigned kexts
     */
    count = CFArrayGetCount(sPendedUnsignedKextPaths);
    if (count > 0 && sUnsignedKextNotificationRunLoopSource == NULL) {
        unsignedKextAlertMessageArray = CFArrayCreateMutable(kCFAllocatorDefault,
                                                             0,
                                                             &kCFTypeArrayCallBacks);
        if (unsignedKextAlertMessageArray == NULL) {
            goto finish;
        }
        CFArrayAppendValue(unsignedKextAlertMessageArray,
                           CFSTR("The following kernel extensions are not "
                                 "signed.\r"));
        for (i = 0; i < count; i ++) {
            kextPath = (CFStringRef) CFArrayGetValueAtIndex(
                                                            sPendedUnsignedKextPaths, i);
            if (kextPath) {
                CFArrayAppendValue(unsignedKextAlertMessageArray,
                                   CFSTR("\r"));
                CFArrayAppendValue(unsignedKextAlertMessageArray,
                                   kextPath);
            }
        }
        CFArrayAppendValue(unsignedKextAlertMessageArray,
                           CFSTR("\r\rPlease contact the vendor for each "
                                 "kernel extension for updated software."));

        CFArrayRemoveAllValues(sPendedUnsignedKextPaths);
        kextd_raise_unsignedkext_notification(CFSTR("Kernel extensions are not signed"),
                                              unsignedKextAlertMessageArray);
    }
#endif

    /* handle alerts for kext signature verification errors
     */
    count = CFArrayGetCount(sPendedInvalidSignedKextPaths);
    if (count > 0 && sInvalidSigNotificationRunLoopSource == NULL) {
        invalidSigAlertMessageArray = CFArrayCreateMutable(kCFAllocatorDefault,
                                                           0,
                                                           &kCFTypeArrayCallBacks);
        if (invalidSigAlertMessageArray == NULL) {
            goto finish;
        }
        CFArrayAppendValue(invalidSigAlertMessageArray,
                           (count > 1 ? CFSTR("The following kernel extensions are not "
                                              "from identified developers but will "
                                              "still be loaded. \r")
                            : CFSTR("The kernel extension at \"")) );
        for (i = 0; i < count; i ++) {
            kextPath = (CFStringRef) CFArrayGetValueAtIndex(
                                                            sPendedInvalidSignedKextPaths, i);
            if (kextPath) {
                if (count > 1) {
                    CFArrayAppendValue(invalidSigAlertMessageArray, CFSTR("\r"));
                }
                CFArrayAppendValue(invalidSigAlertMessageArray, kextPath);
            }
        }
        if (count == 1) {
            CFArrayAppendValue(invalidSigAlertMessageArray,
                               CFSTR("\" is not from an identified developer "
                                     "but will still be loaded."));
        }
        CFArrayAppendValue(invalidSigAlertMessageArray,
                           CFSTR("\r\rPlease contact the kernel extension "
                                 "vendor for updated software."));

        CFArrayRemoveAllValues(sPendedInvalidSignedKextPaths);
        kextd_raise_invalidsig_notification(
                                            (count > 1  ? CFSTR("Kernel extensions are not from identified "
                                                                "developers")
                                             : CFSTR("Kernel extension is not from an identified "
                                                     "developer")),
                                            invalidSigAlertMessageArray);
    }

#if 1 // <rdar://problem/12811081> warn user about excluded kexts
    count = CFArrayGetCount(sPendedExcludedKextPaths);
    if (count > 0 && sExcludedKextNotificationRunLoopSource == NULL) {
        excludedAlertMessageArray =
        CFArrayCreateMutable(kCFAllocatorDefault,
                             0,
                             &kCFTypeArrayCallBacks);
        if (excludedAlertMessageArray == NULL) {
            goto finish;
        }
        excludedAlertHeaderArray =
        CFArrayCreateMutable(kCFAllocatorDefault,
                             0,
                             &kCFTypeArrayCallBacks);
        if (excludedAlertHeaderArray == NULL) {
            goto finish;
        }
        if (count > 1) {
            CFArrayAppendValue(excludedAlertHeaderArray,
                           CFSTR("Some system extensions are not compatible with this version of macOS and can’t be used:"));
            for (i = 0; i < count; i ++) {
                kextPath = (CFStringRef) CFArrayGetValueAtIndex(
                                                                sPendedExcludedKextPaths, i);
                if (kextPath) {
                    CFRange             myRange;
                    myRange = CFStringFind(kextPath, CFSTR("/"), kCFCompareBackwards);
                    // just get kext name if possible, not the full path
                    if (myRange.length != 0 && myRange.location++ < CFStringGetLength(kextPath)) {
                        CFStringRef     myString;
                        myRange.length = CFStringGetLength(kextPath) - myRange.location;

                        myString = CFStringCreateWithSubstring(kCFAllocatorDefault,
                                                               kextPath,
                                                               myRange);
                        if (myString) {
                            CFArrayAppendValue(excludedAlertMessageArray,
                                               myString);
                            SAFE_RELEASE(myString);
                        }
                        else {
                            // fall back to full path
                            CFArrayAppendValue(excludedAlertMessageArray,
                                               kextPath);
                        }
                    }
                    else {
                        // fall back to full path
                        CFArrayAppendValue(excludedAlertMessageArray,
                                           kextPath);
                    }
                    CFArrayAppendValue(excludedAlertMessageArray,
                                       CFSTR("\r"));
                }
            }
            // one extra for "Please contact the developer..." to look good
            CFArrayAppendValue(excludedAlertMessageArray,
                               CFSTR("\r"));
        }
        else {
            kextPath = (CFStringRef) CFArrayGetValueAtIndex(
                                                            sPendedExcludedKextPaths, 0 );
            if (kextPath == NULL) {
                goto finish;
            }
            CFArrayAppendValue(excludedAlertHeaderArray,
                           CFSTR("The system extension \""));
            CFRange             myRange;
            myRange = CFStringFind(kextPath, CFSTR("/"), kCFCompareBackwards);
            // just get kext name if possible, not the full path
            if (myRange.length != 0 && myRange.location++ < CFStringGetLength(kextPath)) {
                CFStringRef     myString;
                myRange.length = CFStringGetLength(kextPath) - myRange.location;

                myString = CFStringCreateWithSubstring(kCFAllocatorDefault,
                                                       kextPath,
                                                       myRange);
                if (myString) {
                    CFArrayAppendValue(excludedAlertHeaderArray,
                                   myString);
                   SAFE_RELEASE(myString);
                }
                else {
                    // fall back to full path
                    CFArrayAppendValue(excludedAlertHeaderArray,
                                   kextPath);
                }
            }
            else {
                // fall back to full path
                CFArrayAppendValue(excludedAlertHeaderArray,
                               kextPath);
            }
            CFArrayAppendValue(excludedAlertHeaderArray,
                           CFSTR("\" is not compatible with this version of macOS and can’t be used."));
        }
        CFArrayAppendValue(excludedAlertMessageArray,
                           CFSTR("Please contact the developer for updated software."));

        CFArrayRemoveAllValues(sPendedExcludedKextPaths);
        kextd_raise_excludedkext_notification(excludedAlertHeaderArray,
                                              excludedAlertMessageArray);
    }
#endif // <rdar://problem/12811081>

finish:
    SAFE_RELEASE(nonsecureAlertMessageArray);
    SAFE_RELEASE(noLoadAlertMessageArray);
    SAFE_RELEASE(excludedAlertMessageArray);
    SAFE_RELEASE(revokedCertAlertMessageArray);
    SAFE_RELEASE(invalidSigAlertMessageArray);
    SAFE_RELEASE(unsignedKextAlertMessageArray);
    SAFE_RELEASE(excludedAlertHeaderArray);
    return;
}

/******************************************************************************
 ******************************************************************************/
Boolean recordNonsecureKexts(CFArrayRef kextList)
{
    Boolean     result             = false;
    CFStringRef nonsecureKextPath  = NULL;  // must release
    CFIndex     count, i;

    if (kextList && (count = CFArrayGetCount(kextList))) {
        for (i = 0; i < count; i ++) {
            OSKextRef checkKext = (OSKextRef)CFArrayGetValueAtIndex(kextList, i);

            SAFE_RELEASE_NULL(nonsecureKextPath);

            if (OSKextIsAuthentic(checkKext)) {
                continue;
            }
            nonsecureKextPath = copyKextPath(checkKext);
            if (!nonsecureKextPath) {
                OSKextLogMemError();
                goto finish;
            }
            if (!CFDictionaryGetValue(sNotifiedNonsecureKextPaths,
                nonsecureKextPath)) {

                CFArrayAppendValue(sPendedNonsecureKextPaths,
                    nonsecureKextPath);
                CFDictionarySetValue(sNotifiedNonsecureKextPaths,
                    nonsecureKextPath, kCFBooleanTrue);

                result = true;
            }
        }
    }

finish:
    SAFE_RELEASE(nonsecureKextPath);

    if (result) {
        CFRunLoopSourceSignal(sNotificationQueueRunLoopSource);
        CFRunLoopWakeUp(CFRunLoopGetCurrent());
    }
    return result;
}

/*******************************************************************************
 *******************************************************************************/
Boolean recordNoLoadKextPath(CFStringRef theKextPath)
{
    Boolean     result = false;

    if (theKextPath == NULL) {
        goto finish;
    }
    CFArrayAppendValue(sPendedNoLoadKextPaths, theKextPath);
    result = true;

finish:
    return result;
}

/*******************************************************************************
 *******************************************************************************/
void sendNoLoadKextNotification(void)
{
    if (CFArrayGetCount(sPendedNoLoadKextPaths)) {
        CFRunLoopSourceSignal(sNotificationQueueRunLoopSource);
        CFRunLoopWakeUp(CFRunLoopGetCurrent());
    }
    return;
}

/*******************************************************************************
 *******************************************************************************/
void recordRevokedCertKextPath(CFStringRef theKextPath)
{
    if (theKextPath == NULL) {
        goto finish;
    }
    CFArrayAppendValue(sPendedRevokedCertKextPaths, theKextPath);

finish:
    return;
}

/*******************************************************************************
 *******************************************************************************/
void sendRevokedCertKextPath(void)
{
    if (CFArrayGetCount(sPendedRevokedCertKextPaths)) {
        CFRunLoopSourceSignal(sNotificationQueueRunLoopSource);
        CFRunLoopWakeUp(CFRunLoopGetCurrent());
    }
    return;
}

/*******************************************************************************
 *******************************************************************************/
Boolean recordInvalidSignedKextPath(CFStringRef theKextPath)
{
    Boolean     result              = false;

    if (theKextPath == NULL) {
        goto finish;
    }
    CFArrayAppendValue(sPendedInvalidSignedKextPaths, theKextPath);
    result = true;

finish:
    return result;
}

/*******************************************************************************
 *******************************************************************************/
void sendInvalidSignedKextNotification(void)
{
    if (CFArrayGetCount(sPendedInvalidSignedKextPaths)) {
        CFRunLoopSourceSignal(sNotificationQueueRunLoopSource);
        CFRunLoopWakeUp(CFRunLoopGetCurrent());
    }
    return;
}

#if 0 // not yet
/*******************************************************************************
 *******************************************************************************/
Boolean recordUnsignedKextPath(CFStringRef theKextPath)
{
    Boolean     result              = false;

    if (theKextPath == NULL) {
        goto finish;
    }
    CFArrayAppendValue(sPendedUnsignedKextPaths, theKextPath);
    result = true;

finish:
    return result;
}

/*******************************************************************************
 *******************************************************************************/
void sendUnsignedKextNotification(void)
{
    if (CFArrayGetCount(sPendedUnsignedKextPaths)) {
        CFRunLoopSourceSignal(sNotificationQueueRunLoopSource);
        CFRunLoopWakeUp(CFRunLoopGetCurrent());
    }
    return;
}
#endif

/*******************************************************************************
 *******************************************************************************/
Boolean recordExcludedKextPath(CFStringRef theKextPath)
{
    Boolean     result              = false;

    if (theKextPath == NULL) {
        goto finish;
    }
    CFArrayAppendValue(sPendedExcludedKextPaths, theKextPath);
    result = true;

finish:
    return result;
}

/*******************************************************************************
 *******************************************************************************/
void sendExcludedKextNotification(void)
{
    if (CFArrayGetCount(sPendedExcludedKextPaths)) {
        CFRunLoopSourceSignal(sNotificationQueueRunLoopSource);
        CFRunLoopWakeUp(CFRunLoopGetCurrent());
    }
    return;
}

/*******************************************************************************
 *******************************************************************************/
static CFMutableDictionaryRef createAlertDict(
                                              CFTypeRef alertHeader,
                                              CFTypeRef alertMessageArray )
{
    CFMutableDictionaryRef alertDict               = NULL;  // do not release
    CFURLRef               iokitFrameworkBundleURL = NULL;  // must release

    /* Do not alert if we're doing a system install */
    if ( doingSystemInstall() ) {
        goto finish;
    }

    OSKextLog(/* kext */ NULL,
              kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
              "Raising user notification.");

    if (sConsoleUser == (uid_t)-1) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
                  "No logged in user.");
        goto finish;
    }

    iokitFrameworkBundleURL = CFURLCreateWithFileSystemPath(
                                                            kCFAllocatorDefault,
                                                            CFSTR("/System/Library/Frameworks/IOKit.framework"),
                                                            kCFURLPOSIXPathStyle, true);
    if (!iokitFrameworkBundleURL) {
        goto finish;
    }

    alertDict = CFDictionaryCreateMutable(
                                          kCFAllocatorDefault, 0,
                                          &kCFTypeDictionaryKeyCallBacks,
                                          &kCFTypeDictionaryValueCallBacks);
    if (!alertDict) {
        goto finish;
    }

    CFDictionarySetValue(alertDict, kCFUserNotificationLocalizationURLKey,
                         iokitFrameworkBundleURL);
    CFDictionarySetValue(alertDict, kCFUserNotificationAlertHeaderKey,
                         alertHeader);
    CFDictionarySetValue(alertDict, kCFUserNotificationDefaultButtonTitleKey,
                         CFSTR("OK"));
    CFDictionarySetValue(alertDict, kCFUserNotificationAlertMessageKey,
                         alertMessageArray);
finish:
    SAFE_RELEASE(iokitFrameworkBundleURL);

    return(alertDict);
}

/*******************************************************************************
 *******************************************************************************/
static void kextd_raise_nonsecure_notification(
                                               CFStringRef alertHeader,
                                               CFArrayRef  alertMessageArray )
{
    CFMutableDictionaryRef alertDict               = NULL;  // must release
    SInt32                 userNotificationError   = 0;

    alertDict = createAlertDict(alertHeader, alertMessageArray);
    if (alertDict == NULL) {
        goto finish;
    }

    sNonSecureNotification = CFUserNotificationCreate(kCFAllocatorDefault,
                                                      0 /* time interval */, kCFUserNotificationCautionAlertLevel,
                                                      &userNotificationError, alertDict);
    if (!sNonSecureNotification) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "Can't create user notification - %d",
                  (int)userNotificationError);
        goto finish;
    }

    sNonSecureNotificationRunLoopSource = CFUserNotificationCreateRunLoopSource(
                                                                                kCFAllocatorDefault,
                                                                                sNonSecureNotification,
                                                                                &_notificationDismissed,
                                                                                /* order */ 5 /* xxx - cheesy! */ );
    if (!sNonSecureNotificationRunLoopSource) {
        CFRelease(sNonSecureNotification);
        sNonSecureNotification = NULL;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                       sNonSecureNotificationRunLoopSource,
                       kCFRunLoopDefaultMode);
finish:
    SAFE_RELEASE(alertDict);

    return;
}

/*******************************************************************************
 *******************************************************************************/
static void kextd_raise_noload_notification(
                                            CFStringRef alertHeader,
                                            CFArrayRef  alertMessageArray )
{
    CFMutableDictionaryRef alertDict               = NULL;  // must release
    SInt32                 userNotificationError   = 0;

    alertDict = createAlertDict(alertHeader, alertMessageArray);
    if (alertDict == NULL) {
        goto finish;
    }

    sNoLoadKextNotification =
    CFUserNotificationCreate(
                             kCFAllocatorDefault,
                             0 /* time interval */,
                             kCFUserNotificationCautionAlertLevel,
                             &userNotificationError,
                             alertDict );
    if (!sNoLoadKextNotification) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "Can't create user notification - %d",
                  (int)userNotificationError);
        goto finish;
    }

    sNoLoadKextNotificationRunLoopSource =
    CFUserNotificationCreateRunLoopSource(
                                          kCFAllocatorDefault,
                                          sNoLoadKextNotification,
                                          &_notificationDismissed,
                                          /* order */ 5 /* xxx - cheesy! */ );
    if (!sNoLoadKextNotificationRunLoopSource) {
        CFRelease(sNoLoadKextNotification);
        sNoLoadKextNotification = NULL;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                       sNoLoadKextNotificationRunLoopSource,
                       kCFRunLoopDefaultMode);
finish:
    SAFE_RELEASE(alertDict);

    return;
}

/*******************************************************************************
 *******************************************************************************/
static void kextd_raise_revokedcert_notification(
                                                 CFStringRef alertHeader,
                                                 CFArrayRef  alertMessageArray )
{
    CFMutableDictionaryRef alertDict               = NULL;  // must release
    SInt32                 userNotificationError   = 0;

    alertDict = createAlertDict(alertHeader, alertMessageArray);
    if (alertDict == NULL) {
        goto finish;
    }

    /* retitle default button */
    CFDictionarySetValue(alertDict, kCFUserNotificationDefaultButtonTitleKey,
                         CFSTR("Reveal in Finder"));

    sRevokedCertNotification =
    CFUserNotificationCreate(
                             kCFAllocatorDefault,
                             0 /* time interval */,
                             kCFUserNotificationCautionAlertLevel,
                             &userNotificationError,
                             alertDict );
    if (!sRevokedCertNotification) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "Can't create user notification - %d",
                  (int)userNotificationError);
        goto finish;
    }

    sRevokedCertNotificationRunLoopSource =
    CFUserNotificationCreateRunLoopSource(
                                          kCFAllocatorDefault,
                                          sRevokedCertNotification,
                                          &_notificationDismissed,
                                          /* order */ 5 /* xxx - cheesy! */ );
    if (!sRevokedCertNotificationRunLoopSource) {
        CFRelease(sRevokedCertNotification);
        sRevokedCertNotification = NULL;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                       sRevokedCertNotificationRunLoopSource,
                       kCFRunLoopDefaultMode);
finish:
    SAFE_RELEASE(alertDict);

    return;
}

/*******************************************************************************
 *******************************************************************************/
static void kextd_raise_invalidsig_notification(
                                                CFStringRef alertHeader,
                                                CFArrayRef  alertMessageArray )
{
    CFMutableDictionaryRef alertDict               = NULL;  // must release
    SInt32                 userNotificationError   = 0;

    alertDict = createAlertDict(alertHeader, alertMessageArray);
    if (alertDict == NULL) {
        goto finish;
    }

    sInvalidSigNotification =
    CFUserNotificationCreate(
                             kCFAllocatorDefault,
                             0 /* time interval */,
                             kCFUserNotificationCautionAlertLevel,
                             &userNotificationError,
                             alertDict );
    if (!sInvalidSigNotification) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "Can't create user notification - %d",
                  (int)userNotificationError);
        goto finish;
    }

    sInvalidSigNotificationRunLoopSource =
    CFUserNotificationCreateRunLoopSource(
                                          kCFAllocatorDefault,
                                          sInvalidSigNotification,
                                          &_notificationDismissed,
                                          /* order */ 5 /* xxx - cheesy! */ );
    if (!sInvalidSigNotificationRunLoopSource) {
        CFRelease(sInvalidSigNotification);
        sInvalidSigNotification = NULL;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                       sInvalidSigNotificationRunLoopSource,
                       kCFRunLoopDefaultMode);
finish:
    SAFE_RELEASE(alertDict);

    return;
}

#if 0 // not yet
/*******************************************************************************
 *******************************************************************************/
static void kextd_raise_unsignedkext_notification(
                                                  CFStringRef alertHeader,
                                                  CFArrayRef  alertMessageArray )
{
    CFMutableDictionaryRef alertDict               = NULL;  // must release
    SInt32                 userNotificationError   = 0;

    alertDict = createAlertDict(alertHeader, alertMessageArray);
    if (alertDict == NULL) {
        goto finish;
    }

    sUnsignedKextNotification =
    CFUserNotificationCreate(
                             kCFAllocatorDefault,
                             0 /* time interval */,
                             kCFUserNotificationCautionAlertLevel,
                             &userNotificationError,
                             alertDict );
    if (!sUnsignedKextNotification) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "Can't create user notification - %d",
                  (int)userNotificationError);
        goto finish;
    }

    sUnsignedKextNotificationRunLoopSource =
    CFUserNotificationCreateRunLoopSource(
                                          kCFAllocatorDefault,
                                          sUnsignedKextNotification,
                                          &_notificationDismissed,
                                          /* order */ 5 /* xxx - cheesy! */ );
    if (!sUnsignedKextNotificationRunLoopSource) {
        CFRelease(sUnsignedKextNotification);
        sUnsignedKextNotification = NULL;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                       sUnsignedKextNotificationRunLoopSource,
                       kCFRunLoopDefaultMode);
finish:
    SAFE_RELEASE(alertDict);

    return;
}
#endif

/*******************************************************************************
 *******************************************************************************/
static void kextd_raise_excludedkext_notification(
                                                  CFTypeRef alertHeader,
                                                  CFTypeRef alertMessageArray )
{
    CFMutableDictionaryRef alertDict               = NULL;  // must release
    SInt32                 userNotificationError   = 0;

    alertDict = createAlertDict(alertHeader, alertMessageArray);
    if (alertDict == NULL) {
        goto finish;
    }

    sExcludedKextNotification =
    CFUserNotificationCreate(
                             kCFAllocatorDefault,
                             0 /* time interval */,
                             kCFUserNotificationCautionAlertLevel,
                             &userNotificationError,
                             alertDict );
    if (!sExcludedKextNotification) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "Can't create user notification - %d",
                  (int)userNotificationError);
        goto finish;
    }

    sExcludedKextNotificationRunLoopSource =
    CFUserNotificationCreateRunLoopSource(
                                          kCFAllocatorDefault,
                                          sExcludedKextNotification,
                                          &_notificationDismissed,
                                          /* order */ 5 /* xxx - cheesy! */ );
    if (!sExcludedKextNotificationRunLoopSource) {
        CFRelease(sExcludedKextNotification);
        sExcludedKextNotification = NULL;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                       sExcludedKextNotificationRunLoopSource,
                       kCFRunLoopDefaultMode);
finish:
    SAFE_RELEASE(alertDict);

    return;
}

/*******************************************************************************
 *******************************************************************************/
void _notificationDismissed(
                            CFUserNotificationRef userNotification,
                            CFOptionFlags         responseFlags)
{
    if (sNonSecureNotification && sNonSecureNotification == userNotification) {
        CFRelease(sNonSecureNotification);
        sNonSecureNotification = NULL;
        if (sNonSecureNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sNonSecureNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sNonSecureNotificationRunLoopSource);
            sNonSecureNotificationRunLoopSource = NULL;
        }
    }
    else if (sNoLoadKextNotification && sNoLoadKextNotification == userNotification) {
        CFRelease(sNoLoadKextNotification);
        sNoLoadKextNotification = NULL;
        if (sNoLoadKextNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sNoLoadKextNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sNoLoadKextNotificationRunLoopSource);
            sNoLoadKextNotificationRunLoopSource = NULL;
        }
    }
    else if (sRevokedCertNotification && sRevokedCertNotification == userNotification) {
        CFRelease(sRevokedCertNotification);
        sRevokedCertNotification = NULL;
        if (sRevokedCertNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sRevokedCertNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sRevokedCertNotificationRunLoopSource);
            sRevokedCertNotificationRunLoopSource = NULL;
        }

        // CFArrayRef CFArrayCreateCopy
        if (sPendedRevokedCertKextPaths) {
            revealInFinder(sPendedRevokedCertKextPaths);
            CFArrayRemoveAllValues(sPendedRevokedCertKextPaths);
        } // sPendedRevokedCertKextPaths
    }
    else if (sInvalidSigNotification && sInvalidSigNotification == userNotification) {
        CFRelease(sInvalidSigNotification);
        sInvalidSigNotification = NULL;
        if (sInvalidSigNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sInvalidSigNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sInvalidSigNotificationRunLoopSource);
            sInvalidSigNotificationRunLoopSource = NULL;
        }
    }
#if 0 // not yet
    else if (sUnsignedKextNotification && sUnsignedKextNotification == userNotification) {
        CFRelease(sUnsignedKextNotification);
        sUnsignedKextNotification = NULL;
        if (sUnsignedKextNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sUnsignedKextNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sUnsignedKextNotificationRunLoopSource);
            sUnsignedKextNotificationRunLoopSource = NULL;
        }
    }
#endif
    else if (sExcludedKextNotification && sExcludedKextNotification == userNotification) {
        CFRelease(sExcludedKextNotification);
        sExcludedKextNotification = NULL;
        if (sExcludedKextNotificationRunLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                                  sExcludedKextNotificationRunLoopSource,
                                  kCFRunLoopDefaultMode);
            CFRelease(sExcludedKextNotificationRunLoopSource);
            sExcludedKextNotificationRunLoopSource = NULL;
        }
    }

    /* check to see if there are any other alerts pending */
    CFRunLoopSourceSignal(sNotificationQueueRunLoopSource);
    CFRunLoopWakeUp(CFRunLoopGetCurrent());

    return;
}

#include <ApplicationServices/ApplicationServices.h>
static const char kFinderBundleID[] = { "com.apple.finder" };

static void revealInFinder(CFArrayRef theArray)
{
    CFIndex     myCount, i;

    if (theArray == NULL)       return;

    myCount = CFArrayGetCount(theArray);

    for (i = 0; i < myCount; i ++) {
        CFStringRef         myKextPath          = NULL;  // do not release
        CFURLRef            myURL               = NULL;  // must release
        OSErr               myResult;
        AEDesc              myTargetDesc        = { typeNull, NULL };
        AEDesc              myFileDesc          = { typeNull, NULL };
        AEDescList          myParmList          = { typeNull, NULL };
        AppleEvent          myRevealEvent       = { typeNull, NULL };
        AppleEvent          myActivateEvent     = { typeNull, NULL };
        char                myPathString[2 * PATH_MAX];

        myKextPath = (CFStringRef) CFArrayGetValueAtIndex(theArray, i);
        if (myKextPath == NULL)   continue;

        /* NOTE - we create the URL from the path we are given then
         * extract the c string path from the URL in order to get the
         * correct URL prefix on the path.  The AppleEvent system requires
         * this.  Passing the full UNIX path does not work for AppleEvents
         */
        myURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                              myKextPath,
                                              kCFURLPOSIXPathStyle,
                                              true );
        if (myURL == NULL)  continue;
        myKextPath = CFURLGetString(myURL);

        if (myKextPath == NULL ||
            CFStringGetCString(myKextPath,
                               myPathString,
                               sizeof(myPathString),
                               kCFStringEncodingUTF8) == false) {
            SAFE_RELEASE_NULL(myURL);
            continue;
        }
        SAFE_RELEASE_NULL(myURL);

        myResult = AECreateDesc(typeApplicationBundleID,
                                &kFinderBundleID,
                                sizeof(kFinderBundleID),
                                &myTargetDesc);

        if (myResult == noErr) {
            myResult = AECreateDesc(typeFileURL,
                                    myPathString,
                                    strlen(myPathString),
                                    &myFileDesc);
        }
        if (myResult == noErr) {
            myResult = AECreateList(NULL, 0, false, &myParmList);
        }
        if (myResult == noErr) {
            AEPutDesc(&myParmList, 1, &myFileDesc);
        }
        if (myResult == noErr) {
            myResult = AECreateAppleEvent(kAEMiscStandards,
                                          kAESelect,
                                          &myTargetDesc,
                                          kAutoGenerateReturnID,
                                          kAnyTransactionID,
                                          &myRevealEvent);
        }
        if (myResult == noErr) {
            myResult = AEPutParamDesc(&myRevealEvent,
                                      keyDirectObject,
                                      &myParmList);
        }
        if (myResult == noErr) {
            myResult = AECreateAppleEvent(kAEMiscStandards,
                                          kAEActivate,
                                          &myTargetDesc,
                                          kAutoGenerateReturnID,
                                          kAnyTransactionID,
                                          &myActivateEvent);
        }
        if (myResult == noErr) {
            AESendMessage(&myActivateEvent, NULL, kAENoReply, 0);
            AESendMessage(&myRevealEvent, NULL, kAENoReply, 0);
        }
        AEDisposeDesc(&myTargetDesc);
        AEDisposeDesc(&myFileDesc);
        AEDisposeDesc(&myParmList);
        AEDisposeDesc(&myRevealEvent);
        AEDisposeDesc(&myActivateEvent);
    } // for loop...

    return;
}

/*******************************************************************************
 * writeKextAlertPlist() - update or create one of our alert plist files:
 *     invalidsignedkextalert.plist
 *     noloadkextalert.plist
 *     excludedkextalert.plist
 * with the given array of kext paths (CFStrings).
 * The key for the array in the plist dictionary is "Alerts sent".
 *
 * We use these plist files to control which kexts we have displayed an alert
 * dialog about.
 * Marketing only wanted us to alert once.  Accees to this routine needs to be
 * synchronized (all callers use a dispatch queue).
 *
 * The plist files are located at:
 * /System/Library/Caches/com.apple.kext.caches/Startup/
 *
 * The plist looks something like:
 <plist version="1.0">
 <dict>
    <key>Alerts sent</key>
    <array>
        <dict>
            <key>CFBundleIdentifier</key>
            <string>com.foocompany.driver.foo</string>
            <key>CFBundleVersion</key>
            <string>2.1</string>
            <key>KextPathKey</key>
            <string>/System/Library/Extensions/foo.kext</string>
        </dict>
    </array>
 </dict>
 </plist>
 *
 * see addKextToAlertDict() for layout of theDict.
 * NOTE - this routine must drop reference to theDict.
 *******************************************************************************/

void writeKextAlertPlist( CFDictionaryRef theDict, int theAlertType )
{
    CFArrayRef              myKextArray;             // do NOT release
    CFURLRef                myURL           = NULL;  // must release
    CFStringRef             myPath          = NULL;  // must release
    CFReadStreamRef         readStream      = NULL;  // must release
    CFWriteStreamRef        writeStream     = NULL;  // must release
    CFDictionaryRef         alertPlist      = NULL;  // must release
    CFMutableDictionaryRef  alertDict       = NULL;  // must release
    Boolean                 fileExists;
    Boolean                 closeReadStream     = false;
    Boolean                 closeWriteStream    = false;

    if (validateKextsAlertDict(theDict) != 0) {
        goto finish;
    }

    myKextArray = (CFArrayRef) CFDictionaryGetValue(theDict, CFSTR("KextInfoArrayKey"));
    myPath = createPathFromAlertType(NULL, theAlertType);
    if (myPath == NULL) {
        OSKextLogMemError();
        goto finish;
    }

    myURL = CFURLCreateWithFileSystemPath( kCFAllocatorDefault,
                                           myPath,
                                           kCFURLPOSIXPathStyle,
                                           false );
    if (myURL == NULL) {
        OSKextLogMemError();
        goto finish;
    }
    fileExists = CFURLResourceIsReachable(myURL, NULL);

    /* grab existing data and append to it */
    if (fileExists) {
        readStream = CFReadStreamCreateWithFile(kCFAllocatorDefault, myURL);
        if (readStream == NULL) {
            OSKextLogMemError();
            goto finish;
        }
        closeReadStream = CFReadStreamOpen(readStream);
        if (closeReadStream == false) {
            OSKextLogMemError();
            goto finish;
        }

        /* read in the existing contents of alert plist file */
        alertPlist = CFPropertyListCreateWithStream(
                                                    kCFAllocatorDefault,
                                                    readStream,
                                                    0,
                                                    kCFPropertyListMutableContainersAndLeaves,
                                                    NULL, NULL);
        if (alertPlist == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        CFMutableArrayRef sentArray = NULL;  // do not release
        sentArray = (CFMutableArrayRef)
                CFDictionaryGetValue(alertPlist, CFSTR("Alerts sent"));

        if (sentArray == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        /* add any kext paths that are not already known */
        Boolean     didAppend = false;

        didAppend = sendKextAlertNotifications(&sentArray, myKextArray, theAlertType);

        /* now replace previous plist with our updated one */
        if (didAppend) {
            writeStream = CFWriteStreamCreateWithFile(kCFAllocatorDefault, myURL);
            if (writeStream == NULL) {
                OSKextLogMemError();
                goto finish;
            }
            closeWriteStream = CFWriteStreamOpen(writeStream);
            if (closeWriteStream == false) {
                OSKextLogMemError();
                goto finish;
            }

            CFPropertyListWrite(alertPlist,
                                writeStream,
                                kCFPropertyListXMLFormat_v1_0,
                                0,
                                NULL);
        }
    }
    else {
        /* plist does not exist, create one */
        alertDict = CFDictionaryCreateMutable(
                                              kCFAllocatorDefault, 0,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);
        if (alertDict == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        /* add our array to the dictionary */
        CFDictionarySetValue(alertDict, CFSTR("Alerts sent"), myKextArray);

        alertPlist = CFPropertyListCreateDeepCopy(
                                                  kCFAllocatorDefault,
                                                  alertDict,
                                                  kCFPropertyListMutableContainersAndLeaves );
        if (alertPlist == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        writeStream = CFWriteStreamCreateWithFile(kCFAllocatorDefault, myURL);
        if (writeStream == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        closeWriteStream = CFWriteStreamOpen(writeStream);
        if (closeWriteStream == false) {
            OSKextLogMemError();
            goto finish;
        }

        CFPropertyListWrite(alertPlist,
                            writeStream,
                            kCFPropertyListXMLFormat_v1_0,
                            0,
                            NULL);

        sendKextAlertNotifications(NULL, myKextArray, theAlertType);
    }

finish:
#if 0
    if (alertPlist) {
        OSKextLogCFString(NULL,
                          kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                          CFSTR("%s: alertPlist %@"),
                          __func__, alertPlist);
    }
#endif
    if (closeReadStream)    CFReadStreamClose(readStream);
    if (closeWriteStream)   CFWriteStreamClose(writeStream);
    SAFE_RELEASE(myURL);
    SAFE_RELEASE(readStream);
    SAFE_RELEASE(writeStream);
    SAFE_RELEASE(alertPlist);
    SAFE_RELEASE(alertDict);
    SAFE_RELEASE(myPath);
    SAFE_RELEASE(theDict);

    return;
}


/*******************************************************************************
 * sendRevokedCertAlert() - build an array of kexts with revoked certs then
 * send to put up an alert.
 * NOTE - this routine must drop reference to theDict.
 *******************************************************************************/
void sendRevokedCertAlert( CFDictionaryRef theDict )
{
    CFArrayRef  myKextArray;             // do NOT release
    CFIndex     count, i;

    if (validateKextsAlertDict(theDict) != 0) {
        goto finish;
    }

    myKextArray = (CFArrayRef)
    CFDictionaryGetValue(theDict, CFSTR("KextInfoArrayKey"));
    if (myKextArray == NULL ||
        CFGetTypeID(myKextArray) != CFArrayGetTypeID() ) {
        goto finish;
    }

    count = CFArrayGetCount(myKextArray);
    for (i = 0; i < count; i++) {
        CFDictionaryRef         myDict;                 // do NOT release
        CFStringRef             myKextPath;             // do NOT release

        myDict = (CFDictionaryRef)
        CFArrayGetValueAtIndex(myKextArray, i);
        if (myDict == NULL ||
            CFGetTypeID(myDict) != CFDictionaryGetTypeID()) {
            continue;
        }

        myKextPath = CFDictionaryGetValue(myDict, CFSTR("KextPathKey"));
        recordRevokedCertKextPath(myKextPath);
    } // for loop...

    sendRevokedCertKextPath();

finish:
    SAFE_RELEASE(theDict);
    return;

}

/* theKextsArray is an array of dictionaries of kext info for kexts
 * we may want to send an alert about.  Each kext info dictionary
 * contains bundle ID, bundle version and kext path
 * (all are CFStrings)
 * If we have already alerted before there will be a list of kexts we
 * brought to the attention of the user.  We do not want to alert more than
 * once about the same kext or class of kexts when we have bundle ID to
 * mappings to a vendor or product.  theSentAlertsArray is an array of the
 * kexts we have alerted.
 */
// NOTE - we have decided to not use bundle mappings for the alert messages
// at this point.  So for now myMappingKey will always be NULL.
static Boolean sendKextAlertNotifications(CFMutableArrayRef *theSentAlertsArrayPtr,
                                          CFArrayRef theKextsArray,
                                          int theAlertType)
{
    Boolean     didAppend = false;
    CFIndex     count, i;

    count = CFArrayGetCount(theKextsArray);
    for (i = 0; i < count; i++) {
        CFDictionaryRef         myDict;                 // do NOT release
        CFStringRef             myKextMessage;          // do NOT release
        CFStringRef             myBundleID;             // do NOT release
        CFStringRef             myMappingKey = NULL;    // must release

        myDict = (CFDictionaryRef) CFArrayGetValueAtIndex(theKextsArray, i);
        if (myDict == NULL)   continue;

        myBundleID = (CFStringRef) CFDictionaryGetValue(myDict, kCFBundleIdentifierKey);
        myMappingKey = createBundleMappingKey(myBundleID);

        if (theSentAlertsArrayPtr) {
            /* skip this one if we already have alerted for it */
            if ( isInAlertsSentArray(*theSentAlertsArrayPtr, myDict, myMappingKey) ) {
                SAFE_RELEASE_NULL(myMappingKey);
                continue;
            }
            didAppend = true;
            CFArrayAppendValue(*theSentAlertsArrayPtr, myDict);
        }

        /* nag user about this kext */
        myKextMessage = getKextAlertMessage(myDict, myMappingKey);
        if (theAlertType == INVALID_SIGNATURE_KEXT_ALERT) {
            recordInvalidSignedKextPath(myKextMessage);
        }
        else if (theAlertType == NO_LOAD_KEXT_ALERT) {
            recordNoLoadKextPath(myKextMessage);
        }
        else if (theAlertType == EXCLUDED_KEXT_ALERT) {
            recordExcludedKextPath(myKextMessage);
        }
#if 0 // not yet
        else if (theAlertType == UNSIGNED_KEXT_ALERT) {
            recordUnsignedKextPath(myKextMessage);
        }
#endif
        SAFE_RELEASE_NULL(myMappingKey);
    } // for loop...

    if (theAlertType == INVALID_SIGNATURE_KEXT_ALERT) {
        sendInvalidSignedKextNotification();
    }
    else if (theAlertType == NO_LOAD_KEXT_ALERT) {
        sendNoLoadKextNotification();
    }
    else if (theAlertType == EXCLUDED_KEXT_ALERT) {
        sendExcludedKextNotification();
    }
#if 0 // not yet
    else if (theAlertType == UNSIGNED_KEXT_ALERT) {
        sendUnsignedKextNotification();
    }
#endif
    return(didAppend);
}


/* The alert message is either going to be the kext path or a product
 * or vendor name mapped from the bundle ID.
 */
// NOTE - we have decided to not use bundle mappings for the alert messages
// at this point.  So for now theMappingKey will always be NULL and this routine
// will always return the kext path.
static CFStringRef getKextAlertMessage(
                                       CFDictionaryRef theDict,
                                       CFStringRef theMappingKey )
{
    CFStringRef     myKextMessage = NULL;

    if (theMappingKey && sKextTranslationsPlist) {
        CFDictionaryRef     myMappingDict = NULL;       // do NOT release

        myMappingDict = (CFDictionaryRef)
            CFDictionaryGetValue(sKextTranslationsPlist,
                                 CFSTR("BundleMappings"));

        if (myMappingDict) {
            myKextMessage = (CFStringRef)
            CFDictionaryGetValue(myMappingDict,
                                 theMappingKey);
        }
    }

    if (myKextMessage == NULL) {
        /* fall back to kext path */
        myKextMessage = CFDictionaryGetValue(theDict,
                                             CFSTR("KextPathKey"));
    }

    return(myKextMessage);
}

/*******************************************************************************
 * writeKextLoadPlist() - update or create the plist file tracking kexts we
 * have loaded and message traced.
 * The key for the array in the plist dictionary is "Alerts sent".  >> todo change this key name?
 *
 * The plist files is located at:
 * /System/Library/Caches/com.apple.kext.caches/Startup/loadedkextmt.plist
 *
 * The plist looks something like:
cat loadedkextmt.plist
 <dict>
    <key>Alerts sent</key>
    <array>
        <dict>
            <key>com.apple.message.bundleID</key>
            <string>com.softraid.driver.SoftRAID</string>
            <key>com.apple.message.hash</key>
            <string>4567bcd500cf46ec773fc895902cfc33ebbaab00</string>
            <key>com.apple.message.kextname</key>
            <string>SoftRAID.kext</string>
            <key>com.apple.message.version</key>
            <string>4.4</string>
            </dict>
        <dict>
            <key>com.apple.message.bundleID</key>
            <string>com.promise.driver.stex</string>
            <key>com.apple.message.hash</key>
            <string>8d70fb592ec9e6581fbe7dc78b15e915fcbc0db8</string>
            <key>com.apple.message.kextname</key>
            <string>PromiseSTEX.kext</string>
            <key>com.apple.message.version</key>
            <string>5.1.62</string>
        </dict>
    </array>
 </dict>
 *
 */
 #define OBSOLETE_LOADED_KEXT_MT_ALERT_FULL_PATH \
 _kOSKextCachesRootFolder "/" \
 _kOSKextStartupCachesSubfolder "/" \
 "loadedkextmt.plist"

#define LOADED_KEXT_MT_ALERT_FULL_PATH \
"/var/db/loadedkextmt.plist"

static Boolean
MergeRequests(CFMutableDictionaryRef dict, CFStringRef key, CFArrayRef requests)
{
    CFMutableArrayRef existingRequests;
    Boolean           didAlter = false;
    CFIndex           count, i;

    if (!requests) {
        return (false);
    }
    existingRequests = (CFMutableArrayRef) CFDictionaryGetValue(dict, key);
    if (existingRequests == NULL) {
        didAlter = true;
        CFDictionarySetValue(dict, key, requests);
    } else {
        count = CFArrayGetCount(requests);
        for (i = 0; i < count; i++) {
            CFTypeRef value = CFArrayGetValueAtIndex(requests, i);
            if (!CFArrayContainsValue(existingRequests, RANGE_ALL(existingRequests), value)) {
                didAlter = true;
                CFArrayAppendValue(existingRequests, value);
            }
        }
    }
    return (didAlter);
}

void writeKextLoadPlist( CFArrayRef kextArray )
{
    CFURLRef                myURL           = NULL;  // must release
    CFReadStreamRef         readStream      = NULL;  // must release
    CFWriteStreamRef        writeStream     = NULL;  // must release
    CFMutableDictionaryRef  alertPlist      = NULL;  // must release
    CFMutableDictionaryRef  alertDict       = NULL;  // must release
    CFArrayRef              loadRequests    = NULL;  // must release
    CFMutableArrayRef       userRequests    = NULL;  // must release
    CFTypeRef               bundleID;
    Boolean                 fileExists;
    Boolean                 closeReadStream     = false;
    Boolean                 closeWriteStream    = false;
    Boolean                 didAlter            = false;
    CFIndex                 count, i;

    if (kextArray == NULL || CFArrayGetCount(kextArray) < 1) {
        goto finish;
    }
    loadRequests = OSKextCopyAllRequestedIdentifiers();

    userRequests = CFArrayCreateMutable(kCFAllocatorDefault,
                                        0,
                                        &kCFTypeArrayCallBacks);
    if (userRequests == NULL) {
        goto finish;
    }

    count = CFArrayGetCount(kextArray);
    for (i = 0; i < count; i++) {
        // the information for each kext is stored as a dictionary
        CFMutableDictionaryRef kextDict = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(kextArray, i);
        if (CFDictionaryGetValue(kextDict, CFSTR(kMessageTracerUserLoadKey))) {
            CFDictionaryRemoveValue(kextDict, CFSTR(kMessageTracerUserLoadKey));
            bundleID = CFDictionaryGetValue(kextDict, CFSTR(kMessageTracerBundleIDKey));
            if (!bundleID) {
                continue;
            }
            CFArrayAppendValue(userRequests, bundleID);
        }
    }

    unlink(OBSOLETE_LOADED_KEXT_MT_ALERT_FULL_PATH);

    myURL = CFURLCreateFromFileSystemRepresentation(
                                    kCFAllocatorDefault,
                                    (UInt8 *) LOADED_KEXT_MT_ALERT_FULL_PATH,
                                    strlen(LOADED_KEXT_MT_ALERT_FULL_PATH),
                                    false );
    if (myURL == NULL) {
        OSKextLogMemError();
        goto finish;
    }
    fileExists = CFURLResourceIsReachable(myURL, NULL);

    /* grab existing data */
    if (fileExists) {
        readStream = CFReadStreamCreateWithFile(kCFAllocatorDefault, myURL);
        if (readStream == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        closeReadStream = CFReadStreamOpen(readStream);
        if (closeReadStream == false) {
            OSKextLogMemError();
            goto finish;
        }

        /* read in the existing contents of plist */
        alertPlist = (CFMutableDictionaryRef) CFPropertyListCreateWithStream(
                                                    kCFAllocatorDefault,
                                                    readStream,
                                                    0,
                                                    kCFPropertyListMutableContainersAndLeaves,
                                                    NULL, NULL);
        if (alertPlist == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        /* merge in any new load requests */
        didAlter |= MergeRequests(alertPlist, CFSTR("All requests"), loadRequests);
        didAlter |= MergeRequests(alertPlist, CFSTR("User requests"), userRequests);

        /* add any kext paths that are not already known */

        CFMutableArrayRef sentArray = NULL;  // do not release
        sentArray = (CFMutableArrayRef)
            CFDictionaryGetValue(alertPlist, CFSTR("Alerts sent"));

        if (sentArray == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        count = CFArrayGetCount(kextArray);
        for (i = 0; i < count; i++) {
            // the information for each kext is stored as a dictionary
            CFDictionaryRef kextDict = CFArrayGetValueAtIndex(kextArray, i);
            if (!CFArrayContainsValue(sentArray, RANGE_ALL(sentArray), kextDict)) {
                didAlter = true;
                CFArrayAppendValue(sentArray, kextDict);
            }
        }

        /* now replace previous plist with our updated one */
        if (didAlter) {
            writeStream = CFWriteStreamCreateWithFile(kCFAllocatorDefault, myURL);
            if (writeStream == NULL) {
                OSKextLogMemError();
                goto finish;
            }
            closeWriteStream = CFWriteStreamOpen(writeStream);
            if (closeWriteStream == false) {
                OSKextLogMemError();
                goto finish;
            }

            CFPropertyListWrite(alertPlist,
                                writeStream,
                                kCFPropertyListXMLFormat_v1_0,
                                0,
                                NULL);
        }
    }
    else {
        /* plist does not exist, create one */
        alertDict = CFDictionaryCreateMutable(
                                              kCFAllocatorDefault, 0,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);
        if (alertDict == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        /* add our array to the dictionary */
        CFDictionarySetValue(alertDict, CFSTR("Alerts sent"), kextArray);
        if (loadRequests) {
            CFDictionarySetValue(alertDict, CFSTR("All requests"), loadRequests);
        }
        if (userRequests) {
            CFDictionarySetValue(alertDict, CFSTR("User requests"), userRequests);
        }

        alertPlist = (CFMutableDictionaryRef) CFPropertyListCreateDeepCopy(
                                                  kCFAllocatorDefault,
                                                  alertDict,
                                                  kCFPropertyListMutableContainersAndLeaves );
        if (alertPlist == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        writeStream = CFWriteStreamCreateWithFile(kCFAllocatorDefault, myURL);
        if (writeStream == NULL) {
            OSKextLogMemError();
            goto finish;
        }

        closeWriteStream = CFWriteStreamOpen(writeStream);
        if (closeWriteStream == false) {
            OSKextLogMemError();
            goto finish;
        }

        CFPropertyListWrite(alertPlist,
                            writeStream,
                            kCFPropertyListXMLFormat_v1_0,
                            0,
                            NULL);
    }

finish:
    if (closeReadStream)    CFReadStreamClose(readStream);
    if (closeWriteStream)   CFWriteStreamClose(writeStream);
    SAFE_RELEASE(myURL);
    SAFE_RELEASE(readStream);
    SAFE_RELEASE(writeStream);
    SAFE_RELEASE(alertPlist);
    SAFE_RELEASE(alertDict);
    SAFE_RELEASE(loadRequests);
    SAFE_RELEASE(userRequests);
    SAFE_RELEASE(kextArray);

    return;
}

/* the BundleMappings dictionary contains key / values that map
 * bundle IDs to product / vendor names.  The key is either a full
 * bundle ID or some partial bundle ID.  The more of the bundle ID
 * for a key that matches means a more specific match.  For example,
 * key com.foo.driver is a better match than a key with com.foo
 * Matching starts with a full bundle ID and works backwards to the
 * next '.'.  So if we had bundle ID com.foo.driver.bar we look for
 * matches in this order:  com.foo.driver.bar, com.foo.driver,
 * com.foo
 */
// NOTE - we have decided to not use bundle mappings for the alert messages
// at this point.  So for now this routine just bails out quickly because
// sKextTranslationsPlist will always be NULL.
static CFStringRef createBundleMappingKey( CFStringRef theBundleID )
{
    CFDictionaryRef     myMappingDict = NULL;       // do NOT release
    CFMutableStringRef  myMatchKey = NULL;          // do NOT release

    if (theBundleID == NULL || sKextTranslationsPlist == NULL) {
        goto finish;
    };

    /* see if there are any BundleMappings for this bundle ID */
    myMappingDict = (CFDictionaryRef)
        CFDictionaryGetValue(sKextTranslationsPlist,
                             CFSTR("BundleMappings"));

    if (myMappingDict == NULL) {
        goto finish;
    }

    myMatchKey = CFStringCreateMutableCopy(kCFAllocatorDefault,
                                           0, theBundleID);
    while (myMatchKey) {
        CFStringRef         myProductString = NULL; // do not release
        CFRange             myRange;

        myProductString = CFDictionaryGetValue(myMappingDict,
                                               myMatchKey);
        if (myProductString) {
            /* found a product string with this key so we are done! */
           break;
        }

        /* trim off anything from the last '.' including the '.' */
        myRange = CFStringFind(myMatchKey, CFSTR("."), kCFCompareBackwards);
        if (myRange.length != 0) {
            myRange.length = CFStringGetLength(myMatchKey) - myRange.location;
            CFStringDelete(myMatchKey, myRange);
            myRange = CFStringFind(myMatchKey, CFSTR("."), 0);
            if (myRange.length == 0) {
                /* we are done if no more '.'s left.  Clean up
                 * myMatchKey so we know there was no match.
                 */
                SAFE_RELEASE_NULL(myMatchKey);
            }
        }
    }

finish:
    return(myMatchKey);
}

/* This is the routine that controls our "alert only once" policy.
 * theSentArray is an array of kext info dictionaries for kexts we have
 * displayed an alert for.  We use kext bundle ID and version to determine
 * if a kext has been alerted.
 */
static Boolean isInAlertsSentArray(CFArrayRef theSentArray,
                                   CFDictionaryRef theDict,
                                   CFStringRef theMappingKey)
{
    Boolean             myResult = false;
    CFIndex             myCount, i;

    if (theSentArray == NULL || theDict == NULL)    return(false);

    if (CFArrayContainsValue(theSentArray, RANGE_ALL(theSentArray), theDict)) {
        return(true);
    }

    myCount = CFArrayGetCount(theSentArray);
    if (theMappingKey && myCount > 0) {
        for (i = 0; i < myCount; i++) {
            CFDictionaryRef     myKextAlertedDict;
            CFStringRef         myAlertedBundleID;

            myKextAlertedDict = (CFDictionaryRef)
                CFArrayGetValueAtIndex(theSentArray, i);
            if (myKextAlertedDict == NULL ||
                CFGetTypeID(myKextAlertedDict) != CFDictionaryGetTypeID()) {
                continue;
            }

            myAlertedBundleID = (CFStringRef)
            CFDictionaryGetValue(myKextAlertedDict, kCFBundleIdentifierKey);

            /* if our bundle mapping key matches any part of a bundle ID in
             * the sent alerts array then we have alreay messaged about
             * this class of kexts and should not alert again.
             */
            if (myAlertedBundleID &&
                CFGetTypeID(myAlertedBundleID) == CFStringGetTypeID()) {
                CFRange     myRange;
                myRange = CFStringFind(myAlertedBundleID, theMappingKey, 0);
                if (myRange.length > 0) {
                    myResult = true;
                    break;
                }
            }
        } // for loop...
    }

    return( myResult);
}


/* The full paths to each of the alert plist files currently supported
 */
#define INVALIDSIGNED_KEXT_ALERT_FULL_PATH   \
_kOSKextCachesRootFolder "/" \
_kOSKextStartupCachesSubfolder "/" \
"invalidsignedkextalert.plist"

#if 0 // not yet
#define UNSIGNED_KEXT_ALERT_FULL_PATH   \
_kOSKextCachesRootFolder "/" \
_kOSKextStartupCachesSubfolder "/" \
"unsignedkextalert.plist"
#endif

#define NO_LOAD_KEXT_ALERT_FULL_PATH   \
_kOSKextCachesRootFolder "/" \
_kOSKextStartupCachesSubfolder "/" \
"noloadkextalert.plist"

#define EXCLUDED_KEXT_ALERT_FULL_PATH   \
_kOSKextCachesRootFolder "/" \
_kOSKextStartupCachesSubfolder "/" \
"excludedkextalert.plist"

#define TRANSLATIONS_FULL_PATH   \
_kOSKextCachesRootFolder "/" \
_kOSKextStartupCachesSubfolder "/" \
"kexttranslation.plist"


static CFStringRef createPathFromAlertType( CFStringRef theVolRoot,
                                            int theAlertType )
{
    CFStringRef myPath              = NULL; // do NOT release

    if (theVolRoot) {
        if (theAlertType == INVALID_SIGNATURE_KEXT_ALERT) {
            myPath = CFStringCreateWithFormat(
                                              kCFAllocatorDefault,
                                              /* formatOptions */ NULL,
                                              CFSTR("%@%s"),
                                              theVolRoot,
                                              INVALIDSIGNED_KEXT_ALERT_FULL_PATH);
        }
#if 0 // not yet
        else if (theAlertType == UNSIGNED_KEXT_ALERT) {
            myPath = CFStringCreateWithFormat(
                                              kCFAllocatorDefault,
                                              /* formatOptions */ NULL,
                                              CFSTR("%@%s"),
                                              theVolRoot,
                                              UNSIGNED_KEXT_ALERT_FULL_PATH);
        }
#endif
        else if (theAlertType == NO_LOAD_KEXT_ALERT) {
            myPath = CFStringCreateWithFormat(
                                              kCFAllocatorDefault,
                                              /* formatOptions */ NULL,
                                              CFSTR("%@%s"),
                                              theVolRoot,
                                              NO_LOAD_KEXT_ALERT_FULL_PATH);
        }
        else if (theAlertType == EXCLUDED_KEXT_ALERT) {
            myPath = CFStringCreateWithFormat(
                                              kCFAllocatorDefault,
                                              /* formatOptions */ NULL,
                                              CFSTR("%@%s"),
                                              theVolRoot,
                                              EXCLUDED_KEXT_ALERT_FULL_PATH);
        }
        else {
            goto finish;
        }
#if 0 // disable this for now.  We will go with kext file name or path.
        // enable this if we decide to translate a kext bundle ID to a
        // product of vendor name.
        if (sKextTranslationsPlist == NULL) {
            myTranslatePath = CFStringCreateWithFormat(
                                                       kCFAllocatorDefault,
                                                       /* formatOptions */ NULL,
                                                       CFSTR("%@%s"),
                                                       theVolRoot,
                                                       TRANSLATIONS_FULL_PATH);
        }
#endif
    }
    else {
        if (theAlertType == INVALID_SIGNATURE_KEXT_ALERT) {
            myPath = CFStringCreateWithCString(
                                               kCFAllocatorDefault,
                                               INVALIDSIGNED_KEXT_ALERT_FULL_PATH,
                                               kCFStringEncodingUTF8 );
        }
#if 0 // not yet
        else if (theAlertType == UNSIGNED_KEXT_ALERT) {
            myPath = CFStringCreateWithCString(
                                               kCFAllocatorDefault,
                                               UNSIGNED_KEXT_ALERT_FULL_PATH,
                                               kCFStringEncodingUTF8 );
        }
#endif
        else if (theAlertType == NO_LOAD_KEXT_ALERT) {
            myPath = CFStringCreateWithCString(
                                               kCFAllocatorDefault,
                                               NO_LOAD_KEXT_ALERT_FULL_PATH,
                                               kCFStringEncodingUTF8 );
        }
        else if (theAlertType == EXCLUDED_KEXT_ALERT) {
            myPath = CFStringCreateWithCString(
                                               kCFAllocatorDefault,
                                               EXCLUDED_KEXT_ALERT_FULL_PATH,
                                               kCFStringEncodingUTF8 );
        }
        else {
            goto finish;
        }
#if 0 // disable this for now.  We will go with kext file name or path.
        // enable this if we decide to translate a kext bundle ID to a
        // product of vendor name.
        if (sKextTranslationsPlist == NULL) {
            myTranslatePath = CFStringCreateWithCString(
                                                        kCFAllocatorDefault,
                                                        TRANSLATIONS_FULL_PATH,
                                                        kCFStringEncodingUTF8 );
        }
#endif
    }
#if 0 // disable this for now.  We will go with kext file name or path.
    if (myTranslatePath) {
        CFURLRef    myURL           = NULL;  // must release

        myURL = CFURLCreateWithFileSystemPath( kCFAllocatorDefault,
                                              myTranslatePath,
                                              kCFURLPOSIXPathStyle,
                                              false );
        if (myURL && CFURLResourceIsReachable(myURL, NULL)) {
            CFReadStreamRef         readStream      = NULL;  // must release

            readStream = CFReadStreamCreateWithFile(kCFAllocatorDefault, myURL);
            if (readStream) {
                if (CFReadStreamOpen(readStream)) {
                    /* read in contents of kexttranslation.plist */
                    sKextTranslationsPlist = CFPropertyListCreateWithStream(
                                                                            kCFAllocatorDefault,
                                                                            readStream,
                                                                            0,
                                                                            kCFPropertyListMutableContainersAndLeaves,
                                                                            NULL, NULL);
                    CFReadStreamClose(readStream);
                }
                SAFE_RELEASE(readStream);
            }
        }
        SAFE_RELEASE(myURL);
    } /* myTranslatePath */
#endif

finish:
    //SAFE_RELEASE(myTranslatePath);

    return( myPath );
}


/*******************************************************************************
 * Do some sanity checking on this dictionary, do not trust the source.
 *******************************************************************************/
static int validateKextsAlertDict( CFDictionaryRef theDict )
{
    CFArrayRef  myKextArray;            // do NOT release
    CFIndex     count, i;
    int         result = -1;

    if (theDict == NULL || CFGetTypeID(theDict) != CFDictionaryGetTypeID()) {
        OSKextLogCFString(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                          CFSTR("%s invalid dictionary type \n"),
                          __func__);
        goto finish;
    }

    myKextArray = (CFArrayRef) CFDictionaryGetValue(theDict, CFSTR("KextInfoArrayKey"));
    if (myKextArray == NULL) {
        OSKextLogCFString(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                          CFSTR("%s null array \n"),
                          __func__);
        goto finish;
    }
    if ( CFGetTypeID(myKextArray) != CFArrayGetTypeID() ) {
        OSKextLogCFString(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                          CFSTR("%s invalid array type \n"),
                          __func__);
        goto finish;
    }
    if (CFArrayGetCount(myKextArray) < 1 || CFArrayGetCount(myKextArray) > 10) {
        OSKextLogCFString(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                          CFSTR("%s invalid array count %lu \n"),
                          __func__, CFArrayGetCount(myKextArray));
        goto finish;
    }

    count = CFArrayGetCount(myKextArray);
    for (i = 0; i < count; i++) {
        CFDictionaryRef         myDict;                 // do NOT release
        CFStringRef             myString;               // do NOT release

        myDict = (CFDictionaryRef) CFArrayGetValueAtIndex(myKextArray, i);
        if (myDict == NULL || CFGetTypeID(myDict) != CFDictionaryGetTypeID()) {
            OSKextLogCFString(/* kext */ NULL,
                              kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                              CFSTR("%s invalid kext array dictionary \n"),
                              __func__);
            goto finish;
        }

        if (CFDictionaryGetCount(myDict) > 3) {
            OSKextLogCFString(/* kext */ NULL,
                              kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                              CFSTR("%s invalid dictionary count %lu \n"),
                              __func__, CFDictionaryGetCount(myDict));
            goto finish;
        }

        myString = CFDictionaryGetValue(myDict, CFSTR("KextPathKey"));
        if (myString == NULL ||
            CFGetTypeID(myString) != CFStringGetTypeID() ||
            CFStringGetLength(myString) > 1024) {
            OSKextLogCFString(/* kext */ NULL,
                              kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                              CFSTR("%s invalid kext path value \n"),
                              __func__);
            goto finish;
        }

        myString = CFDictionaryGetValue(myDict, kCFBundleIdentifierKey);
        if (myString == NULL ||
            CFGetTypeID(myString) != CFStringGetTypeID() ||
            CFStringGetLength(myString) > 256) {
            OSKextLogCFString(/* kext */ NULL,
                              kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                              CFSTR("%s invalid kext bundle ID value \n"),
                              __func__);
            goto finish;
        }

        myString = CFDictionaryGetValue(myDict, kCFBundleVersionKey);
        if (myString == NULL ||
            CFGetTypeID(myString) != CFStringGetTypeID() ||
            CFStringGetLength(myString) > 256) {
            OSKextLogCFString(/* kext */ NULL,
                              kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                              CFSTR("%s invalid kext bundle version value \n"),
                              __func__);
            goto finish;
        }

    } // for loop...
    result = 0;

finish:
    return result;
}

/*******************************************************************************
 * Installer folks tell us the best way to determine if we are doing a system
 * install is to look for "/private/etc/rc.cdrom".
 *******************************************************************************/
Boolean doingSystemInstall(void)
{
    CFURLRef    myURL           = NULL;  // must release
    Boolean     result          = false;

    myURL = CFURLCreateWithString(NULL,
                                  CFSTR("file://localhost/private/etc/rc.cdrom"),
                                  NULL);
    if (myURL) {
        result = CFURLResourceIsReachable(myURL, NULL);
    }
    SAFE_RELEASE(myURL);
    return result;
}


#endif /* ifndef NO_CFUserNotification */
