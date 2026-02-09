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
/*
     File:       CFNetwork/CFHTTPConnectionPriv.h
 
     Contains:   CoreFoundation Network HTTP connection SPI
 
     Copyright:  © 2004-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file contains unreleased SPI's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFHTTPConnectionPriv.i
                     Revision:        1.6
                     Dated:           2004/06/08 00:02:25
                     Last change by:  rew
                     Last comment:    Add symbol to allow CFHTTPConnection to handle streamed uploads
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFHTTPCONNECTIONPRIV__
#define __CFHTTPCONNECTIONPRIV__

#ifndef __CFNETWORKDEFS__
#include <CFNetwork/CFNetworkDefs.h>
#endif

#ifndef __COREFOUNDATION__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifndef __CFNETWORK__
#include <CFNetwork/CFNetwork.h>
#endif



#include <AvailabilityMacros.h>

#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint on
#endif


/*
 *  _CFHTTPConnectionType
 *  
 *  Discussion:
 *    The different kinds of connections that can be passed in to
 *    CFHTTPConnectionCreate()
 */
enum _CFHTTPConnectionType {

  /*
   * A direct HTTP connection to the server
   */
  kHTTP                         = 0,

  /*
   * A direct HTTPS connection to the server
   */
  kHTTPS                        = 1,

  /*
   * A connection to an HTTP proxy
   */
  kHTTPProxy                    = 2,

  /*
   * A connection to an HTTPS proxy
   */
  kHTTPSProxy                   = 3,
  kSOCKSProxy                   = 4
};
typedef enum _CFHTTPConnectionType _CFHTTPConnectionType;



/*
 *  CFHTTPConnectionRef
 *  
 *  Discussion:
 *    A connection to a particular host over which HTTP requests may be
 *    enqueued
 */
typedef CFTypeRef                       CFHTTPConnectionRef;
/*
 *  kCFStreamErrorHTTPConnectionLost
 *  
 *  Discussion:
 *    The error in the domain kCFStreamErrorDomainHTTP returned when an
 *    upstream request has detected that the connection is dying (often
 *    because of a request limit from the server). If you receive this
 *    error, you may assume that there is nothing wrong with your
 *    request itself, just that the connection did not survive long
 *    enough to process your request.
 *  
 */
extern const SInt32 kCFStreamErrorHTTPConnectionLost                 AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
/*
 *  _kCFStreamPropertyHTTPConnection
 *  
 *  Discussion:
 *    Retrievable property on streams returned by
 *    CFHTTPConnectionEnqueue, below.  Returns the CFHTTPConnectionRef
 *    on which the stream is scheduled.
 *  
 */
extern const CFStringRef _kCFStreamPropertyHTTPConnection            AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
/*
 *  CFHTTPConnectionCreate()
 *  
 *  Discussion:
 *    Creates a new HTTP connection.
 *  
 *  Parameters:
 *    
 *    alloc:
 *      The CFAllocator to be used to create the new connection
 *    
 *    host:
 *      The target host for the new connection
 *    
 *    port:
 *      The target port for the new connection
 *    
 *    connectionType:
 *      The type of connection desired; taken from
 *      _CFHTTPConnectionType above.
 *    
 *    streamProperties:
 *      Any additional properties to be set on the underlying socket
 *      stream(s) before the connection is first used
 *  
 *  Result:
 *    Returns the newly created CFHTTPConnection, or NULL if
 *    unsuccessful
 *  
 */
extern CFHTTPConnectionRef 
CFHTTPConnectionCreate(
  CFAllocatorRef    alloc,
  CFStringRef       host,
  SInt32            port,
  UInt32            connectionType,
  CFDictionaryRef   streamProperties)                         AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/*
 *  CFHTTPConnectionEnqueue()
 *  
 *  Discussion:
 *    Enqueues the given HTTP request on the given connection.
 *  
 *  Parameters:
 *    
 *    connection:
 *      The connection you wish to send the request over
 *    
 *    request:
 *      The request you wish to send
 *  
 *  Result:
 *    A CFReadStream with which you can monitor the request's progress
 *    through the connection. Note that the request will not actually
 *    be scheduled on the connection until the returned stream is
 *    opened.
 *  
 */
extern CFReadStreamRef 
CFHTTPConnectionEnqueue(
  CFHTTPConnectionRef   connection,
  CFHTTPMessageRef      request)                              AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/*
 *  CFHTTPConnectionEnqueueWithBodyStream()
 *  
 *  Discussion:
 *    Enqueues the given HTTP request on the given connection.
 *  
 *  Parameters:
 *    
 *    connection:
 *      The connection you wish to send the request over
 *    
 *    request:
 *      The request you wish to send
 *    
 *    bodyStream:
 *      An unopened read stream which will return the body bytes to be
 *      transmitted with request
 *  
 *  Result:
 *    A CFReadStream with which you can monitor the request's progress
 *    through the connection. Note that the request will not actually
 *    be scheduled on the connection until the returned stream is
 *    opened.
 *  
 */
extern CFReadStreamRef 
CFHTTPConnectionEnqueueWithBodyStream(
  CFHTTPConnectionRef   connection,
  CFHTTPMessageRef      request,
  CFReadStreamRef       bodyStream)                           AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/*
 *  CFHTTPConnectionSetShouldPipeline()
 *  
 *  Discussion:
 *    Sets whether the given connection will pipeline outgoing
 *    requests, transmitting the requests in sequence without waiting
 *    for the full responses to earlier requests.
 *  
 *  Parameters:
 *    
 *    connection:
 *      The connection to be configured
 *    
 *    shouldPipeline:
 *      TRUE if you wish the connection to pipeline outgoing requests;
 *      FALSE if you wish the connection to wait for one request's
 *      response before sending the next request
 *  
 */
extern void 
CFHTTPConnectionSetShouldPipeline(
  CFHTTPConnectionRef   connection,
  Boolean               shouldPipeline)                       AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/*
 *  CFHTTPConnectionLost()
 *  
 *  Discussion:
 *    Informs the connection that after the current response, no
 *    further request can be processed. Causes the connection to error
 *    out all further requests on the connection, returning an error
 *    code of {kCFStreamErrorDomainHTTP,
 *    kCFStreamErrorHTTPConnectionLost}
 *  
 *  Parameters:
 *    
 *    conn:
 *      The connection that has been lost
 *  
 */
extern void 
CFHTTPConnectionLost(CFHTTPConnectionRef conn)                AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/*
 *  CFHTTPConnectionInvalidate()
 *  
 *  Discussion:
 *    Invalidates the given connection, stopping all traffic on that
 *    connection and causing all requests in progress to return the
 *    error given.  This also closes the underlying system resources to
 *    be release, and the connection to the remote host to be severed.
 *  
 *  Parameters:
 *    
 *    connection:
 *      The connection to be shut down
 *    
 *    error:
 *      The error to be returned by all active requests on the
 *      connection being shut down
 *  
 */
extern void 
CFHTTPConnectionInvalidate(
  CFHTTPConnectionRef   connection,
  CFStreamError *       error)                                AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/*
 *  CFHTTPConnectionAcceptsRequests()
 *  
 *  Discussion:
 *    Returns whether the given connection would current accept new
 *    requests.  It would not accept new requests if the connection has
 *    been invalidated, or if the connection has detected an underlying
 *    networking error, rendering its network connection to the remote
 *    host unusable.
 *  
 *  Parameters:
 *    
 *    connection:
 *      The connection to query about its status
 *  
 *  Result:
 *    TRUE if the connection currently believes it can process further
 *    requests; FALSE otherwise. Note that a TRUE return value does not
 *    guarantee that future requests will be accepted - the connection
 *    could become invalid between this call and accepting the new
 *    request, or between accepting the new request and the resulting
 *    stream being opened - but a FALSE return can be used as an early
 *    indicator of a bad connection.
 *  
 */
extern Boolean 
CFHTTPConnectionAcceptsRequests(CFHTTPConnectionRef connection) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/*
 *  CFHTTPConnectionGetLastAccessTime()
 *  
 *  Discussion:
 *    Returns the last time the connection had an active request in it
 *  
 *  Parameters:
 *    
 *    connection:
 *      The connection to query about its last access tiem
 *  
 *  Result:
 *    The last time the connection had an active request
 *  
 */
extern CFAbsoluteTime 
CFHTTPConnectionGetLastAccessTime(CFHTTPConnectionRef connection) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  CFHTTPConnectionGetQueueDepth()
 *  
 *  Discussion:
 *    Returns the current depth of the request queue, counting from the
 *    request currently receiving its response
 *  
 *  Parameters:
 *    
 *    conn:
 *      The connection to query about its queue
 *  
 *  Result:
 *    The current depth of the queue
 *  
 */
extern int 
CFHTTPConnectionGetQueueDepth(CFHTTPConnectionRef conn)       AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  _CFHTTPGetConnectionInfoForProxyURL()
 *  
 */
extern void 
_CFHTTPGetConnectionInfoForProxyURL(
  CFURLRef           proxyURL,
  CFHTTPMessageRef   request,
  CFStringRef *      host,
  SInt32 *           port,
  UInt32 *           theType,
  CFDictionaryRef *  streamProperties)                        AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;




/*
 *  _CFProxyStreamCallBack
 *  
 *  Discussion:
 *    Callback function which is called once an asynchronous proxy
 *    lookup completes
 *  
 *  Parameters:
 *    
 *    proxyStream:
 *      The proxy stream returned when the asynchronous lookup began
 *    
 *    clientInfo:
 *      The opaque client context passed in to
 *      _CFNetworkFindProxyForURLAsync, below
 */
typedef CALLBACK_API_C( void , _CFProxyStreamCallBack )(CFReadStreamRef proxyStream, void *clientInfo);
/*
 *  _CFNetworkFindProxyForURLAsync()
 *  
 *  Discussion:
 *    Given a url, host name, and proxy dictionary (as returned by SC)
 *    produce the proxy list of attempts for establishing connnections.
 *  
 *  Parameters:
 *    
 *    scheme:
 *      The target or intended scheme which the callee thinks applies
 *      to the given url.  This value if set trumps the scheme on the
 *      url.
 *    
 *    url:
 *      A CFURLRef representing the object to be obtained.  If NULL,
 *      host must be specified.
 *    
 *    host:
 *      A CFStringRef representing the far server which will receive
 *      the connection.  If NULL, url must be specified in which case
 *      the host field from the URL will be used.
 *    
 *    proxies:
 *      A CFDictionaryRef contatining all proxy information to be
 *      considered.  All proxy information will be consulted and the
 *      best fit for the given url and host will be returned.
 *    
 *    cb:
 *      A callback function to be called once an answer to "which
 *      proxy?" is available.  If NULL, this function will block until
 *      an answer is available.
 *    
 *    clientInfo:
 *      A pointer of the caller's choosing that will be passed back
 *      when cb is invoked.  It is the caller's responsibility to
 *      ensure that the clientInfo pointer remains good until either cb
 *      is invoked or proxyStream is closed.
 *    
 *    proxyStream:
 *      The stream returned when async operation is required.  Must not
 *      be NULL if cb is non-NULL.  The caller should schedule the
 *      returned stream on whichever run loop it wishes to receive cb. 
 *      The caller must not set the client of proxyStream.
 *  
 *  Result:
 *    A CFMutableArrayRef containing the list of proxies to be
 *    attempted in respective order.  Individual proxy items are
 *    CFURLRef's where the scheme indicates the proxy type (i.e. http,
 *    https, socks4, etc.), the host indicates the proxy host name, the
 *    port indicates the proxy port, the username is the user's
 *    username on the proxy (if needed), and the password is the user's
 *    password for accessing the proxy (if needed). The username and
 *    password would primarily be needed for SOCKS.  A kCFNull in the
 *    list indicates that a direct connection is to be used and proxies
 *    should be bypassed for that attempt.  If all entries in the list
 *    are attempted, the connection has failed.  If NULL is returned,
 *    asynchronous operation has begun, and the caller should wait for
 *    their callback to be invoked, or poll via
 *    _CFNetworkCopyProxyFromProxyStream, below.
 *  
 */
extern CFMutableArrayRef 
_CFNetworkFindProxyForURLAsync(
  CFStringRef              scheme,
  CFURLRef                 url,
  CFStringRef              host,
  CFDictionaryRef          proxies,
  _CFProxyStreamCallBack   cb,
  void *                   clientInfo,
  CFReadStreamRef *        proxyStream)                       AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/*
 *  _CFNetworkCopyProxyFromProxyStream()
 *  
 *  Discussion:
 *    Once an asynchronous search for the correct proxy has begun (via
 *    _CFNetworkFindProxyForURLAsync, above), call this function to
 *    discover whether the correct proxy has been found, and if so,
 *    what that proxy is.
 *  
 *  Parameters:
 *    
 *    proxyStream:
 *      The proxy stream returned by _CFNetworkFindProxyForURLAsync,
 *      above
 *    
 *    isComplete:
 *      This out parameter is set to true if the asynchronous search is
 *      now complete, or false otherwise.
 *  
 *  Result:
 *    A CFMutableArrayRef giving the discovered proxies, in the same
 *    format as returned by _CFNetworkFindProxyForURLAsync, above.
 *  
 */
extern CFMutableArrayRef 
_CFNetworkCopyProxyFromProxyStream(
  CFReadStreamRef   proxyStream,
  Boolean *         isComplete)                               AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CFHTTPCONNECTIONPRIV__ */

