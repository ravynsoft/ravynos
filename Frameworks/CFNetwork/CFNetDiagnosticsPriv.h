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
     File:       CFNetwork/CFNetDiagnosticsPriv.h
 
     Contains:   CFNetDiagnostics private interfaces
 
     Version:    1.0
 
     Copyright:  © 2004-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file contains unreleased SPI's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFNetDiagnosticsPriv.i
                     Revision:        1.4
                     Dated:           2004/10/01 16:45:11
                     Last change by:  jwyld
                     Last comment:    3817528 Drop in a bunch of DATA segment changes
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFNETDIAGNOSTICSPRIV__
#define __CFNETDIAGNOSTICSPRIV__

#ifndef __COREFOUNDATION__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifndef __CFNETDIAGNOSTICS__
#include <CFNetwork/CFNetDiagnostics.h>
#endif


/*FIXME I need to be converted to headerdoc*/

/*    
    CFNetDiagnosticNotifyKey

    This key is for use with the APIs in notify.h, such as notify_register_check().
    This key's value will be increased every time a diagnosis is completed, no matter 
    whether it succeeded or failed.
*/

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

extern const char *CFNetDiagnosticNotifyKey                          AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
/*  
    standardized protocol key values
    We may have to add more.
*/
extern const CFStringRef CFNetDiagnosticProtocolHTTP                 AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
extern const CFStringRef CFNetDiagnosticProtocolFTP                  AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
extern const CFStringRef CFNetDiagnosticProtocolSMTP                 AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
extern const CFStringRef CFNetDiagnosticProtocolIMAP                 AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
extern const CFStringRef CFNetDiagnosticProtocolOSCAR                AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
extern const CFStringRef CFNetDiagnosticProtocolUnknown              AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
/*
    CFNetDiagnosticCreateBasic() builds an NDDescriptorRef based on parameters
    passed in by the application. Any parameter may be NULL. If a parameter is NULL that
    entry in the details dictionary will be omitted.
*/
extern CFNetDiagnosticRef 
CFNetDiagnosticCreateBasic(
  CFAllocatorRef   allocator,
  CFStringRef      remoteHost,
  CFStringRef      protocol,
  CFNumberRef      port)                                      AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


extern void 
CFNetDiagnosticSetProtocol(
  CFNetDiagnosticRef   details,
  CFStringRef          name)                                  AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


extern void 
CFNetDiagnosticSetServiceID(
  CFNetDiagnosticRef   details,
  CFStringRef          name)                                  AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


/* 
    CFNetDiagnosticCreateStatusActive() returns a status value that can be used to display basic information 
    about the connection. If the caller wishes they may pass in a pointer to a CFStringRef
    that will be used to passive back a localized description of the problem. It is the
    callers responsibility to release the CFStringRef. If the callers does not want a
    description they may pass in NULL.
*/
extern CFNetDiagnosticStatus 
CFNetDiagnosticCopyNetworkStatusActively(
  CFNetDiagnosticRef   details,
  CFNumberRef          timeout,
  CFStringRef *        description)                           AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CFNETDIAGNOSTICSPRIV__ */

