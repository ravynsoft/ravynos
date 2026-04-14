#include "pdcrypto_dummy.h"

#if KERNEL
#include <sys/systm.h>
#else
#include <stdio.h>
#endif

#include "pdcrypto_digest_final.h"

/*
 * to print what is used by xnu during boot
 */

void pdchmac_init_fn_dummy(const struct ccdigest_info *di,
                           cchmac_ctx_t ctx,
                           unsigned long key_len, const void *key)
{
    printf("%s\n", __func__);
}

void pdchmac_update_fn_dummy(const struct ccdigest_info *di,
                             cchmac_ctx_t ctx,
                             unsigned long data_len,
                             const void *data)
{
    printf("%s\n", __func__);
}

void pdchmac_final_fn_dummy(const struct ccdigest_info *di,
                            cchmac_ctx_t ctx,
                            unsigned char *mac)
{
    printf("%s\n", __func__);
}

void pdchmac_fn_dummy(const struct ccdigest_info *di,
                      unsigned long key_len,
                      const void *key,
                      unsigned long data_len,
                      const void *data,
                      unsigned char *mac)
{
    printf("%s\n", __func__);
}

static int pdcmode_ecb_init_dummy(const struct ccmode_ecb *ecb, ccecb_ctx *ctx,
                                   size_t key_len, const void *key)
{
    printf("%s\n", __func__);
	return -1;
}

static int pdcmode_cbc_init_dummy(const struct ccmode_cbc *cbc, cccbc_ctx *ctx,
                                   size_t key_len, const void *key)
{
    printf("%s\n", __func__);
	return -1;
}

const struct ccmode_ecb pdcaes_ecb_encrypt_dummy = {
    .init = pdcmode_ecb_init_dummy
};

const struct ccmode_ecb pdcaes_ecb_decrypt_dummy = {
    .init = pdcmode_ecb_init_dummy
};

static int pdcmode_aes_cbc_init_dummy(const struct ccmode_cbc *cbc, cccbc_ctx *ctx,
                                       size_t key_len, const void *key)
{
    printf("%s\n", __func__);
	return -1;
}

const struct ccmode_cbc pdcaes_cbc_encrypt_dummy = {
    .init = pdcmode_aes_cbc_init_dummy
};

const struct ccmode_cbc pdcaes_cbc_decrypt_dummy = {
    .init = pdcmode_aes_cbc_init_dummy
};

const struct ccmode_xts pdcaes_xts_encrypt_dummy;
const struct ccmode_xts pdcaes_xts_decrypt_dummy;
const struct ccmode_gcm pdcaes_gcm_encrypt_dummy;
const struct ccmode_gcm pdcaes_gcm_decrypt_dummy;

const struct ccmode_ecb pdcdes_ecb_encrypt_dummy = {
    .init = pdcmode_ecb_init_dummy
};

const struct ccmode_ecb pdcdes_ecb_decrypt_dummy = {
    .init = pdcmode_ecb_init_dummy
};

const struct ccmode_cbc pdcdes_cbc_encrypt_dummy = {
    .init = pdcmode_cbc_init_dummy
};

const struct ccmode_cbc pdcdes_cbc_decrypt_dummy = {
    .init = pdcmode_cbc_init_dummy
};

const struct ccmode_ecb pdctdes_ecb_encrypt_dummy = {
    .init = pdcmode_ecb_init_dummy
};

const struct ccmode_ecb pdctdes_ecb_decrypt_dummy = {
    .init = pdcmode_ecb_init_dummy
};

const struct ccmode_cbc pdctdes_cbc_encrypt_dummy = {
    .init = pdcmode_cbc_init_dummy
};

const struct ccmode_cbc pdctdes_cbc_decrypt_dummy = {
    .init = pdcmode_cbc_init_dummy
};

const struct ccrc4_info pdcrc4_info_dummy;

const struct ccmode_ecb pdcblowfish_ecb_encrypt_dummy = {
    .init = pdcmode_ecb_init_dummy
};

const struct ccmode_ecb pdcblowfish_ecb_decrypt_dummy = {
    .init = pdcmode_ecb_init_dummy
};


const struct ccmode_ecb pdccast_ecb_encrypt_dummy = {
    .init = pdcmode_ecb_init_dummy
};

const struct ccmode_ecb pdccast_ecb_decrypt_dummy = {
    .init = pdcmode_ecb_init_dummy
};


int pdcdes_key_is_weak_fn_dummy(void *key,
                                unsigned long  length)
{
    printf("%s\n", __func__);
    return -1;
}

void pdcdes_key_set_odd_parity_fn_dummy(void *key,
                                        unsigned long length)
{
    printf("%s\n", __func__);
}

void pdcpad_xts_decrypt_fn_dummy(const struct ccmode_xts *xts,
                                 ccxts_ctx *ctx,
                                 unsigned long nbytes,
                                 const void *in,
                                 void *out)
{
    printf("%s\n", __func__);
}

void pdcpad_xts_encrypt_fn_dummy(const struct ccmode_xts *xts,
                                 ccxts_ctx *ctx,
                                 unsigned long nbytes,
                                 const void *in,
                                 void *out)
{
    printf("%s\n", __func__);
}
