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
     File:       CFNetwork/CFFTPStreamPriv.h
 
     Contains:   CoreFoundation Network FTP streams header (private)
 
     Copyright:  © 2003-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file contains unreleased SPI's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFFTPStreamPriv.i
                     Revision:        1.4
                     Dated:           2004/06/01 17:53:05
                     Last change by:  rew
                     Last comment:    Updating all copyrights to include 2004
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFFTPSTREAMPRIV__
#define __CFFTPSTREAMPRIV__

#ifndef __CFNETWORKDEFS__
#include <CFNetwork/CFNetworkDefs.h>
#endif

#ifndef __CFSTREAM__
#include <CoreFoundation/CFStream.h>
#endif




#include <AvailabilityMacros.h>

#if PRAGMA_ONCE
#pragma once
#endif

#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint on
#endif

/*
 *  _kCFStreamPropertyFTPLogInOnly
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations.  CFBoolean
 *    type to indicate that the stream should only log into the server
 *    and then stop at the idle state.
 *  
 */
extern const CFStringRef _kCFStreamPropertyFTPLogInOnly              AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;

/*
 *  _kCFStreamPropertyFTPRemoveResource
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations.  CFBoolean
 *    type to indicate that the write stream should remove the
 *    referenced resource instead of creating it.
 *  
 */
extern const CFStringRef _kCFStreamPropertyFTPRemoveResource         AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;

/*
 *  _kCFStreamPropertyFTPNewResourceName
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations.  CFURL
 *    type indicating the new path name.  This will cause the request
 *    url to be renamed to the given url.
 *  
 */
extern const CFStringRef _kCFStreamPropertyFTPNewResourceName        AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;

#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif


#endif /* __CFFTPSTREAMPRIV__ */

