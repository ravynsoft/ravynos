/*
 * Copyright (c) 1999-2006 Apple Computer, Inc. All rights reserved.
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
	File:		smf.c

	Contains:	platform-dependent malloc/free
 
*/

#include "smf.h"
#include <libkern/libkern.h>

/*
 * IOFree requires the exact allocation size, so we store it in a
 * header prepended to each allocation.
 */
typedef struct {
    DWORD size;
} mm_hdr_t;

SMFAPI void mmInit( void )
{
	return;
}

SMFAPI MMPTR mmMalloc(DWORD request)
{
    DWORD total = request + sizeof(mm_hdr_t);
    mm_hdr_t *hdr = IOMalloc(total);

    if (hdr == NULL)
    {
        printf ("Couldn't allocate kernel memory!\n");
        return NULL;
    }
    hdr->size = total;
    return (MMPTR)(hdr + 1);
}

SMFAPI void mmFree(MMPTR ptrnum)
{
    if (ptrnum == NULL) return;
    mm_hdr_t *hdr = ((mm_hdr_t *)ptrnum) - 1;
    IOFree(hdr, hdr->size);
}

SMFAPI LPVOID mmGetPtr(MMPTR ptrnum)
{
	return (LPVOID)ptrnum;
}

SMFAPI void mmReturnPtr(__unused MMPTR ptrnum)
{
	/* nothing */
	return;
}

