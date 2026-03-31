/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
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
/* IOUnserialize.h created by rsulack on Mon 23-Nov-1998 */
/* IOCFUnserialize.h creates CF collections Mon 30-Aug-1999 */

#ifndef __IOKIT_IOCFUNSERIALIZE_H
#define __IOKIT_IOCFUNSERIALIZE_H

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>

#if defined(__cplusplus)
extern "C" {
#endif

// on success IOCFUnserialize sets errorString to 0 and returns
// the unserialized object.

// on failure IOCFUnserialize sets errorString to a CFString object 
// containing a error message suitable for logging and returns 0

CF_RETURNS_RETAINED
CFTypeRef
IOCFUnserialize(const char *buffer,
                CFAllocatorRef allocator,
                CFOptionFlags options,
                CFStringRef *errorString);

CF_RETURNS_RETAINED
CFTypeRef
IOCFUnserializeBinary(const char	* buffer,
					  size_t          bufferSize,
					  CFAllocatorRef  allocator,
					  CFOptionFlags	  options,
					  CFStringRef	* errorString);

CF_RETURNS_RETAINED
CFTypeRef
IOCFUnserializeWithSize(const char	  * buffer,
						size_t          bufferSize,
						CFAllocatorRef	allocator,
						CFOptionFlags	options,
						CFStringRef	  * errorString);

#if defined(__cplusplus)
}
#endif

#endif /* __IOKIT_IOCFUNSERIALIZE_H */
