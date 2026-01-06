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

#include "IODataQueueClientPrivate.h"
#include <IOKit/IODataQueueShared.h>

#include <mach/mach.h>
#include <IOKit/OSMessageNotification.h>
#include <libkern/OSAtomic.h>


static IOReturn _IODataQueueSendDataAvailableNotification(IODataQueueMemory *dataQueue, mach_msg_header_t *msgh);

Boolean IODataQueueDataAvailable(IODataQueueMemory *dataQueue)
{
    return (dataQueue && (dataQueue->head != dataQueue->tail));
}

IODataQueueEntry *__IODataQueuePeek(IODataQueueMemory *dataQueue, uint64_t qSize, size_t *entrySize)
{
    IODataQueueEntry *entry = 0;
    UInt32            headOffset;
    UInt32            tailOffset;
    size_t            size = 0;
    
    if (!dataQueue) {
        return NULL;
    }
    
    // Read head and tail with acquire barrier
    headOffset = __c11_atomic_load((_Atomic UInt32 *)&dataQueue->head, __ATOMIC_RELAXED);
    tailOffset = __c11_atomic_load((_Atomic UInt32 *)&dataQueue->tail, __ATOMIC_ACQUIRE);
    
    if (headOffset != tailOffset) {
        IODataQueueEntry *  head        = 0;
        UInt32              headSize    = 0;
        UInt32              queueSize   = qSize ? qSize : dataQueue->queueSize;
        
        if (headOffset > queueSize) {
            return NULL;
        }
        
        head         = (IODataQueueEntry *)((char *)dataQueue->queue + headOffset);
        headSize     = head->size;
        
        // Check if there's enough room before the end of the queue for a header.
        // If there is room, check if there's enough room to hold the header and
        // the data.
        
        if ((headOffset > UINT32_MAX - DATA_QUEUE_ENTRY_HEADER_SIZE) ||
            (headOffset + DATA_QUEUE_ENTRY_HEADER_SIZE > queueSize) ||
            (headOffset + DATA_QUEUE_ENTRY_HEADER_SIZE > UINT32_MAX - headSize) ||
            (headOffset + headSize + DATA_QUEUE_ENTRY_HEADER_SIZE > queueSize)) {
            // No room for the header or the data, wrap to the beginning of the queue.
            // Note: wrapping even with the UINT32_MAX checks, as we have to support
            // queueSize of UINT32_MAX
            entry = dataQueue->queue;
            
            size = entry ? entry->size : 0;
            
            if ((size > UINT32_MAX - DATA_QUEUE_ENTRY_HEADER_SIZE) ||
                (size + DATA_QUEUE_ENTRY_HEADER_SIZE > queueSize)) {
                return NULL;
            }
            
            if (entrySize) {
                *entrySize = size;
            }
            
        } else {
            entry = head;
            
            if (entrySize) {
                *entrySize = headSize;
            }
        }
    }
    
    return entry;
}

IODataQueueEntry *IODataQueuePeek(IODataQueueMemory *dataQueue)
{
    size_t entrySize = 0;
    
    return __IODataQueuePeek(dataQueue, 0, &entrySize);
}

IODataQueueEntry *_IODataQueuePeek(IODataQueueMemory *dataQueue, uint64_t queueSize,  size_t *entrySize)
{
    return __IODataQueuePeek(dataQueue, queueSize, entrySize);
}

IOReturn
__IODataQueueDequeue(IODataQueueMemory *dataQueue, uint64_t qSize, void *data, uint32_t *dataSize)
{
    IOReturn            retVal          = kIOReturnSuccess;
    IODataQueueEntry *  entry           = 0;
    UInt32              entrySize       = 0;
    UInt32              headOffset      = 0;
    UInt32              tailOffset      = 0;
    UInt32              newHeadOffset   = 0;
    
    if (!dataQueue || (data && !dataSize)) {
        return kIOReturnBadArgument;
    }
    
    // Read head and tail with acquire barrier
    headOffset = __c11_atomic_load((_Atomic UInt32 *)&dataQueue->head, __ATOMIC_RELAXED);
    tailOffset = __c11_atomic_load((_Atomic UInt32 *)&dataQueue->tail, __ATOMIC_ACQUIRE);
    
    if (headOffset != tailOffset) {
        IODataQueueEntry *  head        = 0;
        UInt32              headSize    = 0;
        UInt32              queueSize   = qSize ? qSize : dataQueue->queueSize;
        
        if (headOffset > queueSize) {
            return kIOReturnError;
        }
        
        head         = (IODataQueueEntry *)((char *)dataQueue->queue + headOffset);
        headSize     = head->size;
        
        // we wrapped around to beginning, so read from there
        // either there was not even room for the header
        if ((headOffset > UINT32_MAX - DATA_QUEUE_ENTRY_HEADER_SIZE) ||
            (headOffset + DATA_QUEUE_ENTRY_HEADER_SIZE > queueSize) ||
            // or there was room for the header, but not for the data
            (headOffset + DATA_QUEUE_ENTRY_HEADER_SIZE > UINT32_MAX - headSize) ||
            (headOffset + headSize + DATA_QUEUE_ENTRY_HEADER_SIZE > queueSize)) {
            // Note: we have to wrap to the beginning even with the UINT32_MAX checks
            // because we have to support a queueSize of UINT32_MAX.
            entry           = dataQueue->queue;
            entrySize       = entry->size;
            if ((entrySize > UINT32_MAX - DATA_QUEUE_ENTRY_HEADER_SIZE) ||
                (entrySize + DATA_QUEUE_ENTRY_HEADER_SIZE > queueSize)) {
                return kIOReturnError;
            }
            newHeadOffset   = entrySize + DATA_QUEUE_ENTRY_HEADER_SIZE;
            // else it is at the end
        } else {
            entry           = head;
            entrySize       = entry->size;
            if ((entrySize > UINT32_MAX - DATA_QUEUE_ENTRY_HEADER_SIZE) ||
                (entrySize + DATA_QUEUE_ENTRY_HEADER_SIZE > UINT32_MAX - headOffset) ||
                (entrySize + DATA_QUEUE_ENTRY_HEADER_SIZE + headOffset > queueSize)) {
                return kIOReturnError;
            }
            newHeadOffset   = headOffset + entrySize + DATA_QUEUE_ENTRY_HEADER_SIZE;
        }
    } else {
        // empty queue
        return kIOReturnUnderrun;
    }
    
    if (data) {
        if (entrySize > *dataSize) {
            // not enough space
            return kIOReturnNoSpace;
        }
        memcpy(data, &(entry->data), entrySize);
        *dataSize = entrySize;
    }
    
    __c11_atomic_store((_Atomic UInt32 *)&dataQueue->head, newHeadOffset, __ATOMIC_RELEASE);
    
    if (newHeadOffset == tailOffset) {
        //
        // If we are making the queue empty, then we need to make sure
        // that either the enqueuer notices, or we notice the enqueue
        // that raced with our making of the queue empty.
        //
        __c11_atomic_thread_fence(__ATOMIC_SEQ_CST);
    }
    
    return retVal;
}

IOReturn
IODataQueueDequeue(IODataQueueMemory *dataQueue, void *data, uint32_t *dataSize)
{
    return __IODataQueueDequeue(dataQueue, 0, data, dataSize);
}

IOReturn _IODataQueueDequeue(IODataQueueMemory *dataQueue, uint64_t queueSize, void *data, uint32_t *dataSize)
{
    return __IODataQueueDequeue(dataQueue, queueSize, data, dataSize);
}

static IOReturn
__IODataQueueEnqueue(IODataQueueMemory *dataQueue, uint64_t qSize, mach_msg_header_t *msgh, uint32_t dataSize, void *data, IODataQueueClientEnqueueReadBytesCallback callback, void * refcon)
{
    UInt32              head;
    UInt32              tail;
    UInt32              newTail;
    UInt32              queueSize   = qSize ? qSize : dataQueue->queueSize;
    UInt32              entrySize   = dataSize + DATA_QUEUE_ENTRY_HEADER_SIZE;
    IOReturn            retVal      = kIOReturnSuccess;
    IODataQueueEntry *  entry;
    
    // Force a single read of head and tail
    tail = __c11_atomic_load((_Atomic UInt32 *)&dataQueue->tail, __ATOMIC_RELAXED);
    head = __c11_atomic_load((_Atomic UInt32 *)&dataQueue->head, __ATOMIC_ACQUIRE);
    
    // Check for overflow of entrySize
    if (dataSize > UINT32_MAX - DATA_QUEUE_ENTRY_HEADER_SIZE) {
        return kIOReturnOverrun;
    }
    // Check for underflow of (getQueueSize() - tail)
    if (queueSize < tail || queueSize < head) {
        return kIOReturnUnderrun;
    }

    if ( tail >= head )
    {
        // Is there enough room at the end for the entry?
        if ((entrySize <= UINT32_MAX - tail) &&
            ((tail + entrySize) <= queueSize) )
        {
            entry = (IODataQueueEntry *)((UInt8 *)dataQueue->queue + tail);

            if ( data )
                memcpy(&(entry->data), data, dataSize);
            else if ( callback )
                (*callback)(refcon, &(entry->data), dataSize);

            entry->size = dataSize;            

            // The tail can be out of bound when the size of the new entry
            // exactly matches the available space at the end of the queue.
            // The tail can range from 0 to queueSize inclusive.

            newTail = tail + entrySize;
        }
        else if ( head > entrySize )     // Is there enough room at the beginning?
        {
            entry = (IODataQueueEntry *)((UInt8 *)dataQueue->queue);
            
            if ( data ) 
                memcpy(&(entry->data), data, dataSize);
            else if ( callback )
                (*callback)(refcon, &(entry->data), dataSize);

            // Wrap around to the beginning, but do not allow the tail to catch
            // up to the head.

            entry->size = dataSize;

            // We need to make sure that there is enough room to set the size before
            // doing this. The user client checks for this and will look for the size
            // at the beginning if there isn't room for it at the end.

            if ( ( queueSize - tail ) >= DATA_QUEUE_ENTRY_HEADER_SIZE )
            {
                ((IODataQueueEntry *)((UInt8 *)dataQueue->queue + tail))->size = dataSize;
            }

            newTail = entrySize;
        }
        else
        {
            retVal = kIOReturnOverrun;  // queue is full
        }
    }
    else
    {
        // Do not allow the tail to catch up to the head when the queue is full.
        // That's why the comparison uses a '>' rather than '>='.

        if ( (head - tail) > entrySize )
        {
            entry = (IODataQueueEntry *)((UInt8 *)dataQueue->queue + tail);

            if ( data )
                memcpy(&(entry->data), data, dataSize);
            else if ( callback )
                (*callback)(refcon, &(entry->data), dataSize);

            entry->size = dataSize;
            
            newTail = tail + entrySize;
        }
        else
        {
            retVal = kIOReturnOverrun;  // queue is full
        }
    }
    
    // Send notification (via mach message) that data is available.    
    
    if ( retVal == kIOReturnSuccess ) {
        // Publish the data we just enqueued
        __c11_atomic_store((_Atomic UInt32 *)&dataQueue->tail, newTail, __ATOMIC_RELEASE);
        
        if (tail != head) {
            //
            // The memory barrier below pairs with the one in dequeue
            // so that either our store to the tail cannot be missed by
            // the next dequeue attempt, or we will observe the dequeuer
            // making the queue empty.
            //
            // Of course, if we already think the queue is empty,
            // there's no point paying this extra cost.
            //
            __c11_atomic_thread_fence(__ATOMIC_SEQ_CST);
            head = __c11_atomic_load((_Atomic UInt32 *)&dataQueue->head, __ATOMIC_RELAXED);
        }
        
        if (tail == head) {
            // Send notification (via mach message) that data is now available.
            retVal = _IODataQueueSendDataAvailableNotification(dataQueue, msgh);
        }
#if TARGET_OS_SIMULATOR
        else
        {
            retVal = _IODataQueueSendDataAvailableNotification(dataQueue, msgh);
        }
#endif
    }

    else if ( retVal == kIOReturnOverrun ) {
        // Send extra data available notification, this will fail and we will
        // get a send possible notification when the client starts responding
        (void) _IODataQueueSendDataAvailableNotification(dataQueue, msgh);
    }

    return retVal;
}

IOReturn
IODataQueueEnqueue(IODataQueueMemory *dataQueue, void *data, uint32_t dataSize)
{
    return __IODataQueueEnqueue(dataQueue, 0, NULL, dataSize, data, NULL, NULL);
}


IOReturn
_IODataQueueEnqueueWithReadCallback(IODataQueueMemory *dataQueue, uint64_t queueSize, mach_msg_header_t *msgh, uint32_t dataSize, IODataQueueClientEnqueueReadBytesCallback callback, void * refcon)
{
    return __IODataQueueEnqueue(dataQueue, queueSize, msgh, dataSize, NULL, callback, refcon);
}


IOReturn IODataQueueWaitForAvailableData(IODataQueueMemory *dataQueue, mach_port_t notifyPort)
{
    IOReturn kr;
    struct {
            mach_msg_header_t msgHdr;
//            OSNotificationHeader notifyHeader;
            mach_msg_trailer_t trailer;
    } msg;
    
    if (dataQueue && (notifyPort != MACH_PORT_NULL)) {
        kr = mach_msg(&msg.msgHdr, MACH_RCV_MSG, 0, sizeof(msg), notifyPort, 0, MACH_PORT_NULL);
    } else {
        kr = kIOReturnBadArgument;
    }

    return kr;
}

mach_port_t IODataQueueAllocateNotificationPort()
{
    mach_port_t        port = MACH_PORT_NULL;
    mach_port_limits_t    limits;
    mach_msg_type_number_t    info_cnt;
    kern_return_t             kr;

    kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
    if (kr != KERN_SUCCESS)
        return MACH_PORT_NULL;

    info_cnt = MACH_PORT_LIMITS_INFO_COUNT;

    kr = mach_port_get_attributes(mach_task_self(),
                                  port,
                                  MACH_PORT_LIMITS_INFO,
                                  (mach_port_info_t)&limits,
                                  &info_cnt);
    if (kr != KERN_SUCCESS) {
        mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
        return MACH_PORT_NULL;
    }

    limits.mpl_qlimit = 1;    // Set queue to only 1 message

    kr = mach_port_set_attributes(mach_task_self(),
                                  port,
                                  MACH_PORT_LIMITS_INFO,
                                  (mach_port_info_t)&limits,
                                  MACH_PORT_LIMITS_INFO_COUNT);
    if (kr != KERN_SUCCESS) {
        mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
        return MACH_PORT_NULL;
    }

    return port;
}

IOReturn IODataQueueSetNotificationPort(IODataQueueMemory *dataQueue, mach_port_t notifyPort)
{
    IODataQueueAppendix *   appendix    = NULL;
    UInt32                  queueSize   = 0;
            
    if ( !dataQueue )
        return kIOReturnBadArgument;
        
    queueSize = dataQueue->queueSize;
    
    appendix = (IODataQueueAppendix *)((UInt8 *)dataQueue + queueSize + DATA_QUEUE_MEMORY_HEADER_SIZE);

    appendix->msgh.msgh_bits        = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
    appendix->msgh.msgh_size        = sizeof(appendix->msgh);
    appendix->msgh.msgh_remote_port = notifyPort;
    appendix->msgh.msgh_local_port  = MACH_PORT_NULL;
    appendix->msgh.msgh_id          = 0;

    return kIOReturnSuccess;
}

static void
__IODataQueueConsumeUnsentMessage(mach_msg_header_t *hdr)
{
    mach_port_t port = hdr->msgh_local_port;
    if (MACH_PORT_VALID(port)) {
        switch (MACH_MSGH_BITS_LOCAL(hdr->msgh_bits)) {
            case MACH_MSG_TYPE_MOVE_SEND:
            case MACH_MSG_TYPE_MOVE_SEND_ONCE:
                mach_port_deallocate(mach_task_self(), port);
                break;
        }
    }
    mach_msg_destroy(hdr);
}

IOReturn _IODataQueueSendDataAvailableNotification(IODataQueueMemory *dataQueue, mach_msg_header_t *msgh)
{
    kern_return_t kr;
    mach_msg_header_t header;
    
    if (!msgh) {
        IODataQueueAppendix *appendix = NULL;
        
        appendix = (IODataQueueAppendix *)((UInt8 *)dataQueue + dataQueue->queueSize + DATA_QUEUE_MEMORY_HEADER_SIZE);
        
        if ( appendix->msgh.msgh_remote_port == MACH_PORT_NULL )
            return kIOReturnSuccess;  // return success if no port is declared
        
        header = appendix->msgh;
    } else {
        header = *msgh;
    }
    
    kr = mach_msg(&header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, header.msgh_size, 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
    switch(kr) {
        case MACH_SEND_INVALID_DEST:
        case MACH_SEND_TIMED_OUT:    // Notification already sent
            // Clean up pseudo-receive
            __IODataQueueConsumeUnsentMessage(&header);
            break;
        case MACH_MSG_SUCCESS:
            break;
        default:
            // perhaps add log here
            break;
    }
    
    return kr;
}
