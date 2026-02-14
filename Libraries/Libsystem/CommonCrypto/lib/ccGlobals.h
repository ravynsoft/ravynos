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

/*
 *  ccGlobals.h - CommonCrypto global DATA
 */

#ifndef CCGLOBALS_H
#define CCGLOBALS_H

#if defined(_MSC_VER) || defined(__ANDROID__)
#else
#include <os/log.h>
#include <os/assumes.h>
#endif

#include <corecrypto/ccdh.h>
#include <corecrypto/ccdigest.h>
#include "CommonCryptorPriv.h"
#include "basexx.h"

#include "crc.h"
#include <CommonNumerics/CommonCRC.h>
#include <CommonCrypto/CommonDigestSPI.h>

#if defined(_WIN32)
    #define _LIBCOMMONCRYPTO_HAS_ALLOC_ONCE 0
#elif __has_include(<os/alloc_once_private.h>)
        #include <os/alloc_once_private.h>
        #if defined(OS_ALLOC_ONCE_KEY_LIBCOMMONCRYPTO)
            #define _LIBCOMMONCRYPTO_HAS_ALLOC_ONCE 1
        #endif
#else
    #define _LIBCOMMONCRYPTO_HAS_ALLOC_ONCE 0
#endif

#define CN_SUPPORTED_CRCS kCN_CRC_64_ECMA_182+1
#define CN_STANDARD_BASE_ENCODERS kCNEncodingBase16+1

#define  CC_MAX_N_DIGESTS (kCCDigestMax)

struct cc_globals_s {
    crcInfo crcSelectionTab[CN_SUPPORTED_CRCS]; // CommonCRC.c
    BaseEncoderFrame encoderTab[CN_STANDARD_BASE_ENCODERS];
	const struct ccdigest_info *digest_info[CC_MAX_N_DIGESTS];// CommonDigest.c
};

typedef struct cc_globals_s *cc_globals_t;
void init_globals(void *g);

__attribute__((__pure__))
static inline cc_globals_t
_cc_globals(void) {
#if _LIBCOMMONCRYPTO_HAS_ALLOC_ONCE
    cc_globals_t globals =  (cc_globals_t) os_alloc_once(OS_ALLOC_ONCE_KEY_LIBCOMMONCRYPTO,
                                        sizeof(struct cc_globals_s),
                                        init_globals);
   if(OS_EXPECT(globals==NULL, 0)){
        struct _os_alloc_once_s *slot = &_os_alloc_once_table[OS_ALLOC_ONCE_KEY_LIBCOMMONCRYPTO-1];
        os_log_fault(OS_LOG_DEFAULT, "slot=%p once=%li, ptr=%p", slot, slot->once, slot->ptr);
        slot = &_os_alloc_once_table[OS_ALLOC_ONCE_KEY_LIBCOMMONCRYPTO];
        os_log_fault(OS_LOG_DEFAULT, "slot=%p once=%li, ptr=%p", slot, slot->once, slot->ptr);
        slot = &_os_alloc_once_table[OS_ALLOC_ONCE_KEY_LIBCOMMONCRYPTO+1];
        os_log_fault(OS_LOG_DEFAULT, "slot=%p once=%li, ptr=%p", slot, slot->once, slot->ptr);

        os_crash("output of os_alloc_once() is NULL");
    }
    return globals;
#else
    extern dispatch_once_t cc_globals_init;
    extern struct cc_globals_s cc_globals_storage;    
    cc_dispatch_once(&cc_globals_init, &cc_globals_storage, init_globals);
    return &cc_globals_storage;
#endif
}

#endif /* CCGLOBALS_H */
