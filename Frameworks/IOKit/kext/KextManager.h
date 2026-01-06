/*
 * Copyright (c) 2000-2008, 2012 Apple Inc. All rights reserved.
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
#ifndef __KEXTMANAGER_H__
#define __KEXTMANAGER_H__

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSReturn.h>

#include <sys/cdefs.h>

__BEGIN_DECLS

/*!
 * @header KextManager.h
 *
 * @abstract
 * The KextManager API provides a simple interface for applications
 * to load kernel extensions (kexts) via RPC to kextd, and to look up the
 * URLs for kexts by bundle identifier.
 */
 
/*!
 * @function KextManagerCreateURLForBundleIdentifier
 * @abstract Create a URL locating a kext with a given bundle identifier.
 *
 * @param    allocator
 *           The allocator to use to allocate memory for the new object.
 *           Pass <code>NULL</code> or <code>kCFAllocatorDefault</code>
 *           to use the current default allocator.
 * @param    kextIdentifier
 *           The bundle identifier to look up.
 *
 * @result
 * A CFURLRef locating a kext with the requested bundle identifier.
 * Returns <code>NULL</code> if the kext cannot be found, or on error.
 *
 * @discussion
 * Kexts are looked up first by whether they are loaded, second by version.
 * Specifically, if <code>kextIdentifier</code> identifies a kext
 * that is currently loaded,
 * the returned URL will locate that kext if it's still present on disk.
 * If the requested kext is not loaded,
 * or if its bundle is not at the location it was originally loaded from,
 * the returned URL will locate the latest version of the desired kext,
 * if one can be found within the system extensions folder.
 * If no version of the kext can be found, <code>NULL</code> is returned.
 */
CFURLRef KextManagerCreateURLForBundleIdentifier(
    CFAllocatorRef allocator,
    CFStringRef    kextIdentifier) AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;

/*!
 * @function KextManagerLoadKextWithIdentifier
 * @abstract
 * Request the kext loading system to load a kext with a given bundle identifier.
 *
 * @param    kextIdentifier
 *           The bundle identifier of the kext to look up and load.
 * @param    dependencyKextAndFolderURLs
 *           An array of additional URLs, of individual kexts and
 *           of folders that may contain kexts.
 *
 * @result
 * <code>kOSReturnSuccess</code> if the kext is successfully loaded
 * (or is already loaded), otherwise returns on error.
 *
 * @discussion
 * <code>kextIdentifier</code> is looked up in the system extensions
 * folder and among any kexts from <code>dependencyKextAndFolderURLs</code>.
 * Any non-kext URLs in <code>dependencyKextAndFolderURLs</code>
 * are scanned at the top level for kexts and plugins of kexts.
 *
 * Either the calling process must have an effective user id of 0 (superuser),
 * or the kext being loaded and all its dependencies must reside in
 * /System and have an OSBundleAllowUserLoad property of <code>true</code>.
 */
OSReturn KextManagerLoadKextWithIdentifier(
    CFStringRef    kextIdentifier,
    CFArrayRef     dependencyKextAndFolderURLs) AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function KextManagerLoadKextWithURL
 * @abstract
 * Request the kext loading system to load a kext with a given URL.
 *
 * @param    kextURL
 *           The URL of the kext to load.
 * @param    dependencyKextAndFolderURLs
 *           An array of additional URLs, of individual kexts and
 *           of folders that may contain kexts.
 *
 * @result
 * <code>kOSReturnSuccess</code> if the kext is successfully loaded
 * (or is already loaded), otherwise returns on error.
 *
 * @discussion
 * Any non-kext URLs in <code>dependencyKextAndFolderURLs</code>
 * are scanned at the top level for kexts and plugins of kexts.
 *
 * Either the calling process must have an effective user id of 0 (superuser),
 * or the kext being loaded and all its dependencies must reside in
 * /System and have an OSBundleAllowUserLoad property of <code>true</code>.
 */
OSReturn KextManagerLoadKextWithURL(
    CFURLRef    kextURL,
    CFArrayRef  dependencyKextAndFolderURLs) AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function KextManagerUnloadKextWithIdentifier
 * @abstract
 * Request the kernel to unload a kext with a given bundle identifier.
 *
 * @param    kextIdentifier
 *           The bundle identifier of the kext to unload.
 *
 * @result
 * <code>kOSReturnSuccess</code> if the kext is
 * found and successfully unloaded,
 * otherwise returns on error.
 * See <code>/usr/include/libkern/OSKextLib.h</code>
 * for error codes.
 *
 * @discussion
 * The calling process must have an effective user id of 0 (superuser).
 */
OSReturn KextManagerUnloadKextWithIdentifier(
    CFStringRef kextIdentifier) AVAILABLE_MAC_OS_X_VERSION_10_7_AND_LATER;

/*!
 * @function KextManagerCopyLoadedKextInfo
 * @abstract Returns information about loaded kexts in a dictionary.
 *
 * @param    kextIdentifiers   An array of kext identifiers to read from the kernel.
 *                             Pass <code>NULL</code> to read info for all loaded kexts.
 * @param    infoKeys          An array of info keys to read from the kernel.
 *                             Pass <code>NULL</code> to read all information.
 * @result
 * A dictionary, keyed by bundle identifier,
 * of dictionaries containing information about loaded kexts.
 *
 * @discussion
 * The information keys returned by this function are listed below.
 * Some are taken directly from the kext's information property list,
 * and some are generated at run time.
 * Never assume a given key will be present for a kext.
 *
 * <ul>
 *   <li><code>CFBundleIdentifier</code> - CFString</li>
 *   <li><code>CFBundleVersion</code> - CFString (note: version strings may be canonicalized
 *       but their numeric values will be the same; "1.2.0" may become "1.2", for example)</li>
 *   <li><code>OSBundleCompatibleVersion</code> - CFString</li>
 *   <li><code>OSBundleIsInterface</code> - CFBoolean</li>
 *   <li><code>OSKernelResource</code> - CFBoolean</li>
 *   <li><code>OSBundleCPUType</code> - CFNumber</li>
 *   <li><code>OSBundleCPUSubtype</code> - CFNumber</li>
 *   <li><code>OSBundlePath</code> - CFString (this is merely a hint stored in the kernel;
 *       the kext is not guaranteed to be at this path)</li>
 *   <li><code>OSBundleExecutablePath</code> - CFString
 *       (the absolute path to the executable within the kext bundle; a hint as above)</li>
 *   <li><code>OSBundleUUID</code> - CFData (the UUID of the kext executable, if it has one)</li>
 *   <li><code>OSBundleStarted</code> - CFBoolean (true if the kext is running)</li>
 *   <li><code>OSBundlePrelinked</code> - CFBoolean (true if the kext is loaded from a prelinked kernel)</li>
 *   <li><code>OSBundleLoadTag</code> - CFNumber (the "Index" given by kextstat)</li>
 *   <li><code>OSBundleLoadAddress</code> - CFNumber</li>
 *   <li><code>OSBundleLoadSize</code> - CFNumber</li>
 *   <li><code>OSBundleWiredSize</code> - CFNumber</li>
 *   <li><code>OSBundleDependencies</code> - CFArray of load tags identifying immediate link dependencies</li>
 *   <li><code>OSBundleRetainCount</code> - CFNumber (the OSObject retain count of the kext itself)</li>
 *   <li><code>OSBundleClasses</code> - CFArray of CFDictionary containing info on C++ classes
 *       defined by the kext:</li>
 *       <ul>
 *         <li><code>OSMetaClassName</code> - CFString</li>
 *         <li><code>OSMetaClassSuperclassName</code> - CFString, absent for root classes</li>
 *         <li><code>OSMetaClassTrackingCount</code> - CFNumber giving the instance count
 *             of the class itself, <i>plus</i> 1 for each direct subclass with any instances</li>
 *       </ul>
 * </ul>
 */
CFDictionaryRef KextManagerCopyLoadedKextInfo(
    CFArrayRef  kextIdentifiers,
    CFArrayRef  infoKeys) AVAILABLE_MAC_OS_X_VERSION_10_7_AND_LATER;


__END_DECLS

#endif /* __KEXTMANAGER_H__ */
