/*	CFStringEncodingConverterPriv.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFSTRINGENCODINGCONVERTERPRIV__)
#define __COREFOUNDATION_CFSTRINGENCODINGCONVERTERPRIV__ 1

#include <CoreFoundation/CFBase.h>
#include "CFStringEncodingConverterExt.h"

CF_PRIVATE  const CFStringEncodingConverter __CFConverterASCII;
CF_PRIVATE  const CFStringEncodingConverter __CFConverterISOLatin1;
CF_PRIVATE  const CFStringEncodingConverter __CFConverterWinLatin1;
CF_PRIVATE  const CFStringEncodingConverter __CFConverterNextStepLatin;
CF_PRIVATE  const CFStringEncodingConverter __CFConverterUTF8;

#if TARGET_OS_MAC
CF_PRIVATE  const CFStringEncodingConverter __CFConverterMacRoman;
#endif

CF_PRIVATE  CFStringEncoding *__CFStringEncodingCreateListOfAvailablePlatformConverters(CFAllocatorRef allocator, CFIndex *numberOfConverters);
CF_PRIVATE  const CFStringEncodingConverter *__CFStringEncodingGetExternalConverter(uint32_t encoding);
CF_PRIVATE  CFIndex __CFStringEncodingPlatformUnicodeToBytes(uint32_t encoding, uint32_t flags, const UniChar *characters, CFIndex numChars, CFIndex *usedCharLen, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen);
CF_PRIVATE  CFIndex __CFStringEncodingPlatformBytesToUnicode(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, CFIndex *usedByteLen, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen);
CF_PRIVATE  CFIndex __CFStringEncodingPlatformCharLengthForBytes(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes);
CF_PRIVATE  CFIndex __CFStringEncodingPlatformByteLengthForCharacters(uint32_t encoding, uint32_t flags, const UniChar *characters, CFIndex numChars);

/* Returns required length of destination buffer for conversion.  These functions are faster than specifying 0 to maxByteLen (maxCharLen), but unnecessarily optimal length
 */
CF_PRIVATE CFIndex CFStringEncodingCharLengthForBytes(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes);
CF_PRIVATE CFIndex CFStringEncodingByteLengthForCharacters(uint32_t encoding, uint32_t flags, const UniChar *characters, CFIndex numChars);

CF_PRIVATE bool CFStringEncodingIsValidEncoding(uint32_t encoding);

/* Returns kCFStringEncodingInvalidId terminated encoding list
 */
CF_PRIVATE const CFStringEncoding *CFStringEncodingListOfAvailableEncodings(void);

CF_PRIVATE const CFStringEncodingConverter *CFStringEncodingGetConverter(uint32_t encoding);

#endif /* ! __COREFOUNDATION_CFSTRINGENCODINGCONVERTERPRIV__ */

