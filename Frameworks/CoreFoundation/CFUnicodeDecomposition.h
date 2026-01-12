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

/*
 *  CFUnicodeDecomposition.h
 *  CoreFoundation
 *
 *  Created by aki on Wed Oct 03 2001.
 *  Copyright (c) 2001-2014, Apple Inc. All rights reserved.
 *
 */

#if !defined(__COREFOUNDATION_CFUNICODEDECOMPOSITION__)
#define __COREFOUNDATION_CFUNICODEDECOMPOSITION__ 1

#include <CoreFoundation/CFUniChar.h>

CF_EXTERN_C_BEGIN

CF_INLINE bool CFUniCharIsDecomposableCharacter(UTF32Char character, bool isHFSPlusCanonical) {
    if (isHFSPlusCanonical && !isHFSPlusCanonical) return false;	// hack to get rid of "unused" warning
    if (character < 0x80) return false;
    return CFUniCharIsMemberOf(character, kCFUniCharHFSPlusDecomposableCharacterSet);
}

CF_EXPORT CFIndex CFUniCharDecomposeCharacter(UTF32Char character, UTF32Char *convertedChars, CFIndex maxBufferLength);
CF_EXPORT CFIndex CFUniCharCompatibilityDecompose(UTF32Char *convertedChars, CFIndex length, CFIndex maxBufferLength);

CF_EXPORT bool CFUniCharDecompose(const UTF16Char *src, CFIndex length, CFIndex *consumedLength, void *dst, CFIndex maxLength, CFIndex *filledLength, bool needToReorder, uint32_t dstFormat, bool isHFSPlus);

CF_EXPORT void CFUniCharPrioritySort(UTF32Char *characters, CFIndex length);

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFUNICODEDECOMPOSITION__ */

