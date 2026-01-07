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
 *  ccdebug.h - CommonCrypto debug macros
 *
 */

#if defined (_WIN32)
#define __unused
#endif

#if defined(COMMON_DIGEST_FUNCTIONS) || defined(COMMON_CMAC_FUNCTIONS) || defined(COMMON_GCM_FUNCTIONS) \
    || defined (COMMON_AESSHOEFLY_FUNCTIONS) || defined(COMMON_CASTSHOEFLY_FUNCTIONS) \
    || defined (COMMON_CRYPTOR_FUNCTIONS) || defined(COMMON_HMAC_FUNCTIONS) \
    || defined(COMMON_KEYDERIVATION_FUNCTIONS) || defined(COMMON_SYMMETRIC_KEYWRAP_FUNCTIONS) \
    || defined(COMMON_RSA_FUNCTIONS) || defined(COMMON_EC_FUNCTIONS) || defined(COMMON_DH_FUNCTIONS) \
    || defined(COMMON_BIGNUM_FUNCTIONS) || defined(COMMON_RANDOM_FUNCTIONS)

    #define DIAGNOSTIC
#endif


#ifdef DIAGNOSTIC
    #include <os/log.h>
    #define CC_DEBUG_LOG(lvl,fmt, ...)      os_log(OS_LOG_TYPE_DEBUG, __FUNCTION__ ## "  " ## fmt , __VA_ARGS__)
#else
    #define CC_DEBUG_LOG(...)  
#endif /* DEBUG */


