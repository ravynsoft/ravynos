//
//  CommonNistKeyDerivation.h
//  CommonCrypto
//
//  Created by Richard Murphy on 12/15/13.
//  Copyright (c) 2013 Platform Security. All rights reserved.
//

#ifndef CommonCrypto_CommonNistKeyDerivation_h
#define CommonCrypto_CommonNistKeyDerivation_h

#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigestSPI.h>

#if defined(_MSC_VER)
#include <availability.h>
#else
#include <os/availability.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
    @typedef     CCKDFParametersRef

    @abstract    Opaque reference to a CCKDFParameters object.

    @discussion  A CCKDFParameters reference may be initialized using one
                 of the CCKDFParametersCreate* functions declared below.
 */
typedef struct CCKDFParameters *CCKDFParametersRef;

/*!
    @enum       CCKDFAlgorithm
    @abstract   Key Derivation algorithms implemented by this module.

    @constant   kCCKDFAlgorithmPBKDF2_HMAC

    @constant   kCCKDFAlgorithmCTR_HMAC
    @constant   kCCKDFAlgorithmCTR_HMAC_FIXED
    @constant   kCCKDFAlgorithmFB_HMAC
    @constant   kCCKDFAlgorithmFB_HMAC_FIXED
 	@constant   kCCKDFAlgorithmDPIPE_HMAC
*/
enum {
    kCCKDFAlgorithmPBKDF2_HMAC = 0,
    kCCKDFAlgorithmCTR_HMAC,
    kCCKDFAlgorithmCTR_HMAC_FIXED,
    kCCKDFAlgorithmFB_HMAC,         // UNIMP
    kCCKDFAlgorithmFB_HMAC_FIXED,   // UNIMP
    kCCKDFAlgorithmDPIPE_HMAC,      // UNIMP
    kCCKDFAlgorithmHKDF,
    kCCKDFAlgorithmAnsiX963
};
typedef uint32_t CCKDFAlgorithm;

CCStatus
CCKeyDerivationHMac(CCKDFAlgorithm algorithm, CCDigestAlgorithm digest,
                    unsigned rounds, // ignored except for PBKDF
                    const void *keyDerivationKey, size_t keyDerivationKeyLen,
                    const void *label, size_t labelLen,
                    const void *context, size_t contextLen, // or FIXED buffer (label | context)
                    const void *iv, size_t ivLen,           // for FB
                    const void *salt, size_t saltLen,       // for PBKDF
                    void *derivedKey, size_t derivedKeyLen)
API_DEPRECATED_WITH_REPLACEMENT("CCDeriveKey", macos(10.10, 10.15), ios(8.0, 13.0));

/*!
    @function   CCKDFParametersCreatePbkdf2

    @abstract   Creates a CCKDFParameters object that will hold parameters for
                key derivation with PBKDF2 with an HMAC PRF.

    @param      params         A CCKDFParametersRef pointer.

    @param      rounds         Number of iterations.

    @param      salt           Salt.

    @param      saltLen        Length of salt.

    @result     Possible error returns are kCCParamError and kCCMemoryFailure.
*/

CCStatus
CCKDFParametersCreatePbkdf2(CCKDFParametersRef *params,
                            unsigned rounds, const void *salt, size_t saltLen)
API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));

/*!
    @function   CCKDFParametersCreateCtrHmac

    @abstract   Creates a CCKDFParameters object that will hold parameters for
                key derivation with NIST SP800-108 KDF in Counter Mode with an
                HMAC PRF.

    @param      params         A CCKDFParametersRef pointer.

    @param      label          Label to identify purpose of derived key.

    @param      labelLen       Length of label.

    @param      context        Data shared between entities.

    @param      contextLen     Length of context.

    @result     Possible error returns are kCCParamError and kCCMemoryFailure.
*/

CCStatus
CCKDFParametersCreateCtrHmac(CCKDFParametersRef *params,
                             const void *label, size_t labelLen,
                             const void *context, size_t contextLen)
API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));

/*!
    @function   CCKDFParametersCreateCtrHmacFixed

    @abstract   Creates a CCKDFParameters object that will hold parameters for
                key derivation with NIST SP800-108 KDF in Counter Mode with an
                HMAC PRF.

    @param      params         A CCKDFParametersRef pointer.

    @param      context        Data shared between entities.

    @param      contextLen     Length of context.

    @result     Possible error returns are kCCParamError and kCCMemoryFailure.
*/

CCStatus
CCKDFParametersCreateCtrHmacFixed(CCKDFParametersRef *params,
                                  const void *context, size_t contextLen)
API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));

/*!
    @function   CCKDFParametersCreateHkdf

    @abstract   Creates a CCKDFParameters object that will hold parameters for
                key derivation with HKDF as defined by RFC 5869.

    @param      params         A CCKDFParametersRef pointer.

    @param      salt           Salt.

    @param      saltLen        Length of salt.

    @param      context        Data shared between entities.

    @param      contextLen     Length of context.

    @result     Possible error returns are kCCParamError and kCCMemoryFailure.
*/

CCStatus
CCKDFParametersCreateHkdf(CCKDFParametersRef *params,
                          const void *salt, size_t saltLen,
                          const void *context, size_t contextLen)
API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));

/*!
    @function    CCKDFParametersCreateAnsiX963

    @abstract    Creates a CCKDFParameters object that will hold parameters for
                 key derivation with ANSI x9.63 KDF.

    @discussion  This implements the ANSI X9.63 Key Derivation Function as
                 defined by section 3.6.1 of <http://www.secg.org/sec1-v2.pdf>.

    @param       params            A CCKDFParametersRef pointer.

    @param       sharedinfo        Data shared between entities.

    @param       sharedinfoLen     Length of sharedinfo.

    @result      Possible error returns are kCCParamError and kCCMemoryFailure.
*/

CCStatus
CCKDFParametersCreateAnsiX963(CCKDFParametersRef *params,
                              const void *sharedinfo, size_t sharedinfoLen)
API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));

/*!
    @function   CCKDFParametersDestroy

    @abstract   Clear and release a CCKDFParametersRef.

    @param      params     A CCKDFParameters reference.
*/

void
CCKDFParametersDestroy(CCKDFParametersRef params)
API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));

/*!
    @function   CCDeriveKey

    @abstract   Generic key derivation function supporting multiple key
                derivation algorithms.

    @param      params               A CCKDFParametersRef with pointers to the
                                     chosen KDF's parameters.

    @param      digest               The digest algorithm to use.

    @param      keyDerivationKey     The input key material to derive from.

    @param      keyDerivationKeyLen  Length of the input key material.

    @param      derivedKey           Output buffer for the derived key.

    @param      derivedKeyLen        Desired length of the derived key.

    @result     Possible error returns are kCCParamError, kCCMemoryFailure, and
                kCCUnimplemented.
*/

CCStatus
CCDeriveKey(const CCKDFParametersRef params, CCDigestAlgorithm digest,
            const void *keyDerivationKey, size_t keyDerivationKeyLen,
            void *derivedKey, size_t derivedKeyLen)
API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));


#ifdef __cplusplus
}
#endif

#endif
