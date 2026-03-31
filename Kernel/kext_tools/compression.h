/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef __COMPRESSION_H__
#define __COMPRESSION_H__

#include <stdlib.h>

u_int32_t local_adler32(
    u_int8_t * buffer,
    int32_t    length);

int decompress_lzss(
    u_int8_t       * dst,
    u_int32_t        dstlen,
    u_int8_t * src,
    u_int32_t        srclen);

u_int8_t * compress_lzss(
    u_int8_t       * dst,
    u_int32_t        dstlen,
    u_int8_t * src,
    u_int32_t        srclen);

#endif /* __COMPRESSION_H__ */
