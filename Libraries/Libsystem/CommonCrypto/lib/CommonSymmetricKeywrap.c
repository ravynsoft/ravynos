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

// #define COMMON_SYMMETRIC_KEYWRAP_FUNCTIONS
#include <CommonCrypto/CommonSymmetricKeywrap.h>
#include <CommonCrypto/CommonCryptor.h>
#include "CommonCryptorPriv.h"
#include <AssertMacros.h>
#include "ccdebug.h"
#include <corecrypto/ccwrap_priv.h>


static const uint8_t rfc3394_iv_data[] = {
    0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6
};

const uint8_t * const CCrfc3394_iv = rfc3394_iv_data;
const size_t CCrfc3394_ivLen = sizeof(rfc3394_iv_data);

int
CCSymmetricKeyWrap(CCWrappingAlgorithm __unused algorithm,
				   const uint8_t *iv, const size_t ivLen,
				   const uint8_t *kek, size_t kekLen,
				   const uint8_t *rawKey, size_t rawKeyLen,
				   uint8_t  *wrappedKey, size_t *wrappedKeyLen)
{
    CC_DEBUG_LOG("Entering\n");
    int err = kCCUnspecifiedError;

    const struct ccmode_ecb *ccmode = getCipherMode(kCCAlgorithmAES128, kCCModeECB, kCCEncrypt).ecb;
    ccecb_ctx_decl(ccmode->size, ctx);

    require_action((kekLen == CCAES_KEY_SIZE_128 ||
                    kekLen == CCAES_KEY_SIZE_192 ||
                    kekLen == CCAES_KEY_SIZE_256),
                   out, err = kCCParamError);
    require_action(wrappedKeyLen && (*wrappedKeyLen >= ccwrap_wrapped_size(rawKeyLen)), out, err = kCCParamError);

    /* due to rdar://problem/44095510, we tolerate a null IV */
    /* if the IV is not null, it must be long enough to satisfy corecrypto */
    /* if the IV is null, use the default IV */
    require_action((iv == NULL) || (ivLen >= CCWRAP_SEMIBLOCK), out, err = kCCParamError);
    if (iv == NULL) {
        iv = CCrfc3394_iv;
    }

    int ccrc = ccmode->init(ccmode, ctx, kekLen, kek);
    require_action(ccrc == CCERR_OK, out, err = kCCParamError);

    require_action(ccwrap_auth_encrypt_withiv(ccmode, ctx, rawKeyLen, rawKey, wrappedKeyLen, wrappedKey, iv) == CCERR_OK, out, err = kCCParamError);

    err = kCCSuccess;

out:
    if (err != kCCSuccess) {
        cc_clear(*wrappedKeyLen, wrappedKey);
        *wrappedKeyLen = 0;
    }
    ccecb_ctx_clear(ccmode->size, ctx);
    return err;
}

int
CCSymmetricKeyUnwrap(CCWrappingAlgorithm __unused algorithm,
					 const uint8_t *iv, const size_t ivLen,
					 const uint8_t *kek, size_t kekLen,
					 const uint8_t  *wrappedKey, size_t wrappedKeyLen,
                     uint8_t  *rawKey, size_t *rawKeyLen)
{
    CC_DEBUG_LOG("Entering\n");
    int err = kCCUnspecifiedError;
    const uint8_t *ivs[2] = { iv, CCrfc3394_iv };
    int i;
    size_t tmpRawKeyLen = 0;
    void *tmpRawKey = NULL;

    const struct ccmode_ecb *ccmode = getCipherMode(kCCAlgorithmAES128, kCCModeECB, kCCDecrypt).ecb;
    ccecb_ctx_decl(ccmode->size, ctx);

    require_action((kekLen == CCAES_KEY_SIZE_128 ||
                    kekLen == CCAES_KEY_SIZE_192 ||
                    kekLen == CCAES_KEY_SIZE_256),
                   out, err = kCCParamError);
    require_action(rawKeyLen && (*rawKeyLen >= ccwrap_unwrapped_size(wrappedKeyLen)), out, err = kCCParamError);

    /* due to rdar://problem/44095510, we tolerate a null IV */
    /* if the IV is not null, it must be long enough to satisfy corecrypto */
    require_action((iv == NULL) || (ivLen >= CCWRAP_SEMIBLOCK), out, err = kCCParamError);

    int ccrc = ccmode->init(ccmode, ctx, kekLen, kek);
    require_action(ccrc == CCERR_OK, out, err = kCCParamError);

    /* ccwrap_auth_decrypt_withiv clears the out buffer on failure */
    /* we need scratch space in case rawKey is an alias for wrappedKey */
    tmpRawKey = malloc(*rawKeyLen);
    require_action(tmpRawKey != NULL, out, err = kCCMemoryFailure);

    err = kCCDecodeError;

    for (i = 0; i < 2; i += 1) {
        if (ivs[i] == NULL) {
            continue;
        }

        /* ccwrap_auth_decrypt_withiv modifies this out parameter, so we need to set it on each iteration */
        tmpRawKeyLen = *rawKeyLen;

        if (ccwrap_auth_decrypt_withiv(ccmode, ctx, wrappedKeyLen, wrappedKey, &tmpRawKeyLen, tmpRawKey, ivs[i]) == CCERR_OK) {
            err = kCCSuccess;
            break;
        }
    }

    if (err == kCCSuccess) {
        memcpy(rawKey, tmpRawKey, tmpRawKeyLen);
    } else {
        cc_clear(tmpRawKeyLen, rawKey);
    }

    cc_clear(*rawKeyLen, tmpRawKey);
    free(tmpRawKey);

 out:
    *rawKeyLen = tmpRawKeyLen;
    ccecb_ctx_clear(ccmode->size, ctx);
    return err;
}

size_t
CCSymmetricWrappedSize(CCWrappingAlgorithm __unused algorithm, size_t rawKeyLen)
{
    CC_DEBUG_LOG("Entering\n");
    return ccwrap_wrapped_size(rawKeyLen);
}

size_t
CCSymmetricUnwrappedSize(CCWrappingAlgorithm __unused algorithm, size_t wrappedKeyLen)
{
    CC_DEBUG_LOG("Entering\n");
    return ccwrap_unwrapped_size(wrappedKeyLen);
}
