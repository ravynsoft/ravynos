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
     File:       CFNetwork/CFHostPriv.h
 
     Contains:   CoreFoundation Network CFHost header (private)
 
     Copyright:  © 2004-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file contains unreleased SPI's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFHostPriv.i
                     Revision:        1.4
                     Dated:           2004/06/01 17:53:05
                     Last change by:  rew
                     Last comment:    Updating all copyrights to include 2004
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFHOSTPRIV__
#define __CFHOSTPRIV__

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

#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint on
#endif

/*
 *  CFHostGetInfo()
 *  
 *  Discussion:
 *    Tries to retrieve the information of the given type from the
 *    given host.  Returns a CFTypeRef containing the information.  The
 *    actual type of object returned is dictated by the type of
 *    information being requested.  NULL is returned otherwise.
 *  
 *  Mac OS X threading:
 *    Not thread safe
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
 *    info:
 *      The CFHostInfoType information type to retrieve.
 *    
 *    hasBeenResolved:
 *      A reference to a Boolean which returns FALSE if the information
 *      was not available (e.g. CFHostStartInfoResolution has not been
 *      called), otherwise TRUE will be returned.
 *  
 */
extern CFTypeRef 
CFHostGetInfo(
  CFHostRef        theHost,
  CFHostInfoType   info,
  Boolean *        hasBeenResolved)                           AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CFHOSTPRIV__ */

