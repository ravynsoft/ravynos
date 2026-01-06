/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#ifndef __IOKIT_IOCFURLACCESS_H
#define __IOKIT_IOCFURLACCESS_H

#include <CoreFoundation/CoreFoundation.h>

CFTypeRef IOURLCreatePropertyFromResource(CFAllocatorRef alloc, CFURLRef url, CFStringRef property, SInt32 *errorCode);

Boolean IOURLCreateDataAndPropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFDataRef *resourceData, CFDictionaryRef *properties, CFArrayRef desiredProperties, SInt32 *errorCode);

Boolean IOURLWriteDataAndPropertiesToResource(CFURLRef url, CFDataRef dataToWrite, CFDictionaryRef propertiesToWrite, int32_t *errorCode);

#ifdef HAVE_CFURLACCESS

#define kIOURLFileExists		kCFURLFileExists
#define kIOURLFileDirectoryContents	kCFURLFileDirectoryContents
#define kIOURLFileLength		kCFURLFileLength
#define kIOURLFileLastModificationTime	kCFURLFileLastModificationTime
#define kIOURLFilePOSIXMode		kCFURLFilePOSIXMode
#define kIOURLFileOwnerID		kCFURLFileOwnerID

/* Common error codes; this list is expected to grow */

typedef CFURLError IOURLError;

enum {
    kIOURLUnknownError 			= kCFURLUnknownError,
    kIOURLUnknownSchemeError 		= kCFURLUnknownSchemeError,
    kIOURLResourceNotFoundError 	= kCFURLResourceNotFoundError,
    kIOURLResourceAccessViolationError 	= kCFURLResourceAccessViolationError,
    kIOURLRemoteHostUnavailableError 	= kCFURLRemoteHostUnavailableError,
    kIOURLImproperArgumentsError 	= kCFURLImproperArgumentsError,
    kIOURLUnknownPropertyKeyError 	= kCFURLUnknownPropertyKeyError,
    kIOURLPropertyKeyUnavailableError 	= kCFURLPropertyKeyUnavailableError,
    kIOURLTimeoutError 			= kCFURLTimeoutError
};

#else /* !HAVE_CFURLACCESS */

#define kIOURLFileExists		CFSTR("kIOURLFileExists")
#define kIOURLFileDirectoryContents	CFSTR("kIOURLFileDirectoryContents")
#define kIOURLFileLength		CFSTR("kIOURLFileLength")
#define kIOURLFileLastModificationTime	CFSTR("kIOURLFileLastModificationTime")
#define kIOURLFilePOSIXMode		CFSTR("kIOURLFilePOSIXMode")
#define kIOURLFileOwnerID		CFSTR("kIOURLFileOwnerID")

/* Common error codes; this list is expected to grow */

typedef enum {
      kIOURLUnknownError = -10,
          kIOURLUnknownSchemeError = -11,
          kIOURLResourceNotFoundError = -12,
          kIOURLResourceAccessViolationError = -13,
          kIOURLRemoteHostUnavailableError = -14,
          kIOURLImproperArgumentsError = -15,
          kIOURLUnknownPropertyKeyError = -16,
          kIOURLPropertyKeyUnavailableError = -17,
          kIOURLTimeoutError = -18
} IOURLError;

#endif /* !HAVE_CFURLACCESS */

#endif /* __IOKIT_IOCFURLACCESS_H */
