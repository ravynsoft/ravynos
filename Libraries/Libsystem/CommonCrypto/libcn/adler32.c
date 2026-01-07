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

static const int adler_mod_value = 65521;

static uint64_t
adler32_setup() { return 0; }

static uint64_t
adler32_implementation(size_t len, const void *in, uint64_t __unused crc)
{
    uint32_t a = 1, b = 0;
    const uint8_t *bytes = in;
    
    for (size_t i = 0; i < len; i++) {
        a = (a + bytes[i]) % adler_mod_value;
        b = (b + a) % adler_mod_value;
    }
    return (b << 16) | a;
}

static uint64_t
adler32_final(size_t __unused length, uint64_t crc) { return crc; }


static uint64_t
adler32_oneshot(size_t len, const void *in)
{
    return adler32_implementation(len, in, 0);
}



const crcDescriptor adler32 = {
    .name = "adler-32",
    .defType = functions,
    .def.funcs.setup = adler32_setup,
    .def.funcs.update = adler32_implementation,
    .def.funcs.final = adler32_final,
    .def.funcs.oneshot = adler32_oneshot
};
