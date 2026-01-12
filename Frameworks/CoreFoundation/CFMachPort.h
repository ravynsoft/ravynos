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

/*	CFMachPort.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFMACHPORT__)
#define __COREFOUNDATION_CFMACHPORT__ 1

#include <CoreFoundation/CFRunLoop.h>
#include <mach/port.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct CF_BRIDGED_MUTABLE_TYPE(NSMachPort) __CFMachPort * CFMachPortRef;

typedef struct {
    CFIndex	version;
    void *	info;
    const void *(*retain)(const void *info);
    void	(*release)(const void *info);
    CFStringRef	(*copyDescription)(const void *info);
} CFMachPortContext;

typedef void (*CFMachPortCallBack)(CFMachPortRef port, void *msg, CFIndex size, void *info);
typedef void (*CFMachPortInvalidationCallBack)(CFMachPortRef port, void *info);

CF_EXPORT CFTypeID	CFMachPortGetTypeID(void);

CF_EXPORT CFMachPortRef	CFMachPortCreate(CFAllocatorRef allocator, CFMachPortCallBack callout, CFMachPortContext *context, Boolean *shouldFreeInfo);
CF_EXPORT CFMachPortRef	CFMachPortCreateWithPort(CFAllocatorRef allocator, mach_port_t portNum, CFMachPortCallBack callout, CFMachPortContext *context, Boolean *shouldFreeInfo);

CF_EXPORT mach_port_t	CFMachPortGetPort(CFMachPortRef port);
CF_EXPORT void		CFMachPortGetContext(CFMachPortRef port, CFMachPortContext *context);
CF_EXPORT void		CFMachPortInvalidate(CFMachPortRef port);
CF_EXPORT Boolean	CFMachPortIsValid(CFMachPortRef port);
CF_EXPORT CFMachPortInvalidationCallBack CFMachPortGetInvalidationCallBack(CFMachPortRef port);
CF_EXPORT void		CFMachPortSetInvalidationCallBack(CFMachPortRef port, CFMachPortInvalidationCallBack callout);

CF_EXPORT CFRunLoopSourceRef	CFMachPortCreateRunLoopSource(CFAllocatorRef allocator, CFMachPortRef port, CFIndex order);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFMACHPORT__ */

