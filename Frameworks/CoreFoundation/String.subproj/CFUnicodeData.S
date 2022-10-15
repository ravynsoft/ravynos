// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2016 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//

#include <CoreFoundation/CFAsmMacros.h>

#if defined(__ELF__)
.section .rodata
#endif

#if defined(__BIG_ENDIAN__)
    .global _C_LABEL(__CFUnicodeDataB)
_C_LABEL(__CFUnicodeDataB):
    .incbin CF_CHARACTERSET_UNICODE_DATA_B

    .global _C_LABEL(__CFUnicodeDataBEnd)
_C_LABEL(__CFUnicodeDataBEnd):
    .byte 0

    .global _C_LABEL(__CFUnicodeDataBSize)
_C_LABEL(__CFUnicodeDataBSize):
    .int _C_LABEL(__CFUnicodeDataBEnd) - _C_LABEL(__CFUnicodeDataB)
#else
    .global _C_LABEL(__CFUnicodeDataL)
_C_LABEL(__CFUnicodeDataL):
    .incbin CF_CHARACTERSET_UNICODE_DATA_L

    .global _C_LABEL(__CFUnicodeDataLEnd)
_C_LABEL(__CFUnicodeDataLEnd):
    .byte 0

    .global _C_LABEL(__CFUnicodeDataLSize)
_C_LABEL(__CFUnicodeDataLSize):
    .int _C_LABEL(__CFUnicodeDataLEnd) - _C_LABEL(__CFUnicodeDataL)
#endif

NO_EXEC_STACK_DIRECTIVE
SAFESEH_REGISTRATION_DIRECTIVE
