/*
 * Copyright (c) 2012 Apple, Inc. All Rights Reserved.
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

#ifndef CommonNumerics_basexx_h
#define CommonNumerics_basexx_h

#include <stdint.h>
#include <stddef.h>
#include "../lib/ccDispatch.h"
#include <CommonNumerics/CommonNumerics.h>
#include <CommonNumerics/CommonBaseXX.h>

typedef struct encoderConstants_t {
    uint32_t    baseNum;
    uint32_t    log;
    uint32_t    inputBlocksize;
    uint32_t    outputBlocksize;
    uint8_t     basemask;
} encoderConstants;

typedef struct baseEncoder_t {
    const char *name;
    CNEncodings encoding;
    const char *charMap;
    const encoderConstants *values;
    uint8_t baseNum;
    uint8_t padding;
} BaseEncoder;

typedef BaseEncoder *BaseEncoderRefCustom;
typedef const BaseEncoder *BaseEncoderRef;

#define CC_BASE_REVERSE_MAP_SIZE 256
// This manages a global context for encoders.
typedef struct coderFrame_t {
    uint8_t reverseMap[CC_BASE_REVERSE_MAP_SIZE];
    BaseEncoderRef encoderRef;
} BaseEncoderFrame, *CoderFrame;

extern const BaseEncoder defaultBase64;
extern const BaseEncoder defaultBase32; // RFC 4678 Base32Alphabet
extern const BaseEncoder recoveryBase32;
extern const BaseEncoder hexBase32;
extern const BaseEncoder defaultBase16;
void setReverseMap(CoderFrame frame);


#endif
