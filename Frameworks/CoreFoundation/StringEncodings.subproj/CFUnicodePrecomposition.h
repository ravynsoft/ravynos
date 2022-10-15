/*
 	CFUnicodePrecomposition.h
 	CoreFoundation

 	Created by aki on Wed Oct 03 2001.
 	Copyright (c) 2001-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#if !defined(__COREFOUNDATION_CFUNICODEPRECOMPOSITION__)
#define __COREFOUNDATION_CFUNICODEPRECOMPOSITION__ 1

#include <CoreFoundation/CFUniChar.h>

CF_EXTERN_C_BEGIN

CF_EXPORT bool CFUniCharPrecompose(const UTF16Char *characters, CFIndex length, CFIndex *consumedLength, UTF16Char *precomposed, CFIndex maxLength, CFIndex *filledLength);

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFUNICODEPRECOMPOSITION__ */

