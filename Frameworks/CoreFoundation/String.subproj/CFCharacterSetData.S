// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//

#include <CoreFoundation/CFAsmMacros.h>

#if defined(__ELF__)
.section .rodata
#endif

    .global _C_LABEL(__CFCharacterSetBitmapData)
_C_LABEL(__CFCharacterSetBitmapData):
    .incbin CF_CHARACTERSET_BITMAP

    .global _C_LABEL(__CFCharacterSetBitmapDataEnd)
_C_LABEL(__CFCharacterSetBitmapDataEnd):
    .byte 0

    .global _C_LABEL(__CFCharacterSetBitmapDataSize)
_C_LABEL(__CFCharacterSetBitmapDataSize):
    .int _C_LABEL(__CFCharacterSetBitmapDataEnd) - _C_LABEL(__CFCharacterSetBitmapData)

NO_EXEC_STACK_DIRECTIVE
SAFESEH_REGISTRATION_DIRECTIVE
