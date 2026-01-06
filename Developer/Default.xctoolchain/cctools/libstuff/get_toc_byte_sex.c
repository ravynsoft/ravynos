/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
#include <stdlib.h>
#include <string.h>
#include <ar.h>
#ifndef AR_EFMT1
#define	AR_EFMT1	"#1/"		/* extended format #1 */
#endif
#include <mach-o/loader.h>
#include <stuff/bytesex.h>
#include <stuff/rnd.h>

/*
 * get_toc_byte_sex() guesses the byte sex of the table of contents of the
 * library mapped in at the address, addr, of size, size based on the first
 * object file's bytesex.  If it can't figure it out, because the library has
 * no object file members or is malformed it will return UNKNOWN_BYTE_SEX.
 */
__private_extern__
enum byte_sex
get_toc_byte_sex(
char *addr,
uint64_t size)
{
     uint32_t magic;
     uint32_t ar_name_size;
     struct ar_hdr *ar_hdr;
     char *p;

	ar_hdr = (struct ar_hdr *)(addr + SARMAG);

	p = addr + SARMAG + sizeof(struct ar_hdr) +
	    rnd(strtoul(ar_hdr->ar_size, NULL, 10), sizeof(short));
	while(p + sizeof(struct ar_hdr) + sizeof(uint32_t) < addr + size){
	    ar_hdr = (struct ar_hdr *)p;
	    if(strncmp(ar_hdr->ar_name, AR_EFMT1, sizeof(AR_EFMT1) - 1) == 0)
		ar_name_size = (uint32_t)
		    strtoul(ar_hdr->ar_name + sizeof(AR_EFMT1) - 1, NULL, 10);
	    else
		ar_name_size = 0;
	    p += sizeof(struct ar_hdr);
	    memcpy(&magic, p + ar_name_size, sizeof(uint32_t));
	    if(magic == MH_MAGIC || magic == MH_MAGIC_64)
		return(get_host_byte_sex());
	    else if(magic == SWAP_INT(MH_MAGIC) ||
		    magic == SWAP_INT(MH_MAGIC_64))
		return(get_host_byte_sex() == BIG_ENDIAN_BYTE_SEX ?
		       LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX);
	    p += rnd(strtoul(ar_hdr->ar_size, NULL, 10), sizeof(short));
	}
	return(UNKNOWN_BYTE_SEX);
}
