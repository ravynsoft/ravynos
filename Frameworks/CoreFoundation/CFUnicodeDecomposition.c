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

/*    CFUnicodeDecomposition.c
    Copyright (c) 1999-2014, Apple Inc. All rights reserved.
    Responsibility: Aki Inoue
*/

#include <string.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFCharacterSet.h>
#include <CoreFoundation/CFUniChar.h>
#include <CoreFoundation/CFUnicodeDecomposition.h>
#include "CFInternal.h"
#include "CFUniCharPriv.h"

// Canonical Decomposition
static UTF32Char *__CFUniCharDecompositionTable = NULL;
static uint32_t __CFUniCharDecompositionTableLength = 0;
static UTF32Char *__CFUniCharMultipleDecompositionTable = NULL;

static const uint8_t *__CFUniCharDecomposableBitmapForBMP = NULL;
static const uint8_t *__CFUniCharHFSPlusDecomposableBitmapForBMP = NULL;

static CFLock_t __CFUniCharDecompositionTableLock = CFLockInit;

static const uint8_t **__CFUniCharCombiningPriorityTable = NULL;
static uint8_t __CFUniCharCombiningPriorityTableNumPlane = 0;

static void __CFUniCharLoadDecompositionTable(void) {

    __CFLock(&__CFUniCharDecompositionTableLock);

    if (NULL == __CFUniCharDecompositionTable) {
        const uint32_t *bytes = (uint32_t *)CFUniCharGetMappingData(kCFUniCharCanonicalDecompMapping);

        if (NULL == bytes) {
            __CFUnlock(&__CFUniCharDecompositionTableLock);
            return;
        }

        __CFUniCharDecompositionTableLength = *(bytes++);
        __CFUniCharDecompositionTable = (UTF32Char *)bytes;
        __CFUniCharMultipleDecompositionTable = (UTF32Char *)((intptr_t)bytes + __CFUniCharDecompositionTableLength);

        __CFUniCharDecompositionTableLength /= (sizeof(uint32_t) * 2);
        __CFUniCharDecomposableBitmapForBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, 0);
        __CFUniCharHFSPlusDecomposableBitmapForBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharHFSPlusDecomposableCharacterSet, 0);

        CFIndex idx;

        __CFUniCharCombiningPriorityTableNumPlane = CFUniCharGetNumberOfPlanesForUnicodePropertyData(kCFUniCharCombiningProperty);
        __CFUniCharCombiningPriorityTable = (const uint8_t **)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(uint8_t *) * __CFUniCharCombiningPriorityTableNumPlane, 0);
        for (idx = 0;idx < __CFUniCharCombiningPriorityTableNumPlane;idx++) __CFUniCharCombiningPriorityTable[idx] = (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, idx);
    }

    __CFUnlock(&__CFUniCharDecompositionTableLock);
}

static CFLock_t __CFUniCharCompatibilityDecompositionTableLock = CFLockInit;
static UTF32Char *__CFUniCharCompatibilityDecompositionTable = NULL;
static uint32_t __CFUniCharCompatibilityDecompositionTableLength = 0;
static UTF32Char *__CFUniCharCompatibilityMultipleDecompositionTable = NULL;

static void __CFUniCharLoadCompatibilityDecompositionTable(void) {

    __CFLock(&__CFUniCharCompatibilityDecompositionTableLock);

    if (NULL == __CFUniCharCompatibilityDecompositionTable) {
        const uint32_t *bytes = (uint32_t *)CFUniCharGetMappingData(kCFUniCharCompatibilityDecompMapping);

        if (NULL == bytes) {
            __CFUnlock(&__CFUniCharCompatibilityDecompositionTableLock);
            return;
        }

        __CFUniCharCompatibilityDecompositionTableLength = *(bytes++);
        __CFUniCharCompatibilityDecompositionTable = (UTF32Char *)bytes;
        __CFUniCharCompatibilityMultipleDecompositionTable = (UTF32Char *)((intptr_t)bytes + __CFUniCharCompatibilityDecompositionTableLength);

        __CFUniCharCompatibilityDecompositionTableLength /= (sizeof(uint32_t) * 2);
    }

    __CFUnlock(&__CFUniCharCompatibilityDecompositionTableLock);
}

CF_INLINE bool __CFUniCharIsDecomposableCharacterWithFlag(UTF32Char character, bool isHFSPlus) {
    return CFUniCharIsMemberOfBitmap(character, (character < 0x10000 ? (isHFSPlus ? __CFUniCharHFSPlusDecomposableBitmapForBMP : __CFUniCharDecomposableBitmapForBMP) : CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, ((character >> 16) & 0xFF))));
}

CF_INLINE uint8_t __CFUniCharGetCombiningPropertyForCharacter(UTF32Char character) { return CFUniCharGetCombiningPropertyForCharacter(character, (((character) >> 16) < __CFUniCharCombiningPriorityTableNumPlane ? __CFUniCharCombiningPriorityTable[(character) >> 16] : NULL)); }

CF_INLINE bool __CFUniCharIsNonBaseCharacter(UTF32Char character) { return ((0 == __CFUniCharGetCombiningPropertyForCharacter(character)) ? false : true); } // the notion of non-base in normalization is characters with non-0 combining class

typedef struct {
    uint32_t _key;
    uint32_t _value;
} __CFUniCharDecomposeMappings;

static uint32_t __CFUniCharGetMappedValue(const __CFUniCharDecomposeMappings *theTable, uint32_t numElem, UTF32Char character) {
    const __CFUniCharDecomposeMappings *p, *q, *divider;

    if ((character < theTable[0]._key) || (character > theTable[numElem-1]._key)) {
        return 0;
    }
    p = theTable;
    q = p + (numElem-1);
    while (p <= q) {
        divider = p + ((q - p) >> 1);    /* divide by 2 */
        if (character < divider->_key) { q = divider - 1; }
        else if (character > divider->_key) { p = divider + 1; }
        else { return divider->_value; }
    }
    return 0;
}

static void __CFUniCharPrioritySort(UTF32Char *characters, CFIndex length) {
    UTF32Char *end = characters + length;

    while ((characters < end) && (0 == __CFUniCharGetCombiningPropertyForCharacter(*characters))) ++characters;

    if ((end - characters) > 1) {
        uint32_t p1, p2;
        UTF32Char *ch1, *ch2;
        bool changes = true;

        do {
            changes = false;
            ch1 = characters; ch2 = characters + 1;
            p2 = __CFUniCharGetCombiningPropertyForCharacter(*ch1);
            while (ch2 < end) {
                p1 = p2; p2 = __CFUniCharGetCombiningPropertyForCharacter(*ch2);
                if (p1 > p2) {
                    UTF32Char tmp = *ch1; *ch1 = *ch2; *ch2 = tmp;
                    changes = true;
                }
                ++ch1; ++ch2;
            }
        } while (changes);
    }
}

static CFIndex __CFUniCharRecursivelyDecomposeCharacter(UTF32Char character, UTF32Char *convertedChars, CFIndex maxBufferLength) {
    uint32_t value = __CFUniCharGetMappedValue((const __CFUniCharDecomposeMappings *)__CFUniCharDecompositionTable, __CFUniCharDecompositionTableLength, character);
    CFIndex length = CFUniCharConvertFlagToCount(value);
    UTF32Char firstChar = value & 0xFFFFFF;
    UTF32Char *mappings = (length > 1 ? __CFUniCharMultipleDecompositionTable + firstChar : &firstChar);
    CFIndex usedLength = 0;

    if (maxBufferLength < length) return 0;

    if (value & kCFUniCharRecursiveDecompositionFlag) {
        usedLength = __CFUniCharRecursivelyDecomposeCharacter(*mappings, convertedChars, maxBufferLength - length);

        --length; // Decrement for the first char
        if (!usedLength || usedLength + length > maxBufferLength) return 0;
        ++mappings;
        convertedChars += usedLength;
    }

    usedLength += length;

    while (length--) *(convertedChars++) = *(mappings++);

    return usedLength;
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

CFIndex CFUniCharDecomposeCharacter(UTF32Char character, UTF32Char *convertedChars, CFIndex maxBufferLength) {
    if (NULL == __CFUniCharDecompositionTable) __CFUniCharLoadDecompositionTable();
    if (character >= HANGUL_SBASE && character <= (HANGUL_SBASE + HANGUL_SCOUNT)) {
        CFIndex length;

        character -= HANGUL_SBASE;

        length = (character % HANGUL_TCOUNT ? 3 : 2);

        if (maxBufferLength < length) return 0;

        *(convertedChars++) = character / HANGUL_NCOUNT + HANGUL_LBASE;
        *(convertedChars++) = (character % HANGUL_NCOUNT) / HANGUL_TCOUNT + HANGUL_VBASE;
        if (length > 2) *convertedChars = (character % HANGUL_TCOUNT) + HANGUL_TBASE;
        return length;
    } else {
        return __CFUniCharRecursivelyDecomposeCharacter(character, convertedChars, maxBufferLength);
    }
}

CF_INLINE bool __CFProcessReorderBuffer(UTF32Char *buffer, CFIndex length, void **dst, CFIndex dstLength, CFIndex *filledLength, uint32_t dstFormat) {
    if (length > 1) __CFUniCharPrioritySort(buffer, length);
    return CFUniCharFillDestinationBuffer(buffer, length, dst, dstLength, filledLength, dstFormat);
}

#define MAX_BUFFER_LENGTH (32)
bool CFUniCharDecompose(const UTF16Char *src, CFIndex length, CFIndex *consumedLength, void *dst, CFIndex maxLength, CFIndex *filledLength, bool needToReorder, uint32_t dstFormat, bool isHFSPlus) {
    CFIndex usedLength = 0;
    CFIndex originalLength = length;
    UTF32Char buffer[MAX_BUFFER_LENGTH];
    UTF32Char *decompBuffer = buffer;
    CFIndex decompBufferSize = MAX_BUFFER_LENGTH;
    CFIndex decompBufferLen = 0;
    CFIndex segmentLength = 0;
    UTF32Char currentChar;

    if (NULL == __CFUniCharDecompositionTable) __CFUniCharLoadDecompositionTable();

    while ((length - segmentLength) > 0) {
        currentChar = *(src++);

        if (currentChar < 0x80) {
            if (decompBufferLen > 0) {
                if (!__CFProcessReorderBuffer(decompBuffer, decompBufferLen, &dst, maxLength, &usedLength, dstFormat)) break;
                length -= segmentLength;
                segmentLength = 0;
                decompBufferLen = 0;
            }

            if (maxLength > 0) {
                if (usedLength >= maxLength) break;
                switch (dstFormat) {
                case kCFUniCharUTF8Format: *(uint8_t *)dst = currentChar; dst = (uint8_t *)dst + sizeof(uint8_t); break;
                case kCFUniCharUTF16Format: *(UTF16Char *)dst = currentChar; dst = (uint8_t *)dst + sizeof(UTF16Char); break;
                case kCFUniCharUTF32Format: *(UTF32Char *)dst = currentChar; dst = (uint8_t *)dst + sizeof(UTF32Char); break;
                }
            }

            --length;
            ++usedLength;
        } else {
            if (CFUniCharIsSurrogateLowCharacter(currentChar)) { // Stray surrogagte
                if (dstFormat != kCFUniCharUTF16Format) break;
            } else if (CFUniCharIsSurrogateHighCharacter(currentChar)) {
                if (((length - segmentLength) > 1) && CFUniCharIsSurrogateLowCharacter(*src)) {
                    currentChar = CFUniCharGetLongCharacterForSurrogatePair(currentChar, *(src++));
                } else {
                    if (dstFormat != kCFUniCharUTF16Format) break;
                }
            }

            if (needToReorder && __CFUniCharIsNonBaseCharacter(currentChar)) {
                if ((decompBufferLen + 1) >= decompBufferSize) {
                    UTF32Char *newBuffer;
                    
                    decompBufferSize += MAX_BUFFER_LENGTH;
                    newBuffer = (UTF32Char *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(UTF32Char) * decompBufferSize, 0);
                    memmove(newBuffer, decompBuffer, (decompBufferSize - MAX_BUFFER_LENGTH) * sizeof(UTF32Char));
                    if (decompBuffer != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, decompBuffer);
                    decompBuffer = newBuffer;
                }
                
                if (__CFUniCharIsDecomposableCharacterWithFlag(currentChar, isHFSPlus)) { // Vietnamese accent, etc.
                    decompBufferLen += CFUniCharDecomposeCharacter(currentChar, decompBuffer + decompBufferLen, decompBufferSize - decompBufferLen);
                } else {
                    decompBuffer[decompBufferLen++] = currentChar;
                }
            } else {
                if (decompBufferLen > 0) {
                    if (!__CFProcessReorderBuffer(decompBuffer, decompBufferLen, &dst, maxLength, &usedLength, dstFormat)) break;
                    length -= segmentLength;
                    segmentLength = 0;
                }

                if (__CFUniCharIsDecomposableCharacterWithFlag(currentChar, isHFSPlus)) {
                    decompBufferLen = CFUniCharDecomposeCharacter(currentChar, decompBuffer, MAX_BUFFER_LENGTH);
                } else {
                    decompBufferLen = 1;
                    *decompBuffer = currentChar;
                }

                if (!needToReorder || (decompBufferLen == 1)) {
                    if (!CFUniCharFillDestinationBuffer(decompBuffer, decompBufferLen, &dst, maxLength, &usedLength, dstFormat)) break;
                    length -= ((currentChar > 0xFFFF) ? 2 : 1);
                    decompBufferLen = 0;
                    continue;
                }
            }

            segmentLength += ((currentChar > 0xFFFF) ? 2 : 1);
        }
    }

    if ((decompBufferLen > 0) && __CFProcessReorderBuffer(decompBuffer, decompBufferLen, &dst, maxLength, &usedLength, dstFormat)) length -= segmentLength;

    if (decompBuffer != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, decompBuffer);

    if (consumedLength) *consumedLength = originalLength - length;
    if (filledLength) *filledLength = usedLength;

    return ((length > 0) ? false : true);
}

#define MAX_COMP_DECOMP_LEN (32)

static CFIndex __CFUniCharRecursivelyCompatibilityDecomposeCharacter(UTF32Char character, UTF32Char *convertedChars) {
    uint32_t value = __CFUniCharGetMappedValue((const __CFUniCharDecomposeMappings *)__CFUniCharCompatibilityDecompositionTable, __CFUniCharCompatibilityDecompositionTableLength, character);
    CFIndex length = CFUniCharConvertFlagToCount(value);
    UTF32Char firstChar = value & 0xFFFFFF;
    const UTF32Char *mappings = (length > 1 ? __CFUniCharCompatibilityMultipleDecompositionTable + firstChar : &firstChar);
    CFIndex usedLength = length;
    UTF32Char currentChar;
    CFIndex currentLength;

    while (length-- > 0) {
        currentChar = *(mappings++);
        if (__CFUniCharIsDecomposableCharacterWithFlag(currentChar, false)) {
            currentLength = __CFUniCharRecursivelyDecomposeCharacter(currentChar, convertedChars, MAX_COMP_DECOMP_LEN - length);
            convertedChars += currentLength;
            usedLength += (currentLength - 1);
        } else if (CFUniCharIsMemberOf(currentChar, kCFUniCharCompatibilityDecomposableCharacterSet)) {
            currentLength = __CFUniCharRecursivelyCompatibilityDecomposeCharacter(currentChar, convertedChars);
            convertedChars += currentLength;
            usedLength += (currentLength - 1);
        } else {
            *(convertedChars++) = currentChar;
        }
    }

    return usedLength;
}

CF_INLINE void __CFUniCharMoveBufferFromEnd1(UTF32Char *convertedChars, CFIndex length, CFIndex delta) {
    const UTF32Char *limit = convertedChars;
    UTF32Char *dstP;

    convertedChars += length;
    dstP = convertedChars + delta;

    while (convertedChars > limit) *(--dstP) = *(--convertedChars);
}

CF_PRIVATE CFIndex CFUniCharCompatibilityDecompose(UTF32Char *convertedChars, CFIndex length, CFIndex maxBufferLength) {
    UTF32Char currentChar;
    UTF32Char buffer[MAX_COMP_DECOMP_LEN];
    const UTF32Char *bufferP;
    const UTF32Char *limit = convertedChars + length;
    CFIndex filledLength;

    if (NULL == __CFUniCharCompatibilityDecompositionTable) __CFUniCharLoadCompatibilityDecompositionTable();

    while (convertedChars < limit) {
        currentChar = *convertedChars;

        if (CFUniCharIsMemberOf(currentChar, kCFUniCharCompatibilityDecomposableCharacterSet)) {
            filledLength = __CFUniCharRecursivelyCompatibilityDecomposeCharacter(currentChar, buffer);

            if (filledLength + length - 1 > maxBufferLength) return 0;

            if (filledLength > 1) __CFUniCharMoveBufferFromEnd1(convertedChars + 1, limit - convertedChars - 1, filledLength - 1);

            bufferP = buffer;
            length += (filledLength - 1);
            while (filledLength-- > 0) *(convertedChars++) = *(bufferP++);
        } else {
            ++convertedChars;
        }
    }
    
    return length;
}

CF_EXPORT void CFUniCharPrioritySort(UTF32Char *characters, CFIndex length) {
    __CFUniCharPrioritySort(characters, length);
}

#undef MAX_BUFFER_LENGTH
#undef MAX_COMP_DECOMP_LEN
#undef HANGUL_SBASE
#undef HANGUL_LBASE
#undef HANGUL_VBASE
#undef HANGUL_TBASE
#undef HANGUL_SCOUNT
#undef HANGUL_LCOUNT
#undef HANGUL_VCOUNT
#undef HANGUL_TCOUNT
#undef HANGUL_NCOUNT

