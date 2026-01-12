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
 *  CFUnicodePrecomposition.h
 *  CoreFoundation
 *
 *  Created by aki on Wed Oct 03 2001.
 *  Copyright (c) 2001-2014, Apple Inc. All rights reserved.
 *
 */

#if !defined(__COREFOUNDATION_CFUNICODEPRECOMPOSITION__)
#define __COREFOUNDATION_CFUNICODEPRECOMPOSITION__ 1

#include <CoreFoundation/CFUniChar.h>

CF_EXTERN_C_BEGIN

// As you can see, this function cannot precompose Hangul Jamo
CF_EXPORT UTF32Char CFUniCharPrecomposeCharacter(UTF32Char base, UTF32Char combining);

CF_EXPORT bool CFUniCharPrecompose(const UTF16Char *characters, CFIndex length, CFIndex *consumedLength, UTF16Char *precomposed, CFIndex maxLength, CFIndex *filledLength);

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFUNICODEPRECOMPOSITION__ */

