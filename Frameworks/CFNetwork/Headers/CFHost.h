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
     File:       CFNetwork/CFHost.h
 
     Contains:   CoreFoundation CFHost header
 
     Copyright:  © 2001-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file may contain unreleased API's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFHost.i
                     Revision:        1.8
                     Dated:           2004/06/01 17:53:05
                     Last change by:  rew
                     Last comment:    Updating all copyrights to include 2004
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFHOST__
#define __CFHOST__

#ifndef __CFNETWORKDEFS__
#include <CFNetwork/CFNetworkDefs.h>
#endif

#ifndef __COREFOUNDATION__
#include <CoreFoundation/CoreFoundation.h>
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
 *  CFHostRef
 *  
 *  Discussion:
 *    This is the type of a reference to a host name or address lookup.
 */
typedef struct __CFHost*                CFHostRef;

/*
 *  kCFStreamErrorDomainNetDB
 *  
 *  Discussion:
 *    Errors listed in netdb.h
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const SInt32 kCFStreamErrorDomainNetDB                        AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFStreamErrorDomainSystemConfiguration
 *  
 *  Discussion:
 *    Errors listed in SystemConfiguration/SystemConfiguration.h
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const SInt32 kCFStreamErrorDomainSystemConfiguration          AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/*
 *  CFHostInfoType
 *  
 *  Discussion:
 *    Host information types to be resolved.
 */
enum CFHostInfoType {

  /*
   * Results value is a CFArray of CFData's (each being a struct
   * sockaddr)
   */
  kCFHostAddresses              = 0,

  /*
   * Results value is a CFArray of CFString's
   */
  kCFHostNames                  = 1,

  /*
   * Results value is a CFData wrapping SCNetworkConnectionFlags
   * (defined in SystemConfiguration/SCNetwork.h)
   */
  kCFHostReachability           = 2
};
typedef enum CFHostInfoType CFHostInfoType;


/*
 *  CFHostClientContext
 *  
 *  Discussion:
 *    Structure containing the user-defined data and callbacks for
 *    CFHost objects.
 */
struct CFHostClientContext {

  /*
   * The version number of the structure type being passed in as a
   * parameter to the CFHost client function. Valid version number is
   * currently 0.
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
typedef struct CFHostClientContext      CFHostClientContext;

/*
 *  CFHostClientCallBack
 *  
 *  Discussion:
 *    Callback function which is called upon error or completion of an
 *    asynchronous resolve.
 *  
 *  Parameters:
 *    
 *    theHost:
 *      Host whose resolution is complete.
 *    
 *    typeInfo:
 *      Enum representing which info resolution is complete.
 *    
 *    error:
 *      Reference to an error structure if the resolution failed.
 *    
 *    info:
 *      Client's info reference which was passed into the client
 *      context.
 */
typedef CALLBACK_API_C( void , CFHostClientCallBack )(CFHostRef theHost, CFHostInfoType typeInfo, const CFStreamError *error, void *info);


/*
 *  CFHostGetTypeID()
 *  
 *  Discussion:
 *    Returns the type identifier of all CFHost instances.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFTypeID 
CFHostGetTypeID(void)                                         AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostCreateWithName()
 *  
 *  Discussion:
 *    Creates a new host object with the given name.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    allocator:
 *      The CFAllocator which should be used to allocate memory for the
 *      host. If this reference is not a valid CFAllocator, the
 *      behavior is undefined.
 *    
 *    hostname:
 *      A CFStringRef representing the name of the host. Must be
 *      non-NULL.  If this If this reference is not a valid
 *      CFStringRef, the behavior is undefined.
 *  
 *  Result:
 *    A valid CFHostRef which may now be resolved, or NULL if
 *    unsuccessful.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFHostRef 
CFHostCreateWithName(
  CFAllocatorRef   allocator,
  CFStringRef      hostname)                                  AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostCreateWithAddress()
 *  
 *  Discussion:
 *    Creates a new host object with the given address.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    allocator:
 *      The CFAllocator which should be used to allocate memory for the
 *      host. If this reference is not a valid CFAllocator, the
 *      behavior is undefined.
 *    
 *    addr:
 *      A CFDataRef containing a struct sockaddr which is the address
 *      of the host. Must be non-NULL.  If this If this reference is
 *      not a valid CFDataRef, the behavior is undefined.
 *  
 *  Result:
 *    A valid CFHostRef which may now be resolved, or NULL if
 *    unsuccessful.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFHostRef 
CFHostCreateWithAddress(
  CFAllocatorRef   allocator,
  CFDataRef        addr)                                      AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostCreateCopy()
 *  
 *  Discussion:
 *    Creates a new host object as a copy of host argument.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      The CFAllocator which should be used to allocate memory for the
 *      new host. If this reference is not a valid CFAllocator, the
 *      behavior is undefined.
 *    
 *    host:
 *      A CFHostRef representing the original host. Must be non-NULL. 
 *      If this If this reference is not a valid CFHostRef, the
 *      behavior is undefined.
 *  
 *  Result:
 *    A valid CFHostRef which contains a copy of all previously
 *    resolved data from the original.  NULL is returned in the case of
 *    failure.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFHostRef 
CFHostCreateCopy(
  CFAllocatorRef   alloc,
  CFHostRef        host)                                      AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostStartInfoResolution()
 *  
 *  Discussion:
 *    Performs a lookup for the given host.  It will search for the
 *    requested information if there is no other active request. 
 *    Previously cached information of the given type will be released.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    theHost:
 *      The CFHostRef which should be resolved. Must be non-NULL. If
 *      this reference is not a valid CFHostRef, the behavior is
 *      undefined.
 *    
 *    info:
 *      The enum representing the type of information to be retrieved. 
 *      If the value is not a valid type, the behavior is undefined.
 *    
 *    error:
 *      A reference to a CFStreamError structure which will be filled
 *      with any error information should an error occur.  May be set
 *      to NULL if error information is not wanted.
 *  
 *  Result:
 *    Returns TRUE on success and FALSE on failure.  In asynchronous
 *    mode, this function will return immediately.  In synchronous
 *    mode, it will block until the resolve has completed or until the
 *    resolve is cancelled.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHostStartInfoResolution(
  CFHostRef        theHost,
  CFHostInfoType   info,
  CFStreamError *  error)         /* can be NULL */           AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostGetAddressing()
 *  
 *  Discussion:
 *    Tries to retrieve the known addresses from the given host.
 *    Returns a CFArrayRef of addresses if known and there were some.
 *    NULL is returned otherwise.  Each address is a CFDataRef wrapping
 *    a struct sockaddr.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    The function gets the data in a thread-safe manner, but the
 *    resulting data is not safe.  Since it is returned as a matter of
 *    a get opposed to a copy, the data is not safe if the host is
 *    being altered from another thread.
 *  
 *  Parameters:
 *    
 *    theHost:
 *      The CFHostRef which contains the relevant information. Must be
 *      non-NULL. If this reference is not a valid CFHostRef, the
 *      behavior is undefined.
 *    
 *    hasBeenResolved:
 *      A reference to a Boolean which returns FALSE if the information
 *      was not available (e.g. CFHostStartInfoResolution has not been
 *      called), otherwise TRUE will be returned.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFArrayRef 
CFHostGetAddressing(
  CFHostRef   theHost,
  Boolean *   hasBeenResolved)       /* can be NULL */        AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostGetNames()
 *  
 *  Discussion:
 *    Tries to retrieve the names/aliases from the given host. Returns
 *    a CFArrayRef of names for the given host.  NULL is returned
 *    otherwise.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    The function gets the data in a thread-safe manner, but the
 *    resulting data is not safe.  Since it is returned as a matter of
 *    a get opposed to a copy, the data is not safe if the host is
 *    being altered from another thread.
 *  
 *  Parameters:
 *    
 *    theHost:
 *      The CFHostRef which contains the relevant information. Must be
 *      non-NULL. If this reference is not a valid CFHostRef, the
 *      behavior is undefined.
 *    
 *    hasBeenResolved:
 *      A reference to a Boolean which returns FALSE if the information
 *      was not available (e.g. CFHostStartInfoResolution has not been
 *      called), otherwise TRUE will be returned.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFArrayRef 
CFHostGetNames(
  CFHostRef   theHost,
  Boolean *   hasBeenResolved)       /* can be NULL */        AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostGetReachability()
 *  
 *  Discussion:
 *    Tries to retrieve the reachability of the given host. Returns a
 *    CFDataRef which wraps the reachability flags. NULL will be
 *    returned if the value has not been resolved. The possible values
 *    of these flags is declared in SystemConfiguration/SCNetwork.h.
 *    Returns FALSE if the information was not available, otherwise
 *    TRUE will be returned with the results containing the requested
 *    information.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    The function gets the data in a thread-safe manner, but the
 *    resulting data is not safe.  Since it is returned as a matter of
 *    a get opposed to a copy, the data is not safe if the host is
 *    being altered from another thread.
 *  
 *  Parameters:
 *    
 *    theHost:
 *      The CFHostRef which contains the relevant information. Must be
 *      non-NULL. If this reference is not a valid CFHostRef, the
 *      behavior is undefined.
 *    
 *    hasBeenResolved:
 *      A reference to a Boolean which returns FALSE if the information
 *      was not available (e.g. CFHostStartInfoResolution has not been
 *      called), otherwise TRUE will be returned.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFDataRef 
CFHostGetReachability(
  CFHostRef   theHost,
  Boolean *   hasBeenResolved)       /* can be NULL */        AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostCancelInfoResolution()
 *  
 *  Discussion:
 *    Cancels an outstanding asynchronous or synchronous resolve.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    theHost:
 *      The CFHostRef which is currently resolving.  Must be non-NULL.
 *      If this reference is not a valid CFHostRef, the behavior is
 *      undefined.
 *    
 *    info:
 *      The enum representing which resolution to be canceled.  If the
 *      value is not a valid type, the behavior is undefined.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern void 
CFHostCancelInfoResolution(
  CFHostRef        theHost,
  CFHostInfoType   info)                                      AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostSetClient()
 *  
 *  Discussion:
 *    Associates a client context and callback function with a
 *    CFHostRef.  This is required for asynchronous usage.  If not set,
 *    resolve will take place synchronously.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    theHost:
 *      The CFHostRef which is getting a client.  Must be non-NULL. If
 *      this reference is not a valid CFHostRef, the behavior is
 *      undefined.
 *    
 *    clientCB:
 *      A CFHostClientCallBack which will be called when the resolve
 *      completes or is cancelled.  Use NULL to remove the client
 *      association with a host object.
 *    
 *    clientContext:
 *      A CFHostClientContext which is used to set the contextual
 *      information associated with the host object.  The info pointer
 *      from the struct will be passed to the callback function. If
 *      setting a client, this value must be non-NULL.
 *  
 *  Result:
 *    Returns TRUE if the procedure was a success, otherwise it returns
 *    FALSE.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHostSetClient(
  CFHostRef              theHost,
  CFHostClientCallBack   clientCB,            /* can be NULL */
  CFHostClientContext *  clientContext)       /* can be NULL */ AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostScheduleWithRunLoop()
 *  
 *  Discussion:
 *    Schedules the given host on a run loop and mode so the client
 *    will receive its callbacks on that loop and mode.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    theHost:
 *      The CFHostRef which is being scheduled.  Must be non-NULL. If
 *      this reference is not a valid CFHostRef, the behavior is
 *      undefined.
 *    
 *    runLoop:
 *      A CFRunLoopRef on which the host should be scheduled. Must be
 *      non-NULL.  If this reference is not a valid CFRunLoopRef, the
 *      behavior is undefined.
 *    
 *    runLoopMode:
 *      A CFStringRef which is the mode in which the run loop will be
 *      running when notification occurs.  Must be non-NULL. If this
 *      reference is not a valid CFStringRef, the behavior is undefined.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern void 
CFHostScheduleWithRunLoop(
  CFHostRef      theHost,
  CFRunLoopRef   runLoop,
  CFStringRef    runLoopMode)                                 AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFHostUnscheduleFromRunLoop()
 *  
 *  Discussion:
 *    Unschedules the given host from a run loop and mode so the client
 *    will not receive its callbacks on that loop and mode.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    theHost:
 *      The CFHostRef which is being unscheduled.  Must be non-NULL. If
 *      this reference is not a valid CFHostRef, the behavior is
 *      undefined.
 *    
 *    runLoop:
 *      A CFRunLoopRef on which the host is scheduled and should now be
 *      unscheduled.  Must be non-NULL.  If this reference is not a
 *      valid CFRunLoopRef, the behavior is undefined.
 *    
 *    runLoopMode:
 *      A CFStringRef which is the mode in which the host is scheduled
 *      and should be unscheduled.  Must be non-NULL. If this reference
 *      is not a valid CFStringRef, the behavior is undefined.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern void 
CFHostUnscheduleFromRunLoop(
  CFHostRef      theHost,
  CFRunLoopRef   runLoop,
  CFStringRef    runLoopMode)                                 AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif

#pragma options align=reset

#ifdef __cplusplus
}
#endif

#endif /* __CFHOST__ */

