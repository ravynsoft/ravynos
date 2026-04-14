
//  Created by rafirafi on 3/17/16.
//  Copyright (c) 2016 rafirafi. All rights reserved.

// needed by xnu/osfmk/vm/vm_compressor_backing_store.c

#if KERNEL
#include <sys/systm.h>
#endif

#include <stddef.h>
#include <corecrypto/ccmode.h>
#include <corecrypto/ccaes.h>
#include <corecrypto/ccn.h>
#include "cc_abort.h"

#include "aes128.h"

static int pdcmode_aes_cbc_init(const struct ccmode_cbc *cbc, cccbc_ctx *ctx, size_t key_len, const void *key)
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

/*
 * corecrypto api uses aes_ecb to perform aes_cbc block decryption
 * but it was simpler to reuse the tinyaes128 aes_cbc and set custom to NULL
 */

static int pdcmode_aes_cbc_encrypt(const cccbc_ctx *ctx, cccbc_iv *iv, unsigned long nblocks, const void *in, void *out)
{
    printf("%s\n", __func__);

    AES128_CBC_encrypt((struct _pdcmode_aes128_ctx *)ctx, (struct pdccbc_iv *)iv, nblocks, in, out);
	return 0;
}

static int pdcmode_aes_cbc_decrypt(const cccbc_ctx *ctx, cccbc_iv *iv, unsigned long nblocks, const void *in, void *out)
{
    printf("%s\n", __func__);

    AES128_CBC_decrypt((struct _pdcmode_aes128_ctx *)ctx, (struct pdccbc_iv *)iv, nblocks, in, out);
	return 0;
}

const struct ccmode_cbc pdcaes_cbc_encrypt = {
    .size = ccn_sizeof_size(sizeof(struct _pdcmode_aes128_ctx)),
    .block_size = CCAES_BLOCK_SIZE,
    .init = pdcmode_aes_cbc_init,
    .cbc = pdcmode_aes_cbc_encrypt,
    .custom = NULL
};

const struct ccmode_cbc pdcaes_cbc_decrypt = {
    .size = ccn_sizeof_size(sizeof(struct _pdcmode_aes128_ctx)),
    .block_size = CCAES_BLOCK_SIZE,
    .init = pdcmode_aes_cbc_init,
    .cbc = pdcmode_aes_cbc_decrypt,
    .custom = NULL
};
