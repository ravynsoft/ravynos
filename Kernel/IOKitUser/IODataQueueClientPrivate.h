/*
 * Copyright (c) 2013 Apple Computer, Inc. All rights reserved.
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

#ifndef _IOKITUSER_IODATAQUEUE_PRIVATE_H
#define _IOKITUSER_IODATAQUEUE_PRIVATE_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <IOKit/IODataQueueClient.h>

typedef uint32_t (*IODataQueueClientEnqueueReadBytesCallback)(void * refcon, void *data, uint32_t dataSize);

IOReturn
_IODataQueueEnqueueWithReadCallback(IODataQueueMemory *dataQueue, uint64_t queueSize, mach_msg_header_t *msgh, uint32_t dataSize, IODataQueueClientEnqueueReadBytesCallback callback, void * refcon);

/*
 * Internal Peek and Dequeue functions. These functions allow us to pass in the
 * queue size, since queue size is part of shared memory and may be modified by
 * a bad client. See rdar://42664354&42630424&42679853
 */
IODataQueueEntry *_IODataQueuePeek(IODataQueueMemory *dataQueue, uint64_t queueSize,  size_t *entrySize);

IOReturn _IODataQueueDequeue(IODataQueueMemory *dataQueue, uint64_t queueSize, void *data, uint32_t *dataSize);

__END_DECLS

#endif /* _IOKITUSER_IODATAQUEUE_PRIVATE_H */
