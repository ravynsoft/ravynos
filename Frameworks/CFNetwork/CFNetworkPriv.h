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
     File:       CFNetwork/CFNetworkPriv.h
 
     Contains:   CoreFoundation Network private framework header
 
     Copyright:  © 2001-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file contains unreleased SPI's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFNetworkPriv.i
                     Revision:        1.13
                     Dated:           2004/06/01 17:53:05
                     Last change by:  rew
                     Last comment:    Updating all copyrights to include 2004
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFNETWORKPRIV__
#define __CFNETWORKPRIV__

#ifndef __CFNETWORK__
#include <CFNetwork/CFNetwork.h>
#endif


#ifndef __CFHTTPMESSAGEPRIV__
#include <CFNetwork/CFHTTPMessagePriv.h>
#endif

#ifndef __CFSOCKETSTREAMPRIV__
#include <CFNetwork/CFSocketStreamPriv.h>
#endif

#ifndef __CFSERVERPRIV__
#include <CFNetwork/CFServerPriv.h>
#endif

#ifndef __CFHTTPSERVERPRIV__
#include <CFNetwork/CFHTTPServerPriv.h>
#endif

#ifndef __CFHTTPSTREAMPRIV__
#include <CFNetwork/CFHTTPStreamPriv.h>
#endif

#ifndef __CFFTPSTREAMPRIV__
#include <CFNetwork/CFFTPStreamPriv.h>
#endif

#ifndef __CFHTTPCONNECTIONPRIV__
#include <CFNetwork/CFHTTPConnectionPriv.h>
#endif

#ifndef __CFHOSTPRIV__
#include <CFNetwork/CFHostPriv.h>
#endif

#ifndef __CFNETDIAGNOSTICSPRIV__
#include <CFNetwork/CFNetDiagnosticsPriv.h>
#endif

#ifndef __CFNETSERVICESPRIV__
#include <CFNetwork/CFNetServicesPriv.h>
#endif



#endif /* __CFNETWORKPRIV__ */

