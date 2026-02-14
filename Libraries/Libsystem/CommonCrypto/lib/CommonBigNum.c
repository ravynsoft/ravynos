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

// #define COMMON_BIGNUM_FUNCTIONS

#include <stdlib.h>
#include <CommonCrypto/CommonBigNum.h>
#include "CommonBigNumPriv.h"
#include <CommonCrypto/CommonRandomSPI.h>
#include "ccdebug.h"
#include <corecrypto/ccz.h>
#include <corecrypto/ccn.h> /* For ccn_sizeof(). */
#include <corecrypto/cc_priv.h> /* For CC_LOAD32_BE. */

static void *
cc_alloc(void *ctx CC_UNUSED, size_t size) {
    return malloc(size);
}

static void
cc_free(void *ctx CC_UNUSED, size_t oldsize, void *p) {
    cc_clear(oldsize, p);
    free(p);
}

static void *
cc_realloc(void *ctx CC_UNUSED, size_t oldsize,
                 void *p, size_t newsize) {
    void *r = malloc(newsize);
    memcpy(r, p, oldsize);
    cc_clear(oldsize, p);
    free(p);
    return r;
}

static struct ccz_class ccz_c = {
	.ctx = 0,
	.ccz_alloc = cc_alloc,
	.ccz_realloc = cc_realloc,
	.ccz_free = cc_free
};

CCBigNumRef
CCCreateBigNum(CCStatus *status)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = malloc(ccz_size((struct ccz_class *)&ccz_c));
    if (status)
        *status = r ? kCCSuccess : kCCMemoryFailure;
    if (r)
        ccz_init((struct ccz_class *)&ccz_c, r);
    return (CCBigNumRef)r;
}

CCStatus
CCBigNumClear(CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)bn;
    ccz_zero(r);
    return kCCSuccess;
}

void
CCBigNumFree(CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)bn;
    ccz_free(r);
    free(r);
}

CCBigNumRef
CCBigNumCopy(CCStatus *status, const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    const ccz *s = (ccz *)bn;
    ccz *r = (ccz *)CCCreateBigNum(status);
    if (r)
        ccz_set(r, s);
    return (CCBigNumRef)r;
}

uint32_t
CCBigNumBitCount(const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    const ccz *s = (ccz *)bn;
    return (uint32_t) ccz_bitlen(s);
}

uint32_t
CCBigNumZeroLSBCount(const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    const ccz *s = (ccz *)bn;
    return (uint32_t) ccz_trailing_zeros(s);
}

uint32_t
CCBigNumByteCount(const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    const ccz *s = (ccz *)bn;
    return (uint32_t) ccz_write_uint_size(s);
}

CCBigNumRef
CCBigNumFromData(CCStatus *status, const void *s, size_t len)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)CCCreateBigNum(status);
    if (r) {
        ccz_read_uint(r, len, s);
    }
    return (CCBigNumRef)r;
}

size_t
CCBigNumToData(CCStatus *  __unused status, const CCBigNumRef bn, void *to)
{
    CC_DEBUG_LOG("Entering\n");
    const ccz *s = (ccz *)bn;
    size_t to_size = ccz_write_uint_size(s);
    ccz_write_uint(s, to_size, to);
    return to_size;
}

CCBigNumRef
CCBigNumFromHexString(CCStatus *status, const char *in)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)CCCreateBigNum(status);
    if (r) {
        if (ccz_read_radix(r, strlen(in), in, 16)) {
            ccz_zero(r);
            if (status)
                *status = kCCDecodeError;
            return NULL;
        }
    }
    return (CCBigNumRef)r;
}

char *
CCBigNumToHexString(CCStatus *  __unused status, const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    const ccz *s = (ccz *)bn;
    size_t to_size = ccz_write_radix_size(s, 16);
    char *to = malloc(to_size+1);
    ccz_write_radix(s, to_size, to, 16);
    to[to_size] = 0;
    return to;
}


CCBigNumRef
CCBigNumFromDecimalString(CCStatus *status, const char *in)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)CCCreateBigNum(status);
    if (r) {
        if (ccz_read_radix(r, strlen(in), in, 10)) {
            ccz_zero(r);
            if (status)
                *status = kCCDecodeError;
            return NULL;
        }
    }
    return (CCBigNumRef)r;
}

char *
CCBigNumToDecimalString(CCStatus * __unused status, const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    const ccz *s = (ccz *)bn;
    size_t to_size = ccz_write_radix_size(s, 10);
    char *to = malloc(to_size+1);
    ccz_write_radix(s, to_size, to, 10);
    to[to_size] = 0;
    return to;
}


int
CCBigNumCompare(const CCBigNumRef bn1, const CCBigNumRef bn2)
{
    CC_DEBUG_LOG("Entering\n");
    const ccz *s = (const ccz *)bn1;
    const ccz *t = (const ccz *)bn2;
	return ccz_cmp(s, t);
}

int
CCBigNumCompareI(const CCBigNumRef bn1, const uint32_t num)
{
    CC_DEBUG_LOG("Entering\n");
    const ccz *s = (const ccz *)bn1;
	return ccz_cmpi(s, num);
}

CCStatus
CCBigNumSetNegative(CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)bn;
    ccz_neg(r);
    return kCCSuccess;
}

CCStatus
CCBigNumSetI(CCBigNumRef bn, uint64_t num)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)bn;
    ccz_seti(r, num);
    return kCCSuccess;
}

uint32_t
CCBigNumGetI(CCStatus *status, const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    /* TODO: This could be done more efficiently if we pushed a ccz_readi and
       ccn_readi routine all the way down into corecrypto. */
    ccz *s = (ccz *)bn;
    uint32_t v = 0;
    if(ccz_write_int_size(s) > sizeof(v)) {
        *status = kCCOverflow;
        return 0;
    }

    uint8_t to[sizeof(v)];
    ccz_write_uint(s, sizeof(v), to);
    CC_LOAD32_BE(v, to);

    if (status)
        *status = kCCSuccess;
    return v;
}

CCBigNumRef
CCBigNumCreateRandom(CCStatus *status, int __unused bits, int top, int bottom)
{
    CC_DEBUG_LOG("Entering\n");
    struct ccrng_state *rng = ccDRBGGetRngState();
    ccz *r = (ccz *)CCCreateBigNum(status);
    if (r && top > 0) {
        do {
            ccz_random_bits(r, top, rng);
        } while(ccz_bitlen(r) - ccz_trailing_zeros(r) < (unsigned long) bottom);
    }
    return (CCBigNumRef)r;
}

CCStatus
CCBigNumAdd(CCBigNumRef res, const CCBigNumRef a, const CCBigNumRef b)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)res;
    ccz *s = (ccz *)a;
    ccz *t = (ccz *)b;
    ccz_add(r, s, t);
    return kCCSuccess;
}

CCStatus
CCBigNumAddI(CCBigNumRef res, const CCBigNumRef a, const uint32_t b)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)res;
    ccz *s = (ccz *)a;
    ccz_addi(r, s, b);
    return kCCSuccess;
}

CCStatus
CCBigNumSub(CCBigNumRef res, const CCBigNumRef a, const CCBigNumRef b)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)res;
    ccz *s = (ccz *)a;
    ccz *t = (ccz *)b;
    ccz_sub(r, s, t);
    return kCCSuccess;
}

CCStatus
CCBigNumSubI(CCBigNumRef res, const CCBigNumRef a, const uint32_t b)
{
    CC_DEBUG_LOG("Entering\n");
    ccz *r = (ccz *)res;
    ccz *s = (ccz *)a;
    ccz_subi(r, s, b);
    return kCCSuccess;
}

CCStatus
CCBigNumMul(CCBigNumRef res, const CCBigNumRef a, const CCBigNumRef b)
{
    CC_DEBUG_LOG("Entering\n");
    ccz_mul((ccz *)res, (ccz *)a, (ccz *)b);
    return kCCSuccess;
}

CCStatus
CCBigNumMulI(CCBigNumRef res, const CCBigNumRef a, const uint32_t b)
{
    CC_DEBUG_LOG("Entering\n");
    ccz_muli((ccz *)res, (ccz *)a, b);
    return kCCSuccess;
}

CCStatus
CCBigNumDiv(CCBigNumRef quotient, CCBigNumRef remainder, const CCBigNumRef a, const CCBigNumRef b)
{
    CC_DEBUG_LOG("Entering\n");
    ccz_divmod((ccz *)quotient, (ccz *)remainder, (ccz *)a, (ccz *)b);
    return kCCSuccess;
}

CCStatus
CCBigNumDiv2(CCBigNumRef res, const CCBigNumRef a)
{
    CC_DEBUG_LOG("Entering\n");
    ccz_div2((ccz *)res, (ccz *)a);
    return kCCSuccess;
}

CCStatus
CCBigNumMod(CCBigNumRef res, CCBigNumRef dividend, CCBigNumRef modulus)
{
    CC_DEBUG_LOG("Entering\n");
    ccz_mod((ccz *)res, (ccz *)dividend, (ccz *)modulus);
    return kCCSuccess;
}

CCStatus
CCBigNumModI(uint32_t *res, CCBigNumRef dividend, uint32_t modulus)
{
    CC_DEBUG_LOG("Entering\n");
    CCStatus status = 0;
    CCBigNumRef mod = NULL;
    CCBigNumRef r = CCCreateBigNum(&status);
    if(!r) goto err;
    mod = CCCreateBigNum(&status);
    if(!mod) goto err;
    status = CCBigNumSetI(mod, modulus);
    ccz_mod((ccz *)r, (ccz *) dividend, (ccz *)mod);

    *res = CCBigNumGetI(&status, r);
err:
    if(r) CCBigNumFree(r);
    if(mod) CCBigNumFree(mod);
    return status;
}

CCStatus
CCBigNumSquare(CCBigNumRef res, const CCBigNumRef a)
{
    CC_DEBUG_LOG("Entering\n");
    ccz_sqr((ccz *)res, (ccz *)a);
    return kCCSuccess;
}

CCStatus
CCBigNumGCD(CCBigNumRef res, const CCBigNumRef a, const CCBigNumRef b)
{
    CC_DEBUG_LOG("Entering\n");
    ccz_gcd((ccz *)res, (ccz *)a, (ccz *)b);
    return kCCSuccess;
}

CCStatus
CCBigNumLCM(CCBigNumRef res, const CCBigNumRef a, const CCBigNumRef b)
{
    CC_DEBUG_LOG("Entering\n");
    ccz_lcm((ccz *)res, (ccz *)a, (ccz *)b);
    return kCCSuccess;
}

CCStatus
CCBigNumMulMod(CCBigNumRef res, const CCBigNumRef a, const CCBigNumRef b, const CCBigNumRef modulus)
{
    CC_DEBUG_LOG("Entering\n");
	ccz_mulmod((ccz *)res, (ccz *)a, (ccz *)b, (ccz *)modulus);
    return kCCSuccess;
}

CCStatus
CCBigNumSquareMod(CCBigNumRef res, const CCBigNumRef a, const CCBigNumRef modulus)
{
    CC_DEBUG_LOG("Entering\n");
	ccz_sqrmod((ccz *)res, (ccz *)a, (ccz *)modulus);
    return kCCSuccess;
}

CCStatus
CCBigNumInverseMod(CCBigNumRef res, const CCBigNumRef a, const CCBigNumRef modulus)
{
    int status=0;
    CC_DEBUG_LOG("Entering\n");
	status=ccz_invmod((ccz *)res, (ccz *)a, (ccz *)modulus);
    return (status==0)?kCCSuccess:kCCParamError;
}

CCStatus
CCBigNumModExp(CCBigNumRef res, const CCBigNumRef a, const CCBigNumRef power, const CCBigNumRef modulus)
{
    CC_DEBUG_LOG("Entering\n");
	ccz_expmod((ccz *)res, (ccz *)a, (ccz *)power, (ccz *) modulus);
    return kCCSuccess;
}

CCStatus
CCBigNumLeftShift(CCBigNumRef res, const CCBigNumRef a, const uint32_t digits)
{
    CC_DEBUG_LOG("Entering\n");
    ccz_lsl((ccz *) res, (ccz *) a, digits);
    return kCCSuccess;
}

CCStatus
CCBigNumRightShift(CCBigNumRef res, const CCBigNumRef a, const uint32_t digits)
{
    CC_DEBUG_LOG("Entering\n");
    ccz_lsr((ccz *)res, (ccz *)a, digits);
    return kCCSuccess;
}

CCStatus
CCBigNumMontgomerySetup(CCBigNumRef  __unused num, uint32_t *  __unused rho)
{
    CC_DEBUG_LOG("Entering\n");
	return kCCUnimplemented; // ccLTCErr(mp_montgomery_setup(num, rho));
}

CCStatus
CCBigNumMontgomeryNormalization(CCBigNumRef __unused a, CCBigNumRef __unused b)
{
    CC_DEBUG_LOG("Entering\n");
	return kCCUnimplemented; // ccLTCErr(mp_montgomery_normalization(a, b));
}

CCStatus
CCBigNumMontgomeryReduce(CCBigNumRef __unused x, CCBigNumRef __unused n, uint32_t __unused rho)
{
    CC_DEBUG_LOG("Entering\n");
	return kCCUnimplemented; // ccLTCErr(mp_montgomery_reduce(x, n, rho));
}

bool
CCBigNumIsPrime(CCStatus *status, const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    if (status)
        *status = kCCSuccess;

    /* TODO: Figure out right number of rounds (or depth). */
    return ccz_is_prime((ccz *) bn, 16);
}

bool
CCBigNumIsOdd(CCStatus *status, const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    if (status)
        *status = kCCSuccess;

    return ccz_is_odd((ccz *) bn);
}

bool
CCBigNumIsZero(CCStatus *status, const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    if (status)
        *status = kCCSuccess;

    return ccz_is_zero((ccz *) bn);
}

bool
CCBigNumIsNegative(CCStatus *status, const CCBigNumRef bn)
{
    CC_DEBUG_LOG("Entering\n");
    if (status)
        *status = kCCSuccess;

    return ccz_is_negative((ccz *) bn);
}
