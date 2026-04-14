//
//  pdcrypto_digest_update.c
//  pdcrypto
//
//  Created by rafirafi on 3/17/16.
//  Copyright (c) 2016 rafirafi. All rights reserved.
//
//  directly from xnu only the function anme was changed
//
//  xnu https://opensource.apple.com/source/xnu/xnu-2782.40.9
//  License https://opensource.apple.com/source/xnu/xnu-2782.40.9/APPLE_LICENSE

#include <stddef.h>
#include "pdcrypto_digest_update.h"

#include <corecrypto/ccdigest.h>
#include <corecrypto/cc_priv.h>

void ccdigest_update(const struct ccdigest_info *di, ccdigest_ctx_t ctx,
                      unsigned long len, const void *data) {
    char * data_ptr = (char *) data;
    while (len > 0) {
        if (ccdigest_num(di, ctx) == 0 && len > di->block_size) {
            unsigned long nblocks = len / di->block_size;
            di->compress(ccdigest_state(di, ctx), nblocks, data_ptr);
            unsigned long nbytes = nblocks * di->block_size;
            len -= nbytes;
            data_ptr += nbytes;
            ccdigest_nbits(di, ctx) += nbytes * 8;
        } else {
            unsigned long n = di->block_size - ccdigest_num(di, ctx);
            if (len < n)
                n = len;
            CC_MEMCPY(ccdigest_data(di, ctx) + ccdigest_num(di, ctx), data_ptr, n);
            /* typecast: less than block size, will always fit into an int */
            ccdigest_num(di, ctx) += (unsigned int)n;
            len -= n;
            data_ptr += n;
            if (ccdigest_num(di, ctx) == di->block_size) {
                di->compress(ccdigest_state(di, ctx), 1, ccdigest_data(di, ctx));
                ccdigest_nbits(di, ctx) += ccdigest_num(di, ctx) * 8;
                ccdigest_num(di, ctx) = 0;
            }
        }
    }
}
