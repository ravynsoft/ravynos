/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
#ifndef __HFS_ENDIAN_H__
#define __HFS_ENDIAN_H__

/*
 * hfs_endian.h
 *
 * This file prototypes endian swapping routines for the HFS/HFS Plus
 * volume format.
 */
#include <hfs/hfs_format.h>
#include <libkern/OSByteOrder.h>

/*********************/
/* BIG ENDIAN Macros */
/*********************/
#if BYTE_ORDER == BIG_ENDIAN

    /* HFS is always big endian, make swaps into no-ops */
    #define SWAP_BE16(__a) (__a)
    #define SWAP_BE32(__a) (__a)
    #define SWAP_BE64(__a) (__a)
    
    /* HFS is always big endian, no swapping needed */
    #define SWAP_HFSMDB(__a)
    #define SWAP_HFSPLUSVH(__a)

/************************/
/* LITTLE ENDIAN Macros */
/************************/
#elif BYTE_ORDER == LITTLE_ENDIAN

    /* HFS is always big endian, make swaps actually swap */
    #define SWAP_BE16(__a) 							OSSwapBigToHostInt16 (__a)
    #define SWAP_BE32(__a) 							OSSwapBigToHostInt32 (__a)
    #define SWAP_BE64(__a) 							OSSwapBigToHostInt64 (__a)
    
    #define SWAP_HFSMDB(__a)						hfs_swap_HFSMasterDirectoryBlock ((__a))
    #define SWAP_HFSPLUSVH(__a)						hfs_swap_HFSPlusVolumeHeader ((__a));

#else
#warning Unknown byte order
#error
#endif

#ifdef __cplusplus
extern "C" {
#endif

void hfs_swap_HFSMasterDirectoryBlock (void *buf);
void hfs_swap_HFSPlusVolumeHeader (void *buf);

#ifdef __cplusplus
}
#endif

#endif /* __HFS_FORMAT__ */
