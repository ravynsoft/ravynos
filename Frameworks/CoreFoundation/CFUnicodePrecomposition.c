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

/*	CFUnicodePrecomposition.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: Aki Inoue
*/

#include <string.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFCharacterSet.h>
#include "CFUniChar.h"
#include "CFUnicodePrecomposition.h"
#include "CFInternal.h"
#include "CFUniCharPriv.h"

// Canonical Precomposition
static UTF32Char *__CFUniCharPrecompSourceTable = NULL;
static uint32_t __CFUniCharPrecompositionTableLength = 0;
static uint16_t *__CFUniCharBMPPrecompDestinationTable = NULL;
static uint32_t *__CFUniCharNonBMPPrecompDestinationTable = NULL;

static const uint8_t *__CFUniCharNonBaseBitmapForBMP_P = NULL; // Adding _P so the symbol name is different from the one in CFUnicodeDecomposition.c
static const uint8_t *__CFUniCharCombiningClassForBMP = NULL;

static CFLock_t __CFUniCharPrecompositionTableLock = CFLockInit;

static void __CFUniCharLoadPrecompositionTable(void) {

    __CFLock(&__CFUniCharPrecompositionTableLock);

    if (NULL == __CFUniCharPrecompSourceTable) {
        const uint32_t *bytes = (const uint32_t *)CFUniCharGetMappingData(kCFUniCharCanonicalPrecompMapping);
        uint32_t bmpMappingLength;

        if (NULL == bytes) {
            __CFUnlock(&__CFUniCharPrecompositionTableLock);
            return;
        }

        __CFUniCharPrecompositionTableLength = *(bytes++);
        bmpMappingLength = *(bytes++);
        __CFUniCharPrecompSourceTable = (UTF32Char *)bytes;
        __CFUniCharBMPPrecompDestinationTable = (uint16_t *)((intptr_t)bytes + (__CFUniCharPrecompositionTableLength * sizeof(UTF32Char) * 2));
        __CFUniCharNonBMPPrecompDestinationTable = (uint32_t *)(((intptr_t)__CFUniCharBMPPrecompDestinationTable) + bmpMappingLength);

        __CFUniCharNonBaseBitmapForBMP_P = CFUniCharGetBitmapPtrForPlane(kCFUniCharNonBaseCharacterSet, 0);
        __CFUniCharCombiningClassForBMP = (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, 0);
    }

    __CFUnlock(&__CFUniCharPrecompositionTableLock);
}

 // Adding _P so the symbol name is different from the one in CFUnicodeDecomposition.c
#define __CFUniCharIsNonBaseCharacter	__CFUniCharIsNonBaseCharacter_P
CF_INLINE bool __CFUniCharIsNonBaseCharacter(UTF32Char character) {
    return CFUniCharIsMemberOfBitmap(character, (character < 0x10000 ? __CFUniCharNonBaseBitmapForBMP_P : CFUniCharGetBitmapPtrForPlane(kCFUniCharNonBaseCharacterSet, ((character >> 16) & 0xFF))));
}

typedef struct {
    UTF16Char _key;
    UTF16Char _value;
} __CFUniCharPrecomposeBMPMappings;

static UTF16Char __CFUniCharGetMappedBMPValue(const __CFUniCharPrecomposeBMPMappings *theTable, uint32_t numElem, UTF16Char character) {
    const __CFUniCharPrecomposeBMPMappings *p, *q, *divider;

    if ((character < theTable[0]._key) || (character > theTable[numElem-1]._key)) {
        return 0;
    }
    p = theTable;
    q = p + (numElem-1);
    while (p <= q) {
        divider = p + ((q - p) >> 1);	/* divide by 2 */
        if (character < divider->_key) { q = divider - 1; }
        else if (character > divider->_key) { p = divider + 1; }
        else { return divider->_value; }
    }
    return 0;
}

typedef struct {
    UTF32Char _key;
    uint32_t _value;
} __CFUniCharPrecomposeMappings;

static uint32_t __CFUniCharGetMappedValue_P(const __CFUniCharPrecomposeMappings *theTable, uint32_t numElem, UTF32Char character) {
    const __CFUniCharPrecomposeMappings *p, *q, *divider;

    if ((character < theTable[0]._key) || (character > theTable[numElem-1]._key)) {
        return 0;
    }
    p = theTable;
    q = p + (numElem-1);
    while (p <= q) {
        divider = p + ((q - p) >> 1);	/* divide by 2 */
        if (character < divider->_key) { q = divider - 1; }
        else if (character > divider->_key) { p = divider + 1; }
        else { return divider->_value; }
    }
    return 0;
}

CF_PRIVATE
UTF32Char CFUniCharPrecomposeCharacter(UTF32Char base, UTF32Char combining) {
    uint32_t value;

    if (NULL == __CFUniCharPrecompSourceTable) __CFUniCharLoadPrecompositionTable();

    if (!(value = __CFUniCharGetMappedValue_P((const __CFUniCharPrecomposeMappings *)__CFUniCharPrecompSourceTable, __CFUniCharPrecompositionTableLength, combining))) return 0xFFFD;

    // We don't have precomposition in non-BMP
    if (value & kCFUniCharNonBmpFlag) {
        value = __CFUniCharGetMappedValue_P((const __CFUniCharPrecomposeMappings *)((uint32_t *)__CFUniCharNonBMPPrecompDestinationTable + (value & 0xFFFF)), (value >> 16) & 0x7FFF, base);
    } else {
        value = __CFUniCharGetMappedBMPValue((const __CFUniCharPrecomposeBMPMappings *)((uint32_t *)__CFUniCharBMPPrecompDestinationTable + (value & 0xFFFF)), (value >> 16), base);
    }
    return (value ? value : 0xFFFD);
}

#define HANGUL_SBASE 0xAC00
#define HANGUL_LBASE 0x1100
#define HANGUL_VBASE 0x1161
#define HANGUL_TBASE 0x11A7
#define HANGUL_SCOUNT 11172
#define HANGUL_LCOUNT 19
#define HANGUL_VCOUNT 21
#define HANGUL_TCOUNT 28
#define HANGUL_NCOUNT (HANGUL_VCOUNT * HANGUL_TCOUNT)

CF_INLINE void __CFUniCharMoveBufferFromEnd0(UTF16Char *convertedChars, CFIndex length, CFIndex delta) {
    const UTF16Char *limit = convertedChars;
    UTF16Char *dstP;

    convertedChars += length;
    dstP = convertedChars + delta;

    while (convertedChars > limit) *(--dstP) = *(--convertedChars);
}

bool CFUniCharPrecompose(const UTF16Char *characters, CFIndex length, CFIndex *consumedLength, UTF16Char *precomposed, CFIndex maxLength, CFIndex *filledLength) {
    UTF32Char currentChar = 0, lastChar = 0, precomposedChar = 0xFFFD;
    CFIndex originalLength = length, usedLength = 0;
    UTF16Char *currentBase = precomposed;
    uint8_t currentClass, lastClass = 0;
    bool currentBaseIsBMP = true;
    bool isPrecomposed;

    if (NULL == __CFUniCharPrecompSourceTable) __CFUniCharLoadPrecompositionTable();

    while (length > 0) {
        currentChar = *(characters++);
        --length;

        if (CFUniCharIsSurrogateHighCharacter(currentChar) && (length > 0) && CFUniCharIsSurrogateLowCharacter(*characters)) {
            currentChar = CFUniCharGetLongCharacterForSurrogatePair(currentChar, *(characters++));
            --length;
        }

        if (lastChar && __CFUniCharIsNonBaseCharacter(currentChar)) {
            isPrecomposed = (precomposedChar == 0xFFFD ? false : true);
            if (isPrecomposed) lastChar = precomposedChar;

            currentClass = (currentChar > 0xFFFF ? CFUniCharGetUnicodeProperty(currentChar, kCFUniCharCombiningProperty) : CFUniCharGetCombiningPropertyForCharacter(currentChar, __CFUniCharCombiningClassForBMP));

            if ((lastClass == 0) || (currentClass > lastClass)) {
                if ((precomposedChar = CFUniCharPrecomposeCharacter(lastChar, currentChar)) == 0xFFFD) {
                    if (isPrecomposed) precomposedChar = lastChar;
                    lastClass = currentClass;
                } else {
                    continue;
                }
            }
            if (currentChar > 0xFFFF) { // Non-BMP
                usedLength += 2;
                if (usedLength > maxLength) break;
                currentChar -= 0x10000;
                *(precomposed++) = (UTF16Char)((currentChar >> 10) + 0xD800UL);
                *(precomposed++) = (UTF16Char)((currentChar & 0x3FF) + 0xDC00UL);
            } else {
                ++usedLength;
                if (usedLength > maxLength) break;
                *(precomposed++) = (UTF16Char)currentChar;
            }
        } else {
            if ((currentChar >= HANGUL_LBASE) && (currentChar < (HANGUL_LBASE + 0xFF))) { // Hangul Jamo
                int8_t lIndex = currentChar - HANGUL_LBASE;

                if ((length > 0) && (0 <= lIndex) && (lIndex <= HANGUL_LCOUNT)) {
                    int16_t vIndex = *characters - HANGUL_VBASE;

                    if ((vIndex >= 0) && (vIndex <= HANGUL_VCOUNT)) {
                        int16_t tIndex = 0;

                        ++characters; --length;

                        if (length > 0) {
                            tIndex = *characters - HANGUL_TBASE;
                            if ((tIndex < 0) || (tIndex > HANGUL_TCOUNT)) {
                                tIndex = 0;
                            } else {
                                ++characters; --length;
                            }
                        }
                        currentChar = (lIndex * HANGUL_VCOUNT + vIndex) * HANGUL_TCOUNT + tIndex + HANGUL_SBASE;
                    }
                }
            }

            if (precomposedChar != 0xFFFD) {
                if (currentBaseIsBMP) { // Non-BMP
                    if (lastChar > 0xFFFF) { // Last char was Non-BMP
                        --usedLength;
                        memmove(currentBase + 1, currentBase + 2, (precomposed - (currentBase + 2)) * sizeof(UTF16Char));
                    }
                    *(currentBase) = (UTF16Char)precomposedChar;
                } else {
                    if (lastChar < 0x10000) { // Last char was BMP
                        ++usedLength;
                        if (usedLength > maxLength) break;
                        __CFUniCharMoveBufferFromEnd0(currentBase + 1, precomposed - (currentBase + 1), 1);
                    }
                    precomposedChar -= 0x10000;
                    *currentBase = (UTF16Char)((precomposedChar >> 10) + 0xD800UL);
                    *(currentBase + 1) = (UTF16Char)((precomposedChar & 0x3FF) + 0xDC00UL);
                }
                precomposedChar = 0xFFFD;
            }
            currentBase = precomposed;

            lastChar = currentChar;
            lastClass = 0;

            if (currentChar > 0xFFFF) { // Non-BMP
                usedLength += 2;
                if (usedLength > maxLength) break;
                currentChar -= 0x10000;
                *(precomposed++) = (UTF16Char)((currentChar >> 10) + 0xD800UL);
                *(precomposed++) = (UTF16Char)((currentChar & 0x3FF) + 0xDC00UL);
                currentBaseIsBMP = false;
            } else {
                ++usedLength;
                if (usedLength > maxLength) break;
                *(precomposed++) = (UTF16Char)currentChar;
                currentBaseIsBMP = true;
            }
        }
    }

    if (precomposedChar != 0xFFFD) {
        if (currentChar > 0xFFFF) { // Non-BMP
            if (lastChar < 0x10000) { // Last char was BMP
                ++usedLength;
                if (usedLength > maxLength) {
                    if (consumedLength) *consumedLength = originalLength - length;
                    if (filledLength) *filledLength = usedLength;
                    return false;
                }
                __CFUniCharMoveBufferFromEnd0(currentBase + 1, precomposed - (currentBase + 1), 1);
            }
            precomposedChar -= 0x10000;
            *currentBase = (UTF16Char)((precomposedChar >> 10) + 0xD800UL);
            *(currentBase + 1) = (UTF16Char)((precomposedChar & 0x3FF) + 0xDC00UL);
        } else {
            if (lastChar > 0xFFFF) { // Last char was Non-BMP
                --usedLength;
                memmove(currentBase + 1, currentBase + 2, (precomposed - (currentBase + 2)) * sizeof(UTF16Char));
            }
            *(currentBase) = (UTF16Char)precomposedChar;
        }
    }

    if (consumedLength) *consumedLength = originalLength - length;
    if (filledLength) *filledLength = usedLength;

    return true;
}

#undef __CFUniCharIsNonBaseCharacter
#undef HANGUL_SBASE
#undef HANGUL_LBASE
#undef HANGUL_VBASE
#undef HANGUL_TBASE
#undef HANGUL_SCOUNT
#undef HANGUL_LCOUNT
#undef HANGUL_VCOUNT
#undef HANGUL_TCOUNT
#undef HANGUL_NCOUNT

