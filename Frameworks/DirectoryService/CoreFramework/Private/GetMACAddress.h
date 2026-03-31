/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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

/*!
 * @header GetMACAddress
 */

#ifndef __GetMACAddress_h__
#define __GetMACAddress_h__	1

#include <DirectoryServiceCore/PrivateTypes.h>
#include <CoreFoundation/CoreFoundation.h>

__BEGIN_DECLS

SInt32 GetMACAddress( CFStringRef *theLZMACAddress, CFStringRef *theNLZMACAddress, bool bWithColons );
CFStringRef GetMACAddressFormattedStr(unsigned char* addr, bool bLeadingZeros, bool bWithColons );

__END_DECLS

#endif	// __GetMACAddress_h__
