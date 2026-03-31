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
#ifndef __OSKEXTPRIVATE_H__
#define __OSKEXTPRIVATE_H__

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSKextLib.h>
#include <System/libkern/OSKextLibPrivate.h>
#include <mach-o/arch.h>
#include <paths.h>
#include <sys/stat.h>

#include "OSKext.h"

/* If you aren't IOKitUser or kext_tools, you shouldn't be using this
 * file. Its contents will change without warning.
 */
 
#if PRAGMA_MARK
/********************************************************************/
#pragma mark Standard System Extensions Folders
/********************************************************************/
#endif
/*
 * We use "/System/Library/Extensions", "/Library/Extensions", and /AppleInternal/Library/Extensions - 11860417
 * ... and /System/Library/DriverExtensions - 46043955, and /Library/Apple/System/Library/Extensions - 45895023
 */
#define _kOSKextNumSystemExtensionsFolders (6)

#define _kOSKextSystemLibraryExtensionsFolder           \
            "/System/Library/Extensions"
#define _kOSKextLibraryExtensionsFolder                 \
            "/Library/Extensions"
#define _kOSKextAppleInternalLibraryExtensionsFolder    \
            "/AppleInternal/Library/Extensions"
#define _kOSKextSystemLibraryDriverExtensionsFolder     \
            "/System/Library/DriverExtensions"
#define _kOSKextLibraryDriverExtensionsFolder           \
            "/Library/DriverExtensions"
#define _kOSKextLibraryAppleExtensionsFolder            \
            "/Library/Apple/System/Library/Extensions"

#if PRAGMA_MARK
/********************************************************************/
#pragma mark Kext Cache Folders & Files
/********************************************************************/
#endif
/*********************************************************************
* All kext cache files now live under /System/Library/Caches in
* com.apple.kext.caches. The system extensions folders are duplicated
* under this node, and at their bottom are the individual cache files
* for id->URL mapping, and for I/O Kit personalities (owned by the
* kext_tools project, specifically kextd(8) and kextcache(8)).
*
* Here's a schematic:
* ______________________________________________________________________
* /System/Library/Caches/com.apple.kext.caches/System/Library/Extensions/ ...
*     ID->URL Cache: KextIdentifiers.plist.gz (OSBundles?)
*     Personalities Cache: IOKitPersonalities_<arch>.plist.gz
*
* System boot caches (prelinked kernel and mkext) are symlinked in the
* com.apple.kext.caches folder. See the kext_tools project for more.
*********************************************************************/
#define _kOSKextCachesRootFolder                       \
    "/System/Library/Caches/com.apple.kext.caches"
#define _kOSKextDeferredBootcachesInstallScriptPath    \
    "/private/var/install/shove_kernels"
#define _kOSKextTemporaryPrelinkedKernelsPath          \
    "/Library/Apple/System/Library/PrelinkedKernels"
#define _kOSKextPrelinkedKernelsPath                   \
    "/System/Library/PrelinkedKernels"

#define _kOSKextDirectoryCachesSubfolder   "Directories"
#define _kOSKextStartupCachesSubfolder     "Startup"

#define _kOSKextStartupMkextFilename       "Extensions.mkext"
#define _kOSKextStartupMkextFolderPath     _kOSKextCachesRootFolder       "/" \
                                           _kOSKextStartupCachesSubfolder
#define _kOSKextStartupMkextPath           _kOSKextStartupMkextFolderPath "/" \
                                           _kOSKextStartupMkextFilename

#define _kOSKextIdentifierCacheBasename    "KextIdentifiers"
#define _kOSKextPrelinkedKernelBasename    "kernelcache"  // deprecated - use _kOSKextPrelinkedKernelFileName
#define _kOSKextPrelinkedKernelFileName    "prelinkedkernel"

#define _kOSKextCacheFileMode      (0644)
#define _kOSKextCacheFileModeMask  (0777)
#define _kOSKextCacheFolderMode    (0755)

#if PRAGMA_MARK
/********************************************************************/
#pragma mark Cache Functions
/********************************************************************/
#endif

extern char OSKextExecutableVariant[];

typedef enum {
    _kOSKextCacheFormatRaw,
    _kOSKextCacheFormatCFXML,
    _kOSKextCacheFormatCFBinary,
    _kOSKextCacheFormatIOXML,
} _OSKextCacheFormat;

Boolean _OSKextReadCache(
    CFTypeRef                 folderURLsOrURL,  // CFArray or CFURLRef
    CFStringRef               cacheName,
    const NXArchInfo        * arch,
    _OSKextCacheFormat        format,
    Boolean                   parseXMLFlag,
    CFPropertyListRef       * cacheContentsOut);
Boolean _OSKextCreateFolderForCacheURL(CFURLRef cacheURL);
Boolean _OSKextWriteCache(
    CFTypeRef                 folderURLsOrURL,  // CFArray or CFURLRef
    CFStringRef               cacheName,
    const NXArchInfo        * arch,
    _OSKextCacheFormat        format,
    CFPropertyListRef         plist);
Boolean _OSKextReadFromIdentifierCacheForFolder(
    CFURLRef            anURL,
    CFMutableArrayRef * kextsOut);
Boolean _OSKextWriteIdentifierCacheForKextsInDirectory(
    CFArrayRef kextArray,
    CFURLRef   directoryURL,
    Boolean    forceFlag);
CFArrayRef _OSKextCopyKernelRequests(void);
OSReturn _OSKextSendResource(
    CFDictionaryRef request,
    OSReturn        requestResult,
    CFDataRef       resource);
CFURLRef OSKextGetExecutableURL(OSKextRef aKext);
CFURLRef OSKextGetKernelExecutableURL(OSKextRef aKext);
CFURLRef OSKextGetUserExecutableURL(OSKextRef aKext);
CFStringRef OSKextCopyExecutableName(OSKextRef aKext);
bool _OSKextIdentifierHasApplePrefix(OSKextRef aKext);

/*!
 * @function OSKextSetTargetString
 * @abstract Set the name of the current running target.
 *
 * @result
 * <code>true</code>, if the target is successfully set.
 * <code>false</code>, otherwise.
 */
Boolean OSKextSetTargetString(const char * target);


/*!
 * @function OSKextSetTargetString
 * @abstract Get the name of the current running target.
 *
 * @result
 * A <code>CFStringRef</code> representation of the running target.
 */
CFStringRef OSKextGetTargetString(void);

#if PRAGMA_MARK
/********************************************************************/
#pragma mark URL Utilities
/********************************************************************/
#endif

CFStringRef _CFURLCopyAbsolutePath(CFURLRef anURL);

#if PRAGMA_MARK
/********************************************************************/
#pragma mark Misc Functions
/********************************************************************/
#endif

/* Used by embedded so they can better control which kexts they get.
 * This must be called when no kexts are opened.
 */
void _OSKextSetStrictRecordingByLastOpened(Boolean flag);

/* Used to determine if authentication checks should be performed
 * when doing dependency resolution, ensuring load lists reflect
 * only kexts that would pass authentication checks during linking.
 */
void _OSKextSetStrictAuthentication(Boolean flag);

/* Used to enable clients to provide custom authentication logic.
 * Ideally this should be called before kexts are opened, or at least
 * before they are processed, to ensure the authentic bit means the
 * same level of authentication on all OSKext objects in a client.
 *
 * The authentication function will be called with an OSKextRef to
 * authenticate and a context, provided when the function was registered.
 * It should return whether the kext passes authentication, and this
 * decision will be cached on the OSKext objects.
 */
typedef Boolean (*OSKextAuthFnPtr)(OSKextRef, void *);
void _OSKextSetAuthenticationFunction(OSKextAuthFnPtr authFn, void *context);

/* Used to allow clients to audit kext loads. After the client registers
 * their callback function, calls to OSKextLoad() will result in a call to this
 * function, provided that the kext has passed all error checking.
 */
typedef Boolean (*OSKextLoadAuditFnPtr)(OSKextRef);
void _OSKextSetLoadAuditFunction(OSKextLoadAuditFnPtr authFn);

/* Used to enable clients to add arbitrary keys to personalities of kexts before
 * they are returned from clients or sent to the kernel via
 * OSKextSendPersonalitiesOfKextsToKernel. If this function returns false, then
 * the personality will not be included. Clients should take special care not
 * to call any functions in this callback that result in calls to
 * OSKextCopyPersonalitiesOfKexts or OSKextCopyPersonalitiesArray.
 */
typedef Boolean (*OSKextPersonalityPatcherFnPtr)(OSKextRef, CFMutableDictionaryRef);
void _OSKextSetPersonalityPatcherFunction(OSKextPersonalityPatcherFnPtr patcherFn);

/* The basic filesystem authentication checks historically performed
 * by the OSKext API, exposed for use by custom authentication methods.
 */
Boolean
_OSKextBasicFilesystemAuthentication(OSKextRef aKext, __unused void *context);


#if PRAGMA_MARK
/********************************************************************/
#pragma mark Logging Macros for Common Errors
/********************************************************************/
#endif

#if DEBUG

#define OSKextLogMemError()   \
    OSKextLog(NULL, \
        kOSKextLogErrorLevel | kOSKextLogGeneralFlag, \
        "Error - memory allocation failure, %s, line %d.", __FILE__, __LINE__)
#define OSKextLogStringError(kext)   \
    OSKextLog((kext), \
        kOSKextLogErrorLevel | kOSKextLogGeneralFlag, \
        "Error - string/URL conversion failure, %s, line %d.", __FILE__, __LINE__)

#else /* DEBUG */

#define OSKextLogMemError()   \
    OSKextLog(NULL, \
        kOSKextLogErrorLevel | kOSKextLogGeneralFlag, \
        "Memory allocation failure.")
#define OSKextLogStringError(kext)   \
    OSKextLog((kext), \
        kOSKextLogErrorLevel | kOSKextLogGeneralFlag, \
        "String/URL conversion failure.")

#endif /* DEBUG */

#endif /* __OSKEXTPRIVATE_H__ */
