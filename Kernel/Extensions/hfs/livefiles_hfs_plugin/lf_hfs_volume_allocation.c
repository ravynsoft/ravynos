/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
*
*  lf_hfs_volume_allocation.c
*  livefiles_hfs
*
*  Created by Or Haimovich on 22/3/18.
*/

#include <sys/disk.h>

#include "lf_hfs_volume_allocation.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_endian.h"
#include "lf_hfs_format.h"
#include "lf_hfs_locks.h"
#include "lf_hfs_rangelist.h"
#include "lf_hfs_raw_read_write.h"
#include "lf_hfs_readwrite_ops.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_journal.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_generic_buf.h"

#pragma clang diagnostic ignored "-Waddress-of-packed-member"

enum {
    /*
     * HFSDBG_ALLOC_ENABLED: Log calls to BlockAllocate and
     * BlockDeallocate, including the internal BlockAllocateXxx
     * routines so we can see how an allocation was satisfied.
     *
     * HFSDBG_EXT_CACHE_ENABLED: Log routines that read or write the
     * free extent cache.
     *
     * HFSDBG_UNMAP_ENABLED: Log events involving the trim list.
     *
     * HFSDBG_BITMAP_ENABLED: Log accesses to the volume bitmap (setting
     * or clearing bits, scanning the bitmap).
     */
    HFSDBG_ALLOC_ENABLED        = 1,
    HFSDBG_EXT_CACHE_ENABLED    = 2,
    HFSDBG_UNMAP_ENABLED        = 4,
    HFSDBG_BITMAP_ENABLED        = 8
};

enum {
    kBytesPerWord           =    4,
    kBitsPerByte            =    8,
    kBitsPerWord            =    32,

    kBitsWithinWordMask     =    kBitsPerWord-1
};

#define kLowBitInWordMask       0x00000001ul
#define kHighBitInWordMask      0x80000000ul
#define kAllBitsSetInWord       0xFFFFFFFFul

#define HFS_MIN_SUMMARY_BLOCKSIZE 4096

#define ALLOC_DEBUG 0

static OSErr ReadBitmapBlock(
                             ExtendedVCB        *vcb,
                             u_int32_t        bit,
                             u_int32_t        **buffer,
                             GenericLFBufPtr  *blockRef,
                             hfs_block_alloc_flags_t flags);

static OSErr ReleaseBitmapBlock(
                                ExtendedVCB        *vcb,
                                GenericLFBufPtr    blockRef,
                                Boolean            dirty);

static OSErr hfs_block_alloc_int(hfsmount_t *hfsmp,
                                 HFSPlusExtentDescriptor *extent,
                                 hfs_block_alloc_flags_t flags,
                                 hfs_alloc_extra_args_t *ap);

static OSErr BlockFindAny(
                          ExtendedVCB        *vcb,
                          u_int32_t        startingBlock,
                          u_int32_t        endingBlock,
                          u_int32_t        maxBlocks,
                          hfs_block_alloc_flags_t flags,
                          Boolean            trustSummary,
                          u_int32_t        *actualStartBlock,
                          u_int32_t        *actualNumBlocks);

static OSErr BlockFindAnyBitmap(
                                ExtendedVCB        *vcb,
                                u_int32_t        startingBlock,
                                u_int32_t        endingBlock,
                                u_int32_t        maxBlocks,
                                hfs_block_alloc_flags_t flags,
                                u_int32_t        *actualStartBlock,
                                u_int32_t        *actualNumBlocks);

static OSErr BlockFindContig(
                             ExtendedVCB        *vcb,
                             u_int32_t        startingBlock,
                             u_int32_t        minBlocks,
                             u_int32_t        maxBlocks,
                             hfs_block_alloc_flags_t flags,
                             u_int32_t        *actualStartBlock,
                             u_int32_t        *actualNumBlocks);

static OSErr BlockFindContiguous(
                                 ExtendedVCB        *vcb,
                                 u_int32_t        startingBlock,
                                 u_int32_t        endingBlock,
                                 u_int32_t        minBlocks,
                                 u_int32_t        maxBlocks,
                                 Boolean            useMetaZone,
                                 Boolean            trustSummary,
                                 u_int32_t        *actualStartBlock,
                                 u_int32_t        *actualNumBlocks,
                                 hfs_block_alloc_flags_t flags);

static OSErr BlockFindKnown(
                            ExtendedVCB        *vcb,
                            u_int32_t        maxBlocks,
                            u_int32_t        *actualStartBlock,
                            u_int32_t        *actualNumBlocks);

static OSErr hfs_alloc_try_hard(hfsmount_t *hfsmp,
                                HFSPlusExtentDescriptor *extent,
                                uint32_t max_blocks,
                                hfs_block_alloc_flags_t flags);

static OSErr BlockMarkAllocatedInternal (
                                         ExtendedVCB        *vcb,
                                         u_int32_t        startingBlock,
                                         u_int32_t        numBlocks,
                                         hfs_block_alloc_flags_t flags);

static OSErr BlockMarkFreeInternal(
                                   ExtendedVCB    *vcb,
                                   u_int32_t    startingBlock,
                                   u_int32_t    numBlocks,
                                   Boolean     do_validate);


static OSErr ReadBitmapRange (struct hfsmount *hfsmp, uint32_t offset, uint32_t iosize,
                              uint32_t **buffer, GenericLFBuf **blockRef);

static OSErr ReleaseScanBitmapRange( GenericLFBufPtr bp );

static int hfs_track_unmap_blocks (struct hfsmount *hfsmp, u_int32_t offset,
                                   u_int32_t numBlocks, struct jnl_trim_list *list);

static int hfs_alloc_scan_range(struct hfsmount *hfsmp,
                                u_int32_t startbit,
                                u_int32_t *bitToScan,
                                struct jnl_trim_list *list);

static int hfs_scan_range_size (struct hfsmount* hfsmp, uint32_t start, uint32_t *iosize);
/* Bitmap Re-use Detection */
static inline int extents_overlap (uint32_t start1, uint32_t len1,
                                   uint32_t start2, uint32_t len2) {
    return !( ((start1 + len1) <= start2) || ((start2 + len2) <= start1) );
}

/* Summary Table Functions */
static int hfs_set_summary (struct hfsmount *hfsmp, uint32_t summarybit, uint32_t inuse);
static int hfs_get_summary_index (struct hfsmount *hfsmp, uint32_t block, uint32_t *index);
static int hfs_find_summary_free (struct hfsmount *hfsmp, uint32_t block, uint32_t *newblock);
static int hfs_get_summary_allocblock (struct hfsmount *hfsmp, uint32_t summarybit, uint32_t *alloc);
static int hfs_release_summary (struct hfsmount *hfsmp, uint32_t start, uint32_t length);
static int hfs_check_summary (struct hfsmount *hfsmp, uint32_t start, uint32_t *freeblocks);

/* Used in external mount code to initialize the summary table */
int hfs_init_summary (struct hfsmount *hfsmp);

#if ALLOC_DEBUG
void hfs_validate_summary (struct hfsmount *hfsmp);
#endif


/* Functions for manipulating free extent cache */
static void remove_free_extent_cache(struct hfsmount *hfsmp, u_int32_t startBlock, u_int32_t blockCount);
static Boolean add_free_extent_cache(struct hfsmount *hfsmp, u_int32_t startBlock, u_int32_t blockCount);
static void sanity_check_free_ext(struct hfsmount *hfsmp, int check_allocated);

static void hfs_release_reserved(hfsmount_t *hfsmp, struct rl_entry *range, int list);


#if ALLOC_DEBUG
/*
 * Validation Routine to verify that the TRIM list maintained by the journal
 * is in good shape relative to what we think the bitmap should have.  We should
 * never encounter allocated blocks in the TRIM list, so if we ever encounter them,
 * we panic.
 */
int trim_validate_bitmap (struct hfsmount *hfsmp);
int trim_validate_bitmap (struct hfsmount *hfsmp) {
    u_int64_t blockno_offset;
    u_int64_t numblocks;
    int i;
    int count;
    u_int32_t startblk;
    u_int32_t blks;
    int err = 0;
    uint32_t alloccount = 0;

    if (hfsmp->jnl) {
        struct journal *jnl = (struct journal*)hfsmp->jnl;
        if (jnl->active_tr) {
            struct jnl_trim_list *trim = &(jnl->active_tr->trim);
            count = trim->extent_count;
            for (i = 0; i < count; i++) {
                blockno_offset = trim->extents[i].offset;
                blockno_offset = blockno_offset - (uint64_t)hfsmp->hfsPlusIOPosOffset;
                blockno_offset = blockno_offset / hfsmp->blockSize;
                numblocks = trim->extents[i].length / hfsmp->blockSize;

                startblk = (u_int32_t)blockno_offset;
                blks = (u_int32_t) numblocks;
                err = hfs_count_allocated (hfsmp, startblk, blks, &alloccount);

                if (err == 0 && alloccount != 0) {
                    LFHFS_LOG(LEVEL_ERROR, "trim_validate_bitmap: %d blocks @ ABN %d are allocated!", alloccount, startblk);
                    hfs_assert(0);
                }
            }
        }
    }
    return 0;
}

#endif

/*
 ;________________________________________________________________________________
 ;
 ; Routine:        hfs_issue_unmap
 ;
 ; Function:    Issue a DKIOCUNMAP for all blocks currently tracked by the jnl_trim_list
 ;
 ; Input Arguments:
 ;    hfsmp            - The volume containing the allocation blocks.
 ;  list            - The list of currently tracked trim ranges.
 ;________________________________________________________________________________
 */

static int hfs_issue_unmap (struct hfsmount *hfsmp, struct jnl_trim_list *list)
{
    dk_unmap_t unmap;
    int error = 0;

    if (list->extent_count > 0 && list->extents != NULL) {
        bzero(&unmap, sizeof(unmap));
        unmap.extents = list->extents;
        unmap.extentsCount = list->extent_count;
        
        /* Issue a TRIM and flush them out */
        error = ioctl(hfsmp->hfs_devvp->psFSRecord->iFD, DKIOCUNMAP, &unmap);
        
        bzero (list->extents, (list->allocated_count * sizeof(dk_extent_t)));
        bzero (&unmap, sizeof(unmap));
        list->extent_count = 0;
    }
    
    return error;
}

/*
 ;________________________________________________________________________________
 ;
 ; Routine:        hfs_track_unmap_blocks
 ;
 ; Function:    Make note of a range of allocation blocks that should be
 ;                unmapped (trimmed).  That is, the given range of blocks no
 ;                longer have useful content, and the device can unmap the
 ;                previous contents.  For example, a solid state disk may reuse
 ;                the underlying storage for other blocks.
 ;
 ;                This routine is only supported for journaled volumes.
 ;
 ;              *****NOTE*****:
 ;              This function should *NOT* be used when the volume is fully
 ;              mounted.  This function is intended to support a bitmap iteration
 ;              at mount time to fully inform the SSD driver of the state of all blocks
 ;              at mount time, and assumes that there is no allocation/deallocation
 ;              interference during its iteration.,
 ;
 ; Input Arguments:
 ;    hfsmp            - The volume containing the allocation blocks.
 ;    offset          - The first allocation block of the extent being freed.
 ;    numBlocks        - The number of allocation blocks of the extent being freed.
 ;  list            - The list of currently tracked trim ranges.
 ;________________________________________________________________________________
 */
static int hfs_track_unmap_blocks (struct hfsmount *hfsmp, u_int32_t start, u_int32_t numBlocks, struct jnl_trim_list *list) {
    u_int64_t offset;
    u_int64_t length;
    int error = 0;

    if ((hfsmp->jnl != NULL))
    {
        if ((hfsmp->hfs_flags & HFS_UNMAP) && list->allocated_count && list->extents != NULL)
        {
            
            int extent_no = list->extent_count;
            offset = (u_int64_t) start * hfsmp->blockSize + (u_int64_t) hfsmp->hfsPlusIOPosOffset;
            length = (u_int64_t) numBlocks * hfsmp->blockSize;

            list->extents[extent_no].offset = offset;
            list->extents[extent_no].length = length;
            list->extent_count++;
            if (list->extent_count == list->allocated_count) {
                error = hfs_issue_unmap (hfsmp, list);
            }
        }
    }

    return error;
}

/*
 ;________________________________________________________________________________
 ;
 ; Routine:        hfs_trim_callback
 ;
 ; Function:        This function is called when a transaction that freed extents
 ;                (via hfs_unmap_free_extent/journal_trim_add_extent) has been
 ;                written to the on-disk journal.  This routine will add those
 ;                extents to the free extent cache so that they can be reused.
 ;
 ;                CAUTION: This routine is called while the journal's trim lock
 ;                is held shared, so that no other thread can reuse any portion
 ;                of those extents.  We must be very careful about which locks
 ;                we take from within this callback, to avoid deadlock.  The
 ;                call to add_free_extent_cache will end up taking the cache's
 ;                lock (just long enough to add these extents to the cache).
 ;
 ;                CAUTION: If the journal becomes invalid (eg., due to an I/O
 ;                error when trying to write to the journal), this callback
 ;                will stop getting called, even if extents got freed before
 ;                the journal became invalid!
 ;
 ; Input Arguments:
 ;    arg                - The hfsmount of the volume containing the extents.
 ;    extent_count    - The number of extents freed in the transaction.
 ;    extents            - An array of extents (byte ranges) that were freed.
 ;________________________________________________________________________________
 */

void
hfs_trim_callback(void *arg, uint32_t extent_count, const dk_extent_t *extents)
{
    uint32_t i;
    uint32_t startBlock, numBlocks;
    struct hfsmount *hfsmp = arg;

    for (i=0; i<extent_count; ++i) {
        /* Convert the byte range in *extents back to a range of allocation blocks. */
        startBlock = (uint32_t)((extents[i].offset - hfsmp->hfsPlusIOPosOffset) / hfsmp->blockSize);
        numBlocks = (uint32_t)(extents[i].length / hfsmp->blockSize);
        (void) add_free_extent_cache(hfsmp, startBlock, numBlocks);
    }
}


/*
 ;________________________________________________________________________________
 ;
 ; Routine:        ScanUnmapBlocks
 ;
 ; Function:    Traverse the bitmap, and potentially issue DKIOCUNMAPs to the underlying
 ;                device as needed so that the underlying disk device is as
 ;                up-to-date as possible with which blocks are unmapped.
 ;                Additionally build up the summary table as needed.
 ;
 ;                This function reads the bitmap in large block size
 ;                 (up to 1MB) unlike the runtime which reads the bitmap
 ;                 in 4K block size.  So if this function is being called
 ;                after the volume is mounted and actively modified, the
 ;                caller needs to invalidate all of the existing buffers
 ;                associated with the bitmap vnode before calling this
 ;                 function.  If the buffers are not invalidated, it can
 ;                cause buf_t collision and potential data corruption.
 ;
 ; Input Arguments:
 ;    hfsmp            - The volume containing the allocation blocks.
 ;________________________________________________________________________________
 */

u_int32_t ScanUnmapBlocks (struct hfsmount *hfsmp)
{
    u_int32_t blocks_scanned = 0;
    int error = 0;
    struct jnl_trim_list trimlist;

    /*
     *struct jnl_trim_list {
     uint32_t    allocated_count;
     uint32_t    extent_count;
     dk_extent_t *extents;
     };
     */
    bzero (&trimlist, sizeof(trimlist));

    /*
     * Any trim related work should be tied to whether the underlying
     * storage media supports UNMAP, as any solid state device would
     * on desktop or embedded.
     *
     * We do this because we may want to scan the full bitmap on
     * desktop for spinning media for the purposes of building up the
     * summary table.
     *
     * We also avoid sending TRIMs down to the underlying media if the
     * mount is read-only.
     */

    if ((hfsmp->hfs_flags & HFS_UNMAP) &&
        ((hfsmp->hfs_flags & HFS_READ_ONLY) == 0)) {
        /* If the underlying device supports unmap and the mount is read-write, initialize */
        int alloc_count = ((u_int32_t)PAGE_SIZE) / sizeof(dk_extent_t);
        void *extents = hfs_malloc(alloc_count * sizeof(dk_extent_t));
        trimlist.extents = (dk_extent_t*)extents;
        trimlist.allocated_count = alloc_count;
        trimlist.extent_count = 0;
    }

    while ((blocks_scanned < hfsmp->totalBlocks) && (error == 0)){

        error = hfs_alloc_scan_range (hfsmp, blocks_scanned, &blocks_scanned, &trimlist);

        if (error) {
            LFHFS_LOG(LEVEL_DEBUG, "ScanUnmapBlocks: bitmap scan range error: %d on vol=%s\n", error, hfsmp->vcbVN);
            break;
        }
    }

    if ((hfsmp->hfs_flags & HFS_UNMAP) && ((hfsmp->hfs_flags & HFS_READ_ONLY) == 0)) {
        if (error == 0) {
            hfs_issue_unmap(hfsmp, &trimlist);
        }
        if (trimlist.extents) {
            hfs_free(trimlist.extents);
        }
    }
    
    /*
     * This is in an #if block because hfs_validate_summary prototype and function body
     * will only show up if ALLOC_DEBUG is on, to save wired memory ever so slightly.
     */
#if ALLOC_DEBUG
    sanity_check_free_ext(hfsmp, 1);
    if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
        /* Validate the summary table too! */
        hfs_validate_summary(hfsmp);
        LFHFS_LOG(LEVEL_DEBUG, "ScanUnmapBlocks: Summary validation complete on %s\n", hfsmp->vcbVN);
    }
#endif

    return error;
}

static void add_to_reserved_list(hfsmount_t *hfsmp, uint32_t start,
                                 uint32_t count, int list,
                                 struct rl_entry **reservation)
{
    struct rl_entry *range, *next_range;

    if (list == HFS_TENTATIVE_BLOCKS) {
        int nranges = 0;
        // Don't allow more than 4 tentative reservations
        TAILQ_FOREACH_SAFE(range, &hfsmp->hfs_reserved_ranges[HFS_TENTATIVE_BLOCKS],
                           rl_link, next_range) {
            if (++nranges > 3)
                hfs_release_reserved(hfsmp, range, HFS_TENTATIVE_BLOCKS);
        }
    }

    range = hfs_malloc(sizeof(*range));
    range->rl_start = start;
    range->rl_end = start + count - 1;
    TAILQ_INSERT_HEAD(&hfsmp->hfs_reserved_ranges[list], range, rl_link);
    *reservation = range;
}

static void hfs_release_reserved(hfsmount_t *hfsmp,
                                 struct rl_entry *range,
                                 int list)
{
    if (range->rl_start == -1)
        return;

    TAILQ_REMOVE(&hfsmp->hfs_reserved_ranges[list], range, rl_link);

    if (rl_len(range) > 0) {
        if (list == HFS_TENTATIVE_BLOCKS)
            hfsmp->tentativeBlocks -= rl_len(range);
        else {
            hfs_assert(hfsmp->lockedBlocks >= rl_len(range));
            hfsmp->lockedBlocks -= rl_len(range);
        }
        hfs_release_summary(hfsmp, (uint32_t)range->rl_start, (uint32_t)rl_len(range));
        add_free_extent_cache(hfsmp, (uint32_t)range->rl_start, (uint32_t)rl_len(range));
    }

    range->rl_start = -1;
    range->rl_end   = -2;
}

static void hfs_free_locked_internal(hfsmount_t *hfsmp,
                                     struct rl_entry **reservation,
                                     int list)
{
    if (*reservation) {
        hfs_release_reserved(hfsmp, *reservation, list);
        hfs_free(*reservation);
        *reservation = NULL;
    }
}

void hfs_free_tentative(hfsmount_t *hfsmp, struct rl_entry **reservation)
{
    hfs_free_locked_internal(hfsmp, reservation, HFS_TENTATIVE_BLOCKS);
}

void hfs_free_locked(hfsmount_t *hfsmp, struct rl_entry **reservation)
{
    hfs_free_locked_internal(hfsmp, reservation, HFS_LOCKED_BLOCKS);
}

OSErr BlockAllocate (
                     hfsmount_t        *hfsmp,                /* which volume to allocate space on */
                     u_int32_t        startingBlock,        /* preferred starting block, or 0 for no preference */
                     u_int32_t        minBlocks,        /* desired number of blocks to allocate */
                     u_int32_t        maxBlocks,        /* maximum number of blocks to allocate */
                     hfs_block_alloc_flags_t flags,            /* option flags */
                     u_int32_t        *actualStartBlock,    /* actual first block of allocation */
                     u_int32_t        *actualNumBlocks)
{
    hfs_alloc_extra_args_t extra_args = {
        .max_blocks = maxBlocks
    };

    HFSPlusExtentDescriptor extent = { startingBlock, minBlocks };

    OSErr err = hfs_block_alloc_int(hfsmp, &extent, flags, &extra_args);

    *actualStartBlock = extent.startBlock;
    *actualNumBlocks  = extent.blockCount;

    return err;
}

errno_t hfs_block_alloc(hfsmount_t *hfsmp,
                        HFSPlusExtentDescriptor *extent,
                        hfs_block_alloc_flags_t flags,
                        hfs_alloc_extra_args_t *ap)
{
    return MacToVFSError(hfs_block_alloc_int(hfsmp, extent, flags, ap));
}

/*
 ;________________________________________________________________________________
 ;
 ; Routine:       hfs_block_alloc_int
 ;
 ; Function:   Allocate space on a volume.    If contiguous allocation is requested,
 ;               at least the requested number of bytes will be allocated or an
 ;               error will be returned.    If contiguous allocation is not forced,
 ;               the space will be allocated with the first largest extent available
 ;               at the requested starting allocation block.  If there is not enough
 ;               room there, a block allocation of less than the requested size will be
 ;               allocated.
 ;
 ;               If the requested starting block is 0 (for new file allocations),
 ;               the volume's allocation block pointer will be used as a starting
 ;               point.
 ;
 ; Input Arguments:
 ;   hfsmp           - Pointer to the HFS mount structure.
 ;   extent          - startBlock indicates the block to start
 ;                     searching from and blockCount is the number of
 ;                     blocks required.  Depending on the flags used,
 ;                     more or less blocks may be returned.  The
 ;                     allocated extent is returned via this
 ;                     parameter.
 ;   flags           - Flags to specify options like contiguous, use
 ;                     metadata zone, skip free block check, etc.
 ;   ap              - Additional arguments used depending on flags.
 ;                     See hfs_alloc_extra_args_t and below.
 ;
 ; Output:
 ;   (result)        - Error code, zero for successful allocation
 ;   extent          - If successful, the allocated extent.
 ;
 ; Side effects:
 ;     The volume bitmap is read and updated; the volume bitmap cache may be changed.
 ;
 ; HFS_ALLOC_TENTATIVE
 ; Blocks will be reserved but not marked allocated.  They can be
 ; stolen if free space is limited.  Tentative blocks can be used by
 ; passing HFS_ALLOC_USE_TENTATIVE and passing in the resevation.
 ; @ap->reservation_out is used to store the reservation.
 ;
 ; HFS_ALLOC_USE_TENTATIVE
 ; Use blocks previously returned with HFS_ALLOC_TENTATIVE.
 ; @ap->reservation_in should be set to whatever @ap->reservation_out
 ; was set to when HFS_ALLOC_TENTATIVE was used.  If the tentative
 ; reservation was stolen, a normal allocation will take place.
 ;
 ; HFS_ALLOC_LOCKED
 ; Blocks will be reserved but not marked allocated.  Unlike tentative
 ; reservations they cannot be stolen.  It is safe to write to these
 ; blocks.  @ap->reservation_out is used to store the reservation.
 ;
 ; HFS_ALLOC_COMMIT
 ; This will take blocks previously returned with HFS_ALLOC_LOCKED and
 ; mark them allocated on disk.  @ap->reservation_in is used.
 ;
 ; HFS_ALLOC_ROLL_BACK
 ; Take blocks that were just recently deallocated and mark them
 ; allocated.  This is for roll back situations.  Blocks got
 ; deallocated and then something went wrong and we need to roll back
 ; by marking the blocks allocated.
 ;
 ; HFS_ALLOC_FORCECONTIG
 ; It will not return fewer than @min_blocks.
 ;
 ; HFS_ALLOC_TRY_HARD
 ; We will perform an exhaustive search to try and find @max_blocks.
 ; It will not return fewer than @min_blocks.
 ;
 ;________________________________________________________________________________
 */
OSErr hfs_block_alloc_int(hfsmount_t *hfsmp,
                          HFSPlusExtentDescriptor *extent,
                          hfs_block_alloc_flags_t flags,
                          hfs_alloc_extra_args_t *ap)
{
    OSErr     err = 0;
    u_int32_t freeBlocks;
    Boolean   updateAllocPtr = false;        //    true if nextAllocation needs to be updated
    Boolean   forceContiguous = false;
    Boolean   forceFlush;

    uint32_t startingBlock = extent->startBlock;
    uint32_t minBlocks = extent->blockCount;
    uint32_t maxBlocks = (ap && ap->max_blocks) ? ap->max_blocks : minBlocks;

    if (ISSET(flags, HFS_ALLOC_COMMIT)) {
        if (ap == NULL || ap->reservation_in == NULL) {
            err = paramErr;
            goto exit;
        }
        extent->startBlock = (uint32_t)(*ap->reservation_in)->rl_start;
        extent->blockCount = (uint32_t)rl_len(*ap->reservation_in);
        goto mark_allocated;
    }

    if (ISSET(flags, HFS_ALLOC_ROLL_BACK))
        goto mark_allocated;

    freeBlocks = hfs_freeblks(hfsmp, 0);

    if (ISSET(flags, HFS_ALLOC_USE_TENTATIVE)) {
        if (ap == NULL || ap->reservation_in == NULL) {
            err = paramErr;
            goto exit;
        }
        struct rl_entry *range = *ap->reservation_in;

        if (range && range->rl_start != -1) {
            /*
             * It's possible that we have a tentative reservation
             * but there aren't enough free blocks due to loaned blocks
             * or insufficient space in the backing store.
             */
            uint32_t count = (uint32_t)min(min(maxBlocks, rl_len(range)), freeBlocks);

            if (count >= minBlocks) {
                extent->startBlock = (uint32_t)range->rl_start;
                extent->blockCount = count;

                // Should we go straight to commit?
                if (!ISSET(flags, HFS_ALLOC_LOCKED))
                    SET(flags, HFS_ALLOC_COMMIT);

                goto mark_allocated;
            }
        }

        /*
         * We can't use the tentative reservation so free it and allocate
         * normally.
         */
        hfs_free_tentative(hfsmp, ap->reservation_in);
        CLR(flags, HFS_ALLOC_USE_TENTATIVE);
    }

    if (ISSET(flags, HFS_ALLOC_FORCECONTIG | HFS_ALLOC_TRY_HARD))
        forceContiguous = true;

    if (flags & HFS_ALLOC_FLUSHTXN) {
        forceFlush = true;
    }
    else {
        forceFlush = false;
    }

    hfs_assert(hfsmp->freeBlocks >= hfsmp->tentativeBlocks);

    // See if we have to steal tentative blocks
    if (freeBlocks < hfsmp->tentativeBlocks + minBlocks)
        SET(flags, HFS_ALLOC_IGNORE_TENTATIVE);

    /* Skip free block check if blocks are being allocated for relocating
     * data during truncating a volume.
     *
     * During hfs_truncatefs(), the volume free block count is updated
     * before relocating data to reflect the total number of free blocks
     * that will exist on the volume after resize is successful.  This
     * means that we have reserved allocation blocks required for relocating
     * the data and hence there is no need to check the free blocks.
     * It will also prevent resize failure when the number of blocks in
     * an extent being relocated is more than the free blocks that will
     * exist after the volume is resized.
     */
    if ((flags & HFS_ALLOC_SKIPFREEBLKS) == 0) {
        //    If the disk is already full, don't bother.
        if (freeBlocks == 0) {
            err = dskFulErr;
            goto exit;
        }
        if (forceContiguous && freeBlocks < minBlocks) {
            err = dskFulErr;
            goto exit;
        }

        /*
         * Clip if necessary so we don't over-subscribe the free blocks.
         */
        if (minBlocks > freeBlocks) {
            minBlocks = freeBlocks;
        }
        if (maxBlocks > freeBlocks) {
            maxBlocks = freeBlocks;
        }
    }

    if (ISSET(flags, HFS_ALLOC_TRY_HARD)) {
        err = hfs_alloc_try_hard(hfsmp, extent, maxBlocks, flags);
        if (err)
            goto exit;

        goto mark_allocated;
    }

    //
    //    If caller didn't specify a starting block number, then use the volume's
    //    next block to allocate from.
    //
    if (startingBlock == 0) {
        hfs_lock_mount (hfsmp);
        startingBlock = hfsmp->nextAllocation;
        hfs_unlock_mount(hfsmp);
        updateAllocPtr = true;
    }

    if (startingBlock >= hfsmp->allocLimit) {
        startingBlock = 0; /* overflow so start at beginning */
    }

    //
    //    If the request must be contiguous, then find a sequence of free blocks
    //    that is long enough.  Otherwise, find the first free block.
    //
    if (forceContiguous) {
        err = BlockFindContig(hfsmp, startingBlock, minBlocks, maxBlocks,
                              flags, &extent->startBlock, &extent->blockCount);
        /*
         * If we allocated from a new position then also update the roving allocator.
         * This will keep the roving allocation pointer up-to-date even
         * if we are using the new R/B tree allocator, since
         * it doesn't matter to us here, how the underlying allocator found
         * the block to vend out.
         */
        if ((err == noErr) &&
            (extent->startBlock > startingBlock) &&
            ((extent->startBlock < hfsmp->hfs_metazone_start) ||
             (extent->startBlock > hfsmp->hfs_metazone_end))) {
                updateAllocPtr = true;
            }
    } else {
        /*
         * Scan the bitmap once, gather the N largest free extents, then
         * allocate from these largest extents.  Repeat as needed until
         * we get all the space we needed.  We could probably build up
         * that list when the higher level caller tried (and failed) a
         * contiguous allocation first.
         *
         * Note that the free-extent cache will be cease to be updated if
         * we are using the red-black tree for allocations.  If we jettison
         * the tree, then we will reset the free-extent cache and start over.
         */

        /* Disable HFS_ALLOC_FLUSHTXN if needed */
        if (forceFlush) {
            flags &= ~HFS_ALLOC_FLUSHTXN;
        }

        /*
         * BlockFindKnown only examines the free extent cache; anything in there will
         * have been committed to stable storage already.
         */
        err = BlockFindKnown(hfsmp, maxBlocks, &extent->startBlock,
                             &extent->blockCount);

        /* dskFulErr out of BlockFindKnown indicates an empty Free Extent Cache */

        if (err == dskFulErr) {
            /*
             * Now we have to do a bigger scan.  Start at startingBlock and go up until the
             * allocation limit.  We 'trust' the summary bitmap in this call, if it tells us
             * that it could not find any free space.
             */
            err = BlockFindAny(hfsmp, startingBlock, hfsmp->allocLimit,
                               maxBlocks, flags, true,
                               &extent->startBlock, &extent->blockCount);
        }
        if (err == dskFulErr) {
            /*
             * Vary the behavior here if the summary table is on or off.
             * If it is on, then we don't trust it it if we get into this case and
             * basically do a full scan for maximum coverage.
             * If it is off, then we trust the above and go up until the startingBlock.
             */
            if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
                err = BlockFindAny(hfsmp, 1, hfsmp->allocLimit, maxBlocks,
                                   flags, false,
                                   &extent->startBlock, &extent->blockCount);
            }
            else {
                err = BlockFindAny(hfsmp, 1, startingBlock, maxBlocks,
                                   flags, false,
                                   &extent->startBlock, &extent->blockCount);
            }

            /*
             * Last Resort: Find/use blocks that may require a journal flush.
             */
            if (err == dskFulErr && forceFlush) {
                flags |= HFS_ALLOC_FLUSHTXN;
                err = BlockFindAny(hfsmp, 1, hfsmp->allocLimit, maxBlocks,
                                   flags, false,
                                   &extent->startBlock, &extent->blockCount);
            }
        }
    }

    if (err)
        goto exit;

mark_allocated:

    // Handle alignment
    if (ap && ap->alignment && extent->blockCount < ap->max_blocks) {
        /*
         * See the comment in FileMgrInternal.h for alignment
         * semantics.
         */
        uint32_t rounding = ((extent->blockCount + ap->alignment_offset)
                             % ap->alignment);

        // @minBlocks is still the minimum
        if (extent->blockCount >= minBlocks + rounding)
            extent->blockCount -= rounding;
    }

    err = BlockMarkAllocatedInternal(hfsmp, extent->startBlock,
                                     extent->blockCount, flags);

    if (err)
        goto exit;

    // if we actually allocated something then go update the
    // various bits of state that we maintain regardless of
    // whether there was an error (i.e. partial allocations
    // still need to update things like the free block count).
    //
    if (extent->blockCount != 0) {
        //
        //    If we used the volume's roving allocation pointer, then we need to update it.
        //    Adding in the length of the current allocation might reduce the next allocate
        //    call by avoiding a re-scan of the already allocated space.  However, the clump
        //    just allocated can quite conceivably end up being truncated or released when
        //    the file is closed or its EOF changed.  Leaving the allocation pointer at the
        //    start of the last allocation will avoid unnecessary fragmentation in this case.
        //
        hfs_lock_mount (hfsmp);

        if (!ISSET(flags, HFS_ALLOC_USE_TENTATIVE | HFS_ALLOC_COMMIT)) {
            lf_lck_spin_lock(&hfsmp->vcbFreeExtLock);
            if (hfsmp->vcbFreeExtCnt == 0 && hfsmp->hfs_freed_block_count == 0) {
                hfsmp->sparseAllocation = extent->startBlock;
            }
            lf_lck_spin_unlock(&hfsmp->vcbFreeExtLock);
            if (extent->blockCount < hfsmp->hfs_freed_block_count) {
                hfsmp->hfs_freed_block_count -= extent->blockCount;
            } else {
                hfsmp->hfs_freed_block_count = 0;
            }

            if (updateAllocPtr &&
                ((extent->startBlock < hfsmp->hfs_metazone_start) ||
                 (extent->startBlock > hfsmp->hfs_metazone_end))) {
                    HFS_UPDATE_NEXT_ALLOCATION(hfsmp, extent->startBlock);
                }

            (void) remove_free_extent_cache(hfsmp, extent->startBlock, extent->blockCount);
        }

        if (ISSET(flags, HFS_ALLOC_USE_TENTATIVE)) {
            if (ap == NULL || ap->reservation_in == NULL) {
                err = paramErr;
                goto exit;
            }
            (*ap->reservation_in)->rl_start += extent->blockCount;
            hfsmp->tentativeBlocks -= extent->blockCount;
            if (rl_len(*ap->reservation_in) <= 0)
                hfs_free_tentative(hfsmp, ap->reservation_in);
        } else if (ISSET(flags, HFS_ALLOC_COMMIT)) {
            // Handle committing locked extents
            hfs_assert(hfsmp->lockedBlocks >= extent->blockCount);
            (*ap->reservation_in)->rl_start += extent->blockCount;
            hfsmp->lockedBlocks -= extent->blockCount;
            hfs_free_locked(hfsmp, ap->reservation_in);
        }

        /*
         * Update the number of free blocks on the volume
         *
         * Skip updating the free blocks count if the block are
         * being allocated to relocate data as part of hfs_truncatefs()
         */

        if (ISSET(flags, HFS_ALLOC_TENTATIVE)) {
            hfsmp->tentativeBlocks += extent->blockCount;
        } else if (ISSET(flags, HFS_ALLOC_LOCKED)) {
            hfsmp->lockedBlocks += extent->blockCount;
        } else if ((flags & HFS_ALLOC_SKIPFREEBLKS) == 0) {
            hfsmp->freeBlocks -= extent->blockCount;
        }
        MarkVCBDirty(hfsmp);
        hfs_unlock_mount(hfsmp);

        if (ISSET(flags, HFS_ALLOC_TENTATIVE)) {
            hfs_assert(ap);
            add_to_reserved_list(hfsmp, extent->startBlock, extent->blockCount,
                                 0, ap->reservation_out);
        } else if (ISSET(flags, HFS_ALLOC_LOCKED)) {
            hfs_assert(ap);
            add_to_reserved_list(hfsmp, extent->startBlock, extent->blockCount,
                                 1, ap->reservation_out);
        }

        if (ISSET(flags, HFS_ALLOC_IGNORE_TENTATIVE)) {
            /*
             * See if we used tentative blocks.  Note that we cannot
             * free the reservations here because we don't have access
             * to the external pointers.  All we can do is update the
             * reservations and they'll be cleaned up when whatever is
             * holding the pointers calls us back.
             *
             * We use the rangelist code to detect overlaps and
             * constrain the tentative block allocation.  Note that
             * @end is inclusive so that our rangelist code will
             * resolve the various cases for us.  As a result, we need
             * to ensure that we account for it properly when removing
             * the blocks from the tentative count in the mount point
             * and re-inserting the remainder (either head or tail)
             */
            struct rl_entry *range, *next_range;
            struct rl_head *ranges = &hfsmp->hfs_reserved_ranges[HFS_TENTATIVE_BLOCKS];
            const uint32_t start = extent->startBlock;
            const uint32_t end = start + extent->blockCount - 1;
            TAILQ_FOREACH_SAFE(range, ranges, rl_link, next_range) {
                switch (rl_overlap(range, start, end)) {
                    case RL_OVERLAPCONTAINSRANGE:
                        // Keep the bigger part
                        if (start - range->rl_start > range->rl_end - end) {
                            // Discard the tail
                            hfsmp->tentativeBlocks -= range->rl_end + 1 - start;
                            hfs_release_summary(hfsmp, end + 1, (uint32_t)(range->rl_end - end));
                            const uint32_t old_end = (uint32_t)range->rl_end;
                            range->rl_end = start - 1;
                            add_free_extent_cache(hfsmp, end + 1, old_end - end);
                        } else {
                            // Discard the head
                            hfsmp->tentativeBlocks -= end + 1 - range->rl_start;
                            hfs_release_summary(hfsmp, (uint32_t)range->rl_start, (uint32_t)(start - range->rl_start));
                            const uint32_t old_start = (uint32_t)range->rl_start;
                            range->rl_start = end + 1;
                            add_free_extent_cache(hfsmp, old_start,
                                                  start - old_start);
                        }
                        hfs_assert(range->rl_end >= range->rl_start);
                        break;
                    case RL_MATCHINGOVERLAP:
                    case RL_OVERLAPISCONTAINED:
                        hfsmp->tentativeBlocks -= rl_len(range);
                        range->rl_end = range->rl_start - 1;
                        hfs_release_reserved(hfsmp, range, HFS_TENTATIVE_BLOCKS);
                        break;
                    case RL_OVERLAPSTARTSBEFORE:
                        hfsmp->tentativeBlocks -= range->rl_end + 1 - start;
                        range->rl_end = start - 1;
                        hfs_assert(range->rl_end >= range->rl_start);
                        break;
                    case RL_OVERLAPENDSAFTER:
                        hfsmp->tentativeBlocks -= end + 1 - range->rl_start;
                        range->rl_start = end + 1;
                        hfs_assert(range->rl_end >= range->rl_start);
                        break;
                    case RL_NOOVERLAP:
                        break;
                }
            }
        }
    }

exit:

    if (ALLOC_DEBUG) {
        if (err == noErr) {
            if (extent->startBlock >= hfsmp->totalBlocks) {
                LFHFS_LOG(LEVEL_ERROR, "BlockAllocate: vending invalid blocks!");
                hfs_assert(0);
            }
            if (extent->startBlock >= hfsmp->allocLimit) {
                LFHFS_LOG(LEVEL_ERROR, "BlockAllocate: vending block past allocLimit!");
                hfs_assert(0);
            }

            if ((extent->startBlock + extent->blockCount) >= hfsmp->totalBlocks) {
                LFHFS_LOG(LEVEL_ERROR, "BlockAllocate: vending too many invalid blocks!");
                hfs_assert(0);
            }

            if ((extent->startBlock + extent->blockCount) >= hfsmp->allocLimit) {
                LFHFS_LOG(LEVEL_ERROR, "BlockAllocate: vending too many invalid blocks past allocLimit!");
                hfs_assert(0);
            }
        }
    }

    if (err) {
        // Just to be safe...
        extent->startBlock = 0;
        extent->blockCount = 0;
    }

    // KBZ : For now, make sure clusters fills with zeros.
    raw_readwrite_zero_fill_fill( hfsmp, extent->startBlock, extent->blockCount );

    return err;
}


/*
 ;________________________________________________________________________________
 ;
 ; Routine:       BlockDeallocate
 ;
 ; Function:    Update the bitmap to deallocate a run of disk allocation blocks
 ;
 ; Input Arguments:
 ;     vcb        - Pointer to ExtendedVCB for the volume to free space on
 ;     firstBlock    - First allocation block to be freed
 ;     numBlocks    - Number of allocation blocks to free up (must be > 0!)
 ;
 ; Output:
 ;     (result)    - Result code
 ;
 ; Side effects:
 ;     The volume bitmap is read and updated; the volume bitmap cache may be changed.
 ;     The Allocator's red-black trees may also be modified as a result.
 ;
 ;________________________________________________________________________________
 */

OSErr BlockDeallocate (
                       ExtendedVCB        *vcb,            //    Which volume to deallocate space on
                       u_int32_t        firstBlock,        //    First block in range to deallocate
                       u_int32_t        numBlocks,         //    Number of contiguous blocks to deallocate
                       hfs_block_alloc_flags_t flags)
{
    if (ISSET(flags, HFS_ALLOC_TENTATIVE | HFS_ALLOC_LOCKED))
        return 0;

    OSErr            err;
    struct hfsmount *hfsmp;
    hfsmp = VCBTOHFS(vcb);

    //
    //    If no blocks to deallocate, then exit early
    //
    if (numBlocks == 0) {
        err = noErr;
        goto Exit;
    }


    if (ALLOC_DEBUG) {
        if (firstBlock >= hfsmp->totalBlocks) {
            LFHFS_LOG(LEVEL_ERROR, "BlockDeallocate: freeing invalid blocks!");
            hfs_assert(0);
        }

        if ((firstBlock + numBlocks) >= hfsmp->totalBlocks) {
            LFHFS_LOG(LEVEL_ERROR, "BlockDeallocate: freeing too many invalid blocks!");
            hfs_assert(0);
        }
    }

    /*
     * If we're using the summary bitmap, then try to mark the bits
     * as potentially usable/free before actually deallocating them.
     * It is better to be slightly speculative here for correctness.
     */

    (void) hfs_release_summary (hfsmp, firstBlock, numBlocks);

    err = BlockMarkFreeInternal(vcb, firstBlock, numBlocks, true);

    if (err) {
        goto Exit;
    }

    //
    //    Update the volume's free block count, and mark the VCB as dirty.
    //
    hfs_lock_mount(hfsmp);
    /*
     * Do not update the free block count.  This flags is specified
     * when a volume is being truncated.
     */
    if ((flags & HFS_ALLOC_SKIPFREEBLKS) == 0) {
        vcb->freeBlocks += numBlocks;
    }

    vcb->hfs_freed_block_count += numBlocks;

    if (vcb->nextAllocation == (firstBlock + numBlocks)) {
        HFS_UPDATE_NEXT_ALLOCATION(vcb, (vcb->nextAllocation - numBlocks));
    }

    if (hfsmp->jnl == NULL)
    {
        /*
         * In the journal case, we'll add the free extent once the journal
         * calls us back to tell us it wrote the transaction to disk.
         */
        (void) add_free_extent_cache(vcb, firstBlock, numBlocks);

        /*
         * If the journal case, we'll only update sparseAllocation once the
         * free extent cache becomes empty (when we remove the last entry
         * from the cache).  Skipping it here means we're less likely to
         * find a recently freed extent via the bitmap before it gets added
         * to the free extent cache.
         */
        if (firstBlock < vcb->sparseAllocation) {
            vcb->sparseAllocation = firstBlock;
        }
    }

    MarkVCBDirty(vcb);
    hfs_unlock_mount(hfsmp);

Exit:

    return err;
}


u_int8_t freebitcount[16] = {
    4, 3, 3, 2, 3, 2, 2, 1,  /* 0 1 2 3 4 5 6 7 */
    3, 2, 2, 1, 2, 1, 1, 0,  /* 8 9 A B C D E F */
};

u_int32_t
MetaZoneFreeBlocks(ExtendedVCB *vcb)
{
    u_int32_t freeblocks;
    u_int32_t *currCache;
    GenericLFBufPtr blockRef;
    u_int32_t bit;
    u_int32_t lastbit;
    int bytesleft;
    int bytesperblock;
    u_int8_t byte;
    u_int8_t *buffer;

    blockRef = 0;
    bytesleft = freeblocks = 0;
    buffer = NULL;
    bit = VCBTOHFS(vcb)->hfs_metazone_start;
    if (bit == 1)
        bit = 0;

    lastbit = VCBTOHFS(vcb)->hfs_metazone_end;
    bytesperblock = vcb->vcbVBMIOSize;

    /*
     *  Count all the bits from bit to lastbit.
     */
    while (bit < lastbit) {
        /*
         *  Get next bitmap block.
         */
        if (bytesleft == 0) {
            if (blockRef) {
                (void) ReleaseBitmapBlock(vcb, blockRef, false);
                blockRef = 0;
            }
            if (ReadBitmapBlock(vcb, bit, &currCache, &blockRef,
                                HFS_ALLOC_IGNORE_TENTATIVE) != 0) {
                return (0);
            }
            buffer = (u_int8_t *)currCache;
            bytesleft = bytesperblock;
        }
        byte = *buffer++;
        freeblocks += freebitcount[byte & 0x0F];
        freeblocks += freebitcount[(byte >> 4) & 0x0F];
        bit += kBitsPerByte;
        --bytesleft;
    }
    if (blockRef)
        (void) ReleaseBitmapBlock(vcb, blockRef, false);

    return (freeblocks);
}


/*
 * Obtain the next allocation block (bit) that's
 * outside the metadata allocation zone.
 */
static u_int32_t NextBitmapBlock(
                                 ExtendedVCB        *vcb,
                                 u_int32_t        bit)
{
    struct  hfsmount *hfsmp = VCBTOHFS(vcb);

    if ((hfsmp->hfs_flags & HFS_METADATA_ZONE) == 0)
        return (bit);
    /*
     * Skip over metadata allocation zone.
     */
    if ((bit >= hfsmp->hfs_metazone_start) &&
        (bit <= hfsmp->hfs_metazone_end)) {
        bit = hfsmp->hfs_metazone_end + 1;
    }
    return (bit);
}


// Assumes @bitmap is aligned to 8 bytes and multiple of 8 bytes.
static void bits_set(void *bitmap, int start, int end)
{
    const int start_bit = start & 63;
    const int end_bit   = end   & 63;

#define LEFT_MASK(bit)    OSSwapHostToBigInt64(0xffffffffffffffffull << (64 - bit))
#define RIGHT_MASK(bit)    OSSwapHostToBigInt64(0xffffffffffffffffull >> bit)

    uint64_t *p = (uint64_t *)bitmap + start / 64;

    if ((start & ~63) == (end & ~63)) {
        // Start and end in same 64 bits
        *p |= RIGHT_MASK(start_bit) & LEFT_MASK(end_bit);
    } else {
        *p++ |= RIGHT_MASK(start_bit);

        int nquads = (end - end_bit - start - 1) / 64;

        while (nquads--)
            *p++ = 0xffffffffffffffffull;

        if (end_bit)
            *p |= LEFT_MASK(end_bit);
    }
}

// Modifies the buffer and applies any reservations that we might have
static GenericLFBufPtr process_reservations(hfsmount_t *hfsmp, GenericLFBufPtr bp, off_t offset, hfs_block_alloc_flags_t flags, bool always_copy)
{

#if 0
    bool taken_copy = false;
#else
#pragma unused (always_copy)
#endif

    void *buffer = bp->pvData;
    const uint64_t nbytes = bp->uValidBytes;
    const off_t end = offset + nbytes * 8 - 1;

    for (int i = (ISSET(flags, HFS_ALLOC_IGNORE_TENTATIVE)
                  ? HFS_LOCKED_BLOCKS : HFS_TENTATIVE_BLOCKS); i < 2; ++i) {
        struct rl_entry *entry;
        TAILQ_FOREACH(entry, &hfsmp->hfs_reserved_ranges[i], rl_link) {
            uint32_t a, b;

            enum rl_overlaptype overlap_type = rl_overlap(entry, offset, end);

            if (overlap_type == RL_NOOVERLAP)
                continue;

#if 0
            /*
             * If always_copy is false, we only take a copy if B_LOCKED is
             * set because ReleaseScanBitmapRange doesn't invalidate the
             * buffer in that case.
             */
            if (!taken_copy && (always_copy || ISSET(buf_flags(bp), B_LOCKED))) {
                buf_t new_bp = buf_create_shadow(bp, true, 0, NULL, NULL);
                buf_brelse(bp);
                bp = new_bp;
                buf_setflags(bp, B_NOCACHE);
                buffer = (void *)buf_dataptr(bp);
                taken_copy = true;
            }
#endif
            switch (overlap_type) {
                case RL_OVERLAPCONTAINSRANGE:
                case RL_MATCHINGOVERLAP:
                    memset(buffer, 0xff, nbytes);
                    return bp;
                case RL_OVERLAPISCONTAINED:
                    a = (uint32_t)entry->rl_start;
                    b = (uint32_t)entry->rl_end;
                    break;
                case RL_OVERLAPSTARTSBEFORE:
                    a = (uint32_t)offset;
                    b = (uint32_t)entry->rl_end;
                    break;
                case RL_OVERLAPENDSAFTER:
                    a = (uint32_t)entry->rl_start;
                    b = (uint32_t)end;
                    break;
                case RL_NOOVERLAP:
                    __builtin_unreachable();
            }

            a -= offset;
            b -= offset;

            hfs_assert(b >= a);

            // b is inclusive
            bits_set(buffer, a, b + 1);
        }
    } // for (;;)

    return bp;
}

/*
 ;_______________________________________________________________________
 ;
 ; Routine:    ReadBitmapBlock
 ;
 ; Function:    Read in a bitmap block corresponding to a given allocation
 ;            block (bit).  Return a pointer to the bitmap block.
 ;
 ; Inputs:
 ;    vcb            --    Pointer to ExtendedVCB
 ;    bit            --    Allocation block whose bitmap block is desired
 ;
 ; Outputs:
 ;    buffer        --    Pointer to bitmap block corresonding to "block"
 ;    blockRef
 ;_______________________________________________________________________
 */
static OSErr ReadBitmapBlock(ExtendedVCB        *vcb,
                             u_int32_t        bit,
                             u_int32_t        **buffer,
                             GenericLFBufPtr  *blockRef,
                             hfs_block_alloc_flags_t flags)
{
    OSErr           err = 0;
    GenericLFBufPtr bp  = NULL;
    struct vnode    *vp = NULL;
    daddr64_t       block;
    u_int32_t       blockSize;

    /*
     * volume bitmap blocks are protected by the allocation file lock
     */
    REQUIRE_FILE_LOCK(vcb->hfs_allocation_vp, false);

    blockSize = (u_int32_t)vcb->vcbVBMIOSize;
    if (blockSize == 0) return EINVAL; //Devision protection
    block = (daddr64_t)(bit / (blockSize * kBitsPerByte));

    /* HFS+ / HFSX */
    vp = vcb->hfs_allocation_vp;    /* use allocation file vnode */

    bp = lf_hfs_generic_buf_allocate(vp, block, blockSize, 0);
    err = lf_hfs_generic_buf_read(bp);

    if ( err )
    {
        lf_hfs_generic_buf_release(bp);
        *blockRef = NULL;
        *buffer = NULL;
    }
    else
    {
        if (!ISSET(flags, HFS_ALLOC_IGNORE_RESERVED)) {
            bp = process_reservations(vcb, bp, block * blockSize * 8, flags, /* always_copy: */ true);
        }
        
        bp->uFlags = flags;

        *blockRef = bp;
        *buffer = bp->pvData;
    }

    return err;
}


/*
 ;_______________________________________________________________________
 ;
 ; Routine:    ReadBitmapRange
 ;
 ; Function:    Read in a range of the bitmap starting at the given offset.
 ;            Use the supplied size to determine the amount of I/O to generate
 ;            against the bitmap file. Return a pointer to the bitmap block.
 ;
 ; Inputs:
 ;    hfsmp        --    Pointer to hfs mount
 ;    offset        --    byte offset into the bitmap file
 ;    size        --  How much I/O to generate against the bitmap file.
 ;
 ; Outputs:
 ;    buffer        --    Pointer to bitmap block data corresonding to "block"
 ;    blockRef    --  struct 'buf' pointer which MUST be released in a subsequent call.
 ;_______________________________________________________________________
 */
static OSErr ReadBitmapRange(struct hfsmount *hfsmp, uint32_t offset, uint32_t iosize, uint32_t **buffer, GenericLFBuf **blockRef)
{

    OSErr err           = 0;
    GenericLFBufPtr bp  = NULL;
    struct vnode    *vp = NULL;
    daddr64_t block;

    /*
     * volume bitmap blocks are protected by the allocation file lock
     */
    REQUIRE_FILE_LOCK(hfsmp->hfs_allocation_vp, false);

    vp = hfsmp->hfs_allocation_vp;    /* use allocation file vnode */

    /*
     * The byte offset argument must be converted into bitmap-relative logical
     * block numbers before using it in buf_meta_bread.
     *
     * lf_hfs_generic_buf_read (and the things it calls) will eventually try to
     * reconstruct the byte offset into the file by multiplying the logical
     * block number passed in below by the given iosize.
     * So we prepare for that by converting the byte offset back into
     * logical blocks in terms of iosize units.
     *
     * The amount of I/O requested and the byte offset should be computed
     * based on the helper function in the frame that called us, so we can
     * get away with just doing a simple divide here.
     */
    block = (daddr64_t)(offset / iosize);

    bp = lf_hfs_generic_buf_allocate(vp, block, iosize, 0);
    err = lf_hfs_generic_buf_read(bp);

    if ( err )
    {
        lf_hfs_generic_buf_release(bp);
        *blockRef = 0;
        *buffer = NULL;
    }
    else
    {
        bp = process_reservations(hfsmp, bp, (offset * 8), 0, /* always_copy: */ false);
        *blockRef = bp;
        *buffer = bp->pvData;
    }

    return err;
}


/*
 ;_______________________________________________________________________
 ;
 ; Routine:    ReleaseBitmapBlock
 ;
 ; Function:    Relase a bitmap block.
 ;
 ; Inputs:
 ;    vcb
 ;    blockRef
 ;    dirty
 ;_______________________________________________________________________
 */
static OSErr ReleaseBitmapBlock( ExtendedVCB *vcb, GenericLFBufPtr blockRef, Boolean dirty)
{

    GenericLFBufPtr bp = blockRef;

    if (blockRef == 0) {
        if (dirty)
        {
            LFHFS_LOG(LEVEL_ERROR, "ReleaseBitmapBlock: missing bp");
            hfs_assert(0);
        }
        return (0);
    }

    if (bp)
    {
        if (dirty)
        {
            hfs_block_alloc_flags_t flags = (uint32_t)bp->uFlags;

            if (!ISSET(flags, HFS_ALLOC_IGNORE_RESERVED))
            {
                LFHFS_LOG(LEVEL_ERROR, "Modified read-only bitmap buffer!");
                hfs_assert(0);
            }

            struct hfsmount *hfsmp = VCBTOHFS(vcb);
            if (hfsmp->jnl)
            {
                journal_modify_block_end(hfsmp->jnl, bp, NULL, NULL);
            }
            else
            {
                lf_hfs_generic_buf_write(bp);
                lf_hfs_generic_buf_release(bp);
            }
        } else {
            lf_hfs_generic_buf_release(bp);
        }
    }

    return (0);
}

/*
 * ReleaseScanBitmapRange
 *
 * This is used to release struct bufs that were created for use by
 * bitmap scanning code.  Because they may be of sizes different than the
 * typical runtime manipulation code, we want to force them to be purged out
 * of the buffer cache ASAP, so we'll release them differently than in the
 * ReleaseBitmapBlock case.
 *
 * Additionally, because we know that we're only reading the blocks and that they
 * should have been clean prior to reading them, we will never
 * issue a write to them (thus dirtying them).
 */

static OSErr ReleaseScanBitmapRange( GenericLFBufPtr bp )
{
    if (bp)
    {
        lf_hfs_generic_buf_release(bp);
    }

    return (0);
}

/*
 * @extent.startBlock, on input, contains a preferred block for the
 * allocation.  @extent.blockCount, on input, contains the minimum
 * number of blocks acceptable.  Upon success, the result is conveyed
 * in @extent.
 */
static OSErr hfs_alloc_try_hard(hfsmount_t *hfsmp,
                                HFSPlusExtentDescriptor *extent,
                                uint32_t max_blocks,
                                hfs_block_alloc_flags_t flags)
{
    OSErr err = dskFulErr;

    const uint32_t min_blocks = extent->blockCount;

    // It's > rather than >= because the last block is always reserved
    if (extent->startBlock > 0 && extent->startBlock < hfsmp->allocLimit
        && hfsmp->allocLimit - extent->startBlock > max_blocks) {
        /*
         * This is just checking to see if there's an extent starting
         * at extent->startBlock that will suit.  We only check for
         * @max_blocks here; @min_blocks is ignored.
         */

        err = BlockFindContiguous(hfsmp, extent->startBlock, extent->startBlock + max_blocks,
                                  max_blocks, max_blocks, true, true,
                                  &extent->startBlock, &extent->blockCount, flags);

        if (err != dskFulErr)
            return err;
    }

    err = BlockFindKnown(hfsmp, max_blocks, &extent->startBlock,
                         &extent->blockCount);

    if (!err) {
        if (extent->blockCount >= max_blocks)
            return 0;
    } else if (err != dskFulErr)
        return err;

    // Try a more exhaustive search
    return BlockFindContiguous(hfsmp, 1, hfsmp->allocLimit,
                               min_blocks, max_blocks,
                               /* useMetaZone: */ true,
                               /* trustSummary: */ true,
                               &extent->startBlock, &extent->blockCount, flags);
}

/*
 _______________________________________________________________________

 Routine:    BlockFindContig

 Function:   Find a contiguous group of allocation blocks.  If the
 minimum cannot be satisfied, nothing is returned.  The
 caller guarantees that there are enough free blocks
 (though they may not be contiguous, in which case this
 call will fail).

 Inputs:
 vcb                Pointer to volume where space is to be allocated
 startingBlock    Preferred first block for allocation
 minBlocks        Minimum number of contiguous blocks to allocate
 maxBlocks        Maximum number of contiguous blocks to allocate
 flags

 Outputs:
 actualStartBlock    First block of range allocated, or 0 if error
 actualNumBlocks        Number of blocks allocated, or 0 if error
 _______________________________________________________________________
 */
static OSErr BlockFindContig(
                             ExtendedVCB        *vcb,
                             u_int32_t        startingBlock,
                             u_int32_t        minBlocks,
                             u_int32_t        maxBlocks,
                             hfs_block_alloc_flags_t flags,
                             u_int32_t        *actualStartBlock,
                             u_int32_t        *actualNumBlocks)
{
    OSErr retval = noErr;
    uint32_t currentStart = startingBlock;

    uint32_t foundStart = 0; // values to emit to caller
    uint32_t foundCount = 0;

    uint32_t collision_start = 0;  // if we have to re-allocate a recently deleted extent, use this
    uint32_t collision_count = 0;

    int allowReuse = (flags & HFS_ALLOC_FLUSHTXN);
    Boolean useMetaZone = (flags & HFS_ALLOC_METAZONE);

    struct hfsmount *hfsmp = VCBTOHFS(vcb);

    while ((retval == noErr) && (foundStart == 0) && (foundCount == 0)) {

        /* Try and find something that works. */

        /*
         * NOTE: If the only contiguous free extent of at least minBlocks
         * crosses startingBlock (i.e. starts before, ends after), then we
         * won't find it. Earlier versions *did* find this case by letting
         * the second search look past startingBlock by minBlocks.  But
         * with the free extent cache, this can lead to duplicate entries
         * in the cache, causing the same blocks to be allocated twice.
         */
        retval = BlockFindContiguous(vcb, currentStart, vcb->allocLimit, minBlocks,
                                     maxBlocks, useMetaZone, true, &foundStart, &foundCount, flags);

        if (retval == dskFulErr && currentStart != 0) {
            /*
             * We constrain the endingBlock so we don't bother looking for ranges
             * that would overlap those found in the previous call, if the summary bitmap
             * is not on for this volume.  If it is, then we assume that it was not trust
             * -worthy and do a full scan.
             */
            if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
                retval = BlockFindContiguous(vcb, 1, vcb->allocLimit, minBlocks,
                                             maxBlocks, useMetaZone, false, &foundStart, &foundCount, flags);
            }
            else {
                retval = BlockFindContiguous(vcb, 1, currentStart, minBlocks,
                                             maxBlocks, useMetaZone, false, &foundStart, &foundCount, flags);
            }
        }

        if (retval != noErr) {
            goto bailout;
        }

        /* Do we overlap with the recently found collision extent? */
        if (collision_start) {
            if (extents_overlap (foundStart, foundCount, collision_start, collision_count)) {
                /*
                 * We've looped around, and the only thing we could use was the collision extent.
                 * Since we are allowed to use it, go ahead and do so now.
                 */
                if(allowReuse) {
                    /*
                     * then we couldn't find anything except values which might have been
                     * recently deallocated. just return our cached value if we are allowed to.
                     */
                    foundStart = collision_start;
                    foundCount = collision_count;
                    goto bailout;
                }
                else {
                    /* Otherwise, we looped around and couldn't find anything that wouldn't require a journal flush. */
                    retval = dskFulErr;
                    goto bailout;
                }
            }
        }
        /*
         * If we found something good, we'd break out of the loop at the top; foundCount
         * and foundStart should be set.
         */

    } // end while loop.

bailout:

    if (retval == noErr) {
        *actualStartBlock = foundStart;
        *actualNumBlocks = foundCount;
    }

    return retval;

}


/*
 _______________________________________________________________________

 Routine:    BlockFindAny

 Function: Find one or more allocation blocks and may return fewer than
 requested.  The caller guarantees that there is at least one
 free block.

 Inputs:
 vcb                Pointer to volume where space is to be allocated
 startingBlock    Preferred first block for allocation
 endingBlock        Last block to check + 1
 maxBlocks        Maximum number of contiguous blocks to allocate
 useMetaZone

 Outputs:
 actualStartBlock    First block of range allocated, or 0 if error
 actualNumBlocks        Number of blocks allocated, or 0 if error
 _______________________________________________________________________
 */

static OSErr BlockFindAny(
                          ExtendedVCB        *vcb,
                          u_int32_t        startingBlock,
                          register u_int32_t    endingBlock,
                          u_int32_t        maxBlocks,
                          hfs_block_alloc_flags_t flags,
                          Boolean            trustSummary,
                          u_int32_t        *actualStartBlock,
                          u_int32_t        *actualNumBlocks)
{

    /*
     * If it is enabled, scan through the summary table to find the first free block.
     *
     * If it reports that there are not any free blocks, we could have a false
     * positive, so in that case, use the input arguments as a pass through.
     */
    uint32_t start_blk  = startingBlock;
    uint32_t end_blk = endingBlock;
    struct hfsmount *hfsmp;
    OSErr err;

    hfsmp = (struct hfsmount*)vcb;
    if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
        uint32_t suggested_start;

        /*
         * If the summary table is enabled, scan through it to find the first free
         * block.  If there was an error, or we couldn't find anything free in the
         * summary table, then just leave the start_blk fields unmodified. We wouldn't
         * have gotten to this point if the mount point made it look like there was possibly
         * free space in the FS.
         */
        err = hfs_find_summary_free (hfsmp, startingBlock, &suggested_start);
        if (err == 0) {
            start_blk = suggested_start;
        }
        else {
            /* Differentiate between ENOSPC and a more esoteric error in the above call. */
            if ((err == ENOSPC) && (trustSummary)) {
                /*
                 * The 'trustSummary' argument is for doing a full scan if we really
                 * really, need the space and we think it's somewhere but can't find it in the
                 * summary table. If it's true, then we trust the summary table and return
                 * dskFulErr if we couldn't find it above.
                 */
                return dskFulErr;
            }
            /*
             * If either trustSummary was false or we got a different errno, then we
             * want to fall through to the real bitmap single i/o code...
             */
        }
    }

    err =  BlockFindAnyBitmap(vcb, start_blk, end_blk, maxBlocks,
                              flags, actualStartBlock, actualNumBlocks);

    return err;
}


/*
 * BlockFindAnyBitmap finds free ranges by scanning the bitmap to
 * figure out where the free allocation blocks are.  Inputs and
 * outputs are the same as for BlockFindAny.
 */

static OSErr BlockFindAnyBitmap(
                                ExtendedVCB        *vcb,
                                u_int32_t        startingBlock,
                                register u_int32_t    endingBlock,
                                u_int32_t        maxBlocks,
                                hfs_block_alloc_flags_t flags,
                                u_int32_t        *actualStartBlock,
                                u_int32_t        *actualNumBlocks)
{
    OSErr            err;
    register u_int32_t    block = 0;        //    current block number
    register u_int32_t    currentWord;    //    Pointer to current word within bitmap block
    register u_int32_t    bitMask;        //    Word with given bits already set (ready to OR in)
    register u_int32_t    wordsLeft;        //    Number of words left in this bitmap block
    u_int32_t  *buffer = NULL;
    u_int32_t  *currCache = NULL;
    GenericLFBufPtr  blockRef = 0;
    u_int32_t  bitsPerBlock;
    u_int32_t  wordsPerBlock;
    struct hfsmount *hfsmp = VCBTOHFS(vcb);
    Boolean useMetaZone = (flags & HFS_ALLOC_METAZONE);

    /*
     * When we're skipping the metadata zone and the start/end
     * range overlaps with the metadata zone then adjust the
     * start to be outside of the metadata zone.  If the range
     * is entirely inside the metadata zone then we can deny the
     * request (dskFulErr).
     */
    if (!useMetaZone && (vcb->hfs_flags & HFS_METADATA_ZONE)) {
        if (startingBlock <= vcb->hfs_metazone_end) {
            if (endingBlock > (vcb->hfs_metazone_end + 2))
                startingBlock = vcb->hfs_metazone_end + 1;
            else {
                err = dskFulErr;
                goto Exit;
            }
        }
    }

    //    Since this routine doesn't wrap around
    if (maxBlocks > (endingBlock - startingBlock)) {
        maxBlocks = endingBlock - startingBlock;
    }

    //
    //    Pre-read the first bitmap block
    //
    err = ReadBitmapBlock(vcb, startingBlock, &currCache, &blockRef, flags);
    if (err != noErr) goto Exit;
    buffer = currCache;

    //
    //    Set up the current position within the block
    //
    {
        u_int32_t wordIndexInBlock;

        bitsPerBlock  = vcb->vcbVBMIOSize * kBitsPerByte;
        wordsPerBlock = vcb->vcbVBMIOSize / kBytesPerWord;

        wordIndexInBlock = (startingBlock & (bitsPerBlock-1)) / kBitsPerWord;
        buffer += wordIndexInBlock;
        wordsLeft = wordsPerBlock - wordIndexInBlock;
        currentWord = SWAP_BE32 (*buffer);
        bitMask = kHighBitInWordMask >> (startingBlock & kBitsWithinWordMask);
    }

    /*
     * While loop 1:
     *        Find the first unallocated block starting at 'block'
     */
    uint32_t summary_block_scan = 0;

    block=startingBlock;
    while (block < endingBlock) {
        if ((currentWord & bitMask) == 0)
            break;

        //    Next bit
        ++block;
        bitMask >>= 1;
        if (bitMask == 0) {
            //    Next word
            bitMask = kHighBitInWordMask;
            ++buffer;

            if (--wordsLeft == 0) {
                //    Next block
                buffer = currCache = NULL;
                if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
                    /*
                     * If summary_block_scan is non-zero, then we must have
                     * pulled a bitmap file block into core, and scanned through
                     * the entire thing.  Because we're in this loop, we are
                     * implicitly trusting that the bitmap didn't have any knowledge
                     * about this particular block.  As a result, update the bitmap
                     * (lazily, now that we've scanned it) with our findings that
                     * this particular block is completely used up.
                     */
                    if (summary_block_scan != 0) {
                        uint32_t summary_bit;
                        (void) hfs_get_summary_index (hfsmp, summary_block_scan, &summary_bit);
                        hfs_set_summary (hfsmp, summary_bit, 1);
                    }
                }

                err = ReleaseBitmapBlock(vcb, blockRef, false);
                if (err != noErr) goto Exit;

                /*
                 * Skip over metadata blocks.
                 */
                if (!useMetaZone) {
                    block = NextBitmapBlock(vcb, block);
                }
                if (block >= endingBlock) {
                    err = dskFulErr;
                    goto Exit;
                }

                err = ReadBitmapBlock(vcb, block, &currCache, &blockRef, flags);
                if (err != noErr) goto Exit;
                buffer = currCache;
                summary_block_scan = block;
                wordsLeft = wordsPerBlock;
            }
            currentWord = SWAP_BE32 (*buffer);
        }
    }

    //    Did we get to the end of the bitmap before finding a free block?
    //    If so, then couldn't allocate anything.
    if (block >= endingBlock) {
        err = dskFulErr;
        goto Exit;
    }

#if LF_HFS_CHECK_UNMAPPED
    /*
     * Don't move forward just yet.  Verify that either one of the following
     * two conditions is true:
     * 1) journaling is not enabled
     * 2) block is not currently on any pending TRIM list.
     */
    if (hfsmp->jnl != NULL && (forceFlush == false)) {
        int recently_deleted = 0;
        uint32_t nextblk;
        err = CheckUnmappedBytes (hfsmp, (uint64_t) block, 1, &recently_deleted, &nextblk);
        if ((err == 0) && (recently_deleted)) {

            /* release the bitmap block & unset currCache.  we may jump past it. */
            err = ReleaseBitmapBlock(vcb, blockRef, false);
            currCache = NULL;
            if (err != noErr) {
                goto Exit;
            }
            /* set our start to nextblk, and re-do the search. */
            startingBlock = nextblk;
            goto restartSearchAny;
        }
    }
#endif

    //    Return the first block in the allocated range
    *actualStartBlock = block;

    //    If we could get the desired number of blocks before hitting endingBlock,
    //    then adjust endingBlock so we won't keep looking.  Ideally, the comparison
    //    would be (block + maxBlocks) < endingBlock, but that could overflow.  The
    //    comparison below yields identical results, but without overflow.
    if (block < (endingBlock-maxBlocks)) {
        endingBlock = block + maxBlocks;    //    if we get this far, we've found enough
    }

    /*
     * While loop 2:
     *        Scan the bitmap, starting at 'currentWord' in the current
     *        bitmap block.  Continue iterating through the bitmap until
     *         either we hit an allocated block, or until we have accumuluated
     *        maxBlocks worth of bitmap.
     */

    /* Continue until we see an allocated block */
    while ((currentWord & bitMask) == 0) {
        //    Move to the next block.  If no more, then exit.
        ++block;
        if (block == endingBlock) {
            break;
        }

        //    Next bit
        bitMask >>= 1;
        if (bitMask == 0) {
            //    Next word
            bitMask = kHighBitInWordMask;
            ++buffer;

            if (--wordsLeft == 0) {
                //    Next block
                buffer = currCache = NULL;

                /* We're only reading the bitmap here, so mark it as clean */
                err = ReleaseBitmapBlock(vcb, blockRef, false);
                if (err != noErr) {
                    goto Exit;
                }

                /*
                 * Skip over metadata blocks.
                 */
                if (!useMetaZone) {
                    u_int32_t nextBlock;
                    nextBlock = NextBitmapBlock(vcb, block);
                    if (nextBlock != block) {
                        goto Exit;  /* allocation gap, so stop */
                    }
                }

                if (block >= endingBlock) {
                    goto Exit;
                }

                err = ReadBitmapBlock(vcb, block, &currCache, &blockRef, flags);
                if (err != noErr) {
                    goto Exit;
                }
                buffer = currCache;
                wordsLeft = wordsPerBlock;
            }
            currentWord = SWAP_BE32 (*buffer);
        }
    }

Exit:
    if (currCache) {
        /* Release the bitmap reference prior to marking bits in-use */
        (void) ReleaseBitmapBlock(vcb, blockRef, false);
        currCache = NULL;
    }

    if (err == noErr) {
        *actualNumBlocks = block - *actualStartBlock;

        // sanity check
        if ((*actualStartBlock + *actualNumBlocks) > vcb->allocLimit) {
            LFHFS_LOG(LEVEL_ERROR, "BlockFindAnyBitmap: allocation overflow on \"%s\"", vcb->vcbVN);
            hfs_assert(0);
        }
    }
    else {
        *actualStartBlock = 0;
        *actualNumBlocks = 0;
    }

    return err;
}


/*
 _______________________________________________________________________

 Routine:    BlockFindKnown

 Function:   Return a potential extent from the free extent cache.  The
 returned extent *must* be marked allocated and removed
 from the cache by the *caller*.

 Inputs:
 vcb                Pointer to volume where space is to be allocated
 maxBlocks        Maximum number of contiguous blocks to allocate

 Outputs:
 actualStartBlock    First block of range allocated, or 0 if error
 actualNumBlocks        Number of blocks allocated, or 0 if error

 Returns:
 dskFulErr        Free extent cache is empty
 _______________________________________________________________________
 */

static OSErr BlockFindKnown(
                            ExtendedVCB        *vcb,
                            u_int32_t        maxBlocks,
                            u_int32_t        *actualStartBlock,
                            u_int32_t        *actualNumBlocks)
{
    OSErr            err;
    u_int32_t        foundBlocks;
    struct hfsmount *hfsmp = VCBTOHFS(vcb);

    hfs_lock_mount (hfsmp);
    lf_lck_spin_lock(&vcb->vcbFreeExtLock);
    if ( vcb->vcbFreeExtCnt == 0 ||
        vcb->vcbFreeExt[0].blockCount == 0) {
        lf_lck_spin_unlock(&vcb->vcbFreeExtLock);
        hfs_unlock_mount(hfsmp);
        return dskFulErr;
    }
    lf_lck_spin_unlock(&vcb->vcbFreeExtLock);
    hfs_unlock_mount(hfsmp);

    lf_lck_spin_lock(&vcb->vcbFreeExtLock);

    //    Just grab up to maxBlocks of the first (largest) free exent.
    *actualStartBlock = vcb->vcbFreeExt[0].startBlock;
    foundBlocks = vcb->vcbFreeExt[0].blockCount;
    if (foundBlocks > maxBlocks)
        foundBlocks = maxBlocks;
    *actualNumBlocks = foundBlocks;

    lf_lck_spin_unlock(&vcb->vcbFreeExtLock);

    // sanity check
    if ((*actualStartBlock + *actualNumBlocks) > vcb->allocLimit)
    {
        LFHFS_LOG(LEVEL_ERROR, "BlockAllocateKnown() found allocation overflow on \"%s\"", vcb->vcbVN);
        hfs_mark_inconsistent(vcb, HFS_INCONSISTENCY_DETECTED);
        err = EIO;
    } else
        err = 0;

    return err;
}

/*
 * BlockMarkAllocated
 *
 * This is a wrapper function around the internal calls which will actually mark the blocks
 * as in-use.  It will mark the blocks in the red-black tree if appropriate.  We need to do
 * this logic here to avoid callers having to deal with whether or not the red-black tree
 * is enabled.
 */

OSErr BlockMarkAllocated(
                         ExtendedVCB        *vcb,
                         u_int32_t        startingBlock,
                         register u_int32_t    numBlocks)
{
    return BlockMarkAllocatedInternal(vcb, startingBlock, numBlocks, 0);
}


/*
 _______________________________________________________________________

 Routine:    BlockMarkAllocatedInternal

 Function:    Mark a contiguous group of blocks as allocated (set in the
 bitmap).  It assumes those bits are currently marked
 deallocated (clear in the bitmap).  Note that this function
 must be called regardless of whether or not the bitmap or
 tree-based allocator is used, as all allocations must correctly
 be marked on-disk.  If the tree-based approach is running, then
 this will be done before the node is removed from the tree.

 Inputs:
 vcb                Pointer to volume where space is to be allocated
 startingBlock    First block number to mark as allocated
 numBlocks        Number of blocks to mark as allocated
 _______________________________________________________________________
 */
static
OSErr BlockMarkAllocatedInternal (
                                  ExtendedVCB        *vcb,
                                  u_int32_t        startingBlock,
                                  u_int32_t    numBlocks,
                                  hfs_block_alloc_flags_t flags)
{
    OSErr            err;
    register u_int32_t    *currentWord;    //    Pointer to current word within bitmap block
    register u_int32_t    wordsLeft;        //    Number of words left in this bitmap block
    register u_int32_t    bitMask;        //    Word with given bits already set (ready to OR in)
    u_int32_t        firstBit;        //    Bit index within word of first bit to allocate
    u_int32_t        numBits;        //    Number of bits in word to allocate
    u_int32_t        *buffer = NULL;
    GenericLFBufPtr  blockRef = NULL;
    u_int32_t        bitsPerBlock;
    u_int32_t        wordsPerBlock;
    // XXXdbg
    struct hfsmount *hfsmp = VCBTOHFS(vcb);

#if DEBUG

    if (!ISSET(flags, HFS_ALLOC_COMMIT)
        || ISSET(flags, HFS_ALLOC_USE_TENTATIVE)) {
        struct rl_entry *range;
        TAILQ_FOREACH(range, &hfsmp->hfs_reserved_ranges[HFS_LOCKED_BLOCKS], rl_link) {
            hfs_assert(rl_overlap(range, startingBlock,
                                  startingBlock + numBlocks - 1) == RL_NOOVERLAP);
        }
    }

#endif

#if LF_HFS_CHECK_UNMAPPED
    int force_flush = 0;
    /*
     * Since we are about to mark these bits as in-use
     * in the bitmap, decide if we need to alert the caller
     * that a journal flush might be appropriate. It's safe to
     * poke at the journal pointer here since we MUST have
     * called start_transaction by the time this function is invoked.
     * If the journal is enabled, then it will have taken the requisite
     * journal locks.  If it is not enabled, then we have taken
     * a shared lock on the global lock.
     */
    if (hfsmp->jnl) {
        uint32_t ignore;
        err = CheckUnmappedBytes (hfsmp, (uint64_t) startingBlock, (uint64_t)numBlocks, &force_flush, &ignore);
        if ((err == 0) && (force_flush)) {
            journal_request_immediate_flush (hfsmp->jnl);
        }
    }

    hfs_unmap_alloc_extent(vcb, startingBlock, numBlocks);
#endif
    
    /*
     * Don't make changes to the disk if we're just reserving.  Note that
     * we could do better in the tentative case because we could, in theory,
     * avoid the journal flush above.  However, that would mean that we would
     * need to catch the callback to stop it incorrectly addding the extent
     * to our free cache.
     */
    if (ISSET(flags, HFS_ALLOC_LOCKED | HFS_ALLOC_TENTATIVE)) {
        err = 0;
        goto Exit;
    }

    //
    //    Pre-read the bitmap block containing the first word of allocation
    //

    err = ReadBitmapBlock(vcb, startingBlock, &buffer, &blockRef,
                          HFS_ALLOC_IGNORE_RESERVED);
    if (err != noErr) goto Exit;
    //
    //    Initialize currentWord, and wordsLeft.
    //
    {
        u_int32_t wordIndexInBlock;

        bitsPerBlock  = vcb->vcbVBMIOSize * kBitsPerByte;
        wordsPerBlock = vcb->vcbVBMIOSize / kBytesPerWord;

        wordIndexInBlock = (startingBlock & (bitsPerBlock-1)) / kBitsPerWord;
        currentWord = buffer + wordIndexInBlock;
        wordsLeft = wordsPerBlock - wordIndexInBlock;
    }

    // XXXdbg
    if (hfsmp->jnl) {
        journal_modify_block_start(hfsmp->jnl, blockRef);
    }

    //
    //    If the first block to allocate doesn't start on a word
    //    boundary in the bitmap, then treat that first word
    //    specially.
    //

    firstBit = startingBlock % kBitsPerWord;
    if (firstBit != 0) {
        bitMask = kAllBitsSetInWord >> firstBit;    //    turn off all bits before firstBit
        numBits = kBitsPerWord - firstBit;            //    number of remaining bits in this word
        if (numBits > numBlocks) {
            numBits = numBlocks;                    //    entire allocation is inside this one word
            bitMask &= ~(kAllBitsSetInWord >> (firstBit + numBits));    //    turn off bits after last
        }
#if DEBUG
        if ((*currentWord & SWAP_BE32 (bitMask)) != 0) {
            LFHFS_LOG(LEVEL_ERROR, "BlockMarkAllocatedInternal: blocks already allocated!");
            hfs_assert(0);
        }
#endif
        *currentWord |= SWAP_BE32 (bitMask);        //    set the bits in the bitmap
        numBlocks -= numBits;                        //    adjust number of blocks left to allocate

        ++currentWord;                                //    move to next word
        --wordsLeft;                                //    one less word left in this block
    }

    //
    //    Allocate whole words (32 blocks) at a time.
    //

    bitMask = kAllBitsSetInWord;                    //    put this in a register for 68K
    while (numBlocks >= kBitsPerWord) {
        if (wordsLeft == 0) {
            //    Read in the next bitmap block
            startingBlock += bitsPerBlock;            //    generate a block number in the next bitmap block

            buffer = NULL;
            err = ReleaseBitmapBlock(vcb, blockRef, true);
            if (err != noErr) goto Exit;

            err = ReadBitmapBlock(vcb, startingBlock, &buffer, &blockRef,
                                  HFS_ALLOC_IGNORE_RESERVED);
            if (err != noErr) goto Exit;

            // XXXdbg
            if (hfsmp->jnl) {
                journal_modify_block_start(hfsmp->jnl, blockRef);
            }
            
            //    Readjust currentWord and wordsLeft
            currentWord = buffer;
            wordsLeft = wordsPerBlock;
        }
#if DEBUG
        if (*currentWord != 0) {
            LFHFS_LOG(LEVEL_ERROR, "BlockMarkAllocatedInternal: blocks already allocated!");
            hfs_assert(0);
        }
#endif
        *currentWord = SWAP_BE32 (bitMask);
        numBlocks -= kBitsPerWord;

        ++currentWord;                                //    move to next word
        --wordsLeft;                                //    one less word left in this block
    }

    //
    //    Allocate any remaining blocks.
    //

    if (numBlocks != 0) {
        bitMask = ~(kAllBitsSetInWord >> numBlocks);    //    set first numBlocks bits
        if (wordsLeft == 0) {
            //    Read in the next bitmap block
            startingBlock += bitsPerBlock;                //    generate a block number in the next bitmap block

            buffer = NULL;
            err = ReleaseBitmapBlock(vcb, blockRef, true);
            if (err != noErr) goto Exit;

            err = ReadBitmapBlock(vcb, startingBlock, &buffer, &blockRef,
                                  HFS_ALLOC_IGNORE_RESERVED);
            if (err != noErr) goto Exit;
            // XXXdbg
            if (hfsmp->jnl) {
                journal_modify_block_start(hfsmp->jnl, blockRef);
            }
            currentWord = buffer;
        }
#if DEBUG
        if ((*currentWord & SWAP_BE32 (bitMask)) != 0) {
            LFHFS_LOG(LEVEL_ERROR, "BlockMarkAllocatedInternal: blocks already allocated!");
            hfs_assert(0);
        }
#endif
        *currentWord |= SWAP_BE32 (bitMask);            //    set the bits in the bitmap

        //    No need to update currentWord or wordsLeft
    }

Exit:

    if (buffer)
        (void)ReleaseBitmapBlock(vcb, blockRef, true);

    return err;
}


/*
 * BlockMarkFree
 *
 * This is a wrapper function around the internal calls which will actually mark the blocks
 * as freed.  It will mark the blocks in the red-black tree if appropriate.  We need to do
 * this logic here to avoid callers having to deal with whether or not the red-black tree
 * is enabled.
 *
 */
OSErr BlockMarkFree(
                    ExtendedVCB        *vcb,
                    u_int32_t        startingBlock,
                    register u_int32_t    numBlocks)
{
    return BlockMarkFreeInternal(vcb, startingBlock, numBlocks, true);
}


/*
 * BlockMarkFreeUnused
 *
 * Scan the bitmap block beyond end of current file system for bits
 * that are marked as used.  If any of the bits are marked as used,
 * this function marks them free.
 *
 * Note:  This was specifically written to mark all bits beyond
 * end of current file system during hfs_extendfs(), which makes
 * sure that all the new blocks added to the file system are
 * marked as free.   We expect that all the blocks beyond end of
 * current file system are always marked as free, but there might
 * be cases where are marked as used.  This function assumes that
 * the number of blocks marked as used incorrectly are relatively
 * small, otherwise this can overflow journal transaction size
 * on certain file system configurations (example, large unused
 * bitmap with relatively small journal).
 *
 * Input:
 *     startingBlock: First block of the range to mark unused
 *     numBlocks: Number of blocks in the range to mark unused
 *
 * Returns: zero on success, non-zero on error.
 */
OSErr BlockMarkFreeUnused(ExtendedVCB *vcb, u_int32_t startingBlock, register u_int32_t    numBlocks)
{
    int error = 0;
    struct hfsmount *hfsmp = VCBTOHFS(vcb);
    u_int32_t curNumBlocks;
    u_int32_t  bitsPerBlock;
    u_int32_t lastBit;

    /* Use the optimal bitmap I/O size instead of bitmap block size */
    bitsPerBlock  = hfsmp->vcbVBMIOSize * kBitsPerByte;

    /*
     * First clear any non bitmap allocation block aligned bits
     *
     * Calculate the first bit in the bitmap block next to
     * the bitmap block containing the bit for startingBlock.
     * Using this value, we calculate the total number of
     * bits to be marked unused from startingBlock to the
     * end of bitmap block containing startingBlock.
     */
    lastBit = ((startingBlock + (bitsPerBlock - 1))/bitsPerBlock) * bitsPerBlock;
    curNumBlocks = lastBit - startingBlock;
    if (curNumBlocks > numBlocks) {
        curNumBlocks = numBlocks;
    }
    error = BlockMarkFreeInternal(vcb, startingBlock, curNumBlocks, false);
    if (error) {
        return error;
    }
    startingBlock += curNumBlocks;
    numBlocks -= curNumBlocks;

    /*
     * Check a full bitmap block for any 'used' bit.  If any bit is used,
     * mark all the bits only in that bitmap block as free.  This ensures
     * that we do not write unmodified bitmap blocks and do not
     * overwhelm the journal.
     *
     * The code starts by checking full bitmap block at a time, and
     * marks entire bitmap block as free only if any bit in that bitmap
     * block is marked as used.  In the end, it handles the last bitmap
     * block which might be partially full by only checking till the
     * caller-specified last bit and if any bit is set, only mark that
     * range as free.
     */
    while (numBlocks) {
        if (numBlocks >= bitsPerBlock) {
            curNumBlocks = bitsPerBlock;
        } else {
            curNumBlocks = numBlocks;
        }
        if (hfs_isallocated(hfsmp, startingBlock, curNumBlocks) == true) {
            error = BlockMarkFreeInternal(vcb, startingBlock, curNumBlocks, false);
            if (error) {
                return error;
            }
        }
        startingBlock += curNumBlocks;
        numBlocks -= curNumBlocks;
    }

    return error;
}

/*
 _______________________________________________________________________

 Routine:    BlockMarkFreeInternal

 Function:    Mark a contiguous group of blocks as free (clear in the
 bitmap).  It assumes those bits are currently marked
 allocated (set in the bitmap).

 Inputs:
 vcb                Pointer to volume where space is to be freed
 startingBlock    First block number to mark as freed
 numBlocks        Number of blocks to mark as freed
 do_validate     If true, validate that the blocks being
 deallocated to check if they are within totalBlocks
 for current volume and whether they were allocated
 before they are marked free.
 _______________________________________________________________________
 */
static
OSErr BlockMarkFreeInternal(
                            ExtendedVCB        *vcb,
                            u_int32_t        startingBlock_in,
                            register u_int32_t    numBlocks_in,
                            Boolean         do_validate)
{
    OSErr        err;
    u_int32_t    startingBlock = startingBlock_in;
    u_int32_t    numBlocks = numBlocks_in;
    uint32_t    unmapStart = startingBlock_in;
    uint32_t    unmapCount = numBlocks_in;
    uint32_t    wordIndexInBlock;
    u_int32_t    *currentWord;    //    Pointer to current word within bitmap block
    u_int32_t    wordsLeft;        //    Number of words left in this bitmap block
    u_int32_t    bitMask;        //    Word with given bits already set (ready to OR in)
    u_int32_t    currentBit;        //    Bit index within word of current bit to allocate
    u_int32_t    numBits;        //    Number of bits in word to allocate
    u_int32_t    *buffer = NULL;
    GenericLFBufPtr  blockRef = NULL;
    u_int32_t    bitsPerBlock;
    u_int32_t    wordsPerBlock;
    // XXXdbg
    struct hfsmount *hfsmp = VCBTOHFS(vcb);

    /*
     * NOTE: We use vcb->totalBlocks instead of vcb->allocLimit because we
     * need to be able to free blocks being relocated during hfs_truncatefs.
     */
    if ((do_validate == true) &&
        (startingBlock + numBlocks > vcb->totalBlocks)) {
#if ALLOC_DEBUG || DEBUG
        LFHFS_LOG(LEVEL_ERROR, "lockMarkFreeInternal() free non-existent blocks at %u (numBlock=%u) on vol %s\n", startingBlock, numBlocks, vcb->vcbVN);
        hfs_assert(0);
        __builtin_unreachable();
#else
        LFHFS_LOG(LEVEL_ERROR, "BlockMarkFreeInternal() trying to free non-existent blocks starting at %u (numBlock=%u) on volume %s\n", startingBlock, numBlocks, vcb->vcbVN);
        hfs_mark_inconsistent(vcb, HFS_INCONSISTENCY_DETECTED);
        err = EIO;
        goto Exit;
#endif
    }

    //
    //    Pre-read the bitmap block containing the first word of allocation
    //

    err = ReadBitmapBlock(vcb, startingBlock, &buffer, &blockRef,
                          HFS_ALLOC_IGNORE_RESERVED);
    if (err != noErr) goto Exit;

    // XXXdbg
    if (hfsmp->jnl) {
        journal_modify_block_start(hfsmp->jnl, blockRef);
    }

    uint32_t min_unmap = 0, max_unmap = UINT32_MAX;

    // Work out the bounds of any unmap we can send down
    struct rl_entry *range;
    for (int i = 0; i < 2; ++i) {
        TAILQ_FOREACH(range, &hfsmp->hfs_reserved_ranges[i], rl_link) {
            if (range->rl_start < startingBlock
                && range->rl_end >= min_unmap) {
                min_unmap = (uint32_t)(range->rl_end + 1);
            }
            if (range->rl_end >= startingBlock + numBlocks
                && range->rl_start < max_unmap) {
                max_unmap = (uint32_t)range->rl_start;
            }
        }
    }

    //
    //    Figure out how many bits and words per bitmap block.
    //
    bitsPerBlock  = vcb->vcbVBMIOSize * kBitsPerByte;
    wordsPerBlock = vcb->vcbVBMIOSize / kBytesPerWord;
    wordIndexInBlock = (startingBlock & (bitsPerBlock-1)) / kBitsPerWord;

    //
    // Look for a range of free blocks immediately before startingBlock
    // (up to the start of the current bitmap block).  Set unmapStart to
    // the first free block.
    //
    currentWord = buffer + wordIndexInBlock;
    currentBit = startingBlock % kBitsPerWord;
    bitMask = kHighBitInWordMask >> currentBit;
    while (unmapStart > min_unmap) {
        // Move currentWord/bitMask back by one bit
        bitMask <<= 1;
        if (bitMask == 0) {
            if (--currentWord < buffer)
                break;
            bitMask = kLowBitInWordMask;
        }

        if (*currentWord & SWAP_BE32(bitMask))
            break;    // Found an allocated block.  Stop searching.
        --unmapStart;
        ++unmapCount;
    }

    //
    //    If the first block to free doesn't start on a word
    //    boundary in the bitmap, then treat that first word
    //    specially.
    //

    currentWord = buffer + wordIndexInBlock;
    wordsLeft = wordsPerBlock - wordIndexInBlock;
    currentBit = startingBlock % kBitsPerWord;
    if (currentBit != 0) {
        bitMask = kAllBitsSetInWord >> currentBit;    //    turn off all bits before currentBit
        numBits = kBitsPerWord - currentBit;        //    number of remaining bits in this word
        if (numBits > numBlocks) {
            numBits = numBlocks;                    //    entire allocation is inside this one word
            bitMask &= ~(kAllBitsSetInWord >> (currentBit + numBits));    //    turn off bits after last
        }
        if ((do_validate == true) &&
            (*currentWord & SWAP_BE32 (bitMask)) != SWAP_BE32 (bitMask)) {
            goto Corruption;
        }
        *currentWord &= SWAP_BE32 (~bitMask);        //    clear the bits in the bitmap
        numBlocks -= numBits;                        //    adjust number of blocks left to free

        ++currentWord;                                //    move to next word
        --wordsLeft;                                //    one less word left in this block
    }

    //
    //    Free whole words (32 blocks) at a time.
    //

    while (numBlocks >= kBitsPerWord) {
        if (wordsLeft == 0) {
            //    Read in the next bitmap block
            startingBlock += bitsPerBlock;            //    generate a block number in the next bitmap block

            buffer = NULL;
            err = ReleaseBitmapBlock(vcb, blockRef, true);
            if (err != noErr) goto Exit;

            err = ReadBitmapBlock(vcb, startingBlock, &buffer, &blockRef,
                                  HFS_ALLOC_IGNORE_RESERVED);
            if (err != noErr) goto Exit;
            // XXXdbg
            if (hfsmp->jnl) {
                journal_modify_block_start(hfsmp->jnl, blockRef);
            }

            //    Readjust currentWord and wordsLeft
            currentWord = buffer;
            wordsLeft = wordsPerBlock;
        }
        if ((do_validate == true) &&
            (*currentWord != SWAP_BE32 (kAllBitsSetInWord))) {
            goto Corruption;
        }
        *currentWord = 0;                            //    clear the entire word
        numBlocks -= kBitsPerWord;

        ++currentWord;                                //    move to next word
        --wordsLeft;                                    //    one less word left in this block
    }

    //
    //    Free any remaining blocks.
    //

    if (numBlocks != 0) {
        bitMask = ~(kAllBitsSetInWord >> numBlocks);    //    set first numBlocks bits
        if (wordsLeft == 0) {
            //    Read in the next bitmap block
            startingBlock += bitsPerBlock;                //    generate a block number in the next bitmap block

            buffer = NULL;
            err = ReleaseBitmapBlock(vcb, blockRef, true);
            if (err != noErr) goto Exit;

            err = ReadBitmapBlock(vcb, startingBlock, &buffer, &blockRef,
                                  HFS_ALLOC_IGNORE_RESERVED);
            if (err != noErr) goto Exit;

            // XXXdbg
            if (hfsmp->jnl) {
                journal_modify_block_start(hfsmp->jnl, blockRef);
            }

            currentWord = buffer;
        }
        if ((do_validate == true) &&
            (*currentWord & SWAP_BE32 (bitMask)) != SWAP_BE32 (bitMask)) {
            goto Corruption;
        }
        *currentWord &= SWAP_BE32 (~bitMask);            //    clear the bits in the bitmap

        //    No need to update currentWord or wordsLeft
    }

    //
    // Look for a range of free blocks immediately after the range we just freed
    // (up to the end of the current bitmap block).
    //
    wordIndexInBlock = ((startingBlock_in + numBlocks_in - 1) & (bitsPerBlock-1)) / kBitsPerWord;
    wordsLeft = wordsPerBlock - wordIndexInBlock;
    currentWord = buffer + wordIndexInBlock;
    currentBit = (startingBlock_in + numBlocks_in - 1) % kBitsPerWord;
    bitMask = kHighBitInWordMask >> currentBit;
    while (unmapStart + unmapCount < max_unmap) {
        // Move currentWord/bitMask/wordsLeft forward one bit
        bitMask >>= 1;
        if (bitMask == 0) {
            if (--wordsLeft == 0)
                break;
            ++currentWord;
            bitMask = kHighBitInWordMask;
        }

        if (*currentWord & SWAP_BE32(bitMask))
            break;    // Found an allocated block.  Stop searching.
        ++unmapCount;
    }

Exit:

    if (buffer)
        (void)ReleaseBitmapBlock(vcb, blockRef, true);
    return err;

Corruption:
#if DEBUG
    LFHFS_LOG(LEVEL_ERROR, "BlockMarkFreeInternal: blocks not allocated!");
    hfs_assert(0);
    __builtin_unreachable();
#else
    LFHFS_LOG(LEVEL_ERROR, "BlockMarkFreeInternal() trying to free unallocated blocks on volume %s <%u, %u>\n",
                    vcb->vcbVN, startingBlock_in, numBlocks_in);

    hfs_mark_inconsistent(vcb, HFS_INCONSISTENCY_DETECTED);
    err = EIO;
    goto Exit;
#endif
}


/*
 _______________________________________________________________________

 Routine:    BlockFindContiguous

 Function:    Find a contiguous range of blocks that are free (bits
 clear in the bitmap).  If a contiguous range of the
 minimum size can't be found, an error will be returned.
 This is only needed to support the bitmap-scanning logic,
 as the red-black tree should be able to do this by internally
 searching its tree.

 Inputs:
 vcb                Pointer to volume where space is to be allocated
 startingBlock    Preferred first block of range
 endingBlock        Last possible block in range + 1
 minBlocks        Minimum number of blocks needed.  Must be > 0.
 maxBlocks        Maximum (ideal) number of blocks desired
 useMetaZone    OK to dip into metadata allocation zone

 Outputs:
 actualStartBlock    First block of range found, or 0 if error
 actualNumBlocks        Number of blocks found, or 0 if error

 Returns:
 noErr            Found at least minBlocks contiguous
 dskFulErr        No contiguous space found, or all less than minBlocks
 _______________________________________________________________________
 */

static OSErr BlockFindContiguous(
                                 ExtendedVCB        *vcb,
                                 u_int32_t        startingBlock,
                                 u_int32_t        endingBlock,
                                 u_int32_t        minBlocks,
                                 u_int32_t        maxBlocks,
                                 Boolean            useMetaZone,
                                 Boolean            trustSummary,
                                 u_int32_t        *actualStartBlock,
                                 u_int32_t        *actualNumBlocks,
                                 hfs_block_alloc_flags_t flags)
{
    OSErr            err;
    register u_int32_t    currentBlock;        //    Block we're currently looking at.
    u_int32_t            firstBlock;            //    First free block in current extent.
    u_int32_t            stopBlock;            //    If we get to this block, stop searching for first free block.
    u_int32_t            foundBlocks;        //    Number of contiguous free blocks in current extent.
    u_int32_t            *buffer = NULL;
    register u_int32_t    *currentWord;
    register u_int32_t    bitMask;
    register u_int32_t    wordsLeft;
    register u_int32_t    tempWord;
    GenericLFBufPtr  blockRef = 0;
    u_int32_t  wordsPerBlock;
    struct hfsmount *hfsmp = (struct hfsmount*) vcb;
    HFSPlusExtentDescriptor best = { 0, 0 };

    /*
     * When we're skipping the metadata zone and the start/end
     * range overlaps with the metadata zone then adjust the
     * start to be outside of the metadata zone.  If the range
     * is entirely inside the metadata zone then we can deny the
     * request (dskFulErr).
     */
    if (!useMetaZone && (vcb->hfs_flags & HFS_METADATA_ZONE)) {
        if (startingBlock <= vcb->hfs_metazone_end) {
            if (endingBlock > (vcb->hfs_metazone_end + 2))
                startingBlock = vcb->hfs_metazone_end + 1;
            else
                goto DiskFull;
        }
    }

    if ((endingBlock - startingBlock) < minBlocks)
    {
        //    The set of blocks we're checking is smaller than the minimum number
        //    of blocks, so we couldn't possibly find a good range.
        goto DiskFull;
    }

    stopBlock = endingBlock - minBlocks + 1;
    currentBlock = startingBlock;
    firstBlock = 0;

    /*
     * Skip over metadata blocks.
     */
    if (!useMetaZone)
        currentBlock = NextBitmapBlock(vcb, currentBlock);

    /*
     * Use the summary table if we can.  Skip over any totally
     * allocated blocks.  currentBlock should now point to the first
     * block beyond the metadata zone if the metazone allocations are not
     * allowed in this invocation.
     */
    if ((trustSummary) && (hfsmp->hfs_flags & HFS_SUMMARY_TABLE)) {
        uint32_t suggestion;
        err = hfs_find_summary_free (hfsmp, currentBlock, &suggestion);
        if (err && err != ENOSPC)
            goto ErrorExit;
        if (err == ENOSPC || suggestion >= stopBlock)
            goto DiskFull;
        currentBlock = suggestion;
    }


    //
    //    Pre-read the first bitmap block.
    //
    err = ReadBitmapBlock(vcb, currentBlock, &buffer, &blockRef, flags);
    if ( err != noErr ) goto ErrorExit;

    //
    //    Figure out where currentBlock is within the buffer.
    //
    wordsPerBlock = vcb->vcbVBMIOSize / kBytesPerWord;

    wordsLeft = (currentBlock / kBitsPerWord) & (wordsPerBlock-1);    // Current index into buffer
    currentWord = buffer + wordsLeft;
    wordsLeft = wordsPerBlock - wordsLeft;

    uint32_t remaining = (hfsmp->freeBlocks - hfsmp->lockedBlocks
                          - (ISSET(flags, HFS_ALLOC_IGNORE_TENTATIVE)
                             ? 0 : hfsmp->tentativeBlocks));

    /*
     * This outer do-while loop is the main body of this function.  Its job is
     * to search through the blocks (until we hit 'stopBlock'), and iterate
     * through swaths of allocated bitmap until it finds free regions.
     */

    do
    {
        foundBlocks = 0;
        /*
         * We will try and update the summary table as we search
         * below.  Note that we will never update the summary table
         * for the first and last blocks that the summary table
         * covers.  Ideally, we should, but the benefits probably
         * aren't that significant so we leave things alone for now.
         */
        uint32_t summary_block_scan = 0;
        /*
         * Inner while loop 1:
         *        Look for free blocks, skipping over allocated ones.
         *
         * Initialization starts with checking the initial partial word
         * if applicable.
         */
        bitMask = currentBlock & kBitsWithinWordMask;
        if (bitMask)
        {
            tempWord = SWAP_BE32(*currentWord);            //    Fetch the current word only once
            bitMask = kHighBitInWordMask >> bitMask;
            while (tempWord & bitMask)
            {
                bitMask >>= 1;
                ++currentBlock;
            }

            //    Did we find an unused bit (bitMask != 0), or run out of bits (bitMask == 0)?
            if (bitMask)
                goto FoundUnused;

            //    Didn't find any unused bits, so we're done with this word.
            ++currentWord;
            --wordsLeft;
        }

        //
        //    Check whole words
        //
        while (currentBlock < stopBlock)
        {
            //    See if it's time to read another block.
            if (wordsLeft == 0)
            {
                buffer = NULL;
                if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
                    /*
                     * If summary_block_scan is non-zero, then we must have
                     * pulled a bitmap file block into core, and scanned through
                     * the entire thing.  Because we're in this loop, we are
                     * implicitly trusting that the bitmap didn't have any knowledge
                     * about this particular block.  As a result, update the bitmap
                     * (lazily, now that we've scanned it) with our findings that
                     * this particular block is completely used up.
                     */
                    if (summary_block_scan != 0) {
                        uint32_t summary_bit;
                        err = hfs_get_summary_index (hfsmp, summary_block_scan, &summary_bit);
                        if (err != noErr) goto ErrorExit;
                        hfs_set_summary (hfsmp, summary_bit, 1);
                    }
                }
                err = ReleaseBitmapBlock(vcb, blockRef, false);
                if (err != noErr) goto ErrorExit;

                /*
                 * Skip over metadata blocks.
                 */
                if (!useMetaZone) {
                    currentBlock = NextBitmapBlock(vcb, currentBlock);
                    if (currentBlock >= stopBlock) {
                        goto LoopExit;
                    }
                }

                /* Skip over fully allocated bitmap blocks if we can */
                if ((trustSummary) && (hfsmp->hfs_flags & HFS_SUMMARY_TABLE)) {
                    uint32_t suggestion;
                    err = hfs_find_summary_free (hfsmp, currentBlock, &suggestion);
                    if (err && err != ENOSPC)
                        goto ErrorExit;
                    if (err == ENOSPC || suggestion >= stopBlock)
                        goto LoopExit;
                    currentBlock = suggestion;
                }

                err = ReadBitmapBlock(vcb, currentBlock, &buffer, &blockRef, flags);
                if ( err != noErr ) goto ErrorExit;

                /*
                 * Set summary_block_scan to be the block we just read into the block cache.
                 *
                 * At this point, we've just read an allocation block worth of bitmap file
                 * into the buffer above, but we don't know if it is completely allocated or not.
                 * If we find that it is completely allocated/full then we will jump
                 * through this loop again and set the appropriate summary bit as fully allocated.
                 */
                summary_block_scan = currentBlock;
                currentWord = buffer;
                wordsLeft = wordsPerBlock;
            }

            //    See if any of the bits are clear
            if ((tempWord = SWAP_BE32(*currentWord)) + 1)    //    non-zero if any bits were clear
            {
                //    Figure out which bit is clear
                bitMask = kHighBitInWordMask;
                while (tempWord & bitMask)
                {
                    bitMask >>= 1;
                    ++currentBlock;
                }

                break;        //    Found the free bit; break out to FoundUnused.
            }

            //    Keep looking at the next word
            currentBlock += kBitsPerWord;
            ++currentWord;
            --wordsLeft;
        }

    FoundUnused:
        //    Make sure the unused bit is early enough to use
        if (currentBlock >= stopBlock)
        {
            break;
        }

        //    Remember the start of the extent
        firstBlock = currentBlock;


        /*
         * Inner while loop 2:
         *        We get here if we find a free block. Count the number
         *         of contiguous free blocks observed.
         *
         * Initialization starts with checking the initial partial word
         * if applicable.
         */
        bitMask = currentBlock & kBitsWithinWordMask;
        if (bitMask)
        {
            tempWord = SWAP_BE32(*currentWord);            //    Fetch the current word only once
            bitMask = kHighBitInWordMask >> bitMask;
            while (bitMask && !(tempWord & bitMask))
            {
                bitMask >>= 1;
                ++currentBlock;
            }

            //    Did we find a used bit (bitMask != 0), or run out of bits (bitMask == 0)?
            if (bitMask)
                goto FoundUsed;

            //    Didn't find any used bits, so we're done with this word.
            ++currentWord;
            --wordsLeft;
        }

        //
        //    Check whole words
        //
        while (currentBlock < endingBlock)
        {
            //    See if it's time to read another block.
            if (wordsLeft == 0)
            {
                buffer = NULL;
                err = ReleaseBitmapBlock(vcb, blockRef, false);
                if (err != noErr) goto ErrorExit;

                /*
                 * Skip over metadata blocks.
                 */
                if (!useMetaZone) {
                    u_int32_t nextBlock;

                    nextBlock = NextBitmapBlock(vcb, currentBlock);
                    if (nextBlock != currentBlock) {
                        goto LoopExit;  /* allocation gap, so stop */
                    }
                }

                err = ReadBitmapBlock(vcb, currentBlock, &buffer, &blockRef, flags);
                if ( err != noErr ) goto ErrorExit;

                currentWord = buffer;
                wordsLeft = wordsPerBlock;
            }

            //    See if any of the bits are set
            if ((tempWord = SWAP_BE32(*currentWord)) != 0)
            {
                //    Figure out which bit is set
                bitMask = kHighBitInWordMask;
                while (!(tempWord & bitMask))
                {
                    bitMask >>= 1;
                    ++currentBlock;
                }

                break;        //    Found the used bit; break out to FoundUsed.
            }

            //    Keep looking at the next word
            currentBlock += kBitsPerWord;
            ++currentWord;
            --wordsLeft;

            //    If we found at least maxBlocks, we can quit early.
            if ((currentBlock - firstBlock) >= maxBlocks)
                break;
        }

    FoundUsed:
        //    Make sure we didn't run out of bitmap looking for a used block.
        //    If so, pin to the end of the bitmap.
        if (currentBlock > endingBlock)
            currentBlock = endingBlock;

        //    Figure out how many contiguous free blocks there were.
        //    Pin the answer to maxBlocks.
        foundBlocks = currentBlock - firstBlock;
        if (foundBlocks > maxBlocks)
            foundBlocks = maxBlocks;

        if (remaining) {
            if (foundBlocks > remaining) {
                LFHFS_LOG( LEVEL_DEBUG, "hfs: found more blocks than are indicated free!\n");
                remaining = UINT32_MAX;
            } else
                remaining -= foundBlocks;
        }

        if (ISSET(flags, HFS_ALLOC_TRY_HARD)) {
            if (foundBlocks > best.blockCount) {
                best.startBlock = firstBlock;
                best.blockCount = foundBlocks;
            }

            if (foundBlocks >= maxBlocks || best.blockCount >= remaining)
                break;

            /*
             * Note that we will go ahead and add this free extent to our
             * cache below but that's OK because we'll remove it again if we
             * decide to use this extent.
             */
        } else if (foundBlocks >= minBlocks)
            break;        //    Found what we needed!

        /*
         * We did not find the total blocks we were looking for, but
         * add this free block run to our free extent cache list, if possible.
         */

        // If we're ignoring tentative ranges, we need to account for them here
        if (ISSET(flags, HFS_ALLOC_IGNORE_TENTATIVE)) {
            struct rl_entry free_extent = rl_make(firstBlock, firstBlock + foundBlocks - 1);
            struct rl_entry *range;;
            TAILQ_FOREACH(range, &hfsmp->hfs_reserved_ranges[HFS_TENTATIVE_BLOCKS], rl_link) {
                rl_subtract(&free_extent, range);
                if (rl_len(range) == 0)
                    break;
            }
            firstBlock = (uint32_t)free_extent.rl_start;
            foundBlocks = (uint32_t)rl_len(&free_extent);
        }
    } while (currentBlock < stopBlock);
LoopExit:

    if (ISSET(flags, HFS_ALLOC_TRY_HARD)) {
        firstBlock = best.startBlock;
        foundBlocks = best.blockCount;
    }

    //    Return the outputs.
    if (foundBlocks < minBlocks)
    {
    DiskFull:
        err = dskFulErr;
    ErrorExit:
        *actualStartBlock = 0;
        *actualNumBlocks = 0;
    }
    else
    {
        err = noErr;
        *actualStartBlock = firstBlock;
        *actualNumBlocks = foundBlocks;
        /*
         * Sanity check for overflow
         */
        if ((firstBlock + foundBlocks) > vcb->allocLimit) {
            LFHFS_LOG(LEVEL_ERROR, "blk allocation overflow on \"%s\" sb:0x%08x eb:0x%08x cb:0x%08x fb:0x%08x stop:0x%08x min:0x%08x found:0x%08x",
                      vcb->vcbVN, startingBlock, endingBlock, currentBlock,
                      firstBlock, stopBlock, minBlocks, foundBlocks);
            hfs_assert(0);
        }
    }

    if (buffer)
        (void) ReleaseBitmapBlock(vcb, blockRef, false);

    return err;
}


/*
 * Count number of bits set in the given 32-bit unsigned number
 *
 * Returns:
 *     Number of bits set
 */
static int num_bits_set(u_int32_t num)
{
    return __builtin_popcount(num);
}

/*
 * For a given range of blocks, find the total number of blocks
 * allocated.  If 'stop_on_first' is true, it stops as soon as it
 * encounters the first allocated block.  This option is useful
 * to determine if any block is allocated or not.
 *
 * Inputs:
 *     startingBlock    First allocation block number of the range to be scanned.
 *     numBlocks    Total number of blocks that need to be scanned.
 *     stop_on_first    Stop the search after the first allocated block is found.
 *
 * Output:
 *     allocCount    Total number of allocation blocks allocated in the given range.
 *
 *             On error, it is the number of allocated blocks found
 *             before the function got an error.
 *
 *             If 'stop_on_first' is set,
 *                 allocCount = 1 if any allocated block was found.
 *                 allocCount = 0 if no allocated block was found.
 *
 * Returns:
 *     0 on success, non-zero on failure.
 */
static int
hfs_isallocated_internal(struct hfsmount *hfsmp, u_int32_t startingBlock,
                         u_int32_t numBlocks, Boolean stop_on_first, u_int32_t *allocCount)
{
    u_int32_t  *currentWord;   // Pointer to current word within bitmap block
    u_int32_t  wordsLeft;      // Number of words left in this bitmap block
    u_int32_t  bitMask;        // Word with given bits already set (ready to test)
    u_int32_t  firstBit;       // Bit index within word of first bit to allocate
    u_int32_t  numBits;        // Number of bits in word to allocate
    u_int32_t  *buffer = NULL;
    GenericLFBufPtr  blockRef;
    u_int32_t  bitsPerBlock;
    u_int32_t  wordsPerBlock;
    u_int32_t  blockCount = 0;
    int  error;

    /*
     * Pre-read the bitmap block containing the first word of allocation
     */
    error = ReadBitmapBlock(hfsmp, startingBlock, &buffer, &blockRef,
                            HFS_ALLOC_IGNORE_TENTATIVE);
    if (error)
        goto JustReturn;

    /*
     * Initialize currentWord, and wordsLeft.
     */
    {
        u_int32_t wordIndexInBlock;

        bitsPerBlock  = hfsmp->vcbVBMIOSize * kBitsPerByte;
        wordsPerBlock = hfsmp->vcbVBMIOSize / kBytesPerWord;

        wordIndexInBlock = (startingBlock & (bitsPerBlock-1)) / kBitsPerWord;
        currentWord = buffer + wordIndexInBlock;
        wordsLeft = wordsPerBlock - wordIndexInBlock;
    }

    /*
     * First test any non word aligned bits.
     */
    firstBit = startingBlock % kBitsPerWord;
    if (firstBit != 0) {
        bitMask = kAllBitsSetInWord >> firstBit;
        numBits = kBitsPerWord - firstBit;
        if (numBits > numBlocks) {
            numBits = numBlocks;
            bitMask &= ~(kAllBitsSetInWord >> (firstBit + numBits));
        }
        if ((*currentWord & SWAP_BE32 (bitMask)) != 0) {
            if (stop_on_first) {
                blockCount = 1;
                goto Exit;
            }
            blockCount += num_bits_set(*currentWord & SWAP_BE32 (bitMask));
        }
        numBlocks -= numBits;
        ++currentWord;
        --wordsLeft;
    }

    /*
     * Test whole words (32 blocks) at a time.
     */
    while (numBlocks >= kBitsPerWord) {
        if (wordsLeft == 0) {
            /* Read in the next bitmap block. */
            startingBlock += bitsPerBlock;

            buffer = NULL;
            error = ReleaseBitmapBlock(hfsmp, blockRef, false);
            if (error) goto Exit;

            error = ReadBitmapBlock(hfsmp, startingBlock, &buffer, &blockRef,
                                    HFS_ALLOC_IGNORE_TENTATIVE);
            if (error) goto Exit;

            /* Readjust currentWord and wordsLeft. */
            currentWord = buffer;
            wordsLeft = wordsPerBlock;
        }
        if (*currentWord != 0) {
            if (stop_on_first) {
                blockCount = 1;
                goto Exit;
            }
            blockCount += num_bits_set(*currentWord);
        }
        numBlocks -= kBitsPerWord;
        ++currentWord;
        --wordsLeft;
    }

    /*
     * Test any remaining blocks.
     */
    if (numBlocks != 0) {
        bitMask = ~(kAllBitsSetInWord >> numBlocks);
        if (wordsLeft == 0) {
            /* Read in the next bitmap block */
            startingBlock += bitsPerBlock;

            buffer = NULL;
            error = ReleaseBitmapBlock(hfsmp, blockRef, false);
            if (error) goto Exit;

            error = ReadBitmapBlock(hfsmp, startingBlock, &buffer, &blockRef,
                                    HFS_ALLOC_IGNORE_TENTATIVE);
            if (error) goto Exit;

            currentWord = buffer;
        }
        if ((*currentWord & SWAP_BE32 (bitMask)) != 0) {
            if (stop_on_first) {
                blockCount = 1;
                goto Exit;
            }
            blockCount += num_bits_set(*currentWord & SWAP_BE32 (bitMask));
        }
    }
Exit:
    if (buffer) {
        (void)ReleaseBitmapBlock(hfsmp, blockRef, false);
    }
    if (allocCount) {
        *allocCount = blockCount;
    }

JustReturn:

    return (error);
}

/*
 * Count total number of blocks that are allocated in the given
 * range from the bitmap.  This is used to preflight total blocks
 * that need to be relocated during volume resize.
 *
 * The journal or allocation file lock must be held.
 *
 * Returns:
 *     0 on success, non-zero on failure.
 *     On failure, allocCount is zero.
 */
int
hfs_count_allocated(struct hfsmount *hfsmp, u_int32_t startBlock,
                    u_int32_t numBlocks, u_int32_t *allocCount)
{
    return hfs_isallocated_internal(hfsmp, startBlock, numBlocks, false, allocCount);
}

/*
 * Test to see if any blocks in a range are allocated.
 *
 * Note:  On error, this function returns 1, which means that
 * one or more blocks in the range are allocated.  This function
 * is primarily used for volume resize and we do not want
 * to report to the caller that the blocks are free when we
 * were not able to deterministically find it out.  So on error,
 * we always report that the blocks are allocated.
 *
 * The journal or allocation file lock must be held.
 *
 * Returns
 *    0 if all blocks in the range are free.
 *    1 if blocks in the range are allocated, or there was an error.
 */
int
hfs_isallocated(struct hfsmount *hfsmp, u_int32_t startingBlock, u_int32_t numBlocks)
{
    int error;
    u_int32_t allocCount;

    error = hfs_isallocated_internal(hfsmp, startingBlock, numBlocks, true, &allocCount);
    if (error) {
        /* On error, we always say that the blocks are allocated
         * so that volume resize does not return false success.
         */
        return 1;
    } else {
        /* The function was deterministically able to find out
         * if there was any block allocated or not.  In that case,
         * the value in allocCount is good enough to be returned
         * back to the caller.
         */
        return allocCount;
    }
}

/*
 * CONFIG_HFS_RBTREE
 * Check to see if the red-black tree is live.  Allocation file lock must be held
 * shared or exclusive to call this function. Note that we may call this even if
 * HFS is built without activating the red-black tree code.
 */
int
hfs_isrbtree_active(struct hfsmount *hfsmp){

#pragma unused (hfsmp)

    /* Just return 0 for now */
    return 0;
}



/* Summary Table Functions */
/*
 * hfs_check_summary:
 *
 * This function should be used to query the summary table to see if we can
 * bypass a bitmap block or not when we're trying to find a free allocation block.
 *
 *
 * Inputs:
 *         allocblock - allocation block number. Will be used to infer the correct summary bit.
 *         hfsmp -- filesystem in question.
 *
 * Output Arg:
 *        *freeblocks - set to 1 if we believe at least one free blocks in this vcbVBMIOSize
 *         page of bitmap file.
 *
 *
 * Returns:
 *         0 on success
 *        EINVAL on error
 *
 */

static int hfs_check_summary (struct hfsmount *hfsmp, uint32_t allocblock, uint32_t *freeblocks) {

    int err = EINVAL;
    if (hfsmp->vcbVBMIOSize) {
        if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
            uint32_t index;
            if (hfs_get_summary_index (hfsmp, allocblock, &index)) {
                *freeblocks = 0;
                return EINVAL;
            }

            /* Ok, now that we have the bit index into the array, what byte is it in ? */
            uint32_t byteindex = index / kBitsPerByte;
            uint8_t current_byte = hfsmp->hfs_summary_table[byteindex];
            uint8_t bit_in_byte = index % kBitsPerByte;

            if (current_byte & (1 << bit_in_byte)) {
                /*
                 * We do not believe there is anything free in the
                 * entire vcbVBMIOSize'd block.
                 */
                *freeblocks = 0;
            }
            else {
                /* Looks like there might be a free block here... */
                *freeblocks = 1;
            }
        }
        err = 0;
    }

    return err;
}

/*
 * hfs_release_summary
 *
 * Given an extent that is about to be de-allocated on-disk, determine the number
 * of summary bitmap bits that need to be marked as 'potentially available'.
 * Then go ahead and mark them as free.
 *
 *    Inputs:
 *         hfsmp         - hfs mount
 *         block         - starting allocation block.
 *         length        - length of the extent.
 *
 *     Returns:
 *        EINVAL upon any errors.
 */
static int hfs_release_summary(struct hfsmount *hfsmp, uint32_t start_blk, uint32_t length) {
    int err = EINVAL;
    uint32_t end_blk = (start_blk + length) - 1;

    if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
        /* Figure out what the starting / ending block's summary bits are */
        uint32_t start_bit;
        uint32_t end_bit;
        uint32_t current_bit;

        err = hfs_get_summary_index (hfsmp, start_blk, &start_bit);
        if (err) {
            goto release_err;
        }
        err = hfs_get_summary_index (hfsmp, end_blk, &end_bit);
        if (err) {
            goto release_err;
        }

        if (ALLOC_DEBUG) {
            if (start_bit > end_bit) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_release_summary: start > end!, %d %d ", start_bit, end_bit);
                hfs_assert(0);
            }
        }
        current_bit = start_bit;
        while (current_bit <= end_bit) {
            err = hfs_set_summary (hfsmp, current_bit, 0);
            current_bit++;
        }
    }

release_err:
    return err;
}

/*
 * hfs_find_summary_free
 *
 * Given a allocation block as input, returns an allocation block number as output as a
 * suggestion for where to start scanning the bitmap in order to find free blocks.  It will
 * determine the vcbVBMIOsize of the input allocation block, convert that into a summary
 * bit, then keep iterating over the summary bits in order to find the first free one.
 *
 * Inputs:
 *        hfsmp         - hfs mount
 *         block        - starting allocation block
 *         newblock     - output block as suggestion
 *
 * Returns:
 *         0 on success
 *         ENOSPC if we could not find a free block
 */

int hfs_find_summary_free (struct hfsmount *hfsmp, uint32_t block,  uint32_t *newblock) {

    int err = ENOSPC;
    uint32_t bit_index = 0;
    uint32_t maybe_has_blocks = 0;

    if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
        uint32_t byte_index;
        uint8_t curbyte;
        uint8_t bit_in_byte;
        uint32_t summary_cap;

        /*
         * We generate a cap for the summary search because the summary table
         * always represents a full summary of the bitmap FILE, which may
         * be way more bits than are necessary for the actual filesystem
         * whose allocations are mapped by the bitmap.
         *
         * Compute how much of hfs_summary_size is useable for the given number
         * of allocation blocks eligible on this FS.
         */
        err = hfs_get_summary_index (hfsmp, hfsmp->allocLimit - 1, &summary_cap);
        if (err) {
            goto summary_exit;
        }

        /* Check the starting block first */
        err = hfs_check_summary (hfsmp, block, &maybe_has_blocks);
        if (err) {
            goto summary_exit;
        }

        if (maybe_has_blocks) {
            /*
             * It looks like the initial start block could have something.
             * Short-circuit and just use that.
             */
            *newblock = block;
            goto summary_exit;
        }

        /*
         * OK, now we know that the first block was useless.
         * Get the starting summary bit, and find it in the array
         */
        maybe_has_blocks = 0;
        err = hfs_get_summary_index (hfsmp, block, &bit_index);
        if (err) {
            goto summary_exit;
        }

        /* Iterate until we find something. */
        while (bit_index <= summary_cap) {
            byte_index = bit_index / kBitsPerByte;
            curbyte = hfsmp->hfs_summary_table[byte_index];
            bit_in_byte = bit_index % kBitsPerByte;

            if (curbyte & (1 << bit_in_byte)) {
                /* nothing here.  increment and move on */
                bit_index++;
            }
            else {
                /*
                 * found something! convert bit_index back into
                 * an allocation block for use. 'newblock' will now
                 * contain the proper allocation block # based on the bit
                 * index.
                 */
                err = hfs_get_summary_allocblock (hfsmp, bit_index, newblock);
                if (err) {
                    goto summary_exit;
                }
                maybe_has_blocks = 1;
                break;
            }
        }

        /* If our loop didn't find anything, set err to ENOSPC */
        if (maybe_has_blocks == 0) {
            err = ENOSPC;
        }
    }

    /* If the summary table is not active for this mount, we'll just return ENOSPC */
summary_exit:
    if (maybe_has_blocks) {
        err = 0;
    }

    return err;
}

/*
 * hfs_get_summary_allocblock
 *
 * Convert a summary bit into an allocation block number to use to start searching for free blocks.
 *
 * Inputs:
 *        hfsmp             - hfs mount
 *         summarybit         - summmary bit index
 *        *alloc            - allocation block number in the bitmap file.
 *
 * Output:
 *        0 on success
 *         EINVAL on failure
 */
int hfs_get_summary_allocblock (struct hfsmount *hfsmp, uint32_t
                                summarybit, uint32_t *alloc) {
    uint32_t bits_per_iosize = hfsmp->vcbVBMIOSize * kBitsPerByte;
    uint32_t allocblk;

    allocblk = summarybit * bits_per_iosize;

    if (allocblk >= hfsmp->totalBlocks) {
        return EINVAL;
    }
    else {
        *alloc = allocblk;
    }

    return 0;
}


/*
 * hfs_set_summary:
 *
 * This function should be used to manipulate the summary table
 *
 * The argument 'inuse' will set the value of the bit in question to one or zero
 * depending on its value.
 *
 * Inputs:
 *         hfsmp         - hfs mount
 *        summarybit    - the bit index into the summary table to set/unset.
 *         inuse        - the value to assign to the bit.
 *
 * Returns:
 *         0 on success
 *        EINVAL on error
 *
 */

static int hfs_set_summary (struct hfsmount *hfsmp, uint32_t summarybit, uint32_t inuse) {

    int err = EINVAL;
    if (hfsmp->vcbVBMIOSize) {
        if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {

            if (ALLOC_DEBUG) {
                if (hfsmp->hfs_summary_table == NULL) {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_set_summary: no table for %p ", hfsmp);
                    hfs_assert(0);
                }
            }

            /* Ok, now that we have the bit index into the array, what byte is it in ? */
            uint32_t byte_index = summarybit / kBitsPerByte;
            uint8_t current_byte = hfsmp->hfs_summary_table[byte_index];
            uint8_t bit_in_byte = summarybit % kBitsPerByte;

            if (inuse) {
                current_byte = (current_byte | (1 << bit_in_byte));
            }
            else {
                current_byte = (current_byte & ~(1 << bit_in_byte));
            }

            hfsmp->hfs_summary_table[byte_index] = current_byte;
        }
        err = 0;
    }

    return err;
}


/*
 * hfs_get_summary_index:
 *
 * This is a helper function which determines what summary bit represents the vcbVBMIOSize worth
 * of IO against the bitmap file.
 *
 * Returns:
 *        0 on success
 *         EINVAL on failure
 */
static int hfs_get_summary_index (struct hfsmount *hfsmp, uint32_t block, uint32_t* index) {
    uint32_t summary_bit;
    uint32_t bits_per_iosize;
    int err = EINVAL;

    if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
        /* Is the input block bigger than the total number of blocks? */
        if (block >= hfsmp->totalBlocks) {
            return EINVAL;
        }

        /* Is there even a vbmIOSize set? */
        if (hfsmp->vcbVBMIOSize == 0) {
            return EINVAL;
        }

        bits_per_iosize = hfsmp->vcbVBMIOSize * kBitsPerByte;

        summary_bit = block / bits_per_iosize;

        *index = summary_bit;
        err = 0;
    }

    return err;
}

/*
 * hfs_init_summary
 *
 * From a given mount structure, compute how big the summary table should be for the given
 * filesystem, then allocate and bzero the memory.
 *
 * Returns:
 * 0 on success
 * EINVAL on failure
 */
int
hfs_init_summary (struct hfsmount *hfsmp) {

    uint32_t summary_size;
    uint32_t summary_size_bytes;
    uint8_t *summary_table;

    if (hfsmp->hfs_allocation_cp == NULL) {
        if (ALLOC_DEBUG) {
            LFHFS_LOG(LEVEL_DEBUG, "hfs_init_summary: summary table cannot progress without a bitmap cnode! \n");
        }
        return EINVAL;
    }
    /*
     * The practical maximum size of the summary table is 16KB:
     *
     *        (512MB maximum bitmap size / (4k -- min alloc block size)) / 8 bits/byte.
     *
     * HFS+ will allow filesystems with allocation block sizes smaller than 4k, but
     * the end result is that we'll start to issue I/O in 2k or 1k sized chunks, which makes
     * supporting this much worse.  The math would instead look like this:
     * (512MB / 2k) / 8 == 32k.
     *
     * So, we will disallow the summary table if the allocation block size is < 4k.
     */

    if (hfsmp->blockSize < HFS_MIN_SUMMARY_BLOCKSIZE) {
        LFHFS_LOG(LEVEL_ERROR, "hfs_init_summary: summary table not allowed on FS with block size of %d\n", hfsmp->blockSize);
        return EINVAL;
    }

    summary_size = hfsmp->hfs_allocation_cp->c_blocks;

    if (ALLOC_DEBUG) {
        LFHFS_LOG(LEVEL_DEBUG, "HFS Summary Table Initialization: Bitmap %u blocks\n",
                  hfsmp->hfs_allocation_cp->c_blocks);
    }

    /*
     * If the bitmap IO size is not the same as the allocation block size then
     * then re-compute the number of summary bits necessary.  Note that above, the
     * the default size is the number of allocation blocks in the bitmap *FILE*
     * (not the number of bits in the bitmap itself).  If the allocation block size
     * is large enough though, we may need to increase this.
     */
    if (hfsmp->blockSize != hfsmp->vcbVBMIOSize) {
        uint64_t lrg_size = (uint64_t) hfsmp->hfs_allocation_cp->c_blocks * (uint64_t) hfsmp->blockSize;
        lrg_size = lrg_size / (uint64_t)hfsmp->vcbVBMIOSize;

        /* With a full bitmap and 64k-capped iosize chunks, this would be 64k */
        summary_size = (uint32_t) lrg_size;
    }

    /*
     * If the block size is the same as the IO Size, then the total number of blocks
     * is already equal to the number of IO units, which is our number of summary bits.
     */

    summary_size_bytes = summary_size / kBitsPerByte;
    /* Always add one byte, just in case we have a dangling number of bits */
    summary_size_bytes++;

    if (ALLOC_DEBUG) {
        LFHFS_LOG(LEVEL_DEBUG, "HFS Summary Table: vcbVBMIOSize %d summary bits %d \n", hfsmp->vcbVBMIOSize, summary_size);
        LFHFS_LOG(LEVEL_DEBUG, "HFS Summary Table Size (in bytes) %d \n", summary_size_bytes);

        
    }

    /* Store the field in the mount point */
    hfsmp->hfs_summary_size = summary_size;
    hfsmp->hfs_summary_bytes = summary_size_bytes;

    summary_table = hfs_mallocz(summary_size_bytes);

    /* enable the summary table */
    hfsmp->hfs_flags |= HFS_SUMMARY_TABLE;
    hfsmp->hfs_summary_table = summary_table;

    if (ALLOC_DEBUG) {
        if (hfsmp->hfs_summary_table == NULL) {
            LFHFS_LOG(LEVEL_ERROR, "HFS Summary Init: no table for %p\n", hfsmp);
            hfs_assert(0);
        }
    }
    return 0;
}

#if ALLOC_DEBUG
/*
 * hfs_validate_summary
 *
 * Validation routine for the summary table.  Debug-only function.
 *
 * Bitmap lock must be held.
 *
 */
void hfs_validate_summary (struct hfsmount *hfsmp) {
    uint32_t i;
    int err;

    /*
     * Iterate over all of the bits in the summary table, and verify if
     * there really are free blocks in the pages that we believe may
     * may contain free blocks.
     */

    if (hfsmp->hfs_summary_table == NULL) {
        LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: No HFS summary table!");
        hfs_assert(0);
    }

    /* 131072 bits == 16384 bytes.  This is the theoretical max size of the summary table. we add 1 byte for slop */
    if (hfsmp->hfs_summary_size == 0 || hfsmp->hfs_summary_size > 131080) {
        LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: Size is bad! %d", hfsmp->hfs_summary_size);
        hfs_assert(0);
    }

    if (hfsmp->vcbVBMIOSize == 0) {
        LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: no VCB VBM IO Size !");
        hfs_assert(0);
    }

    LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: summary validation beginning on %s\n", hfsmp->vcbVN);
    LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: summary validation %d summary bits, %d summary blocks\n", hfsmp->hfs_summary_size, hfsmp->totalBlocks);

    /* iterate through all possible summary bits */
    for (i = 0; i < hfsmp->hfs_summary_size ; i++) {

        uint32_t bits_per_iosize = hfsmp->vcbVBMIOSize * kBitsPerByte;
        uint32_t byte_offset = hfsmp->vcbVBMIOSize * i;

        /* Compute the corresponding allocation block for the summary bit. */
        uint32_t alloc_block = i * bits_per_iosize;

        /*
         * We use a uint32_t pointer here because it will speed up
         * access to the real bitmap data on disk.
         */
        uint32_t *block_data;
        struct buf *bp;
        int counter;
        int counter_max;
        int saw_free_bits = 0;

        /* Get the block */
        if ((err = ReadBitmapRange (hfsmp, byte_offset, hfsmp->vcbVBMIOSize, &block_data,  &bp))) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: error (%d) in ReadBitmapRange!", err);
            hfs_assert(0);
        }

        /* Query the status of the bit and then make sure we match */
        uint32_t maybe_has_free_blocks;
        err = hfs_check_summary (hfsmp, alloc_block, &maybe_has_free_blocks);
        if (err) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: hfs_check_summary returned error (%d) ", err);
            hfs_assert(0);
        }
        counter_max = hfsmp->vcbVBMIOSize / kBytesPerWord;

        for (counter = 0; counter < counter_max; counter++) {
            uint32_t word = block_data[counter];

            /* We assume that we'll not find any free bits here. */
            if (word != kAllBitsSetInWord) {
                if (maybe_has_free_blocks) {
                    /* All done */
                    saw_free_bits = 1;
                    break;
                } else {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: hfs_check_summary saw free bits!");
                    hfs_assert(0);
                }
            }
        }

        if (maybe_has_free_blocks && (saw_free_bits == 0)) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: did not see free bits !");
            hfs_assert(0);
        }

        /* Release the block. */
        if ((err =  ReleaseScanBitmapRange (bp))) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: Error (%d) in ReleaseScanBitmapRange", err);
            hfs_assert(0);
        }
    }

    LFHFS_LOG(LEVEL_ERROR, "hfs_validate_summary: summary validation completed successfully on %s\n", hfsmp->vcbVN);
    return;
}
#endif

/*
 * hfs_alloc_scan_range:
 *
 * This function should be used to scan large ranges of the allocation bitmap
 * at one time.  It makes two key assumptions:
 *
 *         1) Bitmap lock is held during the duration of the call (exclusive)
 *         2) There are no pages in the buffer cache for any of the bitmap
 *         blocks that we may encounter.  It *MUST* be completely empty.
 *
 * The expected use case is when we are scanning the bitmap in full while we are
 * still mounting the filesystem in order to issue TRIMs or build up the summary
 * table for the mount point. It should be done after any potential journal replays
 * are completed and their I/Os fully issued.
 *
 * The key reason for assumption (2) above is that this function will try to issue
 * I/O against the bitmap file in chunks as large a possible -- essentially as
 * much as the buffer layer will handle (1MB).  Because the size of these I/Os
 * is larger than what would be expected during normal runtime we must invalidate
 * the buffers as soon as we are done with them so that they do not persist in
 * the buffer cache for other threads to find, as they'll typically be doing
 * allocation-block size I/Os instead.
 *
 * Input Args:
 *        hfsmp         - hfs mount data structure
 *         startbit     - allocation block # to start our scan. It must be aligned
 *                    on a vcbVBMIOsize boundary.
 *        list        - journal trim list data structure for issuing TRIMs
 *
 * Output Args:
 *        bitToScan     - Return the next bit to scan if this function is called again.
 *                    Caller will supply this into the next invocation
 *                    of this call as 'startbit'.
 */

static int hfs_alloc_scan_range(struct hfsmount *hfsmp, u_int32_t startbit,
                                u_int32_t *bitToScan, struct jnl_trim_list *list) {

    int error;
    int readwrite = 1;
    u_int32_t curAllocBlock;
    GenericLFBufPtr blockRef = NULL;
    u_int32_t *buffer = NULL;
    u_int32_t free_offset = 0; //tracks the start of the current free range
    u_int32_t size = 0; // tracks the length of the current free range.
    u_int32_t iosize = 0; //how much io we should generate against the bitmap
    u_int32_t byte_off; // byte offset into the bitmap file.
    u_int32_t completed_size; // how much io was actually completed
    u_int32_t last_bitmap_block;
    u_int32_t current_word;
    u_int32_t word_index = 0;

    /* summary table building */
    uint32_t summary_bit = 0;
    uint32_t saw_free_blocks = 0;
    uint32_t last_marked = 0;

    if (hfsmp->hfs_flags & HFS_READ_ONLY) {
        readwrite = 0;
    }

    /*
     * Compute how much I/O we should generate here.
     * hfs_scan_range_size will validate that the start bit
     * converted into a byte offset into the bitmap file,
     * is aligned on a VBMIOSize boundary.
     */
    error = hfs_scan_range_size (hfsmp, startbit, &iosize);
    if (error) {
        if (ALLOC_DEBUG) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_alloc_scan_range: hfs_scan_range_size error %d\n", error);
            hfs_assert(0);
        }
        return error;
    }

    if (iosize < hfsmp->vcbVBMIOSize) {
        if (ALLOC_DEBUG) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_alloc_scan_range: iosize too small! (iosize %d)\n", iosize);
            hfs_assert(0);
        }
        return EINVAL;
    }

    /* hfs_scan_range_size should have verified startbit.  Convert it to bytes */
    byte_off = startbit / kBitsPerByte;

    /*
     * When the journal replays blocks, it does so by writing directly to the disk
     * device (bypassing any filesystem vnodes and such).  When it finishes its I/Os
     * it also immediately re-reads and invalidates the range covered by the bp so
     * it does not leave anything lingering in the cache (for iosize reasons).
     *
     * As such, it is safe to do large I/Os here with ReadBitmapRange.
     *
     * NOTE: It is not recommended, but it is possible to call the function below
     * on sections of the bitmap that may be in core already as long as the pages are not
     * dirty.  In that case, we'd notice that something starting at that
     * logical block of the bitmap exists in the metadata cache, and we'd check
     * if the iosize requested is the same as what was already allocated for it.
     * Odds are pretty good we're going to request something larger.  In that case,
     * we just free the existing memory associated with the buf and reallocate a
     * larger range. This function should immediately invalidate it as soon as we're
     * done scanning, so this shouldn't cause any coherency issues.
     */

    error = ReadBitmapRange(hfsmp, byte_off, iosize, &buffer, &blockRef);
    if (error) {
        if (ALLOC_DEBUG) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_alloc_scan_range: start %d iosize %d ReadBitmapRange error %d\n", startbit, iosize, error);
            hfs_assert(0);
        }
        return error;
    }

    /*
     * At this point, we have a giant wired buffer that represents some portion of
     * the bitmap file that we want to analyze.   We may not have gotten all 'iosize'
     * bytes though, so clip our ending bit to what we actually read in.
     */
    completed_size = blockRef->uValidBytes;
    last_bitmap_block = completed_size * kBitsPerByte;
    last_bitmap_block = last_bitmap_block + startbit;

    /* Cap the last block to the total number of blocks if required */
    if (last_bitmap_block > hfsmp->totalBlocks) {
        last_bitmap_block = hfsmp->totalBlocks;
    }

    /* curAllocBlock represents the logical block we're analyzing. */
    curAllocBlock = startbit;
    word_index = 0;
    size = 0;

    if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
        if (hfs_get_summary_index (hfsmp, startbit, &summary_bit)) {
            error = EINVAL;
            if (ALLOC_DEBUG) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_alloc_scan_range: Could not acquire summary index for %u", startbit);
                hfs_assert(0);
            }
            return error;
        }
        /*
         * summary_bit should now be set to the summary bit corresponding to
         * the allocation block of the first bit that we're supposed to scan
         */
    }
    saw_free_blocks = 0;

    while (curAllocBlock < last_bitmap_block) {
        u_int32_t bit;

        /* Update the summary table as needed */
        if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE) {
            if (ALLOC_DEBUG) {
                if (hfsmp->hfs_summary_table == NULL) {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_alloc_scan_range: no summary table!");
                    hfs_assert(0);
                }
            }

            uint32_t temp_summary;
            error = hfs_get_summary_index (hfsmp, curAllocBlock, &temp_summary);
            if (error) {
                if (ALLOC_DEBUG) {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_alloc_scan_range: could not get summary index for %u", curAllocBlock);
                    hfs_assert(0);
                }
                return EINVAL;
            }

            if (ALLOC_DEBUG) {
                if (temp_summary < summary_bit) {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_alloc_scan_range: backwards summary bit?\n");
                    hfs_assert(0);
                }
            }

            /*
             * If temp_summary is greater than summary_bit, then this
             * means that the next allocation block crosses a vcbVBMIOSize boundary
             * and we should treat this range of on-disk data as part of a new summary
             * bit.
             */
            if (temp_summary > summary_bit) {
                if (saw_free_blocks == 0) {
                    /* Mark the bit as totally consumed in the summary table */
                    hfs_set_summary (hfsmp, summary_bit, 1);
                }
                else {
                    /* Mark the bit as potentially free in summary table */
                    hfs_set_summary (hfsmp, summary_bit, 0);
                }
                last_marked = summary_bit;
                /*
                 * Any time we set the summary table, update our counter which tracks
                 * what the last bit that was fully marked in the summary table.
                 *
                 * Then reset our marker which says we haven't seen a free bit yet.
                 */
                saw_free_blocks = 0;
                summary_bit = temp_summary;
            }
        } /* End summary table conditions */

        current_word = SWAP_BE32(buffer[word_index]);
        /* Iterate through the word 1 bit at a time... */
        for (bit = 0 ; bit < kBitsPerWord ; bit++, curAllocBlock++) {
            if (curAllocBlock >= last_bitmap_block) {
                break;
            }
            u_int32_t allocated = (current_word & (kHighBitInWordMask >> bit));

            if (allocated) {
                if (size != 0) {
                    if (readwrite) {
                        /* Insert the previously tracked range of free blocks to the trim list */
                        hfs_track_unmap_blocks (hfsmp, free_offset, size, list);
                    }
                    add_free_extent_cache (hfsmp, free_offset, size);
                    size = 0;
                    free_offset = 0;
                }
            }
            else {
                /* Not allocated */
                size++;
                if (free_offset == 0) {
                    /* Start a new run of free spcae at curAllocBlock */
                    free_offset = curAllocBlock;
                }
                if (saw_free_blocks == 0) {
                    saw_free_blocks = 1;
                }
            }
        } /* end for loop iterating through the word */

        if (curAllocBlock < last_bitmap_block) {
            word_index++;
        }

    } /* End while loop (iterates through last_bitmap_block) */


    /*
     * We've (potentially) completed our pass through this region of bitmap,
     * but one thing we may not have done is updated that last summary bit for
     * the last page we scanned, because we would have never transitioned across
     * a vcbVBMIOSize boundary again.  Check for that and update the last bit
     * as needed.
     *
     * Note that 'last_bitmap_block' is *not* inclusive WRT the very last bit in the bitmap
     * for the region of bitmap on-disk that we were scanning. (it is one greater).
     */
    if ((curAllocBlock >= last_bitmap_block) &&
        (hfsmp->hfs_flags & HFS_SUMMARY_TABLE)) {
        uint32_t temp_summary;
        /* temp_block should be INSIDE the region we just scanned, so subtract 1 */
        uint32_t temp_block = last_bitmap_block - 1;
        error = hfs_get_summary_index (hfsmp, temp_block, &temp_summary);
        if (error) {
            if (ALLOC_DEBUG) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_alloc_scan_range: end bit curAllocBlock %u, last_bitmap_block %u", curAllocBlock, last_bitmap_block);
                hfs_assert(0);
            }
            return EINVAL;
        }

        /* Did we already update this in the table? */
        if (temp_summary > last_marked) {
            if (saw_free_blocks == 0) {
                hfs_set_summary (hfsmp, temp_summary, 1);
            }
            else {
                hfs_set_summary (hfsmp, temp_summary, 0);
            }
        }
    }

    /*
     * We may have been tracking a range of free blocks that hasn't been inserted yet.
     * Keep the logic for the TRIM and free extent separate from that of the summary
     * table management even though they are closely linked.
     */
    if (size != 0) {
        if (readwrite) {
            hfs_track_unmap_blocks (hfsmp, free_offset, size, list);
        }
        add_free_extent_cache (hfsmp, free_offset, size);
    }

    /*
     * curAllocBlock represents the next block we need to scan when we return
     * to this function.
     */
    *bitToScan = curAllocBlock;
    ReleaseScanBitmapRange(blockRef);

    return 0;

}



/*
 * Compute the maximum I/O size to generate against the bitmap file
 * Will attempt to generate at LEAST VBMIOsize I/Os for interior ranges of the bitmap.
 *
 * Inputs:
 *        hfsmp        -- hfsmount to look at
 *        bitmap_off     -- bit offset into the bitmap file
 *
 * Outputs:
 *         iosize    -- iosize to generate.
 *
 * Returns:
 *        0 on success; EINVAL otherwise
 */
static int hfs_scan_range_size (struct hfsmount *hfsmp, uint32_t bitmap_st, uint32_t *iosize) {

    /*
     * The maximum bitmap size is 512MB regardless of ABN size, so we can get away
     * with 32 bit math in this function.
     */

    uint32_t bitmap_len;
    uint32_t remaining_bitmap;
    uint32_t target_iosize;
    uint32_t bitmap_off;

    /* Is this bit index not word aligned?  If so, immediately fail. */
    if (bitmap_st % kBitsPerWord) {
        if (ALLOC_DEBUG) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_scan_range_size: unaligned start bit! bitmap_st %d \n", bitmap_st);
            hfs_assert(0);
        }
        return EINVAL;
    }

    /* bitmap_off is in bytes, not allocation blocks/bits */
    bitmap_off = bitmap_st / kBitsPerByte;

    if ((hfsmp->totalBlocks <= bitmap_st) || (bitmap_off > (512 * 1024 * 1024))) {
        if (ALLOC_DEBUG) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_scan_range_size: invalid start! bitmap_st %d, bitmap_off %d\n", bitmap_st, bitmap_off);
            hfs_assert(0);
        }
        return EINVAL;
    }

    /*
     * Also invalid if it's not at least aligned to HFS bitmap logical
     * block boundaries.  We don't have to emit an iosize that's an
     * exact multiple of the VBMIOSize, but it must start on such
     * a boundary.
     *
     * The vcbVBMIOSize may be SMALLER than the allocation block size
     * on a FS with giant allocation blocks, but it will never be
     * greater than it, so it should be safe to start I/O
     * aligned on a VBMIOsize boundary.
     */
    if (bitmap_off & (hfsmp->vcbVBMIOSize - 1)) {
        if (ALLOC_DEBUG) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_scan_range_size: unaligned start! bitmap_off %d\n", bitmap_off);
            hfs_assert(0);
        }
        return EINVAL;
    }

    /*
     * Generate the total bitmap file length in bytes, then round up
     * that value to the end of the last allocation block, if needed (It
     * will probably be needed).  We won't scan past the last actual
     * allocation block.
     *
     * Unless we're completing the bitmap scan (or bitmap < 1MB), we
     * have to complete the I/O on VBMIOSize boundaries, but we can only read
     * up until the end of the bitmap file.
     */
    bitmap_len = roundup(hfsmp->totalBlocks, hfsmp->blockSize * 8) / 8;

    remaining_bitmap = bitmap_len - bitmap_off;

    /*
     * io size is the MIN of the maximum I/O we can generate or the
     * remaining amount of bitmap.
     */
    target_iosize = MIN((MAXBSIZE), remaining_bitmap);
    *iosize = target_iosize;

    return 0;
}

/*
 * Remove an extent from the list of free extents.
 *
 * This is a low-level routine.     It does not handle overlaps or splitting;
 * that is the responsibility of the caller.  The input extent must exactly
 * match an extent already in the list; it will be removed, and any following
 * extents in the list will be shifted up.
 *
 * Inputs:
 *    startBlock - Start of extent to remove
 *    blockCount - Number of blocks in extent to remove
 *
 * Result:
 *    The index of the extent that was removed.
 */
static void remove_free_extent_list(struct hfsmount *hfsmp, int index)
{
    if (index < 0 || (uint32_t)index >= hfsmp->vcbFreeExtCnt) {
        if (ALLOC_DEBUG)
        {
            LFHFS_LOG(LEVEL_ERROR, "remove_free_extent_list: %p: index (%d) out of range (0, %u)", hfsmp, index, hfsmp->vcbFreeExtCnt);
            hfs_assert(0);
        }
        else
            LFHFS_LOG(LEVEL_ERROR, "remove_free_extent_list: %p: index (%d) out of range (0, %u)", hfsmp, index, hfsmp->vcbFreeExtCnt);
        return;
    }
    int shift_count = hfsmp->vcbFreeExtCnt - index - 1;
    if (shift_count > 0) {
        memmove(&hfsmp->vcbFreeExt[index], &hfsmp->vcbFreeExt[index+1], shift_count * sizeof(hfsmp->vcbFreeExt[0]));
    }
    hfsmp->vcbFreeExtCnt--;
}


/*
 * Add an extent to the list of free extents.
 *
 * This is a low-level routine.     It does not handle overlaps or coalescing;
 * that is the responsibility of the caller.  This routine *does* make
 * sure that the extent it is adding is inserted in the correct location.
 * If the list is full, this routine will handle either removing the last
 * extent in the list to make room for the new extent, or ignoring the
 * new extent if it is "worse" than the last extent in the list.
 *
 * Inputs:
 *    startBlock - Start of extent to add
 *    blockCount - Number of blocks in extent to add
 *
 * Result:
 *    The index where the extent that was inserted, or kMaxFreeExtents
 *    if the extent was not inserted (the list was full, and the extent
 *    being added was "worse" than everything in the list).
 */
static int add_free_extent_list(struct hfsmount *hfsmp, u_int32_t startBlock, u_int32_t blockCount)
{
    uint32_t i;

    /* ALLOC_DEBUG: Make sure no extents in the list overlap or are contiguous with the input extent. */
    if (ALLOC_DEBUG) {
        uint32_t endBlock = startBlock + blockCount;
        for (i = 0; i < hfsmp->vcbFreeExtCnt; ++i) {
            if (endBlock < hfsmp->vcbFreeExt[i].startBlock ||
                startBlock > (hfsmp->vcbFreeExt[i].startBlock + hfsmp->vcbFreeExt[i].blockCount)) {
                continue;
            }
            LFHFS_LOG(LEVEL_ERROR, "add_free_extent_list: extent(%u %u) overlaps existing extent (%u %u) at index %d",
                      startBlock, blockCount, hfsmp->vcbFreeExt[i].startBlock, hfsmp->vcbFreeExt[i].blockCount, i);
            hfs_assert(0);
        }
    }

    /* Figure out what index the new extent should be inserted at. */
    for (i = 0; i < hfsmp->vcbFreeExtCnt; ++i) {
        /* The list is sorted by decreasing size. */
        if (blockCount > hfsmp->vcbFreeExt[i].blockCount) {
            break;
        }
    }

    /* When we get here, i is the index where the extent should be inserted. */
    if (i == kMaxFreeExtents) {
        /*
         * The new extent is worse than anything already in the list,
         * and the list is full, so just ignore the extent to be added.
         */
        return i;
    }

    /*
     * Grow the list (if possible) to make room for an insert.
     */
    if (hfsmp->vcbFreeExtCnt < kMaxFreeExtents)
        hfsmp->vcbFreeExtCnt++;

    /*
     * If we'll be keeping any extents after the insert position, then shift them.
     */
    int shift_count = hfsmp->vcbFreeExtCnt - i - 1;
    if (shift_count > 0) {
        memmove(&hfsmp->vcbFreeExt[i+1], &hfsmp->vcbFreeExt[i], shift_count * sizeof(hfsmp->vcbFreeExt[0]));
    }

    /* Finally, store the new extent at its correct position. */
    hfsmp->vcbFreeExt[i].startBlock = startBlock;
    hfsmp->vcbFreeExt[i].blockCount = blockCount;
    return i;
}


/*
 * Remove an entry from free extent cache after it has been allocated.
 *
 * This is a high-level routine.  It handles removing a portion of a
 * cached extent, potentially splitting it into two (if the cache was
 * already full, throwing away the extent that would sort last).  It
 * also handles removing an extent that overlaps multiple extents in
 * the cache.
 *
 * Inputs:
 *    hfsmp        - mount point structure
 *    startBlock    - starting block of the extent to be removed.
 *    blockCount    - number of blocks of the extent to be removed.
 */
static void remove_free_extent_cache(struct hfsmount *hfsmp, u_int32_t startBlock, u_int32_t blockCount)
{
    u_int32_t i, insertedIndex;
    u_int32_t currentStart, currentEnd, endBlock;
    int extentsRemoved = 0;

    endBlock = startBlock + blockCount;

    lf_lck_spin_lock(&hfsmp->vcbFreeExtLock);

    /*
     * Iterate over all of the extents in the free extent cache, removing or
     * updating any entries that overlap with the input extent.
     */
    for (i = 0; i < hfsmp->vcbFreeExtCnt; ++i) {
        currentStart = hfsmp->vcbFreeExt[i].startBlock;
        currentEnd = currentStart + hfsmp->vcbFreeExt[i].blockCount;

        /*
         * If the current extent is entirely before or entirely after the
         * the extent to be removed, then we keep it as-is.
         */
        if (currentEnd <= startBlock || currentStart >= endBlock) {
            continue;
        }

        /*
         * If the extent being removed entirely contains the current extent,
         * then remove the current extent.
         */
        if (startBlock <= currentStart && endBlock >= currentEnd) {
            remove_free_extent_list(hfsmp, i);

            /*
             * We just removed the extent at index i.  The extent at
             * index i+1 just got shifted to index i.  So decrement i
             * to undo the loop's "++i", and the next iteration will
             * examine index i again, which contains the next extent
             * in the list.
             */
            --i;
            ++extentsRemoved;
            continue;
        }

        /*
         * If the extent being removed is strictly "in the middle" of the
         * current extent, then we need to split the current extent into
         * two discontiguous extents (the "head" and "tail").  The good
         * news is that we don't need to examine any other extents in
         * the list.
         */
        if (startBlock > currentStart && endBlock < currentEnd) {
            remove_free_extent_list(hfsmp, i);
            add_free_extent_list(hfsmp, currentStart, startBlock - currentStart);
            add_free_extent_list(hfsmp, endBlock, currentEnd - endBlock);
            break;
        }

        /*
         * The only remaining possibility is that the extent to be removed
         * overlaps the start or end (but not both!) of the current extent.
         * So we need to replace the current extent with a shorter one.
         *
         * The only tricky part is that the updated extent might be at a
         * different index than the original extent.  If the updated extent
         * was inserted after the current extent, then we need to re-examine
         * the entry at index i, since it now contains the extent that was
         * previously at index i+1.     If the updated extent was inserted
         * before or at the same index as the removed extent, then the
         * following extents haven't changed position.
         */
        remove_free_extent_list(hfsmp, i);
        if (startBlock > currentStart) {
            /* Remove the tail of the current extent. */
            insertedIndex = add_free_extent_list(hfsmp, currentStart, startBlock - currentStart);
        } else {
            /* Remove the head of the current extent. */
            insertedIndex = add_free_extent_list(hfsmp, endBlock, currentEnd - endBlock);
        }
        if (insertedIndex > i) {
            --i;    /* Undo the "++i" in the loop, so we examine the entry at index i again. */
        }
    }

    lf_lck_spin_unlock(&hfsmp->vcbFreeExtLock);
    sanity_check_free_ext(hfsmp, 0);

    return;
}


/*
 * Add an entry to free extent cache after it has been deallocated.
 *
 * This is a high-level routine.  It will merge overlapping or contiguous
 * extents into a single, larger extent.
 *
 * If the extent provided has blocks beyond current allocLimit, it is
 * clipped to allocLimit (so that we won't accidentally find and allocate
 * space beyond allocLimit).
 *
 * Inputs:
 *    hfsmp        - mount point structure
 *    startBlock    - starting block of the extent to be removed.
 *    blockCount    - number of blocks of the extent to be removed.
 *
 * Returns:
 *    true        - if the extent was added successfully to the list
 *    false        - if the extent was not added to the list, maybe because
 *              the extent was beyond allocLimit, or is not best
 *              candidate to be put in the cache.
 */
static Boolean add_free_extent_cache(struct hfsmount *hfsmp, u_int32_t startBlock, u_int32_t blockCount)
{
    Boolean retval = false;
    uint32_t endBlock;
    uint32_t currentEnd;
    uint32_t i;

#if DEBUG
    for (i = 0; i < 2; ++i) {
        struct rl_entry *range;
        TAILQ_FOREACH(range, &hfsmp->hfs_reserved_ranges[i], rl_link) {
            hfs_assert(rl_overlap(range, startBlock,
                                  startBlock + blockCount - 1) == RL_NOOVERLAP);
        }
    }
#endif

    /* No need to add extent that is beyond current allocLimit */
    if (startBlock >= hfsmp->allocLimit) {
        goto out_not_locked;
    }

    /* If end of the free extent is beyond current allocLimit, clip the extent */
    if ((startBlock + blockCount) > hfsmp->allocLimit) {
        blockCount = hfsmp->allocLimit - startBlock;
    }

    lf_lck_spin_lock(&hfsmp->vcbFreeExtLock);

    /*
     * Make a pass through the free extent cache, looking for known extents that
     * overlap or are contiguous with the extent to be added.  We'll remove those
     * extents from the cache, and incorporate them into the new extent to be added.
     */
    endBlock = startBlock + blockCount;
    for (i=0; i < hfsmp->vcbFreeExtCnt; ++i) {
        currentEnd = hfsmp->vcbFreeExt[i].startBlock + hfsmp->vcbFreeExt[i].blockCount;
        if (hfsmp->vcbFreeExt[i].startBlock > endBlock || currentEnd < startBlock) {
            /* Extent i does not overlap and is not contiguous, so keep it. */
            continue;
        } else {
            /* We need to remove extent i and combine it with the input extent. */
            if (hfsmp->vcbFreeExt[i].startBlock < startBlock)
                startBlock = hfsmp->vcbFreeExt[i].startBlock;
            if (currentEnd > endBlock)
                endBlock = currentEnd;

            remove_free_extent_list(hfsmp, i);
            /*
             * We just removed the extent at index i.  The extent at
             * index i+1 just got shifted to index i.  So decrement i
             * to undo the loop's "++i", and the next iteration will
             * examine index i again, which contains the next extent
             * in the list.
             */
            --i;
        }
    }
    add_free_extent_list(hfsmp, startBlock, endBlock - startBlock);

    lf_lck_spin_unlock(&hfsmp->vcbFreeExtLock);

out_not_locked:
    sanity_check_free_ext(hfsmp, 0);

    return retval;
}

/* Debug function to check if the free extent cache is good or not */
static void sanity_check_free_ext(struct hfsmount *hfsmp, int check_allocated)
{
    u_int32_t i, j;

    /* Do not do anything if debug is not on */
    if (ALLOC_DEBUG == 0) {
        return;
    }

    lf_lck_spin_lock(&hfsmp->vcbFreeExtLock);

    if (hfsmp->vcbFreeExtCnt > kMaxFreeExtents)
    {
        LFHFS_LOG(LEVEL_ERROR, "sanity_check_free_ext: free extent count (%u) is too large", hfsmp->vcbFreeExtCnt);
    }

    /*
     * Iterate the Free extent cache and ensure no entries are bogus or refer to
     * allocated blocks.
     */
    for(i=0; i < hfsmp->vcbFreeExtCnt; i++) {
        u_int32_t start, nblocks;

        start   = hfsmp->vcbFreeExt[i].startBlock;
        nblocks = hfsmp->vcbFreeExt[i].blockCount;

        /* Check if any of the blocks in free extent cache are allocated.
         * This should not be enabled always because it might take
         * very long for large extents that get added to the list.
         *
         * We have to drop vcbFreeExtLock while we call hfs_isallocated
         * because it is going to do I/O.  Note that the free extent
         * cache could change.  That's a risk we take when using this
         * debugging code.  (Another alternative would be to try to
         * detect when the free extent cache changed, and perhaps
         * restart if the list changed while we dropped the lock.)
         */
        if (check_allocated) {
            lf_lck_spin_unlock(&hfsmp->vcbFreeExtLock);
            if (hfs_isallocated(hfsmp, start, nblocks)) {
                LFHFS_LOG(LEVEL_ERROR, "sanity_check_free_ext: slot %d:(%u,%u) in the free extent array is allocated\n",
                          i, start, nblocks);
                hfs_assert(0);
            }
            lf_lck_spin_lock(&hfsmp->vcbFreeExtLock);
        }

        /* Check if any part of the extent is beyond allocLimit */
        if ((start > hfsmp->allocLimit) || ((start + nblocks) > hfsmp->allocLimit)) {
            LFHFS_LOG(LEVEL_ERROR, "sanity_check_free_ext: slot %d:(%u,%u) in the free extent array is beyond allocLimit=%u\n",
                      i, start, nblocks, hfsmp->allocLimit);
            hfs_assert(0);
        }

        /* Check if there are any duplicate start blocks */
        for(j=i+1; j < hfsmp->vcbFreeExtCnt; j++) {
            if (start == hfsmp->vcbFreeExt[j].startBlock) {
                LFHFS_LOG(LEVEL_ERROR, "sanity_check_free_ext: slot %d:(%u,%u) and %d:(%u,%u) are duplicate\n",
                          i, start, nblocks, j, hfsmp->vcbFreeExt[j].startBlock,
                          hfsmp->vcbFreeExt[j].blockCount);
                hfs_assert(0);
            }
        }

        /* Check if the entries are out of order */
        if ((i+1) != hfsmp->vcbFreeExtCnt) {
            /* normally sorted by block count (descending) */
            if (hfsmp->vcbFreeExt[i].blockCount < hfsmp->vcbFreeExt[i+1].blockCount) {
                LFHFS_LOG(LEVEL_ERROR, "sanity_check_free_ext: %d:(%u,%u) and %d:(%u,%u) are out of order\n",
                          i, start, nblocks, i+1, hfsmp->vcbFreeExt[i+1].startBlock,
                          hfsmp->vcbFreeExt[i+1].blockCount);
                hfs_assert(0);
            }
        }
    }
    lf_lck_spin_unlock(&hfsmp->vcbFreeExtLock);
}
