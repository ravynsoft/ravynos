/*	CFPlatformConverters.c
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Foundation Team
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

#if TARGET_OS_WIN32
    return (IsValidCodePage(CFStringConvertEncodingToWindowsCodepage(encoding)) ? true : false);
#else
    return false;
#endif
}

#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX

static const CFStringEncodingConverter __CFICUBootstrap = {
    .toBytes.standard = NULL,
    .toUnicode.standard = NULL,
    .maxBytesPerChar = 6,
    .maxDecomposedCharLen = 4,
    .encodingClass = kCFStringEncodingConverterICU,
    .toBytesLen = NULL,
    .toUnicodeLen = NULL,
    .toBytesFallback = NULL,
    .toUnicodeFallback = NULL,
    .toBytesPrecompose = NULL,
    .isValidCombiningChar = NULL,
};

#endif

static const CFStringEncodingConverter __CFPlatformBootstrap = {
    .toBytes.standard = NULL,
    .toUnicode.standard = NULL,
    .maxBytesPerChar = 6,
    .maxDecomposedCharLen = 4,
    .encodingClass = kCFStringEncodingConverterPlatformSpecific,
    .toBytesLen = NULL,
    .toUnicodeLen = NULL,
    .toBytesFallback = NULL,
    .toUnicodeFallback = NULL,
    .toBytesPrecompose = NULL,
    .isValidCombiningChar = NULL,
};

CF_PRIVATE const CFStringEncodingConverter *__CFStringEncodingGetExternalConverter(uint32_t encoding) {

    // we prefer Text Encoding Converter ICU since it's more reliable
    if (__CFIsPlatformConverterAvailable(encoding)) {
        return &__CFPlatformBootstrap;
    } else {
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_LINUX
        if (__CFStringEncodingGetICUName(encoding)) {
            return &__CFICUBootstrap;
        }
#endif
        return NULL;
    }
}

#if TARGET_OS_MAC
CF_PRIVATE CFStringEncoding *__CFStringEncodingCreateListOfAvailablePlatformConverters(CFAllocatorRef allocator, CFIndex *numberOfConverters) {

    return NULL;
}
#elif TARGET_OS_WIN32
        
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

#if TARGET_OS_WIN32
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
#endif /* TARGET_OS_WIN32 */

    return kCFStringEncodingConverterUnavailable;
}

CF_PRIVATE CFIndex __CFStringEncodingPlatformBytesToUnicode(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, CFIndex *usedByteLen, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {

#if TARGET_OS_WIN32
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
#endif /* TARGET_OS_WIN32 */

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

