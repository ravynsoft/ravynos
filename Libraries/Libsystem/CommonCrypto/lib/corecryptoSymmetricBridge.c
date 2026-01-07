/*                                                                              
 * Copyright (c) 2012 Apple Inc. All Rights Reserved.
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

#include "corecryptoSymmetricBridge.h"
#include <corecrypto/ccrc4.h>
#include "ccdebug.h"


static void *noMode(void) { return NULL; }

// RC4 as a mode trick ...

static int rc4ModeInit(const struct ccmode_ofb * __unused ofb, ccofb_ctx *ctx,
                       size_t key_len, const void *key,
                       const void * __unused iv)
{
    ccrc4_eay.init((ccrc4_ctx *)ctx, key_len, key);
    return 0;
}

static int rc4crypt(ccofb_ctx *ctx, size_t nbytes, const void *in, void *out)
{
    ccrc4_eay.crypt((ccrc4_ctx *) ctx, nbytes, in, out);
    return 0;
}

typedef struct eay_rc4_key_st
{
	uint32_t x,y;
	uint32_t data[256];
} eay_RC4_KEY;

static const struct ccmode_ofb rc4mode = {
    .size = sizeof(eay_RC4_KEY),
    .block_size = 1,
    .init = rc4ModeInit,
    .ofb = rc4crypt,
};



static const struct ccmode_ofb *cc_rc4_crypt_mode(void)
{
    return &rc4mode;
}


// 2 dimensional array of various mode/cipher contexts
// encrypt/decrypt x algorithm matching the list in CommonCryptor.h

const modeList ccmodeList[CC_SUPPORTED_CIPHERS][CC_DIRECTIONS] = {
    { // AES
        { ccaes_ecb_encrypt_mode, ccaes_cbc_encrypt_mode, ccaes_cfb_encrypt_mode, ccaes_cfb8_encrypt_mode, ccaes_ctr_crypt_mode, ccaes_ofb_crypt_mode, ccaes_xts_encrypt_mode, ccaes_gcm_encrypt_mode, ccaes_ccm_encrypt_mode },
        { ccaes_ecb_decrypt_mode, ccaes_cbc_decrypt_mode, ccaes_cfb_decrypt_mode, ccaes_cfb8_decrypt_mode, ccaes_ctr_crypt_mode, ccaes_ofb_crypt_mode, ccaes_xts_decrypt_mode, ccaes_gcm_decrypt_mode,  ccaes_ccm_decrypt_mode }
    },
    
    { // DES
        { ccdes_ecb_encrypt_mode, ccdes_cbc_encrypt_mode, ccdes_cfb_encrypt_mode, ccdes_cfb8_encrypt_mode, ccdes_ctr_crypt_mode, ccdes_ofb_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode },
        { ccdes_ecb_decrypt_mode, ccdes_cbc_decrypt_mode, ccdes_cfb_decrypt_mode, ccdes_cfb8_decrypt_mode, ccdes_ctr_crypt_mode, ccdes_ofb_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode }
    },
    
    { // DES3
        { ccdes3_ecb_encrypt_mode, ccdes3_cbc_encrypt_mode, ccdes3_cfb_encrypt_mode, ccdes3_cfb8_encrypt_mode, ccdes3_ctr_crypt_mode, ccdes3_ofb_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode },
        { ccdes3_ecb_decrypt_mode, ccdes3_cbc_decrypt_mode, ccdes3_cfb_decrypt_mode, ccdes3_cfb8_decrypt_mode, ccdes3_ctr_crypt_mode, ccdes3_ofb_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode }
    },
    
    { // CAST
        { cccast_ecb_encrypt_mode, cccast_cbc_encrypt_mode, cccast_cfb_encrypt_mode, cccast_cfb8_encrypt_mode, cccast_ctr_crypt_mode, cccast_ofb_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode },
        { cccast_ecb_decrypt_mode, cccast_cbc_decrypt_mode, cccast_cfb_decrypt_mode, cccast_cfb8_decrypt_mode, cccast_ctr_crypt_mode, cccast_ofb_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode }
    },
    
    { // RC4 - hijack OFB to put in streaming cipher descriptor
        { (ecb_p) noMode, (cbc_p) noMode, (cfb_p) noMode, (cfb8_p) noMode, (ctr_p) noMode, cc_rc4_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode },
        { (ecb_p) noMode, (cbc_p) noMode, (cfb_p) noMode, (cfb8_p) noMode, (ctr_p) noMode, cc_rc4_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode },
    },

    
    { // RC2
        { ccrc2_ecb_encrypt_mode, ccrc2_cbc_encrypt_mode, ccrc2_cfb_encrypt_mode, ccrc2_cfb8_encrypt_mode, ccrc2_ctr_crypt_mode, ccrc2_ofb_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode },
        { ccrc2_ecb_decrypt_mode, ccrc2_cbc_decrypt_mode, ccrc2_cfb_decrypt_mode, ccrc2_cfb8_decrypt_mode, ccrc2_ctr_crypt_mode, ccrc2_ofb_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode }
    },

    { // Blowfish
        { ccblowfish_ecb_encrypt_mode, ccblowfish_cbc_encrypt_mode, ccblowfish_cfb_encrypt_mode, ccblowfish_cfb8_encrypt_mode, ccblowfish_ctr_crypt_mode, ccblowfish_ofb_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode },
        { ccblowfish_ecb_decrypt_mode, ccblowfish_cbc_decrypt_mode, ccblowfish_cfb_decrypt_mode, ccblowfish_cfb8_decrypt_mode, ccblowfish_ctr_crypt_mode, ccblowfish_ofb_crypt_mode, (xts_p) noMode, (gcm_p) noMode, (ccm_p) noMode }
    },
};


// Thunks
//ECB

static size_t ccecb_mode_get_ctx_size(corecryptoMode modeObject) { return modeObject.ecb->size; }
static size_t ccecb_mode_get_block_size(corecryptoMode modeObject) { return modeObject.ecb->block_size; }
static int ccecb_mode_setup(corecryptoMode modeObj, const void * __unused IV,
                             const void *key, size_t keylen, const void * __unused tweak,
                             size_t __unused tweaklen, int __unused options, modeCtx ctx)
{
    return modeObj.ecb->init(modeObj.ecb, ctx.ecb, keylen, key);
}

static int ccecb_mode_crypt(corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    return modeObj.ecb->ecb(ctx.ecb, len / ccecb_mode_get_block_size(modeObj), in, out);
}

const cc2CCModeDescriptor ccecb_mode = {
    .mode_get_ctx_size = ccecb_mode_get_ctx_size,
    .mode_get_block_size = ccecb_mode_get_block_size,
    .mode_setup = ccecb_mode_setup,
    .mode_encrypt = ccecb_mode_crypt,
    .mode_decrypt = ccecb_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};

// CBC

static size_t cccbc_mode_get_ctx_size(const corecryptoMode modeObject) { return modeObject.cbc->size + 16; }
static size_t cccbc_mode_get_block_size(const corecryptoMode modeObject) { return modeObject.cbc->block_size; }
static int cccbc_mode_setup(const corecryptoMode modeObj, const void *iv,
                             const void *key, size_t keylen, const void * __unused tweak,
                             size_t __unused tweaklen, int __unused options, modeCtx ctx)
{
    memcpy(ctx.cbc->iv, iv, modeObj.cbc->block_size);
    return modeObj.cbc->init(modeObj.cbc, &ctx.cbc->cbc, keylen, key);
}

static int cccbc_mode_crypt(const corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    return modeObj.cbc->cbc(&ctx.cbc->cbc, (cccbc_iv *) ctx.cbc->iv, len / cccbc_mode_get_block_size(modeObj), in, out);
}

static int cccbc_getiv(const corecryptoMode modeObj, void *iv, uint32_t *len, modeCtx ctx)
{
    if(*len < cccbc_mode_get_block_size(modeObj)) {
        *len = (uint32_t) cccbc_mode_get_block_size(modeObj);
        return -1;
    }
    memcpy(iv, ctx.cbc->iv, *len = (uint32_t) cccbc_mode_get_block_size(modeObj));
    return 0;
}

static int cccbc_setiv(const corecryptoMode modeObj, const void *iv, uint32_t len, modeCtx ctx)
{
    if(len != cccbc_mode_get_block_size(modeObj)) return -1;
    memcpy(ctx.cbc->iv, iv, cccbc_mode_get_block_size(modeObj));
    return 0;
}

const cc2CCModeDescriptor cccbc_mode = {
    .mode_get_ctx_size = cccbc_mode_get_ctx_size,
    .mode_get_block_size = cccbc_mode_get_block_size,
    .mode_setup = cccbc_mode_setup,
    .mode_encrypt = cccbc_mode_crypt,
    .mode_decrypt = cccbc_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = cccbc_setiv,
    .mode_getiv = cccbc_getiv
};

// CFB

static size_t cccfb_mode_get_ctx_size(const corecryptoMode modeObject) { return modeObject.cfb->size; }
static size_t cccfb_mode_get_block_size(const corecryptoMode modeObject) { return modeObject.cfb->block_size; }
static int cccfb_mode_setup(const corecryptoMode modeObj, const void *iv,
                             const void *key, size_t keylen, const void * __unused tweak,
                             size_t __unused tweaklen, int __unused options, modeCtx ctx)
{
    return modeObj.cfb->init(modeObj.cfb, ctx.cfb, keylen, key, iv);
}

static int cccfb_mode_crypt(const corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    return modeObj.cfb->cfb(ctx.cfb, len / cccfb_mode_get_block_size(modeObj), in, out);
}

const cc2CCModeDescriptor cccfb_mode = {
    .mode_get_ctx_size = cccfb_mode_get_ctx_size,
    .mode_get_block_size = cccfb_mode_get_block_size,
    .mode_setup = cccfb_mode_setup,
    .mode_encrypt = cccfb_mode_crypt,
    .mode_decrypt = cccfb_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};


// CFB8

static size_t cccfb8_mode_get_ctx_size(const corecryptoMode modeObject) { return modeObject.cfb8->size; }
static size_t cccfb8_mode_get_block_size(const corecryptoMode modeObject) { return modeObject.cfb8->block_size; }
static int cccfb8_mode_setup(const corecryptoMode modeObj, const void *iv,
                              const void *key, size_t keylen, const void * __unused tweak,
                              size_t __unused tweaklen, int __unused options, modeCtx ctx)
{
    return modeObj.cfb8->init(modeObj.cfb8, ctx.cfb8, keylen, key, iv);
}

static int cccfb8_mode_crypt(const corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    return modeObj.cfb8->cfb8(ctx.cfb8, len / cccfb8_mode_get_block_size(modeObj), in, out);
}

const cc2CCModeDescriptor cccfb8_mode = {
    .mode_get_ctx_size = cccfb8_mode_get_ctx_size,
    .mode_get_block_size = cccfb8_mode_get_block_size,
    .mode_setup = cccfb8_mode_setup,
    .mode_encrypt = cccfb8_mode_crypt,
    .mode_decrypt = cccfb8_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};

// CTR

static size_t ccctr_mode_get_ctx_size(const corecryptoMode modeObject) { return modeObject.ctr->size; }
static size_t ccctr_mode_get_block_size(const corecryptoMode modeObject) { return modeObject.ctr->block_size; }
static int ccctr_mode_setup(const corecryptoMode modeObj, const void *iv,
                             const void *key, size_t keylen, const void * __unused tweak,
                             size_t __unused tweaklen, int __unused options, modeCtx ctx)
{
    return modeObj.ctr->init(modeObj.ctr, ctx.ctr, keylen, key, iv);
}

static int ccctr_mode_crypt(const corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    return modeObj.ctr->ctr(ctx.ctr, len / ccctr_mode_get_block_size(modeObj), in, out);
}

static int ccctr_setiv(const corecryptoMode modeObj, const void *iv, uint32_t len, modeCtx ctx)
{
    if(len != modeObj.ctr->ecb_block_size) return -1;
    modeObj.ctr->setctr(modeObj.ctr, ctx.ctr, iv);
    return 0;
}

const cc2CCModeDescriptor ccctr_mode = {
    .mode_get_ctx_size = ccctr_mode_get_ctx_size,
    .mode_get_block_size = ccctr_mode_get_block_size,
    .mode_setup = ccctr_mode_setup,
    .mode_encrypt = ccctr_mode_crypt,
    .mode_decrypt = ccctr_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = ccctr_setiv,
    .mode_getiv = NULL
};

// OFB

static size_t ccofb_mode_get_ctx_size(const corecryptoMode modeObject) { return modeObject.ofb->size; }
static size_t ccofb_mode_get_block_size(const corecryptoMode modeObject) { return modeObject.ofb->block_size; }
static int ccofb_mode_setup(const corecryptoMode modeObj, const void *iv,
                             const void *key, size_t keylen, const void * __unused tweak,
                             size_t __unused tweaklen, int __unused options, modeCtx ctx)
{
    return modeObj.ofb->init(modeObj.ofb, ctx.ofb, keylen, key, iv);
}

static int ccofb_mode_crypt(const corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    return modeObj.ofb->ofb(ctx.ofb, len / ccofb_mode_get_block_size(modeObj), in, out);
}

const cc2CCModeDescriptor ccofb_mode = {
    .mode_get_ctx_size = ccofb_mode_get_ctx_size,
    .mode_get_block_size = ccofb_mode_get_block_size,
    .mode_setup = ccofb_mode_setup,
    .mode_encrypt = ccofb_mode_crypt,
    .mode_decrypt = ccofb_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};

// XTS
/* For now we always schedule both encrypt and decrypt contexts for AES-XTS.  Original CommonCrypto support
 * allowed a "both" (kCCEncrypt and kCCDecrypt) capability used for AES-XTS block I/O.  The initialization 
 * and correct mode objext and context passing are done at the CommonCryptor layer.
 */


static size_t ccxts_mode_get_ctx_size(const corecryptoMode modeObject) { return modeObject.xts->size; }
static size_t ccxts_mode_get_block_size(const corecryptoMode modeObject) { return modeObject.xts->block_size; }
static int ccxts_mode_setup(const corecryptoMode modeObj, const void * __unused iv,
                             const void *key, size_t keylen, const void *tweak,
                             size_t __unused tweaklen, int __unused options, modeCtx ctx)
{
    return modeObj.xts->init(modeObj.xts, ctx.xts, keylen, key, tweak);
}

#ifdef UNUSED_INTERFACE
static void ccxts_mode_crypt(const corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    modeObj.xts->xts(ctx.xts, len / ccxts_mode_get_block_size(modeObj), in, out);
}

static int ccxts_setiv(const corecryptoMode modeObj, const void *iv, uint32_t len, modeCtx ctx)
{
    if(len != modeObj.xts->block_size) return -1;
    modeObj.xts->set_tweak(ctx.xts, iv);
    return 0;
}

static int ccxts_getiv(const corecryptoMode modeObj, void *iv, uint32_t *len, modeCtx ctx)
{
    if(*len < modeObj.xts->block_size) {
        *len = modeObj.xts->block_size;
        return -1;
    }
    memcpy(iv, modeObj.xts->xts(ctx.xts, 0, NULL, NULL), *len = modeObj.xts->block_size);
    return 0;
}
#endif

/*
 * These match what we had in libtomcrypt - they really are "this is a logical block" routines, so need
 * to handle partial blocks - so we use corecrypto's xts pad routines in every case.
 */

static int ccxts_mode_encrypt_tweak(const corecryptoMode modeObj, const void *in, size_t len, void *out, const void *iv, modeCtx ctx)
{
    int rc = CCERR_OK;
    ccxts_tweak_decl(ccxts_context_size(modeObj.xts), tweak);
    rc = modeObj.xts->set_tweak(ctx.xts, tweak, iv);
    ccpad_xts_encrypt(modeObj.xts, ctx.xts, tweak, len, in, out);
    return rc;
}

static int ccxts_mode_decrypt_tweak(const corecryptoMode modeObj, const void *in, size_t len, void *out, const void *iv, modeCtx ctx)
{
    int rc = CCERR_OK;
    ccxts_tweak_decl(ccxts_context_size(modeObj.xts), tweak);
    rc = modeObj.xts->set_tweak(ctx.xts, tweak, iv);
    ccpad_xts_decrypt(modeObj.xts, ctx.xts, tweak, len, in, out);
    return rc;
}


const cc2CCModeDescriptor ccxts_mode = {
    .mode_get_ctx_size = ccxts_mode_get_ctx_size,
    .mode_get_block_size = ccxts_mode_get_block_size,
    .mode_setup = ccxts_mode_setup,
    .mode_encrypt = NULL,
    .mode_decrypt = NULL,
    .mode_encrypt_tweaked = ccxts_mode_encrypt_tweak,
    .mode_decrypt_tweaked = ccxts_mode_decrypt_tweak,
    .mode_done = NULL,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};

// GCM

static size_t ccgcm_mode_get_ctx_size(const corecryptoMode modeObject) { return modeObject.gcm->size; }
static size_t ccgcm_mode_get_block_size(const corecryptoMode modeObject) { return modeObject.gcm->block_size; }
static int ccgcm_mode_setup(const corecryptoMode modeObj, const void * __unused iv,
                             const void *key, size_t keylen, const void * __unused tweak,
                             size_t __unused tweaklen, int __unused  options, modeCtx ctx)
{
    return modeObj.gcm->init(modeObj.gcm, ctx.gcm, keylen, key);
}

static int ccgcm_mode_crypt(const corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    return modeObj.gcm->gcm(ctx.gcm, len, in, out);
}

static int ccgcm_setiv(const corecryptoMode modeObj, const void *iv, uint32_t len, modeCtx ctx)
{
    return ccgcm_set_iv_legacy(modeObj.gcm, ctx.gcm, len, iv);
}


const cc2CCModeDescriptor ccgcm_mode = {
    .mode_get_ctx_size = ccgcm_mode_get_ctx_size,
    .mode_get_block_size = ccgcm_mode_get_block_size,
    .mode_setup = ccgcm_mode_setup,
    .mode_encrypt = ccgcm_mode_crypt,
    .mode_decrypt = ccgcm_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = NULL,
    .mode_setiv = ccgcm_setiv,
    .mode_getiv = NULL
};

// CCM

static size_t ccccm_mode_get_ctx_size(const corecryptoMode modeObject) { return modeObject.ccm->size + sizeof(ccm_nonce_ctx); }
static size_t ccccm_mode_get_block_size(const corecryptoMode modeObject) { return modeObject.ccm->block_size; }
static int ccccm_mode_setup(const corecryptoMode modeObj, const void * __unused iv,
                             const void *key, size_t keylen, const void * __unused tweak,
                             size_t __unused tweaklen, int __unused options, modeCtx ctx)
{
    int rc = modeObj.ccm->init(modeObj.ccm, &ctx.ccm->ccm, keylen, key);
    ctx.ccm->nonce_size = (size_t) 0xffffffffffffffff;
    ctx.ccm->mac_size = (size_t) 0xffffffffffffffff;
    ctx.ccm->ad_len = (size_t) 0xffffffffffffffff;
    ctx.ccm->total_len = (size_t) 0xffffffffffffffff;
    return rc;
}

static int ccccm_mode_crypt(const corecryptoMode modeObj, const void *in, void *out, size_t len, modeCtx ctx)
{
    return modeObj.ccm->ccm(&ctx.ccm->ccm, (ccccm_nonce *) &ctx.ccm->nonce, len, in, out);
}

static int ccccm_mode_done(const corecryptoMode modeObj, modeCtx ctx)
{
    modeObj.ccm->finalize(&ctx.ccm->ccm, (ccccm_nonce *) &ctx.ccm->nonce, ctx.ccm->mac);
    ctx.ccm->mac_size = ctx.ccm->nonce.mac_size;
    return 0;
}

const cc2CCModeDescriptor ccccm_mode = {
    .mode_get_ctx_size = ccccm_mode_get_ctx_size,
    .mode_get_block_size = ccccm_mode_get_block_size,
    .mode_setup = ccccm_mode_setup,
    .mode_encrypt = ccccm_mode_crypt,
    .mode_decrypt = ccccm_mode_crypt,
    .mode_encrypt_tweaked = NULL,
    .mode_decrypt_tweaked = NULL,
    .mode_done = ccccm_mode_done,
    .mode_setiv = NULL,
    .mode_getiv = NULL
};


// Padding

static int ccpkcs7_encrypt_pad(modeCtx ctx, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj, void *buff, size_t len, void *cipherText, size_t *moved)
{
    ccpad_pkcs7_encrypt(modeObj.cbc, &ctx.cbc->cbc, (cccbc_iv*) ctx.cbc->iv, len, buff, cipherText);
    *moved = modeptr->mode_get_block_size(modeObj);
    return 0;
}
static int ccpkcs7_decrypt_pad(modeCtx ctx, const cc2CCModeDescriptor * __unused modeptr, const corecryptoMode modeObj, void *buff, size_t len, void *plainText, size_t *moved)
{
    *moved = ccpad_pkcs7_decrypt(modeObj.cbc, &ctx.cbc->cbc, (cccbc_iv*) ctx.cbc->iv, len, buff, plainText);
    return 0;
}


static int ccpkcs7_encrypt_ecb_pad(modeCtx ctx, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj, void *buff, size_t len, void *cipherText, size_t *moved)
{
    ccpad_pkcs7_ecb_encrypt(modeObj.ecb, ctx.ecb, len, buff, cipherText);
    *moved = modeptr->mode_get_block_size(modeObj);
    return 0;
}
static int ccpkcs7_decrypt_ecb_pad(modeCtx ctx, const cc2CCModeDescriptor * __unused modeptr, const corecryptoMode modeObj, void *buff, size_t len, void *plainText, size_t *moved)
{
    *moved = ccpad_pkcs7_ecb_decrypt(modeObj.ecb, ctx.ecb, len, buff, plainText);
    return 0;
}


/*
 * Maximum space needed for padding.
 */

// Utility functions
static inline size_t cc_round_down_by_blocksize(const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj, size_t length) {
    size_t blocksize = modeptr->mode_get_block_size(modeObj);
    return length / blocksize * blocksize;
}

static inline size_t cc_round_up_by_blocksize(const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj, size_t length) {
    return cc_round_down_by_blocksize(modeptr, modeObj, length + modeptr->mode_get_block_size(modeObj) - 1);
}

#define MAXBLOCKSIZE_PKCS7 128

static size_t ccpkcs7_padlen(int encrypting, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj, size_t inputLength, bool final)
{
    size_t retval;

    if(final) {
        if(encrypting) retval = cc_round_down_by_blocksize(modeptr, modeObj, inputLength+modeptr->mode_get_block_size(modeObj)); // round up to blocksize
        else retval = inputLength; // largest would be inputLength - 1 actually.
    } else {
        if(encrypting) retval = cc_round_down_by_blocksize(modeptr, modeObj, inputLength); // round down to blocksize
        else {
            if(inputLength && (inputLength % modeptr->mode_get_block_size(modeObj) == 0)) inputLength--;
            retval = cc_round_down_by_blocksize(modeptr, modeObj, inputLength);
        }
    }
    
    return retval;
}

/*
 * How many bytes to reserve to enable padding - this is pre-encrypt/decrypt bytes.
 */

static size_t ccpkcs7_reserve(int encrypt, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj)
{
    if(encrypt) {
		return 0;
    } else {
    	return modeptr->mode_get_block_size(modeObj);
    }
}

const cc2CCPaddingDescriptor ccpkcs7_pad = {
    .encrypt_pad = ccpkcs7_encrypt_pad,
    .decrypt_pad = ccpkcs7_decrypt_pad,
    .padlen = ccpkcs7_padlen,
    .padreserve = ccpkcs7_reserve,
};

const cc2CCPaddingDescriptor ccpkcs7_ecb_pad = {
    .encrypt_pad = ccpkcs7_encrypt_ecb_pad,
    .decrypt_pad = ccpkcs7_decrypt_ecb_pad,
    .padlen = ccpkcs7_padlen,
    .padreserve = ccpkcs7_reserve,
};


static int cccts3_encrypt_pad(modeCtx ctx, const cc2CCModeDescriptor * __unused modeptr, const corecryptoMode modeObj, void *buff, size_t len, void *cipherText, size_t *moved)
{
    ccpad_cts3_encrypt(modeObj.cbc, &ctx.cbc->cbc, (cccbc_iv*) ctx.cbc->iv, len, buff, cipherText);
    *moved = len;
    return 0;
}
static int cccts3_decrypt_pad(modeCtx ctx, const cc2CCModeDescriptor * __unused modeptr, const corecryptoMode modeObj, void *buff, size_t len, void *plainText, size_t *moved)
{
    ccpad_cts3_decrypt(modeObj.cbc, &ctx.cbc->cbc, (cccbc_iv*) ctx.cbc->iv, len, buff, plainText);
    *moved = len;
    return 0;
}



/*
 * Maximum space needed for padding.
 */

#define MAXBLOCKSIZE_PKCS7 128

static size_t cccts3_padlen(int encrypting, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj, size_t inputLength, bool final)
{
    size_t retval;
    size_t blocksize = modeptr->mode_get_block_size(modeObj);
    if(final) {
        if(encrypting) retval = cc_round_up_by_blocksize(modeptr, modeObj, inputLength); // round up to blocksize
        else retval = inputLength; // largest would be inputLength - 1 actually.
    } else {
        if(encrypting) {
            if(inputLength <= blocksize) retval = 0;
            else retval = cc_round_down_by_blocksize(modeptr, modeObj, inputLength - blocksize);
        }
        else retval = inputLength; // largest would be inputLength - 1 actually.
    }
    
    return retval;
}

/*
 * How many bytes to reserve to enable padding - this is pre-encrypt/decrypt bytes.
 */

static size_t cccts3_reserve(int __unused encrypt, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj)
{
    return modeptr->mode_get_block_size(modeObj) * 2;
}

const cc2CCPaddingDescriptor cccts3_pad = {
    .encrypt_pad = cccts3_encrypt_pad,
    .decrypt_pad = cccts3_decrypt_pad,
    .padlen = cccts3_padlen,
    .padreserve = cccts3_reserve,
};


static int ccnopad_encrypt_pad(modeCtx  __unused ctx, const cc2CCModeDescriptor * __unused modeptr, const corecryptoMode __unused modeObj, void * __unused buff, size_t len, void * __unused cipherText, size_t *moved)
{
    *moved = 0;
    if(len != 0) return -1;
    return 0;
}
static int ccnopad_decrypt_pad(modeCtx __unused ctx, const cc2CCModeDescriptor * __unused modeptr, const corecryptoMode  __unused modeObj, void *  __unused buff, size_t len, void * __unused plainText, size_t *moved)
{
    *moved = 0;
    if(len != 0) return -1;
    return 0;
}

/*
 * Maximum space needed for padding.
 */

static size_t ccnopad_padlen(int __unused encrypting, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj, size_t inputLength, bool __unused final)
{
    return cc_round_down_by_blocksize(modeptr, modeObj, inputLength);
}

/*
 * How many bytes to reserve to enable padding - this is pre-encrypt/decrypt bytes.
 */

static size_t ccnopad_reserve(int __unused encrypt, const cc2CCModeDescriptor *  __unused modeptr, const corecryptoMode __unused modeObj)
{
    return 0;
}

const cc2CCPaddingDescriptor ccnopad_pad = {
    .encrypt_pad = ccnopad_encrypt_pad,
    .decrypt_pad = ccnopad_decrypt_pad,
    .padlen = ccnopad_padlen,
    .padreserve = ccnopad_reserve,
};


