/*	CFURLPriv.h
	Copyright (c) 2008-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
#include <CoreFoundation/CFDate.h>
#if TARGET_OS_MAC
#include <sys/mount.h>
#endif

CF_EXTERN_C_BEGIN

/* Like CFURLGetBytes(), but allows the output encoding to be specified. */
CF_EXPORT
CFIndex CFURLGetBytesUsingEncoding(CFURLRef url, UInt8 *buffer, CFIndex bufferLength, CFStringEncoding encoding);

#pragma mark - FileURL

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
} API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));


/*
    Private File System Property Keys
*/
CF_EXPORT const CFStringRef _kCFURLPathKey API_DEPRECATED("Use the kCFURLPathKey or NSURLPathKey public property keys instead", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLPathKey or NSURLPathKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIDKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Volume ID (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLInodeNumberKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* 64-bit inode number (the inode number from the file system) (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLFileIDKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* 64-bit file ID (for tracking a file by ID. This may or may not be the inode number) (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLParentDirectoryIDKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* 64-bit file ID (for tracking a parent directory by ID. This may or may not be the inode number) (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLDistinctLocalizedNameKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* The localized name, if it is distinct from the real name. Otherwise, NULL (CFString) */

CF_EXPORT const CFStringRef _kCFURLNameExtensionKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* The name extension (CFString) */

CF_EXPORT const CFStringRef _kCFURLFinderInfoKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* A 16-byte Finder Info structure immediately followed by a 16-byte Extended Finder Info structure (CFData) */

CF_EXPORT const CFStringRef _kCFURLHFSTypeCodeKey API_AVAILABLE(macos(12.0), ios(15.0), watchos(8.0), tvos(15.0));
    /* A legacy 4-character code which identifies the file type. (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLIsUserNoDumpKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* True if resource's UF_NODUMP flag is set (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsUserAppendKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* True if resource's UF_APPEND flag is set (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsUserOpaqueKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* True if resource's UF_OPAQUE flag is set (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsCompressedKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* True if resource's data is transparently compressed by the system on its storage device (UF_COMPRESSED flag is set) (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsUserTrackedKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* True if resource's UF_TRACKED flag is set (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsUserDataVaultKey API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));
    /* True if resource's UF_DATAVAULT flag is set (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsSystemArchivedKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* True if resource's SF_ARCHIVED flag is set (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsSystemAppendKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* True if resource's SF_APPEND flag is set (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsRestrictedKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* True if resource is restricted (SF_RESTRICTED flag is set) (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsSystemNoUnlinkKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* True if resource's SF_NOUNLINK flag is set (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsSystemFirmlinkKey API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
    /* True if resource's SF_FIRMLINK flag is set (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsSystemDatalessFaultKey API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
    /* True if resource's SF_DATALESS flag is set (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLFileFlagsKey API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
    /* file flags from ATTR_CMN_FLAGS (same as stat(2)'s st_flags). (Read-only, UInt32 CFNumber) */

CF_EXPORT const CFStringRef _kCFURLGenerationCountKey API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
    /* the generation count from ATTR_CMN_GEN_COUNT. (Read-only, UInt32 CFNumber) */

CF_EXPORT const CFStringRef _kCFURLIsApplicationKey API_DEPRECATED("Use kCFURLIsApplicationKey (API) instead", macos(10.6,10.11), ios(4.0,9.0), watchos(2.0,2.0), tvos(9.0,9.0));
/* Deprecated and scheduled for removal in 10.12/10.0 - Use the kCFURLIsApplicationKey or NSURLIsApplicationKey public property keys */

CF_EXPORT const CFStringRef _kCFURLApplicationIsAppletKey API_AVAILABLE(macos(10.11)) API_UNAVAILABLE(ios, watchos, tvos);
/* The item is an OSA or Automator applet. Only applies to applications. (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLApplicationIsPlaceholderKey API_AVAILABLE(macos(11.2), ios(14.2), watchos(7.2), tvos(14.2));
/* The item is a placeholder while an app installs or is an uninstalled iOS system app. Only applies to applications. (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLApplicationIsBetaKey API_AVAILABLE(macos(12.0), ios(15.0), watchos(8.0), tvos(15.0));
/* The item is a TestFlight beta app. (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLApplicationHasSupportedFormatKey API_AVAILABLE(macos(10.11)) API_UNAVAILABLE(ios, watchos, tvos);
/* The item is an application that can be executed on the current system. (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCanSetHiddenExtensionKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* True if the filename extension can be hidden or unhidden (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLIsReadableKey API_DEPRECATED("Use the kCFURLIsReadableKey or NSURLIsReadableKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLIsReadableKey or NSURLIsReadableKey public property keys */
/* never implemented and scheduled for removal in 10.10/8.0 */CF_EXPORT const CFStringRef _kCFURLUserCanReadKey API_DEPRECATED("Not supported", macos(10.0,10.6), ios(2.0,4.0), watchos(2.0,2.0), tvos(9.0,9.0));

CF_EXPORT const CFStringRef _kCFURLIsWriteableKey API_DEPRECATED("Use the kCFURLIsWritableKey or NSURLIsWritableKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLIsWritableKey or NSURLIsWritableKey public property keys */
/* never implemented and scheduled for removal in 10.10/8.0 */CF_EXPORT const CFStringRef _kCFURLUserCanWriteKey API_DEPRECATED("Not supported", macos(10.0,10.6), ios(2.0,4.0), watchos(2.0,2.0), tvos(9.0,9.0));

CF_EXPORT const CFStringRef _kCFURLIsExecutableKey API_DEPRECATED("Use the kCFURLIsExecutableKey or NSURLIsExecutableKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLIsExecutableKey or NSURLIsExecutableKey public property keys */
/* never implemented and scheduled for removal in 10.10/8.0 */CF_EXPORT const CFStringRef _kCFURLUserCanExecuteKey API_DEPRECATED("Not supported", macos(10.0,10.6), ios(2.0,4.0), watchos(2.0,2.0), tvos(9.0,9.0));

CF_EXPORT const CFStringRef _kCFURLParentDirectoryIsVolumeRootKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* True if the parent directory is the root of a volume (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLFileSecurityKey API_DEPRECATED("Use the kCFURLFileSecurityKey or NSURLFileSecurityKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLFileSecurityKey or NSURLFileSecurityKey public property keys */

CF_EXPORT const CFStringRef _kCFURLFileSizeOfResourceForkKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Size in bytes of the resource fork (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLFileAllocatedSizeOfResourceForkKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Size in bytes of the blocks allocated for the resource fork (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLEffectiveIconImageDataKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Icon image data, i.e. raw pixel data (CFData) */

CF_EXPORT const CFStringRef _kCFURLTypeBindingKey API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
    /* Type binding for icon (Read-only, value type CFData) */

CF_EXPORT const CFStringRef _kCFURLCustomIconImageDataKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Icon image data of the item's custom icon, if any (CFData) */

CF_EXPORT const CFStringRef _kCFURLEffectiveIconFlattenedReferenceDataKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Icon flattened reference, suitable for cheaply sharing the effective icon reference across processess (CFData) */

CF_EXPORT const CFStringRef _kCFURLBundleIdentifierKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* If resource is a bundle, the bundle identifier (CFString) */

CF_EXPORT const CFStringRef _kCFURLVersionKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* If resource is a bundle, the bundle version (CFBundleVersion) as a string (CFString) */

CF_EXPORT const CFStringRef _kCFURLShortVersionStringKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* If resource is a bundle, the bundle short version (CFBundleShortVersionString) as a string (CFString) */

CF_EXPORT const CFStringRef _kCFURLOwnerIDKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* 32-bit owner ID (uid_t). (CFNumber) Note: Almost all clients should use the kCFURLFileSecurityKey or NSURLFileSecurityKey public property keys and CFFileSecurityGetOwner() instead of this. */

CF_EXPORT const CFStringRef _kCFURLGroupIDKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* 32-bit group ID (gid_t) (CFNumber) Note: Almost all clients should use the kCFURLFileSecurityKey or NSURLFileSecurityKey public property keys and CFFileSecurityGetGroup() instead of this. */

CF_EXPORT const CFStringRef _kCFURLStatModeKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* 32-bit group ID (mode_t) (CFNumber) Note: Almost all clients should use the kCFURLFileSecurityKey or NSURLFileSecurityKey public property keys and CFFileSecurityGetMode() instead of this. */

/* To determine which dictionary to request from _kCFURLLocalizedNameDictionaryKey or _kCFURLLocalizedNameWithExtensionsHiddenDictionaryKey, you can consult _LSGetShowAllExtensionsPreference() on macOS. On iOS, extensions are always hidden. */

CF_EXPORT const CFStringRef _kCFURLLocalizedNameDictionaryKey API_AVAILABLE(macos(10.7), ios(9.0), watchos(2.0), tvos(9.0));
    /* For items with localized display names, the dictionary of all available localizations. The keys are the cannonical locale strings for the available localizations. (CFDictionary) */

CF_EXPORT const CFStringRef _kCFURLLocalizedNameWithExtensionsHiddenDictionaryKey API_AVAILABLE(macosx(10.13), ios(11.0), watchos(4.0), tvos(11.0));
    /* For items with localized display names, the dictionary of all available localizations with extensions hidden if safe. The keys are the cannonical locale strings for the available localizations. (CFDictionary) */

CF_EXPORT const CFStringRef _kCFURLLocalizedTypeDescriptionDictionaryKey API_AVAILABLE(macos(10.7), ios(9.0), watchos(2.0), tvos(9.0));
    /* The dictionary of all available localizations of the item kind string. The keys are the cannonical locale strings for the available localizations. (CFDictionary) */

CF_EXPORT const CFStringRef _kCFURLApplicationCategoriesKey API_AVAILABLE(macos(10.7)) API_UNAVAILABLE(ios, watchos, tvos);
    /* The array of category UTI strings associated with the url. (CFArray) */

CF_EXPORT const CFStringRef _kCFURLApplicationHighResolutionModeIsMagnifiedKey API_AVAILABLE(macos(10.7)) API_UNAVAILABLE(ios, watchos, tvos);
    /* True if the app runs with magnified 1x graphics on a 2x display (Per-user, CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCanSetApplicationHighResolutionModeIsMagnifiedKey API_AVAILABLE(macos(10.7)) API_UNAVAILABLE(ios, watchos, tvos);
    /* True if the app can run in either magnified or native resolution modes (Read only, CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLWriterBundleIdentifierKey API_AVAILABLE(macos(10.8)) API_UNAVAILABLE(ios, watchos, tvos);
    /* The bundle identifier of the process writing to this object (Read-write, value type CFString) */

CF_EXPORT const CFStringRef _kCFURLApplicationNapIsDisabledKey API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos);
    /* True if app nap is disabled (Applications only, Per-user, CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCanSetApplicationNapIsDisabledKey API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos);
    /* True if the ApplicationNapIsDisabled property value can be changed (Applications only, Read only, CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCanSetStrongBindingKey API_AVAILABLE(macos(10.11)) API_UNAVAILABLE(ios, watchos, tvos);
    /* True if the strong binding can be changed (Read only, CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLStrongBindingKey API_AVAILABLE(macos(10.11)) API_UNAVAILABLE(ios, watchos, tvos);
    /* The application to which the file is strongly bound (Read-write, value type CFURL) */

CF_EXPORT const CFStringRef _kCFURLArchitecturesValidOnCurrentSystemKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* Array of CFStringRefs, each element an architecture identifier. The array includes the list of executable architectures found in the application bundle's executable that can be executed on the current system. (Read-only, value type CFArray of CFStrings) */

CF_EXPORT
const CFStringRef _kCFURLApplicationArchitecturesKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* The complete list of executable architectures found in the application bundle's executable (Read-only, value type CFArray of CFString) */

CF_EXPORT
const CFStringRef _kCFURLApplicationSupportedRegionsKey API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));
    /* The complete list of regions supported by the application as found in the application bundle’s Info.plist (Read-only, value type CFArray of CFString) */

CF_EXPORT const CFStringRef _kCFURLFaultLogicalFileIsHiddenKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* True if the fault logical file is hidden. (Read only, CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLLocalizedNameComponentsKey API_AVAILABLE(macosx(10.13), ios(11.0), watchos(4.0), tvos(11.0));
    /* An array containing the base name of the file and (if present) the extension to be used for display. Does not include extra Unicode visual ordering characters added by the system. For Finder use. (Read-only, value type CFArray of CFStrings) */

CF_EXPORT const CFStringRef _kCFURLApplicationPrefersExternalGPUKey API_AVAILABLE(macos(10.14)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Default is false. True means the application will be steered towards the eGPU regardless of which displays it is attached to. (Read-write, CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCanSetApplicationPrefersExternalGPUKey API_AVAILABLE(macos(10.14)) API_UNAVAILABLE(ios, watchos, tvos);
    /* False if app’s Info.plist specifies a eGPU policy, True if app does not specify an policy. Finder does not show a checkbox when this value is false. (Read-only, CFBoolean) */

#if !RC_HIDE_J316
CF_EXPORT const CFStringRef _kCFURLApplicationPrefersSafeApertureSystemFullScreenCompatibilityKey API_AVAILABLE(macos(12.0)) API_UNAVAILABLE(ios, watchos, tvos);
CF_EXPORT const CFStringRef _kCFURLApplicationPrefersSafeApertureAppFullScreenCompatibilityKey API_AVAILABLE(macos(12.0)) API_UNAVAILABLE(ios, watchos, tvos);
CF_EXPORT const CFStringRef _kCFURLApplicationPrefersSafeApertureWindowedCompatibilityKey API_AVAILABLE(macos(12.0)) API_UNAVAILABLE(ios, watchos, tvos);
CF_EXPORT const CFStringRef _kCFURLCanSetApplicationPrefersSafeApertureWindowedCompatibilityKey API_AVAILABLE(macos(12.0)) API_UNAVAILABLE(ios, watchos, tvos);
#endif // RC_HIDE_J316

CF_EXPORT const CFStringRef _kCFURLApplicationDeviceManagementPolicyKey API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
    /* For app bundle URLs, value is the Device Management framework's policy for the application. If the value is unavailable, returns DMFPolicyOK. For non-app URLs, value is nil. The calling process must be properly entitled with the Device Management framework to use this property. (Read-only, value type CFNumber) */

CF_EXPORT const CFStringRef _kCFURLIsExcludedFromCloudBackupKey API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));
    /* true if resource should be excluded from iCloud backups, false otherwise (Read-write, value type CFBoolean). */

CF_EXPORT const CFStringRef _kCFURLIsExcludedFromUnencryptedBackupKey API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));
    /* true if resource should be excluded from unencrypted backups, false otherwise (Read-write, value type CFBoolean). */

CF_EXPORT const CFStringRef _kCFURLDeviceRefNumKey API_AVAILABLE(macos(10.15)) API_UNAVAILABLE(ios, watchos, tvos);
    /* an unique per-volume non-persistent identifier for volumes (much like _kCFURLVolumeRefNumKey) that is also unique per-device when the volume is really two devices (i.e. ROSP) (64-bit integer CFNumber). */

CF_EXPORT const CFStringRef _kCFURLContentTypeKey API_AVAILABLE(macos(10.16), ios(14.0), watchos(7.0), tvos(14.0));
    /* the file type of the resource (CFTypeRef/UTType *). */

/* Additional volume properties */

CF_EXPORT const CFStringRef _kCFURLVolumeRefNumKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* The Carbon File Manager's FSVolumeRefNum for the resource volume (CFNumber) */

CF_EXPORT const CFStringRef _kCFURLVolumeUUIDStringKey API_DEPRECATED("Use the kCFURLVolumeUUIDStringKey or NSURLVolumeUUIDStringKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeUUIDStringKey or NSURLVolumeUUIDStringKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeCreationDateKey API_DEPRECATED("Use the kCFURLVolumeCreationDateKey or NSURLVolumeCreationDateKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeCreationDateKey or NSURLVolumeCreationDateKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIsLocalKey API_DEPRECATED("Use the kCFURLVolumeIsLocalKey or NSURLVolumeIsLocalKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsLocalKey or NSURLVolumeIsLocalKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIsAutomountKey API_DEPRECATED("Use the kCFURLVolumeIsAutomountedKey or NSURLVolumeIsAutomountedKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsAutomountedKey or NSURLVolumeIsAutomountedKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeDontBrowseKey API_DEPRECATED("Use the kCFURLVolumeIsBrowsableKey or NSURLVolumeIsBrowsableKey public property keys (Note: value is inverse of _kCFURLVolumeDontBrowseKey)", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsBrowsableKey or NSURLVolumeIsBrowsableKey public property keys (Note: value is inverse of _kCFURLVolumeDontBrowseKey) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsReadOnlyKey API_DEPRECATED("Use the kCFURLVolumeIsReadOnlyKey or NSURLVolumeIsReadOnlyKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsReadOnlyKey or NSURLVolumeIsReadOnlyKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIsQuarantinedKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Mounted quarantined (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsEjectableKey API_DEPRECATED("Use the kCFURLVolumeIsEjectableKey or NSURLVolumeIsEjectableKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsEjectableKey or NSURLVolumeIsEjectableKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIsRemovableKey API_DEPRECATED("Use the kCFURLVolumeIsRemovableKey or NSURLVolumeIsRemovableKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsRemovableKey or NSURLVolumeIsRemovableKey public property keys */

CF_EXPORT const CFStringRef _kCFURLVolumeIsInternalKey API_DEPRECATED("Use the kCFURLVolumeIsInternalKey or NSURLVolumeIsInternalKey public property keys (Note: this has slightly different behavior than the public VolumeIsInternal key)", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsInternalKey or NSURLVolumeIsInternalKey public property keys (Note: this has slightly different behavior than the public VolumeIsInternal key) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsExternalKey API_DEPRECATED("Use the kCFURLVolumeIsInternalKey or NSURLVolumeIsInternalKey public property keys (Note: this has slightly different behavior than the public VolumeIsInternal key)", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeIsInternalKey or NSURLVolumeIsInternalKey public property keys (Note: this has slightly different behavior than the public VolumeIsInternal key) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsDiskImageKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Volume is a mounted disk image (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLDiskImageBackingURLKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* If volume is a mounted disk image, the URL of the backing disk image (CFURL) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsFileVaultKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Volume uses File Vault encryption (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeSupportsFileProtectionKey API_DEPRECATED("Use the kCFURLVolumeSupportsFileProtectionKey or NSURLVolumeSupportsFileProtectionKey public property keys", macosx(10.16, 10.16), ios(14.0, 14.0), watchos(7.0, 7.0), tvos(14.0, 14.0));
    /* true if the volume supports data protection for files. (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsiDiskKey API_DEPRECATED("No supported", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - there are no more iDisks */

CF_EXPORT const CFStringRef _kCFURLVolumeiDiskUserNameKey API_DEPRECATED("Not supported", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - there are no more iDisks */

CF_EXPORT const CFStringRef _kCFURLVolumeIsLocaliDiskMirrorKey API_DEPRECATED("Not supported", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - there are no more iDisks */

CF_EXPORT const CFStringRef _kCFURLVolumeIsiPodKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Volume is on an iPod (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsCDKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Volume is a CD (audio or CD-ROM). (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsDVDKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Volume is a DVD (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsDeviceFileSystemKey API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0));
    /* Volume is devfs (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsHFSStandardKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /* Volume is HFS standard (which includes AFP volumes). Directory IDs, but not file IDs, can be looked up. (CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIOMediaIconFamilyNameKey API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Volume's IOMediaIconFamilyName. (CFStringRef) */

CF_EXPORT const CFStringRef _kCFURLVolumeIOMediaIconBundleIdentifierKey API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Volume's IOMediaIconBundleIdentifier. (CFStringRef) */

CF_EXPORT const CFStringRef _kCFURLVolumeQuarantinePropertiesKey API_AVAILABLE(macos(10.10)) API_UNAVAILABLE(ios, watchos, tvos);
    /* The quarantine properties for the volume on which the resource resides as defined in LSQuarantine.h.=To remove quarantine information from a volume, pass kCFNull as the value when setting this property. (Read-write, value type CFDictionary) */

CF_EXPORT const CFStringRef _kCFURLVolumeOpenFolderURLKey API_AVAILABLE(macos(10.10)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Returns a URL to the folder the Finder should open when a HFS volume is mounted, or NULL if there is none. (Read-only, value type CFURL) */

CF_EXPORT const CFStringRef _kCFURLResolvedFromBookmarkDataKey API_DEPRECATED("Not supported", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal later in 10.9/7.0 since it is unused (*/

CF_EXPORT const CFStringRef _kCFURLVolumeMountPointStringKey API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
    /*	the volume mountpoint string (Read-only, value type CFString) */

CF_EXPORT const CFStringRef _kCFURLVolumeDeviceIDKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
    /* the volume's dev_t (Read-only, value type CFNumber) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsTimeMachineKey API_AVAILABLE(macos(10.11)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Volume is the Time Machine volume (Read-write, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsAirportKey API_AVAILABLE(macos(10.11)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Volume is an airport volume (Read-write, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsVideoDiskKey API_AVAILABLE(macos(10.13)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Volume is video disk (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsDVDVideoKey API_AVAILABLE(macos(10.13)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Volume is DVD video (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsBDVideoKey API_AVAILABLE(macos(10.13)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Volume is BD video (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsMobileTimeMachineKey API_AVAILABLE(macos(10.13)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Volume is mobile time machine (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLVolumeIsNetworkOpticalKey API_AVAILABLE(macos(10.13)) API_UNAVAILABLE(ios, watchos, tvos);
    /* Volume is network optical (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCompleteMountURLKey API_DEPRECATED("Use the kCFURLVolumeURLForRemountingKey or NSURLVolumeURLForRemountingKey public property keys", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));
    /* Deprecated and scheduled for removal in 10.10/8.0 - Use the kCFURLVolumeURLForRemountingKey or NSURLVolumeURLForRemountingKey public property keys */

CF_EXPORT const CFStringRef _kCFURLUbiquitousItemDownloadRequestedKey API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));
    /* Is this Ubiquity item scheduled for download? (this is also true for items that are already downloaded). Use startDownloadingUbiquitousItemAtURL:error: to make this true (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef _kCFURLCloudDocsPlaceholderDictionaryKey API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
    /* Returns the placeholder dictionary for a side-fault file (Read-only, value type CFDictionary) */

CF_EXPORT const CFStringRef _kCFURLCloudDocsPlaceholderLogicalNameKey API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
    /* Returns the placeholder dictionary for a side-fault file (Read-only, value type CFString) */

// Temporary holding place for future API.

CF_EXPORT const CFStringRef kCFURLUbiquitousItemDownloadRequestedKey API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));
/* Is this Ubiquity item scheduled for download? (this is also true for items that are already downloaded). Use startDownloadingUbiquitousItemAtURL:error: to make this true (Read-only, value type CFBoolean) */

CF_EXPORT const CFStringRef kCFURLUbiquitousItemContainerDisplayNameKey API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
    /* Returns the localized name of the ubiquity container that contains this item (Read-only, value type CFString) */

CF_EXPORT const CFStringRef kCFURLUbiquitousItemIsSharedKey; // true if the ubiquitous item is shared. (Read-only, value type boolean NSNumber)

CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemCurrentUserRoleKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0)); // Replaced by kCFURLUbiquitousSharedItemCurrentUserRoleKey.
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemRoleOwner API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0)); // the current user is the owner of this shared item.
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemRoleParticipant API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0)); // the current user is a participant of this shared item.

CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemOwnerNameComponentsKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0)); // returns a NSPersonNameComponents, or nil if the current user. (Read-only, value type NSPersonNameComponents)
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemMostRecentEditorNameComponentsKey API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0)); // returns a NSPersonNameComponents for the most recent editro fo the file, or nil if the current user. (Read-only, value type NSPersonNameComponents)

CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemCurrentUserPermissionsKey API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0)); // returns the permissions for a participant of this shared item, or nil if not shared. (Read-only, value type NSString). Possible values below.
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemPermissionsReadOnly API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0)); // participants are only allowed to read this item
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemPermissionsReadWrite API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0)); // participants are allowed to both read and write this item

// Deprecated. Will be removed.
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemRoleKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0)); // Replaced by kCFURLUbiquitousSharedItemCurrentUserRoleKey.
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemOwnerNameKey API_DEPRECATED("Replaced by kCFURLUbiquitousSharedItemOwnerNameComponentsKey", macos(10.11,10.11), ios(9.0,9.0), watchos(2.0,2.0), tvos(9.0,9.0));
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemPermissionsKey API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0)); // returns the permissions for a participant of this shared item, or nil if not shared. (Read-only, value type NSString). Possible values below.
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemReadOnlyPermissions API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFStringRef kCFURLUbiquitousSharedItemReadWritePermissions API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));


// these keys are defined here, not in CFURL.h, because they return NSImage values which can only be used by Foundation
CF_EXPORT const CFStringRef kCFURLThumbnailDictionaryKey API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
CF_EXPORT const CFStringRef kCFURLThumbnailKey API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
// The values of thumbnails in the dictionary returned by NSURLThumbnailDictionaryKey
CF_EXPORT const CFStringRef kCFThumbnail1024x1024SizeKey API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));

// This private key is only for the use of CFURLPromises and the URL cache code in CoreServicesInternal
CF_EXPORT const CFStringRef _kCFURLPromisePhysicalURLKey API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));


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
    kCFURLResourceIsSystemCompressed API_DEPRECATED("Use kCFURLResourceIsCompressed instead", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0))
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
Boolean _CFURLGetResourcePropertyFlags(CFURLRef url, CFURLResourcePropertyFlags mask, CFURLResourcePropertyFlags *flags, CFErrorRef *error) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));

#if TARGET_OS_MAC || TARGET_OS_IPHONE
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
Boolean _CFURLCopyResourcePropertyValuesAndFlags( CFURLRef url, CFURLFilePropertyBitmap requestProperties, CFURLFilePropertyBitmap *actualProperties, struct _CFURLFilePropertyValues *properties, CFURLResourcePropertyFlags propertyFlagsMask, CFURLResourcePropertyFlags *propertyFlags, CFErrorRef *error) API_AVAILABLE(macos(10.7), ios(4.0), watchos(2.0), tvos(9.0));
#endif

/*
    Volume property flags
 */
typedef CF_OPTIONS(unsigned long long, CFURLVolumePropertyFlags) {
    kCFURLVolumeIsLocal                                 =                0x1LL,	// Local device (vs. network device)
    kCFURLVolumeIsAutomount				=                0x2LL,	// Mounted by the automounter
    kCFURLVolumeDontBrowse				=                0x4LL,	// Hidden from user browsing
    kCFURLVolumeIsReadOnly				=                0x8LL,	// Mounted read-only
    kCFURLVolumeIsQuarantined                           =               0x10LL,	// Mounted with quarantine bit
    kCFURLVolumeIsEjectable				=               0x20LL,
    kCFURLVolumeIsRemovable				=               0x40LL,
    kCFURLVolumeIsInternal				=               0x80LL,
    kCFURLVolumeIsExternal				=              0x100LL,
    kCFURLVolumeIsDiskImage				=              0x200LL,
    kCFURLVolumeIsFileVault				=              0x400LL,
    kCFURLVolumeIsLocaliDiskMirror API_DEPRECATED("iDisk no longer supported", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0))
                                                        =              0x800LL, // Deprecated and scheduled for removal in 10.10/8.0 - there are no more iDisks
    kCFURLVolumeIsiPod                                  =             0x1000LL,
    kCFURLVolumeIsiDisk API_DEPRECATED("iDisk no longer supported", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0))
                                                        =             0x2000LL, // Deprecated and scheduled for removal in 10.10/8.0 - there are no more iDisks
    kCFURLVolumeIsCD                                    =             0x4000LL,
    kCFURLVolumeIsDVD                                   =             0x8000LL,
    kCFURLVolumeIsDeviceFileSystem			=	     0x10000LL,
    kCFURLVolumeIsTimeMachine API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos) 
                                                        =	     0x20000LL,
    kCFURLVolumeIsAirport API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos) 
                                                        =	     0x40000LL,
    kCFURLVolumeIsVideoDisk API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos) 
                                                        =	     0x80000LL,
    kCFURLVolumeIsDVDVideo API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos) 
                                                        =	    0x100000LL,
    kCFURLVolumeIsBDVideo API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos) 
                                                        =	    0x200000LL,
    kCFURLVolumeIsMobileTimeMachine API_AVAILABLE(macos(10.9))  API_UNAVAILABLE(ios, watchos, tvos) 
                                                        =	    0x400000LL,
    kCFURLVolumeIsNetworkOptical API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos) 
                                                        =	    0x800000LL,
    kCFURLVolumeIsBeingRepaired API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos) 
                                                        =	   0x1000000LL,
    kCFURLVolumeIsBeingUnmounted API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, watchos, tvos) 
                                                        =	   0x2000000LL,
    kCFURLVolumeIsRootFileSystem API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0))
                                                        =	   0x4000000LL,
    kCFURLVolumeIsEncrypted API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0))
                                                        =	   0x8000000LL,
    kCFURLVolumeSupportsFileProtection API_AVAILABLE(macos(10.16), ios(14.0), watchos(7.0), tvos(14.0))
                                                        =	  0x10000000LL,

    // IMPORTANT: The values of the following flags must stay in sync with the
    // VolumeCapabilities flags in CarbonCore (FileIDTreeStorage.h)
    kCFURLVolumeSupportsPersistentIDs                   =        0x100000000LL,
    kCFURLVolumeSupportsSearchFS			=        0x200000000LL,
    kCFURLVolumeSupportsExchange			=        0x400000000LL,
    // reserved                                                  0x800000000LL,
    kCFURLVolumeSupportsSymbolicLinks                   =       0x1000000000LL,
    kCFURLVolumeSupportsDenyModes			=       0x2000000000LL,
    kCFURLVolumeSupportsCopyFile			=       0x4000000000LL,
    kCFURLVolumeSupportsReadDirAttr			=       0x8000000000LL,
    kCFURLVolumeSupportsJournaling			=      0x10000000000LL,
    kCFURLVolumeSupportsRename                          =      0x20000000000LL,
    kCFURLVolumeSupportsFastStatFS			=      0x40000000000LL,
    kCFURLVolumeSupportsCaseSensitiveNames		=      0x80000000000LL,
    kCFURLVolumeSupportsCasePreservedNames		=     0x100000000000LL,
    kCFURLVolumeSupportsFLock                           =     0x200000000000LL,
    kCFURLVolumeHasNoRootDirectoryTimes                 =     0x400000000000LL,
    kCFURLVolumeSupportsExtendedSecurity		=     0x800000000000LL,
    kCFURLVolumeSupports2TBFileSize			=    0x1000000000000LL,
    kCFURLVolumeSupportsHardLinks			=    0x2000000000000LL,
    kCFURLVolumeSupportsMandatoryByteRangeLocks         =    0x4000000000000LL,
    kCFURLVolumeSupportsPathFromID			=    0x8000000000000LL,
    // reserved                                             0x10000000000000LL,
    kCFURLVolumeIsJournaling                            =   0x20000000000000LL,
    kCFURLVolumeSupportsSparseFiles			=   0x40000000000000LL,
    kCFURLVolumeSupportsZeroRuns			=   0x80000000000000LL,
    kCFURLVolumeSupportsVolumeSizes			=  0x100000000000000LL,
    kCFURLVolumeSupportsRemoteEvents                    =  0x200000000000000LL,
    kCFURLVolumeSupportsHiddenFiles			=  0x400000000000000LL,
    kCFURLVolumeSupportsDecmpFSCompression		=  0x800000000000000LL,
    kCFURLVolumeHas64BitObjectIDs			= 0x1000000000000000LL,
    kCFURLVolumeSupportsFileCloning API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0))
                                                        = 0x2000000000000000LL,
    kCFURLVolumeSupportsSwapRenaming API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0))
                                                        = 0x4000000000000000LL,
    kCFURLVolumeSupportsExclusiveRenaming API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0))
                                                        = 0x8000000000000000LL,
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
Boolean _CFURLGetVolumePropertyFlags(CFURLRef url, CFURLVolumePropertyFlags mask, CFURLVolumePropertyFlags *flags, CFErrorRef *error) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));


/*  _CFURLCopyResourcePropertyForKeyFromCache works like CFURLCopyResourcePropertyForKey
    only it never causes I/O. If the property value requested is cached (or known
    to be not available) for the resource, return TRUE and the property value. The
    property value returned could be NULL meaning that property is not available
    for the resource. If the property value requested is not cached or the resource,
    FALSE is returned.

    Only for use by DesktopServices!
 */
CF_EXPORT
Boolean _CFURLCopyResourcePropertyForKeyFromCache(CFURLRef url, CFStringRef key, void *cfTypeRefValue) API_AVAILABLE(macos(10.8), ios(8.3), watchos(2.0), tvos(9.0));

/*  _CFURLCopyResourcePropertiesForKeysFromCache works like CFURLCopyResourcePropertiesForKeys
    only it never causes I/O. If the property values requested are cached (or known
    to be not available) for the resource, return a CFDictionary. Property values
    not available for the resource are not included in the CFDictionary.
    If the values requested are not cached, return NULL.

    Only for use by DesktopServices!
 */
CF_EXPORT
CFDictionaryRef _CFURLCopyResourcePropertiesForKeysFromCache(CFURLRef url, CFArrayRef keys) API_AVAILABLE(macos(10.8), ios(8.3), watchos(2.0), tvos(9.0));

/*  _CFURLCacheResourcePropertyForKey works like CFURLCopyResourcePropertyForKey
    only it does not return the property value -- it just ensures the value is cached.
    If no errors occur, TRUE is returned. If an error occurs, FALSE is returned
    and the optional output error is set to a valid CFErrorRef (which must be
    released by the caller.
 
    Only for use by DesktopServices!
 */
CF_EXPORT
Boolean _CFURLCacheResourcePropertyForKey(CFURLRef url, CFStringRef key, CFErrorRef *error) API_AVAILABLE(macos(10.8), ios(8.3), watchos(2.0), tvos(9.0));

/*  _CFURLCacheResourcePropertiesForKeys works like CFURLCopyResourcePropertiesForKeys
    only it does not return the property values -- it just ensures the values is cached.
    If no errors occur, TRUE is returned. If an error occurs, FALSE is returned
    and the optional output error is set to a valid CFErrorRef (which must be
    released by the caller.

    Only for use by DesktopServices!
 */
CF_EXPORT
Boolean _CFURLCacheResourcePropertiesForKeys(CFURLRef url, CFArrayRef keys, CFErrorRef *error) API_AVAILABLE(macos(10.8), ios(8.3), watchos(2.0), tvos(9.0));

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
CFArrayRef _CFURLCreateDisplayPathComponentsArray(CFURLRef url, CFErrorRef *error) API_AVAILABLE(macos(10.7), ios(4.0), watchos(2.0), tvos(9.0));

/* Returns true for URLs that locate file system resources. */
CF_EXPORT
Boolean _CFURLIsFileURL(CFURLRef url) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));

/* Deprecated and scheduled for removal in 10.10/8.0 - Use the public API CFURLIsFileReferenceURL() */
CF_EXPORT
Boolean _CFURLIsFileReferenceURL(CFURLRef url) API_DEPRECATED("Use CFURLIsFileReferenceURL() instead", macos(10.6,10.9), ios(4.0,7.0), watchos(2.0,2.0), tvos(9.0,9.0));

/* For use by Core Services */
CF_EXPORT 
void *__CFURLResourceInfoPtr(CFURLRef url) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));

CF_EXPORT 
void __CFURLSetResourceInfoPtr(CFURLRef url, void *ptr) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));


/* Creates a URL from posixFilePath (only kCFURLPOSIXPathStyle paths are supported). It determines if the file system object is a directory or not to ensure the URL path is correctly terminated with a '/' or not. It also pre-caches the file system properties specified by keys. Note: not all resource properties can be pre-cached -- just those properties that come from the file system. */
CF_EXPORT
CFURLRef _CFURLCreateWithFileSystemPathCachingResourcePropertiesForKeys(CFAllocatorRef allocator, CFStringRef posixFilePath, CFArrayRef keys, CFErrorRef *error) API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0));

struct FSCatalogInfo;
struct HFSUniStr255;

/* _CFURLGetCatalogInfo is used by LaunchServices */
CF_EXPORT
SInt32 _CFURLGetCatalogInfo(CFURLRef url, UInt32 whichInfo, struct FSCatalogInfo *catalogInfo, struct HFSUniStr255 *name) API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0));

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
Boolean _CFURLReplaceObject( CFAllocatorRef allocator, CFURLRef originalItemURL, CFURLRef newItemURL, CFStringRef newName, CFStringRef backupItemName, CFOptionFlags options, CFURLRef *resultingURL, CFErrorRef *error ) API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0));

CF_EXPORT
Boolean _CFURLIsProtectedDirectory(CFURLRef directoryURL) API_AVAILABLE(macos(10.10)) API_UNAVAILABLE(ios, watchos, tvos);

/* _CFURLAttachSecurityScopeToFileURL attaches a sandbox extension to the file URL object. The URL object will then be security-scoped and will be usable with the NSURL's -startAccessingSecurityScopedResource method and CFURL's CFURLStartAccessingSecurityScopedResource() function. The URL object must be a file URL. If the URL object already has a sandbox extension attached, the new extension replaces the previous sandbox extension. If NULL is passed for the sandboxExtension, the sandbox extension (if any) is removed from the URL object. Callers would be responsible for ensuring the sandbox extension matches the URL's file system path.
    Note: The sandbox extension is a C-string INCLUDING the terminating nul character stored in a CFData object.
 */
CF_EXPORT
void _CFURLAttachSecurityScopeToFileURL(CFURLRef url, CFDataRef sandboxExtension) API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));

/* _CFURLCopySecurityScopeFromFileURL copies the sandbox extension attached to the file URL object. If the URL is not a file URL or doesn't have a sandbox extension, NULL will be returned.
 */
CF_EXPORT
CFDataRef _CFURLCopySecurityScopeFromFileURL(CFURLRef url) API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));

/** _CFURLNoteSecurityScopedResourceMove should be called preserve access to a scoped resource after it has been moved on the file system.
 The process will lose access to sourceURL and gain access to destinationURL. For use by only FileCoordination!
 */
CF_EXPORT
Boolean _CFURLNoteSecurityScopedResourceMoved(CFURLRef sourceURL, CFURLRef destinationURL) API_AVAILABLE(macos(12.0), ios(15.0), watchos(8.0), tvos(15.0));

CF_EXPORT
void _CFURLSetPermanentResourcePropertyForKey(CFURLRef url, CFStringRef key, CFTypeRef propertyValue) API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));


#pragma mark - Bookmarks

// Returns a string describing the bookmark data. For debugging purposes only.
CF_EXPORT
CFStringRef _CFURLBookmarkCopyDescription(CFDataRef bookmarkRef) API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));

#if TARGET_OS_MAC || TARGET_OS_IPHONE
// private CFURLBookmarkCreationOptions
enum {
    kCFURLBookmarkCreationSecurityScopeRevocable API_AVAILABLE(macos(10.16)) API_UNAVAILABLE(ios, watchos, tvos) = ( 1 << 25 ), // if used with kCFURLBookmarkCreationWithSecurityScope, the bookmark can be revoked on a per-app basis.
    kCFURLBookmarkCreationWithFileProvider API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0)) = ( 1UL << 26 ), // private option to create bookmarks with file provider string. The file provider string overrides the rest of the bookmark data at resolution time.
    kCFURLBookmarkOperatingInsideScopedBookmarksAgent = (1UL << 27), // private option used internally by ScopedBookmarkAgent to prevent recursion between the agent and the framework code. Available 10_7, NA
    kCFURLBookmarkCreationAllowCreationIfResourceDoesNotExistMask = ( 1UL << 28 ),    // allow creation of a bookmark to a file: scheme with a CFURLRef of item which may not exist.  If the filesystem item does not exist, the created bookmark contains essentially no properties beyond the url string. Available 10_7, 5_0.
    kCFURLBookmarkCreationDoNotIncludeSandboxExtensionsMask = ( 1UL << 29 ),  // If set, sandbox extensions are not included in created bookmarks. Ordinarily, bookmarks (except those created suitable for putting into a bookmark file) will have a sandbox extension added for the item. Available 10_7, NA.
    kCFURLBookmarkCreationAllowOnlyReadAccess API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0)) = ( 1UL << 30 ), // at resolution time only read access to the resource will be granted (works with regular non-security scoped bookmarks)
    kCFURLBookmarkCreationSuitableForOdocAppleEvent API_DEPRECATED("kCFURLBookmarkCreationSuitableForOdocAppleEvent does nothing and has no effect on bookmark resolution", macos(10.6, 10.11)) API_UNAVAILABLE(ios, watchos, tvos) = ( 1UL << 31 ),   // add properties we guarantee will be in an odoc AppleEvent. Available 10_10, NA (but supported back to 10.6).
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
    kCFBookmarkResolutionPerformRelativeResolutionFirstMask API_AVAILABLE(macos(10.8), ios(6.0), watchos(2.0), tvos(9.0)) = ( 1UL << 11 ), // perform relative resolution before absolute resolution. If this bit is set, for this to be useful a relative URL must also have been passed in and the bookmark when created must have been created relative to another url.
    kCFURLBookmarkResolutionAllowingPromisedItem API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0)) = ( 1UL << 12 ), // If kCFURLBookmarkResolutionAllowingPromisedItem is set, resolving a bookmark may return promise item URL if the target has been evicted to the cloud (instead of downloading the evicted document during bookmark resolution). Clients must use NSPromisedItems and NSFileCoordinator API to access promised item URLs. kCFURLBookmarkResolutionAllowingPromisedItem is ignored when resolving security-scoped bookmarks.
    kCFBookmarkResolutionQuarantineMountedNetworkVolumesMask API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0)) = ( 1UL << 13 ), // quarantine any network volume mounted during resolution
    kCFURLBookmarkResolutionFailPromisedItem API_AVAILABLE(macosx(11.0), ios(14.0), watchos(7.0), tvos(14.0)) = ( 1UL << 14 ), // If kCFURLBookmarkResolutionFailPromisedItem is set, resolving a bookmark will fail if the target has been evicted to the cloud (instead of downloading the evicted document during bookmark resolution).
    kCFURLBookmarkResolutionWithoutExtendingAccess API_AVAILABLE(macos(11.2), ios(14.2), watchos(7.2), tvos(14.2)) = ( 1 << 15 ), // Disable automatic ephemeral extension of the sandbox during resolution. Instead, call `CFURLStartAccessingSecurityScopedResource` on the returned URL when ready to use the resource.
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
CFURLBookmarkMatchResult _CFURLBookmarkDataCompare(CFDataRef bookmark1Ref, CFDataRef bookmark2Ref, CFURLRef relativeToURL, CFArrayRef* matchingPropertyKeys) API_AVAILABLE(macos(10.7)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
OSStatus _CFURLBookmarkDataToAliasHandle(CFDataRef bookmarkRef, void* aliasHandleP) API_DEPRECATED("don't use AliasHandles", macos(10.7, 10.15)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
CFURLRef _CFURLCreateByResolvingAliasFile(CFAllocatorRef allocator, CFURLRef url, CFURLBookmarkResolutionOptions options, CFArrayRef propertiesToInclude, CFErrorRef *error ) API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));

/*
 The following are properties that can be asked of bookmark data objects in addition to the resource properties
 from CFURL itself.
 */

extern const CFStringRef kCFURLBookmarkOriginalPathKey API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0));
extern const CFStringRef kCFURLBookmarkOriginalRelativePathKey API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0));
extern const CFStringRef kCFURLBookmarkOriginalRelativePathComponentsArrayKey API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0));
extern const CFStringRef kCFURLBookmarkOriginalVolumeNameKey API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0));
extern const CFStringRef kCFURLBookmarkOriginalVolumeCreationDateKey API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0));
extern const CFStringRef kCFURLBookmarkFileProviderStringKey API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
extern const CFStringRef _kCFURLBookmarkURLStringKey API_AVAILABLE(macosx(10.13), ios(11.0), watchos(4.0), tvos(11.0));
#endif // TARGET_OS_MAC || TARGET_OS_IPHONE

#pragma mark - Revocable Bookmarks

CF_EXPORT const CFStringRef _kCFURLRevocableBookmarkBundleIdentifierKey API_AVAILABLE(macos(10.16)) API_UNAVAILABLE(ios, watchos, tvos);	// CFStringRef
CF_EXPORT const CFStringRef _kCFURLRevocableBookmarkAppIdentifierKey API_AVAILABLE(macos(10.16)) API_UNAVAILABLE(ios, watchos, tvos);		// CFStringRef
CF_EXPORT const CFStringRef _kCFURLRevocableBookmarkActiveStatusKey API_AVAILABLE(macos(10.16)) API_UNAVAILABLE(ios, watchos, tvos);		// CFBooleanRef
CF_EXPORT const CFStringRef _kCFURLRevocableBookmarkSaltKey API_AVAILABLE(macos(10.16)) API_UNAVAILABLE(ios, watchos, tvos);			// CFDataRef

/** Fetch a list of  clients.
 *
 * Returns an array of dictionaries, one per client app. Keys from the namespace _kCFURLRevocableBookmarkKey.
 */
CF_EXPORT CFArrayRef _CFURLRevocableBookmarksCopyClients(void) API_AVAILABLE(macos(10.16)) API_UNAVAILABLE(ios, watchos, tvos);

/** Fetch a list of bundle identifiers for active clients. */
CF_EXPORT CFArrayRef _CFURLRevocableBookmarksCopyClientBundleIdentifiers(Boolean includeInactive) API_AVAILABLE(macos(10.16)) API_UNAVAILABLE(ios, watchos, tvos);

/** Set the active state of the app with the given bundle identifier. This does not delete the security token for the app, thus is less secure than revoking the bundle identifier. */
CF_EXPORT Boolean _CFURLRevocableBookmarksSetActiveStatusForBundleIdentifier(CFStringRef identifier, Boolean active) API_AVAILABLE(macos(10.16)) API_UNAVAILABLE(ios, watchos, tvos);

/** Securely revokes all bookmarks for the bundie identifier. This is not reversable. */
CF_EXPORT Boolean _CFURLRevocableBookmarksRevokeForBundleIdentifier(CFStringRef identifier) API_AVAILABLE(macos(10.16)) API_UNAVAILABLE(ios, watchos, tvos);

/** Notification sent when the set of active clients changes. */
CF_EXPORT const CFNotificationName _kCFURLRevocableBookmarksClientsDidChangeNotification API_AVAILABLE(macos(10.16)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFURLPRIV__ */

