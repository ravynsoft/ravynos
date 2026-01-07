/* 
 * Copyright (c) 2011 Apple Computer, Inc. All Rights Reserved.
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

// #define COMMON_CMAC_FUNCTIONS

#define CC_CHANGEFUNCTION_28544056_cccmac_init 1

#include  <CommonCrypto/CommonCMACSPI.h>
#include "CommonCryptorPriv.h"
#include "ccdebug.h"

#include <corecrypto/cccmac.h>
#include <corecrypto/ccaes.h>

void CCAESCmac(const void *key,
               const uint8_t *data,
               size_t dataLength,			/* length of data in bytes */
               void *macOut)				/* MAC written here */
{
    cccmac_one_shot_generate(ccaes_cbc_encrypt_mode(),
                              CCAES_KEY_SIZE_128, key,
                              dataLength, data,
                              CCAES_BLOCK_SIZE, macOut);
}

struct CCCmacContext {
    cccmac_ctx_t ctxptr;
};

CCCmacContextPtr
CCAESCmacCreate(const void *key, size_t keyLength)
{
    // Allocations
    CCCmacContextPtr retval = (CCCmacContextPtr) malloc(sizeof(struct CCCmacContext));
    if(!retval) return NULL;

    const struct ccmode_cbc *cbc = ccaes_cbc_encrypt_mode();
    retval->ctxptr = malloc(cccmac_ctx_size(cbc));
    if(retval->ctxptr == NULL) {
        free(retval);
        return NULL;
    }

    // Initialization (key length check)
    if (key==NULL
        || cccmac_init(cbc, retval->ctxptr,
                    keyLength, key)!=0) {
        free(retval->ctxptr);
        free(retval);
        return NULL;
    }
    
    return retval;
}

void CCAESCmacUpdate(CCCmacContextPtr ctx, const void *data, size_t dataLength) {
    cccmac_update(ctx->ctxptr,dataLength,data);
}

void CCAESCmacFinal(CCCmacContextPtr ctx, void *macOut) {
    cccmac_final_generate(ctx->ctxptr, 16, macOut);
}

void CCAESCmacDestroy(CCCmacContextPtr ctx) {
    if(ctx) {
        free(ctx->ctxptr);
        free(ctx);
    }
}

size_t
CCAESCmacOutputSizeFromContext(CCCmacContextPtr ctx) {
    return cccmac_cbc(ctx->ctxptr)->block_size;
}

