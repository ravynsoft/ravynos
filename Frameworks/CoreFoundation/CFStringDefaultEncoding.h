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

/*	CFStringDefaultEncoding.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFSTRINGDEFAULTENCODING__)
#define __COREFOUNDATION_CFSTRINGDEFAULTENCODING__ 1

#include <CoreFoundation/CFBase.h>

#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
#include <stdlib.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/param.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <xlocale.h>

CF_EXTERN_C_BEGIN

#define __kCFUserEncodingEnvVariableName ("__CF_USER_TEXT_ENCODING")
#define __kCFMaxDefaultEncodingFileLength (64)
#define __kCFUserEncodingFileName ("/.CFUserTextEncoding")

CF_EXPORT void _CFStringGetUserDefaultEncoding(UInt32 *oScriptValue, UInt32 *oRegionValue);
CF_EXPORT void _CFStringGetInstallationEncodingAndRegion(uint32_t *encoding, uint32_t *region);
CF_EXPORT Boolean _CFStringSaveUserDefaultEncoding(UInt32 iScriptValue, UInt32 iRegionValue);

CF_INLINE void __CFStringGetUserDefaultEncoding(UInt32 *oScriptValue, UInt32 *oRegionValue) { _CFStringGetUserDefaultEncoding(oScriptValue, oRegionValue); }
CF_INLINE void __CFStringGetInstallationEncodingAndRegion(uint32_t *encoding, uint32_t *region) { _CFStringGetInstallationEncodingAndRegion(encoding, region); }
CF_INLINE void __CFStringSaveUserDefaultEncoding(UInt32 iScriptValue, UInt32 iRegionValue) { _CFStringSaveUserDefaultEncoding(iScriptValue, iRegionValue); }

CF_EXTERN_C_END

#endif

#endif /* ! __COREFOUNDATION_CFSTRINGDEFAULTENCODING__ */


