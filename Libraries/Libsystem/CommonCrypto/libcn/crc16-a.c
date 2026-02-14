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
const crcDescriptor crc16_a = {
    .name = "crc16-a",
    .defType = model,
    .def.parms.width = 2,
    .def.parms.poly = 0x1021,
    .def.parms.initial_value = 0x6363, 
    // The initial value
    // differs from http://regregex.bbcmicro.net/crc-catalogue.htm
    // but matches page 11 of http://wg8.de/wg8n1496_17n3613_Ballot_FCD14443-3.pdf
    .def.parms.final_xor = 0,
    .def.parms.weak_check = 0xBF05,
    .def.parms.reflect_reverse = REFLECT_REVERSE,
};
