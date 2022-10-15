/*	CFUniChar.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
    return (UTF32Char)((((UTF32Char)surrogateHigh - 0xD800UL) << 10) + ((UTF32Char)surrogateLow - 0xDC00UL) + 0x0010000UL);
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

