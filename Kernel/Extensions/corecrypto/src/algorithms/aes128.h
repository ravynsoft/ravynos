#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>

struct _pdcmode_aes128_ctx {
    uint8_t RoundKey[176];
};

struct pdccbc_iv {
    uint8_t b[16];
};

void AES128_set_key(struct _pdcmode_aes128_ctx *ctx, const void *key);

void AES128_ECB_encrypt(const struct _pdcmode_aes128_ctx *ctx, unsigned long nblocks, const void *in, void *out);
void AES128_ECB_decrypt(const struct _pdcmode_aes128_ctx *ctx, unsigned long nblocks, const void *in, void *out);

void AES128_CBC_encrypt(const struct _pdcmode_aes128_ctx *ctx, struct pdccbc_iv* iv, unsigned long nblocks, const void *in, void *out);
void AES128_CBC_decrypt(const struct _pdcmode_aes128_ctx *ctx, struct pdccbc_iv* iv, unsigned long nblocks, const void *in, void *out);

#endif //_AES_H_
