/*
 * Copyright (c) 2014-2015 Apple Inc. All rights reserved.
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

#ifndef HFS_EXTENTS_H_
#define HFS_EXTENTS_H_

#include <stdint.h>
#include <stdbool.h>

#include "hfs_format.h"

#if !HFS_EXTENTS_TEST && !HFS_ALLOC_TEST
#include "hfs_cnode.h"
#include "hfs.h"
#include "BTreesInternal.h"
#endif

typedef struct hfs_ext_iter {
	struct vnode		   *vp;			// If NULL, this is an xattr extent
	BTreeIterator			bt_iter;
	uint8_t					ndx;		// Index in group
	bool					last_in_fork;
	uint32_t				file_block;
	uint32_t				group_block_count;
	HFSPlusExtentRecord		group;
} hfs_ext_iter_t;

errno_t hfs_ext_find(vnode_t vp, off_t offset, hfs_ext_iter_t *iter);

errno_t hfs_ext_replace(hfsmount_t *hfsmp, vnode_t vp,
						uint32_t file_block,
						const HFSPlusExtentDescriptor *repl,
						int count,
						HFSPlusExtentRecord catalog_extents);

bool hfs_ext_iter_is_catalog_extents(hfs_ext_iter_t *iter);

static inline void hfs_ext_copy_rec(const HFSPlusExtentRecord src,
									HFSPlusExtentRecord dst)
{
	memcpy(dst, src, sizeof(HFSPlusExtentRecord));
}

static inline uint32_t hfs_ext_end(const HFSPlusExtentDescriptor *ext)
{
	return ext->startBlock + ext->blockCount;
}

#endif // HFS_EXTENTS_H_
