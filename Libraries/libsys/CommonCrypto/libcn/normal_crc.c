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

uint64_t
crc_normal_init(crcInfoPtr crc)
{
    cc_dispatch_once(&crc->table_init, crc, gen_std_crc_table);
    return crc->descriptor->def.parms.initial_value;
}

static inline uint8_t
crc_table_value8(uint8_t *table, uint8_t p, uint8_t crc) {
    uint8_t t = (uint8_t) (crc << 8);
    return table[((crc) ^ p) & 0xff] ^ t;
}

static inline uint16_t
crc_table_value16(uint16_t *table, uint8_t p, uint16_t crc) {
    uint16_t t = (uint16_t) (crc << 8);
    return table[((crc>>8) ^ p) & 0xff] ^ t;
}

static inline uint32_t
crc_table_value32(uint32_t *table, uint8_t p, uint32_t crc) {
    return table[((crc>>24) ^ p) & 0xff] ^ (crc << 8);
}

static inline uint64_t
crc_table_value64(uint64_t *table, uint8_t p, uint64_t crc) {
    return table[((crc>>56) ^ p) & 0xffULL] ^ (crc << 8);
}


uint64_t
crc_normal_update(crcInfoPtr crc, uint8_t *p, size_t len, uint64_t current)
{
    while (len--) {
        switch (crc->descriptor->def.parms.width) {
            case 1: current = crc_table_value8(crc->table.bytes, *p, (uint8_t) current); break;
            case 2: current = crc_table_value16(crc->table.b16, *p, (uint16_t) current); break;
            case 4: current = crc_table_value32(crc->table.b32, *p, (uint32_t) current); break;
            case 8: current = crc_table_value64(crc->table.b64, *p, current); break;
        }
        p++;
    }
    return current & descmaskfunc(crc->descriptor);
}


uint64_t
crc_normal_final(crcInfoPtr crc, uint64_t current)
{
    current = (current ^ crc->descriptor->def.parms.final_xor) & descmaskfunc(crc->descriptor);
    return current;
}

 uint64_t
 crc_normal_oneshot(crcInfoPtr crc, uint8_t *p, size_t len)
 {
     uint64_t current  = crc_normal_init(crc);
     current = crc_normal_update(crc, p, len, current);
     return crc_normal_final(crc, current);
 }

