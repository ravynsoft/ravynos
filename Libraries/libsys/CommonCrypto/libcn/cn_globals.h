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

#ifndef CommonNumerics_cn_globals_h
#define CommonNumerics_cn_globals_h


/*
 *  cn_globals.h - CommonNumerics global DATA
 */

#include <asl.h>
#include "crc.h"
#include "basexx.h"
#include <CommonNumerics/CommonCRC.h>

#if __has_include(<os/alloc_once_private.h>)
#include <os/alloc_once_private.h>
#if defined(OS_ALLOC_ONCE_KEY_LIBCOMMONNUMERICS)
#define _LIBCOMMONNUMERICS_HAS_ALLOC_ONCE 1
#endif
#endif

#define CN_SUPPORTED_CRCS kCN_CRC_64_ECMA_182+1
#define CN_STANDARD_BASE_ENCODERS kCNEncodingBase16+1

struct cn_globals_s {
	// CommonCRC.c
    dispatch_once_t crc_init;
    crcInfo crcSelectionTab[CN_SUPPORTED_CRCS];
    dispatch_once_t basexx_init;
    BaseEncoderFrame encoderTab[CN_STANDARD_BASE_ENCODERS];
};
typedef struct cn_globals_s *cn_globals_t;

__attribute__((__pure__))
static inline cn_globals_t
_cn_globals(void) {
#if _LIBCOMMONNUMERICS_HAS_ALLOC_ONCE
	return (cn_globals_t) os_alloc_once(OS_ALLOC_ONCE_KEY_LIBCOMMONNUMERICS,
                                        sizeof(struct cn_globals_s),
                                        NULL);
#else
	static struct cn_globals_s storage;
	return &storage;
#endif
}

#endif
