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

/*	CFPlatformConverters.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Aki Inoue
*/

#include "CFInternal.h"
#include <CoreFoundation/CFString.h>
#include "CFStringEncodingConverterExt.h"
#include <CoreFoundation/CFStringEncodingExt.h>
#include "CFUniChar.h"
#include "CFUnicodeDecomposition.h"
#include "CFStringEncodingConverterPriv.h"
#include "CFICUConverters.h"


CF_INLINE bool __CFIsPlatformConverterAvailable(int encoding) {

#if DEPLOYMENT_TARGET_WINDOWS
    return (IsValidCodePage(CFStringConvertEncodingToWindowsCodepage(encoding)) ? true : false);
#else
    return false;
#endif
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX

static const CFStringEncodingConverter __CFICUBootstrap = {
    NULL /* toBytes */, NULL /* toUnicode */, 6 /* maxBytesPerChar */, 4 /* maxDecomposedCharLen */,
    kCFStringEncodingConverterICU /* encodingClass */,
    NULL /* toBytesLen */, NULL /* toUnicodeLen */, NULL /* toBytesFallback */,
    NULL /* toUnicodeFallback */, NULL /* toBytesPrecompose */, NULL, /* isValidCombiningChar */
};

#endif

static const CFStringEncodingConverter __CFPlatformBootstrap = {
    NULL /* toBytes */, NULL /* toUnicode */, 6 /* maxBytesPerChar */, 4 /* maxDecomposedCharLen */,
    kCFStringEncodingConverterPlatformSpecific /* encodingClass */,
    NULL /* toBytesLen */, NULL /* toUnicodeLen */, NULL /* toBytesFallback */,
    NULL /* toUnicodeFallback */, NULL /* toBytesPrecompose */, NULL, /* isValidCombiningChar */
};

CF_PRIVATE const CFStringEncodingConverter *__CFStringEncodingGetExternalConverter(uint32_t encoding) {

    // we prefer Text Encoding Converter ICU since it's more reliable
    if (__CFIsPlatformConverterAvailable(encoding)) {
        return &__CFPlatformBootstrap;
    } else {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
        if (__CFStringEncodingGetICUName(encoding)) {
            return &__CFICUBootstrap;
        }
#endif
        return NULL;
    }
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
CF_PRIVATE CFStringEncoding *__CFStringEncodingCreateListOfAvailablePlatformConverters(CFAllocatorRef allocator, CFIndex *numberOfConverters) {

    return NULL;
}
#elif DEPLOYMENT_TARGET_WINDOWS
        
#include <tchar.h>

static uint32_t __CFWin32EncodingIndex = 0;
static CFStringEncoding *__CFWin32EncodingList = NULL;

static char CALLBACK __CFWin32EnumCodePageProc(LPTSTR string) {
    uint32_t encoding = CFStringConvertWindowsCodepageToEncoding(_tcstoul(string, NULL, 10));
    CFIndex idx;
    
    if (encoding != kCFStringEncodingInvalidId) { // We list only encodings we know
        if (__CFWin32EncodingList) {
            for (idx = 0;idx < (CFIndex)__CFWin32EncodingIndex;idx++) if (__CFWin32EncodingList[idx] == encoding) break;
            if (idx != __CFWin32EncodingIndex) return true;
            __CFWin32EncodingList[__CFWin32EncodingIndex] = encoding;
        }
        ++__CFWin32EncodingIndex;
    }
    return true;
}

CF_PRIVATE CFStringEncoding *__CFStringEncodingCreateListOfAvailablePlatformConverters(CFAllocatorRef allocator, CFIndex *numberOfConverters) {
    CFStringEncoding *encodings;

    EnumSystemCodePages((CODEPAGE_ENUMPROC)&__CFWin32EnumCodePageProc, CP_INSTALLED);
    __CFWin32EncodingList = (uint32_t *)CFAllocatorAllocate(allocator, sizeof(uint32_t) * __CFWin32EncodingIndex, 0);
    EnumSystemCodePages((CODEPAGE_ENUMPROC)&__CFWin32EnumCodePageProc, CP_INSTALLED);

    *numberOfConverters = __CFWin32EncodingIndex;
    encodings = __CFWin32EncodingList;

    __CFWin32EncodingIndex = 0;
    __CFWin32EncodingList = NULL;

    return encodings;
}
#else
CF_PRIVATE CFStringEncoding *__CFStringEncodingCreateListOfAvailablePlatformConverters(CFAllocatorRef allocator, CFIndex *numberOfConverters) { return NULL; }
#endif

CF_PRIVATE CFIndex __CFStringEncodingPlatformUnicodeToBytes(uint32_t encoding, uint32_t flags, const UniChar *characters, CFIndex numChars, CFIndex *usedCharLen, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {

#if DEPLOYMENT_TARGET_WINDOWS
    WORD dwFlags = 0;
    CFIndex usedLen;

    if ((kCFStringEncodingUTF7 != encoding) && (kCFStringEncodingGB_18030_2000 != encoding) && (0x0800 != (encoding & 0x0F00))) { // not UTF-7/GB18030/ISO-2022-*
        dwFlags |= (flags & (kCFStringEncodingAllowLossyConversion|kCFStringEncodingSubstituteCombinings) ? WC_DEFAULTCHAR : 0);
        dwFlags |= (flags & kCFStringEncodingComposeCombinings ? WC_COMPOSITECHECK : 0);
        dwFlags |= (flags & kCFStringEncodingIgnoreCombinings ? WC_DISCARDNS : 0);
    }

    if ((usedLen = WideCharToMultiByte(CFStringConvertEncodingToWindowsCodepage(encoding), dwFlags, (LPCWSTR)characters, numChars, (LPSTR)bytes, maxByteLen, NULL, NULL)) == 0) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            CPINFO cpInfo;

            if (!GetCPInfo(CFStringConvertEncodingToWindowsCodepage(encoding), &cpInfo)) {
                cpInfo.MaxCharSize = 1; // Is this right ???
            }
            if (cpInfo.MaxCharSize == 1) {
                numChars = maxByteLen;
            } else {
                usedLen = WideCharToMultiByte(CFStringConvertEncodingToWindowsCodepage(encoding), dwFlags, (LPCWSTR)characters, numChars, NULL, 0, NULL, NULL);
                usedLen -= maxByteLen;
                numChars = (numChars > usedLen ? numChars - usedLen : 1);
            }
            if (WideCharToMultiByte(CFStringConvertEncodingToWindowsCodepage(encoding), dwFlags, (LPCWSTR)characters, numChars, (LPSTR)bytes, maxByteLen, NULL, NULL) == 0) {
                if (usedCharLen) *usedCharLen = 0;
                if (usedByteLen) *usedByteLen = 0;
            } else {
                CFIndex lastUsedLen = 0;

                while ((usedLen = WideCharToMultiByte(CFStringConvertEncodingToWindowsCodepage(encoding), dwFlags, (LPCWSTR)characters, ++numChars, (LPSTR)bytes, maxByteLen, NULL, NULL))) lastUsedLen = usedLen;
                if (usedCharLen) *usedCharLen = (numChars - 1);
                if (usedByteLen) *usedByteLen = lastUsedLen;
            }

            return kCFStringEncodingInsufficientOutputBufferLength;
        } else {
            return kCFStringEncodingInvalidInputStream;
        }
    } else {
        if (usedCharLen) *usedCharLen = numChars;
        if (usedByteLen) *usedByteLen = usedLen;
        return kCFStringEncodingConversionSuccess;
    }
#endif /* DEPLOYMENT_TARGET_WINDOWS */

    return kCFStringEncodingConverterUnavailable;
}

CF_PRIVATE CFIndex __CFStringEncodingPlatformBytesToUnicode(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, CFIndex *usedByteLen, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {

#if DEPLOYMENT_TARGET_WINDOWS
    WORD dwFlags = 0;
    CFIndex usedLen;

    if ((kCFStringEncodingUTF7 != encoding) && (kCFStringEncodingGB_18030_2000 != encoding) && (0x0800 != (encoding & 0x0F00))) { // not UTF-7/GB18030/ISO-2022-*
        dwFlags |= (flags & (kCFStringEncodingAllowLossyConversion|kCFStringEncodingSubstituteCombinings) ? 0 : MB_ERR_INVALID_CHARS);
        dwFlags |= (flags & (kCFStringEncodingUseCanonical|kCFStringEncodingUseHFSPlusCanonical) ? MB_COMPOSITE : MB_PRECOMPOSED);
    }

    if ((usedLen = MultiByteToWideChar(CFStringConvertEncodingToWindowsCodepage(encoding), dwFlags, (LPCSTR)bytes, numBytes, (LPWSTR)characters, maxCharLen)) == 0) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            CPINFO cpInfo;

            if (!GetCPInfo(CFStringConvertEncodingToWindowsCodepage(encoding), &cpInfo)) {
                cpInfo.MaxCharSize = 1; // Is this right ???
            }
            if (cpInfo.MaxCharSize == 1) {
                numBytes = maxCharLen;
            } else {
                usedLen = MultiByteToWideChar(CFStringConvertEncodingToWindowsCodepage(encoding), dwFlags, (LPCSTR)bytes, numBytes, (LPWSTR)characters, maxCharLen);
                usedLen -= maxCharLen;
                numBytes = (numBytes > usedLen ? numBytes - usedLen : 1);
            }
            while ((usedLen = MultiByteToWideChar(CFStringConvertEncodingToWindowsCodepage(encoding), dwFlags, (LPCSTR)bytes, numBytes, (LPWSTR)characters, maxCharLen)) == 0) {
                if ((--numBytes) == 0) break;
            }
            if (usedCharLen) *usedCharLen = usedLen;
            if (usedByteLen) *usedByteLen = numBytes;

            return kCFStringEncodingInsufficientOutputBufferLength;
        } else {
            return kCFStringEncodingInvalidInputStream;
        }
    } else {
        if (usedCharLen) *usedCharLen = usedLen;
        if (usedByteLen) *usedByteLen = numBytes;
        return kCFStringEncodingConversionSuccess;
    }
#endif /* DEPLOYMENT_TARGET_WINDOWS */

    return kCFStringEncodingConverterUnavailable;
}

CF_PRIVATE CFIndex __CFStringEncodingPlatformCharLengthForBytes(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes) {
    CFIndex usedCharLen;
    return (__CFStringEncodingPlatformBytesToUnicode(encoding, flags, bytes, numBytes, NULL, NULL, 0, &usedCharLen) == kCFStringEncodingConversionSuccess ? usedCharLen : 0);
}

CF_PRIVATE CFIndex __CFStringEncodingPlatformByteLengthForCharacters(uint32_t encoding, uint32_t flags, const UniChar *characters, CFIndex numChars) {
    CFIndex usedByteLen;
    return (__CFStringEncodingPlatformUnicodeToBytes(encoding, flags, characters, numChars, NULL, NULL, 0, &usedByteLen) == kCFStringEncodingConversionSuccess ? usedByteLen : 0);
}

#undef __CFCarbonCore_GetTextEncodingBase0

