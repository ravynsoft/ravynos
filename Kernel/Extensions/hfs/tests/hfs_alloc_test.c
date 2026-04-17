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

//  Radar Component: HFS | X

#include <sys/param.h>

#include <sys/disk.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <libkern/OSByteOrder.h>
#include <hfs/hfs_format.h>

#include "test-utils.h"

// For panic
#include <mach/mach.h>

#include <stdio.h>

#define HFS_ALLOC_TEST 1
#define RANGELIST_TEST 1
#define KERNEL 1
#define HFS 1
#define DEBUG 1

#include "../core/rangelist.h"

#define SYSCTL_DECL(a)
#define SYSCTL_NODE(a, b, c, d, e, f)
#define SYSCTL_INT(a, b, c, d, e, f, g)

typedef int16_t OSErr;

typedef struct cnode {
	uint32_t c_blocks;
} cnode_t;

typedef struct journal {


} journal;

#define HFS_READ_ONLY             0x00001
#define HFS_METADATA_ZONE         0x00080
#define HFS_HAS_SPARSE_DEVICE     0x00400
#define HFS_UNMAP                0x200000
#define HFS_SUMMARY_TABLE        0x800000
#define HFS_CS                  0x1000000

#define SFL_BITMAP				0x0004

enum hfs_locktype {
	HFS_SHARED_LOCK = 1,
	HFS_EXCLUSIVE_LOCK = 2
};

typedef struct vnode {
} *vnode_t;

#define kMaxFreeExtents		10

typedef struct hfsmount {
	cnode_t	   *hfs_allocation_cp;
	uint32_t	blockSize;
	struct journal *jnl;
	uint64_t	hfs_logical_bytes;
	uint32_t	hfsPlusIOPosOffset;
	uint8_t		vcbVN[256];
	uint32_t	hfs_flags;
	int32_t		hfs_raw_dev;			/* device mounted */
	uint32_t	totalBlocks, allocLimit, freeBlocks, tentativeBlocks, lockedBlocks;
	uint32_t	sparseAllocation, nextAllocation;
	uint32_t	hfs_metazone_start, hfs_metazone_end;
	uint32_t	vcbFreeExtCnt;
	u_int32_t	hfs_freed_block_count;
	int16_t		vcbFlags;
	uint32_t	vcbVBMIOSize;
	vnode_t		hfs_allocation_vp;
	uint16_t 	vcbSigWord;
	HFSPlusExtentDescriptor vcbFreeExt[kMaxFreeExtents];
	uint8_t    *hfs_summary_table;
	uint32_t	hfs_summary_size;
	uint32_t	hfs_summary_bytes;	/* number of BYTES in summary table */
	struct rl_head hfs_reserved_ranges[2];
} hfsmount_t;

typedef hfsmount_t ExtendedVCB;

typedef struct _dk_cs_map {
	dk_extent_t	cm_extent;
	uint64_t	cm_bytes_mapped;
} _dk_cs_map_t;

struct jnl_trim_list {
	uint32_t	allocated_count;
	uint32_t	extent_count;
	dk_extent_t *extents;
};

typedef enum hfs_flush_mode {
	HFS_FLUSH_JOURNAL,              // Flush journal
	HFS_FLUSH_JOURNAL_META,         // Flush journal and metadata blocks
	HFS_FLUSH_FULL,                 // Flush journal and does a cache flush
	HFS_FLUSH_CACHE,                // Flush track cache to media
	HFS_FLUSH_BARRIER,              // Barrier-only flush to ensure write order
} hfs_flush_mode_t;

typedef bool Boolean;

int hfs_isallocated(struct hfsmount *hfsmp, uint32_t startingBlock,
					uint32_t numBlocks);

static int journal_trim_add_extent(__unused journal *jnl,
								   __unused uint64_t offset,
								   __unused uint64_t length)
{
	return 0;
}

static int journal_trim_remove_extent(__unused journal *jnl,
									  __unused uint64_t offset,
									  __unused uint64_t length)
{
	return 0;
}

static int journal_trim_extent_overlap(__unused journal *jnl,
									   __unused uint64_t offset,
									   __unused uint64_t length,
									   __unused uint64_t *end)
{
	return 0;
}

int
hfs_systemfile_lock(__unused struct hfsmount *hfsmp, __unused int flags, __unused enum hfs_locktype locktype)
{
	return 0;
}

void
hfs_systemfile_unlock(__unused struct hfsmount *hfsmp, __unused int flags)
{
	
}

typedef struct vfs_context * vfs_context_t;

#define VNOP_IOCTL(a, b, c, d, e)		((void)c, 0)

#define VCBTOHFS(x)		(x)

u_int32_t hfs_freeblks(struct hfsmount * hfsmp, __unused int wantreserve)
{
	return hfsmp->freeBlocks - hfsmp->lockedBlocks;
}

enum {
	noErr			= 0,
	dskFulErr		= -34,		/*disk full*/
	bdNamErr		= -37,		/*there may be no bad names in the final system!*/
	paramErr		= -50,		/*error in user parameter list*/
	memFullErr		= -108,		/*Not enough room in heap zone*/
	fileBoundsErr		= -1309,	/*file's EOF, offset, mark or size is too big*/
	kTECUsedFallbacksStatus	= -8783,
};

static void hfs_lock_mount(__unused struct hfsmount *hfsmp)
{
}
static void hfs_unlock_mount(__unused struct hfsmount *hfsmp)
{
}

OSErr BlockDeallocate (ExtendedVCB		*vcb,			//	Which volume to deallocate space on
					   u_int32_t		firstBlock,		//	First block in range to deallocate
					   u_int32_t		numBlocks, 		//	Number of contiguous blocks to deallocate
					   u_int32_t 		flags);

#define lck_spin_lock(x)		((void)0)
#define lck_spin_unlock(x)		((void)0)

static void HFS_UPDATE_NEXT_ALLOCATION(hfsmount_t *hfsmp, 
									   uint32_t new_nextAllocation)
{
	hfsmp->nextAllocation = new_nextAllocation;
}

static void MarkVCBDirty(ExtendedVCB *vcb)
{
	vcb->vcbFlags |= 0xFF00;
}

#define hfs_generate_volume_notifications(x)		((void)0)
#define REQUIRE_FILE_LOCK(a, b)		((void)0)
#define journal_modify_block_start(a, b)		(0)
#define journal_modify_block_end(a, b, c, d)	(0)

#define SWAP_BE32(x)		OSSwapBigToHostInt32(x)

typedef enum {
	HFS_INCONSISTENCY_DETECTED,

	// Used when unable to rollback an operation that failed
	HFS_ROLLBACK_FAILED,

	// Used when the latter part of an operation failed, but we chose not to roll back
	HFS_OP_INCOMPLETE,

	// Used when someone told us to force an fsck on next mount
	HFS_FSCK_FORCED,
} hfs_inconsistency_reason_t;

static void hfs_mark_inconsistent(__unused struct hfsmount *hfsmp,
								  __unused hfs_inconsistency_reason_t reason)
{
	assert(false);
}

static int journal_request_immediate_flush(__unused journal *jnl)
{
	return 0;
}

enum {
	// These are indices into the array below

	// Tentative ranges can be claimed back at any time
	HFS_TENTATIVE_BLOCKS	= 0,

	// Locked ranges cannot be claimed back, but the allocation
	// won't have been written to disk yet
	HFS_LOCKED_BLOCKS		= 1,
};

static inline __attribute__((const))
off_t hfs_blk_to_bytes(uint32_t blk, uint32_t blk_size)
{
	return (off_t)blk * blk_size; 		// Avoid the overflow
}

typedef unsigned char 		Str31[32];
#define EXTERN_API_C(x) extern x
typedef const unsigned char *	ConstUTF8Param;
#define CALLBACK_API_C(_type, _name)	_type ( * _name)
typedef struct vnode* FileReference;
typedef struct filefork FCB;
typedef int64_t daddr64_t;

#include "../core/FileMgrInternal.h"

/*
 * Map HFS Common errors (negative) to BSD error codes (positive).
 * Positive errors (ie BSD errors) are passed through unchanged.
 */
short MacToVFSError(OSErr err)
{
	if (err >= 0)
		return err;

	/* BSD/VFS internal errnos */
#if 0
	switch (err) {
		case ERESERVEDNAME: /* -8 */
			return err;
	}
#endif

	switch (err) {
		case dskFulErr:			/*    -34 */
		case btNoSpaceAvail:		/* -32733 */
			return ENOSPC;
		case fxOvFlErr:			/* -32750 */
			return EOVERFLOW;

		case btBadNode:			/* -32731 */
			return EIO;

		case memFullErr:		/*  -108 */
			return ENOMEM;		/*   +12 */

		case cmExists:			/* -32718 */
		case btExists:			/* -32734 */
			return EEXIST;		/*    +17 */

		case cmNotFound:		/* -32719 */
		case btNotFound:		/* -32735 */
			return ENOENT;		/*     28 */

		case cmNotEmpty:		/* -32717 */
			return ENOTEMPTY;	/*     66 */

		case cmFThdDirErr:		/* -32714 */
			return EISDIR;		/*     21 */

		case fxRangeErr:		/* -32751 */
			return ERANGE;

		case bdNamErr:			/*   -37 */
			return ENAMETOOLONG;	/*    63 */

		case paramErr:			/*   -50 */
		case fileBoundsErr:		/* -1309 */
			return EINVAL;		/*   +22 */

#if 0
		case fsBTBadNodeSize:
			return ENXIO;
#endif

		default:
			return EIO;		/*   +5 */
	}
}

#define min(a, b)	\
	({ typeof(a) a_ = (a); typeof(b) b_ = (b); a_ < b_ ? a_ : b_; })

errno_t hfs_find_free_extents(struct hfsmount *hfsmp,
							  void (*callback)(void *data, off_t), void *callback_arg);

static void hfs_journal_lock(__unused hfsmount_t *hfsmp)
{
}

static errno_t hfs_flush(__unused hfsmount_t *hfsmp, __unused hfs_flush_mode_t x)
{
	return 0;
}

static void hfs_journal_unlock(__unused hfsmount_t *hfsmp)
{
}

typedef struct BTreeIterator { } BTreeIterator;

#define HFS_SYSCTL(...)

static void *hfs_malloc(size_t size)
{
    return malloc(size);
}

static void hfs_free(void *ptr, __unused size_t size)
{
    return free(ptr);
}

static void *hfs_mallocz(size_t size)
{
    return calloc(1, size);
}

bool panic_on_assert = true;

void hfs_assert_fail(const char *file, unsigned line, const char *expr)
{
	assert_fail_(file, line, "%s", expr);
}

#include "../core/VolumeAllocation.c"
#include "../core/rangelist.c"

static void *bitmap;

typedef struct buf {
	void *ptr;
	int size;
	void *fsprivate;
	bool is_shadow;
} *buf_t;

errno_t buf_bdwrite(__unused buf_t bp)
{
	return 0;
}

void buf_brelse(buf_t bp)
{
	if (bp->is_shadow)
		free(bp->ptr);
	free(bp);
}

uint32_t buf_count(__unused buf_t bp)
{
	return bp->size;
}

uintptr_t buf_dataptr(__unused buf_t bp)
{
	return (uintptr_t)bp->ptr;
}

int32_t buf_flags(__unused buf_t bp)
{
	return 0;
}

void buf_markinvalid(__unused buf_t bp)
{
}

errno_t buf_meta_bread(__unused vnode_t vp, daddr64_t blkno, int size,
					   __unused kauth_cred_t cred, buf_t *bpp)
{
	buf_t bp = calloc(1, sizeof(struct buf));

	bp->ptr  = bitmap + blkno * 4096;
	bp->size = size;

	*bpp = bp;

	return 0;
}

buf_t
buf_create_shadow(buf_t bp, boolean_t force_copy,
				  uintptr_t external_storage,
				  __unused void (*iodone)(buf_t, void *), __unused void *arg)
{
	assert(force_copy && !external_storage);

	buf_t new_bp = calloc(1, sizeof(struct buf));

	new_bp->ptr = malloc(bp->size);

	memcpy(new_bp->ptr, bp->ptr, bp->size);
	new_bp->size = bp->size;
	new_bp->is_shadow = true;

	return new_bp;
}

void *buf_fsprivate(buf_t bp)
{
	return bp->fsprivate;
}

void buf_setflags(__unused buf_t bp, __unused int32_t flags)
{
}

void buf_setfsprivate(buf_t bp, void *fsprivate)
{
	bp->fsprivate = fsprivate;
}

unsigned int kdebug_enable;

void kernel_debug(__unused uint32_t  debugid,
				  __unused uintptr_t arg1,
				  __unused uintptr_t arg2,
				  __unused uintptr_t arg3,
				  __unused uintptr_t arg4,
				  __unused uintptr_t arg5)
{
}

int
buf_invalidateblks(__unused vnode_t vp,
				   __unused int flags,
				   __unused int slpflag,
				   __unused int slptimeo)
{
	return 0;
}

#define BITMAP_CHUNK_SIZE	80

errno_t get_more_bits(bitmap_context_t *bitmap_ctx)
{
	uint32_t	start_bit;
	uint32_t	iosize = 0;
	uint32_t	last_bitmap_block;
	
	start_bit = bitmap_ctx->run_offset;
	
	if (start_bit >= bitmap_ctx->hfsmp->totalBlocks) {
		bitmap_ctx->chunk_end = 0;
		bitmap_ctx->bitmap = NULL;
		return 0;
	}
	
	iosize = BITMAP_CHUNK_SIZE;
	last_bitmap_block = start_bit + iosize;
	
	if (last_bitmap_block > bitmap_ctx->hfsmp->totalBlocks)
		last_bitmap_block = bitmap_ctx->hfsmp->totalBlocks;
	
	bitmap_ctx->chunk_current = 0;
	bitmap_ctx->chunk_end = last_bitmap_block - start_bit;
	if (bitmap_ctx->run_offset != 0)
		bitmap_ctx->bitmap += iosize / 8;
	
	return 0;
}

static errno_t update_summary_table(__unused bitmap_context_t *bitmap_ctx,
									__unused uint32_t start,
									__unused uint32_t count,
									__unused bool set)
{
	return 0;
}

int
hfs_find_free_extents_test(hfsmount_t *hfsmp)
{
	uint8_t bitmap[] = {
		/*  0:  */	0xff, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff,
		/*  64: */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		/* 128: */	0xff, 0xfe, 0xcf, 0xff, 0xff, 0xff, 0xff, 0x00,
		/* 192: */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* 256: */	0xff, 0xfe, 0xcf, 0xff, 0xff, 0xff, 0xff, 0x00,
	};
	
	assert(bit_count_set(bitmap, 0, 32) == 15);
	assert(bit_count_clr(bitmap, 15, 64) == 3);
	assert(bit_count_set(bitmap, 48, 160) == 16 + 64 + 15);
	assert(bit_count_set(bitmap, 48, 320) == 16 + 64 + 15);
	assert(bit_count_clr(bitmap, 190, 260) == 2 + 64);
	assert(bit_count_clr(bitmap, 190, 320) == 2 + 64);
	assert(bit_count_clr(bitmap, 316, 320) == 4);
	
	hfsmp->totalBlocks = sizeof(bitmap) * 8;
	
	struct bitmap_context ctx = {
		.hfsmp = hfsmp,
		.bitmap = bitmap,
	};
	
	uint32_t count;
	
	assert(!hfs_bit_count_set(&ctx, &count) && count == 15);
	assert(!hfs_bit_count_clr(&ctx, &count) && count == 3);
	assert(!hfs_bit_count_set(&ctx, &count) && count == 125);
	assert(!hfs_bit_count_clr(&ctx, &count) && count == 1);
	assert(!hfs_bit_count_set(&ctx, &count) && count == 2);
	assert(!hfs_bit_count_clr(&ctx, &count) && count == 2);
	assert(!hfs_bit_count_set(&ctx, &count) && count == 36);
	assert(!hfs_bit_count_clr(&ctx, &count) && count == 72);
	assert(!hfs_bit_count_set(&ctx, &count) && count == 15);
	assert(!hfs_bit_count_clr(&ctx, &count) && count == 1);
	assert(!hfs_bit_count_set(&ctx, &count) && count == 2);
	assert(!hfs_bit_count_clr(&ctx, &count) && count == 2);
	assert(!hfs_bit_count_set(&ctx, &count) && count == 36);
	assert(!hfs_bit_count_clr(&ctx, &count) && count == 8);
	
	assert(hfs_bit_offset(&ctx) == 320);
	
	return 0;
}

int main(void)
{
	const int blocks = 100000;

	size_t bitmap_size = howmany(blocks, 8);
	bitmap = calloc(1, bitmap_size);

	cnode_t alloc_cp = {
		.c_blocks = howmany(bitmap_size, 4096),
	};

	struct journal jnl;

	struct hfsmount mnt = {
		.allocLimit = blocks,
		.totalBlocks = blocks,
		.freeBlocks = blocks,
		.blockSize = 4096,
		.vcbVBMIOSize = 4096,
		.hfs_allocation_cp = &alloc_cp,
		.vcbSigWord = kHFSPlusSigWord,
		.jnl = &jnl,
		.hfs_logical_bytes = blocks * 4096,
	};

	assert(!hfs_init_summary (&mnt));

	uint32_t start, count;
	assert(!BlockAllocate(&mnt, 0, 10, 10, 0, &start, &count)
		   && start == 0 && count == 10);
	assert(!BlockAllocate(&mnt, 0, 10, 10, 0, &start, &count)
		   && start == 10 && count == 10);
	assert(!BlockAllocate(&mnt, 0, 32768 - 20, 32768 - 20, 0, &start, &count)
		   && start == 20 && count == 32768 - 20);

	assert(!ScanUnmapBlocks(&mnt));

	assert(!hfs_find_summary_free(&mnt, 0, &start) && start == 32768);

	assert(!BlockAllocate(&mnt, 0, blocks - 32768, blocks - 32768, 0,
						  &start, &count)
		   && start == 32768 && count == blocks - 32768);

	assert(BlockAllocate(&mnt, 0, 1, 1, 0, &start, &count) == dskFulErr);

	assert(!BlockDeallocate(&mnt, 1, 1, 0));
	assert(!BlockDeallocate(&mnt, 3, 1, 0));

	assert(!hfs_find_summary_free(&mnt, 0, &start) && start == 0);
	assert(!hfs_find_summary_free(&mnt, 1, &start) && start == 1);
	assert(!hfs_find_summary_free(&mnt, 32767, &start) && start == 32767);

	assert(BlockAllocate(&mnt, 0, 2, 2, HFS_ALLOC_FORCECONTIG,
						 &start, &count) == dskFulErr);

	// The last block never gets marked
	assert(!hfs_find_summary_free(&mnt, 32768, &start) && start == 98304);

	assert(!BlockDeallocate(&mnt, 33000, 1, 0));

	assert(!hfs_find_summary_free(&mnt, 32768, &start) && start == 32768);
	assert(!hfs_find_summary_free(&mnt, 65535, &start) && start == 65535);
	assert(!hfs_find_summary_free(&mnt, 65536, &start) && start == 98304);

	assert(!BlockDeallocate(&mnt, 33001, 1, 0));
	assert(!BlockAllocate(&mnt, 0, 2, 2, HFS_ALLOC_FORCECONTIG,
						  &start, &count) && start == 33000 && count == 2);

	// Test tentative allocations

	HFSPlusExtentDescriptor ext = {
		.blockCount = 1,
	};

	struct rl_entry *reservation = NULL;

	hfs_alloc_extra_args_t args = {
		.max_blocks = 2,
		.reservation_out = &reservation,
	};

	assert(!hfs_block_alloc(&mnt, &ext, HFS_ALLOC_TENTATIVE, &args)
		   && ext.startBlock == 1 && ext.blockCount == 1);

	assert(rl_len(reservation) == 1);

	// This shouldn't use the tentative block
	assert(!BlockAllocate(&mnt, 0, 1, 1, 0, &start, &count)
		   && start == 3 && count == 1);

	// This should steal the tentative block
	assert(!BlockAllocate(&mnt, 0, 1, 1, 0, &start, &count)
		   && start == 1 && count == 1);
	assert(reservation->rl_start == -1 && reservation->rl_end == -2);

	// Free 200
	assert(!BlockDeallocate(&mnt, 32700, 200, 0));

	// Make 100 tentative
	ext.blockCount = 100;

	args.max_blocks = 100;
	assert(!hfs_block_alloc(&mnt, &ext, HFS_ALLOC_TENTATIVE, &args)
		   && ext.startBlock == 32700 && ext.blockCount == 100);

	// This should allocate the other 100
	assert(!BlockAllocate(&mnt, 0, 100, 100, 0, &start, &count)
		   && start == 32800 && count == 100);

	assert(mnt.tentativeBlocks == 100);

	// Allocate 25 in the middle of the tentative block
	assert(!BlockAllocate(&mnt, 32750, 25, 25, 0, &start, &count)
		   && start == 32750 && count == 25);

	// That should have split the reservation
	assert(reservation->rl_start == 32700 && reservation->rl_end == 32749);

	assert(mnt.tentativeBlocks == 50);

	// The tail should have been freed
	assert(mnt.vcbFreeExtCnt == 1
		   && mnt.vcbFreeExt[0].startBlock == 32775
		   && mnt.vcbFreeExt[0].blockCount == 25);

	// Allocate the bit we just freed
	assert(!BlockAllocate(&mnt, 32705, 1, 25, HFS_ALLOC_FORCECONTIG,
						  &start, &count)
		   && start == 32775 && count == 25);

	// Split the tentative reservation again
	assert(!BlockAllocate(&mnt, 32705, 1, 3, 0,
						  &start, &count)
		   && start == 32705 && count == 3);

	// This time head should have been free
	assert(mnt.vcbFreeExtCnt == 1
		   && mnt.vcbFreeExt[0].startBlock == 32700
		   && mnt.vcbFreeExt[0].blockCount == 5);

	// Reservation will be tail
	assert(reservation->rl_start == 32708 && reservation->rl_end == 32749);

	assert(mnt.tentativeBlocks == 42);

	// Free up what we just allocated
	assert(!BlockDeallocate(&mnt, 32705, 3, 0));

	// This should allocate something overlapping the start of the reservation
	assert(!BlockAllocate(&mnt, 0, 15, 15, HFS_ALLOC_FORCECONTIG,
						  &start, &count)
		   && start == 32700 && count == 15);

	// Should have eaten into start of reservation
	assert(reservation->rl_start == 32715 && reservation->rl_end == 32749);

	// Free up a bit at the end
	assert(!BlockDeallocate(&mnt, 32750, 5, 0));

	// Allocate a bit overlapping end of reservation
	assert(!BlockAllocate(&mnt, 32740, 15, 15, HFS_ALLOC_FORCECONTIG,
						  &start, &count)
		   && start == 32740 && count == 15);

	// Should have eaten into end of reservation
	assert(reservation->rl_start == 32715 && reservation->rl_end == 32739);

	assert(!BlockDeallocate(&mnt, 32700, 15, 0));

	ext.startBlock = 0;
	ext.blockCount = 40;

	struct rl_entry *locked_reservation;

	args.max_blocks = 40;
	args.reservation_in = &reservation;
	args.reservation_out = &locked_reservation;

	assert(!hfs_block_alloc(&mnt, &ext,
							HFS_ALLOC_TRY_HARD | HFS_ALLOC_USE_TENTATIVE | HFS_ALLOC_LOCKED,
							&args));

	assert(ext.startBlock == 32700 && ext.blockCount == 40);
	assert(reservation == NULL);

	hfs_free_locked(&mnt, &locked_reservation);

	assert(mnt.freeBlocks == 40 && mnt.lockedBlocks == 0);

	args.reservation_out = &reservation;

	assert(!hfs_block_alloc(&mnt, &ext, HFS_ALLOC_TRY_HARD | HFS_ALLOC_TENTATIVE,
							&args));

	assert(mnt.freeBlocks == 40 && mnt.tentativeBlocks == 40
		   && rl_len(reservation) == 40);

	// Hack lockedBlocks so that hfs_free_blocks returns 20
	mnt.lockedBlocks = 20;

	ext.blockCount = 20; // Minimum

	args.reservation_in = &reservation;

	assert(!hfs_block_alloc(&mnt, &ext,
							HFS_ALLOC_TRY_HARD | HFS_ALLOC_USE_TENTATIVE,
							&args));

	// Should have only returned 20 blocks
	assert(rl_len(reservation) == 20 && ext.blockCount == 20
		   && ext.startBlock == 32700);

	mnt.lockedBlocks = 10;

	// ENOSPC because minimum == 20
	assert(hfs_block_alloc(&mnt, &ext,
						   HFS_ALLOC_TRY_HARD | HFS_ALLOC_USE_TENTATIVE,
						   &args) == ENOSPC);

	mnt.lockedBlocks = 0;
	assert(!hfs_block_alloc(&mnt, &ext,
							HFS_ALLOC_TRY_HARD | HFS_ALLOC_USE_TENTATIVE,
							&args));

	// Should use up the remaining reservation
	assert(!reservation && ext.startBlock == 32720 && ext.blockCount == 20);

	assert(!BlockDeallocate(&mnt, 32700, 40, 0));

	// Check alignment
	args.alignment = 16;
	args.max_blocks = 1000;
	ext.blockCount = 1;
	assert(!hfs_block_alloc(&mnt, &ext, HFS_ALLOC_TRY_HARD, &args));

	assert(ext.startBlock == 32700 && ext.blockCount == 32);

	assert(!BlockDeallocate(&mnt, 32700, 32, 0));

	args.alignment_offset = 3;
	ext.blockCount = 1;
	assert(!hfs_block_alloc(&mnt, &ext, HFS_ALLOC_TRY_HARD, &args));

	assert(ext.startBlock == 32700 && ext.blockCount == 29);
	
	hfs_find_free_extents_test(&mnt);

	printf("[PASSED] hfs_alloc_test\n");

	return 0;
}
