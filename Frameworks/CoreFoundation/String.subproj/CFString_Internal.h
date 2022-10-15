/*	CFString_Internal.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFStringEncodingConverterExt.h>
#include "CFInternal.h"

CF_ASSUME_NONNULL_BEGIN

CF_PRIVATE void __CFSetCharToUniCharFunc(CFStringEncodingCheapEightBitToUnicodeProc _Nullable func);
CF_PRIVATE UniChar const * __CFCharToUniCharTable;
CF_PRIVATE CFIndex CFUniCharCompatibilityDecompose(UTF32Char *convertedChars, CFIndex length, CFIndex maxBufferLength);
__attribute__((cold))
CF_PRIVATE void __CFStringHandleOutOfMemory(CFTypeRef _Nullable obj) CLANG_ANALYZER_NORETURN;


CF_ASSUME_NONNULL_END

