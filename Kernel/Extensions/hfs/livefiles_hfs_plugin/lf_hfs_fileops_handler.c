/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_fileops_handler.c
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 31/12/2017.
*/

#include "lf_hfs_fileops_handler.h"
#include "lf_hfs_dirops_handler.h"
#include "lf_hfs.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_vnode.h"
#include "lf_hfs_raw_read_write.h"
#include "lf_hfs_vnops.h"
#include "lf_hfs_xattr.h"
#include "lf_hfs_cnode.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_file_extent_mapping.h"
#include "lf_hfs_readwrite_ops.h"
#include "lf_hfs_file_mgr_internal.h"


int LFHFS_Read ( UVFSFileNode psNode, uint64_t uOffset, size_t iLength, void *pvBuf, size_t *iActuallyRead )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Read  (psNode %p, uOffset %llu, iLength %lu)\n", psNode, uOffset, iLength);
    VERIFY_NODE_IS_VALID(psNode);
    
    struct vnode *vp = (vnode_t)psNode;
    struct cnode *cp;
    struct filefork *fp;
    uint64_t filesize;
    int retval = 0;
    int took_truncate_lock = 0;
    *iActuallyRead = 0;
    
    /* Preflight checks */
    if (!vnode_isreg(vp)) {
        /* can only read regular files */
        return ( vnode_isdir(vp) ? EISDIR : EPERM );
    }

    cp = VTOC(vp);
    fp = VTOF(vp);

    /* Protect against a size change. */
    hfs_lock_truncate(cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT);
    took_truncate_lock = 1;

    filesize = fp->ff_size;
    /*
     * Check the file size. Note that per POSIX spec, we return 0 at
     * file EOF, so attempting a read at an offset that is too big
     * should just return 0 on HFS+. Since the return value was initialized
     * to 0 above, we just jump to exit.  HFS Standard has its own behavior.
     */
    if (uOffset > filesize)
    {
        LFHFS_LOG( LEVEL_ERROR, "LFHFS_Read: wanted offset is greater then file size\n" );
        goto exit;
    }

    // If we asked to read above the file size, adjust the read size;
    if ( uOffset + iLength > filesize )
    {
        iLength = filesize - uOffset;
    }

    uint64_t uReadStartCluster;
    retval = raw_readwrite_read( vp, uOffset, pvBuf, iLength, iActuallyRead, &uReadStartCluster );

    cp->c_touch_acctime = TRUE;

exit:
    if (took_truncate_lock)
    {
        hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
    }
    return retval;
}


int LFHFS_Write ( UVFSFileNode psNode, uint64_t uOffset, size_t iLength, const void *pvBuf, size_t *iActuallyWrite )
{
#pragma unused (psNode, uOffset, iLength, pvBuf, iActuallyWrite)

    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Write (psNode %p, uOffset %llu, iLength %lu)\n", psNode, uOffset, iLength);
    VERIFY_NODE_IS_VALID(psNode);
    
    *iActuallyWrite = 0;
    struct vnode *vp = (vnode_t)psNode;
    struct cnode *cp;
    struct filefork *fp;
    struct hfsmount *hfsmp;
    off_t origFileSize;
    off_t writelimit;
    off_t bytesToAdd = 0;
    off_t actualBytesAdded;
    off_t filebytes;
    int eflags = kEFReserveMask;
    int retval = 0;
    int lockflags;
    int cnode_locked = 0;

    int took_truncate_lock = 0;
    size_t iActualLengthToWrite = iLength;

    if (!vnode_isreg(vp))
    {
        return ( vnode_isdir(vp) ? EISDIR : EPERM );  /* Can only write regular files */
    }

    cp = VTOC(vp);
    fp = VTOF(vp);
    hfsmp = VTOHFS(vp);
    
    /*
     * Protect against a size change.
     */
    hfs_lock_truncate(cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT);
    took_truncate_lock = 1;

    origFileSize = fp->ff_size;
    writelimit = uOffset + iLength;

    /*
     * We may need an exclusive truncate lock for several reasons, all
     * of which are because we may be writing to a (portion of a) block
     * for the first time, and we need to make sure no readers see the
     * prior, uninitialized contents of the block.  The cases are:
     *
     * 1. We have unallocated (delayed allocation) blocks.  We may be
     *    allocating new blocks to the file and writing to them.
     *    (A more precise check would be whether the range we're writing
     *    to contains delayed allocation blocks.)
     * 2. We need to extend the file.  The bytes between the old EOF
     *    and the new EOF are not yet initialized.  This is important
     *    even if we're not allocating new blocks to the file.  If the
     *    old EOF and new EOF are in the same block, we still need to
     *    protect that range of bytes until they are written for the
     *    first time.
     *
     * If we had a shared lock with the above cases, we need to try to upgrade
     * to an exclusive lock.  If the upgrade fails, we will lose the shared
     * lock, and will need to take the truncate lock again; the took_truncate_lock
     * flag will still be set, causing us to try for an exclusive lock next time.
     */
    if ((cp->c_truncatelockowner == HFS_SHARED_OWNER) &&
        ((fp->ff_unallocblocks != 0) ||
         (writelimit > origFileSize)))
    {
            lf_lck_rw_lock_shared_to_exclusive(&cp->c_truncatelock);
            /* Store the owner in the c_truncatelockowner field if we successfully upgrade */
            cp->c_truncatelockowner = pthread_self();
    }

    if ( (retval = hfs_lock(VTOC(vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT))) {
        goto exit;
    }
    cnode_locked = 1;

    filebytes = blk_to_bytes(fp->ff_blocks, hfsmp->blockSize);

    if ((off_t)uOffset > filebytes
        && (blk_to_bytes(hfs_freeblks(hfsmp, ISSET(eflags, kEFReserveMask)) , hfsmp->blockSize) < (off_t)uOffset - filebytes))
    {
        retval = ENOSPC;
        goto exit;
    }

    /* Check if we do not need to extend the file */
    if (writelimit <= filebytes) {
        goto sizeok;
    }

    bytesToAdd = writelimit - filebytes;
    if (hfs_start_transaction(hfsmp) != 0) {
        retval = EINVAL;
        goto exit;
    }

    while (writelimit > filebytes)
    {
        bytesToAdd = writelimit - filebytes;

        /* Protect extents b-tree and allocation bitmap */
        lockflags = SFL_BITMAP;
        if (overflow_extents(fp))
            lockflags |= SFL_EXTENTS;
        lockflags = hfs_systemfile_lock(hfsmp, lockflags, HFS_EXCLUSIVE_LOCK);

        retval = MacToVFSError(ExtendFileC (hfsmp, (FCB*)fp, bytesToAdd,
                                            0, eflags, &actualBytesAdded));

        hfs_systemfile_unlock(hfsmp, lockflags);

        if ((actualBytesAdded == 0) && (retval == E_NONE))
            retval = ENOSPC;
        if (retval != E_NONE)
            break;
        filebytes = (off_t)fp->ff_blocks * (off_t)hfsmp->blockSize;
    }
    
    (void) hfs_update(vp, 0);
    (void) hfs_volupdate(hfsmp, VOL_UPDATE, 0);
    (void) hfs_end_transaction(hfsmp);

    /*
     * If we didn't grow the file enough try a partial write.
     * POSIX expects this behavior.
     */
    if ((retval == ENOSPC) && (filebytes > (off_t)uOffset)) {
        retval = 0;
        iActualLengthToWrite -= bytesToAdd;
        writelimit = filebytes;
    }
sizeok:
    if (retval == E_NONE) {
        off_t filesize;

        if (writelimit > fp->ff_size) {
            filesize = writelimit;
            struct timeval tv;
            rl_add(fp->ff_size, writelimit - 1 , &fp->ff_invalidranges);
            microuptime(&tv);
            cp->c_zftimeout = (uint32_t)(tv.tv_sec + ZFTIMELIMIT);
        } else
            filesize = fp->ff_size;


        // Fill last cluster with zeros.
        if ( origFileSize < (off_t)uOffset )
        {
            raw_readwrite_zero_fill_last_block_suffix(vp);
        }

        if (filesize > fp->ff_size) {
            fp->ff_new_size = filesize;
        }

        uint64_t uActuallyWritten;
        retval = raw_readwrite_write(vp, uOffset, (void*)pvBuf, iActualLengthToWrite, &uActuallyWritten);
        *iActuallyWrite = uActuallyWritten;
        if (retval) {
            fp->ff_new_size = 0;    /* no longer extending; use ff_size */
            goto ioerr_exit;
        }

        if (filesize > origFileSize) {
            fp->ff_size = filesize;
        }
        fp->ff_new_size = 0;    /* ff_size now has the correct size */
    }

    hfs_flush(hfsmp, HFS_FLUSH_CACHE);

ioerr_exit:
    if (!cnode_locked)
    {
        hfs_lock(cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_ALLOW_NOEXISTS);
        cnode_locked = 1;
    }

    if (*iActuallyWrite > 0)
    {
        cp->c_flag |= C_MODIFIED;
        cp->c_touch_chgtime = TRUE;
        cp->c_touch_modtime = TRUE;
        hfs_incr_gencount(cp);
    }
    if (retval)
    {
        (void)hfs_truncate(vp, origFileSize, IO_SYNC, 0);
    }
    else if (*iActuallyWrite > 0)
    {
        retval = hfs_update(vp, 0);
    }

    /* Updating vcbWrCnt doesn't need to be atomic. */
    hfsmp->vcbWrCnt++;

exit:
    if (retval && took_truncate_lock
        && cp->c_truncatelockowner == pthread_self()) {
        fp->ff_new_size = 0;
        rl_remove(fp->ff_size, RL_INFINITY, &fp->ff_invalidranges);
    }

    if (cnode_locked) {
        hfs_unlock(cp);
    }

    if (took_truncate_lock) {
        hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
    }

    return (retval);
}

int LFHFS_Create ( UVFSFileNode psNode, const char *pcName, const UVFSFileAttributes *psAttr, UVFSFileNode *ppsOutNode )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Create\n");
    VERIFY_NODE_IS_VALID(psNode);
    
    int iError = 0;
    vnode_t psParentVnode = (vnode_t)psNode;

    if (!vnode_isdir(psParentVnode))
    {
        iError = ENOTDIR;
        goto exit;
    }
    
    //@param cnp Name information for new directory.
    struct componentname sNewFileComponentName = {0};
    sNewFileComponentName.cn_nameptr = (char*) pcName;
    sNewFileComponentName.cn_namelen = (int) strlen(pcName);

    iError = hfs_vnop_create(psParentVnode, (vnode_t*)ppsOutNode, &sNewFileComponentName, (UVFSFileAttributes *) psAttr);
    if (iError)
        goto exit;

    //Since hfs_vnop_create doesn’t allocate clusters for new files.
    //In case of non-zero given size, we need to call setAttr, after successfully creating the file.
    if ((psAttr->fa_validmask & UVFS_FA_VALID_SIZE) != 0 && psAttr->fa_size != 0)
    {
        iError =  hfs_vnop_setattr( (vnode_t) *ppsOutNode, psAttr );
        //In case of a failure in setAttr, need to remove the created file
        if (iError)
        {
            DIROPS_RemoveInternal(psParentVnode, pcName);
            LFHFS_Reclaim((vnode_t) *ppsOutNode);
        }
    }

exit:
    return iError;
}

int LFHFS_GetAttr ( UVFSFileNode psNode, UVFSFileAttributes *psOutAttr )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_GetAttr\n");
    VERIFY_NODE_IS_VALID(psNode);
    
    int iErr            = 0;
    vnode_t vp          = (vnode_t)psNode;
    
    hfs_lock(VTOC(vp),0,0);
    vnode_GetAttrInternal(vp, psOutAttr);
    hfs_unlock(VTOC(vp));
    
    return iErr;
}

int LFHFS_SetAttr ( UVFSFileNode psNode, const UVFSFileAttributes *psSetAttr, UVFSFileAttributes *psOutAttr )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_SetAttr\n");
    VERIFY_NODE_IS_VALID(psNode);

    vnode_t psVnode = (vnode_t)psNode;

    int iErr = hfs_vnop_setattr( psVnode, psSetAttr );
    if ( iErr != 0 )
    {
        goto exit;
    }

    iErr = LFHFS_GetAttr( psNode, psOutAttr );

exit:
    return iErr;
}

int LFHFS_Reclaim ( UVFSFileNode psNode )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Reclaim\n");

    int iErr = 0;
    vnode_t vp = (vnode_t)psNode;
    
    if ( psNode != NULL )
    {
        VERIFY_NODE_IS_VALID_FOR_RECLAIM(psNode);
        
        iErr = hfs_vnop_reclaim(vp);
        psNode = NULL;
    }

    return iErr;
}

int LFHFS_ReadLink ( UVFSFileNode psNode, void *pvOutBuf, size_t iBufSize, size_t *iActuallyRead, UVFSFileAttributes *psOutAttr )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_ReadLink\n");
    VERIFY_NODE_IS_VALID(psNode);

    int iErr = 0;
    *iActuallyRead = 0;
    vnode_t vp = (vnode_t)psNode;

    iErr = hfs_vnop_readlink(vp, pvOutBuf, iBufSize, iActuallyRead);
    if ( iErr != 0 )
    {
        goto exit;
    }

    iErr = LFHFS_GetAttr( psNode, psOutAttr );
    if ( iErr != 0 )
    {
        goto exit;
    }

exit:
    return iErr;
}

int LFHFS_SymLink ( UVFSFileNode psNode, const char *pcName, const char *psContent, const UVFSFileAttributes *psAttr, UVFSFileNode *ppsOutNode )
{
    VERIFY_NODE_IS_VALID(psNode);
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_SymLink\n");

    int iErr = 0;
    vnode_t psParentVnode        = (vnode_t)psNode;

    if (!vnode_isdir(psParentVnode))
    {
        iErr = ENOTDIR;
        goto exit;
    }

    vnode_t psSymLinkVnode = {0};
    struct componentname sCompName  = {0};
    sCompName.cn_nameiop            = CREATE;
    sCompName.cn_flags              = ISLASTCN;
    sCompName.cn_pnbuf              = (char *)pcName;
    sCompName.cn_pnlen              = (int)strlen(pcName);
    sCompName.cn_nameptr            = (char *)pcName;
    sCompName.cn_namelen            = (int)strlen(pcName);
    sCompName.cn_hash               = 0;
    sCompName.cn_consume            = (int)strlen(pcName);

    iErr = hfs_vnop_symlink( psParentVnode, &psSymLinkVnode, &sCompName, (char*)psContent, (UVFSFileAttributes *)psAttr );

    *ppsOutNode = (UVFSFileNode)psSymLinkVnode;

exit:
    return iErr;
}

int LFHFS_Rename (UVFSFileNode psFromDirNode, UVFSFileNode psFromNode, const char *pcFromName, UVFSFileNode psToDirNode, UVFSFileNode psToNode, const char *pcToName, uint32_t flags __unused)
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Rename\n");

    VERIFY_NODE_IS_VALID(psFromDirNode);
    VERIFY_NODE_IS_VALID(psToDirNode);
    if ( psFromNode != NULL )
    {
        VERIFY_NODE_IS_VALID(psFromNode);
    }
    if ( psToNode != NULL )
    {
        VERIFY_NODE_IS_VALID(psToNode);
    }

    int iErr = 0;
    vnode_t psFromParentVnode = (vnode_t)psFromDirNode;
    vnode_t psToParentVnode = (vnode_t)psToDirNode;

    if (!vnode_isdir(psFromParentVnode) || !vnode_isdir(psToParentVnode))
    {
        iErr = ENOTDIR;
        goto exit;
    }

    UVFSFileNode psFromFileNode = {0};
    UVFSFileNode psToFileNode   = {0};
    bool bGotFromNode = (psFromNode != NULL);
    bool bGotToNode   = (psToNode != NULL);

    vnode_t psFromVnode = (vnode_t) psFromNode;

    if (!bGotFromNode)
    {
        iErr = DIROPS_LookupInternal( psFromDirNode, pcFromName, &psFromFileNode );
        if ( iErr != 0 )
        {
            goto exit;
        }
        psFromVnode = (vnode_t)psFromFileNode;
    }

    vnode_t psToVnode = psToNode;
    if (!bGotToNode)
    {
        iErr = DIROPS_LookupInternal( psToDirNode, pcToName, &psToFileNode );
        if ( !iErr )
        {
           psToVnode = (vnode_t)psToFileNode;
        }
        else if (iErr != ENOENT)
        {
            goto exit;
        }
    }

    // If only one of the vnodes is of type directory,
    // we can't allow the rename
    if (psToVnode)
    {
        if (vnode_isdir(psFromVnode) && !vnode_isdir(psToVnode))
        {
            iErr = ENOTDIR;
            goto exit;
        }

        if (!vnode_isdir(psFromVnode) && vnode_isdir(psToVnode))
        {
            iErr = EISDIR;
            goto exit;
        }
    }
    struct componentname sFromCompName  = {0};
    sFromCompName.cn_nameiop            = RENAME;
    sFromCompName.cn_flags              = ISLASTCN;
    sFromCompName.cn_pnbuf              = (char *)pcFromName;
    sFromCompName.cn_pnlen              = (int)strlen(pcFromName);
    sFromCompName.cn_nameptr            = (char *)pcFromName;
    sFromCompName.cn_namelen            = (int)strlen(pcFromName);
    sFromCompName.cn_hash               = 0;
    sFromCompName.cn_consume            = (int)strlen(pcFromName);

    struct componentname sToCompName  = {0};
    sToCompName.cn_nameiop            = RENAME;
    sToCompName.cn_flags              = ISLASTCN;
    sToCompName.cn_pnbuf              = (char *)pcToName;
    sToCompName.cn_pnlen              = (int)strlen(pcToName);
    sToCompName.cn_nameptr            = (char *)pcToName;
    sToCompName.cn_namelen            = (int)strlen(pcToName);
    sToCompName.cn_hash               = 0;
    sToCompName.cn_consume            = (int)strlen(pcToName);

    iErr = hfs_vnop_renamex(psFromParentVnode, psFromVnode, &sFromCompName, psToParentVnode, psToVnode, &sToCompName);

    if (!bGotFromNode)
        LFHFS_Reclaim(psFromVnode);
    if (!bGotToNode && psToVnode)
        LFHFS_Reclaim(psToVnode);

exit:
    return iErr;
}

int LFHFS_Link ( UVFSFileNode psFromNode, UVFSFileNode psToDirNode, const char *pcToName, UVFSFileAttributes* psOutFileAttrs, UVFSFileAttributes* psOutDirAttrs )
{
    VERIFY_NODE_IS_VALID(psFromNode);
    VERIFY_NODE_IS_VALID(psToDirNode);

    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Link\n");
    int iErr = 0;

    vnode_t psFromVnode = (vnode_t)psFromNode;
    vnode_t psToDirVnode = (vnode_t)psToDirNode;

    if (!vnode_isdir(psToDirVnode))
    {
        return ENOTDIR;
    }

    /* Preflight checks */
    if (!vnode_isreg(psFromVnode)) {
        /* can only create hardlinks for regular files */
        return ( vnode_isdir(psFromVnode) ? EISDIR : EPERM );
    }

    struct componentname sToCompName  = {0};
    sToCompName.cn_nameiop            = CREATE;
    sToCompName.cn_flags              = ISLASTCN;
    sToCompName.cn_pnbuf              = (char *)pcToName;
    sToCompName.cn_pnlen              = (int)strlen(pcToName);
    sToCompName.cn_nameptr            = (char *)pcToName;
    sToCompName.cn_namelen            = (int)strlen(pcToName);
    sToCompName.cn_hash               = 0;
    sToCompName.cn_consume            = (int)strlen(pcToName);

    iErr = hfs_vnop_link(psFromVnode, psToDirVnode, &sToCompName);
    if ( iErr != 0 )
    {
        goto exit;
    }

    iErr = LFHFS_GetAttr( psFromNode, psOutFileAttrs );
    if ( iErr != 0 )
    {
        LFHFS_LOG(LEVEL_ERROR, "LFHFS_Link: Failed in getting FromNode Attr\n");
        goto exit;
    }

    iErr = LFHFS_GetAttr( psToDirNode, psOutDirAttrs );
    if ( iErr != 0 )
    {
        LFHFS_LOG(LEVEL_ERROR, "LFHFS_Link: Failed in getting ToDir Attr\n");
        goto exit;
    }

exit:
    return iErr;
}

int LFHFS_GetXAttr ( UVFSFileNode psNode, const char *pcAttr, void *pvOutBuf, size_t iBufSize, size_t *iActualSize )
{
    int iErr = 0;

    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_GetXAttr\n");

    VERIFY_NODE_IS_VALID(psNode);

    iErr = hfs_vnop_getxattr((vnode_t)psNode, pcAttr, pvOutBuf, iBufSize, iActualSize);

    return iErr;
}

int LFHFS_SetXAttr ( UVFSFileNode psNode, const char *pcAttr, const void *pvInBuf, size_t iBufSize, UVFSXattrHow How )
{
    int iErr = 0;

    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_SetXAttr\n");

    VERIFY_NODE_IS_VALID(psNode);

    if (How == UVFSXattrHowRemove)
    {
        iErr = hfs_vnop_removexattr((vnode_t)psNode, pcAttr);
    }
    else
    {
        iErr = hfs_vnop_setxattr((vnode_t)psNode, pcAttr, pvInBuf, iBufSize, How);
    }

    return iErr;
}

int LFHFS_ListXAttr ( UVFSFileNode psNode, void *pvOutBuf, size_t iBufSize, size_t *iActualSize )
{
    int iErr = 0;

    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_ListXAttr\n");

    VERIFY_NODE_IS_VALID(psNode);

    iErr = hfs_vnop_listxattr((vnode_t)psNode, pvOutBuf, iBufSize, iActualSize);

    return iErr;
}

int
LFHFS_StreamLookup ( UVFSFileNode psFileNode, UVFSStreamNode *ppsOutNode )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_StreamLookup\n");
    VERIFY_NODE_IS_VALID(psFileNode);
    
    vnode_t psVnode = (vnode_t)psFileNode;
    vnode_t psRscVnode = NULL;
    
    if (IS_DIR(psVnode)) {
        return EISDIR;
    }
    
    int iError = hfs_vgetrsrc(psVnode, &psRscVnode);
    
    if (!iError)
        hfs_unlock (VTOC(psRscVnode));
    
    *ppsOutNode = (UVFSStreamNode) psRscVnode;
    
    return iError;
}

int
LFHFS_StreamReclaim (UVFSStreamNode psStreamNode )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_StreamReclaim\n");
    
    int iError = 0;
    vnode_t psVnode = (vnode_t) psStreamNode;

    if ( psVnode != NULL )
    {
        VERIFY_NODE_IS_VALID_FOR_RECLAIM(psVnode);
        
        iError = hfs_vnop_reclaim(psVnode);
        psVnode = NULL;
    }
    
    return iError;
}

int
LFHFS_StreamRead (UVFSStreamNode psStreamNode, uint64_t uOffset, size_t iLength, void *pvBuf, size_t *iActuallyRead )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_StreamRead  (psNode %p, uOffset %llu, iLength %lu)\n", psStreamNode, uOffset, iLength);
    VERIFY_NODE_IS_VALID(psStreamNode);
    
    struct vnode *vp = (vnode_t)psStreamNode;
    struct cnode *cp;
    struct filefork *fp;
    uint64_t filesize;
    int retval = 0;
    int took_truncate_lock = 0;
    *iActuallyRead = 0;
    
    /* Preflight checks */
    if (!vnode_isreg(vp)) {
        /* can only read regular files */
        return ( vnode_isdir(vp) ? EISDIR : EPERM );
    }
    
    cp = VTOC(vp);
    fp = VTOF(vp);
    
    /* Protect against a size change. */
    hfs_lock_truncate(cp, HFS_SHARED_LOCK, HFS_LOCK_DEFAULT);
    took_truncate_lock = 1;
    
    filesize = fp->ff_size;
    /*
     * Check the file size. Note that per POSIX spec, we return 0 at
     * file EOF, so attempting a read at an offset that is too big
     * should just return 0 on HFS+. Since the return value was initialized
     * to 0 above, we just jump to exit.  HFS Standard has its own behavior.
     */
    if (uOffset > filesize)
    {
        LFHFS_LOG( LEVEL_ERROR, "LFHFS_Read: wanted offset is greater then file size\n" );
        goto exit;
    }
    
    // If we asked to read above the file size, adjust the read size;
    if ( uOffset + iLength > filesize )
    {
        iLength = filesize - uOffset;
    }
    
    uint64_t uReadStartCluster;
    retval = raw_readwrite_read( vp, uOffset, pvBuf, iLength, iActuallyRead, &uReadStartCluster );
    
    cp->c_touch_acctime = TRUE;
    
exit:
    if (took_truncate_lock)
    {
        hfs_unlock_truncate(cp, HFS_LOCK_DEFAULT);
    }
    return retval;
}

