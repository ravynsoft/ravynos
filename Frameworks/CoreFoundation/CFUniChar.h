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

/*	CFUniChar.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFUNICHAR__)
#define __COREFOUNDATION_CFUNICHAR__ 1


#include <CoreFoundation/CFByteOrder.h>
#include <CoreFoundation/CFBase.h>

CF_EXTERN_C_BEGIN

#define kCFUniCharBitShiftForByte	(3)
#define kCFUniCharBitShiftForMask	(7)

CF_INLINE bool CFUniCharIsSurrogateHighCharacter(UniChar character) {
    return ((character >= 0xD800UL) && (character <= 0xDBFFUL) ? true : false);
}

CF_INLINE bool CFUniCharIsSurrogateLowCharacter(UniChar character) {
    return ((character >= 0xDC00UL) && (character <= 0xDFFFUL) ? true : false);
}

CF_INLINE UTF32Char CFUniCharGetLongCharacterForSurrogatePair(UniChar surrogateHigh, UniChar surrogateLow) {
    return ((surrogateHigh - 0xD800UL) << 10) + (surrogateLow - 0xDC00UL) + 0x0010000UL;
}

// The following values coinside TextEncodingFormat format defines in TextCommon.h
enum {
    kCFUniCharUTF16Format = 0,
    kCFUniCharUTF8Format = 2,
    kCFUniCharUTF32Format = 3
};

CF_INLINE bool CFUniCharIsMemberOfBitmap(UTF16Char theChar, const uint8_t *bitmap) {
    return (bitmap && (bitmap[(theChar) >> kCFUniCharBitShiftForByte] & (((uint32_t)1) << (theChar & kCFUniCharBitShiftForMask))) ? true : false);
}

CF_INLINE void CFUniCharAddCharacterToBitmap(UTF16Char theChar, uint8_t *bitmap) {
    bitmap[(theChar) >> kCFUniCharBitShiftForByte] |= (((uint32_t)1) << (theChar & kCFUniCharBitShiftForMask));
}

CF_INLINE void CFUniCharRemoveCharacterFromBitmap(UTF16Char theChar, uint8_t *bitmap) {
    bitmap[(theChar) >> kCFUniCharBitShiftForByte] &= ~(((uint32_t)1) << (theChar & kCFUniCharBitShiftForMask));
}

enum {
    kCFUniCharControlCharacterSet = 1,
    kCFUniCharWhitespaceCharacterSet,
    kCFUniCharWhitespaceAndNewlineCharacterSet,
    kCFUniCharDecimalDigitCharacterSet,
    kCFUniCharLetterCharacterSet,
    kCFUniCharLowercaseLetterCharacterSet,
    kCFUniCharUppercaseLetterCharacterSet,
    kCFUniCharNonBaseCharacterSet,
    kCFUniCharCanonicalDecomposableCharacterSet,
    kCFUniCharDecomposableCharacterSet = kCFUniCharCanonicalDecomposableCharacterSet,
    kCFUniCharAlphaNumericCharacterSet,
    kCFUniCharPunctuationCharacterSet,
    kCFUniCharIllegalCharacterSet,
    kCFUniCharTitlecaseLetterCharacterSet,
    kCFUniCharSymbolAndOperatorCharacterSet,
    kCFUniCharNewlineCharacterSet,

    kCFUniCharCompatibilityDecomposableCharacterSet = 100, // internal character sets begins here
    kCFUniCharHFSPlusDecomposableCharacterSet,
    kCFUniCharStrongRightToLeftCharacterSet,
    kCFUniCharHasNonSelfLowercaseCharacterSet,
    kCFUniCharHasNonSelfUppercaseCharacterSet,
    kCFUniCharHasNonSelfTitlecaseCharacterSet,
    kCFUniCharHasNonSelfCaseFoldingCharacterSet,
    kCFUniCharHasNonSelfMirrorMappingCharacterSet,
    kCFUniCharControlAndFormatterCharacterSet,
    kCFUniCharCaseIgnorableCharacterSet,
    kCFUniCharGraphemeExtendCharacterSet
};

CF_EXPORT bool CFUniCharIsMemberOf(UTF32Char theChar, uint32_t charset);

// This function returns NULL for kCFUniCharControlCharacterSet, kCFUniCharWhitespaceCharacterSet, kCFUniCharWhitespaceAndNewlineCharacterSet, & kCFUniCharIllegalCharacterSet
CF_EXPORT const uint8_t *CFUniCharGetBitmapPtrForPlane(uint32_t charset, uint32_t plane);

enum {
    kCFUniCharBitmapFilled = (uint8_t)0,
    kCFUniCharBitmapEmpty = (uint8_t)0xFF,
    kCFUniCharBitmapAll = (uint8_t)1
};

CF_EXPORT uint8_t CFUniCharGetBitmapForPlane(uint32_t charset, uint32_t plane, void *bitmap, bool isInverted);

CF_EXPORT uint32_t CFUniCharGetNumberOfPlanes(uint32_t charset);

enum {
    kCFUniCharToLowercase = 0,
    kCFUniCharToUppercase,
    kCFUniCharToTitlecase,
    kCFUniCharCaseFold
};

enum {
    kCFUniCharCaseMapFinalSigma = (1UL << 0),
    kCFUniCharCaseMapAfter_i = (1UL << 1),
    kCFUniCharCaseMapMoreAbove = (1UL << 2),
    kCFUniCharCaseMapDutchDigraph = (1UL << 3),
    kCFUniCharCaseMapGreekTonos = (1UL << 4)
};

CF_EXPORT CFIndex CFUniCharMapCaseTo(UTF32Char theChar, UTF16Char *convertedChar, CFIndex maxLength, uint32_t ctype, uint32_t flags, const uint8_t *langCode);

CF_EXPORT uint32_t CFUniCharGetConditionalCaseMappingFlags(UTF32Char theChar, UTF16Char *buffer, CFIndex currentIndex, CFIndex length, uint32_t type, const uint8_t *langCode, uint32_t lastFlags);

enum {
    kCFUniCharBiDiPropertyON = 0,
    kCFUniCharBiDiPropertyL,
    kCFUniCharBiDiPropertyR,
    kCFUniCharBiDiPropertyAN,
    kCFUniCharBiDiPropertyEN,
    kCFUniCharBiDiPropertyAL,
    kCFUniCharBiDiPropertyNSM,
    kCFUniCharBiDiPropertyCS,
    kCFUniCharBiDiPropertyES,
    kCFUniCharBiDiPropertyET,
    kCFUniCharBiDiPropertyBN,
    kCFUniCharBiDiPropertyS,
    kCFUniCharBiDiPropertyWS,
    kCFUniCharBiDiPropertyB,
    kCFUniCharBiDiPropertyRLO,
    kCFUniCharBiDiPropertyRLE,
    kCFUniCharBiDiPropertyLRO,
    kCFUniCharBiDiPropertyLRE,
    kCFUniCharBiDiPropertyPDF
};

enum {
    kCFUniCharCombiningProperty = 0,
    kCFUniCharBidiProperty
};

// The second arg 'bitmap' has to be the pointer to a specific plane
CF_INLINE uint8_t CFUniCharGetBidiPropertyForCharacter(UTF16Char character, const uint8_t *bitmap) {
    if (bitmap) {
        uint8_t value = bitmap[(character >> 8)];

        if (value > kCFUniCharBiDiPropertyPDF) {
            bitmap = bitmap + 256 + ((value - kCFUniCharBiDiPropertyPDF - 1) * 256);
            return bitmap[character % 256];
        } else {
            return value;
        }
    }
    return kCFUniCharBiDiPropertyL;
}

CF_INLINE uint8_t CFUniCharGetCombiningPropertyForCharacter(UTF16Char character, const uint8_t *bitmap) {
    if (bitmap) {
        uint8_t value = bitmap[(character >> 8)];

        if (value) {
            bitmap = bitmap + 256 + ((value - 1) * 256);
            return bitmap[character % 256];
        }
    }
    return 0;
}

CF_EXPORT const void *CFUniCharGetUnicodePropertyDataForPlane(uint32_t propertyType, uint32_t plane);
CF_EXPORT uint32_t CFUniCharGetNumberOfPlanesForUnicodePropertyData(uint32_t propertyType);
CF_EXPORT uint32_t CFUniCharGetUnicodeProperty(UTF32Char character, uint32_t propertyType);

CF_EXPORT bool CFUniCharFillDestinationBuffer(const UTF32Char *src, CFIndex srcLength, void **dst, CFIndex dstLength, CFIndex *filledLength, uint32_t dstFormat);

// UTF32 support

CF_INLINE bool CFUniCharToUTF32(const UTF16Char *src, CFIndex length, UTF32Char *dst, bool allowLossy, bool isBigEndien) {
    const UTF16Char *limit = src + length;
    UTF32Char character;

    while (src < limit) {
        character = *(src++);

        if (CFUniCharIsSurrogateHighCharacter(character)) {
            if ((src < limit) && CFUniCharIsSurrogateLowCharacter(*src)) {
                character = CFUniCharGetLongCharacterForSurrogatePair(character, *(src++));
            } else {
                if (!allowLossy) return false;
                character = 0xFFFD; // replacement character
            }
        } else if (CFUniCharIsSurrogateLowCharacter(character)) {
            if (!allowLossy) return false;
            character = 0xFFFD; // replacement character
        }

        *(dst++) = (isBigEndien ? CFSwapInt32HostToBig(character) : CFSwapInt32HostToLittle(character));
    }

    return true;
}

CF_INLINE bool CFUniCharFromUTF32(const UTF32Char *src, CFIndex length, UTF16Char *dst, bool allowLossy, bool isBigEndien) {
    const UTF32Char *limit = src + length;
    UTF32Char character;

    while (src < limit) {
        character = (isBigEndien ? CFSwapInt32BigToHost(*(src++)) : CFSwapInt32LittleToHost(*(src++)));

        if (character < 0x10000) { // BMP
            if (allowLossy) {
                if (CFUniCharIsSurrogateHighCharacter(character)) {
                    UTF32Char otherCharacter = 0xFFFD; // replacement character

                    if (src < limit) {
                        otherCharacter = (isBigEndien ? CFSwapInt32BigToHost(*src) : CFSwapInt32LittleToHost(*src));

                        
                        if ((otherCharacter < 0x10000) && CFUniCharIsSurrogateLowCharacter(otherCharacter)) {
                            *(dst++) = character; ++src;
                        } else {
                            otherCharacter = 0xFFFD; // replacement character
                        }
                    }

                    character = otherCharacter;
                } else if (CFUniCharIsSurrogateLowCharacter(character)) {
                    character = 0xFFFD; // replacement character
                }
            } else {
                if (CFUniCharIsSurrogateHighCharacter(character) || CFUniCharIsSurrogateLowCharacter(character)) return false;
            }
        } else if (character < 0x110000) { // non-BMP
            character -= 0x10000;
            *(dst++) = (UTF16Char)((character >> 10) + 0xD800UL);
            character = (UTF16Char)((character & 0x3FF) + 0xDC00UL);
        } else {
            if (!allowLossy) return false;
            character = 0xFFFD; // replacement character
        }

        *(dst++) = character;
    }
    return true;
}

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFUNICHAR__ */

