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
     File:       CFNetwork/CFServerPriv.h
 
     Contains:   CoreFoundation Network server SPI
 
     Copyright:  © 2003-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file contains unreleased SPI's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFServerPriv.i
                     Revision:        1.3
                     Dated:           2004/06/01 17:53:06
                     Last change by:  rew
                     Last comment:    Updating all copyrights to include 2004
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFSERVERPRIV__
#define __CFSERVERPRIV__

#ifndef __CFNETWORKDEFS__
#include <CFNetwork/CFNetworkDefs.h>
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


/*
 *  _CFServerRef
 *  
 *  Discussion:
 *    This is the type of a reference to a TCP server.  Although
 *    individual functions are thread-safe, _CFServerRef itself is not
 *    thread-safe.
 */
typedef struct __CFServer*              _CFServerRef;

/*
 *  _CFServerContext
 *  
 *  Discussion:
 *    Structure containing the user-defined data and callbacks for
 *    _CFServerRef objects.
 */
struct _CFServerContext {

  /*
   * The version number of the structure type being passed in as a
   * parameter to the CFServer creation function. Valid version number
   * is currently 0.
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
typedef struct _CFServerContext         _CFServerContext;

/*
 *  _CFServerCallBack
 *  
 *  Discussion:
 *    Callback which is invoked as a new connection is accepted.
 *  
 *  Parameters:
 *    
 *    server:
 *      The instance of the server which is receiving the connection.
 *    
 *    sock:
 *      The native socket which has been accepted or -1 if there was an
 *      error.
 *    
 *    error:
 *      A reference to a CFStreamError which contains any error which
 *      may have occurred in the server.
 *    
 *    info:
 *      The reference from the server context which was given when the
 *      server was created.
 */
typedef CALLBACK_API_C( void , _CFServerCallBack )(_CFServerRef server, CFSocketNativeHandle sock, const CFStreamError *error, void *info);
/*
 *  _CFServerGetTypeID()
 *  
 *  Discussion:
 *    Returns the type identifier of all _CFServer instances.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 */
extern CFTypeID 
_CFServerGetTypeID(void)                                      AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  _CFServerCreate()
 *  
 *  Discussion:
 *    Create a new instance of a server object.  This will allocate
 *    only the underlying structures and set aside the resources. Use
 *    _CFServerStart to actually start the server listening.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      Allocator to use for allocating.  NULL indicates the default
 *      allocator.
 *    
 *    callback:
 *      Function to call as incoming connections are accepted
 *    
 *    context:
 *      Reference to a context block which will be copied into the
 *      server context for the callbacks.
 *  
 */
extern _CFServerRef 
_CFServerCreate(
  CFAllocatorRef      alloc,
  _CFServerCallBack   callback,
  _CFServerContext *  context)                                AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/*
 *  _CFServerGetPort()
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
 *      is not a valid _CFServerRef, the behavior is undefined.
 *  
 */
extern UInt32 
_CFServerGetPort(_CFServerRef server)                         AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/*
 *  _CFServerStart()
 *  
 *  Discussion:
 *    Starts the socket listening on the given port.  Registers the
 *    service by name and type on the local network.  The socket and
 *    service will be registered on the current run loop in the common
 *    modes.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    server:
 *      Reference to the server.  Must be non-NULL.
 *    
 *    name:
 *      Unique name to register on the network.  NULL indicates to use
 *      the computer's name.  The name must be unique in the local
 *      domain and for the given service type.
 *    
 *    serviceType:
 *      Service type being registered.  Service types can be retrieved
 *      from IANA (www.iana.org).  Examples include _http._tcp. and
 *      _echo._tcp.
 *    
 *    port:
 *      TCP port on which to listen.  If a well-known port is not
 *      required, set to zero and one will be assigned.
 *  
 */
extern Boolean 
_CFServerStart(
  _CFServerRef   server,
  CFStringRef    name,
  CFStringRef    serviceType,
  UInt32         port)                                        AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/*
 *  _CFServerInvalidate()
 *  
 *  Discussion:
 *    Removes the client and its associated data from the server
 *    reference. This ensures that the client will no longer get
 *    callbacks associated with this instance of the object.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    server:
 *      Reference to the server.  Must be non-NULL.
 *  
 */
extern void 
_CFServerInvalidate(_CFServerRef server)                      AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



#pragma options align=reset

#ifdef __cplusplus
}
#endif

#endif /* __CFSERVERPRIV__ */

