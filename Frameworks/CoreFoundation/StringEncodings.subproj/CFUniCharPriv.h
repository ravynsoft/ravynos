/*	CFUniCharPriv.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFUNICHARPRIV__)
#define __COREFOUNDATION_CFUNICHARPRIV__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFUniChar.h>

#define kCFUniCharRecursiveDecompositionFlag	(1UL << 30)
#define kCFUniCharNonBmpFlag			(1UL << 31)
#define CFUniCharConvertCountToFlag(count)	((count & 0x1F) << 24)
#define CFUniCharConvertFlagToCount(flag)	((flag >> 24) & 0x1F)

enum {
    kCFUniCharCanonicalDecompMapping = (kCFUniCharCaseFold + 1),
    kCFUniCharCanonicalPrecompMapping,
    kCFUniCharCompatibilityDecompMapping
};

CF_PRIVATE const void *CFUniCharGetMappingData(uint32_t type);

CF_PRIVATE uint8_t CFUniCharGetBitmapForPlane(uint32_t charset, uint32_t plane, void *bitmap, bool isInverted);

CF_PRIVATE uint32_t CFUniCharGetNumberOfPlanes(uint32_t charset);

CF_PRIVATE uint32_t CFUniCharGetConditionalCaseMappingFlags(UTF32Char theChar, UTF16Char *buffer, CFIndex currentIndex, CFIndex length, uint32_t type, const uint8_t *langCode, uint32_t lastFlags);

CF_PRIVATE uint32_t CFUniCharGetNumberOfPlanesForUnicodePropertyData(uint32_t propertyType);
CF_PRIVATE uint32_t CFUniCharGetUnicodeProperty(UTF32Char character, uint32_t propertyType);

// As you can see, this function cannot precompose Hangul Jamo
CF_PRIVATE UTF32Char CFUniCharPrecomposeCharacter(UTF32Char base, UTF32Char combining);

#endif /* ! __COREFOUNDATION_CFUNICHARPRIV__ */

