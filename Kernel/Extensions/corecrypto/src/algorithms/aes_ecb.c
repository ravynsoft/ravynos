//  Created by rafirafi on 3/17/16.
//  Copyright (c) 2016 rafirafi. All rights reserved.

#if KERNEL
#include <sys/systm.h>
#endif

#include <stddef.h>
#include <corecrypto/ccmode.h>
#include <corecrypto/ccaes.h>
#include <corecrypto/ccn.h>
#include "cc_abort.h"

#include "aes128.h"

static int pdcmode_aes_ecb_init(const struct ccmode_ecb *ecb, ccecb_ctx *ctx, size_t key_len, const void *key)
{
    printf("%s\n", __func__);

    // normalize key lenght
    //  " Key lengths in the range 16 <= key_len <= 32 are given in bytes,
    //   those in the range 128 <= key_len <= 256 are given in bits " xnu/libkern/libkern/crypto/aes.h
    if (key_len > 32) {
        assert(key_len % 8 == 0);
        key_len /= 8;
    }

    // only 128 case implemented here
    if (key_len != CCAES_KEY_SIZE_128) {
        cc_abort("%s key len != 128\n", __func__);
    }

    AES128_set_key((struct _pdcmode_aes128_ctx *)ctx, key);
	return 0;
}

static int pdcmode_aes_ecb_encrypt(const ccecb_ctx *ctx, unsigned long nblocks, const void *in, void *out)
{
    printf("%s\n", __func__);

    AES128_ECB_encrypt((struct _pdcmode_aes128_ctx *)ctx, nblocks, in, out);
	return 0;
}

static int pdcmode_aes_ecb_decrypt(const ccecb_ctx *ctx, unsigned long nblocks, const void *in, void *out)
{
    printf("%s\n", __func__);

    AES128_ECB_decrypt((struct _pdcmode_aes128_ctx *)ctx, nblocks, in, out);
	return 0;
}

const struct ccmode_ecb pdcaes_ecb_encrypt = {
    .size = ccn_sizeof_size(sizeof(struct _pdcmode_aes128_ctx)),
    .block_size = CCAES_BLOCK_SIZE,
    .init = pdcmode_aes_ecb_init,
    .ecb = pdcmode_aes_ecb_encrypt
};

const struct ccmode_ecb pdcaes_ecb_decrypt = {
    .size = ccn_sizeof_size(sizeof(struct _pdcmode_aes128_ctx)),
    .block_size = CCAES_BLOCK_SIZE,
    .init = pdcmode_aes_ecb_init,
    .ecb = pdcmode_aes_ecb_decrypt
};
