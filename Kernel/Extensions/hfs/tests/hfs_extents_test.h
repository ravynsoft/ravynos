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

#ifndef hfs_hfs_extents_test_h
#define hfs_hfs_extents_test_h

// Stuff that allow us to build hfs_extents.c for testing

#define KERNEL 1
#define __APPLE_API_PRIVATE 1
#define HFS_EXTENTS_TEST 1

#include <stdint.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <assert.h>
#include <unistd.h>
#include <sys/queue.h>

#include "../core/hfs_format.h"

#define VTOF(v)				(v)->ffork
#define VTOHFS(v)			(v)->mount
#define VNODE_IS_RSRC(v)	(v)->is_rsrc
#define VTOC(v)				(v)->cnode

struct BTreeHint{
	unsigned long			writeCount;
	u_int32_t				nodeNum;			// node the key was last seen in
	u_int16_t				index;				// index then key was last seen at
	u_int16_t				reserved1;
	u_int32_t				reserved2;
};
typedef struct BTreeHint BTreeHint;

typedef struct BTreeIterator {
	BTreeHint	hint;
	uint16_t	version;
	uint16_t	reserved;
	uint32_t	hitCount;			// Total number of leaf records hit
	uint32_t	maxLeafRecs;		// Max leaf records over iteration
	struct extent_group *group;
	HFSPlusExtentKey key;
} BTreeIterator;

typedef struct filefork {
	uint32_t ff_blocks, ff_unallocblocks;
	LIST_HEAD(extent_groups, extent_group) groups;
} filefork_t;

typedef struct cnode {
	uint32_t c_fileid;
	filefork_t *c_datafork;
} cnode_t;

typedef struct hfsmount {
	cnode_t *hfs_extents_cp;
	uint32_t blockSize;
} hfsmount_t;

typedef struct vnode {
	filefork_t *ffork;
	hfsmount_t *mount;
	bool is_rsrc;
	cnode_t *cnode;
} *vnode_t;

struct FSBufferDescriptor {
	void *		bufferAddress;
	uint32_t	itemSize;
	uint32_t	itemCount;
};
typedef struct FSBufferDescriptor FSBufferDescriptor;

static inline int32_t
BTSearchRecord		(__unused filefork_t					*filePtr,
					 __unused BTreeIterator				*searchIterator,
					 __unused FSBufferDescriptor			*record,
					 __unused u_int16_t					*recordLen,
					 __unused BTreeIterator				*resultIterator )
{
	return ENOTSUP;
}

/* Constants for HFS fork types */
enum {
	kHFSDataForkType = 0x0, 	/* data fork */
	kHFSResourceForkType = 0xff	/* resource fork */
};

static inline void *hfs_malloc(size_t size)
{
    return malloc(size);
}

static inline void hfs_free(void *ptr, __unused size_t size)
{
    return free(ptr);
}

static inline __attribute__((const))
uint64_t hfs_blk_to_bytes(uint32_t blk, uint32_t blk_size)
{
	return (uint64_t)blk * blk_size; 		// Avoid the overflow
}

int32_t BTDeleteRecord(filefork_t    *filePtr,
					   BTreeIterator *iterator);

#define HFS_ALLOC_ROLL_BACK			0x800	//Reallocate blocks that were just deallocated
typedef uint32_t hfs_block_alloc_flags_t;

typedef struct hfs_alloc_extra_args hfs_alloc_extra_args_t;

static inline errno_t hfs_block_alloc(__unused hfsmount_t *hfsmp,
									  __unused HFSPlusExtentDescriptor *extent,
									  __unused hfs_block_alloc_flags_t flags,
									  __unused hfs_alloc_extra_args_t *extra_args)
{
	return ENOTSUP;
}

#define BlockDeallocate(m, b, c, f)		(int16_t)0
#define BTFlushPath(ff)					(int32_t)0

static inline int hfs_flushvolumeheader(__unused struct hfsmount *hfsmp, 
										__unused int waitfor, 
										__unused int altflush)
{
	return 0;
}

#define hfs_mark_inconsistent(m, r)		(void)0

static inline errno_t MacToVFSError(errno_t err)
{
	return err;
}

struct hfs_ext_iter;

uint32_t hfs_total_blocks(const HFSPlusExtentDescriptor *ext, int count);
errno_t hfs_ext_iter_next_group(struct hfs_ext_iter *iter);
errno_t hfs_ext_iter_update(struct hfs_ext_iter *iter,
							HFSPlusExtentDescriptor *extents,
							int count,
							HFSPlusExtentRecord cat_extents);
errno_t hfs_ext_iter_check_group(struct hfs_ext_iter *iter);

static inline uint32_t ff_allocblocks(filefork_t *ff)
{
	return ff->ff_blocks - ff->ff_unallocblocks;
}

#endif
