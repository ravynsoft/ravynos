/*
 * Copyright (c) 1999-2002, 2005, 2008 Apple Inc. All rights reserved.
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
	File:		SExtents.c

	Contains:	Routines to map file positions to volume positions, and manipulate the extents B-Tree.

	Version:	HFS Plus 1.0

	Written by:	Dave Heller, Mark Day

	Copyright:	© 1996-1999 by Apple Computer, Inc., all rights reserved.
*/


#include "BTree.h"
#include "Scavenger.h"

/*
============================================================
Public (Exported) Routines:
============================================================
	DeallocateFile		Deallocate all disk space allocated to a specified file.
					Both forks are deallocated.

	ExtendFileC		Allocate more space to a given file.


	MapFileBlockC	Convert (map) an offset within a given file into a
					physical disk address.
					
	TruncateFileC	Truncates the disk space allocated to a file.  The file
					space is truncated to a specified new physical EOF, rounded
					up to the next allocation block boundry.  There is an option
					to truncate to the end of the extent containing the new EOF.
	
	FlushExtentFile
					Flush the extents file for a given volume.

	AdjustEOF
					Copy EOF, physical length, and extent records from one FCB
					to all other FCBs for that fork.  This is used when a file is
					grown or shrunk as the result of a Write, SetEOF, or Allocate.

	MapLogicalToPhysical
					Map some position in a file to a volume block number.  Also
					returns the number of contiguous bytes that are mapped there.
					This is a queued HFSDispatch call that does the equivalent of
					MapFileBlockC, using a parameter block.

	UpdateExtentRecord
					If the extent record came from the extents file, write out
					the updated record; otherwise, copy the updated record into
					the FCB resident extent record.  If the record has no extents,
					and was in the extents file, then delete the record instead.

	ReleaseExtents
					Deallocate all allocation blocks in all extents of an extent
					data record.
============================================================
Internal Routines:
============================================================
	FindExtentRecord
					Search the extents BTree for a particular extent record.
	SearchExtentFile
					Search the FCB and extents file for an extent record that
					contains a given file position (in bytes).
	SearchExtentRecord
					Search a given extent record to see if it contains a given
					file position (in bytes).  Used by SearchExtentFile.
	TruncateExtents
					Deallocate blocks and delete extent records for all allocation
					blocks beyond a certain point in a file.  The starting point
					must be the first file allocation block for some extent record
					for the file.
	DeallocateFork
					Deallocate all allocation blocks belonging to a given fork.
*/

enum
{
	kTwoGigSectors			= 0x00400000,
	
	kDataForkType			= 0,
	kResourceForkType		= 0xFF,
	
	kPreviousRecord			= -1,
	
	kSectorSize				= 512		// Size of a physical sector
};

static OSErr ExtentsToExtDataRec(
	HFSPlusExtentRecord	oldExtents,
	HFSExtentRecord		newExtents);

OSErr FindExtentRecord(
	const SVCB		*vcb,
	UInt8					forkType,
	UInt32					fileID,
	UInt32					startBlock,
	Boolean					allowPrevious,
	HFSPlusExtentKey			*foundKey,
	HFSPlusExtentRecord		foundData,
	UInt32					*foundHint);

OSErr DeleteExtentRecord(
	const SVCB		*vcb,
	UInt8					forkType,
	UInt32					fileID,
	UInt32					startBlock);

static OSErr CreateExtentRecord(
	const SVCB		*vcb,
	HFSPlusExtentKey			*key,
	HFSPlusExtentRecord		extents,
	UInt32					*hint);

OSErr GetFCBExtentRecord(
	const SVCB		*vcb,
	const SFCB			*fcb,
	HFSPlusExtentRecord		extents);

static OSErr SetFCBExtentRecord(
	const SVCB			*vcb,
	SFCB				*fcb,
	HFSPlusExtentRecord		extents);

static OSErr SearchExtentFile(
	const SVCB		*vcb,
	const SFCB 			*fcb,
	UInt64 					filePosition,
	HFSPlusExtentKey			*foundExtentKey,
	HFSPlusExtentRecord		foundExtentData,
	UInt32					*foundExtentDataIndex,
	UInt32					*extentBTreeHint,
	UInt32					*endingFABNPlusOne );

static OSErr SearchExtentRecord(
	const SVCB		*vcb,
	UInt32					searchFABN,
	const HFSPlusExtentRecord	extentData,
	UInt32					extentDataStartFABN,
	UInt32					*foundExtentDataOffset,
	UInt32					*endingFABNPlusOne,
	Boolean					*noMoreExtents);

#if 0
static OSErr DeallocateFork(
	SVCB 		*vcb,
	HFSCatalogNodeID		fileID,
	UInt8				forkType,
	HFSPlusExtentRecord	catalogExtents,
	Boolean *			recordDeleted);

static OSErr TruncateExtents(
	SVCB			*vcb,
	UInt8				forkType,
	UInt32				fileID,
	UInt32				startBlock,
	Boolean *			recordDeleted);
#endif

static OSErr MapFileBlockFromFCB(
	const SVCB		*vcb,
	const SFCB			*fcb,
	UInt64					offset,			// Desired offset in bytes from start of file
	UInt32					*firstFABN,		// FABN of first block of found extent
	UInt32					*firstBlock,	// Corresponding allocation block number
	UInt32					*nextFABN);		// FABN of block after end of extent

static Boolean ExtentsAreIntegral(
	const HFSPlusExtentRecord extentRecord,
	UInt32		mask,
	UInt32		*blocksChecked,
	Boolean		*checkedLastExtent);

//_________________________________________________________________________________
//
//	Routine:	FindExtentRecord
//
//	Purpose:	Search the extents BTree for an extent record matching the given
//				FileID, fork, and starting file allocation block number.
//
//	Inputs:
//		vcb				Volume to search
//		forkType		0 = data fork, -1 = resource fork
//		fileID			File's FileID (HFSCatalogNodeID)
//		startBlock		Starting file allocation block number
//		allowPrevious	If the desired record isn't found and this flag is set,
//						then see if the previous record belongs to the same fork.
//						If so, then return it.
//
//	Outputs:
//		foundKey	The key data for the record actually found
//		foundData	The extent record actually found (NOTE: on an HFS volume, the
//					fourth entry will be zeroes.
//		foundHint	The BTree hint to find the node again
//_________________________________________________________________________________
OSErr FindExtentRecord(
	const SVCB	*vcb,
	UInt8				forkType,
	UInt32				fileID,
	UInt32				startBlock,
	Boolean				allowPrevious,
	HFSPlusExtentKey		*foundKey,
	HFSPlusExtentRecord	foundData,
	UInt32				*foundHint)
{
	OSErr				err;
	UInt16				foundSize;
	
	err = noErr;
	
	if (vcb->vcbSignature == kHFSSigWord) {
		HFSExtentKey		key;
		HFSExtentKey		extentKey;
		HFSExtentRecord	extentData;

		key.keyLength	= kHFSExtentKeyMaximumLength;
		key.forkType	= forkType;
		key.fileID		= fileID;
		key.startBlock	= startBlock;
		
		err = SearchBTreeRecord(vcb->vcbExtentsFile, &key, kNoHint, &extentKey, &extentData,
								&foundSize, foundHint);

		if (err == btNotFound && allowPrevious) {
			err = GetBTreeRecord(vcb->vcbExtentsFile, kPreviousRecord, &extentKey, &extentData,
								 &foundSize, foundHint);

			//	A previous record may not exist, so just return btNotFound (like we would if
			//	it was for the wrong file/fork).
			if (err == (OSErr) fsBTStartOfIterationErr)		//¥¥ fsBTStartOfIterationErr is type unsigned long
				err = btNotFound;

			if (err == noErr) {
				//	Found a previous record.  Does it belong to the same fork of the same file?
				if (extentKey.fileID != fileID || extentKey.forkType != forkType)
					err = btNotFound;
			}
		}

		if (err == noErr) {
			UInt16	i;
			
			//	Copy the found key back for the caller
			foundKey->keyLength 	= kHFSPlusExtentKeyMaximumLength;
			foundKey->forkType		= extentKey.forkType;
			foundKey->pad			= 0;
			foundKey->fileID		= extentKey.fileID;
			foundKey->startBlock	= extentKey.startBlock;
			
			//	Copy the found data back for the caller
			foundData[0].startBlock = extentData[0].startBlock;
			foundData[0].blockCount = extentData[0].blockCount;
			foundData[1].startBlock = extentData[1].startBlock;
			foundData[1].blockCount = extentData[1].blockCount;
			foundData[2].startBlock = extentData[2].startBlock;
			foundData[2].blockCount = extentData[2].blockCount;
			
			for (i = 3; i < kHFSPlusExtentDensity; ++i)
			{
				foundData[i].startBlock = 0;
				foundData[i].blockCount = 0;
			}
		}
	}
	else {		// HFS Plus volume
		HFSPlusExtentKey		key;
		HFSPlusExtentKey		extentKey;
		HFSPlusExtentRecord	extentData;

		key.keyLength	= kHFSPlusExtentKeyMaximumLength;
		key.forkType	= forkType;
		key.pad			= 0;
		key.fileID		= fileID;
		key.startBlock	= startBlock;
		
		err = SearchBTreeRecord(vcb->vcbExtentsFile, &key, kNoHint, &extentKey, &extentData,
								&foundSize, foundHint);

		if (err == btNotFound && allowPrevious) {
			err = GetBTreeRecord(vcb->vcbExtentsFile, kPreviousRecord, &extentKey, &extentData,
								 &foundSize, foundHint);

			//	A previous record may not exist, so just return btNotFound (like we would if
			//	it was for the wrong file/fork).
			if (err == (OSErr) fsBTStartOfIterationErr)		//¥¥ fsBTStartOfIterationErr is type unsigned long
				err = btNotFound;

			if (err == noErr) {
				//	Found a previous record.  Does it belong to the same fork of the same file?
				if (extentKey.fileID != fileID || extentKey.forkType != forkType)
					err = btNotFound;
			}
		}

		if (err == noErr) {
			//	Copy the found key back for the caller
			CopyMemory(&extentKey, foundKey, sizeof(HFSPlusExtentKey));
			//	Copy the found data back for the caller
			CopyMemory(&extentData, foundData, sizeof(HFSPlusExtentRecord));
		}
	}
	
	return err;
}



static OSErr CreateExtentRecord(
	const SVCB	*vcb,
	HFSPlusExtentKey		*key,
	HFSPlusExtentRecord	extents,
	UInt32				*hint)
{
	OSErr				err;
	
	err = noErr;
	
	if (vcb->vcbSignature == kHFSSigWord) {
		HFSExtentKey		hfsKey;
		HFSExtentRecord	data;
		
		hfsKey.keyLength  = kHFSExtentKeyMaximumLength;
		hfsKey.forkType	  = key->forkType;
		hfsKey.fileID	  = key->fileID;
		hfsKey.startBlock = key->startBlock;
		
		err = ExtentsToExtDataRec(extents, data);
		if (err == noErr)
			err = InsertBTreeRecord(vcb->vcbExtentsFile, &hfsKey, data, sizeof(HFSExtentRecord), hint);
	}
	else {		// HFS Plus volume
		err = InsertBTreeRecord(vcb->vcbExtentsFile, key, extents, sizeof(HFSPlusExtentRecord), hint);
	}
	
	return err;
}


OSErr DeleteExtentRecord(
	const SVCB	*vcb,
	UInt8				forkType,
	UInt32				fileID,
	UInt32				startBlock)
{
	OSErr				err;
	
	err = noErr;
	
	if (vcb->vcbSignature == kHFSSigWord) {
		HFSExtentKey	key;

		key.keyLength	= kHFSExtentKeyMaximumLength;
		key.forkType	= forkType;
		key.fileID		= fileID;
		key.startBlock	= startBlock;
		
		err = DeleteBTreeRecord( vcb->vcbExtentsFile, &key );
	}
	else {		//	HFS Plus volume
		HFSPlusExtentKey	key;

		key.keyLength	= kHFSPlusExtentKeyMaximumLength;
		key.forkType	= forkType;
		key.pad			= 0;
		key.fileID		= fileID;
		key.startBlock	= startBlock;
		
		err = DeleteBTreeRecord( vcb->vcbExtentsFile, &key );
	}
	
	return err;
}



//_________________________________________________________________________________
//
// Routine:		MapFileBlock
//
// Function: 	Maps a file position into a physical disk address.
//
// Input:		A2.L  -  VCB pointer
//				(A1,D1.W)  -  FCB pointer
//				D4.L  -  number of bytes desired
//				D5.L  -  file position (byte address)
//
// Output:		D3.L  -  physical start block
//				D6.L  -  number of contiguous bytes available (up to D4 bytes)
//				D0.L  -  result code												<01Oct85>
//						   0 = ok
//						   FXRangeErr = file position beyond mapped range			<17Oct85>
//						   FXOvFlErr = extents file overflow						<17Oct85>
//						   other = error											<17Oct85>
//
// Called By:	Log2Phys (read/write in place), Cache (map a file block).
//_________________________________________________________________________________

OSErr MapFileBlockC (
	SVCB		*vcb,				// volume that file resides on
	SFCB			*fcb,				// FCB of file
	UInt32			numberOfBytes,		// number of contiguous bytes desired
	UInt64			sectorOffset,		// starting offset within file (in 512-byte sectors)
	UInt64			*startSector,		// first 512-byte volume sector (NOT an allocation block)
	UInt32			*availableBytes)	// number of contiguous bytes (up to numberOfBytes)
{
	OSErr				err;
	UInt32				allocBlockSize;			//	Size of the volume's allocation block, in sectors
	HFSPlusExtentKey	foundKey;
	HFSPlusExtentRecord	foundData;
	UInt32				foundIndex;
	UInt32				hint;
	UInt32				firstFABN = 0;				// file allocation block of first block in found extent
	UInt32				nextFABN;				// file allocation block of block after end of found extent
	UInt64				dataEnd;				// (offset) end of range that is contiguous (in sectors)
	UInt32				startBlock = 0;			// volume allocation block corresponding to firstFABN
	UInt64				temp;
	
	
//	LogStartTime(kTraceMapFileBlock);

	allocBlockSize = vcb->vcbBlockSize >> kSectorShift;
	
	err = MapFileBlockFromFCB(vcb, fcb, sectorOffset, &firstFABN, &startBlock, &nextFABN);
	if (err != noErr) {
		err = SearchExtentFile(vcb, fcb, sectorOffset, &foundKey, foundData, &foundIndex, &hint, &nextFABN);
		if (err == noErr) {
			startBlock = foundData[foundIndex].startBlock;
			firstFABN = nextFABN - foundData[foundIndex].blockCount;
		}
	}
	
	if (err != noErr)
	{
	//	LogEndTime(kTraceMapFileBlock, err);

		return err;
	}

	//
	//	Determine the end of the available space.  It will either be the end of the extent,
	//	or the file's PEOF, whichever is smaller.
	//
	
	// Get fork's physical size, in sectors
	temp = fcb->fcbPhysicalSize >> kSectorShift;
	dataEnd = (UInt64) nextFABN * allocBlockSize;		// Assume valid data through end of this extent
	if (temp < dataEnd)							// Is PEOF shorter?
		dataEnd = temp;							// Yes, so only map up to PEOF
	
	//
	//	Compute the absolute sector number that contains the offset of the given file
	//
	temp  = sectorOffset - ((UInt64) firstFABN * allocBlockSize);	// offset in sectors from start of this extent
	temp += (UInt64)startBlock * (UInt64)allocBlockSize;	// offset in sectors from start of allocation block space
	if (vcb->vcbSignature == kHFSPlusSigWord)
		temp += vcb->vcbEmbeddedOffset/512;		// offset into the wrapper
	else
		temp += vcb->vcbAlBlSt;						// offset in sectors from start of volume
	
	//	Return the desired sector for file position "offset"
	*startSector = temp;
	
	//
	//	Determine the number of contiguous sectors until the end of the extent
	//	(or the amount they asked for, whichever comes first).  In any case,
	//	we never map more than 2GB per call.
	//
	temp = dataEnd - sectorOffset;
	if (temp >= kTwoGigSectors)
		temp = kTwoGigSectors-1;				// never map more than 2GB per call
	temp <<= kSectorShift;						// convert sectors to bytes
	if (temp > numberOfBytes)
		*availableBytes = numberOfBytes;		// more there than they asked for, so pin the output
	else
		*availableBytes = (UInt32)temp;
	
//	LogEndTime(kTraceMapFileBlock, noErr);

	return noErr;
}


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Routine:	ReleaseExtents
//
//	Function: 	Release the extents of a single extent data record.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

#if 1
OSErr ReleaseExtents(
	SVCB 			*vcb,
	const HFSPlusExtentRecord	extentRecord,
	UInt32					*numReleasedAllocationBlocks,
	Boolean 				*releasedLastExtent)
{
	UInt32	extentIndex;
	UInt32	numberOfExtents;
	OSErr	err = noErr;
	
	*numReleasedAllocationBlocks = 0;
	*releasedLastExtent = false;
	
	if (vcb->vcbSignature == kHFSPlusSigWord)
		numberOfExtents = kHFSPlusExtentDensity;
	else
		numberOfExtents = kHFSExtentDensity;
	
	for( extentIndex = 0; extentIndex < numberOfExtents; extentIndex++)
	{
		UInt32	numAllocationBlocks;
		
		// Loop over the extent record and release the blocks associated with each extent.
		
		numAllocationBlocks = extentRecord[extentIndex].blockCount;
		if ( numAllocationBlocks == 0 )
		{
			*releasedLastExtent = true;
			break;
		}

		err = ReleaseBitmapBits( extentRecord[extentIndex].startBlock, numAllocationBlocks );
		if ( err != noErr )
			break;
					
		*numReleasedAllocationBlocks += numAllocationBlocks;		//	bump FABN to beg of next extent
	}

	return( err );
}
#endif


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Routine:	TruncateExtents
//
//	Purpose:	Delete extent records whose starting file allocation block number
//				is greater than or equal to a given starting block number.  The
//				allocation blocks represented by the extents are deallocated.
//
//	Inputs:
//		vcb			Volume to operate on
//		fileID		Which file to operate on
//		startBlock	Starting file allocation block number for first extent
//					record to delete.
//
//	Outputs:
//		recordDeleted	Set to true if any extents B-tree record was deleted.
//						Unchanged otherwise.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

static OSErr TruncateExtents(
	SVCB		*vcb,
	UInt8			forkType,
	UInt32			fileID,
	UInt32			startBlock,
	Boolean *		recordDeleted)
{
	OSErr				err;
	Boolean				releasedLastExtent;
	UInt32				numberExtentsReleased;
	UInt32				hint;
	HFSPlusExtentKey		key;
	HFSPlusExtentRecord	extents;
	
	while (true) {
		err = FindExtentRecord(vcb, forkType, fileID, startBlock, false, &key, extents, &hint);
		if (err != noErr) {
			if (err == btNotFound)
				err = noErr;
			break;
		}
		
		err = ReleaseExtents( vcb, extents, &numberExtentsReleased, &releasedLastExtent );
		if (err != noErr) break;
		
		err = DeleteExtentRecord(vcb, forkType, fileID, startBlock);
		if (err != noErr) break;

		*recordDeleted = true;	//	We did delete a record
		startBlock += numberExtentsReleased;
	}
	
	return err;
}


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Routine:	DeallocateFork
//
//	Function: 	De-allocates all disk space allocated to a specified fork.
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
static OSErr DeallocateFork(
	SVCB 		*vcb,
	HFSCatalogNodeID		fileID,
	UInt8				forkType,
	HFSPlusExtentRecord	catalogExtents,
	Boolean *			recordDeleted)		//	set to true if any record was deleted
{
	OSErr				err;
	UInt32				numReleasedAllocationBlocks;
	Boolean				releasedLastExtent;
	
	//	Release the catalog extents
	err = ReleaseExtents( vcb, catalogExtents, &numReleasedAllocationBlocks, &releasedLastExtent );
	
	// Release the extra extents, if present
	if (err == noErr && !releasedLastExtent)
		err = TruncateExtents(vcb, forkType, fileID, numReleasedAllocationBlocks, recordDeleted);

	return( err );
}


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Routine:	FlushExtentFile
//
//	Function: 	Flushes the extent file for a specified volume
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

OSErr FlushExtentFile( SVCB *vcb )
{
	OSErr	err;

	err = BTFlushPath(vcb->vcbExtentsFile);
	if ( err == noErr )
	{
		// If the FCB for the extent "file" is dirty, mark the VCB as dirty.
		
		if( ( vcb->vcbExtentsFile->fcbFlags & fcbModifiedMask ) != 0 )
		{
			(void) MarkVCBDirty( vcb );
			err = FlushVolumeControlBlock( vcb );
		}
	}
	
	return( err );
}


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Routine:	DeallocateFile
//
//	Function: 	De-allocates all disk space allocated to a specified file.
//				The space occupied by both forks is deallocated.
//
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

OSErr DeallocateFile(SVCB *vcb, CatalogRecord * fileRec)
{
	int i;
	OSErr errDF, errRF;
	Boolean recordDeleted = false;

	errDF = errRF = 0;

	if (fileRec->recordType == kHFSFileRecord) {
		HFSPlusExtentRecord dataForkExtents;
		HFSPlusExtentRecord rsrcForkExtents;

		ClearMemory(&dataForkExtents, sizeof(dataForkExtents));
		ClearMemory(&rsrcForkExtents, sizeof(rsrcForkExtents));

		for (i = 0; i < kHFSExtentDensity; ++i) {
			dataForkExtents[i].startBlock =
				(UInt32) (fileRec->hfsFile.dataExtents[i].startBlock);
			dataForkExtents[i].blockCount =
				(UInt32) (fileRec->hfsFile.dataExtents[i].blockCount);

			rsrcForkExtents[i].startBlock =
				(UInt32) (fileRec->hfsFile.rsrcExtents[i].startBlock);
			rsrcForkExtents[i].blockCount =
				(UInt32) (fileRec->hfsFile.rsrcExtents[i].blockCount);
		}
		
		errDF = DeallocateFork(vcb, fileRec->hfsFile.fileID, kDataForkType,
				dataForkExtents, &recordDeleted );

		errRF = DeallocateFork(vcb, fileRec->hfsFile.fileID, kResourceForkType,
			rsrcForkExtents, &recordDeleted );
	}
	else if (fileRec->recordType == kHFSPlusFileRecord) {
		errDF = DeallocateFork(vcb, fileRec->hfsPlusFile.fileID, kDataForkType,
			fileRec->hfsPlusFile.dataFork.extents, &recordDeleted );

		errRF = DeallocateFork(vcb, fileRec->hfsPlusFile.fileID, kResourceForkType,
			fileRec->hfsPlusFile.resourceFork.extents, &recordDeleted );
	}

	if (recordDeleted)
		(void) FlushExtentFile(vcb);
	
	MarkVCBDirty(vcb);
		
	return (errDF ? errDF : errRF);
}


//_________________________________________________________________________________
//
// Routine:		Extendfile
//
// Function: 	Extends the disk space allocated to a file.
//
// Input:		A2.L  -  VCB pointer
//				A1.L  -  pointer to FCB array
//				D1.W  -  file refnum
//				D3.B  -  option flags
//							kEFContigMask - force contiguous allocation
//							kEFAllMask - allocate all requested bytes or none
//							NOTE: You may not set both options.
//				D4.L  -  number of additional bytes to allocate
//
// Output:		D0.W  -  result code
//							 0 = ok
//							 -n = IO error
//				D6.L  -  number of bytes allocated
//
// Called by:	FileAloc,FileWrite,SetEof
//
// Note: 		ExtendFile updates the PEOF in the FCB.
//_________________________________________________________________________________

OSErr ExtendFileC (
	SVCB		*vcb,				// volume that file resides on
	SFCB			*fcb,				// FCB of file to truncate
	UInt32			sectorsToAdd,		// number of sectors to allocate
	UInt32			flags,				// EFContig and/or EFAll
	UInt32			*actualSectorsAdded)// number of bytes actually allocated
{
	OSErr				err;
	Boolean				wantContig;
	Boolean				needsFlush;
	UInt32				sectorsPerBlock;
	UInt32				blocksToAdd;	//	number of blocks we'd like to add
	UInt32				blocksPerClump;	//	number of blocks in clump size
	UInt32				maxBlocksToAdd;	//	max blocks we want to add
	UInt32				eofBlocks;		//	current EOF in blocks
	HFSPlusExtentKey	foundKey;		//	from SearchExtentFile
	HFSPlusExtentRecord	foundData;
	UInt32				foundIndex;		//	from SearchExtentFile
	UInt32				hint;			//	from SearchExtentFile
	UInt32				nextBlock;		//	from SearchExtentFile
	UInt32				startBlock;
	UInt32				actualStartBlock;
	UInt32				actualNumBlocks;
	UInt32				numExtentsPerRecord;
	UInt32				blocksAdded;

	needsFlush = false;		//	Assume the B-tree header doesn't need to be updated
	blocksAdded = 0;
	*actualSectorsAdded = 0;

	if (vcb->vcbSignature == kHFSPlusSigWord)
		numExtentsPerRecord = kHFSPlusExtentDensity;
	else
		numExtentsPerRecord = kHFSExtentDensity;

	//
	//	Round up the request to whole allocation blocks
	//
	sectorsPerBlock = vcb->vcbBlockSize >> kSectorShift;
	blocksToAdd = DivideAndRoundUp(sectorsToAdd, sectorsPerBlock);

	//
	//	Determine the physical EOF in allocation blocks
	//
	eofBlocks = (UInt32)(fcb->fcbPhysicalSize / vcb->vcbBlockSize);

	//
	//	Make sure the request won't make the file too big (>=2GB).
	//	[2350148] Always limit HFS files.
	//	¥¥	Shouldn't really fail if allOrNothing is false
	//	¥¥	Adjust for clump size here?
	//
	if ( vcb->vcbSignature == kHFSPlusSigWord )
	{
		//	Allow it to grow beyond 2GB.
	}
	else
	{
		UInt32 maxFileBlocks;	//	max legal EOF, in blocks
		maxFileBlocks = (kTwoGigSectors-1) / sectorsPerBlock;
		if (blocksToAdd > maxFileBlocks || (blocksToAdd + eofBlocks) > maxFileBlocks) {
			err = fileBoundsErr;
			goto ErrorExit;
		}
	}

	//
	//	If allocation is all-or-nothing, then make sure there
	//	are enough free blocks.  (A quick test)
	//
	if ((flags & kEFAllMask) && blocksToAdd > vcb->vcbFreeBlocks) {
		err = dskFulErr;
		goto ErrorExit;
	}
	
	//
	//	There may be blocks allocated beyond the physical EOF
	//	(because we allocated the rest of the clump size, or
	//	because of a PBAllocate or PBAllocContig call).
	//	If these extra blocks exist, then use them to satisfy
	//	part or all of the request.
	//
	//	¥¥	What, if anything, would break if the physical EOF always
	//	¥¥	represented ALL extents allocated to the file (including
	//	¥¥	the clump size roundup)?
	//
	//	Note: (blocks * sectorsPerBlock - 1) is the sector offset
	//	of the last sector in the last block.
	//
	err = SearchExtentFile(vcb, fcb, (eofBlocks+blocksToAdd) * sectorsPerBlock - 1, &foundKey, foundData, &foundIndex, &hint, &nextBlock);
	if (err == noErr) {
		//	Enough blocks are already allocated.  Just update the FCB to reflect the new length.
		eofBlocks += blocksToAdd;		//	new EOF, in blocks
		blocksAdded += blocksToAdd;
		goto Exit;
	}
	if (err != fxRangeErr)		// Any real error?
		goto ErrorExit;				// Yes, so exit immediately

	//
	//	There wasn't enough already allocated.  But there might have been
	//	a few allocated blocks beyond the physical EOF.  So, set the physical
	//	EOF to match the end of the last extent.
	//
	if (nextBlock > eofBlocks) {
		//	There were (nextBlock - eofBlocks) extra blocks past physical EOF
		blocksAdded += nextBlock - eofBlocks;
		blocksToAdd -= nextBlock - eofBlocks;
		eofBlocks = nextBlock;
	}
	
	//
	//	We still need to allocate more blocks.
	//
	//	First try a contiguous allocation (of the whole amount).
	//	If that fails, get whatever we can.
	//		If forceContig, then take whatever we got
	//		else, keep getting bits and pieces (non-contig)
	//
	//	¥¥	Need to do clump size calculations
	//
	blocksPerClump = fcb->fcbClumpSize / vcb->vcbBlockSize;
	if (blocksPerClump == 0)
		blocksPerClump = 1;

	err = noErr;
	wantContig = true;
	do {
		//	Make maxBlocksToAdd equal to blocksToAdd rounded up to a multiple
		//	of the file's clump size.  This gives the file room to grow some
		//	more without fragmenting.
		if (flags & kEFNoClumpMask) {
			//	Caller said not to round up, so only allocate what was asked for.
			maxBlocksToAdd = blocksToAdd;
		}
		else {
			//	Round up to multiple of clump size
			maxBlocksToAdd = DivideAndRoundUp(blocksToAdd, blocksPerClump);
			maxBlocksToAdd *= blocksPerClump;
		}
		
		//	Try to allocate the new space contiguous with the end of the previous
		//	extent.  If this succeeds, the last extent grows and the file does not
		//	become any more fragmented.
		startBlock = foundData[foundIndex].startBlock + foundData[foundIndex].blockCount;
		err = BlockAllocate(vcb, startBlock, blocksToAdd, maxBlocksToAdd, wantContig, &actualStartBlock, &actualNumBlocks);
		if (err == dskFulErr) {
			if (flags & kEFContigMask)
				break;			// AllocContig failed because not enough contiguous space
			if (wantContig) {
				//	Couldn't get one big chunk, so get whatever we can.
				err = noErr;
				wantContig = false;
				continue;
			}
			if (actualNumBlocks != 0)
				err = noErr;
		}
		if (err == noErr) {
			//	Add the new extent to the existing extent record, or create a new one.
			if (actualStartBlock == startBlock) {
				//	We grew the file's last extent, so just adjust the number of blocks.
				foundData[foundIndex].blockCount += actualNumBlocks;
				err = UpdateExtentRecord(vcb, fcb, &foundKey, foundData, hint);
				if (err != noErr) break;
			}
			else {
				UInt16	i;

				//	Need to add a new extent.  See if there is room in the current record.
				if (foundData[foundIndex].blockCount != 0)	//	Is current extent free to use?
					++foundIndex;							// 	No, so use the next one.
				if (foundIndex == numExtentsPerRecord) {
					//	This record is full.  Need to create a new one.
					if (fcb->fcbFileID == kHFSExtentsFileID || (flags & kEFNoExtOvflwMask)) {
						(void) BlockDeallocate(vcb, actualStartBlock, actualNumBlocks);
						err = fxOvFlErr;		// Oops.  Can't extend extents file past first record.
						break;
					}
					
					foundKey.keyLength = kHFSPlusExtentKeyMaximumLength;
					if (fcb->fcbFlags & fcbResourceMask)
						foundKey.forkType = kResourceForkType;
					else
						foundKey.forkType = kDataForkType;
					foundKey.pad = 0;
					foundKey.fileID = fcb->fcbFileID;
					foundKey.startBlock = nextBlock;
					
					foundData[0].startBlock = actualStartBlock;
					foundData[0].blockCount = actualNumBlocks;
					
					// zero out remaining extents...
					for (i = 1; i < kHFSPlusExtentDensity; ++i)
					{
						foundData[i].startBlock = 0;
						foundData[i].blockCount = 0;
					}

					foundIndex = 0;
					
					err = CreateExtentRecord(vcb, &foundKey, foundData, &hint);
					if (err == fxOvFlErr) {
						//	We couldn't create an extent record because extents B-tree
						//	couldn't grow.  Dellocate the extent just allocated and
						//	return a disk full error.
						(void) BlockDeallocate(vcb, actualStartBlock, actualNumBlocks);
						err = dskFulErr;
					}
					if (err != noErr) break;

					needsFlush = true;		//	We need to update the B-tree header
				}
				else {
					//	Add a new extent into this record and update.
					foundData[foundIndex].startBlock = actualStartBlock;
					foundData[foundIndex].blockCount = actualNumBlocks;
					err = UpdateExtentRecord(vcb, fcb, &foundKey, foundData, hint);
					if (err != noErr) break;
				}
			}
			
			// Figure out how many bytes were actually allocated.
			// NOTE: BlockAllocate could have allocated more than the minimum
			// we asked for (up to our requested maximum).
			// Don't set the PEOF beyond what our client asked for.
			nextBlock += actualNumBlocks;
			if (actualNumBlocks > blocksToAdd) {
				blocksAdded += blocksToAdd;
				eofBlocks += blocksToAdd;
				blocksToAdd = 0;
			}
			else {
				blocksAdded += actualNumBlocks;
				blocksToAdd -= actualNumBlocks;
				eofBlocks += actualNumBlocks;
			}

			//	If contiguous allocation was requested, then we've already got one contiguous
			//	chunk.  If we didn't get all we wanted, then adjust the error to disk full.
			if (flags & kEFContigMask) {
				if (blocksToAdd != 0)
					err = dskFulErr;
				break;			//	We've already got everything that's contiguous
			}
		}
	} while (err == noErr && blocksToAdd);

ErrorExit:
Exit:
	*actualSectorsAdded = blocksAdded * sectorsPerBlock;
	if (blocksAdded) {
		fcb->fcbPhysicalSize = (UInt64)eofBlocks * (UInt64)vcb->vcbBlockSize;
		fcb->fcbFlags |= fcbModifiedMask;
	}

	// [2355121] If we created a new extent record, then update the B-tree header
	if (needsFlush)
		(void) FlushExtentFile(vcb);

	return err;
}



//_________________________________________________________________________________
//
// Routine:		TruncateFileC
//
// Function: 	Truncates the disk space allocated to a file.  The file space is
//				truncated to a specified new PEOF rounded up to the next allocation
//				block boundry.  If the 'TFTrunExt' option is specified, the file is
//				truncated to the end of the extent containing the new PEOF.
//
// Input:		A2.L  -  VCB pointer
//				A1.L  -  pointer to FCB array
//				D1.W  -  file refnum
//				D2.B  -  option flags
//						   TFTrunExt - truncate to the extent containing new PEOF
//				D3.L  -  new PEOF
//
// Output:		D0.W  -  result code
//							 0 = ok
//							 -n = IO error
//
// Note: 		TruncateFile updates the PEOF in the FCB.
//_________________________________________________________________________________

#if 0
OSErr TruncateFileC (
	SVCB		*vcb,				// volume that file resides on
	SFCB			*fcb,				// FCB of file to truncate
	UInt32			eofSectors,			// new physical size for file
	Boolean			truncateToExtent)	// if true, truncate to end of extent containing newPEOF
{
	OSErr				err;
	UInt32				nextBlock;		//	next file allocation block to consider
	UInt32				startBlock;		//	Physical (volume) allocation block number of start of a range
	UInt32				physNumBlocks;	//	Number of allocation blocks in file (according to PEOF)
	UInt32				numBlocks;
	HFSPlusExtentKey		key;			//	key for current extent record; key->keyLength == 0 if FCB's extent record
	UInt32				hint;			//	BTree hint corresponding to key
	HFSPlusExtentRecord	extentRecord;
	UInt32				extentIndex;
	UInt32				extentNextBlock;
	UInt32				numExtentsPerRecord;
	UInt32				sectorsPerBlock;
	UInt8				forkType;
	Boolean				extentChanged;	// true if we actually changed an extent
	Boolean				recordDeleted;	// true if an extent record got deleted

	recordDeleted = false;
	sectorsPerBlock = vcb->vcbBlockSize >> kSectorShift;
	
	if (vcb->vcbSignature == kHFSPlusSigWord)
		numExtentsPerRecord = kHFSPlusExtentDensity;
	else
		numExtentsPerRecord = kHFSExtentDensity;

	if (fcb->fcbFlags & fcbResourceMask)
		forkType = kResourceForkType;
	else
		forkType = kDataForkType;

	//	Compute number of allocation blocks currently in file
	physNumBlocks = fcb->fcbPhysicalSize / vcb->vcbBlockSize;
	
	//
	//	Round newPEOF up to a multiple of the allocation block size.  If new size is
	//	two gigabytes or more, then round down by one allocation block (??? really?
	//	shouldn't that be an error?).
	//
	nextBlock = DivideAndRoundUp(eofSectors, sectorsPerBlock);	// number of allocation blocks to remain in file
	eofSectors = nextBlock * sectorsPerBlock;					// rounded up to multiple of block size
	if ((fcb->fcbFlags & fcbLargeFileMask) == 0 && eofSectors >= kTwoGigSectors) {
		#if DEBUG_BUILD
			DebugStr("\pHFS: Trying to truncate a file to 2GB or more");
		#endif
		err = fileBoundsErr;
		goto ErrorExit;
	}
	
	//
	//	Update FCB's length
	//
	fcb->fcbPhysicalSize = (UInt64)nextBlock * (UInt64)vcb->vcbBlockSize;
	fcb->fcbFlags |= fcbModifiedMask;
	
	//
	//	If the new PEOF is 0, then truncateToExtent has no meaning (we should always deallocate
	//	all storage).
	//
	if (eofSectors == 0) {
		int i;
		
		//	Find the catalog extent record
		err = GetFCBExtentRecord(vcb, fcb, extentRecord);
		if (err != noErr) goto ErrorExit;	//	got some error, so return it
		
		//	Deallocate all the extents for this fork
		err = DeallocateFork(vcb, fcb->fcbFileID, forkType, extentRecord, &recordDeleted);
		if (err != noErr) goto ErrorExit;	//	got some error, so return it
		
		//	Update the catalog extent record (making sure it's zeroed out)
		if (err == noErr) {
			for (i=0; i < numExtentsPerRecord; i++) {
				extentRecord[i].startBlock = 0;
				extentRecord[i].blockCount = 0;
			}
		}
		err = SetFCBExtentRecord((VCB *) vcb, fcb, extentRecord);
		goto Done;
	}
	
	//
	//	Find the extent containing byte (peof-1).  This is the last extent we'll keep.
	//	(If truncateToExtent is true, we'll keep the whole extent; otherwise, we'll only
	//	keep up through peof).  The search will tell us how many allocation blocks exist
	//	in the found extent plus all previous extents.
	//
	err = SearchExtentFile(vcb, fcb, eofSectors-1, &key, extentRecord, &extentIndex, &hint, &extentNextBlock);
	if (err != noErr) goto ErrorExit;

	extentChanged = false;		//	haven't changed the extent yet
	
	if (!truncateToExtent) {
		//
		//	Shorten this extent.  It may be the case that the entire extent gets
		//	freed here.
		//
		numBlocks = extentNextBlock - nextBlock;	//	How many blocks in this extent to free up
		if (numBlocks != 0) {
			//	Compute first volume allocation block to free
			startBlock = extentRecord[extentIndex].startBlock + extentRecord[extentIndex].blockCount - numBlocks;
			//	Free the blocks in bitmap
			err = BlockDeallocate(vcb, startBlock, numBlocks);
			if (err != noErr) goto ErrorExit;
			//	Adjust length of this extent
			extentRecord[extentIndex].blockCount -= numBlocks;
			//	If extent is empty, set start block to 0
			if (extentRecord[extentIndex].blockCount == 0)
				extentRecord[extentIndex].startBlock = 0;
			//	Remember that we changed the extent record
			extentChanged = true;
		}
	}
	
	//
	//	Now move to the next extent in the record, and set up the file allocation block number
	//
	nextBlock = extentNextBlock;		//	Next file allocation block to free
	++extentIndex;						//	Its index within the extent record
	
	//
	//	Release all following extents in this extent record.  Update the record.
	//
	while (extentIndex < numExtentsPerRecord && extentRecord[extentIndex].blockCount != 0) {
		numBlocks = extentRecord[extentIndex].blockCount;
		//	Deallocate this extent
		err = BlockDeallocate(vcb, extentRecord[extentIndex].startBlock, numBlocks);
		if (err != noErr) goto ErrorExit;
		//	Update next file allocation block number
		nextBlock += numBlocks;
		//	Zero out start and length of this extent to delete it from record
		extentRecord[extentIndex].startBlock = 0;
		extentRecord[extentIndex].blockCount = 0;
		//	Remember that we changed an extent
		extentChanged = true;
		//	Move to next extent in record
		++extentIndex;
	}
	
	//
	//	If any of the extents in the current record were changed, then update that
	//	record (in the FCB, or extents file).
	//
	if (extentChanged) {
		err = UpdateExtentRecord(vcb, fcb, &key, extentRecord, hint);
		if (err != noErr) goto ErrorExit;
	}
	
	//
	//	If there are any following allocation blocks, then we need
	//	to seach for their extent records and delete those allocation
	//	blocks.
	//
	if (nextBlock < physNumBlocks)
		err = TruncateExtents(vcb, forkType, fcb->fcbFileID, nextBlock, &recordDeleted);

Done:
ErrorExit:

#if DEBUG_BUILD
	if (err == fxRangeErr)
		DebugStr("\pAbout to return fxRangeErr");
#endif

	//	[2355121] If we actually deleted extent records, then update the B-tree header
	if (recordDeleted)
		(void) FlushExtentFile(vcb);

	return err;
}
#endif


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Routine:	SearchExtentRecord (was XRSearch)
//
//	Function: 	Searches extent record for the extent mapping a given file
//				allocation block number (FABN).
//
//	Input:		searchFABN  			-  desired FABN
//				extentData  			-  pointer to extent data record (xdr)
//				extentDataStartFABN  	-  beginning FABN for extent record
//
//	Output:		foundExtentDataOffset  -  offset to extent entry within xdr
//							result = noErr, offset to extent mapping desired FABN
//							result = FXRangeErr, offset to last extent in record
//				endingFABNPlusOne	-  ending FABN +1
//				noMoreExtents		- True if the extent was not found, and the
//									  extent record was not full (so don't bother
//									  looking in subsequent records); false otherwise.
//
//	Result:		noErr = ok
//				FXRangeErr = desired FABN > last mapped FABN in record
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

static OSErr SearchExtentRecord(
	const SVCB		*vcb,
	UInt32					searchFABN,
	const HFSPlusExtentRecord	extentData,
	UInt32					extentDataStartFABN,
	UInt32					*foundExtentIndex,
	UInt32					*endingFABNPlusOne,
	Boolean					*noMoreExtents)
{
	OSErr	err = noErr;
	UInt32	extentIndex;
	UInt32	numberOfExtents;
	UInt32	numAllocationBlocks;
	Boolean	foundExtent;
	
	*endingFABNPlusOne 	= extentDataStartFABN;
	*noMoreExtents		= false;
	foundExtent			= false;

	if (vcb->vcbSignature == kHFSPlusSigWord)
		numberOfExtents = kHFSPlusExtentDensity;
	else
		numberOfExtents = kHFSExtentDensity;
	
	for( extentIndex = 0; extentIndex < numberOfExtents; ++extentIndex )
	{
		
		// Loop over the extent record and find the search FABN.
		
		numAllocationBlocks = extentData[extentIndex].blockCount;
		if ( numAllocationBlocks == 0 )
		{
			break;
		}

		*endingFABNPlusOne += numAllocationBlocks;
		
		if( searchFABN < *endingFABNPlusOne )
		{
			// Found the extent.
			foundExtent = true;
			break;
		}
	}
	
	if( foundExtent )
	{
		// Found the extent. Note the extent offset
		*foundExtentIndex = extentIndex;
	}
	else
	{
		// Did not find the extent. Set foundExtentDataOffset accordingly
		if( extentIndex > 0 )
		{
			*foundExtentIndex = extentIndex - 1;
		}
		else
		{
			*foundExtentIndex = 0;
		}
		
		// If we found an empty extent, then set noMoreExtents.
		if (extentIndex < numberOfExtents)
			*noMoreExtents = true;

		// Finally, return an error to the caller
		err = fxRangeErr;
	}

	return( err );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Routine:	SearchExtentFile (was XFSearch)
//
//	Function: 	Searches extent file (including the FCB resident extent record)
//				for the extent mapping a given file position.
//
//	Input:		vcb  			-  VCB pointer
//				fcb  			-  FCB pointer
//				filePosition  	-  file position (byte address)
//
// Output:		foundExtentKey  		-  extent key record (xkr)
//							If extent was found in the FCB's resident extent record,
//							then foundExtentKey->keyLength will be set to 0.
//				foundExtentData			-  extent data record(xdr)
//				foundExtentIndex  	-  index to extent entry in xdr
//							result =  0, offset to extent mapping desired FABN
//							result = FXRangeErr, offset to last extent in record
//									 (i.e., kNumExtentsPerRecord-1)
//				extentBTreeHint  		-  BTree hint for extent record
//							kNoHint = Resident extent record
//				endingFABNPlusOne  		-  ending FABN +1
//
//	Result:
//		noErr			Found an extent that contains the given file position
//		FXRangeErr		Given position is beyond the last allocated extent
//		(other)			(some other internal I/O error)
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

static OSErr SearchExtentFile(
	const SVCB 	*vcb,
	const SFCB 		*fcb,
	UInt64 				sectorOffset,
	HFSPlusExtentKey	*foundExtentKey,
	HFSPlusExtentRecord	foundExtentData,
	UInt32				*foundExtentIndex,
	UInt32				*extentBTreeHint,
	UInt32				*endingFABNPlusOne )
{
	OSErr				err;
	UInt32				filePositionBlock;
	Boolean				noMoreExtents = true;
	
	filePositionBlock = (UInt32)(sectorOffset / (vcb->vcbBlockSize >> kSectorShift));
	
	//	Search the resident FCB first.
	err = GetFCBExtentRecord(vcb, fcb, foundExtentData);
	if (err == noErr)
		err = SearchExtentRecord( vcb, filePositionBlock, foundExtentData, 0,
									foundExtentIndex, endingFABNPlusOne, &noMoreExtents );

	if( err == noErr ) {
		// Found the extent. Set results accordingly
		*extentBTreeHint = kNoHint;			// no hint, because not in the BTree
		foundExtentKey->keyLength = 0;		// 0 = the FCB itself
		
		goto Exit;
	}
	
	//	Didn't find extent in FCB.  If FCB's extent record wasn't full, there's no point
	//	in searching the extents file.  Note that SearchExtentRecord left us pointing at
	//	the last valid extent (or the first one, if none were valid).  This means we need
	//	to fill in the hint and key outputs, just like the "if" statement above.
	if ( noMoreExtents ) {
		*extentBTreeHint = kNoHint;			// no hint, because not in the BTree
		foundExtentKey->keyLength = 0;		// 0 = the FCB itself
		err = fxRangeErr;		// There are no more extents, so must be beyond PEOF
		goto Exit;
	}
	
	//
	//	Find the desired record, or the previous record if it is the same fork
	//
	err = FindExtentRecord(vcb, (fcb->fcbFlags & fcbResourceMask) ? kResourceForkType : kDataForkType,
						   fcb->fcbFileID, filePositionBlock, true, foundExtentKey, foundExtentData, extentBTreeHint);

	if (err == btNotFound) {
		//
		//	If we get here, the desired position is beyond the extents in the FCB, and there are no extents
		//	in the extents file.  Return the FCB's extents and a range error.
		//
		*extentBTreeHint = kNoHint;
		foundExtentKey->keyLength = 0;
		err = GetFCBExtentRecord(vcb, fcb, foundExtentData);
		//	Note: foundExtentIndex and endingFABNPlusOne have already been set as a result of the very
		//	first SearchExtentRecord call in this function (when searching in the FCB's extents, and
		//	we got a range error).
		
		return fxRangeErr;
	}
	
	//
	//	If we get here, there was either a BTree error, or we found an appropriate record.
	//	If we found a record, then search it for the correct index into the extents.
	//
	if (err == noErr) {
		//	Find appropriate index into extent record
		err = SearchExtentRecord(vcb, filePositionBlock, foundExtentData, foundExtentKey->startBlock,
								 foundExtentIndex, endingFABNPlusOne, &noMoreExtents);
	}

Exit:
	return err;
}



//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Routine:	UpdateExtentRecord
//
//	Function: 	Write new extent data to an existing extent record with a given key.
//				If all of the extents are empty, and the extent record is in the
//				extents file, then the record is deleted.
//
//	Input:		vcb			  			-	the volume containing the extents
//				fcb						-	the file that owns the extents
//				extentFileKey  			-	pointer to extent key record (xkr)
//						If the key length is 0, then the extents are actually part
//						of the catalog record, stored in the FCB.
//				extentData  			-	pointer to extent data record (xdr)
//				extentBTreeHint			-	hint for given key, or kNoHint
//
//	Result:		noErr = ok
//				(other) = error from BTree
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

OSErr UpdateExtentRecord (
	const SVCB		*vcb,
	SFCB					*fcb,
	const HFSPlusExtentKey	*extentFileKey,
	HFSPlusExtentRecord		extentData,
	UInt32					extentBTreeHint)
{
	OSErr	err;
	UInt32	foundHint;
	UInt16	foundDataSize;
	
	if (extentFileKey->keyLength == 0) {	// keyLength == 0 means the FCB's extent record
		err = SetFCBExtentRecord(vcb, fcb, extentData);
		fcb->fcbFlags |= fcbModifiedMask;
	}
	else {
		//
		//	Need to find and change a record in Extents BTree
		//
		if (vcb->vcbSignature == kHFSSigWord) {
			HFSExtentKey		key;			// Actual extent key used on disk in HFS
			HFSExtentKey		foundKey;		// The key actually found during search
			HFSExtentRecord	foundData;		// The extent data actually found

			key.keyLength	= kHFSExtentKeyMaximumLength;
			key.forkType	= extentFileKey->forkType;
			key.fileID		= extentFileKey->fileID;
			key.startBlock	= extentFileKey->startBlock;

			err = SearchBTreeRecord(vcb->vcbExtentsFile, &key, extentBTreeHint,
									&foundKey, &foundData, &foundDataSize, &foundHint);
			
			if (err == noErr)
				err = ExtentsToExtDataRec(extentData, (HFSExtentDescriptor *)&foundData);

			if (err == noErr)
				err = ReplaceBTreeRecord(vcb->vcbExtentsFile, &foundKey, foundHint, &foundData, foundDataSize, &foundHint);
		}
		else {		//	HFS Plus volume
			HFSPlusExtentKey		foundKey;		// The key actually found during search
			HFSPlusExtentRecord	foundData;		// The extent data actually found

			err = SearchBTreeRecord(vcb->vcbExtentsFile, extentFileKey, extentBTreeHint,
									&foundKey, &foundData, &foundDataSize, &foundHint);

			if (err == noErr)
				CopyMemory(extentData, &foundData, sizeof(HFSPlusExtentRecord));

			if (err == noErr)
				err = ReplaceBTreeRecord(vcb->vcbExtentsFile, &foundKey, foundHint, &foundData, foundDataSize, &foundHint);
		}
	}
	
	return err;
}


void ExtDataRecToExtents(
	const HFSExtentRecord		oldExtents,
	HFSPlusExtentRecord		newExtents)
{
	UInt32	i;

	// copy the first 3 extents
	newExtents[0].startBlock = oldExtents[0].startBlock;
	newExtents[0].blockCount = oldExtents[0].blockCount;
	newExtents[1].startBlock = oldExtents[1].startBlock;
	newExtents[1].blockCount = oldExtents[1].blockCount;
	newExtents[2].startBlock = oldExtents[2].startBlock;
	newExtents[2].blockCount = oldExtents[2].blockCount;

	// zero out the remaining ones
	for (i = 3; i < kHFSPlusExtentDensity; ++i)
	{
		newExtents[i].startBlock = 0;
		newExtents[i].blockCount = 0;
	}
}



static OSErr ExtentsToExtDataRec(
	HFSPlusExtentRecord	oldExtents,
	HFSExtentRecord		newExtents)
{
	OSErr	err;
	
	err = noErr;

	// copy the first 3 extents
	newExtents[0].startBlock = oldExtents[0].startBlock;
	newExtents[0].blockCount = oldExtents[0].blockCount;
	newExtents[1].startBlock = oldExtents[1].startBlock;
	newExtents[1].blockCount = oldExtents[1].blockCount;
	newExtents[2].startBlock = oldExtents[2].startBlock;
	newExtents[2].blockCount = oldExtents[2].blockCount;

	#if DEBUG_BUILD
		if (oldExtents[3].startBlock || oldExtents[3].blockCount) {
			DebugStr("\pExtentRecord with > 3 extents is invalid for HFS");
			err = fsDSIntErr;
		}
	#endif
	
	return err;
}


OSErr GetFCBExtentRecord(
	const SVCB	*vcb,
	const SFCB		*fcb,
	HFSPlusExtentRecord	extents)
{
	if (vcb->vcbSignature == kHFSPlusSigWord)
		CopyMemory(fcb->fcbExtents32, extents, sizeof(HFSPlusExtentRecord));
	else
		ExtDataRecToExtents(fcb->fcbExtents16, extents);
	return noErr;
}



static OSErr SetFCBExtentRecord(
	const SVCB				*vcb,
	SFCB					*fcb,
	HFSPlusExtentRecord		extents)
{

	#if DEBUG_BUILD
		if (fcb->fcbVolume != vcb)
			DebugStr("\pVCB does not match FCB");
	#endif

	if (vcb->vcbSignature == kHFSPlusSigWord)
		CopyMemory(extents, fcb->fcbExtents32, sizeof(HFSPlusExtentRecord));
	else
		(void) ExtentsToExtDataRec(extents, fcb->fcbExtents16);
	
	return noErr;
}



//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Routine:	MapFileBlockFromFCB
//
//	Function: 	Determine if the given file offset is within the set of extents
//				stored in the FCB.  If so, return the file allocation
//				block number of the start of the extent, volume allocation block number
//				of the start of the extent, and file allocation block number immediately
//				following the extent.
//
//	Input:		vcb			  			-	the volume containing the extents
//				fcb						-	the file that owns the extents
//				offset					-	desired offset in 512-byte sectors
//
//	Output:		firstFABN				-	file alloc block number of start of extent
//				firstBlock				-	volume alloc block number of start of extent
//				nextFABN				-	file alloc block number of next extent
//
//	Result:		noErr		= ok
//				fxRangeErr	= beyond FCB's extents
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
static OSErr MapFileBlockFromFCB(
	const SVCB		*vcb,
	const SFCB			*fcb,
	UInt64					sectorOffset,	// Desired offset in sectors from start of file
	UInt32					*firstFABN,		// FABN of first block of found extent
	UInt32					*firstBlock,	// Corresponding allocation block number
	UInt32					*nextFABN)		// FABN of block after end of extent
{
	UInt32	index;
	UInt32	offsetBlocks;
	
	offsetBlocks = (UInt32)(sectorOffset / (vcb->vcbBlockSize >> kSectorShift));
	
	if (vcb->vcbSignature == kHFSSigWord) {
		const HFSExtentDescriptor *extent;
		UInt32	blockCount;
		UInt32	currentFABN;
		
		extent = fcb->fcbExtents16;
		currentFABN = 0;
		
		for (index=0; index<kHFSExtentDensity; index++) {

			blockCount = extent->blockCount;

			if (blockCount == 0)
				return fxRangeErr;				//	ran out of extents!

			//	Is it in this extent?
			if (offsetBlocks < blockCount) {
				*firstFABN	= currentFABN;
				*firstBlock	= extent->startBlock;
				currentFABN += blockCount;		//	faster to add these as UInt16 first, then extend to UInt32
				*nextFABN	= currentFABN;
				return noErr;					//	found the right extent
			}

			//	Not in current extent, so adjust counters and loop again
			offsetBlocks -= blockCount;
			currentFABN += blockCount;
			extent++;
		}
	}
	else {
		const HFSPlusExtentDescriptor	*extent;
		UInt32	blockCount;
		UInt32	currentFABN;
		
		extent = fcb->fcbExtents32;
		currentFABN = 0;
		
		for (index=0; index<kHFSPlusExtentDensity; index++) {

			blockCount = extent->blockCount;

			if (blockCount == 0)
				return fxRangeErr;				//	ran out of extents!

			//	Is it in this extent?
			if (offsetBlocks < blockCount) {
				*firstFABN	= currentFABN;
				*firstBlock	= extent->startBlock;
				*nextFABN	= currentFABN + blockCount;
				return noErr;					//	found the right extent
			}

			//	Not in current extent, so adjust counters and loop again
			offsetBlocks -= blockCount;
			currentFABN += blockCount;
			extent++;
		}
	}
	
	//	If we fall through here, the extent record was full, but the offset was
	//	beyond those extents.
	
	return fxRangeErr;
}


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Routine:	ZeroFileBlocks
//
//	Function: 	Write all zeros to a range of a file.  Currently used when
//				extending a B-Tree, so that all the new allocation blocks
//				contain zeros (to prevent them from accidentally looking
//				like real data).
//
//	Input:		vcb			  			-	the volume
//				fcb						-	the file
//				startingSector			-	the first 512-byte sector to write
//				numberOfSectors			-	the number of sectors to zero
//
//	Result:		noErr		= ok
//				fxRangeErr	= beyond FCB's extents
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
#define FSBufferSize 32768

OSErr	ZeroFileBlocks( SVCB *vcb, SFCB *fcb, UInt32 startingSector, UInt32 numberOfSectors )
{
	Ptr					buffer;
	OSErr					err;
	HIOParam				iopb;
	UInt32					requestedBytes;
	UInt32					actualBytes;									//	Bytes actually read by CacheReadInPlace
	UInt32					bufferSizeSectors	= FSBufferSize >> kSectorShift;
	UInt64					currentPosition		= startingSector << kSectorShift;
		
	buffer	= AllocateMemory(FSBufferSize);
	if ( buffer == NULL )
		return( fileBoundsErr );
		
	ClearMemory( buffer, FSBufferSize );						//	Zero our buffer
	ClearMemory( &iopb, sizeof(iopb) );										//	Zero our param block
	
	iopb.ioRefNum	= ResolveFileRefNum( fcb );
	iopb.ioBuffer	= buffer;
	iopb.ioPosMode |= noCacheMask;											//	OR with the high byte

	do
	{
		if ( numberOfSectors > bufferSizeSectors )
			requestedBytes = FSBufferSize;
		else
			requestedBytes = numberOfSectors << kSectorShift;
		
		err = CacheWriteInPlace( vcb, iopb.ioRefNum, &iopb, currentPosition, requestedBytes, &actualBytes );

		if ( err || actualBytes == 0 )
			goto BAIL;

		//	Don't update ioActCount to force writing from beginning of zero buffer
		currentPosition	+= actualBytes;
		numberOfSectors	-= (actualBytes >> kSectorShift);

	} while( numberOfSectors > 0 );

BAIL:
	DisposeMemory(buffer);

	if ( err == noErr && numberOfSectors != 0 )
		err = eofErr;

	return( err );
}

//_________________________________________________________________________________
//
// Routine:		ExtentsAreIntegral
//
// Purpose:		Ensure that each extent can hold an integral number of nodes
//				Called by the NodesAreContiguous function
//_________________________________________________________________________________

static Boolean ExtentsAreIntegral(
	const HFSPlusExtentRecord extentRecord,
	UInt32		mask,
	UInt32		*blocksChecked,
	Boolean		*checkedLastExtent)
{
	UInt32		blocks;
	UInt32		extentIndex;

	*blocksChecked = 0;
	*checkedLastExtent = false;
	
	for(extentIndex = 0; extentIndex < kHFSPlusExtentDensity; extentIndex++)
	{		
		blocks = extentRecord[extentIndex].blockCount;
		
		if ( blocks == 0 )
		{
			*checkedLastExtent = true;
			break;
		}

		*blocksChecked += blocks;

		if (blocks & mask)
			return false;
	}
	
	return true;
}

//_________________________________________________________________________________
//
// Routine:		NodesAreContiguous
//
// Purpose:		Ensure that all b-tree nodes are contiguous on disk
//				Called by BTOpenPath during volume mount
//_________________________________________________________________________________

Boolean NodesAreContiguous(
	SFCB		*fcb,
	UInt32		nodeSize)
{
	SVCB	*vcb;
	UInt32		mask;
	UInt32		startBlock;
	UInt32		blocksChecked;
	UInt32		hint;
	HFSPlusExtentKey	key;
	HFSPlusExtentRecord	extents;
	OSErr			result;
	Boolean			lastExtentReached;
	
	
	vcb = (SVCB *)fcb->fcbVolume;

	if (vcb->vcbBlockSize >= nodeSize)
		return true;

	mask = (nodeSize / vcb->vcbBlockSize) - 1;

	// check the local extents
	(void) GetFCBExtentRecord(vcb, fcb, extents);
	if ( !ExtentsAreIntegral(extents, mask, &blocksChecked, &lastExtentReached) )
		return false;

	if (lastExtentReached || ((UInt64)blocksChecked * (UInt64)vcb->vcbBlockSize) >= fcb->fcbPhysicalSize)
		return true;

	startBlock = blocksChecked;

	// check the overflow extents (if any)
	while ( !lastExtentReached )
	{
		result = FindExtentRecord(vcb, kDataForkType, fcb->fcbFileID, startBlock, false, &key, extents, &hint);
		if (result) break;

		if ( !ExtentsAreIntegral(extents, mask, &blocksChecked, &lastExtentReached) )
			return false;

		startBlock += blocksChecked;
	}

	return true;
}
