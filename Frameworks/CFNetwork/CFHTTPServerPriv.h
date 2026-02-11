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
     File:       CFNetwork/CFHTTPServerPriv.h
 
     Contains:   CoreFoundation Network HTTP server SPI
 
     Copyright:  © 2003-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file contains unreleased SPI's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFHTTPServerPriv.i
                     Revision:        1.7
                     Dated:           2004/06/01 17:53:05
                     Last change by:  rew
                     Last comment:    Updating all copyrights to include 2004
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFHTTPSERVERPRIV__
#define __CFHTTPSERVERPRIV__

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

#pragma options align=mac68k

#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint on
#endif


/*
 *  _CFHTTPServerError
 *  
 *  Discussion:
 *    HTTP server's error domain and its errors.
 */
enum _CFHTTPServerError {

  /*
   * CFStreamError domain for HTTP server errors.
   */
  kCFStreamErrorDomainCFHTTPServer = 20,

  /*
   * Critical failure and server should be shutdown and destroyed.
   */
  kCFStreamErrorCFHTTPServerInternal = 1,

  /*
   * A timeout occurred on the given request (and response).
   */
  kCFStreamErrorCFHTTPServerTimeout = 2
};
typedef enum _CFHTTPServerError _CFHTTPServerError;



/*
 *  _CFHTTPServerRef
 *  
 *  Discussion:
 *    This is the type of a reference to a HTTP server.  Although
 *    individual functions are thread-safe, _CFHTTPServerRef itself is
 *    not thread-safe.
 */
typedef struct __CFHTTPServer*          _CFHTTPServerRef;

/*
 *  _CFHTTPServerContext
 *  
 *  Discussion:
 *    Structure containing the user-defined data and callbacks for
 *    _CFHTTPServerRef objects.
 */
struct _CFHTTPServerContext {

  /*
   * The version number of the structure type being passed in as a
   * parameter to the CFHTTPServer creation function. Valid version
   * number is currently 0.
   */
  CFIndex             version;

  /*
   * An arbitrary pointer to client-defined data, which can be
   * associated with the host and is passed to the callbacks.
   */
  void *              info;

  /*
   * The callback used to add a retain for the host on the info pointer
   * for the life of the host, and may be used for temporary references
   * the host needs to take. This callback returns the actual info
   * pointer to store in the host, almost always just the pointer
   * passed as the parameter.
   */
  CFAllocatorRetainCallBack  retain;

  /*
   * The callback used to remove a retain previously added for the host
   * on the info pointer.
   */
  CFAllocatorReleaseCallBack  release;

  /*
   * The callback used to create a descriptive string representation of
   * the info pointer (or the data pointed to by the info pointer) for
   * debugging purposes. This is used by the CFCopyDescription()
   * function.
   */
  CFAllocatorCopyDescriptionCallBack  copyDescription;
};
typedef struct _CFHTTPServerContext     _CFHTTPServerContext;

/*
 *  _CFHTTPServerAcceptNewConnectionCallBack
 *  
 *  Discussion:
 *    Callback which is invoked as a new connection is accepted.
 *  
 *  Parameters:
 *    
 *    server:
 *      The instance of the server which is receiving the connection.
 *    
 *    peer:
 *      A CFDataRef which contains a struct sockaddr holding the peer's
 *      network address.
 *    
 *    info:
 *      The reference from the server context which was given when the
 *      server was created.
 *  
 *  Result:
 *    A boolean value of TRUE if the server should accept the
 *    connection.  FALSE should be returned if the server should deny
 *    the connection.
 */
typedef CALLBACK_API_C( Boolean , _CFHTTPServerAcceptNewConnectionCallBack )(_CFHTTPServerRef server, CFDataRef peer, void *info);

/*
 *  _CFHTTPServerAcceptNewRequestCallBack
 *  
 *  Discussion:
 *    Callback which is invoked as a request begins to arrive.  This
 *    callback will be called after the headers have arrived but before
 *    the body.
 *  
 *  Parameters:
 *    
 *    server:
 *      The instance of the server which is receiving the request.
 *    
 *    headers:
 *      The headers which have been received.
 *    
 *    peer:
 *      A CFDataRef which contains a struct sockaddr holding the peer's
 *      network address.
 *    
 *    info:
 *      The reference from the server context which was given when the
 *      server was created.
 *  
 *  Result:
 *    A boolean value of TRUE if the server should accept the request. 
 *    FALSE should be returned if the server should deny the request. 
 *    All requests on the outstanding connection will be cancelled too.
 */
typedef CALLBACK_API_C( Boolean , _CFHTTPServerAcceptNewRequestCallBack )(_CFHTTPServerRef server, CFHTTPMessageRef headers, CFDataRef peer, void *info);

/*
 *  _CFHTTPServerDidReceiveRequestCallBack
 *  
 *  Discussion:
 *    Callback which is invoked when a request has been successfully
 *    received.
 *  
 *  Parameters:
 *    
 *    server:
 *      The instance of the server which received the given request.
 *    
 *    request:
 *      The request which was received.  Use this request in order to
 *      add a response.
 *    
 *    info:
 *      The reference from the server context which was given when the
 *      server was created.
 */
typedef CALLBACK_API_C( void , _CFHTTPServerDidReceiveRequestCallBack )(_CFHTTPServerRef server, CFHTTPMessageRef request, void *info);

/*
 *  _CFHTTPServerDidSendResponseCallBack
 *  
 *  Discussion:
 *    Callback which is invoked when an individual response has been
 *    successfully sent.
 *  
 *  Parameters:
 *    
 *    server:
 *      The instance of the server with which the request and response
 *      were associated.
 *    
 *    request:
 *      The original request with which the response was associated.
 *    
 *    response:
 *      The massaged response which was sent over the wire.  This is a
 *      copy of the original response and has no body.  It is just the
 *      headers.
 *    
 *    info:
 *      The reference from the server context which was given when the
 *      server was created.
 */
typedef CALLBACK_API_C( void , _CFHTTPServerDidSendResponseCallBack )(_CFHTTPServerRef server, CFHTTPMessageRef request, CFHTTPMessageRef response, void *info);

/*
 *  _CFHTTPServerErrorCallBack
 *  
 *  Discussion:
 *    Callback which is invoked when errors occur in the server.  All
 *    requests and responses on a single persistent connection will
 *    receive the error at the same time if it was the connection
 *    itself which had the error.
 *  
 *  Parameters:
 *    
 *    server:
 *      The instance of the server which received the error.
 *    
 *    error:
 *      Reference to the CFStreamError which contains the error
 *      information.
 *    
 *    request:
 *      If non-NULL, the request which was being actively managed when
 *      the error was received.
 *    
 *    response:
 *      If non-NULL, the response which was being actively managed when
 *      the error was received.
 *    
 *    info:
 *      The reference from the server context which was given when the
 *      server was created.
 */
typedef CALLBACK_API_C( void , _CFHTTPServerErrorCallBack )(_CFHTTPServerRef server, const CFStreamError *error, CFHTTPMessageRef request, CFHTTPMessageRef response, void *info);

/*
 *  _CFHTTPServerCallBacks
 *  
 *  Discussion:
 *    The set of callbacks which will be invoked as different events
 *    occur inside of the server.
 */
struct _CFHTTPServerCallBacks {

  /*
   * The version number of the structure type being passed in as a
   * parameter to the CFHTTPServer creation function. Valid version
   * number is currently 0.
   */
  CFIndex             version;

  /*
   * Callback invoked when a new incoming connection is accepted.
   */
  _CFHTTPServerAcceptNewConnectionCallBack  acceptNewConnectionCallBack;

  /*
   * Callback invoked when a new incoming request has started but after
   * the headers have arrived.
   */
  _CFHTTPServerAcceptNewRequestCallBack  acceptNewRequestCallBack;

  /*
   * Callback invoked when a full request has been received.
   */
  _CFHTTPServerDidReceiveRequestCallBack  didReceiveRequestCallBack;

  /*
   * Callback invoked when an individual response has been completely
   * sent.
   */
  _CFHTTPServerDidSendResponseCallBack  didSendResponseCallBack;

  /*
   * Callback invoked if there is an error on the server.
   */
  _CFHTTPServerErrorCallBack  errorCallBack;
};
typedef struct _CFHTTPServerCallBacks   _CFHTTPServerCallBacks;
/*
 *  _CFHTTPServerGetTypeID()
 *  
 *  Discussion:
 *    Returns the type identifier of all _CFHTTPServer instances.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 */
extern CFTypeID 
_CFHTTPServerGetTypeID(void)                                  AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  _CFHTTPServerCreate()
 *  
 *  Discussion:
 *    Creates a new HTTP server.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      The CFAllocator which should be used to allocate memory for the
 *      server. If this reference is not a valid CFAllocator, the
 *      behavior is undefined.
 *    
 *    callbacks:
 *      The callbacks which should be called as requests and responses
 *      are handled or if errors occur.
 *    
 *    context:
 *      A _CFHTTPServerContext which is used to set the contextual
 *      information associated with the server object. The info pointer
 *      from the struct will be passed to the callback function.
 *  
 *  Result:
 *    Returns NULL if unsuccessful, otherwise a _CFHTTPServerRef will
 *    be returned.
 *  
 */
extern _CFHTTPServerRef 
_CFHTTPServerCreate(
  CFAllocatorRef                  alloc,
  const _CFHTTPServerCallBacks *  callbacks,
  _CFHTTPServerContext *          context)                    AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  _CFHTTPServerStart()
 *  
 *  Discussion:
 *    Starts the server listening and handling incoming connections on
 *    the given port.  It also Rendezvous advertises itself on the
 *    local domain with the given name and type if provided.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    server:
 *      The server being started.  Must be non-NULL. If this reference
 *      is not a valid _CFHTTPServerRef, the behavior is undefined.
 *    
 *    name:
 *      The name to use for Rendezvous advertising.  The empty string
 *      or NULL indicate to use the machine's name.
 *    
 *    serviceType:
 *      The type of service being advertised on Rendezvous. If the
 *      service type and name are both NULL, the server will not
 *      Rendezvous advertise itself.
 *    
 *    port:
 *      The port on which the server should be listening for new
 *      connections.  Use zero to indicate that the server can assign
 *      one for the service (use this is a well-known port has not been
 *      assigned).
 *  
 *  Result:
 *    Returns TRUE is the server was started.  It returns FALSE if
 *    there was a failure to start the server.
 *  
 */
extern Boolean 
_CFHTTPServerStart(
  _CFHTTPServerRef   server,
  CFStringRef        name,
  CFStringRef        serviceType,
  UInt32             port)                                    AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  _CFHTTPServerInvalidate()
 *  
 *  Discussion:
 *    Invalidates the server, so that it no longer accepts connections
 *    and the client will no longer receive callbacks. All outstanding
 *    requests/responses are terminated.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    server:
 *      The server being invalidated.  Must be non-NULL. If this
 *      reference is not a valid _CFHTTPServerRef, the behavior is
 *      undefined.
 *  
 */
extern void 
_CFHTTPServerInvalidate(_CFHTTPServerRef server)              AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  _CFHTTPServerGetPort()
 *  
 *  Discussion:
 *    Returns the port on which the server is listening. If not
 *    currently listening, it will return zero.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    server:
 *      The server being queried.  Must be non-NULL. If this reference
 *      is not a valid _CFHTTPServerRef, the behavior is undefined.
 *  
 */
extern UInt32 
_CFHTTPServerGetPort(_CFHTTPServerRef server)                 AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  _CFHTTPServerCopyPeerAddressForRequest()
 *  
 *  Discussion:
 *    Given an incoming request, this function will retrieve the peer's
 *    address.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    server:
 *      The server which received the request.
 *    
 *    request:
 *      The incoming request whose peer wishes to be known.
 *  
 *  Result:
 *    Returns a CFDataRef which contains a struct sockaddr holding the
 *    address of the peer.  NULL is returned if not known.
 *  
 */
extern CFDataRef 
_CFHTTPServerCopyPeerAddressForRequest(
  _CFHTTPServerRef   server,
  CFHTTPMessageRef   request)                                 AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  _CFHTTPServerAddResponse()
 *  
 *  Discussion:
 *    Adds the given response for the request to the server. Only one
 *    response should be added per individual request.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    server:
 *      The server on which the request was received.  Must be
 *      non-NULL.  If this reference is not a valid _CFHTTPServerRef,
 *      the behavior is undefined.
 *    
 *    request:
 *      The request which was received and for which the response being
 *      added corresponds.
 *    
 *    response:
 *      The response being added for the given request.
 *  
 */
extern void 
_CFHTTPServerAddResponse(
  _CFHTTPServerRef   server,
  CFHTTPMessageRef   request,
  CFHTTPMessageRef   response)                                AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  _CFHTTPServerAddStreamedResponse()
 *  
 *  Discussion:
 *    Adds the given response headers and body for the request to the
 *    server.  Only one response should be added per individual request.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    server:
 *      The server on which the request was received.  Must be
 *      non-NULL.  If this reference is not a valid _CFHTTPServerRef,
 *      the behavior is undefined.
 *    
 *    request:
 *      The request which was received and for which the response being
 *      added corresponds.
 *    
 *    responseHeaders:
 *      The headers for the response being added.
 *    
 *    body:
 *      An unopened CFReadStreamRef which will be used for stream- ing
 *      the body's data to the requesting client.
 *  
 */
extern void 
_CFHTTPServerAddStreamedResponse(
  _CFHTTPServerRef   server,
  CFHTTPMessageRef   request,
  CFHTTPMessageRef   responseHeaders,
  CFReadStreamRef    body)                                    AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif

#pragma options align=reset

#ifdef __cplusplus
}
#endif

#endif /* __CFHTTPSERVERPRIV__ */

