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

#include "ccGlobals.h"
#include "ccdebug.h"
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonCryptorSPI.h>
#include "CommonCryptorPriv.h"
#include <corecrypto/ccrc4.h>

typedef struct current_rc4_key_st
{
	uint32_t x,y;
	uint32_t data[256];
} RC4_KEY;

#ifndef	NDEBUG
#define ASSERT(s)
#else
#define ASSERT(s)	assert(s)
#endif

void CC_RC4_set_key(void *ctx, int len, const unsigned char *data)
{
    CC_DEBUG_LOG("Entering\n");
    ASSERT(sizeof(RC4_KEY) == ccrc4_eay.size);
    ccrc4_eay.init(ctx, len, data);
}

void CC_RC4(void *ctx, unsigned long len, const unsigned char *indata,
            unsigned char *outdata)
{
    CC_DEBUG_LOG("Entering\n");
    ccrc4_eay.crypt(ctx, len, indata, outdata);
}
