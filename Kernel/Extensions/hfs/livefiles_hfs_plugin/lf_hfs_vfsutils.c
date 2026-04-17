/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_vfsutils.c
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 18/3/18.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>
#include <mach/mach.h>
#include <sys/disk.h>

#include "lf_hfs.h"
#include "lf_hfs_locks.h"
#include "lf_hfs_format.h"
#include "lf_hfs.h"
#include "lf_hfs_endian.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_mount.h"
#include "lf_hfs_utils.h"
#include "lf_hfs_logger.h"
#include "lf_hfs_raw_read_write.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_vfsops.h"
#include "lf_hfs_file_mgr_internal.h"
#include "lf_hfs_btrees_internal.h"
#include "lf_hfs_format.h"
#include "lf_hfs_file_extent_mapping.h"
#include "lf_hfs_sbunicode.h"
#include "lf_hfs_xattr.h"
#include "lf_hfs_unicode_wrappers.h"
#include "lf_hfs_link.h"
#include "lf_hfs_btree.h"
#include "lf_hfs_journal.h"

static int hfs_late_journal_init(struct hfsmount *hfsmp, HFSPlusVolumeHeader *vhp, void *_args);
u_int32_t GetFileInfo(ExtendedVCB *vcb, const char *name,
                      struct cat_attr *fattr, struct cat_fork *forkinfo);

//*******************************************************************************
//    Routine:    hfs_MountHFSVolume
//
//
//*******************************************************************************
unsigned char hfs_catname[] = "Catalog B-tree";
unsigned char hfs_extname[] = "Extents B-tree";
unsigned char hfs_vbmname[] = "Volume Bitmap";
unsigned char hfs_attrname[] = "Attribute B-tree";
unsigned char hfs_startupname[] = "Startup File";

//*******************************************************************************
//
// Sanity check Volume Header Block:
//        Input argument *vhp is a pointer to a HFSPlusVolumeHeader block that has
//        not been endian-swapped and represents the on-disk contents of this sector.
//        This routine will not change the endianness of vhp block.
//
//*******************************************************************************
int hfs_ValidateHFSPlusVolumeHeader(struct hfsmount *hfsmp, HFSPlusVolumeHeader *vhp)
{
    u_int16_t signature = SWAP_BE16(vhp->signature);
    u_int16_t hfs_version = SWAP_BE16(vhp->version);

    if (signature == kHFSPlusSigWord)
    {
        if (hfs_version != kHFSPlusVersion)
        {
            LFHFS_LOG(LEVEL_ERROR, "hfs_ValidateHFSPlusVolumeHeader: invalid HFS+ version: %x\n", hfs_version);

            return (EINVAL);
        }
    } else if (signature == kHFSXSigWord)
    {
        if (hfs_version != kHFSXVersion)
        {
            LFHFS_LOG(LEVEL_ERROR, "hfs_ValidateHFSPlusVolumeHeader: invalid HFSX version: %x\n", hfs_version);
            return (EINVAL);
        }
    } else
    {
        /* Removed printf for invalid HFS+ signature because it gives
         * false error for UFS root volume
         */
        LFHFS_LOG(LEVEL_DEBUG, "hfs_ValidateHFSPlusVolumeHeader: unknown Volume Signature : %x\n", signature);
        return (EINVAL);
    }

    /* Block size must be at least 512 and a power of 2 */
    u_int32_t blockSize = SWAP_BE32(vhp->blockSize);
    if (blockSize < 512 || !powerof2(blockSize))
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_ValidateHFSPlusVolumeHeader: invalid blocksize (%d) \n", blockSize);
        return (EINVAL);
    }

    if (blockSize < hfsmp->hfs_logical_block_size)
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_ValidateHFSPlusVolumeHeader: invalid physical blocksize (%d), hfs_logical_blocksize (%d) \n",
                blockSize, hfsmp->hfs_logical_block_size);
        return (EINVAL);
    }
    return 0;
}

//*******************************************************************************
//    Routine:    hfs_MountHFSPlusVolume
//
//
//*******************************************************************************

int hfs_CollectBtreeStats(struct hfsmount *hfsmp, HFSPlusVolumeHeader *vhp, off_t embeddedOffset, void *args)
{
    int retval = 0;
    register ExtendedVCB *vcb = HFSTOVCB(hfsmp);
    u_int32_t blockSize; blockSize = SWAP_BE32(vhp->blockSize);

    /*
     * pull in the volume UUID while we are still single-threaded.
     * This brings the volume UUID into the cached one dangling off of the HFSMP
     * Otherwise it would have to be computed on first access.
     */
    uuid_t throwaway;
    hfs_getvoluuid (hfsmp, throwaway);

    /*
     * We now always initiate a full bitmap scan even if the volume is read-only because this is
     * our only shot to do I/Os of dramaticallly different sizes than what the buffer cache ordinarily
     * expects. TRIMs will not be delivered to the underlying media if the volume is not
     * read-write though.
     */
    hfsmp->scan_var = 0;

    hfs_scan_blocks(hfsmp);

    if (hfsmp->jnl && (hfsmp->hfs_flags & HFS_READ_ONLY) == 0)
    {
        hfs_flushvolumeheader(hfsmp, 0);
    }

    /* kHFSHasFolderCount is only supported/updated on HFSX volumes */
    if ((hfsmp->hfs_flags & HFS_X) != 0)
    {
        hfsmp->hfs_flags |= HFS_FOLDERCOUNT;
    }

    // Check if we need to do late journal initialization.  This only
    // happens if a previous version of MacOS X (or 9) touched the disk.
    // In that case hfs_late_journal_init() will go re-locate the journal
    // and journal_info_block files and validate that they're still kosher.
    if ( (vcb->vcbAtrb & kHFSVolumeJournaledMask) &&
        (SWAP_BE32(vhp->lastMountedVersion) != kHFSJMountVersion) &&
        (hfsmp->jnl == NULL))
    {

        retval = hfs_late_journal_init(hfsmp, vhp, args);
        if (retval != 0)
        {
            if (retval == EROFS)
            {
                // EROFS is a special error code that means the volume has an external
                // journal which we couldn't find.  in that case we do not want to
                // rewrite the volume header - we'll just refuse to mount the volume.
                LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: hfs_late_journal_init returned (%d), maybe an external jnl?\n", retval);
                retval = EINVAL;
                goto ErrorExit;
            }

            hfsmp->jnl = NULL;

            // if the journal failed to open, then set the lastMountedVersion
            // to be "FSK!" which fsck_hfs will see and force the fsck instead
            // of just bailing out because the volume is journaled.
            if (!(hfsmp->hfs_flags & HFS_READ_ONLY))
            {
                hfsmp->hfs_flags |= HFS_NEED_JNL_RESET;

                uint64_t mdb_offset = (uint64_t)((embeddedOffset / blockSize) + HFS_PRI_SECTOR(blockSize));

                void *pvBuffer = hfs_malloc(hfsmp->hfs_physical_block_size);
                if (pvBuffer == NULL)
                {
                    retval = ENOMEM;
                    goto ErrorExit;
                }

                retval = raw_readwrite_read_mount( hfsmp->hfs_devvp, HFS_PHYSBLK_ROUNDDOWN(mdb_offset, hfsmp->hfs_log_per_phys), hfsmp->hfs_physical_block_size, pvBuffer, hfsmp->hfs_physical_block_size, NULL, NULL);
                if (retval)
                {
                    LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: JNL header raw_readwrite_read_mount failed with %d\n", retval);
                    hfs_free(pvBuffer);
                    goto ErrorExit;
                }

                HFSPlusVolumeHeader *jvhp = (HFSPlusVolumeHeader *)(pvBuffer + HFS_PRI_OFFSET(hfsmp->hfs_physical_block_size));

                if (SWAP_BE16(jvhp->signature) == kHFSPlusSigWord || SWAP_BE16(jvhp->signature) == kHFSXSigWord)
                {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_MountHFSPlusVolume: Journal replay fail.  Writing lastMountVersion as FSK!\n");
                    jvhp->lastMountedVersion = SWAP_BE32(kFSKMountVersion);

                    retval = raw_readwrite_write_mount( hfsmp->hfs_devvp, HFS_PHYSBLK_ROUNDDOWN(mdb_offset, hfsmp->hfs_log_per_phys), hfsmp->hfs_physical_block_size, pvBuffer, hfsmp->hfs_physical_block_size, NULL, NULL);
                    hfs_free(pvBuffer);
                    if (retval)
                    {
                        LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: JNL header raw_readwrite_write_mount failed with %d\n", retval);
                        goto ErrorExit;
                    }
                }
                else
                {
                    hfs_free(pvBuffer);
                }
            }

            LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: hfs_late_journal_init returned (%d)\n", retval);
            retval = EINVAL;
            goto ErrorExit;
        }
        else if (hfsmp->jnl)
        {
            hfsmp->hfs_mp->mnt_flag |= MNT_JOURNALED;
        }
    }
    else if (hfsmp->jnl || ((vcb->vcbAtrb & kHFSVolumeJournaledMask) && (hfsmp->hfs_flags & HFS_READ_ONLY)))
    {
        struct cat_attr jinfo_attr, jnl_attr;
        if (hfsmp->hfs_flags & HFS_READ_ONLY)
        {
            vcb->vcbAtrb &= ~kHFSVolumeJournaledMask;
        }

        // if we're here we need to fill in the fileid's for the
        // journal and journal_info_block.
        hfsmp->hfs_jnlinfoblkid = GetFileInfo(vcb, ".journal_info_block", &jinfo_attr, NULL);
        hfsmp->hfs_jnlfileid    = GetFileInfo(vcb, ".journal", &jnl_attr, NULL);
        if (hfsmp->hfs_jnlinfoblkid == 0 || hfsmp->hfs_jnlfileid == 0)
        {
            LFHFS_LOG(LEVEL_DEFAULT, "hfs_MountHFSPlusVolume: danger! couldn't find the file-id's for the journal or journal_info_block\n");
            LFHFS_LOG(LEVEL_DEFAULT, "hfs_MountHFSPlusVolume: jnlfileid %llu, jnlinfoblkid %llu\n", hfsmp->hfs_jnlfileid, hfsmp->hfs_jnlinfoblkid);
        }

        if (hfsmp->hfs_flags & HFS_READ_ONLY)
        {
            vcb->vcbAtrb |= kHFSVolumeJournaledMask;
        }

        if (hfsmp->jnl == NULL)
        {
            hfsmp->hfs_mp->mnt_flag &= ~(u_int64_t)((unsigned int)MNT_JOURNALED);
        }
    }

    if ( !(vcb->vcbAtrb & kHFSVolumeHardwareLockMask) )    // if the disk is not write protected
    {
        MarkVCBDirty( vcb );    // mark VCB dirty so it will be written
    }

    /*
     * Distinguish 3 potential cases involving content protection:
     * 1. mount point bit set; vcbAtrb does not support it. Fail.
     * 2. mount point bit set; vcbattrb supports it. we're good.
     * 3. mount point bit not set; vcbatrb supports it, turn bit on, then good.
     */
    if (hfsmp->hfs_mp->mnt_flag & MNT_CPROTECT)
    {
        /* Does the mount point support it ? */
        if ((vcb->vcbAtrb & kHFSContentProtectionMask) == 0)
        {
            /* Case 1 above */
            retval = EINVAL;
            goto ErrorExit;
        }
    }
    else
    {
        /* not requested in the mount point. Is it in FS? */
        if (vcb->vcbAtrb & kHFSContentProtectionMask)
        {
            /* Case 3 above */
            hfsmp->hfs_mp->mnt_flag |= MNT_CPROTECT;
        }
    }

#if LF_HFS_CHECK_UNMAPPED // TBD:
    /*
     * Establish a metadata allocation zone.
     */
    hfs_metadatazone_init(hfsmp, false);


    /*
     * Make any metadata zone adjustments.
     */
    if (hfsmp->hfs_flags & HFS_METADATA_ZONE)
    {
        /* Keep the roving allocator out of the metadata zone. */
        if (vcb->nextAllocation >= hfsmp->hfs_metazone_start &&
            vcb->nextAllocation <= hfsmp->hfs_metazone_end)
        {
            HFS_UPDATE_NEXT_ALLOCATION(hfsmp, hfsmp->hfs_metazone_end + 1);
        }
    }
    else
#endif
    {
        if (vcb->nextAllocation <= 1)
        {
            vcb->nextAllocation = hfsmp->hfs_min_alloc_start;
        }
    }
    vcb->sparseAllocation = hfsmp->hfs_min_alloc_start;

    /* Setup private/hidden directories for hardlinks. */
    hfs_privatedir_init(hfsmp, FILE_HARDLINKS);
    hfs_privatedir_init(hfsmp, DIR_HARDLINKS);

    hfs_remove_orphans(hfsmp);

    /* See if we need to erase unused Catalog nodes due to <rdar://problem/6947811>. */
    retval = hfs_erase_unused_nodes(hfsmp);
    if (retval)
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: hfs_erase_unused_nodes returned (%d) for %s \n", retval, hfsmp->vcbVN);
        goto ErrorExit;
    }

    /* Enable extent-based extended attributes by default */
    hfsmp->hfs_flags |= HFS_XATTR_EXTENTS;

    return (0);

ErrorExit:
    /*
     * A fatal error occurred and the volume cannot be mounted, so
     * release any resources that we acquired...
     */
    hfsUnmount(hfsmp);

    LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: encountered error (%d)\n", retval);

    return (retval);
}


int hfs_MountHFSPlusVolume(struct hfsmount *hfsmp, HFSPlusVolumeHeader *vhp, off_t embeddedOffset, u_int64_t disksize, bool bFailForDirty)
{
    int retval = 0;

    register ExtendedVCB *vcb;
    struct cat_desc cndesc;
    struct cat_attr cnattr;
    struct cat_fork cfork;
    u_int32_t blockSize;
    uint64_t spare_sectors;
    int newvnode_flags = 0;
    BTreeInfoRec btinfo;

    u_int16_t signature = SWAP_BE16(vhp->signature);

    retval = hfs_ValidateHFSPlusVolumeHeader(hfsmp, vhp);
    if (retval)
        return retval;

    if (signature == kHFSXSigWord)
    {
        /* The in-memory signature is always 'H+'. */
        signature = kHFSPlusSigWord;
        hfsmp->hfs_flags |= HFS_X;
    }

    blockSize = SWAP_BE32(vhp->blockSize);
    /* don't mount a writable volume if its dirty, it must be cleaned by fsck_hfs */
    if ((hfsmp->hfs_flags & HFS_READ_ONLY) == 0 &&
        hfsmp->jnl == NULL &&
        (SWAP_BE32(vhp->attributes) & kHFSVolumeUnmountedMask) == 0 &&
        bFailForDirty)
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: cannot mount dirty non-journaled volumes\n");
        return (EINVAL);
    }

    /* Make sure we can live with the physical block size. */
    if ((disksize & (hfsmp->hfs_logical_block_size - 1)) ||
        (embeddedOffset & (hfsmp->hfs_logical_block_size - 1)))
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: hfs_logical_blocksize (%d) \n",hfsmp->hfs_logical_block_size);
        return (ENXIO);
    }

    /*
     * If allocation block size is less than the physical block size,
     * same data could be cached in two places and leads to corruption.
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
     * The same logic to ensure good hfs_physical_block_size is also in
     * hfs_mountfs so that hfs_mountfs, hfs_MountHFSPlusVolume and
     * later are doing the I/Os using same block size.
     */
    if (blockSize < hfsmp->hfs_physical_block_size)
    {
        hfsmp->hfs_physical_block_size = hfsmp->hfs_logical_block_size;
        hfsmp->hfs_log_per_phys = 1;
    }

    /*
     * The VolumeHeader seems OK: transfer info from it into VCB
     * Note - the VCB starts out clear (all zeros)
     */
    vcb = HFSTOVCB(hfsmp);

    vcb->vcbSigWord     = signature;
    vcb->vcbJinfoBlock  = SWAP_BE32(vhp->journalInfoBlock);
    vcb->vcbLsMod       = to_bsd_time(SWAP_BE32(vhp->modifyDate));
    vcb->vcbAtrb        = SWAP_BE32(vhp->attributes);
    vcb->vcbClpSiz      = SWAP_BE32(vhp->rsrcClumpSize);
    vcb->vcbNxtCNID     = SWAP_BE32(vhp->nextCatalogID);
    vcb->vcbVolBkUp     = to_bsd_time(SWAP_BE32(vhp->backupDate));
    vcb->vcbWrCnt       = SWAP_BE32(vhp->writeCount);
    vcb->vcbFilCnt      = SWAP_BE32(vhp->fileCount);
    vcb->vcbDirCnt      = SWAP_BE32(vhp->folderCount);

    /* copy 32 bytes of Finder info */
    bcopy(vhp->finderInfo, vcb->vcbFndrInfo, sizeof(vhp->finderInfo));

    vcb->vcbAlBlSt = 0;        /* hfs+ allocation blocks start at first block of volume */
    if ((hfsmp->hfs_flags & HFS_READ_ONLY) == 0)
    {
        vcb->vcbWrCnt++;    /* compensate for write of Volume Header on last flush */
    }

    /* Now fill in the Extended VCB info */
    vcb->nextAllocation     = SWAP_BE32(vhp->nextAllocation);
    vcb->totalBlocks        = SWAP_BE32(vhp->totalBlocks);
    vcb->allocLimit         = vcb->totalBlocks;
    vcb->freeBlocks         = SWAP_BE32(vhp->freeBlocks);
    vcb->blockSize          = blockSize;
    vcb->encodingsBitmap    = SWAP_BE64(vhp->encodingsBitmap);
    vcb->localCreateDate    = SWAP_BE32(vhp->createDate);

    vcb->hfsPlusIOPosOffset    = (uint32_t) embeddedOffset;

    /* Default to no free block reserve */
    vcb->reserveBlocks = 0;

    /*
     * Update the logical block size in the mount struct
     * (currently set up from the wrapper MDB) using the
     * new blocksize value:
     */
    hfsmp->hfs_logBlockSize = BestBlockSizeFit(vcb->blockSize, MAXBSIZE, hfsmp->hfs_logical_block_size);
    vcb->vcbVBMIOSize = MIN(vcb->blockSize, MAXPHYSIO);

    /*
     * Validate and initialize the location of the alternate volume header.
     *
     * Note that there may be spare sectors beyond the end of the filesystem that still
     * belong to our partition.
     */
    spare_sectors = hfsmp->hfs_logical_block_count - (((uint64_t)vcb->totalBlocks * blockSize) / hfsmp->hfs_logical_block_size);

    /*
     * Differentiate between "innocuous" spare sectors and the more unusual
     * degenerate case:
     *
     * *** Innocuous spare sectors exist if:
     *
     * A) the number of bytes assigned to the partition (by multiplying logical
     * block size * logical block count) is greater than the filesystem size
     * (by multiplying allocation block count and allocation block size)
     *
     * and
     *
     * B) the remainder is less than the size of a full allocation block's worth of bytes.
     *
     * This handles the normal case where there may be a few extra sectors, but the two
     * are fundamentally in sync.
     *
     * *** Degenerate spare sectors exist if:
     * A) The number of bytes assigned to the partition (by multiplying logical
     * block size * logical block count) is greater than the filesystem size
     * (by multiplying allocation block count and block size).
     *
     * and
     *
     * B) the remainder is greater than a full allocation's block worth of bytes.
     * In this case,  a smaller file system exists in a larger partition.
     * This can happen in various ways, including when volume is resized but the
     * partition is yet to be resized.  Under this condition, we have to assume that
     * a partition management software may resize the partition to match
     * the file system size in the future.  Therefore we should update
     * alternate volume header at two locations on the disk,
     *   a. 1024 bytes before end of the partition
     *   b. 1024 bytes before end of the file system
     */

    if (spare_sectors > (uint64_t)(blockSize / hfsmp->hfs_logical_block_size))
    {
        /*
         * Handle the degenerate case above. FS < partition size.
         * AVH located at 1024 bytes from the end of the partition
         */
        hfsmp->hfs_partition_avh_sector = (hfsmp->hfsPlusIOPosOffset / hfsmp->hfs_logical_block_size) + HFS_ALT_SECTOR(hfsmp->hfs_logical_block_size, hfsmp->hfs_logical_block_count);

        /* AVH located at 1024 bytes from the end of the filesystem */
        hfsmp->hfs_fs_avh_sector = (hfsmp->hfsPlusIOPosOffset / hfsmp->hfs_logical_block_size) + HFS_ALT_SECTOR(hfsmp->hfs_logical_block_size, (((uint64_t)vcb->totalBlocks * blockSize) / hfsmp->hfs_logical_block_size));
    }
    else
    {
        /* Innocuous spare sectors; Partition & FS notion are in sync */
        hfsmp->hfs_partition_avh_sector = (hfsmp->hfsPlusIOPosOffset / hfsmp->hfs_logical_block_size) + HFS_ALT_SECTOR(hfsmp->hfs_logical_block_size, hfsmp->hfs_logical_block_count);

        hfsmp->hfs_fs_avh_sector = hfsmp->hfs_partition_avh_sector;
    }

    LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: partition_avh_sector=%qu, fs_avh_sector=%qu\n", hfsmp->hfs_partition_avh_sector, hfsmp->hfs_fs_avh_sector);

    bzero(&cndesc, sizeof(cndesc));
    cndesc.cd_parentcnid = kHFSRootParentID;
    cndesc.cd_flags |= CD_ISMETA;
    bzero(&cnattr, sizeof(cnattr));
    cnattr.ca_linkcount = 1;
    cnattr.ca_mode = S_IFREG;

    /*
     * Set up Extents B-tree vnode
     */
    cndesc.cd_nameptr = hfs_extname;
    cndesc.cd_namelen = strlen((char *)hfs_extname);
    cndesc.cd_cnid = cnattr.ca_fileid = kHFSExtentsFileID;

    cfork.cf_size    = SWAP_BE64 (vhp->extentsFile.logicalSize);
    cfork.cf_new_size= 0;
    cfork.cf_clump   = SWAP_BE32 (vhp->extentsFile.clumpSize);
    cfork.cf_blocks  = SWAP_BE32 (vhp->extentsFile.totalBlocks);
    cfork.cf_vblocks = 0;
    cnattr.ca_blocks = cfork.cf_blocks;

    for (int iExtentCounter = 0; iExtentCounter < kHFSPlusExtentDensity; iExtentCounter++)
    {
        cfork.cf_extents[iExtentCounter].startBlock = SWAP_BE32 (vhp->extentsFile.extents[iExtentCounter].startBlock);
        cfork.cf_extents[iExtentCounter].blockCount = SWAP_BE32 (vhp->extentsFile.extents[iExtentCounter].blockCount);
    }

    retval = hfs_getnewvnode(hfsmp, NULL, NULL, &cndesc, 0, &cnattr, &cfork, &hfsmp->hfs_extents_vp, &newvnode_flags);
    if (retval)
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: hfs_getnewvnode returned (%d) getting extentoverflow BT\n", retval);
        goto ErrorExit;
    }

    hfsmp->hfs_extents_cp = VTOC(hfsmp->hfs_extents_vp);
    retval = MacToVFSError(BTOpenPath(VTOF(hfsmp->hfs_extents_vp), (KeyCompareProcPtr) CompareExtentKeysPlus));

    hfs_unlock(hfsmp->hfs_extents_cp);

    if (retval)
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: BTOpenPath returned (%d) getting extentoverflow BT\n", retval);
        goto ErrorExit;
    }

    /*
     * Set up Catalog B-tree vnode
     */
    cndesc.cd_nameptr = hfs_catname;
    cndesc.cd_namelen = strlen((char *)hfs_catname);
    cndesc.cd_cnid = cnattr.ca_fileid = kHFSCatalogFileID;

    cfork.cf_size    = SWAP_BE64 (vhp->catalogFile.logicalSize);
    cfork.cf_clump   = SWAP_BE32 (vhp->catalogFile.clumpSize);
    cfork.cf_blocks  = SWAP_BE32 (vhp->catalogFile.totalBlocks);
    cfork.cf_vblocks = 0;
    cnattr.ca_blocks = cfork.cf_blocks;

    for (int iExtentCounter = 0; iExtentCounter < kHFSPlusExtentDensity; iExtentCounter++)
    {
        cfork.cf_extents[iExtentCounter].startBlock = SWAP_BE32 (vhp->catalogFile.extents[iExtentCounter].startBlock);
        cfork.cf_extents[iExtentCounter].blockCount = SWAP_BE32 (vhp->catalogFile.extents[iExtentCounter].blockCount);
    }

    retval = hfs_getnewvnode(hfsmp, NULL, NULL, &cndesc, 0, &cnattr, &cfork, &hfsmp->hfs_catalog_vp, &newvnode_flags);
    if (retval)
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: hfs_getnewvnode returned (%d) getting catalog BT\n", retval);
        goto ErrorExit;
    }
    hfsmp->hfs_catalog_cp = VTOC(hfsmp->hfs_catalog_vp);
    retval = MacToVFSError(BTOpenPath(VTOF(hfsmp->hfs_catalog_vp), (KeyCompareProcPtr) CompareExtendedCatalogKeys));

    if (retval)
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: BTOpenPath returned (%d) getting catalog BT\n", retval);
        hfs_unlock(hfsmp->hfs_catalog_cp);
        goto ErrorExit;
    }

    if ((hfsmp->hfs_flags & HFS_X) &&
        BTGetInformation(VTOF(hfsmp->hfs_catalog_vp), 0, &btinfo) == 0)
    {
        if (btinfo.keyCompareType == kHFSBinaryCompare)
        {
            hfsmp->hfs_flags |= HFS_CASE_SENSITIVE;
            /* Install a case-sensitive key compare */
            (void) BTOpenPath(VTOF(hfsmp->hfs_catalog_vp), (KeyCompareProcPtr)cat_binarykeycompare);
        }
    }

    hfs_unlock(hfsmp->hfs_catalog_cp);

    /*
     * Set up Allocation file vnode
     */
    cndesc.cd_nameptr = hfs_vbmname;
    cndesc.cd_namelen = strlen((char *)hfs_vbmname);
    cndesc.cd_cnid = cnattr.ca_fileid = kHFSAllocationFileID;

    cfork.cf_size    = SWAP_BE64 (vhp->allocationFile.logicalSize);
    cfork.cf_clump   = SWAP_BE32 (vhp->allocationFile.clumpSize);
    cfork.cf_blocks  = SWAP_BE32 (vhp->allocationFile.totalBlocks);
    cfork.cf_vblocks = 0;
    cnattr.ca_blocks = cfork.cf_blocks;

    for (int iExtentCounter = 0; iExtentCounter < kHFSPlusExtentDensity; iExtentCounter++)
    {
        cfork.cf_extents[iExtentCounter].startBlock = SWAP_BE32 (vhp->allocationFile.extents[iExtentCounter].startBlock);
        cfork.cf_extents[iExtentCounter].blockCount = SWAP_BE32 (vhp->allocationFile.extents[iExtentCounter].blockCount);
    }

    retval = hfs_getnewvnode(hfsmp, NULL, NULL, &cndesc, 0, &cnattr, &cfork, &hfsmp->hfs_allocation_vp, &newvnode_flags);
    if (retval)
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: hfs_getnewvnode returned (%d) getting bitmap\n", retval);
        goto ErrorExit;
    }
    hfsmp->hfs_allocation_cp = VTOC(hfsmp->hfs_allocation_vp);
    hfs_unlock(hfsmp->hfs_allocation_cp);

    /*
     * Set up Attribute B-tree vnode
     */
    if (vhp->attributesFile.totalBlocks != 0) {
        cndesc.cd_nameptr = hfs_attrname;
        cndesc.cd_namelen = strlen((char *)hfs_attrname);
        cndesc.cd_cnid = cnattr.ca_fileid = kHFSAttributesFileID;

        cfork.cf_size    = SWAP_BE64 (vhp->attributesFile.logicalSize);
        cfork.cf_clump   = SWAP_BE32 (vhp->attributesFile.clumpSize);
        cfork.cf_blocks  = SWAP_BE32 (vhp->attributesFile.totalBlocks);
        cfork.cf_vblocks = 0;
        cnattr.ca_blocks = cfork.cf_blocks;
        
        for (int iExtentCounter = 0; iExtentCounter < kHFSPlusExtentDensity; iExtentCounter++)
        {
            cfork.cf_extents[iExtentCounter].startBlock = SWAP_BE32 (vhp->attributesFile.extents[iExtentCounter].startBlock);
            cfork.cf_extents[iExtentCounter].blockCount = SWAP_BE32 (vhp->attributesFile.extents[iExtentCounter].blockCount);
        }
        retval = hfs_getnewvnode(hfsmp, NULL, NULL, &cndesc, 0, &cnattr, &cfork, &hfsmp->hfs_attribute_vp, &newvnode_flags);
        if (retval)
        {
            LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: hfs_getnewvnode returned (%d) getting EA BT\n", retval);
            goto ErrorExit;
        }
        hfsmp->hfs_attribute_cp = VTOC(hfsmp->hfs_attribute_vp);

        retval = MacToVFSError(BTOpenPath(VTOF(hfsmp->hfs_attribute_vp),(KeyCompareProcPtr) hfs_attrkeycompare));
        hfs_unlock(hfsmp->hfs_attribute_cp);
        if (retval)
        {
            LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: BTOpenPath returned (%d) getting EA BT\n", retval);
            goto ErrorExit;
        }

        /* Initialize vnode for virtual attribute data file that spans the
         * entire file system space for performing I/O to attribute btree
         * We hold iocount on the attrdata vnode for the entire duration
         * of mount (similar to btree vnodes)
         */
        retval = init_attrdata_vnode(hfsmp);
        if (retval)
        {
            LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: init_attrdata_vnode returned (%d) for virtual EA file\n", retval);
            goto ErrorExit;
        }
    }

    /*
     * Set up Startup file vnode
     */
    if (vhp->startupFile.totalBlocks != 0) {
        cndesc.cd_nameptr = hfs_startupname;
        cndesc.cd_namelen = strlen((char *)hfs_startupname);
        cndesc.cd_cnid = cnattr.ca_fileid = kHFSStartupFileID;

        cfork.cf_size    = SWAP_BE64 (vhp->startupFile.logicalSize);
        cfork.cf_clump   = SWAP_BE32 (vhp->startupFile.clumpSize);
        cfork.cf_blocks  = SWAP_BE32 (vhp->startupFile.totalBlocks);
        cfork.cf_vblocks = 0;
        cnattr.ca_blocks = cfork.cf_blocks;
        for (int iExtentCounter = 0; iExtentCounter < kHFSPlusExtentDensity; iExtentCounter++)
        {
            cfork.cf_extents[iExtentCounter].startBlock = SWAP_BE32 (vhp->startupFile.extents[iExtentCounter].startBlock);
            cfork.cf_extents[iExtentCounter].blockCount = SWAP_BE32 (vhp->startupFile.extents[iExtentCounter].blockCount);
        }

        retval = hfs_getnewvnode(hfsmp, NULL, NULL, &cndesc, 0, &cnattr, &cfork, &hfsmp->hfs_startup_vp, &newvnode_flags);
        if (retval)
        {
            LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: hfs_getnewvnode returned (%d) getting startup file\n", retval);
            goto ErrorExit;
        }
        hfsmp->hfs_startup_cp = VTOC(hfsmp->hfs_startup_vp);
        hfs_unlock(hfsmp->hfs_startup_cp);
    }

    /*
     * Pick up volume name and create date
     *
     * Acquiring the volume name should not manipulate the bitmap, only the catalog
     * btree and possibly the extents overflow b-tree.
     */
    retval = cat_idlookup(hfsmp, kHFSRootFolderID, 0, 0, &cndesc, &cnattr, NULL);
    if (retval)
    {
        LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: cat_idlookup returned (%d) getting rootfolder \n", retval);
        goto ErrorExit;
    }
    vcb->hfs_itime = cnattr.ca_itime;
    vcb->volumeNameEncodingHint = cndesc.cd_encoding;
    bcopy(cndesc.cd_nameptr, vcb->vcbVN, min(255, cndesc.cd_namelen));
    cat_releasedesc(&cndesc);

    return (0);

ErrorExit:
    /*
     * A fatal error occurred and the volume cannot be mounted, so
     * release any resources that we acquired...
     */
    hfsUnmount(hfsmp);

    LFHFS_LOG(LEVEL_DEBUG, "hfs_MountHFSPlusVolume: encountered error (%d)\n", retval);

    return (retval);
}

u_int32_t BestBlockSizeFit(u_int32_t allocationBlockSize, u_int32_t blockSizeLimit, u_int32_t baseMultiple) {
    /*
     Compute the optimal (largest) block size (no larger than allocationBlockSize) that is less than the
     specified limit but still an even multiple of the baseMultiple.
     */
    int baseBlockCount, blockCount;
    u_int32_t trialBlockSize;

    if (allocationBlockSize % baseMultiple != 0) {
        /*
         Whoops: the allocation blocks aren't even multiples of the specified base:
         no amount of dividing them into even parts will be a multiple, either then!
         */
        return 512;        /* Hope for the best */
    };

    /* Try the obvious winner first, to prevent 12K allocation blocks, for instance,
     from being handled as two 6K logical blocks instead of 3 4K logical blocks.
     Even though the former (the result of the loop below) is the larger allocation
     block size, the latter is more efficient: */
    if (allocationBlockSize % PAGE_SIZE == 0) return (u_int32_t)PAGE_SIZE;

    /* No clear winner exists: pick the largest even fraction <= MAXBSIZE: */
    baseBlockCount = allocationBlockSize / baseMultiple;                /* Now guaranteed to be an even multiple */

    for (blockCount = baseBlockCount; blockCount > 0; --blockCount) {
        trialBlockSize = blockCount * baseMultiple;
        if (allocationBlockSize % trialBlockSize == 0) {                /* An even multiple? */
            if ((trialBlockSize <= blockSizeLimit) &&
                (trialBlockSize % baseMultiple == 0)) {
                return trialBlockSize;
            };
        };
    };

    /* Note: we should never get here, since blockCount = 1 should always work,
     but this is nice and safe and makes the compiler happy, too ... */
    return 512;
}

/*
 * Lock the HFS global journal lock
 */
int
hfs_lock_global (struct hfsmount *hfsmp, enum hfs_locktype locktype)
{
    pthread_t thread = pthread_self();

    if (hfsmp->hfs_global_lockowner == thread) {
        LFHFS_LOG(LEVEL_ERROR, "hfs_lock_global: locking against myself!");
        hfs_assert(0);
    }

    if (locktype == HFS_SHARED_LOCK) {
        lf_lck_rw_lock_shared (&hfsmp->hfs_global_lock);
        hfsmp->hfs_global_lockowner = HFS_SHARED_OWNER;
    }
    else {
        lf_lck_rw_lock_exclusive (&hfsmp->hfs_global_lock);
        hfsmp->hfs_global_lockowner = thread;
    }

    return 0;
}

/*
 * Unlock the HFS global journal lock
 */
void
hfs_unlock_global (struct hfsmount *hfsmp)
{
    pthread_t thread = pthread_self();

    /* HFS_LOCK_EXCLUSIVE */
    if (hfsmp->hfs_global_lockowner == thread) {
        hfsmp->hfs_global_lockowner = NULL;
        lf_lck_rw_unlock_exclusive(&hfsmp->hfs_global_lock);
    }
    /* HFS_LOCK_SHARED */
    else {
        lf_lck_rw_unlock_shared(&hfsmp->hfs_global_lock);
    }
}

int
hfs_start_transaction(struct hfsmount *hfsmp)
{
    int ret = 0, unlock_on_err = 0;
    pthread_t thread = pthread_self();

#ifdef HFS_CHECK_LOCK_ORDER
    /*
     * You cannot start a transaction while holding a system
     * file lock. (unless the transaction is nested.)
     */
    if (hfsmp->jnl && journal_owner(hfsmp->jnl) != thread) {
        if (hfsmp->hfs_catalog_cp && hfsmp->hfs_catalog_cp->c_lockowner == thread) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_start_transaction: bad lock order (cat before jnl)\n");
            hfs_assert(0);
        }
        if (hfsmp->hfs_attribute_cp && hfsmp->hfs_attribute_cp->c_lockowner == thread) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_start_transaction: bad lock order (attr before jnl)\n");
            hfs_assert(0);
        }
        if (hfsmp->hfs_extents_cp && hfsmp->hfs_extents_cp->c_lockowner == thread) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_start_transaction: bad lock order (ext before jnl)\n");
            hfs_assert(0);
        }
    }
#endif /* HFS_CHECK_LOCK_ORDER */

again:

    if (hfsmp->jnl) {
        if (journal_owner(hfsmp->jnl) != thread)
        {
            /*
             * The global lock should be held shared if journal is
             * active to prevent disabling.  If we're not the owner
             * of the journal lock, verify that we're not already
             * holding the global lock exclusive before moving on.
             */
            if (hfsmp->hfs_global_lockowner == thread) {
                ret = EBUSY;
                goto out;
            }

            hfs_lock_global (hfsmp, HFS_SHARED_LOCK);

            // Things could have changed
            if (!hfsmp->jnl) {
                hfs_unlock_global(hfsmp);
                goto again;
            }
            unlock_on_err = 1;
        }
    }
    else
    {
        // No journal
        if (hfsmp->hfs_global_lockowner != thread) {
            hfs_lock_global(hfsmp, HFS_EXCLUSIVE_LOCK);

            // Things could have changed
            if (hfsmp->jnl) {
                hfs_unlock_global(hfsmp);
                goto again;
            }

            ExtendedVCB * vcb = HFSTOVCB(hfsmp);
            if (vcb->vcbAtrb & kHFSVolumeUnmountedMask) {
                // clear kHFSVolumeUnmountedMask
                hfs_flushvolumeheader(hfsmp, HFS_FVH_SKIP_TRANSACTION);
            }
            unlock_on_err = 1;
        }
    }

    if (hfsmp->jnl)
    {
        ret = journal_start_transaction(hfsmp->jnl);
    }
    else
    {
        ret = 0;
    }

    if (ret == 0)
        ++hfsmp->hfs_transaction_nesting;

    goto out;

out:
    if (ret != 0 && unlock_on_err) {
        hfs_unlock_global (hfsmp);
    }

    return ret;
}

int
hfs_end_transaction(struct hfsmount *hfsmp)
{
    int ret;

    hfs_assert(!hfsmp->jnl || journal_owner(hfsmp->jnl) == pthread_self());
    hfs_assert(hfsmp->hfs_transaction_nesting > 0);

    if (hfsmp->jnl && hfsmp->hfs_transaction_nesting == 1)
        hfs_flushvolumeheader(hfsmp, HFS_FVH_FLUSH_IF_DIRTY);

    bool need_unlock = !--hfsmp->hfs_transaction_nesting;

    if (hfsmp->jnl)
    {
        ret = journal_end_transaction(hfsmp->jnl);
    }
    else
    {
        ret = 0;
    }

    if (need_unlock) {
        hfs_unlock_global (hfsmp);
    }

    return ret;
}


/*
 * Flush the contents of the journal to the disk.
 *
 *  - HFS_FLUSH_JOURNAL
 *      Wait to write in-memory journal to the disk consistently.
 *      This means that the journal still contains uncommitted
 *      transactions and the file system metadata blocks in
 *      the journal transactions might be written asynchronously
 *      to the disk.  But there is no guarantee that they are
 *      written to the disk before returning to the caller.
 *      Note that this option is sufficient for file system
 *      data integrity as it guarantees consistent journal
 *      content on the disk.
 *
 *  - HFS_FLUSH_JOURNAL_META
 *      Wait to write in-memory journal to the disk
 *      consistently, and also wait to write all asynchronous
 *      metadata blocks to its corresponding locations
 *      consistently on the disk. This is overkill in normal
 *      scenarios but is useful whenever the metadata blocks
 *      are required to be consistent on-disk instead of
 *      just the journalbeing consistent; like before live
 *      verification and live volume resizing.  The update of the
 *      metadata doesn't include a barrier of track cache flush.
 *
 *  - HFS_FLUSH_FULL
 *      HFS_FLUSH_JOURNAL + force a track cache flush to media
 *
 *  - HFS_FLUSH_CACHE
 *      Force a track cache flush to media.
 *
 *  - HFS_FLUSH_BARRIER
 *      Barrier-only flush to ensure write order
 *
 */
errno_t hfs_flush(struct hfsmount *hfsmp, hfs_flush_mode_t mode) {
    errno_t error  = 0;
    int    options = 0;
    dk_synchronize_t sync_req = { .options = DK_SYNCHRONIZE_OPTION_BARRIER };
    
    switch (mode) {
        case HFS_FLUSH_JOURNAL_META:
            // wait for journal, metadata blocks and previous async flush to finish
            SET(options, JOURNAL_WAIT_FOR_IO);
            
            // no break
            
        case HFS_FLUSH_JOURNAL:
        case HFS_FLUSH_JOURNAL_BARRIER:
        case HFS_FLUSH_FULL:
            
            if (mode == HFS_FLUSH_JOURNAL_BARRIER &&
                !(hfsmp->hfs_flags & HFS_FEATURE_BARRIER))
                mode = HFS_FLUSH_FULL;
            
            if (mode == HFS_FLUSH_FULL)
                SET(options, JOURNAL_FLUSH_FULL);
            
            /* Only peek at hfsmp->jnl while holding the global lock */
            hfs_lock_global (hfsmp, HFS_SHARED_LOCK);
            
            if (hfsmp->jnl) {
                ExtendedVCB * vcb = HFSTOVCB(hfsmp);
                if (!(vcb->vcbAtrb & kHFSVolumeUnmountedMask)) {
                    // Set kHFSVolumeUnmountedMask
                    hfs_flushvolumeheader(hfsmp, HFS_FVH_MARK_UNMOUNT);
                }
                error = journal_flush(hfsmp->jnl, options);
            }
                
            hfs_unlock_global (hfsmp);
            
            /*
             * This may result in a double barrier as
             * journal_flush may have issued a barrier itself
             */
            if (mode == HFS_FLUSH_JOURNAL_BARRIER)
                error = ioctl(hfsmp->hfs_devvp->psFSRecord->iFD, DKIOCSYNCHRONIZE, (caddr_t)&sync_req);
            break;
            
        case HFS_FLUSH_CACHE:
            // Do a full sync
            sync_req.options = 0;
            
            // no break
            
        case HFS_FLUSH_BARRIER:
            // If barrier only flush doesn't support, fall back to use full flush.
            if (!(hfsmp->hfs_flags & HFS_FEATURE_BARRIER))
                sync_req.options = 0;
            
            error = ioctl(hfsmp->hfs_devvp->psFSRecord->iFD, DKIOCSYNCHRONIZE, (caddr_t)&sync_req);
            break;
            
        default:
            error = EINVAL;
    }
    
    return error;
}


#define MALLOC_TRACER 0

#if MALLOC_TRACER
#define MALLOC_TRACER_SIZE 100000
typedef struct {
    void    *pv;
    size_t  uSize;
} MallocTracer_S;
MallocTracer_S gpsMallocTracer[MALLOC_TRACER_SIZE];
MallocTracer_S gpsFreeTracer[MALLOC_TRACER_SIZE];
uint32_t guIndex = 0, guOutdex = 0, guSize=0, guTotal = 0;
uint64_t guTotalConsumption = 0;
#endif

void*
hfs_malloc(size_t size)
{
    if (!size) {
        panic("Malloc size is 0");
    }
    void *pv = malloc(size);
    
#if MALLOC_TRACER
    gpsMallocTracer[guIndex].pv = pv;
    gpsMallocTracer[guIndex].uSize = (uint32_t)size;
    guIndex = (guIndex+1) % MALLOC_TRACER_SIZE;
    guTotal++;
    guSize++;
    guTotalConsumption += size;
#endif
    return pv;
}

void
hfs_free(void *ptr)
{
    if (!ptr)
        return;
    
    free(ptr);
    
#if MALLOC_TRACER
    gpsFreeTracer[guOutdex].pv = ptr;
    bool bCont = true;
    uint32_t u=guIndex;
    do {
        u = (u)?(u-1):(MALLOC_TRACER_SIZE-1);
        if (gpsMallocTracer[u].pv == ptr) {
            break;
        }
        bCont = (guTotal<MALLOC_TRACER_SIZE)?(u):(u != guIndex);
    } while( bCont );
    
    if (!bCont) {
        panic("undetectable free");
        assert(0);
    }
    //gpsFreeTracer[guOutdex].uSize = gpsMallocTracer[u].uSize;
    //gpsFreeTracer[guOutdex].uSize = guSize;
    gpsFreeTracer[guOutdex].uSize = guIndex;
    
    guOutdex = (guOutdex+1) % MALLOC_TRACER_SIZE;
    guSize--;
    guTotalConsumption -= gpsMallocTracer[u].uSize;
#endif
}

void*
hfs_mallocz(size_t size)
{
    void *ptr = hfs_malloc(size);
    if ( ptr == NULL )
        return ptr;
    bzero(ptr, size);
    return ptr;
}

/*
 * Lock the HFS mount lock
 *
 * Note: this is a mutex, not a rw lock!
 */
void
hfs_lock_mount (struct hfsmount *hfsmp)
{
    lf_lck_mtx_lock (&(hfsmp->hfs_mutex));
}

/*
 * Unlock the HFS mount lock
 *
 * Note: this is a mutex, not a rw lock!
 */
void hfs_unlock_mount (struct hfsmount *hfsmp)
{
    lf_lck_mtx_unlock (&(hfsmp->hfs_mutex));
}

/*
 * ReleaseMetaFileVNode
 *
 * vp    L - -
 */
static void ReleaseMetaFileVNode(struct vnode *vp)
{
    struct filefork *fp;

    if (vp && (fp = VTOF(vp)))
    {
        if (fp->fcbBTCBPtr != NULL)
        {
            (void)hfs_lock(VTOC(vp), HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
            (void) BTClosePath(fp);
            hfs_unlock(VTOC(vp));
        }

        /* release the node even if BTClosePath fails */
        hfs_vnop_reclaim(vp);
    }
}

/*************************************************************
 *
 * Unmounts a hfs volume.
 *    At this point vflush() has been called (to dump all non-metadata files)
 *
 *************************************************************/

int
hfsUnmount( register struct hfsmount *hfsmp)
{

    /* Get rid of our attribute data vnode (if any).  This is done
     * after the vflush() during mount, so we don't need to worry
     * about any locks.
     */
    if (hfsmp->hfs_attrdata_vp) {
        ReleaseMetaFileVNode(hfsmp->hfs_attrdata_vp);
        hfsmp->hfs_attrdata_vp = NULL;
    }

    if (hfsmp->hfs_startup_vp) {
        ReleaseMetaFileVNode(hfsmp->hfs_startup_vp);
        hfsmp->hfs_startup_cp = NULL;
        hfsmp->hfs_startup_vp = NULL;
    }

    if (hfsmp->hfs_attribute_vp) {
        ReleaseMetaFileVNode(hfsmp->hfs_attribute_vp);
        hfsmp->hfs_attribute_cp = NULL;
        hfsmp->hfs_attribute_vp = NULL;
    }

    if (hfsmp->hfs_catalog_vp) {
        ReleaseMetaFileVNode(hfsmp->hfs_catalog_vp);
        hfsmp->hfs_catalog_cp = NULL;
        hfsmp->hfs_catalog_vp = NULL;
    }

    if (hfsmp->hfs_extents_vp) {
        ReleaseMetaFileVNode(hfsmp->hfs_extents_vp);
        hfsmp->hfs_extents_cp = NULL;
        hfsmp->hfs_extents_vp = NULL;
    }

    if (hfsmp->hfs_allocation_vp) {
        ReleaseMetaFileVNode(hfsmp->hfs_allocation_vp);
        hfsmp->hfs_allocation_cp = NULL;
        hfsmp->hfs_allocation_vp = NULL;
    }
    return (0);
}

/*
 * RequireFileLock
 *
 * Check to see if a vnode is locked in the current context
 * This is to be used for debugging purposes only!!
 */
void RequireFileLock(FileReference vp, int shareable)
{
    int locked;

    /* The extents btree and allocation bitmap are always exclusive. */
    if (VTOC(vp)->c_fileid == kHFSExtentsFileID ||
        VTOC(vp)->c_fileid == kHFSAllocationFileID) {
        shareable = 0;
    }

    locked = VTOC(vp)->c_lockowner == pthread_self();

    if (!locked && !shareable)
    {
        switch (VTOC(vp)->c_fileid) {
            case kHFSExtentsFileID:
                LFHFS_LOG(LEVEL_ERROR, "RequireFileLock: extents btree not locked! v: 0x%08X\n #\n", (u_int)vp);
                break;
            case kHFSCatalogFileID:
                LFHFS_LOG(LEVEL_ERROR, "RequireFileLock: catalog btree not locked! v: 0x%08X\n #\n", (u_int)vp);
                break;
            case kHFSAllocationFileID:
                /* The allocation file can hide behind the jornal lock. */
                if (VTOHFS(vp)->jnl == NULL)
                {
                    LFHFS_LOG(LEVEL_ERROR, "RequireFileLock: allocation file not locked! v: 0x%08X\n #\n", (u_int)vp);
                }
                return;
            case kHFSStartupFileID:
                LFHFS_LOG(LEVEL_ERROR, "RequireFileLock: startup file not locked! v: 0x%08X\n #\n", (u_int)vp);
                break;
            case kHFSAttributesFileID:
                LFHFS_LOG(LEVEL_ERROR, "RequireFileLock: attributes btree not locked! v: 0x%08X\n #\n", (u_int)vp);
                break;
            default:
                return;
        }
        hfs_assert(0);
    }
}

/*
 * Test if fork has overflow extents.
 *
 * Returns:
 *     non-zero - overflow extents exist
 *     zero     - overflow extents do not exist
 */
bool overflow_extents(struct filefork *fp)
{
    u_int32_t blocks;

    if (fp->ff_extents[7].blockCount == 0)
        return false;

    blocks = fp->ff_extents[0].blockCount +
    fp->ff_extents[1].blockCount +
    fp->ff_extents[2].blockCount +
    fp->ff_extents[3].blockCount +
    fp->ff_extents[4].blockCount +
    fp->ff_extents[5].blockCount +
    fp->ff_extents[6].blockCount +
    fp->ff_extents[7].blockCount;

    return fp->ff_blocks > blocks;
}


/*
 * Lock HFS system file(s).
 *
 * This function accepts a @flags parameter which indicates which
 * system file locks are required.  The value it returns should be
 * used in a subsequent call to hfs_systemfile_unlock.  The caller
 * should treat this value as opaque; it may or may not have a
 * relation to the @flags field that is passed in.  The *only*
 * guarantee that we make is that a value of zero means that no locks
 * were taken and that there is no need to call hfs_systemfile_unlock
 * (although it is harmless to do so).  Recursion is supported but
 * care must still be taken to ensure correct lock ordering.  Note
 * that requests for certain locks may cause other locks to also be
 * taken, including locks that are not possible to ask for via the
 * @flags parameter.
 */
int
hfs_systemfile_lock(struct hfsmount *hfsmp, int flags, enum hfs_locktype locktype)
{
    pthread_t thread = pthread_self();

    /*
     * Locking order is Catalog file, Attributes file, Startup file, Bitmap file, Extents file
     */
    if (flags & SFL_CATALOG) {
        if (hfsmp->hfs_catalog_cp
            && hfsmp->hfs_catalog_cp->c_lockowner != thread) {
#ifdef HFS_CHECK_LOCK_ORDER
            if (hfsmp->hfs_attribute_cp && hfsmp->hfs_attribute_cp->c_lockowner == current_thread()) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_systemfile_lock: bad lock order (Attributes before Catalog)");
                hfs_assert(0);
            }
            if (hfsmp->hfs_startup_cp && hfsmp->hfs_startup_cp->c_lockowner == current_thread()) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_systemfile_lock: bad lock order (Startup before Catalog)");
                hfs_assert(0);
            }
            if (hfsmp-> hfs_extents_cp && hfsmp->hfs_extents_cp->c_lockowner == current_thread()) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_systemfile_lock: bad lock order (Extents before Catalog)");
                hfs_assert(0);
            }
#endif /* HFS_CHECK_LOCK_ORDER */

            (void) hfs_lock(hfsmp->hfs_catalog_cp, locktype, HFS_LOCK_DEFAULT);
            /*
             * When the catalog file has overflow extents then
             * also acquire the extents b-tree lock if its not
             * already requested.
             */
            if (((flags & SFL_EXTENTS) == 0) &&
                (hfsmp->hfs_catalog_vp != NULL) &&
                (overflow_extents(VTOF(hfsmp->hfs_catalog_vp)))) {
                flags |= SFL_EXTENTS;
            }
        } else {
            flags &= ~SFL_CATALOG;
        }
    }

    if (flags & SFL_ATTRIBUTE) {
        if (hfsmp->hfs_attribute_cp
            && hfsmp->hfs_attribute_cp->c_lockowner != thread) {
#ifdef HFS_CHECK_LOCK_ORDER
            if (hfsmp->hfs_startup_cp && hfsmp->hfs_startup_cp->c_lockowner == current_thread()) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_systemfile_lock: bad lock order (Startup before Attributes)");
                hfs_assert(0);
            }
            if (hfsmp->hfs_extents_cp && hfsmp->hfs_extents_cp->c_lockowner == current_thread()) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_systemfile_lock: bad lock order (Extents before Attributes)");
                hfs_assert(0);
            }
#endif /* HFS_CHECK_LOCK_ORDER */

            (void) hfs_lock(hfsmp->hfs_attribute_cp, locktype, HFS_LOCK_DEFAULT);
            /*
             * When the attribute file has overflow extents then
             * also acquire the extents b-tree lock if its not
             * already requested.
             */
            if (((flags & SFL_EXTENTS) == 0) &&
                (hfsmp->hfs_attribute_vp != NULL) &&
                (overflow_extents(VTOF(hfsmp->hfs_attribute_vp)))) {
                flags |= SFL_EXTENTS;
            }
        } else {
            flags &= ~SFL_ATTRIBUTE;
        }
    }

    if (flags & SFL_STARTUP) {
        if (hfsmp->hfs_startup_cp
            && hfsmp->hfs_startup_cp->c_lockowner != thread) {
#ifdef HFS_CHECK_LOCK_ORDER
            if (hfsmp-> hfs_extents_cp && hfsmp->hfs_extents_cp->c_lockowner == current_thread()) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_systemfile_lock: bad lock order (Extents before Startup)");
                hfs_assert(0);
            }
#endif /* HFS_CHECK_LOCK_ORDER */

            (void) hfs_lock(hfsmp->hfs_startup_cp, locktype, HFS_LOCK_DEFAULT);
            /*
             * When the startup file has overflow extents then
             * also acquire the extents b-tree lock if its not
             * already requested.
             */
            if (((flags & SFL_EXTENTS) == 0) &&
                (hfsmp->hfs_startup_vp != NULL) &&
                (overflow_extents(VTOF(hfsmp->hfs_startup_vp)))) {
                flags |= SFL_EXTENTS;
            }
        } else {
            flags &= ~SFL_STARTUP;
        }
    }

    /*
     * To prevent locks being taken in the wrong order, the extent lock
     * gets a bitmap lock as well.
     */
    if (flags & (SFL_BITMAP | SFL_EXTENTS)) {
        if (hfsmp->hfs_allocation_cp) {
            (void) hfs_lock(hfsmp->hfs_allocation_cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
            /*
             * The bitmap lock is also grabbed when only extent lock
             * was requested. Set the bitmap lock bit in the lock
             * flags which callers will use during unlock.
             */
            flags |= SFL_BITMAP;

        } else {
            flags &= ~SFL_BITMAP;
        }
    }

    if (flags & SFL_EXTENTS) {
        /*
         * Since the extents btree lock is recursive we always
         * need exclusive access.
         */
        if (hfsmp->hfs_extents_cp) {
            (void) hfs_lock(hfsmp->hfs_extents_cp, HFS_EXCLUSIVE_LOCK, HFS_LOCK_DEFAULT);
        } else {
            flags &= ~SFL_EXTENTS;
        }
    }

    return (flags);
}

/*
 * unlock HFS system file(s).
 */
void
hfs_systemfile_unlock(struct hfsmount *hfsmp, int flags)
{
    if (!flags)
        return;

    if (flags & SFL_STARTUP && hfsmp->hfs_startup_cp) {
        hfs_unlock(hfsmp->hfs_startup_cp);
    }
    if (flags & SFL_ATTRIBUTE && hfsmp->hfs_attribute_cp) {
        hfs_unlock(hfsmp->hfs_attribute_cp);
    }
    if (flags & SFL_CATALOG && hfsmp->hfs_catalog_cp) {
        hfs_unlock(hfsmp->hfs_catalog_cp);
    }
    if (flags & SFL_BITMAP && hfsmp->hfs_allocation_cp) {
        hfs_unlock(hfsmp->hfs_allocation_cp);
    }
    if (flags & SFL_EXTENTS && hfsmp->hfs_extents_cp) {
        hfs_unlock(hfsmp->hfs_extents_cp);
    }
}

u_int32_t
hfs_freeblks(struct hfsmount * hfsmp, int wantreserve)
{
    u_int32_t freeblks;
    u_int32_t rsrvblks;
    u_int32_t loanblks;

    /*
     * We don't bother taking the mount lock
     * to look at these values since the values
     * themselves are each updated atomically
     * on aligned addresses.
     */
    freeblks = hfsmp->freeBlocks;
    rsrvblks = hfsmp->reserveBlocks;
    loanblks = hfsmp->loanedBlocks + hfsmp->lockedBlocks;
    if (wantreserve) {
        if (freeblks > rsrvblks)
            freeblks -= rsrvblks;
        else
            freeblks = 0;
    }
    if (freeblks > loanblks)
        freeblks -= loanblks;
    else
        freeblks = 0;

    return (freeblks);
}

/*
 * Map HFS Common errors (negative) to BSD error codes (positive).
 * Positive errors (ie BSD errors) are passed through unchanged.
 */
short MacToVFSError(OSErr err)
{
    if (err >= 0)
        return err;

    /* BSD/VFS internal errnos */
    switch (err) {
        case HFS_ERESERVEDNAME: /* -8 */
            return err;
    }

    switch (err) {
        case dskFulErr:            /*    -34 */
        case btNoSpaceAvail:        /* -32733 */
            return ENOSPC;
        case fxOvFlErr:            /* -32750 */
            return EOVERFLOW;

        case btBadNode:            /* -32731 */
            return EIO;

        case memFullErr:        /*  -108 */
            return ENOMEM;        /*   +12 */

        case cmExists:            /* -32718 */
        case btExists:            /* -32734 */
            return EEXIST;        /*    +17 */

        case cmNotFound:        /* -32719 */
        case btNotFound:        /* -32735 */
            return ENOENT;        /*     28 */

        case cmNotEmpty:        /* -32717 */
            return ENOTEMPTY;    /*     66 */

        case cmFThdDirErr:        /* -32714 */
            return EISDIR;        /*     21 */

        case fxRangeErr:        /* -32751 */
            return ERANGE;

        case bdNamErr:            /*   -37 */
            return ENAMETOOLONG;    /*    63 */

        case paramErr:            /*   -50 */
        case fileBoundsErr:        /* -1309 */
            return EINVAL;        /*   +22 */

        case fsBTBadNodeSize:
            return ENXIO;

        default:
            return EIO;        /*   +5 */
    }
}

/*
 * Find the current thread's directory hint for a given index.
 *
 * Requires an exclusive lock on directory cnode.
 *
 * Use detach if the cnode lock must be dropped while the hint is still active.
 */
directoryhint_t*
hfs_getdirhint(struct cnode *dcp, int index, int detach)
{

    directoryhint_t *hint;
    boolean_t need_remove, need_init;
    const u_int8_t* name;
    struct timeval tv;
    microtime(&tv);

    /*
     *  Look for an existing hint first.  If not found, create a new one (when
     *  the list is not full) or recycle the oldest hint.  Since new hints are
     *  always added to the head of the list, the last hint is always the
     *  oldest.
     */
    TAILQ_FOREACH(hint, &dcp->c_hintlist, dh_link)
    {
        if (hint->dh_index == index)
            break;
    }
    if (hint != NULL)
    { /* found an existing hint */
        need_init = false;
        need_remove = true;
    }
    else
    { /* cannot find an existing hint */
        need_init = true;
        if (dcp->c_dirhintcnt < HFS_MAXDIRHINTS)
        { /* we don't need recycling */
            /* Create a default directory hint */
            hint = hfs_malloc(sizeof(struct directoryhint));
            ++dcp->c_dirhintcnt;
            need_remove = false;
        }
        else
        {
            /* recycle the last (i.e., the oldest) hint */
            hint = TAILQ_LAST(&dcp->c_hintlist, hfs_hinthead);
            if ((hint->dh_desc.cd_flags & CD_HASBUF) && (name = hint->dh_desc.cd_nameptr))
            {
                hint->dh_desc.cd_nameptr = NULL;
                hint->dh_desc.cd_namelen = 0;
                hint->dh_desc.cd_flags &= ~CD_HASBUF;
                hfs_free((void*)name);
            }
            need_remove = true;
        }
    }

    if (need_remove)
        TAILQ_REMOVE(&dcp->c_hintlist, hint, dh_link);

    if (detach)
        --dcp->c_dirhintcnt;
    else
        TAILQ_INSERT_HEAD(&dcp->c_hintlist, hint, dh_link);

    if (need_init)
    {
        hint->dh_index = index;
        hint->dh_desc.cd_flags = 0;
        hint->dh_desc.cd_encoding = 0;
        hint->dh_desc.cd_namelen = 0;
        hint->dh_desc.cd_nameptr = NULL;
        hint->dh_desc.cd_parentcnid = dcp->c_fileid;
        hint->dh_desc.cd_hint = dcp->c_childhint;
        hint->dh_desc.cd_cnid = 0;
    }
    hint->dh_time = (uint32_t) tv.tv_sec;
    return (hint);
}

/*
 * Insert a detached directory hint back into the list of dirhints.
 *
 * Requires an exclusive lock on directory cnode.
 */
void
hfs_insertdirhint(struct cnode *dcp, directoryhint_t * hint)
{
    directoryhint_t *test;

    TAILQ_FOREACH(test, &dcp->c_hintlist, dh_link)
    {
        if (test == hint)
        {
            LFHFS_LOG(LEVEL_ERROR, "hfs_insertdirhint: hint %p already on list!", hint);
            hfs_assert(0);
        }
    }

    TAILQ_INSERT_HEAD(&dcp->c_hintlist, hint, dh_link);
    ++dcp->c_dirhintcnt;
}

/*
 * Release a single directory hint.
 *
 * Requires an exclusive lock on directory cnode.
 */
void
hfs_reldirhint(struct cnode *dcp, directoryhint_t * relhint)
{
    const u_int8_t * name;
    directoryhint_t *hint;

    /* Check if item is on list (could be detached) */
    TAILQ_FOREACH(hint, &dcp->c_hintlist, dh_link)
    {
        if (hint == relhint)
        {
            TAILQ_REMOVE(&dcp->c_hintlist, relhint, dh_link);
            --dcp->c_dirhintcnt;
            break;
        }
    }
    name = relhint->dh_desc.cd_nameptr;
    if ((relhint->dh_desc.cd_flags & CD_HASBUF) && (name != NULL))
    {
        relhint->dh_desc.cd_nameptr = NULL;
        relhint->dh_desc.cd_namelen = 0;
        relhint->dh_desc.cd_flags &= ~CD_HASBUF;
        hfs_free((void*)name);
    }
    hfs_free(relhint);
}

/*
 * Perform a case-insensitive compare of two UTF-8 filenames.
 *
 * Returns 0 if the strings match.
 */
int
hfs_namecmp(const u_int8_t *str1, size_t len1, const u_int8_t *str2, size_t len2)
{
    u_int16_t *ustr1, *ustr2;
    size_t ulen1, ulen2;
    size_t maxbytes;
    int cmp = -1;

    if (len1 != len2)
        return (cmp);

    maxbytes = kHFSPlusMaxFileNameChars << 1;
    ustr1 = hfs_malloc(maxbytes << 1);
    ustr2 = ustr1 + (maxbytes >> 1);

    if (utf8_decodestr(str1, len1, ustr1, &ulen1, maxbytes, ':', UTF_DECOMPOSED | UTF_ESCAPE_ILLEGAL) != 0)
        goto out;
    if (utf8_decodestr(str2, len2, ustr2, &ulen2, maxbytes, ':', UTF_DECOMPOSED | UTF_ESCAPE_ILLEGAL) != 0)
        goto out;

    ulen1 = ulen1 / sizeof(UniChar);
    ulen2 = ulen2 / sizeof(UniChar);
    cmp = FastUnicodeCompare(ustr1, ulen1, ustr2, ulen2);
out:
    hfs_free(ustr1);
    return (cmp);
}

/*
 * Perform a case-insensitive apendix cmp of two UTF-8 filenames.
 *
 * Returns 0 if the str2 is the same as the end of str1.
 */
int
hfs_apendixcmp(const u_int8_t *str1, size_t len1, const u_int8_t *str2, size_t len2)
{
    u_int16_t *ustr1, *ustr2, *original_allocation;
    size_t ulen1, ulen2;
    size_t maxbytes;
    int cmp = -1;

    maxbytes = kHFSPlusMaxFileNameChars << 1;
    ustr1 = hfs_malloc(maxbytes << 1);
    ustr2 = ustr1 + (maxbytes >> 1);
    original_allocation = ustr1;

    if (utf8_decodestr(str1, len1, ustr1, &ulen1, maxbytes, ':', UTF_DECOMPOSED | UTF_ESCAPE_ILLEGAL) != 0)
        goto out;
    if (utf8_decodestr(str2, len2, ustr2, &ulen2, maxbytes, ':', UTF_DECOMPOSED | UTF_ESCAPE_ILLEGAL) != 0)
        goto out;

    ulen1 = ulen1 / sizeof(UniChar);
    ulen2 = ulen2 / sizeof(UniChar);
    ustr1+= ulen1 - ulen2;
    cmp = FastUnicodeCompare(ustr1, ulen2, ustr2, ulen2);
out:
    hfs_free(original_allocation);
    return (cmp);
}

/*
 * Perform a case-insensitive strstr of two UTF-8 filenames.
 *
 * Returns 0 if the str2 in str1 match.
 */
int
hfs_strstr(const u_int8_t *str1, size_t len1, const u_int8_t *str2, size_t len2)
{
    u_int16_t *ustr1, *ustr2, *original_allocation;
    size_t ulen1, ulen2;
    size_t maxbytes;
    int cmp = 0;

    maxbytes = kHFSPlusMaxFileNameChars << 1;
    ustr1 = hfs_malloc(maxbytes << 1);
    ustr2 = ustr1 + (maxbytes >> 1);
    original_allocation = ustr1;
    if (utf8_decodestr(str1, len1, ustr1, &ulen1, maxbytes, ':', UTF_DECOMPOSED | UTF_ESCAPE_ILLEGAL) != 0)
    {
        goto out;
    }
    if (utf8_decodestr(str2, len2, ustr2, &ulen2, maxbytes, ':', UTF_DECOMPOSED | UTF_ESCAPE_ILLEGAL) != 0)
    {
        goto out;
    }

    ulen1 = ulen1 / sizeof(UniChar);
    ulen2 = ulen2 / sizeof(UniChar);

    do {
        if (ulen1-- < ulen2)
        {
            cmp = 1;
            break;
        }
    } while (FastUnicodeCompare(ustr1++, ulen2, ustr2, ulen2) != 0);

out:
    hfs_free(original_allocation);
    return cmp;
}

/*
 * Release directory hints for given directory
 *
 * Requires an exclusive lock on directory cnode.
 */
void
hfs_reldirhints(struct cnode *dcp, int stale_hints_only)
{
    struct timeval tv;
    directoryhint_t *hint, *prev;
    const u_int8_t * name;
    
    if (stale_hints_only)
        microuptime(&tv);
    
    /* searching from the oldest to the newest, so we can stop early when releasing stale hints only */
    TAILQ_FOREACH_REVERSE_SAFE(hint, &dcp->c_hintlist, hfs_hinthead, dh_link, prev) {
        if (stale_hints_only && (tv.tv_sec - hint->dh_time) < HFS_DIRHINT_TTL)
            break;  /* stop here if this entry is too new */
        name = hint->dh_desc.cd_nameptr;
        if ((hint->dh_desc.cd_flags & CD_HASBUF) && (name != NULL)) {
            hint->dh_desc.cd_nameptr = NULL;
            hint->dh_desc.cd_namelen = 0;
            hint->dh_desc.cd_flags &= ~CD_HASBUF;
            hfs_free((void *)name);
        }
        TAILQ_REMOVE(&dcp->c_hintlist, hint, dh_link);
        hfs_free(hint);
        --dcp->c_dirhintcnt;
    }
}

/* hfs_erase_unused_nodes
 *
 * Check wheter a volume may suffer from unused Catalog B-tree nodes that
 * are not zeroed (due to <rdar://problem/6947811>).  If so, just write
 * zeroes to the unused nodes.
 *
 * How do we detect when a volume needs this repair?  We can't always be
 * certain.  If a volume was created after a certain date, then it may have
 * been created with the faulty newfs_hfs.  Since newfs_hfs only created one
 * clump, we can assume that if a Catalog B-tree is larger than its clump size,
 * that means that the entire first clump must have been written to, which means
 * there shouldn't be unused and unwritten nodes in that first clump, and this
 * repair is not needed.
 *
 * We have defined a bit in the Volume Header's attributes to indicate when the
 * unused nodes have been repaired.  A newer newfs_hfs will set this bit.
 * As will fsck_hfs when it repairs the unused nodes.
 */
int hfs_erase_unused_nodes(struct hfsmount *hfsmp)
{
    int result;
    struct filefork *catalog;
    int lockflags;

    if (hfsmp->vcbAtrb & kHFSUnusedNodeFixMask)
    {
        /* This volume has already been checked and repaired. */
        return 0;
    }

    if ((hfsmp->localCreateDate < kHFSUnusedNodesFixDate))
    {
        /* This volume is too old to have had the problem. */
        hfsmp->vcbAtrb |= kHFSUnusedNodeFixMask;
        return 0;
    }

    catalog = hfsmp->hfs_catalog_cp->c_datafork;
    if (catalog->ff_size > catalog->ff_clumpsize)
    {
        /* The entire first clump must have been in use at some point. */
        hfsmp->vcbAtrb |= kHFSUnusedNodeFixMask;
        return 0;
    }

    /*
     * If we get here, we need to zero out those unused nodes.
     *
     * We start a transaction and lock the catalog since we're going to be
     * making on-disk changes.  But note that BTZeroUnusedNodes doens't actually
     * do its writing via the journal, because that would be too much I/O
     * to fit in a transaction, and it's a pain to break it up into multiple
     * transactions.  (It behaves more like growing a B-tree would.)
     */
    LFHFS_LOG(LEVEL_DEBUG, "hfs_erase_unused_nodes: updating volume %s.\n", hfsmp->vcbVN);
    result = hfs_start_transaction(hfsmp);
    if (result)
        goto done;
    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_EXCLUSIVE_LOCK);
    result = BTZeroUnusedNodes(catalog);
//    vnode_waitforwrites(hfsmp->hfs_catalog_vp, 0, 0, 0, "hfs_erase_unused_nodes");
    hfs_systemfile_unlock(hfsmp, lockflags);
    hfs_end_transaction(hfsmp);
    if (result == 0)
        hfsmp->vcbAtrb |= kHFSUnusedNodeFixMask;

    LFHFS_LOG(LEVEL_DEBUG, "hfs_erase_unused_nodes: done updating volume %s.\n", hfsmp->vcbVN);

done:
    return result;
}

/*
 * On HFS Plus Volumes, there can be orphaned files or directories
 * These are files or directories that were unlinked while busy.
 * If the volume was not cleanly unmounted then some of these may
 * have persisted and need to be removed.
 */
void
hfs_remove_orphans(struct hfsmount * hfsmp)
{
    BTreeIterator * iterator = NULL;
    FSBufferDescriptor btdata;
    struct HFSPlusCatalogFile filerec;
    struct HFSPlusCatalogKey * keyp;
    FCB *fcb;
    ExtendedVCB *vcb;
    char filename[32];
    char tempname[32];
    size_t namelen;
    cat_cookie_t cookie;
    int catlock = 0;
    int catreserve = 0;
    bool started_tr = false;
    int lockflags;
    int result;
    int orphaned_files = 0;
    int orphaned_dirs = 0;

    bzero(&cookie, sizeof(cookie));

    if (hfsmp->hfs_flags & HFS_CLEANED_ORPHANS)
        return;

    vcb = HFSTOVCB(hfsmp);
    fcb = VTOF(hfsmp->hfs_catalog_vp);

    btdata.bufferAddress = &filerec;
    btdata.itemSize = sizeof(filerec);
    btdata.itemCount = 1;

    iterator = hfs_mallocz(sizeof(BTreeIterator));
    if (iterator == NULL)
        return;

    /* Build a key to "temp" */
    keyp = (HFSPlusCatalogKey*)&iterator->key;
    keyp->parentID = hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid;
    keyp->nodeName.length = 4;  /* "temp" */
    keyp->keyLength = kHFSPlusCatalogKeyMinimumLength + keyp->nodeName.length * 2;
    keyp->nodeName.unicode[0] = 't';
    keyp->nodeName.unicode[1] = 'e';
    keyp->nodeName.unicode[2] = 'm';
    keyp->nodeName.unicode[3] = 'p';

    /*
     * Position the iterator just before the first real temp file/dir.
     */
    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_EXCLUSIVE_LOCK);
    (void) BTSearchRecord(fcb, iterator, NULL, NULL, iterator);
    hfs_systemfile_unlock(hfsmp, lockflags);

    /* Visit all the temp files/dirs in the HFS+ private directory. */
    for (;;) {
        lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_EXCLUSIVE_LOCK);
        result = BTIterateRecord(fcb, kBTreeNextRecord, iterator, &btdata, NULL);
        hfs_systemfile_unlock(hfsmp, lockflags);
        if (result)
            break;
        if (keyp->parentID != hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid)
            break;

        (void) utf8_encodestr(keyp->nodeName.unicode, keyp->nodeName.length * 2,
                              (u_int8_t *)filename, &namelen, sizeof(filename), 0, UTF_ADD_NULL_TERM);

        (void) snprintf(tempname, sizeof(tempname), "%s%d", HFS_DELETE_PREFIX, filerec.fileID);

        /*
         * Delete all files (and directories) named "tempxxx",
         * where xxx is the file's cnid in decimal.
         *
         */
        if (bcmp(tempname, filename, namelen + 1) != 0)
            continue;

        struct filefork dfork;
        struct filefork rfork;
        struct cnode cnode;
        int mode = 0;

        bzero(&dfork, sizeof(dfork));
        bzero(&rfork, sizeof(rfork));
        bzero(&cnode, sizeof(cnode));

        if (hfs_start_transaction(hfsmp) != 0) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_remove_orphans: failed to start transaction\n");
            goto exit;
        }
        started_tr = true;

        /*
         * Reserve some space in the Catalog file.
         */
        if (cat_preflight(hfsmp, CAT_DELETE, &cookie) != 0) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_remove_orphans: cat_preflight failed\n");
            goto exit;
        }
        catreserve = 1;

        lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG | SFL_ATTRIBUTE | SFL_EXTENTS | SFL_BITMAP, HFS_EXCLUSIVE_LOCK);
        catlock = 1;

        /* Build a fake cnode */
        cat_convertattr(hfsmp, (CatalogRecord *)&filerec, &cnode.c_attr,  &dfork.ff_data, &rfork.ff_data);
        cnode.c_desc.cd_parentcnid = hfsmp->hfs_private_desc[FILE_HARDLINKS].cd_cnid;
        cnode.c_desc.cd_nameptr = (const u_int8_t *)filename;
        cnode.c_desc.cd_namelen = namelen;
        cnode.c_desc.cd_cnid = cnode.c_attr.ca_fileid;
        cnode.c_blocks = dfork.ff_blocks + rfork.ff_blocks;

        /* Position iterator at previous entry */
        if (BTIterateRecord(fcb, kBTreePrevRecord, iterator,
                            NULL, NULL) != 0) {
            break;
        }

        /* Truncate the file to zero (both forks) */
        if (dfork.ff_blocks > 0) {
            u_int64_t fsize;

            dfork.ff_cp = &cnode;
            cnode.c_datafork = &dfork;
            cnode.c_rsrcfork = NULL;
            fsize = (u_int64_t)dfork.ff_blocks * (u_int64_t)HFSTOVCB(hfsmp)->blockSize;
            while (fsize > 0) {
                if (fsize > HFS_BIGFILE_SIZE) {
                    fsize -= HFS_BIGFILE_SIZE;
                } else {
                    fsize = 0;
                }

                if (TruncateFileC(vcb, (FCB*)&dfork, fsize, 1, 0, cnode.c_attr.ca_fileid, false) != 0) {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_remove_orphans: error truncating data fork!\n");
                    break;
                }

                //
                // if we're iteratively truncating this file down,
                // then end the transaction and start a new one so
                // that no one transaction gets too big.
                //
                if (fsize > 0) {
                    /* Drop system file locks before starting
                     * another transaction to preserve lock order.
                     */
                    hfs_systemfile_unlock(hfsmp, lockflags);
                    catlock = 0;
                    hfs_end_transaction(hfsmp);

                    if (hfs_start_transaction(hfsmp) != 0) {
                        started_tr = false;
                        goto exit;
                    }
                    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG | SFL_ATTRIBUTE | SFL_EXTENTS | SFL_BITMAP, HFS_EXCLUSIVE_LOCK);
                    catlock = 1;
                }
            }
        }

        if (rfork.ff_blocks > 0) {
            rfork.ff_cp = &cnode;
            cnode.c_datafork = NULL;
            cnode.c_rsrcfork = &rfork;
            if (TruncateFileC(vcb, (FCB*)&rfork, 0, 1, 1, cnode.c_attr.ca_fileid, false) != 0) {
                LFHFS_LOG(LEVEL_ERROR, "hfs_remove_orphans: error truncating rsrc fork!\n");
                break;
            }
        }

        // Deal with extended attributes
        if (ISSET(cnode.c_attr.ca_recflags, kHFSHasAttributesMask)) {
            // hfs_removeallattr uses its own transactions
            hfs_systemfile_unlock(hfsmp, lockflags);
            catlock = false;
            hfs_end_transaction(hfsmp);

            hfs_removeallattr(hfsmp, cnode.c_attr.ca_fileid, &started_tr);

            if (!started_tr) {
                if (hfs_start_transaction(hfsmp) != 0) {
                    LFHFS_LOG(LEVEL_ERROR, "hfs_remove_orphans:: failed to start transaction\n");
                    goto exit;
                }
                started_tr = true;
            }

            lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG | SFL_ATTRIBUTE | SFL_EXTENTS | SFL_BITMAP, HFS_EXCLUSIVE_LOCK);
        }

        /* Remove the file or folder record from the Catalog */
        if (cat_delete(hfsmp, &cnode.c_desc, &cnode.c_attr) != 0) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_remove_orphans: error deleting cat rec for id %d!\n", cnode.c_desc.cd_cnid);
            hfs_systemfile_unlock(hfsmp, lockflags);
            catlock = 0;
            hfs_volupdate(hfsmp, VOL_UPDATE, 0);
            break;
        }

        mode = cnode.c_attr.ca_mode & S_IFMT;

        if (mode == S_IFDIR) {
            orphaned_dirs++;
        }
        else {
            orphaned_files++;
        }

        /* Update parent and volume counts */
        hfsmp->hfs_private_attr[FILE_HARDLINKS].ca_entries--;
        if (mode == S_IFDIR) {
            DEC_FOLDERCOUNT(hfsmp, hfsmp->hfs_private_attr[FILE_HARDLINKS]);
        }

        (void)cat_update(hfsmp, &hfsmp->hfs_private_desc[FILE_HARDLINKS],
                         &hfsmp->hfs_private_attr[FILE_HARDLINKS], NULL, NULL);

        /* Drop locks and end the transaction */
        hfs_systemfile_unlock(hfsmp, lockflags);
        cat_postflight(hfsmp, &cookie);
        catlock = catreserve = 0;

        /*
         Now that Catalog is unlocked, update the volume info, making
         sure to differentiate between files and directories
         */
        if (mode == S_IFDIR) {
            hfs_volupdate(hfsmp, VOL_RMDIR, 0);
        }
        else{
            hfs_volupdate(hfsmp, VOL_RMFILE, 0);
        }

        hfs_end_transaction(hfsmp);
        started_tr = false;
    } /* end for */

exit:

    if (orphaned_files > 0 || orphaned_dirs > 0)
        LFHFS_LOG(LEVEL_ERROR, "hfs_remove_orphans: Removed %d orphaned / unlinked files and %d directories \n", orphaned_files, orphaned_dirs);

    if (catlock) {
        hfs_systemfile_unlock(hfsmp, lockflags);
    }
    if (catreserve) {
        cat_postflight(hfsmp, &cookie);
    }
    if (started_tr) {
        hfs_end_transaction(hfsmp);
    }

    hfs_free(iterator);
    hfsmp->hfs_flags |= HFS_CLEANED_ORPHANS;
}


u_int32_t GetFileInfo(ExtendedVCB *vcb, const char *name,
            struct cat_attr *fattr, struct cat_fork *forkinfo) {
    
    struct hfsmount * hfsmp;
    struct cat_desc jdesc;
    int lockflags;
    int error;
    
    if (vcb->vcbSigWord != kHFSPlusSigWord)
        return (0);
    
    hfsmp = VCBTOHFS(vcb);
    
    memset(&jdesc, 0, sizeof(struct cat_desc));
    jdesc.cd_parentcnid = kRootDirID;
    jdesc.cd_nameptr = (const u_int8_t *)name;
    jdesc.cd_namelen = strlen(name);
    
    lockflags = hfs_systemfile_lock(hfsmp, SFL_CATALOG, HFS_SHARED_LOCK);
    error = cat_lookup(hfsmp, &jdesc, 0, NULL, fattr, forkinfo, NULL);
    hfs_systemfile_unlock(hfsmp, lockflags);
    
    if (error == 0) {
        return (fattr->ca_fileid);
    } else if (hfsmp->hfs_flags & HFS_READ_ONLY) {
        return (0);
    }
    
    return (0);    /* XXX what callers expect on an error */
}


int hfs_early_journal_init(struct hfsmount *hfsmp, HFSPlusVolumeHeader *vhp,
					   void *_args, off_t embeddedOffset, daddr64_t mdb_offset,
					   HFSMasterDirectoryBlock *mdbp) {

	JournalInfoBlock *jibp;
	void             *bp = NULL;
    void             *jinfo_bp = NULL;
	int               sectors_per_fsblock, arg_flags=0, arg_tbufsz=0;
	int               retval = 0;
	uint32_t		  blksize = hfsmp->hfs_logical_block_size;
	struct vnode     *devvp;
	struct hfs_mount_args *args = _args;
	u_int32_t	     jib_flags;
	u_int64_t	     jib_offset;
	u_int64_t	     jib_size;
	
	devvp = hfsmp->hfs_devvp;

	if (args != NULL && (args->flags & HFSFSMNT_EXTENDED_ARGS)) {
		arg_flags  = args->journal_flags;
		arg_tbufsz = args->journal_tbuffer_size;
	}

	sectors_per_fsblock = SWAP_BE32(vhp->blockSize) / blksize;
	
    // Read Journal Info
	jinfo_bp = hfs_malloc(hfsmp->hfs_physical_block_size);
    if (!jinfo_bp) {
        goto cleanup_dev_name;
    }

    uint32_t ujournalInfoBlock = SWAP_BE32(vhp->journalInfoBlock);
    uint64_t u64JournalOffset  =
        (daddr64_t)((embeddedOffset/blksize) + ((u_int64_t)ujournalInfoBlock*sectors_per_fsblock));
    retval = raw_readwrite_read_mount(devvp, u64JournalOffset, hfsmp->hfs_physical_block_size,
                                      jinfo_bp, hfsmp->hfs_physical_block_size, NULL, NULL);
    
    if (retval) {
        goto cleanup_dev_name;
    }

	jibp       = jinfo_bp;
	jib_flags  = SWAP_BE32(jibp->flags);
	jib_size   = SWAP_BE64(jibp->size);

	if (!(jib_flags & kJIJournalInFSMask)) {
        goto cleanup_dev_name;
    }

    hfsmp->jvp = hfsmp->hfs_devvp;
    jib_offset = SWAP_BE64(jibp->offset);

	// save this off for the hack-y check in hfs_remove()
	hfsmp->jnl_start = jib_offset / SWAP_BE32(vhp->blockSize);
	hfsmp->jnl_size  = jib_size;

	if ((hfsmp->hfs_flags & HFS_READ_ONLY) && (hfsmp->hfs_mp->mnt_flag & MNT_ROOTFS) == 0) {
	    // if the file system is read-only, check if the journal is empty.
	    // if it is, then we can allow the mount.  otherwise we have to
	    // return failure.
	    retval = journal_is_clean(hfsmp->jvp,
				      jib_offset + embeddedOffset,
				      jib_size,
				      devvp,
				      hfsmp->hfs_logical_block_size,
                      hfsmp->hfs_mp);

	    hfsmp->jnl = NULL;

        hfs_free(jinfo_bp);
        jinfo_bp = NULL;

	    if (retval) {
		    LFHFS_LOG(LEVEL_ERROR, "hfs: early journal init: the volume is read-only and journal is dirty.  Can not mount volume.\n");
	    }

	    goto cleanup_dev_name;
	}

	if (jib_flags & kJIJournalNeedInitMask) {
		LFHFS_LOG(LEVEL_ERROR, "hfs: Initializing the journal (joffset 0x%llx sz 0x%llx)...\n",
			   jib_offset + embeddedOffset, jib_size);
		hfsmp->jnl = journal_create(hfsmp->jvp,
									jib_offset + embeddedOffset,
									jib_size,
									devvp,
									blksize,
									arg_flags,
									arg_tbufsz,
									NULL,
                                    hfsmp->hfs_mp,
									hfsmp->hfs_mp);

		// no need to start a transaction here... if this were to fail
		// we'd just re-init it on the next mount.
		jib_flags &= ~kJIJournalNeedInitMask;
		jibp->flags  = SWAP_BE32(jib_flags);
        raw_readwrite_write_mount(devvp, u64JournalOffset, hfsmp->hfs_physical_block_size,
                                          jinfo_bp, hfsmp->hfs_physical_block_size, NULL, NULL);
		jinfo_bp = NULL;
		jibp     = NULL;
	} else {
		LFHFS_LOG(LEVEL_DEFAULT, "hfs: Opening the journal (jib_offset 0x%llx size 0x%llx vhp_blksize %d)...\n",
			   jib_offset + embeddedOffset,
			   jib_size, SWAP_BE32(vhp->blockSize));
				
		hfsmp->jnl = journal_open(hfsmp->jvp,
								  jib_offset + embeddedOffset,
								  jib_size,
								  devvp,
								  blksize,
								  arg_flags,
								  arg_tbufsz,
                                  NULL,
                                  hfsmp->hfs_mp,
								  hfsmp->hfs_mp);

        if (hfsmp->jnl && mdbp) { 
			// reload the mdb because it could have changed
			// if the journal had to be replayed.
			if (mdb_offset == 0) {
				mdb_offset = (daddr64_t)((embeddedOffset / blksize) + HFS_PRI_SECTOR(blksize));
			}
            
            bp = hfs_malloc(hfsmp->hfs_physical_block_size);
            if (!bp) {
                goto cleanup_dev_name;
            }
            
            uint64_t u64MDBOffset = HFS_PHYSBLK_ROUNDDOWN(mdb_offset, hfsmp->hfs_log_per_phys);
            retval = raw_readwrite_read_mount(devvp, u64MDBOffset, hfsmp->hfs_physical_block_size, bp, hfsmp->hfs_physical_block_size, NULL, NULL);
            
            if (retval) {
                LFHFS_LOG(LEVEL_ERROR, "hfs: failed to reload the mdb after opening the journal (retval %d)!\n", retval);
                goto cleanup_dev_name;
            }

			bcopy(bp + HFS_PRI_OFFSET(hfsmp->hfs_physical_block_size), mdbp, 512);
		}
	}

	// if we expected the journal to be there and we couldn't
	// create it or open it then we have to bail out.
	if (hfsmp->jnl == NULL) {
		LFHFS_LOG(LEVEL_ERROR, "hfs: early jnl init: failed to open/create the journal (retval %d).\n", retval);
		retval = EINVAL;
		goto cleanup_dev_name;
	}

cleanup_dev_name:
    if (bp)
        hfs_free(bp);

    if (jinfo_bp)
        hfs_free(jinfo_bp);

	return retval;
}

//
// This function will go and re-locate the .journal_info_block and
// the .journal files in case they moved (which can happen if you
// run Norton SpeedDisk).  If we fail to find either file we just
// disable journaling for this volume and return.  We turn off the
// journaling bit in the vcb and assume it will get written to disk
// later (if it doesn't on the next mount we'd do the same thing
// again which is harmless).  If we disable journaling we don't
// return an error so that the volume is still mountable.
//
// If the info we find for the .journal_info_block and .journal files
// isn't what we had stored, we re-set our cached info and proceed
// with opening the journal normally.
//
static int hfs_late_journal_init(struct hfsmount *hfsmp, HFSPlusVolumeHeader *vhp, void *_args) {
    JournalInfoBlock *jibp;
    void             *jinfo_bp = NULL;
    int               sectors_per_fsblock, arg_flags=0, arg_tbufsz=0;
    int               retval, write_jibp = 0, recreate_journal = 0;
    struct vnode     *devvp;
    struct cat_attr   jib_attr, jattr;
    struct cat_fork   jib_fork, jfork;
    ExtendedVCB      *vcb;
    u_int32_t         fid;
    struct hfs_mount_args *args = _args;
    u_int32_t         jib_flags;
    u_int64_t         jib_offset;
    u_int64_t         jib_size;
    
    devvp = hfsmp->hfs_devvp;
    vcb = HFSTOVCB(hfsmp);
    
    if (args != NULL && (args->flags & HFSFSMNT_EXTENDED_ARGS)) {
        if (args->journal_disable) {
            return 0;
        }
        
        arg_flags  = args->journal_flags;
        arg_tbufsz = args->journal_tbuffer_size;
    }
    
    fid = GetFileInfo(vcb, ".journal_info_block", &jib_attr, &jib_fork);
    if (fid == 0 || jib_fork.cf_extents[0].startBlock == 0 || jib_fork.cf_size == 0) {
        LFHFS_LOG(LEVEL_ERROR, "hfs: can't find the .journal_info_block! disabling journaling (start: %d).\n",
               fid ? jib_fork.cf_extents[0].startBlock : 0);
        vcb->vcbAtrb &= ~kHFSVolumeJournaledMask;
        return 0;
    }
    hfsmp->hfs_jnlinfoblkid = fid;
    
    // make sure the journal_info_block begins where we think it should.
    if (SWAP_BE32(vhp->journalInfoBlock) != jib_fork.cf_extents[0].startBlock) {
        LFHFS_LOG(LEVEL_ERROR, "hfs: The journal_info_block moved (was: %d; is: %d).  Fixing up\n",
               SWAP_BE32(vhp->journalInfoBlock), jib_fork.cf_extents[0].startBlock);
        
        vcb->vcbJinfoBlock    = jib_fork.cf_extents[0].startBlock;
        vhp->journalInfoBlock = SWAP_BE32(jib_fork.cf_extents[0].startBlock);
        recreate_journal = 1;
    }
    
    
    sectors_per_fsblock = SWAP_BE32(vhp->blockSize) / hfsmp->hfs_logical_block_size;

    // Read journal info
    jinfo_bp = hfs_malloc(hfsmp->hfs_physical_block_size);
    if (!jinfo_bp) {
        LFHFS_LOG(LEVEL_ERROR, "hfs: can't alloc memory.\n");
        vcb->vcbAtrb &= ~kHFSVolumeJournaledMask;
        return 0;
    }
    
    uint64_t u64JournalOffset =
        (vcb->hfsPlusIOPosOffset / hfsmp->hfs_logical_block_size +
        ((u_int64_t)SWAP_BE32(vhp->journalInfoBlock)*sectors_per_fsblock));
    
    retval = raw_readwrite_read_mount(devvp, u64JournalOffset, hfsmp->hfs_physical_block_size, jinfo_bp, hfsmp->hfs_physical_block_size, NULL, NULL);
    
    if (retval) {
        if (jinfo_bp) {
            hfs_free(jinfo_bp);
        }
        LFHFS_LOG(LEVEL_ERROR, "hfs: can't read journal info block. disabling journaling.\n");
        vcb->vcbAtrb &= ~kHFSVolumeJournaledMask;
        return 0;
    }
    
    jibp       = jinfo_bp;
    jib_flags  = SWAP_BE32(jibp->flags);
    jib_offset = SWAP_BE64(jibp->offset);
    jib_size   = SWAP_BE64(jibp->size);
    
    fid = GetFileInfo(vcb, ".journal", &jattr, &jfork);
    if (fid == 0 || jfork.cf_extents[0].startBlock == 0 || jfork.cf_size == 0) {
        LFHFS_LOG(LEVEL_ERROR, "hfs: can't find the journal file! disabling journaling (start: %d)\n",
               fid ? jfork.cf_extents[0].startBlock : 0);
        hfs_free(jinfo_bp);
        vcb->vcbAtrb &= ~kHFSVolumeJournaledMask;
        return 0;
    }
    hfsmp->hfs_jnlfileid = fid;
    
    // make sure the journal file begins where we think it should.
    if ((jib_flags & kJIJournalInFSMask) && (jib_offset / (u_int64_t)vcb->blockSize) != jfork.cf_extents[0].startBlock) {
        LFHFS_LOG(LEVEL_ERROR, "hfs: The journal file moved (was: %lld; is: %d).  Fixing up\n",
               (jib_offset / (u_int64_t)vcb->blockSize), jfork.cf_extents[0].startBlock);
        
        jib_offset = (u_int64_t)jfork.cf_extents[0].startBlock * (u_int64_t)vcb->blockSize;
        write_jibp   = 1;
        recreate_journal = 1;
    }
    
    // check the size of the journal file.
    if (jib_size != (u_int64_t)jfork.cf_extents[0].blockCount*vcb->blockSize) {
        LFHFS_LOG(LEVEL_ERROR, "hfs: The journal file changed size! (was %lld; is %lld).  Fixing up.\n",
               jib_size, (u_int64_t)jfork.cf_extents[0].blockCount*vcb->blockSize);
        
        jib_size = (u_int64_t)jfork.cf_extents[0].blockCount * vcb->blockSize;
        write_jibp = 1;
        recreate_journal = 1;
    }
    
    if (!(jib_flags & kJIJournalInFSMask)) {
        LFHFS_LOG(LEVEL_ERROR, "hfs: No support for journal on a different volume\n");
        hfs_free(jinfo_bp);
        vcb->vcbAtrb &= ~kHFSVolumeJournaledMask;
        return 0;
    }
                  
    hfsmp->jvp = hfsmp->hfs_devvp;
    jib_offset += (off_t)vcb->hfsPlusIOPosOffset;
    
    // save this off for the hack-y check in hfs_remove()
    hfsmp->jnl_start = jib_offset / SWAP_BE32(vhp->blockSize);
    hfsmp->jnl_size  = jib_size;

    if ((hfsmp->hfs_flags & HFS_READ_ONLY) && (hfsmp->hfs_mp->mnt_flag & MNT_ROOTFS) == 0) {
        // if the file system is read-only, check if the journal is empty.
        // if it is, then we can allow the mount.  otherwise we have to
        // return failure.
        retval = journal_is_clean(hfsmp->jvp,
                                  jib_offset,
                                  jib_size,
                                  devvp,
                                  hfsmp->hfs_logical_block_size,
                                  hfsmp->hfs_mp);
        
        hfsmp->jnl = NULL;
        
        hfs_free(jinfo_bp);
        
        if (retval) {
            LFHFS_LOG(LEVEL_ERROR, "hfs_late_journal_init: volume on is read-only and journal is dirty.  Can not mount volume.\n");
        }
        
        return retval;
    }
    
    if ((jib_flags & kJIJournalNeedInitMask) || recreate_journal) {
        LFHFS_LOG(LEVEL_ERROR, "hfs: Initializing the journal (joffset 0x%llx sz 0x%llx)...\n",
               jib_offset, jib_size);
        hfsmp->jnl = journal_create(hfsmp->jvp,
                                    jib_offset,
                                    jib_size,
                                    devvp,
                                    hfsmp->hfs_logical_block_size,
                                    arg_flags,
                                    arg_tbufsz,
                                    NULL,
                                    hfsmp->hfs_mp,
                                    hfsmp->hfs_mp);

        // no need to start a transaction here... if this were to fail
        // we'd just re-init it on the next mount.
        jib_flags &= ~kJIJournalNeedInitMask;
        write_jibp   = 1;
        
    } else {
        //
        // if we weren't the last person to mount this volume
        // then we need to throw away the journal because it
        // is likely that someone else mucked with the disk.
        // if the journal is empty this is no big deal.  if the
        // disk is dirty this prevents us from replaying the
        // journal over top of changes that someone else made.
        //
        arg_flags |= JOURNAL_RESET;
        
        //printf("hfs: Opening the journal (joffset 0x%llx sz 0x%llx vhp_blksize %d)...\n",
        //       jib_offset,
        //       jib_size, SWAP_BE32(vhp->blockSize));
        
        hfsmp->jnl = journal_open(hfsmp->jvp,
                                  jib_offset,
                                  jib_size,
                                  devvp,
                                  hfsmp->hfs_logical_block_size,
                                  arg_flags,
                                  arg_tbufsz,
                                  NULL, 
                                  hfsmp->hfs_mp,
                                  hfsmp->hfs_mp);
    }
    
    
    if (write_jibp) {
        jibp->flags  = SWAP_BE32(jib_flags);
        jibp->offset = SWAP_BE64(jib_offset);
        jibp->size   = SWAP_BE64(jib_size);

        uint64_t uActualWrite = 0;
        retval = raw_readwrite_write_mount(devvp, u64JournalOffset, hfsmp->hfs_physical_block_size, jinfo_bp, hfsmp->hfs_physical_block_size, &uActualWrite, NULL);
    }
    
    if (jinfo_bp) {
        hfs_free(jinfo_bp);
    }
    
    // if we expected the journal to be there and we couldn't
    // create it or open it then we have to bail out.
    if (hfsmp->jnl == NULL) {
        LFHFS_LOG(LEVEL_ERROR, "hfs: late jnl init: failed to open/create the journal (retval %d).\n", retval);
        return EINVAL;
    }
    
    return 0;
}
    
