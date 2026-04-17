/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * hfs_endian.c
 *
 * This file implements endian swapping routines for the HFS/HFS Plus
 * volume format.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <libkern/OSByteOrder.h>
#include <hfs/hfs_format.h>

#include "hfs_endian.h"

#undef ENDIAN_DEBUG
#if 0
/* Private swapping routines */
int hfs_swap_HFSPlusBTInternalNode (BlockDescriptor *src, HFSCatalogNodeID fileID, int unswap);
int hfs_swap_HFSBTInternalNode (BlockDescriptor *src, HFSCatalogNodeID fileID, int unswap);
#endif

static void hfs_swap_HFSPlusForkData (HFSPlusForkData *src);

/*
 * hfs_swap_HFSMasterDirectoryBlock
 *
 *  Specially modified to swap parts of the finder info
 */
void
hfs_swap_HFSMasterDirectoryBlock (
    void *buf
)
{
    HFSMasterDirectoryBlock *src = (HFSMasterDirectoryBlock *)buf;

    src->drSigWord		= SWAP_BE16 (src->drSigWord);
    src->drCrDate		= SWAP_BE32 (src->drCrDate);
    src->drLsMod		= SWAP_BE32 (src->drLsMod);
    src->drAtrb			= SWAP_BE16 (src->drAtrb);
    src->drNmFls		= SWAP_BE16 (src->drNmFls);
    src->drVBMSt		= SWAP_BE16 (src->drVBMSt);
    src->drAllocPtr		= SWAP_BE16 (src->drAllocPtr);
    src->drNmAlBlks		= SWAP_BE16 (src->drNmAlBlks);
    src->drAlBlkSiz		= SWAP_BE32 (src->drAlBlkSiz);
    src->drClpSiz		= SWAP_BE32 (src->drClpSiz);
    src->drAlBlSt		= SWAP_BE16 (src->drAlBlSt);
    src->drNxtCNID		= SWAP_BE32 (src->drNxtCNID);
    src->drFreeBks		= SWAP_BE16 (src->drFreeBks);

    /* Don't swap drVN */

    src->drVolBkUp		= SWAP_BE32 (src->drVolBkUp);
    src->drVSeqNum		= SWAP_BE16 (src->drVSeqNum);
    src->drWrCnt		= SWAP_BE32 (src->drWrCnt);
    src->drXTClpSiz		= SWAP_BE32 (src->drXTClpSiz);
    src->drCTClpSiz		= SWAP_BE32 (src->drCTClpSiz);
    src->drNmRtDirs		= SWAP_BE16 (src->drNmRtDirs);
    src->drFilCnt		= SWAP_BE32 (src->drFilCnt);
    src->drDirCnt		= SWAP_BE32 (src->drDirCnt);

    /* Swap just the 'blessed folder' in drFndrInfo */
    src->drFndrInfo[0]	= SWAP_BE32 (src->drFndrInfo[0]);

    src->drEmbedSigWord	= SWAP_BE16 (src->drEmbedSigWord);
	src->drEmbedExtent.startBlock = SWAP_BE16 (src->drEmbedExtent.startBlock);
	src->drEmbedExtent.blockCount = SWAP_BE16 (src->drEmbedExtent.blockCount);

    src->drXTFlSize		= SWAP_BE32 (src->drXTFlSize);
	src->drXTExtRec[0].startBlock = SWAP_BE16 (src->drXTExtRec[0].startBlock);
	src->drXTExtRec[0].blockCount = SWAP_BE16 (src->drXTExtRec[0].blockCount);
	src->drXTExtRec[1].startBlock = SWAP_BE16 (src->drXTExtRec[1].startBlock);
	src->drXTExtRec[1].blockCount = SWAP_BE16 (src->drXTExtRec[1].blockCount);
	src->drXTExtRec[2].startBlock = SWAP_BE16 (src->drXTExtRec[2].startBlock);
	src->drXTExtRec[2].blockCount = SWAP_BE16 (src->drXTExtRec[2].blockCount);

    src->drCTFlSize		= SWAP_BE32 (src->drCTFlSize);
	src->drCTExtRec[0].startBlock = SWAP_BE16 (src->drCTExtRec[0].startBlock);
	src->drCTExtRec[0].blockCount = SWAP_BE16 (src->drCTExtRec[0].blockCount);
	src->drCTExtRec[1].startBlock = SWAP_BE16 (src->drCTExtRec[1].startBlock);
	src->drCTExtRec[1].blockCount = SWAP_BE16 (src->drCTExtRec[1].blockCount);
	src->drCTExtRec[2].startBlock = SWAP_BE16 (src->drCTExtRec[2].startBlock);
	src->drCTExtRec[2].blockCount = SWAP_BE16 (src->drCTExtRec[2].blockCount);
}

/*
 * hfs_swap_HFSPlusVolumeHeader
 */
void
hfs_swap_HFSPlusVolumeHeader (
    void *buf
)
{
    HFSPlusVolumeHeader *src = (HFSPlusVolumeHeader *)buf;

    src->signature			= SWAP_BE16 (src->signature);
    src->version			= SWAP_BE16 (src->version);
    src->attributes			= SWAP_BE32 (src->attributes);
    src->lastMountedVersion	= SWAP_BE32 (src->lastMountedVersion);

    src->journalInfoBlock	= SWAP_BE32 (src->journalInfoBlock);

    src->createDate			= SWAP_BE32 (src->createDate);
    src->modifyDate			= SWAP_BE32 (src->modifyDate);
    src->backupDate			= SWAP_BE32 (src->backupDate);
    src->checkedDate		= SWAP_BE32 (src->checkedDate);
    src->fileCount			= SWAP_BE32 (src->fileCount);
    src->folderCount		= SWAP_BE32 (src->folderCount);
    src->blockSize			= SWAP_BE32 (src->blockSize);
    src->totalBlocks		= SWAP_BE32 (src->totalBlocks);
    src->freeBlocks			= SWAP_BE32 (src->freeBlocks);
    src->nextAllocation		= SWAP_BE32 (src->nextAllocation);
    src->rsrcClumpSize		= SWAP_BE32 (src->rsrcClumpSize);
    src->dataClumpSize		= SWAP_BE32 (src->dataClumpSize);
    src->nextCatalogID		= SWAP_BE32 (src->nextCatalogID);
    src->writeCount			= SWAP_BE32 (src->writeCount);
    src->encodingsBitmap	= SWAP_BE64 (src->encodingsBitmap);

    /* Don't swap finderInfo */

    hfs_swap_HFSPlusForkData (&src->allocationFile);
    hfs_swap_HFSPlusForkData (&src->extentsFile);
    hfs_swap_HFSPlusForkData (&src->catalogFile);
    hfs_swap_HFSPlusForkData (&src->attributesFile);
    hfs_swap_HFSPlusForkData (&src->startupFile);
}

/*
 * hfs_swap_HFSPlusForkData
 *
 *  There's still a few spots where we still need to swap the fork data.
 */
void
hfs_swap_HFSPlusForkData (
    HFSPlusForkData *src
)
{
    int i;

	src->logicalSize		= SWAP_BE64 (src->logicalSize);

	src->clumpSize			= SWAP_BE32 (src->clumpSize);
	src->totalBlocks		= SWAP_BE32 (src->totalBlocks);

    for (i = 0; i < kHFSPlusExtentDensity; i++) {
        src->extents[i].startBlock	= SWAP_BE32 (src->extents[i].startBlock);
        src->extents[i].blockCount	= SWAP_BE32 (src->extents[i].blockCount);
    }
}
