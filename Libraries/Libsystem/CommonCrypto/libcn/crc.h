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

#ifndef CommonNumerics_crc_h
#define CommonNumerics_crc_h


#if defined (_WIN32)
#define __unused
#endif

#include <stdint.h>
#include <stddef.h>
#include "../lib/ccDispatch.h"

#define MASK08 0x00000000000000ffLL
#define MASK16 0x000000000000ffffLL
#define MASK32 0x00000000ffffffffLL
#define MASK64 0xffffffffffffffffLL



#define WEAK_CHECK_INPUT "123456789"

// Utility Functions

uint8_t reflect_byte(uint8_t b);
uint64_t reflect(uint64_t w, size_t bits);
uint64_t reverse_poly(uint64_t poly, size_t width);

typedef uint64_t (*cccrc_setup_p)(void);
typedef uint64_t (*cccrc_update_p)(size_t len, const void *in, uint64_t crc);
typedef uint64_t (*cccrc_final_p)(size_t length, uint64_t crc);
typedef uint64_t (*cccrc_oneshot_p)(size_t len, const void *in);

#define NO_REFLECT_REVERSE 0
#define REFLECT_IN 1
#define REVERSE_OUT 2
#define REFLECT_REVERSE 3

typedef struct crcModelParms_t {
    int width; // width in bytes
    int reflect_reverse;
    uint64_t mask;
    uint64_t poly;
    uint64_t initial_value;
    uint64_t final_xor;
    uint64_t weak_check;
} crcModelParms;

typedef struct crcFuncs_t {
    cccrc_setup_p setup;
    cccrc_update_p update;
    cccrc_final_p final;
    cccrc_oneshot_p oneshot;
} crcFuncs;

enum crcType_t {
    model = 0,
    functions = 1,
};
typedef uint32_t crcType;

typedef struct crcDescriptor_t {
    const char *name;
    const crcType defType;
    union ddef {
        crcModelParms parms;
        crcFuncs funcs;
    } def;
} crcDescriptor;

typedef const crcDescriptor *crcDescriptorPtr;

typedef struct crcInfo_t {
    dispatch_once_t table_init;
    crcDescriptorPtr descriptor;
    size_t size;
    union {
        uint8_t *bytes;
        uint16_t *b16;
        uint32_t *b32;
        uint64_t *b64;
    } table;
} crcInfo, *crcInfoPtr;


void gen_std_crc_table(void *c);
void dump_crc_table(crcInfoPtr crc);
uint64_t crc_normal_init(crcInfoPtr crc);
uint64_t crc_normal_update(crcInfoPtr crc, uint8_t *p, size_t len, uint64_t current);
uint64_t crc_normal_final(crcInfoPtr crc, uint64_t current);
uint64_t crc_normal_oneshot(crcInfoPtr crc, uint8_t *p, size_t len);

uint64_t crc_reverse_init(crcInfoPtr crc);
uint64_t crc_reverse_update(crcInfoPtr crc, uint8_t *p, size_t len, uint64_t current);
uint64_t crc_reverse_final(crcInfoPtr crc, uint64_t current);
uint64_t crc_reverse_oneshot(crcInfoPtr crc, uint8_t *p, size_t len);

static inline uint64_t descmaskfunc(crcDescriptorPtr descriptor) {
    switch(descriptor->def.parms.width) {
        case 1: return MASK08;
        case 2: return MASK16;
        case 4: return MASK32;
        case 8: return MASK64;
    }
    return 0;
}

extern const crcDescriptor crc8;
extern const crcDescriptor crc8_icode;
extern const crcDescriptor crc8_itu;
extern const crcDescriptor crc8_rohc;
extern const crcDescriptor crc8_wcdma;
extern const crcDescriptor crc16;
extern const crcDescriptor crc16_ccitt_true;
extern const crcDescriptor crc16_ccitt_false;
extern const crcDescriptor crc16_usb;
extern const crcDescriptor crc16_xmodem;
extern const crcDescriptor crc16_dect_r;
extern const crcDescriptor crc16_dect_x;
extern const crcDescriptor crc16_icode;
extern const crcDescriptor crc16_verifone;
extern const crcDescriptor crc16_a;
extern const crcDescriptor crc16_b;
extern const crcDescriptor crc32;
extern const crcDescriptor crc32_castagnoli;
extern const crcDescriptor crc32_bzip2;
extern const crcDescriptor crc32_mpeg_2;
extern const crcDescriptor crc32_posix;
extern const crcDescriptor crc32_xfer;
extern const crcDescriptor adler32;
extern const crcDescriptor crc64_ecma_182;

#endif
