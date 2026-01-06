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
 *  IOMIGMachPort.c
 *
 *  Created by Rob Yepez on 1/29/09.
 *  Copyright 2008 Apple Inc.. All rights reserved.
 *
 */

#include <AssertMacros.h>
#include <CoreFoundation/CFRuntime.h>

#include <dispatch/private.h>
#include <pthread.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <servers/bootstrap.h>
#include "IOMIGMachPort.h"
#include <os/log.h>

typedef struct __IOMIGMachPort {
    CFRuntimeBase                       cfBase;   // base CFType information
  
    CFRunLoopRef                        runLoop;
    CFStringRef                         runLoopMode;
    
    dispatch_queue_t                    dispatchQueue;
    dispatch_mach_t                     dispatchChannel;
    
    CFMachPortRef                       port;
    CFRunLoopSourceRef                  source;
    CFIndex                             maxMessageSize;
    
    IOMIGMachPortDemuxCallback          demuxCallback;
    void *                              demuxRefcon;
    
    IOMIGMachPortTerminationCallback    terminationCallback;
    void *                              terminationRefcon;
} __IOMIGMachPort, *__IOMIGMachPortRef;


static void             __IOMIGMachPortRelease(CFTypeRef object);
static void             __IOMIGMachPortRegister(void);
static void             __IOMIGMachPortPortCallback(CFMachPortRef port, void *msg, CFIndex size __unused, void *info);
static Boolean          __NoMoreSenders(mach_msg_header_t *request, mach_msg_header_t *reply);
static os_log_t         __IOMIGMachPortLog();


static const CFRuntimeClass __IOMIGMachPortClass = {
    0,                          // version
    "IOMIGMachPort",          // className
    NULL,                       // init
    NULL,                       // copy
    __IOMIGMachPortRelease,   // finalize
    NULL,                       // equal
    NULL,                       // hash
    NULL,                       // copyFormattingDesc
    NULL,
    NULL,
    NULL
};

static pthread_once_t           __IOMIGMachPortTypeInit   = PTHREAD_ONCE_INIT;
static CFTypeID                 __IOMIGMachPortTypeID     = _kCFRuntimeNotATypeID;
static CFMutableDictionaryRef   __ioPortCache             = NULL;
static pthread_mutex_t          __ioPortCacheLock         = PTHREAD_MUTEX_INITIALIZER;

//------------------------------------------------------------------------------
// __IOMIGMachPortRegister
//------------------------------------------------------------------------------
void __IOMIGMachPortRegister(void)
{
    __ioPortCache         = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, NULL);
    __IOMIGMachPortTypeID = _CFRuntimeRegisterClass(&__IOMIGMachPortClass);
}

//------------------------------------------------------------------------------
// __IOMIGMachPortRelease
//------------------------------------------------------------------------------
void __IOMIGMachPortRelease(CFTypeRef object)
{
    IOMIGMachPortRef migPort = (IOMIGMachPortRef)object;
    
    if ( migPort->port ) {
        CFMachPortInvalidate(migPort->port);
        kern_return_t kr =  mach_port_mod_refs(mach_task_self(),
                                               CFMachPortGetPort(migPort->port),
                                               MACH_PORT_RIGHT_RECEIVE,
                                               -1);
        if (kr != KERN_SUCCESS) {
            os_log_error(__IOMIGMachPortLog(), "__IOMIGMachPortRelease mach_port_mod_refs:%s", mach_error_string(kr));
        }

        CFRelease(migPort->port);
    }
    
    if ( migPort->source ) {
        CFRelease(migPort->source);
    }
    
    if ( migPort->dispatchChannel ) {
        dispatch_mach_cancel(migPort->dispatchChannel);
        dispatch_release(migPort->dispatchChannel);
        migPort->dispatchChannel = NULL;
    }
}

//------------------------------------------------------------------------------
// IOMIGMachPortGetTypeID
//------------------------------------------------------------------------------
CFTypeID IOMIGMachPortGetTypeID(void)
{
    if ( _kCFRuntimeNotATypeID == __IOMIGMachPortTypeID )
        pthread_once(&__IOMIGMachPortTypeInit, __IOMIGMachPortRegister);
    
    return __IOMIGMachPortTypeID;
}

//------------------------------------------------------------------------------
// IOMIGMachPortCreate
//------------------------------------------------------------------------------
IOMIGMachPortRef IOMIGMachPortCreate(CFAllocatorRef allocator, CFIndex maxMessageSize, mach_port_t port)
{
    IOMIGMachPortRef    migPort  = NULL;
    void *              offset  = NULL;
    uint32_t            size;
    
    require(maxMessageSize > 0, exit);
    
    /* allocate service */
    size    = sizeof(__IOMIGMachPort) - sizeof(CFRuntimeBase);
    migPort  = ( IOMIGMachPortRef)_CFRuntimeCreateInstance(allocator, IOMIGMachPortGetTypeID(), size, NULL);
    
    require(migPort, exit);

    offset = migPort;
    bzero(offset + sizeof(CFRuntimeBase), size);
        
    CFMachPortContext context = {0, migPort, NULL, NULL, NULL};
    migPort->port = (port != MACH_PORT_NULL) ? 
            CFMachPortCreateWithPort(allocator, port, __IOMIGMachPortPortCallback, &context, NULL) :
            CFMachPortCreate(allocator, __IOMIGMachPortPortCallback, &context, NULL);
    
    require(migPort->port, exit);

    // The following code was previously active. It's redundant, and in fact, will always result in error.
    // Receive rights can only ever have user ref count of 1. Attempting to modify the ref count of
    // a receive right by a delta other than 0 or -1 will result in KERN_INVALID_VALUE from ipc_right_delta().
    /*
    mach_port_mod_refs(mach_task_self(),
                       CFMachPortGetPort(migPort->port),
                       MACH_PORT_RIGHT_RECEIVE,
                       1);
     */
    
    migPort->maxMessageSize = maxMessageSize;
    
    return migPort;

exit:
    if ( migPort )
        CFRelease(migPort);
        
    return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOMIGMachPortChannelCallback
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void
__IOMIGMachPortChannelCallback(void *info, dispatch_mach_reason_t reason,
        dispatch_mach_msg_t msg, __unused mach_error_t error)
{
    IOMIGMachPortRef   migPort = (IOMIGMachPortRef)info;
    size_t             size;
    mach_msg_header_t *hdr;

    switch (reason) {
        case DISPATCH_MACH_MESSAGE_RECEIVED:
            hdr = dispatch_mach_msg_get_msg(msg, &size);
            __IOMIGMachPortPortCallback(migPort->port, hdr, size, migPort);
            break;

        case DISPATCH_MACH_CANCELED:
            CFRelease(migPort);
            break;

        default:
            break;
    }
}


//------------------------------------------------------------------------------
// IOMIGMachPortScheduleWithDispatchQueue
//------------------------------------------------------------------------------
void IOMIGMachPortScheduleWithDispatchQueue(IOMIGMachPortRef migPort, dispatch_queue_t queue)
{
    
    mach_port_t port = CFMachPortGetPort(migPort->port);
    
    migPort->dispatchQueue = queue;
    
    require(migPort->dispatchQueue, exit);
    
    // init the sources
    if ( !migPort->dispatchChannel ) {
        dispatch_mach_t dm = dispatch_mach_create_f(dispatch_queue_get_label(queue),
                queue, migPort, __IOMIGMachPortChannelCallback);
        require(dm, exit);
        
        migPort->dispatchChannel = dm;
        CFRetain(migPort);
        dispatch_mach_connect(dm, port, MACH_PORT_NULL, NULL);
    }

exit:
    return;
}

//------------------------------------------------------------------------------
// IOMIGMachPortUnscheduleFromDispatchQueue
//------------------------------------------------------------------------------
void IOMIGMachPortUnscheduleFromDispatchQueue(IOMIGMachPortRef migPort, dispatch_queue_t queue)
{
    if ( !queue || !migPort->dispatchQueue)
        return;
    
    if ( queue != migPort->dispatchQueue )
        return;
    
    migPort->dispatchQueue = NULL;

    if ( migPort->dispatchChannel ) {
        dispatch_mach_cancel(migPort->dispatchChannel);
        dispatch_release(migPort->dispatchChannel);
        migPort->dispatchChannel = NULL;
    }
}

//------------------------------------------------------------------------------
// IOMIGMachPortScheduleWithRunLoop
//------------------------------------------------------------------------------
void IOMIGMachPortScheduleWithRunLoop(IOMIGMachPortRef migPort, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{    
    migPort->runLoop        = runLoop;
    migPort->runLoopMode    = runLoopMode;
    
    require(migPort->runLoop, exit);
    require(migPort->runLoopMode, exit);

    // init the sources
    if ( !migPort->source ) {
        migPort->source = CFMachPortCreateRunLoopSource(CFGetAllocator(migPort), migPort->port, 1);
        require(migPort->source, exit);
    }

    CFRunLoopAddSource(runLoop, migPort->source, runLoopMode);
        
exit:
    return;
}

//------------------------------------------------------------------------------
// IOMIGMachPortUnscheduleFromRunLoop
//------------------------------------------------------------------------------
void IOMIGMachPortUnscheduleFromRunLoop(IOMIGMachPortRef migPort, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
    if ( !runLoop || !runLoopMode || !migPort->runLoop || !migPort->runLoopMode)
        return;
        
    if ( !CFEqual(runLoop, migPort->runLoop) || !CFEqual(runLoopMode, migPort->runLoopMode) )
        return;
        
    migPort->runLoop     = NULL;
    migPort->runLoopMode = NULL;
    
    if ( migPort->source )
        CFRunLoopRemoveSource(runLoop, migPort->source, runLoopMode);
}

//------------------------------------------------------------------------------
// IOMIGMachPortGetPort
//------------------------------------------------------------------------------
mach_port_t IOMIGMachPortGetPort(IOMIGMachPortRef migPort)
{
    return CFMachPortGetPort(migPort->port);
}

//------------------------------------------------------------------------------
// IOMIGMachPortRegisterDemuxCallback
//------------------------------------------------------------------------------
void IOMIGMachPortRegisterDemuxCallback(IOMIGMachPortRef migPort, IOMIGMachPortDemuxCallback callback, void *refcon)
{
    migPort->demuxCallback = callback;
    migPort->demuxRefcon   = refcon;
}

//------------------------------------------------------------------------------
// IOMIGMachPortRegisterTerminationCallback
//------------------------------------------------------------------------------
void IOMIGMachPortRegisterTerminationCallback(IOMIGMachPortRef migPort, IOMIGMachPortTerminationCallback callback, void *refcon)
{
    migPort->terminationCallback = callback;
    migPort->terminationRefcon   = refcon;
}

static void
__IOMIGMachConsumeUnsentMessage(mach_msg_header_t *hdr)
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

//------------------------------------------------------------------------------
// __IOMIGMachPortPortCallback
//------------------------------------------------------------------------------
void __IOMIGMachPortPortCallback(CFMachPortRef port __unused, void *msg, CFIndex size __unused, void *info)
{
    IOMIGMachPortRef  migPort      = (IOMIGMachPortRef)info;
    mig_reply_error_t * bufRequest  = msg;
    mig_reply_error_t * bufReply    = NULL;
    mach_msg_return_t   mr;
    int                 options;

    require(migPort, exit);
    CFRetain(migPort);
    
    bufReply = CFAllocatorAllocate(NULL, migPort->maxMessageSize, 0);
    require(bufReply, exit);
    
    // let's see if we have no more senders
    if ( __NoMoreSenders(&bufRequest->Head, &bufReply->Head) ) {
        if ( migPort->terminationCallback )
            (*migPort->terminationCallback)(migPort, migPort->terminationRefcon);
        else {
            goto exit;
        }
    } else {
        if ( migPort->demuxCallback )
            (*migPort->demuxCallback)(migPort, &bufRequest->Head, &bufReply->Head, migPort->demuxRefcon);
        else {
            mach_msg_destroy(&bufRequest->Head);
            goto exit;
        }
    }

    if (!(bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) &&
        (bufReply->RetCode != KERN_SUCCESS)) {        
        
        //This return code is a little tricky -- it appears that the
        //demux routine found an error of some sort, but since that
        //error would not normally get returned either to the local
        //user or the remote one, we pretend it's ok.
        require(bufReply->RetCode != MIG_NO_REPLY, exit);

        // destroy any out-of-line data in the request buffer but don't destroy
        // the reply port right (since we need that to send an error message).

        bufRequest->Head.msgh_remote_port = MACH_PORT_NULL;
        mach_msg_destroy(&bufRequest->Head);
    }

    if (bufReply->Head.msgh_remote_port == MACH_PORT_NULL) {
        //no reply port, so destroy the reply
        if (bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) {
            mach_msg_destroy(&bufReply->Head);
        }
        
        goto exit;
    }

    // send reply.
    // We don't want to block indefinitely because the migPort
    // isn't receiving messages from the reply port.
    // If we have a send-once right for the reply port, then
    // this isn't a concern because the send won't block.
    // If we have a send right, we need to use MACH_SEND_TIMEOUT.
    // To avoid falling off the kernel's fast RPC path unnecessarily,
    // we only supply MACH_SEND_TIMEOUT when absolutely necessary.

    options = MACH_SEND_MSG;
    if (MACH_MSGH_BITS_REMOTE(bufReply->Head.msgh_bits) != MACH_MSG_TYPE_MOVE_SEND_ONCE) {
       options |= MACH_SEND_TIMEOUT;
    }

    mr = mach_msg(&bufReply->Head,
              options,
              bufReply->Head.msgh_size,
              0,
              MACH_PORT_NULL,
              MACH_MSG_TIMEOUT_NONE,
              MACH_PORT_NULL);


    // Has a message error occurred?
    switch (mr) {
        case MACH_SEND_INVALID_DEST:
        case MACH_SEND_TIMED_OUT:
            __IOMIGMachConsumeUnsentMessage(&bufReply->Head);
            break;

        default :
            // Includes success case.
            break;
    }
    
exit:
    if ( bufReply )
        CFAllocatorDeallocate(NULL, bufReply);
        
    if ( migPort )
        CFRelease(migPort);
}

//------------------------------------------------------------------------------
// __NoMoreSenders
//------------------------------------------------------------------------------
Boolean __NoMoreSenders(mach_msg_header_t *request, mach_msg_header_t *reply)
{
	mach_no_senders_notification_t	*Request = (mach_no_senders_notification_t *)request;
	mig_reply_error_t               *Reply   = (mig_reply_error_t *)reply;

	reply->msgh_bits        = MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(request->msgh_bits), 0);
	reply->msgh_remote_port = request->msgh_remote_port;
	reply->msgh_size        = sizeof(mig_reply_error_t);	// Minimal size: update as needed
	reply->msgh_local_port  = MACH_PORT_NULL;
	reply->msgh_id          = request->msgh_id + 100;

	if ((Request->not_header.msgh_id > MACH_NOTIFY_LAST) ||
	    (Request->not_header.msgh_id < MACH_NOTIFY_FIRST)) {
		Reply->NDR     = NDR_record;
		Reply->RetCode = MIG_BAD_ID;
		return FALSE;	// if this is not a notification message 
	}

	switch (Request->not_header.msgh_id) {
		case MACH_NOTIFY_NO_SENDERS :
			Reply->Head.msgh_bits		= 0;
			Reply->Head.msgh_remote_port	= MACH_PORT_NULL;
			Reply->RetCode			= KERN_SUCCESS;
			return TRUE;
		default :
			break;
	}

	Reply->NDR     = NDR_record;
	Reply->RetCode = MIG_BAD_ID;
	return FALSE;	// if this is not a notification we are handling
}

//------------------------------------------------------------------------------
//IOMIGMachPortCacheAdd
//------------------------------------------------------------------------------
void IOMIGMachPortCacheAdd(mach_port_t port, CFTypeRef server)
{
    pthread_mutex_lock(&__ioPortCacheLock);

    CFDictionarySetValue(__ioPortCache, (void *)(uintptr_t)port, server);
    
    pthread_mutex_unlock(&__ioPortCacheLock);
}

//------------------------------------------------------------------------------
// IOMIGMachPortCacheRemove
//------------------------------------------------------------------------------
void IOMIGMachPortCacheRemove(mach_port_t port)
{
    pthread_mutex_lock(&__ioPortCacheLock);
    
    CFDictionaryRemoveValue(__ioPortCache, (void *)(uintptr_t)port);
    
    pthread_mutex_unlock(&__ioPortCacheLock);
}

//------------------------------------------------------------------------------
// IOMIGMachPortCacheCopy
//------------------------------------------------------------------------------
CFTypeRef IOMIGMachPortCacheCopy(mach_port_t port)
{
    CFTypeRef server;
    
    pthread_mutex_lock(&__ioPortCacheLock);
    
    server = (CFTypeRef)CFDictionaryGetValue(__ioPortCache, (void *)(uintptr_t)port);
    if ( server ) CFRetain(server);
    
    pthread_mutex_unlock(&__ioPortCacheLock);
    
    return server;
}

//------------------------------------------------------------------------------
// __IOMIGMachPortLog
//------------------------------------------------------------------------------
os_log_t __IOMIGMachPortLog()
{
    static os_log_t log;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{

        log = os_log_create("com.apple.iokit.iomigmachport", "default");

    });
    return log;
}

