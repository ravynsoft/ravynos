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

#include "crc.h"

const crcDescriptor crc64_ecma_182 = {
    .name = "crc64-ecma-182",
    .defType = model,
    .def.parms.width = 8,
    .def.parms.poly = 0x42F0E1EBA9EA3693ULL,
    .def.parms.initial_value = 0xffffffffffffffffULL,
    .def.parms.final_xor = 0xffffffffffffffffULL,
    .def.parms.weak_check = 0x62EC59E3F1A4F00AULL,
    .def.parms.reflect_reverse = NO_REFLECT_REVERSE,
};
