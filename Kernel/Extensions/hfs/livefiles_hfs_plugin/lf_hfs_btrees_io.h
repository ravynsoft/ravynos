//
//  lf_hfs_btrees_io.h
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#ifndef lf_hfs_btrees_io_h
#define lf_hfs_btrees_io_h

#include <stdio.h>


#include "lf_hfs.h"
#include "lf_hfs_btrees_internal.h"

/* BTree accessor routines */
OSStatus SetBTreeBlockSize(FileReference vp, ByteCount blockSize,
                                  ItemCount minBlockCount);

OSStatus GetBTreeBlock(FileReference vp, uint64_t blockNum,
                              GetBlockOptions options, BlockDescriptor *block);

OSStatus ReleaseBTreeBlock(FileReference vp, BlockDescPtr blockPtr,
                                  ReleaseBlockOptions options);

OSStatus ExtendBTreeFile(FileReference vp, FSSize minEOF, FSSize maxEOF);

void ModifyBlockStart(FileReference vp, BlockDescPtr blockPtr);

int hfs_create_attr_btree(struct hfsmount *hfsmp, u_int32_t nodesize, u_int32_t nodecnt);

u_int16_t get_btree_nodesize(struct vnode *vp);

#endif /* lf_hfs_btrees_io_h */
