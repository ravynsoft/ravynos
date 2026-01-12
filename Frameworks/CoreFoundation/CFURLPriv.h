/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFURLPriv.h
	Copyright (c) 2008-2014, Apple Inc. All rights reserved.
        Responsibility: Jim Luther/Chris Linn
 */

#if !defined(__COREFOUNDATION_CFURLPRIV__)
#define __COREFOUNDATION_CFURLPRIV__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFFileSecurity.h>
#include <CoreFoundation/CFURLEnumerator.h>
#include <CoreFoundation/CFDate.h>
#if TARGET_OS_MAC
#include <sys/mount.h>
#endif

CF_EXTERN_C_BEGIN

// The kCFURLxxxxError enums are error codes in the Cocoa error domain and they mirror the exact same codes in <Foundation/FoundationErrors.h> (i.e. kCFURLReadNoPermissionError = NSFileReadNoPermissionError = 257). They were added to CFURLPriv.h so that CarbonCore and later CoreServicesInternal could return these error codes in the Cocoa error domain. If your code links with Foundation, you should use the codes in <Foundation/FoundationErrors.h>, not these codes.
enum {
    // Resource I/O related errors, with kCFErrorURLKey containing URL
    kCFURLNoSuchResourceError = 4,			   // Attempt to do a file system operation on a non-existent file
    kCFURLResourceLockingError = 255,			   // Couldn't get a lock on file
    kCFURLReadUnknownError = 256,                          // Read error (reason unknown)
    kCFURLReadNoPermissionError = 257,                     // Read error (permission problem)
    kCFURLReadInvalidResourceNameError = 258,              // Read error (invalid file name)
    kCFURLReadCorruptResourceError = 259,                  // Read error (file corrupt, bad format, etc)
    kCFURLReadNoSuchResourceError = 260,                   // Read error (no such file)
    kCFURLReadInapplicableStringEncodingError = 261,       // Read error (string encoding not applicable) also kCFStringEncodingErrorKey
    kCFURLReadUnsupportedSchemeError = 262,		   // Read error (unsupported URL scheme)
    kCFURLReadTooLargeError = 263,			   // Read error (file too large)
    kCFURLReadUnknownStringEncodingError = 264,		   // Read error (string encoding of file contents could not be determined)
    kCFURLWriteUnknownError = 512,			   // Write error (reason unknown)
    kCFURLWriteNoPermissionError = 513,                    // Write error (permission problem)
    kCFURLWriteInvalidResourceNameError = 514,             // Write error (invalid file name)
    kCFURLWriteInapplicableStringEncodingError = 517,      // Write error (string encoding not applicable) also kCFStringEncodingErrorKey
    kCFURLWriteUnsupportedSchemeError = 518,		   // Write error (unsupported URL scheme)
    kCFURLWriteOutOfSpaceError = 640,                      // Write error (out of storage space)
    kCFURLWriteVolumeReadOnlyError = 642,		   // Write error (readonly volume)
} CF_ENUM_AVAILABLE(10_5, 2_0);


/*
    Private File System Property Keys
*/
CF_EXPORT const CFStringRef _kCFURLPathKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLPathKey or NSURLPathKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIDKey CF_AVAILABLE(10_6, 4_0);
    /* Volume ID (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLInodeNumberKey CF_AVAILABLE(10_6, 4_0);
    /* 64-bit inode number (the inode number from the file system) (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLFileIDKey CF_AVAILABLE(10_6, 4_0);
    /* 64-bit file ID (for tracking a file by ID. This may or may not be the inode number) (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLParentDirectoryIDKey CF_AVAILABLE(10_6, 4_0);
    /* 64-bit file ID (for tracking a parent directory by ID. This may or may not be the inode number) (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLDistinctLocalizedNameKey CF_AVAILABLE(10_6, 4_0);
    /* The localized name, if it is distinct from the real name. Otherwise, NULL (CFString) */

CF_EXPORT const CFStringRef _kCFURLNameExtensionKey CF_AVAILABLE(10_6, 4_0);
    /* The name extension (CFString) */

CF_EXPORT const CFStringRef _kCFURLFinderInfoKey CF_AVAILABLE(10_6, 4_0);
    /* A 16-byte Finder Info structure immediately followed by a 16-byte Extended Finder Info structure (CFData) */

CF_EXPORT const CFStringRef _kCFURLIsCompressedKey CF_AVAILABLE(10_6, 4_0);
    /* True if resource's data is transparently compressed by the system on its storage device (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsApplicationKey CF_AVAILABLE(10_6, 4_0);
    /* True if resource is an application (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCanSetHiddenExtensionKey CF_AVAILABLE(10_6, 4_0);
    /* True if the filename extension can be hidden or unhidden (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsReadableKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLIsReadableKey or NSURLIsReadableKey public property keys */
/* never implemented and scheduled for removal in 10.10/8.0 */CF_EXPORT const CFStringRef _kCFURLUserCanReadKey CF_DEPRECATED(10_0, 10_6, 2_0, 4_0);

CF_EXPORT const CFStringRef _kCFURLIsWriteableKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLIsWritableKey or NSURLIsWritableKey public property keys */
/* never implemented and scheduled for removal in 10.10/8.0 */CF_EXPORT const CFStringRef _kCFURLUserCanWriteKey CF_DEPRECATED(10_0, 10_6, 2_0, 4_0);

CF_EXPORT const CFStringRef _kCFURLIsExecutableKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLIsExecutableKey or NSURLIsExecutableKey public property keys */
/* never implemented and scheduled for removal in 10.10/8.0 */CF_EXPORT const CFStringRef _kCFURLUserCanExecuteKey CF_DEPRECATED(10_0, 10_6, 2_0, 4_0);

CF_EXPORT const CFStringRef _kCFURLParentDirectoryIsVolumeRootKey CF_AVAILABLE(10_6, 4_0);
    /* True if the parent directory is the root of a volume (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLFileSecurityKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLFileSecurityKey or NSURLFileSecurityKey public property keys */

CF_EXPORT const CFStringRef _kCFURLFileSizeOfResourceForkKey CF_AVAILABLE(10_6, 4_0);
    /* Size in bytes of the resource fork (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLFileAllocatedSizeOfResourceForkKey CF_AVAILABLE(10_6, 4_0);
    /* Size in bytes of the blocks allocated for the resource fork (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLEffectiveIconImageDataKey CF_AVAILABLE(10_6, 4_0);
    /* Icon image data, i.e. raw pixel data (CFData) */

CF_EXPORT const CFStringRef _kCFURLTypeBindingKey CF_AVAILABLE(10_10, 8_0);
    /* Type binding for icon (Read-only, value type CFData) */

CF_EXPORT const CFStringRef _kCFURLCustomIconImageDataKey CF_AVAILABLE(10_6, 4_0);
    /* Icon image data of the item's custom icon, if any (CFData) */

CF_EXPORT const CFStringRef _kCFURLEffectiveIconFlattenedReferenceDataKey CF_AVAILABLE(10_6, 4_0);
    /* Icon flattened reference, suitable for cheaply sharing the effective icon reference across processess (CFData) */

CF_EXPORT const CFStringRef _kCFURLBundleIdentifierKey CF_AVAILABLE(10_6, 4_0);
    /* If resource is a bundle, the bundle identifier (CFString) */

CF_EXPORT const CFStringRef _kCFURLVersionKey CF_AVAILABLE(10_6, 4_0);
    /* If resource is a bundle, the bundle version (CFBundleVersion) as a string (CFString) */

CF_EXPORT const CFStringRef _kCFURLShortVersionStringKey CF_AVAILABLE(10_6, 4_0);
    /* If resource is a bundle, the bundle short version (CFBundleShortVersionString) as a string (CFString) */

CF_EXPORT const CFStringRef _kCFURLOwnerIDKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal later in 10.9/7.0 since it is unused - Use the kCFURLFileSecurityKey or NSURLFileSecurityKey public property keys and CFFileSecurityGetOwner() */

CF_EXPORT const CFStringRef _kCFURLGroupIDKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal later in 10.9/7.0 since it is unused - Use the kCFURLFileSecurityKey or NSURLFileSecurityKey public property keys and CFFileSecurityGetGroup() */

CF_EXPORT const CFStringRef _kCFURLStatModeKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal later in 10.9/7.0 since it is unused - Use the kCFURLFileSecurityKey or NSURLFileSecurityKey public property keys and CFFileSecurityGetMode() */

CF_EXPORT const CFStringRef _kCFURLLocalizedNameDictionaryKey CF_AVAILABLE(10_7, NA);
    /* For items with localized display names, the dictionary of all available localizations. The keys are the cannonical locale strings for the available localizations. (CFDictionary) */

CF_EXPORT const CFStringRef _kCFURLLocalizedTypeDescriptionDictionaryKey CF_AVAILABLE(10_7, NA);
    /* The dictionary of all available localizations of the item kind string. The keys are the cannonical locale strings for the available localizations. (CFDictionary) */

CF_EXPORT const CFStringRef _kCFURLApplicationCategoriesKey CF_AVAILABLE(10_7, NA);
    /* The array of category UTI strings associated with the url. (CFArray) */

CF_EXPORT const CFStringRef _kCFURLApplicationHighResolutionModeIsMagnifiedKey CF_AVAILABLE(10_7, NA);
    /* True if the app runs with magnified 1x graphics on a 2x display (Per-user, CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCanSetApplicationHighResolutionModeIsMagnifiedKey CF_AVAILABLE(10_7, NA);
    /* True if the app can run in either magnified or native resolution modes (Read only, CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLWriterBundleIdentifierKey CF_AVAILABLE(10_8, NA);
    /* The bundle identifier of the process writing to this object (Read-write, value type CFString) */

CF_EXPORT const CFStringRef _kCFURLApplicationNapIsDisabledKey CF_AVAILABLE(10_9, NA);
    /* True if app nap is disabled (Applications only, Per-user, CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCanSetApplicationNapIsDisabledKey CF_AVAILABLE(10_9, NA);
    /* True if the ApplicationNapIsDisabled property value can be changed (Applications only, Read only, CFBoolean) */

/* Additional volume properties */

CF_EXPORT const CFStringRef _kCFURLVolumeRefNumKey CF_AVAILABLE(10_6, 4_0);
    /* The Carbon File Manager's FSVolumeRefNum for the resource volume (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLVolumeUUIDStringKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeUUIDStringKey or NSURLVolumeUUIDStringKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeCreationDateKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeCreationDateKey or NSURLVolumeCreationDateKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIsLocalKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsLocalKey or NSURLVolumeIsLocalKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIsAutomountKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsAutomountedKey or NSURLVolumeIsAutomountedKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeDontBrowseKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsBrowsableKey or NSURLVolumeIsBrowsableKey public property keys (Note: value is inverse of _kCFURLVolumeDontBrowseKey) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsReadOnlyKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsReadOnlyKey or NSURLVolumeIsReadOnlyKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIsQuarantinedKey CF_AVAILABLE(10_6, 4_0);
    /* Mounted quarantined (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsEjectableKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsEjectableKey or NSURLVolumeIsEjectableKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIsRemovableKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsRemovableKey or NSURLVolumeIsRemovableKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIsInternalKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsInternalKey or NSURLVolumeIsInternalKey public property keys (Note: this has slightly different behavior than the public VolumeIsInternal key) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsExternalKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsInternalKey or NSURLVolumeIsInternalKey public property keys (Note: this has slightly different behavior than the public VolumeIsInternal key) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsDiskImageKey CF_AVAILABLE(10_6, 4_0);
    /* Volume is a mounted disk image (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLDiskImageBackingURLKey CF_AVAILABLE(10_6, 4_0);
    /* If volume is a mounted disk image, the URL of the backing disk image (CFURL) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsFileVaultKey CF_AVAILABLE(10_6, 4_0);
    /* Volume uses File Vault encryption (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsiDiskKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - there are no more iDisks */

CF_EXPORT const CFStringRef _kCFURLVolumeiDiskUserNameKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - there are no more iDisks */

CF_EXPORT const CFStringRef _kCFURLVolumeIsLocaliDiskMirrorKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - there are no more iDisks */

CF_EXPORT const CFStringRef _kCFURLVolumeIsiPodKey CF_AVAILABLE(10_6, 4_0);
    /* Volume is on an iPod (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsCDKey CF_AVAILABLE(10_6, 4_0);
    /* Volume is a CD (audio or CD-ROM). (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsDVDKey CF_AVAILABLE(10_6, 4_0);
    /* Volume is a DVD (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsDeviceFileSystemKey CF_AVAILABLE(10_7, 5_0);
    /* Volume is devfs (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsHFSStandardKey CF_AVAILABLE(10_6, 4_0);
    /* Volume is HFS standard (which includes AFP volumes). Directory IDs, but not file IDs, can be looked up. (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIOMediaIconFamilyNameKey CF_AVAILABLE(10_9, NA);
    /* Volume's IOMediaIconFamilyName. (CFStringRef) */

CF_EXPORT const CFStringRef _kCFURLVolumeIOMediaIconBundleIdentifierKey CF_AVAILABLE(10_9, NA);
    /* Volume's IOMediaIconBundleIdentifier. (CFStringRef) */

CF_EXPORT const CFStringRef _kCFURLVolumeQuarantinePropertiesKey CF_AVAILABLE(10_10, NA);
    /* The quarantine properties for the volume on which the resource resides as defined in LSQuarantine.h.=To remove quarantine information from a volume, pass kCFNull as the value when setting this property. (Read-write, value type CFDictionary) */

CF_EXPORT const CFStringRef _kCFURLVolumeOpenFolderURLKey CF_AVAILABLE(10_10, NA);
    /* Returns a URL to the folder the Finder should open when a HFS volume is mounted, or NULL if there is none. (Read-only, value type CFURL) */

CF_EXPORT const CFStringRef _kCFURLResolvedFromBookmarkDataKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal later in 10.9/7.0 since it is unused (*/

CF_EXPORT const CFStringRef _kCFURLVolumeMountPointStringKey CF_AVAILABLE(10_6, 4_0);
    /*	the volume mountpoint string (Read-only, value type CFString) */

CF_EXPORT const CFStringRef _kCFURLCompleteMountURLKey CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeURLForRemountingKey or NSURLVolumeURLForRemountingKey public property keys */

CF_EXPORT const CFStringRef _kCFURLUbiquitousItemDownloadRequestedKey CF_AVAILABLE(10_9, 7_0);
    /* Is this Ubiquity item scheduled for download? (this is also true for items that are already downloaded). Use startDownloadingUbiquitousItemAtURL:error: to make this true (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCloudDocsPlaceholderDictionaryKey CF_AVAILABLE(10_10, 8_0);
    /* Returns the placeholder dictionary for a side-fault file (Read-only, value type CFDictionary) */

CF_EXPORT const CFStringRef _kCFURLCloudDocsPlaceholderLogicalNameKey CF_AVAILABLE(10_10, 8_0);
    /* Returns the placeholder dictionary for a side-fault file (Read-only, value type CFString) */

// Temporary holding place for future API.

CF_EXPORT const CFStringRef kCFURLUbiquitousItemDownloadRequestedKey CF_AVAILABLE(10_9, 7_0);
/* Is this Ubiquity item scheduled for download? (this is also true for items that are already downloaded). Use startDownloadingUbiquitousItemAtURL:error: to make this true (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef kCFURLUbiquitousItemContainerDisplayNameKey CF_AVAILABLE(10_10, 8_0);
    /* Returns the localized name of the ubiquity container that contains this item (Read-only, value type CFString) */

CF_EXPORT const CFStringRef kCFURLUbiquitousItemIsSharedKey; // true if the ubiquitous item is shared. (Read-only, value type boolean NSNumber)

CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemRoleKey /*CF_AVAILABLE(10_11, 9_0)*/; // returns the current user's role for this shared item, or nil if not shared. (Read-only, value type NSString). Possible values below.
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemRoleOwner /*CF_AVAILABLE(10_11, 9_0)*/; // the current user is the owner of this shared item.
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemRoleParticipant /*CF_AVAILABLE(10_11, 9_0)*/; // the current user is a participant of this shared item.

CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemOwnerNameKey /*CF_AVAILABLE(10_11, 9_0)*/; // returns the name of the owner of a shared item, or nil if the current user. (Read-only, value type NSString)

CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemPermissionsKey /*CF_AVAILABLE(10_11, 9_0)*/; // returns the current user's permissions for this shared item. (Read-only, value type NSString). Possible values below.
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemPermissionsReadOnly /*CF_AVAILABLE(10_11, 9_0)*/; // the user is only allowed to read this item
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemPermissionsReadWrite /*CF_AVAILABLE(10_11, 9_0)*/; // the user is allowed to both read and write this item

// Deprecated. Will be removed.
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemReadOnlyPermissions /*CF_AVAILABLE(10_11, 9_0)*/; // the user is only allowed to read this item
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemReadWritePermissions /*CF_AVAILABLE(10_11, 9_0)*/; // the user is allowed to both read and write this item


// these keys are defined here, not in CFURL.h, because they return NSImage values which can only be used by Foundation
CF_EXPORT const CFStringRef kCFURLThumbnailDictionaryKey CF_AVAILABLE(10_10, 8_0);
CF_EXPORT const CFStringRef kCFURLThumbnailKey CF_AVAILABLE(10_10, 8_0);
// The values of thumbnails in the dictionary returned by NSURLThumbnailDictionaryKey
CF_EXPORT const CFStringRef kCFThumbnail1024x1024SizeKey CF_AVAILABLE(10_10, 8_0);


/*
    Some common boolean properties can be accessed as a bitfield
    for better performance -- see _CFURLGetResourcePropertyFlags() and
    _CFURLCopyResourcePropertyValuesAndFlags(), below.
 */
enum {
    kCFURLResourceIsRegularFile         = 0x00000001,
    kCFURLResourceIsDirectory           = 0x00000002,
    kCFURLResourceIsSymbolicLink        = 0x00000004,
    kCFURLResourceIsVolume              = 0x00000008,
    kCFURLResourceIsPackage             = 0x00000010,
    kCFURLResourceIsSystemImmutable     = 0x00000020,
    kCFURLResourceIsUserImmutable       = 0x00000040,
    kCFURLResourceIsHidden              = 0x00000080,
    kCFURLResourceHasHiddenExtension    = 0x00000100,
    kCFURLResourceIsApplication         = 0x00000200,
    kCFURLResourceIsCompressed          = 0x00000400,
    kCFURLResourceIsSystemCompressed CF_ENUM_DEPRECATED(10_6, 10_9, 4_0, 7_0)
                                        = 0x00000400,  /* Deprecated and scheduled for removal in 10.10/8.0 - Use kCFURLResourceIsCompressed */
    kCFURLCanSetHiddenExtension         = 0x00000800,
    kCFURLResourceIsReadable		= 0x00001000,
    kCFURLResourceIsWriteable		= 0x00002000,
    kCFURLResourceIsExecutable		= 0x00004000,   /* execute files or search directories */
    kCFURLIsAliasFile                   = 0x00008000,
    kCFURLIsMountTrigger		= 0x00010000,
};
typedef unsigned long long CFURLResourcePropertyFlags;


/*
    _CFURLGetResourceFlags - Returns a bit array of resource flags in the "flags"
    output parameter. Only flags whose corresponding bits are set in the "mask" parameter
    are valid in the output bit array. Returns true on success, false if an error occurs.
    Optional output error: the error is set to a valid CFErrorRef if and only if the function 
    returns false. A valid output error must be released by the caller.
 */
CF_EXPORT
Boolean _CFURLGetResourcePropertyFlags(CFURLRef url, CFURLResourcePropertyFlags mask, CFURLResourcePropertyFlags *flags, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);


/*
    File resource properties which can be obtained with _CFURLCopyFilePropertyValuesAndFlags().
 */
typedef CF_OPTIONS(unsigned long long, CFURLFilePropertyBitmap) {
    kCFURLName				    = 0x0000000000000001,
    kCFURLLinkCount			    = 0x0000000000000002,
    kCFURLVolumeIdentifier		    = 0x0000000000000004,
    kCFURLObjectIdentifier		    = 0x0000000000000008,
    kCFURLCreationDate			    = 0x0000000000000010,
    kCFURLContentModificationDate	    = 0x0000000000000020,
    kCFURLAttributeModificationDate	    = 0x0000000000000040,
    kCFURLFileSize			    = 0x0000000000000080,
    kCFURLFileAllocatedSize		    = 0x0000000000000100,
    kCFURLFileSizeOfResourceFork	    = 0x0000000000000200,
    kCFURLFileAllocatedSizeOfResourceFork   = 0x0000000000000400,
    kCFURLFinderInfo			    = 0x0000000000000800,
    kCFURLFileSecurity			    = 0x0000000000001000,
};

/*
    The structure where _CFURLCopyFilePropertyValuesAndFlags() returns file resource properties.
 */
struct _CFURLFilePropertyValues {
    CFStringRef		name;		/* you are responsible for releasing this if you ask for it and get it */
    uint32_t		linkCount;
    uint64_t		volumeIdentifier;
    uint64_t		objectIdentifier;
    CFAbsoluteTime	creationDate;
    CFAbsoluteTime	contentModificationDate;
    CFAbsoluteTime	attributeModificationDate;
    uint64_t		fileSize;
    uint64_t		fileAllocatedSize;
    uint64_t		fileSizeOfResourceFork;
    uint64_t		fileAllocatedSizeOfResourceFork;
    uint8_t		finderInfo[32];
    CFFileSecurityRef	fileSecurity;	/* you are responsible for releasing this if you ask for it and get it */
};
typedef struct _CFURLFilePropertyValues _CFURLFilePropertyValues;

/*
    _CFURLCopyResourcePropertyValuesAndFlags - Returns property values as simple types
    whenever possible. Returns a bit array of resource flags in the "flags"
    output parameter. Only flags whose corresponding bits are set in the "mask" parameter
    are valid in the output bit array. Returns true on success, false if an error occurs.
    Optional output error: the error is set to a valid CFErrorRef if and only if the function 
    returns false. A valid output error must be released by the caller.
 */
CF_EXPORT
Boolean _CFURLCopyResourcePropertyValuesAndFlags( CFURLRef url, CFURLFilePropertyBitmap requestProperties, CFURLFilePropertyBitmap *actualProperties, struct _CFURLFilePropertyValues *properties, CFURLResourcePropertyFlags propertyFlagsMask, CFURLResourcePropertyFlags *propertyFlags, CFErrorRef *error) CF_AVAILABLE(10_7, 4_0);

/*
    Volume property flags
 */
typedef CF_OPTIONS(unsigned long long, CFURLVolumePropertyFlags) {
	kCFURLVolumeIsLocal				=                0x1LL,	// Local device (vs. network device)
	kCFURLVolumeIsAutomount				=                0x2LL,	// Mounted by the automounter
	kCFURLVolumeDontBrowse				=                0x4LL,	// Hidden from user browsing
	kCFURLVolumeIsReadOnly				=                0x8LL,	// Mounted read-only
	kCFURLVolumeIsQuarantined		        =               0x10LL,	// Mounted with quarantine bit
	kCFURLVolumeIsEjectable				=               0x20LL,
	kCFURLVolumeIsRemovable				=               0x40LL,
	kCFURLVolumeIsInternal				=               0x80LL,
	kCFURLVolumeIsExternal				=              0x100LL,
	kCFURLVolumeIsDiskImage				=              0x200LL,
	kCFURLVolumeIsFileVault				=              0x400LL,
	kCFURLVolumeIsLocaliDiskMirror CF_ENUM_DEPRECATED(10_6, 10_9, 4_0, 7_0)
                                                        =              0x800LL, // Deprecated and scheduled for removal in 10.10/8.0 - there are no more iDisks
	kCFURLVolumeIsiPod				=             0x1000LL,
	kCFURLVolumeIsiDisk CF_ENUM_DEPRECATED(10_6, 10_9, 4_0, 7_0)
                                                        =             0x2000LL, // Deprecated and scheduled for removal in 10.10/8.0 - there are no more iDisks
	kCFURLVolumeIsCD				=             0x4000LL,
	kCFURLVolumeIsDVD				=             0x8000LL,
	kCFURLVolumeIsDeviceFileSystem			=	     0x10000LL,
        kCFURLVolumeIsTimeMachine CF_ENUM_AVAILABLE_MAC(10_9)
                                                        =	     0x20000LL,
        kCFURLVolumeIsAirport CF_ENUM_AVAILABLE_MAC(10_9)
                                                        =	     0x40000LL,
        kCFURLVolumeIsVideoDisk CF_ENUM_AVAILABLE_MAC(10_9)
                                                        =	     0x80000LL,
        kCFURLVolumeIsDVDVideo CF_ENUM_AVAILABLE_MAC(10_9)
                                                        =	    0x100000LL,
        kCFURLVolumeIsBDVideo CF_ENUM_AVAILABLE_MAC(10_9)
                                                        =	    0x200000LL,
        kCFURLVolumeIsMobileTimeMachine CF_ENUM_AVAILABLE_MAC(10_9)
                                                        =	    0x400000LL,
        kCFURLVolumeIsNetworkOptical CF_ENUM_AVAILABLE_MAC(10_9)
                                                        =	    0x800000LL,
        kCFURLVolumeIsBeingRepaired CF_ENUM_AVAILABLE_MAC(10_9)
                                                        =	   0x1000000LL,
        kCFURLVolumeIsBeingUnmounted CF_ENUM_AVAILABLE_MAC(10_9)
                                                        =	   0x2000000LL,
    
// IMPORTANT: The values of the following flags must stay in sync with the
// VolumeCapabilities flags in CarbonCore (FileIDTreeStorage.h)
	kCFURLVolumeSupportsPersistentIDs		=        0x100000000LL,
	kCFURLVolumeSupportsSearchFS			=        0x200000000LL,
	kCFURLVolumeSupportsExchange			=        0x400000000LL,
	// reserved						 0x800000000LL,
	kCFURLVolumeSupportsSymbolicLinks		=       0x1000000000LL,
	kCFURLVolumeSupportsDenyModes			=       0x2000000000LL,
	kCFURLVolumeSupportsCopyFile			=       0x4000000000LL,
	kCFURLVolumeSupportsReadDirAttr			=       0x8000000000LL,
	kCFURLVolumeSupportsJournaling			=      0x10000000000LL,
	kCFURLVolumeSupportsRename			=      0x20000000000LL,
	kCFURLVolumeSupportsFastStatFS			=      0x40000000000LL,
	kCFURLVolumeSupportsCaseSensitiveNames		=      0x80000000000LL,
	kCFURLVolumeSupportsCasePreservedNames		=     0x100000000000LL,
	kCFURLVolumeSupportsFLock			=     0x200000000000LL,
	kCFURLVolumeHasNoRootDirectoryTimes		=     0x400000000000LL,
	kCFURLVolumeSupportsExtendedSecurity		=     0x800000000000LL,
	kCFURLVolumeSupports2TBFileSize			=    0x1000000000000LL,
	kCFURLVolumeSupportsHardLinks			=    0x2000000000000LL,
	kCFURLVolumeSupportsMandatoryByteRangeLocks	=    0x4000000000000LL,
	kCFURLVolumeSupportsPathFromID			=    0x8000000000000LL,
	// reserved					    0x10000000000000LL,
	kCFURLVolumeIsJournaling			=   0x20000000000000LL,
	kCFURLVolumeSupportsSparseFiles			=   0x40000000000000LL,
	kCFURLVolumeSupportsZeroRuns			=   0x80000000000000LL,
	kCFURLVolumeSupportsVolumeSizes			=  0x100000000000000LL,
	kCFURLVolumeSupportsRemoteEvents		=  0x200000000000000LL,
	kCFURLVolumeSupportsHiddenFiles			=  0x400000000000000LL,
	kCFURLVolumeSupportsDecmpFSCompression		=  0x800000000000000LL,
	kCFURLVolumeHas64BitObjectIDs			= 0x1000000000000000LL,
	kCFURLVolumePropertyFlagsAll			= 0xffffffffffffffffLL
};


/*
    _CFURLGetVolumePropertyFlags - Returns a bit array of volume properties.
    Only flags whose corresponding bits are set in the "mask" parameter are valid 
    in the output bit array. Returns true on success, false if an error occurs.
    Optional output error: the error is set to a valid CFErrorRef if and only if the function 
    returns false. A valid output error must be released by the caller.
 */
CF_EXPORT
Boolean _CFURLGetVolumePropertyFlags(CFURLRef url, CFURLVolumePropertyFlags mask, CFURLVolumePropertyFlags *flags, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);


/*  _CFURLCopyResourcePropertyForKeyFromCache works like CFURLCopyResourcePropertyForKey
    only it never causes I/O. If the property value requested is cached (or known
    to be not available) for the resource, return TRUE and the property value. The
    property value returned could be NULL meaning that property is not available
    for the resource. If the property value requested is not cached or the resource,
    FALSE is returned.

    Only for use by DesktopServices!
 */
CF_EXPORT
Boolean _CFURLCopyResourcePropertyForKeyFromCache(CFURLRef url, CFStringRef key, void *cfTypeRefValue) CF_AVAILABLE(10_8, NA);

/*  _CFURLCopyResourcePropertiesForKeysFromCache works like CFURLCopyResourcePropertiesForKeys
    only it never causes I/O. If the property values requested are cached (or known
    to be not available) for the resource, return a CFDictionary. Property values
    not available for the resource are not included in the CFDictionary.
    If the values requested are not cached, return NULL.

    Only for use by DesktopServices!
 */
CF_EXPORT
CFDictionaryRef _CFURLCopyResourcePropertiesForKeysFromCache(CFURLRef url, CFArrayRef keys) CF_AVAILABLE(10_8, NA);

/*  _CFURLCacheResourcePropertyForKey works like CFURLCopyResourcePropertyForKey
    only it does not return the property value -- it just ensures the value is cached.
    If no errors occur, TRUE is returned. If an error occurs, FALSE is returned
    and the optional output error is set to a valid CFErrorRef (which must be
    released by the caller.
 
    Only for use by DesktopServices!
 */
CF_EXPORT
Boolean _CFURLCacheResourcePropertyForKey(CFURLRef url, CFStringRef key, CFErrorRef *error) CF_AVAILABLE(10_8, NA);

/*  _CFURLCacheResourcePropertiesForKeys works like CFURLCopyResourcePropertiesForKeys
    only it does not return the property values -- it just ensures the values is cached.
    If no errors occur, TRUE is returned. If an error occurs, FALSE is returned
    and the optional output error is set to a valid CFErrorRef (which must be
    released by the caller.

    Only for use by DesktopServices!
 */
CF_EXPORT
Boolean _CFURLCacheResourcePropertiesForKeys(CFURLRef url, CFArrayRef keys, CFErrorRef *error) CF_AVAILABLE(10_8, NA);

/*
    _CFURLCreateDisplayPathComponentsArray()

    Summary:
	_FileURLCreateDisplayPathComponentsArray creates a CFArray of
	CFURLs for each component in the path leading up to the target
	URL. This routine is suitable for clients who wish to show the
	path leading up to a file system item. NOTE: This routine can be
	I/O intensive, so use it sparingly, and cache the results if
	possible.

    Discussion:
	The CFURLs in the result CFArray are ordered from the target URL
	to the root of the display path. For example, if the target URL
	is file://localhost/System/Library/ the CFURLs in the array will
	be ordered: file://localhost/System/Library/,
	file://localhost/System/, and then file://localhost/

    Parameters:
      
	targetURL:
	    The target URL.

	error:
	    A pointer to a CFErrorRef, or NULL. If error is non-NULL and
	    the function result is NULL, this will be filled in with a
	    CFErrorRef representing the error that occurred.

    Result:
	A CFArray or NULL if an error occurred.
 */
CF_EXPORT
CFArrayRef _CFURLCreateDisplayPathComponentsArray(CFURLRef url, CFErrorRef *error) CF_AVAILABLE(10_7, 4_0);

/* Returns true for URLs that locate file system resources. */
CF_EXPORT
Boolean _CFURLIsFileURL(CFURLRef url) CF_AVAILABLE(10_6, 4_0);

/* Deprecated and scheduled for removal in 10.10/8.0 - Use the public API CFURLIsFileReferenceURL() */
CF_EXPORT
Boolean _CFURLIsFileReferenceURL(CFURLRef url) CF_DEPRECATED(10_6, 10_9, 4_0, 7_0);

/* For use by Core Services */
CF_EXPORT 
void *__CFURLResourceInfoPtr(CFURLRef url) CF_AVAILABLE(10_6, 4_0);

CF_EXPORT 
void __CFURLSetResourceInfoPtr(CFURLRef url, void *ptr) CF_AVAILABLE(10_6, 4_0);


struct FSCatalogInfo;
struct HFSUniStr255;

/* _CFURLGetCatalogInfo is used by LaunchServices */
CF_EXPORT
SInt32 _CFURLGetCatalogInfo(CFURLRef url, UInt32 whichInfo, struct FSCatalogInfo *catalogInfo, struct HFSUniStr255 *name) CF_AVAILABLE(10_7, 5_0);

/* _CFURLReplaceObject SPI */

/* options for _CFURLReplaceObject */
enum {
//  _CFURLItemReplacementUsingOriginalMetadataOnly  = 1,    // not used
    _CFURLItemReplacementUsingNewMetadataOnly       = 2,
//  _CFURLItemReplacementByMergingMetadata          = 3,    // not used
    _CFURLItemReplacementWithoutDeletingBackupItem  = 1 << 4
};

/* _CFURLReplaceObject is the underlying implementation for -[NSFileManager replaceItemAtURL:withItemAtURL:backupItemName:options:resultingItemURL:error:] with one additional argument: newName. The optional newName argument can be used to rename the replacement (for example, when replacing "document.rtf" with "document.rtfd") while still preserving the document's metadata. If newName is used, there must be a file or directory at originalItemURL -- if originalItemURL does not exist and newName is not NULL, an error will be returned.
 */
CF_EXPORT 
Boolean _CFURLReplaceObject( CFAllocatorRef allocator, CFURLRef originalItemURL, CFURLRef newItemURL, CFStringRef newName, CFStringRef backupItemName, CFOptionFlags options, CFURLRef *resultingURL, CFErrorRef *error ) CF_AVAILABLE(10_7, 5_0);

CF_EXPORT
Boolean _CFURLIsProtectedDirectory(CFURLRef directoryURL) CF_AVAILABLE(10_10, NA);

/* _CFURLAttachSecurityScopeToFileURL attaches a sandbox extension to the file URL object. The URL object will then be security-scoped and will be usable with the NSURL's -startAccessingSecurityScopedResource method and CFURL's CFURLStartAccessingSecurityScopedResource() function. The URL object must be a file URL. If the URL object already has a sandbox extension attached, the new extension replaces the previous sandbox extension. If NULL is passed for the sandboxExtension, the sandbox extension (if any) is removed from the URL object. Callers would be responsible for ensuring the sandbox extension matches the URL's file system path.
 */
CF_EXPORT
void _CFURLAttachSecurityScopeToFileURL(CFURLRef url, CFDataRef sandboxExtension) CF_AVAILABLE(10_10, 8_0);

/* _CFURLCopySecurityScopeFromFileURL copies the sandbox extension attached to the file URL object. If the URL is not a file URL or doesn't have a sandbox extension, NULL will be returned.
 */
CF_EXPORT
CFDataRef _CFURLCopySecurityScopeFromFileURL(CFURLRef url) CF_AVAILABLE(10_10, 8_0);

CF_EXPORT
void _CFURLSetPermanentResourcePropertyForKey(CFURLRef url, CFStringRef key, CFTypeRef propertyValue) CF_AVAILABLE(10_10, 8_0);

CF_EXPORT
CFURLEnumeratorResult _CFURLEnumeratorGetURLsBulk(CFURLEnumeratorRef enumerator, CFIndex maximumURLs, CFIndex *actualURLs, CFURLRef *urls, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);

// Returns a string describing the bookmark data. For debugging purposes only.
CF_EXPORT
CFStringRef _CFURLBookmarkCopyDescription(CFDataRef bookmarkRef) CF_AVAILABLE(10_10, 8_0);

// private CFURLBookmarkCreationOptions
enum {
    kCFURLBookmarkCreationWithFileProvider CF_ENUM_AVAILABLE(10_10, 8_0) = ( 1UL << 26 ), // private option to create bookmarks with file provider string. The file provider string overrides the rest of the bookmark data at resolution time.
    kCFURLBookmarkOperatingInsideScopedBookmarksAgent = (1UL << 27), // private option used internally by ScopedBookmarkAgent to prevent recursion between the agent and the framework code. Available 10_7, NA
    kCFURLBookmarkCreationAllowCreationIfResourceDoesNotExistMask = ( 1UL << 28 ),    // allow creation of a bookmark to a file: scheme with a CFURLRef of item which may not exist.  If the filesystem item does not exist, the created bookmark contains essentially no properties beyond the url string. Available 10_7, 5_0.
    kCFURLBookmarkCreationDoNotIncludeSandboxExtensionsMask = ( 1UL << 29 ),  // If set, sandbox extensions are not included in created bookmarks. Ordinarily, bookmarks (except those created suitable for putting into a bookmark file) will have a sandbox extension added for the item. Available 10_7, NA.
    kCFURLBookmarkCreationSuitableForOdocAppleEvent = ( 1UL << 31 ),   // add properties we guarantee will be in an odoc AppleEvent. Available 10_10, NA (but supported back to 10.6).
};

// private CFURLBookmarkFileCreationOptions
enum {
    // FIXME: These three options (kCFBookmarkFileCreationWithoutOverwritingExistingFile, kCFBookmarkFileCreationWithoutAppendingAliasExtension, and kCFBookmarkFileCreationWithoutCreatingResourceFork) are not implemented and have never been used.
    kCFBookmarkFileCreationWithoutOverwritingExistingFile   = ( 1UL << 8 ), // if destination file already exists don't overwrite it and return an error
    kCFBookmarkFileCreationWithoutAppendingAliasExtension   = ( 1UL << 9 ), // don't add / change whatever extension is on the created alias file
    kCFBookmarkFileCreationWithoutCreatingResourceFork      = ( 1UL << 10 ), // don't create the resource-fork half of the alias file
};

// private CFURLBookmarkResolutionOptions
enum {
    kCFBookmarkResolutionPerformRelativeResolutionFirstMask CF_ENUM_AVAILABLE(10_8, 6_0) = ( 1UL << 11 ), // perform relative resolution before absolute resolution. If this bit is set, for this to be useful a relative URL must also have been passed in and the bookmark when created must have been created relative to another url.
};

typedef CF_ENUM(CFIndex, CFURLBookmarkMatchResult) {
    kCFURLBookmarkComparisonUnableToCompare = 0x00000000,   /* the two bookmarks could not be compared for some reason */
    kCFURLBookmarkComparisonNoMatch         = 0x00001000,   /* Bookmarks do not refer to the same item */
    kCFURLBookmarkComparisonUnlikelyToMatch = 0x00002000,   /* it is unlikely that the two items refer to the same filesystem item */
    kCFURLBookmarkComparisonLikelyToMatch   = 0x00004000,   /* it is likely that the two items refer to the same filesystem item ( but, they may not ) */
    kCFURLBookmarkComparisonMatch           = 0x00008000,   /* the two items refer to the same item, but other information in the bookmarks may not match */
    kCFURLBookmarkComparisonExactMatch      = 0x0000f000    /* the two bookmarks are identical */
}; // Available 10_7, NA.

/* The relativeToURL and matchingPropertyKeys parameters are not used and are ignored */
CF_EXPORT
CFURLBookmarkMatchResult _CFURLBookmarkDataCompare(CFDataRef bookmark1Ref, CFDataRef bookmark2Ref, CFURLRef relativeToURL, CFArrayRef* matchingPropertyKeys) CF_AVAILABLE(10_7, NA);

CF_EXPORT
OSStatus _CFURLBookmarkDataToAliasHandle(CFDataRef bookmarkRef, void* aliasHandleP) CF_AVAILABLE(10_7, NA);

CF_EXPORT
CFURLRef _CFURLCreateByResolvingAliasFile(CFAllocatorRef allocator, CFURLRef url, CFURLBookmarkResolutionOptions options, CFArrayRef propertiesToInclude, CFErrorRef *error ) CF_AVAILABLE(10_10, 8_0);

/*
 The following are properties that can be asked of bookmark data objects in addition to the resource properties
 from CFURL itself.
 */

extern const CFStringRef kCFURLBookmarkOriginalPathKey CF_AVAILABLE(10_7, 5_0);
extern const CFStringRef kCFURLBookmarkOriginalRelativePathKey CF_AVAILABLE(10_7, 5_0);
extern const CFStringRef kCFURLBookmarkOriginalRelativePathComponentsArrayKey CF_AVAILABLE(10_7, 5_0);
extern const CFStringRef kCFURLBookmarkOriginalVolumeNameKey CF_AVAILABLE(10_7, 5_0);
extern const CFStringRef kCFURLBookmarkOriginalVolumeCreationDateKey CF_AVAILABLE(10_7, 5_0);
extern const CFStringRef kCFURLBookmarkFileProviderStringKey CF_AVAILABLE(10_10, 8_0);

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFURLPRIV__ */

