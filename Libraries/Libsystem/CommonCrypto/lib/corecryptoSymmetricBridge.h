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

#ifndef CommonCrypto_corecryptoSymmetricBridge_h
#define CommonCrypto_corecryptoSymmetricBridge_h

#include <stdbool.h>
#include <corecrypto/ccn.h>
#include <corecrypto/ccmode.h>
#include <corecrypto/ccmode_impl.h>
#include <corecrypto/ccmode_factory.h>
#include <corecrypto/ccaes.h>
#include <corecrypto/ccdes.h>
#include <corecrypto/cccast.h>
#include <corecrypto/ccrc2.h>
#include <corecrypto/ccblowfish.h>
#include <corecrypto/ccpad.h>

#define CC_SUPPORTED_CIPHERS 7
#define CC_DIRECTIONS 2

typedef union {
    const struct ccmode_ecb *ecb;
    const struct ccmode_cbc *cbc;
    const struct ccmode_cfb *cfb;
    const struct ccmode_cfb8 *cfb8;
    const struct ccmode_ctr *ctr;
    const struct ccmode_ofb *ofb;
    const struct ccmode_xts *xts;
    const struct ccmode_gcm *gcm;
    const struct ccmode_ccm *ccm;
} corecryptoMode;

typedef const struct ccmode_ecb* (*ecb_p) (void);
typedef const struct ccmode_cbc* (*cbc_p) (void);
typedef const struct ccmode_cfb* (*cfb_p) (void);
typedef const struct ccmode_cfb8* (*cfb8_p) (void);
typedef const struct ccmode_ctr* (*ctr_p) (void);
typedef const struct ccmode_ofb* (*ofb_p) (void);
typedef const struct ccmode_xts* (*xts_p) (void);
typedef const struct ccmode_gcm* (*gcm_p) (void);
typedef const struct ccmode_ccm* (*ccm_p) (void);



typedef struct modes_t {
    ecb_p   ecb;
    cbc_p   cbc;
    cfb_p   cfb;
    cfb8_p  cfb8;
    ctr_p   ctr;
    ofb_p   ofb;
    xts_p   xts;
    gcm_p   gcm;
    ccm_p   ccm;
} modeList;

extern const modeList ccmodeList[CC_SUPPORTED_CIPHERS][CC_DIRECTIONS];

typedef struct cbc_with_iv_t {
    uint8_t iv[16];
    cccbc_ctx cbc;
} cbc_iv_ctx;

typedef struct ccm_with_nonce_t {
    size_t total_len;
    size_t mac_size;
    size_t nonce_size;
    size_t ad_len;
    uint8_t nonce_buf[16];
    uint8_t mac[16];
    struct _ccmode_ccm_nonce nonce;
    ccccm_ctx ccm;
} ccm_nonce_ctx;

typedef union {
    void *data;
    ccecb_ctx *ecb;
    cbc_iv_ctx *cbc;
    cccfb_ctx *cfb;
    cccfb8_ctx *cfb8;
    ccctr_ctx *ctr;
    ccofb_ctx *ofb;
    ccxts_ctx *xts;
    ccgcm_ctx *gcm;
    ccm_nonce_ctx *ccm;
} modeCtx;


#pragma mark Modes

/** Setup the mode
 @param modeObj		ccmode to setup
 @param iv		The initial vector
 @param key		The input symmetric key
 @param keylen		The length of the input key (octets)
 @param tweak		The input tweak or salt
 @param tweaklen	The length of the tweak or salt (if variable)
 (octets)
 @param options		Mask for any mode options
 @param ctx		[out] The destination of the mode context
 */

typedef int (*ccmode_setup_p)(const corecryptoMode modeObj, const void *iv,
                            const void *key, size_t keylen, const void *tweak,
                            size_t tweaklen, int options, modeCtx ctx);
/** Encrypt a block
 @param pt		The plaintext
 @param ct		[out] The ciphertext
 @param len		the length of data (in == out) octets
 @param ctx		The mode context
 */

typedef int (*ccmode_encrypt_p)(const corecryptoMode modeObj, const void *pt, void *ct, size_t len, modeCtx ctx);

/** Decrypt a block
 @param ct		The ciphertext
 @param pt		[out] The plaintext
 @param len		the length of data (in == out) octets
 @param ctx		The mode context
 */
typedef int (*ccmode_decrypt_p)(const corecryptoMode modeObj, const void *ct, void *pt, size_t len, modeCtx ctx);

/** Encrypt a block with a tweak (XTS mode currently)
 @param pt		The plaintext
 @param ct		[out] The ciphertext
 @param len		the length of data (in == out) octets
 @param tweak		The 128--bit encryption tweak (e.g. sector
 number)
 @param ctx		The mode context
  */
typedef int (*ccmode_encrypt_tweaked_p)(const corecryptoMode modeObj, const void *pt, size_t len,
                                      void *ct, const void *tweak, modeCtx ctx);
/** Decrypt a block with a tweak (XTS mode currently)
 @param ct		The ciphertext
 @param pt		[out] The plaintext
 @param len		the length of data (in == out) octets
 @param ctx		The mode context
 */
typedef int (*ccmode_decrypt_tweaked_p)(const corecryptoMode modeObj, const void *ct, size_t len,
                                      void *pt, const void *tweak, modeCtx ctx);
/** Terminate the mode
 @param ctx		[out] The mode context
 */
typedef int (*ccmode_done_p)(const corecryptoMode modeObj, modeCtx ctx);
/** Set an Initial Vector
 @param iv		The initial vector
 @param len		The length of the initial vector
 @param ctx		The mode context
 */
typedef int (*ccmode_setiv_p)(const corecryptoMode modeObj, const void *iv, uint32_t len, modeCtx ctx);
/** Get an Initial Vector
 @param iv		[out] The initial vector
 @param len		The length of the initial vector
 @param ctx		The mode context
 */
typedef int (*ccmode_getiv_p)(const corecryptoMode modeObj, void *iv, uint32_t *len, modeCtx ctx);

/** Get the mode context size
 @param modeObj a pointer to the mode object.
 @return the size of the context
 */
typedef size_t (*ccmode_get_ctx_size)(const corecryptoMode modeObj);

/** Get the mode block size
 @param modeObj a pointer to the mode object.
 @return the size of the block
 */
typedef size_t (*ccmode_get_block_size)(const corecryptoMode modeObj);

typedef struct cc2CCModeDescriptor_t {
//    ccBufStrat              bufStrat;
    ccmode_get_ctx_size     mode_get_ctx_size;
    ccmode_get_block_size   mode_get_block_size;
	ccmode_setup_p          mode_setup;
	ccmode_encrypt_p        mode_encrypt;
	ccmode_decrypt_p        mode_decrypt;
	ccmode_encrypt_tweaked_p mode_encrypt_tweaked;
	ccmode_decrypt_tweaked_p mode_decrypt_tweaked;
	ccmode_done_p           mode_done;
	ccmode_setiv_p          mode_setiv;
	ccmode_getiv_p          mode_getiv;
} cc2CCModeDescriptor, *cc2CCModeDescriptorPtr;


extern const cc2CCModeDescriptor ccecb_mode;
extern const cc2CCModeDescriptor cccbc_mode;
extern const cc2CCModeDescriptor cccfb_mode;
extern const cc2CCModeDescriptor cccfb8_mode;
extern const cc2CCModeDescriptor ccctr_mode;
extern const cc2CCModeDescriptor ccofb_mode;
extern const cc2CCModeDescriptor ccxts_mode;
extern const cc2CCModeDescriptor ccgcm_mode;
extern const cc2CCModeDescriptor ccccm_mode;


// Buffer and Padding Handling

/*
 * Fill out the padding for a buffer.  The blocksize and starting points are
 * used to determine how much needs to be padded.  If startpoint is 0
 * then a full new buffer is added.  Blocksize cannot be greater than 256.
 */

typedef int (*cc_encrypt_pad_p)(modeCtx ctx, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj, void *buff, size_t startpoint, void *cipherText, size_t *moved);
typedef int (*cc_decrypt_pad_p)(modeCtx ctx, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj, void *buff, size_t startpoint, void *plainText, size_t *moved);

/*
 * Maximum space needed for padding.
 */

typedef size_t (*ccpadlen_p) (int encrypt, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj, size_t inputLength, bool final);

/*
 * How many bytes to reserve to enable padding - this is pre-encrypt/decrypt bytes.
 */

typedef size_t (*ccreserve_p) (int encrypt, const cc2CCModeDescriptor *modeptr, const corecryptoMode modeObj);

typedef struct cc2CCPaddingDescriptor_t {
    cc_encrypt_pad_p    encrypt_pad;
    cc_decrypt_pad_p    decrypt_pad;
    ccpadlen_p          padlen;
    ccreserve_p         padreserve;
} cc2CCPaddingDescriptor, *cc2CCPaddingDescriptorPtr;

extern const cc2CCPaddingDescriptor ccnopad_pad;
extern const cc2CCPaddingDescriptor cccts3_pad;
extern const cc2CCPaddingDescriptor ccpkcs7_pad;
extern const cc2CCPaddingDescriptor ccpkcs7_ecb_pad;

#endif
