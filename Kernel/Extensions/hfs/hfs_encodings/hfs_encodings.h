/*
 * Copyright (c) 2000-2015 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 1997-2000 Apple Inc. All Rights Reserved
 */

#ifndef _HFS_ENCODINGS_H_
#define _HFS_ENCODINGS_H_

#if !TARGET_OS_IPHONE

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_UNSTABLE

#define CTL_HFS_NAMES { \
	{ 0, 0 }, \
	{ "encodingbias", CTLTYPE_INT }, \
}

__BEGIN_DECLS

/*
 * HFS Filename Encoding Converters Interface
 *
 * Private Interface for adding hfs filename
 * encoding converters. These are not needed
 * for HFS Plus volumes (since they already
 * have Unicode filenames).
 *
 * Used by HFS Encoding Converter Kernel Modules
 * to register their encoding conversion routines.
 */

typedef int (* hfs_to_unicode_func_t)(const uint8_t hfs_str[32], uint16_t *uni_str,
		u_int32_t maxCharLen, u_int32_t *usedCharLen);

typedef int (* unicode_to_hfs_func_t)(uint16_t *uni_str, u_int32_t unicodeChars,
        uint8_t hfs_str[32]);

int hfs_relconverter (u_int32_t encoding);
int hfs_getconverter(u_int32_t encoding, hfs_to_unicode_func_t *get_unicode,
					 unicode_to_hfs_func_t *get_hfsname);
int hfs_addconverter(int kmod_id, u_int32_t encoding,
					 hfs_to_unicode_func_t get_unicode,
					 unicode_to_hfs_func_t get_hfsname);
int hfs_remconverter(int kmod_id, u_int32_t encoding);

u_int32_t hfs_pickencoding(const u_int16_t *src, int len);
u_int32_t hfs_getencodingbias(void);
void hfs_setencodingbias(u_int32_t bias);
int mac_roman_to_utf8(const uint8_t hfs_str[32], uint32_t maxDstLen, uint32_t *actualDstLen,
					  unsigned char* dstStr);
int utf8_to_mac_roman(uint32_t srcLen, const unsigned char* srcStr, uint8_t dstStr[32]);
int mac_roman_to_unicode(const uint8_t hfs_str[32], uint16_t *uni_str, u_int32_t maxCharLen, u_int32_t *usedCharLen);
int unicode_to_mac_roman(uint16_t *uni_str, u_int32_t unicodeChars, uint8_t hfs_str[32]);

__END_DECLS

#endif /* __APPLE_API_UNSTABLE */

#endif // !TARGET_OS_IPHONE

#endif /* ! _HFS_ENCODINGS_H_ */
