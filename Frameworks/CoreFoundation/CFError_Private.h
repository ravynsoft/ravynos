/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFError_Private.h
        Copyright (c) 2006-2014, Apple Inc. All rights reserved.
	
	This is Apple-internal SPI for CFError.
*/

#if !defined(__COREFOUNDATION_CFERRORPRIVATE__)
#define __COREFOUNDATION_CFERRORPRIVATE__ 1

#include <CoreFoundation/CFError.h>

CF_EXTERN_C_BEGIN

/* This callback function is consulted if a key is not present in the userInfo dictionary. Note that setting a callback for the same domain again simply replaces the previous callback. Set NULL as the callback to remove it.
*/
typedef CFTypeRef (*CFErrorUserInfoKeyCallBack)(CFErrorRef err, CFStringRef key);
CF_EXPORT void CFErrorSetCallBackForDomain(CFStringRef domainName, CFErrorUserInfoKeyCallBack callBack) CF_AVAILABLE(10_5, 2_0);
CF_EXPORT CFErrorUserInfoKeyCallBack CFErrorGetCallBackForDomain(CFStringRef domainName) CF_AVAILABLE(10_5, 2_0);


/* A key for "true" debugging descriptions which should never be shown to the user. It's only used when the CFError is shown to the console, and nothing else is available. For instance the rather terse and techie OSStatus descriptions are in this boat.
*/
CF_EXPORT const CFStringRef kCFErrorDebugDescriptionKey CF_AVAILABLE(10_5, 2_0);


CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFERRORPRIVATE__ */

