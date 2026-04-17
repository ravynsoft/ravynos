//
//  pdcrypto_sha1.c
//  pdcrypto
//
//  Created by rafirafi on 3/17/16.
//  Copyright (c) 2016 rafirafi. All rights reserved.
//

#include <stddef.h>
#include <corecrypto/ccsha1.h>
#include <corecrypto/ccdigest_priv.h>

#include "pdcrypto_digest_final.h"

void pdcsha1_compress(ccdigest_state_t state, unsigned long nblocks, const void *data);

const uint32_t pdcsha1_initial_state[5] = {
    0x67452301UL, // A
    0xefcdab89UL, // B
    0x98badcfeUL, // C
    0x10325476UL, // D
    0xc3d2e1f0UL  // E
};

const struct ccdigest_info ccsha1_ltc_di = {
    .output_size = CCSHA1_OUTPUT_SIZE,
    .state_size = CCSHA1_STATE_SIZE,
    .block_size = CCSHA1_BLOCK_SIZE,
    .oid_size = ccoid_sha1_len,
    .oid = (unsigned char *)CC_DIGEST_OID_SHA1,
    .initial_state = pdcsha1_initial_state,
    .compress = pdcsha1_compress,
    .final = pdcdigest_final_64be
};

const struct ccdigest_info ccsha1_eay_di = {
	.output_size = CCSHA1_OUTPUT_SIZE,
	.state_size = CCSHA1_STATE_SIZE,
	.block_size = CCSHA1_BLOCK_SIZE,
	.oid_size = ccoid_sha1_len,
	.oid = (unsigned char *)CC_DIGEST_OID_SHA1,
	.initial_state = pdcsha1_initial_state,
	.compress = pdcsha1_compress,
	.final = pdcdigest_final_64be
};

const struct ccdigest_info *ccsha1_di(void) {
	return &ccsha1_ltc_di;
}
