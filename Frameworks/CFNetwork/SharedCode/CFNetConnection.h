/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
#if !defined(__CFNETWORK_CFCONNECTION__)
#define __CFNETWORK_CFCONNECTION__ 1

#include <CFNetwork/CFNetwork.h>
#include "CFNetworkInternal.h"
#include <CoreFoundation/CFStreamAbstract.h>
#include <CFNetwork/CFSocketStream.h>
#include <CoreFoundation/CFRuntime.h>
#include <sys/types.h>
#include <AvailabilityMacros.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  _CFNetConnectionRef
 *  
 *  Discussion:
 *    This is the type of a reference to a net connection. A
 *    CFNetConnection maintains a pair of streams on which a queue of
 *    requests are serviced.  A given net connection request represents
 *    a request and response pair.
 */
typedef struct __CFNetConnection*       _CFNetConnectionRef;

/*
 *  _CFNetConnectionState
 *  
 *  Discussion:
 *    Indicators of state for individual requests and responses on a
 *    CFNetConnection.
 */
enum _CFNetConnectionState {

  /*
   * Request or response is not in the queue
   */
  kNotQueued                    = 0,

  /*
   * Request or response is in the queue
   */
  kQueued                       = 1,

  /*
   * Request is in the process of sending
   */
  kTransmittingRequest          = 2,

  /*
   * Request has been sent but the response hasn't started to arrive
   */
  kWaitingForResponse           = 3,

  /*
   * Response is in the process of being received
   */
  kReceivingResponse            = 4,

  /*
   * Request has been transmitted and the response has been received. 
   * The request is now complete.
   */
  kFinished                     = 5,

  /*
   * Request has been cancelled and removed from the queue.
   */
  kCancelled                    = 6,

  /*
   * Something has happened with the connection such that the given
   * queued request has now been dropped from the queue.  The user
   * should now requeue the request on a new net connection.
   */
  kOrphaned                     = 7
};
typedef enum _CFNetConnectionState _CFNetConnectionState;


typedef CALLBACK_API_C( const void *, _CFNetConnectionCreateCallBack )(CFAllocatorRef alloc, const void *info);
typedef CALLBACK_API_C( void , _CFNetConnectionFinalizeCallBack )(CFAllocatorRef alloc, const void *info);
typedef CALLBACK_API_C( CFStreamError , _CFNetConnectionCreateStreamsCallBack )(CFAllocatorRef allocator, const void *info, CFWriteStreamRef *requestStream, CFReadStreamRef *responseStream);
typedef CALLBACK_API_C( void , _CFNetConnectionStateChangedCallBack )(void *request, int newState, CFStreamError *err, _CFNetConnectionRef connection, const void *info);
typedef CALLBACK_API_C( void , _CFNetConnectionTransmitRequest )(void *request, _CFNetConnectionRef connection, const void *info);
typedef CALLBACK_API_C( void , _CFNetConnectionReceiveResponse )(void *request, _CFNetConnectionRef connection, const void *info);
typedef CALLBACK_API_C( void , _CFNetConnectionResponseStreamCallBack )(void *request, CFReadStreamRef stream, CFStreamEventType eventType, _CFNetConnectionRef conn, const void *info);
typedef CALLBACK_API_C( void , _CFNetConnectionRequestStreamCallBack )(void *request, CFWriteStreamRef stream, CFStreamEventType eventType, _CFNetConnectionRef conn, const void *info);
typedef CALLBACK_API_C( CFArrayRef , _CFNetConnectionRunLoopArrayCallBack )(void *request, _CFNetConnectionRef conn, const void *info);
struct _CFNetConnectionCallBacks {
  CFIndex             version;
  _CFNetConnectionCreateCallBack  create;
  _CFNetConnectionFinalizeCallBack  finalize;
  _CFNetConnectionCreateStreamsCallBack  createConnectionStreams;
  _CFNetConnectionStateChangedCallBack  requestStateChanged;
  _CFNetConnectionTransmitRequest  transmitRequest;
  _CFNetConnectionReceiveResponse  receiveResponse;
  _CFNetConnectionResponseStreamCallBack  responseStreamCallBack;
  _CFNetConnectionRequestStreamCallBack  requestStreamCallBack;
  _CFNetConnectionRunLoopArrayCallBack  runLoopAndModesArrayForRequest;
};
typedef struct _CFNetConnectionCallBacks _CFNetConnectionCallBacks;
/* Net connection*/
/*
 *  _CFNetConnectionGetTypeID()
 *  
 */
extern CFTypeID 
_CFNetConnectionGetTypeID(void)                               AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  _CFNetConnectionGetInfoPointer()
 *  
 */
extern const void * 
_CFNetConnectionGetInfoPointer(_CFNetConnectionRef conn)      AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Create and return a new connection object*/
/*
 *  _CFNetConnectionCreate()
 *  Note - in 10.3, this function did not take the isThreadSafe argument; that was added for 10.4
 *  
 */
extern _CFNetConnectionRef 
_CFNetConnectionCreate(
  CFAllocatorRef                     alloc,
  const void *                       info,
  const _CFNetConnectionCallBacks *  callBacks,
  Boolean isThreadSafe)               AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/* Enqueues req on conn's transmission queue.  If this is the first such request, this will cause the connection to open streams to the server*/
/*
 *  _CFNetConnectionEnqueue()
 *  
 */
extern Boolean 
_CFNetConnectionEnqueue(
  _CFNetConnectionRef   conn,
  void *                req)                                  AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Cancel an enqueued request; this will cause the connection to in all ways "forget" the request.  Returns FALSE if the request is currently mid-transmission, in which case the connection cannot safely remove the request.*/
/*
 *  _CFNetConnectionDequeue()
 *  
 */
extern Boolean 
_CFNetConnectionDequeue(
  _CFNetConnectionRef   conn,
  void *                req)                                  AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/*
 *  _CFNetConnectionGetCurrentRequest()
 *  
 *  Discussion:
 *    Returns the head queued request or NULL if there is nothing
 *    queued.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    conn:
 *      The connection to query for the head request.
 *  
 *  Result:
 *    Returns a pointer to the head request if there is one, otherwise
 *    it returns NULL.
 *  
 */
extern void * 
_CFNetConnectionGetCurrentRequest(_CFNetConnectionRef conn)   AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/* Replaces oldReq with newReq in the connection's queue.*/
/*
 *  _CFNetConnectionReplaceRequest()
 *  
 */
extern void 
_CFNetConnectionReplaceRequest(
  _CFNetConnectionRef   conn,
  void *                oldReq,
  void *                newReq)                               AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Returns the response streams*/
/*
 *  _CFNetConnectionGetResponseStream()
 *  
 */
extern CFReadStreamRef 
_CFNetConnectionGetResponseStream(_CFNetConnectionRef conn)   AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Returns the request streams*/
/*
 *  _CFNetConnectionGetRequestStream()
 *  
 */
extern CFWriteStreamRef 
_CFNetConnectionGetRequestStream(_CFNetConnectionRef conn)    AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Informs the connection that the given request considers its response complete, and the connection should break its connection to the request and advance to the next response*/
/*
 *  _CFNetConnectionResponseIsComplete()
 *  
 */
extern void 
_CFNetConnectionResponseIsComplete(
  _CFNetConnectionRef   conn,
  void *                req)                                  AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Informs the connection that the given request considers its request complete (i.e. fully transmitted), and the connection should advance to the next request to be transmitted*/
/*
 *  _CFNetConnectionRequestIsComplete()
 *  
 */
extern void 
_CFNetConnectionRequestIsComplete(
  _CFNetConnectionRef   conn,
  void *                req)                                  AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Error out the connection with the given error.  All queued requests will be orphaned.*/
/*
 *  _CFNetConnectionErrorOccurred()
 *  
 */
extern void 
_CFNetConnectionErrorOccurred(
  _CFNetConnectionRef   conn,
  CFStreamError *       err)                                  AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Announces that the persistent connection has been lost; all responses after the current one must be orphaned*/
/*
 *  _CFNetConnectionLost()
 *  
 */
extern void 
_CFNetConnectionLost(_CFNetConnectionRef conn)                AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Set whether requests will be transmitted without waiting for previous responses*/
/*
 *  _CFNetConnectionSetShouldPipeline()
 *  
 */
extern void 
_CFNetConnectionSetShouldPipeline(
  _CFNetConnectionRef   conn,
  Boolean               shouldPipeline)                       AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;

extern CFAbsoluteTime 
_CFNetConnectionGetLastAccessTime(_CFNetConnectionRef arg);

extern int
_CFNetConnectionGetQueueDepth(_CFNetConnectionRef conn);

/*
 *  _CFNetConnectionSetAllowsNewRequests()
 *  
 */
extern void 
_CFNetConnectionSetAllowsNewRequests(
  _CFNetConnectionRef   conn,
  Boolean               allowRequests)                        AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  _CFNetConnectionWillEnqueueRequests()
 *  
 */
extern Boolean 
_CFNetConnectionWillEnqueueRequests(_CFNetConnectionRef conn) AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Gets the connection's current opinion about the request's state.  If advanceConnection is TRUE, calling this will cause the connection to attempt to further the state of its queue, and may cause calls back in to the request.  If the connection knows nothing about the request, it will return kOrphaned, and the calling request should forget any tie to this connection*/
/*
 *  _CFNetConnectionGetState()
 *  
 */
extern int 
_CFNetConnectionGetState(
  _CFNetConnectionRef   conn,
  Boolean               advanceConnection,
  void *                req)                                  AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Informs the connection that the given request has been scheduled and asks the connection to take any appropriate action*/
/*
 *  _CFNetConnectionSchedule()
 *  
 */
extern void 
_CFNetConnectionSchedule(
  _CFNetConnectionRef   conn,
  void *                req,
  CFRunLoopRef          rl,
  CFStringRef           mode)                                 AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Informs the connection that the given request has been unscheduled, and asks the connection to take any appropriate action*/
/*
 *  _CFNetConnectionUnschedule()
 *  
 */
extern void 
_CFNetConnectionUnschedule(
  _CFNetConnectionRef   conn,
  void *                req,
  CFRunLoopRef          rl,
  CFStringRef           mode)                                 AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/* Informs the connection that the given request has been unscheduled, and asks the connection to take any appropriate action*/
/*
 *  _CFNetConnectionIsEmpty()
 *  
 */
extern Boolean 
_CFNetConnectionIsEmpty(_CFNetConnectionRef conn)             AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

// Currently not used, so hand dead-stripping for now.
//extern Boolean
//_CFNetConnectionIsPipelining(_CFNetConnectionRef arg);


typedef struct __CFNetConnectionCache *		CFNetConnectionCacheRef;
typedef struct __CFNetConnectionCacheKey*	_CFNetConnectionCacheKey;


//
// Net connection cache
//
CFNetConnectionCacheRef createConnectionCache();
#if defined(__WIN32__)
void releaseConnectionCache(CFNetConnectionCacheRef cache);
#endif	/* defined(__WIN32__) */
void lockConnectionCache(CFNetConnectionCacheRef cache);
void unlockConnectionCache(CFNetConnectionCacheRef cache);
extern
_CFNetConnectionRef findOrCreateNetConnection(CFNetConnectionCacheRef connectionCache, CFAllocatorRef allocator, const _CFNetConnectionCallBacks *callbacks, const void *info, _CFNetConnectionCacheKey key, Boolean persistent, CFDictionaryRef connectionProperties);	// This routine ties the two objects (connection cache & connection)
extern
void removeFromConnectionCache(CFNetConnectionCacheRef cache, _CFNetConnectionRef conn, _CFNetConnectionCacheKey key);

// These two callbacks are shared across protocols
const void *connCacheKeyRetain(CFAllocatorRef allocator, const void *value);
void connCacheKeyRelease(CFAllocatorRef allocator,  const void *value);

//
// Net connection cache key
//
_CFNetConnectionCacheKey createConnectionCacheKey(CFStringRef host, SInt32 port, UInt32 connType, CFDictionaryRef properties);
void releaseConnectionCacheKey(_CFNetConnectionCacheKey theKey);
void getValuesFromKey(const _CFNetConnectionCacheKey theKey, CFStringRef *host, SInt32 *port, UInt32 *connType, CFDictionaryRef *properties);

#ifdef DEBUG
void printKey(_CFNetConnectionCacheKey key);
#endif	/* DEBUG */

#if defined(__cplusplus)
}
#endif

#endif /* ! __CFNETWORK_CFNETCONNECTION__ */

