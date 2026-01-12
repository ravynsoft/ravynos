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

/*	CFMessagePort.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFMESSAGEPORT__)
#define __COREFOUNDATION_CFMESSAGEPORT__ 1

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFData.h>
#include <dispatch/dispatch.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct CF_BRIDGED_MUTABLE_TYPE(NSMessagePort) __CFMessagePort * CFMessagePortRef;

enum {
    kCFMessagePortSuccess = 0,
    kCFMessagePortSendTimeout = -1,
    kCFMessagePortReceiveTimeout = -2,
    kCFMessagePortIsInvalid = -3,
    kCFMessagePortTransportError = -4,
    kCFMessagePortBecameInvalidError = -5
};

typedef struct {
    CFIndex	version;
    void *	info;
    const void *(*retain)(const void *info);
    void	(*release)(const void *info);
    CFStringRef	(*copyDescription)(const void *info);
} CFMessagePortContext;

typedef CFDataRef (*CFMessagePortCallBack)(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
/* If callout wants to keep a hold of the data past the return of the callout, it must COPY the data. This includes the case where the data is given to some routine which _might_ keep a hold of it; System will release returned CFData. */
typedef void (*CFMessagePortInvalidationCallBack)(CFMessagePortRef ms, void *info);

CF_EXPORT CFTypeID	CFMessagePortGetTypeID(void);

CF_EXPORT CFMessagePortRef	CFMessagePortCreateLocal(CFAllocatorRef allocator, CFStringRef name, CFMessagePortCallBack callout, CFMessagePortContext *context, Boolean *shouldFreeInfo);
CF_EXPORT CFMessagePortRef	CFMessagePortCreateRemote(CFAllocatorRef allocator, CFStringRef name);

CF_EXPORT Boolean	CFMessagePortIsRemote(CFMessagePortRef ms);
CF_EXPORT CFStringRef	CFMessagePortGetName(CFMessagePortRef ms);
CF_EXPORT Boolean	CFMessagePortSetName(CFMessagePortRef ms, CFStringRef newName);
CF_EXPORT void		CFMessagePortGetContext(CFMessagePortRef ms, CFMessagePortContext *context);
CF_EXPORT void		CFMessagePortInvalidate(CFMessagePortRef ms);
CF_EXPORT Boolean	CFMessagePortIsValid(CFMessagePortRef ms);
CF_EXPORT CFMessagePortInvalidationCallBack CFMessagePortGetInvalidationCallBack(CFMessagePortRef ms);
CF_EXPORT void CFMessagePortSetInvalidationCallBack(CFMessagePortRef ms, CFMessagePortInvalidationCallBack callout);

/* NULL replyMode argument means no return value expected, dont wait for it */
CF_EXPORT SInt32	CFMessagePortSendRequest(CFMessagePortRef remote, SInt32 msgid, CFDataRef data, CFTimeInterval sendTimeout, CFTimeInterval rcvTimeout, CFStringRef replyMode, CFDataRef *returnData);

CF_EXPORT CFRunLoopSourceRef	CFMessagePortCreateRunLoopSource(CFAllocatorRef allocator, CFMessagePortRef local, CFIndex order);

CF_EXPORT void CFMessagePortSetDispatchQueue(CFMessagePortRef ms, dispatch_queue_t queue) CF_AVAILABLE(10_6, 4_0);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFMESSAGEPORT__ */

