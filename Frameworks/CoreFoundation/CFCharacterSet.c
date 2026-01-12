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

/*	CFCharacterSet.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: Aki Inoue
*/

#include <CoreFoundation/CFCharacterSet.h>
#include <CoreFoundation/CFByteOrder.h>
#include "CFCharacterSetPriv.h"
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFString.h>
#include "CFInternal.h"
#include <CoreFoundation/CFUniChar.h>
#include "CFUniCharPriv.h"
#include <stdlib.h>
#include <string.h>


#define BITSPERBYTE	8	/* (CHAR_BIT * sizeof(unsigned char)) */
#define LOG_BPB		3
#define LOG_BPLW	5
#define NUMCHARACTERS	65536

#define MAX_ANNEX_PLANE	(16)

/* Number of things in the array keeping the bits.
*/
#define __kCFBitmapSize		(NUMCHARACTERS / BITSPERBYTE)

/* How many elements max can be in an __kCFCharSetClassString CFCharacterSet
*/
#define __kCFStringCharSetMax 64

/* The last builtin set ID number
*/
#define __kCFLastBuiltinSetID kCFCharacterSetNewline

/* How many elements in the "singles" array before we use binary search.
*/
#define __kCFSetBreakeven 10

/* This tells us, within 1k or so, whether a thing is POTENTIALLY in the set (in the bitmap blob of the private structure) before we bother to do specific checking.
*/
#define __CFCSetBitsInRange(n, i)	(i[n>>15] & (1L << ((n>>10) % 32)))

/* Compact bitmap params
*/
#define __kCFCompactBitmapNumPages (256)

#define __kCFCompactBitmapMaxPages (128) // the max pages allocated

#define __kCFCompactBitmapPageSize (__kCFBitmapSize / __kCFCompactBitmapNumPages)

typedef struct {
    CFCharacterSetRef *_nonBMPPlanes;
    unsigned int _validEntriesBitmap;
    unsigned char _numOfAllocEntries;
    unsigned char _isAnnexInverted;
    uint16_t _padding;
} CFCharSetAnnexStruct;

struct __CFCharacterSet {
    CFRuntimeBase _base;
    CFHashCode _hashValue;
    union {
        struct {
            CFIndex _type;
        } _builtin;
        struct {
            UInt32 _firstChar;
            CFIndex _length;
        } _range;
        struct {
            UniChar *_buffer;
            CFIndex _length;
        } _string;
        struct {
            uint8_t *_bits;
        } _bitmap;
        struct {
            uint8_t *_cBits;
        } _compactBitmap;
   } _variants;
   CFCharSetAnnexStruct *_annex;
};

/* _base._info values interesting for CFCharacterSet
*/
enum {
    __kCFCharSetClassTypeMask = 0x0070,
      __kCFCharSetClassBuiltin = 0x0000,
      __kCFCharSetClassRange = 0x0010,
      __kCFCharSetClassString = 0x0020,
      __kCFCharSetClassBitmap = 0x0030,
      __kCFCharSetClassSet = 0x0040,
      __kCFCharSetClassCompactBitmap = 0x0040,

    __kCFCharSetIsInvertedMask = 0x0008,
      __kCFCharSetIsInverted = 0x0008,

    __kCFCharSetHasHashValueMask = 0x00004,
      __kCFCharSetHasHashValue = 0x0004,

    /* Generic CFBase values */
    __kCFCharSetIsMutableMask = 0x0001,
      __kCFCharSetIsMutable = 0x0001,
};

/* Inline accessor macros for _base._info
*/
CF_INLINE Boolean __CFCSetIsMutable(CFCharacterSetRef cset) {return (cset->_base._cfinfo[CF_INFO_BITS] & __kCFCharSetIsMutableMask) == __kCFCharSetIsMutable;}
CF_INLINE Boolean __CFCSetIsBuiltin(CFCharacterSetRef cset) {return (cset->_base._cfinfo[CF_INFO_BITS] & __kCFCharSetClassTypeMask) == __kCFCharSetClassBuiltin;}
CF_INLINE Boolean __CFCSetIsRange(CFCharacterSetRef cset) {return (cset->_base._cfinfo[CF_INFO_BITS] & __kCFCharSetClassTypeMask) == __kCFCharSetClassRange;}
CF_INLINE Boolean __CFCSetIsString(CFCharacterSetRef cset) {return (cset->_base._cfinfo[CF_INFO_BITS] & __kCFCharSetClassTypeMask) == __kCFCharSetClassString;}
CF_INLINE Boolean __CFCSetIsBitmap(CFCharacterSetRef cset) {return (cset->_base._cfinfo[CF_INFO_BITS] & __kCFCharSetClassTypeMask) == __kCFCharSetClassBitmap;}
CF_INLINE Boolean __CFCSetIsCompactBitmap(CFCharacterSetRef cset) {return (cset->_base._cfinfo[CF_INFO_BITS] & __kCFCharSetClassTypeMask) == __kCFCharSetClassCompactBitmap;}
CF_INLINE Boolean __CFCSetIsInverted(CFCharacterSetRef cset) {return (cset->_base._cfinfo[CF_INFO_BITS] & __kCFCharSetIsInvertedMask) == __kCFCharSetIsInverted;}
CF_INLINE Boolean __CFCSetHasHashValue(CFCharacterSetRef cset) {return (cset->_base._cfinfo[CF_INFO_BITS] & __kCFCharSetHasHashValueMask) == __kCFCharSetHasHashValue;}
CF_INLINE UInt32 __CFCSetClassType(CFCharacterSetRef cset) {return (cset->_base._cfinfo[CF_INFO_BITS] & __kCFCharSetClassTypeMask);}

CF_INLINE void __CFCSetPutIsMutable(CFMutableCharacterSetRef cset, Boolean isMutable) {(isMutable ? (cset->_base._cfinfo[CF_INFO_BITS] |= __kCFCharSetIsMutable) : (cset->_base._cfinfo[CF_INFO_BITS] &= ~ __kCFCharSetIsMutable));}
CF_INLINE void __CFCSetPutIsInverted(CFMutableCharacterSetRef cset, Boolean isInverted) {(isInverted ? (cset->_base._cfinfo[CF_INFO_BITS] |= __kCFCharSetIsInverted) : (cset->_base._cfinfo[CF_INFO_BITS] &= ~__kCFCharSetIsInverted));}
CF_INLINE void __CFCSetPutHasHashValue(CFMutableCharacterSetRef cset, Boolean hasHash) {(hasHash ? (cset->_base._cfinfo[CF_INFO_BITS] |= __kCFCharSetHasHashValue) : (cset->_base._cfinfo[CF_INFO_BITS] &= ~__kCFCharSetHasHashValue));}
CF_INLINE void __CFCSetPutClassType(CFMutableCharacterSetRef cset, UInt32 classType) {cset->_base._cfinfo[CF_INFO_BITS] &= ~__kCFCharSetClassTypeMask;  cset->_base._cfinfo[CF_INFO_BITS] |= classType;}

CF_PRIVATE Boolean __CFCharacterSetIsMutable(CFCharacterSetRef cset) {return __CFCSetIsMutable(cset);}

/* Inline contents accessor macros
*/
CF_INLINE CFCharacterSetPredefinedSet __CFCSetBuiltinType(CFCharacterSetRef cset) {return cset->_variants._builtin._type;}
CF_INLINE UInt32 __CFCSetRangeFirstChar(CFCharacterSetRef cset) {return cset->_variants._range._firstChar;}
CF_INLINE CFIndex __CFCSetRangeLength(CFCharacterSetRef cset) {return cset->_variants._range._length;}
CF_INLINE UniChar *__CFCSetStringBuffer(CFCharacterSetRef cset) {return (UniChar*)(cset->_variants._string._buffer);}
CF_INLINE CFIndex __CFCSetStringLength(CFCharacterSetRef cset) {return cset->_variants._string._length;}
CF_INLINE uint8_t *__CFCSetBitmapBits(CFCharacterSetRef cset) {return cset->_variants._bitmap._bits;}
CF_INLINE uint8_t *__CFCSetCompactBitmapBits(CFCharacterSetRef cset) {return cset->_variants._compactBitmap._cBits;}

CF_INLINE void __CFCSetPutBuiltinType(CFMutableCharacterSetRef cset, CFCharacterSetPredefinedSet type) {cset->_variants._builtin._type = type;}
CF_INLINE void __CFCSetPutRangeFirstChar(CFMutableCharacterSetRef cset, UInt32 first) {cset->_variants._range._firstChar = first;}
CF_INLINE void __CFCSetPutRangeLength(CFMutableCharacterSetRef cset, CFIndex length) {cset->_variants._range._length = length;}
CF_INLINE void __CFCSetPutStringBuffer(CFMutableCharacterSetRef cset, UniChar *theBuffer) {cset->_variants._string._buffer = theBuffer;}
CF_INLINE void __CFCSetPutStringLength(CFMutableCharacterSetRef cset, CFIndex length) {cset->_variants._string._length = length;}
CF_INLINE void __CFCSetPutBitmapBits(CFMutableCharacterSetRef cset, uint8_t *bits) {cset->_variants._bitmap._bits = bits;}
CF_INLINE void __CFCSetPutCompactBitmapBits(CFMutableCharacterSetRef cset, uint8_t *bits) {cset->_variants._compactBitmap._cBits = bits;}

/* Validation funcs
*/
#if defined(CF_ENABLE_ASSERTIONS)
CF_INLINE void __CFCSetValidateBuiltinType(CFCharacterSetPredefinedSet type, const char *func) {
    CFAssert2(type > 0 && type <= __kCFLastBuiltinSetID, __kCFLogAssertion, "%s: Unknowen builtin type %d", func, type);
}
CF_INLINE void __CFCSetValidateRange(CFRange theRange, const char *func) {
    CFAssert3(theRange.location >= 0 && theRange.location + theRange.length <= 0x1FFFFF, __kCFLogAssertion, "%s: Range out of Unicode range (location -> %d length -> %d)", func, theRange.location, theRange.length);
}
CF_INLINE void __CFCSetValidateTypeAndMutability(CFCharacterSetRef cset, const char *func) {
    __CFGenericValidateType(cset, __kCFCharacterSetTypeID);
    CFAssert1(__CFCSetIsMutable(cset), __kCFLogAssertion, "%s: Immutable character set passed to mutable function", func);
}
#else
#define __CFCSetValidateBuiltinType(t,f)
#define __CFCSetValidateRange(r,f)
#define __CFCSetValidateTypeAndMutability(r,f)
#endif

/* Inline utility funcs
*/
static Boolean __CFCSetIsEqualBitmap(const UInt32 *bits1, const UInt32 *bits2) {
    CFIndex length = __kCFBitmapSize / sizeof(UInt32);

    if (bits1 == bits2) {
        return true;
    } else if (bits1 && bits2) {
        if (bits1 == (const UInt32 *)-1) {
            while (length--) if ((UInt32)-1 != *bits2++) return false;
        } else if (bits2 == (const UInt32 *)-1) {
            while (length--) if ((UInt32)-1 != *bits1++) return false;
        } else {
            while (length--) if (*bits1++ != *bits2++) return false;
        }
        return true;
    } else if (!bits1 && !bits2) { // empty set
        return true;
    } else {
        if (bits2) bits1 = bits2;
        if (bits1 == (const UInt32 *)-1) return false;
        while (length--) if (*bits1++) return false;
        return true;
    }
}

CF_INLINE Boolean __CFCSetIsEqualBitmapInverted(const UInt32 *bits1, const UInt32 *bits2) {
    CFIndex length = __kCFBitmapSize / sizeof(UInt32);

    while (length--) if (*bits1++ != ~(*(bits2++))) return false;
    return true;
}

static Boolean __CFCSetIsBitmapEqualToRange(const UInt32 *bits, UniChar firstChar, UniChar lastChar, Boolean isInverted) {
    CFIndex firstCharIndex = firstChar >> LOG_BPB;
    CFIndex lastCharIndex = lastChar >> LOG_BPB;
    CFIndex length;
    UInt32 value;

    if (firstCharIndex == lastCharIndex) {
        value = ((((UInt32)0xFF) << (firstChar & (BITSPERBYTE - 1))) & (((UInt32)0xFF) >> ((BITSPERBYTE - 1) - (lastChar & (BITSPERBYTE - 1))))) << (((sizeof(UInt32) - 1) - (firstCharIndex % sizeof(UInt32))) * BITSPERBYTE);
        value = CFSwapInt32HostToBig(value);
        firstCharIndex = lastCharIndex = firstChar >> LOG_BPLW;
        if (*(bits + firstCharIndex) != (isInverted ? ~value : value)) return FALSE;
    } else {
        UInt32 firstCharMask;
        UInt32 lastCharMask;

        length = firstCharIndex % sizeof(UInt32);
        firstCharMask = (((((UInt32)0xFF) << (firstChar & (BITSPERBYTE - 1))) & 0xFF) << (((sizeof(UInt32) - 1) - length) * BITSPERBYTE)) | (((UInt32)0xFFFFFFFF) >> ((length + 1) * BITSPERBYTE));
        
        length = lastCharIndex % sizeof(UInt32);
        lastCharMask = ((((UInt32)0xFF) >> ((BITSPERBYTE - 1) - (lastChar & (BITSPERBYTE - 1)))) << (((sizeof(UInt32) - 1) - length) * BITSPERBYTE)) | (((UInt32)0xFFFFFFFF) << ((sizeof(UInt32) - length) * BITSPERBYTE));

        firstCharIndex = firstChar >> LOG_BPLW;
        lastCharIndex = lastChar >> LOG_BPLW;
        
        if (firstCharIndex == lastCharIndex) {
            firstCharMask &= lastCharMask;
            value = CFSwapInt32HostToBig(firstCharMask & lastCharMask);
            if (*(bits + firstCharIndex) != (isInverted ? ~value : value)) return FALSE;
        } else {
            value = CFSwapInt32HostToBig(firstCharMask);
            if (*(bits + firstCharIndex) != (isInverted ? ~value : value)) return FALSE;

            value = CFSwapInt32HostToBig(lastCharMask);
            if (*(bits + lastCharIndex) != (isInverted ? ~value : value)) return FALSE;
        }
    }

    length = firstCharIndex;
    value = (isInverted ? ((UInt32)0xFFFFFFFF) : 0);
    while (length--) {
        if (*(bits++) != value) return FALSE;
    }

    ++bits; // Skip firstCharIndex
    length = (lastCharIndex - (firstCharIndex + 1));
    value = (isInverted ? 0 : ((UInt32)0xFFFFFFFF));
    while (length-- > 0) {
        if (*(bits++) != value) return FALSE;
    }
    if (firstCharIndex != lastCharIndex) ++bits;

    length = (0xFFFF >> LOG_BPLW) - lastCharIndex;
    value = (isInverted ? ((UInt32)0xFFFFFFFF) : 0);
    while (length--) {
        if (*(bits++) != value) return FALSE;
    }

    return TRUE;
}

CF_INLINE Boolean __CFCSetIsBitmapSupersetOfBitmap(const UInt32 *bits1, const UInt32 *bits2, Boolean isInverted1, Boolean isInverted2) {
    CFIndex length = __kCFBitmapSize / sizeof(UInt32);
    UInt32 val1, val2;

    while (length--) {
        val2 = (isInverted2 ? ~(*(bits2++)) : *(bits2++));
        val1 = (isInverted1 ? ~(*(bits1++)) : *(bits1++)) & val2;
        if (val1 != val2) return false;
    }

    return true;
}

CF_INLINE Boolean __CFCSetHasNonBMPPlane(CFCharacterSetRef cset) { return ((cset)->_annex && (cset)->_annex->_validEntriesBitmap ? true : false); }
CF_INLINE Boolean __CFCSetAnnexIsInverted (CFCharacterSetRef cset) { return ((cset)->_annex && (cset)->_annex->_isAnnexInverted ? true : false); }
CF_INLINE UInt32 __CFCSetAnnexValidEntriesBitmap(CFCharacterSetRef cset) { return ((cset)->_annex ? (cset)->_annex->_validEntriesBitmap : 0); }

CF_INLINE Boolean __CFCSetIsEmpty(CFCharacterSetRef cset) {
    if (__CFCSetHasNonBMPPlane(cset) || __CFCSetAnnexIsInverted(cset)) return false;

    switch (__CFCSetClassType(cset)) {
        case __kCFCharSetClassRange: if (!__CFCSetRangeLength(cset)) return true; break;
        case __kCFCharSetClassString: if (!__CFCSetStringLength(cset)) return true; break;
        case __kCFCharSetClassBitmap: if (!__CFCSetBitmapBits(cset)) return true; break;
        case __kCFCharSetClassCompactBitmap: if (!__CFCSetCompactBitmapBits(cset)) return true; break;
    }
    return false;
}

CF_INLINE void __CFCSetBitmapAddCharacter(uint8_t *bitmap, UniChar theChar) {
    bitmap[(theChar) >> LOG_BPB] |= (((unsigned)1) << (theChar & (BITSPERBYTE - 1)));
}

CF_INLINE void __CFCSetBitmapRemoveCharacter(uint8_t *bitmap, UniChar theChar) {
    bitmap[(theChar) >> LOG_BPB] &= ~(((unsigned)1) << (theChar & (BITSPERBYTE - 1)));
}

CF_INLINE Boolean __CFCSetIsMemberBitmap(const uint8_t *bitmap, UniChar theChar) {
    return ((bitmap[(theChar) >> LOG_BPB] & (((unsigned)1) << (theChar & (BITSPERBYTE - 1)))) ? true : false);
}

#define NUM_32BIT_SLOTS	(NUMCHARACTERS / 32)

CF_INLINE void __CFCSetBitmapFastFillWithValue(UInt32 *bitmap, uint8_t value) {
    UInt32 mask = (value << 24) | (value << 16) | (value << 8) | value;
    UInt32 numSlots = NUMCHARACTERS / 32;

    while (numSlots--) *(bitmap++) = mask;
}

CF_INLINE void __CFCSetBitmapAddCharactersInRange(uint8_t *bitmap, UniChar firstChar, UniChar lastChar) {
    if (firstChar == lastChar) {
        bitmap[firstChar >> LOG_BPB] |= (((unsigned)1) << (firstChar & (BITSPERBYTE - 1)));
    } else {
        UInt32 idx = firstChar >> LOG_BPB;
        UInt32 max = lastChar >> LOG_BPB;

        if (idx == max) {
            bitmap[idx] |= (((unsigned)0xFF) << (firstChar & (BITSPERBYTE - 1))) & (((unsigned)0xFF) >> ((BITSPERBYTE - 1) - (lastChar & (BITSPERBYTE - 1))));
        } else {
            bitmap[idx] |= (((unsigned)0xFF) << (firstChar & (BITSPERBYTE - 1)));
            bitmap[max] |= (((unsigned)0xFF) >> ((BITSPERBYTE - 1) - (lastChar & (BITSPERBYTE - 1))));

            ++idx;
            while (idx < max) bitmap[idx++] = 0xFF;
        }
    }
}

CF_INLINE void __CFCSetBitmapRemoveCharactersInRange(uint8_t *bitmap, UniChar firstChar, UniChar lastChar) {
    UInt32 idx = firstChar >> LOG_BPB;
    UInt32 max = lastChar >> LOG_BPB;

    if (idx == max) {
        bitmap[idx] &= ~((((unsigned)0xFF) << (firstChar & (BITSPERBYTE - 1))) & (((unsigned)0xFF) >> ((BITSPERBYTE - 1) - (lastChar & (BITSPERBYTE - 1)))));
    } else {
        bitmap[idx] &= ~(((unsigned)0xFF) << (firstChar & (BITSPERBYTE - 1)));
        bitmap[max] &= ~(((unsigned)0xFF) >> ((BITSPERBYTE - 1) - (lastChar & (BITSPERBYTE - 1))));

        ++idx;
        while (idx < max) bitmap[idx++] = 0;
    }
}

#define __CFCSetAnnexBitmapSetPlane(bitmap,plane)	((bitmap) |= (1 << (plane)))
#define __CFCSetAnnexBitmapClearPlane(bitmap,plane)	((bitmap) &= (~(1 << (plane))))
#define __CFCSetAnnexBitmapGetPlane(bitmap,plane)	((bitmap) & (1 << (plane)))

CF_INLINE void __CFCSetAllocateAnnexForPlane(CFCharacterSetRef cset, int plane) {
    if (cset->_annex == NULL) {
        ((CFMutableCharacterSetRef)cset)->_annex = (CFCharSetAnnexStruct *)CFAllocatorAllocate(CFGetAllocator(cset), sizeof(CFCharSetAnnexStruct), 0);
        cset->_annex->_numOfAllocEntries = plane;
        cset->_annex->_isAnnexInverted = false;
        cset->_annex->_validEntriesBitmap = 0;
        cset->_annex->_nonBMPPlanes = ((plane > 0) ? (CFCharacterSetRef*)CFAllocatorAllocate(CFGetAllocator(cset), sizeof(CFCharacterSetRef) * plane, 0) : NULL);
    } else if (cset->_annex->_numOfAllocEntries < plane) {
        cset->_annex->_numOfAllocEntries = plane;
        if (NULL == cset->_annex->_nonBMPPlanes) {
            cset->_annex->_nonBMPPlanes = (CFCharacterSetRef*)CFAllocatorAllocate(CFGetAllocator(cset), sizeof(CFCharacterSetRef) * plane, 0);
        } else {
            cset->_annex->_nonBMPPlanes = (CFCharacterSetRef*)CFAllocatorReallocate(CFGetAllocator(cset), (void *)cset->_annex->_nonBMPPlanes, sizeof(CFCharacterSetRef) * plane, 0);
        }
    }
}

CF_INLINE void __CFCSetAnnexSetIsInverted(CFCharacterSetRef cset, Boolean flag) {
    if (flag) __CFCSetAllocateAnnexForPlane(cset, 0);
    if (cset->_annex) ((CFMutableCharacterSetRef)cset)->_annex->_isAnnexInverted = flag;
}
                                      
CF_INLINE void __CFCSetPutCharacterSetToAnnexPlane(CFCharacterSetRef cset, CFCharacterSetRef annexCSet, int plane) {
    __CFCSetAllocateAnnexForPlane(cset, plane);
    if (__CFCSetAnnexBitmapGetPlane(cset->_annex->_validEntriesBitmap, plane)) CFRelease(cset->_annex->_nonBMPPlanes[plane - 1]);
    if (annexCSet) {
        cset->_annex->_nonBMPPlanes[plane - 1] = (CFCharacterSetRef)CFRetain(annexCSet);
        __CFCSetAnnexBitmapSetPlane(cset->_annex->_validEntriesBitmap, plane);
    } else {
        __CFCSetAnnexBitmapClearPlane(cset->_annex->_validEntriesBitmap, plane);
    }
}

CF_INLINE CFCharacterSetRef __CFCSetGetAnnexPlaneCharacterSet(CFCharacterSetRef cset, int plane) {
    __CFCSetAllocateAnnexForPlane(cset, plane);
    if (!__CFCSetAnnexBitmapGetPlane(cset->_annex->_validEntriesBitmap, plane)) {
        cset->_annex->_nonBMPPlanes[plane - 1] = (CFCharacterSetRef)CFCharacterSetCreateMutable(CFGetAllocator(cset));
        __CFCSetAnnexBitmapSetPlane(cset->_annex->_validEntriesBitmap, plane);
    }
    return cset->_annex->_nonBMPPlanes[plane - 1];
}

CF_INLINE CFCharacterSetRef __CFCSetGetAnnexPlaneCharacterSetNoAlloc(CFCharacterSetRef cset, int plane) {
    return (cset->_annex && __CFCSetAnnexBitmapGetPlane(cset->_annex->_validEntriesBitmap, plane) ? cset->_annex->_nonBMPPlanes[plane - 1] : NULL);
}

CF_INLINE void __CFCSetDeallocateAnnexPlane(CFCharacterSetRef cset) {
    if (cset->_annex) {
        int idx;
        
        for (idx = 0;idx < MAX_ANNEX_PLANE;idx++) {
            if (__CFCSetAnnexBitmapGetPlane(cset->_annex->_validEntriesBitmap, idx + 1)) {
                CFRelease(cset->_annex->_nonBMPPlanes[idx]);
            }
        }
        CFAllocatorDeallocate(CFGetAllocator(cset), cset->_annex->_nonBMPPlanes);
        CFAllocatorDeallocate(CFGetAllocator(cset), cset->_annex);
        ((CFMutableCharacterSetRef)cset)->_annex = NULL;
    }
}

CF_INLINE uint8_t __CFCSetGetHeaderValue(const uint8_t *bitmap, int *numPages) {
    uint8_t value = *bitmap;

    if ((value == 0) || (value == UINT8_MAX)) {
        int numBytes = __kCFCompactBitmapPageSize - 1;

        while (numBytes > 0) {
            if (*(++bitmap) != value) break;
            --numBytes;
        }
        if (numBytes == 0) return value;
    }
    return (uint8_t)(++(*numPages));
}

CF_INLINE bool __CFCSetIsMemberInCompactBitmap(const uint8_t *compactBitmap, UTF16Char character) {
    uint8_t value = compactBitmap[(character >> 8)]; // Assuming __kCFCompactBitmapNumPages == 256

    if (value == 0) {
        return false;
    } else if (value == UINT8_MAX) {
        return true;
    } else {
        compactBitmap += (__kCFCompactBitmapNumPages + (__kCFCompactBitmapPageSize * (value - 1)));
        character &= 0xFF; // Assuming __kCFCompactBitmapNumPages == 256
        return ((compactBitmap[(character / BITSPERBYTE)] & (1 << (character % BITSPERBYTE))) ? true : false);
    }
}

CF_INLINE uint32_t __CFCSetGetCompactBitmapSize(const uint8_t *compactBitmap) {
    uint32_t length = __kCFCompactBitmapNumPages;
    uint32_t size = __kCFCompactBitmapNumPages;
    uint8_t value;

    while (length-- > 0) {
        value = *(compactBitmap++);
        if ((value != 0) && (value != UINT8_MAX)) size += __kCFCompactBitmapPageSize;
    }
    return size;
}

CF_INLINE void __CFExpandCompactBitmap(const uint8_t *src, uint8_t *dst) {
    const uint8_t *srcBody = src + __kCFCompactBitmapNumPages;
    int i;
    uint8_t value;

    for (i = 0;i < __kCFCompactBitmapNumPages;i++) {
        value = *(src++);
        if ((value == 0) || (value == UINT8_MAX)) {
            memset(dst, value, __kCFCompactBitmapPageSize);
        } else {
            memmove(dst, srcBody, __kCFCompactBitmapPageSize);
            srcBody += __kCFCompactBitmapPageSize;
        }
        dst += __kCFCompactBitmapPageSize;
    }
}


static void __CFCheckForExpandedSet(CFCharacterSetRef cset) {
    static int8_t __CFNumberOfPlanesForLogging = -1;
    static bool warnedOnce = false;

    if (0 > __CFNumberOfPlanesForLogging) {
        const char *envVar = __CFgetenv("CFCharacterSetCheckForExpandedSet");
        long value = (envVar ? strtol_l(envVar, NULL, 0, NULL) : 0);
        __CFNumberOfPlanesForLogging = (int8_t)(((value > 0) && (value <= 16)) ? value : 0);
    }

    if (__CFNumberOfPlanesForLogging) {
        uint32_t entries = __CFCSetAnnexValidEntriesBitmap(cset);
        int count = 0;

        while (entries) {
            if ((entries & 1) && (++count >= __CFNumberOfPlanesForLogging)) {
                if (!warnedOnce) {
                    CFLog(kCFLogLevelWarning, CFSTR("An expanded CFMutableCharacter has been detected.  Recommend to compact with CFCharacterSetCreateCopy"));
		    warnedOnce = true;
		}
                break;
            }
            entries >>= 1;
        }
    }
}

static void __CFCSetGetBitmap(CFCharacterSetRef cset, uint8_t *bits) {
    uint8_t *bitmap;
    CFIndex length = __kCFBitmapSize;

    if (__CFCSetIsBitmap(cset) && (bitmap = __CFCSetBitmapBits(cset))) {
        memmove(bits, bitmap, __kCFBitmapSize);
    } else {
        Boolean isInverted = __CFCSetIsInverted(cset);
        uint8_t value = (isInverted ? (uint8_t)-1 : 0);

        bitmap = bits;
        while (length--) *bitmap++ = value; // Initialize the buffer

        if (!__CFCSetIsEmpty(cset)) {
            switch (__CFCSetClassType(cset)) {
                case __kCFCharSetClassBuiltin: {
                    UInt8 result = CFUniCharGetBitmapForPlane(__CFCSetBuiltinType(cset), 0, bits, (isInverted != 0));
                    if (result == kCFUniCharBitmapEmpty && isInverted) {
                        length = __kCFBitmapSize;
                        bitmap = bits;
                        while (length--) *bitmap++ = 0;
                    } else if (result == kCFUniCharBitmapAll && !isInverted) {
                        length = __kCFBitmapSize;
                        bitmap = bits;
                        while (length--) *bitmap++ = (UInt8)0xFF;
                    }
                }
                    break;

                case __kCFCharSetClassRange: {
                    UInt32 theChar = __CFCSetRangeFirstChar(cset);
                    if (theChar < NUMCHARACTERS) { // the range starts in BMP
                        length = __CFCSetRangeLength(cset);
                        if (theChar + length >= NUMCHARACTERS) length = NUMCHARACTERS - theChar;
                        if (isInverted) {
                            __CFCSetBitmapRemoveCharactersInRange(bits, theChar, (UniChar)(theChar + length) - 1);
                        } else {
                            __CFCSetBitmapAddCharactersInRange(bits, theChar, (UniChar)(theChar + length) - 1);
                        }
                    }
                }
                    break;

                case __kCFCharSetClassString: {
                    const UniChar *buffer = __CFCSetStringBuffer(cset);
                    length = __CFCSetStringLength(cset);
                    while (length--) (isInverted ? __CFCSetBitmapRemoveCharacter(bits, *buffer++) : __CFCSetBitmapAddCharacter(bits, *buffer++));
                }
                    break;

                case __kCFCharSetClassCompactBitmap:
                    __CFExpandCompactBitmap(__CFCSetCompactBitmapBits(cset), bits);
                    break;
            }
        }
    }
}

static Boolean __CFCharacterSetEqual(CFTypeRef cf1, CFTypeRef cf2);

static Boolean __CFCSetIsEqualAnnex(CFCharacterSetRef cf1, CFCharacterSetRef cf2) {
    CFCharacterSetRef subSet1;
    CFCharacterSetRef subSet2;
    Boolean isAnnexInvertStateIdentical = (__CFCSetAnnexIsInverted(cf1) == __CFCSetAnnexIsInverted(cf2) ? true: false);
    int idx;

    if (isAnnexInvertStateIdentical) {
        if (__CFCSetAnnexValidEntriesBitmap(cf1) != __CFCSetAnnexValidEntriesBitmap(cf2)) return false;
        for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
            subSet1 = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(cf1, idx);
            subSet2 = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(cf2, idx);

            if (subSet1 && !__CFCharacterSetEqual(subSet1, subSet2)) return false;
        }
    } else {
        uint8_t bitsBuf[__kCFBitmapSize];
        uint8_t bitsBuf2[__kCFBitmapSize];

        for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
            subSet1 = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(cf1, idx);
            subSet2 = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(cf2, idx);

            if (subSet1 == NULL && subSet2 == NULL) {
                return false;
            } else if (subSet1 == NULL) {
                if (__CFCSetIsBitmap(subSet2)) {
                    if (!__CFCSetIsEqualBitmap((const UInt32 *)__CFCSetBitmapBits(subSet2), (const UInt32 *)-1)) {
                        return false;
                    }
                } else {
                    __CFCSetGetBitmap(subSet2, bitsBuf);
                    if (!__CFCSetIsEqualBitmap((const UInt32 *)bitsBuf, (const UInt32 *)-1)) {
                        return false;
                    }
                }
            } else if (subSet2 == NULL) {
                if (__CFCSetIsBitmap(subSet1)) {
                    if (!__CFCSetIsEqualBitmap((const UInt32 *)__CFCSetBitmapBits(subSet1), (const UInt32 *)-1)) {
                        return false;
                    }
                } else {
                    __CFCSetGetBitmap(subSet1, bitsBuf);
                    if (!__CFCSetIsEqualBitmap((const UInt32 *)bitsBuf, (const UInt32 *)-1)) {
                        return false;
                    }
                }
            } else {
                Boolean isBitmap1 = __CFCSetIsBitmap(subSet1);
                Boolean isBitmap2 = __CFCSetIsBitmap(subSet2);

                if (isBitmap1 && isBitmap2) {
                    if (!__CFCSetIsEqualBitmapInverted((const UInt32 *)__CFCSetBitmapBits(subSet1), (const UInt32 *)__CFCSetBitmapBits(subSet2))) {
                        return false;
                    }
                } else if (!isBitmap1 && !isBitmap2) {
                    __CFCSetGetBitmap(subSet1, bitsBuf);
                    __CFCSetGetBitmap(subSet2, bitsBuf2);
                    if (!__CFCSetIsEqualBitmapInverted((const UInt32 *)bitsBuf, (const UInt32 *)bitsBuf2)) {
                        return false;
                    }
                } else {
                    if (isBitmap2) {
                        CFCharacterSetRef tmp = subSet2;
                        subSet2 = subSet1;
                        subSet1 = tmp;
                    }
                    __CFCSetGetBitmap(subSet2, bitsBuf);
                    if (!__CFCSetIsEqualBitmapInverted((const UInt32 *)__CFCSetBitmapBits(subSet1), (const UInt32 *)bitsBuf)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

/* Compact bitmap
*/
static uint8_t *__CFCreateCompactBitmap(CFAllocatorRef allocator, const uint8_t *bitmap) {
    const uint8_t *src;
    uint8_t *dst;
    int i;
    int numPages = 0;
    uint8_t header[__kCFCompactBitmapNumPages];

    src = bitmap;
    for (i = 0;i < __kCFCompactBitmapNumPages;i++) {
        header[i] = __CFCSetGetHeaderValue(src, &numPages);

        // Allocating more pages is probably not interesting enough to be compact
        if (numPages > __kCFCompactBitmapMaxPages) return NULL;
        src += __kCFCompactBitmapPageSize;
    }

    dst = (uint8_t *)CFAllocatorAllocate(allocator, __kCFCompactBitmapNumPages + (__kCFCompactBitmapPageSize * numPages), 0);

    if (numPages > 0) {
        uint8_t *dstBody = dst + __kCFCompactBitmapNumPages;

        src = bitmap;
        for (i = 0;i < __kCFCompactBitmapNumPages;i++) {
            dst[i] = header[i];
    
            if ((dst[i] != 0) && (dst[i] != UINT8_MAX)) {
                memmove(dstBody, src, __kCFCompactBitmapPageSize);
                dstBody += __kCFCompactBitmapPageSize;
            }
            src += __kCFCompactBitmapPageSize;
        }
    } else {
        memmove(dst, header, __kCFCompactBitmapNumPages);
    }

    return dst;
}

static void __CFCSetMakeCompact(CFMutableCharacterSetRef cset) {
    if (__CFCSetIsBitmap(cset) && __CFCSetBitmapBits(cset)) {
        uint8_t *bitmap = __CFCSetBitmapBits(cset);
        uint8_t *cBitmap = __CFCreateCompactBitmap(CFGetAllocator(cset), bitmap);

        if (cBitmap) {
            CFAllocatorDeallocate(CFGetAllocator(cset), bitmap);
            __CFCSetPutClassType(cset, __kCFCharSetClassCompactBitmap);
            __CFCSetPutCompactBitmapBits(cset, cBitmap);
        }
    }
}

static void __CFCSetAddNonBMPPlanesInRange(CFMutableCharacterSetRef cset, CFRange range) {
    int firstChar = (range.location & 0xFFFF);
    int maxChar = range.location + range.length;
    int idx = range.location >> 16; // first plane
    int maxPlane = (maxChar - 1) >> 16; // last plane
    CFRange planeRange;
    CFMutableCharacterSetRef annexPlane;

    maxChar &= 0xFFFF;

    for (idx = (idx ? idx : 1);idx <= maxPlane;idx++) {
        planeRange.location = __CFMax(firstChar, 0);
        planeRange.length = (idx == maxPlane && maxChar ? maxChar : 0x10000) - planeRange.location;
        if (__CFCSetAnnexIsInverted(cset)) {
            if ((annexPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSetNoAlloc(cset, idx))) {
                CFCharacterSetRemoveCharactersInRange(annexPlane, planeRange);
                if (__CFCSetIsEmpty(annexPlane) && !__CFCSetIsInverted(annexPlane)) {
                    CFRelease(annexPlane);
                    __CFCSetAnnexBitmapClearPlane(cset->_annex->_validEntriesBitmap, idx);
                }
            }
        } else {
            CFCharacterSetAddCharactersInRange((CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSet(cset, idx), planeRange);
        }
    }
    if (!__CFCSetHasNonBMPPlane(cset) && !__CFCSetAnnexIsInverted(cset)) __CFCSetDeallocateAnnexPlane(cset);
}

static void __CFCSetRemoveNonBMPPlanesInRange(CFMutableCharacterSetRef cset, CFRange range) {
    int firstChar = (range.location & 0xFFFF);
    int maxChar = range.location + range.length;
    int idx = range.location >> 16; // first plane
    int maxPlane = (maxChar - 1) >> 16; // last plane
    CFRange planeRange;
    CFMutableCharacterSetRef annexPlane;

    maxChar &= 0xFFFF;

    for (idx = (idx ? idx : 1);idx <= maxPlane;idx++) {
        planeRange.location = __CFMax(firstChar, 0);
        planeRange.length = (idx == maxPlane && maxChar ? maxChar : 0x10000) - planeRange.location;
        if (__CFCSetAnnexIsInverted(cset)) {
            CFCharacterSetAddCharactersInRange((CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSet(cset, idx), planeRange);
        } else {
            if ((annexPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSetNoAlloc(cset, idx))) {
                CFCharacterSetRemoveCharactersInRange(annexPlane, planeRange);
                if(__CFCSetIsEmpty(annexPlane) && !__CFCSetIsInverted(annexPlane)) {
                    CFRelease(annexPlane);
                    __CFCSetAnnexBitmapClearPlane(cset->_annex->_validEntriesBitmap, idx);
                }
            }
        }
    }
    if (!__CFCSetHasNonBMPPlane(cset) && !__CFCSetAnnexIsInverted(cset)) __CFCSetDeallocateAnnexPlane(cset);
}

static void __CFCSetMakeBitmap(CFMutableCharacterSetRef cset) {
    if (!__CFCSetIsBitmap(cset) || !__CFCSetBitmapBits(cset)) {
        CFAllocatorRef allocator = CFGetAllocator(cset);
        uint8_t *bitmap = (uint8_t *)CFAllocatorAllocate(allocator, __kCFBitmapSize, 0);
        __CFCSetGetBitmap(cset, bitmap);
        
        if (__CFCSetIsBuiltin(cset)) {
            CFIndex numPlanes = CFUniCharGetNumberOfPlanes(__CFCSetBuiltinType(cset));

            if (numPlanes > 1) {
                CFMutableCharacterSetRef annexSet;
                uint8_t *annexBitmap = NULL;
                int idx;
                UInt8 result;

                __CFCSetAllocateAnnexForPlane(cset, numPlanes - 1);
                for (idx = 1;idx < numPlanes;idx++) {
                    if (NULL == annexBitmap) {
                        annexBitmap = (uint8_t *)CFAllocatorAllocate(allocator, __kCFBitmapSize, 0);
                    }
                    result = CFUniCharGetBitmapForPlane(__CFCSetBuiltinType(cset), idx, annexBitmap, false);
                    if (result == kCFUniCharBitmapEmpty) continue;
                    if (result == kCFUniCharBitmapAll) {
                        CFIndex bitmapLength = __kCFBitmapSize;
                        uint8_t *bytes = annexBitmap;
                        while (bitmapLength-- > 0) *(bytes++) = (uint8_t)0xFF;
                    }
                    annexSet = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSet(cset, idx);
                    __CFCSetPutClassType(annexSet, __kCFCharSetClassBitmap);
                    __CFCSetPutBitmapBits(annexSet, annexBitmap);
                    __CFCSetPutIsInverted(annexSet, false);
                    __CFCSetPutHasHashValue(annexSet, false);
                    annexBitmap = NULL;
                }
                if (annexBitmap) CFAllocatorDeallocate(allocator, annexBitmap);
            }
        } else if (__CFCSetIsCompactBitmap(cset) && __CFCSetCompactBitmapBits(cset)) {
            CFAllocatorDeallocate(allocator, __CFCSetCompactBitmapBits(cset));
            __CFCSetPutCompactBitmapBits(cset, NULL);
        } else if (__CFCSetIsString(cset) && __CFCSetStringBuffer(cset)) {
            CFAllocatorDeallocate(allocator, __CFCSetStringBuffer(cset));
            __CFCSetPutStringBuffer(cset, NULL);
        } else if (__CFCSetIsRange(cset)) { // We may have to allocate annex here
            Boolean needsToInvert = (!__CFCSetHasNonBMPPlane(cset) && __CFCSetIsInverted(cset) ? true : false);
            __CFCSetAddNonBMPPlanesInRange(cset, CFRangeMake(__CFCSetRangeFirstChar(cset), __CFCSetRangeLength(cset)));
            if (needsToInvert) __CFCSetAnnexSetIsInverted(cset, true);
        }
        __CFCSetPutClassType(cset, __kCFCharSetClassBitmap);
        __CFCSetPutBitmapBits(cset, bitmap);
        __CFCSetPutIsInverted(cset, false);
    }    
}

CF_INLINE CFMutableCharacterSetRef __CFCSetGenericCreate(CFAllocatorRef allocator, UInt32 flags) {
    CFMutableCharacterSetRef cset;
    CFIndex size = sizeof(struct __CFCharacterSet) - sizeof(CFRuntimeBase);

    cset = (CFMutableCharacterSetRef)_CFRuntimeCreateInstance(allocator, CFCharacterSetGetTypeID(), size, NULL);
    if (NULL == cset) return NULL;

    cset->_base._cfinfo[CF_INFO_BITS] |= flags;
    cset->_hashValue = 0;
    cset->_annex = NULL;

    return cset;
}

static void __CFApplySurrogatesInString(CFMutableCharacterSetRef cset, CFStringRef string, void (*applyer)(CFMutableCharacterSetRef, CFRange)) {
    CFStringInlineBuffer buffer;
    CFIndex index, length = CFStringGetLength(string);
    CFRange range = CFRangeMake(0, 0);
    UTF32Char character;

    CFStringInitInlineBuffer(string, &buffer, CFRangeMake(0, length));

    for (index = 0;index < length;index++) {
	character = __CFStringGetCharacterFromInlineBufferQuick(&buffer, index);

	if (CFStringIsSurrogateHighCharacter(character) && ((index + 1) < length)) {
	    UTF16Char other = __CFStringGetCharacterFromInlineBufferQuick(&buffer, index + 1);

	    if (CFStringIsSurrogateLowCharacter(other)) {
		character = CFStringGetLongCharacterForSurrogatePair(character, other);

		if ((range.length + range.location) == character) {
		    ++range.length;
		} else {
		    if (range.length > 0) applyer(cset, range);
		    range.location = character;
		    range.length = 1;
		}
	    }

	    ++index; // skip the low surrogate
	}
    }

    if (range.length > 0) applyer(cset, range);
}


/* Bsearch theChar for __kCFCharSetClassString
*/
CF_INLINE Boolean __CFCSetBsearchUniChar(const UniChar *theTable, CFIndex length, UniChar theChar) {
    const UniChar *p, *q, *divider;

    if ((theChar < theTable[0]) || (theChar > theTable[length - 1])) return false;

    p = theTable;
    q = p + (length - 1);
    while (p <= q) {
        divider = p + ((q - p) >> 1);	/* divide by 2 */
        if (theChar < *divider) q = divider - 1;
        else if (theChar > *divider) p = divider + 1;
        else return true;
    }
    return false;
}

/* Array of instantiated builtin set. Note builtin set ID starts with 1 so the array index is ID - 1
*/
static CFCharacterSetRef *__CFBuiltinSets = NULL;

/* Global lock for character set
*/
static OSSpinLock __CFCharacterSetLock = OS_SPINLOCK_INIT;

/* CFBase API functions
*/
static Boolean __CFCharacterSetEqual(CFTypeRef cf1, CFTypeRef cf2) {
    Boolean isInvertStateIdentical = (__CFCSetIsInverted((CFCharacterSetRef)cf1) == __CFCSetIsInverted((CFCharacterSetRef)cf2) ? true: false);
    Boolean isAnnexInvertStateIdentical = (__CFCSetAnnexIsInverted((CFCharacterSetRef)cf1) == __CFCSetAnnexIsInverted((CFCharacterSetRef)cf2) ? true: false);
    CFIndex idx;
    CFCharacterSetRef subSet1;
    uint8_t bitsBuf[__kCFBitmapSize];
    uint8_t *bits;
    Boolean isBitmap1;
    Boolean isBitmap2;

    if (__CFCSetHasHashValue((CFCharacterSetRef)cf1) && __CFCSetHasHashValue((CFCharacterSetRef)cf2) && ((CFCharacterSetRef)cf1)->_hashValue != ((CFCharacterSetRef)cf2)->_hashValue) return false;
    if (__CFCSetIsEmpty((CFCharacterSetRef)cf1) && __CFCSetIsEmpty((CFCharacterSetRef)cf2) && !isInvertStateIdentical) return false;

    if ((__CFCSetClassType((CFCharacterSetRef)cf1) == __CFCSetClassType((CFCharacterSetRef)cf2)) && !__CFCSetIsCompactBitmap((CFCharacterSetRef)cf1)) { // Types are identical, we can do it fast
        switch (__CFCSetClassType((CFCharacterSetRef)cf1)) {
            case __kCFCharSetClassBuiltin:
                return (__CFCSetBuiltinType((CFCharacterSetRef)cf1) == __CFCSetBuiltinType((CFCharacterSetRef)cf2) && isInvertStateIdentical ? true : false);

            case __kCFCharSetClassRange:
                return (__CFCSetRangeFirstChar((CFCharacterSetRef)cf1) == __CFCSetRangeFirstChar((CFCharacterSetRef)cf2) && __CFCSetRangeLength((CFCharacterSetRef)cf1) && __CFCSetRangeLength((CFCharacterSetRef)cf2) && isInvertStateIdentical ? true : false);

            case __kCFCharSetClassString:
                if (isInvertStateIdentical) {
                    const UniChar *buf1 = __CFCSetStringBuffer((CFCharacterSetRef)cf1);
                    const UniChar *buf1End = buf1 + __CFCSetStringLength((CFCharacterSetRef)cf1);
                    const UniChar *buf2 = __CFCSetStringBuffer((CFCharacterSetRef)cf2);
                    const UniChar *buf2End = buf2 + __CFCSetStringLength((CFCharacterSetRef)cf2);

                    while ((buf1 < buf1End) && (buf2 < buf2End)) {
                        UniChar char1 = *buf1;
                        UniChar char2 = *buf2;

                        if (char1 != char2) return false;

                        do { ++buf1; } while ((buf1 < buf1End) && (char1 == *buf1));
                        do { ++buf2; } while ((buf2 < buf2End) && (char2 == *buf2));
                    }
                } else {
                    return false;
                }
                break;

            case __kCFCharSetClassBitmap:
                if (!__CFCSetIsEqualBitmap((const UInt32 *)__CFCSetBitmapBits((CFCharacterSetRef)cf1), (const UInt32 *)__CFCSetBitmapBits((CFCharacterSetRef)cf2))) return false;
                break;
        }
        return __CFCSetIsEqualAnnex((CFCharacterSetRef)cf1, (CFCharacterSetRef)cf2);
    }

    // Check for easy empty cases
    if (__CFCSetIsEmpty((CFCharacterSetRef)cf1) || __CFCSetIsEmpty((CFCharacterSetRef)cf2)) {
        CFCharacterSetRef emptySet = (__CFCSetIsEmpty((CFCharacterSetRef)cf1) ? (CFCharacterSetRef)cf1 : (CFCharacterSetRef)cf2);
        CFCharacterSetRef nonEmptySet = (emptySet == cf1 ? (CFCharacterSetRef)cf2 : (CFCharacterSetRef)cf1);

        if (__CFCSetIsBuiltin(nonEmptySet)) {
            return false;
        } else if (__CFCSetIsRange(nonEmptySet)) {
            if (isInvertStateIdentical) {
                return (__CFCSetRangeLength(nonEmptySet) ? false : true);
            } else {
                return (__CFCSetRangeLength(nonEmptySet) == 0x110000 ? true : false);
            }
        } else {
            if (__CFCSetAnnexIsInverted(nonEmptySet)) {
                if (__CFCSetAnnexValidEntriesBitmap(nonEmptySet) != 0x1FFFE) return false;
            } else {
                if (__CFCSetAnnexValidEntriesBitmap(nonEmptySet)) return false;
            }

            if (__CFCSetIsBitmap(nonEmptySet)) {
                bits = __CFCSetBitmapBits(nonEmptySet);
            } else {
                bits = bitsBuf;
                __CFCSetGetBitmap(nonEmptySet, bitsBuf);
            }

            if (__CFCSetIsEqualBitmap(NULL, (const UInt32 *)bits)) {
                if (!__CFCSetAnnexIsInverted(nonEmptySet)) return true;
            } else {
                return false;
            }

            // Annex set has to be CFRangeMake(0x10000, 0xfffff)
            for (idx = 1;idx < MAX_ANNEX_PLANE;idx++) {
                if (__CFCSetIsBitmap(nonEmptySet)) {
                    if (!__CFCSetIsEqualBitmap((__CFCSetAnnexIsInverted(nonEmptySet) ? NULL : (const UInt32 *)-1), (const UInt32 *)bitsBuf)) return false;
                } else {
                    __CFCSetGetBitmap(__CFCSetGetAnnexPlaneCharacterSetNoAlloc(nonEmptySet, idx), bitsBuf);
                    if (!__CFCSetIsEqualBitmap((const UInt32 *)-1, (const UInt32 *)bitsBuf)) return false;
                }
            }
            return true;
        }
    }

    if (__CFCSetIsBuiltin((CFCharacterSetRef)cf1) || __CFCSetIsBuiltin((CFCharacterSetRef)cf2)) {
        CFCharacterSetRef builtinSet = (__CFCSetIsBuiltin((CFCharacterSetRef)cf1) ? (CFCharacterSetRef)cf1 : (CFCharacterSetRef)cf2);
        CFCharacterSetRef nonBuiltinSet = (builtinSet == cf1 ? (CFCharacterSetRef)cf2 : (CFCharacterSetRef)cf1);
        

        if (__CFCSetIsRange(nonBuiltinSet)) {
            UTF32Char firstChar = __CFCSetRangeFirstChar(nonBuiltinSet);
            UTF32Char lastChar = (firstChar + __CFCSetRangeLength(nonBuiltinSet) - 1);
            uint8_t firstPlane = (firstChar >> 16) & 0xFF;
            uint8_t lastPlane = (lastChar >> 16) & 0xFF;
            uint8_t result;

            for (idx = 0;idx < MAX_ANNEX_PLANE;idx++) {
                result = CFUniCharGetBitmapForPlane(__CFCSetBuiltinType(builtinSet), idx, bitsBuf, (isInvertStateIdentical != 0));

                if (idx < firstPlane || idx > lastPlane) {
                    if (result == kCFUniCharBitmapAll) {
                        return false;
                    } else if (result == kCFUniCharBitmapFilled) {
                        if (!__CFCSetIsEqualBitmap(NULL, (const UInt32 *)bitsBuf)) return false;
                    }
                } else if (idx > firstPlane && idx < lastPlane) {
                    if (result == kCFUniCharBitmapEmpty) {
                        return false;
                    } else if (result == kCFUniCharBitmapFilled) {
                        if (!__CFCSetIsEqualBitmap((const UInt32 *)-1, (const UInt32 *)bitsBuf)) return false;
                    }
                } else {
                    if (result == kCFUniCharBitmapEmpty) {
                        return false;
                    } else if (result == kCFUniCharBitmapAll) {
                        if (idx == firstPlane) {
                            if (((firstChar & 0xFFFF) != 0) || (firstPlane == lastPlane && ((lastChar & 0xFFFF) != 0xFFFF))) return false;
                        } else {
                            if (((lastChar & 0xFFFF) != 0xFFFF) || (firstPlane == lastPlane && ((firstChar & 0xFFFF) != 0))) return false;
                        }
                    } else {
                        if (idx == firstPlane) {
                            if (!__CFCSetIsBitmapEqualToRange((const UInt32 *)bitsBuf, firstChar & 0xFFFF, (firstPlane == lastPlane ? lastChar & 0xFFFF : 0xFFFF), false)) return false;
                        } else {
                            if (!__CFCSetIsBitmapEqualToRange((const UInt32 *)bitsBuf, (firstPlane == lastPlane ? firstChar & 0xFFFF : 0), lastChar & 0xFFFF, false)) return false;
                        }
                    }
                }
            }
            return true;
        } else  {
            uint8_t bitsBuf2[__kCFBitmapSize];
            uint8_t result;

            result = CFUniCharGetBitmapForPlane(__CFCSetBuiltinType(builtinSet), 0, bitsBuf, (__CFCSetIsInverted(builtinSet) != 0));
            if (result == kCFUniCharBitmapFilled) {
                if (__CFCSetIsBitmap(nonBuiltinSet)) {
                    if (!__CFCSetIsEqualBitmap((const UInt32 *)bitsBuf, (const UInt32 *)__CFCSetBitmapBits(nonBuiltinSet))) return false;
                } else {

                    __CFCSetGetBitmap(nonBuiltinSet, bitsBuf2);
                    if (!__CFCSetIsEqualBitmap((const UInt32 *)bitsBuf, (const UInt32 *)bitsBuf2)) {
                        return false;
                    }
                }
            } else {
                if (__CFCSetIsBitmap(nonBuiltinSet)) {
                    if (!__CFCSetIsEqualBitmap((result == kCFUniCharBitmapAll ? (const UInt32*)-1 : NULL), (const UInt32 *)__CFCSetBitmapBits(nonBuiltinSet))) return false;
                } else {
                    __CFCSetGetBitmap(nonBuiltinSet, bitsBuf);
                    if (!__CFCSetIsEqualBitmap((result == kCFUniCharBitmapAll ? (const UInt32*)-1: NULL), (const UInt32 *)bitsBuf)) return false;
                }
            }

            isInvertStateIdentical = (__CFCSetIsInverted(builtinSet) == __CFCSetAnnexIsInverted(nonBuiltinSet) ? true : false);

            for (idx = 1;idx < MAX_ANNEX_PLANE;idx++) {
                result = CFUniCharGetBitmapForPlane(__CFCSetBuiltinType(builtinSet), idx, bitsBuf, !isInvertStateIdentical);
                subSet1 = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(nonBuiltinSet, idx);

                if (result == kCFUniCharBitmapFilled) {
                    if (NULL == subSet1) {
                        return false;
                    } else if (__CFCSetIsBitmap(subSet1)) {
                        if (!__CFCSetIsEqualBitmap((const UInt32*)bitsBuf, (const UInt32*)__CFCSetBitmapBits(subSet1))) {
                            return false;
                        }
                    } else {

                        __CFCSetGetBitmap(subSet1, bitsBuf2);
                        if (!__CFCSetIsEqualBitmap((const UInt32*)bitsBuf, (const UInt32*)bitsBuf2)) {
                            return false;
                        }
                    }
                } else {
                    if (NULL == subSet1) {
                        if (result == kCFUniCharBitmapAll) {
                            return false;
                        }
                    } else if (__CFCSetIsBitmap(subSet1)) {
                        if (!__CFCSetIsEqualBitmap((result == kCFUniCharBitmapAll ? (const UInt32*)-1: NULL), (const UInt32*)__CFCSetBitmapBits(subSet1))) {
                            return false;
                        }
                    } else {
                        __CFCSetGetBitmap(subSet1, bitsBuf);
                        if (!__CFCSetIsEqualBitmap((result == kCFUniCharBitmapAll ? (const UInt32*)-1: NULL), (const UInt32*)bitsBuf)) {
                            return false;
                        }
                    }
                }
            }
            return true;
        }
    }

    if (__CFCSetIsRange((CFCharacterSetRef)cf1) || __CFCSetIsRange((CFCharacterSetRef)cf2)) {
        CFCharacterSetRef rangeSet = (__CFCSetIsRange((CFCharacterSetRef)cf1) ? (CFCharacterSetRef)cf1 : (CFCharacterSetRef)cf2);
        CFCharacterSetRef nonRangeSet = (rangeSet == cf1 ? (CFCharacterSetRef)cf2 : (CFCharacterSetRef)cf1);
        UTF32Char firstChar = __CFCSetRangeFirstChar(rangeSet);
        UTF32Char lastChar = (firstChar + __CFCSetRangeLength(rangeSet) - 1);
        uint8_t firstPlane = (firstChar >> 16) & 0xFF;
        uint8_t lastPlane = (lastChar >> 16) & 0xFF;
        Boolean isRangeSetInverted = __CFCSetIsInverted(rangeSet);

        if (__CFCSetIsBitmap(nonRangeSet)) {
            bits = __CFCSetBitmapBits(nonRangeSet);
        } else {
            bits = bitsBuf;
            __CFCSetGetBitmap(nonRangeSet, bitsBuf);
        }
        if (firstPlane == 0) {
            if (!__CFCSetIsBitmapEqualToRange((const UInt32*)bits, firstChar, (lastPlane == 0 ? lastChar : 0xFFFF), isRangeSetInverted)) return false;
	    firstPlane = 1;
	    firstChar = 0;
        } else {
            if (!__CFCSetIsEqualBitmap((const UInt32*)bits, (isRangeSetInverted ? (const UInt32 *)-1 : NULL))) return false;
	    firstChar &= 0xFFFF;
        }

	lastChar &= 0xFFFF;

        isAnnexInvertStateIdentical = (isRangeSetInverted == __CFCSetAnnexIsInverted(nonRangeSet) ? true : false);

        for (idx = 1;idx < MAX_ANNEX_PLANE;idx++) {
            subSet1 = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(nonRangeSet, idx);
            if (NULL == subSet1) {
                if (idx < firstPlane || idx > lastPlane) {
                    if (!isAnnexInvertStateIdentical) return false;
                } else if (idx > firstPlane && idx < lastPlane) {
                    if (isAnnexInvertStateIdentical) return false;
                } else if (idx == firstPlane) {
                    if (isAnnexInvertStateIdentical || firstChar || (idx == lastPlane && lastChar != 0xFFFF)) return false;
                } else if (idx == lastPlane) {
                    if (isAnnexInvertStateIdentical || (idx == firstPlane && firstChar) || (lastChar != 0xFFFF)) return false;
                }
            } else {
                if (__CFCSetIsBitmap(subSet1)) {
                    bits = __CFCSetBitmapBits(subSet1);
                } else {
                    __CFCSetGetBitmap(subSet1, bitsBuf);
                    bits = bitsBuf;
                }

                if (idx < firstPlane || idx > lastPlane) {
                    if (!__CFCSetIsEqualBitmap((const UInt32*)bits, (isAnnexInvertStateIdentical ? NULL : (const UInt32 *)-1))) return false;
                } else if (idx > firstPlane && idx < lastPlane) {
                    if (!__CFCSetIsEqualBitmap((const UInt32*)bits, (isAnnexInvertStateIdentical ? (const UInt32 *)-1 : NULL))) return false;
                } else if (idx == firstPlane) {
                    if (!__CFCSetIsBitmapEqualToRange((const UInt32*)bits, firstChar, (idx == lastPlane ? lastChar : 0xFFFF), !isAnnexInvertStateIdentical)) return false;
                } else if (idx == lastPlane) {
                    if (!__CFCSetIsBitmapEqualToRange((const UInt32*)bits, (idx == firstPlane ? firstChar : 0), lastChar, !isAnnexInvertStateIdentical)) return false;
                }
            }
        }
        return true;
    }

    isBitmap1 = __CFCSetIsBitmap((CFCharacterSetRef)cf1);
    isBitmap2 = __CFCSetIsBitmap((CFCharacterSetRef)cf2);

    if (isBitmap1 && isBitmap2) {
        if (!__CFCSetIsEqualBitmap((const UInt32 *)__CFCSetBitmapBits((CFCharacterSetRef)cf1), (const UInt32 *)__CFCSetBitmapBits((CFCharacterSetRef)cf2))) return false;
    } else if (!isBitmap1 && !isBitmap2) {
        uint8_t bitsBuf2[__kCFBitmapSize];

        __CFCSetGetBitmap((CFCharacterSetRef)cf1, bitsBuf);
        __CFCSetGetBitmap((CFCharacterSetRef)cf2, bitsBuf2);

        if (!__CFCSetIsEqualBitmap((const UInt32*)bitsBuf, (const UInt32*)bitsBuf2)) {
            return false;
        }
    } else {
        if (isBitmap2) {
            CFCharacterSetRef tmp = (CFCharacterSetRef)cf2;
            cf2 = cf1;
            cf1 = tmp;
        }

        __CFCSetGetBitmap((CFCharacterSetRef)cf2, bitsBuf);

        if (!__CFCSetIsEqualBitmap((const UInt32 *)__CFCSetBitmapBits((CFCharacterSetRef)cf1), (const UInt32 *)bitsBuf)) return false;
    }
    return __CFCSetIsEqualAnnex((CFCharacterSetRef)cf1, (CFCharacterSetRef)cf2);
}

static CFHashCode __CFCharacterSetHash(CFTypeRef cf) {
    if (!__CFCSetHasHashValue((CFCharacterSetRef)cf)) {
        if (__CFCSetIsEmpty((CFCharacterSetRef)cf)) {
            ((CFMutableCharacterSetRef)cf)->_hashValue = (__CFCSetIsInverted((CFCharacterSetRef)cf) ? ((UInt32)0xFFFFFFFF) : 0);
        } else if (__CFCSetIsBitmap( (CFCharacterSetRef) cf  )) {
            ((CFMutableCharacterSetRef)cf)->_hashValue = CFHashBytes(__CFCSetBitmapBits((CFCharacterSetRef)cf), __kCFBitmapSize);
        } else {
            uint8_t bitsBuf[__kCFBitmapSize];
            __CFCSetGetBitmap((CFCharacterSetRef)cf, bitsBuf);
            ((CFMutableCharacterSetRef)cf)->_hashValue = CFHashBytes(bitsBuf, __kCFBitmapSize);
        }
        __CFCSetPutHasHashValue((CFMutableCharacterSetRef)cf, true);
    }
    return ((CFCharacterSetRef)cf)->_hashValue;
}

static CFStringRef  __CFCharacterSetCopyDescription(CFTypeRef cf) {
    CFMutableStringRef string;
    CFIndex idx;
    CFIndex length;

    if (__CFCSetIsEmpty((CFCharacterSetRef)cf)) {
	return (CFStringRef)(__CFCSetIsInverted((CFCharacterSetRef)cf) ? CFRetain(CFSTR("<CFCharacterSet All>")) : CFRetain(CFSTR("<CFCharacterSet Empty>")));
    }

    switch (__CFCSetClassType((CFCharacterSetRef)cf)) {
        case __kCFCharSetClassBuiltin:
            switch (__CFCSetBuiltinType((CFCharacterSetRef)cf)) {
                case kCFCharacterSetControl: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined Control Set>"));
                case kCFCharacterSetWhitespace : return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined Whitespace Set>"));
                case kCFCharacterSetWhitespaceAndNewline: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined WhitespaceAndNewline Set>"));
                case kCFCharacterSetDecimalDigit: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined DecimalDigit Set>"));
                case kCFCharacterSetLetter: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined Letter Set>"));
                case kCFCharacterSetLowercaseLetter: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined LowercaseLetter Set>"));
                case kCFCharacterSetUppercaseLetter: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined UppercaseLetter Set>"));
                case kCFCharacterSetNonBase: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined NonBase Set>"));
                case kCFCharacterSetDecomposable: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined Decomposable Set>"));
                case kCFCharacterSetAlphaNumeric: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined AlphaNumeric Set>"));
                case kCFCharacterSetPunctuation: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined Punctuation Set>"));
                case kCFCharacterSetIllegal: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined Illegal Set>"));
                case kCFCharacterSetCapitalizedLetter: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined CapitalizedLetter Set>"));
                case kCFCharacterSetSymbol: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined Symbol Set>"));
                case kCFCharacterSetNewline: return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Predefined Newline Set>"));
            }
            break;

        case __kCFCharSetClassRange:
            return CFStringCreateWithFormat(CFGetAllocator((CFCharacterSetRef)cf), NULL, CFSTR("<CFCharacterSet Range(%u, %ld)>"), (unsigned int)__CFCSetRangeFirstChar((CFCharacterSetRef)cf), (long)__CFCSetRangeLength((CFCharacterSetRef)cf));

        case __kCFCharSetClassString: {
            CFStringRef format = CFSTR("<CFCharacterSet Items(");

            length = __CFCSetStringLength((CFCharacterSetRef)cf);
            string = CFStringCreateMutable(CFGetAllocator(cf), CFStringGetLength(format) + 7 * length + 2); // length of format + "U+XXXX "(7) * length + ")>"(2)
            CFStringAppend(string, format);
            for (idx = 0;idx < length;idx++) {
                CFStringAppendFormat(string, NULL, CFSTR("%sU+%04X"), (idx > 0 ? " " : ""), (unsigned int)((__CFCSetStringBuffer((CFCharacterSetRef)cf))[idx]));
            }
            CFStringAppend(string, CFSTR(")>"));
            return string;
        }

        case __kCFCharSetClassBitmap:
        case __kCFCharSetClassCompactBitmap:
            return (CFStringRef)CFRetain(CFSTR("<CFCharacterSet Bitmap>")); // ??? Should generate description for 8k bitmap ?
    }
    CFAssert1(0, __kCFLogAssertion, "%s: Internal inconsistency error: unknown character set type", __PRETTY_FUNCTION__); // We should never come here
    return NULL;
}

static void __CFCharacterSetDeallocate(CFTypeRef cf) {
    CFAllocatorRef allocator = CFGetAllocator(cf);

    if (__CFCSetIsBuiltin((CFCharacterSetRef)cf) && !__CFCSetIsMutable((CFCharacterSetRef)cf) && !__CFCSetIsInverted((CFCharacterSetRef)cf)) {
        CFCharacterSetRef sharedSet = CFCharacterSetGetPredefined(__CFCSetBuiltinType((CFCharacterSetRef)cf));
        if (sharedSet == cf) { // We're trying to dealloc the builtin set
            CFAssert1(0, __kCFLogAssertion, "%s: Trying to deallocate predefined set. The process is likely to crash.", __PRETTY_FUNCTION__);
            return; // We never deallocate builtin set
        }
    }

    if (__CFCSetIsString((CFCharacterSetRef)cf) && __CFCSetStringBuffer((CFCharacterSetRef)cf)) CFAllocatorDeallocate(allocator, __CFCSetStringBuffer((CFCharacterSetRef)cf));
    else if (__CFCSetIsBitmap((CFCharacterSetRef)cf) && __CFCSetBitmapBits((CFCharacterSetRef)cf)) CFAllocatorDeallocate(allocator, __CFCSetBitmapBits((CFCharacterSetRef)cf));
    else if (__CFCSetIsCompactBitmap((CFCharacterSetRef)cf) && __CFCSetCompactBitmapBits((CFCharacterSetRef)cf)) CFAllocatorDeallocate(allocator, __CFCSetCompactBitmapBits((CFCharacterSetRef)cf));
    __CFCSetDeallocateAnnexPlane((CFCharacterSetRef)cf);
}

static CFTypeID __kCFCharacterSetTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFCharacterSetClass = {
    0,
    "CFCharacterSet",
    NULL,      // init
    NULL,      // copy
    __CFCharacterSetDeallocate,
    __CFCharacterSetEqual,
    __CFCharacterSetHash,
    NULL,      // 
    __CFCharacterSetCopyDescription
};

static bool __CFCheckForExapendedSet = false;

CF_PRIVATE void __CFCharacterSetInitialize(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{
        __kCFCharacterSetTypeID = _CFRuntimeRegisterClass(&__CFCharacterSetClass); // initOnce covered
        const char *checkForExpandedSet = __CFgetenv("__CF_DEBUG_EXPANDED_SET");
        if (checkForExpandedSet && (*checkForExpandedSet == 'Y')) __CFCheckForExapendedSet = true;
        __CFBuiltinSets = (CFCharacterSetRef *)CFAllocatorAllocate((CFAllocatorRef)CFRetain(__CFGetDefaultAllocator()), sizeof(CFCharacterSetRef) * __kCFLastBuiltinSetID, 0);
        memset(__CFBuiltinSets, 0, sizeof(CFCharacterSetRef) * __kCFLastBuiltinSetID);
    });
}

/* Public functions
*/

CFTypeID CFCharacterSetGetTypeID(void) {
    return __kCFCharacterSetTypeID;
}

/*** CharacterSet creation ***/
/* Functions to create basic immutable characterset.
*/
CFCharacterSetRef CFCharacterSetGetPredefined(CFCharacterSetPredefinedSet theSetIdentifier) {
    CFCharacterSetRef cset;

    __CFCSetValidateBuiltinType(theSetIdentifier, __PRETTY_FUNCTION__);

    OSSpinLockLock(&__CFCharacterSetLock);
    cset = ((NULL != __CFBuiltinSets) ? __CFBuiltinSets[theSetIdentifier - 1] : NULL);
    OSSpinLockUnlock(&__CFCharacterSetLock);

    if (NULL != cset) return cset;

    if (!(cset = __CFCSetGenericCreate(kCFAllocatorSystemDefault, __kCFCharSetClassBuiltin))) return NULL;
    __CFCSetPutBuiltinType((CFMutableCharacterSetRef)cset, theSetIdentifier);

    OSSpinLockLock(&__CFCharacterSetLock);
    __CFBuiltinSets[theSetIdentifier - 1] = cset;
    OSSpinLockUnlock(&__CFCharacterSetLock);

    return cset;
}

CFCharacterSetRef CFCharacterSetCreateWithCharactersInRange(CFAllocatorRef allocator, CFRange theRange) {
    CFMutableCharacterSetRef cset;

    __CFCSetValidateRange(theRange, __PRETTY_FUNCTION__);

    if (theRange.length) {
        if (!(cset = __CFCSetGenericCreate(allocator, __kCFCharSetClassRange))) return NULL;
        __CFCSetPutRangeFirstChar(cset, theRange.location);
        __CFCSetPutRangeLength(cset, theRange.length);
    } else {
        if (!(cset = __CFCSetGenericCreate(allocator, __kCFCharSetClassBitmap))) return NULL;
        __CFCSetPutBitmapBits(cset, NULL);
        __CFCSetPutHasHashValue(cset, true); // _hashValue is 0
    }

    return cset;
}

static int chcompar(const void *a, const void *b) {
    return -(int)(*(UniChar *)b - *(UniChar *)a);
}

CFCharacterSetRef CFCharacterSetCreateWithCharactersInString(CFAllocatorRef allocator, CFStringRef theString) {
    CFIndex length;

    length = CFStringGetLength(theString);
    if (length < __kCFStringCharSetMax) {
        CFMutableCharacterSetRef cset;

        if (!(cset = __CFCSetGenericCreate(allocator, __kCFCharSetClassString))) return NULL;
        __CFCSetPutStringBuffer(cset, (UniChar *)CFAllocatorAllocate(CFGetAllocator(cset), __kCFStringCharSetMax * sizeof(UniChar), 0));
        __CFCSetPutStringLength(cset, length);
        CFStringGetCharacters(theString, CFRangeMake(0, length), __CFCSetStringBuffer(cset));
        qsort(__CFCSetStringBuffer(cset), length, sizeof(UniChar), chcompar);

        if (0 == length) {
	    __CFCSetPutHasHashValue(cset, true); // _hashValue is 0
	} else if (length > 1) { // Check for surrogate
	    const UTF16Char *characters = __CFCSetStringBuffer(cset);
	    const UTF16Char *charactersLimit = characters + length;

	    if ((*characters < 0xDC00UL) && (*(charactersLimit - 1) > 0xDBFFUL)) { // might have surrogate chars
		while (characters < charactersLimit) {
		    if (CFStringIsSurrogateHighCharacter(*characters) || CFStringIsSurrogateLowCharacter(*characters)) {
			CFRelease(cset);
			cset = NULL;
			break;
		    }
		    ++characters;
		}
	    }		
	}
	if (NULL != cset) return cset;
    }

    CFMutableCharacterSetRef mcset = CFCharacterSetCreateMutable(allocator);
    CFCharacterSetAddCharactersInString(mcset, theString);
    __CFCSetMakeCompact(mcset);
    __CFCSetPutIsMutable(mcset, false);
    return mcset;
}

CFCharacterSetRef CFCharacterSetCreateWithBitmapRepresentation(CFAllocatorRef allocator, CFDataRef theData) {
    CFMutableCharacterSetRef cset;
    CFIndex length;

    if (!(cset = __CFCSetGenericCreate(allocator, __kCFCharSetClassBitmap))) return NULL;

    if (theData && (length = CFDataGetLength(theData)) > 0) {
        uint8_t *bitmap;
        uint8_t *cBitmap;

        if (length < __kCFBitmapSize) {
            bitmap = (uint8_t *)CFAllocatorAllocate(allocator, __kCFBitmapSize, 0);
            memmove(bitmap, CFDataGetBytePtr(theData), length);
            memset(bitmap + length, 0, __kCFBitmapSize - length);

            cBitmap = __CFCreateCompactBitmap(allocator, bitmap);

            if (cBitmap == NULL) {
                __CFCSetPutBitmapBits(cset, bitmap);
            } else {
                CFAllocatorDeallocate(allocator, bitmap);
                __CFCSetPutCompactBitmapBits(cset, cBitmap);
                __CFCSetPutClassType(cset, __kCFCharSetClassCompactBitmap);
            }
        } else {
            cBitmap = __CFCreateCompactBitmap(allocator, CFDataGetBytePtr(theData));

            if (cBitmap == NULL) {
                bitmap = (uint8_t *)CFAllocatorAllocate(allocator, __kCFBitmapSize, 0);
                memmove(bitmap, CFDataGetBytePtr(theData), __kCFBitmapSize);

                __CFCSetPutBitmapBits(cset, bitmap);
            } else {
                __CFCSetPutCompactBitmapBits(cset, cBitmap);
                __CFCSetPutClassType(cset, __kCFCharSetClassCompactBitmap);
            }

            if (length > __kCFBitmapSize) {
                CFMutableCharacterSetRef annexSet;
                const uint8_t *bytes = CFDataGetBytePtr(theData) + __kCFBitmapSize;
    
                length -= __kCFBitmapSize;
    
                while (length > 1) {
                    annexSet = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSet(cset, *(bytes++));
                    --length; // Decrement the plane no byte

                    if (length < __kCFBitmapSize) {
                        bitmap = (uint8_t *)CFAllocatorAllocate(allocator, __kCFBitmapSize, 0);
                        memmove(bitmap, bytes, length);
                        memset(bitmap + length, 0, __kCFBitmapSize - length);
            
                        cBitmap = __CFCreateCompactBitmap(allocator, bitmap);
            
                        if (cBitmap == NULL) {
                            __CFCSetPutBitmapBits(annexSet, bitmap);
                        } else {
                            CFAllocatorDeallocate(allocator, bitmap);
                            __CFCSetPutCompactBitmapBits(annexSet, cBitmap);
                            __CFCSetPutClassType(annexSet, __kCFCharSetClassCompactBitmap);
                        }
                    } else {
                        cBitmap = __CFCreateCompactBitmap(allocator, bytes);
            
                        if (cBitmap == NULL) {
                            bitmap = (uint8_t *)CFAllocatorAllocate(allocator, __kCFBitmapSize, 0);
                            memmove(bitmap, bytes, __kCFBitmapSize);
            
                            __CFCSetPutBitmapBits(annexSet, bitmap);
                        } else {
                            __CFCSetPutCompactBitmapBits(annexSet, cBitmap);
                            __CFCSetPutClassType(annexSet, __kCFCharSetClassCompactBitmap);
                        }
                    }
                    length -= __kCFBitmapSize;
                    bytes += __kCFBitmapSize;
                }
            }
        }
    } else {
        __CFCSetPutBitmapBits(cset, NULL);
        __CFCSetPutHasHashValue(cset, true); // Hash value is 0
    }

    return cset;
}

CFCharacterSetRef CFCharacterSetCreateInvertedSet(CFAllocatorRef alloc, CFCharacterSetRef theSet) {
    CFMutableCharacterSetRef cset;
    
    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, CFCharacterSetRef , (NSCharacterSet *)theSet, invertedSet);

    cset = CFCharacterSetCreateMutableCopy(alloc, theSet);
    CFCharacterSetInvert(cset);
    __CFCSetPutIsMutable(cset, false);

    return cset;
}

/* Functions to create mutable characterset.
*/
CFMutableCharacterSetRef CFCharacterSetCreateMutable(CFAllocatorRef allocator) {
    CFMutableCharacterSetRef cset;

    if (!(cset = __CFCSetGenericCreate(allocator, __kCFCharSetClassBitmap| __kCFCharSetIsMutable))) return NULL;
    __CFCSetPutBitmapBits(cset, NULL);
    __CFCSetPutHasHashValue(cset, true); // Hash value is 0

    return cset;
}

static CFMutableCharacterSetRef __CFCharacterSetCreateCopy(CFAllocatorRef alloc, CFCharacterSetRef theSet, bool isMutable) {
    CFMutableCharacterSetRef cset;

    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, CFMutableCharacterSetRef , (NSCharacterSet *)theSet, mutableCopy);

    __CFGenericValidateType(theSet, __kCFCharacterSetTypeID);

    if (!isMutable && !__CFCSetIsMutable(theSet)) {
        return (CFMutableCharacterSetRef)CFRetain(theSet);
    }

    cset = CFCharacterSetCreateMutable(alloc);

    __CFCSetPutClassType(cset, __CFCSetClassType(theSet));
    __CFCSetPutHasHashValue(cset, __CFCSetHasHashValue(theSet));
    __CFCSetPutIsInverted(cset, __CFCSetIsInverted(theSet));
    cset->_hashValue = theSet->_hashValue;

    switch (__CFCSetClassType(theSet)) {
        case __kCFCharSetClassBuiltin:
            __CFCSetPutBuiltinType(cset, __CFCSetBuiltinType(theSet));
            break;

        case __kCFCharSetClassRange:
            __CFCSetPutRangeFirstChar(cset, __CFCSetRangeFirstChar(theSet));
            __CFCSetPutRangeLength(cset, __CFCSetRangeLength(theSet));
            break;

        case __kCFCharSetClassString:
			__CFCSetPutStringBuffer(cset, (UniChar *)CFAllocatorAllocate(alloc, __kCFStringCharSetMax * sizeof(UniChar), 0));

            __CFCSetPutStringLength(cset, __CFCSetStringLength(theSet));
            memmove(__CFCSetStringBuffer(cset), __CFCSetStringBuffer(theSet), __CFCSetStringLength(theSet) * sizeof(UniChar));
            break;

        case __kCFCharSetClassBitmap:
            if (__CFCSetBitmapBits(theSet)) {
                uint8_t * bitmap = (isMutable ? NULL : __CFCreateCompactBitmap(alloc, __CFCSetBitmapBits(theSet)));

                if (bitmap == NULL) {
                    bitmap = (uint8_t *)CFAllocatorAllocate(alloc, sizeof(uint8_t) * __kCFBitmapSize, 0);
                    memmove(bitmap, __CFCSetBitmapBits(theSet), __kCFBitmapSize);
                    __CFCSetPutBitmapBits(cset, bitmap);
                } else {
                    __CFCSetPutCompactBitmapBits(cset, bitmap);
                    __CFCSetPutClassType(cset, __kCFCharSetClassCompactBitmap);
                }
            } else {
                __CFCSetPutBitmapBits(cset, NULL);
            }
            break;

        case __kCFCharSetClassCompactBitmap: {
            const uint8_t *compactBitmap = __CFCSetCompactBitmapBits(theSet);

            if (compactBitmap) {
                uint32_t size = __CFCSetGetCompactBitmapSize(compactBitmap);
                uint8_t *newBitmap = (uint8_t *)CFAllocatorAllocate(alloc, size, 0);

                memmove(newBitmap, compactBitmap, size);
                __CFCSetPutCompactBitmapBits(cset, newBitmap);
            }
        }
            break;

        default:
            CFAssert1(0, __kCFLogAssertion, "%s: Internal inconsistency error: unknown character set type", __PRETTY_FUNCTION__); // We should never come here
    }
    if (__CFCSetHasNonBMPPlane(theSet)) {
        CFMutableCharacterSetRef annexPlane;
        int idx;

        for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
            if ((annexPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, idx))) {
                annexPlane = __CFCharacterSetCreateCopy(alloc, annexPlane, isMutable);
                __CFCSetPutCharacterSetToAnnexPlane(cset, annexPlane, idx);
                CFRelease(annexPlane);
            }
        }
        __CFCSetAnnexSetIsInverted(cset, __CFCSetAnnexIsInverted(theSet));
    } else if (__CFCSetAnnexIsInverted(theSet)) {
        __CFCSetAnnexSetIsInverted(cset, true);
    }
    
    return cset;
}

CFCharacterSetRef CFCharacterSetCreateCopy(CFAllocatorRef alloc, CFCharacterSetRef theSet) {
    return __CFCharacterSetCreateCopy(alloc, theSet, false);
}

CFMutableCharacterSetRef CFCharacterSetCreateMutableCopy(CFAllocatorRef alloc, CFCharacterSetRef theSet) {
    return __CFCharacterSetCreateCopy(alloc, theSet, true);
}

/*** Basic accessors ***/
Boolean CFCharacterSetIsCharacterMember(CFCharacterSetRef theSet, UniChar theChar) {
    CFIndex length;
    Boolean isInverted;
    Boolean result = false;
    
    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, Boolean, (NSCharacterSet *)theSet, longCharacterIsMember:(UTF32Char)theChar);
    
    __CFGenericValidateType(theSet, __kCFCharacterSetTypeID);
    
    isInverted = __CFCSetIsInverted(theSet);
    
    switch (__CFCSetClassType(theSet)) {
        case __kCFCharSetClassBuiltin:
            result = (CFUniCharIsMemberOf(theChar, __CFCSetBuiltinType(theSet)) ? !isInverted : isInverted);
            break;
            
        case __kCFCharSetClassRange:
            length = __CFCSetRangeLength(theSet);
            result = (length && __CFCSetRangeFirstChar(theSet) <= theChar && theChar < __CFCSetRangeFirstChar(theSet) + length ? !isInverted : isInverted);
            break;
            
        case __kCFCharSetClassString:
            result = ((length = __CFCSetStringLength(theSet)) ? (__CFCSetBsearchUniChar(__CFCSetStringBuffer(theSet), length, theChar) ? !isInverted : isInverted) : isInverted);
            break;
            
        case __kCFCharSetClassBitmap:
            result = (__CFCSetCompactBitmapBits(theSet) ? (__CFCSetIsMemberBitmap(__CFCSetBitmapBits(theSet), theChar) ? true : false) : isInverted);
            break;
            
        case __kCFCharSetClassCompactBitmap:
            result = (__CFCSetCompactBitmapBits(theSet) ? (__CFCSetIsMemberInCompactBitmap(__CFCSetCompactBitmapBits(theSet), theChar) ? true : false) : isInverted);
            break;
            
        default:
            CFAssert1(0, __kCFLogAssertion, "%s: Internal inconsistency error: unknown character set type", __PRETTY_FUNCTION__); // We should never come here
            break;
    }
    
    return result;
}

Boolean CFCharacterSetIsLongCharacterMember(CFCharacterSetRef theSet, UTF32Char theChar) {
    CFIndex length;
    UInt32 plane = (theChar >> 16);
    Boolean isAnnexInverted = false;
    Boolean isInverted;
    Boolean result = false;

    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, Boolean, (NSCharacterSet *)theSet, longCharacterIsMember:(UTF32Char)theChar);

    __CFGenericValidateType(theSet, __kCFCharacterSetTypeID);

    if (plane) {
        CFCharacterSetRef annexPlane;

        if (__CFCSetIsBuiltin(theSet)) {
            isInverted = __CFCSetIsInverted(theSet);
            return (CFUniCharIsMemberOf(theChar, __CFCSetBuiltinType(theSet)) ? !isInverted : isInverted); 
        }

        isAnnexInverted = __CFCSetAnnexIsInverted(theSet);

        if ((annexPlane = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, plane)) == NULL) {
            if (!__CFCSetHasNonBMPPlane(theSet) && __CFCSetIsRange(theSet)) {
                isInverted = __CFCSetIsInverted(theSet);
                length = __CFCSetRangeLength(theSet);
                return (length && __CFCSetRangeFirstChar(theSet) <= theChar && theChar < __CFCSetRangeFirstChar(theSet) + length ? !isInverted : isInverted);
            } else {
                return (isAnnexInverted ? true : false);
            }
        } else {
            theSet = annexPlane;
            theChar &= 0xFFFF;
        }
    }

    isInverted = __CFCSetIsInverted(theSet);

    switch (__CFCSetClassType(theSet)) {
        case __kCFCharSetClassBuiltin:
            result = (CFUniCharIsMemberOf(theChar, __CFCSetBuiltinType(theSet)) ? !isInverted : isInverted);
            break;

        case __kCFCharSetClassRange:
            length = __CFCSetRangeLength(theSet);
            result = (length && __CFCSetRangeFirstChar(theSet) <= theChar && theChar < __CFCSetRangeFirstChar(theSet) + length ? !isInverted : isInverted);
            break;

        case __kCFCharSetClassString:
            result = ((length = __CFCSetStringLength(theSet)) ? (__CFCSetBsearchUniChar(__CFCSetStringBuffer(theSet), length, theChar) ? !isInverted : isInverted) : isInverted);
            break;

        case __kCFCharSetClassBitmap:
            result = (__CFCSetCompactBitmapBits(theSet) ? (__CFCSetIsMemberBitmap(__CFCSetBitmapBits(theSet), theChar) ? true : false) : isInverted);
            break;

        case __kCFCharSetClassCompactBitmap:
            result = (__CFCSetCompactBitmapBits(theSet) ? (__CFCSetIsMemberInCompactBitmap(__CFCSetCompactBitmapBits(theSet), theChar) ? true : false) : isInverted);
            break;

        default:
            CFAssert1(0, __kCFLogAssertion, "%s: Internal inconsistency error: unknown character set type", __PRETTY_FUNCTION__); // We should never come here
            return false; // To make compiler happy
    }

    return (result ? !isAnnexInverted : isAnnexInverted);
}

Boolean CFCharacterSetIsSurrogatePairMember(CFCharacterSetRef theSet, UniChar surrogateHigh, UniChar surrogateLow) {
    return CFCharacterSetIsLongCharacterMember(theSet, CFCharacterSetGetLongCharacterForSurrogatePair(surrogateHigh, surrogateLow));
}


static inline CFCharacterSetRef __CFCharacterSetGetExpandedSetForNSCharacterSet(const void *characterSet) {
    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, CFCharacterSetRef , (NSCharacterSet *)characterSet, _expandedCFCharacterSet);
    return NULL;
}

Boolean CFCharacterSetIsSupersetOfSet(CFCharacterSetRef theSet, CFCharacterSetRef theOtherSet) {
    CFMutableCharacterSetRef copy;
    CFCharacterSetRef expandedSet = NULL;
    CFCharacterSetRef expandedOtherSet = NULL;
    Boolean result;

    if ((!CF_IS_OBJC(__kCFCharacterSetTypeID, theSet) || (expandedSet = __CFCharacterSetGetExpandedSetForNSCharacterSet(theSet))) && (!CF_IS_OBJC(__kCFCharacterSetTypeID, theOtherSet) || (expandedOtherSet = __CFCharacterSetGetExpandedSetForNSCharacterSet(theOtherSet)))) { // Really CF, we can do some trick here
        if (expandedSet) theSet = expandedSet;
        if (expandedOtherSet) theOtherSet = expandedOtherSet;
    
        __CFGenericValidateType(theSet, __kCFCharacterSetTypeID);
        __CFGenericValidateType(theOtherSet, __kCFCharacterSetTypeID);

        if (__CFCSetIsEmpty(theSet)) {
            if (__CFCSetIsInverted(theSet)) {
                return TRUE; // Inverted empty set covers all range
            } else if (!__CFCSetIsEmpty(theOtherSet) || __CFCSetIsInverted(theOtherSet)) {
                return FALSE;
            }
        } else if (__CFCSetIsEmpty(theOtherSet) && !__CFCSetIsInverted(theOtherSet)) {
            return TRUE;
        } else {
            if (__CFCSetIsBuiltin(theSet) || __CFCSetIsBuiltin(theOtherSet)) {
                if (__CFCSetClassType(theSet) == __CFCSetClassType(theOtherSet) && __CFCSetBuiltinType(theSet) == __CFCSetBuiltinType(theOtherSet) && !__CFCSetIsInverted(theSet) && !__CFCSetIsInverted(theOtherSet)) return TRUE;
            } else if (__CFCSetIsRange(theSet) || __CFCSetIsRange(theOtherSet)) {
                if (__CFCSetClassType(theSet) == __CFCSetClassType(theOtherSet)) {
                    if (__CFCSetIsInverted(theSet)) {
                        if (__CFCSetIsInverted(theOtherSet)) {
                            return (__CFCSetRangeFirstChar(theOtherSet) > __CFCSetRangeFirstChar(theSet) || (__CFCSetRangeFirstChar(theSet) + __CFCSetRangeLength(theSet)) > (__CFCSetRangeFirstChar(theOtherSet) + __CFCSetRangeLength(theOtherSet)) ? FALSE : TRUE);
                        } else {
                            return ((__CFCSetRangeFirstChar(theOtherSet) + __CFCSetRangeLength(theOtherSet)) <= __CFCSetRangeFirstChar(theSet) || (__CFCSetRangeFirstChar(theSet) + __CFCSetRangeLength(theSet)) <= __CFCSetRangeFirstChar(theOtherSet) ? TRUE : FALSE);
                        }
                    } else {
                        if (__CFCSetIsInverted(theOtherSet)) {
                            return ((__CFCSetRangeFirstChar(theSet) == 0 && __CFCSetRangeLength(theSet) == 0x110000) || (__CFCSetRangeFirstChar(theOtherSet) == 0 && (UInt32)__CFCSetRangeLength(theOtherSet) <= __CFCSetRangeFirstChar(theSet)) || ((__CFCSetRangeFirstChar(theSet) + __CFCSetRangeLength(theSet)) <= __CFCSetRangeFirstChar(theOtherSet) && (__CFCSetRangeFirstChar(theOtherSet) + __CFCSetRangeLength(theOtherSet)) == 0x110000) ? TRUE : FALSE);
                        } else {
                            return (__CFCSetRangeFirstChar(theOtherSet) < __CFCSetRangeFirstChar(theSet) || (__CFCSetRangeFirstChar(theSet) + __CFCSetRangeLength(theSet)) < (__CFCSetRangeFirstChar(theOtherSet) + __CFCSetRangeLength(theOtherSet)) ? FALSE : TRUE);
                        }
                    }
                }
            } else {
                UInt32 theSetAnnexMask = __CFCSetAnnexValidEntriesBitmap(theSet);
                UInt32 theOtherSetAnnexMask = __CFCSetAnnexValidEntriesBitmap(theOtherSet);
                Boolean isTheSetAnnexInverted = __CFCSetAnnexIsInverted(theSet);
                Boolean isTheOtherSetAnnexInverted = __CFCSetAnnexIsInverted(theOtherSet);
                uint8_t theSetBuffer[__kCFBitmapSize];
                uint8_t theOtherSetBuffer[__kCFBitmapSize];

                // We mask plane 1 to plane 16
                if (isTheSetAnnexInverted) theSetAnnexMask = (~theSetAnnexMask) & (0xFFFF << 1);
                if (isTheOtherSetAnnexInverted) theOtherSetAnnexMask = (~theOtherSetAnnexMask) & (0xFFFF << 1);

                __CFCSetGetBitmap(theSet, theSetBuffer);
                __CFCSetGetBitmap(theOtherSet, theOtherSetBuffer);

                if (!__CFCSetIsBitmapSupersetOfBitmap((const UInt32 *)theSetBuffer, (const UInt32 *)theOtherSetBuffer, FALSE, FALSE)) return FALSE;

                if (theOtherSetAnnexMask) {
                    CFCharacterSetRef theSetAnnex;
                    CFCharacterSetRef theOtherSetAnnex;
                    uint32_t idx;

                    if ((theSetAnnexMask & theOtherSetAnnexMask) != theOtherSetAnnexMask) return FALSE;

                    for (idx = 1;idx <= 16;idx++) {
                        theSetAnnex = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, idx);
                        if (NULL == theSetAnnex) continue; // This case is already handled by the mask above

                        theOtherSetAnnex = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(theOtherSet, idx);

                        if (NULL == theOtherSetAnnex) {
                            if (isTheOtherSetAnnexInverted) {
                                __CFCSetGetBitmap(theSetAnnex, theSetBuffer);
                                if (!__CFCSetIsEqualBitmap((const UInt32 *)theSetBuffer, (isTheSetAnnexInverted ? NULL : (const UInt32 *)-1))) return FALSE;
                            }
                        } else {
                            __CFCSetGetBitmap(theSetAnnex, theSetBuffer);
                            __CFCSetGetBitmap(theOtherSetAnnex, theOtherSetBuffer);
                            if (!__CFCSetIsBitmapSupersetOfBitmap((const UInt32 *)theSetBuffer, (const UInt32 *)theOtherSetBuffer, isTheSetAnnexInverted, isTheOtherSetAnnexInverted)) return FALSE;
                        }
                    }
                }

                return TRUE;
            }
        }
    }

    copy = CFCharacterSetCreateMutableCopy(kCFAllocatorSystemDefault, theSet);
    CFCharacterSetIntersect(copy, theOtherSet);
    result = __CFCharacterSetEqual(copy, theOtherSet);
    CFRelease(copy);

    return result;
}

Boolean CFCharacterSetHasMemberInPlane(CFCharacterSetRef theSet, CFIndex thePlane) {
    Boolean isInverted = __CFCSetIsInverted(theSet);

    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, Boolean, (NSCharacterSet *)theSet, hasMemberInPlane:(uint8_t)thePlane);

    if (__CFCSetIsEmpty(theSet)) {
        return (isInverted ? TRUE : FALSE);
    } else if (__CFCSetIsBuiltin(theSet)) {
        CFCharacterSetPredefinedSet type = __CFCSetBuiltinType(theSet);

        if (type == kCFCharacterSetControl) {
            if (isInverted || (thePlane == 14)) {
                return TRUE; // There is no plane that covers all values || Plane 14 has language tags
            } else {
                return (CFUniCharGetBitmapPtrForPlane(type, thePlane) ? TRUE : FALSE);
            }
        } else if ((type < kCFCharacterSetDecimalDigit) || (type == kCFCharacterSetNewline)) {
            return (thePlane && !isInverted ? FALSE : TRUE);
        } else if (__CFCSetBuiltinType(theSet) == kCFCharacterSetIllegal) {
            return (isInverted ? (thePlane < 3 || thePlane > 13 ? TRUE : FALSE) : TRUE); // This is according to Unicode 3.1
        } else {
            if (isInverted) {
                return TRUE; // There is no plane that covers all values
            } else {
                return (CFUniCharGetBitmapPtrForPlane(type, thePlane) ? TRUE : FALSE);
            }
        }
    } else if (__CFCSetIsRange(theSet)) {
        UTF32Char firstChar = __CFCSetRangeFirstChar(theSet);
        UTF32Char lastChar = (firstChar + __CFCSetRangeLength(theSet) - 1);
        CFIndex firstPlane = firstChar >> 16;
        CFIndex lastPlane = lastChar >> 16;

        if (isInverted) {
            if (thePlane < firstPlane || thePlane > lastPlane) {
                return TRUE;
            } else if (thePlane > firstPlane && thePlane < lastPlane) {
                return FALSE;
            } else {
                firstChar &= 0xFFFF;
                lastChar &= 0xFFFF;
                if (thePlane == firstPlane) {
                    return (firstChar || (firstPlane == lastPlane && lastChar != 0xFFFF) ? TRUE : FALSE);
                } else {
                    return (lastChar != 0xFFFF || (firstPlane == lastPlane && firstChar) ? TRUE : FALSE);
                }
            }
        } else {
            return (thePlane < firstPlane || thePlane > lastPlane ? FALSE : TRUE);
        }
    } else {
        if (thePlane == 0) {
            switch (__CFCSetClassType(theSet)) {
                case __kCFCharSetClassString: if (!__CFCSetStringLength(theSet)) return isInverted; break;
                case __kCFCharSetClassCompactBitmap: return (__CFCSetCompactBitmapBits(theSet) ? TRUE : FALSE); break;
                case __kCFCharSetClassBitmap: return (__CFCSetBitmapBits(theSet) ? TRUE : FALSE); break;
            }
            return TRUE;
        } else {
            CFCharacterSetRef annex = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, thePlane);
            if (annex) {
                if (__CFCSetIsRange(annex)) {
                    return (__CFCSetAnnexIsInverted(theSet) && (__CFCSetRangeFirstChar(annex) == 0) && (__CFCSetRangeLength(annex) == 0x10000) ? FALSE : TRUE);
                } else if (__CFCSetIsBitmap(annex)) {
                    return (__CFCSetAnnexIsInverted(theSet) && __CFCSetIsEqualBitmap((const UInt32 *)__CFCSetBitmapBits(annex), (const UInt32 *)-1) ? FALSE : TRUE);
                } else {
                    uint8_t bitsBuf[__kCFBitmapSize];
                    __CFCSetGetBitmap(annex, bitsBuf);
                    return (__CFCSetAnnexIsInverted(theSet) && __CFCSetIsEqualBitmap((const UInt32 *)bitsBuf, (const UInt32 *)-1) ? FALSE : TRUE);
                }
            } else {
                return __CFCSetAnnexIsInverted(theSet);
            }
        }
    }
 
    return FALSE;
}


CFDataRef CFCharacterSetCreateBitmapRepresentation(CFAllocatorRef alloc, CFCharacterSetRef theSet) {
    CFMutableDataRef data;
    int numNonBMPPlanes = 0;
    int planeIndices[MAX_ANNEX_PLANE];
    int idx;
    int length;
    bool isAnnexInverted;

    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, CFDataRef , (NSCharacterSet *)theSet, _retainedBitmapRepresentation);

    __CFGenericValidateType(theSet, __kCFCharacterSetTypeID);

    isAnnexInverted = (__CFCSetAnnexIsInverted(theSet) != 0);

    if (__CFCSetHasNonBMPPlane(theSet)) {
        for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
            if (isAnnexInverted || __CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, idx)) {
                planeIndices[numNonBMPPlanes++] = idx;
            }
        }
    } else if (__CFCSetIsBuiltin(theSet)) {
        numNonBMPPlanes = (__CFCSetIsInverted(theSet) ? MAX_ANNEX_PLANE : CFUniCharGetNumberOfPlanes(__CFCSetBuiltinType(theSet)) - 1);
    } else if (__CFCSetIsRange(theSet)) {
        UInt32 firstChar = __CFCSetRangeFirstChar(theSet);
        UInt32 lastChar = __CFCSetRangeFirstChar(theSet) + __CFCSetRangeLength(theSet) - 1;
        int firstPlane = (firstChar >> 16);
        int lastPlane = (lastChar >> 16);
        bool isInverted = (__CFCSetIsInverted(theSet) != 0);

        if (lastPlane > 0) {
            if (firstPlane == 0) {
                firstPlane = 1;
                firstChar = 0x10000;
            }
            numNonBMPPlanes = (lastPlane - firstPlane) + 1;
            if (isInverted) {
                numNonBMPPlanes = MAX_ANNEX_PLANE - numNonBMPPlanes;
                if (firstPlane == lastPlane) {
                    if (((firstChar & 0xFFFF) > 0) || ((lastChar & 0xFFFF) < 0xFFFF)) ++numNonBMPPlanes;
                } else {
                    if ((firstChar & 0xFFFF) > 0) ++numNonBMPPlanes;
                    if ((lastChar & 0xFFFF) < 0xFFFF) ++numNonBMPPlanes;
                }
            }
        } else if (isInverted) {
	    numNonBMPPlanes = MAX_ANNEX_PLANE;
	}
    } else if (isAnnexInverted) {
        numNonBMPPlanes = MAX_ANNEX_PLANE;
    }

    length = __kCFBitmapSize + ((__kCFBitmapSize + 1) * numNonBMPPlanes);
    data = CFDataCreateMutable(alloc, length);
    CFDataSetLength(data, length);
    __CFCSetGetBitmap(theSet, CFDataGetMutableBytePtr(data));

    if (numNonBMPPlanes > 0) {
        uint8_t *bytes = CFDataGetMutableBytePtr(data) + __kCFBitmapSize;

        if (__CFCSetHasNonBMPPlane(theSet)) {
            CFCharacterSetRef subset;

            for (idx = 0;idx < numNonBMPPlanes;idx++) {
                *(bytes++) = planeIndices[idx];
                if ((subset = __CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, planeIndices[idx])) == NULL) {
                    __CFCSetBitmapFastFillWithValue((UInt32 *)bytes, (isAnnexInverted ? 0xFF : 0));
                } else {
                    __CFCSetGetBitmap(subset, bytes);
		    if (isAnnexInverted) {
			uint32_t count = __kCFBitmapSize / sizeof(uint32_t);
			uint32_t *bits = (uint32_t *)bytes;

			while (count-- > 0) {
			    *bits = ~(*bits);
			    ++bits;
			}
		    }
                }
                bytes += __kCFBitmapSize;
            }
        } else if (__CFCSetIsBuiltin(theSet)) {
            UInt8 result;
            CFIndex delta;
            Boolean isInverted = __CFCSetIsInverted(theSet);

            for (idx = 0;idx < numNonBMPPlanes;idx++) {
                if ((result = CFUniCharGetBitmapForPlane(__CFCSetBuiltinType(theSet), idx + 1, bytes + 1,  (isInverted != 0))) == kCFUniCharBitmapEmpty) continue;
                *(bytes++) = idx + 1;
                if (result == kCFUniCharBitmapAll) {
                    CFIndex bitmapLength = __kCFBitmapSize;
                    while (bitmapLength-- > 0) *(bytes++) = (uint8_t)0xFF;
                } else {
                    bytes += __kCFBitmapSize;
                }
            }
            delta = bytes - (const uint8_t *)CFDataGetBytePtr(data);
            if (delta < length) CFDataSetLength(data, delta);
        } else if (__CFCSetIsRange(theSet)) {
            UInt32 firstChar = __CFCSetRangeFirstChar(theSet);
            UInt32 lastChar = __CFCSetRangeFirstChar(theSet) + __CFCSetRangeLength(theSet) - 1;
            int firstPlane = (firstChar >> 16);
            int lastPlane = (lastChar >> 16);

            if (firstPlane == 0) {
                firstPlane = 1;
                firstChar = 0x10000;
            }
            if (__CFCSetIsInverted(theSet)) {
                // Mask out the plane byte
                firstChar &= 0xFFFF;
                lastChar &= 0xFFFF;

                for (idx = 1;idx < firstPlane;idx++) { // Fill up until the first plane
                    *(bytes++) = idx;
                    __CFCSetBitmapFastFillWithValue((UInt32 *)bytes, 0xFF);
                    bytes += __kCFBitmapSize;
                }
                if (firstPlane == lastPlane) {
                    if ((firstChar > 0) || (lastChar < 0xFFFF)) {
                        *(bytes++) = idx;
                   	__CFCSetBitmapFastFillWithValue((UInt32 *)bytes, 0xFF);
                        __CFCSetBitmapRemoveCharactersInRange(bytes, firstChar, lastChar);
                        bytes += __kCFBitmapSize;
                    }
                } else if (firstPlane < lastPlane) {
                    if (firstChar > 0) {
                        *(bytes++) = idx;
                   	__CFCSetBitmapFastFillWithValue((UInt32 *)bytes, 0);
                        __CFCSetBitmapAddCharactersInRange(bytes, 0, firstChar - 1);
                        bytes += __kCFBitmapSize;
                    }
                    if (lastChar < 0xFFFF) {
                        *(bytes++) = idx;
                   	__CFCSetBitmapFastFillWithValue((UInt32 *)bytes, 0);
                        __CFCSetBitmapAddCharactersInRange(bytes, lastChar, 0xFFFF);
                        bytes += __kCFBitmapSize;
                    }
                }
                for (idx = lastPlane + 1;idx <= MAX_ANNEX_PLANE;idx++) {
                    *(bytes++) = idx;
                    __CFCSetBitmapFastFillWithValue((UInt32 *)bytes, 0xFF);
                    bytes += __kCFBitmapSize;
                }
            } else {
                for (idx = firstPlane;idx <= lastPlane;idx++) {
                    *(bytes++) = idx;
                    __CFCSetBitmapAddCharactersInRange(bytes, (idx == firstPlane ? firstChar : 0), (idx == lastPlane ? lastChar : 0xFFFF));
		    bytes += __kCFBitmapSize;
                }
            }
        } else if (isAnnexInverted) {
            for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
                *(bytes++) = idx;
                __CFCSetBitmapFastFillWithValue((UInt32 *)bytes, 0xFF);
                bytes += __kCFBitmapSize;
            }
        }
    }

    return data;
}

/*** MutableCharacterSet functions ***/
void CFCharacterSetAddCharactersInRange(CFMutableCharacterSetRef theSet, CFRange theRange) {
    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, void, (NSMutableCharacterSet *)theSet, addCharactersInRange:NSMakeRange(theRange.location, theRange.length));

    __CFCSetValidateTypeAndMutability(theSet, __PRETTY_FUNCTION__);
    __CFCSetValidateRange(theRange, __PRETTY_FUNCTION__);

    if (__CFCSetIsBuiltin((CFCharacterSetRef)theSet) && !__CFCSetIsMutable((CFCharacterSetRef)theSet) && !__CFCSetIsInverted((CFCharacterSetRef)theSet)) {
        CFCharacterSetRef sharedSet = CFCharacterSetGetPredefined(__CFCSetBuiltinType((CFCharacterSetRef)theSet));
        if (sharedSet == theSet) { // We're trying to dealloc the builtin set
            CFAssert1(0, __kCFLogAssertion, "%s: Trying to mutable predefined set.", __PRETTY_FUNCTION__);
            return; // We don't mutate builtin set
        }
    }

    if (!theRange.length || (__CFCSetIsInverted(theSet) && __CFCSetIsEmpty(theSet))) return; // Inverted && empty set contains all char

    if (!__CFCSetIsInverted(theSet)) {
        if (__CFCSetIsEmpty(theSet)) {
            __CFCSetPutClassType(theSet, __kCFCharSetClassRange);
            __CFCSetPutRangeFirstChar(theSet, theRange.location);
            __CFCSetPutRangeLength(theSet, theRange.length);
            __CFCSetPutHasHashValue(theSet, false);
            return;
       } else if (__CFCSetIsRange(theSet)) {
            CFIndex firstChar = __CFCSetRangeFirstChar(theSet);
            CFIndex length = __CFCSetRangeLength(theSet);

            if (firstChar == theRange.location) {
                __CFCSetPutRangeLength(theSet, __CFMax(length, theRange.length));
                __CFCSetPutHasHashValue(theSet, false);
                return;
            } else if (firstChar < theRange.location && theRange.location <= firstChar + length) {
                if (firstChar + length < theRange.location + theRange.length) __CFCSetPutRangeLength(theSet, theRange.length + (theRange.location - firstChar));
                __CFCSetPutHasHashValue(theSet, false);
                return;
            } else if (theRange.location < firstChar && firstChar <= theRange.location + theRange.length) {
                __CFCSetPutRangeFirstChar(theSet, theRange.location);
                __CFCSetPutRangeLength(theSet, length + (firstChar - theRange.location));
                __CFCSetPutHasHashValue(theSet, false);
                return;
            }
        } else if (__CFCSetIsString(theSet) && __CFCSetStringLength(theSet) + theRange.length < __kCFStringCharSetMax) {
            UniChar *buffer;
            if (!__CFCSetStringBuffer(theSet))
				__CFCSetPutStringBuffer(theSet, (UniChar *)CFAllocatorAllocate(CFGetAllocator(theSet), __kCFStringCharSetMax * sizeof(UniChar), 0));
            buffer = __CFCSetStringBuffer(theSet) + __CFCSetStringLength(theSet);
            __CFCSetPutStringLength(theSet, __CFCSetStringLength(theSet) + theRange.length);
            while (theRange.length--) *buffer++ = (UniChar)theRange.location++;
            qsort(__CFCSetStringBuffer(theSet), __CFCSetStringLength(theSet), sizeof(UniChar), chcompar);
            __CFCSetPutHasHashValue(theSet, false);
            return;
        }
    }

    // OK, I have to be a bitmap
    __CFCSetMakeBitmap(theSet);
    __CFCSetAddNonBMPPlanesInRange(theSet, theRange);
    if (theRange.location < 0x10000) { // theRange is in BMP
        if (theRange.location + theRange.length >= NUMCHARACTERS) theRange.length = NUMCHARACTERS - theRange.location;
        __CFCSetBitmapAddCharactersInRange(__CFCSetBitmapBits(theSet), (UniChar)theRange.location, (UniChar)(theRange.location + theRange.length - 1));
    }
    __CFCSetPutHasHashValue(theSet, false);

    if (__CFCheckForExapendedSet) __CFCheckForExpandedSet(theSet);
}

void CFCharacterSetRemoveCharactersInRange(CFMutableCharacterSetRef theSet, CFRange theRange) {
    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, void, (NSMutableCharacterSet *)theSet, removeCharactersInRange:NSMakeRange(theRange.location, theRange.length));

    __CFCSetValidateTypeAndMutability(theSet, __PRETTY_FUNCTION__);
    __CFCSetValidateRange(theRange, __PRETTY_FUNCTION__);
    
    if (__CFCSetIsBuiltin((CFCharacterSetRef)theSet) && !__CFCSetIsMutable((CFCharacterSetRef)theSet) && !__CFCSetIsInverted((CFCharacterSetRef)theSet)) {
        CFCharacterSetRef sharedSet = CFCharacterSetGetPredefined(__CFCSetBuiltinType((CFCharacterSetRef)theSet));
        if (sharedSet == theSet) { // We're trying to dealloc the builtin set
            CFAssert1(0, __kCFLogAssertion, "%s: Trying to mutable predefined set.", __PRETTY_FUNCTION__);
            return; // We don't mutate builtin set
        }
    }

    if (!theRange.length || (!__CFCSetIsInverted(theSet) && __CFCSetIsEmpty(theSet))) return; // empty set

    if (__CFCSetIsInverted(theSet)) {
        if (__CFCSetIsEmpty(theSet)) {
            __CFCSetPutClassType(theSet, __kCFCharSetClassRange);
            __CFCSetPutRangeFirstChar(theSet, theRange.location);
            __CFCSetPutRangeLength(theSet, theRange.length);
            __CFCSetPutHasHashValue(theSet, false);
            return;
       } else if (__CFCSetIsRange(theSet)) {
            CFIndex firstChar = __CFCSetRangeFirstChar(theSet);
            CFIndex length = __CFCSetRangeLength(theSet);

            if (firstChar == theRange.location) {
                __CFCSetPutRangeLength(theSet, __CFMin(length, theRange.length));
                __CFCSetPutHasHashValue(theSet, false);
                return;
            } else if (firstChar < theRange.location && theRange.location <= firstChar + length) {
                if (firstChar + length < theRange.location + theRange.length) __CFCSetPutRangeLength(theSet, theRange.length + (theRange.location - firstChar));
                __CFCSetPutHasHashValue(theSet, false);
                return;
            } else if (theRange.location < firstChar && firstChar <= theRange.location + theRange.length) {
                __CFCSetPutRangeFirstChar(theSet, theRange.location);
                __CFCSetPutRangeLength(theSet, length + (firstChar - theRange.location));
                __CFCSetPutHasHashValue(theSet, false);
                return;
            }
        } else if (__CFCSetIsString(theSet) && __CFCSetStringLength(theSet) + theRange.length < __kCFStringCharSetMax) {
            UniChar *buffer;
            if (!__CFCSetStringBuffer(theSet))
				__CFCSetPutStringBuffer(theSet, (UniChar *)CFAllocatorAllocate(CFGetAllocator(theSet), __kCFStringCharSetMax * sizeof(UniChar), 0));
            buffer = __CFCSetStringBuffer(theSet) + __CFCSetStringLength(theSet);
            __CFCSetPutStringLength(theSet, __CFCSetStringLength(theSet) + theRange.length);
            while (theRange.length--) *buffer++ = (UniChar)theRange.location++;
            qsort(__CFCSetStringBuffer(theSet), __CFCSetStringLength(theSet), sizeof(UniChar), chcompar);
            __CFCSetPutHasHashValue(theSet, false);
            return;
        }
    }

    // OK, I have to be a bitmap
    __CFCSetMakeBitmap(theSet);
    __CFCSetRemoveNonBMPPlanesInRange(theSet, theRange);
    if (theRange.location < 0x10000) { // theRange is in BMP
        if (theRange.location + theRange.length > NUMCHARACTERS) theRange.length = NUMCHARACTERS - theRange.location;
        if (theRange.location == 0 && theRange.length == NUMCHARACTERS) { // Remove all
            CFAllocatorDeallocate(CFGetAllocator(theSet), __CFCSetBitmapBits(theSet));
            __CFCSetPutBitmapBits(theSet, NULL);
        } else {
            __CFCSetBitmapRemoveCharactersInRange(__CFCSetBitmapBits(theSet), (UniChar)theRange.location, (UniChar)(theRange.location + theRange.length - 1));
        }
    }

    __CFCSetPutHasHashValue(theSet, false);
    if (__CFCheckForExapendedSet) __CFCheckForExpandedSet(theSet);
}

void CFCharacterSetAddCharactersInString(CFMutableCharacterSetRef theSet,  CFStringRef theString) {
    UniChar *buffer;
    CFIndex length;
    BOOL hasSurrogate = NO;

    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, void, (NSMutableCharacterSet *)theSet, addCharactersInString:(NSString *)theString);

    __CFCSetValidateTypeAndMutability(theSet, __PRETTY_FUNCTION__);
    
    if (__CFCSetIsBuiltin((CFCharacterSetRef)theSet) && !__CFCSetIsMutable((CFCharacterSetRef)theSet) && !__CFCSetIsInverted((CFCharacterSetRef)theSet)) {
        CFCharacterSetRef sharedSet = CFCharacterSetGetPredefined(__CFCSetBuiltinType((CFCharacterSetRef)theSet));
        if (sharedSet == theSet) { // We're trying to dealloc the builtin set
            CFAssert1(0, __kCFLogAssertion, "%s: Trying to mutable predefined set.", __PRETTY_FUNCTION__);
            return; // We don't mutate builtin set
        }
    }

    if ((__CFCSetIsEmpty(theSet) && __CFCSetIsInverted(theSet)) || !(length = CFStringGetLength(theString))) return;

    if (!__CFCSetIsInverted(theSet)) {
        CFIndex newLength = length + (__CFCSetIsEmpty(theSet) ? 0 : (__CFCSetIsString(theSet) ? __CFCSetStringLength(theSet) : __kCFStringCharSetMax));

        if (newLength < __kCFStringCharSetMax) {
	    buffer = __CFCSetStringBuffer(theSet);

	    if (NULL == buffer) {
		buffer = (UniChar *)CFAllocatorAllocate(CFGetAllocator(theSet), __kCFStringCharSetMax * sizeof(UniChar), 0);
	    } else {
		buffer += __CFCSetStringLength(theSet);
	    }

            CFStringGetCharacters(theString, CFRangeMake(0, length), (UniChar*)buffer);

	    if (length > 1) {
		UTF16Char *characters = buffer;
		const UTF16Char *charactersLimit = characters + length;
		
		while (characters < charactersLimit) {
		    if (CFStringIsSurrogateHighCharacter(*characters) || CFStringIsSurrogateLowCharacter(*characters)) {
			memmove(characters, characters + 1, (charactersLimit - (characters + 1)) * sizeof(*characters));
			--charactersLimit;
			hasSurrogate = YES;
		    } else {
			++characters;
		    }
		}

		newLength -= (length - (charactersLimit - buffer));
	    }

	    if (0 == newLength) {
		if (NULL == __CFCSetStringBuffer(theSet)) CFAllocatorDeallocate(CFGetAllocator(theSet), buffer);
	    } else {
		if (NULL == __CFCSetStringBuffer(theSet)) {
		    __CFCSetPutClassType(theSet, __kCFCharSetClassString);
		    __CFCSetPutStringBuffer(theSet, buffer);
		}
		__CFCSetPutStringLength(theSet, newLength);
		qsort(__CFCSetStringBuffer(theSet), newLength, sizeof(UniChar), chcompar);
	    }
            __CFCSetPutHasHashValue(theSet, false);

	    if (hasSurrogate) __CFApplySurrogatesInString(theSet, theString, &CFCharacterSetAddCharactersInRange);

            return;
        }
    }

    // OK, I have to be a bitmap
    __CFCSetMakeBitmap(theSet);
    CFStringInlineBuffer inlineBuffer;
    CFIndex idx;
    
    CFStringInitInlineBuffer(theString, &inlineBuffer, CFRangeMake(0, length));

    for (idx = 0;idx < length;idx++) {
	UTF16Char character = __CFStringGetCharacterFromInlineBufferQuick(&inlineBuffer, idx);

	if (CFStringIsSurrogateHighCharacter(character) || CFStringIsSurrogateLowCharacter(character)) {
	    hasSurrogate = YES;
	} else {
	    __CFCSetBitmapAddCharacter(__CFCSetBitmapBits(theSet), character);
	}
    }

    __CFCSetPutHasHashValue(theSet, false);

    if (__CFCheckForExapendedSet) __CFCheckForExpandedSet(theSet);

    if (hasSurrogate) __CFApplySurrogatesInString(theSet, theString, &CFCharacterSetAddCharactersInRange);
}

void CFCharacterSetRemoveCharactersInString(CFMutableCharacterSetRef theSet, CFStringRef theString) {
    UniChar *buffer;
    CFIndex length;
    BOOL hasSurrogate = NO;

    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, void, (NSMutableCharacterSet *)theSet, removeCharactersInString:(NSString *)theString);

    __CFCSetValidateTypeAndMutability(theSet, __PRETTY_FUNCTION__);
    
    if (__CFCSetIsBuiltin((CFCharacterSetRef)theSet) && !__CFCSetIsMutable((CFCharacterSetRef)theSet) && !__CFCSetIsInverted((CFCharacterSetRef)theSet)) {
        CFCharacterSetRef sharedSet = CFCharacterSetGetPredefined(__CFCSetBuiltinType((CFCharacterSetRef)theSet));
        if (sharedSet == theSet) { // We're trying to dealloc the builtin set
            CFAssert1(0, __kCFLogAssertion, "%s: Trying to mutable predefined set.", __PRETTY_FUNCTION__);
            return; // We don't mutate builtin set
        }
    }

    if ((__CFCSetIsEmpty(theSet) && !__CFCSetIsInverted(theSet)) || !(length = CFStringGetLength(theString))) return;

    if (__CFCSetIsInverted(theSet)) {
        CFIndex newLength = length + (__CFCSetIsEmpty(theSet) ? 0 : (__CFCSetIsString(theSet) ? __CFCSetStringLength(theSet) : __kCFStringCharSetMax));
	
        if (newLength < __kCFStringCharSetMax) {
	    buffer = __CFCSetStringBuffer(theSet);
	    
	    if (NULL == buffer) {
		buffer = (UniChar *)CFAllocatorAllocate(CFGetAllocator(theSet), __kCFStringCharSetMax * sizeof(UniChar), 0);
	    } else {
		buffer += __CFCSetStringLength(theSet);
	    }
	    
            CFStringGetCharacters(theString, CFRangeMake(0, length), (UniChar*)buffer);
	    
	    if (length > 1) {
		UTF16Char *characters = buffer;
		const UTF16Char *charactersLimit = characters + length;
		
		while (characters < charactersLimit) {
		    if (CFStringIsSurrogateHighCharacter(*characters) || CFStringIsSurrogateLowCharacter(*characters)) {
			memmove(characters, characters + 1, charactersLimit - (characters + 1));
			--charactersLimit;
			hasSurrogate = YES;
		    }
		    ++characters;
		}
		
		newLength -= (length - (charactersLimit - buffer));
	    }
	    
	    if (NULL == __CFCSetStringBuffer(theSet)) {
		__CFCSetPutClassType(theSet, __kCFCharSetClassString);
		__CFCSetPutStringBuffer(theSet, buffer);
	    }
            __CFCSetPutStringLength(theSet, newLength);
            qsort(__CFCSetStringBuffer(theSet), newLength, sizeof(UniChar), chcompar);
            __CFCSetPutHasHashValue(theSet, false);
	    
	    if (hasSurrogate) __CFApplySurrogatesInString(theSet, theString, &CFCharacterSetRemoveCharactersInRange);
	    
            return;
        }
    }

    // OK, I have to be a bitmap
    __CFCSetMakeBitmap(theSet);
    CFStringInlineBuffer inlineBuffer;
    CFIndex idx;
    
    CFStringInitInlineBuffer(theString, &inlineBuffer, CFRangeMake(0, length));
    
    for (idx = 0;idx < length;idx++) {
	UTF16Char character = __CFStringGetCharacterFromInlineBufferQuick(&inlineBuffer, idx);
	
	if (CFStringIsSurrogateHighCharacter(character) || CFStringIsSurrogateLowCharacter(character)) {
	    hasSurrogate = YES;
	} else {
	    __CFCSetBitmapRemoveCharacter(__CFCSetBitmapBits(theSet), character);
	}
    }
    
    __CFCSetPutHasHashValue(theSet, false);
    if (__CFCheckForExapendedSet) __CFCheckForExpandedSet(theSet);

    if (hasSurrogate) __CFApplySurrogatesInString(theSet, theString, &CFCharacterSetRemoveCharactersInRange);
}

void CFCharacterSetUnion(CFMutableCharacterSetRef theSet, CFCharacterSetRef theOtherSet) {
    CFCharacterSetRef expandedSet = NULL;

    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, void, (NSMutableCharacterSet *)theSet, formUnionWithCharacterSet:(NSCharacterSet *)theOtherSet);

    __CFCSetValidateTypeAndMutability(theSet, __PRETTY_FUNCTION__);
    
    if (__CFCSetIsBuiltin((CFCharacterSetRef)theSet) && !__CFCSetIsMutable((CFCharacterSetRef)theSet) && !__CFCSetIsInverted((CFCharacterSetRef)theSet)) {
        CFCharacterSetRef sharedSet = CFCharacterSetGetPredefined(__CFCSetBuiltinType((CFCharacterSetRef)theSet));
        if (sharedSet == theSet) { // We're trying to dealloc the builtin set
            CFAssert1(0, __kCFLogAssertion, "%s: Trying to mutable predefined set.", __PRETTY_FUNCTION__);
            return; // We don't mutate builtin set
        }
    }

    if (__CFCSetIsEmpty(theSet) && __CFCSetIsInverted(theSet)) return; // Inverted empty set contains all char

    if (!CF_IS_OBJC(__kCFCharacterSetTypeID, theOtherSet) || (expandedSet = __CFCharacterSetGetExpandedSetForNSCharacterSet(theOtherSet))) { // Really CF, we can do some trick here
        if (expandedSet) theOtherSet = expandedSet;

        if (__CFCSetIsEmpty(theOtherSet)) {
            if (__CFCSetIsInverted(theOtherSet)) {
                if (__CFCSetIsString(theSet) && __CFCSetStringBuffer(theSet)) {
                    CFAllocatorDeallocate(CFGetAllocator(theSet), __CFCSetStringBuffer(theSet));
                } else if (__CFCSetIsBitmap(theSet) && __CFCSetBitmapBits(theSet)) {
                    CFAllocatorDeallocate(CFGetAllocator(theSet), __CFCSetBitmapBits(theSet));
                } else if (__CFCSetIsCompactBitmap(theSet) && __CFCSetCompactBitmapBits(theSet)) {
                    CFAllocatorDeallocate(CFGetAllocator(theSet), __CFCSetCompactBitmapBits(theSet));
                }
                __CFCSetPutClassType(theSet, __kCFCharSetClassRange);
                __CFCSetPutRangeLength(theSet, 0);
                __CFCSetPutIsInverted(theSet, true);
                __CFCSetPutHasHashValue(theSet, false);
                __CFCSetDeallocateAnnexPlane(theSet);
            }
        } else if (__CFCSetIsBuiltin(theOtherSet) && __CFCSetIsEmpty(theSet)) { // theSet can be builtin set
            __CFCSetPutClassType(theSet, __kCFCharSetClassBuiltin);
            __CFCSetPutBuiltinType(theSet, __CFCSetBuiltinType(theOtherSet));
	    if (__CFCSetIsInverted(theOtherSet)) __CFCSetPutIsInverted(theSet, true);
	    if (__CFCSetAnnexIsInverted(theOtherSet)) __CFCSetAnnexSetIsInverted(theSet, true);
            __CFCSetPutHasHashValue(theSet, false);
        } else {
            if (__CFCSetIsRange(theOtherSet)) {
                if (__CFCSetIsInverted(theOtherSet)) {
                    UTF32Char firstChar = __CFCSetRangeFirstChar(theOtherSet);
                    CFIndex length = __CFCSetRangeLength(theOtherSet);

                    if (firstChar > 0) CFCharacterSetAddCharactersInRange(theSet, CFRangeMake(0, firstChar));
                    firstChar += length;
                    length = 0x110000 - firstChar;
                    CFCharacterSetAddCharactersInRange(theSet, CFRangeMake(firstChar, length));
                } else {
                    CFCharacterSetAddCharactersInRange(theSet, CFRangeMake(__CFCSetRangeFirstChar(theOtherSet), __CFCSetRangeLength(theOtherSet)));
                }
            } else if (__CFCSetIsString(theOtherSet)) {
                CFStringRef string = CFStringCreateWithCharactersNoCopy(CFGetAllocator(theSet), __CFCSetStringBuffer(theOtherSet), __CFCSetStringLength(theOtherSet), kCFAllocatorNull);
                CFCharacterSetAddCharactersInString(theSet, string);
                CFRelease(string);
            } else {
                __CFCSetMakeBitmap(theSet);
                if (__CFCSetIsBitmap(theOtherSet)) {
                    UInt32 *bitmap1 = (UInt32*)__CFCSetBitmapBits(theSet);
                    UInt32 *bitmap2 = (UInt32*)__CFCSetBitmapBits(theOtherSet);
                    CFIndex length = __kCFBitmapSize / sizeof(UInt32);
                    while (length--) *bitmap1++ |= *bitmap2++;
                } else {
                    UInt32 *bitmap1 = (UInt32*)__CFCSetBitmapBits(theSet);
                    UInt32 *bitmap2;
                    CFIndex length = __kCFBitmapSize / sizeof(UInt32);
                    uint8_t bitmapBuffer[__kCFBitmapSize];
                    __CFCSetGetBitmap(theOtherSet, bitmapBuffer);
                    bitmap2 = (UInt32*)bitmapBuffer;
                    while (length--) *bitmap1++ |= *bitmap2++;
                }
                __CFCSetPutHasHashValue(theSet, false);
            }
            if (__CFCSetHasNonBMPPlane(theOtherSet)) {
                CFMutableCharacterSetRef otherSetPlane;
                int idx;

                for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
                    if ((otherSetPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSetNoAlloc(theOtherSet, idx))) {
                        CFCharacterSetUnion((CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSet(theSet, idx), otherSetPlane);
                    }
                }
	    } else if (__CFCSetAnnexIsInverted(theOtherSet)) {
		if (__CFCSetHasNonBMPPlane(theSet)) __CFCSetDeallocateAnnexPlane(theSet);
		__CFCSetAnnexSetIsInverted(theSet, true);
            } else if (__CFCSetIsBuiltin(theOtherSet)) {
                CFMutableCharacterSetRef annexPlane;
                uint8_t bitmapBuffer[__kCFBitmapSize];
                uint8_t result;
                int planeIndex;
                Boolean isOtherAnnexPlaneInverted = __CFCSetAnnexIsInverted(theOtherSet);
                UInt32 *bitmap1;
                UInt32 *bitmap2;
                CFIndex length;

                for (planeIndex = 1;planeIndex <= MAX_ANNEX_PLANE;planeIndex++) {
                    result = CFUniCharGetBitmapForPlane(__CFCSetBuiltinType(theOtherSet), planeIndex, bitmapBuffer, (isOtherAnnexPlaneInverted != 0));
                    if (result != kCFUniCharBitmapEmpty) {
                        annexPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSet(theSet, planeIndex);
                        if (result == kCFUniCharBitmapAll) {
                            CFCharacterSetAddCharactersInRange(annexPlane, CFRangeMake(0x0000, 0x10000));
                        } else {
                            __CFCSetMakeBitmap(annexPlane);
                            bitmap1 = (UInt32 *)__CFCSetBitmapBits(annexPlane);
                            length = __kCFBitmapSize / sizeof(UInt32);
                            bitmap2 = (UInt32*)bitmapBuffer;
                            while (length--) *bitmap1++ |= *bitmap2++;
                        }
                    }
                }
            }
        }
        if (__CFCheckForExapendedSet) __CFCheckForExpandedSet(theSet);
    } else { // It's NSCharacterSet
        CFDataRef bitmapRep = CFCharacterSetCreateBitmapRepresentation(kCFAllocatorSystemDefault, theOtherSet);
        const UInt32 *bitmap2 = (bitmapRep && CFDataGetLength(bitmapRep) ? (const UInt32 *)CFDataGetBytePtr(bitmapRep) : NULL);
        if (bitmap2) {
            UInt32 *bitmap1;
            CFIndex length = __kCFBitmapSize / sizeof(UInt32);
            __CFCSetMakeBitmap(theSet);
            bitmap1 = (UInt32*)__CFCSetBitmapBits(theSet);
            while (length--) *bitmap1++ |= *bitmap2++;
            __CFCSetPutHasHashValue(theSet, false);
        }
        CFRelease(bitmapRep);
    }
}

void CFCharacterSetIntersect(CFMutableCharacterSetRef theSet, CFCharacterSetRef theOtherSet) {
    CFCharacterSetRef expandedSet = NULL;

    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, void, (NSMutableCharacterSet *)theSet, formIntersectionWithCharacterSet:(NSCharacterSet *)theOtherSet);

    __CFCSetValidateTypeAndMutability(theSet, __PRETTY_FUNCTION__);
    
    if (__CFCSetIsBuiltin((CFCharacterSetRef)theSet) && !__CFCSetIsMutable((CFCharacterSetRef)theSet) && !__CFCSetIsInverted((CFCharacterSetRef)theSet)) {
        CFCharacterSetRef sharedSet = CFCharacterSetGetPredefined(__CFCSetBuiltinType((CFCharacterSetRef)theSet));
        if (sharedSet == theSet) { // We're trying to dealloc the builtin set
            CFAssert1(0, __kCFLogAssertion, "%s: Trying to mutable predefined set.", __PRETTY_FUNCTION__);
            return; // We don't mutate builtin set
        }
    }

    if (__CFCSetIsEmpty(theSet) && !__CFCSetIsInverted(theSet)) return; // empty set

    if (!CF_IS_OBJC(__kCFCharacterSetTypeID, theOtherSet) || (expandedSet = __CFCharacterSetGetExpandedSetForNSCharacterSet(theOtherSet))) { // Really CF, we can do some trick here
        if (expandedSet) theOtherSet = expandedSet;

        if (__CFCSetIsEmpty(theOtherSet)) {
           if (!__CFCSetIsInverted(theOtherSet)) {
                if (__CFCSetIsString(theSet) && __CFCSetStringBuffer(theSet)) {
                    CFAllocatorDeallocate(CFGetAllocator(theSet), __CFCSetStringBuffer(theSet));
                } else if (__CFCSetIsBitmap(theSet) && __CFCSetBitmapBits(theSet)) {
                    CFAllocatorDeallocate(CFGetAllocator(theSet), __CFCSetBitmapBits(theSet));
                } else if (__CFCSetIsCompactBitmap(theSet) && __CFCSetCompactBitmapBits(theSet)) {
                    CFAllocatorDeallocate(CFGetAllocator(theSet), __CFCSetCompactBitmapBits(theSet));
                }
                __CFCSetPutClassType(theSet, __kCFCharSetClassBitmap);
                __CFCSetPutBitmapBits(theSet, NULL);
                __CFCSetPutIsInverted(theSet, false);
                theSet->_hashValue = 0;
                __CFCSetPutHasHashValue(theSet, true);
                __CFCSetDeallocateAnnexPlane(theSet);
            }
        } else if (__CFCSetIsEmpty(theSet)) { // non inverted empty set contains all character
            __CFCSetPutClassType(theSet, __CFCSetClassType(theOtherSet));
            __CFCSetPutHasHashValue(theSet, __CFCSetHasHashValue(theOtherSet));
            __CFCSetPutIsInverted(theSet, __CFCSetIsInverted(theOtherSet));
            theSet->_hashValue = theOtherSet->_hashValue;
            if (__CFCSetHasNonBMPPlane(theOtherSet)) {
                CFMutableCharacterSetRef otherSetPlane;
                int idx;
                for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
                    if ((otherSetPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSetNoAlloc(theOtherSet, idx))) {
                        otherSetPlane = (CFMutableCharacterSetRef)CFCharacterSetCreateMutableCopy(CFGetAllocator(theSet), otherSetPlane);
                        __CFCSetPutCharacterSetToAnnexPlane(theSet, otherSetPlane, idx);
                        CFRelease(otherSetPlane);
                    }
                }
                __CFCSetAnnexSetIsInverted(theSet, __CFCSetAnnexIsInverted(theOtherSet));
            }

            switch (__CFCSetClassType(theOtherSet)) {
                case __kCFCharSetClassBuiltin:
                    __CFCSetPutBuiltinType(theSet, __CFCSetBuiltinType(theOtherSet));
                    break;

                case __kCFCharSetClassRange:
                    __CFCSetPutRangeFirstChar(theSet, __CFCSetRangeFirstChar(theOtherSet));
                    __CFCSetPutRangeLength(theSet, __CFCSetRangeLength(theOtherSet));
                    break;

                case __kCFCharSetClassString:
                    __CFCSetPutStringLength(theSet, __CFCSetStringLength(theOtherSet));
                    if (!__CFCSetStringBuffer(theSet))
						__CFCSetPutStringBuffer(theSet, (UniChar *)CFAllocatorAllocate(CFGetAllocator(theSet), __kCFStringCharSetMax * sizeof(UniChar), 0));
                   memmove(__CFCSetStringBuffer(theSet), __CFCSetStringBuffer(theOtherSet), __CFCSetStringLength(theSet) * sizeof(UniChar));
                    break;

                case __kCFCharSetClassBitmap:
					__CFCSetPutBitmapBits(theSet, (uint8_t *)CFAllocatorAllocate(CFGetAllocator(theSet), sizeof(uint8_t) * __kCFBitmapSize, 0));
                    memmove(__CFCSetBitmapBits(theSet), __CFCSetBitmapBits(theOtherSet), __kCFBitmapSize);
                    break;

                case __kCFCharSetClassCompactBitmap: {
                    const uint8_t *cBitmap = __CFCSetCompactBitmapBits(theOtherSet);
                    uint8_t *newBitmap;
                    uint32_t size = __CFCSetGetCompactBitmapSize(cBitmap);
                    newBitmap = (uint8_t *)CFAllocatorAllocate(CFGetAllocator(theSet), sizeof(uint8_t) * size, 0);
                    __CFCSetPutBitmapBits(theSet, newBitmap);
                    memmove(newBitmap, cBitmap, size);
                    }
                    break;

                default:
                    CFAssert1(0, __kCFLogAssertion, "%s: Internal inconsistency error: unknown character set type", __PRETTY_FUNCTION__); // We should never come here
            }
        } else {
            __CFCSetMakeBitmap(theSet);
            if (__CFCSetIsBitmap(theOtherSet)) {
                UInt32 *bitmap1 = (UInt32*)__CFCSetBitmapBits(theSet);
                UInt32 *bitmap2 = (UInt32*)__CFCSetBitmapBits(theOtherSet);
                CFIndex length = __kCFBitmapSize / sizeof(UInt32);
                while (length--) *bitmap1++ &= *bitmap2++;
            } else {
                UInt32 *bitmap1 = (UInt32*)__CFCSetBitmapBits(theSet);
                UInt32 *bitmap2;
                CFIndex length = __kCFBitmapSize / sizeof(UInt32);
                uint8_t bitmapBuffer[__kCFBitmapSize];
                __CFCSetGetBitmap(theOtherSet, bitmapBuffer);
                bitmap2 = (UInt32*)bitmapBuffer;
                while (length--) *bitmap1++ &= *bitmap2++;
            }
            __CFCSetPutHasHashValue(theSet, false);
            if (__CFCSetHasNonBMPPlane(theOtherSet)) {
                CFMutableCharacterSetRef annexPlane;
                CFMutableCharacterSetRef otherSetPlane;
                CFMutableCharacterSetRef emptySet = CFCharacterSetCreateMutable(NULL);
                int idx;
                for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
                    if ((otherSetPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSetNoAlloc(theOtherSet, idx))) {
                        if (__CFCSetAnnexIsInverted(theOtherSet)) CFCharacterSetInvert(otherSetPlane);
                        annexPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSet(theSet, idx);
                        if (__CFCSetAnnexIsInverted(theSet)) CFCharacterSetInvert(annexPlane);
                        CFCharacterSetIntersect(annexPlane, otherSetPlane);
                        if (__CFCSetAnnexIsInverted(theSet)) CFCharacterSetInvert(annexPlane);
                        if (__CFCSetAnnexIsInverted(theOtherSet)) CFCharacterSetInvert(otherSetPlane);
                        if (__CFCSetIsEmpty(annexPlane) && !__CFCSetIsInverted(annexPlane)) __CFCSetPutCharacterSetToAnnexPlane(theSet, NULL, idx);
                    } else if ((annexPlane = (CFMutableCharacterSetRef) __CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, idx))) {
                        if (__CFCSetAnnexIsInverted(theSet)) { // if the annexPlane is inverted, we need to set the plane to empty
                            CFCharacterSetInvert(annexPlane);
                            CFCharacterSetIntersect(annexPlane, emptySet);
                            CFCharacterSetInvert(annexPlane);
                        } else {  // the annexPlane is not inverted, we can clear the plane
                            __CFCSetPutCharacterSetToAnnexPlane(theSet, NULL, idx);
                        }
                    } else if ((__CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, idx) == NULL) && __CFCSetAnnexIsInverted(theSet)) {
                        // the set has no such annex plane and the annex plane is inverted, it means the set contains everything in the annex plane
                        annexPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSet(theSet, idx);
                        if (__CFCSetAnnexIsInverted(theSet)) CFCharacterSetInvert(annexPlane);
                        CFCharacterSetIntersect(annexPlane, emptySet);
                        if (__CFCSetAnnexIsInverted(theSet)) CFCharacterSetInvert(annexPlane);
                    }
                }
                CFRelease(emptySet);
                if (!__CFCSetHasNonBMPPlane(theSet)) __CFCSetDeallocateAnnexPlane(theSet);
            } else if (__CFCSetIsBuiltin(theOtherSet) && !__CFCSetAnnexIsInverted(theOtherSet)) {
                CFMutableCharacterSetRef annexPlane;
                uint8_t bitmapBuffer[__kCFBitmapSize];
                uint8_t result;
                int planeIndex;
                UInt32 *bitmap1;
                UInt32 *bitmap2;
                CFIndex length;

                for (planeIndex = 1;planeIndex <= MAX_ANNEX_PLANE;planeIndex++) {
                    annexPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, planeIndex);
                    if (annexPlane) {
                        result = CFUniCharGetBitmapForPlane(__CFCSetBuiltinType(theOtherSet), planeIndex, bitmapBuffer, false);
                        if (result == kCFUniCharBitmapEmpty) {
                            __CFCSetPutCharacterSetToAnnexPlane(theSet, NULL, planeIndex);
                        } else if (result == kCFUniCharBitmapFilled) {
                            Boolean isEmpty = true;

                            __CFCSetMakeBitmap(annexPlane);
                            bitmap1 = (UInt32 *)__CFCSetBitmapBits(annexPlane);
                            length = __kCFBitmapSize / sizeof(UInt32);
                            bitmap2 = (UInt32*)bitmapBuffer;

                            while (length--) {
                                if ((*bitmap1++ &= *bitmap2++)) isEmpty = false;
                            }
                            if (isEmpty) __CFCSetPutCharacterSetToAnnexPlane(theSet, NULL, planeIndex);
                        }
                    }
                }
                if (!__CFCSetHasNonBMPPlane(theSet)) __CFCSetDeallocateAnnexPlane(theSet);
            } else if (__CFCSetIsRange(theOtherSet)) {
                CFMutableCharacterSetRef tempOtherSet = CFCharacterSetCreateMutable(CFGetAllocator(theSet));
                CFMutableCharacterSetRef annexPlane;
                CFMutableCharacterSetRef otherSetPlane;
                CFMutableCharacterSetRef emptySet = CFCharacterSetCreateMutable(NULL);
                int idx;

                __CFCSetAddNonBMPPlanesInRange(tempOtherSet, CFRangeMake(__CFCSetRangeFirstChar(theOtherSet), __CFCSetRangeLength(theOtherSet)));
                
                for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
                    if ((otherSetPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSetNoAlloc(tempOtherSet, idx))) {
                        annexPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSet(theSet, idx);
                        if (__CFCSetAnnexIsInverted(tempOtherSet)) CFCharacterSetInvert(otherSetPlane);
                        if (__CFCSetAnnexIsInverted(theSet)) CFCharacterSetInvert(annexPlane);
                        CFCharacterSetIntersect(annexPlane, otherSetPlane);
                        if (__CFCSetAnnexIsInverted(theSet)) CFCharacterSetInvert(annexPlane);
                        if (__CFCSetAnnexIsInverted(tempOtherSet)) CFCharacterSetInvert(otherSetPlane);
                        if (__CFCSetIsEmpty(annexPlane) && !__CFCSetIsInverted(annexPlane)) __CFCSetPutCharacterSetToAnnexPlane(theSet, NULL, idx);
                    } else if ((annexPlane = (CFMutableCharacterSetRef) __CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, idx))) {
                        if (__CFCSetAnnexIsInverted(theSet)) {
                            CFCharacterSetInvert(annexPlane);
                            CFCharacterSetIntersect(annexPlane, emptySet);
                            CFCharacterSetInvert(annexPlane);
                        } else {
                            __CFCSetPutCharacterSetToAnnexPlane(theSet, NULL, idx);
                        }
                    } else if ((__CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, idx) == NULL) && __CFCSetAnnexIsInverted(theSet)) {
                        // the set has no such annex plane and the annex plane is inverted, it means the set contains everything in the annex plane
                        annexPlane = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSet(theSet, idx);
                        if (__CFCSetAnnexIsInverted(theSet)) CFCharacterSetInvert(annexPlane);
                        CFCharacterSetIntersect(annexPlane, emptySet);
                        if (__CFCSetAnnexIsInverted(theSet)) CFCharacterSetInvert(annexPlane);
                    }
                }
                if (!__CFCSetHasNonBMPPlane(theSet)) __CFCSetDeallocateAnnexPlane(theSet);
                CFRelease(tempOtherSet);
                CFRelease(emptySet);
            } else if ((__CFCSetHasNonBMPPlane(theSet) || __CFCSetAnnexIsInverted(theSet)) && !__CFCSetAnnexIsInverted(theOtherSet)) {
                __CFCSetDeallocateAnnexPlane(theSet);
            }
        }
        if (__CFCheckForExapendedSet) __CFCheckForExpandedSet(theSet);
    } else { // It's NSCharacterSet
        CFDataRef bitmapRep = CFCharacterSetCreateBitmapRepresentation(kCFAllocatorSystemDefault, theOtherSet);
        const UInt32 *bitmap2 = (bitmapRep && CFDataGetLength(bitmapRep) ? (const UInt32 *)CFDataGetBytePtr(bitmapRep) : NULL);
        if (bitmap2) {
            UInt32 *bitmap1;
            CFIndex length = __kCFBitmapSize / sizeof(UInt32);
            __CFCSetMakeBitmap(theSet);
            bitmap1 = (UInt32*)__CFCSetBitmapBits(theSet);
            while (length--) *bitmap1++ &= *bitmap2++;
            __CFCSetPutHasHashValue(theSet, false);
        }
        CFRelease(bitmapRep);
    }
}

void CFCharacterSetInvert(CFMutableCharacterSetRef theSet) {

    CF_OBJC_FUNCDISPATCHV(__kCFCharacterSetTypeID, void, (NSMutableCharacterSet *)theSet, invert);

    __CFCSetValidateTypeAndMutability(theSet, __PRETTY_FUNCTION__);
    
    if (__CFCSetIsBuiltin((CFCharacterSetRef)theSet) && !__CFCSetIsMutable((CFCharacterSetRef)theSet) && !__CFCSetIsInverted((CFCharacterSetRef)theSet)) {
        CFCharacterSetRef sharedSet = CFCharacterSetGetPredefined(__CFCSetBuiltinType((CFCharacterSetRef)theSet));
        if (sharedSet == theSet) { // We're trying to dealloc the builtin set
            CFAssert1(0, __kCFLogAssertion, "%s: Trying to mutable predefined set.", __PRETTY_FUNCTION__);
            return; // We don't mutate builtin set
        }
    }

    __CFCSetPutHasHashValue(theSet, false);

    if (__CFCSetClassType(theSet) == __kCFCharSetClassBitmap) {
        CFIndex idx;
        CFIndex count = __kCFBitmapSize / sizeof(UInt32);
        UInt32 *bitmap = (UInt32*) __CFCSetBitmapBits(theSet);

        if (NULL == bitmap) {
            bitmap =  (UInt32 *)CFAllocatorAllocate(CFGetAllocator(theSet), __kCFBitmapSize, 0);
            __CFCSetPutBitmapBits(theSet, (uint8_t *)bitmap);
            for (idx = 0;idx < count;idx++) bitmap[idx] = ((UInt32)0xFFFFFFFF);
        } else {
            for (idx = 0;idx < count;idx++) bitmap[idx] = ~(bitmap[idx]);
        }
        __CFCSetAllocateAnnexForPlane(theSet, 0); // We need to alloc annex to invert
    } else if (__CFCSetClassType(theSet) == __kCFCharSetClassCompactBitmap) {
        uint8_t *bitmap = __CFCSetCompactBitmapBits(theSet);
        int idx;
        int length = 0;
        uint8_t value;

        for (idx = 0;idx < __kCFCompactBitmapNumPages;idx++) {
            value = bitmap[idx];

            if (value == 0) {
                bitmap[idx] = UINT8_MAX;
            } else if (value == UINT8_MAX) {
                bitmap[idx] = 0;
            } else {
                length += __kCFCompactBitmapPageSize;
            }
        }
        bitmap += __kCFCompactBitmapNumPages;
        for (idx = 0;idx < length;idx++) bitmap[idx] = ~(bitmap[idx]);
        __CFCSetAllocateAnnexForPlane(theSet, 0); // We need to alloc annex to invert
    } else {
        __CFCSetPutIsInverted(theSet, !__CFCSetIsInverted(theSet));
    }
    __CFCSetAnnexSetIsInverted(theSet, !__CFCSetAnnexIsInverted(theSet));
}

void CFCharacterSetCompact(CFMutableCharacterSetRef theSet) {
    if (__CFCSetIsBitmap(theSet) && __CFCSetBitmapBits(theSet)) __CFCSetMakeCompact(theSet);
    if (__CFCSetHasNonBMPPlane(theSet)) {
        CFMutableCharacterSetRef annex;
        int idx;

        for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
            if ((annex = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, idx)) && __CFCSetIsBitmap(annex) && __CFCSetBitmapBits(annex)) {
                __CFCSetMakeCompact(annex);
            }
        }
    }
}

void CFCharacterSetFast(CFMutableCharacterSetRef theSet) {
    if (__CFCSetIsCompactBitmap(theSet) && __CFCSetCompactBitmapBits(theSet)) __CFCSetMakeBitmap(theSet);
    if (__CFCSetHasNonBMPPlane(theSet)) {
        CFMutableCharacterSetRef annex;
        int idx;

        for (idx = 1;idx <= MAX_ANNEX_PLANE;idx++) {
            if ((annex = (CFMutableCharacterSetRef)__CFCSetGetAnnexPlaneCharacterSetNoAlloc(theSet, idx)) && __CFCSetIsCompactBitmap(annex) && __CFCSetCompactBitmapBits(annex)) {
                __CFCSetMakeBitmap(annex);
            }
        }
    }
}

/* Keyed-coding support
*/
CFCharacterSetKeyedCodingType _CFCharacterSetGetKeyedCodingType(CFCharacterSetRef cset) {
    if (CF_IS_OBJC(__kCFCharacterSetTypeID, cset)) return kCFCharacterSetKeyedCodingTypeBitmap;

    switch (__CFCSetClassType(cset)) {
        case __kCFCharSetClassBuiltin: return ((__CFCSetBuiltinType(cset) < kCFCharacterSetSymbol) ? kCFCharacterSetKeyedCodingTypeBuiltin : kCFCharacterSetKeyedCodingTypeBuiltinAndBitmap);
        case __kCFCharSetClassRange: return kCFCharacterSetKeyedCodingTypeRange;

        case __kCFCharSetClassString: // We have to check if we have non-BMP here
            if (!__CFCSetHasNonBMPPlane(cset) && !__CFCSetAnnexIsInverted(cset)) return kCFCharacterSetKeyedCodingTypeString; // BMP only. we can archive the string
        /* fallthrough */

        default:
            return kCFCharacterSetKeyedCodingTypeBitmap;
    }
}

CFCharacterSetPredefinedSet _CFCharacterSetGetKeyedCodingBuiltinType(CFCharacterSetRef cset) { return __CFCSetBuiltinType(cset); }
CFRange _CFCharacterSetGetKeyedCodingRange(CFCharacterSetRef cset) { return CFRangeMake(__CFCSetRangeFirstChar(cset), __CFCSetRangeLength(cset)); }
CFStringRef _CFCharacterSetCreateKeyedCodingString(CFCharacterSetRef cset) { return CFStringCreateWithCharacters(kCFAllocatorSystemDefault, __CFCSetStringBuffer(cset), __CFCSetStringLength(cset)); }

bool _CFCharacterSetIsInverted(CFCharacterSetRef cset) { return (__CFCSetIsInverted(cset) != 0); }
void _CFCharacterSetSetIsInverted(CFCharacterSetRef cset, bool flag) { __CFCSetPutIsInverted((CFMutableCharacterSetRef)cset, flag); }

/* Inline buffer support
*/
void CFCharacterSetInitInlineBuffer(CFCharacterSetRef cset, CFCharacterSetInlineBuffer *buffer) {
    memset(buffer, 0, sizeof(CFCharacterSetInlineBuffer));
    buffer->cset = cset;
    buffer->rangeLimit = 0x10000;

    if (CF_IS_OBJC(__kCFCharacterSetTypeID, cset)) {
        CFCharacterSetRef expandedSet = __CFCharacterSetGetExpandedSetForNSCharacterSet(cset);

        if (NULL == expandedSet) {
            buffer->flags = kCFCharacterSetNoBitmapAvailable;
            buffer->rangeLimit = 0x110000;

            return;
        } else {
            cset = expandedSet;
        }
    }

    switch (__CFCSetClassType(cset)) {
        case __kCFCharSetClassBuiltin:
            buffer->bitmap = CFUniCharGetBitmapPtrForPlane(__CFCSetBuiltinType(cset), 0);
            buffer->rangeLimit = 0x110000;
            if (NULL == buffer->bitmap) {
                buffer->flags = kCFCharacterSetNoBitmapAvailable;
            } else {
                if (__CFCSetIsInverted(cset)) buffer->flags = kCFCharacterSetIsInverted;
            }
            break;
            
        case __kCFCharSetClassRange:
            buffer->rangeStart = __CFCSetRangeFirstChar(cset);
            buffer->rangeLimit = __CFCSetRangeFirstChar(cset) + __CFCSetRangeLength(cset);
            if (__CFCSetIsInverted(cset)) buffer->flags = kCFCharacterSetIsInverted;
            return;
            
        case __kCFCharSetClassString:
            buffer->flags = kCFCharacterSetNoBitmapAvailable;
            if (__CFCSetStringLength(cset) > 0) {
                buffer->rangeStart = *__CFCSetStringBuffer(cset);
                buffer->rangeLimit = *(__CFCSetStringBuffer(cset) + __CFCSetStringLength(cset) - 1) + 1;

                if (__CFCSetIsInverted(cset)) {
                    if (0 == buffer->rangeStart) {
                        buffer->rangeStart = buffer->rangeLimit;
                        buffer->rangeLimit = 0x10000;
                    } else if (0x10000 == buffer->rangeLimit) {
                        buffer->rangeLimit = buffer->rangeStart;
                        buffer->rangeStart = 0;
                    } else {
                        buffer->rangeStart = 0;
                        buffer->rangeLimit = 0x10000;
                    }
                }
            }
            break;
            
        case __kCFCharSetClassBitmap:
        case __kCFCharSetClassCompactBitmap:
            buffer->bitmap = __CFCSetCompactBitmapBits(cset);
            if (NULL == buffer->bitmap) {
                buffer->flags = kCFCharacterSetIsCompactBitmap;
                if (__CFCSetIsInverted(cset)) buffer->flags |= kCFCharacterSetIsInverted;
            } else {
                if (__kCFCharSetClassCompactBitmap == __CFCSetClassType(cset)) buffer->flags = kCFCharacterSetIsCompactBitmap;
            }
            break;
            
        default:
            CFAssert1(0, __kCFLogAssertion, "%s: Internal inconsistency error: unknown character set type", __PRETTY_FUNCTION__); // We should never come here
            return;
    }

    if (__CFCSetAnnexIsInverted(cset)) {
        buffer->rangeLimit = 0x110000;
    } else if (__CFCSetHasNonBMPPlane(cset)) {
        CFIndex index;

        for (index = MAX_ANNEX_PLANE;index > 0;index--) {
            if (NULL != __CFCSetGetAnnexPlaneCharacterSetNoAlloc(cset, index)) {
                buffer->rangeLimit = (index + 1) << 16;
                break;
            }
        }
    }
}
