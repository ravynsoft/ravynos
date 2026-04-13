/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_fsops_handler.c
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 31/12/2017.
*/

#include <sys/attr.h>
#include "lf_hfs.h"
#include "lf_hfs_fsops_handler.h"
#include "lf_hfs_dirops_handler.h"
#include "lf_hfs_fileops_handler.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_cnode.h"
#include "lf_hfs_vnode.h"
#include "lf_hfs_endian.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_generic_buf.h"
#include "lf_hfs_raw_read_write.h"
#include "lf_hfs_journal.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_mount.h"
#include "lf_hfs_readwrite_ops.h"

#include "lf_hfs_vnops.h"

static int
FSOPS_GetRootVnode(struct vnode* psDevVnode, struct vnode** ppsRootVnode)
{
    return (hfs_vfs_root(psDevVnode->sFSParams.vnfs_mp, ppsRootVnode));
}

//---------------------------------- API Implementation ------------------------------------------

uint64_t FSOPS_GetOffsetFromClusterNum(vnode_t vp, uint64_t uClusterNum)
{
    return (HFSTOVCB(vp->sFSParams.vnfs_mp->psHfsmount)->hfsPlusIOPosOffset + uClusterNum * HFSTOVCB(vp->sFSParams.vnfs_mp->psHfsmount)->blockSize);
}

int
LFHFS_Taste ( int iFd )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Taste %d\n", iFd);

    int iError = 0;
    u_int32_t log_blksize;
    void* pvBuffer = NULL;

    HFSMasterDirectoryBlock *psMasterBlock = hfs_malloc(kMDBSize);
    if ( psMasterBlock == NULL )
    {
        iError = ENOMEM;
        LFHFS_LOG(LEVEL_ERROR, "HFS_Taste: failed to malloc psMasterBlock\n");
        goto exit;
    }
    
    /* Get the logical block size (treated as physical block size everywhere) */
    if (ioctl(iFd, DKIOCGETBLOCKSIZE, &log_blksize))
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: DKIOCGETBLOCKSIZE failed - setting to default -512\n");
        log_blksize = kMDBSize;
    }
    
    if (log_blksize == 0 || log_blksize > 1024*1024*1024)
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_mountfs: logical block size 0x%x looks bad.  Not mounting.\n", log_blksize);
        iError = ENXIO;
        goto exit;
    }

    if (log_blksize > kMDBSize)
    {
        pvBuffer = hfs_malloc(log_blksize);
        if ( pvBuffer == NULL )
        {
            iError = ENOMEM;
            LFHFS_LOG(LEVEL_ERROR, "HFS_Taste: failed to malloc pvBuffer\n");
            goto exit;
        }
    }
    else
    {
        pvBuffer = (void*) psMasterBlock;
    }
    
    // Read VolumeHeader from offset 1024
    off_t   uVolHdrOffset  = 1024;
    off_t   uBlockNum      = uVolHdrOffset / log_blksize;
    off_t   uOffsetInBlock = uVolHdrOffset % log_blksize;

    ssize_t iReadBytes = pread(iFd, pvBuffer, log_blksize, uBlockNum * log_blksize);
    if ( iReadBytes < uOffsetInBlock + kMDBSize ) {
        iError = (iReadBytes < 0) ? errno : EIO;
        LFHFS_LOG(LEVEL_ERROR, "HFS_Taste: failed to read Master Directory Block with err %d (%ld)\n", iError, iReadBytes);
        
        if (log_blksize > kMDBSize) {
            hfs_free(pvBuffer);
        }
        goto exit;
    }

    if (log_blksize > kMDBSize) {
        memcpy(psMasterBlock, pvBuffer + uOffsetInBlock, kMDBSize);
        hfs_free(pvBuffer);
    }
    
    //Validate Signiture
    uint32_t drSigWord = SWAP_BE16(psMasterBlock->drSigWord);
    if ((drSigWord != kHFSPlusSigWord) &&
        (drSigWord != kHFSXSigWord))
    {
        iError = EINVAL;
        LFHFS_LOG(LEVEL_DEBUG, "HFS_Taste: invalid volume signature %d\n", SWAP_BE16(psMasterBlock->drSigWord));
        goto exit;
    }

exit:
    if (psMasterBlock)
        hfs_free(psMasterBlock);
    return iError;
}

int
LFHFS_ScanVols (int iFd, UVFSScanVolsRequest *psRequest, UVFSScanVolsReply *psReply )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_ScanVols\n");
    
    if ( psRequest == NULL || psReply == NULL )
    {
        return EINVAL;
    }
    else if (psRequest->sr_volid > 0)
    {
        return UVFS_SCANVOLS_EOF_REACHED;
    }

    // Tell UVFS that we have a single, non-access controlled volume.
    psReply->sr_volid = 0;
    psReply->sr_volac = UAC_UNLOCKED;

    return hfs_ScanVolGetVolName(iFd, psReply->sr_volname);
}

int
LFHFS_Init ( void )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Init\n");

    int iErr = 0;

    iErr = LFHFS_LoggerInit();
    if ( iErr != 0 )
    {
        goto exit;
    }

    iErr = raw_readwrite_zero_fill_init();
    if ( iErr != 0 )
    {
        goto exit;
    }

    hfs_chashinit();

    // Initializing Buffer cache
    lf_hfs_generic_buf_cache_init();

    BTReserveSetup();

    journal_init();

exit:
    return iErr;
}

void
LFHFS_Fini ( void )
{
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Fini\n");

    raw_readwrite_zero_fill_de_init();

    // De-Initializing Buffer cache
    lf_hfs_generic_buf_cache_deinit();
}

int
LFHFS_Mount ( int iFd, UVFSVolumeId puVolId, UVFSMountFlags puMountFlags,
    __unused UVFSVolumeCredential *psVolumeCreds, UVFSFileNode *ppsRootNode )
{
    LFHFS_LOG(LEVEL_DEBUG, "HFS_Mount %d\n", iFd);
    int iError = 0;

    struct mount* psMount            = hfs_mallocz(sizeof(struct mount));
    struct vnode* psDevVnode         = hfs_mallocz(sizeof(struct vnode));
    struct cnode* psDevCnode         = hfs_mallocz(sizeof(struct cnode));
    struct filefork* psDevFileFork   = hfs_mallocz(sizeof(struct filefork));
    FileSystemRecord_s *psFSRecord   = hfs_mallocz(sizeof(FileSystemRecord_s));

    if ( psMount == NULL || psDevVnode == NULL || psDevCnode == NULL || psDevFileFork == NULL || psFSRecord == NULL )
    {
        iError = ENOMEM;
        LFHFS_LOG(LEVEL_ERROR, "HFS_Mount: failed to malloc initial system files\n");
        goto fail;
    }

    if (puVolId != 0)
    {
        iError = EINVAL;
        LFHFS_LOG(LEVEL_ERROR, "HFS_Mount: unknown volume ID\n");
        goto fail;
    }

    psFSRecord->iFD             = iFd;
    psDevVnode->psFSRecord      = psFSRecord;
    psDevVnode->sFSParams.vnfs_marksystem = 1;
    psDevVnode->bIsMountVnode   = true;

    // Initializing inputs for hfs_mount
    psDevFileFork->ff_data.cf_blocks                = 3;
    psDevFileFork->ff_data.cf_extents[0].blockCount = 1;
    psDevFileFork->ff_data.cf_extents[0].startBlock = 0;

    psDevVnode->sFSParams.vnfs_fsnode   = psDevCnode;
    psDevCnode->c_vp                    = psDevVnode;
    psDevVnode->is_rsrc                 = false;
    psDevCnode->c_datafork              = psDevFileFork;
    psDevVnode->sFSParams.vnfs_mp       = psMount;

    psMount->mnt_flag = (puMountFlags == UVFS_MOUNT_RDONLY)? MNT_RDONLY : 0;
    // Calling to kext hfs_mount
    iError = hfs_mount(psMount, psDevVnode, 0);
    if (iError)
        goto fail;

    struct vnode* psRootVnode;
    // Creating root vnode
    iError = FSOPS_GetRootVnode(psDevVnode,&psRootVnode);
    if (iError)
        goto fail;
    *ppsRootNode = (UVFSFileNode) psRootVnode;

    goto end;

fail:
    if (psFSRecord)
        hfs_free(psFSRecord);
    if (psMount)
        hfs_free(psMount);
    if (psDevVnode)
        hfs_free(psDevVnode);
    if (psDevCnode)
        hfs_free(psDevCnode);
    if (psDevFileFork)
        hfs_free(psDevFileFork);
end:
    return iError;
}

int
LFHFS_Unmount ( UVFSFileNode psRootNode, UVFSUnmountHint hint )
{
    VERIFY_NODE_IS_VALID(psRootNode);
    LFHFS_LOG(LEVEL_DEBUG, "HFS_Unmount (psRootNode %p) (hint %u)\n", psRootNode, hint);
    
    int iError = 0;
    struct vnode       *psRootVnode = (struct vnode*) psRootNode;
    FileSystemRecord_s *psFSRecord  = VPTOFSRECORD(psRootVnode);
    struct mount       *psMount     = psRootVnode->sFSParams.vnfs_mp;
    struct cnode       *psDevCnode  = VTOHFS(psRootVnode)->hfs_devvp->sFSParams.vnfs_fsnode;
    struct hfsmount    *psHfsMp     = psMount->psHfsmount;
    psFSRecord->uUnmountHint        = hint;

    #if HFS_CRASH_TEST
        CRASH_ABORT(CRASH_ABORT_ON_UNMOUNT, psHfsMp, NULL);
    #endif
    
    hfs_vnop_reclaim(psRootVnode);

    if (!psHfsMp->jnl) {
        hfs_flushvolumeheader(psHfsMp, HFS_FVH_SKIP_TRANSACTION | HFS_FVH_MARK_UNMOUNT);
    }

    hfs_unmount(psMount);

    hfs_free(psFSRecord);
    hfs_free(psMount);
    hfs_free(psDevCnode->c_datafork);
    hfs_free(psDevCnode);

    return iError;
}

int
LFHFS_SetFSAttr ( UVFSFileNode psNode, const char *pcAttr, const UVFSFSAttributeValue *psAttrVal, size_t uLen, UVFSFSAttributeValue *psOutAttrVal, size_t uOutLen )
{
#pragma unused (psNode, pcAttr, psAttrVal, uLen)
    VERIFY_NODE_IS_VALID(psNode);

    if (pcAttr == NULL || psAttrVal == NULL || psOutAttrVal == NULL) return EINVAL;

    if (strcmp(pcAttr, LI_FSATTR_PREALLOCATE) == 0)
    {
         if (uLen < sizeof (UVFSFSAttributeValue) || uOutLen < sizeof (UVFSFSAttributeValue))
             return EINVAL;

         LIFilePreallocateArgs_t* psPreAllocReq = (LIFilePreallocateArgs_t *) ((void *) psAttrVal->fsa_opaque);
         LIFilePreallocateArgs_t* psPreAllocRes = (LIFilePreallocateArgs_t *) ((void *) psOutAttrVal->fsa_opaque);

         memcpy (psPreAllocRes, psPreAllocReq, sizeof(LIFilePreallocateArgs_t));
         return hfs_vnop_preallocate(psNode, psPreAllocReq, psPreAllocRes);
    }

    return ENOTSUP;
}

int
LFHFS_GetFSAttr ( UVFSFileNode psNode, const char *pcAttr, UVFSFSAttributeValue *psAttrVal, size_t uLen, size_t *puRetLen )
{
    VERIFY_NODE_IS_VALID(psNode);
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_GetFSAttr (psNode %p)\n", psNode);

    int iError = 0;
    vnode_t psVnode = (vnode_t)psNode;
    struct hfsmount *psMount = psVnode->sFSParams.vnfs_mp->psHfsmount;

    if (strcmp(pcAttr, UVFS_FSATTR_PC_LINK_MAX)==0)
    {
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }

        if ( vnode_isreg(psVnode) )
        {
            psAttrVal->fsa_number = HFS_LINK_MAX;
        }
        else
        {
            psAttrVal->fsa_number = 1;
        }
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_PC_NAME_MAX)==0)
    {
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = MAXPATHLEN;
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_PC_NO_TRUNC)==0)
    {
        *puRetLen = sizeof(bool);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_bool = true;
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_PC_FILESIZEBITS)==0)
    {
        // The number of bits used to represent the size (in bytes) of a file
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = 64;
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_PC_XATTR_SIZE_BITS)==0)
    {
        // The number of bits used to represent the size (in bytes) of an extended attribute.
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = HFS_XATTR_SIZE_BITS;
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_BLOCKSIZE)==0)
    {
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = psMount->blockSize;
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_IOSIZE)==0)
    {
        // Size (in bytes) of the optimal transfer block size
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = 1024*1024*128;
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_TOTALBLOCKS)==0)
    {
        // Total number of file system blocks
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = psMount->totalBlocks;
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_BLOCKSFREE)==0)
    {
        // Total number of free file system blocks
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = hfs_freeblks( psMount, 0 );
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_BLOCKSAVAIL)==0)
    {
        // Total number of free file system blocks available for allocation to files (in our case - the same as UVFS_FSATTR_BLOCKSFREE)
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = hfs_freeblks( psMount, 1 );
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_BLOCKSUSED)==0)
    {
        // Number of file system blocks currently allocated for some use (TOTAL_BLOCKS - BLOCKSAVAIL)
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = psMount->totalBlocks - hfs_freeblks( psMount, 1 );
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_CNAME)==0)
    {
        char* pcName;
        //The file name
        if (IS_ROOT(psVnode))
            pcName = "";
        else
            pcName = (char*) psVnode->sFSParams.vnfs_cnp->cn_nameptr;

        if (pcName ==  NULL)
            return EINVAL;

        *puRetLen = strlen(pcName) + 1;
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        strlcpy(psAttrVal->fsa_string, pcName, *puRetLen);
        return 0;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_FSTYPENAME)==0)
    {
        *puRetLen = 4;
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        // A string representing the type of file system
        size_t n = strlcpy(psAttrVal->fsa_string, "HFS", *puRetLen);
        if (n >= *puRetLen)
        {
             *(psAttrVal->fsa_string + (*puRetLen - 1)) = '\0'; // Must be null terminated
        }
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_FSSUBTYPE)==0)
    {
#define HFS_PLUS_STR                            "HFS Plus"
#define HFS_PLUS_JOURNALED_STR                  "HFS Plus (Journaled)"
#define HFS_PLUS_CASE_SENS_STR                  "HFS Plus (Case Sensitive)"
#define HFS_PLUS_CASE_SENS_JOURNALED_STR        "HFS Plus (Case Sensitive, Journaled)"

        char* pcFSSubType = HFS_PLUS_STR;
        if ( (psMount->hfs_flags & HFS_CASE_SENSITIVE) && psMount->jnl )
        {
            pcFSSubType = HFS_PLUS_CASE_SENS_JOURNALED_STR;
        }
        else if ( psMount->hfs_flags & HFS_CASE_SENSITIVE )
        {
            pcFSSubType = HFS_PLUS_CASE_SENS_STR;
        }
        else if ( psMount->jnl )
        {
            pcFSSubType = HFS_PLUS_JOURNALED_STR;
        }

        *puRetLen = strlen( pcFSSubType ) + 1;
        if ( uLen < *puRetLen )
        {
            return E2BIG;
        }

        strlcpy( psAttrVal->fsa_string, pcFSSubType, *puRetLen);
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_VOLNAME)==0)
    {
        *puRetLen = strlen((char *)psMount->vcbVN)+1; // Add 1 for the NULL terminator
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        strlcpy(psAttrVal->fsa_string, (char *)psMount->vcbVN, *puRetLen);
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_VOLUUID)==0)
    {
        *puRetLen = sizeof(uuid_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        hfs_getvoluuid( psMount, psAttrVal->fsa_opaque );
        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_CAPS_FORMAT)==0)
    {
        // A bitmask indicating the capabilities of the volume format
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }

        psAttrVal->fsa_number =
            VOL_CAP_FMT_PERSISTENTOBJECTIDS     |
            VOL_CAP_FMT_SYMBOLICLINKS           |
            VOL_CAP_FMT_HARDLINKS               |
            VOL_CAP_FMT_JOURNAL                 |
            (psMount->jnl ? VOL_CAP_FMT_JOURNAL_ACTIVE : 0)                             |
            (psMount->hfs_flags & HFS_CASE_SENSITIVE ? VOL_CAP_FMT_CASE_SENSITIVE : 0)  |
            VOL_CAP_FMT_CASE_PRESERVING         |
            VOL_CAP_FMT_2TB_FILESIZE            |
            VOL_CAP_FMT_HIDDEN_FILES            |
            /* XXX rdar://problem/48128963 VOL_CAP_FMT_PATH_FROM_ID */ 0;

        goto end;
    }

    if (strcmp(pcAttr, UVFS_FSATTR_CAPS_INTERFACES)==0)
    {
        // A bitmask indicating the interface capabilities of the file system
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }

        psAttrVal->fsa_number =
#if LF_HFS_NATIVE_SEARCHFS_SUPPORT
            VOL_CAP_INT_SEARCHFS |
#endif
            VOL_CAP_INT_EXTENDED_ATTR;

        goto end;
    }
    
    if (strcmp(pcAttr, UVFS_FSATTR_LAST_MTIME)==0)
    {
        // system lsat mounted time
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = psMount->hfs_last_mounted_mtime;
        goto end;
    }
    
    if (strcmp(pcAttr, UVFS_FSATTR_MOUNT_TIME)==0)
    {
        // system mount time
        *puRetLen = sizeof(uint64_t);
        if (uLen < *puRetLen)
        {
            return E2BIG;
        }
        psAttrVal->fsa_number = psMount->hfs_mount_time;
        goto end;
    }

    iError = ENOTSUP;
end:
    return iError;
}

// kHFSVolumeUnmountedMask: this bit is used to indicate whether the volume is dirty (for which fsck needs to run prior to mount) or clean.
// For non-journaled volumes:
// - Each operation that causes metadata modification clears this bit.
// - A Sync operation that takes place after all 'dirtying' operations are completed sets this bit.
// Syncronization between the 'dirtying' operations and the Sync is performed by the hfs_global_lock().
// For journaled volumes, the volume is considered clean after a journal has been committed to the media.
int LFHFS_Sync(UVFSFileNode psNode) {
    VERIFY_NODE_IS_VALID(psNode);
    LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Sync (psNode %p)\n", psNode);
    
    int iErr = 0;
    vnode_t psVnode = (vnode_t)psNode;
    struct hfsmount *psMount = psVnode->sFSParams.vnfs_mp->psHfsmount;
    bool bNeedUnlock = false;

    lf_lck_mtx_lock(&psMount->sync_mutex);
    psMount->hfs_syncer_thread = pthread_self();
    
    if (psMount->jnl) {
        
        hfs_flush(psMount, HFS_FLUSH_JOURNAL_META);

    } else {

        if (psMount->hfs_global_lockowner != pthread_self()) {
            hfs_lock_global(psMount, HFS_EXCLUSIVE_LOCK);
            bNeedUnlock = true;
        }

        hfs_flushvolumeheader(psMount, HFS_FVH_SKIP_TRANSACTION | HFS_FVH_MARK_UNMOUNT);
        
        if (bNeedUnlock) {
            hfs_unlock_global(psMount);
        }
    }

    psMount->hfs_syncer_thread = NULL;
    lf_lck_mtx_unlock(&psMount->sync_mutex);
    
    return(iErr);
}

int
LFHFS_Check( int fdToCheck , __unused UVFSVolumeId volId,
    __unused UVFSVolumeCredential *volumeCreds, check_flags_t how )
{
    return fsck_hfs(fdToCheck, how);
}

UVFSFSOps HFS_fsOps = {
    .fsops_version      = UVFS_FSOPS_VERSION_CURRENT,

    .fsops_init         = LFHFS_Init,
    .fsops_fini         = LFHFS_Fini,

    .fsops_taste        = LFHFS_Taste,
    .fsops_scanvols     = LFHFS_ScanVols,
    .fsops_mount        = LFHFS_Mount,
    .fsops_sync         = LFHFS_Sync,
    .fsops_unmount      = LFHFS_Unmount,

    .fsops_getfsattr    = LFHFS_GetFSAttr,
    .fsops_setfsattr    = LFHFS_SetFSAttr,

    .fsops_getattr      = LFHFS_GetAttr,
    .fsops_setattr      = LFHFS_SetAttr,
    .fsops_lookup       = LFHFS_Lookup,
    .fsops_reclaim      = LFHFS_Reclaim,
    .fsops_readlink     = LFHFS_ReadLink,
    .fsops_read         = LFHFS_Read,
    .fsops_write        = LFHFS_Write,
    .fsops_create       = LFHFS_Create,
    .fsops_mkdir        = LFHFS_MkDir,
    .fsops_symlink      = LFHFS_SymLink,
    .fsops_remove       = LFHFS_Remove,
    .fsops_rmdir        = LFHFS_RmDir,
    .fsops_rename       = LFHFS_Rename,
    .fsops_readdir      = LFHFS_ReadDir,
    .fsops_readdirattr  = LFHFS_ReadDirAttr,
    .fsops_link         = LFHFS_Link,
    .fsops_check        = LFHFS_Check,

    .fsops_getxattr     = LFHFS_GetXAttr,
    .fsops_setxattr     = LFHFS_SetXAttr,
    .fsops_listxattr    = LFHFS_ListXAttr,

    .fsops_scandir      = LFHFS_ScanDir,
    .fsops_scanids      = LFHFS_ScanIDs,
    
    .fsops_stream_lookup = LFHFS_StreamLookup,
    .fsops_stream_reclaim = LFHFS_StreamReclaim,
    .fsops_stream_read = LFHFS_StreamRead,
};

#if HFS_CRASH_TEST
CrashAbortFunction_FP gpsCrashAbortFunctionArray[CRASH_ABORT_LAST] = {0};
#endif

__attribute__((visibility("default")))
void
livefiles_plugin_init(UVFSFSOps **ops)
{
    if (ops) {
        *ops = &HFS_fsOps;
    }

    return;
}
