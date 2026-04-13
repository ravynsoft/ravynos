//
//  lf_hfs_readwrite_ops.c
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#include "lf_hfs_readwrite_ops.h"
#include "lf_hfs_rangelist.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_file_extent_mapping.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_cnode.h"
#include "lf_hfs_file_mgr_internal.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_vnops.h"
#include "lf_hfs_raw_read_write.h"

#include <assert.h>

static int  do_hfs_truncate(struct vnode *vp, off_t length, int flags, int skip);

static int
do_hfs_truncate(struct vnode *vp, off_t length, int flags, int truncateflags)
{
    register struct cnode *cp = VTOC(vp);
    struct filefork *fp = VTOF(vp);
    int retval;
    off_t bytesToAdd;
    off_t actualBytesAdded;
    off_t filebytes;
    u_int32_t fileblocks;
    int blksize;
    struct hfsmount *hfsmp;
    int lockflags;
    int suppress_times = (truncateflags & HFS_TRUNCATE_SKIPTIMES);

    blksize = VTOVCB(vp)->blockSize;
    fileblocks = fp->ff_blocks;
    filebytes = (off_t)fileblocks * (off_t)blksize;

    if (length < 0)
        return (EINVAL);

    /* This should only happen with a corrupt filesystem */
    if ((off_t)fp->ff_size < 0)
        return (EINVAL);

    hfsmp = VTOHFS(vp);

    retval = E_NONE;
    /*
     * Lengthen the size of the file. We must ensure that the
     * last byte of the file is allocated. Since the smallest
     * value of ff_size is 0, length will be at least 1.
     */
    if (length > (off_t)fp->ff_size) {
        /*
         * If we don't have enough physical space then
         * we need to extend the physical size.
         */
        if (length > filebytes) {
            int eflags = kEFReserveMask;
            u_int32_t blockHint = 0;

            /* All or nothing and don't round up to clumpsize. */
            eflags |= kEFAllMask | kEFNoClumpMask;

            if (hfs_start_transaction(hfsmp) != 0) {
                retval = EINVAL;
                goto Err_Exit;
            }

            /* Protect extents b-tree and allocation bitmap */
            lockflags = SFL_BITMAP;
            if (overflow_extents(fp))
                lockflags |= SFL_EXTENTS;
            lockflags = hfs_systemfile_lock(hfsmp, lockflags, HFS_EXCLUSIVE_LOCK);

            /*
             * Keep growing the file as long as the current EOF is
             * less than the desired value.
             */
            while ((length > filebytes) && (retval == E_NONE)) {
                bytesToAdd = length - filebytes;
                retval = MacToVFSError(ExtendFileC(VTOVCB(vp),
                                                   (FCB*)fp,
                                                   bytesToAdd,
                                                   blockHint,
                                                   eflags,
                                                   &actualBytesAdded));

                filebytes = (off_t)fp->ff_blocks * (off_t)blksize;
                if (actualBytesAdded == 0 && retval == E_NONE) {
                    if (length > filebytes)
                        length = filebytes;
                    break;
                }
            } /* endwhile */

            hfs_systemfile_unlock(hfsmp, lockflags);

            if (hfsmp->jnl) {
                hfs_update(vp, 0);
                hfs_volupdate(hfsmp, VOL_UPDATE, 0);
            }

            hfs_end_transaction(hfsmp);

            if (retval)
                goto Err_Exit;
        }

        if (ISSET(flags, IO_NOZEROFILL))
        {

        }
        else
        {
            if (!vnode_issystem(vp) && retval == E_NONE) {
                if (length > (off_t)fp->ff_size) {
                    struct timeval tv;

                    /* Extending the file: time to fill out the current last page w. zeroes? */
                    retval = raw_readwrite_zero_fill_last_block_suffix(vp);
                    if (retval) goto Err_Exit;

                    microuptime(&tv);
//                    Currently disabling the rl_add, sice the
//                    data is being filled with 0's and that a valid content for us
//                    rl_add(fp->ff_size, length - 1, &fp->ff_invalidranges);
                    cp->c_zftimeout = (uint32_t)tv.tv_sec + ZFTIMELIMIT;
                }
            }else{
                LFHFS_LOG(LEVEL_ERROR, "hfs_truncate: invoked on non-UBC object?!");
                hfs_assert(0);
            }
        }
        if (suppress_times == 0) {
            cp->c_touch_modtime = TRUE;
        }
        fp->ff_size = length;

    } else { /* Shorten the size of the file */

        if ((off_t)fp->ff_size > length) {
            /* Any space previously marked as invalid is now irrelevant: */
            rl_remove(length, fp->ff_size - 1, &fp->ff_invalidranges);
        }

        /*
         * Account for any unmapped blocks. Note that the new
         * file length can still end up with unmapped blocks.
         */
        if (fp->ff_unallocblocks > 0) {
            u_int32_t finalblks;
            u_int32_t loanedBlocks;

            hfs_lock_mount(hfsmp);
            loanedBlocks = fp->ff_unallocblocks;
            cp->c_blocks -= loanedBlocks;
            fp->ff_blocks -= loanedBlocks;
            fp->ff_unallocblocks = 0;

            hfsmp->loanedBlocks -= loanedBlocks;

            finalblks = (uint32_t)((length + blksize - 1) / blksize);
            if (finalblks > fp->ff_blocks) {
                /* calculate required unmapped blocks */
                loanedBlocks = finalblks - fp->ff_blocks;
                hfsmp->loanedBlocks += loanedBlocks;

                fp->ff_unallocblocks = loanedBlocks;
                cp->c_blocks += loanedBlocks;
                fp->ff_blocks += loanedBlocks;
            }
            hfs_unlock_mount (hfsmp);
        }
        if (hfs_start_transaction(hfsmp) != 0) {
            retval = EINVAL;
            goto Err_Exit;
        }

        if (fp->ff_unallocblocks == 0) {
            /* Protect extents b-tree and allocation bitmap */
            lockflags = SFL_BITMAP;
            if (overflow_extents(fp))
                lockflags |= SFL_EXTENTS;
            lockflags = hfs_systemfile_lock(hfsmp, lockflags, HFS_EXCLUSIVE_LOCK);

            retval = MacToVFSError(TruncateFileC(VTOVCB(vp), (FCB*)fp, length, 0,
                                                 FORK_IS_RSRC (fp), FTOC(fp)->c_fileid, false));

            hfs_systemfile_unlock(hfsmp, lockflags);
        }
        if (hfsmp->jnl) {
            if (retval == 0) {
                fp->ff_size = length;
            }
            hfs_update(vp, 0);
            hfs_volupdate(hfsmp, VOL_UPDATE, 0);
        }

        hfs_end_transaction(hfsmp);

        if (retval) goto Err_Exit;

        /*
         * Only set update flag if the logical length changes & we aren't
         * suppressing modtime updates.
         */
        if (((off_t)fp->ff_size != length) && (suppress_times == 0)) {
            cp->c_touch_modtime = TRUE;
        }
        fp->ff_size = length;
    }
    
    cp->c_flag |= C_MODIFIED;
    cp->c_touch_chgtime = TRUE;    /* status changed */
    if (suppress_times == 0) {
        cp->c_touch_modtime = TRUE;    /* file data was modified */

        /*
         * If we are not suppressing the modtime update, then
         * update the gen count as well.
         */
        if (S_ISREG(cp->c_attr.ca_mode) || S_ISLNK (cp->c_attr.ca_mode)) {
            hfs_incr_gencount(cp);
        }
    }

    retval = hfs_update(vp, 0);

Err_Exit:

    return (retval);
}

int
hfs_vnop_blockmap(struct vnop_blockmap_args *ap)
{
    struct vnode *vp = ap->a_vp;
    struct cnode *cp;
    struct filefork *fp;
    struct hfsmount *hfsmp;
    size_t bytesContAvail = ap->a_size;
    int retval = E_NONE;
    int syslocks = 0;
    int lockflags = 0;
    struct rl_entry *invalid_range;
    enum rl_overlaptype overlaptype;
    int started_tr = 0;
    int tooklock = 0;

    /* Do not allow blockmap operation on a directory */
    if (vnode_isdir(vp)) {
        return (ENOTSUP);
    }

    /*
     * Check for underlying vnode requests and ensure that logical
     * to physical mapping is requested.
     */
    if (ap->a_bpn == NULL)
        return (0);

    hfsmp = VTOHFS(vp);
    cp = VTOC(vp);
    fp = VTOF(vp);

    if ( !vnode_issystem(vp) && !vnode_islnk(vp) ) {
        if (cp->c_lockowner != pthread_self()) {
            hfs_lock(VTOC(vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS);
            tooklock = 1;
        }

        // For reads, check the invalid ranges
        if (ISSET(ap->a_flags, VNODE_READ)) {
            if (ap->a_foffset >= fp->ff_size) {
                retval = ERANGE;
                goto exit;
            }

            overlaptype = rl_scan(&fp->ff_invalidranges, ap->a_foffset,
                                  ap->a_foffset + (off_t)bytesContAvail - 1,
                                  &invalid_range);
            switch(overlaptype) {
                case RL_MATCHINGOVERLAP:
                case RL_OVERLAPCONTAINSRANGE:
                case RL_OVERLAPSTARTSBEFORE:
                    /* There's no valid block for this byte offset */
                    *ap->a_bpn = (daddr64_t)-1;
                    /* There's no point limiting the amount to be returned
                     * if the invalid range that was hit extends all the way
                     * to the EOF (i.e. there's no valid bytes between the
                     * end of this range and the file's EOF):
                     */
                    if (((off_t)fp->ff_size > (invalid_range->rl_end + 1)) &&
                        ((size_t)(invalid_range->rl_end + 1 - ap->a_foffset) < bytesContAvail)) {
                        bytesContAvail = invalid_range->rl_end + 1 - ap->a_foffset;
                    }

                    retval = 0;
                    goto exit;

                case RL_OVERLAPISCONTAINED:
                case RL_OVERLAPENDSAFTER:
                    /* The range of interest hits an invalid block before the end: */
                    if (invalid_range->rl_start == ap->a_foffset) {
                        /* There's actually no valid information to be had starting here: */
                        *ap->a_bpn = (daddr64_t)-1;
                        if (((off_t)fp->ff_size > (invalid_range->rl_end + 1)) &&
                            ((size_t)(invalid_range->rl_end + 1 - ap->a_foffset) < bytesContAvail)) {
                            bytesContAvail = invalid_range->rl_end + 1 - ap->a_foffset;
                        }

                        retval = 0;
                        goto exit;
                    } else {
                        /*
                         * Sadly, the lower layers don't like us to
                         * return unaligned ranges, so we skip over
                         * any invalid ranges here that are less than
                         * a page: zeroing of those bits is not our
                         * responsibility (it's dealt with elsewhere).
                         */
                        do {
                            off_t rounded_start = (((uint64_t)(invalid_range->rl_start) + (off_t)PAGE_MASK) & ~((off_t)PAGE_MASK));
                            if ((off_t)bytesContAvail < rounded_start - ap->a_foffset)
                                break;
                            if (rounded_start < invalid_range->rl_end + 1) {
                                bytesContAvail = rounded_start - ap->a_foffset;
                                break;
                            }
                        } while ((invalid_range = TAILQ_NEXT(invalid_range,
                                                             rl_link)));
                    }
                    break;

                case RL_NOOVERLAP:
                    break;
            } // switch
        }
    }

retry:

    /* Check virtual blocks only when performing write operation */
    if ((ap->a_flags & VNODE_WRITE) && (fp->ff_unallocblocks != 0)) {
        if (hfs_start_transaction(hfsmp) != 0) {
            retval = EINVAL;
            goto exit;
        } else {
            started_tr = 1;
        }
        syslocks = SFL_EXTENTS | SFL_BITMAP;

    } else if (overflow_extents(fp)) {
        syslocks = SFL_EXTENTS;
    }

    if (syslocks)
        lockflags = hfs_systemfile_lock(hfsmp, syslocks, HFS_EXCLUSIVE_LOCK);

    /*
     * Check for any delayed allocations.
     */
    if ((ap->a_flags & VNODE_WRITE) && (fp->ff_unallocblocks != 0)) {
        int64_t actbytes;
        u_int32_t loanedBlocks;

        //
        // Make sure we have a transaction.  It's possible
        // that we came in and fp->ff_unallocblocks was zero
        // but during the time we blocked acquiring the extents
        // btree, ff_unallocblocks became non-zero and so we
        // will need to start a transaction.
        //
        if (started_tr == 0) {
            if (syslocks) {
                hfs_systemfile_unlock(hfsmp, lockflags);
                syslocks = 0;
            }
            goto retry;
        }

        /*
         * Note: ExtendFileC will Release any blocks on loan and
         * aquire real blocks.  So we ask to extend by zero bytes
         * since ExtendFileC will account for the virtual blocks.
         */

        loanedBlocks = fp->ff_unallocblocks;
        retval = ExtendFileC(hfsmp, (FCB*)fp, 0, 0,
                             kEFAllMask | kEFNoClumpMask, &actbytes);

        if (retval) {
            fp->ff_unallocblocks = loanedBlocks;
            cp->c_blocks += loanedBlocks;
            fp->ff_blocks += loanedBlocks;

            hfs_lock_mount (hfsmp);
            hfsmp->loanedBlocks += loanedBlocks;
            hfs_unlock_mount (hfsmp);

            hfs_systemfile_unlock(hfsmp, lockflags);
            cp->c_flag |= C_MODIFIED;
            if (started_tr) {
                (void) hfs_update(vp, 0);
                (void) hfs_volupdate(hfsmp, VOL_UPDATE, 0);

                hfs_end_transaction(hfsmp);
                started_tr = 0;
            }
            goto exit;
        }
    }

    retval = MapFileBlockC(hfsmp, (FCB *)fp, bytesContAvail, ap->a_foffset,
                           ap->a_bpn, &bytesContAvail);
    if (syslocks) {
        hfs_systemfile_unlock(hfsmp, lockflags);
    }

    if (retval) {
        /* On write, always return error because virtual blocks, if any,
         * should have been allocated in ExtendFileC().  We do not
         * allocate virtual blocks on read, therefore return error
         * only if no virtual blocks are allocated.  Otherwise we search
         * rangelist for zero-fills
         */
        if ((MacToVFSError(retval) != ERANGE) ||
            (ap->a_flags & VNODE_WRITE) ||
            ((ap->a_flags & VNODE_READ) && (fp->ff_unallocblocks == 0))) {
            goto exit;
        }

        /* Validate if the start offset is within logical file size */
        if (ap->a_foffset >= fp->ff_size) {
            goto exit;
        }

        /*
         * At this point, we have encountered a failure during
         * MapFileBlockC that resulted in ERANGE, and we are not
         * servicing a write, and there are borrowed blocks.
         *
         * However, the cluster layer will not call blockmap for
         * blocks that are borrowed and in-cache.  We have to assume
         * that because we observed ERANGE being emitted from
         * MapFileBlockC, this extent range is not valid on-disk.  So
         * we treat this as a mapping that needs to be zero-filled
         * prior to reading.
         */

        if (fp->ff_size - ap->a_foffset < (off_t)bytesContAvail)
            bytesContAvail = fp->ff_size - ap->a_foffset;

        *ap->a_bpn = (daddr64_t) -1;
        retval = 0;

        goto exit;
    }

exit:
    if (retval == 0) {
        if (ISSET(ap->a_flags, VNODE_WRITE)) {
            struct rl_entry *r = TAILQ_FIRST(&fp->ff_invalidranges);

            // See if we might be overlapping invalid ranges...
            if (r && (ap->a_foffset + (off_t)bytesContAvail) > r->rl_start) {
                /*
                 * Mark the file as needing an update if we think the
                 * on-disk EOF has changed.
                 */
                if (ap->a_foffset <= r->rl_start)
                    SET(cp->c_flag, C_MODIFIED);

                /*
                 * This isn't the ideal place to put this.  Ideally, we
                 * should do something *after* we have successfully
                 * written to the range, but that's difficult to do
                 * because we cannot take locks in the callback.  At
                 * present, the cluster code will call us with VNODE_WRITE
                 * set just before it's about to write the data so we know
                 * that data is about to be written.  If we get an I/O
                 * error at this point then chances are the metadata
                 * update to follow will also have an I/O error so the
                 * risk here is small.
                 */
                rl_remove(ap->a_foffset, ap->a_foffset + bytesContAvail - 1,
                          &fp->ff_invalidranges);

                if (!TAILQ_FIRST(&fp->ff_invalidranges)) {
                    cp->c_flag &= ~C_ZFWANTSYNC;
                    cp->c_zftimeout = 0;
                }
            }
        }

        if (ap->a_run)
            *ap->a_run = bytesContAvail;

        if (ap->a_poff)
            *(int *)ap->a_poff = 0;
    }

    if (started_tr) {
        hfs_update(vp, TRUE);
        hfs_volupdate(hfsmp, VOL_UPDATE, 0);
        hfs_end_transaction(hfsmp);
    }

    if (tooklock)
        hfs_unlock(cp);

    return (MacToVFSError(retval));
}

int
hfs_prepare_release_storage (struct hfsmount *hfsmp, struct vnode *vp) {

    struct filefork *fp = VTOF(vp);
    struct cnode *cp = VTOC(vp);

    /* Cannot truncate an HFS directory! */
    if (IS_DIR(vp))
    {
        return (EISDIR);
    }

    /* This should only happen with a corrupt filesystem */
    if ((off_t)fp->ff_size < 0)
        return (EINVAL);

    /*
     * We cannot just check if fp->ff_size == length (as an optimization)
     * since there may be extra physical blocks that also need truncation.
     */

    /* Wipe out any invalid ranges which have yet to be backed by disk */
    rl_remove(0, fp->ff_size - 1, &fp->ff_invalidranges);

    /*
     * Account for any unmapped blocks. Since we're deleting the
     * entire file, we don't have to worry about just shrinking
     * to a smaller number of borrowed blocks.
     */
    if (fp->ff_unallocblocks > 0)
    {
        u_int32_t loanedBlocks;

        hfs_lock_mount (hfsmp);
        loanedBlocks = fp->ff_unallocblocks;
        cp->c_blocks -= loanedBlocks;
        fp->ff_blocks -= loanedBlocks;
        fp->ff_unallocblocks = 0;

        hfsmp->loanedBlocks -= loanedBlocks;

        hfs_unlock_mount (hfsmp);
    }

    return 0;
}

int
hfs_release_storage (struct hfsmount *hfsmp, struct filefork *datafork, struct filefork *rsrcfork, u_int32_t fileid)
{
    int error = 0;
    int blksize = hfsmp->blockSize;

    /* Data Fork */
    if (datafork)
    {
        datafork->ff_size = 0;

        u_int32_t fileblocks = datafork->ff_blocks;
        off_t filebytes = (off_t)fileblocks * (off_t)blksize;

        /* We killed invalid ranges and loaned blocks before we removed the catalog entry */

        while (filebytes > 0) {
            if (filebytes > HFS_BIGFILE_SIZE) {
                filebytes -= HFS_BIGFILE_SIZE;
            } else {
                filebytes = 0;
            }

            /* Start a transaction, and wipe out as many blocks as we can in this iteration */
            if (hfs_start_transaction(hfsmp) != 0) {
                error = EINVAL;
                break;
            }

            if (datafork->ff_unallocblocks == 0)
            {
                /* Protect extents b-tree and allocation bitmap */
                int lockflags = SFL_BITMAP;
                if (overflow_extents(datafork))
                    lockflags |= SFL_EXTENTS;
                lockflags = hfs_systemfile_lock(hfsmp, lockflags, HFS_EXCLUSIVE_LOCK);

                error = MacToVFSError(TruncateFileC(HFSTOVCB(hfsmp), datafork, filebytes, 1, 0, fileid, false));

                hfs_systemfile_unlock(hfsmp, lockflags);
            }
            (void) hfs_volupdate(hfsmp, VOL_UPDATE, 0);

            /* Finish the transaction and start over if necessary */
            hfs_end_transaction(hfsmp);

            if (error) {
                break;
            }
        }
    }

    /* Resource fork */
    if (error == 0 && rsrcfork)
    {
        rsrcfork->ff_size = 0;

        u_int32_t fileblocks = rsrcfork->ff_blocks;
        off_t filebytes = (off_t)fileblocks * (off_t)blksize;

        /* We killed invalid ranges and loaned blocks before we removed the catalog entry */

        while (filebytes > 0)
        {
            if (filebytes > HFS_BIGFILE_SIZE)
            {
                filebytes -= HFS_BIGFILE_SIZE;
            }
            else
            {
                filebytes = 0;
            }

            /* Start a transaction, and wipe out as many blocks as we can in this iteration */
            if (hfs_start_transaction(hfsmp) != 0)
            {
                error = EINVAL;
                break;
            }

            if (rsrcfork->ff_unallocblocks == 0)
            {
                /* Protect extents b-tree and allocation bitmap */
                int lockflags = SFL_BITMAP;
                if (overflow_extents(rsrcfork))
                    lockflags |= SFL_EXTENTS;
                lockflags = hfs_systemfile_lock(hfsmp, lockflags, HFS_EXCLUSIVE_LOCK);

                error = MacToVFSError(TruncateFileC(HFSTOVCB(hfsmp), rsrcfork, filebytes, 1, 1, fileid, false));

                hfs_systemfile_unlock(hfsmp, lockflags);
            }
            (void) hfs_volupdate(hfsmp, VOL_UPDATE, 0);

            /* Finish the transaction and start over if necessary */
            hfs_end_transaction(hfsmp);

            if (error)
            {
                break;
            }
        }
    }

    return error;
}

/*
 * Truncate a cnode to at most length size, freeing (or adding) the
 * disk blocks.
 */
int
hfs_truncate(struct vnode *vp, off_t length, int flags, int truncateflags)
{
    struct filefork *fp = VTOF(vp);
    off_t filebytes;
    u_int32_t fileblocks;
    int blksize;
    errno_t error = 0;
    struct cnode *cp = VTOC(vp);
    hfsmount_t *hfsmp = VTOHFS(vp);

    /* Cannot truncate an HFS directory! */
    if (vnode_isdir(vp)) {
        return (EISDIR);
    }

    blksize = hfsmp->blockSize;
    fileblocks = fp->ff_blocks;
    filebytes = (off_t)fileblocks * (off_t)blksize;

    bool caller_has_cnode_lock = (cp->c_lockowner == pthread_self());

    if (!caller_has_cnode_lock) {
        error = hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
        if (error)
            return error;
    }

    if (vnode_islnk(vp) && cp->c_datafork->ff_symlinkptr) {
        hfs_free(cp->c_datafork->ff_symlinkptr);
        cp->c_datafork->ff_symlinkptr = NULL;
    }

    // have to loop truncating or growing files that are
    // really big because otherwise transactions can get
    // enormous and consume too many kernel resources.

    if (length < filebytes) {
        while (filebytes > length) {
            if ((filebytes - length) > HFS_BIGFILE_SIZE) {
                filebytes -= HFS_BIGFILE_SIZE;
            } else {
                filebytes = length;
            }
            error = do_hfs_truncate(vp, filebytes, flags, truncateflags);
            if (error)
                break;
        }
    } else if (length > filebytes) {
        const bool keep_reserve = false; //cred && suser(cred, NULL) != 0;

        if (hfs_freeblks(hfsmp, keep_reserve) < howmany(length - filebytes, blksize))
        {
            error = ENOSPC;
        }
        else
        {
            while (filebytes < length) {
                if ((length - filebytes) > HFS_BIGFILE_SIZE) {
                    filebytes += HFS_BIGFILE_SIZE;
                } else {
                    filebytes = length;
                }
                error = do_hfs_truncate(vp, filebytes, flags, truncateflags);
                if (error)
                    break;
            }
        }
    } else /* Same logical size */ {

        error = do_hfs_truncate(vp, length, flags, truncateflags);
    }

    if (!caller_has_cnode_lock)
        hfs_unlock(cp);

    return error;
}

/*
 * Preallocate file storage space.
 */
int
hfs_vnop_preallocate(struct vnode * vp, LIFilePreallocateArgs_t* psPreAllocReq, LIFilePreallocateArgs_t* psPreAllocRes)
{
    struct cnode *cp = VTOC(vp);
    struct filefork *fp = VTOF(vp);
    struct hfsmount *hfsmp = VTOHFS(vp);
    ExtendedVCB *vcb = VTOVCB(vp);
    int retval = E_NONE , retval2 = E_NONE;

    off_t length = psPreAllocReq->length;
    psPreAllocRes->bytesallocated = 0;

    if (vnode_isdir(vp) || vnode_islnk(vp)) {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_preallocate: Cannot change size of a directory or symlink!");
        return EPERM;
    }
    
    if (length == 0)
        return (0);
    
     if (psPreAllocReq->flags & LI_PREALLOCATE_ALLOCATEFROMVOL){
         LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_preallocate: Not supporting LI_PREALLOCATE_ALLOCATEFROMVOL mode\n");
         return ENOTSUP;
     }
        
    hfs_lock_truncate(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);

    if ((retval = hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT))) {
        goto err_exit;
    }
    
    off_t filebytes = (off_t)fp->ff_blocks * (off_t)vcb->blockSize;
    off_t startingPEOF = filebytes;

    /* If no changes are necesary, then we're done */
    if (filebytes == length)
        goto exit;

    u_int32_t extendFlags = kEFNoClumpMask;
    if (psPreAllocReq->flags & LI_PREALLOCATE_ALLOCATECONTIG)
        extendFlags |= kEFContigMask;
    if (psPreAllocReq->flags & LI_PREALLOCATE_ALLOCATEALL)
        extendFlags |= kEFAllMask;

    
    /*
     * Lengthen the size of the file. We must ensure that the
     * last byte of the file is allocated. Since the smallest
     * value of filebytes is 0, length will be at least 1.
     */
    if (length > filebytes)
    {
        off_t total_bytes_added = 0, orig_request_size, moreBytesRequested, actualBytesAdded;
        orig_request_size = moreBytesRequested = length - filebytes;

        while ((length > filebytes) && (retval == E_NONE))
        {
            off_t bytesRequested;
            
            if (hfs_start_transaction(hfsmp) != 0)
            {
                retval = EINVAL;
                goto err_exit;
            }

            /* Protect extents b-tree and allocation bitmap */
            int lockflags = SFL_BITMAP;
            if (overflow_extents(fp)) 
                lockflags |= SFL_EXTENTS;
            lockflags = hfs_systemfile_lock(hfsmp, lockflags, HFS_EXCLUSIVE_LOCK);

            if (moreBytesRequested >= HFS_BIGFILE_SIZE) {
                bytesRequested = HFS_BIGFILE_SIZE;
            } else {
                bytesRequested = moreBytesRequested;
            }

            retval = MacToVFSError(ExtendFileC(vcb,
                        (FCB*)fp,
                        bytesRequested,
                        0,
                        extendFlags,
                        &actualBytesAdded));

            if (retval == E_NONE)
            {
                psPreAllocRes->bytesallocated += actualBytesAdded;
                total_bytes_added += actualBytesAdded;
                moreBytesRequested -= actualBytesAdded;
            }
            
            filebytes = (off_t)fp->ff_blocks * (off_t)vcb->blockSize;
            hfs_systemfile_unlock(hfsmp, lockflags);

            if (hfsmp->jnl) {
                (void) hfs_update(vp, 0);
                (void) hfs_volupdate(hfsmp, VOL_UPDATE, 0);
            }

            hfs_end_transaction(hfsmp);
        }

        /*
         * if we get an error and no changes were made then exit
         * otherwise we must do the hfs_update to reflect the changes
         */
        if (retval && (startingPEOF == filebytes))
            goto err_exit;
        
        /*
         * Adjust actualBytesAdded to be allocation block aligned, not
         * clump size aligned.
         * NOTE: So what we are reporting does not affect reality
         * until the file is closed, when we truncate the file to allocation
         * block size.
         */
        if (total_bytes_added != 0 && orig_request_size < total_bytes_added)
            psPreAllocRes->bytesallocated = roundup(orig_request_size, (off_t)vcb->blockSize);
    } else {
        //No need to touch anything else, just unlock and go out
        goto err_exit;
    }

exit:
    cp->c_flag |= C_MODIFIED;
    cp->c_touch_chgtime = TRUE;
    cp->c_touch_modtime = TRUE;
    retval2 = hfs_update(vp, 0);

    if (retval == 0)
        retval = retval2;
    
err_exit:
    hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
    hfs_unlock(cp);
    return (retval);
}
