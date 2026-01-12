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

/*
 *  CFStringEncodingDatabase.h
 *  CoreFoundation
 *
 *  Created by Aki Inoue on 07/12/05.
 *  Copyright (c) 2007-2014, Apple Inc. All rights reserved.
 *
 */

#include <CoreFoundation/CFString.h>

CF_PRIVATE uint16_t __CFStringEncodingGetWindowsCodePage(CFStringEncoding encoding);
CF_PRIVATE CFStringEncoding __CFStringEncodingGetFromWindowsCodePage(uint16_t codepage);

CF_PRIVATE bool __CFStringEncodingGetCanonicalName(CFStringEncoding encoding, char *buffer, CFIndex bufferSize);
CF_PRIVATE CFStringEncoding __CFStringEncodingGetFromCanonicalName(const char *canonicalName);

CF_PRIVATE CFStringEncoding __CFStringEncodingGetMostCompatibleMacScript(CFStringEncoding encoding);

CF_PRIVATE const char *__CFStringEncodingGetName(CFStringEncoding encoding); // Returns simple non-localizd name
