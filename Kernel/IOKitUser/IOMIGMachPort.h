/*
 * Copyright (c) 2009 Apple, Inc.  All Rights Reserved.
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

/*
 *  IOMIGMachPort.h
 *
 *  Created by Roberto Yepez on 1/29/09.
 *  Copyright 2009 Apple, Inc. All rights reserved.
 *
 */

#ifndef _IO_MIG_MACH_PORT_H_
#define _IO_MIG_MACH_PORT_H_

#include <dispatch/dispatch.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach.h>

__BEGIN_DECLS

typedef struct __IOMIGMachPort * IOMIGMachPortRef;

typedef void (*IOMIGMachPortTerminationCallback)(IOMIGMachPortRef client, void * refcon);
typedef Boolean (*IOMIGMachPortDemuxCallback)(IOMIGMachPortRef client, mach_msg_header_t * request, mach_msg_header_t * reply, void *refcon);

    
CF_EXPORT
CFTypeID IOMIGMachPortGetTypeID(void);

CF_EXPORT
IOMIGMachPortRef IOMIGMachPortCreate(CFAllocatorRef allocator, CFIndex maxMessageSize, mach_port_t port);

CF_EXPORT
mach_port_t IOMIGMachPortGetPort(IOMIGMachPortRef migPort);

CF_EXPORT
void IOMIGMachPortRegisterTerminationCallback(IOMIGMachPortRef client, IOMIGMachPortTerminationCallback callback, void *refcon);

CF_EXPORT
void IOMIGMachPortRegisterDemuxCallback(IOMIGMachPortRef client, IOMIGMachPortDemuxCallback callback, void *refcon);

CF_EXPORT
void IOMIGMachPortScheduleWithRunLoop(IOMIGMachPortRef server, CFRunLoopRef runLoop, CFStringRef runLoopMode);

CF_EXPORT
void IOMIGMachPortUnscheduleFromRunLoop(IOMIGMachPortRef server, CFRunLoopRef runLoop, CFStringRef runLoopMode);

CF_EXPORT
void IOMIGMachPortScheduleWithDispatchQueue(IOMIGMachPortRef server, dispatch_queue_t queue);

CF_EXPORT
void IOMIGMachPortUnscheduleFromDispatchQueue(IOMIGMachPortRef server, dispatch_queue_t queue);

// PORT CACHE SUPPORT
CF_EXPORT
void IOMIGMachPortCacheAdd(mach_port_t port, CFTypeRef server);

CF_EXPORT
void IOMIGMachPortCacheRemove(mach_port_t port);

CF_EXPORT
CFTypeRef IOMIGMachPortCacheCopy(mach_port_t port);

__END_DECLS


#endif /* _IO_MIG_MACH_PORT_H_ */
