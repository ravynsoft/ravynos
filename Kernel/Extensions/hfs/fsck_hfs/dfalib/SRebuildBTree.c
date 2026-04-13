/*
 * Copyright (c) 2002-2005, 2007-2009 Apple Inc. All rights reserved.
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
	File:		SRebuildBTree.c

	Contains:	This file contains BTree rebuilding code.
	
	Written by:	Jerry Cottingham

	Copyright:	� 1986, 1990, 1992-2002 by Apple Computer, Inc., all rights reserved.

*/

#define SHOW_ELAPSED_TIMES  0
#define DEBUG_REBUILD  0

extern void MyIndirectLog(const char *);

#if SHOW_ELAPSED_TIMES
#include <sys/time.h>
#endif

#include "Scavenger.h"
#include "../cache.h"

/* internal routine prototypes */

/*static*/ OSErr 	CreateNewBTree( SGlobPtr theSGlobPtr, int FileID );
static OSErr 	DeleteBTree( SGlobPtr theSGlobPtr, SFCB * theFCBPtr );
static OSErr 	InitializeBTree(	BTreeControlBlock * theBTreeCBPtr,
        							UInt32 *			theBytesUsedPtr,
        							UInt32 *			theMapNodeCountPtr 	);
static OSErr 	ReleaseExtentsInExtentsBTree(	SGlobPtr theSGlobPtr, 
												SFCB * theFCBPtr );
static OSErr 	ValidateCatalogRecordLength(	SGlobPtr theSGlobPtr,
										CatalogRecord * theRecPtr,
										UInt32 theRecSize );
static OSErr ValidateAttributeRecordLength (SGlobPtr s, HFSPlusAttrRecord * theRecPtr, UInt32 theRecSize);
static OSErr ValidateExtentRecordLength (SGlobPtr s, ExtentRecord * theRecPtr, UInt32 theRecSize);
static OSErr 	WriteMapNodes(  BTreeControlBlock * theBTreeCBPtr, 
								UInt32 				theFirstMapNode, 
								UInt32 				theNodeCount );

#if DEBUG_REBUILD 
static void PrintBTHeaderRec( BTHeaderRec * thePtr );
static void PrintNodeDescriptor( NodeDescPtr thePtr );
static void PrintBTreeKey( KeyPtr thePtr, BTreeControlBlock * theBTreeCBPtr );
static void PrintBTreeData(void *data, UInt32 length);
static void PrintIndexNodeRec( UInt32 theNodeNum );
static void PrintLeafNodeRec( HFSPlusCatalogFolder * thePtr );
#endif 

void SETOFFSET ( void *buffer, UInt16 btNodeSize, SInt16 recOffset, SInt16 vecOffset );
#define SETOFFSET( buf,ndsiz,offset,rec )		\
	( *(SInt16 *)((UInt8 *)(buf) + (ndsiz) + (-2 * (rec))) = (offset) )


//_________________________________________________________________________________
//
//	Routine:	RebuildBTree
//
//	Purpose:	Attempt to rebuild a B-Tree file using an existing B-Tree
//				file.  When successful a new BT-ree file will exist and
//				the old one will be deleted.  The MDB an alternate MDB
//				will be updated to point to the new file.  
//				
//				The tree is rebuilt by walking through every record.  We use
//				BTScanNextRecord(), which iterates sequentially through the
//				nodes in the tree (starting at the first node), and extracts
//				each record from each leaf node.  It does not use the node
//				forward or backward links; this allows it to rebuild the tree
//				when the index nodes are non-reliable, or the leaf node links
//				are damaged.
//
//				The rebuild will be aborted (leaving the existing btree
//				as it was found) if there are errors retreiving the nodes or
//				records, or if there are errors inserting the records into
//				the new tree.
//
//	Inputs:
//		SGlobPtr->calculatedCatalogBTCB or SGlobPtr->calculatedAttributesBTCB
//				need this as a model and in order to extract leaf records.
//		SGlobPtr->calculatedCatalogFCB or SGlobPtr->calculatedAttributesFCB
//				need this as a model and in order to extract leaf records.
//		SGlobPtr->calculatedRepairFCB
//				pointer to our repair FCB.
//		SGlobPtr->calculatedRepairBTCB
//				pointer to our repair BTreeControlBlock.
//
//	Outputs:
//		SGlobPtr->calculatedVCB
//				this will get mostly filled in here.  On input it is not fully
//				set up.
//		SGlobPtr->calculatedCatalogFCB or SGlobPtr->calculatedAttributesFCB
//				tis will refer to the new catalog file.
//
//	Result:
//		various error codes when problem occur or noErr if rebuild completed
//
//	to do:
//		have an option where we get back what we can.
//
//	Notes:
//		- requires input BTCB and FCBs to be valid!
//_________________________________________________________________________________

OSErr	RebuildBTree( SGlobPtr theSGlobPtr, int FileID )
{
	BlockDescriptor  		myBlockDescriptor;
	BTreeKeyPtr 			myCurrentKeyPtr;
	void * 					myCurrentDataPtr;
	SFCB *					myFCBPtr = NULL;
	SFCB *					oldFCBPtr = NULL;
	SVCB *					myVCBPtr;
	UInt32					myDataSize;
	UInt32					myHint;
	OSErr					myErr;
	Boolean 				isHFSPlus;
	UInt32					numRecords = 0;
	
#if SHOW_ELAPSED_TIMES 
	struct timeval 			myStartTime;
	struct timeval 			myEndTime;
	struct timeval 			myElapsedTime;
	struct timezone 		zone;
#endif
 
	theSGlobPtr->TarID = FileID;
	theSGlobPtr->TarBlock = 0;
	myBlockDescriptor.buffer = NULL;
	myVCBPtr = theSGlobPtr->calculatedVCB;
	if (kHFSCatalogFileID == FileID) {
		oldFCBPtr = theSGlobPtr->calculatedCatalogFCB;
	} else if (kHFSAttributesFileID == FileID) {
		oldFCBPtr = theSGlobPtr->calculatedAttributesFCB;
		/*
		 * 12447845
		 * If we don't have an attributes btree, then we should just
		 * quit now -- nothing to rebuild.
		 */
		if (oldFCBPtr->fcbLogicalSize == 0) {
			if (debug)
				plog("Requested attributes btree rebuild, but attributes file size is 0\n");
			myErr = 0;
			goto ExitThisRoutine;
		}
	} else if (kHFSExtentsFileID == FileID) {
		oldFCBPtr = theSGlobPtr->calculatedExtentsFCB;
	} else {
		abort();
	}

	myErr = BTScanInitialize( oldFCBPtr, &theSGlobPtr->scanState );
	if ( noErr != myErr )
		goto ExitThisRoutine;

	// some VCB fields that we need may not have been calculated so we get it from the MDB.
	// this can happen because the fsck_hfs code path to fully set up the VCB may have been 
	// aborted if an error was found that would trigger a rebuild.  For example,
	// if a leaf record was found to have a keys out of order then the verify phase of the
	// B-Tree check would be aborted and we would come directly (if allowable) to here.
	isHFSPlus = ( myVCBPtr->vcbSignature == kHFSPlusSigWord );

	if (!isHFSPlus) {
		myErr = noMacDskErr;
		goto ExitThisRoutine;
	}

	myErr = GetVolumeObjectVHBorMDB( &myBlockDescriptor );
	if ( noErr != myErr )
		goto ExitThisRoutine;

	if ( isHFSPlus )
	{
		HFSPlusVolumeHeader	*		myVHBPtr;
		
		myVHBPtr = (HFSPlusVolumeHeader	*) myBlockDescriptor.buffer;
		myVCBPtr->vcbFreeBlocks = myVHBPtr->freeBlocks;
		myVCBPtr->vcbFileCount = myVHBPtr->fileCount;
		myVCBPtr->vcbFolderCount = myVHBPtr->folderCount;
		myVCBPtr->vcbEncodingsBitmap = myVHBPtr->encodingsBitmap;
		myVCBPtr->vcbRsrcClumpSize = myVHBPtr->rsrcClumpSize;
		myVCBPtr->vcbDataClumpSize = myVHBPtr->dataClumpSize;
		
		//	check out creation and last mod dates
		myVCBPtr->vcbCreateDate	= myVHBPtr->createDate;	
		myVCBPtr->vcbModifyDate	= myVHBPtr->modifyDate;		
		myVCBPtr->vcbCheckedDate = myVHBPtr->checkedDate;		
		myVCBPtr->vcbBackupDate = myVHBPtr->backupDate;	
		myVCBPtr->vcbCatalogFile->fcbClumpSize = myVHBPtr->catalogFile.clumpSize;
		if (myVCBPtr->vcbAttributesFile != NULL) {
			myVCBPtr->vcbAttributesFile->fcbClumpSize = myVHBPtr->attributesFile.clumpSize;
		}
		myVCBPtr->vcbExtentsFile->fcbClumpSize = myVHBPtr->extentsFile.clumpSize;

		// 3882639: Removed check for volume attributes in HFS Plus 
		myVCBPtr->vcbAttributes = myVHBPtr->attributes;

		CopyMemory( myVHBPtr->finderInfo, myVCBPtr->vcbFinderInfo, sizeof(myVCBPtr->vcbFinderInfo) );
	}
	else
	{
		HFSMasterDirectoryBlock	*	myMDBPtr;
		myMDBPtr = (HFSMasterDirectoryBlock	*) myBlockDescriptor.buffer;
		myVCBPtr->vcbFreeBlocks = myMDBPtr->drFreeBks;
		myVCBPtr->vcbFileCount = myMDBPtr->drFilCnt;
		myVCBPtr->vcbFolderCount = myMDBPtr->drDirCnt;
		myVCBPtr->vcbDataClumpSize = myMDBPtr->drClpSiz;
		myVCBPtr->vcbCatalogFile->fcbClumpSize = myMDBPtr->drCTClpSiz;
		myVCBPtr->vcbNmRtDirs = myMDBPtr->drNmRtDirs;

		//	check out creation and last mod dates
		myVCBPtr->vcbCreateDate	= myMDBPtr->drCrDate;		
		myVCBPtr->vcbModifyDate	= myMDBPtr->drLsMod;			

		//	verify volume attribute flags
		if ( (myMDBPtr->drAtrb & VAtrb_Msk) == 0 )
			myVCBPtr->vcbAttributes = myMDBPtr->drAtrb;
		else 
			myVCBPtr->vcbAttributes = VAtrb_DFlt;
		myVCBPtr->vcbBackupDate = myMDBPtr->drVolBkUp;		
		myVCBPtr->vcbVSeqNum = myMDBPtr->drVSeqNum;		
		CopyMemory( myMDBPtr->drFndrInfo, myVCBPtr->vcbFinderInfo, sizeof(myMDBPtr->drFndrInfo) );
	}
	(void) ReleaseVolumeBlock( myVCBPtr, &myBlockDescriptor, kReleaseBlock );
	myBlockDescriptor.buffer = NULL;

	// create the new BTree file
	if (FileID == kHFSCatalogFileID || FileID == kHFSAttributesFileID || FileID == kHFSExtentsFileID) {
		myErr = CreateNewBTree( theSGlobPtr, FileID );
	} else {
		myErr = EINVAL;
	}
	if ( noErr != myErr ) {
#if DEBUG_REBUILD
		plog("CreateNewBTree returned %d\n", myErr);
#endif
		if (myErr == dskFulErr) {
			fsckPrint(theSGlobPtr->context, E_DiskFull);
		}
		goto ExitThisRoutine;
	}
	myFCBPtr = theSGlobPtr->calculatedRepairFCB;

#if SHOW_ELAPSED_TIMES
	gettimeofday( &myStartTime, &zone );
#endif
	
#if DEBUG_REBUILD
	if (debug) {
		int i;
		HFSPlusExtentDescriptor *te = (HFSPlusExtentDescriptor*)&theSGlobPtr->calculatedRepairFCB->fcbExtents32;
		printf("Extent records for rebuilt file %u:\n", FileID);
		for (i = 0; i < kHFSPlusExtentDensity; i++) {
			printf("\t[ %u, %u ]\n", te[i].startBlock, te[i].blockCount);
		}
	}
#endif

	while ( true )
	{
		/* scan the btree for leaf records */
		myErr = BTScanNextRecord( &theSGlobPtr->scanState, 
								  (void **) &myCurrentKeyPtr, 
								  (void **) &myCurrentDataPtr, 
								  &myDataSize  );
		if ( noErr != myErr )
			break;
		
		/* do some validation on the record */
		theSGlobPtr->TarBlock = theSGlobPtr->scanState.nodeNum;
		if (FileID == kHFSCatalogFileID) {
			myErr = ValidateCatalogRecordLength( theSGlobPtr, myCurrentDataPtr, myDataSize );
		} else if (FileID == kHFSAttributesFileID) {
			myErr = ValidateAttributeRecordLength( theSGlobPtr, myCurrentDataPtr, myDataSize );
		} else if (FileID == kHFSExtentsFileID) {
			myErr = ValidateExtentRecordLength( theSGlobPtr, myCurrentDataPtr, myDataSize );
		}
		if ( noErr != myErr )
		{
#if DEBUG_REBUILD 
			{
				plog( "%s - Record length validation (file %d) failed! \n", __FUNCTION__, FileID );
				plog( "%s - record %d in node %d is not recoverable. \n", 
						__FUNCTION__, (theSGlobPtr->scanState.recordNum - 1), 
						theSGlobPtr->scanState.nodeNum );
			}
#endif
			myErr = R_RFail;
			break;  // this implementation does not handle partial rebuilds (all or none)
		}

		/* insert this record into the new btree file */
		myErr = InsertBTreeRecord( myFCBPtr, myCurrentKeyPtr,
								   myCurrentDataPtr, myDataSize, &myHint );
		if ( noErr != myErr )
		{
#if DEBUG_REBUILD 
			{
				plog( "%s - InsertBTreeRecord failed with err %d 0x%02X \n", 
					__FUNCTION__, myErr, myErr );
				plog( "%s - numRecords = %d\n", __FUNCTION__, numRecords);
				plog( "%s - record %d in node %d is not recoverable. \n", 
						__FUNCTION__, (theSGlobPtr->scanState.recordNum - 1), 
						theSGlobPtr->scanState.nodeNum );
				PrintBTreeKey( myCurrentKeyPtr, theSGlobPtr->calculatedCatalogBTCB );
				PrintBTreeData( myCurrentDataPtr, myDataSize );
			}
			if (myErr == btExists)
				continue;
#endif
			if (dskFulErr == myErr)
			{
				fsckPrint(theSGlobPtr->context, E_DiskFull);
			}               
			myErr = R_RFail;
			break;  // this implementation does not handle partial rebuilds (all or none)
		}
		numRecords++;
#if DEBUG_REBUILD
		if (debug && ((numRecords % 1000) == 0))
			plog("btree file %d:  %u records\n", FileID, numRecords);
#endif
	}

#if SHOW_ELAPSED_TIMES
	gettimeofday( &myEndTime, &zone );
	timersub( &myEndTime, &myStartTime, &myElapsedTime );
	plog( "\n%s - rebuild btree %u %u records elapsed time \n", __FUNCTION__, FileID, numRecords );
	plog( ">>>>>>>>>>>>> secs %d msecs %d \n\n", myElapsedTime.tv_sec, myElapsedTime.tv_usec );
#endif

	if ( btNotFound == myErr )
		myErr = noErr;
	if ( noErr != myErr )
		goto ExitThisRoutine;

	/* update our header node on disk from our BTreeControlBlock */
	myErr = BTFlushPath( myFCBPtr );
	if ( noErr != myErr ) 
		goto ExitThisRoutine;
	myErr = CacheFlush( myVCBPtr->vcbBlockCache );
	if ( noErr != myErr ) 
		goto ExitThisRoutine;

	/* switch old file with our new one */
	if (FileID == kHFSCatalogFileID) {
		theSGlobPtr->calculatedRepairFCB = theSGlobPtr->calculatedCatalogFCB;
		theSGlobPtr->calculatedCatalogFCB = myFCBPtr;
		myVCBPtr->vcbCatalogFile = myFCBPtr;
		theSGlobPtr->calculatedCatalogFCB->fcbFileID = kHFSCatalogFileID;
		theSGlobPtr->calculatedRepairBTCB = theSGlobPtr->calculatedCatalogBTCB;
	} else if (FileID == kHFSAttributesFileID) {
		theSGlobPtr->calculatedRepairFCB = theSGlobPtr->calculatedAttributesFCB;
		theSGlobPtr->calculatedAttributesFCB = myFCBPtr;
		myVCBPtr->vcbAttributesFile = myFCBPtr;
		if (theSGlobPtr->calculatedAttributesFCB == NULL) {
			if (debug) {
				plog("Can't rebuilt attributes btree when there is no attributes file\n");
			}
			myErr = noErr;
			goto ExitThisRoutine;
		}
		theSGlobPtr->calculatedAttributesFCB->fcbFileID = kHFSAttributesFileID;
		theSGlobPtr->calculatedRepairBTCB = theSGlobPtr->calculatedAttributesBTCB;
	} else if (FileID == kHFSExtentsFileID) {
		theSGlobPtr->calculatedRepairFCB = theSGlobPtr->calculatedExtentsFCB;
		theSGlobPtr->calculatedExtentsFCB = myFCBPtr;
		myVCBPtr->vcbExtentsFile = myFCBPtr;
		theSGlobPtr->calculatedExtentsFCB->fcbFileID = kHFSExtentsFileID;
		theSGlobPtr->calculatedRepairBTCB = theSGlobPtr->calculatedExtentsBTCB;
	}
	
	// todo - add code to allow new btree file to be allocated in extents.
	// Note when we do allow this the swap of btree files gets even more 
	// tricky since extent record key contains the file ID.  The rebuilt 
	// file has file ID kHFSCatalogFileID/kHFSCatalogFileID when it is created.
	
	MarkVCBDirty( myVCBPtr );
	myErr = FlushAlternateVolumeControlBlock( myVCBPtr, isHFSPlus );
	if ( noErr != myErr )
	{
		// we may be totally screwed if we get here, try to recover
		if (FileID == kHFSCatalogFileID) {
			theSGlobPtr->calculatedCatalogFCB = theSGlobPtr->calculatedRepairFCB;
			theSGlobPtr->calculatedRepairFCB = myFCBPtr;
			myVCBPtr->vcbCatalogFile = theSGlobPtr->calculatedCatalogFCB;
		} else if (FileID == kHFSAttributesFileID) {
			theSGlobPtr->calculatedAttributesFCB = theSGlobPtr->calculatedRepairFCB;
			theSGlobPtr->calculatedRepairFCB = myFCBPtr;
			myVCBPtr->vcbAttributesFile = theSGlobPtr->calculatedAttributesFCB;
		} else if (FileID == kHFSExtentsFileID) {
			theSGlobPtr->calculatedExtentsFCB = theSGlobPtr->calculatedRepairFCB;
			theSGlobPtr->calculatedRepairFCB = myFCBPtr;
			myVCBPtr->vcbExtentsFile = theSGlobPtr->calculatedExtentsFCB;
		}
		MarkVCBDirty( myVCBPtr );
		(void) FlushAlternateVolumeControlBlock( myVCBPtr, isHFSPlus );
		goto ExitThisRoutine;
	}	

	/* release space occupied by old BTree file */
	(void) DeleteBTree( theSGlobPtr, theSGlobPtr->calculatedRepairFCB );
	if (FileID == kHFSExtentsFileID)
		(void)FlushExtentFile(myVCBPtr);

ExitThisRoutine:
	if ( myBlockDescriptor.buffer != NULL )
		(void) ReleaseVolumeBlock( myVCBPtr, &myBlockDescriptor, kReleaseBlock );

	if ( myErr != noErr && myFCBPtr != NULL ) 
		(void) DeleteBTree( theSGlobPtr, myFCBPtr );
	BTScanTerminate( &theSGlobPtr->scanState  );

	return( myErr );
	
} /* RebuildBTree */

//_________________________________________________________________________________
//
//	Routine:	CreateNewBTree
//
//	Purpose:	Create and Initialize a new B-Tree on the target volume 
//				using the physical size of the old (being rebuilt) file as an initial 
//				size.
//
//	NOTES:		we force this to be contiguous in order to get this into Jaguar.
//				Allowing the new file to go into extents makes the swap
//				of the old and new files complicated.  The extent records
//				are keyed by file ID and the new (rebuilt) btree file starts out as
//				file Id kHFSCatalogFileID/kHFSCatalogFileID/kHFSCatalogFileID.
//				 If there were extents then we would have to fix up the extent records in the extent B-Tree.
//
//	todo:		Don't force new file to be contiguous
//
//	Inputs:
//		SGlobPtr	global state set up by fsck_hfs.  We depend upon the 
//					manufactured and repair FCBs.
//
//	Outputs:
//		calculatedRepairBTCB 	fill in the BTreeControlBlock for new B-Tree file.
//		calculatedRepairFCB		fill in the SFCB for the new B-Tree file
//
//	Result:
//		various error codes when problems occur or noErr if all is well
//
//_________________________________________________________________________________

/*static*/ OSErr CreateNewBTree( SGlobPtr theSGlobPtr, int FileID )
{
	OSErr					myErr;
	BTreeControlBlock *		myBTreeCBPtr, * oldBCBPtr;
	SVCB *					myVCBPtr;
	SFCB *					myFCBPtr, * oldFCBPtr;
	UInt32 					myBytesUsed = 0;
	UInt32 					myMapNodeCount;
	UInt32					myNumBlocks;
	FSSize					myNewEOF;
	BTHeaderRec				myHeaderRec;
	
	myBTreeCBPtr = theSGlobPtr->calculatedRepairBTCB;
	myFCBPtr = theSGlobPtr->calculatedRepairFCB;
	ClearMemory( (Ptr) myFCBPtr, sizeof( *myFCBPtr ) );
	ClearMemory( (Ptr) myBTreeCBPtr, sizeof( *myBTreeCBPtr ) );

	if (FileID == kHFSCatalogFileID) {
		oldFCBPtr = theSGlobPtr->calculatedCatalogFCB;
		oldBCBPtr = theSGlobPtr->calculatedCatalogBTCB;
	} else if (FileID == kHFSAttributesFileID) {
		oldFCBPtr = theSGlobPtr->calculatedAttributesFCB;
		oldBCBPtr = theSGlobPtr->calculatedAttributesBTCB;
	} else if (FileID == kHFSExtentsFileID) {
		oldFCBPtr = theSGlobPtr->calculatedExtentsFCB;
		oldBCBPtr = theSGlobPtr->calculatedExtentsBTCB;
	} else
		abort();
		
	// Create new FCB
	myVCBPtr = oldFCBPtr->fcbVolume;
	if (FileID == kHFSCatalogFileID)
		myFCBPtr->fcbFileID = kHFSCatalogFileID;
	else if (FileID == kHFSAttributesFileID)
		myFCBPtr->fcbFileID = kHFSAttributesFileID;
	else if (FileID == kHFSExtentsFileID)
		myFCBPtr->fcbFileID = kHFSExtentsFileID;

	myFCBPtr->fcbVolume = myVCBPtr;
	myFCBPtr->fcbBtree = myBTreeCBPtr;
	myFCBPtr->fcbBlockSize = oldBCBPtr->nodeSize;

	// Create new BTree Control Block
	myBTreeCBPtr->fcbPtr = myFCBPtr;
	myBTreeCBPtr->btreeType = kHFSBTreeType;
	myBTreeCBPtr->keyCompareType = oldBCBPtr->keyCompareType;
	myBTreeCBPtr->keyCompareProc = oldBCBPtr->keyCompareProc;
	myBTreeCBPtr->nodeSize = oldBCBPtr->nodeSize;
	myBTreeCBPtr->maxKeyLength = oldBCBPtr->maxKeyLength;
	if (myVCBPtr->vcbSignature == kHFSPlusSigWord) {
		if (FileID == kHFSExtentsFileID)
			myBTreeCBPtr->attributes = kBTBigKeysMask;
		else
			myBTreeCBPtr->attributes = ( kBTBigKeysMask + kBTVariableIndexKeysMask );
	}

	myBTreeCBPtr->getBlockProc = GetFileBlock;
	myBTreeCBPtr->releaseBlockProc = ReleaseFileBlock;
	myBTreeCBPtr->setEndOfForkProc = SetEndOfForkProc;

	myNewEOF = oldFCBPtr->fcbPhysicalSize;

	myNumBlocks = myNewEOF / myVCBPtr->vcbBlockSize;
	myErr = BlockFindAll( myBTreeCBPtr->fcbPtr, myNumBlocks);
	ReturnIfError( myErr );
	myBTreeCBPtr->fcbPtr->fcbPhysicalSize = myNewEOF;
	myErr = ZeroFileBlocks( myVCBPtr, myBTreeCBPtr->fcbPtr, 0, (UInt32)(myNewEOF >> kSectorShift) );
	ReturnIfError( myErr );

	/* now set real values in our BTree Control Block */
	myFCBPtr->fcbLogicalSize = myFCBPtr->fcbPhysicalSize;		// new B-tree looks at fcbLogicalSize
	if (FileID == kHFSCatalogFileID)
		myFCBPtr->fcbClumpSize = myVCBPtr->vcbCatalogFile->fcbClumpSize; 
	else if (FileID == kHFSAttributesFileID)
		myFCBPtr->fcbClumpSize = myVCBPtr->vcbAttributesFile->fcbClumpSize; 
	else if (FileID == kHFSExtentsFileID)
		myFCBPtr->fcbClumpSize = myVCBPtr->vcbExtentsFile->fcbClumpSize;
	
	myBTreeCBPtr->totalNodes = (UInt32)( myFCBPtr->fcbPhysicalSize / myBTreeCBPtr->nodeSize );
	myBTreeCBPtr->freeNodes = myBTreeCBPtr->totalNodes;

	// Initialize our new BTree (write out header node and an empty leaf node)
	myErr = InitializeBTree( myBTreeCBPtr, &myBytesUsed, &myMapNodeCount );
	ReturnIfError( myErr );
	
	// Update our BTreeControlBlock from BTHeaderRec we just wrote out
	myErr = GetBTreeHeader( theSGlobPtr, myFCBPtr, &myHeaderRec );
	ReturnIfError( myErr );
	
	myBTreeCBPtr->treeDepth = myHeaderRec.treeDepth;
	myBTreeCBPtr->rootNode = myHeaderRec.rootNode;
	myBTreeCBPtr->leafRecords = myHeaderRec.leafRecords;
	myBTreeCBPtr->firstLeafNode = myHeaderRec.firstLeafNode;
	myBTreeCBPtr->lastLeafNode = myHeaderRec.lastLeafNode;
	myBTreeCBPtr->totalNodes = myHeaderRec.totalNodes;
	myBTreeCBPtr->freeNodes = myHeaderRec.freeNodes;
	myBTreeCBPtr->maxKeyLength = myHeaderRec.maxKeyLength;

	if ( myMapNodeCount > 0 )
	{
		myErr = WriteMapNodes( myBTreeCBPtr, (myBytesUsed / myBTreeCBPtr->nodeSize ), myMapNodeCount );
		ReturnIfError( myErr );
	}

	return( myErr );
	
} /* CreateNewBTree */


/*
 * InitializeBTree
 *	
 * This routine manufactures and writes out a B-Tree header 
 * node and an empty leaf node.
 *
 * Note: Since large volumes can have bigger b-trees they
 * might need to have map nodes setup.
 *
 * this routine originally came from newfs_hfs.tproj ( see 
 * WriteCatalogFile in file makehfs.c) and was modified for fsck_hfs.
 */
static OSErr InitializeBTree(	BTreeControlBlock * theBTreeCBPtr,
        						UInt32 *			theBytesUsedPtr,
        						UInt32 *			theMapNodeCountPtr 	)
{
	OSErr				myErr;
	BlockDescriptor		myNode;
	Boolean 			isHFSPlus = false;
	SVCB *				myVCBPtr;
	BTNodeDescriptor *	myNodeDescPtr;
	BTHeaderRec *		myHeaderRecPtr;
	UInt8 *				myBufferPtr;
	UInt8 *				myBitMapPtr;
	UInt32				myNodeBitsInHeader;
	UInt32				temp;
	SInt16				myOffset;

	myVCBPtr = theBTreeCBPtr->fcbPtr->fcbVolume;
	isHFSPlus = ( myVCBPtr->vcbSignature == kHFSPlusSigWord) ;
	*theMapNodeCountPtr = 0;

	myErr = GetNewNode( theBTreeCBPtr, kHeaderNodeNum, &myNode );
	ReturnIfError( myErr );
	
	myBufferPtr = (UInt8 *) myNode.buffer;
	
	/* FILL IN THE NODE DESCRIPTOR:  */
	myNodeDescPtr 		= (BTNodeDescriptor *) myBufferPtr;
	myNodeDescPtr->kind 	= kBTHeaderNode;
	myNodeDescPtr->numRecords = 3;
	myOffset = sizeof( BTNodeDescriptor );

	SETOFFSET( myBufferPtr, theBTreeCBPtr->nodeSize, myOffset, 1 );

	/* FILL IN THE HEADER RECORD:  */
	myHeaderRecPtr = (BTHeaderRec *)((UInt8 *)myBufferPtr + myOffset);
	myHeaderRecPtr->treeDepth		= 0;
	myHeaderRecPtr->rootNode		= 0;
	myHeaderRecPtr->firstLeafNode	= 0;
	myHeaderRecPtr->lastLeafNode	= 0;
	myHeaderRecPtr->nodeSize		= theBTreeCBPtr->nodeSize;
	myHeaderRecPtr->totalNodes		= theBTreeCBPtr->totalNodes;
	myHeaderRecPtr->freeNodes		= myHeaderRecPtr->totalNodes - 1;  /* header node */
	myHeaderRecPtr->clumpSize		= theBTreeCBPtr->fcbPtr->fcbClumpSize;

	myHeaderRecPtr->attributes	= theBTreeCBPtr->attributes;
	myHeaderRecPtr->maxKeyLength = theBTreeCBPtr->maxKeyLength;
	myHeaderRecPtr->keyCompareType = theBTreeCBPtr->keyCompareType;

	myOffset += sizeof( BTHeaderRec );
	SETOFFSET( myBufferPtr, theBTreeCBPtr->nodeSize, myOffset, 2 );

	myOffset += kBTreeHeaderUserBytes;
	SETOFFSET( myBufferPtr, theBTreeCBPtr->nodeSize, myOffset, 3 );

	/* FIGURE OUT HOW MANY MAP NODES (IF ANY):  */
	myNodeBitsInHeader = 8 * (theBTreeCBPtr->nodeSize
							- sizeof(BTNodeDescriptor)
							- sizeof(BTHeaderRec)
							- kBTreeHeaderUserBytes
							- (4 * sizeof(SInt16)) );

	if ( myHeaderRecPtr->totalNodes > myNodeBitsInHeader ) 
	{
		UInt32	nodeBitsInMapNode;
		
		myNodeDescPtr->fLink = myHeaderRecPtr->lastLeafNode + 1;
		nodeBitsInMapNode = 8 * (theBTreeCBPtr->nodeSize
								- sizeof(BTNodeDescriptor)
								- (2 * sizeof(SInt16))
								- 2 );
		*theMapNodeCountPtr = (myHeaderRecPtr->totalNodes - myNodeBitsInHeader +
			(nodeBitsInMapNode - 1)) / nodeBitsInMapNode;
		myHeaderRecPtr->freeNodes = myHeaderRecPtr->freeNodes - *theMapNodeCountPtr;
	}

	/* 
	 * FILL IN THE MAP RECORD, MARKING NODES THAT ARE IN USE.
	 * Note - worst case (32MB alloc blk) will have only 18 nodes in use.
	 */
	myBitMapPtr = ((UInt8 *)myBufferPtr + myOffset);
	temp = myHeaderRecPtr->totalNodes - myHeaderRecPtr->freeNodes;

	/* Working a byte at a time is endian safe */
	while ( temp >= 8 ) 
	{ 
		*myBitMapPtr = 0xFF; 
		temp -= 8; 
		myBitMapPtr++; 
	}
	*myBitMapPtr = ~(0xFF >> temp);
	myOffset += myNodeBitsInHeader / 8;

	SETOFFSET( myBufferPtr, theBTreeCBPtr->nodeSize, myOffset, 4 );

	*theBytesUsedPtr = 
		( myHeaderRecPtr->totalNodes - myHeaderRecPtr->freeNodes - *theMapNodeCountPtr ) 
			* theBTreeCBPtr->nodeSize;

	/* write header node */
	myErr = UpdateNode( theBTreeCBPtr, &myNode );
	M_ExitOnError( myErr );
	
	return	noErr;

ErrorExit:
	(void) ReleaseNode( theBTreeCBPtr, &myNode );
		
	return( myErr );
	
} /* InitializeBTree */


/*
 * WriteMapNodes
 *	
 * This routine manufactures and writes out a B-Tree map 
 * node (or nodes if there are more than one).
 *
 * this routine originally came from newfs_hfs.tproj ( see 
 * WriteMapNodes in file makehfs.c) and was modified for fsck_hfs.
 */

static OSErr WriteMapNodes(	BTreeControlBlock * theBTreeCBPtr, 
							UInt32 				theFirstMapNode, 
							UInt32 				theNodeCount )
{
	OSErr				myErr;
	UInt16				i;
	UInt32				mapRecordBytes;
	BTNodeDescriptor *	myNodeDescPtr;
	BlockDescriptor		myNode;

	myNode.buffer = NULL;
	
	/*
	 * Note - worst case (32MB alloc blk) will have
	 * only 18 map nodes. So don't bother optimizing
	 * this section to do multiblock writes!
	 */
	for ( i = 0; i < theNodeCount; i++ ) 
	{
		myErr = GetNewNode( theBTreeCBPtr, theFirstMapNode, &myNode );
		M_ExitOnError( myErr );
	
		myNodeDescPtr = (BTNodeDescriptor *) myNode.buffer;
		myNodeDescPtr->kind			= kBTMapNode;
		myNodeDescPtr->numRecords	= 1;
		
		/* note: must be long word aligned (hence the extra -2) */
		mapRecordBytes = theBTreeCBPtr->nodeSize - sizeof(BTNodeDescriptor) - 2 * sizeof(SInt16) - 2;	
	
		SETOFFSET( myNodeDescPtr, theBTreeCBPtr->nodeSize, sizeof(BTNodeDescriptor), 1 );
		SETOFFSET( myNodeDescPtr, theBTreeCBPtr->nodeSize, sizeof(BTNodeDescriptor) + mapRecordBytes, 2) ;

		if ( (i + 1) < theNodeCount )
			myNodeDescPtr->fLink = ++theFirstMapNode;  /* point to next map node */
		else
			myNodeDescPtr->fLink = 0;  /* this is the last map node */

		myErr = UpdateNode( theBTreeCBPtr, &myNode );
		M_ExitOnError( myErr );
	}
	
	return	noErr;

ErrorExit:
	(void) ReleaseNode( theBTreeCBPtr, &myNode );
		
	return( myErr );
	
} /* WriteMapNodes */


/*
 * DeleteBTree
 *	
 * This routine will realease all space associated with the BTree
 * file represented by the FCB passed in.  
 *
 */

enum
{
	kDataForkType			= 0,
	kResourceForkType		= 0xFF
};

static OSErr DeleteBTree( SGlobPtr theSGlobPtr, SFCB * theFCBPtr )
{
	OSErr			myErr;
	SVCB *			myVCBPtr;
	int				i;
	Boolean 		isHFSPlus;
	Boolean			checkExtentsBTree = true;

	myVCBPtr = theFCBPtr->fcbVolume;
	isHFSPlus = ( myVCBPtr->vcbSignature == kHFSPlusSigWord) ;
	
	if ( isHFSPlus )
	{
		for ( i = 0; i < kHFSPlusExtentDensity; ++i ) 
		{
			if ( theFCBPtr->fcbExtents32[ i ].blockCount == 0 )
			{
				checkExtentsBTree = false;
				break;
			}
			(void) BlockDeallocate( myVCBPtr, 
									theFCBPtr->fcbExtents32[ i ].startBlock, 
									theFCBPtr->fcbExtents32[ i ].blockCount );
			theFCBPtr->fcbExtents32[ i ].startBlock = 0;
			theFCBPtr->fcbExtents32[ i ].blockCount = 0;
		}
	}
	else
	{
		for ( i = 0; i < kHFSExtentDensity; ++i ) 
		{
			if ( theFCBPtr->fcbExtents16[ i ].blockCount == 0 )
			{
				checkExtentsBTree = false;
				break;
			}
			(void) BlockDeallocate( myVCBPtr, 
									theFCBPtr->fcbExtents16[ i ].startBlock, 
									theFCBPtr->fcbExtents16[ i ].blockCount );
			theFCBPtr->fcbExtents16[ i ].startBlock = 0;
			theFCBPtr->fcbExtents16[ i ].blockCount = 0;
		}
	}
	
	if ( checkExtentsBTree )
	{
		(void) ReleaseExtentsInExtentsBTree( theSGlobPtr, theFCBPtr );
		(void) FlushExtentFile( myVCBPtr );
	}

	(void) MarkVCBDirty( myVCBPtr );
	(void) FlushAlternateVolumeControlBlock( myVCBPtr, isHFSPlus );
	myErr = noErr;
	
	return( myErr );
	
} /* DeleteBTree */


/*
 * ReleaseExtentsInExtentsBTree
 *	
 * This routine will locate extents in the extent BTree then release the space
 * associated with the extents.  It will also delete the BTree record for the
 * extent.
 *
 */

static OSErr ReleaseExtentsInExtentsBTree( 	SGlobPtr theSGlobPtr, 
											SFCB * theFCBPtr )
{
	BTreeIterator       myIterator;
	ExtentRecord		myExtentRecord;
	FSBufferDescriptor  myBTRec;
	ExtentKey *			myKeyPtr;
	SVCB *				myVCBPtr;
	UInt16              myRecLen;
	UInt16				i;
	OSErr				myErr;
	Boolean 			isHFSPlus;

	myVCBPtr = theFCBPtr->fcbVolume;
	isHFSPlus = ( myVCBPtr->vcbSignature == kHFSPlusSigWord );

	// position just before the first extent record for the given File ID.  We 
	// pass in the file ID and a start block of 0 which will put us in a 
	// position for BTIterateRecord (with kBTreeNextRecord) to get the first 
	// extent record.
	ClearMemory( &myIterator, sizeof(myIterator) );
	myBTRec.bufferAddress = &myExtentRecord;
	myBTRec.itemCount = 1;
	myBTRec.itemSize = sizeof(myExtentRecord);
	myKeyPtr = (ExtentKey *) &myIterator.key;

	BuildExtentKey( isHFSPlus, kDataForkType, theFCBPtr->fcbFileID, 
					0, (void *) myKeyPtr );

	// it is now a simple process of getting the next extent record and 
	// cleaning up the allocated space for each one until we hit a 
	// different file ID.
	for ( ;; )
	{
		myErr = BTIterateRecord( theSGlobPtr->calculatedExtentsFCB, 
								 kBTreeNextRecord, &myIterator,
								 &myBTRec, &myRecLen );
		if ( noErr != myErr )
		{
			myErr = noErr;
			break;
		}
		
		/* deallocate space for the extents we found */
		if ( isHFSPlus )
		{
			// we're done if this is a different File ID
			if ( myKeyPtr->hfsPlus.fileID != theFCBPtr->fcbFileID ||
				 myKeyPtr->hfsPlus.forkType != kDataForkType )
					break;

			for ( i = 0; i < kHFSPlusExtentDensity; ++i ) 
			{
				if ( myExtentRecord.hfsPlus[ i ].blockCount == 0 )
					break;

				(void) BlockDeallocate( myVCBPtr, 
										myExtentRecord.hfsPlus[ i ].startBlock,
										myExtentRecord.hfsPlus[ i ].blockCount );
			}
		}
		else
		{
			// we're done if this is a different File ID
			if ( myKeyPtr->hfs.fileID != theFCBPtr->fcbFileID ||
				 myKeyPtr->hfs.forkType != kDataForkType )
					break;

			for ( i = 0; i < kHFSExtentDensity; ++i ) 
			{
				if ( myExtentRecord.hfs[ i ].blockCount == 0 )
					break;

				(void) BlockDeallocate( myVCBPtr, 
										myExtentRecord.hfs[ i ].startBlock,
										myExtentRecord.hfs[ i ].blockCount );
			}
		}

		/* get rid of this extent BTree record */
		myErr = DeleteBTreeRecord( theSGlobPtr->calculatedExtentsFCB, myKeyPtr );
	}
	
	return( myErr );

} /* ReleaseExtentsInExtentsBTree */


/*
 * ValidateExtentRecordLength
 * This routine will ensure that an extent record is the right size.
 * This should always be the size of HFSPlusExtentRecord.
 */
static OSErr ValidateExtentRecordLength (SGlobPtr s, ExtentRecord * theRecPtr, UInt32 theRecSize)
{
	Boolean isHFSPlus = ( s->calculatedVCB->vcbSignature == kHFSPlusSigWord );
	if (isHFSPlus) {
		if (theRecSize != sizeof(HFSPlusExtentRecord))
			return -1;
	} else {
		if (theRecSize != sizeof(HFSExtentRecord))
			return -1;
	}

	return noErr;
}

/*
 * ValidateAttributeRecordLength
 *
 * This routine will make sure that the given HFS+ attributes file record
 * is of the correct length.
 *
 */
static OSErr ValidateAttributeRecordLength (SGlobPtr s, HFSPlusAttrRecord * theRecPtr, UInt32 theRecSize)
{
	OSErr retval = noErr;
	static UInt32 maxInlineSize;

	if (maxInlineSize == 0) {
		/* The maximum size of an inline attribute record is nodesize / 2 minus a bit */
		/* These calculations taken from hfs_xattr.c:getmaxinlineattrsize */
		maxInlineSize = s->calculatedAttributesBTCB->nodeSize;
		maxInlineSize -= sizeof(BTNodeDescriptor);	// Minus node descriptor
		maxInlineSize -= 3 * sizeof(u_int16_t);	// Minus 3 index slots
		maxInlineSize /= 2;				// 2 key/rec pairs minimum
		maxInlineSize -= sizeof(HFSPlusAttrKey);	// Minus maximum key size
		maxInlineSize &= 0xFFFFFFFE;				// Multiple of two
	}
	switch (theRecPtr->recordType) {
	case kHFSPlusAttrInlineData:
		if (theRecSize > maxInlineSize) {
			if (debug)
				plog("Inline Attribute size %u is larger than maxsize %u\n", theRecSize, maxInlineSize);
			retval = -1;
		}
		break;
	case kHFSPlusAttrForkData:
		if (theRecSize != sizeof(HFSPlusAttrForkData)) {
			if (debug)
				plog("Fork Data attribute size %u is larger then HFSPlusAttrForkData size %u\n", theRecSize, sizeof(HFSPlusAttrForkData));
			retval = -1;
		}
		break;
	case kHFSPlusAttrExtents:
		if (theRecSize != sizeof(HFSPlusAttrExtents)) {
			if (debug)
				plog("Extents Data attribute size %u is larger than HFSPlusAttrExtents size %u\n", theRecSize, sizeof(HFSPlusAttrExtents));
			retval = -1;
		}
		break;
	default:
		// Right now, we don't support any other kind
		if (debug)
			plog("Unknown attribute type %u\n", theRecPtr->recordType);
		retval = -1;
		break;
	}
	return retval;
}

/*
 * ValidateCatalogRecordLength
 *	
 * This routine will make sure the given HFS (plus and standard) catalog record
 * is of the correct length.
 *
 */

static OSErr ValidateCatalogRecordLength( 	SGlobPtr theSGlobPtr,
									CatalogRecord * theRecPtr,
									UInt32 theRecSize )
{
	SVCB *					myVCBPtr;
	Boolean 				isHFSPlus = false;

	myVCBPtr = theSGlobPtr->calculatedVCB;
	isHFSPlus = ( myVCBPtr->vcbSignature == kHFSPlusSigWord );
	
	if ( isHFSPlus )
	{
		switch ( theRecPtr->recordType ) 
		{
		case kHFSPlusFolderRecord:
			if ( theRecSize != sizeof( HFSPlusCatalogFolder ) )
			{
				return( -1 );
			}
			break;
	
		case kHFSPlusFileRecord:
			if ( theRecSize != sizeof(HFSPlusCatalogFile) )
			{
				return( -1 );
			}
			break;
	
		case kHFSPlusFolderThreadRecord:
			/* Fall through */
	
		case kHFSPlusFileThreadRecord:
			if ( theRecSize > sizeof(HFSPlusCatalogThread) ||
				 theRecSize < sizeof(HFSPlusCatalogThread) - sizeof(HFSUniStr255) + sizeof(UniChar) ) 
			{
				return( -1 );
			}
			break;
	
		default:
			return( -1 );
		}
	}
	else
	{
		switch ( theRecPtr->recordType ) 
		{
		case kHFSFolderRecord:
			if ( theRecSize != sizeof(HFSCatalogFolder) )
				return( -1 );
			break;
	
		case kHFSFileRecord:
			if ( theRecSize != sizeof(HFSCatalogFile) )
				return( -1 );
			break;
	
		case kHFSFolderThreadRecord:
			/* Fall through */
		case kHFSFileThreadRecord:
			if ( theRecSize != sizeof(HFSCatalogThread)) 
				return( -1 );
			break;
	
		default:
			return( -1 );
		}
	}
	
	return( noErr );
	
} /* ValidateCatalogRecordLength */


#if DEBUG_REBUILD 
static void PrintNodeDescriptor( NodeDescPtr thePtr )
{
	plog( "\n xxxxxxxx BTNodeDescriptor xxxxxxxx \n" );
	plog( "   fLink %d \n", thePtr->fLink );
	plog( "   bLink %d \n", thePtr->bLink );
	plog( "   kind %d ", thePtr->kind );
	if ( thePtr->kind == kBTLeafNode )
		plog( "%s \n", "kBTLeafNode" );
	else if ( thePtr->kind == kBTIndexNode )
		plog( "%s \n", "kBTIndexNode" );
	else if ( thePtr->kind == kBTHeaderNode )
		plog( "%s \n", "kBTHeaderNode" );
	else if ( thePtr->kind == kBTMapNode )
		plog( "%s \n", "kBTMapNode" );
	else
		plog( "do not know?? \n" );
	plog( "   height %d \n", thePtr->height );
	plog( "   numRecords %d \n", thePtr->numRecords );

} /* PrintNodeDescriptor */


static void PrintBTHeaderRec( BTHeaderRec * thePtr )
{
	plog( "\n xxxxxxxx BTHeaderRec xxxxxxxx \n" );
	plog( "   treeDepth %d \n", thePtr->treeDepth );
	plog( "   rootNode %d \n", thePtr->rootNode );
	plog( "   leafRecords %d \n", thePtr->leafRecords );
	plog( "   firstLeafNode %d \n", thePtr->firstLeafNode );
	plog( "   lastLeafNode %d \n", thePtr->lastLeafNode );
	plog( "   nodeSize %d \n", thePtr->nodeSize );
	plog( "   maxKeyLength %d \n", thePtr->maxKeyLength );
	plog( "   totalNodes %d \n", thePtr->totalNodes );
	plog( "   freeNodes %d \n", thePtr->freeNodes );
	plog( "   clumpSize %d \n", thePtr->clumpSize );
	plog( "   btreeType 0x%02X \n", thePtr->btreeType );
	plog( "   attributes 0x%02X \n", thePtr->attributes );

} /* PrintBTHeaderRec */

			
static void PrintBTreeKey( KeyPtr thePtr, BTreeControlBlock * theBTreeCBPtr )
{
	int		myKeyLength, i;
	UInt8 *	myPtr = (UInt8 *)thePtr;
	char	ascii[17];
	UInt8	byte;
	
	ascii[16] = '\0';
	
	myKeyLength = CalcKeySize( theBTreeCBPtr, thePtr) ;
	plog( "\n xxxxxxxx BTreeKey (length %d) xxxxxxxx \n", myKeyLength );
	for ( i = 0; i < myKeyLength; i++ )
	{
		byte = *(myPtr + i);
		plog( "%02X ", byte );
		if (byte < 32 || byte > 126)
			ascii[i & 0xF] = '.';
		else
			ascii[i & 0xF] = byte;
		
		if ((i & 0xF) == 0xF)
		{
			plog("  %s\n", ascii);
		}
	}
	
	if (i & 0xF)
	{
		int j;
		for (j = i & 0xF; j < 16; ++j)
			plog("   ");
		ascii[i & 0xF] = 0;
		plog("  %s\n", ascii);
	}
} /* PrintBTreeKey */

static void PrintBTreeData(void *data, UInt32 length)
{
	UInt32	i;
	UInt8 *	myPtr = (UInt8 *)data;
	char	ascii[17];
	UInt8	byte;
	
	ascii[16] = '\0';

	plog( "\n xxxxxxxx BTreeData (length %d) xxxxxxxx \n", length );
	for ( i = 0; i < length; i++ )
	{
		byte = *(myPtr + i);
		plog( "%02X ", byte );
		if (byte < 32 || byte > 126)
			ascii[i & 0xF] = '.';
		else
			ascii[i & 0xF] = byte;
		
		if ((i & 0xF) == 0xF)
		{
			plog("  %s\n", ascii);
		}
	}
	
	if (i & 0xF)
	{
		int j;
		for (j = i & 0xF; j < 16; ++j)
			plog("   ");
		ascii[i & 0xF] = 0;
		plog("  %s\n", ascii);
	}
}

static void PrintIndexNodeRec( UInt32 theNodeNum )
{
	plog( "\n xxxxxxxx IndexNodeRec xxxxxxxx \n" );
	plog( "   node number %d \n", theNodeNum );

} /* PrintIndexNodeRec */
			
static void PrintLeafNodeRec( HFSPlusCatalogFolder * thePtr )
{
	plog( "\n xxxxxxxx LeafNodeRec xxxxxxxx \n" );
	plog( "   recordType %d ", thePtr->recordType );
	if ( thePtr->recordType == kHFSPlusFolderRecord )
		plog( "%s \n", "kHFSPlusFolderRecord" );
	else if ( thePtr->recordType == kHFSPlusFileRecord )
		plog( "%s \n", "kHFSPlusFileRecord" );
	else if ( thePtr->recordType == kHFSPlusFolderThreadRecord )
		plog( "%s \n", "kHFSPlusFolderThreadRecord" );
	else if ( thePtr->recordType == kHFSPlusFileThreadRecord )
		plog( "%s \n", "kHFSPlusFileThreadRecord" );
	else
		plog( "do not know?? \n" );

} /* PrintLeafNodeRec */


#endif // DEBUG_REBUILD
