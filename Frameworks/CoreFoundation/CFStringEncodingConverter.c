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

/*	CFStringEncodingConverter.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Aki Inoue
*/

#include "CFInternal.h"
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include "CFICUConverters.h"
#include <CoreFoundation/CFUniChar.h>
#include <CoreFoundation/CFPriv.h>
#include "CFUnicodeDecomposition.h"
#include "CFStringEncodingConverterExt.h"
#include "CFStringEncodingConverterPriv.h"
#include <stdlib.h>

typedef CFIndex (*_CFToBytesProc)(const void *converter, uint32_t flags, const UniChar *characters, CFIndex numChars, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen);
typedef CFIndex (*_CFToUnicodeProc)(const void *converter, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen);

typedef struct {
    const CFStringEncodingConverter *definition;
    _CFToBytesProc toBytes;
    _CFToUnicodeProc toUnicode;
    _CFToUnicodeProc toCanonicalUnicode;
    CFStringEncodingToBytesFallbackProc toBytesFallback;
    CFStringEncodingToUnicodeFallbackProc toUnicodeFallback;
} _CFEncodingConverter;

/* Macros
*/
#define TO_BYTE(conv,flags,chars,numChars,bytes,max,used) (conv->toBytes ? conv->toBytes(conv,flags,chars,numChars,bytes,max,used) : ((CFStringEncodingToBytesProc)conv->definition->toBytes)(flags,chars,numChars,bytes,max,used))
#define TO_UNICODE(conv,flags,bytes,numBytes,chars,max,used) (conv->toUnicode ?  (flags & (kCFStringEncodingUseCanonical|kCFStringEncodingUseHFSPlusCanonical) ? conv->toCanonicalUnicode(conv,flags,bytes,numBytes,chars,max,used) : conv->toUnicode(conv,flags,bytes,numBytes,chars,max,used)) : ((CFStringEncodingToUnicodeProc)conv->definition->toUnicode)(flags,bytes,numBytes,chars,max,used))

#define ASCIINewLine 0x0a
#define kSurrogateHighStart 0xD800
#define kSurrogateHighEnd 0xDBFF
#define kSurrogateLowStart 0xDC00
#define kSurrogateLowEnd 0xDFFF

static const uint8_t __CFMaximumConvertedLength = 20;

/* Mapping 128..255 to lossy ASCII
*/
static const struct {
    unsigned char chars[4];
} _toLossyASCIITable[] = {
    {{' ', 0, 0, 0}}, // NO-BREAK SPACE
    {{'!', 0, 0, 0}}, // INVERTED EXCLAMATION MARK
    {{'c', 0, 0, 0}}, // CENT SIGN
    {{'L', 0, 0, 0}}, // POUND SIGN
    {{'$', 0, 0, 0}}, // CURRENCY SIGN
    {{'Y', 0, 0, 0}}, // YEN SIGN
    {{'|', 0, 0, 0}}, // BROKEN BAR
    {{0, 0, 0, 0}}, // SECTION SIGN
    {{0, 0, 0, 0}}, // DIAERESIS
    {{'(', 'C', ')', 0}}, // COPYRIGHT SIGN
    {{'a', 0, 0, 0}}, // FEMININE ORDINAL INDICATOR
    {{'<', '<', 0, 0}}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    {{0, 0, 0, 0}}, // NOT SIGN
    {{'-', 0, 0, 0}}, // SOFT HYPHEN
    {{'(', 'R', ')', 0}}, // REGISTERED SIGN
    {{0, 0, 0, 0}}, // MACRON
    {{0, 0, 0, 0}}, // DEGREE SIGN
    {{'+', '-', 0, 0}}, // PLUS-MINUS SIGN
    {{'2', 0, 0, 0}}, // SUPERSCRIPT TWO
    {{'3', 0, 0, 0}}, // SUPERSCRIPT THREE
    {{0, 0, 0, 0}}, // ACUTE ACCENT
    {{0, 0, 0, 0}}, // MICRO SIGN
    {{0, 0, 0, 0}}, // PILCROW SIGN
    {{0, 0, 0, 0}}, // MIDDLE DOT
    {{0, 0, 0, 0}}, // CEDILLA
    {{'1', 0, 0, 0}}, // SUPERSCRIPT ONE
    {{'o', 0, 0, 0}}, // MASCULINE ORDINAL INDICATOR
    {{'>', '>', 0, 0}}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    {{'1', '/', '4', 0}}, // VULGAR FRACTION ONE QUARTER
    {{'1', '/', '2', 0}}, // VULGAR FRACTION ONE HALF
    {{'3', '/', '4', 0}}, // VULGAR FRACTION THREE QUARTERS
    {{'?', 0, 0, 0}}, // INVERTED QUESTION MARK
    {{'A', 0, 0, 0}}, // LATIN CAPITAL LETTER A WITH GRAVE
    {{'A', 0, 0, 0}}, // LATIN CAPITAL LETTER A WITH ACUTE
    {{'A', 0, 0, 0}}, // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    {{'A', 0, 0, 0}}, // LATIN CAPITAL LETTER A WITH TILDE
    {{'A', 0, 0, 0}}, // LATIN CAPITAL LETTER A WITH DIAERESIS
    {{'A', 0, 0, 0}}, // LATIN CAPITAL LETTER A WITH RING ABOVE
    {{'A', 'E', 0, 0}}, // LATIN CAPITAL LETTER AE
    {{'C', 0, 0, 0}}, // LATIN CAPITAL LETTER C WITH CEDILLA
    {{'E', 0, 0, 0}}, // LATIN CAPITAL LETTER E WITH GRAVE
    {{'E', 0, 0, 0}}, // LATIN CAPITAL LETTER E WITH ACUTE
    {{'E', 0, 0, 0}}, // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
    {{'E', 0, 0, 0}}, // LATIN CAPITAL LETTER E WITH DIAERESIS
    {{'I', 0, 0, 0}}, // LATIN CAPITAL LETTER I WITH GRAVE
    {{'I', 0, 0, 0}}, // LATIN CAPITAL LETTER I WITH ACUTE
    {{'I', 0, 0, 0}}, // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    {{'I', 0, 0, 0}}, // LATIN CAPITAL LETTER I WITH DIAERESIS
    {{'T', 'H', 0, 0}}, // LATIN CAPITAL LETTER ETH (Icelandic)
    {{'N', 0, 0, 0}}, // LATIN CAPITAL LETTER N WITH TILDE
    {{'O', 0, 0, 0}}, // LATIN CAPITAL LETTER O WITH GRAVE
    {{'O', 0, 0, 0}}, // LATIN CAPITAL LETTER O WITH ACUTE
    {{'O', 0, 0, 0}}, // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    {{'O', 0, 0, 0}}, // LATIN CAPITAL LETTER O WITH TILDE
    {{'O', 0, 0, 0}}, // LATIN CAPITAL LETTER O WITH DIAERESIS
    {{'X', 0, 0, 0}}, // MULTIPLICATION SIGN
    {{'O', 0, 0, 0}}, // LATIN CAPITAL LETTER O WITH STROKE
    {{'U', 0, 0, 0}}, // LATIN CAPITAL LETTER U WITH GRAVE
    {{'U', 0, 0, 0}}, // LATIN CAPITAL LETTER U WITH ACUTE
    {{'U', 0, 0, 0}}, // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
    {{'U', 0, 0, 0}}, // LATIN CAPITAL LETTER U WITH DIAERESIS
    {{'Y', 0, 0, 0}}, // LATIN CAPITAL LETTER Y WITH ACUTE
    {{'t', 'h', 0, 0}}, // LATIN CAPITAL LETTER THORN (Icelandic)
    {{'s', 0, 0, 0}}, // LATIN SMALL LETTER SHARP S (German)
    {{'a', 0, 0, 0}}, // LATIN SMALL LETTER A WITH GRAVE
    {{'a', 0, 0, 0}}, // LATIN SMALL LETTER A WITH ACUTE
    {{'a', 0, 0, 0}}, // LATIN SMALL LETTER A WITH CIRCUMFLEX
    {{'a', 0, 0, 0}}, // LATIN SMALL LETTER A WITH TILDE
    {{'a', 0, 0, 0}}, // LATIN SMALL LETTER A WITH DIAERESIS
    {{'a', 0, 0, 0}}, // LATIN SMALL LETTER A WITH RING ABOVE
    {{'a', 'e', 0, 0}}, // LATIN SMALL LETTER AE
    {{'c', 0, 0, 0}}, // LATIN SMALL LETTER C WITH CEDILLA
    {{'e', 0, 0, 0}}, // LATIN SMALL LETTER E WITH GRAVE
    {{'e', 0, 0, 0}}, // LATIN SMALL LETTER E WITH ACUTE
    {{'e', 0, 0, 0}}, // LATIN SMALL LETTER E WITH CIRCUMFLEX
    {{'e', 0, 0, 0}}, // LATIN SMALL LETTER E WITH DIAERESIS
    {{'i', 0, 0, 0}}, // LATIN SMALL LETTER I WITH GRAVE
    {{'i', 0, 0, 0}}, // LATIN SMALL LETTER I WITH ACUTE
    {{'i', 0, 0, 0}}, // LATIN SMALL LETTER I WITH CIRCUMFLEX
    {{'i', 0, 0, 0}}, // LATIN SMALL LETTER I WITH DIAERESIS
    {{'T', 'H', 0, 0}}, // LATIN SMALL LETTER ETH (Icelandic)
    {{'n', 0, 0, 0}}, // LATIN SMALL LETTER N WITH TILDE
    {{'o', 0, 0, 0}}, // LATIN SMALL LETTER O WITH GRAVE
    {{'o', 0, 0, 0}}, // LATIN SMALL LETTER O WITH ACUTE
    {{'o', 0, 0, 0}}, // LATIN SMALL LETTER O WITH CIRCUMFLEX
    {{'o', 0, 0, 0}}, // LATIN SMALL LETTER O WITH TILDE
    {{'o', 0, 0, 0}}, // LATIN SMALL LETTER O WITH DIAERESIS
    {{'/', 0, 0, 0}}, // DIVISION SIGN
    {{'o', 0, 0, 0}}, // LATIN SMALL LETTER O WITH STROKE
    {{'u', 0, 0, 0}}, // LATIN SMALL LETTER U WITH GRAVE
    {{'u', 0, 0, 0}}, // LATIN SMALL LETTER U WITH ACUTE
    {{'u', 0, 0, 0}}, // LATIN SMALL LETTER U WITH CIRCUMFLEX
    {{'u', 0, 0, 0}}, // LATIN SMALL LETTER U WITH DIAERESIS
    {{'y', 0, 0, 0}}, // LATIN SMALL LETTER Y WITH ACUTE
    {{'t', 'h', 0, 0}}, // LATIN SMALL LETTER THORN (Icelandic)
    {{'y', 0, 0, 0}}, // LATIN SMALL LETTER Y WITH DIAERESIS
};

CF_INLINE CFIndex __CFToASCIILatin1Fallback(UniChar character, uint8_t *bytes, CFIndex maxByteLen) {
    const uint8_t *losChars = (const uint8_t*)_toLossyASCIITable + (character - 0xA0) * sizeof(uint8_t[4]);
    CFIndex numBytes = 0;
    CFIndex idx, max = (maxByteLen && (maxByteLen < 4) ? maxByteLen : 4);

    for (idx = 0;idx < max;idx++) {
        if (losChars[idx]) {
            if (maxByteLen) bytes[idx] = losChars[idx];
            ++numBytes;
        } else {
            break;
        }
    }

    return numBytes;
}

static CFIndex __CFDefaultToBytesFallbackProc(const UniChar *characters, CFIndex numChars, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    CFIndex processCharLen = 1, filledBytesLen = 1;
    uint8_t byte = '?';

    if (*characters < 0xA0) { // 0x80 to 0x9F maps to ASCII C0 range
        byte = (uint8_t)(*characters - 0x80);
    } else if (*characters < 0x100) {
        *usedByteLen = __CFToASCIILatin1Fallback(*characters, bytes, maxByteLen);
        return 1;
    } else if (*characters >= kSurrogateHighStart && *characters <= kSurrogateLowEnd) {
        processCharLen = (numChars > 1 && *characters <= kSurrogateLowStart && *(characters + 1) >= kSurrogateLowStart && *(characters + 1) <= kSurrogateLowEnd ? 2 : 1);
    } else if (CFUniCharIsMemberOf(*characters, kCFUniCharWhitespaceCharacterSet)) {
        byte = ' ';
    } else if (CFUniCharIsMemberOf(*characters, kCFUniCharWhitespaceAndNewlineCharacterSet)) {
        byte = ASCIINewLine;
    } else if (*characters == 0x2026) { // ellipsis
        if (0 == maxByteLen) {
            filledBytesLen = 3;
        } else if (maxByteLen > 2) {
            memset(bytes, '.', 3);
            *usedByteLen = 3;
            return processCharLen;
        }
    } else if (CFUniCharIsMemberOf(*characters, kCFUniCharDecomposableCharacterSet)) {
        UTF32Char decomposed[MAX_DECOMPOSED_LENGTH];

        (void)CFUniCharDecomposeCharacter(*characters, decomposed, MAX_DECOMPOSED_LENGTH);
        if (*decomposed < 0x80) {
            byte = (uint8_t)(*decomposed);
        } else {
            UTF16Char theChar = *decomposed;

            return __CFDefaultToBytesFallbackProc(&theChar, 1, bytes, maxByteLen, usedByteLen);
        }
    }
    
    if (maxByteLen) *bytes = byte;
    *usedByteLen = filledBytesLen;
    return processCharLen;
}

static CFIndex __CFDefaultToUnicodeFallbackProc(const uint8_t *bytes, CFIndex numBytes, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {
    if (maxCharLen) *characters = (UniChar)'?';
    *usedCharLen = 1;
    return 1;
}

#define TO_BYTE_FALLBACK(conv,chars,numChars,bytes,max,used) (conv->toBytesFallback(chars,numChars,bytes,max,used))
#define TO_UNICODE_FALLBACK(conv,bytes,numBytes,chars,max,used) (conv->toUnicodeFallback(bytes,numBytes,chars,max,used))

#define EXTRA_BASE (0x0F00)

/* Wrapper funcs for non-standard converters
*/
static CFIndex __CFToBytesCheapEightBitWrapper(const void *converter, uint32_t flags, const UniChar *characters, CFIndex numChars, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    CFIndex processedCharLen = 0;
    CFIndex length = (maxByteLen && (maxByteLen < numChars) ? maxByteLen : numChars);
    uint8_t byte;

    while (processedCharLen < length) {
        if (!((CFStringEncodingCheapEightBitToBytesProc)((const _CFEncodingConverter*)converter)->definition->toBytes)(flags, characters[processedCharLen], &byte)) break;

        if (maxByteLen) bytes[processedCharLen] = byte;
        processedCharLen++;
    }

    *usedByteLen = processedCharLen;
    return processedCharLen;
}

static CFIndex __CFToUnicodeCheapEightBitWrapper(const void *converter, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {
    CFIndex processedByteLen = 0;
    CFIndex length = (maxCharLen && (maxCharLen < numBytes) ? maxCharLen : numBytes);
    UniChar character;

    while (processedByteLen < length) {
        if (!((CFStringEncodingCheapEightBitToUnicodeProc)((const _CFEncodingConverter*)converter)->definition->toUnicode)(flags, bytes[processedByteLen], &character)) break;

        if (maxCharLen) characters[processedByteLen] = character;
        processedByteLen++;
    }

    *usedCharLen = processedByteLen;
    return processedByteLen;
}

static CFIndex __CFToCanonicalUnicodeCheapEightBitWrapper(const void *converter, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {
    CFIndex processedByteLen = 0;
    CFIndex theUsedCharLen = 0;
    UTF32Char charBuffer[MAX_DECOMPOSED_LENGTH];
    CFIndex usedLen;
    UniChar character;
    bool isHFSPlus = (flags & kCFStringEncodingUseHFSPlusCanonical ? true : false);

    while ((processedByteLen < numBytes) && (!maxCharLen || (theUsedCharLen < maxCharLen))) {
        if (!((CFStringEncodingCheapEightBitToUnicodeProc)((const _CFEncodingConverter*)converter)->definition->toUnicode)(flags, bytes[processedByteLen], &character)) break;

        if (CFUniCharIsDecomposableCharacter(character, isHFSPlus)) {
            CFIndex idx;

            usedLen = CFUniCharDecomposeCharacter(character, charBuffer, MAX_DECOMPOSED_LENGTH);
            *usedCharLen = theUsedCharLen;

            for (idx = 0;idx < usedLen;idx++) {
                if (charBuffer[idx] > 0xFFFF) { // Non-BMP
                    if (theUsedCharLen + 2 > maxCharLen)  return processedByteLen;
                    theUsedCharLen += 2;
                    if (maxCharLen) {
                        charBuffer[idx] = charBuffer[idx] - 0x10000;
                        *(characters++) = (UniChar)(charBuffer[idx] >> 10) + 0xD800UL;
                        *(characters++) = (UniChar)(charBuffer[idx] & 0x3FF) + 0xDC00UL;
                    }
                } else {
                    if (theUsedCharLen + 1 > maxCharLen)  return processedByteLen;
                    ++theUsedCharLen;
                    *(characters++) = charBuffer[idx];
                }
            }
        } else {
            if (maxCharLen) *(characters++) = character;
            ++theUsedCharLen;
        }
        processedByteLen++;
    }

    *usedCharLen = theUsedCharLen;
    return processedByteLen;
}

static CFIndex __CFToBytesStandardEightBitWrapper(const void *converter, uint32_t flags, const UniChar *characters, CFIndex numChars, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    CFIndex processedCharLen = 0;
    uint8_t byte;
    CFIndex usedLen;

    *usedByteLen = 0;

    while (numChars && (!maxByteLen || (*usedByteLen < maxByteLen))) {
        if (!(usedLen = ((CFStringEncodingStandardEightBitToBytesProc)((const _CFEncodingConverter*)converter)->definition->toBytes)(flags, characters, numChars, &byte))) break;

        if (maxByteLen) bytes[*usedByteLen] = byte;
        (*usedByteLen)++;
        characters += usedLen;
        numChars -= usedLen;
        processedCharLen += usedLen;
    }

    return processedCharLen;
}

static CFIndex __CFToUnicodeStandardEightBitWrapper(const void *converter, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {
    CFIndex processedByteLen = 0;
    UniChar charBuffer[__CFMaximumConvertedLength];
    CFIndex usedLen;

    *usedCharLen = 0;

    while ((processedByteLen < numBytes) && (!maxCharLen || (*usedCharLen < maxCharLen))) {
        if (!(usedLen = ((CFStringEncodingCheapEightBitToUnicodeProc)((const _CFEncodingConverter*)converter)->definition->toUnicode)(flags, bytes[processedByteLen], charBuffer))) break;

        if (maxCharLen) {
            CFIndex idx;

            if (*usedCharLen + usedLen > maxCharLen) break;

            for (idx = 0;idx < usedLen;idx++) {
                characters[*usedCharLen + idx] = charBuffer[idx];
            }
        }
        *usedCharLen += usedLen;
        processedByteLen++;
    }

    return processedByteLen;
}

static CFIndex __CFToCanonicalUnicodeStandardEightBitWrapper(const void *converter, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {
    CFIndex processedByteLen = 0;
    UniChar charBuffer[__CFMaximumConvertedLength];
    UTF32Char decompBuffer[MAX_DECOMPOSED_LENGTH];
    CFIndex usedLen;
    CFIndex decompedLen;
    CFIndex idx, decompIndex;
    bool isHFSPlus = (flags & kCFStringEncodingUseHFSPlusCanonical ? true : false);
    CFIndex theUsedCharLen = 0;

    while ((processedByteLen < numBytes) && (!maxCharLen || (theUsedCharLen < maxCharLen))) {
        if (!(usedLen = ((CFStringEncodingCheapEightBitToUnicodeProc)((const _CFEncodingConverter*)converter)->definition->toUnicode)(flags, bytes[processedByteLen], charBuffer))) break;

        for (idx = 0;idx < usedLen;idx++) {
            if (CFUniCharIsDecomposableCharacter(charBuffer[idx], isHFSPlus)) {
                decompedLen = CFUniCharDecomposeCharacter(charBuffer[idx], decompBuffer, MAX_DECOMPOSED_LENGTH);
                *usedCharLen = theUsedCharLen;

                for (decompIndex = 0;decompIndex < decompedLen;decompIndex++) {
                    if (decompBuffer[decompIndex] > 0xFFFF) { // Non-BMP
                        if (theUsedCharLen + 2 > maxCharLen)  return processedByteLen;
                        theUsedCharLen += 2;
                        if (maxCharLen) {
                            charBuffer[idx] = charBuffer[idx] - 0x10000;
                            *(characters++) = (charBuffer[idx] >> 10) + 0xD800UL;
                            *(characters++) = (charBuffer[idx] & 0x3FF) + 0xDC00UL;
                        }
                    } else {
                        if (theUsedCharLen + 1 > maxCharLen)  return processedByteLen;
                        ++theUsedCharLen;
                        *(characters++) = charBuffer[idx];
                    }
                }
            } else {
                if (maxCharLen) *(characters++) = charBuffer[idx];
                ++theUsedCharLen;
            }
        }
        processedByteLen++;
    }

    *usedCharLen = theUsedCharLen;
    return processedByteLen;
}

static CFIndex __CFToBytesCheapMultiByteWrapper(const void *converter, uint32_t flags, const UniChar *characters, CFIndex numChars, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    CFIndex processedCharLen = 0;
    uint8_t byteBuffer[__CFMaximumConvertedLength];
    CFIndex usedLen;

    *usedByteLen = 0;

    while ((processedCharLen < numChars) && (!maxByteLen || (*usedByteLen < maxByteLen))) {
        if (!(usedLen = ((CFStringEncodingCheapMultiByteToBytesProc)((const _CFEncodingConverter*)converter)->definition->toBytes)(flags, characters[processedCharLen], byteBuffer))) break;

        if (maxByteLen) {
            CFIndex idx;

            if (*usedByteLen + usedLen > maxByteLen) break;

            for (idx = 0;idx <usedLen;idx++) {
                bytes[*usedByteLen + idx] = byteBuffer[idx];
            }
        }

        *usedByteLen += usedLen;
        processedCharLen++;
    }

    return processedCharLen;
}

static CFIndex __CFToUnicodeCheapMultiByteWrapper(const void *converter, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {
    CFIndex processedByteLen = 0;
    UniChar character;
    CFIndex usedLen;

    *usedCharLen = 0;

    while (numBytes && (!maxCharLen || (*usedCharLen < maxCharLen))) {
        if (!(usedLen = ((CFStringEncodingCheapMultiByteToUnicodeProc)((const _CFEncodingConverter*)converter)->definition->toUnicode)(flags, bytes, numBytes, &character))) break;

        if (maxCharLen) *(characters++) = character;
        (*usedCharLen)++;
        processedByteLen += usedLen;
        bytes += usedLen;
        numBytes -= usedLen;
    }

    return processedByteLen;
}

static CFIndex __CFToCanonicalUnicodeCheapMultiByteWrapper(const void *converter, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {
    CFIndex processedByteLen = 0;
    UTF32Char charBuffer[MAX_DECOMPOSED_LENGTH];
    UniChar character;
    CFIndex usedLen;
    CFIndex decomposedLen;
    CFIndex theUsedCharLen = 0;
    bool isHFSPlus = (flags & kCFStringEncodingUseHFSPlusCanonical ? true : false);

    while (numBytes && (!maxCharLen || (theUsedCharLen < maxCharLen))) {
        if (!(usedLen = ((CFStringEncodingCheapMultiByteToUnicodeProc)((const _CFEncodingConverter*)converter)->definition->toUnicode)(flags, bytes, numBytes, &character))) break;

        if (CFUniCharIsDecomposableCharacter(character, isHFSPlus)) {
            CFIndex idx;

            decomposedLen = CFUniCharDecomposeCharacter(character, charBuffer, MAX_DECOMPOSED_LENGTH);
            *usedCharLen = theUsedCharLen;

            for (idx = 0;idx < decomposedLen;idx++) {
                if (charBuffer[idx] > 0xFFFF) { // Non-BMP
                    if (theUsedCharLen + 2 > maxCharLen)  return processedByteLen;
                    theUsedCharLen += 2;
                    if (maxCharLen) {
                        charBuffer[idx] = charBuffer[idx] - 0x10000;
                        *(characters++) = (UniChar)(charBuffer[idx] >> 10) + 0xD800UL;
                        *(characters++) = (UniChar)(charBuffer[idx] & 0x3FF) + 0xDC00UL;
                    }
                } else {
                    if (theUsedCharLen + 1 > maxCharLen)  return processedByteLen;
                    ++theUsedCharLen;
                    *(characters++) = charBuffer[idx];
                }
            }
        } else {
            if (maxCharLen) *(characters++) = character;
            ++theUsedCharLen;
        }

        processedByteLen += usedLen;
        bytes += usedLen;
        numBytes -= usedLen;
    }
    *usedCharLen = theUsedCharLen;
    return processedByteLen;
}

/* static functions
*/
CF_INLINE _CFEncodingConverter *__CFEncodingConverterFromDefinition(const CFStringEncodingConverter *definition, CFStringEncoding encoding) {
#define NUM_OF_ENTRIES_CYCLE (10)
    static uint32_t _currentIndex = 0;
    static uint32_t _allocatedSize = 0;
    static _CFEncodingConverter *_allocatedEntries = NULL;
    _CFEncodingConverter *converter;


    if ((_currentIndex + 1) >= _allocatedSize) {
        _currentIndex = 0;
        _allocatedSize = 0;
        _allocatedEntries = NULL;
    }
    if (_allocatedEntries == NULL) { // Not allocated yet
        _allocatedEntries = (_CFEncodingConverter *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(_CFEncodingConverter) * NUM_OF_ENTRIES_CYCLE, 0);
        _allocatedSize = NUM_OF_ENTRIES_CYCLE;
        converter = &(_allocatedEntries[_currentIndex]);
    } else {
        converter = &(_allocatedEntries[++_currentIndex]);
    }

    memset(converter, 0, sizeof(_CFEncodingConverter));

    converter->definition = definition;

    switch (definition->encodingClass) {
        case kCFStringEncodingConverterStandard:
            converter->toBytes = NULL;
            converter->toUnicode = NULL;
            converter->toCanonicalUnicode = NULL;
            break;

        case kCFStringEncodingConverterCheapEightBit:
            converter->toBytes = __CFToBytesCheapEightBitWrapper;
            converter->toUnicode = __CFToUnicodeCheapEightBitWrapper;
            converter->toCanonicalUnicode = __CFToCanonicalUnicodeCheapEightBitWrapper;
            break;

        case kCFStringEncodingConverterStandardEightBit:
            converter->toBytes = __CFToBytesStandardEightBitWrapper;
            converter->toUnicode = __CFToUnicodeStandardEightBitWrapper;
            converter->toCanonicalUnicode = __CFToCanonicalUnicodeStandardEightBitWrapper;
            break;

        case kCFStringEncodingConverterCheapMultiByte:
            converter->toBytes = __CFToBytesCheapMultiByteWrapper;
            converter->toUnicode = __CFToUnicodeCheapMultiByteWrapper;
            converter->toCanonicalUnicode = __CFToCanonicalUnicodeCheapMultiByteWrapper;
            break;

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
        case kCFStringEncodingConverterICU:
            converter->toBytes = (_CFToBytesProc)__CFStringEncodingGetICUName(encoding);
            break;
#endif

        case kCFStringEncodingConverterPlatformSpecific:
            break;
            
        default: // Shouln't be here
            return NULL;
    }

    converter->toBytesFallback = (definition->toBytesFallback ? definition->toBytesFallback : __CFDefaultToBytesFallbackProc);
    converter->toUnicodeFallback = (definition->toUnicodeFallback ? definition->toUnicodeFallback : __CFDefaultToUnicodeFallbackProc);

    return converter;
}

CF_INLINE const CFStringEncodingConverter *__CFStringEncodingConverterGetDefinition(CFStringEncoding encoding) {
    switch (encoding) {
        case kCFStringEncodingUTF8:
            return &__CFConverterUTF8;

        case kCFStringEncodingMacRoman:
            return &__CFConverterMacRoman;
            
        case kCFStringEncodingWindowsLatin1:
            return &__CFConverterWinLatin1;

        case kCFStringEncodingASCII:
            return &__CFConverterASCII;

        case kCFStringEncodingISOLatin1:
            return &__CFConverterISOLatin1;


        case kCFStringEncodingNextStepLatin:
            return &__CFConverterNextStepLatin;


        default:
            return __CFStringEncodingGetExternalConverter(encoding);
    }
}

static const _CFEncodingConverter *__CFGetConverter(uint32_t encoding) {
    const _CFEncodingConverter *converter = NULL;
    const _CFEncodingConverter **commonConverterSlot = NULL;
    static _CFEncodingConverter *commonConverters[3] = {NULL, NULL, NULL}; // UTF8, MacRoman/WinLatin1, and the default encoding*
    static CFMutableDictionaryRef mappingTable = NULL;
    static OSSpinLock lock = OS_SPINLOCK_INIT;

    switch (encoding) {
	case kCFStringEncodingUTF8: commonConverterSlot = (const _CFEncodingConverter **)&(commonConverters[0]); break;

	    /* the swith here should avoid possible bootstrap issues in the default: case below when invoked from CFStringGetSystemEncoding() */
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
	case kCFStringEncodingMacRoman: commonConverterSlot = (const _CFEncodingConverter **)&(commonConverters[1]); break;
#elif DEPLOYMENT_TARGET_WINDOWS
	case kCFStringEncodingWindowsLatin1: commonConverterSlot = (const _CFEncodingConverter **)(&(commonConverters[1])); break;
#else
#warning This case must match __defaultEncoding value defined in CFString.c
	case kCFStringEncodingISOLatin1: commonConverterSlot = (const _CFEncodingConverter **)(&(commonConverters[1])); break;
#endif

	default: if (CFStringGetSystemEncoding() == encoding) commonConverterSlot = (const _CFEncodingConverter **)&(commonConverters[2]); break;
    }

    OSSpinLockLock(&lock);
    converter = ((NULL == commonConverterSlot) ? ((NULL == mappingTable) ? NULL : (const _CFEncodingConverter *)CFDictionaryGetValue(mappingTable, (const void *)(uintptr_t)encoding)) : *commonConverterSlot);
    OSSpinLockUnlock(&lock);

    if (NULL == converter) {
        const CFStringEncodingConverter *definition = __CFStringEncodingConverterGetDefinition(encoding);

        if (NULL != definition) {
            OSSpinLockLock(&lock);
            converter = ((NULL == commonConverterSlot) ? ((NULL == mappingTable) ? NULL : (const _CFEncodingConverter *)CFDictionaryGetValue(mappingTable, (const void *)(uintptr_t)encoding)) : *commonConverterSlot);

            if (NULL == converter) {
                converter = __CFEncodingConverterFromDefinition(definition, encoding);

		if (NULL == commonConverterSlot) {
		    if (NULL == mappingTable) mappingTable = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);

		    CFDictionarySetValue(mappingTable, (const void *)(uintptr_t)encoding, converter);
		} else {
		    *commonConverterSlot = converter;
		}
            }
            OSSpinLockUnlock(&lock);
        }
    }

    return converter;
}

/* Public API
*/
uint32_t CFStringEncodingUnicodeToBytes(uint32_t encoding, uint32_t flags, const UniChar *characters, CFIndex numChars, CFIndex *usedCharLen, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    if (encoding == kCFStringEncodingUTF8) {
        static CFStringEncodingToBytesProc __CFToUTF8 = NULL;
        CFIndex convertedCharLen;
        CFIndex usedLen;


        if ((flags & kCFStringEncodingUseCanonical) || (flags & kCFStringEncodingUseHFSPlusCanonical)) {
            (void)CFUniCharDecompose(characters, numChars, &convertedCharLen, (void *)bytes, maxByteLen, &usedLen, true, kCFUniCharUTF8Format, (flags & kCFStringEncodingUseHFSPlusCanonical ? true : false));
        } else {
            if (!__CFToUTF8) {
                const CFStringEncodingConverter *utf8Converter = CFStringEncodingGetConverter(kCFStringEncodingUTF8);
                __CFToUTF8 = (CFStringEncodingToBytesProc)utf8Converter->toBytes;
            }
            convertedCharLen = __CFToUTF8(0, characters, numChars, bytes, maxByteLen, &usedLen);
        }
        if (usedCharLen) *usedCharLen = convertedCharLen;
        if (usedByteLen) *usedByteLen = usedLen;

        if (convertedCharLen == numChars) {
            return kCFStringEncodingConversionSuccess;
        } else if ((maxByteLen > 0) && ((maxByteLen - usedLen) < 10)) { // could be filled outbuf
            UTF16Char character = characters[convertedCharLen];
            
            if (((character >= kSurrogateLowStart) && (character <= kSurrogateLowEnd)) || ((character >= kSurrogateHighStart) && (character <= kSurrogateHighEnd) && ((1 == (numChars - convertedCharLen)) || (characters[convertedCharLen + 1] < kSurrogateLowStart) || (characters[convertedCharLen + 1] > kSurrogateLowEnd)))) return kCFStringEncodingInvalidInputStream;
            
            return kCFStringEncodingInsufficientOutputBufferLength;
        } else {
            return kCFStringEncodingInvalidInputStream;
        }
    } else {
        const _CFEncodingConverter *converter = __CFGetConverter(encoding);
        CFIndex usedLen = 0;
        CFIndex localUsedByteLen;
        CFIndex theUsedByteLen = 0;
        uint32_t theResult = kCFStringEncodingConversionSuccess;
        CFStringEncodingToBytesPrecomposeProc toBytesPrecompose = NULL;
        CFStringEncodingIsValidCombiningCharacterProc isValidCombiningChar = NULL;

        if (!converter) return kCFStringEncodingConverterUnavailable;

        if (flags & kCFStringEncodingSubstituteCombinings) {
            if (!(flags & kCFStringEncodingAllowLossyConversion)) isValidCombiningChar = converter->definition->isValidCombiningChar;
       } else {
            isValidCombiningChar = converter->definition->isValidCombiningChar;
            if (!(flags & kCFStringEncodingIgnoreCombinings)) {
                toBytesPrecompose = converter->definition->toBytesPrecompose;
                flags |= kCFStringEncodingComposeCombinings;
            }
        }

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
        if (kCFStringEncodingConverterICU == converter->definition->encodingClass) return __CFStringEncodingICUToBytes((const char *)converter->toBytes, flags, characters, numChars, usedCharLen, bytes, maxByteLen, usedByteLen);
#endif

        /* Platform converter */
        if (kCFStringEncodingConverterPlatformSpecific == converter->definition->encodingClass) return __CFStringEncodingPlatformUnicodeToBytes(encoding, flags, characters, numChars, usedCharLen, bytes, maxByteLen, usedByteLen);

        while ((usedLen < numChars) && (!maxByteLen || (theUsedByteLen < maxByteLen))) {
            if ((usedLen += TO_BYTE(converter, flags, characters + usedLen, numChars - usedLen, bytes + theUsedByteLen, (maxByteLen ? maxByteLen - theUsedByteLen : 0), &localUsedByteLen)) < numChars) {
                CFIndex dummy;

                if (isValidCombiningChar && (usedLen > 0) && isValidCombiningChar(characters[usedLen])) {
                    if (toBytesPrecompose) {
                        CFIndex localUsedLen = usedLen;

                        while (isValidCombiningChar(characters[--usedLen]));
                        theUsedByteLen += localUsedByteLen;
                        if (converter->definition->maxBytesPerChar > 1) {
                            TO_BYTE(converter, flags, characters + usedLen, localUsedLen - usedLen, NULL, 0, &localUsedByteLen);
                            theUsedByteLen -= localUsedByteLen;
                        } else {
                            theUsedByteLen--;
                        }
                        if ((localUsedLen = toBytesPrecompose(flags, characters + usedLen, numChars - usedLen, bytes + theUsedByteLen, (maxByteLen ? maxByteLen - theUsedByteLen : 0), &localUsedByteLen)) > 0) {
                            usedLen += localUsedLen;
                            if ((usedLen < numChars) && isValidCombiningChar(characters[usedLen])) { // There is a non-base char not combined remaining
                                theUsedByteLen += localUsedByteLen;
                                theResult = kCFStringEncodingInvalidInputStream;
                                break;
                            }
                        } else if (flags & kCFStringEncodingAllowLossyConversion) {
                            uint8_t lossyByte = CFStringEncodingMaskToLossyByte(flags);

                            if (lossyByte) {
                                while (isValidCombiningChar(characters[++usedLen]));
                                localUsedByteLen = 1;
                                if (maxByteLen) *(bytes + theUsedByteLen) = lossyByte;
                            } else {
                                ++usedLen;
                                usedLen += TO_BYTE_FALLBACK(converter, characters + usedLen, numChars - usedLen, bytes + theUsedByteLen, (maxByteLen ? maxByteLen - theUsedByteLen : 0), &localUsedByteLen);
                            }
                        } else {
                            theResult = kCFStringEncodingInvalidInputStream;
                            break;
                        }
                    } else if (maxByteLen && ((maxByteLen == theUsedByteLen + localUsedByteLen) || TO_BYTE(converter, flags, characters + usedLen, numChars - usedLen, NULL, 0, &dummy))) { // buffer was filled up
                                    theUsedByteLen += localUsedByteLen;
                                    theResult = kCFStringEncodingInsufficientOutputBufferLength;
                                    break;
                    } else if (flags & kCFStringEncodingIgnoreCombinings) {
                        while ((++usedLen < numChars) && isValidCombiningChar(characters[usedLen]));
                    } else {
                        uint8_t lossyByte = CFStringEncodingMaskToLossyByte(flags);

                        theUsedByteLen += localUsedByteLen;
                        if (lossyByte) {
                            ++usedLen;
                            localUsedByteLen = 1;
                            if (maxByteLen) *(bytes + theUsedByteLen) = lossyByte;
                        } else {
                            usedLen += TO_BYTE_FALLBACK(converter, characters + usedLen, numChars - usedLen, bytes + theUsedByteLen, (maxByteLen ? maxByteLen - theUsedByteLen : 0), &localUsedByteLen);
                        }
                    }
                } else if (maxByteLen && ((maxByteLen == theUsedByteLen + localUsedByteLen) || TO_BYTE(converter, flags, characters + usedLen, numChars - usedLen, NULL, 0, &dummy))) { // buffer was filled up
                    theUsedByteLen += localUsedByteLen;

                    if (flags & kCFStringEncodingAllowLossyConversion && !CFStringEncodingMaskToLossyByte(flags)) {
                        CFIndex localUsedLen;

                        localUsedByteLen = 0;
                        // after the buffer is full, we still try out all the rest of the characters
                        // if all characters cannot be converted, we mark the result as insufficient output buffer
                        while ((usedLen < numChars) && !localUsedByteLen && (localUsedLen = TO_BYTE_FALLBACK(converter, characters + usedLen, numChars - usedLen, NULL, 0, &localUsedByteLen))) {
                            if (localUsedByteLen == 0) {
                                usedLen += localUsedLen;
                            }
                        }
                    }
                    if (usedLen < numChars) theResult = kCFStringEncodingInsufficientOutputBufferLength;
                    break;
                } else if (flags & kCFStringEncodingAllowLossyConversion) {
                    uint8_t lossyByte = CFStringEncodingMaskToLossyByte(flags);

                    theUsedByteLen += localUsedByteLen;
                    if (lossyByte) {
                        ++usedLen;
                        localUsedByteLen = 1;
                        if (maxByteLen) *(bytes + theUsedByteLen) = lossyByte;
                    } else {
                        usedLen += TO_BYTE_FALLBACK(converter, characters + usedLen, numChars - usedLen, bytes + theUsedByteLen, (maxByteLen ? maxByteLen - theUsedByteLen : 0), &localUsedByteLen);
                    }
                } else {
                    theUsedByteLen += localUsedByteLen;
                    theResult = kCFStringEncodingInvalidInputStream;
                    break;
                }
            }
            theUsedByteLen += localUsedByteLen;
        }

        if (usedLen < numChars && maxByteLen && theResult == kCFStringEncodingConversionSuccess) {
            if (flags & kCFStringEncodingAllowLossyConversion && !CFStringEncodingMaskToLossyByte(flags)) {
                CFIndex localUsedLen;

                localUsedByteLen = 0;
                while ((usedLen < numChars) && !localUsedByteLen && (localUsedLen = TO_BYTE_FALLBACK(converter, characters + usedLen, numChars - usedLen, NULL, 0, &localUsedByteLen))) {
                    if (!localUsedByteLen) {
                        usedLen += localUsedLen;
                    }
                }
            }
            if (usedLen < numChars) theResult = kCFStringEncodingInsufficientOutputBufferLength;
        }
        if (usedByteLen) *usedByteLen = theUsedByteLen;
        if (usedCharLen) *usedCharLen = usedLen;

        return theResult;
    }
}

uint32_t CFStringEncodingBytesToUnicode(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, CFIndex *usedByteLen, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {
    const _CFEncodingConverter *converter = __CFGetConverter(encoding);
    CFIndex usedLen = 0;
    CFIndex theUsedCharLen = 0;
    CFIndex localUsedCharLen;
    uint32_t theResult = kCFStringEncodingConversionSuccess;

    if (!converter) return kCFStringEncodingConverterUnavailable;

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    if (kCFStringEncodingConverterICU == converter->definition->encodingClass) return __CFStringEncodingICUToUnicode((const char *)converter->toBytes, flags, bytes, numBytes, usedByteLen, characters, maxCharLen, usedCharLen);
#endif

    /* Platform converter */
    if (kCFStringEncodingConverterPlatformSpecific == converter->definition->encodingClass) return __CFStringEncodingPlatformBytesToUnicode(encoding, flags, bytes, numBytes, usedByteLen, characters, maxCharLen, usedCharLen);

    while ((usedLen < numBytes) && (!maxCharLen || (theUsedCharLen < maxCharLen))) {
        if ((usedLen += TO_UNICODE(converter, flags, bytes + usedLen, numBytes - usedLen, characters + theUsedCharLen, (maxCharLen ? maxCharLen - theUsedCharLen : 0), &localUsedCharLen)) < numBytes) {
            CFIndex tempUsedCharLen;

            if (maxCharLen && ((maxCharLen == theUsedCharLen + localUsedCharLen) || (((flags & (kCFStringEncodingUseCanonical|kCFStringEncodingUseHFSPlusCanonical)) || (maxCharLen == theUsedCharLen + localUsedCharLen + 1)) && TO_UNICODE(converter, flags, bytes + usedLen, numBytes - usedLen, NULL, 0, &tempUsedCharLen)))) { // buffer was filled up
                theUsedCharLen += localUsedCharLen;
                theResult = kCFStringEncodingInsufficientOutputBufferLength;
                break;
            } else if (flags & kCFStringEncodingAllowLossyConversion) {
                theUsedCharLen += localUsedCharLen;
                usedLen += TO_UNICODE_FALLBACK(converter, bytes + usedLen, numBytes - usedLen, characters + theUsedCharLen, (maxCharLen ? maxCharLen - theUsedCharLen : 0), &localUsedCharLen);
            } else {
                theUsedCharLen += localUsedCharLen;
                theResult = kCFStringEncodingInvalidInputStream;
                break;
            }
        }
        theUsedCharLen += localUsedCharLen;
    }

    if (usedLen < numBytes && maxCharLen && theResult == kCFStringEncodingConversionSuccess) {
        theResult = kCFStringEncodingInsufficientOutputBufferLength;
    }
    if (usedCharLen) *usedCharLen = theUsedCharLen;
    if (usedByteLen) *usedByteLen = usedLen;

    return theResult;
}

CF_PRIVATE bool CFStringEncodingIsValidEncoding(uint32_t encoding) {
    return (CFStringEncodingGetConverter(encoding) ? true : false);
}

CF_PRIVATE CFIndex CFStringEncodingCharLengthForBytes(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes) {
    const _CFEncodingConverter *converter = __CFGetConverter(encoding);

    if (converter) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
        if (kCFStringEncodingConverterICU == converter->definition->encodingClass) return __CFStringEncodingICUCharLength((const char *)converter->toBytes, flags, bytes, numBytes);
#endif
        
        if (kCFStringEncodingConverterPlatformSpecific == converter->definition->encodingClass) return __CFStringEncodingPlatformCharLengthForBytes(encoding, flags, bytes, numBytes);

        if (1 == converter->definition->maxBytesPerChar) return numBytes;

        if (NULL == converter->definition->toUnicodeLen) {
            CFIndex usedByteLen = 0;
            CFIndex totalLength = 0;
            CFIndex usedCharLen;

            while (numBytes > 0) {
                usedByteLen = TO_UNICODE(converter, flags, bytes, numBytes, NULL, 0, &usedCharLen);

                bytes += usedByteLen;
                numBytes -= usedByteLen;
                totalLength += usedCharLen;

                if (numBytes > 0) {
                    if (0 == (flags & kCFStringEncodingAllowLossyConversion)) return 0;

                    usedByteLen = TO_UNICODE_FALLBACK(converter, bytes, numBytes, NULL, 0, &usedCharLen);

                    bytes += usedByteLen;
                    numBytes -= usedByteLen;
                    totalLength += usedCharLen;
                }
            }

            return totalLength;
        } else {
            return converter->definition->toUnicodeLen(flags, bytes, numBytes);
        }
    }

    return 0;
}

CF_PRIVATE CFIndex CFStringEncodingByteLengthForCharacters(uint32_t encoding, uint32_t flags, const UniChar *characters, CFIndex numChars) {
    const _CFEncodingConverter *converter = __CFGetConverter(encoding);

    if (converter) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
        if (kCFStringEncodingConverterICU == converter->definition->encodingClass) return __CFStringEncodingICUByteLength((const char *)converter->toBytes, flags, characters, numChars);
#endif

        if (kCFStringEncodingConverterPlatformSpecific == converter->definition->encodingClass) return __CFStringEncodingPlatformByteLengthForCharacters(encoding, flags, characters, numChars);

        if (1 == converter->definition->maxBytesPerChar) return numChars;

        if (NULL == converter->definition->toBytesLen) {
            CFIndex usedByteLen;

            return ((kCFStringEncodingConversionSuccess == CFStringEncodingUnicodeToBytes(encoding, flags, characters, numChars, NULL, NULL, 0, &usedByteLen)) ? usedByteLen : 0);
        } else {
            return converter->definition->toBytesLen(flags, characters, numChars);
        }
    }

    return 0;
}

void CFStringEncodingRegisterFallbackProcedures(uint32_t encoding, CFStringEncodingToBytesFallbackProc toBytes, CFStringEncodingToUnicodeFallbackProc toUnicode) {
    _CFEncodingConverter *converter = (_CFEncodingConverter *)__CFGetConverter(encoding);

    if (NULL != converter) {
       const CFStringEncodingConverter *body = CFStringEncodingGetConverter(encoding);

        converter->toBytesFallback = ((NULL == toBytes) ? ((NULL == body) ? __CFDefaultToBytesFallbackProc : body->toBytesFallback) : toBytes);
        converter->toUnicodeFallback = ((NULL == toUnicode) ? ((NULL == body) ? __CFDefaultToUnicodeFallbackProc : body->toUnicodeFallback) : toUnicode);
    }
}

CF_PRIVATE const CFStringEncodingConverter *CFStringEncodingGetConverter(uint32_t encoding) {
    const _CFEncodingConverter *converter = __CFGetConverter(encoding);

    return ((NULL == converter) ? NULL : converter->definition);
}

static const CFStringEncoding __CFBuiltinEncodings[] = {
    kCFStringEncodingMacRoman,
    kCFStringEncodingWindowsLatin1,
    kCFStringEncodingISOLatin1,
    kCFStringEncodingNextStepLatin,
    kCFStringEncodingASCII,
    kCFStringEncodingUTF8,
    /* These seven are available only in CFString-level */
    kCFStringEncodingNonLossyASCII,

    kCFStringEncodingUTF16,
    kCFStringEncodingUTF16BE,
    kCFStringEncodingUTF16LE,

    kCFStringEncodingUTF32,
    kCFStringEncodingUTF32BE,
    kCFStringEncodingUTF32LE,

    kCFStringEncodingInvalidId,
};

static CFComparisonResult __CFStringEncodingComparator(const void *v1, const void *v2, void *context) {
    CFComparisonResult val1 = (*(const CFStringEncoding *)v1) & 0xFFFF;
    CFComparisonResult val2 = (*(const CFStringEncoding *)v2) & 0xFFFF;

    return ((val1 == val2) ? ((CFComparisonResult)(*(const CFStringEncoding *)v1) - (CFComparisonResult)(*(const CFStringEncoding *)v2)) : val1 - val2);
}

static void __CFStringEncodingFliterDupes(CFStringEncoding *encodings, CFIndex numSlots) {
    CFStringEncoding last = kCFStringEncodingInvalidId;
    const CFStringEncoding *limitEncodings = encodings + numSlots;

    while (encodings < limitEncodings) {
        if (last == *encodings) {
            if ((encodings + 1) < limitEncodings) memmove(encodings, encodings + 1, sizeof(CFStringEncoding) * (limitEncodings - encodings - 1));
            --limitEncodings;
        } else {
            last = *(encodings++);
        }
    }
}

CF_PRIVATE const CFStringEncoding *CFStringEncodingListOfAvailableEncodings(void) {
    static const CFStringEncoding *encodings = NULL;

    if (NULL == encodings) {
        CFStringEncoding *list = (CFStringEncoding *)__CFBuiltinEncodings;
        CFIndex numICUConverters = 0, numPlatformConverters = 0;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
        CFStringEncoding *icuConverters = __CFStringEncodingCreateICUEncodings(NULL, &numICUConverters);
#else
        CFStringEncoding *icuConverters = NULL;
#endif
        CFStringEncoding *platformConverters = __CFStringEncodingCreateListOfAvailablePlatformConverters(NULL, &numPlatformConverters);

        if ((NULL != icuConverters) || (NULL != platformConverters)) {
            CFIndex numSlots = (sizeof(__CFBuiltinEncodings) / sizeof(*__CFBuiltinEncodings)) + numICUConverters + numPlatformConverters;

            list = (CFStringEncoding *)CFAllocatorAllocate(NULL, sizeof(CFStringEncoding) * numSlots, 0);

            memcpy(list, __CFBuiltinEncodings, sizeof(__CFBuiltinEncodings));

            if (NULL != icuConverters) {
                memcpy(list + (sizeof(__CFBuiltinEncodings) / sizeof(*__CFBuiltinEncodings)), icuConverters, sizeof(CFStringEncoding) * numICUConverters);
                CFAllocatorDeallocate(NULL, icuConverters);
            }

            if (NULL != platformConverters) {
                memcpy(list + (sizeof(__CFBuiltinEncodings) / sizeof(*__CFBuiltinEncodings)) + numICUConverters, platformConverters, sizeof(CFStringEncoding) * numPlatformConverters);
                CFAllocatorDeallocate(NULL, platformConverters);
            }

            CFQSortArray(list, numSlots, sizeof(CFStringEncoding), (CFComparatorFunction)__CFStringEncodingComparator, NULL);
            __CFStringEncodingFliterDupes(list, numSlots);
        }
        if (!OSAtomicCompareAndSwapPtrBarrier(NULL, list, (void * volatile *)&encodings) && (list != __CFBuiltinEncodings)) CFAllocatorDeallocate(NULL, list);
    }

    return encodings;
}

#undef TO_BYTE
#undef TO_UNICODE
#undef ASCIINewLine
#undef kSurrogateHighStart
#undef kSurrogateHighEnd
#undef kSurrogateLowStart
#undef kSurrogateLowEnd
#undef TO_BYTE_FALLBACK
#undef TO_UNICODE_FALLBACK
#undef EXTRA_BASE
#undef NUM_OF_ENTRIES_CYCLE

