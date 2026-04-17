/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_endian.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 18/3/18.
 */

#ifndef lf_hfs_endian_h
#define lf_hfs_endian_h

#include <stdio.h>
#include "lf_hfs_btrees_internal.h"


/*********************/
/* BIG ENDIAN Macros */
/*********************/
#define SWAP_BE16(__a)                             OSSwapBigToHostInt16 (__a)
#define SWAP_BE32(__a)                             OSSwapBigToHostInt32 (__a)
#define SWAP_BE64(__a)                             OSSwapBigToHostInt64 (__a)


/*
 * Constants for the "unswap" argument to hfs_swap_BTNode:
 */
enum HFSBTSwapDirection {
    kSwapBTNodeBigToHost        =    0,
    kSwapBTNodeHostToBig        =    1,

    /*
     * kSwapBTNodeHeaderRecordOnly is used to swap just the header record
     * of a header node from big endian (on disk) to host endian (in memory).
     * It does not swap the node descriptor (forward/backward links, record
     * count, etc.).  It assumes the header record is at offset 0x000E.
     *
     * Since HFS Plus doesn't have fixed B-tree node sizes, we have to read
     * the header record to determine the actual node size for that tree
     * before we can set up the B-tree control block.  We read it initially
     * as 512 bytes, then re-read it once we know the correct node size.  Since
     * we may not have read the entire header node the first time, we can't
     * swap the record offsets, other records, or do most sanity checks.
     */
    kSwapBTNodeHeaderRecordOnly    =    3
};

int  hfs_swap_BTNode (BlockDescriptor *src, vnode_t vp, enum HFSBTSwapDirection direction, u_int8_t allow_empty_node);


#endif /* lf_hfs_endian_h */
