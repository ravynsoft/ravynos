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
     File:       CFNetwork/CFNetDiagnostics.h
 
     Contains:   CFNetDiagnostics interface
 
     Version:    1.0
 
     Copyright:  © 2004-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file may contain unreleased API's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFNetDiagnostics.i
                     Revision:        1.3
                     Dated:           2004/08/03 22:57:05
                     Last change by:  jiarocci
                     Last comment:    Update Interface files to include headerdoc and remove warnings.
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFNETDIAGNOSTICS__
#define __CFNETDIAGNOSTICS__

#ifndef __COREFOUNDATION__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <stdint.h>


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
 *  CFNetDiagnosticStatus
 *  
 *  Discussion:
 *    Returned by the various status and diagnostic calls
 */
typedef int32_t                         CFNetDiagnosticStatus;

/*
 *  CFNetDiagnosticRef
 *  
 *  Discussion:
 *    This is the type used to describe the types of connection that
 *    clients may be querying about
 */
typedef struct __CFNetDiagnostic*       CFNetDiagnosticRef;

/*
 *  CFNetDiagnosticStatusValues
 *  
 *  Discussion:
 *    Values for CFNetDiagnosticStatus
 */
enum CFNetDiagnosticStatusValues {

  /*
   * There is no status, but no error has occured
   */
  kCFNetDiagnosticNoErr         = 0,

  /*
   * An error occured that prevented the call from completing
   */
  kCFNetDiagnosticErr           = -66560L,

  /*
   * The connection appears to be working
   */
  kCFNetDiagnosticConnectionUp  = -66559L,
  kCFNetDiagnosticConnectionIndeterminate = -66558L,

  /*
   * The connection does not appear to be working
   */
  kCFNetDiagnosticConnectionDown = -66557L
};
typedef enum CFNetDiagnosticStatusValues CFNetDiagnosticStatusValues;


/*
 *  CFNetDiagnosticCreateWithStreams()
 *  
 *  Discussion:
 *    Creates a CFNetDiagnosticRef from a pair of CFStreams. Either
 *    stream may be NULL. This is the preferred interface for creating
 *    a CFNetDiagnosticRef.
 *  
 *  Parameters:
 *    
 *    alloc:
 *      The CF allocator to use.
 *    
 *    readStream:
 *      CFReadStreamRef referring to the failed connection. May be NULL.
 *    
 *    writeStream:
 *      CFWriteStreamRef referring to the failed connection. May be
 *      NULL.
 *  
 *  Result:
 *    A CFNetDiagnosticRef referring to the current networking issue.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.4 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFNetDiagnosticRef 
CFNetDiagnosticCreateWithStreams(
  CFAllocatorRef     alloc,
  CFReadStreamRef    readStream,
  CFWriteStreamRef   writeStream)                             AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/*
 *  CFNetDiagnosticCreateWithURL()
 *  
 *  Discussion:
 *    Creates a CFNetDiagnosticRef based on a CFURLRef passed in by the
 *    application.
 *  
 *  Parameters:
 *    
 *    alloc:
 *      The CF allocator to use.
 *    
 *    url:
 *      CFURLRef referring to the failed connection.
 *  
 *  Result:
 *    A CFNetDiagnosticRef referring to the current networking issue.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.4 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFNetDiagnosticRef 
CFNetDiagnosticCreateWithURL(
  CFAllocatorRef   alloc,
  CFURLRef         url)                                       AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  CFNetDiagnosticSetName()
 *  
 *  Discussion:
 *    If the framework requires an application name to be displayed to
 *    the user it will derive it from the bundle identifier of the
 *    currently running application, in that application's current
 *    localization. If you want to override that you may use
 *    CFNetDiagnosticAddName to specify a CFStringRef to be used.
 *  
 *  Parameters:
 *    
 *    details:
 *      CFNetDiagnosticRef referring to the current problem.
 *    
 *    name:
 *      The localized name that should appear to the user when
 *      referring to the application.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.4 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern void 
CFNetDiagnosticSetName(
  CFNetDiagnosticRef   details,
  CFStringRef          name)                                  AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  CFNetDiagnosticDiagnoseProblemInteractively()
 *  
 *  Discussion:
 *    Opens the Network Diagnostics window and returns immediately once
 *    it is open. The client passes in a CFNetDiagnosticRef built with
 *    one of the creator functions.
 *  
 *  Parameters:
 *    
 *    details:
 *      CFNetDiagnosticRef referring to the current problem.
 *  
 *  Result:
 *    A CFNetDiagnosticStatus. Will either be CFNetDiagnosticNoErr, or
 *    CFNetDiagnosticErr if there was an error attempting to run the
 *    diagnosis.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.4 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFNetDiagnosticStatus 
CFNetDiagnosticDiagnoseProblemInteractively(CFNetDiagnosticRef details) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/*
 *  CFNetDiagnosticCopyNetworkStatusPassively()
 *  
 *  Discussion:
 *    Returns a status value that can be used to display basic
 *    information about the connection. If the caller wishes they may
 *    pass in a pointer to a CFStringRef that will be used to pass back
 *    a localized description of the problem. It is the caller's
 *    responsibility to release the CFStringRef. If the caller does not
 *    want a description they may pass in NULL.
 *    CFNetDiagnosticCopyNetworkStatusPassively() is guaranteed not to
 *    cause network activity.
 *  
 *  Parameters:
 *    
 *    details:
 *      CFNetDiagnosticRef referring to the current problem
 *    
 *    description:
 *      A pointer to a CFStringRef that, upon return, will point to a
 *      localized string containing a description of the current
 *      network status. May be NULL. If it is not NULL, the client must
 *      call CFRelease on the returned object.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.4 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFNetDiagnosticStatus 
CFNetDiagnosticCopyNetworkStatusPassively(
  CFNetDiagnosticRef   details,
  CFStringRef *        description)                           AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CFNETDIAGNOSTICS__ */

