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

#ifndef _IOKITUSER_IODATAQUEUE_H
#define _IOKITUSER_IODATAQUEUE_H

#include <sys/cdefs.h>

__BEGIN_DECLS
#include <AvailabilityMacros.h>
#include <libkern/OSTypes.h>
#include <mach/port.h>
#include <IOKit/IOReturn.h>
#include <IOKit/IODataQueueShared.h>
#include <stdint.h> /* uint32_t */

/*!
 * @function IODataQueueDataAvailable
 * @abstract Used to determine if more data is avilable on the queue.
 * @param dataQueue The IODataQueueMemory region mapped from the kenel.
 * @result Returns true if data is available and false if not.
 */
Boolean IODataQueueDataAvailable(IODataQueueMemory *dataQueue);

/*!
 * @function IODataQueuePeek
 * @abstract Used to peek at the next entry on the queue.
 * @discussion This function can be used to look at the next entry which allows the entry to be received without having to copy it with IODataQueueDequeue.  In order to do this, call IODataQueuePeek to get the entry.  Then call IODataQueueDequeue with a NULL data pointer.  That will cause the head to be moved to the next entry, but no memory to be copied.
 * @param dataQueue The IODataQueueMemory region mapped from the kernel.
 * @result Returns a pointer to the next IODataQueueEntry if one is available.  Zero is returned if the queue is empty.
 */
IODataQueueEntry *IODataQueuePeek(IODataQueueMemory *dataQueue);

/*!
 * @function IODataQueueDequeue
 * @abstract Dequeues the next available entry on the queue and copies it into the given data pointer.
 * @discussion This function will dequeue the next available entry on the queue.  If a data pointer is provided, it will copy the data into the memory region if there is enough space available as specified in the dataSize parameter.  If no data pointer is provided, it will simply move the head value past the current entry.
 * @param dataQueue The IODataQueueMemory region mapped from the kernel.
 * @param data A pointer to the data memory region in which to copy the next entry data on the queue.  If this parameter is 0 (NULL), it will simply move to the next entry.
 * @param dataSize A pointer to the size of the data parameter.  On return, this contains the size of the actual entry data - even if the original size was not large enough.
 * @result Returns kIOReturnSuccess on success.  Other return values possible are: kIOReturnUnderrun - queue is empty, kIOReturnBadArgument - no dataQueue or no dataSize, kIOReturnNoSpace - dataSize is too small for entry.
 */
IOReturn IODataQueueDequeue(IODataQueueMemory *dataQueue, void *data, uint32_t *dataSize);

/*!
 * @function IODataQueueWaitForAvailableData
 * @abstract Wait for an incoming dataAvailable message on the given notifyPort.
 * @discussion This method will simply wait for an incoming message on the given notifyPort.  Once it is received, the return from mach_msg() is returned.
 * @param dataQueue The IODataQueueMemory region mapped from the kernel.
 * @param notificationPort Mach port on which to listen for incoming messages.
 * @result Returns kIOReturnSuccess on success.  Returns kIOReturnBadArgument if either dataQueue is 0 (NULL) or notiryPort is MACH_PORT_NULL.  Returns the result of the mach_msg() listen call on the given port.
 */
IOReturn IODataQueueWaitForAvailableData(IODataQueueMemory *dataQueue, mach_port_t notificationPort);

/*!
 * @function IODataQueueAllocateNotificationPort
 * @abstract Allocates and returns a new mach port able to receive data available notifications from an IODataQueue.
 * @discussion This port is intended to be passed down into the kernel and into an IODataQueue to allow it to send the appropriate notification.  The returned mach port is allocated with a queue limit of one message.  This allows only one mach message to be queued up at a time.  The IODataQueue code is written with the restriction in mind and will only queue up a message if no messages alread have been sent.
 * @result Returns a newly allocated mach port on success.  On failure, it returns MACH_PORT_NULL.
 */
mach_port_t IODataQueueAllocateNotificationPort();

/*!
 * @function IODataQueueEnqueue
 * @abstract Enqueues a new entry on the queue.
 * @discussion This method adds a new data entry of dataSize to the queue.  It sets the size parameter of the entry pointed to by the tail value and copies the memory pointed to by the data parameter in place in the queue.  Once that is done, it moves the tail to the next available location.  When attempting to add a new entry towards the end of the queue and there isn't enough space at the end, it wraps back to the beginning.<br>  If the queue is empty when a new entry is added, the port specified in IODataQueueSetNotificationPort will be used to send a message to the client process that data is now available. <br> <b>Please note that using this method without mapped memory create from an IOSharedDataQueue will result in undefined behavior. </b>
 * @param dataQueue The IODataQueueMemory region mapped from the kernel created from an IOSharedDataQueue.
 * @param data Pointer to the data to be added to the queue.
 * @param dataSize Size of the data pointed to by data.
 * @result Returns kIOReturnSuccess on success.  Other return values possible are: kIOReturnOverrun - queue is full.
 */
IOReturn IODataQueueEnqueue(IODataQueueMemory *dataQueue, void *data, uint32_t dataSize) AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
 * @function IODataQueueSetNotificationPort
 * @abstract Creates a simple mach message targeting the mach port specified in port.
 * @discussion This message is sent when data is added to an empty queue.  It is to notify another user process that new data has become available.  <b>Please note that using this method without mapped memory create from an IOSharedDataQueue will result in undefined behavior. </b>
 * @param dataQueue The IODataQueueMemory region mapped from the kernel created from an IOSharedDataQueue.
 * @param notifyPort The mach port to target with the notification message.
 * @result Returns kIOReturnSuccess on success.  Returns kIOReturnBadArgument if either dataQueue is 0 (NULL).
 */
IOReturn IODataQueueSetNotificationPort(IODataQueueMemory *dataQueue, mach_port_t notifyPort) AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

__END_DECLS

#endif /* _IOKITUSER_IODATAQUEUE_H */
