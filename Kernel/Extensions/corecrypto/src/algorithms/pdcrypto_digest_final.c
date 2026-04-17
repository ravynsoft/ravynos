//
//  pd_crypto_digest_final.c
//  pdcrypto
//
//  Created by rafirafi on 3/17/16.
//  Copyright (c) 2016 rafirafi. All rights reserved.
//
//  le version adapted from xnu/osfmk/corecrypto/ccsha1/src/ccdigest_final_64be.c
//  be version copied from xnu, only function name was changed
//
//  xnu https://opensource.apple.com/source/xnu/xnu-2782.40.9
//  License https://opensource.apple.com/source/xnu/xnu-2782.40.9/APPLE_LICENSE

#include <stddef.h>
#include "pdcrypto_digest_final.h"

#include <corecrypto/ccdigest_priv.h>
#include <corecrypto/cc_priv.h>

void pdcdigest_final_64le(const struct ccdigest_info *di, ccdigest_ctx_t ctx,
                          unsigned char *digest) {
    ccdigest_nbits(di, ctx) += ccdigest_num(di, ctx) * 8;
    ccdigest_data(di, ctx)[ccdigest_num(di, ctx)++] = 0x80;

    /* If we don't have at least 8 bytes (for the length) left we need to add
     a second block. */
    if (ccdigest_num(di, ctx) > 64 - 8) {
        while (ccdigest_num(di, ctx) < 64) {
            ccdigest_data(di, ctx)[ccdigest_num(di, ctx)++] = 0;
        }
        di->compress(ccdigest_state(di, ctx), 1, ccdigest_data(di, ctx));
        ccdigest_num(di, ctx) = 0;
    }

    /* pad upto block_size minus 8 with 0s */
    while (ccdigest_num(di, ctx) < 64 - 8) {
        ccdigest_data(di, ctx)[ccdigest_num(di, ctx)++] = 0;
    }

    CC_STORE64_LE(ccdigest_nbits(di, ctx), ccdigest_data(di, ctx) + 64 - 8);
    di->compress(ccdigest_state(di, ctx), 1, ccdigest_data(di, ctx));

    /* copy output */
    for (unsigned int i = 0; i < di->output_size / 4; i++) {
        CC_STORE32_LE(ccdigest_state_u32(di, ctx)[i], digest+(4*i));
    }
}

void pdcdigest_final_64be(const struct ccdigest_info *di, ccdigest_ctx_t ctx,
                          unsigned char *digest) {
    ccdigest_nbits(di, ctx) += ccdigest_num(di, ctx) * 8;
    ccdigest_data(di, ctx)[ccdigest_num(di, ctx)++] = 0x80;
    
    /* If we don't have at least 8 bytes (for the length) left we need to add
     a second block. */
    if (ccdigest_num(di, ctx) > 64 - 8) {
        while (ccdigest_num(di, ctx) < 64) {
            ccdigest_data(di, ctx)[ccdigest_num(di, ctx)++] = 0;
        }
        di->compress(ccdigest_state(di, ctx), 1, ccdigest_data(di, ctx));
        ccdigest_num(di, ctx) = 0;
    }
    
    /* pad upto block_size minus 8 with 0s */
    while (ccdigest_num(di, ctx) < 64 - 8) {
        ccdigest_data(di, ctx)[ccdigest_num(di, ctx)++] = 0;
    }
    
    CC_STORE64_BE(ccdigest_nbits(di, ctx), ccdigest_data(di, ctx) + 64 - 8);
    di->compress(ccdigest_state(di, ctx), 1, ccdigest_data(di, ctx));
    
    /* copy output */
    for (unsigned int i = 0; i < di->output_size / 4; i++) {
        CC_STORE32_BE(ccdigest_state_u32(di, ctx)[i], digest+(4*i));
    }
}

void pdcdigest_final_fn(const struct ccdigest_info *di, ccdigest_ctx_t ctx, void *digest) {
	// TODO: Is this the correct implementation?

#if BYTE_ORDER == BIG_ENDIAN
	pdcdigest_final_64be(di, ctx, (unsigned char *)digest);
#elif BYTE_ORDER == LITTLE_ENDIAN
	pdcdigest_final_64le(di, ctx, (unsigned char *)digest);
#else
	cc_abort("Unsupported byte order");
#endif
}
