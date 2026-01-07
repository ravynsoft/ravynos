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

CCCryptorStatus CCDesIsWeakKey( void *key, size_t length)
{
    CC_DEBUG_LOG("Entering\n");
    return ccdes_key_is_weak(key, length);
}

void CCDesSetOddParity(void *key, size_t Length)
{
    CC_DEBUG_LOG("Entering\n");
    ccdes_key_set_odd_parity(key, Length);
}

uint32_t CCDesCBCCksum(void *in, void *out, size_t length,
                       void *key, size_t keylen, void *ivec)
{
    CC_DEBUG_LOG("Entering\n");
    return ccdes_cbc_cksum(in, out, length, key, keylen, ivec);
}
