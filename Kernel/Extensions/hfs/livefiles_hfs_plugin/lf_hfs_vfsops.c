/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_vfsops.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 18/3/18.
 */

#include "lf_hfs_common.h"
#include <CommonCrypto/CommonDigest.h>
#include <stdatomic.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/disk.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "lf_hfs_logger.h"
#include "lf_hfs_mount.h"
#include "lf_hfs.h"
#include "lf_hfs_catalog.h"
#include "lf_hfs_cnode.h"
#include "lf_hfs_chash.h"
#include "lf_hfs_format.h"
#include "lf_hfs_locks.h"
#include "lf_hfs_endian.h"
#include "lf_hfs_locks.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_raw_read_write.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_volume_allocation.h"
#include "lf_hfs_catalog.h"
#include "lf_hfs_link.h"
#include "lf_hfs_vnops.h"
#include "lf_hfs_generic_buf.h"
#include "lf_hfs_fsops_handler.h"
#include "lf_hfs_journal.h"
#include "lf_hfs_fileops_handler.h"

#include <spawn.h>

static void hfs_locks_destroy(struct hfsmount *hfsmp);
static int  hfs_mountfs(struct vnode *devvp, struct mount *mp, struct hfs_mount_args *args);


static int
setup_posix_file_action_for_fsck(posix_spawn_file_actions_t *file_actions, int fd)
{
    int error;

    if (file_actions == NULL || fd < 0)
    {
        return EINVAL;
    }

    error = posix_spawn_file_actions_init(file_actions);
    if (error)
    {
        goto out;
    }

    error = posix_spawn_file_actions_addinherit_np(file_actions, 0);
    if (error)
    {
        goto out;
    }

    error = posix_spawn_file_actions_addinherit_np(file_actions, 1);
    if (error)
    {
        goto out;
    }

    error = posix_spawn_file_actions_addinherit_np(file_actions, 2);
    if (error)
    {
        goto out;
    }

    error = posix_spawn_file_actions_addinherit_np(file_actions, fd);

out:
    return error;
}

static int
setup_spawnattr_for_fsck(posix_spawnattr_t *spawn_attr)
{
    int error;

    error = posix_spawnattr_init(spawn_attr);
    if (error)
    {
        goto out;
    }
    error = posix_spawnattr_setflags(spawn_attr, POSIX_SPAWN_CLOEXEC_DEFAULT);

out:
    return error;
}


// fsck_mount_and_replay: executed on fsck_hfs -quick
// Try to mount, and if a journaled volume, play the journal.
// Returned values:
// OK if:
// 1) On journaled volumes, the journal has been replayed and the dirty bit cleared.
// 2) On non-journalled volumes, the dirty is cleared.
// EINVAL if:
// 1) On non-journalled volumes the dirty bit is set. Please run fsck_hfs to fix.
// 2) On journalled volume, the replay failed. Try fsck_hfs.
int fsck_mount_and_replay(int iFd) {
    int iErr = 0;
    
    LFHFS_LOG(LEVEL_DEBUG, "fsck_mount_and_replay %d", iFd);
    
    UVFSFileNode sRootNode;

    iErr = LFHFS_Taste(iFd);
    if (iErr) {
        LFHFS_LOG(LEVEL_DEBUG, "LFHFS_Taste returned %d", iErr);
        return iErr;
    }

    UVFSScanVolsRequest sScanVolsReq = {0};
    UVFSScanVolsReply sScanVolsReply = {0};
    iErr = LFHFS_ScanVols(iFd, &sScanVolsReq, &sScanVolsReply);
    if (iErr) {
        LFHFS_LOG(LEVEL_DEBUG, "LFHFS_ScanVol returned %d", iErr);
        return iErr;
    }

    // Mount and replay journal if possible
    iErr = LFHFS_Mount(iFd, 0, 0, NULL, &sRootNode); // On journaled volumes, this replays the journal.
                                         // Non-journaled volumes fails to mount if dirty (Unmounted == 0).
    if (iErr) {
        LFHFS_LOG(LEVEL_DEBUG, "fsck_mount_and_replay: LFHFS_Mount returned %d", iErr);
        return EINVAL;
    }
    
    LFHFS_Unmount (sRootNode, UVFSUnmountHintNone);

    return iErr;
}

#define PATH_TO_FSCK "/System/Library/Filesystems/hfs.fs/Contents/Resources/fsck_hfs"

int
fsck_hfs(int fd, check_flags_t how)
{
    pid_t child;
    pid_t child_found;
    int child_status;
    extern char **environ;
    char fdescfs_path[24];
    posix_spawn_file_actions_t file_actions;
    int result;
    posix_spawnattr_t spawn_attr;

    /*
     * XXXJRT There are dragons related to how the journal is replayed in
     * fsck_hfs.  Until we can sort out the mess, disable running fsck_hfs.
     * <rdar://problem/47262605> USB: Re-enable Detonator fsck_hfs
     */
    if (how == QUICK_CHECK) {
        if (fsck_mount_and_replay(fd) == 0) {
            return(0);
        }
    }

    LFHFS_LOG(LEVEL_DEFAULT, "fsck_hfs - fsck start for %d", fd);
    snprintf(fdescfs_path, sizeof(fdescfs_path), "/dev/fd/%d", fd);
    const char * argv[] = {"fsck_hfs", "-q", fdescfs_path, NULL};

    switch (how)
    {
        case QUICK_CHECK:
            /* Do nothing, already setup for this */
            break;
        case CHECK:
            argv[1] = "-n";
            break;
        case CHECK_AND_REPAIR:
            argv[1] = "-y";
            break;
        default:
            LFHFS_LOG(LEVEL_ERROR, "Invalid how flags for the check, ignoring; %d", how);
            break;
    }

    LFHFS_LOG(LEVEL_DEBUG, "fsck_hfs params: %s %s %s", argv[1], argv[2], argv[3]);
    result = setup_posix_file_action_for_fsck(&file_actions, fd);
    if (result)
    {
        goto out;
    }

    result = setup_spawnattr_for_fsck(&spawn_attr);
    if (result)
    {
        posix_spawn_file_actions_destroy(&file_actions);
        goto out;
    }

    result = posix_spawn(&child,
                         PATH_TO_FSCK,
                         &file_actions,
                         &spawn_attr,
                         (char * const *)argv,
                         environ);
    
    posix_spawn_file_actions_destroy(&file_actions);
    posix_spawnattr_destroy(&spawn_attr);
    if (result)
    {
        LFHFS_LOG(LEVEL_ERROR, "posix_spawn fsck_hfs: error=%d", result);
        goto out;
    }

    // Wait for child to finish, XXXab: revisit, need sensible timeout?
    do {
        child_found = waitpid(child, &child_status, 0);
    } while (child_found == -1 && errno == EINTR);

    if (child_found == -1)
    {
        result = errno;
        LFHFS_LOG(LEVEL_ERROR, "waitpid fsck_hfs: errno=%d", result);
        goto out;
    }

    if (WIFEXITED(child_status))
    {
        result = WEXITSTATUS(child_status);
        if (result)
        {
            LFHFS_LOG(LEVEL_ERROR, "fsck_hfs: exited with status %d", result);
            result = EILSEQ;
        } else {
            LFHFS_LOG(LEVEL_ERROR, "fsck_hfs: exited with status %d", result);
        }
    }
    else
    {
        result = WTERMSIG(child_status);
        LFHFS_LOG(LEVEL_ERROR, "fsck_hfs: terminated by signal %d", result);
        result = EINTR;
    }

out:
    LFHFS_LOG(LEVEL_DEFAULT, "fsck_hfs - fsck finish for %d with err %d", fd, result);
    return result;
}

int
hfs_mount(struct mount *mp, vnode_t devvp, user_addr_t data)
{
    struct hfsmount *hfsmp  = NULL;
    int retval              = 0;
    if ( devvp == NULL )
    {
        retval = EINVAL;
        goto fail;
    }

    retval = hfs_mountfs(devvp, mp, NULL);
    if (retval)
    {
        // ENOTSUP is for regular HFS -> just fail
        if (retval != ENOTSUP)
        {
            //Failed during mount, try to run fsck to fix and try mount again
            retval = fsck_hfs(devvp->psFSRecord->iFD, CHECK_AND_REPAIR);

            // fsck succeeded, try to mount
            if (!retval) {
                retval = hfs_mountfs(devvp, mp, NULL);
                if (!retval)
                    goto mount_passed;
            }
        }

        LFHFS_LOG(LEVEL_ERROR, "hfs_mount: hfs_mountfs returned error=%d\n", retval);
        goto fail;
    }
mount_passed:
    /* After hfs_mountfs succeeds, we should have valid hfsmp */
    hfsmp = VFSTOHFS(mp);

    /* Set up the maximum defrag file size */
    hfsmp->hfs_defrag_max = HFS_INITIAL_DEFRAG_SIZE;

    if (!data)
    {
        // Root mount
        hfsmp->hfs_uid          = UNKNOWNUID;
        hfsmp->hfs_gid          = UNKNOWNGID;
        hfsmp->hfs_dir_mask     = (S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH); /* 0755 */
        hfsmp->hfs_file_mask    = (S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH); /* 0755 */

        /* Establish the free block reserve. */
        hfsmp->reserveBlocks = (uint32_t) ((u_int64_t)hfsmp->totalBlocks * HFS_MINFREE) / 100;
        hfsmp->reserveBlocks = MIN(hfsmp->reserveBlocks, HFS_MAXRESERVE / hfsmp->blockSize);
    }

fail:
    return (retval);
}

static int hfs_InitialMount(struct vnode *devvp, struct mount *mp, struct hfs_mount_args *args, HFSPlusVolumeHeader **vhp, off_t *embeddedOffset, struct hfsmount **hfsmp, bool bFailForDirty)
{
    int retval                      = 0;
    HFSMasterDirectoryBlock *mdbp   = NULL;
    void* pvBuffer                  = NULL;
    int mntwrapper;
    u_int64_t disksize;
    u_int64_t log_blkcnt;
    u_int32_t log_blksize;
    u_int32_t phys_blksize;
    u_int32_t minblksize;
    u_int32_t iswritable;
    u_int64_t mdb_offset;
    u_int32_t device_features = 0;

    mntwrapper = 0;
    minblksize = kHFSBlockSize;
    *hfsmp = NULL;

    /* Get the logical block size (treated as physical block size everywhere) */
    if (ioctl(devvp->psFSRecord->iFD, DKIOCGETBLOCKSIZE, &log_blksize))
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: DKIOCGETBLOCKSIZE failed\n");
        retval = ENXIO;
        goto error_exit;
    }
    if (log_blksize == 0 || log_blksize > 1024*1024*1024)
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_mountfs: logical block size 0x%x looks bad.  Not mounting.\n", log_blksize);
        retval = ENXIO;
        goto error_exit;
    }

    /* Get the physical block size. */
    if (ioctl(devvp->psFSRecord->iFD, DKIOCGETPHYSICALBLOCKSIZE, &phys_blksize))
    {
        if ((retval != ENOTSUP) && (retval != ENOTTY))
        {
            LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: DKIOCGETPHYSICALBLOCKSIZE failed\n");
            retval = ENXIO;
            goto error_exit;
        }
        /* If device does not support this ioctl, assume that physical
         * block size is same as logical block size
         */
        phys_blksize = log_blksize;
    }

    if (phys_blksize == 0 || phys_blksize > MAXBSIZE)
    {
        LFHFS_LOG(LEVEL_ERROR, "hfs_mountfs: physical block size 0x%x looks bad.  Not mounting.\n", phys_blksize);
        retval = ENXIO;
        goto error_exit;
    }

	/* Don't let phys_blksize be smaller than the logical  */
	if (phys_blksize < log_blksize) {
		/*
		 * In the off chance that the phys_blksize is SMALLER than the logical
		 * then don't let that happen.  Pretend that the PHYSICALBLOCKSIZE
		 * ioctl was not supported.
		 */
		 phys_blksize = log_blksize;
	}

    /* Get the number of physical blocks. */
    if (ioctl(devvp->psFSRecord->iFD, DKIOCGETBLOCKCOUNT, &log_blkcnt))
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: DKIOCGETBLOCKCOUNT failed\n");
        retval = ENXIO;
        goto error_exit;
    }

    /* Compute an accurate disk size (i.e. within 512 bytes) */
    disksize = (u_int64_t)log_blkcnt * (u_int64_t)log_blksize;

    /*
     * At this point:
     *   minblksize is the minimum physical block size
     *   log_blksize has our preferred physical block size
     *   log_blkcnt has the total number of physical blocks
     */
    mdbp = hfs_mallocz(kMDBSize);
    if (mdbp == NULL)
    {
        retval = ENOMEM;
        goto error_exit;
    }

    pvBuffer = hfs_malloc(phys_blksize);
    if (pvBuffer == NULL)
    {
        retval = ENOMEM;
        goto error_exit;
    }

    mdb_offset = (uint64_t) HFS_PRI_SECTOR(log_blksize);
    retval = raw_readwrite_read_mount( devvp, HFS_PHYSBLK_ROUNDDOWN(mdb_offset, (phys_blksize/log_blksize)), phys_blksize, pvBuffer, phys_blksize, NULL, NULL);
    if (retval)
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: raw_readwrite_read_mount failed with %d\n", retval);
        goto error_exit;
    }

    bcopy(pvBuffer + HFS_PRI_OFFSET(phys_blksize), mdbp, kMDBSize);
    hfs_free(pvBuffer);
    pvBuffer = NULL;

    *hfsmp = hfs_malloc(sizeof(struct hfsmount));
    if (*hfsmp == NULL)
    {
        retval = ENOMEM;
        goto error_exit;
    }
    memset( *hfsmp, 0, sizeof(struct hfsmount) );

    //Copy read only flag
    if (mp->mnt_flag == MNT_RDONLY) (*hfsmp)->hfs_flags = HFS_READ_ONLY;

    hfs_chashinit_finish(*hfsmp);

    /* Init the ID lookup hashtable */
    hfs_idhash_init (*hfsmp);

    /*
     * See if the disk supports unmap (trim).
     *
     * NOTE: vfs_init_io_attributes has not been called yet, so we can't use the io_flags field
     * returned by vfs_ioattr.  We need to call VNOP_IOCTL ourselves.
     */
    if (ioctl(devvp->psFSRecord->iFD, DKIOCGETFEATURES, &device_features) == 0)
    {
        if (device_features & DK_FEATURE_UNMAP)
        {
            (*hfsmp)->hfs_flags |= HFS_UNMAP;
        }

        if(device_features & DK_FEATURE_BARRIER)
        {
            (*hfsmp)->hfs_flags |= HFS_FEATURE_BARRIER;
        }
    }

    /*
     *  Init the volume information structure
     */
    lf_lck_mtx_init(&(*hfsmp)->hfs_mutex);
    lf_lck_mtx_init(&(*hfsmp)->sync_mutex);
    lf_lck_rw_init(&(*hfsmp)->hfs_global_lock);
    lf_lck_spin_init(&(*hfsmp)->vcbFreeExtLock);

    if (mp)
    {
        mp->psHfsmount = (*hfsmp);
    }

    (*hfsmp)->hfs_mp = mp;            /* Make VFSTOHFS work */
    (*hfsmp)->hfs_raw_dev = 0; //vnode_specrdev(devvp);
    (*hfsmp)->hfs_devvp = devvp;
    (*hfsmp)->hfs_logical_block_size = log_blksize;
    (*hfsmp)->hfs_logical_block_count = log_blkcnt;
    (*hfsmp)->hfs_logical_bytes = (uint64_t) log_blksize * (uint64_t) log_blkcnt;
    (*hfsmp)->hfs_physical_block_size = phys_blksize;
    (*hfsmp)->hfs_log_per_phys = (phys_blksize / log_blksize);
    (*hfsmp)->hfs_flags |= HFS_WRITEABLE_MEDIA;

    if (mp && (mp->mnt_flag & MNT_UNKNOWNPERMISSIONS))
    {
        (*hfsmp)->hfs_flags |= HFS_UNKNOWN_PERMS;
    }

    /* MNT_UNKNOWNPERMISSIONS requires setting up uid, gid, and mask: */
    if (mp && (mp->mnt_flag & MNT_UNKNOWNPERMISSIONS))
    {
        (*hfsmp)->hfs_uid = UNKNOWNUID;
        (*hfsmp)->hfs_gid = UNKNOWNGID;
        //        vfs_setowner(mp, hfsmp->hfs_uid, hfsmp->hfs_gid);            /* tell the VFS */
        (*hfsmp)->hfs_dir_mask = UNKNOWNPERMISSIONS & ALLPERMS;        /* 0777: rwx---rwx */
        (*hfsmp)->hfs_file_mask = UNKNOWNPERMISSIONS & DEFFILEMODE;    /* 0666: no --x by default? */
    }

    /* Find out if disk media is writable. */
    if (ioctl(devvp->psFSRecord->iFD, DKIOCISWRITABLE, &iswritable) == 0)
    {
        if (iswritable)
        {
            (*hfsmp)->hfs_flags |= HFS_WRITEABLE_MEDIA;
        }
        else
        {
            (*hfsmp)->hfs_flags &= ~HFS_WRITEABLE_MEDIA;
        }
    }

    // Reservations
    rl_init(&(*hfsmp)->hfs_reserved_ranges[0]);
    rl_init(&(*hfsmp)->hfs_reserved_ranges[1]);

    // record the current time at which we're mounting this volume
    struct timeval tv;
    microuptime(&tv);
    (*hfsmp)->hfs_mount_time = tv.tv_sec;

    /* Mount an HFS Plus disk */
    int   jnl_disable = 0;

    /* Mount a standard HFS disk */
    if ((SWAP_BE16(mdbp->drSigWord) == kHFSSigWord) && (mntwrapper || (SWAP_BE16(mdbp->drEmbedSigWord) != kHFSPlusSigWord)))
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: Not supporting standard HFS\n");
        retval = ENOTSUP;
        goto error_exit;
    }
    /* Get the embedded Volume Header */
    else if (SWAP_BE16(mdbp->drEmbedSigWord) == kHFSPlusSigWord)
    {
        *embeddedOffset = SWAP_BE16(mdbp->drAlBlSt) * kHFSBlockSize;
        *embeddedOffset += (u_int64_t)SWAP_BE16(mdbp->drEmbedExtent.startBlock) * (u_int64_t)SWAP_BE32(mdbp->drAlBlkSiz);

        /*
         * If the embedded volume doesn't start on a block
         * boundary, then switch the device to a 512-byte
         * block size so everything will line up on a block
         * boundary.
         */
        if ((*embeddedOffset % log_blksize) != 0)
        {
            // LF not support DKIOCSETBLOCKSIZE, return error.
            LFHFS_LOG(LEVEL_DEFAULT, "hfs_mountfs: embedded volume offset not a multiple of physical block size (%d); switching to 512\n", log_blksize);
            retval = ENXIO;
            goto error_exit;
        }

        disksize = (u_int64_t)SWAP_BE16(mdbp->drEmbedExtent.blockCount) * (u_int64_t)SWAP_BE32(mdbp->drAlBlkSiz);

        (*hfsmp)->hfs_logical_block_count = disksize / log_blksize;

        (*hfsmp)->hfs_logical_bytes = (uint64_t) (*hfsmp)->hfs_logical_block_count * (uint64_t) (*hfsmp)->hfs_logical_block_size;

        mdb_offset = (uint64_t)((*embeddedOffset / log_blksize) + HFS_PRI_SECTOR(log_blksize));

        pvBuffer = hfs_malloc(phys_blksize);
        if (pvBuffer == NULL)
        {
            retval = ENOMEM;
            goto error_exit;
        }

        retval = raw_readwrite_read_mount( devvp, HFS_PHYSBLK_ROUNDDOWN(mdb_offset, (phys_blksize/log_blksize)), phys_blksize, pvBuffer, phys_blksize, NULL, NULL);
        if (retval)
        {
            LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: raw_readwrite_read_mount (2) failed with %d\n", retval);
            goto error_exit;
        }

        bcopy(pvBuffer + HFS_PRI_OFFSET(phys_blksize), mdbp, kMDBSize);
        *vhp = (HFSPlusVolumeHeader*) mdbp;
        hfs_free(pvBuffer);
        pvBuffer = NULL;
    }
    else
    { /* pure HFS+ */
        *embeddedOffset = 0;
        *vhp = (HFSPlusVolumeHeader*) mdbp;
    }

    retval = hfs_ValidateHFSPlusVolumeHeader(*hfsmp, *vhp);
    if (retval)
        goto error_exit;

    /*
     * If allocation block size is less than the physical block size,
     * invalidate the buffer read in using native physical block size
     * to ensure data consistency.
     *
     * HFS Plus reserves one allocation block for the Volume Header.
     * If the physical size is larger, then when we read the volume header,
     * we will also end up reading in the next allocation block(s).
     * If those other allocation block(s) is/are modified, and then the volume
     * header is modified, the write of the volume header's buffer will write
     * out the old contents of the other allocation blocks.
     *
     * We assume that the physical block size is same as logical block size.
     * The physical block size value is used to round down the offsets for
     * reading and writing the primary and alternate volume headers.
     *
     * The same logic is also in hfs_MountHFSPlusVolume to ensure that
     * hfs_mountfs, hfs_MountHFSPlusVolume and later are doing the I/Os
     * using same block size.
     */
    if (SWAP_BE32((*vhp)->blockSize) < (*hfsmp)->hfs_physical_block_size)
    {
        phys_blksize = (*hfsmp)->hfs_logical_block_size;
        (*hfsmp)->hfs_physical_block_size = (*hfsmp)->hfs_logical_block_size;
        (*hfsmp)->hfs_log_per_phys = 1;

        if (retval)
            goto error_exit;
    }

    /*
     * On inconsistent disks, do not allow read-write mount
     * unless it is the boot volume being mounted.  We also
     * always want to replay the journal if the journal_replay_only
     * flag is set because that will (most likely) get the
     * disk into a consistent state before fsck_hfs starts
     * looking at it.
     */
    if ( (mp && !(mp->mnt_flag & MNT_ROOTFS))
        && (SWAP_BE32((*vhp)->attributes) & kHFSVolumeInconsistentMask)
        && !((*hfsmp)->hfs_flags & HFS_READ_ONLY))
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: failed to mount non-root inconsistent disk\n");
        retval = EINVAL;
        goto error_exit;
    }

    (*hfsmp)->jnl = NULL;
    (*hfsmp)->jvp = NULL;
    if (args != NULL && (args->flags & HFSFSMNT_EXTENDED_ARGS) && args->journal_disable)
    {
        jnl_disable = 1;
    }

    /*
     * We only initialize the journal here if the last person
     * to mount this volume was journaling aware.  Otherwise
     * we delay journal initialization until later at the end
     * of hfs_MountHFSPlusVolume() because the last person who
     * mounted it could have messed things up behind our back
     * (so we need to go find the .journal file, make sure it's
     * the right size, re-sync up if it was moved, etc).
     */
    uint32_t lastMountedVersion = SWAP_BE32((*vhp)->lastMountedVersion);
    uint32_t attributes         = SWAP_BE32((*vhp)->attributes);
    if (   (lastMountedVersion == kHFSJMountVersion) &&
        (attributes & kHFSVolumeJournaledMask)    &&
        !jnl_disable)
    {

        // if we're able to init the journal, mark the mount
        // point as journaled.
        if ((retval = hfs_early_journal_init(*hfsmp, *vhp, args, *embeddedOffset, mdb_offset, mdbp)) != 0)
        {
            if (retval == EROFS)
            {
                // EROFS is a special error code that means the volume has an external
                // journal which we couldn't find.  in that case we do not want to
                // rewrite the volume header - we'll just refuse to mount the volume.
                LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: hfs_early_journal_init indicated external jnl \n");
                retval = EINVAL;
                goto error_exit;
            }

            // if the journal failed to open, then set the lastMountedVersion
            // to be "FSK!" which fsck_hfs will see and force the fsck instead
            // of just bailing out because the volume is journaled.
            LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: hfs_early_journal_init failed, setting to FSK \n");
            HFSPlusVolumeHeader *jvhp;

            (*hfsmp)->hfs_flags |= HFS_NEED_JNL_RESET;

            if (mdb_offset == 0)
            {
                mdb_offset = (uint64_t)((*embeddedOffset / log_blksize) + HFS_PRI_SECTOR(log_blksize));
            }

            pvBuffer = hfs_malloc(phys_blksize);
            if (pvBuffer == NULL)
            {
                retval = ENOMEM;
                goto error_exit;
            }

            retval = raw_readwrite_read_mount( devvp, HFS_PHYSBLK_ROUNDDOWN(mdb_offset, (*hfsmp)->hfs_log_per_phys), phys_blksize, pvBuffer, phys_blksize, NULL, NULL);
            if (retval)
            {
                LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: raw_readwrite_read_mount (3) failed with %d\n", retval);
                goto error_exit;
            }

            jvhp = (HFSPlusVolumeHeader *)(pvBuffer + HFS_PRI_OFFSET(phys_blksize));

            if (SWAP_BE16(jvhp->signature) == kHFSPlusSigWord || SWAP_BE16(jvhp->signature) == kHFSXSigWord)
            {
                LFHFS_LOG(LEVEL_DEFAULT, "hfs_mountfs: Journal replay fail.  Writing lastMountVersion as FSK!\n");

                jvhp->lastMountedVersion = SWAP_BE32(kFSKMountVersion);
                retval = raw_readwrite_write_mount( devvp, HFS_PHYSBLK_ROUNDDOWN(mdb_offset, (*hfsmp)->hfs_log_per_phys), phys_blksize, pvBuffer, phys_blksize, NULL, NULL );
                if (retval)
                {
                    LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: raw_readwrite_write_mount (1) failed with %d\n", retval);
                    goto error_exit;
                }
                hfs_free(pvBuffer);
                pvBuffer = NULL;
            }

            LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: hfs_early_journal_init failed, erroring out \n");
            retval = EINVAL;
            goto error_exit;
        }
    }

    retval = hfs_MountHFSPlusVolume(*hfsmp, *vhp, *embeddedOffset, disksize, bFailForDirty);
    /*
     * If the backend didn't like our physical blocksize
     * then retry with physical blocksize of 512.
     */
    if ((retval == ENXIO) && (log_blksize > 512) && (log_blksize != minblksize))
    {
        // LF not support DKIOCSETBLOCKSIZE, return error.
        LFHFS_LOG(LEVEL_DEFAULT, "hfs_mountfs: could not use physical block size (%d).\n", log_blksize);
        goto error_exit;
    }
    else if ( retval )
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: hfs_MountHFSPlusVolume encountered failure %d \n", retval);
        goto error_exit;
    }

    return (retval);

error_exit:
    if (pvBuffer)
        hfs_free(pvBuffer);

    hfs_free(mdbp);

    if (*hfsmp)
    {
        hfs_locks_destroy(*hfsmp);
        hfs_delete_chash(*hfsmp);
        hfs_idhash_destroy (*hfsmp);

        hfs_free(*hfsmp);
        *hfsmp = NULL;
    }
    return (retval);
}


int hfs_ScanVolGetVolName(int iFd, char* pcVolumeName)
{
    int retval = 0;

    HFSPlusVolumeHeader *vhp;
    off_t embeddedOffset;
    struct hfsmount *hfsmp;
    struct mount* psMount            = hfs_mallocz(sizeof(struct mount));
    struct vnode* psDevVnode         = hfs_mallocz(sizeof(struct vnode));
    struct cnode* psDevCnode         = hfs_mallocz(sizeof(struct cnode));
    struct filefork* psDevFileFork   = hfs_mallocz(sizeof(struct filefork));
    FileSystemRecord_s *psFSRecord   = hfs_mallocz(sizeof(FileSystemRecord_s));

    if ( psMount == NULL || psDevVnode == NULL || psDevCnode == NULL || psDevFileFork == NULL || psFSRecord == NULL )
    {
        retval = ENOMEM;
        LFHFS_LOG(LEVEL_ERROR, "hfs_ScanVolGetVolName: failed to malloc initial system files\n");
        goto exit;
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

    retval = hfs_InitialMount(psDevVnode, psMount, 0, &vhp, &embeddedOffset, &hfsmp, false);

    if (retval)
    {
        goto exit;
    }
    else
    {
        strlcpy(pcVolumeName, (char*) hfsmp->vcbVN, UVFS_SCANVOLS_VOLNAME_MAX);
    }

    if (vhp) free(vhp);
    if (hfsmp)
    {
        if (hfsmp->jnl) {
            journal_release(hfsmp->jnl);
            hfsmp->jnl = NULL;
        }
        
        hfsUnmount(hfsmp);

        hfs_locks_destroy(hfsmp);
        hfs_delete_chash(hfsmp);
        hfs_idhash_destroy (hfsmp);
        
        hfs_free(hfsmp);
        hfsmp = NULL;
    }

exit:
    if (retval) {
        LFHFS_LOG(LEVEL_ERROR, "hfs_ScanVolGetVolName: failed with error %d, returning empty name and no error\n",retval);
        pcVolumeName[0] = '\0';
    }
    
    if (psMount) free (psMount);
    if (psDevVnode) free (psDevVnode);
    if (psDevCnode) free (psDevCnode);
    if (psDevFileFork) free (psDevFileFork);
    if (psFSRecord) free (psFSRecord);

    return 0;
}

/*
 * Common code for mount and mountroot
 */
static int
hfs_mountfs(struct vnode *devvp, struct mount *mp, struct hfs_mount_args *args)
{
    int retval = 0;

    HFSPlusVolumeHeader *vhp;
    off_t embeddedOffset;
    struct hfsmount *hfsmp;
    retval = hfs_InitialMount(devvp, mp, args, &vhp, &embeddedOffset, &hfsmp, true);
    if ( retval )
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: hfs_InitialMount encountered failure %d \n", retval);
        //No need to go to error_exit, since everything got reset at the Initial Mount
        return retval;
    }

    retval = hfs_CollectBtreeStats(hfsmp, vhp, embeddedOffset, args);
    free(vhp);
    vhp = NULL;
    if ( retval )
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: hfs_CollectBtreeStats encountered failure %d \n", retval);
        goto error_exit;
    }

    // save off a snapshot of the mtime from the previous mount
    // (for matador).
    hfsmp->hfs_last_mounted_mtime = hfsmp->hfs_mtime;

    if ( retval )
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_mountfs: encountered failure %d \n", retval);
        goto error_exit;
    }

    LFHFS_LOG(LEVEL_DEFAULT, "hfs_mountfs: mounted %s on device %s\n", (hfsmp->vcbVN[0] ? (const char*) hfsmp->vcbVN : "unknown"), "unknown device");

    hfs_flushvolumeheader(hfsmp, 0);

    return (0);

error_exit:
    if (vhp) free(vhp);

    if (hfsmp)
    {
        hfsUnmount(hfsmp);

        hfs_locks_destroy(hfsmp);
        hfs_delete_chash(hfsmp);
        hfs_idhash_destroy (hfsmp);

        hfs_free(hfsmp);
        hfsmp = NULL;
    }
    return (retval);
}

/*
 * Destroy all locks, mutexes and spinlocks in hfsmp on unmount or failed mount
 */
static void
hfs_locks_destroy(struct hfsmount *hfsmp)
{

    lf_lck_mtx_destroy(&hfsmp->hfs_mutex);
    lf_lck_mtx_destroy(&hfsmp->sync_mutex);
    lf_lck_rw_destroy(&hfsmp->hfs_global_lock);
    lf_lck_spin_destroy(&hfsmp->vcbFreeExtLock);

    return;
}


/*
 *  Flush any dirty in-memory mount data to the on-disk
 *  volume header.
 *
 *  Note: the on-disk volume signature is intentionally
 *  not flushed since the on-disk "H+" and "HX" signatures
 *  are always stored in-memory as "H+".
 */
int
hfs_flushvolumeheader(struct hfsmount *hfsmp, hfs_flush_volume_header_options_t options)
{
    int retval = 0;

    ExtendedVCB *vcb = HFSTOVCB(hfsmp);
    bool critical = false;
    daddr64_t avh_sector;
    bool altflush = ISSET(options, HFS_FVH_WRITE_ALT);

    void         *pvVolHdrData  = NULL;
    GenericLFBuf *psVolHdrBuf   = NULL;
    void         *pvVolHdr2Data = NULL;
    GenericLFBuf *psVolHdr2Buf  = NULL;
    void         *pvAltHdrData  = NULL;
    GenericLFBuf *psAltHdrBuf   = NULL;


    if (ISSET(options, HFS_FVH_FLUSH_IF_DIRTY) && !hfs_header_needs_flushing(hfsmp)) {
        return 0;
    }

    if (hfsmp->hfs_flags & HFS_READ_ONLY) {
        return 0;
    }

    if (options & HFS_FVH_MARK_UNMOUNT) {
        HFSTOVCB(hfsmp)->vcbAtrb |= kHFSVolumeUnmountedMask;
    } else {
        HFSTOVCB(hfsmp)->vcbAtrb &= ~kHFSVolumeUnmountedMask;
    }
    
    daddr64_t priIDSector = (daddr64_t)((vcb->hfsPlusIOPosOffset / hfsmp->hfs_logical_block_size) + HFS_PRI_SECTOR(hfsmp->hfs_logical_block_size));

    if (!(options & HFS_FVH_SKIP_TRANSACTION)) {
        if (hfs_start_transaction(hfsmp) != 0) {
            return EINVAL;
        }
    }

    psVolHdrBuf = lf_hfs_generic_buf_allocate(hfsmp->hfs_devvp,
                                           HFS_PHYSBLK_ROUNDDOWN(priIDSector, hfsmp->hfs_log_per_phys),
                                           hfsmp->hfs_physical_block_size, GEN_BUF_PHY_BLOCK);
    if (psVolHdrBuf == NULL) {
        retval = ENOMEM;
        goto err_exit;
    }
    pvVolHdrData = psVolHdrBuf->pvData;

    retval = lf_hfs_generic_buf_read(psVolHdrBuf);
    if (retval) {
        LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d reading VH blk (vol=%s)\n", retval, vcb->vcbVN);
        goto err_exit;
    }

    HFSPlusVolumeHeader* volumeHeader = (HFSPlusVolumeHeader *)(pvVolHdrData + HFS_PRI_OFFSET(hfsmp->hfs_physical_block_size));

    /*
     * Sanity check what we just read.  If it's bad, try the alternate instead.
     */
    u_int16_t signature  = SWAP_BE16 (volumeHeader->signature);
    u_int16_t hfsversion = SWAP_BE16 (volumeHeader->version);

    if ((signature != kHFSPlusSigWord && signature != kHFSXSigWord) ||
        (hfsversion < kHFSPlusVersion) || (hfsversion > 100) ||
        (SWAP_BE32 (volumeHeader->blockSize) != vcb->blockSize))
    {
        LFHFS_LOG(LEVEL_DEFAULT, "hfs_flushvolumeheader: corrupt VH on %s, sig 0x%04x, ver %d, blksize %d\n", vcb->vcbVN, signature, hfsversion, SWAP_BE32 (volumeHeader->blockSize));
        hfs_mark_inconsistent(hfsmp, HFS_INCONSISTENCY_DETECTED);

        /* Almost always we read AVH relative to the partition size */
        avh_sector = hfsmp->hfs_partition_avh_sector;

        if (hfsmp->hfs_partition_avh_sector != hfsmp->hfs_fs_avh_sector)
        {
            /*
             * The two altVH offsets do not match --- which means that a smaller file
             * system exists in a larger partition.  Verify that we have the correct
             * alternate volume header sector as per the current parititon size.
             * The GPT device that we are mounted on top could have changed sizes
             * without us knowing.
             *
             * We're in a transaction, so it's safe to modify the partition_avh_sector
             * field if necessary.
             */

            uint64_t sector_count = 0;

            /* Get underlying device block count */
            retval = ioctl(hfsmp->hfs_devvp->psFSRecord->iFD, DKIOCGETBLOCKCOUNT, &sector_count);
            if (retval)
            {
                LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d getting block count (%s) \n", retval, vcb->vcbVN);
                retval = ENXIO;
                goto err_exit;
            }

            /* Partition size was changed without our knowledge */
            if (sector_count != (uint64_t)hfsmp->hfs_logical_block_count)
            {
                hfsmp->hfs_partition_avh_sector = (hfsmp->hfsPlusIOPosOffset / hfsmp->hfs_logical_block_size) + HFS_ALT_SECTOR(hfsmp->hfs_logical_block_size, sector_count);
                /* Note: hfs_fs_avh_sector will remain unchanged */
                LFHFS_LOG(LEVEL_DEFAULT, "hfs_flushvolumeheader: partition size changed, partition_avh_sector=%qu, fs_avh_sector=%qu\n", hfsmp->hfs_partition_avh_sector, hfsmp->hfs_fs_avh_sector);

                /*
                 * We just updated the offset for AVH relative to
                 * the partition size, so the content of that AVH
                 * will be invalid.  But since we are also maintaining
                 * a valid AVH relative to the file system size, we
                 * can read it since primary VH and partition AVH
                 * are not valid.
                 */
                avh_sector = hfsmp->hfs_fs_avh_sector;
            }
        }

        LFHFS_LOG(LEVEL_DEFAULT, "hfs_flushvolumeheader: trying alternate (for %s) avh_sector=%qu\n", (avh_sector == hfsmp->hfs_fs_avh_sector) ? "file system" : "partition", avh_sector);

        if (avh_sector)
        {
            psAltHdrBuf = lf_hfs_generic_buf_allocate(hfsmp->hfs_devvp,
                                                      HFS_PHYSBLK_ROUNDDOWN(avh_sector, hfsmp->hfs_log_per_phys),
                                                      hfsmp->hfs_physical_block_size, GEN_BUF_PHY_BLOCK);
            if (psAltHdrBuf == NULL) {
                retval = ENOMEM;
                goto err_exit;
            }
            pvAltHdrData = psAltHdrBuf->pvData;

            retval = lf_hfs_generic_buf_read(psAltHdrBuf);

            if (retval)
            {
                LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d reading alternate VH blk (vol=%s)\n", retval, vcb->vcbVN);
                goto err_exit;
            }

            HFSPlusVolumeHeader * altVH = (HFSPlusVolumeHeader *)(pvAltHdrData +  HFS_ALT_OFFSET(hfsmp->hfs_physical_block_size));
            signature  = SWAP_BE16(altVH->signature);
            hfsversion = SWAP_BE16(altVH->version);

            if ((signature != kHFSPlusSigWord && signature != kHFSXSigWord) ||
                (hfsversion < kHFSPlusVersion) || (kHFSPlusVersion > 100) ||
                (SWAP_BE32(altVH->blockSize) != vcb->blockSize))
            {
                LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: corrupt alternate VH on %s, sig 0x%04x, ver %d, blksize %d\n", vcb->vcbVN, signature, hfsversion, SWAP_BE32(altVH->blockSize));
                retval = EIO;
                goto err_exit;
            }

            /* The alternate is plausible, so use it. */
            bcopy(altVH, volumeHeader, kMDBSize);
            lf_hfs_generic_buf_release(psAltHdrBuf);
            pvAltHdrData = NULL;
        }
        else
        {
            /* No alternate VH, nothing more we can do. */
            retval = EIO;
            goto err_exit;
        }
    }

    if (hfsmp->jnl)
    {
        journal_modify_block_start(hfsmp->jnl, psVolHdrBuf);
    }

    /*
     * For embedded HFS+ volumes, update create date if it changed
     * (ie from a setattrlist call)
     */
    if ((vcb->hfsPlusIOPosOffset != 0) && (SWAP_BE32 (volumeHeader->createDate) != vcb->localCreateDate))
    {
        HFSMasterDirectoryBlock    *mdb;

        psVolHdr2Buf = lf_hfs_generic_buf_allocate(hfsmp->hfs_devvp,
                                                HFS_PHYSBLK_ROUNDDOWN(HFS_PRI_SECTOR(hfsmp->hfs_logical_block_size), hfsmp->hfs_log_per_phys),
                                                hfsmp->hfs_physical_block_size, GEN_BUF_PHY_BLOCK);
        if (psVolHdr2Buf == NULL) {
            retval = ENOMEM;
            goto err_exit;
        }
        void *pvVolHdr2Data = psVolHdr2Buf->pvData;

        retval = lf_hfs_generic_buf_read(psVolHdr2Buf);

        if (retval)
        {
            lf_hfs_generic_buf_release(psVolHdr2Buf);
            LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d reading alternate VH blk (vol=%s)\n", retval, vcb->vcbVN);
            goto err_exit;
        }

        mdb = (HFSMasterDirectoryBlock *)(pvVolHdr2Data + HFS_PRI_OFFSET(hfsmp->hfs_physical_block_size));

        if ( SWAP_BE32 (mdb->drCrDate) != vcb->localCreateDate )
        {
            if (hfsmp->jnl)
            {
                journal_modify_block_start(hfsmp->jnl, psVolHdr2Buf);
            }
            mdb->drCrDate = SWAP_BE32 (vcb->localCreateDate);    /* pick up the new create date */
            if (hfsmp->jnl)
            {
                journal_modify_block_end(hfsmp->jnl, psVolHdr2Buf, NULL, NULL);
            }
            else
            {
                retval = raw_readwrite_write_mount( hfsmp->hfs_devvp, HFS_PHYSBLK_ROUNDDOWN(HFS_PRI_SECTOR(hfsmp->hfs_logical_block_size), hfsmp->hfs_log_per_phys), hfsmp->hfs_physical_block_size, pvVolHdr2Data, hfsmp->hfs_physical_block_size, NULL, NULL);

                lf_hfs_generic_buf_release(psVolHdr2Buf);
                pvVolHdr2Data = NULL;
                if (retval)
                {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d writing VH blk (vol=%s)\n", retval, vcb->vcbVN);
                    goto err_exit;
                }
            }
        }
        else
        {
            lf_hfs_generic_buf_release(psVolHdr2Buf);                        /* just release it */
            pvVolHdr2Data = NULL;
        }
    }

    hfs_lock_mount (hfsmp);

    /* Note: only update the lower 16 bits worth of attributes */
    volumeHeader->attributes       = SWAP_BE32 (vcb->vcbAtrb);
    volumeHeader->journalInfoBlock = SWAP_BE32 (vcb->vcbJinfoBlock);
    if (hfsmp->jnl)
    {
        volumeHeader->lastMountedVersion = SWAP_BE32 (kHFSJMountVersion);
    }
    else
    {
        volumeHeader->lastMountedVersion = SWAP_BE32 (kHFSPlusMountVersion);
    }
    volumeHeader->createDate      = SWAP_BE32 (vcb->localCreateDate);  /* volume create date is in local time */
    volumeHeader->modifyDate      = SWAP_BE32 (to_hfs_time(vcb->vcbLsMod));
    volumeHeader->backupDate      = SWAP_BE32 (to_hfs_time(vcb->vcbVolBkUp));
    volumeHeader->fileCount       = SWAP_BE32 (vcb->vcbFilCnt);
    volumeHeader->folderCount     = SWAP_BE32 (vcb->vcbDirCnt);
    volumeHeader->totalBlocks     = SWAP_BE32 (vcb->totalBlocks);
    volumeHeader->freeBlocks      = SWAP_BE32 (vcb->freeBlocks + vcb->reclaimBlocks);
    volumeHeader->nextAllocation  = SWAP_BE32 (vcb->nextAllocation);
    volumeHeader->rsrcClumpSize   = SWAP_BE32 (vcb->vcbClpSiz);
    volumeHeader->dataClumpSize   = SWAP_BE32 (vcb->vcbClpSiz);
    volumeHeader->nextCatalogID   = SWAP_BE32 (vcb->vcbNxtCNID);
    volumeHeader->writeCount      = SWAP_BE32 (vcb->vcbWrCnt);
    volumeHeader->encodingsBitmap = SWAP_BE64 (vcb->encodingsBitmap);

    if (bcmp(vcb->vcbFndrInfo, volumeHeader->finderInfo, sizeof(volumeHeader->finderInfo)) != 0)
    {
        bcopy(vcb->vcbFndrInfo, volumeHeader->finderInfo, sizeof(volumeHeader->finderInfo));
        critical = true;
    }

    if (!altflush && !ISSET(options, HFS_FVH_FLUSH_IF_DIRTY))
    {
        goto done;
    }

    /* Sync Extents over-flow file meta data */
    struct filefork * fp = VTOF(vcb->extentsRefNum);
    if (FTOC(fp)->c_flag & C_MODIFIED)
    {
        for (int iExtentCounter = 0; iExtentCounter < kHFSPlusExtentDensity; iExtentCounter++)
        {
            volumeHeader->extentsFile.extents[iExtentCounter].startBlock    = SWAP_BE32 (fp->ff_extents[iExtentCounter].startBlock);
            volumeHeader->extentsFile.extents[iExtentCounter].blockCount    = SWAP_BE32 (fp->ff_extents[iExtentCounter].blockCount);
        }
        volumeHeader->extentsFile.logicalSize = SWAP_BE64 (fp->ff_size);
        volumeHeader->extentsFile.totalBlocks = SWAP_BE32 (fp->ff_blocks);
        volumeHeader->extentsFile.clumpSize   = SWAP_BE32 (fp->ff_clumpsize);
        FTOC(fp)->c_flag &= ~C_MODIFIED;
        altflush = true;
    }

    /* Sync Catalog file meta data */
    fp = VTOF(vcb->catalogRefNum);
    if (FTOC(fp)->c_flag & C_MODIFIED)
    {
        for (int iExtentCounter = 0; iExtentCounter < kHFSPlusExtentDensity; iExtentCounter++)
        {
            volumeHeader->catalogFile.extents[iExtentCounter].startBlock    = SWAP_BE32 (fp->ff_extents[iExtentCounter].startBlock);
            volumeHeader->catalogFile.extents[iExtentCounter].blockCount    = SWAP_BE32 (fp->ff_extents[iExtentCounter].blockCount);
        }
        volumeHeader->catalogFile.logicalSize = SWAP_BE64 (fp->ff_size);
        volumeHeader->catalogFile.totalBlocks = SWAP_BE32 (fp->ff_blocks);
        volumeHeader->catalogFile.clumpSize   = SWAP_BE32 (fp->ff_clumpsize);
        FTOC(fp)->c_flag &= ~C_MODIFIED;
        altflush = true;
    }

    /* Sync Allocation file meta data */
    fp = VTOF(vcb->allocationsRefNum);
    if (FTOC(fp)->c_flag & C_MODIFIED)
    {
        for (int iExtentCounter = 0; iExtentCounter < kHFSPlusExtentDensity; iExtentCounter++)
        {
            volumeHeader->allocationFile.extents[iExtentCounter].startBlock = SWAP_BE32 (fp->ff_extents[iExtentCounter].startBlock);
            volumeHeader->allocationFile.extents[iExtentCounter].blockCount = SWAP_BE32 (fp->ff_extents[iExtentCounter].blockCount);
        }
        volumeHeader->allocationFile.logicalSize = SWAP_BE64 (fp->ff_size);
        volumeHeader->allocationFile.totalBlocks = SWAP_BE32 (fp->ff_blocks);
        volumeHeader->allocationFile.clumpSize   = SWAP_BE32 (fp->ff_clumpsize);
        FTOC(fp)->c_flag &= ~C_MODIFIED;
        altflush = true;
    }

    /* Sync Attribute file meta data */
    if (hfsmp->hfs_attribute_vp)
    {
        fp = VTOF(hfsmp->hfs_attribute_vp);
        for (int iExtentCounter = 0; iExtentCounter < kHFSPlusExtentDensity; iExtentCounter++)
        {
            volumeHeader->attributesFile.extents[iExtentCounter].startBlock = SWAP_BE32 (fp->ff_extents[iExtentCounter].startBlock);
            volumeHeader->attributesFile.extents[iExtentCounter].blockCount = SWAP_BE32 (fp->ff_extents[iExtentCounter].blockCount);
        }
        if (ISSET(FTOC(fp)->c_flag, C_MODIFIED))
        {
            FTOC(fp)->c_flag &= ~C_MODIFIED;
            altflush = true;
        }
        volumeHeader->attributesFile.logicalSize = SWAP_BE64 (fp->ff_size);
        volumeHeader->attributesFile.totalBlocks = SWAP_BE32 (fp->ff_blocks);
        volumeHeader->attributesFile.clumpSize   = SWAP_BE32 (fp->ff_clumpsize);
    }

    /* Sync Startup file meta data */
    if (hfsmp->hfs_startup_vp)
    {
        fp = VTOF(hfsmp->hfs_startup_vp);
        if (FTOC(fp)->c_flag & C_MODIFIED)
        {
            for (int iExtentCounter = 0; iExtentCounter < kHFSPlusExtentDensity; iExtentCounter++)
            {
                volumeHeader->startupFile.extents[iExtentCounter].startBlock = SWAP_BE32 (fp->ff_extents[iExtentCounter].startBlock);
                volumeHeader->startupFile.extents[iExtentCounter].blockCount = SWAP_BE32 (fp->ff_extents[iExtentCounter].blockCount);
            }
            volumeHeader->startupFile.logicalSize = SWAP_BE64 (fp->ff_size);
            volumeHeader->startupFile.totalBlocks = SWAP_BE32 (fp->ff_blocks);
            volumeHeader->startupFile.clumpSize   = SWAP_BE32 (fp->ff_clumpsize);
            FTOC(fp)->c_flag &= ~C_MODIFIED;
            altflush = true;
        }
    }

    if (altflush)
        critical = true;

done:
    MarkVCBClean(hfsmp);
    hfs_unlock_mount (hfsmp);

    /* If requested, flush out the alternate volume header */
    if (altflush) {
        /*
         * The two altVH offsets do not match --- which means that a smaller file
         * system exists in a larger partition.  Verify that we have the correct
         * alternate volume header sector as per the current parititon size.
         * The GPT device that we are mounted on top could have changed sizes
         * without us knowning.
         *
         * We're in a transaction, so it's safe to modify the partition_avh_sector
         * field if necessary.
         */
        if (hfsmp->hfs_partition_avh_sector != hfsmp->hfs_fs_avh_sector)
        {
            uint64_t sector_count;

            /* Get underlying device block count */
            retval = ioctl(hfsmp->hfs_devvp->psFSRecord->iFD, DKIOCGETBLOCKCOUNT, &sector_count);
            if (retval)
            {
                LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d getting block count (%s) \n", retval, vcb->vcbVN);
                retval = ENXIO;
                goto err_exit;
            }

            /* Partition size was changed without our knowledge */
            if (sector_count != (uint64_t)hfsmp->hfs_logical_block_count)
            {
                hfsmp->hfs_partition_avh_sector = (hfsmp->hfsPlusIOPosOffset / hfsmp->hfs_logical_block_size) +  HFS_ALT_SECTOR(hfsmp->hfs_logical_block_size, sector_count);
                /* Note: hfs_fs_avh_sector will remain unchanged */
                LFHFS_LOG(LEVEL_DEFAULT, "hfs_flushvolumeheader: altflush: partition size changed, partition_avh_sector=%qu, fs_avh_sector=%qu\n",
                        hfsmp->hfs_partition_avh_sector, hfsmp->hfs_fs_avh_sector);
            }
        }

        /*
         * First see if we need to write I/O to the "secondary" AVH
         * located at FS Size - 1024 bytes, because this one will
         * always go into the journal.  We put this AVH into the journal
         * because even if the filesystem size has shrunk, this LBA should be
         * reachable after the partition-size modification has occurred.
         * The one where we need to be careful is partitionsize-1024, since the
         * partition size should hopefully shrink.
         *
         * Most of the time this block will not execute.
         */
        if ((hfsmp->hfs_fs_avh_sector) && (hfsmp->hfs_partition_avh_sector != hfsmp->hfs_fs_avh_sector))
        {
            if (pvAltHdrData != NULL)
            {
                panic("We shouldn't be here!");
                hfs_assert(0);
            }

            psAltHdrBuf = lf_hfs_generic_buf_allocate(hfsmp->hfs_devvp,
                                            HFS_PHYSBLK_ROUNDDOWN(hfsmp->hfs_fs_avh_sector, hfsmp->hfs_log_per_phys),
                                            hfsmp->hfs_physical_block_size, GEN_BUF_PHY_BLOCK);

            if (psAltHdrBuf == NULL) {
                retval = ENOMEM;
                goto err_exit;
            }
            pvAltHdrData = psAltHdrBuf->pvData;

            retval = lf_hfs_generic_buf_read(psAltHdrBuf);
            if (retval)
            {
                LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d reading alternate VH blk (vol=%s)\n", retval, vcb->vcbVN);
                goto err_exit;
            }

            if (hfsmp->jnl)
            {
                journal_modify_block_start(hfsmp->jnl, psAltHdrBuf);
            }

            bcopy(volumeHeader, pvAltHdrData + HFS_ALT_OFFSET(hfsmp->hfs_physical_block_size), kMDBSize);

            if (hfsmp->jnl)
            {
                journal_modify_block_end(hfsmp->jnl, psAltHdrBuf, NULL, NULL);
            }
            else
            {
                retval = raw_readwrite_write_mount( hfsmp->hfs_devvp, HFS_PHYSBLK_ROUNDDOWN(hfsmp->hfs_fs_avh_sector, hfsmp->hfs_log_per_phys), hfsmp->hfs_physical_block_size, pvAltHdrData, hfsmp->hfs_physical_block_size, NULL, NULL);
                if (retval)
                {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d writing VH blk (vol=%s)\n", retval, vcb->vcbVN);
                    goto err_exit;
                }
                lf_hfs_generic_buf_release(psAltHdrBuf);
                pvAltHdrData = NULL;
            }
        }

        /*
         * Flush out alternate volume header located at 1024 bytes before
         * end of the partition as part of journal transaction.  In
         * most cases, this will be the only alternate volume header
         * that we need to worry about because the file system size is
         * same as the partition size, therefore hfs_fs_avh_sector is
         * same as hfs_partition_avh_sector. This is the "priority" AVH.
         *
         * However, do not always put this I/O into the journal.  If we skipped the
         * FS-Size AVH write above, then we will put this I/O into the journal as
         * that indicates the two were in sync.  However, if the FS size is
         * not the same as the partition size, we are tracking two.  We don't
         * put it in the journal in that case, since if the partition
         * size changes between uptimes, and we need to replay the journal,
         * this I/O could generate an EIO if during replay it is now trying
         * to access blocks beyond the device EOF.
         */
        if (hfsmp->hfs_partition_avh_sector)
        {
            if (pvAltHdrData != NULL)
            {
                panic("We shouldn't be here!");
                hfs_assert(0);
            }

            psAltHdrBuf = lf_hfs_generic_buf_allocate(hfsmp->hfs_devvp,
                                                      HFS_PHYSBLK_ROUNDDOWN(hfsmp->hfs_fs_avh_sector, hfsmp->hfs_log_per_phys),
                                                      hfsmp->hfs_physical_block_size, GEN_BUF_PHY_BLOCK);
            if (psAltHdrBuf == NULL) {
                retval = ENOMEM;
                goto err_exit;
            }
            pvAltHdrData = psAltHdrBuf->pvData;

            retval = lf_hfs_generic_buf_read(psAltHdrBuf);
            
            if (retval)
            {
                LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d reading alternate VH blk (vol=%s)\n", retval, vcb->vcbVN);
                goto err_exit;
            }

            /* only one AVH, put this I/O in the journal. */
            if ((hfsmp->jnl) && (hfsmp->hfs_partition_avh_sector == hfsmp->hfs_fs_avh_sector)) {
                journal_modify_block_start(hfsmp->jnl, psAltHdrBuf);
            }

            bcopy(volumeHeader, pvAltHdrData + HFS_ALT_OFFSET(hfsmp->hfs_physical_block_size), kMDBSize);

            /* If journaled and we only have one AVH to track */
            if ((hfsmp->jnl) && (hfsmp->hfs_partition_avh_sector == hfsmp->hfs_fs_avh_sector)) {
                journal_modify_block_end (hfsmp->jnl, psAltHdrBuf, NULL, NULL);
            }
            else
            {
                /*
                 * If we don't have a journal or there are two AVH's at the
                 * moment, then this one doesn't go in the journal.  Note that
                 * this one may generate I/O errors, since the partition
                 * can be resized behind our backs at any moment and this I/O
                 * may now appear to be beyond the device EOF.
                 */
                retval = raw_readwrite_write_mount( hfsmp->hfs_devvp, HFS_PHYSBLK_ROUNDDOWN(hfsmp->hfs_fs_avh_sector, hfsmp->hfs_log_per_phys), hfsmp->hfs_physical_block_size, pvAltHdrData, hfsmp->hfs_physical_block_size, NULL, NULL);
                if (retval)
                {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d writing VH blk (vol=%s)\n", retval, vcb->vcbVN);
                    goto err_exit;
                }
                lf_hfs_generic_buf_release(psAltHdrBuf);
                pvAltHdrData = NULL;
                hfs_flush(hfsmp, HFS_FLUSH_CACHE);
            }
        }
    }

    /* Finish modifying the block for the primary VH */
    if (hfsmp->jnl) {
        journal_modify_block_end(hfsmp->jnl, psVolHdrBuf, NULL, NULL);
    }
    else
    {
        retval = raw_readwrite_write_mount( hfsmp->hfs_devvp, HFS_PHYSBLK_ROUNDDOWN(priIDSector, hfsmp->hfs_log_per_phys), hfsmp->hfs_physical_block_size, pvVolHdrData, hfsmp->hfs_physical_block_size, NULL, NULL);
        /* When critical data changes, flush the device cache */
        if (critical && (retval == 0))
        {
            hfs_flush(hfsmp, HFS_FLUSH_CACHE);
        }

        lf_hfs_generic_buf_release(psVolHdrBuf);
        pvVolHdrData = NULL;
        if (retval)
        {
            LFHFS_LOG(LEVEL_ERROR, "hfs_flushvolumeheader: err %d reading VH blk (vol=%s)\n", retval, vcb->vcbVN);
            goto err_exit;
        }
    }
    if (!(options & HFS_FVH_SKIP_TRANSACTION)) {
        hfs_end_transaction(hfsmp);
    }

    return (retval);

err_exit:
    if (pvVolHdrData)
        lf_hfs_generic_buf_release(psVolHdrBuf);
    if (pvVolHdr2Data)
        lf_hfs_generic_buf_release(psVolHdr2Buf);
    if (pvAltHdrData)
        lf_hfs_generic_buf_release(psAltHdrBuf);

    if (!(options & HFS_FVH_SKIP_TRANSACTION)) {
        hfs_end_transaction(hfsmp);
    }
    return retval;
}

/* If a runtime corruption is detected, set the volume inconsistent
 * bit in the volume attributes.  The volume inconsistent bit is a persistent
 * bit which represents that the volume is corrupt and needs repair.
 * The volume inconsistent bit can be set from the kernel when it detects
 * runtime corruption or from file system repair utilities like fsck_hfs when
 * a repair operation fails.  The bit should be cleared only from file system
 * verify/repair utility like fsck_hfs when a verify/repair succeeds.
 */
void hfs_mark_inconsistent(struct hfsmount *hfsmp, hfs_inconsistency_reason_t reason)
{
    hfs_lock_mount (hfsmp);
    if ((hfsmp->vcbAtrb & kHFSVolumeInconsistentMask) == 0)
    {
        hfsmp->vcbAtrb |= kHFSVolumeInconsistentMask;
        MarkVCBDirty(hfsmp);
    }
    if ((hfsmp->hfs_flags & HFS_READ_ONLY)==0)
    {
        switch (reason)
        {
            case HFS_INCONSISTENCY_DETECTED:
                LFHFS_LOG(LEVEL_ERROR, "hfs_mark_inconsistent: Runtime corruption detected on %s, fsck will be forced on next mount.\n",hfsmp->vcbVN);
                break;
            case HFS_ROLLBACK_FAILED:
                LFHFS_LOG(LEVEL_ERROR, "hfs_mark_inconsistent: Failed to roll back; volume `%s' might be inconsistent; fsck will be forced on next mount.\n", hfsmp->vcbVN);
                break;
            case HFS_OP_INCOMPLETE:
                LFHFS_LOG(LEVEL_ERROR, "hfs_mark_inconsistent: Failed to complete operation; volume `%s' might be inconsistent; fsck will be forced on next mount.\n",hfsmp->vcbVN);
                break;
            case HFS_FSCK_FORCED:
                LFHFS_LOG(LEVEL_ERROR, "hfs_mark_inconsistent: fsck requested for `%s'; fsck will be forced on next mount.\n",hfsmp->vcbVN);
                break;
        }
    }
    hfs_unlock_mount (hfsmp);
}

/*
 * Creates a UUID from a unique "name" in the HFS UUID Name space.
 * See version 3 UUID.
 */
void
hfs_getvoluuid(struct hfsmount *hfsmp, uuid_t result_uuid)
{

    if (uuid_is_null(hfsmp->hfs_full_uuid)) {
        uuid_t result;

        CC_MD5_CTX  md5c;
        uint8_t  rawUUID[8];

        ((uint32_t *)rawUUID)[0] = hfsmp->vcbFndrInfo[6];
        ((uint32_t *)rawUUID)[1] = hfsmp->vcbFndrInfo[7];

        CC_MD5_Init( &md5c );
        CC_MD5_Update( &md5c, HFS_UUID_NAMESPACE_ID, sizeof( uuid_t ) );
        CC_MD5_Update( &md5c, rawUUID, sizeof (rawUUID) );
        CC_MD5_Final( result, &md5c );

        result[6] = 0x30 | ( result[6] & 0x0F );
        result[8] = 0x80 | ( result[8] & 0x3F );

        uuid_copy(hfsmp->hfs_full_uuid, result);
    }
    uuid_copy (result_uuid, hfsmp->hfs_full_uuid);

}

/*
 * Call into the allocator code and perform a full scan of the bitmap file.
 *
 * This allows us to TRIM unallocated ranges if needed, and also to build up
 * an in-memory summary table of the state of the allocated blocks.
 */
void hfs_scan_blocks (struct hfsmount *hfsmp)
{
    /*
     * Take the allocation file lock.  Journal transactions will block until
     * we're done here.
     */
    int flags = hfs_systemfile_lock(hfsmp, SFL_BITMAP, HFS_EXCLUSIVE_LOCK);

    /*
     * We serialize here with the HFS mount lock as we're mounting.
     *
     * The mount can only proceed once this thread has acquired the bitmap
     * lock, since we absolutely do not want someone else racing in and
     * getting the bitmap lock, doing a read/write of the bitmap file,
     * then us getting the bitmap lock.
     *
     * To prevent this, the mount thread takes the HFS mount mutex, starts us
     * up, then immediately msleeps on the scan_var variable in the mount
     * point as a condition variable.  This serialization is safe since
     * if we race in and try to proceed while they're still holding the lock,
     * we'll block trying to acquire the global lock.  Since the mount thread
     * acquires the HFS mutex before starting this function in a new thread,
     * any lock acquisition on our part must be linearizably AFTER the mount thread's.
     *
     * Note that the HFS mount mutex is always taken last, and always for only
     * a short time.  In this case, we just take it long enough to mark the
     * scan-in-flight bit.
     */
    (void) hfs_lock_mount (hfsmp);
    hfsmp->scan_var |= HFS_ALLOCATOR_SCAN_INFLIGHT;
    hfs_unlock_mount (hfsmp);

    /* Initialize the summary table */
    if (hfs_init_summary (hfsmp))
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_scan_blocks: could not initialize summary table for %s\n", hfsmp->vcbVN);
    }

    /*
     * ScanUnmapBlocks assumes that the bitmap lock is held when you
     * call the function. We don't care if there were any errors issuing unmaps.
     *
     * It will also attempt to build up the summary table for subsequent
     * allocator use, as configured.
     */
    (void) ScanUnmapBlocks(hfsmp);

    (void) hfs_lock_mount (hfsmp);
    hfsmp->scan_var &= ~HFS_ALLOCATOR_SCAN_INFLIGHT;
    hfsmp->scan_var |= HFS_ALLOCATOR_SCAN_COMPLETED;
    hfs_unlock_mount (hfsmp);

    hfs_systemfile_unlock(hfsmp, flags);
}

/*
 * Look up an HFS object by ID.
 *
 * The object is returned with an iocount reference and the cnode locked.
 *
 * If the object is a file then it will represent the data fork.
 */
int
hfs_vget(struct hfsmount *hfsmp, cnid_t cnid, struct vnode **vpp, int skiplock, int allow_deleted)
{
    struct vnode *vp = NULL;
    struct cat_desc cndesc;
    struct cat_attr cnattr;
    struct cat_fork cnfork;

    u_int32_t linkref = 0;

    int error;

    /* Check for cnids that should't be exported. */
    if ((cnid < kHFSFirstUserCatalogNodeID) &&
        (cnid != kHFSRootFolderID && cnid != kHFSRootParentID)) {
        return (ENOENT);
    }
    /* Don't export our private directories. */
    if (cnid == hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid ||
        cnid == hfsmp->hfs_private_desc[DIR_HARDLINKS].cd_cnid) {
        return (ENOENT);
    }
    /*
     * Check the hash first
     */
    vp = hfs_chash_getvnode(hfsmp, cnid, 0, skiplock, allow_deleted);
    if (vp) {
        *vpp = vp;
        return(0);
    }

    bzero(&cndesc, sizeof(cndesc));
    bzero(&cnattr, sizeof(cnattr));
    bzero(&cnfork, sizeof(cnfork));

    /*
     * Not in hash, lookup in catalog
     */
    if (cnid == kHFSRootParentID) {
        static char hfs_rootname[] = "/";

        cndesc.cd_nameptr = (const u_int8_t *)&hfs_rootname[0];
        cndesc.cd_namelen = 1;
        cndesc.cd_parentcnid = kHFSRootParentID;
        cndesc.cd_cnid = kHFSRootFolderID;
        cndesc.cd_flags = CD_ISDIR;

        cnattr.ca_fileid = kHFSRootFolderID;
        cnattr.ca_linkcount = 1;
        cnattr.ca_entries = 1;
        cnattr.ca_dircount = 1;
        cnattr.ca_mode = (S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
        int lockflags;
        cnid_t pid;
        const char *nameptr;

        lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);
        error = cat_idlookup(hfsmp, cnid, 0, 0, &cndesc, &cnattr, &cnfork);
        hfs_systemfile_unlock(hfsmp, lockflags);

        if (error) {
            *vpp = NULL;
            return (error);
        }

        /*
         * Check for a raw hardlink inode and save its linkref.
         */
        pid = cndesc.cd_parentcnid;
        nameptr = (const char *)cndesc.cd_nameptr;
        if ((pid == hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid) &&
            cndesc.cd_namelen > HFS_INODE_PREFIX_LEN &&
            (bcmp(nameptr, HFS_INODE_PREFIX, HFS_INODE_PREFIX_LEN) == 0)) {
            linkref = (uint32_t) strtoul(&nameptr[HFS_INODE_PREFIX_LEN], NULL, 10);

        } else if ((pid == hfsmp->hfs_private_desc[DIR_HARDLINKS].cd_cnid) &&
                   cndesc.cd_namelen > HFS_DIRINODE_PREFIX_LEN &&
                   (bcmp(nameptr, HFS_DIRINODE_PREFIX, HFS_DIRINODE_PREFIX_LEN) == 0)) {
            linkref = (uint32_t) strtoul(&nameptr[HFS_DIRINODE_PREFIX_LEN], NULL, 10);

        } else if ((pid == hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid) &&
                   cndesc.cd_namelen > HFS_DELETE_PREFIX_LEN &&
                   (bcmp(nameptr, HFS_DELETE_PREFIX, HFS_DELETE_PREFIX_LEN) == 0)) {
            *vpp = NULL;
            cat_releasedesc(&cndesc);
            return (ENOENT);  /* open unlinked file */
        }
    }

    /*
     * Finish initializing cnode descriptor for hardlinks.
     *
     * We need a valid name and parent for reverse lookups.
     */
    if (linkref) {
        cnid_t lastid;
        struct cat_desc linkdesc;
        int linkerr = 0;

        cnattr.ca_linkref = linkref;
        bzero (&linkdesc, sizeof (linkdesc));

        /*
         * If the caller supplied the raw inode value, then we don't know exactly
         * which hardlink they wanted. It's likely that they acquired the raw inode
         * value BEFORE the item became a hardlink, in which case, they probably
         * want the oldest link.  So request the oldest link from the catalog.
         *
         * Unfortunately, this requires that we iterate through all N hardlinks. On the plus
         * side, since we know that we want the last linkID, we can also have this one
         * call give us back the name of the last ID, since it's going to have it in-hand...
         */
        linkerr = hfs_lookup_lastlink (hfsmp, linkref, &lastid, &linkdesc);
        if ((linkerr == 0) && (lastid != 0)) {
            /*
             * Release any lingering buffers attached to our local descriptor.
             * Then copy the name and other business into the cndesc
             */
            cat_releasedesc (&cndesc);
            bcopy (&linkdesc, &cndesc, sizeof(linkdesc));
        }
        /* If it failed, the linkref code will just use whatever it had in-hand below. */
    }

    if (linkref) {
        int newvnode_flags = 0;
        error = hfs_getnewvnode(hfsmp, NULL, NULL, &cndesc, 0, &cnattr, &cnfork, &vp, &newvnode_flags);
        if (error == 0) {
            VTOC(vp)->c_flag |= C_HARDLINK;

            //TBD - this set is for vfs -> since we have the C_HARDLINK
            //      currently disable this set.
            //vnode_setmultipath(vp);
        }
    }
    else
    {
        int newvnode_flags = 0;

        void *buf = hfs_malloc(MAXPATHLEN);

        /* Supply hfs_getnewvnode with a component name. */
        struct componentname cn = {
            .cn_nameiop = LOOKUP,
            .cn_flags    = ISLASTCN,
            .cn_pnlen    = MAXPATHLEN,
            .cn_namelen = cndesc.cd_namelen,
            .cn_pnbuf    = buf,
            .cn_nameptr = buf
        };

        bcopy(cndesc.cd_nameptr, cn.cn_nameptr, cndesc.cd_namelen + 1);
        error = hfs_getnewvnode(hfsmp, NULL, &cn, &cndesc, 0, &cnattr, &cnfork, &vp, &newvnode_flags);

        if (error == 0 && (VTOC(vp)->c_flag & C_HARDLINK)) {
            hfs_savelinkorigin(VTOC(vp), cndesc.cd_parentcnid);
        }

        hfs_free(buf);
    }
    cat_releasedesc(&cndesc);

    *vpp = vp;
    if (vp && skiplock) {
        hfs_unlock(VTOC(vp));
    }
    return (error);
}

int
hfs_GetInfoByID(struct hfsmount *hfsmp, cnid_t cnid, UVFSFileAttributes *file_attrs, char pcName[MAX_UTF8_NAME_LENGTH])
{
    struct vnode *psVnode = NULL;
    int error = hfs_vget(hfsmp, cnid, &psVnode, 0, 0);
    if (error || psVnode == NULL) {
        if (psVnode != NULL) hfs_unlock(VTOC(psVnode));
        hfs_vnop_reclaim(psVnode);
        return EFAULT;
    } else {
        vnode_GetAttrInternal (psVnode, file_attrs);
        hfs_unlock(VTOC(psVnode));
    }

    if (cnid == kHFSRootFolderID)
        pcName[0] = 0;
    else {
        //Make sure we actually have the name in the vnode
        if (psVnode->sFSParams.vnfs_cnp && psVnode->sFSParams.vnfs_cnp->cn_nameptr)
            strlcpy(pcName, (char*) psVnode->sFSParams.vnfs_cnp->cn_nameptr, MAX_UTF8_NAME_LENGTH);
        else
            return EINVAL;
    }

    error = hfs_vnop_reclaim(psVnode);
    return (error);
}

/*
 * Return the root of a filesystem.
 */
int hfs_vfs_root(struct mount *mp, struct vnode **vpp)
{
    return hfs_vget(VFSTOHFS(mp), (cnid_t)kHFSRootFolderID, vpp, 1, 0);
}

/*
 * unmount system call
 */
int hfs_unmount(struct mount *mp)
{
    struct hfsmount *hfsmp = VFSTOHFS(mp);
    int retval = E_NONE;
    
    if (hfsmp->hfs_flags & HFS_SUMMARY_TABLE)
    {
        if (hfsmp->hfs_summary_table)
        {
            int err = 0;
            /*
             * Take the bitmap lock to serialize against a concurrent bitmap scan still in progress
             */
            if (hfsmp->hfs_allocation_vp)
            {
                err = hfs_lock (VTOC(hfsmp->hfs_allocation_vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
            }
            hfs_free(hfsmp->hfs_summary_table);
            hfsmp->hfs_summary_table = NULL;
            hfsmp->hfs_flags &= ~HFS_SUMMARY_TABLE;

            if (err == 0 && hfsmp->hfs_allocation_vp)
            {
                hfs_unlock (VTOC(hfsmp->hfs_allocation_vp));
            }
        }
    }

    /*
     *    Invalidate our caches and release metadata vnodes
     */
    if (hfsmp->jnl) {
        journal_release(hfsmp->jnl);
        hfsmp->jnl = NULL;
    }
    
    hfsUnmount(hfsmp);
    int iFD = hfsmp->hfs_devvp->psFSRecord->iFD;
    // Remove Buffer cache entries realted to the mount
    lf_hfs_generic_buf_cache_clear_by_iFD(iFD);
    
    vnode_rele(hfsmp->hfs_devvp);
    
    hfs_locks_destroy(hfsmp);
    hfs_delete_chash(hfsmp);
    hfs_idhash_destroy(hfsmp);

    hfs_assert(TAILQ_EMPTY(&hfsmp->hfs_reserved_ranges[HFS_TENTATIVE_BLOCKS]) && TAILQ_EMPTY(&hfsmp->hfs_reserved_ranges[HFS_LOCKED_BLOCKS]));
    hfs_assert(!hfsmp->lockedBlocks);

    hfs_free(hfsmp);

    return (retval);
}
/* Update volume encoding bitmap (HFS Plus only)
 *
 * Mark a legacy text encoding as in-use (as needed)
 * in the volume header of this HFS+ filesystem.
 */
void
hfs_setencodingbits(struct hfsmount *hfsmp, u_int32_t encoding)
{
#define  kIndexMacUkrainian    48  /* MacUkrainian encoding is 152 */
#define  kIndexMacFarsi        49  /* MacFarsi encoding is 140 */

    u_int32_t    index;

    switch (encoding)
    {
        case kTextEncodingMacUkrainian:
            index = kIndexMacUkrainian;
            break;
        case kTextEncodingMacFarsi:
            index = kIndexMacFarsi;
            break;
        default:
            index = encoding;
            break;
    }

    /* Only mark the encoding as in-use if it wasn't already set */
    if (index < 64 && (hfsmp->encodingsBitmap & (u_int64_t)(1ULL << index)) == 0) {
        hfs_lock_mount (hfsmp);
        hfsmp->encodingsBitmap |= (u_int64_t)(1ULL << index);
        MarkVCBDirty(hfsmp);
        hfs_unlock_mount(hfsmp);
    }
}

/*
 * Update volume stats
 *
 * On journal volumes this will cause a volume header flush
 */
int
hfs_volupdate(struct hfsmount *hfsmp, enum volop op, int inroot)
{
    struct timeval tv;
    microtime(&tv);
    hfs_lock_mount (hfsmp);

    MarkVCBDirty(hfsmp);
    hfsmp->hfs_mtime = tv.tv_sec;

    switch (op) {
        case VOL_UPDATE:
            break;
        case VOL_MKDIR:
            if (hfsmp->hfs_dircount != 0xFFFFFFFF)
                ++hfsmp->hfs_dircount;
            if (inroot && hfsmp->vcbNmRtDirs != 0xFFFF)
                ++hfsmp->vcbNmRtDirs;
            break;
        case VOL_RMDIR:
            if (hfsmp->hfs_dircount != 0)
                --hfsmp->hfs_dircount;
            if (inroot && hfsmp->vcbNmRtDirs != 0xFFFF)
                --hfsmp->vcbNmRtDirs;
            break;
        case VOL_MKFILE:
            if (hfsmp->hfs_filecount != 0xFFFFFFFF)
                ++hfsmp->hfs_filecount;
            if (inroot && hfsmp->vcbNmFls != 0xFFFF)
                ++hfsmp->vcbNmFls;
            break;
        case VOL_RMFILE:
            if (hfsmp->hfs_filecount != 0)
                --hfsmp->hfs_filecount;
            if (inroot && hfsmp->vcbNmFls != 0xFFFF)
                --hfsmp->vcbNmFls;
            break;
    }

    hfs_unlock_mount (hfsmp);
    hfs_flushvolumeheader(hfsmp, 0);
    
    return (0);
}

