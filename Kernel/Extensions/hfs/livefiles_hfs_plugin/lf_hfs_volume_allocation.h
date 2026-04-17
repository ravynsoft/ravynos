/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_volume_allocation.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 22/3/18.
 */

#ifndef lf_hfs_volume_allocation_h
#define lf_hfs_volume_allocation_h

#include "lf_hfs.h"

int hfs_init_summary (struct hfsmount *hfsmp);
u_int32_t ScanUnmapBlocks (struct hfsmount *hfsmp);
int hfs_isallocated(struct hfsmount *hfsmp, u_int32_t startingBlock, u_int32_t numBlocks);

#endif /* lf_hfs_volume_allocation_h */
