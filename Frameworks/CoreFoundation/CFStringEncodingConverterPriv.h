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

/*	CFStringEncodingConverterPriv.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFSTRINGENCODINGCONVERTERPRIV__)
#define __COREFOUNDATION_CFSTRINGENCODINGCONVERTERPRIV__ 1

#include <CoreFoundation/CFBase.h>
#include "CFStringEncodingConverterExt.h"

extern  const CFStringEncodingConverter __CFConverterASCII;
extern  const CFStringEncodingConverter __CFConverterISOLatin1;
extern  const CFStringEncodingConverter __CFConverterMacRoman;
extern  const CFStringEncodingConverter __CFConverterWinLatin1;
extern  const CFStringEncodingConverter __CFConverterNextStepLatin;
extern  const CFStringEncodingConverter __CFConverterUTF8;

extern  CFStringEncoding *__CFStringEncodingCreateListOfAvailablePlatformConverters(CFAllocatorRef allocator, CFIndex *numberOfConverters);
extern  const CFStringEncodingConverter *__CFStringEncodingGetExternalConverter(uint32_t encoding);
extern  CFIndex __CFStringEncodingPlatformUnicodeToBytes(uint32_t encoding, uint32_t flags, const UniChar *characters, CFIndex numChars, CFIndex *usedCharLen, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen);
extern  CFIndex __CFStringEncodingPlatformBytesToUnicode(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, CFIndex *usedByteLen, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen);
extern  CFIndex __CFStringEncodingPlatformCharLengthForBytes(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes);
extern  CFIndex __CFStringEncodingPlatformByteLengthForCharacters(uint32_t encoding, uint32_t flags, const UniChar *characters, CFIndex numChars);

#endif /* ! __COREFOUNDATION_CFSTRINGENCODINGCONVERTERPRIV__ */

