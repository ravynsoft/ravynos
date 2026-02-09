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
     File:       CFNetwork/CFHTTPStreamPriv.h
 
     Contains:   CoreFoundation Network HTTP streams header (private)
 
     Copyright:  © 2003-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file contains unreleased SPI's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFHTTPStreamPriv.i
                     Revision:        1.5
                     Dated:           2004/06/01 17:53:05
                     Last change by:  rew
                     Last comment:    Updating all copyrights to include 2004
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFHTTPSTREAMPRIV__
#define __CFHTTPSTREAMPRIV__

#ifndef __CFNETWORKDEFS__
#include <CFNetwork/CFNetworkDefs.h>
#endif

#ifndef __CFHTTPSTREAM__
#include <CFNetwork/CFHTTPStream.h>
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

extern const CFStringRef kCFStreamPropertyHTTPRequest                AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;
/* Value is the CFHTTPMessage that represents the request*/
extern const CFStringRef kCFHTTPRedirectionResponse                  AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;
/*The first redirection*/
/*
The following routines filter the read or write stream as if they were coming from 
or going to an HTTP server.  The read stream variant, for instance, strips off the
first bytes and interprets them as an HTTP header.  The write stream formats the 
given request and transmits it before transmitting the write stream bytes.
*/
extern CFReadStreamRef 
CFReadStreamCreateHTTPStream(
  CFAllocatorRef    alloc,
  CFReadStreamRef   readStream,
  Boolean           forResponse)                              AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


extern CFWriteStreamRef 
CFWriteStreamCreateHTTPStream(
  CFAllocatorRef     alloc,
  CFHTTPMessageRef   header,
  Boolean            useFullURL,
  CFWriteStreamRef   socketStream)                            AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


extern CFDataRef 
_CFHTTPMessageCopySerializedMessage(
  CFHTTPMessageRef   msg,
  Boolean            forProxy)                                AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


extern void 
_CFHTTPMessageSetHeader(
  CFHTTPMessageRef   msg,
  CFStringRef        theHeader,
  CFStringRef        value,
  CFIndex            position)                                AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


extern const CFStringRef _kCFStreamPropertyHTTPConnectionStreams     AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;
extern const CFStringRef _kCFStreamPropertyHTTPZeroLengthResponseExpected AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;
extern const CFStringRef _kCFStreamPropertyHTTPProxyProxyAutoConfigURLString AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;
extern const CFStringRef _kCFStreamPropertyHTTPProxyProxyAutoConfigEnable AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;

#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CFHTTPSTREAMPRIV__ */

