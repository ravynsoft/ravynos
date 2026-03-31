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
 *  ProxySupport.h
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on 11/4/04.
 *  Copyright 2004 Apple Computer, Inc. All rights reserved.
 *
 */

#ifndef __PROXYSUPPORT__
#define __PROXYSUPPORT__

#include <CFNetwork/CFNetwork.h>


#if defined(__cplusplus)
extern "C" {
#endif
	

/*!
	@function _CFNetworkCFHostDoesNeedProxy
	@discussion Given a host with a name or an address check to see
		if it matches anything in the bypass list.  This performs
		wildcard matches along with straight comparisons.  Address
		resolutions and reverse lookups are not performed.  In addition if localBypass
		is true, names without periods are treated as not needing the proxy.
	@param host A CFHostRef representing the host which is to be checked.
		Must be non-NULL.  If this If this reference is not a valid
		CFHostRef, the behavior is undefined.
	@param bypasses A CFArrayRef of CFStringRef's indicating host
		names which should be bypassed (not use the proxy).  The
		exception list in the SystemConfiguration proxy dictionary
		can be used directly.
	@param localBypass A CFBooleanRefindicating "local" hosts do not need the proxy.
	@result If a match for the host is not found, TRUE is returned.  
		If a match is not found, FALSE is returned.
*/
// Currently not used, so hand dead-stripping for now.
//extern Boolean _CFNetworkCFHostDoesNeedProxy(CFHostRef host, CFArrayRef bypasses, CFBooleanRef localBypass);

/*!
	@function _CFNetworkDoesNeedProxy
	@discussion Given a host name check to see if it matches anything
		in the bypass list.  This performs wildcard matches along with
		straight comparisons.  Address resolutions and reverse lookups
		are not performed.  In addition if localBypass
		is true, names without periods are treated as not needing the proxy.
	@param host A CFHostRef representing the host which is to be checked.
		Must be non-NULL.  If this If this reference is not a valid
		CFHostRef, the behavior is undefined.
	@param bypasses A CFArrayRef of CFStringRef's indicating host
		names which should be bypassed (not use the proxy).  The
		exception list in the SystemConfiguration proxy dictionary
		can be used directly.
	@param localBypass A CFBooleanRefindicating "local" hosts do not need the proxy.
	@result If a match for the host is not found, TRUE is returned.  
		If a match is not found, FALSE is returned.
*/
extern Boolean _CFNetworkDoesNeedProxy(CFStringRef hostname, CFArrayRef bypasses, CFBooleanRef localBypass);


#if defined(__cplusplus)
}
#endif


#endif	/* __PROXYSUPPORT__ */
