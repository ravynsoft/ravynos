//
//  pdcrypto_md5.c
//  pdcrypto
//
//  Created by rafirafi on 3/17/16.
//  Copyright (c) 2016 rafirafi. All rights reserved.
//

#include <stddef.h>
#include <corecrypto/ccmd5.h>

#include "pdcrypto_digest_final.h"

void pdcmd5_compress(ccdigest_state_t state, unsigned long nblocks, const void *data);

const uint32_t pdcmd5_initial_state[4] = {
    0x67452301UL, // A
    0xefcdab89UL, // B
    0x98badcfeUL, // C
    0x10325476UL  // D
};

#define pdcoid_md5_len  10

const struct ccdigest_info pdcmd5_di = {
    .output_size = CCMD5_OUTPUT_SIZE,
    .state_size = CCMD5_STATE_SIZE,
    .block_size = CCMD5_BLOCK_SIZE,
    .oid_size = pdcoid_md5_len,
    .oid = (unsigned char *)CC_DIGEST_OID_MD5,
    .initial_state = pdcmd5_initial_state,
    .compress = pdcmd5_compress,
    .final = pdcdigest_final_64le
};

