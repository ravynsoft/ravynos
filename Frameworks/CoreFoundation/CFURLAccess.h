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

/*	CFURLAccess.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.

        CFURLAccess is deprecated as of Mac OS X 10.9 and iOS 7.0. The suggested replacement for URLs with network schemes (http, https, ftp, data) are the NSURLSession or NSURLConnection classes. The suggested replacement for URLs with the file scheme are the foundation classes NSFileManager, NSFileHandle and NSURL, or the CoreFoundation classes CFStream and CFURL.
*/

#if !defined(__COREFOUNDATION_CFURLACCESS__)
#define __COREFOUNDATION_CFURLACCESS__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN



/* Attempts to read the data and properties for the given URL.  If
only interested in one of the resourceData and properties, pass NULL
for the other.  If properties is non-NULL and desiredProperties is
NULL, then all properties are fetched.  Returns success or failure;
note that as much work as possible is done even if false is returned.
So for instance if one property is not available, the others are
fetched anyway. errorCode is set to 0 on success, and some other
value on failure.  If non-NULL, it is the caller 's responsibility
to release resourceData and properties.

    Apple reserves for its use all negative error code values; these
values represent errors common to any scheme.  Scheme-specific error
codes should be positive, non-zero, and should be used only if one of
the predefined Apple error codes does not apply.  Error codes should
be publicized and documented with the scheme-specific properties.

NOTE: When asking for the resource data, this call will allocate the entire
resource in memory. This can be very expensive, depending on the size of the
resource (file). Please use CFStream or other techniques if you are downloading
large files.

*/
/* Deprecated -- see top of this file for suggested replacement classes */
CF_EXPORT
Boolean CFURLCreateDataAndPropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFDataRef *resourceData, CFDictionaryRef *properties, CFArrayRef desiredProperties, SInt32 *errorCode) CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "For resource data, use the CFReadStream API. For file resource properties, use CFURLCopyResourcePropertiesForKeys.");

/* Attempts to write the given data and properties to the given URL.
If dataToWrite is NULL, only properties are written out (use
CFURLDestroyResource() to delete a resource).  Properties not present
in propertiesToWrite are left unchanged, hence if propertiesToWrite
is NULL or empty, the URL's properties are not changed at all.
Returns success or failure; errorCode is set as for
CFURLCreateDataAndPropertiesFromResource(), above.
*/
/* Deprecated -- see top of this file for suggested replacement classes */
CF_EXPORT
Boolean CFURLWriteDataAndPropertiesToResource(CFURLRef url, CFDataRef dataToWrite, CFDictionaryRef propertiesToWrite, SInt32 *errorCode) CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "For resource data, use the CFWriteStream API. For file resource properties, use CFURLSetResourcePropertiesForKeys.");

/* Destroys the resource indicated by url.
Returns success or failure; errorCode set as above.
*/
/* Deprecated -- see top of this file for suggested replacement classes */
CF_EXPORT
Boolean CFURLDestroyResource(CFURLRef url, SInt32 *errorCode) CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "Use CFURLGetFileSystemRepresentation and removefile(3) instead.");

/* Convenience method which calls through to CFURLCreateDataAndPropertiesFromResource().
Returns NULL on error and sets errorCode accordingly.
*/
/* Deprecated -- see top of this file for suggested replacement classes */
CF_EXPORT
CFTypeRef CFURLCreatePropertyFromResource(CFAllocatorRef alloc, CFURLRef url, CFStringRef property, SInt32 *errorCode) CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "For file resource properties, use CFURLCopyResourcePropertyForKey.");


/* Common error codes (returned only by the older APIs that predate CFError) */
typedef CF_ENUM(CFIndex, CFURLError) {
    kCFURLUnknownError = -10L,
    kCFURLUnknownSchemeError = -11L,
    kCFURLResourceNotFoundError = -12L,
    kCFURLResourceAccessViolationError = -13L,
    kCFURLRemoteHostUnavailableError = -14L,
    kCFURLImproperArgumentsError = -15L,
    kCFURLUnknownPropertyKeyError = -16L,
    kCFURLPropertyKeyUnavailableError = -17L,
    kCFURLTimeoutError = -18L
} CF_ENUM_DEPRECATED(10_0, 10_9, 2_0, 7_0);

/* Older property keys */

CF_EXPORT
const CFStringRef kCFURLFileExists CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "Use CFURLResourceIsReachable instead.");
CF_EXPORT
const CFStringRef kCFURLFileDirectoryContents CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "Use the CFURLEnumerator API instead.");
CF_EXPORT
const CFStringRef kCFURLFileLength CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "Use CFURLCopyResourcePropertyForKey with kCFURLFileSizeKey instead.");
CF_EXPORT 
const CFStringRef kCFURLFileLastModificationTime CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "Use CFURLCopyResourcePropertyForKey with kCFURLContentModificationDateKey instead.");
CF_EXPORT
const CFStringRef kCFURLFilePOSIXMode CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "Use CFURLCopyResourcePropertyForKey with kCFURLFileSecurityKey and then the CFFileSecurity API instead.");
CF_EXPORT
const CFStringRef kCFURLFileOwnerID CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "Use CFURLCopyResourcePropertyForKey with kCFURLFileSecurityKey and then the CFFileSecurity API instead.");
CF_EXPORT
const CFStringRef kCFURLHTTPStatusCode CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "Use NSHTTPURLResponse methods instead.");
CF_EXPORT
const CFStringRef kCFURLHTTPStatusLine CF_DEPRECATED(10_0, 10_9, 2_0, 7_0, "Use NSHTTPURLResponse methods instead.");

/* The value of kCFURLFileExists is a CFBoolean */
/* The value of kCFURLFileDirectoryContents is a CFArray containing CFURLs.  An empty array means the directory exists, but is empty */
/* The value of kCFURLFileLength is a CFNumber giving the file's length in bytes */
/* The value of kCFURLFileLastModificationTime is a CFDate */
/* The value of kCFURLFilePOSIXMode is a CFNumber as given in stat.h */
/* The value of kCFURLFileOwnerID is a CFNumber representing the owner's uid */

/* Properties for the http: scheme.  Except for the common error codes, above, errorCode will be set to the HTTP response status code upon failure.  Any HTTP header name can also be used as a property */
/* The value of kCFURLHTTPStatusCode is a CFNumber */
/* The value of kCFURLHTTPStatusLine is a CFString */

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFURLACCESS__ */

