/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_vnops.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 20/3/18.
 */

#include <sys/stat.h>
#include <sys/mount.h>

#include "lf_hfs_vnops.h"
#include "lf_hfs.h"
#include "lf_hfs_catalog.h"
#include "lf_hfs_dirops_handler.h"
#include "lf_hfs_fileops_handler.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_attrlist.h"
#include "lf_hfs_btree.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_readwrite_ops.h"
#include "lf_hfs_generic_buf.h"
#include "lf_hfs_endian.h"
#include <sys/stat.h>
#include <sys/mount.h>
#include "lf_hfs_link.h"
#include "lf_hfs_journal.h"
#include "lf_hfs_chash.h"

#define DOT_DIR_SIZE                (UVFS_DIRENTRY_RECLEN(1))
#define DOT_X2_DIR_SIZE             (UVFS_DIRENTRY_RECLEN(2))


/* Options for hfs_removedir and hfs_removefile */
#define HFSRM_SKIP_RESERVE  0x01
#define _PATH_RSRCFORKSPEC     "/..namedfork/rsrc"

void
replace_desc(struct cnode *cp, struct cat_desc *cdp)
{
    // fixes 4348457 and 4463138
    if (&cp->c_desc == cdp) {
        return;
    }

    /* First release allocated name buffer */
    if (cp->c_desc.cd_flags & CD_HASBUF && cp->c_desc.cd_nameptr != 0) {
        const u_int8_t *name = cp->c_desc.cd_nameptr;

        cp->c_desc.cd_nameptr = 0;
        cp->c_desc.cd_namelen = 0;
        cp->c_desc.cd_flags &= ~CD_HASBUF;
        hfs_free((void*)name);
    }
    bcopy(cdp, &cp->c_desc, sizeof(cp->c_desc));

    /* Cnode now owns the name buffer */
    cdp->cd_nameptr = NULL;
    cdp->cd_namelen = 0;
    cdp->cd_flags &= ~CD_HASBUF;
}

static void SynthesizeDotAndDotX2(u_int64_t uCnid, void* puBuff, bool bIsDot, bool bIsLastEntry)
{
    UVFSDirEntry* psDotEntry = (UVFSDirEntry*)puBuff;
    uint8_t uNameLen = bIsDot? 1: 2;
    memset( psDotEntry, 0,  UVFS_DIRENTRY_RECLEN(uNameLen));

    psDotEntry->de_fileid       = uCnid;
    psDotEntry->de_filetype     = UVFS_FA_TYPE_DIR;
    psDotEntry->de_reclen       = bIsLastEntry ? 0 : UVFS_DIRENTRY_RECLEN(uNameLen);
    psDotEntry->de_nextcookie   = uNameLen;
    psDotEntry->de_namelen      = uNameLen;
    uint8_t* puNameBuf          = (uint8_t*)psDotEntry->de_name;
    puNameBuf[0]                = '.';
    if ( bIsDot )
    {
        puNameBuf[1] = '\0';
    }
    else
    {
        puNameBuf[1] = '.';
        puNameBuf[2] = '\0';
    }
}

static int SyntisizeEntries(uint64_t* puOffset, ReadDirBuff_s* psReadDirBuffer, int iIsExtended, u_int64_t uCnid, u_int64_t uParentCnid, UVFSDirEntry** ppsDotDotEntry)
{
    int iError = 0;
    void* pvBuff = NULL;
    if (!iIsExtended)
    {
        //Curently not supporting nonextended ReadDir
        return ENOTSUP;
    }

    if (DOT_DIR_SIZE > psReadDirBuffer->uBufferResid)
    {
        goto exit;
    }

    pvBuff = hfs_malloc(DOT_DIR_SIZE);
    if (pvBuff == NULL)
    {
        LFHFS_LOG(LEVEL_ERROR, "SyntisizeEntries: Failed to allocate buffer for DOT entry\n");
        return ENOMEM;
    }

    if (*puOffset == 0)
    {
        bool bIsEnoughRoomForAll = (DOT_DIR_SIZE + DOT_X2_DIR_SIZE > psReadDirBuffer->uBufferResid);
        SynthesizeDotAndDotX2(uCnid, pvBuff, true, bIsEnoughRoomForAll);
        memcpy(psReadDirBuffer->pvBuffer + READDIR_BUF_OFFSET(psReadDirBuffer) , pvBuff, DOT_DIR_SIZE);
        (*puOffset)++;
        psReadDirBuffer->uBufferResid -= DOT_DIR_SIZE;
    }

    if (DOT_X2_DIR_SIZE > psReadDirBuffer->uBufferResid)
    {
        goto exit;
    }

    hfs_free(pvBuff);
    pvBuff = hfs_malloc(DOT_X2_DIR_SIZE);
    if (pvBuff == NULL)
    {
        LFHFS_LOG(LEVEL_ERROR, "SyntisizeEntries: Failed to allocate buffer for DOTx2 entry\n");
        return ENOMEM;
    }

    if (*puOffset == 1)
    {
        SynthesizeDotAndDotX2(uParentCnid, pvBuff, false, false);
        memcpy(psReadDirBuffer->pvBuffer + READDIR_BUF_OFFSET(psReadDirBuffer), pvBuff, DOT_X2_DIR_SIZE);
        *ppsDotDotEntry = (UVFSDirEntry*) (psReadDirBuffer->pvBuffer + READDIR_BUF_OFFSET(psReadDirBuffer));
        (*puOffset)++;
        psReadDirBuffer->uBufferResid -= DOT_X2_DIR_SIZE;
    }

exit:
    if (pvBuff)
        hfs_free(pvBuff);
    return iError;
}

/*
 *  hfs_vnop_readdir reads directory entries into the buffer pointed
 *  to by uio, in a filesystem independent format.  Up to uio_resid
 *  bytes of data can be transferred.  The data in the buffer is a
 *  series of packed dirent structures where each one contains the
 *  following entries:
 *
 *    u_int32_t   d_fileno;              // file number of entry
 *    u_int16_t   d_reclen;              // length of this record
 *    u_int8_t    d_type;                // file type
 *    u_int8_t    d_namlen;              // length of string in d_name
 *    char        d_name[MAXNAMELEN+1];  // null terminated file name
 *
 *  The current position (uio_offset) refers to the next block of
 *  entries.  The offset can only be set to a value previously
 *  returned by hfs_vnop_readdir or zero.  This offset does not have
 *  to match the number of bytes returned (in uio_resid).
 *
 *  In fact, the offset used by HFS is essentially an index (26 bits)
 *  with a tag (6 bits).  The tag is for associating the next request
 *  with the current request.  This enables us to have multiple threads
 *  reading the directory while the directory is also being modified.
 *
 *  Each tag/index pair is tied to a unique directory hint.  The hint
 *  contains information (filename) needed to build the catalog b-tree
 *  key for finding the next set of entries.
 *
 * If the directory is marked as deleted-but-in-use (cp->c_flag & C_DELETED),
 * do NOT synthesize entries for "." and "..".
 */
int
hfs_vnop_readdir(vnode_t vp, int *eofflag, int *numdirent, ReadDirBuff_s* psReadDirBuffer, uint64_t puCookie, int flags)
{
    struct cnode *cp = NULL;
    struct hfsmount *hfsmp = VTOHFS(vp);
    directoryhint_t *dirhint = NULL;
    directoryhint_t localhint;
    bool bLocalEOFflag = false;
    int error = 0;
    uint64_t offset;
    user_size_t user_original_resid = psReadDirBuffer->uBufferResid;
    int items = 0;
    cnid_t cnid_hint = 0;
    int bump_valence = 0;
    *numdirent = 0;
    uint64_t startoffset = offset = puCookie;
    bool extended = (flags & VNODE_READDIR_EXTENDED);
    bool nfs_cookies = extended && (flags & VNODE_READDIR_REQSEEKOFF);

    if (psReadDirBuffer->pvBuffer == NULL || psReadDirBuffer->uBufferResid < sizeof(UVFSDirEntry))
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_readdir: readDir input is not valid\n");
        return EINVAL;
    }

    /* Note that the dirhint calls require an exclusive lock. */
    if ((error = hfs_lock(VTOC(vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT)))
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_readdir: Failed to lock vnode\n");
        return error;
    }
    cp = VTOC(vp);

    /* Pick up cnid hint (if any). */
    if (nfs_cookies)
    {
        cnid_hint = (cnid_t)(offset >> 32);
        offset &= 0x00000000ffffffffLL;
        if (cnid_hint == INT_MAX)
        { /* searching pass the last item */
            bLocalEOFflag = true;
            goto out;
        }
    }

    /*
     * Synthesize entries for "." and "..", unless the directory has
     * been deleted, but not closed yet (lazy delete in progress).
     */
    UVFSDirEntry* psDotDotEntry = NULL;
    if (!(cp->c_flag & C_DELETED))
    {
        if ( (error = SyntisizeEntries(&offset, psReadDirBuffer, extended, cp->c_cnid, cp->c_parentcnid, &psDotDotEntry)) != 0 )
        {
            LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_readdir: Failed to syntisize dot/dotdot entries\n");
            goto out;
        }
    }

    /* Convert offset into a catalog directory index. */
    int index = (offset & HFS_INDEX_MASK) - 2;
    unsigned int tag = (unsigned int) (offset & ~HFS_INDEX_MASK);

    /* Lock catalog during cat_findname and cat_getdirentries. */
    int lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);

    /* When called from NFS, try and resolve a cnid hint. */
    if (nfs_cookies && cnid_hint != 0)
    {
        if (cat_findname(hfsmp, cnid_hint, &localhint.dh_desc) == 0)
        {
            if ( localhint.dh_desc.cd_parentcnid == cp->c_fileid)
            {
                localhint.dh_index = index - 1;
                localhint.dh_time = 0;
                bzero(&localhint.dh_link, sizeof(localhint.dh_link));
                dirhint = &localhint;  /* don't forget to release the descriptor */
            }
            else
            {
                cat_releasedesc(&localhint.dh_desc);
            }
        }
    }

    /* Get a directory hint (cnode must be locked exclusive) */
    if (dirhint == NULL)
    {
        dirhint = hfs_getdirhint(cp, ((index - 1) & HFS_INDEX_MASK) | tag, 0);

        /* Hide tag from catalog layer. */
        dirhint->dh_index &= HFS_INDEX_MASK;
        if (dirhint->dh_index == HFS_INDEX_MASK)
        {
            dirhint->dh_index = -1;
        }
    }

    if (index == 0)
    {
        dirhint->dh_threadhint = cp->c_dirthreadhint;
    }
    else
    {
        /*
         * If we have a non-zero index, there is a possibility that during the last
         * call to hfs_vnop_readdir we hit EOF for this directory.  If that is the case
         * then we don't want to return any new entries for the caller.  Just return 0
         * items, mark the eofflag, and bail out.  Because we won't have done any work, the
         * code at the end of the function will release the dirhint for us.
         *
         * Don't forget to unlock the catalog lock on the way out, too.
         */
        if (dirhint->dh_desc.cd_flags & CD_EOF)
        {
            error = 0;
            bLocalEOFflag = true;
            offset = startoffset;
            if (user_original_resid > 0) {
                psReadDirBuffer->uBufferResid = user_original_resid;
            }
            hfs_systemfile_unlock (hfsmp, lockflags);

            goto seekoffcalc;
        }
    }

    /* Pack the buffer with dirent entries. */
    error = cat_getdirentries(hfsmp, cp->c_entries, dirhint, psReadDirBuffer, flags, &items, &bLocalEOFflag, psDotDotEntry);

    if (index == 0 && error == 0)
    {
        cp->c_dirthreadhint = dirhint->dh_threadhint;
    }

    hfs_systemfile_unlock(hfsmp, lockflags);

    if (error != 0)
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_readdir: Failed to get dir entries\n");
        goto out;
    }

    /* Get index to the next item */
    index += items;

    if (items >= (int)cp->c_entries)
    {
        bLocalEOFflag = true;
    }

    /*
     * Detect valence FS corruption.
     *
     * We are holding the cnode lock exclusive, so there should not be
     * anybody modifying the valence field of this cnode.  If we enter
     * this block, that means we observed filesystem corruption, because
     * this directory reported a valence of 0, yet we found at least one
     * item.  In this case, we need to minimally self-heal this
     * directory to prevent userland from tripping over a directory
     * that appears empty (getattr of valence reports 0), but actually
     * has contents.
     *
     * We'll force the cnode update at the end of the function after
     * completing all of the normal getdirentries steps.
     */
    if ((cp->c_entries == 0) && (items > 0))
    {
        /* disk corruption */
        cp->c_entries++;
        /* Mark the cnode as dirty. */
        cp->c_flag |= C_MODIFIED;
        LFHFS_LOG(LEVEL_DEBUG, "hfs_vnop_readdir: repairing valence to non-zero! \n");
        bump_valence++;
    }


    /* Convert catalog directory index back into an offset. */
    while (tag == 0)
        tag = (++cp->c_dirhinttag) << HFS_INDEX_BITS;
    offset =  ((index + 2) | tag);
    dirhint->dh_index |= tag;

seekoffcalc:
    cp->c_touch_acctime = TRUE;

    if (numdirent)
    {
        if (startoffset == 0)
            items += 2;
        else if (startoffset == 1)
            items += 1;

        *numdirent = items;
    }

out:
    /* If we didn't do anything then go ahead and dump the hint. */
    if ((dirhint != NULL) && (dirhint != &localhint) && (offset == startoffset))
    {
        hfs_reldirhint(cp, dirhint);
        bLocalEOFflag = true;
    }

    if (eofflag)
    {
        *eofflag = bLocalEOFflag;
    }

    if (dirhint == &localhint)
    {
        cat_releasedesc(&localhint.dh_desc);
    }

    if (bump_valence)
    {
        /* force the update before dropping the cnode lock*/
        hfs_update(vp, 0);
    }

    hfs_unlock(cp);

    return (error);
}

/*
 * readdirattr operation will return attributes for the items in the
 * directory specified.
 *
 * It does not do . and .. entries. The problem is if you are at the root of the
 * hfs directory and go to .. you could be crossing a mountpoint into a
 * different (ufs) file system. The attributes that apply for it may not
 * apply for the file system you are doing the readdirattr on. To make life
 * simpler, this call will only return entries in its directory, hfs like.
 */
int
hfs_vnop_readdirattr(vnode_t vp, int *eofflag, int *numdirent, ReadDirBuff_s* psReadDirBuffer, uint64_t puCookie)
{
    int error;
    uint32_t newstate;
    uint32_t uMaxCount = (uint32_t) psReadDirBuffer->uBufferResid / _UVFS_DIRENTRYATTR_RECLEN(UVFS_DIRENTRYATTR_NAMEOFF,0);

    if (psReadDirBuffer->pvBuffer == NULL || psReadDirBuffer->uBufferResid < sizeof(UVFSDirEntry))
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_readdirattr: buffer input is invalid\n");
        return EINVAL;
    }

    error = hfs_readdirattr_internal(vp, psReadDirBuffer, uMaxCount, &newstate, eofflag, numdirent, puCookie);

    return (error);
}

/*
 * Sync all hfs B-trees.  Use this instead of journal_flush for a volume
 * without a journal.  Note that the volume bitmap does not get written;
 * we rely on fsck_hfs to fix that up (which it can do without any loss
 * of data).
 */
static int
hfs_metasync_all(struct hfsmount *hfsmp)
{
    int lockflags;
    
    /* Lock all of the B-trees so we get a mutually consistent state */
    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG|SFL_EXTENTS|SFL_ATTRIBUTE, HFS_EXCLUSIVE_LOCK);
    
#if LF_HFS_FULL_VNODE_SUPPORT
    //Curently we don't keep any cache for btree buffers.
    //When we will have a cache we will have to flush it out here.
    /* Sync each of the B-trees */
    if (hfsmp->hfs_catalog_vp)
        hfs_btsync(hfsmp->hfs_catalog_vp, 0);
    if (hfsmp->hfs_extents_vp)
        hfs_btsync(hfsmp->hfs_extents_vp, 0);
    if (hfsmp->hfs_attribute_vp)
        hfs_btsync(hfsmp->hfs_attribute_vp, 0);
#endif
    hfs_systemfile_unlock(hfsmp, lockflags);
    
    return 0;
}
/*
 *  cnode must be locked
 */
int
hfs_fsync(struct vnode *vp, int waitfor, hfs_fsync_mode_t fsyncmode)
{
    struct cnode *cp = VTOC(vp);
    struct filefork *fp = NULL;
    int retval = 0;
    struct timeval tv;
    int took_trunc_lock = 0;
    int fsync_default = 1;

    /*
     * Applications which only care about data integrity rather than full
     * file integrity may opt out of (delay) expensive metadata update
     * operations as a performance optimization.
     */
    int wait = (waitfor == MNT_WAIT); /* attributes necessary for data retrieval */
    if (fsyncmode != HFS_FSYNC)
        fsync_default = 0;

    /* HFS directories don't have any data blocks. */
    if (vnode_isdir(vp))
        goto metasync;
    fp = VTOF(vp);

    /*
     * For system files flush the B-tree header and
     * for regular files write out any clusters
     */
    if (vnode_issystem(vp))
    {
        if (VTOF(vp)->fcbBTCBPtr != NULL)
        {
            // XXXdbg
            if (VTOHFS(vp)->jnl == NULL)
            {
                BTFlushPath(VTOF(vp));
            }
        }
    }
    else
    {
//TBD- Since we always flush the data for every file when it is being updated
//     we don't need to do that here.
//        hfs_unlock(cp);
//        hfs_lock_truncate(cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT);
//        took_trunc_lock = 1;
//
//        if (fp->ff_unallocblocks != 0)
//        {
//            hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
//
//            hfs_lock_truncate(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
//        }
//
//#if LF_HFS_FULL_VNODE_SUPPORT
//        /* Don't hold cnode lock when calling into cluster layer. */
//        (void) cluster_push(vp, waitdata ? IO_SYNC : 0);
//#endif
//
//        hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS);
    }
    
    /*
     * When MNT_WAIT is requested and the zero fill timeout
     * has expired then we must explicitly zero out any areas
     * that are currently marked invalid (holes).
     *
     * Files with NODUMP can bypass zero filling here.
     */
    if (fp && (((cp->c_flag & C_ALWAYS_ZEROFILL) && !TAILQ_EMPTY(&fp->ff_invalidranges)) ||
               ((wait || (cp->c_flag & C_ZFWANTSYNC)) &&
                ((cp->c_bsdflags & UF_NODUMP) == 0) &&
                (vnode_issystem(vp) ==0) &&
                cp->c_zftimeout != 0)))
    {
       microtime(&tv);
       if ((cp->c_flag & C_ALWAYS_ZEROFILL) == 0 && fsync_default && tv.tv_sec < (long)cp->c_zftimeout)
       {
           /* Remember that a force sync was requested. */
           cp->c_flag |= C_ZFWANTSYNC;
           goto datasync;
       }
       if (!TAILQ_EMPTY(&fp->ff_invalidranges))
       {
           if (!took_trunc_lock || (cp->c_truncatelockowner == HFS_SHARED_OWNER))
           {
               hfs_unlock(cp);
               if (took_trunc_lock) {
                   hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
               }
               hfs_lock_truncate(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
               hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS);
               took_trunc_lock = 1;
           }

#if LF_HFS_FULL_VNODE_SUPPORT
           hfs_flush_invalid_ranges(vp);
           hfs_unlock(cp);
           (void) cluster_push(vp, waitdata ? IO_SYNC : 0);
           hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS);
#endif
       }
    }

datasync:
    if (took_trunc_lock)
        hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);

//    TBD - symlink can't be dirsty since we always write the data fully to the device
//    else if (fsync_default && vnode_islnk(vp) && vnode_hasdirtyblks(vp) && vnode_isrecycled(vp))
//    {
//        /*
//         * If it's a symlink that's dirty and is about to be recycled,
//         * we need to flush the journal.
//         */
//        fsync_default = 0;
//    }

metasync:
    if (vnode_isreg(vp) && vnode_issystem(vp))
    {
        if (VTOF(vp)->fcbBTCBPtr != NULL)
        {
            microtime(&tv);
            BTSetLastSync(VTOF(vp), (u_int32_t) tv.tv_sec);
        }
        cp->c_touch_acctime = FALSE;
        cp->c_touch_chgtime = FALSE;
        cp->c_touch_modtime = FALSE;
    }
    else
    {
        retval = hfs_update(vp, HFS_UPDATE_FORCE);
        /*
         * When MNT_WAIT is requested push out the catalog record for
         * this file.  If they asked for a full fsync, we can skip this
         * because the journal_flush or hfs_metasync_all will push out
         * all of the metadata changes.
         */
#if 0
        /*
         * As we are not supporting any write buf caches / delay writes,
         * this is not needed.
         */
        if ((retval == 0) && wait && fsync_default && cp->c_hint &&
            !ISSET(cp->c_flag, C_DELETED | C_NOEXISTS)) {
            hfs_metasync(VTOHFS(vp), (daddr64_t)cp->c_hint);
        }
#endif
        /*
         * If this was a full fsync, make sure all metadata
         * changes get to stable storage.
         */
        if (!fsync_default)
        {
            if (VTOHFS(vp)->jnl) {
                if (fsyncmode == HFS_FSYNC_FULL)
                    hfs_flush(VTOHFS(vp), HFS_FLUSH_FULL);
                else
                    hfs_flush(VTOHFS(vp), HFS_FLUSH_JOURNAL_BARRIER);
            }
            else
            {
                retval = hfs_metasync_all(VTOHFS(vp));
                hfs_flush(VTOHFS(vp), HFS_FLUSH_CACHE);
            }
        }
    }

#if LF_HFS_FULL_VNODE_SUPPORT
    if (!hfs_is_dirty(cp) && !ISSET(cp->c_flag, C_DELETED))
        vnode_cleardirty(vp);
#endif

    return (retval);
}

/*
 * hfs_removefile
 *
 * Similar to hfs_vnop_remove except there are additional options.
 * This function may be used to remove directories if they have
 * lots of EA's -- note the 'allow_dirs' argument.
 *
 * This function is able to delete blocks & fork data for the resource
 * fork even if it does not exist in core (and have a backing vnode).
 * It should infer the correct behavior based on the number of blocks
 * in the cnode and whether or not the resource fork pointer exists or
 * not.  As a result, one only need pass in the 'vp' corresponding to the
 * data fork of this file (or main vnode in the case of a directory).
 * Passing in a resource fork will result in an error.
 *
 * Because we do not create any vnodes in this function, we are not at
 * risk of deadlocking against ourselves by double-locking.
 *
 * Requires cnode and truncate locks to be held.
 */
int
hfs_removefile(struct vnode *dvp, struct vnode *vp, struct componentname *cnp,
               int flags, int skip_reserve, int allow_dirs, int only_unlink)
{
    struct cnode *cp;
    struct cnode *dcp;
    struct vnode *rsrc_vp = NULL;
    struct hfsmount *hfsmp;
    struct cat_desc desc;
    int dataforkbusy = 0;
    int rsrcforkbusy = 0;
    int lockflags;
    int error = 0;
    int started_tr = 0;
    int isbigfile = 0, defer_remove=0;
    bool isdir= false;
    int update_vh = 0;

    cp = VTOC(vp);
    dcp = VTOC(dvp);
    hfsmp = VTOHFS(vp);

    /* Check if we lost a race post lookup. */
    if (cp->c_flag & (C_NOEXISTS | C_DELETED)) {
        return (EINVAL);
    }

    if (!hfs_valid_cnode(hfsmp, dvp, cnp, cp->c_fileid, NULL, &error))
    {
        return error;
    }

    /* Make sure a remove is permitted */
    /* Don't allow deleting the journal or journal_info_block. */
    if (VNODE_IS_RSRC(vp) || vnode_issystem(vp) || IsEntryAJnlFile(hfsmp, cp->c_fileid))
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_removefile: Removing %s file is not premited\n", VNODE_IS_RSRC(vp) ? "Resource" : (vnode_issystem(vp)? "System" : "Journal"));
        return (EPERM);
    }
    else
    {
        /*
         * We know it's a data fork.
         * Probe the cnode to see if we have a valid resource fork
         * in hand or not.
         */
        rsrc_vp = cp->c_rsrc_vp;
    }

    /*
     * Hard links require special handling.
     */
    if (cp->c_flag & C_HARDLINK)
    {
        /* A directory hard link with a link count of one is
         * treated as a regular directory.  Therefore it should
         * only be removed using rmdir().
         */
        if (IS_DIR(vp) && (cp->c_linkcount == 1) && (allow_dirs == 0))
        {
            LFHFS_LOG(LEVEL_ERROR, "hfs_removefile: Trying to remove an hardlink directory\n");
            return (EPERM);
        }
        
        return hfs_unlink(hfsmp, dvp, vp, cnp, skip_reserve);
    }

    /* Directories should call hfs_rmdir! (unless they have a lot of attributes) */
    if (IS_DIR(vp))
    {
        if (!allow_dirs)
        {
            return (EPERM);  /* POSIX */
        }
        isdir = true;
    }
    
    /* Sanity check the parent ids. */
    if ((cp->c_parentcnid != hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid) &&
        (cp->c_parentcnid != dcp->c_fileid))
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_removefile: Parent ID's are wrong\n");
        return (EINVAL);
    }
    
    dcp->c_flag |= C_DIR_MODIFICATION;

    // this guy is going away so mark him as such
    cp->c_flag |= C_DELETED;
   
    /*
     * If the caller was operating on a file (as opposed to a
     * directory with EAs), then we need to figure out
     * whether or not it has a valid resource fork vnode.
     *
     * If there was a valid resource fork vnode, then we need
     * to use hfs_truncate to eliminate its data.  If there is
     * no vnode, then we hold the cnode lock which would
     * prevent it from being created.  As a result,
     * we can use the data deletion functions which do not
     * require that a cnode/vnode pair exist.
     */

    /* Check if this file is being used. */
    if ( !isdir )
    {
        dataforkbusy = 0; /*vnode_isinuse(vp, 0);*/
        /*
         * At this point, we know that 'vp' points to the
         * a data fork because we checked it up front. And if
         * there is no rsrc fork, rsrc_vp will be NULL.
         */
        if (rsrc_vp && (cp->c_blocks - VTOF(vp)->ff_blocks))
        {
            rsrcforkbusy = 0; /*vnode_isinuse(rsrc_vp, 0);*/
        }

        /* Check if we have to break the deletion into multiple pieces. */
        isbigfile = cp->c_datafork->ff_size >= HFS_BIGFILE_SIZE;
    }

    /* Check if the file has xattrs.  If it does we'll have to delete them in
     individual transactions in case there are too many */
    if ((hfsmp->hfs_attribute_vp != NULL) &&  (cp->c_attr.ca_recflags & kHFSHasAttributesMask) != 0)
    {
        defer_remove = 1;
    }

    /* If we are explicitly told to only unlink item and move to hidden dir, then do it */
    if (only_unlink)
    {
        defer_remove = 1;
    }

    /*
     * Carbon semantics prohibit deleting busy files.
     * (enforced when VNODE_REMOVE_NODELETEBUSY is requested)
     */
    if (dataforkbusy || rsrcforkbusy)
    {
        if ((flags & VNODE_REMOVE_NODELETEBUSY) || (hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid == 0))
        {
            error = EBUSY;
            goto out;
        }
    }

    /*
     * Do a ubc_setsize to indicate we need to wipe contents if:
     *  1) item is a regular file.
     *  2) Neither fork is busy AND we are not told to unlink this.
     *
     * We need to check for the defer_remove since it can be set without
     * having a busy data or rsrc fork
     */
    if (isdir == 0 && (!dataforkbusy || !rsrcforkbusy) && (defer_remove == 0))
    {
        /*
         * A ubc_setsize can cause a pagein so defer it
         * until after the cnode lock is dropped.  The
         * cnode lock cannot be dropped/reacquired here
         * since we might already hold the journal lock.
         */
        if (!dataforkbusy && cp->c_datafork->ff_blocks && !isbigfile)
        {
            cp->c_flag |= C_NEED_DATA_SETSIZE;
        }
        if (!rsrcforkbusy && rsrc_vp)
        {
            cp->c_flag |= C_NEED_RSRC_SETSIZE;
        }
    }

    if ((error = hfs_start_transaction(hfsmp)) != 0)
    {
        goto out;
    }
    started_tr = 1;

    /*
     * Prepare to truncate any non-busy forks.  Busy forks will
     * get truncated when their vnode goes inactive.
     * Note that we will only enter this region if we
     * can avoid creating an open-unlinked file.  If
     * either region is busy, we will have to create an open
     * unlinked file.
     *
     * Since we are deleting the file, we need to stagger the runtime
     * modifications to do things in such a way that a crash won't
     * result in us getting overlapped extents or any other
     * bad inconsistencies.  As such, we call prepare_release_storage
     * which updates the UBC, updates quota information, and releases
     * any loaned blocks that belong to this file.  No actual
     * truncation or bitmap manipulation is done until *AFTER*
     * the catalog record is removed.
     */
    if (isdir == 0 && (!dataforkbusy && !rsrcforkbusy) && (only_unlink == 0))
    {
        if (!dataforkbusy && !isbigfile && cp->c_datafork->ff_blocks != 0)
        {
            error = hfs_prepare_release_storage (hfsmp, vp);
            if (error)
            {
                goto out;
            }
            update_vh = 1;
        }

        /*
         * If the resource fork vnode does not exist, we can skip this step.
         */
        if (!rsrcforkbusy && rsrc_vp)
        {
            error = hfs_prepare_release_storage (hfsmp, rsrc_vp);
            if (error)
            {
                goto out;
            }
            update_vh = 1;
        }
    }

    /*
     * Protect against a race with rename by using the component
     * name passed in and parent id from dvp (instead of using
     * the cp->c_desc which may have changed).   Also, be aware that
     * because we allow directories to be passed in, we need to special case
     * this temporary descriptor in case we were handed a directory.
     */
    if (isdir)
    {
        desc.cd_flags = CD_ISDIR;
    }
    else
    {
        desc.cd_flags = 0;
    }
    desc.cd_encoding = cp->c_desc.cd_encoding;
    desc.cd_nameptr = (const u_int8_t *)cnp->cn_nameptr; 
    desc.cd_namelen = cnp->cn_namelen;
    desc.cd_parentcnid = dcp->c_fileid;
    desc.cd_hint = cp->c_desc.cd_hint;
    desc.cd_cnid = cp->c_cnid;
    struct timeval tv;
    microtime(&tv);

    /*
     * There are two cases to consider:
     *  1. File/Dir is busy/big/defer_remove ==> move/rename the file/dir
     *  2. File is not in use ==> remove the file
     *
     * We can get a directory in case 1 because it may have had lots of attributes,
     * which need to get removed here.
     */
    if (dataforkbusy || rsrcforkbusy || isbigfile || defer_remove)
    {
        char delname[32];
        struct cat_desc to_desc;
        struct cat_desc todir_desc;

        /*
         * Orphan this file or directory (move to hidden directory).
         * Again, we need to take care that we treat directories as directories,
         * and files as files.  Because directories with attributes can be passed in
         * check to make sure that we have a directory or a file before filling in the
         * temporary descriptor's flags.  We keep orphaned directories AND files in
         * the FILE_HARDLINKS private directory since we're generalizing over all
         * orphaned filesystem objects.
         */
        bzero(&todir_desc, sizeof(todir_desc));
        todir_desc.cd_parentcnid = 2;

        MAKE_DELETED_NAME(delname, sizeof(delname), cp->c_fileid);
        bzero(&to_desc, sizeof(to_desc));
        to_desc.cd_nameptr = (const u_int8_t *)delname;
        to_desc.cd_namelen = strlen(delname);
        to_desc.cd_parentcnid = hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid;
        if (isdir)
        {
            to_desc.cd_flags = CD_ISDIR;
        }
        else
        {
            to_desc.cd_flags = 0;
        }
        to_desc.cd_cnid = cp->c_cnid;

        lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_EXCLUSIVE_LOCK);
        if (!skip_reserve)
        {
            if ((error = cat_preflight(hfsmp, CAT_RENAME, NULL)))
            {
                hfs_systemfile_unlock(hfsmp, lockflags);
                goto out;
            }
        }

        error = cat_rename(hfsmp, &desc, &todir_desc, &to_desc, (struct cat_desc *)NULL);

        if (error == 0)
        {
            hfsmp->hfs_private_attr[FILE_HARDLINKS].ca_entries++;
            if (isdir == 1)
            {
                INC_FOLDERCOUNT(hfsmp, hfsmp->hfs_private_attr[FILE_HARDLINKS]);
            }
            (void) cat_update(hfsmp, &hfsmp->hfs_private_desc[FILE_HARDLINKS], &hfsmp->hfs_private_attr[FILE_HARDLINKS], NULL, NULL);

            /* Update the parent directory */
            if (dcp->c_entries > 0)
                dcp->c_entries--;
            if (isdir == 1)
            {
                DEC_FOLDERCOUNT(hfsmp, dcp->c_attr);
            }
            dcp->c_dirchangecnt++;
            hfs_incr_gencount(dcp);

            dcp->c_ctime = tv.tv_sec;
            dcp->c_mtime = tv.tv_sec;
            (void) cat_update(hfsmp, &dcp->c_desc, &dcp->c_attr, NULL, NULL);

            /* Update the file or directory's state */
            cp->c_flag |= C_DELETED;
            cp->c_ctime = tv.tv_sec;
            --cp->c_linkcount;
            (void) cat_update(hfsmp, &to_desc, &cp->c_attr, NULL, NULL);
        }

        hfs_systemfile_unlock(hfsmp, lockflags);
        if (error)
            goto out;
    }
    else
    {
        /*
         * Nobody is using this item; we can safely remove everything.
         */
        
        struct filefork *temp_rsrc_fork = NULL;
        u_int32_t fileid = cp->c_fileid;

        /*
         * Figure out if we need to read the resource fork data into
         * core before wiping out the catalog record.
         *
         * 1) Must not be a directory
         * 2) cnode's c_rsrcfork ptr must be NULL.
         * 3) rsrc fork must have actual blocks
         */
        if ((isdir == 0) && (cp->c_rsrcfork == NULL) && (cp->c_blocks - VTOF(vp)->ff_blocks))
        {
            /*
             * The resource fork vnode & filefork did not exist.
             * Create a temporary one for use in this function only.
             */
            temp_rsrc_fork = hfs_mallocz(sizeof(struct filefork));
            temp_rsrc_fork->ff_cp = cp;
            rl_init(&temp_rsrc_fork->ff_invalidranges);
        }

        lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG | SFL_ATTRIBUTE | SFL_BITMAP, HFS_EXCLUSIVE_LOCK);

        /* Look up the resource fork first, if necessary */
        if (temp_rsrc_fork)
        {
            error = cat_lookup (hfsmp, &desc, 1, (struct cat_desc*) NULL, (struct cat_attr*) NULL, &temp_rsrc_fork->ff_data, NULL);
            if (error)
            {
                hfs_free(temp_rsrc_fork);
                hfs_systemfile_unlock (hfsmp, lockflags);
                goto out;
            }
        }

        if (!skip_reserve)
        {
            if ((error = cat_preflight(hfsmp, CAT_DELETE, NULL)))
            {
                if (temp_rsrc_fork)
                {
                    hfs_free(temp_rsrc_fork);
                }
                hfs_systemfile_unlock(hfsmp, lockflags);
                goto out;
            }
        }

        error = cat_delete(hfsmp, &desc, &cp->c_attr);

        if (error && error != ENXIO && error != ENOENT)
        {
            LFHFS_LOG(LEVEL_ERROR, "hfs_removefile: deleting file %s (id=%d) vol=%s err=%d\n",
                      cp->c_desc.cd_nameptr, cp->c_attr.ca_fileid, hfsmp->vcbVN, error);
        }

        if (error == 0)
        {
            /* Update the parent directory */
            if (dcp->c_entries > 0)
            {
                dcp->c_entries--;
            }
            dcp->c_dirchangecnt++;
            hfs_incr_gencount(dcp);

            dcp->c_ctime = tv.tv_sec;
            dcp->c_mtime = tv.tv_sec;
            (void) cat_update(hfsmp, &dcp->c_desc, &dcp->c_attr, NULL, NULL);
        }

        hfs_systemfile_unlock(hfsmp, lockflags);

        if (error)
        {
            if (temp_rsrc_fork)
            {
                hfs_free(temp_rsrc_fork);
            }
            goto out;
        }

        /*
         * Now that we've wiped out the catalog record, the file effectively doesn't
         * exist anymore. So update the quota records to reflect the loss of the
         * data fork and the resource fork.
         */

        if (IS_LNK(vp) && cp->c_datafork->ff_symlinkptr)
        {
            hfs_free(cp->c_datafork->ff_symlinkptr);
            cp->c_datafork->ff_symlinkptr = NULL;
        }

        /*
         * If we didn't get any errors deleting the catalog entry, then go ahead
         * and release the backing store now.  The filefork pointers are still valid.
         */
        if (temp_rsrc_fork)
        {
            error = hfs_release_storage (hfsmp, cp->c_datafork, temp_rsrc_fork, fileid);
        }
        else
        {
            /* if cp->c_rsrcfork == NULL, hfs_release_storage will skip over it. */
            error = hfs_release_storage (hfsmp, cp->c_datafork, cp->c_rsrcfork, fileid);
        }
        if (error)
        {
            /*
             * If we encountered an error updating the extents and bitmap,
             * mark the volume inconsistent.  At this point, the catalog record has
             * already been deleted, so we can't recover it at this point. We need
             * to proceed and update the volume header and mark the cnode C_NOEXISTS.
             * The subsequent fsck should be able to recover the free space for us.
             */
            hfs_mark_inconsistent(hfsmp, HFS_OP_INCOMPLETE);
        }
        else
        {
            /* reset update_vh to 0, since hfs_release_storage should have done it for us */
            update_vh = 0;
        }

        /* Get rid of the temporary rsrc fork */
        if (temp_rsrc_fork)
        {
            hfs_free(temp_rsrc_fork);
        }

        cp->c_flag |= C_NOEXISTS;
        cp->c_flag &= ~C_DELETED;

        cp->c_touch_chgtime = TRUE;
        --cp->c_linkcount;

        /*
         * We must never get a directory if we're in this else block.  We could
         * accidentally drop the number of files in the volume header if we did.
         */
        hfs_volupdate(hfsmp, VOL_RMFILE, (dcp->c_cnid == kHFSRootFolderID));
    }

    /*
     * All done with this cnode's descriptor...
     *
     * Note: all future catalog calls for this cnode must be by
     * fileid only.  This is OK for HFS (which doesn't have file
     * thread records) since HFS doesn't support the removal of
     * busy files.
     */
    cat_releasedesc(&cp->c_desc);

out:
    if (error)
    {
        cp->c_flag &= ~C_DELETED;
    }

    if (update_vh)
    {
        /*
         * If we bailed out earlier, we may need to update the volume header
         * to deal with the borrowed blocks accounting.
         */
        hfs_volupdate (hfsmp, VOL_UPDATE, 0);
    }

    if (started_tr)
    {
        hfs_end_transaction(hfsmp);
    }
    
    dcp->c_flag &= ~C_DIR_MODIFICATION;
    //TBD - We have wakeup here but can't see anyone who's msleeping on c_flag...
    //wakeup((caddr_t)&dcp->c_flag);

    return (error);
}

/*
 * Remove a file or link.
 */
int
hfs_vnop_remove(struct vnode* psParentDir,struct vnode *psFileToRemove, struct componentname* psCN, int iFlags)
{
    struct cnode *dcp = VTOC(psParentDir);
    struct cnode *cp = VTOC(psFileToRemove);
    struct vnode *rvp = NULL;
    int error = 0;

    if (psParentDir == psFileToRemove)
    {
        return (EINVAL);
    }

relock:

    hfs_lock_truncate(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);

    if ((error = hfs_lockpair(dcp, cp, HFS_EXCLUSIVE_LOCK)))
    {
        hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
        if (rvp)
        {
            hfs_chash_lower_OpenLookupCounter(cp);
            rvp = NULL;
        }
        return (error);
    }

    /*
     * Lazily respond to determining if there is a valid resource fork
     * vnode attached to 'cp' if it is a regular file or symlink.
     * If the vnode does not exist, then we may proceed without having to
     * create it.
     *
     * If, however, it does exist, then we need to acquire an iocount on the
     * vnode after acquiring its vid.  This ensures that if we have to do I/O
     * against it, it can't get recycled from underneath us in the middle
     * of this call.
     *
     * Note: this function may be invoked for directory hardlinks, so just skip these
     * steps if 'vp' is a directory.
     */
    enum vtype vtype = psFileToRemove->sFSParams.vnfs_vtype;
    if ((vtype == VLNK) || (vtype == VREG))
    {
        if ((cp->c_rsrc_vp) && (rvp == NULL))
        {
            /* We need to acquire the rsrc vnode */
            rvp = cp->c_rsrc_vp;
            hfs_chash_raise_OpenLookupCounter(cp);
            /* Unlock everything to acquire iocount on the rsrc vnode */
            hfs_unlock_truncate (cp, HFS_LOCK_DEFAULT);
            hfs_unlockpair (dcp, cp);

            goto relock;
        }
    }

    /*
     * Check to see if we raced rmdir for the parent directory
     * hfs_removefile already checks for a race on vp/cp
     */
    if (dcp->c_flag & (C_DELETED | C_NOEXISTS))
    {
        error = ENOENT;
        goto rm_done;
    }

    error = hfs_removefile(psParentDir, psFileToRemove, psCN, iFlags, 0, 0, 0);
    
    /*
     * Drop the truncate lock before unlocking the cnode
     * (which can potentially perform a vnode_put and
     * recycle the vnode which in turn might require the
     * truncate lock)
     */
rm_done:
    //Update Directory version
    psParentDir->sExtraData.sDirData.uDirVersion++;

    hfs_unlockpair(dcp, cp);
    hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);

    if (rvp)
    {
        hfs_chash_lower_OpenLookupCounter(cp);
        rvp = NULL;
    }
    return (error);
}

/*
 * Remove a directory.
 */
int
hfs_vnop_rmdir(struct vnode *dvp, struct vnode *vp, struct componentname* psCN)
{
    int error = 0;
    struct cnode *dcp = VTOC(dvp);
    struct cnode *cp = VTOC(vp);

    if (!S_ISDIR(cp->c_mode))
    {
        return (ENOTDIR);
    }
    if (dvp == vp)
    {
        return (EINVAL);
    }

    if ((error = hfs_lockpair(dcp, cp, HFS_EXCLUSIVE_LOCK)))
    {
        return (error);
    }

    /* Check for a race with rmdir on the parent directory */
    if (dcp->c_flag & (C_DELETED | C_NOEXISTS))
    {
        hfs_unlockpair (dcp, cp);
        return ENOENT;
    }

    error = hfs_removedir(dvp, vp, psCN, 0, 0);

    hfs_unlockpair(dcp, cp);

    return (error);
}

/*
 * Remove a directory
 *
 * Both dvp and vp cnodes are locked
 */
int
hfs_removedir(struct vnode *dvp, struct vnode *vp, struct componentname *cnp, int skip_reserve, int only_unlink)
{
    struct cnode *cp;
    struct cnode *dcp;
    struct hfsmount * hfsmp;
    struct cat_desc desc;
    int lockflags;
    int error = 0, started_tr = 0;

    cp = VTOC(vp);
    dcp = VTOC(dvp);
    hfsmp = VTOHFS(vp);
    
    if (cp->c_flag & (C_NOEXISTS | C_DELETED)){
        return (EINVAL);
    }
    
    if (cp->c_entries != 0){
        return (ENOTEMPTY);
    }

    /* Deal with directory hardlinks */
    if (cp->c_flag & C_HARDLINK)
    {
        /*
         * Note that if we have a directory which was a hardlink at any point,
         * its actual directory data is stored in the directory inode in the hidden
         * directory rather than the leaf element(s) present in the namespace.
         *
         * If there are still other hardlinks to this directory,
         * then we'll just eliminate this particular link and the vnode will still exist.
         * If this is the last link to an empty directory, then we'll open-unlink the
         * directory and it will be only tagged with C_DELETED (as opposed to C_NOEXISTS).
         *
         * We could also return EBUSY here.
         */

        return hfs_unlink(hfsmp, dvp, vp, cnp, skip_reserve);
    }

    /*
     * In a few cases, we may want to allow the directory to persist in an
     * open-unlinked state.  If the directory is being open-unlinked (still has usecount
     * references), or if it has EAs, or if it was being deleted as part of a rename,
     * then we go ahead and move it to the hidden directory.
     *
     * If the directory is being open-unlinked, then we want to keep the catalog entry
     * alive so that future EA calls and fchmod/fstat etc. do not cause issues later.
     *
     * If the directory had EAs, then we want to use the open-unlink trick so that the
     * EA removal is not done in one giant transaction.  Otherwise, it could cause a panic
     * due to overflowing the journal.
     *
     * Finally, if it was deleted as part of a rename, we move it to the hidden directory
     * in order to maintain rename atomicity.
     *
     * Note that the allow_dirs argument to hfs_removefile specifies that it is
     * supposed to handle directories for this case.
     */

    if (((hfsmp->hfs_attribute_vp != NULL) && ((cp->c_attr.ca_recflags & kHFSHasAttributesMask) != 0)) || (only_unlink != 0))
    {

        int ret = hfs_removefile(dvp, vp, cnp, 0, 0, 1, only_unlink);
//      Will be released in the layer above where it was created
//      vnode_recycle(vp);
        return ret;
    }

    dcp->c_flag |= C_DIR_MODIFICATION;

    if ((error = hfs_start_transaction(hfsmp)) != 0)
    {
        goto out;
    }
    started_tr = 1;

    /*
     * Verify the directory is empty (and valid).
     * (Rmdir ".." won't be valid since
     *  ".." will contain a reference to
     *  the current directory and thus be
     *  non-empty.)
     */
    if ((dcp->c_bsdflags & (UF_APPEND | SF_APPEND)) || (cp->c_bsdflags & ((UF_IMMUTABLE | SF_IMMUTABLE | UF_APPEND | SF_APPEND))))
    {
        error = EPERM;
        goto out;
    }
    
    /*
     * Protect against a race with rename by using the component
     * name passed in and parent id from dvp (instead of using
     * the cp->c_desc which may have changed).
     */
    desc.cd_nameptr = (const u_int8_t *)cnp->cn_nameptr;
    desc.cd_namelen = cnp->cn_namelen;
    desc.cd_parentcnid = dcp->c_fileid;
    desc.cd_cnid = cp->c_cnid;
    desc.cd_flags = CD_ISDIR;
    desc.cd_encoding = cp->c_encoding;
    desc.cd_hint = 0;

    if (!hfs_valid_cnode(hfsmp, dvp, cnp, cp->c_fileid, NULL, &error))
    {
        error = 0;
        goto out;
    }

    /* Remove entry from catalog */
    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG | SFL_ATTRIBUTE | SFL_BITMAP, HFS_EXCLUSIVE_LOCK);

    if (!skip_reserve)
    {
        /*
         * Reserve some space in the Catalog file.
         */
        if ((error = cat_preflight(hfsmp, CAT_DELETE, NULL)))
        {
            hfs_systemfile_unlock(hfsmp, lockflags);
            goto out;
        }
    }

    error = cat_delete(hfsmp, &desc, &cp->c_attr);

    if (!error)
    {
        /* The parent lost a child */
        if (dcp->c_entries > 0)
            dcp->c_entries--;
        DEC_FOLDERCOUNT(hfsmp, dcp->c_attr);
        dcp->c_dirchangecnt++;
        hfs_incr_gencount(dcp);

        dcp->c_touch_chgtime = TRUE;
        dcp->c_touch_modtime = TRUE;
        dcp->c_flag |= C_MODIFIED;

        hfs_update(dcp->c_vp, 0);
    }

    hfs_systemfile_unlock(hfsmp, lockflags);

    if (error)
        goto out;

    hfs_volupdate(hfsmp, VOL_RMDIR, (dcp->c_cnid == kHFSRootFolderID));

    /* Mark C_NOEXISTS since the catalog entry is now gone */
    cp->c_flag |= C_NOEXISTS;

out:
    dvp->sExtraData.sDirData.uDirVersion++;

    dcp->c_flag &= ~C_DIR_MODIFICATION;
//TBD - We have wakeup here but can't see anyone who's msleeping on c_flag...
//    wakeup((caddr_t)&dcp->c_flag);

    if (started_tr)
    {
        hfs_end_transaction(hfsmp);
    }

    return (error);
}

int hfs_vnop_setattr( vnode_t vp, const UVFSFileAttributes *attr )
{
    int err = 0;
    if ( attr->fa_validmask == 0 )
    {
        return 0;
    }

    if ( ( attr->fa_validmask & READ_ONLY_FA_FIELDS )
         /*|| ( attr->fa_validmask & ~VALID_IN_ATTR_MASK )*/)
    {
        return EINVAL;
    }

    struct cnode *cp = NULL;

    /* Don't allow modification of the journal. */
    struct hfsmount *hfsmp = VTOHFS(vp);
    if (hfs_is_journal_file(hfsmp, VTOC(vp))) {
        return (EPERM);
    }

    /*
     * File size change request.
     * We are guaranteed that this is not a directory, and that
     * the filesystem object is writeable.
     */

    if ( attr->fa_validmask & UVFS_FA_VALID_SIZE )
    {
        if (!vnode_isreg(vp))
        {
            if (vnode_isdir(vp) || vnode_islnk(vp))
            {
                return EPERM;
            }
            //otherwise return EINVAL
            return EINVAL;
        }

        // Take truncate lock
        hfs_lock_truncate(VTOC(vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);

        // hfs_truncate will deal with the cnode lock
        err = hfs_truncate(vp, attr->fa_size, 0, 0);

        hfs_unlock_truncate(VTOC(vp), HFS_LOCK_DEFAULT);
        if (err)
            return err;
    }

    
    if ((err = hfs_lock(VTOC(vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT)))
        return (err);
    cp = VTOC(vp);
    

    if ( attr->fa_validmask & UVFS_FA_VALID_UID )
    {
        cp->c_flag              |= C_MODIFIED;
        cp->c_touch_chgtime     = TRUE;
        cp->c_uid               = attr->fa_uid;
    }

    if ( attr->fa_validmask & UVFS_FA_VALID_GID )
    {
        cp->c_flag              |= C_MODIFIED;
        cp->c_touch_chgtime     = TRUE;
        cp->c_gid               = attr->fa_gid;
    }

    if ( attr->fa_validmask & UVFS_FA_VALID_MODE )
    {
        mode_t new_mode = (cp->c_mode & ~ALLPERMS) | (attr->fa_mode & ALLPERMS);
        if (new_mode != cp->c_mode) {
            cp->c_mode = new_mode;
            cp->c_flag |= C_MINOR_MOD;
        }
    }

    if ( attr->fa_validmask & UVFS_FA_VALID_BSD_FLAGS )
    {
        cp->c_bsdflags = attr->fa_bsd_flags;
    }

    /*
     * Timestamp updates.
     */
    if ( attr->fa_validmask & UVFS_FA_VALID_ATIME )
    {
        cp->c_atime = attr->fa_atime.tv_sec;
        cp->c_touch_acctime = FALSE;
    }

    if ( attr->fa_validmask & UVFS_FA_VALID_BIRTHTIME )
    {
        cp->c_ctime = attr->fa_birthtime.tv_sec;
    }

    if ( attr->fa_validmask & UVFS_FA_VALID_MTIME )
    {
        cp->c_mtime = attr->fa_mtime.tv_sec;
        cp->c_touch_modtime = FALSE;
        cp->c_touch_chgtime = TRUE;

        hfs_clear_might_be_dirty_flag(cp);
    }

    err = hfs_update(vp, 0);

    /* Purge origin cache for cnode, since caller now has correct link ID for it
     * We purge it here since it was acquired for us during lookup, and we no longer need it.
     */
    if ((cp->c_flag & C_HARDLINK) && (!IS_DIR(vp))){
        hfs_relorigin(cp, 0);
    }

    hfs_unlock(cp);
    
    return err;
}

/*
 * Update a cnode's on-disk metadata.
 *
 * The cnode must be locked exclusive.  See declaration for possible
 * options.
 */
int
hfs_update(struct vnode *vp, int options)
{
#pragma unused (options)
    
    struct cnode *cp = VTOC(vp);
    const struct cat_fork *dataforkp = NULL;
    const struct cat_fork *rsrcforkp = NULL;
    struct cat_fork datafork;
    struct cat_fork rsrcfork;
    struct hfsmount *hfsmp;
   int lockflags;
    int error = 0;

    if (ISSET(cp->c_flag, C_NOEXISTS))
        return 0;

    hfsmp = VTOHFS(vp);

    if (((vnode_issystem(vp) && (cp->c_cnid < kHFSFirstUserCatalogNodeID))) ||
        hfsmp->hfs_catalog_vp == NULL){
        return (0);
    }

    if ((hfsmp->hfs_flags & HFS_READ_ONLY) || (cp->c_mode == 0)) {
        CLR(cp->c_flag, C_MODIFIED | C_MINOR_MOD | C_NEEDS_DATEADDED);
        cp->c_touch_acctime = 0;
        cp->c_touch_chgtime = 0;
        cp->c_touch_modtime = 0;
        return (0);
    }

    hfs_touchtimes(hfsmp, cp);

    if (!ISSET(cp->c_flag, C_MODIFIED | C_MINOR_MOD)
        && !hfs_should_save_atime(cp)) {
        // Nothing to update
        return 0;
    }

    bool check_txn = false;
    if (!ISSET(options, HFS_UPDATE_FORCE) && !ISSET(cp->c_flag, C_MODIFIED)) {
        /*
         * This must be a minor modification.  If the current
         * transaction already has an update for this node, then we
         * bundle in the modification.
         */
        if (hfsmp->jnl
            && journal_current_txn(hfsmp->jnl) == cp->c_update_txn) {
            check_txn = true;
        }
        else
        {
            error = 0;
            goto exit;
        }
    }

    error = hfs_start_transaction(hfsmp);
    if ( error != 0 )
    {
        goto exit;
    }

    if (check_txn
        && journal_current_txn(hfsmp->jnl) != cp->c_update_txn) {
        hfs_end_transaction(hfsmp);
        error = 0;
        goto exit;
    }

    /*
     * Modify the values passed to cat_update based on whether or not
     * the file has invalid ranges or borrowed blocks.
     */
    dataforkp = hfs_prepare_fork_for_update(cp->c_datafork, NULL, &datafork, hfsmp->blockSize);
    rsrcforkp = hfs_prepare_fork_for_update(cp->c_rsrcfork, NULL, &rsrcfork, hfsmp->blockSize);

    /*
     * Lock the Catalog b-tree file.
     */
    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_EXCLUSIVE_LOCK);

    error = cat_update(hfsmp, &cp->c_desc, &cp->c_attr, dataforkp, rsrcforkp);

    if (hfsmp->jnl)
        cp->c_update_txn = journal_current_txn(hfsmp->jnl);

    hfs_systemfile_unlock(hfsmp, lockflags);

    CLR(cp->c_flag, C_MODIFIED | C_MINOR_MOD);

    hfs_end_transaction(hfsmp);

exit:

    return error;
}

/*
 * Prepares a fork for cat_update by making sure ff_size and ff_blocks
 * are no bigger than the valid data on disk thus reducing the chance
 * of exposing uninitialised data in the event of a non clean unmount.
 * fork_buf is where to put the temporary copy if required.  (It can
 * be inside pfork.)
 */
const struct cat_fork *
hfs_prepare_fork_for_update(filefork_t *ff, const struct cat_fork *cf, struct cat_fork *cf_buf, uint32_t block_size)
{
    if (!ff)
        return NULL;

    if (!cf)
        cf = &ff->ff_data;
    if (!cf_buf)
        cf_buf = &ff->ff_data;

    off_t max_size = ff->ff_size;
   
    if (!ff->ff_unallocblocks && ff->ff_size <= max_size)
        return cf; // Nothing to do

    if (ff->ff_blocks < ff->ff_unallocblocks) {
        LFHFS_LOG(LEVEL_ERROR, "hfs_prepare_fork_for_update: ff_blocks %d is less than unalloc blocks %d\n",
                  ff->ff_blocks, ff->ff_unallocblocks);
        hfs_assert(0);
    }

    struct cat_fork *out = cf_buf;

    if (out != cf)
        bcopy(cf, out, sizeof(*cf));

    // Adjust cf_blocks for cf_vblocks
    out->cf_blocks -= out->cf_vblocks;

    /*
     * Here we trim the size with the updated cf_blocks.  This is
     * probably unnecessary now because the invalid ranges should
     * catch this (but that wasn't always the case).
     */
    off_t alloc_bytes = blk_to_bytes(out->cf_blocks, block_size);
    if (out->cf_size > alloc_bytes)
        out->cf_size = alloc_bytes;

    // Trim cf_size to first invalid range
    if (out->cf_size > max_size)
        out->cf_size = max_size;

    return out;
}

/*
 * Read contents of a symbolic link.
 */
int
hfs_vnop_readlink( struct vnode *vp, void* data, size_t dataSize, size_t *actuallyRead )
{
    struct cnode *cp;
    struct filefork *fp;
    int error;

    if (!vnode_islnk(vp))
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_readlink: Received node is not a symlink\n");
        return (EINVAL);
    }

    if ((error = hfs_lock(VTOC(vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT)))
        return (error);
    cp = VTOC(vp);
    fp = VTOF(vp);

    /* Zero length sym links are not allowed */
    if (fp->ff_size == 0 || fp->ff_size > MAXPATHLEN) {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_readlink: Symlink is with invalid content length\n");
        error = EINVAL;
        goto exit;
    }

    if ( dataSize < (size_t)fp->ff_size+1 )
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_readlink: Received buffer size is too small\n");
        error = ENOBUFS;
        goto exit;
    }

    /* Cache the path so we don't waste buffer cache resources */
    if (fp->ff_symlinkptr == NULL) {
        GenericLFBufPtr bp = NULL;

        fp->ff_symlinkptr = hfs_mallocz(fp->ff_size);
        if ( fp->ff_symlinkptr == NULL )
        {
            error = ENOMEM;
            goto exit;
        }

        bp = lf_hfs_generic_buf_allocate( vp, 0, roundup((int)fp->ff_size, VTOHFS(vp)->hfs_physical_block_size), 0);
        error = lf_hfs_generic_buf_read(bp);
        if (error) {
            lf_hfs_generic_buf_release(bp);
            if (fp->ff_symlinkptr) {
                hfs_free(fp->ff_symlinkptr);
                fp->ff_symlinkptr = NULL;
            }
            goto exit;
        }
        bcopy(bp->pvData, fp->ff_symlinkptr, (size_t)fp->ff_size);
        lf_hfs_generic_buf_release(bp);
    }

    memcpy(data, fp->ff_symlinkptr, fp->ff_size);
    ((uint8_t*)data)[fp->ff_size] = 0;
    *actuallyRead = fp->ff_size+1;

exit:
    hfs_unlock(cp);
    return (error);
}

/*
 * Make a directory.
 */
int
hfs_vnop_mkdir(vnode_t a_dvp, vnode_t *a_vpp, struct componentname *a_cnp, UVFSFileAttributes* a_vap)
{
    int iErr = 0;
    
    /***** HACK ALERT ********/
    a_cnp->cn_flags |= MAKEENTRY;
    a_vap->fa_type = UVFS_FA_TYPE_DIR;

    iErr = hfs_makenode(a_dvp, a_vpp, a_cnp, a_vap);

#if HFS_CRASH_TEST
        CRASH_ABORT(CRASH_ABORT_MAKE_DIR, a_dvp->mount, NULL);
#endif
    
    return(iErr);
}

/*
 * Create a regular file.
 */
int
hfs_vnop_create(vnode_t a_dvp, vnode_t *a_vpp, struct componentname *a_cnp, UVFSFileAttributes* a_vap)
{
    a_vap->fa_type = UVFS_FA_TYPE_FILE;
    return hfs_makenode(a_dvp, a_vpp, a_cnp, a_vap);    
}

/*
 * Allocate a new node
 */
int
hfs_makenode(struct vnode *dvp, struct vnode **vpp, struct componentname *cnp, UVFSFileAttributes *psGivenAttr)
{
    struct hfsmount *hfsmp = VTOHFS(dvp);
    struct cnode *dcp = NULL;
    struct cnode *cp = NULL;
    struct vnode *tvp = NULL;
    enum vtype vnodetype = UVFSTOV(psGivenAttr->fa_type);
    mode_t mode = MAKEIMODE(vnodetype);
    struct cat_attr attr = {0};
    int lockflags;
    int error, started_tr = 0;

    int newvnode_flags = 0;
    u_int32_t gnv_flags = 0;
    int nocache = 0;
    struct cat_desc out_desc = {0};
    out_desc.cd_flags = 0;
    out_desc.cd_nameptr = NULL;

    if ((error = hfs_lock(VTOC(dvp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT)))
        return (error);
    dcp = VTOC(dvp);
    
    /* Don't allow creation of new entries in open-unlinked directories */
    if (dcp->c_flag & (C_DELETED | C_NOEXISTS))
    {
        error = ENOENT;
        goto exit;
    }

    if ( !(psGivenAttr->fa_validmask & UVFS_FA_VALID_MODE) && (vnodetype != VDIR) )
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_makenode: Invalid mode or type[%#llx, %d]",
				  (unsigned long long)psGivenAttr->fa_validmask, psGivenAttr->fa_type);
        error = EINVAL;
        goto exit;
    }

    if ( ( psGivenAttr->fa_validmask & READ_ONLY_FA_FIELDS ) /*|| ( psGivenAttr->fa_validmask & ~VALID_IN_ATTR_MASK )*/ )
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_makenode: Setting readonly fields or invalid mask[%#llx, %#llx]", (unsigned long long)psGivenAttr->fa_validmask, (unsigned long long)READ_ONLY_FA_FIELDS);
        error = EINVAL;
        goto exit;
    }

    dcp->c_flag |= C_DIR_MODIFICATION;

    *vpp = NULL;

    /* Check if were out of usable disk space. */
    if (hfs_freeblks(hfsmp, 1) == 0)
    {
        error = ENOSPC;
        goto exit;
    }

    struct timeval tv;
    microtime(&tv);

    /* Setup the default attributes */
    if ( psGivenAttr->fa_validmask & UVFS_FA_VALID_MODE )
    {
        mode = (mode & ~ALLPERMS) | (psGivenAttr->fa_mode & ALLPERMS);
    }

    attr.ca_mode = mode;
    attr.ca_linkcount = 1;
    attr.ca_itime = tv.tv_sec;
    attr.ca_atime = attr.ca_ctime = attr.ca_mtime = attr.ca_itime;
    attr.ca_atimeondisk = attr.ca_atime;

    /*
     * HFS+ only: all files get ThreadExists
     */
    if (vnodetype == VDIR)
    {
        if (hfsmp->hfs_flags & HFS_FOLDERCOUNT)
        {
            attr.ca_recflags = kHFSHasFolderCountMask;
        }
    }
    else
    {
        attr.ca_recflags = kHFSThreadExistsMask;
    }

    /*
     * Add the date added to the item. See above, as
     * all of the dates are set to the itime.
     */
    hfs_write_dateadded (&attr, attr.ca_atime);

    /* Initialize the gen counter to 1 */
    hfs_write_gencount(&attr, (uint32_t)1);

    if ( psGivenAttr->fa_validmask & UVFS_FA_VALID_UID )
    {
        attr.ca_uid               = psGivenAttr->fa_uid;
    }

    if ( psGivenAttr->fa_validmask & UVFS_FA_VALID_GID )
    {
        attr.ca_gid               = psGivenAttr->fa_gid;
    }

    /* Tag symlinks with a type and creator. */
    if (vnodetype == VLNK)
    {
        struct FndrFileInfo *fip;

        fip = (struct FndrFileInfo *)&attr.ca_finderinfo;
        fip->fdType    = SWAP_BE32(kSymLinkFileType);
        fip->fdCreator = SWAP_BE32(kSymLinkCreator);
    }

    /* Setup the descriptor */
    struct cat_desc in_desc ={0};
    in_desc.cd_nameptr = (const u_int8_t *)cnp->cn_nameptr;
    in_desc.cd_namelen = cnp->cn_namelen;
    in_desc.cd_parentcnid = dcp->c_fileid;
    in_desc.cd_flags = S_ISDIR(mode) ? CD_ISDIR : 0;
    in_desc.cd_hint = dcp->c_childhint;
    in_desc.cd_encoding = 0;

    if ((error = hfs_start_transaction(hfsmp)) != 0)
    {
        goto exit;
    }
    started_tr = 1;

    // have to also lock the attribute file because cat_create() needs
    // to check that any fileID it wants to use does not have orphaned
    // attributes in it.
    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG | SFL_ATTRIBUTE, HFS_EXCLUSIVE_LOCK);
    cnid_t new_id = 0;

    /* Reserve some space in the Catalog file. */
    error = cat_preflight(hfsmp, CAT_CREATE, NULL);
    if (error != 0)
    {
        hfs_systemfile_unlock(hfsmp, lockflags);
        goto exit;
    }
    
    error = cat_acquire_cnid(hfsmp, &new_id);
    if (error != 0)
    {
        hfs_systemfile_unlock (hfsmp, lockflags);
        goto exit;
    }

    error = cat_create(hfsmp, new_id, &in_desc, &attr, &out_desc);
    if (error == 0) {
        /* Update the parent directory */
        dcp->c_childhint = out_desc.cd_hint;    /* Cache directory's location */
        dcp->c_entries++;

        if (vnodetype == VDIR)
        {
            INC_FOLDERCOUNT(hfsmp, dcp->c_attr);
        }
        dcp->c_dirchangecnt++;
        hfs_incr_gencount(dcp);

        dcp->c_touch_chgtime = dcp->c_touch_modtime = true;
        dcp->c_flag |= C_MODIFIED;

        hfs_update(dcp->c_vp, 0);
    }
    hfs_systemfile_unlock(hfsmp, lockflags);
    if (error)
        goto exit;

    uint32_t txn = hfsmp->jnl ? journal_current_txn(hfsmp->jnl) : 0;

    hfs_volupdate(hfsmp, vnodetype == VDIR ? VOL_MKDIR : VOL_MKFILE, (dcp->c_cnid == kHFSRootFolderID));

    // XXXdbg
    // have to end the transaction here before we call hfs_getnewvnode()
    // because that can cause us to try and reclaim a vnode on a different
    // file system which could cause us to start a transaction which can
    // deadlock with someone on that other file system (since we could be
    // holding two transaction locks as well as various vnodes and we did
    // not obtain the locks on them in the proper order).
    //
    // NOTE: this means that if the quota check fails or we have to update
    //       the change time on a block-special device that those changes
    //       will happen as part of independent transactions.
    //
    if (started_tr)
    {
        hfs_end_transaction(hfsmp);
        started_tr = 0;
    }

    gnv_flags |= GNV_CREATE;
    if (nocache)
    {
        gnv_flags |= GNV_NOCACHE;
    }

    /*
     * Create a vnode for the object just created.
     *
     * NOTE: Maintaining the cnode lock on the parent directory is important,
     * as it prevents race conditions where other threads want to look up entries
     * in the directory and/or add things as we are in the process of creating
     * the vnode below.  However, this has the potential for causing a
     * double lock panic when dealing with shadow files on a HFS boot partition.
     * The panic could occur if we are not cleaning up after ourselves properly
     * when done with a shadow file or in the error cases.  The error would occur if we
     * try to create a new vnode, and then end up reclaiming another shadow vnode to
     * create the new one.  However, if everything is working properly, this should
     * be a non-issue as we would never enter that reclaim codepath.
     *
     * The cnode is locked on successful return.
     */
    error = hfs_getnewvnode(hfsmp, dvp, cnp, &out_desc, gnv_flags, &attr,
                            NULL, &tvp, &newvnode_flags);
    if (error)
        goto exit;

    cp = VTOC(tvp);

    cp->c_update_txn = txn;

    *vpp = tvp;

exit:
    cat_releasedesc(&out_desc);

    //Update Directory version
    dvp->sExtraData.sDirData.uDirVersion++;

    /*
     * Make sure we release cnode lock on dcp.
     */
    if (dcp)
    {
        dcp->c_flag &= ~C_DIR_MODIFICATION;

        //TBD - We have wakeup here but can't see anyone who's msleeping on c_flag...
        //wakeup((caddr_t)&dcp->c_flag);
        hfs_unlock(dcp);
    }

    if (cp != NULL) {
        hfs_unlock(cp);
    }
    if (started_tr) {
        hfs_end_transaction(hfsmp);
    }

    return (error);
}

/*
 * Create a symbolic link.
 */
int
hfs_vnop_symlink(struct vnode *dvp, struct vnode **vpp, struct componentname *cnp, char* symlink_content, UVFSFileAttributes *attrp)
{
    struct vnode *vp = NULL;
    struct cnode *cp = NULL;
    struct hfsmount *hfsmp;
    struct filefork *fp;
    GenericLFBufPtr bp = NULL;
    char *datap;
    int started_tr = 0;
    uint64_t len;
    int error;

    hfsmp = VTOHFS(dvp);

    len = strlen(symlink_content);
    if (len > MAXPATHLEN)
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_symlink: Received symlink content too long\n");
        return (ENAMETOOLONG);
    }

    if (len == 0 )
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_symlink: Received zero length symlink content\n");
        return (EINVAL);
    }

    /* Check for free space */
    if (((u_int64_t)hfs_freeblks(hfsmp, 0) * (u_int64_t)hfsmp->blockSize) < len) {
        return (ENOSPC);
    }
    
    attrp->fa_type       = UVFS_FA_TYPE_SYMLINK;
    attrp->fa_mode      |= S_IFLNK;
    attrp->fa_validmask |= UVFS_FA_VALID_MODE;

    /* Create the vnode */
    if ((error = hfs_makenode(dvp, vpp, cnp, attrp))) {
        goto out;
    }
    vp = *vpp;
    if ((error = hfs_lock(VTOC(vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT))) {
        goto out;
    }
    cp = VTOC(vp);
    fp = VTOF(vp);

    if (cp->c_flag & (C_NOEXISTS | C_DELETED)) {
        goto out;
    }

#if QUOTA
    (void)hfs_getinoquota(cp);
#endif /* QUOTA */

    if ((error = hfs_start_transaction(hfsmp)) != 0) {
        goto out;
    }
    started_tr = 1;

    /*
     * Allocate space for the link.
     *
     * Since we're already inside a transaction,
     *
     * Don't need truncate lock since a symlink is treated as a system file.
     */
    error = hfs_truncate(vp, len, IO_NOZEROFILL, 0);

    /* On errors, remove the symlink file */
    if (error) {
        /*
         * End the transaction so we don't re-take the cnode lock
         * below while inside a transaction (lock order violation).
         */
        hfs_end_transaction(hfsmp);
        /* hfs_removefile() requires holding the truncate lock */
        hfs_unlock(cp);
        hfs_lock_truncate(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
        hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS);

        if (hfs_start_transaction(hfsmp) != 0) {
            started_tr = 0;
            hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
            goto out;
        }

        (void) hfs_removefile(dvp, vp, cnp, 0, 0, 0, 0);
        hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
        goto out;
    }

    /* Write the sym-link to disk */
    bp = lf_hfs_generic_buf_allocate( vp, 0, roundup((int)fp->ff_size, hfsmp->hfs_physical_block_size), 0);
    error = lf_hfs_generic_buf_read( bp );
    if ( error != 0 )
    {
        goto out;
    }

    if (hfsmp->jnl)
    {
        journal_modify_block_start(hfsmp->jnl, bp);
    }
    datap = bp->pvData;
    assert(bp->uDataSize >= len);
    bzero(datap, bp->uDataSize);
    bcopy(symlink_content, datap, len);
    if (hfsmp->jnl)
    {
        journal_modify_block_end(hfsmp->jnl, bp, NULL, NULL);
        bp = NULL;  // block will be released by the journal
    }
    else
    {
        error = lf_hfs_generic_buf_write(bp);
        if ( error != 0 )
        {
            goto out;
        }
    }
out:
    if (started_tr)
        hfs_end_transaction(hfsmp);
    
    if ((cp != NULL) && (vp != NULL)) {
        hfs_unlock(cp);
    }
    if (error) {
        if (vp) {
            //            vnode_put(vp);
        }
        *vpp = NULL;
    }
    
    if ( bp ) {
        lf_hfs_generic_buf_release(bp);
    }
    
    hfs_flush(hfsmp, HFS_FLUSH_FULL);
    
    return (error);
}

/*
 * Rename a cnode.
 *
 * The VFS layer guarantees that:
 *   - source and destination will either both be directories, or
 *     both not be directories.
 *   - all the vnodes are from the same file system
 *
 * When the target is a directory, HFS must ensure that its empty.
 *
 * Note that this function requires up to 6 vnodes in order to work properly
 * if it is operating on files (and not on directories).  This is because only
 * files can have resource forks, and we now require iocounts to be held on the
 * vnodes corresponding to the resource forks (if applicable) as well as
 * the files or directories undergoing rename.  The problem with not holding
 * iocounts on the resource fork vnodes is that it can lead to a deadlock
 * situation: The rsrc fork of the source file may be recycled and reclaimed
 * in order to provide a vnode for the destination file's rsrc fork.  Since
 * data and rsrc forks share the same cnode, we'd eventually try to lock the
 * source file's cnode in order to sync its rsrc fork to disk, but it's already
 * been locked.  By taking the rsrc fork vnodes up front we ensure that they
 * cannot be recycled, and that the situation mentioned above cannot happen.
 */
int
hfs_vnop_renamex(struct vnode *fdvp,struct vnode *fvp, struct componentname *fcnp, struct vnode *tdvp, struct vnode *tvp, struct componentname *tcnp)
{

    /*
     * Note that we only need locals for the target/destination's
     * resource fork vnode (and only if necessary).  We don't care if the
     * source has a resource fork vnode or not.
     */
    struct vnode *tvp_rsrc = NULL;
    struct cnode *tcp      = NULL;
    struct cnode *error_cnode;
    struct cat_desc from_desc;

    struct hfsmount *hfsmp = VTOHFS(tdvp);
    int tvp_deleted = 0;
    int started_tr = 0, got_cookie = 0;
    int took_trunc_lock = 0;
    int lockflags;
    int error;

    int rename_exclusive = 0;

retry:
    /* When tvp exists, take the truncate lock for hfs_removefile(). */
    if (tvp && (vnode_isreg(tvp) || vnode_islnk(tvp))) {
        hfs_lock_truncate(VTOC(tvp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
        took_trunc_lock = 1;
    }

    if (tvp && VTOC(tvp) == NULL)
        return (EINVAL);
    
    error = hfs_lockfour(VTOC(fdvp), VTOC(fvp), VTOC(tdvp), tvp ? VTOC(tvp) : NULL, HFS_EXCLUSIVE_LOCK, &error_cnode);
    if (error)
    {
        if (took_trunc_lock)
        {
            hfs_unlock_truncate(VTOC(tvp), HFS_LOCK_DEFAULT);
            took_trunc_lock = 0;
        }

        /*
         * We hit an error path.  If we were trying to re-acquire the locks
         * after coming through here once, we might have already obtained
         * an iocount on tvp's resource fork vnode.  Drop that before dealing
         * with the failure.  Note this is safe -- since we are in an
         * error handling path, we can't be holding the cnode locks.
         */
        if (tvp_rsrc && tcp)
        {
            hfs_chash_lower_OpenLookupCounter(tcp);
            tvp_rsrc = NULL;
        }

        /*
         * tvp might no longer exist.  If the cause of the lock failure
         * was tvp, then we can try again with tvp/tcp set to NULL.
         * This is ok because the vfs syscall will vnode_put the vnodes
         * after we return from hfs_vnop_rename.
         */
        if ((error == ENOENT) && (tvp != NULL) && (error_cnode == VTOC(tvp))) {
            tcp = NULL;
            tvp = NULL;
            goto retry;
        }

        /* If we want to reintroduce notifications for failed renames, this
         is the place to do it. */

        return (error);
    }

    struct cnode* fdcp = VTOC(fdvp);
    struct cnode* fcp = VTOC(fvp);
    struct cnode* tdcp = VTOC(tdvp);
    tcp = tvp ? VTOC(tvp) : NULL;

    /*
     * If caller requested an exclusive rename (VFS_RENAME_EXCL) and 'tcp' exists
     * then we must fail the operation.
     */
    if (tcp && rename_exclusive)
    {
        error = EEXIST;
        goto out;
    }

    /*
     * Acquire iocounts on the destination's resource fork vnode
     * if necessary. If dst/src are files and the dst has a resource
     * fork vnode, then we need to try and acquire an iocount on the rsrc vnode.
     * If it does not exist, then we don't care and can skip it.
     */
    if ((vnode_isreg(fvp)) || (vnode_islnk(fvp)))
    {
        if ((tvp) && (tcp->c_rsrc_vp) && (tvp_rsrc == NULL))
        {
            tvp_rsrc = tcp->c_rsrc_vp;
            hfs_chash_raise_OpenLookupCounter(tcp);
            
            /* Unlock everything to acquire iocount on this rsrc vnode */
            if (took_trunc_lock)
            {
                hfs_unlock_truncate (VTOC(tvp), HFS_LOCK_DEFAULT);
                took_trunc_lock = 0;
            }
            
            hfs_unlockfour(fdcp, fcp, tdcp, tcp);
            
            goto retry;
        }
    }

    /* Ensure we didn't race src or dst parent directories with rmdir. */
    if (fdcp->c_flag & (C_NOEXISTS | C_DELETED))
    {
        error = ENOENT;
        goto out;
    }

    if (tdcp->c_flag & (C_NOEXISTS | C_DELETED))
    {
        error = ENOENT;
        goto out;
    }


    /* Check for a race against unlink.  The hfs_valid_cnode checks validate
     * the parent/child relationship with fdcp and tdcp, as well as the
     * component name of the target cnodes.
     */
    if ((fcp->c_flag & (C_NOEXISTS | C_DELETED)) || !hfs_valid_cnode(hfsmp, fdvp, fcnp, fcp->c_fileid, NULL, &error))
    {
        error = ENOENT;
        goto out;
    }

    if (tcp && ((tcp->c_flag & (C_NOEXISTS | C_DELETED)) || !hfs_valid_cnode(hfsmp, tdvp, tcnp, tcp->c_fileid, NULL, &error)))
    {
        //
        // hmm, the destination vnode isn't valid any more.
        // in this case we can just drop him and pretend he
        // never existed in the first place.
        //
        if (took_trunc_lock)
        {
            hfs_unlock_truncate(VTOC(tvp), HFS_LOCK_DEFAULT);
            took_trunc_lock = 0;
        }
        error = 0;

        hfs_unlockfour(fdcp, fcp, tdcp, tcp);

        tcp = NULL;
        tvp = NULL;

        // retry the locking with tvp null'ed out
        goto retry;
    }

    fdcp->c_flag |= C_DIR_MODIFICATION;
    if (fdvp != tdvp)
    {
        tdcp->c_flag |= C_DIR_MODIFICATION;
    }

    /*
     * Disallow renaming of a directory hard link if the source and
     * destination parent directories are different, or a directory whose
     * descendant is a directory hard link and the one of the ancestors
     * of the destination directory is a directory hard link.
     */
    if (vnode_isdir(fvp) && (fdvp != tdvp))
    {
        if (fcp->c_flag & C_HARDLINK) {
            error = EPERM;
            goto out;
        }
        if (fcp->c_attr.ca_recflags & kHFSHasChildLinkMask)
        {
            lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);
            if (cat_check_link_ancestry(hfsmp, tdcp->c_fileid, 0))
            {
                error = EPERM;
                hfs_systemfile_unlock(hfsmp, lockflags);
                goto out;
            }
            hfs_systemfile_unlock(hfsmp, lockflags);
        }
    }

    /*
     * The following edge case is caught here:
     * (to cannot be a descendent of from)
     *
     *       o fdvp
     *      /
     *     /
     *    o fvp
     *     \
     *      \
     *       o tdvp
     *      /
     *     /
     *    o tvp
     */
    if (tdcp->c_parentcnid == fcp->c_fileid)
    {
        error = EINVAL;
        goto out;
    }

    /*
     * The following two edge cases are caught here:
     * (note tvp is not empty)
     *
     *       o tdvp               o tdvp
     *      /                    /
     *     /                    /
     *    o tvp            tvp o fdvp
     *     \                    \
     *      \                    \
     *       o fdvp               o fvp
     *      /
     *     /
     *    o fvp
     */
    if (tvp && vnode_isdir(tvp) && (tcp->c_entries != 0) && fvp != tvp)
    {
        error = ENOTEMPTY;
        goto out;
    }

    /*
     * The following edge case is caught here:
     * (the from child and parent are the same)
     *
     *          o tdvp
     *         /
     *        /
     *  fdvp o fvp
     */
    if (fdvp == fvp)
    {
        error = EINVAL;
        goto out;
    }

    /*
     * Make sure "from" vnode and its parent are changeable.
     */
    if ((fcp->c_bsdflags & (SF_IMMUTABLE | UF_IMMUTABLE | UF_APPEND | SF_APPEND)) || (fdcp->c_bsdflags & (UF_APPEND | SF_APPEND)))
    {
        error = EPERM;
        goto out;
    }

    /* Don't allow modification of the journal or journal_info_block */
    if (hfs_is_journal_file(hfsmp, fcp) || (tcp && hfs_is_journal_file(hfsmp, tcp)))
    {
        error = EPERM;
        goto out;
    }

    struct cat_desc out_desc = {0};
    from_desc.cd_nameptr = (const u_int8_t *)fcnp->cn_nameptr;
    from_desc.cd_namelen = fcnp->cn_namelen;
    from_desc.cd_parentcnid = fdcp->c_fileid;
    from_desc.cd_flags = fcp->c_desc.cd_flags & ~(CD_HASBUF | CD_DECOMPOSED);
    from_desc.cd_cnid = fcp->c_cnid;

    struct cat_desc to_desc = {0};
    to_desc.cd_nameptr = (const u_int8_t *)tcnp->cn_nameptr;
    to_desc.cd_namelen = tcnp->cn_namelen;
    to_desc.cd_parentcnid = tdcp->c_fileid;
    to_desc.cd_flags = fcp->c_desc.cd_flags & ~(CD_HASBUF | CD_DECOMPOSED);
    to_desc.cd_cnid = fcp->c_cnid;

    if ((error = hfs_start_transaction(hfsmp)) != 0)
    {
        goto out;
    }
    started_tr = 1;

    /* hfs_vnop_link() and hfs_vnop_rename() set kHFSHasChildLinkMask
     * inside a journal transaction and without holding a cnode lock.
     * As setting of this bit depends on being in journal transaction for
     * concurrency, check this bit again after we start journal transaction for rename
     * to ensure that this directory does not have any descendant that
     * is a directory hard link.
     */
    if (vnode_isdir(fvp) && (fdvp != tdvp))
    {
        if (fcp->c_attr.ca_recflags & kHFSHasChildLinkMask)
        {
            lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);
            if (cat_check_link_ancestry(hfsmp, tdcp->c_fileid, 0)) {
                error = EPERM;
                hfs_systemfile_unlock(hfsmp, lockflags);
                goto out;
            }
            hfs_systemfile_unlock(hfsmp, lockflags);
        }
    }

    // if it's a hardlink then re-lookup the name so
    // that we get the correct cnid in from_desc (see
    // the comment in hfs_removefile for more details)
    if (fcp->c_flag & C_HARDLINK)
    {
        struct cat_desc tmpdesc;
        cnid_t real_cnid;

        tmpdesc.cd_nameptr = (const u_int8_t *)fcnp->cn_nameptr;
        tmpdesc.cd_namelen = fcnp->cn_namelen;
        tmpdesc.cd_parentcnid = fdcp->c_fileid;
        tmpdesc.cd_hint = fdcp->c_childhint;
        tmpdesc.cd_flags = fcp->c_desc.cd_flags & CD_ISDIR;
        tmpdesc.cd_encoding = 0;

        lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);

        if (cat_lookup(hfsmp, &tmpdesc, 0, NULL, NULL, NULL, &real_cnid) != 0)
        {
            hfs_systemfile_unlock(hfsmp, lockflags);
            goto out;
        }

        // use the real cnid instead of whatever happened to be there
        from_desc.cd_cnid = real_cnid;
        hfs_systemfile_unlock(hfsmp, lockflags);
    }

    /*
     * Reserve some space in the Catalog file.
     */
    cat_cookie_t cookie;
    if ((error = cat_preflight(hfsmp, CAT_RENAME + CAT_DELETE, &cookie)))
    {
        goto out;
    }
    got_cookie = 1;

    /*
     * If the destination exists then it may need to be removed.
     *
     * Due to HFS's locking system, we should always move the
     * existing 'tvp' element to the hidden directory in hfs_vnop_rename.
     * Because the VNOP_LOOKUP call enters and exits the filesystem independently
     * of the actual vnop that it was trying to do (stat, link, readlink),
     * we must release the cnode lock of that element during the interim to
     * do MAC checking, vnode authorization, and other calls.  In that time,
     * the item can be deleted (or renamed over). However, only in the rename
     * case is it inappropriate to return ENOENT from any of those calls.  Either
     * the call should return information about the old element (stale), or get
     * information about the newer element that we are about to write in its place.
     *
     * HFS lookup has been modified to detect a rename and re-drive its
     * lookup internally. For other calls that have already succeeded in
     * their lookup call and are waiting to acquire the cnode lock in order
     * to proceed, that cnode lock will not fail due to the cnode being marked
     * C_NOEXISTS, because it won't have been marked as such.  It will only
     * have C_DELETED.  Thus, they will simply act on the stale open-unlinked
     * element.  All future callers will get the new element.
     *
     * To implement this behavior, we pass the "only_unlink" argument to
     * hfs_removefile and hfs_removedir.  This will result in the vnode acting
     * as though it is open-unlinked.  Additionally, when we are done moving the
     * element to the hidden directory, we vnode_recycle the target so that it is
     * reclaimed as soon as possible.  Reclaim and inactive are both
     * capable of clearing out unused blocks for an open-unlinked file or dir.
     */
    if (tvp)
    {
        /*
         * When fvp matches tvp they could be case variants
         * or matching hard links.
         */
        if (fvp == tvp)
        {
            if (!(fcp->c_flag & C_HARDLINK))
            {
                /*
                 * If they're not hardlinks, then fvp == tvp must mean we
                 * are using case-insensitive HFS because case-sensitive would
                 * not use the same vnode for both.  In this case we just update
                 * the catalog for: a -> A
                 */
                goto skip_rm;  /* simple case variant */

            }
            /* For all cases below, we must be using hardlinks */
            else if ((fdvp != tdvp) || (hfsmp->hfs_flags & HFS_CASE_SENSITIVE))
            {
                /*
                 * If the parent directories are not the same, AND the two items
                 * are hardlinks, posix says to do nothing:
                 * dir1/fred <-> dir2/bob   and the op was mv dir1/fred -> dir2/bob
                 * We just return 0 in this case.
                 *
                 * If case sensitivity is on, and we are using hardlinks
                 * then renaming is supposed to do nothing.
                 * dir1/fred <-> dir2/FRED, and op == mv dir1/fred -> dir2/FRED
                 */
                goto out;

            }
            else if (hfs_namecmp((const u_int8_t *)fcnp->cn_nameptr, fcnp->cn_namelen, (const u_int8_t *)tcnp->cn_nameptr, tcnp->cn_namelen) == 0)
            {
                /*
                 * If we get here, then the following must be true:
                 * a) We are running case-insensitive HFS+.
                 * b) Both paths 'fvp' and 'tvp' are in the same parent directory.
                 * c) the two names are case-variants of each other.
                 *
                 * In this case, we are really only dealing with a single catalog record
                 * whose name is being updated.
                 *
                 * op is dir1/fred -> dir1/FRED
                 *
                 * We need to special case the name matching, because if
                 * dir1/fred <-> dir1/bob were the two links, and the
                 * op was dir1/fred -> dir1/bob
                 * That would fail/do nothing.
                 */
                goto skip_rm;  /* case-variant hardlink in the same dir */
            }
            else
            {
                goto out;  /* matching hardlink, nothing to do */
            }
        }


        if (vnode_isdir(tvp))
        {
            /*
             * hfs_removedir will eventually call hfs_removefile on the directory
             * we're working on, because only hfs_removefile does the renaming of the
             * item to the hidden directory.  The directory will stay around in the
             * hidden directory with C_DELETED until it gets an inactive or a reclaim.
             * That way, we can destroy all of the EAs as needed and allow new ones to be
             * written.
             */
            error = hfs_removedir(tdvp, tvp, tcnp, HFSRM_SKIP_RESERVE, 0);
        }
        else
        {
            error = hfs_removefile(tdvp, tvp, tcnp, 0, HFSRM_SKIP_RESERVE, 0, 0);

            /*
             * If the destination file had a resource fork vnode, then we need to get rid of
             * its blocks when there are no more references to it.  Because the call to
             * hfs_removefile above always open-unlinks things, we need to force an inactive/reclaim
             * on the resource fork vnode, in order to prevent block leaks.  Otherwise,
             * the resource fork vnode could prevent the data fork vnode from going out of scope
             * because it holds a v_parent reference on it.  So we mark it for termination
             * with a call to vnode_recycle. hfs_vnop_reclaim has been modified so that it
             * can clean up the blocks of open-unlinked files and resource forks.
             *
             * We can safely call vnode_recycle on the resource fork because we took an iocount
             * reference on it at the beginning of the function.
             */

            if ((error == 0) && (tcp->c_flag & C_DELETED) && (tvp_rsrc))
            {
                hfs_chash_lower_OpenLookupCounter(tcp);
                tvp_rsrc = NULL;
            }
        }

        if (error)
        {
            goto out;
        }

        tvp_deleted = 1;

        if ( ((VTOC(tvp)->c_flag & C_HARDLINK) ==  0 ) || (VTOC(tvp)->c_linkcount == 0) )
        {
            INVALIDATE_NODE(tvp);
        }

        /* Mark 'tcp' as being deleted due to a rename */
        tcp->c_flag |= C_RENAMED;

        /*
         * Aggressively mark tvp/tcp for termination to ensure that we recover all blocks
         * as quickly as possible.
         */
        //TBD -- Need to see what we are doing with recycle
//        vnode_recycle(tvp);
    }

skip_rm:
    /*
     * All done with tvp and fvp.
     *
     * We also jump to this point if there was no destination observed during lookup and namei.
     * However, because only iocounts are held at the VFS layer, there is nothing preventing a
     * competing thread from racing us and creating a file or dir at the destination of this rename
     * operation.  If this occurs, it may cause us to get a spurious EEXIST out of the cat_rename
     * call below.  To preserve rename's atomicity, we need to signal VFS to re-drive the
     * namei/lookup and restart the rename operation.  EEXIST is an allowable errno to be bubbled
     * out of the rename syscall, but not for this reason, since it is a synonym errno for ENOTEMPTY.
     * To signal VFS, we return ERECYCLE (which is also used for lookup restarts). This errno
     * will be swallowed and it will restart the operation.
     */

    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_EXCLUSIVE_LOCK);
    error = cat_rename(hfsmp, &from_desc, &tdcp->c_desc, &to_desc, &out_desc);
    hfs_systemfile_unlock(hfsmp, lockflags);

    if (error)
    {
        if (error == EEXIST)
        {
            error = ERECYCLE;
        }
        goto out;
    }

    /* Update cnode's catalog descriptor */
    replace_desc(fcp, &out_desc);
    fcp->c_parentcnid = tdcp->c_fileid;
    fcp->c_hint = 0;

    /*
     * Now indicate this cnode needs to have date-added written to the
     * finderinfo, but only if moving to a different directory, or if
     * it doesn't already have it.
     */
    if (fdvp != tdvp || !ISSET(fcp->c_attr.ca_recflags, kHFSHasDateAddedMask))
        fcp->c_flag |= C_NEEDS_DATEADDED;

    (void) hfs_update (fvp, 0);

    hfs_volupdate(hfsmp, vnode_isdir(fvp) ? VOL_RMDIR : VOL_RMFILE, (fdcp->c_cnid == kHFSRootFolderID));
    hfs_volupdate(hfsmp, vnode_isdir(fvp) ? VOL_MKDIR : VOL_MKFILE, (tdcp->c_cnid == kHFSRootFolderID));

    /* Update both parent directories. */
    if (fdvp != tdvp)
    {
        if (vnode_isdir(fvp))
        {
            /* If the source directory has directory hard link
             * descendants, set the kHFSHasChildLinkBit in the
             * destination parent hierarchy
             */
            if ((fcp->c_attr.ca_recflags & kHFSHasChildLinkMask) && !(tdcp->c_attr.ca_recflags & kHFSHasChildLinkMask))
            {

                tdcp->c_attr.ca_recflags |= kHFSHasChildLinkMask;

                error = cat_set_childlinkbit(hfsmp, tdcp->c_parentcnid);
                if (error)
                {
                    LFHFS_LOG(LEVEL_DEBUG, "hfs_vnop_rename: error updating parent chain for %u\n", tdcp->c_cnid);
                    error = 0;
                }
            }
            INC_FOLDERCOUNT(hfsmp, tdcp->c_attr);
            DEC_FOLDERCOUNT(hfsmp, fdcp->c_attr);
        }
        tdcp->c_entries++;
        tdcp->c_dirchangecnt++;
        tdcp->c_flag |= C_MODIFIED;
        hfs_incr_gencount(tdcp);

        if (fdcp->c_entries > 0)
            fdcp->c_entries--;
        fdcp->c_dirchangecnt++;
        fdcp->c_flag |= C_MODIFIED;
        fdcp->c_touch_chgtime = TRUE;
        fdcp->c_touch_modtime = TRUE;

        if (ISSET(fcp->c_flag, C_HARDLINK))
        {
            hfs_relorigin(fcp, fdcp->c_fileid);
            if (fdcp->c_fileid != fdcp->c_cnid)
                hfs_relorigin(fcp, fdcp->c_cnid);
        }

        (void) hfs_update(fdvp, 0);
    }
    hfs_incr_gencount(fdcp);

    tdcp->c_childhint = out_desc.cd_hint;    /* Cache directory's location */
    tdcp->c_touch_chgtime = TRUE;
    tdcp->c_touch_modtime = TRUE;

    (void) hfs_update(tdvp, 0);

    /* Update the vnode's name now that the rename has completed. */
    vnode_update_identity(fvp, tdvp, tcnp->cn_nameptr, tcnp->cn_namelen, tcnp->cn_hash, (VNODE_UPDATE_PARENT | VNODE_UPDATE_NAME));

    /*
     * At this point, we may have a resource fork vnode attached to the
     * 'from' vnode.  If it exists, we will want to update its name, because
     * it contains the old name + _PATH_RSRCFORKSPEC. ("/..namedfork/rsrc").
     *
     * Note that the only thing we need to update here is the name attached to
     * the vnode, since a resource fork vnode does not have a separate resource
     * cnode -- it's still 'fcp'.
     */
    if (fcp->c_rsrc_vp)
    {
        char* rsrc_path = NULL;
        int len;

        /* Create a new temporary buffer that's going to hold the new name */
        rsrc_path = hfs_malloc(MAXPATHLEN);
        len = snprintf (rsrc_path, MAXPATHLEN, "%s%s", tcnp->cn_nameptr, _PATH_RSRCFORKSPEC);
        len = MIN(len, MAXPATHLEN);

        /*
         * vnode_update_identity will do the following for us:
         * 1) release reference on the existing rsrc vnode's name.
         * 2) attach the new name to the resource vnode
         * 3) update the vnode's vid
         */
        vnode_update_identity (fcp->c_rsrc_vp, fvp, rsrc_path, len, 0, (VNODE_UPDATE_NAME | VNODE_UPDATE_CACHE));

        /* Free the memory associated with the resource fork's name */
        hfs_free(rsrc_path);
    }
out:
    if (got_cookie)
    {
        cat_postflight(hfsmp, &cookie);
    }
    if (started_tr)
    {
        hfs_end_transaction(hfsmp);
    }

    fdvp->sExtraData.sDirData.uDirVersion++;
    fdcp->c_flag &= ~C_DIR_MODIFICATION;
    //TBD - We have wakeup here but can't see anyone who's msleeping on c_flag...
//    wakeup((caddr_t)&fdcp->c_flag);

    if (fdvp != tdvp)
    {
        tdvp->sExtraData.sDirData.uDirVersion++;
        tdcp->c_flag &= ~C_DIR_MODIFICATION;
        //TBD - We have wakeup here but can't see anyone who's msleeping on c_flag...
//        wakeup((caddr_t)&tdcp->c_flag);

    }

    /* Now vnode_put the resource forks vnodes if necessary */
    if (tvp_rsrc)
    {
        hfs_chash_lower_OpenLookupCounter(tcp);
        tvp_rsrc = NULL;
    }
    
    hfs_unlockfour(fdcp, fcp, tdcp, tcp);

    if (took_trunc_lock)
    {
        hfs_unlock_truncate(VTOC(tvp), HFS_LOCK_DEFAULT);
    }

    /* After tvp is removed the only acceptable error is EIO */
    if (error && tvp_deleted)
        error = EIO;

    return (error);
}

/*
 * link vnode operation
 *
 *  IN vnode_t  a_vp;
 *  IN vnode_t  a_tdvp;
 *  IN struct componentname  *a_cnp;
 *  IN vfs_context_t  a_context;
 */
int
hfs_vnop_link(vnode_t vp, vnode_t tdvp, struct componentname *cnp)
{
    struct hfsmount *hfsmp = VTOHFS(vp);;
    struct cnode *cp  = VTOC(vp);;
    struct cnode *tdcp;
    struct cnode *fdcp = NULL;
    struct cat_desc todesc;
    cnid_t parentcnid;
    int lockflags = 0;
    int intrans = 0;
    enum vtype v_type = vp->sFSParams.vnfs_vtype;
    int error, ret;

    /*
     * For now, return ENOTSUP for a symlink target. This can happen
     * for linkat(2) when called without AT_SYMLINK_FOLLOW.
     */
    if (v_type == VLNK || v_type == VDIR)
        return (EPERM );

    /* Make sure our private directory exists. */
    if (hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid == 0) {
        return (ENOTSUP);
    }

    if (hfs_freeblks(hfsmp, 0) == 0) {
        return (ENOSPC);
    }

    /* Lock the cnodes. */
    if ((error = hfs_lockpair(VTOC(tdvp), VTOC(vp), HFS_EXCLUSIVE_LOCK))) {
        return (error);
    }

    tdcp = VTOC(tdvp);
    /* grab the parent CNID from originlist after grabbing cnode locks */
    parentcnid = hfs_currentparent(cp, /* have_lock: */ true);

    if (tdcp->c_flag & (C_NOEXISTS | C_DELETED)) {
        error = ENOENT;
        goto out;
    }

    /* Check the source for errors:
     * too many links, immutable, race with unlink
     */
    if (cp->c_linkcount >= HFS_LINK_MAX) {
        error = EMLINK;
        goto out;
    }
    if (cp->c_bsdflags & (UF_IMMUTABLE | SF_IMMUTABLE | UF_APPEND | SF_APPEND)) {
        error = EPERM;
        goto out;
    }
    if (cp->c_flag & (C_NOEXISTS | C_DELETED)) {
        error = ENOENT;
        goto out;
    }

    tdcp->c_flag |= C_DIR_MODIFICATION;

    if (hfs_start_transaction(hfsmp) != 0) {
        error = EINVAL;
        goto out;
    }
    intrans = 1;

    todesc.cd_flags = (v_type == VDIR) ? CD_ISDIR : 0;
    todesc.cd_encoding = 0;
    todesc.cd_nameptr = (const u_int8_t *)cnp->cn_nameptr;
    todesc.cd_namelen = cnp->cn_namelen;
    todesc.cd_parentcnid = tdcp->c_fileid;
    todesc.cd_hint = 0;
    todesc.cd_cnid = 0;

    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);

    /* If destination exists then we lost a race with create. */
    if (cat_lookup(hfsmp, &todesc, 0, NULL, NULL, NULL, NULL) == 0) {
        error = EEXIST;
        goto out;
    }
    if (cp->c_flag & C_HARDLINK) {
        struct cat_attr cattr;

        /* If inode is missing then we lost a race with unlink. */
        if ((cat_idlookup(hfsmp, cp->c_fileid, 0, 0, NULL, &cattr, NULL) != 0) ||
            (cattr.ca_fileid != cp->c_fileid)) {
            error = ENOENT;
            goto out;
        }
    } else {
        cnid_t fileid;

        /* If source is missing then we lost a race with unlink. */
        if ((cat_lookup(hfsmp, &cp->c_desc, 0, NULL, NULL, NULL, &fileid) != 0) ||
            (fileid != cp->c_fileid)) {
            error = ENOENT;
            goto out;
        }
    }
    /*
     * All directory links must reside in an non-ARCHIVED hierarchy.
     */
    if (v_type == VDIR) {
        /*
         * - Source parent and destination parent cannot match
         * - A link is not permitted in the root directory
         * - Parent of 'pointed at' directory is not the root directory
         * - The 'pointed at' directory (source) is not an ancestor
         *   of the new directory hard link (destination).
         * - No ancestor of the new directory hard link (destination)
         *   is a directory hard link.
         */
        if ((parentcnid == tdcp->c_fileid) ||
            (tdcp->c_fileid == kHFSRootFolderID) ||
            (parentcnid == kHFSRootFolderID) ||
            cat_check_link_ancestry(hfsmp, tdcp->c_fileid, cp->c_fileid)) {
            error = EPERM;  /* abide by the rules, you did not */
            goto out;
        }
    }
    hfs_systemfile_unlock(hfsmp, lockflags);
    lockflags = 0;

    cp->c_linkcount++;
    cp->c_flag |= C_MODIFIED;
    cp->c_touch_chgtime = TRUE;
    error = hfs_makelink(hfsmp, vp, cp, tdcp, cnp);
    if (error) {
        cp->c_linkcount--;
        hfs_volupdate(hfsmp, VOL_UPDATE, 0);
    } else {
        /* Update the target directory and volume stats */
        tdcp->c_entries++;
        if (v_type == VDIR) {
            INC_FOLDERCOUNT(hfsmp, tdcp->c_attr);
            tdcp->c_attr.ca_recflags |= kHFSHasChildLinkMask;

            /* Set kHFSHasChildLinkBit in the destination hierarchy */
            error = cat_set_childlinkbit(hfsmp, tdcp->c_parentcnid);
            if (error) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_link: error updating destination parent chain for id=%u, vol=%s\n", tdcp->c_cnid, hfsmp->vcbVN);
            }
        }
        tdcp->c_dirchangecnt++;
        tdcp->c_flag |= C_MODIFIED;
        hfs_incr_gencount(tdcp);
        tdcp->c_touch_chgtime = TRUE;
        tdcp->c_touch_modtime = TRUE;

        error = hfs_update(tdvp, 0);
        if (error) {
            if (error != EIO && error != ENXIO) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_link: error %d updating tdvp %p\n", error, tdvp);
                error = EIO;
            }
            hfs_mark_inconsistent(hfsmp, HFS_OP_INCOMPLETE);
        }

        hfs_volupdate(hfsmp, VOL_MKFILE, (tdcp->c_cnid == kHFSRootFolderID));
    }

    if (error == 0 && (ret = hfs_update(vp, 0)) != 0) {
        if (ret != EIO && ret != ENXIO)
            LFHFS_LOG(LEVEL_ERROR, "hfs_vnop_link: error %d updating vp @ %p\n", ret, vp);
        hfs_mark_inconsistent(hfsmp, HFS_OP_INCOMPLETE);
    }

out:
    if (lockflags) {
        hfs_systemfile_unlock(hfsmp, lockflags);
    }
    if (intrans) {
        hfs_end_transaction(hfsmp);
    }

    tdcp->c_flag &= ~C_DIR_MODIFICATION;
    //TBD - We have wakeup here but can't see anyone who's msleeping on c_flag...
//    wakeup((caddr_t)&tdcp->c_flag);

    if (fdcp) {
        hfs_unlockfour(tdcp, cp, fdcp, NULL);
    } else {
        hfs_unlockpair(tdcp, cp);
    }

    return (error);
}

int hfs_removefile_callback(GenericLFBuf *psBuff, void *pvArgs) {
    
    journal_kill_block(((struct hfsmount *)pvArgs)->jnl, psBuff);
    
    return (0);
}


/*
 * hfs_vgetrsrc acquires a resource fork vnode corresponding to the
 * cnode that is found in 'vp'.  The cnode should be locked upon entry
 * and will be returned locked, but it may be dropped temporarily.
 *
 * If the resource fork vnode does not exist, HFS will attempt to acquire an
 * empty (uninitialized) vnode from VFS so as to avoid deadlocks with
 * jetsam. If we let the normal getnewvnode code produce the vnode for us
 * we would be doing so while holding the cnode lock of our cnode.
 *
 * On success, *rvpp wlll hold the resource fork vnode with an
 * iocount.  *Don't* forget the vnode_put.
 */
int
hfs_vgetrsrc( struct vnode *vp, struct vnode **rvpp)
{
    struct hfsmount *hfsmp = VTOHFS(vp);
    struct vnode *rvp = NULL;
    struct cnode *cp = VTOC(vp);
    int error = 0;
    
restart:
    /* Attempt to use existing vnode */
    if ((rvp = cp->c_rsrc_vp)) {
        hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
        hfs_chash_raise_OpenLookupCounter(cp);

    } else {
        struct cat_fork rsrcfork;
        struct cat_desc *descptr = NULL;
        struct cat_desc to_desc;
        int newvnode_flags = 0;
        
        hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
        
        /*
         * We could have raced with another thread here while we dropped our cnode
         * lock.  See if the cnode now has a resource fork vnode and restart if appropriate.
         *
         * Note: We just released the cnode lock, so there is a possibility that the
         * cnode that we just acquired has been deleted or even removed from disk
         * completely, though this is unlikely. If the file is open-unlinked, the
         * check below will resolve it for us.  If it has been completely
         * removed (even from the catalog!), then when we examine the catalog
         * directly, below, while holding the catalog lock, we will not find the
         * item and we can fail out properly.
         */
        if (cp->c_rsrc_vp) {
            /* Drop the empty vnode before restarting */
            hfs_unlock(cp);
            rvp = NULL;
            goto restart;
        }
        
        /*
         * hfs_vgetsrc may be invoked for a cnode that has already been marked
         * C_DELETED.  This is because we need to continue to provide rsrc
         * fork access to open-unlinked files.  In this case, build a fake descriptor
         * like in hfs_removefile.  If we don't do this, buildkey will fail in
         * cat_lookup because this cnode has no name in its descriptor.
         */
        if ((cp->c_flag & C_DELETED ) && (cp->c_desc.cd_namelen == 0)) {
            char delname[32];
            bzero (&to_desc, sizeof(to_desc));
            bzero (delname, 32);
            MAKE_DELETED_NAME(delname, sizeof(delname), cp->c_fileid);
            to_desc.cd_nameptr = (const u_int8_t*) delname;
            to_desc.cd_namelen = strlen(delname);
            to_desc.cd_parentcnid = hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid;
            to_desc.cd_flags = 0;
            to_desc.cd_cnid = cp->c_cnid;
            
            descptr = &to_desc;
        }
        else {
            descptr = &cp->c_desc;
        }
        
        
        int lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);
        
        /*
         * We call cat_idlookup (instead of cat_lookup) below because we can't
         * trust the descriptor in the provided cnode for lookups at this point.
         * Between the time of the original lookup of this vnode and now, the
         * descriptor could have gotten swapped or replaced.  If this occurred,
         * the parent/name combo originally desired may not necessarily be provided
         * if we use the descriptor.  Even worse, if the vnode represents
         * a hardlink, we could have removed one of the links from the namespace
         * but left the descriptor alone, since hfs_unlink does not invalidate
         * the descriptor in the cnode if other links still point to the inode.
         *
         * Consider the following (slightly contrived) scenario:
         * /tmp/a <--> /tmp/b (hardlinks).
         * 1. Thread A: open rsrc fork on /tmp/b.
         * 1a. Thread A: does lookup, goes out to lunch right before calling getnamedstream.
         * 2. Thread B does 'mv /foo/b /tmp/b'
         * 2. Thread B succeeds.
         * 3. Thread A comes back and wants rsrc fork info for /tmp/b.
         *
         * Even though the hardlink backing /tmp/b is now eliminated, the descriptor
         * is not removed/updated during the unlink process.  So, if you were to
         * do a lookup on /tmp/b, you'd acquire an entirely different record's resource
         * fork.
         *
         * As a result, we use the fileid, which should be invariant for the lifetime
         * of the cnode (possibly barring calls to exchangedata).
         *
         * Addendum: We can't do the above for HFS standard since we aren't guaranteed to
         * have thread records for files.  They were only required for directories.  So
         * we need to do the lookup with the catalog name. This is OK since hardlinks were
         * never allowed on HFS standard.
         */
        
        /* Get resource fork data */
        error = cat_idlookup (hfsmp, cp->c_fileid, 0, 1, NULL, NULL, &rsrcfork);
        
        hfs_systemfile_unlock(hfsmp, lockflags);
        if (error) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_vgetrsrc: cat_idlookup failed with error [%d]\n", error);
            hfs_unlock(cp);
            hfs_chash_lower_OpenLookupCounter(cp);
            return (error);
        }
        /*
         * Supply hfs_getnewvnode with a component name.
         */
        struct componentname cn;
        cn.cn_pnbuf = NULL;
        if (descptr->cd_nameptr) {
            void *buf = hfs_malloc(MAXPATHLEN);
            
            cn = (struct componentname){
                .cn_nameiop = LOOKUP,
                .cn_flags    = ISLASTCN,
                .cn_pnlen    = MAXPATHLEN,
                .cn_pnbuf    = buf,
                .cn_nameptr = buf,
                .cn_namelen = snprintf(buf, MAXPATHLEN,
                                       "%s%s", descptr->cd_nameptr,
                                       _PATH_RSRCFORKSPEC)
            };
            
            // Should never happen because cn.cn_nameptr won't ever be long...
            if (cn.cn_namelen >= MAXPATHLEN) {
                hfs_free(buf);
                LFHFS_LOG(LEVEL_ERROR, "hfs_vgetrsrc: cnode name too long [ENAMETOOLONG]\n");
                hfs_unlock(cp);
                hfs_chash_lower_OpenLookupCounter(cp);
                return ENAMETOOLONG;
            }
        }
        
        /*
         * We are about to call hfs_getnewvnode and pass in the vnode that we acquired
         * earlier when we were not holding any locks. The semantics of GNV_USE_VP require that
         * either hfs_getnewvnode consume the vnode and vend it back to us, properly initialized,
         * or it will consume/dispose of it properly if it errors out.
         */
        error = hfs_getnewvnode(hfsmp, NULL, cn.cn_pnbuf ? &cn : NULL,
                                descptr, (GNV_WANTRSRC | GNV_SKIPLOCK),
                                &cp->c_attr, &rsrcfork, &rvp, &newvnode_flags);
                
        hfs_free(cn.cn_pnbuf);
        if (error)
            return (error);
    }  /* End 'else' for rsrc fork not existing */
    
    *rvpp = rvp;
    return (0);
}
