
#include "testbyteBuffer.h"
#include "testmore.h"
#include "capabilities.h"

#if (CCRSA == 0)
entryPoint(CommonRSA,"RSA Cryptography")
#else

#include <CommonCrypto/CommonBigNum.h>
#include <CommonCrypto/CommonRSACryptor.h>
#include <CommonCrypto/CommonRSACryptorSPI.h>
#include <corecrypto/cczp.h>
#include <corecrypto/ccrsa.h>

// static int kTestTestCount = 23;
#define MAXKEYSPACE 512


static int saneKeySize(CCRSACryptorRef key) {
    size_t keySize = CCRSAGetKeySize(key);
    return (keySize < 512 || keySize > 4096);
}

static int roundTripCrypt(CCRSACryptorRef pubKey, CCRSACryptorRef privKey) {
    size_t keySizeBytes = (CCRSAGetKeySize(pubKey)+7)/8;
    char clear[keySizeBytes], cipher[keySizeBytes], decrypted[keySizeBytes];
    cc_clear(keySizeBytes, clear);
    size_t moved = keySizeBytes;
    
    ok(CCRSACryptorCrypt(pubKey, clear, keySizeBytes, cipher, &moved) == 0, "RSA Raw Crypt");
    ok(moved == keySizeBytes, "crypted keySizeBytes");
    ok(CCRSACryptorCrypt(privKey, cipher, keySizeBytes, decrypted, &moved) == 0, "RSA Raw Crypt");
    ok(moved == keySizeBytes, "crypted keySizeBytes");
    
    int rc = memcmp(clear, decrypted, moved);
    ok(rc == 0, "Results are what we started with");
    return rc;
}

static int wrapUnwrap(CCRSACryptorRef publicKey, CCRSACryptorRef privateKey, CCAsymmetricPadding padding) {
    size_t keySizeBytes = CCRSAGetKeySize(publicKey)/8+8;
    byteBuffer keydata = hexStringToBytes("000102030405060708090a0b0c0d0e0f");
    byteBuffer decryptedKey = mallocByteBuffer(keySizeBytes);
    byteBuffer encryptedKey = mallocByteBuffer(keySizeBytes);

	CCCryptorStatus retval;
    int status = 1;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    ok((retval = CCRSACryptorEncrypt(publicKey, padding, keydata->bytes, keydata->len,
                                     encryptedKey->bytes, &encryptedKey->len,
                                     "abcd", 4, kCCDigestSHA1)) == kCCSuccess, "Wrap Key Data with RSA Encryption - ccPKCS1Padding");
    if(retval) goto errout;

    ok((retval = CCRSACryptorDecrypt(privateKey, padding, encryptedKey->bytes, encryptedKey->len,
                                     decryptedKey->bytes, &decryptedKey->len,
                                     "abcd", 4, kCCDigestSHA1)) == kCCSuccess,
       "Unwrap Key Data with RSA Encryption - ccPKCS1Padding");
#pragma clang diagnostic pop
    if(retval) goto errout;
    
    ok(bytesAreEqual(decryptedKey, keydata), "Round Trip ccPKCS1Padding");
    if(bytesAreEqual(decryptedKey, keydata)) status = 0;
errout:
    free(keydata);
    free(decryptedKey);
    free(encryptedKey);
    return status;
}

static int sign_verify(CCRSACryptorRef publicKey, CCRSACryptorRef privateKey, CCAsymmetricPadding padding, CCDigestAlgorithm digest)
{
	CCCryptorStatus retval;
    int status = 1;
    byteBuffer signature = mallocByteBuffer(MAXKEYSPACE*2);
    byteBuffer hash = hexStringToBytes("000102030405060708090a0b0c0d0e0f000102030405060708090a0b0c0d0e0f000102030405060708090a0b0c0d0e0f000102030405060708090a0b0c0d0e0f");
    
    retval = CCRSACryptorSign(privateKey, padding,
                              hash->bytes, CCDigestGetOutputSize(digest),
                              digest, 16,
                              signature->bytes, &signature->len);
    
    ok(retval == kCCSuccess, "RSA Signing");
    if(retval) goto errout;
    
    retval = CCRSACryptorVerify(publicKey, padding,
                                hash->bytes, CCDigestGetOutputSize(digest),
                                digest, 16,
                                signature->bytes, signature->len);
    ok(retval == 0, "RSA Verifying");
    if(retval) goto errout;

    retval = CCRSACryptorVerify(publicKey, padding,
                                hash->bytes, CCDigestGetOutputSize(digest),
                                kCCDigestNone, 16,
                                signature->bytes, signature->len);
    ok(retval == kCCParamError, "RSA wrong digest test");
    if(retval!= kCCParamError) goto errout;
    
    retval = CCRSACryptorVerify(publicKey, padding,
                                hash->bytes, CCDigestGetOutputSize(digest),
                                kCCDigestSHA384, 16,
                                signature->bytes, signature->len);
    ok(retval == kCCDecodeError, "RSA wrong digest test");
    if(retval!= kCCDecodeError) goto errout;


    status = 0;
    
errout:
    free(signature);
    free(hash);
    return status;
}

#define TMPBUFSIZ 4096
static int export_import(CCRSACryptorRef publicKey, CCRSACryptorRef privateKey)
{
    CCRSACryptorRef publicKey2=NULL;
    CCRSACryptorRef privateKey2=NULL;
    int status = 0;

    byteBuffer tmp=mallocByteBuffer(TMPBUFSIZ);

    ok_status(status|=CCRSACryptorExport(publicKey, tmp->bytes, &tmp->len),"RSA Export Public Key");
    ok_status(status|=CCRSACryptorImport(tmp->bytes, tmp->len, &publicKey2),"RSA Import Public Key");
    if(status) {
        goto errOut;
    }

	tmp->len = TMPBUFSIZ;
    ok_status(status|=CCRSACryptorExport(privateKey, tmp->bytes, &tmp->len), "RSA Export Private Key");
    ok_status(status|=CCRSACryptorImport(tmp->bytes, tmp->len, &privateKey2),"RSA Import Private Key");
    if(status) {
        goto errOut;
    }

    ok_status(status|=saneKeySize(publicKey2), "Keysize is realistic");
    ok_status(status|=saneKeySize(privateKey2), "Keysize is realistic");
    if(status) {
        printf("%d keysize\n", CCRSAGetKeySize(publicKey2));
    }
    ok_status(status|=roundTripCrypt(publicKey, privateKey2), "import private works with original public");
    ok_status(status|=roundTripCrypt(publicKey2, privateKey), "import public works with original private");
    
errOut:
    free(tmp);
    CCRSACryptorRelease(publicKey2);
    CCRSACryptorRelease(privateKey2);
    return status;

}

static int compute_crt_components(CCRSACryptorRef rsaKey, void *dp, size_t *np, void *dq, size_t *nq, void *qinv, size_t *nqinv)
{
    size_t nbytes = (CCRSAGetKeySize(rsaKey)+7)/8;
    size_t modulus_sz = nbytes;
    uint8_t modulus_be[modulus_sz]; //be=big endian
    
    size_t d_sz = nbytes;
    uint8_t d_be[d_sz];
    
    size_t p_sz = nbytes;
    uint8_t p_be[p_sz]; //it is longer than needed, but it is fine
    
    size_t q_sz = nbytes;
    uint8_t q_be[q_sz];
    
    ok_or_fail(CCRSAGetKeyComponents(rsaKey, modulus_be, &modulus_sz, d_be, &d_sz, p_be, &p_sz, q_be, &q_sz)== kCCSuccess, "getting RSA key components") ;
    
    // m, p, q, e
    size_t n = ccn_nof(CCRSAGetKeySize(rsaKey));
    cc_unit modulus[n];
    ccn_read_uint(n, modulus, modulus_sz, modulus_be);
    
    *np = ccn_nof_size(p_sz);
    cc_unit p[*np];
    ccn_read_uint(*np, p, p_sz, p_be);
    
    *nq = ccn_nof_size(q_sz);
    ok_or_fail(*np>=*nq, "np must be larger than nq");
   
    cc_unit q[*np]; //set length to np not to nq
    ccn_read_uint(*np, q, q_sz, q_be);
    
    cc_unit d[n];
    ccn_read_uint(n, d, d_sz, d_be);
    
    p[0]--; q[0]--;
    ok_or_fail(ccn_mod(n, dp, n, d, *np, p)==0, "computing dp");
    ok_or_fail(ccn_mod(n, dq, n, d, *np, q)==0, "computing dq");
    p[0]++; q[0]++;
    
    cczp_decl_n(*np, zp);
    CCZP_N(zp) = *np;
    ccn_set(*np, CCZP_PRIME(zp), p);
    ok_or_fail(cczp_inv(zp, qinv, q)==0, "computing qinv");
    *nqinv = cczp_n(zp);
    
    return 1;
}

static int CCRSAGetCRTComponentsTest(CCRSACryptorRef rsaKey)
{
    size_t dp_size, dq_size, qinv_size;
    
    ok_or_fail(CCRSAGetCRTComponentsSizes(rsaKey, &dp_size, &dq_size, &qinv_size)==kCCSuccess, "getting CRT components sizes");

    uint8_t dp[dp_size];
    uint8_t dq[dq_size];
    uint8_t qinv[qinv_size];
    
    CCCryptorStatus rc = CCRSAGetCRTComponents(rsaKey, dp, dp_size, dq, dq_size, qinv, qinv_size);
    ok_or_fail(rc==kCCSuccess, "getting CRT parameters");
    
    //- compute crt parameters
    size_t keybits = CCRSAGetKeySize(rsaKey); //should probably be CCRSAGetKeyNBits
    size_t n =  ccn_nof(keybits); //should probaly be ccn_nof_bits()
    cc_unit dp2[n]; //larger than needed, but thats fine
    cc_unit dq2[n];
    cc_unit qinv2[n];
    
    size_t np, nq, nqinv;
    compute_crt_components(rsaKey, dp2, &np, dq2, &nq, qinv2, &nqinv);
    
    //- compare computed crt parametrs with those received from CCRSAGetCRTComponents()
    cc_unit dp3[n];
    cc_unit dq3[n];
    cc_unit qinv3[n];
    ccn_read_uint(n, dp3, dp_size, dp);
    ccn_read_uint(n, dq3, dq_size, dq);
    ccn_read_uint(n, qinv3, qinv_size, qinv);
    
    ok_or_fail(ccn_cmp(ccn_nof_size(dp_size), dp2, dp3)==0, "compring received dp with computed dp");
    ok_or_fail(ccn_cmp(ccn_nof_size(dq_size), dq2, dq3)==0, "compring received dq with computed dq");
    ok_or_fail(ccn_cmp(ccn_nof_size(qinv_size), qinv2, qinv3)==0, "compring received qinv with computed qinv");
    
    return 1;
}


static int
RSAStdGenTest(size_t keysize, uint32_t exponent)
{
    CCRSACryptorRef publicKey, privateKey, publicKeyClone;
	CCCryptorStatus retval;
    int status = -1;
    ok((retval = CCRSACryptorGeneratePair(keysize, exponent, &publicKey, &privateKey)) == kCCSuccess, "Generated Key Pairs");
    if(retval != kCCSuccess) return -1;
    ok((status = saneKeySize(publicKey)) == 0, "Keysize is realistic");
    ok((status = saneKeySize(privateKey)) == 0, "Keysize is realistic");
    ok((status = roundTripCrypt(publicKey, privateKey)) == 0, "Can perform round-trip encryption");
    ok((publicKeyClone = CCRSACryptorCreatePublicKeyFromPrivateKey(privateKey)) != NULL, "Can make public key from private key");
    ok((status = roundTripCrypt(publicKeyClone, privateKey)) == 0, "Can perform round-trip encryption with cloned public key");
    ok((status = wrapUnwrap(publicKey, privateKey, ccPKCS1Padding)) == 0, "Can perform round-trip PKCS1 wrap/unwrap");
    ok((status = wrapUnwrap(publicKey, privateKey, ccOAEPPadding)) == 0, "Can perform round-trip OAEP wrap/unwrap");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    ok((status = sign_verify(publicKey, privateKey, ccPKCS1Padding, kCCDigestSHA1)) == 0, "Can perform round-trip ccPKCS1Padding sign/verify");
#pragma clang diagnostic pop
    ok((status = sign_verify(publicKey, privateKey, ccRSAPSSPadding, kCCDigestSHA256)) == 0, "Can perform round-trip ccRSAPSSPadding sign/verify");
    ok((status = export_import(publicKey, privateKey)) == 0, "Can perform round-trip import/export");
    CCRSAGetCRTComponentsTest(privateKey);
    
    CCRSACryptorRelease(publicKeyClone);
    CCRSACryptorRelease(publicKey);
    CCRSACryptorRelease(privateKey);
    return status;
}

static const uint8_t kAirTunesRSAPublicKey[] =
{
    0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01, 0x00, 0xE7, 0xD7, 0x44, 0xF2, 0xA2, 0xE2, 0x78,
    0x8B, 0x6C, 0x1F, 0x55, 0xA0, 0x8E, 0xB7, 0x05, 0x44, 0xA8, 0xFA, 0x79, 0x45, 0xAA, 0x8B, 0xE6,
    0xC6, 0x2C, 0xE5, 0xF5, 0x1C, 0xBD, 0xD4, 0xDC, 0x68, 0x42, 0xFE, 0x3D, 0x10, 0x83, 0xDD, 0x2E,
    0xDE, 0xC1, 0xBF, 0xD4, 0x25, 0x2D, 0xC0, 0x2E, 0x6F, 0x39, 0x8B, 0xDF, 0x0E, 0x61, 0x48, 0xEA,
    0x84, 0x85, 0x5E, 0x2E, 0x44, 0x2D, 0xA6, 0xD6, 0x26, 0x64, 0xF6, 0x74, 0xA1, 0xF3, 0x04, 0x92,
    0x9A, 0xDE, 0x4F, 0x68, 0x93, 0xEF, 0x2D, 0xF6, 0xE7, 0x11, 0xA8, 0xC7, 0x7A, 0x0D, 0x91, 0xC9,
    0xD9, 0x80, 0x82, 0x2E, 0x50, 0xD1, 0x29, 0x22, 0xAF, 0xEA, 0x40, 0xEA, 0x9F, 0x0E, 0x14, 0xC0,
    0xF7, 0x69, 0x38, 0xC5, 0xF3, 0x88, 0x2F, 0xC0, 0x32, 0x3D, 0xD9, 0xFE, 0x55, 0x15, 0x5F, 0x51,
    0xBB, 0x59, 0x21, 0xC2, 0x01, 0x62, 0x9F, 0xD7, 0x33, 0x52, 0xD5, 0xE2, 0xEF, 0xAA, 0xBF, 0x9B,
    0xA0, 0x48, 0xD7, 0xB8, 0x13, 0xA2, 0xB6, 0x76, 0x7F, 0x6C, 0x3C, 0xCF, 0x1E, 0xB4, 0xCE, 0x67,
    0x3D, 0x03, 0x7B, 0x0D, 0x2E, 0xA3, 0x0C, 0x5F, 0xFF, 0xEB, 0x06, 0xF8, 0xD0, 0x8A, 0xDD, 0xE4,
    0x09, 0x57, 0x1A, 0x9C, 0x68, 0x9F, 0xEF, 0x10, 0x72, 0x88, 0x55, 0xDD, 0x8C, 0xFB, 0x9A, 0x8B,
    0xEF, 0x5C, 0x89, 0x43, 0xEF, 0x3B, 0x5F, 0xAA, 0x15, 0xDD, 0xE6, 0x98, 0xBE, 0xDD, 0xF3, 0x59,
    0x96, 0x03, 0xEB, 0x3E, 0x6F, 0x61, 0x37, 0x2B, 0xB6, 0x28, 0xF6, 0x55, 0x9F, 0x59, 0x9A, 0x78,
    0xBF, 0x50, 0x06, 0x87, 0xAA, 0x7F, 0x49, 0x76, 0xC0, 0x56, 0x2D, 0x41, 0x29, 0x56, 0xF8, 0x98,
    0x9E, 0x18, 0xA6, 0x35, 0x5B, 0xD8, 0x15, 0x97, 0x82, 0x5E, 0x0F, 0xC8, 0x75, 0x34, 0x3E, 0xC7,
    0x82, 0x11, 0x76, 0x25, 0xCD, 0xBF, 0x98, 0x44, 0x7B, 0x02, 0x03, 0x01, 0x00, 0x01, 0xD4, 0x9D
};

static const uint8_t k8192RSAPrivateKey[] =
{
#include "CommonRSA_key_8k_vect.inc"
};

// This function simulates CCRSACryptorCreateFromData and performs some test on the side.
static CCCryptorStatus CCRSACryptorCreateFromData_and_test_keys(CCRSAKeyType keyType,
                                  const uint8_t *modulus, size_t modulusLength,
                                  const uint8_t *publicExponent, size_t publicExponentLength,
                                  const uint8_t *p, size_t pLength, const uint8_t *q, size_t qLength,
                                                                CCRSACryptorRef *ref){
    
    CCCryptorStatus rv;

    rv=CCRSACryptorCreateFromData(keyType, modulus, modulusLength, publicExponent, publicExponentLength, p, pLength, q, qLength, ref);
    //because of the negative tests we perform, function must return if rv is not kCCSuccess
    if(rv != kCCSuccess)
        return rv;
    
    
    if(keyType==ccRSAKeyPrivate){
        CCRSACryptorRef priv = *ref;
        CCRSACryptorRef pub  = CCRSACryptorCreatePublicKeyFromPrivateKey(priv);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        rv = sign_verify(pub, priv, ccPKCS1Padding, kCCDigestSHA1);
#pragma clang diagnostic pop
        ok(rv==kCCSuccess, "sign/verify of created key");
        
        size_t dp_size, dq_size, qinv_size;
        rv=CCRSAGetCRTComponentsSizes(priv, &dp_size, &dq_size, &qinv_size); ok(rv==kCCSuccess, "getting CRT components sizes");
        uint8_t dp[dp_size], dq[dq_size], qinv[qinv_size];
        rv=CCRSAGetCRTComponents(priv, dp, dp_size, dq, dq_size, qinv, qinv_size); ok(rv==kCCSuccess, "getting CRT components");
        CCRSACryptorRelease(pub);
    }else{
        size_t modulusLength2, exponentLength2;
        assert(modulusLength!=0 && publicExponentLength!=0);
        modulusLength2 = modulusLength;
        exponentLength2 = publicExponentLength;
        uint8_t modulus2[modulusLength2], exponent2[exponentLength2];
        rv = CCRSAGetKeyComponents(*ref, modulus2, &modulusLength2, exponent2, &exponentLength2, NULL, NULL, NULL, NULL);
        ok_or_goto(rv==kCCSuccess, "reading public exponent components", errOut);
        is(modulusLength2, modulusLength, " modulus length mismatch");
        is(exponentLength2, publicExponentLength, " exponent length mismatch");
        if (modulus != NULL)
            ok_memcmp(modulus2, modulus, CC_MIN(modulusLength2,modulusLength), "modulus read mismatch");
        if (publicExponent != NULL)
            ok_memcmp(exponent2, publicExponent, CC_MIN(exponentLength2,publicExponentLength), "public exponent read mismatch");
    }
errOut:
    //this type case is not the best practice, but at this point we don't have access to the internls of
    //the CCRSACryptorRef
    if(rv!=kCCSuccess){
        fprintf(stderr, "\n\n%s key\n", keyType==ccRSAKeyPrivate?"private":(keyType==ccRSAKeyPublic?"public":"wrong type"));
        ccrsa_dump_full_key((struct ccrsa_full_ctx *)(*ref));
    }
    return rv;
    
}

#define MOD_SIZE 2048/8
#define ok_and_release(cond, msg) ok(cond, msg); if(rc==kCCSuccess) CCRSACryptorRelease(cryptor)
static int  CCRSACryptorCreateFromData_tests()
{
    unsigned e = 65537;
    CCRSACryptorRef pub;
    CCRSACryptorRef priv;
    CCCryptorStatus rc;
    
    rc=CCRSACryptorGeneratePair(MOD_SIZE*8, e, &pub, &priv); ok_or_fail(rc==kCCSuccess, "generatin RSA key pairs");
    
    uint8_t mod[MOD_SIZE];
    uint8_t priv_exp[MOD_SIZE];
    uint8_t pub_exp[MOD_SIZE];
    uint8_t p1[MOD_SIZE];
    uint8_t p2[MOD_SIZE];
    size_t mod_sz, p1_sz, p2_sz, priv_exp_sz, pub_exp_sz;
    pub_exp_sz=priv_exp_sz=mod_sz=MOD_SIZE;
    p1_sz=p2_sz=MOD_SIZE;
    
    CCRSAGetKeyComponents(pub, mod, &mod_sz, pub_exp, &pub_exp_sz, p1, &p1_sz, p2, &p2_sz); ok_or_fail(rc==kCCSuccess, "getting public key components");
    CCRSAGetKeyComponents(priv, mod, &mod_sz, priv_exp, &priv_exp_sz, p1, &p1_sz, p2, &p2_sz); ok_or_fail(rc==kCCSuccess, "getting private key components");
    
    CCRSACryptorRef cryptor;
    uint8_t *nul = NULL;
    
    diag("negative tests, expect 6 \"AssertMacros\"");
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, mod, mod_sz  , pub_exp, pub_exp_sz, p1, p1_sz, p2, p2_sz, &cryptor); ok_and_release(rc==kCCSuccess, "RSA keygen consistency test");
    
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, mod, mod_sz, pub_exp, pub_exp_sz, p1,      0, p2,  p2_sz, &cryptor); ok_and_release(rc!=kCCSuccess, "p1 size");
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, mod, mod_sz, pub_exp, pub_exp_sz, p1,  p1_sz, p2,      0, &cryptor); ok_and_release(rc!=kCCSuccess, "p2 size");
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, mod, mod_sz, pub_exp, pub_exp_sz, nul, p1_sz, p2,  p2_sz, &cryptor); ok_and_release(rc!=kCCSuccess, "p1 NULL");
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, mod, mod_sz, pub_exp, pub_exp_sz, p1,  p1_sz, nul, p2_sz, &cryptor); ok_and_release(rc!=kCCSuccess, "p2 NULL");
    
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, mod, mod_sz/2, pub_exp, pub_exp_sz, p1, p1_sz, p2, p2_sz,   &cryptor); ok_and_release(rc==kCCSuccess, "priv mod size"); //ccRSAKeyPrivate doesn't use modulus so no assert error
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, nul, mod_sz  , pub_exp, pub_exp_sz, p1, p1_sz, p2, p2_sz,   &cryptor); ok_and_release(rc==kCCSuccess, "priv mod NULL"); //ccRSAKeyPrivate doesn't use modulus so no assert error
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, mod, mod_sz  , pub_exp, pub_exp_sz, p1, p1_sz, p1, p1_sz,   &cryptor); ok_and_release(rc!=kCCSuccess, "Duplicate prime not allowed");
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, mod, mod_sz  , pub_exp, pub_exp_sz, p1, p1_sz, p1, p1_sz-1, &cryptor); ok_and_release(rc!=kCCSuccess, "bad prime");

    
    //doesn't generate assert
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPublic, mod, 0       , pub_exp, 1, p1, p1_sz, p2, p2_sz, &cryptor); ok_and_release(rc!=kCCSuccess, "public mod size");
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPublic, nul, mod_sz  , pub_exp, 1, p1, p1_sz, p2, p2_sz, &cryptor); ok_and_release(rc!=kCCSuccess, "public mod NULL");
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPublic, mod, mod_sz  , nul    , 1, p1, p1_sz, p2, p2_sz, &cryptor); ok_and_release(rc!=kCCSuccess, "public exp NULL");
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPublic, mod, mod_sz  , pub_exp, 0, p1, p1_sz, p2, p2_sz, &cryptor); ok_and_release(rc!=kCCSuccess, "public exp size");

    // Test to ensure that first prime passed in < second prime passed in works.
    rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, mod, mod_sz  , pub_exp, pub_exp_sz, p2, p2_sz, p1, p1_sz, &cryptor); ok_and_release(rc==kCCSuccess, "Accept q<p, assuming same length");

    CCRSACryptorRelease(pub);
    CCRSACryptorRelease(priv);
    return 1; //ok in test
}

struct rsa_key_data {
    char *sm;
    char *se;
    char *sp1;
    char *sp2;
    int result;
} rsa_key_data[] = {
#include "CommonRSA_key_vects.inc"
};


static int  CCRSACryptorCreateFromData_KATtests()
{
    CCCryptorStatus rc;
    CCRSACryptorRef cryptor=NULL;
    struct rsa_key_data *v;
    
    byteBuffer p1 = NULL;
    byteBuffer p2 = NULL;
    byteBuffer pub_exp = NULL;
    byteBuffer m = NULL;
    
    for (v =rsa_key_data; v->sm!=NULL; v++){
        p1 = hexStringToBytes(v->sp1);
        p2 = hexStringToBytes(v->sp2);
        pub_exp = hexStringToBytes(v->se);
        m = hexStringToBytes(v->sm);

        rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPrivate, NULL, 0, pub_exp->bytes, pub_exp->len, p1->bytes , p1->len, p2->bytes, p2->len, &cryptor);
        if (cryptor != NULL) {
            CCRSACryptorRelease(cryptor);
            cryptor = NULL;
        }
        is(rc, v->result, "create priv key from data");

        rc=CCRSACryptorCreateFromData_and_test_keys(ccRSAKeyPublic, m->bytes, m->len, pub_exp->bytes, pub_exp->len, NULL , 0, NULL, 0, &cryptor);
        if (cryptor != NULL) {
            CCRSACryptorRelease(cryptor);
            cryptor = NULL;
        }
        is(rc, v->result, "create pub key from data");

        free(p1);
        free(p2);
        free(pub_exp);
        free(m);
    }

    return 1;
}



int CommonRSA (int __unused argc, char *const * __unused argv) {
    int verbose = 1;
    int stdgen = 1;
    size_t keystep = 512;

    plan_tests(574);
    CCRSACryptorCreateFromData_tests();
    CCRSACryptorCreateFromData_KATtests();

    if(stdgen) {
        if(verbose) diag("Starting to generate keys stepping %lu", keystep);
        for(size_t keysize = 1024; keysize < 4097; keysize+=keystep) {
            if(verbose) diag("Generating %lu bit keypair", keysize);
            ok(RSAStdGenTest(keysize, 65537) == 0, "Generate Standard RSA Key Pair");
        }
    } /* stdgen */

    CCRSACryptorRef key;
    is(CCRSACryptorImport(kAirTunesRSAPublicKey, sizeof(kAirTunesRSAPublicKey), &key), kCCSuccess, "Imported Airport Key");
    CCRSACryptorRelease(key);

    is(CCRSACryptorImport(k8192RSAPrivateKey, sizeof(k8192RSAPrivateKey), &key), kCCMemoryFailure, "Failed to import 8k key");
    CCRSACryptorRelease(key);

    return 0;
}

#endif /* CCRSA */

