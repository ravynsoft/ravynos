/*
 * Copyright (c) 1999-2015 Apple Inc. All rights reserved.
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
	File:		makehfs.c

	Contains:	Initialization code for HFS and HFS Plus volumes.

	Copyright:	� 1984-1999 by Apple Computer, Inc., all rights reserved.

*/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <err.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wipefs.h>

#include <TargetConditionals.h>

#if TARGET_OS_IPHONE

// CoreServices is not available, so...

/*
 * Mac OS Finder flags
 */
enum {
    kHasBeenInited  = 0x0100,       /* Files only */
    /* Clear if the file contains desktop database */
    /* bit 0x0200 was the letter bit for AOCE, but is now reserved for future use */
    kHasCustomIcon  = 0x0400,       /* Files and folders */
    kIsStationery   = 0x0800,       /* Files only */
    kNameLocked     = 0x1000,       /* Files and folders */
    kHasBundle      = 0x2000,       /* Files only */
    kIsInvisible    = 0x4000,       /* Files and folders */
    kIsAlias        = 0x8000        /* Files only */
};

enum {
    kTextEncodingMacUnicode		= 0x7E,
};

#else // !TARGET_OS_IPHONE

#include <CoreServices/CoreServices.h>

#endif // !TARGET_OS_IPHONE

/*
 * CommonCrypto is meant to be a more stable API than OpenSSL.
 * Defining COMMON_DIGEST_FOR_OPENSSL gives API-compatibility
 * with OpenSSL, so we don't have to change the code.
 */
#define COMMON_DIGEST_FOR_OPENSSL
#include <CommonCrypto/CommonDigest.h>

#include <libkern/OSByteOrder.h>

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFStringEncodingExt.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>

#include <TargetConditionals.h>

extern Boolean _CFStringGetFileSystemRepresentation(CFStringRef string, UInt8 *buffer, CFIndex maxBufLen);


#include <hfs/hfs_format.h>
#include <hfs/hfs_mount.h>
#include "hfs_endian.h"

#include "newfs_hfs.h"

#ifndef NEWFS_HFS_DEBUG
# ifdef DEBUG_BUILD
#  define NEWFS_HFS_DEBUG 1
# else
#  define NEWFS_HFS_DEBUG 0
# endif
#endif

#define HFS_BOOT_DATA	"/usr/share/misc/hfsbootdata"

#define HFS_JOURNAL_FILE	".journal"
#define HFS_JOURNAL_INFO	".journal_info_block"

#define kJournalFileType	0x6a726e6c	/* 'jrnl' */


typedef HFSMasterDirectoryBlock HFS_MDB;

struct filefork {
	UInt16	startBlock;
	UInt16	blockCount;
	UInt32	logicalSize;
	UInt32	physicalSize;
};

struct ExtentRecord {
	HFSPlusExtentKey	key;
	HFSPlusExtentRecord	record;
} __attribute__((aligned(2), packed));
static size_t numOverflowExtents = 0;
static struct ExtentRecord *overflowExtents = NULL;

struct filefork	gDTDBFork, gSystemFork, gReadMeFork;

static void WriteVH __P((const DriveInfo *driveInfo, HFSPlusVolumeHeader *hp));
static void InitVH __P((hfsparams_t *defaults, UInt64 sectors,
		HFSPlusVolumeHeader *header));

static int AllocateExtent(UInt8 *buffer, UInt32 startBlock, UInt32 blockCount);
static int MarkExtentUsed(const DriveInfo *, HFSPlusVolumeHeader *, UInt32, UInt32);

static void WriteExtentsFile __P((const DriveInfo *dip, UInt64 startingSector,
        const hfsparams_t *dp, HFSExtentDescriptor *bbextp, void *buffer,
        UInt32 *bytesUsed, UInt32 *mapNodes));

static void WriteAttributesFile(const DriveInfo *driveInfo, UInt64 startingSector,
        const hfsparams_t *dp, HFSExtentDescriptor *bbextp, void *buffer,
        UInt32 *bytesUsed, UInt32 *mapNodes);

static void WriteCatalogFile __P((const DriveInfo *dip, UInt64 startingSector,
        const hfsparams_t *dp, HFSPlusVolumeHeader *header, void *buffer,
        UInt32 *bytesUsed, UInt32 *mapNodes));
static int  WriteJournalInfo(const DriveInfo *driveInfo, UInt64 startingSector,
			     const hfsparams_t *dp, HFSPlusVolumeHeader *header,
			     void *buffer);
static void InitCatalogRoot_HFSPlus __P((const hfsparams_t *dp, const HFSPlusVolumeHeader *header, void * buffer));

static void WriteMapNodes __P((const DriveInfo *driveInfo, UInt64 diskStart,
		UInt32 firstMapNode, UInt32 mapNodes, UInt16 btNodeSize, void *buffer));
static void WriteBuffer __P((const DriveInfo *driveInfo, UInt64 startingSector,
		UInt64 byteCount, const void *buffer));
static UInt32 Largest __P((UInt32 a, UInt32 b, UInt32 c, UInt32 d ));

static UInt32 GetDefaultEncoding();

static UInt32 UTCToLocal __P((UInt32 utcTime));

static int ConvertUTF8toUnicode __P((const UInt8* source, size_t bufsize,
		UniChar* unibuf, UInt16 *charcount));

#define VOLUMEUUIDVALUESIZE 2
typedef union VolumeUUID {
	UInt32 value[VOLUMEUUIDVALUESIZE];
	struct {
		UInt32 high;
		UInt32 low;
	} v;
} VolumeUUID;
void GenerateVolumeUUID(VolumeUUID *newVolumeID);

void SETOFFSET (void *buffer, UInt16 btNodeSize, SInt16 recOffset, SInt16 vecOffset);
#define SETOFFSET(buf,ndsiz,offset,rec)		\
	(*(SInt16 *)((UInt8 *)(buf) + (ndsiz) + (-2 * (rec))) = (SWAP_BE16 (offset)))

#define BYTESTOBLKS(bytes,blks)		DivideAndRoundUp((bytes),(blks))

#define ROUNDUP(x, u)	(((x) % (u) == 0) ? (x) : ((x)/(u) + 1) * (u))

#if TARGET_OS_IPHONE
#define ENCODING_TO_BIT(e)				 \
	  ((e) < 48 ? (e) : 0)
#else  // !TARGET_OS_IPHONE
#define ENCODING_TO_BIT(e)                               \
          ((e) < 48 ? (e) :                              \
          ((e) == kCFStringEncodingMacUkrainian ? 48 :   \
          ((e) == kCFStringEncodingMacFarsi ? 49 : 0)))
#endif // TARGET_OS_IPHONE


#ifdef DEBUG_BUILD
struct cp_root_xattr {
	u_int16_t vers;
	u_int16_t reserved1;
	u_int64_t reserved2;
	u_int8_t reserved3[16];
} __attribute__((aligned(2), packed)); 
#endif

/*
 * Create a series of (sequential!) extents for the
 * requested file.  It tries to create the requested
 * number, but may be stymied by the file size, and
 * the number of minimum blocks.
 */
static void
createExtents(HFSPlusForkData *file,
	      UInt32 fileID,
	      UInt32 startBlock,
	      size_t numExtents,
	      int minBlocks)
{
	if (NEWFS_HFS_DEBUG == 0) {
		/*
		 * The common case, for non-debug.
		 */
		file->extents[0].startBlock = startBlock;
		file->extents[0].blockCount = file->totalBlocks;
	} else {
		UInt32 blocksLeft, blocksTotal = 0, blockStep;
		int i;
		int firstAdjust = 0;
		
		if (numExtents == 1) {
			// The common case, no need to do any math
			file->extents[0].startBlock = startBlock;
			file->extents[0].blockCount = file->totalBlocks;
			return;
		}
		if (file->totalBlocks < numExtents)
			numExtents = file->totalBlocks;
		
		blocksLeft = file->totalBlocks;
		
		/*
		 * The intent here is to split the number of blocks into the
		 * requested number of extents.  So first we determine how
		 * many blocks should go in each extent -- that's blockStep.
		 * If we have been giving minBlocks, we need to make sure it's
		 * a multiple of that. (In general, the values are going to be
		 * 1 or 2 for minBlocks.)
		 *
		 * If there are more requested extents than blocks, the division
		 * works out to zero... so we limit blockStep to minBlocks.
		 *
		 */
		blockStep = blocksLeft / numExtents;

		/*
		 * To allow invalid extent lengths, set minBlocks to 1, and
		 * comment out the next two if statements.
		 */
		if ((blockStep % minBlocks) != 0)
			blockStep = (blockStep / minBlocks) * minBlocks;
		if (blockStep == 0)
			blockStep = minBlocks;
		
		/*
		 * Now, after that, we may still not have the right number, since
		 * the math may not work out properly.  So we can work around that
		 * by making the first extent have all the spares.
		 */
		if ((blockStep * numExtents) < blocksLeft) {
			// Need to adjust the first one.
			firstAdjust = (int)(blocksLeft - (blockStep * numExtents));
			if ((firstAdjust % minBlocks) != 0)
				firstAdjust = ROUNDUP(firstAdjust, minBlocks);
		}
		
		/*
		 * Now, at this point, start handing out blocks to each extent.
		 * First to the 8 extents in the fork descriptor.
		 */
		for (i = 0; i < 8 && blocksLeft > 0; i++) {
			int n = MIN(blockStep + firstAdjust, blocksLeft);
			file->extents[i].startBlock = startBlock + blocksTotal;
			file->extents[i].blockCount = n;
			blocksLeft -= n;
			blocksTotal += n;
			firstAdjust = 0;
		}
		/*
		 * Then, if there are any left, to the overflow extents.
		 */
		while (blocksLeft > 0) {
			struct ExtentRecord tmp;
			UInt32 bcount = 0;
			memset(&tmp, 0, sizeof(tmp));
			tmp.key.keyLength = SWAP_BE16(sizeof(HFSPlusExtentKey) - sizeof(uint16_t));
			tmp.key.forkType = 0;
			tmp.key.fileID = SWAP_BE32(fileID);
			tmp.key.startBlock = SWAP_BE32(blocksTotal);
			for (i = 0; i < 8 && blocksLeft > 0; i++) {
				int n = MIN(blockStep, blocksLeft);
				tmp.record[i].startBlock = SWAP_BE32(blocksTotal + bcount + startBlock);
				tmp.record[i].blockCount = SWAP_BE32(n);
				bcount += n;
				blocksLeft -= n;
			}
			blocksTotal += bcount;
			overflowExtents = realloc(overflowExtents, (numOverflowExtents+1) * sizeof(*overflowExtents));
			overflowExtents[numOverflowExtents++] = tmp;
		}
	}
	return;
}

/*
 * wipefs() in -lutil knows about multiple filesystem formats.
 * This replaces the code:
 *	WriteBuffer(driveInfo, 0, diskBlocksUsed * kBytesPerSector, NULL);
 *	WriteBuffer(driveInfo, driveInfo->totalSectors - 8, 4 * 1024, NULL);
 * which was used to erase the beginning and end of the filesystem.
 *
 */
static int
dowipefs(int fd)
{
	int err;
	wipefs_ctx handle;

	err = wipefs_alloc(fd, 0/*sectorSize*/, &handle);
	if (err == 0) {
		err = wipefs_wipe(handle);
	}
	wipefs_free(&handle);
	return err;
}


/*
 * make_hfsplus
 *	
 * This routine writes an initial HFS Plus volume structure onto a volume.
 * It is assumed that the disk has already been formatted and verified.
 *
 */
int
make_hfsplus(const DriveInfo *driveInfo, hfsparams_t *defaults)
{
	UInt16			btNodeSize;
	UInt32			sectorsPerBlock;
	UInt32			mapNodes;
	UInt32			sectorsPerNode;
	UInt32			temp;
	UInt32 			bytesUsed;
	UInt32			endOfAttributes;
	UInt32			startOfAllocation;
	UInt64			bytesToZero;
	void			*nodeBuffer = NULL;
	HFSPlusVolumeHeader	*header = NULL;
	UInt64			sector;

	/* Use wipefs() API to clear old metadata from the device.
	 * This should be done before we start writing anything on the 
	 * device as wipefs will internally call ioctl(DKIOCDISCARD) on the 
	 * entire device.
	 */
	(void) dowipefs(driveInfo->fd);

	/* --- Create an HFS Plus header:  */

	header = (HFSPlusVolumeHeader*)malloc((size_t)kBytesPerSector);
	if (header == NULL)
		err(1, NULL);

	/* VH Initialized in native byte order */
	InitVH(defaults, driveInfo->totalSectors, header);

	sectorsPerBlock = header->blockSize / kBytesPerSector;


	/*--- ZERO OUT BEGINNING OF DISK:  */
	/*
	 * Clear out the space to be occupied by the bitmap and B-Trees.
	 * The first chunk is the boot sectors, volume header, allocation bitmap,
	 * journal, Extents B-tree, and Attributes B-tree (if any).
	 * The second chunk is the Catalog B-tree.
	 */
	
	/* Zero out first 1M (to be safe) for volume header */
	WriteBuffer(driveInfo, 0, 1024*1024, NULL);

	if (NEWFS_HFS_DEBUG) {
		/*
		 * Mark each file extent as used individually, rather than doing it all at once.
		 * Also zero out the entire file.
		 */
# define MFU(f)								\
		do {							\
			WriteBuffer(driveInfo,				\
				    header->f.extents[0].startBlock * sectorsPerBlock, \
				    header->f.totalBlocks * header->blockSize, \
				    NULL);				\
			if (MarkExtentUsed(driveInfo, header, header->f.extents[0].startBlock, header->f.totalBlocks) == -1) { \
				errx(1, #f " extent overlap <%u, %u>", header->f.extents[0].startBlock, header->f.totalBlocks); \
			}						\
		} while (0)
		MFU(allocationFile);
		MFU(attributesFile);
		MFU(extentsFile);
# undef MFU
	} else {
		/* Zero out from start of allocation file to end of attribute file; 
		 * will include allocation bitmap, journal, extents btree, and 
		 * attribute btree
		 */
		sector = header->allocationFile.extents[0].startBlock * sectorsPerBlock;
		endOfAttributes = header->attributesFile.extents[0].startBlock + header->attributesFile.totalBlocks;
		startOfAllocation = header->allocationFile.extents[0].startBlock;
		bytesToZero = (UInt64) (endOfAttributes - startOfAllocation + 1) * header->blockSize;
		WriteBuffer(driveInfo, sector, bytesToZero, NULL);

		bytesToZero = (UInt64) header->catalogFile.totalBlocks * header->blockSize;
		sector = header->catalogFile.extents[0].startBlock * sectorsPerBlock;
		WriteBuffer(driveInfo, sector, bytesToZero, NULL);
	}
	/*
	 * Allocate a buffer for the rest of our IO.
	 * Note that in some cases we may need to initialize an EA, so we 
	 * need to use the attribute B-Tree node size in this calculation. 
	 */

	temp = Largest( defaults->catalogNodeSize * 2,
			(defaults->attributesNodeSize * 2),
			header->blockSize,
			(header->catalogFile.extents[0].startBlock + header->catalogFile.totalBlocks + 7) / 8 );
	/* 
	 * If size is not a mutiple of 512, round up to nearest sector
	 */
	if ( (temp & 0x01FF) != 0 )
		temp = (temp + kBytesPerSector) & 0xFFFFFE00;
	
	nodeBuffer = valloc((size_t)temp);
	if (nodeBuffer == NULL)
		err(1, NULL);


		
	/*--- WRITE ALLOCATION BITMAP BITS TO DISK:  */

	/*
	 * XXX - this doesn't work well with using arbitrary extents.
	 *
	 * To do this, we need to find the appropriate area in the file, and
	 * pass that in to AllocateExtent, which is just a bitmap manipulation
	 * routine.  Then we need to write it out at the right place.  Note that
	 * we may have to read it in first, as well, which may mean zeroing out
	 * the entirety of the allocation file first.
	 *
	 * Possible solution:
	 * New function to mark extent as used.
	 * Function should figure out which block(s) for an extent.
	 * Read it in.  Mark the bits used.  Return.
	 * For now, it can assume the allocation extents are contiguous, but
	 * should be extensible to not do that.
	 */
	sector = header->allocationFile.extents[0].startBlock * sectorsPerBlock;
	bzero(nodeBuffer, temp);
	/* Mark volume header as allocated */
	if (header->blockSize == 512) {
		if (MarkExtentUsed(driveInfo, header, 0, 4) == -1) {
			errx(1, "Overlapped extent at <0, 4> (%d)", __LINE__);
		}
	} else if (header->blockSize == 1024) {
		if (MarkExtentUsed(driveInfo, header, 0, 2) == -1) {
			errx(1, "Overlapped extent at <0, 2> (%d)", __LINE__);
		}
	} else {
		if (MarkExtentUsed(driveInfo, header, 0, 1) == -1) {
			errx(1, "Overlapped extent at <0, 1> (%d)", __LINE__);
		}
	}
	if (NEWFS_HFS_DEBUG == 0) {
		/* Mark area from bitmap to end of attributes as allocated */
		if (MarkExtentUsed(driveInfo, header, startOfAllocation, (endOfAttributes - startOfAllocation)) == -1) {
			errx(1, "Overlapped extent at <%u, %u> (%d)\n", startOfAllocation, endOfAttributes - startOfAllocation, __LINE__);
		}
	}

	/* Mark catalog btree blocks as allocated */
	if (NEWFS_HFS_DEBUG) {
		/* Erase the catalog file first */
		WriteBuffer(driveInfo,
			    header->catalogFile.extents[0].startBlock * sectorsPerBlock,
			    header->catalogFile.totalBlocks * header->blockSize,
			    NULL);
	}
	if (MarkExtentUsed(driveInfo, header,
			     header->catalogFile.extents[0].startBlock,
			   header->catalogFile.totalBlocks) == -1) {
		errx(1, "Overlapped catalog extent at <%u, %u>\n", header->catalogFile.extents[0].startBlock, header->catalogFile.totalBlocks);
	}
	
	/*
	 * Write alternate Volume Header bitmap bit to allocations file at
	 * 2nd to last sector on HFS+ volume
	 */
	if (MarkExtentUsed(driveInfo, header, header->totalBlocks - 1, 1) == -1) {
		errx(1, "Overlapped extent for header at <%u, %u>\n", header->totalBlocks - 1, 1);
	}

	/*
	 * If the blockSize is 512 bytes, then the last 1kbyte has to be marked
	 * used via two bits.
	 */
	if ( header->blockSize == 512 ) {
		if (MarkExtentUsed(driveInfo, header, header->totalBlocks - 2, 1) == -1) {
			errx(1, "Overlapped extent for AVH at <%u, %u>\n", header->totalBlocks - 2, 1);
		}
	
	}

	/*--- WRITE FILE EXTENTS B-TREE TO DISK:  */

	btNodeSize = defaults->extentsNodeSize;
	sectorsPerNode = btNodeSize/kBytesPerSector;

	sector = header->extentsFile.extents[0].startBlock * sectorsPerBlock;
	WriteExtentsFile(driveInfo, sector, defaults, NULL, nodeBuffer, &bytesUsed, &mapNodes);

	if (mapNodes > 0) {
		WriteMapNodes(driveInfo, (sector + bytesUsed/kBytesPerSector),
			bytesUsed/btNodeSize, mapNodes, btNodeSize, nodeBuffer);
	}

	

	/*--- WRITE FILE ATTRIBUTES B-TREE TO DISK:  */
	if (defaults->attributesInitialSize) {

		btNodeSize = defaults->attributesNodeSize;
		sectorsPerNode = btNodeSize/kBytesPerSector;
	
		sector = header->attributesFile.extents[0].startBlock * sectorsPerBlock;
		WriteAttributesFile(driveInfo, sector, defaults, NULL, nodeBuffer, &bytesUsed, &mapNodes);
		if (mapNodes > 0) {
			WriteMapNodes(driveInfo, (sector + bytesUsed/kBytesPerSector),
				bytesUsed/btNodeSize, mapNodes, btNodeSize, nodeBuffer);
		}
	}
	
	/*--- WRITE CATALOG B-TREE TO DISK:  */
	
	btNodeSize = defaults->catalogNodeSize;
	sectorsPerNode = btNodeSize/kBytesPerSector;

	sector = header->catalogFile.extents[0].startBlock * sectorsPerBlock;
	WriteCatalogFile(driveInfo, sector, defaults, header, nodeBuffer, &bytesUsed, &mapNodes);

	if (mapNodes > 0) {
		WriteMapNodes(driveInfo, (sector + bytesUsed/kBytesPerSector),
			bytesUsed/btNodeSize, mapNodes, btNodeSize, nodeBuffer);
	}

	/*--- JOURNALING SETUP */
	if (defaults->journaledHFS) {
	    sector = header->journalInfoBlock * sectorsPerBlock;
	    if (NEWFS_HFS_DEBUG) {
		    /*
		     * For debug build, the journal may be located somewhere other
		     * than right after the journalInfoBlock.
		     */
		    if (MarkExtentUsed(driveInfo, header, header->journalInfoBlock, 1) == -1) {
			    errx(1, "Extent overlap for journalInfoBlock <%u, 1>", header->journalInfoBlock);
		    }

		    if (!defaults->journalDevice) {
			    UInt32 jStart = defaults->journalBlock ? defaults->journalBlock : (header->journalInfoBlock + 1);
			    UInt32 jCount = (UInt32)(defaults->journalSize / header->blockSize);
			    if (MarkExtentUsed(driveInfo, header, jStart, jCount) == -1) {
				    errx(1, "Extent overlap for journal <%u, %u>", jStart, jCount);
			    }
		    }
	    }
	    if (WriteJournalInfo(driveInfo, sector, defaults, header, nodeBuffer) != 0) {
		err(EINVAL, "Failed to create the journal");
	    }
	}
	
	/*--- WRITE VOLUME HEADER TO DISK:  */

	/* write header last in case we fail along the way */

	/* Writes both copies of the volume header */
	WriteVH (driveInfo, header);
	/* VH is now big-endian */

	free(nodeBuffer);
	free(header);

	return (0);
}

/*
 * WriteVH
 *
 * Writes the Volume Header (VH) to disk.
 *
 * The VH is byte-swapped if necessary to big endian. Since this
 * is always the last operation, there's no point in unswapping it.
 */
static void
WriteVH (const DriveInfo *driveInfo, HFSPlusVolumeHeader *hp)
{
	SWAP_HFSPLUSVH (hp);

	WriteBuffer(driveInfo, 2, kBytesPerSector, hp);
	WriteBuffer(driveInfo, driveInfo->totalSectors - 2, kBytesPerSector, hp);
}


/*
 * InitVH
 *
 * Initialize a Volume Header record.
 */
static void
InitVH(hfsparams_t *defaults, UInt64 sectors, HFSPlusVolumeHeader *hp)
{
	UInt32	blockSize;
	UInt32	blockCount;
	UInt32	blocksUsed;
	UInt32	bitmapBlocks;
	UInt16	burnedBlocksBeforeVH = 0;
	UInt16	burnedBlocksAfterAltVH = 0;
	UInt32  nextBlock;
	UInt32	allocateBlock;
	VolumeUUID	newVolumeUUID;	
	VolumeUUID*	finderInfoUUIDPtr;
	UInt64	hotFileBandSize;
	UInt64 volsize;

/* 
 * 2 MB is the minimum size for the new behavior with 
 * space after the attr b-tree, and hotfile stuff. 
 */
#define MINVOLSIZE_WITHSPACE 2097152 

	bzero(hp, kBytesPerSector);

	blockSize = defaults->blockSize;
	blockCount = (UInt32)(sectors / (blockSize >> kLog2SectorSize));

	/*
	 * HFSPlusVolumeHeader is located at sector 2, so we may need
	 * to invalidate blocks before HFSPlusVolumeHeader.
	 */
	if ( blockSize == 512 ) {
		burnedBlocksBeforeVH = 2;		/* 2 before VH */
		burnedBlocksAfterAltVH = 1;		/* 1 after altVH */
	} else if ( blockSize == 1024 ) {
		burnedBlocksBeforeVH = 1;
	}
	nextBlock = burnedBlocksBeforeVH + 1;		/* +1 for VH itself */
	if (defaults->fsStartBlock) {
		if (NEWFS_HFS_DEBUG)
			printf ("Laying down metadata starting at allocation block=%u (totalBlocks=%u)\n", (unsigned int)defaults->fsStartBlock, (unsigned int)blockCount);
		nextBlock += defaults->fsStartBlock;	/* lay down file system after this allocation block */
	}
	
	bitmapBlocks = defaults->allocationClumpSize / blockSize;

	/* note: add 2 for the Alternate VH, and VH */
	blocksUsed = 2 + burnedBlocksBeforeVH + burnedBlocksAfterAltVH + bitmapBlocks;

	if (defaults->flags & kMakeCaseSensitive) {
		hp->signature = kHFSXSigWord;
		hp->version = kHFSXVersion;
	} else {
		hp->signature = kHFSPlusSigWord;
		hp->version = kHFSPlusVersion;
	}
	hp->attributes = kHFSVolumeUnmountedMask | kHFSUnusedNodeFixMask;
	if (defaults->flags & kMakeContentProtect) {
		hp->attributes |= kHFSContentProtectionMask;	
	}
	hp->lastMountedVersion = kHFSPlusMountVersion;

	/* NOTE: create date is in local time, not GMT!  */
	hp->createDate = UTCToLocal(defaults->createDate);
	hp->modifyDate = defaults->createDate;
	hp->backupDate = 0;
	hp->checkedDate = defaults->createDate;

//	hp->fileCount = 0;
//	hp->folderCount = 0;

	hp->blockSize = blockSize;
	hp->totalBlocks = blockCount;
	hp->freeBlocks = blockCount;	/* will be adjusted at the end */

	volsize = (UInt64) blockCount * (UInt64) blockSize;

	hp->rsrcClumpSize = defaults->rsrcClumpSize;
	hp->dataClumpSize = defaults->dataClumpSize;
	hp->nextCatalogID = defaults->nextFreeFileID;
	hp->encodingsBitmap = 1;

	/* set up allocation bitmap file */
	hp->allocationFile.clumpSize = defaults->allocationClumpSize;
	hp->allocationFile.logicalSize = defaults->allocationClumpSize;
	hp->allocationFile.totalBlocks = bitmapBlocks;

	if (NEWFS_HFS_DEBUG && defaults->allocationStartBlock)
		allocateBlock = defaults->allocationStartBlock;
	else {
		allocateBlock = nextBlock;
		nextBlock += bitmapBlocks;
	}

	createExtents(&hp->allocationFile, kHFSAllocationFileID, allocateBlock, defaults->allocationExtsCount, 1);

	// This works because the files are contiguous for now
	if (NEWFS_HFS_DEBUG)
		printf ("allocationFile: (%10u, %10u)\n", hp->allocationFile.extents[0].startBlock, hp->allocationFile.totalBlocks);

	/* set up journal files */
	if (defaults->journaledHFS) {
		UInt32		journalBlock;
		hp->fileCount           = 2;
		hp->attributes         |= kHFSVolumeJournaledMask;
		hp->nextCatalogID      += 2;
		
		/*
		 * Allocate 1 block for the journalInfoBlock.  The
		 * journal file size is passed in hfsparams_t.
		 */
		if (NEWFS_HFS_DEBUG && defaults->journalInfoBlock)
			hp->journalInfoBlock = defaults->journalInfoBlock;
		else
			hp->journalInfoBlock = nextBlock++;
		if (NEWFS_HFS_DEBUG && defaults->journalBlock)
			journalBlock = defaults->journalBlock;
		else {
			journalBlock = hp->journalInfoBlock + 1;
			nextBlock += ((defaults->journalSize+blockSize-1) / blockSize);
		}

		if (NEWFS_HFS_DEBUG) {
			printf ("journalInfo   : (%10u, %10u)\n", (u_int32_t)hp->journalInfoBlock, 1);
			printf ("journal       : (%10u, %10u)\n", (u_int32_t)journalBlock, (u_int32_t)((defaults->journalSize + (blockSize-1)) / blockSize));
		}
	    /* XXX What if journal is on a different device? */
	    blocksUsed += 1 + ((defaults->journalSize+blockSize-1) / blockSize);
	} else {
	    hp->journalInfoBlock = 0;
	}

	/* set up extents b-tree file */
	hp->extentsFile.clumpSize = defaults->extentsClumpSize;
	hp->extentsFile.logicalSize = defaults->extentsInitialSize;
	hp->extentsFile.totalBlocks = defaults->extentsInitialSize / blockSize;
	if (NEWFS_HFS_DEBUG && defaults->extentsStartBlock)
		allocateBlock = defaults->extentsStartBlock;
	else {
		allocateBlock = nextBlock;
		nextBlock += hp->extentsFile.totalBlocks;
	}
	createExtents(&hp->extentsFile, kHFSExtentsFileID, allocateBlock, defaults->extentsExtsCount, (defaults->journaledHFS && defaults->extentsNodeSize > hp->blockSize) ? defaults->extentsNodeSize / hp->blockSize : 1);

	blocksUsed += hp->extentsFile.totalBlocks;

	if (NEWFS_HFS_DEBUG)
		printf ("extentsFile   : (%10u, %10u)\n", hp->extentsFile.extents[0].startBlock, hp->extentsFile.totalBlocks);

	/* set up attributes b-tree file */
	if (defaults->attributesInitialSize) {
		hp->attributesFile.clumpSize = defaults->attributesClumpSize;
		hp->attributesFile.logicalSize = defaults->attributesInitialSize;
		hp->attributesFile.totalBlocks = defaults->attributesInitialSize / blockSize;
		if (NEWFS_HFS_DEBUG && defaults->attributesStartBlock)
			allocateBlock = defaults->attributesStartBlock;
		else {
			allocateBlock = nextBlock;
			nextBlock += hp->attributesFile.totalBlocks;
		}
		createExtents(&hp->attributesFile, kHFSAttributesFileID, allocateBlock, defaults->attributesExtsCount, (defaults->journaledHFS && defaults->attributesNodeSize > hp->blockSize) ? defaults->attributesNodeSize / hp->blockSize : 1);
		blocksUsed += hp->attributesFile.totalBlocks;

		if (NEWFS_HFS_DEBUG) {
			printf ("attributesFile: (%10u, %10u)\n", hp->attributesFile.extents[0].startBlock, hp->attributesFile.totalBlocks);
		}
		/*
		 * Leave some room for the Attributes B-tree to grow, if the volsize >= 2MB
		 */
		if (volsize >= MINVOLSIZE_WITHSPACE && defaults->attributesStartBlock == 0) {
			nextBlock += 10 * (hp->attributesFile.clumpSize / blockSize);
		}
	}

	/* set up catalog b-tree file */
	hp->catalogFile.clumpSize = defaults->catalogClumpSize;
	hp->catalogFile.logicalSize = defaults->catalogInitialSize;
	hp->catalogFile.totalBlocks = defaults->catalogInitialSize / blockSize;
	if (NEWFS_HFS_DEBUG && defaults->catalogStartBlock)
		allocateBlock = defaults->catalogStartBlock;
	else {
		allocateBlock = nextBlock;
		nextBlock += hp->catalogFile.totalBlocks;
	}
	createExtents(&hp->catalogFile, kHFSCatalogFileID, allocateBlock, defaults->catalogExtsCount, (defaults->journaledHFS && defaults->catalogNodeSize > hp->blockSize) ? defaults->catalogNodeSize / hp->blockSize : 1);
	blocksUsed += hp->catalogFile.totalBlocks;

	if (NEWFS_HFS_DEBUG)
		printf ("catalogFile   : (%10u, %10u)\n\n", hp->catalogFile.extents[0].startBlock, hp->catalogFile.totalBlocks);

	if ((numOverflowExtents * sizeof(struct ExtentRecord)) >
	    (defaults->extentsNodeSize - sizeof(BTNodeDescriptor) - (sizeof(uint16_t) * numOverflowExtents))) {
		errx(1, "Too many overflow extent records to fit into a single extent node");
	}

	/*
	 * Add some room for the catalog file to grow...
	 */
	nextBlock += 10 * (hp->catalogFile.clumpSize / hp->blockSize);

	/*
	 * Add some room for the hot file band.  This uses the same 5MB per GB
	 * as the kernel.  The kernel only uses hotfiles if the volume is larger
	 * than 10GBytes, so do the same here.
	 */
#define	METADATAZONE_MINIMUM_VOLSIZE	(10ULL * 1024ULL * 1024ULL * 1024ULL)
#define HOTBAND_MINIMUM_SIZE  (10*1024*1024)
#define HOTBAND_MAXIMUM_SIZE  (512*1024*1024)
	if (volsize >= METADATAZONE_MINIMUM_VOLSIZE) {
		hotFileBandSize = (UInt64) blockCount * blockSize / 1024 * 5;
		if (hotFileBandSize > HOTBAND_MAXIMUM_SIZE)
			hotFileBandSize = HOTBAND_MAXIMUM_SIZE;
		else if (hotFileBandSize < HOTBAND_MINIMUM_SIZE)
			hotFileBandSize = HOTBAND_MINIMUM_SIZE;
		nextBlock += hotFileBandSize / blockSize;
	}
	if (NEWFS_HFS_DEBUG && defaults->nextAllocBlock)
		hp->nextAllocation = defaults->nextAllocBlock;
	else
		hp->nextAllocation = nextBlock;
	
	/* Adjust free blocks to reflect everything we have allocated. */
	hp->freeBlocks -= blocksUsed;

	/* Generate and write UUID for the HFS+ disk */
	GenerateVolumeUUID(&newVolumeUUID);
	finderInfoUUIDPtr = (VolumeUUID *)(&hp->finderInfo[24]);
	finderInfoUUIDPtr->v.high = OSSwapHostToBigInt32(newVolumeUUID.v.high); 
	finderInfoUUIDPtr->v.low = OSSwapHostToBigInt32(newVolumeUUID.v.low); 
}

/*
 * AllocateExtent
 *
 * Mark the given extent as in-use in the given bitmap buffer.
 */
static int AllocateExtent(UInt8 *buffer, UInt32 startBlock, UInt32 blockCount)
{
	UInt8 *p;
	
	/* Point to start of extent in bitmap buffer */
	p = buffer + (startBlock / 8);
	
	/*
	 * Important to remember: block 0 is (1 << 7);
	 * block 7 is (1 << 0).
	 */
	/* Partial byte at start of extent */
	if (startBlock & 7)
	{
		UInt8 mask = 0xff;
		unsigned int lShift = 0;
		unsigned int startBit = startBlock & 7;

		/*
		 * Is startBlock + blockCount entirely in
		 * p[0]?
		 */
		if (blockCount < (8 - startBit)) {
			lShift = 8 - (startBit + blockCount);
		}
		mask = (0xff >> startBit) & (0xff << lShift);
		if (NEWFS_HFS_DEBUG && (*p & mask)) {
			fprintf(stderr, "%s(%d):  expected 0, got %x\n", __FUNCTION__, __LINE__, *p & mask);
			return -1;
		}
		*(p++) |= mask;
		/*
		 * We have either set <lShift> or <startBlock & 7> bits.
		 */
		blockCount -= 8 - (lShift + startBit);
//		blockCount -= lShift ? blockCount : (8 - startBit);
//		blockCount -= __builtin_popcount(mask);
	}
	
	/* Fill in whole bytes */
	if (blockCount >= 8)
	{
		if (NEWFS_HFS_DEBUG) {
			/*
			 * Put this in ifdef because it'll slow things down.
			 * For non-debug case, we shouldn't have to worry about
			 * an overlap, anyway.
			 */
			size_t indx;
			for (indx = 0; indx < blockCount / 8; indx++) {
				if (p[indx] != 0) {
					fprintf(stderr, "%s(%d):  Expected 0 at %zu, got 0x%x\n", __FUNCTION__, __LINE__, indx, p[indx]);
					return -1;
				}
				p[indx] = 0xff;
			}
		} else {
			memset(p, 0xFF, blockCount / 8);
		}
		p += blockCount / 8;
		blockCount &= 7;
	}
	
	/* Partial byte at end of extent */
	if (blockCount)
	{
		UInt8 mask = 0xff << (8 - blockCount);
		if (NEWFS_HFS_DEBUG && (*p & mask)) {
			fprintf(stderr, "%s(%d):  Expected 0, got %x\n", __FUNCTION__, __LINE__, *p & mask);
			return -1;
		}
		*(p++) |= mask;
	}
	return 0;
}

/*
 * Mark an extent as being used.
 * This involves finding out where the allocations file is,
 * where in the allocations file the extent starts, and how
 * long it runs.
 *
 * One downside to this implementation is that this does
 * more I/O than the old mechanism, a cost to the flexibility.
 * May have to consider doing caching of some sort.
 */

static int
MarkExtentUsed(const DriveInfo *driveInfo,
	       HFSPlusVolumeHeader *header,
	       UInt32 startBlock,
	       UInt32 blockCount)
{
	size_t bufSize = driveInfo->physSectorSize;
	void *buf;
	uint32_t blocksLeft = blockCount;
	uint32_t curBlock = startBlock;
	static const int kBitsPerByte = 8;
	int status = -1;

	buf = valloc(bufSize);
	if (buf == NULL)
		err(1, NULL);

	/*
	 * We loop through physSectorSize blocks.
	 * This allows us to set as many bits as we need.
	 */
	while (blocksLeft > 0) {
		off_t secNum;
		uint32_t numBlocks;	// The number of blocks to mark as used in this pass.
		uint32_t blockOffset;	// This is the block number of the current range, which starts at curBlock

		memset(buf, 0, sizeof(buf));
		secNum = curBlock / (bufSize * kBitsPerByte);
		blockOffset = curBlock % (bufSize * kBitsPerByte);
		numBlocks = (uint32_t)MIN((bufSize * kBitsPerByte) - blockOffset, blocksLeft);

		/*
		 * Okay, now we've got the block number to read,
		 * the offset into the block, and the number of blocks
		 * to set.
		 *
		 * First we read in the buffer.  To do that, we need to
		 * know where to read.
		 */
		ssize_t nbytes;
		ssize_t nwritten;
		off_t offset;

		/*
		 * XXX
		 * This needs to be changed if/when we support non-contiguous multiple
		 * extents.  At that point, it'll probably have to be a function to search
		 * for the requested offset.  (How many times must MapFileC be written?)
		 * For now, though, the offset is the physical sector offset from the
		 * start of the allocations file.
		 */
		offset = (header->allocationFile.extents[0].startBlock * header->blockSize) +
			(secNum * bufSize);

		nbytes = pread(driveInfo->fd, buf, bufSize, offset);

		if (nbytes < (ssize_t)bufSize) {
			if (nbytes == -1)
				err(1, "%s::pread(%d, %p, %zu, %lld)", __FUNCTION__, driveInfo->fd, buf, bufSize, offset);
			goto exit;
		}

		if (AllocateExtent(buf, blockOffset, numBlocks) == -1) {
			warnx("In-use allocation block in <%u, %u>", blockOffset, numBlocks);
			goto exit;
		}
		nwritten = pwrite(driveInfo->fd, buf, bufSize, offset);
		/*
		 * Normally I'd check for nwritten to be less than bufSize, but since bufSize is
		 * the physical sector size, we shouldn't be able to get less.  So that most likely
		 * means a return value of 0 or -1, neither of which I could do anything about.
		 */
		if (nwritten != (ssize_t)bufSize)
			goto exit;

		// And go get the next set, if needed
		blocksLeft -= numBlocks;
		curBlock += numBlocks;
	}

	status = 0;

exit:
	free(buf);

	return status;
}
/*
 * WriteExtentsFile
 *
 * Initializes and writes out the extents b-tree file.
 *
 * Byte swapping is performed in place. The buffer should not be
 * accessed through direct casting once it leaves this function.
 */
static void
WriteExtentsFile(const DriveInfo *driveInfo, UInt64 startingSector,
        const hfsparams_t *dp, HFSExtentDescriptor *bbextp __unused , void *buffer,
        UInt32 *bytesUsed, UInt32 *mapNodes)
{
	BTNodeDescriptor	*ndp;
	BTHeaderRec		*bthp;
	UInt8			*bmp;
	UInt32			nodeBitsInHeader;
	UInt32			fileSize;
	UInt32			nodeSize;
	UInt32			temp;
	SInt16			offset;

	*mapNodes = 0;
	fileSize = dp->extentsInitialSize;
	nodeSize = dp->extentsNodeSize;

	bzero(buffer, nodeSize);


	/* FILL IN THE NODE DESCRIPTOR:  */
	ndp = (BTNodeDescriptor *)buffer;
	ndp->kind			= kBTHeaderNode;
	ndp->numRecords		= SWAP_BE16 (3);
	offset = sizeof(BTNodeDescriptor);

	SETOFFSET(buffer, nodeSize, offset, 1);


	/* FILL IN THE HEADER RECORD:  */
	bthp = (BTHeaderRec *)((UInt8 *)buffer + offset);
	if (numOverflowExtents) {
		bthp->treeDepth = SWAP_BE16(1);
		bthp->rootNode = SWAP_BE32(1);
		bthp->firstLeafNode = SWAP_BE32(1);
		bthp->lastLeafNode = SWAP_BE32(1);
		bthp->leafRecords = SWAP_BE32(numOverflowExtents);
	} else {
		bthp->treeDepth = 0;
		bthp->rootNode = 0;
		bthp->firstLeafNode = 0;
		bthp->lastLeafNode = 0;
		bthp->leafRecords = 0;
	}	

	bthp->nodeSize		= SWAP_BE16 (nodeSize);
	bthp->totalNodes	= SWAP_BE32 (fileSize / nodeSize);
	bthp->freeNodes		= SWAP_BE32 (SWAP_BE32 (bthp->totalNodes) - (numOverflowExtents ? 2 : 1));  /* header */
	bthp->clumpSize		= SWAP_BE32 (dp->extentsClumpSize);

	bthp->attributes |= SWAP_BE32 (kBTBigKeysMask);
	bthp->maxKeyLength = SWAP_BE16 (kHFSPlusExtentKeyMaximumLength);
	offset += sizeof(BTHeaderRec);

	SETOFFSET(buffer, nodeSize, offset, 2);

	offset += kBTreeHeaderUserBytes;

	SETOFFSET(buffer, nodeSize, offset, 3);


	/* FIGURE OUT HOW MANY MAP NODES (IF ANY):  */
	nodeBitsInHeader = 8 * (nodeSize
					- sizeof(BTNodeDescriptor)
					- sizeof(BTHeaderRec)
					- kBTreeHeaderUserBytes
					- (4 * sizeof(SInt16)) );

	if (SWAP_BE32 (bthp->totalNodes) > nodeBitsInHeader) {
		UInt32	nodeBitsInMapNode;
		
		ndp->fLink		= SWAP_BE32 (SWAP_BE32 (bthp->lastLeafNode) + 1);
		nodeBitsInMapNode = 8 * (nodeSize
						- sizeof(BTNodeDescriptor)
						- (2 * sizeof(SInt16))
						- 2 );
		*mapNodes = (SWAP_BE32 (bthp->totalNodes) - nodeBitsInHeader +
			(nodeBitsInMapNode - 1)) / nodeBitsInMapNode;
		bthp->freeNodes = SWAP_BE32 (SWAP_BE32 (bthp->freeNodes) - *mapNodes);
	}


	/* 
	 * FILL IN THE MAP RECORD, MARKING NODES THAT ARE IN USE.
	 * Note - worst case (32MB alloc blk) will have only 18 nodes in use.
	 */
	bmp = ((UInt8 *)buffer + offset);
	temp = SWAP_BE32 (bthp->totalNodes) - SWAP_BE32 (bthp->freeNodes);

	/* Working a byte at a time is endian safe */
	while (temp >= 8) { *bmp = 0xFF; temp -= 8; bmp++; }
	*bmp = ~(0xFF >> temp);
	offset += nodeBitsInHeader/8;

	SETOFFSET(buffer, nodeSize, offset, 4);
	
	if (NEWFS_HFS_DEBUG && numOverflowExtents) {
		void *node2 = (uint8_t*)buffer + nodeSize;
		size_t i;
		int (^keyCompare)(const void *l, const void *r) = ^(const void *l, const void *r) {
				const struct ExtentRecord *left = (const struct ExtentRecord*)l;
				const struct ExtentRecord *right = (const struct ExtentRecord*)r;
				if (SWAP_BE32(left->key.fileID) != SWAP_BE32(right->key.fileID)) {
					return (SWAP_BE32(left->key.fileID) > SWAP_BE32(right->key.fileID)) ? 1 : -1;
				}
				// forkType will always be 0 for us
				if (SWAP_BE32(left->key.startBlock) != SWAP_BE32(right->key.startBlock)) {
					return (SWAP_BE32(left->key.startBlock) > SWAP_BE32(right->key.startBlock)) ? 1 : -1;
				}
				return 0;
		};

		if (numOverflowExtents > 1) {
			qsort_b(overflowExtents, numOverflowExtents, sizeof(*overflowExtents), keyCompare);
		}
		bzero(node2, nodeSize);
		ndp = (BTNodeDescriptor*)node2;
		ndp->kind = kBTLeafNode;
		ndp->numRecords = SWAP_BE16(numOverflowExtents);
		ndp->height = 1;

		offset = sizeof(BTNodeDescriptor);
		for (i = 0; i < numOverflowExtents; i++) {
			SETOFFSET(node2, nodeSize, offset, 1 + i);
			memcpy(node2 + offset, &overflowExtents[i], sizeof(*overflowExtents));
			offset += sizeof(*overflowExtents);
		}
		SETOFFSET(node2, nodeSize, offset, numOverflowExtents + 1);
	}

	*bytesUsed = (SWAP_BE32 (bthp->totalNodes) - SWAP_BE32 (bthp->freeNodes) - *mapNodes) * nodeSize;

	WriteBuffer(driveInfo, startingSector, *bytesUsed, buffer);

}

/*
 * WriteAttributesFile
 *
 * Initializes and writes out the attributes b-tree file.
 *
 * Byte swapping is performed in place. The buffer should not be
 * accessed through direct casting once it leaves this function.
 */
static void
WriteAttributesFile(const DriveInfo *driveInfo, UInt64 startingSector,
        const hfsparams_t *dp, HFSExtentDescriptor *bbextp __unused, void *buffer,
        UInt32 *bytesUsed, UInt32 *mapNodes)
{
	BTNodeDescriptor	*ndp;
	BTHeaderRec		*bthp;
	UInt8			*bmp;
	UInt32			nodeBitsInHeader;
	UInt32			fileSize;
	UInt32			nodeSize;
	UInt32			temp;
	SInt16			offset;
	int	set_cp_level = 0;

	*mapNodes = 0;
	fileSize = dp->attributesInitialSize;
	nodeSize = dp->attributesNodeSize;

#ifdef DEBUG_BUILD
	/*
	 * If user specified content protection and a protection level,
	 * then verify the protection level is sane.
	 */ 	
	if ((dp->flags & kMakeContentProtect) && (dp->protectlevel != 0)) {
		if ((dp->protectlevel >= 2 ) && (dp->protectlevel <= 4)) {
			set_cp_level = 1;
		}	
	}
#endif


	bzero(buffer, nodeSize);


	/* FILL IN THE NODE DESCRIPTOR:  */
	ndp = (BTNodeDescriptor *)buffer;
	ndp->kind			= kBTHeaderNode;
	ndp->numRecords		= SWAP_BE16 (3);
	offset = sizeof(BTNodeDescriptor);

	SETOFFSET(buffer, nodeSize, offset, 1);


	/* FILL IN THE HEADER RECORD:  */
	bthp = (BTHeaderRec *)((UInt8 *)buffer + offset);
	if (set_cp_level) {
		bthp->treeDepth = SWAP_BE16(1);
		bthp->rootNode = SWAP_BE32(1);
		bthp->firstLeafNode = SWAP_BE32(1);
		bthp->lastLeafNode = SWAP_BE32(1);
		bthp->leafRecords = SWAP_BE32(1);
	}
	else {
		bthp->treeDepth = 0;
		bthp->rootNode = 0;
		bthp->firstLeafNode = 0;
		bthp->lastLeafNode = 0;
		bthp->leafRecords = 0;
	}	

	bthp->nodeSize		= SWAP_BE16 (nodeSize);
	bthp->totalNodes	= SWAP_BE32 (fileSize / nodeSize);
	if (set_cp_level) {
		/* Add 1 node for the first record */
		bthp->freeNodes = SWAP_BE32 (SWAP_BE32 (bthp->totalNodes) - 2); 
	}
	else {
		/* Take the header into account */
		bthp->freeNodes		= SWAP_BE32 (SWAP_BE32 (bthp->totalNodes) - 1);
	}
	bthp->clumpSize		= SWAP_BE32 (dp->attributesClumpSize);

	bthp->attributes |= SWAP_BE32 (kBTBigKeysMask | kBTVariableIndexKeysMask);
	bthp->maxKeyLength = SWAP_BE16 (kHFSPlusAttrKeyMaximumLength);

	offset += sizeof(BTHeaderRec);

	SETOFFSET(buffer, nodeSize, offset, 2);

	offset += kBTreeHeaderUserBytes;

	SETOFFSET(buffer, nodeSize, offset, 3);


	/* FIGURE OUT HOW MANY MAP NODES (IF ANY):  */
	nodeBitsInHeader = 8 * (nodeSize
					- sizeof(BTNodeDescriptor)
					- sizeof(BTHeaderRec)
					- kBTreeHeaderUserBytes
					- (4 * sizeof(SInt16)) );
	if (SWAP_BE32 (bthp->totalNodes) > nodeBitsInHeader) {
		UInt32	nodeBitsInMapNode;
		
		ndp->fLink		= SWAP_BE32 (SWAP_BE32 (bthp->lastLeafNode) + 1);
		nodeBitsInMapNode = 8 * (nodeSize
						- sizeof(BTNodeDescriptor)
						- (2 * sizeof(SInt16))
						- 2 );
		*mapNodes = (SWAP_BE32 (bthp->totalNodes) - nodeBitsInHeader +
			(nodeBitsInMapNode - 1)) / nodeBitsInMapNode;
		bthp->freeNodes = SWAP_BE32 (SWAP_BE32 (bthp->freeNodes) - *mapNodes);
	}


	/* 
	 * FILL IN THE MAP RECORD, MARKING NODES THAT ARE IN USE.
	 * Note - worst case (32MB alloc blk) will have only 18 nodes in use.
	 */
	bmp = ((UInt8 *)buffer + offset);
	temp = SWAP_BE32 (bthp->totalNodes) - SWAP_BE32 (bthp->freeNodes);

	/* Working a byte at a time is endian safe */
	while (temp >= 8) { *bmp = 0xFF; temp -= 8; bmp++; }
	*bmp = ~(0xFF >> temp);
	offset += nodeBitsInHeader/8;

	SETOFFSET(buffer, nodeSize, offset, 4);

#ifdef DEBUG_BUILD
	if (set_cp_level) {
		/* Stuff in the EA on the root folder */
		void *node2 = (uint8_t*)buffer + nodeSize;

		struct cp_root_xattr ea;

		uint8_t canonicalName[256];
		CFStringRef cfstr;

		HFSPlusAttrData *attrData;
		HFSPlusAttrKey *attrKey;
		bzero(node2, nodeSize);
		ndp = (BTNodeDescriptor*)node2;

		ndp->kind = kBTLeafNode;
		ndp->numRecords = SWAP_BE16(1);
		ndp->height = 1;

		offset = sizeof(BTNodeDescriptor);
		SETOFFSET(node2, nodeSize, offset, 1);

		attrKey = (HFSPlusAttrKey*)((uint8_t*)node2 + offset);
		attrKey->fileID = SWAP_BE32(1);
		attrKey->startBlock = 0;
		attrKey->keyLength = SWAP_BE16(sizeof(*attrKey) - sizeof(attrKey->keyLength));

		cfstr = CFStringCreateWithCString(kCFAllocatorDefault, "com.apple.system.cprotect", kCFStringEncodingUTF8);
		if (_CFStringGetFileSystemRepresentation(cfstr, canonicalName, sizeof(canonicalName)) &&
				ConvertUTF8toUnicode(canonicalName,
					sizeof(attrKey->attrName),
					attrKey->attrName, &attrKey->attrNameLen) == 0) {
			attrKey->attrNameLen = SWAP_BE16(attrKey->attrNameLen);
			offset += sizeof(*attrKey);

			/* If the offset is odd, move up to the next even value */
			if (offset & 1) {
				offset++;
			}

			attrData = (HFSPlusAttrData*)((uint8_t*)node2 + offset);
			bzero(&ea, sizeof(ea));
			ea.vers = OSSwapHostToLittleInt16(dp->protectlevel); //(leave in LittleEndian)
			attrData->recordType = SWAP_BE32(kHFSPlusAttrInlineData);
			attrData->attrSize = SWAP_BE32(sizeof(ea));
			memcpy(attrData->attrData, &ea, sizeof(ea));
			offset += sizeof (HFSPlusAttrData) + sizeof(ea) - sizeof(attrData->attrData);
		}
		SETOFFSET (node2, nodeSize, offset, 2);
		CFRelease(cfstr);
	}
#endif

	*bytesUsed = (SWAP_BE32 (bthp->totalNodes) - SWAP_BE32 (bthp->freeNodes) - *mapNodes) * nodeSize;
	WriteBuffer(driveInfo, startingSector, *bytesUsed, buffer);
}

#if !TARGET_OS_IPHONE
static int
get_dev_uuid(const char *disk_name, char *dev_uuid_str, int dev_uuid_len)
{
    io_service_t service;
    CFStringRef uuid_str;
    int ret = EINVAL;

    if (strncmp(disk_name, _PATH_DEV, strlen(_PATH_DEV)) == 0) {
	disk_name += strlen(_PATH_DEV);
    }

    dev_uuid_str[0] = '\0';

    service = IOServiceGetMatchingService(kIOMasterPortDefault, IOBSDNameMatching(kIOMasterPortDefault, 0, disk_name));
    if (service != IO_OBJECT_NULL) {
	uuid_str = IORegistryEntryCreateCFProperty(service, CFSTR(kIOMediaUUIDKey), kCFAllocatorDefault, 0);
	if (uuid_str) {
	    if (CFStringGetFileSystemRepresentation(uuid_str, dev_uuid_str, dev_uuid_len) != 0) {
		ret = 0;
	    }
	    CFRelease(uuid_str);
	}
	IOObjectRelease(service);
    }

    return ret;
}

static int
clear_journal_dev(const char *dev_name)
{
    int fd;

    fd = open(dev_name, O_RDWR);
    if (fd < 0) {
	printf("Failed to open the journal device %s (%s)\n", dev_name, strerror(errno));
	return -1;
    }

    dowipefs(fd);

    close(fd);
    return 0;
}
#endif /* !(TARGET_OS_IPHONE) */


static int
WriteJournalInfo(const DriveInfo *driveInfo, UInt64 startingSector,
		 const hfsparams_t *dp, HFSPlusVolumeHeader *header,
		 void *buffer)
{
    JournalInfoBlock *jibp = buffer;
    UInt32 journalBlock;

    memset(buffer, 0xdb, driveInfo->physSectorSize);
    memset(jibp, 0, sizeof(JournalInfoBlock));
    
	//initialize it to start in FS
	jibp->flags = kJIJournalInFSMask;

#if TARGET_OS_OSX
	if (dp->journalDevice) {
		//if not in FS, then un-set flags
		jibp->flags = 0;

		char uuid_str[64];

		if (get_dev_uuid(dp->journalDevice, uuid_str, sizeof(uuid_str)) == 0) {
			strlcpy((char *)&jibp->reserved[0], uuid_str, sizeof(jibp->reserved));

			// we also need to blast out some zeros to the journal device
			// in case it had a file system on it previously.  that way
			// it's "initialized" in the sense that the previous contents
			// won't get mounted accidently.  if this fails we'll bail out.
			if (clear_journal_dev(dp->journalDevice) != 0) {
				return -1;
			}
		} else {
			printf("FAILED to get the device uuid for device %s\n", dp->journalDevice);
			strlcpy((char *)&jibp->reserved[0], "NO-DEV-UUID", sizeof(jibp->reserved));
			return -1;
		}
	} 
#endif // TARGET_OS_OSX

    jibp->flags  |= kJIJournalNeedInitMask;
    
	if (NEWFS_HFS_DEBUG && dp->journalBlock) {
	    journalBlock = dp->journalBlock;
	}
    else {
	    journalBlock = header->journalInfoBlock + 1;
	}

    jibp->offset  = ((UInt64) journalBlock) * header->blockSize;
    jibp->size    = dp->journalSize;

    jibp->flags  = SWAP_BE32(jibp->flags);
    jibp->offset = SWAP_BE64(jibp->offset);
    jibp->size   = SWAP_BE64(jibp->size);
    
    WriteBuffer(driveInfo, startingSector, driveInfo->physSectorSize, buffer);

    jibp->flags  = SWAP_BE32(jibp->flags);
    jibp->offset = SWAP_BE64(jibp->offset);
    jibp->size   = SWAP_BE64(jibp->size);

	if (jibp->flags & kJIJournalInFSMask) {
		/* 
		 * Zero out the on-disk content of the journal file.
		 *
		 * This is a really ugly hack.  Right now, all of the logic in the code
		 * that calls us (make_hfsplus), uses the value 'sectorsPerBlock' but it
		 * is really hardcoded to assume the sector size is 512 bytes.  The code
		 * in WriteBuffer will massage the I/O to use the actual physical sector
		 * size.   Since WriteBuffer takes a sector # relative to 512 byte sectors,
		 * We need to convert the journal offset in bytes to value that represents
		 * its start LBA in 512 byte sectors.
		 * 
		 * Note further that we swapped to big endian prior to the WriteBuffer call,
		 * but we have swapped back to native after the call.
		 */
		WriteBuffer(driveInfo, jibp->offset / kBytesPerSector, jibp->size, NULL);
	}

    return 0;
}


/*
 * WriteCatalogFile
 *	
 * This routine initializes a Catalog B-Tree.
 *
 * Note: Since large volumes can have bigger b-trees they
 * might need to have map nodes setup.
 */
static void
WriteCatalogFile(const DriveInfo *driveInfo, UInt64 startingSector,
        const hfsparams_t *dp, HFSPlusVolumeHeader *header, void *buffer,
        UInt32 *bytesUsed, UInt32 *mapNodes)
{
	BTNodeDescriptor	*ndp;
	BTHeaderRec		*bthp;
	UInt8			*bmp;
	UInt32			nodeBitsInHeader;
	UInt32			fileSize;
	UInt32			nodeSize;
	UInt32			temp;
	SInt16			offset;

	*mapNodes = 0;
	fileSize = dp->catalogInitialSize;
	nodeSize = dp->catalogNodeSize;

	bzero(buffer, nodeSize);


	/* FILL IN THE NODE DESCRIPTOR:  */
	ndp = (BTNodeDescriptor *)buffer;
	ndp->kind			= kBTHeaderNode;
	ndp->numRecords		= SWAP_BE16 (3);
	offset = sizeof(BTNodeDescriptor);

	SETOFFSET(buffer, nodeSize, offset, 1);


	/* FILL IN THE HEADER RECORD:  */
	bthp = (BTHeaderRec *)((UInt8 *)buffer + offset);
	bthp->treeDepth		= SWAP_BE16 (1);
	bthp->rootNode		= SWAP_BE32 (1);
	bthp->firstLeafNode	= SWAP_BE32 (1);
	bthp->lastLeafNode	= SWAP_BE32 (1);
	bthp->leafRecords	= SWAP_BE32 (dp->journaledHFS ? 6 : 2);
	bthp->nodeSize		= SWAP_BE16 (nodeSize);
	bthp->totalNodes	= SWAP_BE32 (fileSize / nodeSize);
	bthp->freeNodes		= SWAP_BE32 (SWAP_BE32 (bthp->totalNodes) - 2);  /* header and root */
	bthp->clumpSize		= SWAP_BE32 (dp->catalogClumpSize);


	bthp->attributes	|= SWAP_BE32 (kBTVariableIndexKeysMask + kBTBigKeysMask);
	bthp->maxKeyLength	=  SWAP_BE16 (kHFSPlusCatalogKeyMaximumLength);
	if (dp->flags & kMakeCaseSensitive)
		bthp->keyCompareType = kHFSBinaryCompare;
	else
		bthp->keyCompareType = kHFSCaseFolding;
	
	offset += sizeof(BTHeaderRec);

	SETOFFSET(buffer, nodeSize, offset, 2);

	offset += kBTreeHeaderUserBytes;

	SETOFFSET(buffer, nodeSize, offset, 3);

	/* FIGURE OUT HOW MANY MAP NODES (IF ANY):  */
	nodeBitsInHeader = 8 * (nodeSize
					- sizeof(BTNodeDescriptor)
					- sizeof(BTHeaderRec)
					- kBTreeHeaderUserBytes
					- (4 * sizeof(SInt16)) );

	if (SWAP_BE32 (bthp->totalNodes) > nodeBitsInHeader) {
		UInt32	nodeBitsInMapNode;
		
		ndp->fLink = SWAP_BE32 (SWAP_BE32 (bthp->lastLeafNode) + 1);
		nodeBitsInMapNode = 8 * (nodeSize
						- sizeof(BTNodeDescriptor)
						- (2 * sizeof(SInt16))
						- 2 );
		*mapNodes = (SWAP_BE32 (bthp->totalNodes) - nodeBitsInHeader +
			(nodeBitsInMapNode - 1)) / nodeBitsInMapNode;
		bthp->freeNodes = SWAP_BE32 (SWAP_BE32 (bthp->freeNodes) - *mapNodes);
	}

	/* 
	 * FILL IN THE MAP RECORD, MARKING NODES THAT ARE IN USE.
	 * Note - worst case (32MB alloc blk) will have only 18 nodes in use.
	 */
	bmp = ((UInt8 *)buffer + offset);
	temp = SWAP_BE32 (bthp->totalNodes) - SWAP_BE32 (bthp->freeNodes);

	/* Working a byte at a time is endian safe */
	while (temp >= 8) { *bmp = 0xFF; temp -= 8; bmp++; }
	*bmp = ~(0xFF >> temp);
	offset += nodeBitsInHeader/8;

	SETOFFSET(buffer, nodeSize, offset, 4);

	InitCatalogRoot_HFSPlus(dp, header, buffer + nodeSize);

	*bytesUsed = (SWAP_BE32 (bthp->totalNodes) - SWAP_BE32 (bthp->freeNodes) - *mapNodes) * nodeSize;

	WriteBuffer(driveInfo, startingSector, *bytesUsed, buffer);
}


static void
InitCatalogRoot_HFSPlus(const hfsparams_t *dp, const HFSPlusVolumeHeader *header, void * buffer)
{
	BTNodeDescriptor		*ndp;
	HFSPlusCatalogKey		*ckp;
	HFSPlusCatalogKey		*tkp;
	HFSPlusCatalogFolder	*cdp;
	HFSPlusCatalogFile		*cfp;
	HFSPlusCatalogThread	*ctp;
	UInt16					nodeSize;
	SInt16					offset;
	size_t					unicodeBytes;
	UInt8 canonicalName[kHFSPlusMaxFileNameBytes];	// UTF8 character may convert to three bytes, plus a NUL
	CFStringRef cfstr;
	Boolean	cfOK;
	int index = 0;

	nodeSize = dp->catalogNodeSize;
	bzero(buffer, nodeSize);

	/*
	 * All nodes have a node descriptor...
	 */
	ndp = (BTNodeDescriptor *)buffer;
	ndp->kind   = kBTLeafNode;
	ndp->height = 1;
	ndp->numRecords = SWAP_BE16 (dp->journaledHFS ? 6 : 2);
	offset = sizeof(BTNodeDescriptor);
	SETOFFSET(buffer, nodeSize, offset, ++index);

	/*
	 * First record is always the root directory...
	 */
	ckp = (HFSPlusCatalogKey *)((UInt8 *)buffer + offset);
	
	/* Use CFString functions to get a HFSPlus Canonical name */
	cfstr = CFStringCreateWithCString(kCFAllocatorDefault, (char *)dp->volumeName, kCFStringEncodingUTF8);
	cfOK = _CFStringGetFileSystemRepresentation(cfstr, canonicalName, sizeof(canonicalName));

	if (!cfOK || ConvertUTF8toUnicode(canonicalName, sizeof(ckp->nodeName.unicode),
		ckp->nodeName.unicode, &ckp->nodeName.length)) {

		/* On conversion errors "untitled" is used as a fallback. */
		(void) ConvertUTF8toUnicode((UInt8 *)kDefaultVolumeNameStr,
									sizeof(ckp->nodeName.unicode),
									ckp->nodeName.unicode,
									&ckp->nodeName.length);
		warnx("invalid HFS+ name: \"%s\", using \"%s\" instead",
		      dp->volumeName, kDefaultVolumeNameStr);
	}
	CFRelease(cfstr);
	ckp->nodeName.length = SWAP_BE16 (ckp->nodeName.length);

	unicodeBytes = sizeof(UniChar) * SWAP_BE16 (ckp->nodeName.length);

	ckp->keyLength		= SWAP_BE16 (kHFSPlusCatalogKeyMinimumLength + unicodeBytes);
	ckp->parentID		= SWAP_BE32 (kHFSRootParentID);
	offset += SWAP_BE16 (ckp->keyLength) + 2;

	cdp = (HFSPlusCatalogFolder *)((UInt8 *)buffer + offset);
	cdp->recordType		= SWAP_BE16 (kHFSPlusFolderRecord);
	/* folder count is only supported on HFSX volumes */
	if (dp->flags & kMakeCaseSensitive) {
		cdp->flags 		= SWAP_BE16 (kHFSHasFolderCountMask);
	}
	cdp->valence        = SWAP_BE32 (dp->journaledHFS ? 2 : 0);
	cdp->folderID		= SWAP_BE32 (kHFSRootFolderID);
	cdp->createDate		= SWAP_BE32 (dp->createDate);
	cdp->contentModDate	= SWAP_BE32 (dp->createDate);
	cdp->textEncoding	= SWAP_BE32 (kTextEncodingMacUnicode);
	if (dp->flags & kUseAccessPerms) {
		cdp->bsdInfo.ownerID  = SWAP_BE32 (dp->owner);
		cdp->bsdInfo.groupID  = SWAP_BE32 (dp->group);
		cdp->bsdInfo.fileMode = SWAP_BE16 (dp->mask | S_IFDIR);
	}
	offset += sizeof(HFSPlusCatalogFolder);
	SETOFFSET(buffer, nodeSize, offset, ++index);

	/*
	 * Second record is always the root directory thread...
	 */
	tkp = (HFSPlusCatalogKey *)((UInt8 *)buffer + offset);
	tkp->keyLength		= SWAP_BE16 (kHFSPlusCatalogKeyMinimumLength);
	tkp->parentID		= SWAP_BE32 (kHFSRootFolderID);
	// tkp->nodeName.length = 0;

	offset += SWAP_BE16 (tkp->keyLength) + 2;

	ctp = (HFSPlusCatalogThread *)((UInt8 *)buffer + offset);
	ctp->recordType		= SWAP_BE16 (kHFSPlusFolderThreadRecord);
	ctp->parentID		= SWAP_BE32 (kHFSRootParentID);
	bcopy(&ckp->nodeName, &ctp->nodeName, sizeof(UInt16) + unicodeBytes);
	offset += (sizeof(HFSPlusCatalogThread)
			- (sizeof(ctp->nodeName.unicode) - unicodeBytes) );

	SETOFFSET(buffer, nodeSize, offset, ++index);
	
	/*
	 * Add records for ".journal" and ".journal_info_block" files:
	 */
	if (dp->journaledHFS) {
		struct HFSUniStr255 *nodename1, *nodename2;
		size_t uBytes1, uBytes2;
		UInt32 journalBlock;

		/* File record #1 */
		ckp = (HFSPlusCatalogKey *)((UInt8 *)buffer + offset);
		(void) ConvertUTF8toUnicode((UInt8 *)HFS_JOURNAL_FILE, sizeof(ckp->nodeName.unicode),
		                            ckp->nodeName.unicode, &ckp->nodeName.length);
		ckp->nodeName.length = SWAP_BE16 (ckp->nodeName.length);
		uBytes1 = sizeof(UniChar) * SWAP_BE16 (ckp->nodeName.length);
		ckp->keyLength = SWAP_BE16 (kHFSPlusCatalogKeyMinimumLength + uBytes1);
		ckp->parentID  = SWAP_BE32 (kHFSRootFolderID);
		offset += SWAP_BE16 (ckp->keyLength) + 2;
	
		cfp = (HFSPlusCatalogFile *)((UInt8 *)buffer + offset);
		cfp->recordType     = SWAP_BE16 (kHFSPlusFileRecord);
		cfp->flags          = SWAP_BE16 (kHFSThreadExistsMask);
		cfp->fileID         = SWAP_BE32 (dp->nextFreeFileID);
		cfp->createDate     = SWAP_BE32 (dp->createDate + 1);
		cfp->contentModDate = SWAP_BE32 (dp->createDate + 1);
		cfp->textEncoding   = 0;

		cfp->bsdInfo.fileMode     = SWAP_BE16 (S_IFREG);
		cfp->bsdInfo.ownerFlags   = (uint8_t) SWAP_BE16 (((uint16_t)UF_NODUMP));
		cfp->bsdInfo.special.linkCount	= SWAP_BE32(1);
		cfp->userInfo.fdType	  = SWAP_BE32 (kJournalFileType);
		cfp->userInfo.fdCreator	  = SWAP_BE32 (kHFSPlusCreator);
		cfp->userInfo.fdFlags     = SWAP_BE16 (kIsInvisible + kNameLocked);
		cfp->dataFork.logicalSize = SWAP_BE64 (dp->journalSize);
		cfp->dataFork.totalBlocks = SWAP_BE32 ((dp->journalSize+dp->blockSize-1) / dp->blockSize);

		if (NEWFS_HFS_DEBUG && dp->journalBlock)
			journalBlock = dp->journalBlock;
		else
			journalBlock = header->journalInfoBlock + 1;
		cfp->dataFork.extents[0].startBlock = SWAP_BE32 (journalBlock);
		cfp->dataFork.extents[0].blockCount = cfp->dataFork.totalBlocks;

		offset += sizeof(HFSPlusCatalogFile);
		SETOFFSET(buffer, nodeSize, offset, ++index);
		nodename1 = &ckp->nodeName;

		/* File record #2 */
		ckp = (HFSPlusCatalogKey *)((UInt8 *)buffer + offset);
		(void) ConvertUTF8toUnicode((UInt8 *)HFS_JOURNAL_INFO, sizeof(ckp->nodeName.unicode),
		                            ckp->nodeName.unicode, &ckp->nodeName.length);
		ckp->nodeName.length = SWAP_BE16 (ckp->nodeName.length);
		uBytes2 = sizeof(UniChar) * SWAP_BE16 (ckp->nodeName.length);
		ckp->keyLength = SWAP_BE16 (kHFSPlusCatalogKeyMinimumLength + uBytes2);
		ckp->parentID  = SWAP_BE32 (kHFSRootFolderID);
		offset += SWAP_BE16 (ckp->keyLength) + 2;
	
		cfp = (HFSPlusCatalogFile *)((UInt8 *)buffer + offset);
		cfp->recordType     = SWAP_BE16 (kHFSPlusFileRecord);
		cfp->flags          = SWAP_BE16 (kHFSThreadExistsMask);
		cfp->fileID         = SWAP_BE32 (dp->nextFreeFileID + 1);
		cfp->createDate     = SWAP_BE32 (dp->createDate);
		cfp->contentModDate = SWAP_BE32 (dp->createDate);
		cfp->textEncoding   = 0;

		cfp->bsdInfo.fileMode     = SWAP_BE16 (S_IFREG);
		cfp->bsdInfo.ownerFlags   = (uint8_t) SWAP_BE16 (((uint16_t)UF_NODUMP));
		cfp->bsdInfo.special.linkCount	= SWAP_BE32(1);
		cfp->userInfo.fdType	  = SWAP_BE32 (kJournalFileType);
		cfp->userInfo.fdCreator	  = SWAP_BE32 (kHFSPlusCreator);
		cfp->userInfo.fdFlags     = SWAP_BE16 (kIsInvisible + kNameLocked);
		cfp->dataFork.logicalSize = SWAP_BE64(dp->blockSize);;
		cfp->dataFork.totalBlocks = SWAP_BE32(1);

		cfp->dataFork.extents[0].startBlock = SWAP_BE32 (header->journalInfoBlock);
		cfp->dataFork.extents[0].blockCount = cfp->dataFork.totalBlocks;

		offset += sizeof(HFSPlusCatalogFile);
		SETOFFSET(buffer, nodeSize, offset, ++index);
		nodename2 = &ckp->nodeName;

		/* Thread record for file #1 */
		tkp = (HFSPlusCatalogKey *)((UInt8 *)buffer + offset);
		tkp->keyLength = SWAP_BE16 (kHFSPlusCatalogKeyMinimumLength);
		tkp->parentID  = SWAP_BE32 (dp->nextFreeFileID);
		tkp->nodeName.length = 0;
		offset += SWAP_BE16 (tkp->keyLength) + 2;
	
		ctp = (HFSPlusCatalogThread *)((UInt8 *)buffer + offset);
		ctp->recordType = SWAP_BE16 (kHFSPlusFileThreadRecord);
		ctp->parentID   = SWAP_BE32 (kHFSRootFolderID);
		bcopy(nodename1, &ctp->nodeName, sizeof(UInt16) + uBytes1);
		offset += (sizeof(HFSPlusCatalogThread)
				- (sizeof(ctp->nodeName.unicode) - uBytes1) );
		SETOFFSET(buffer, nodeSize, offset, ++index);

		/* Thread record for file #2 */
		tkp = (HFSPlusCatalogKey *)((UInt8 *)buffer + offset);
		tkp->keyLength = SWAP_BE16 (kHFSPlusCatalogKeyMinimumLength);
		tkp->parentID  = SWAP_BE32 (dp->nextFreeFileID + 1);
		tkp->nodeName.length = 0;
		offset += SWAP_BE16 (tkp->keyLength) + 2;
	
		ctp = (HFSPlusCatalogThread *)((UInt8 *)buffer + offset);
		ctp->recordType = SWAP_BE16 (kHFSPlusFileThreadRecord);
		ctp->parentID   = SWAP_BE32 (kHFSRootFolderID);
		bcopy(nodename2, &ctp->nodeName, sizeof(UInt16) + uBytes2);
		offset += (sizeof(HFSPlusCatalogThread)
				- (sizeof(ctp->nodeName.unicode) - uBytes2) );
		SETOFFSET(buffer, nodeSize, offset, ++index);
	}
}

/*
 * WriteMapNodes
 *	
 * Initializes a B-tree map node and writes it out to disk.
 */
static void
WriteMapNodes(const DriveInfo *driveInfo, UInt64 diskStart, UInt32 firstMapNode,
	UInt32 mapNodes, UInt16 btNodeSize, void *buffer)
{
	UInt32	sectorsPerNode;
	UInt32	mapRecordBytes;
	UInt16	i;
	BTNodeDescriptor *nd = (BTNodeDescriptor *)buffer;

	bzero(buffer, btNodeSize);

	nd->kind		= kBTMapNode;
	nd->numRecords	= SWAP_BE16 (1);
	
	/* note: must belong word aligned (hence the extra -2) */
	mapRecordBytes = btNodeSize - sizeof(BTNodeDescriptor) - 2*sizeof(SInt16) - 2;  

	SETOFFSET(buffer, btNodeSize, sizeof(BTNodeDescriptor), 1);
	SETOFFSET(buffer, btNodeSize, sizeof(BTNodeDescriptor) + mapRecordBytes, 2);
	
	sectorsPerNode = btNodeSize/kBytesPerSector;
	
	/*
	 * Note - worst case (32MB alloc blk) will have
	 * only 18 map nodes. So don't bother optimizing
	 * this section to do multiblock writes!
	 */
	for (i = 0; i < mapNodes; i++) {
		if ((i + 1) < mapNodes)
			nd->fLink = SWAP_BE32 (++firstMapNode);  /* point to next map node */
		else
			nd->fLink = 0;  /* this is the last map node */

		WriteBuffer(driveInfo, diskStart, btNodeSize, buffer);
			
		diskStart += sectorsPerNode;
	}
}

/*
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 * NOTE: IF buffer IS NULL, THIS FUNCTION WILL WRITE ZERO'S.
 *
 * startingSector is in terms of 512-byte sectors.
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 */
static void
WriteBuffer(const DriveInfo *driveInfo, UInt64 startingSector, UInt64 byteCount,
	const void *buffer)
{
	off_t sector;
	off_t physSector = 0;
	off_t byteOffsetInPhysSector;
	UInt32 numBytesToIO;
	UInt32 numPhysSectorsToIO;
	UInt32 tempbufSizeInPhysSectors;
	UInt32 tempbufSize;
	UInt32 fd = driveInfo->fd;
	UInt32 physSectorSize = driveInfo->physSectorSize;
	void *tempbuf = NULL;
	int sectorSizeRatio = driveInfo->physSectorSize / kBytesPerSector;
	int status = 0; /* 0: no error; 1: alloc; 2: read; 3: write */

	if (0 == byteCount) {
		goto exit;
	}

	/*@@@@@@@@@@ buffer allocation @@@@@@@@@@*/
	/* try a buffer size for optimal IO, __UP TO 4MB__. if that
	   fails, then try with the minimum allowed buffer size, which
	   is equal to physSectorSize */
	tempbufSizeInPhysSectors = (UInt32)MIN ( (byteCount - 1 + physSectorSize) / physSectorSize,
					     driveInfo->physSectorsPerIO );
	/* limit at 4MB */
	tempbufSizeInPhysSectors = MIN ( tempbufSizeInPhysSectors, (4 * 1024 * 1024) / physSectorSize );
	tempbufSize = tempbufSizeInPhysSectors * physSectorSize;

	if ((tempbuf = valloc(tempbufSize)) == NULL) {
		/* try allocation of smallest allowed size: one
		   physical sector.
		   NOTE: the previous valloc tempbufSize might have
		   already been one physical sector. we don't want to
		   check if that was the case, so just try again.
		*/
		tempbufSizeInPhysSectors = 1;
		tempbufSize = physSectorSize;
		if ((tempbuf = valloc(tempbufSize)) == NULL) {
			status = 1;
			goto exit;
		}
	}

	/*@@@@@@@@@@ io @@@@@@@@@@*/
	sector = driveInfo->sectorOffset + startingSector;
	physSector = sector / sectorSizeRatio;
	byteOffsetInPhysSector =  (sector % sectorSizeRatio) * kBytesPerSector;

	while (byteCount > 0) {
		numPhysSectorsToIO = (UInt32)MIN ( (byteCount - 1 + physSectorSize) / physSectorSize,
				       tempbufSizeInPhysSectors );
		numBytesToIO = (UInt32)MIN(byteCount, (unsigned)((numPhysSectorsToIO * physSectorSize) - byteOffsetInPhysSector));

		/* if IO does not align with physical sector boundaries */
		if ((0 != byteOffsetInPhysSector) || ((numBytesToIO % physSectorSize) != 0)) {
			if (pread(fd, tempbuf, numPhysSectorsToIO * physSectorSize, physSector * physSectorSize) < 0) {
				status = 2;
				goto exit;
			}
		}

		if (NULL != buffer) {
			memcpy(tempbuf + byteOffsetInPhysSector, buffer, numBytesToIO);
		}
		else {
			bzero(tempbuf + byteOffsetInPhysSector, numBytesToIO);
		}

		if (pwrite(fd, tempbuf, numPhysSectorsToIO * physSectorSize, physSector * physSectorSize) < 0) {
			warn("%s:  pwrite(%d, %p, %zu, %lld)", __FUNCTION__, fd, tempbuf, (size_t)(numPhysSectorsToIO * physSectorSize), (long long)(physSector * physSectorSize));
			status = 3;
			goto exit;
		}

		byteOffsetInPhysSector = 0;
		byteCount -= numBytesToIO;
		physSector += numPhysSectorsToIO;
		if (NULL != buffer) {
			buffer += numBytesToIO;
		}
	}

exit:
	if (tempbuf) {
		free(tempbuf);
		tempbuf = NULL;
	}

	if (1 == status) {
		err(1, NULL);
	}
	else if (2 == status) {
		err(1, "read (sector %llu)", physSector);
	}
	else if (3 == status) {
		err(1, "write (sector %llu)", physSector);
	}

	return;
}


static UInt32 Largest( UInt32 a, UInt32 b, UInt32 c, UInt32 d )
{
	/* a := max(a,b) */
	if (a < b)
		a = b;
	/* c := max(c,d) */
	if (c < d)
		c = d;
	
	/* return max(a,c) */
	if (a > c)
		return a;
	else
		return c;
}

/*
 * UTCToLocal - convert from Mac OS GMT time to Mac OS local time
 */
static UInt32 UTCToLocal(UInt32 utcTime)
{
	UInt32 localTime = utcTime;
	struct timezone timeZone;
	struct timeval	timeVal;
	
	if (localTime != 0) {

                /* HFS volumes need timezone info to convert local to GMT */
                (void)gettimeofday( &timeVal, &timeZone );


		localTime -= (timeZone.tz_minuteswest * 60);
		if (timeZone.tz_dsttime)
			localTime += 3600;
	}

        return (localTime);
}

static int
ConvertUTF8toUnicode(const UInt8* source, size_t bufsize, UniChar* unibuf,
	UInt16 *charcount)
{
	UInt8 byte;
	UniChar* target;
	UniChar* targetEnd;

	*charcount = 0;
	target = unibuf;
	targetEnd = (UniChar *)((UInt8 *)unibuf + bufsize);

	while ((byte = *source++)) {
		
		/* check for single-byte ascii */
		if (byte < 128) {
			if (byte == ':')	/* ':' is mapped to '/' */
				byte = '/';

			*target++ = SWAP_BE16 (byte);
		} else {
			UniChar ch;
			UInt8 seq = (byte >> 4);

			switch (seq) {
			case 0xc: /* double-byte sequence (1100 and 1101) */
			case 0xd:
				ch = (byte & 0x1F) << 6;  /* get 5 bits */
				if (((byte = *source++) >> 6) != 2)
					return (EINVAL);
				break;

			case 0xe: /* triple-byte sequence (1110) */
				ch = (byte & 0x0F) << 6;  /* get 4 bits */
				if (((byte = *source++) >> 6) != 2)
					return (EINVAL);
				ch += (byte & 0x3F); ch <<= 6;  /* get 6 bits */
				if (((byte = *source++) >> 6) != 2)
					return (EINVAL);
				break;

			default:
				return (EINVAL);  /* malformed sequence */
			}

			ch += (byte & 0x3F);  /* get last 6 bits */

			if (target >= targetEnd)
				return (ENOBUFS);

			*target++ = SWAP_BE16 (ch);
		}
	}

	*charcount = target - unibuf;

	return (0);
}

/* Generate Volume UUID - similar to code existing in hfs_util */
void GenerateVolumeUUID(VolumeUUID *newVolumeID) {
	SHA_CTX context;
	char randomInputBuffer[26];
	unsigned char digest[20];
	time_t now;
	clock_t uptime;
	int mib[2];
	int sysdata;
	char sysctlstring[128];
	size_t datalen;
	double sysloadavg[3];
	struct vmtotal sysvmtotal;
	
	do {
		/* Initialize the SHA-1 context for processing: */
		SHA1_Init(&context);
		
		/* Now process successive bits of "random" input to seed the process: */
		
		/* The current system's uptime: */
		uptime = clock();
		SHA1_Update(&context, &uptime, sizeof(uptime));
		
		/* The kernel's boot time: */
		mib[0] = CTL_KERN;
		mib[1] = KERN_BOOTTIME;
		datalen = sizeof(sysdata);
		sysctl(mib, 2, &sysdata, &datalen, NULL, 0);
		SHA1_Update(&context, &sysdata, datalen);
		
		/* The system's host id: */
		mib[0] = CTL_KERN;
		mib[1] = KERN_HOSTID;
		datalen = sizeof(sysdata);
		sysctl(mib, 2, &sysdata, &datalen, NULL, 0);
		SHA1_Update(&context, &sysdata, datalen);

		/* The system's host name: */
		mib[0] = CTL_KERN;
		mib[1] = KERN_HOSTNAME;
		datalen = sizeof(sysctlstring);
		sysctl(mib, 2, sysctlstring, &datalen, NULL, 0);
		SHA1_Update(&context, sysctlstring, datalen);

		/* The running kernel's OS release string: */
		mib[0] = CTL_KERN;
		mib[1] = KERN_OSRELEASE;
		datalen = sizeof(sysctlstring);
		sysctl(mib, 2, sysctlstring, &datalen, NULL, 0);
		SHA1_Update(&context, sysctlstring, datalen);

		/* The running kernel's version string: */
		mib[0] = CTL_KERN;
		mib[1] = KERN_VERSION;
		datalen = sizeof(sysctlstring);
		sysctl(mib, 2, sysctlstring, &datalen, NULL, 0);
		SHA1_Update(&context, sysctlstring, datalen);

		/* The system's load average: */
		datalen = sizeof(sysloadavg);
		getloadavg(sysloadavg, 3);
		SHA1_Update(&context, &sysloadavg, datalen);

		/* The system's VM statistics: */
		mib[0] = CTL_VM;
		mib[1] = VM_METER;
		datalen = sizeof(sysvmtotal);
		sysctl(mib, 2, &sysvmtotal, &datalen, NULL, 0);
		SHA1_Update(&context, &sysvmtotal, datalen);

		/* The current GMT (26 ASCII characters): */
		time(&now);
		strncpy(randomInputBuffer, asctime(gmtime(&now)), 26);	/* "Mon Mar 27 13:46:26 2000" */
		SHA1_Update(&context, randomInputBuffer, 26);
		
		/* Pad the accumulated input and extract the final digest hash: */
		SHA1_Final(digest, &context);
	
		memcpy(newVolumeID, digest, sizeof(*newVolumeID));
	} while ((newVolumeID->v.high == 0) || (newVolumeID->v.low == 0));
}


