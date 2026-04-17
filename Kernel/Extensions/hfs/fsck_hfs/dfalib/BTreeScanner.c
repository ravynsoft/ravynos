/*
 * Copyright (c) 1996-2002, 2005, 2012 Apple Computer, Inc. All rights reserved.
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
 *
 *	@(#)BTreeScanner.c
 */


#include "BTreeScanner.h"
#include "Scavenger.h"
#include "../cache.h"
#include "../fsck_hfs.h"

static int FindNextLeafNode(	BTScanState *scanState );
static int ReadMultipleNodes( 	BTScanState *scanState );


//_________________________________________________________________________________
//
//	Routine:	BTScanNextRecord
//
//	Purpose:	Return the next leaf record in a scan.
//
//	Inputs:
//		scanState		Scanner's current state
//
//	Outputs:
//		key				Key of found record (points into buffer)
//		data			Data of found record (points into buffer)
//		dataSize		Size of data in found record
//
//	Result:
//		noErr			Found a valid record
//		btNotFound		No more records
//
//	Notes:
//		This routine returns pointers to the found record's key and data.  It
//		does not copy the key or data to a caller-supplied buffer (like
//		GetBTreeRecord would).  The caller must not modify the key or data.
//_________________________________________________________________________________

int BTScanNextRecord(	BTScanState *	scanState,
						void * *		key,
						void * *		data,
						u_int32_t *		dataSize  )
{
	int				err;
	u_int16_t		dataSizeShort;
	int64_t			maxLeafRecs;
	
	err = noErr;

	//
	//	If this is the first call, there won't be any nodes in the buffer, so go
	//	find the first first leaf node (if any).
	//	
	if ( scanState->nodesLeftInBuffer <= 0 )
		err = FindNextLeafNode( scanState );

	// btcb->leafRecords may be fragile (the B-Tree header could be damaged)
	// so in order to do a sanity check on the max number of leaf records we 
	// could have we use the catalog file's physical size divided by the smallest
	// leaf node record size to get our ceiling.
	maxLeafRecs = scanState->btcb->fcbPtr->fcbPhysicalSize / sizeof( HFSCatalogThread );

	while ( err == noErr ) 
	{ 
		//	See if we have a record in the current node
		err = GetRecordByIndex( scanState->btcb, scanState->currentNodePtr, 
								scanState->recordNum, (KeyPtr *) key, 
								(UInt8 **) data, &dataSizeShort  );
		if ( err == noErr )
		{
			++scanState->recordsFound;
			++scanState->recordNum;
			if (dataSize != NULL)
				*dataSize = dataSizeShort;
			return noErr;
		}
		
		//	We're done with the current node.  See if we've returned all the records
		if ( scanState->recordsFound >= maxLeafRecs )
			return btNotFound;

		//	Move to the first record of the next leaf node
		scanState->recordNum = 0; 
		err = FindNextLeafNode( scanState );
	}
	
	//
	//	If we got an EOF error from FindNextLeafNode, then there are no more leaf
	//	records to be found.
	//
	if ( err == fsEndOfIterationErr )
		err = btNotFound;
	
	return err;
	
} /* BTScanNextRecord */


//_________________________________________________________________________________
//
//	Routine:	FindNextLeafNode
//
//	Purpose:	Point to the next leaf node in the buffer.  Read more nodes
//				into the buffer if needed (and allowed).
//
//	Inputs:
//		scanState		Scanner's current state
//
//	Result:
//		noErr			Found a valid record
//		fsEndOfIterationErr	No more nodes in file
//_________________________________________________________________________________

static int FindNextLeafNode(	BTScanState *scanState )
{
	int					err;
    BlockDescriptor		myBlockDescriptor;
	
	err = noErr;		// Assume everything will be OK
	
	while ( 1 ) 
	{
		++scanState->nodeNum;
		--scanState->nodesLeftInBuffer;
		if ( scanState->nodesLeftInBuffer <= 0 ) 
		{
			//	read some more nodes into buffer
			err = ReadMultipleNodes( scanState );
			if ( err != noErr ) 
				break;
		}
		else 
		{
			//	Adjust to point to the next node in the buffer
			
			//	If we've looked at all nodes in the tree, then we're done
			if ( scanState->nodeNum >= scanState->btcb->totalNodes )
				return fsEndOfIterationErr;

			scanState->currentNodePtr = (BTNodeDescriptor *)((UInt8 *)scanState->currentNodePtr + scanState->btcb->nodeSize);
		}
		
        // need to manufacture a BlockDescriptor since hfs_swap_BTNode expects one as input
        myBlockDescriptor.buffer = (void *) scanState->currentNodePtr;
        myBlockDescriptor.blockHeader = NULL;
        myBlockDescriptor.blockNum = scanState->nodeNum;
        myBlockDescriptor.blockSize = scanState->btcb->nodeSize;
        myBlockDescriptor.blockReadFromDisk = false;
        myBlockDescriptor.fragmented = false;
        err = hfs_swap_BTNode(&myBlockDescriptor, scanState->btcb->fcbPtr, kSwapBTNodeBigToHost);
		if ( err != noErr )
		{
			err = noErr;
			continue;
		}
		
		// NOTE - todo - add less lame sanity check to allow leaf nodes that
		// only have damaged kind. 
		if ( scanState->currentNodePtr->kind == kBTLeafNode )
			break;
	}
	
	return err;
	
} /* FindNextLeafNode */


//_________________________________________________________________________________
//
//	Routine:	ReadMultipleNodes
//
//	Purpose:	Read one or more nodes into the buffer.
//
//	Inputs:
//		theScanStatePtr		Scanner's current state
//
//	Result:
//		noErr				One or nodes were read
//		fsEndOfIterationErr		No nodes left in file, none in buffer
//_________________________________________________________________________________

int CacheRawRead (Cache_t *cache, uint64_t off, uint32_t len, void *buf);

static int ReadMultipleNodes( BTScanState *theScanStatePtr )
{
	int						myErr = noErr;
	BTreeControlBlockPtr  	myBTreeCBPtr;
	UInt64					myPhyBlockNum;
	SInt64  				myPhyOffset;
	UInt64					mySectorOffset; // offset within file (in 512-byte sectors)
	UInt32					myContiguousBytes;
	
	myBTreeCBPtr = theScanStatePtr->btcb;
			
	// map logical block in catalog btree file to physical block on volume
	mySectorOffset = 
		(((UInt64)theScanStatePtr->nodeNum * (UInt64)myBTreeCBPtr->fcbPtr->fcbBlockSize) >> kSectorShift);
	myErr = MapFileBlockC( myBTreeCBPtr->fcbPtr->fcbVolume, myBTreeCBPtr->fcbPtr,
						   theScanStatePtr->bufferSize, mySectorOffset, 
						   &myPhyBlockNum, &myContiguousBytes );
	if ( myErr != noErr )
	{
		myErr = fsEndOfIterationErr;
		goto ExitThisRoutine;
	}
	
	// now read blocks from the device 
	myPhyOffset = (SInt64) ( ( (UInt64) myPhyBlockNum ) << kSectorShift );

	// Go through the cache, so we can get any locked-in journal changes
	Buf_t *tmpbuf = NULL;

	myErr = CacheRead( myBTreeCBPtr->fcbPtr->fcbVolume->vcbBlockCache,
			   myPhyOffset, myContiguousBytes, &tmpbuf );

	if ( myErr == noErr )
	{
		if (tmpbuf)
		{
			if (tmpbuf->Length < myContiguousBytes)
				abort();
			memcpy(theScanStatePtr->bufferPtr, tmpbuf->Buffer, myContiguousBytes);
			CacheRelease(myBTreeCBPtr->fcbPtr->fcbVolume->vcbBlockCache, tmpbuf, 0);
		} else
			abort();
#if DEBUG_BTREE
		/*
		 * This code was written to help debug a cache problem, where CacheRead()
		 * was returning different results than CacheRawRead().  I am leaving it
		 * around because I fear that will happen again, so it can be used for
		 * reference, rather than rewrite it then.
		 */
		size_t tempBufferSize = myContiguousBytes;
		int tempError;
		unsigned char *tempBuffer = malloc(myContiguousBytes);
		if (tempBuffer == NULL)
			abort();
		tempError = CacheRawRead( myBTreeCBPtr->fcbPtr->fcbVolume->vcbBlockCache, 
				      myPhyOffset, myContiguousBytes, tempBuffer);
		if (memcmp(tempBuffer, theScanStatePtr->bufferPtr, myContiguousBytes) != 0)
		{
			uint8_t *raw, *cached;
			fprintf(stderr, "CacheRead and CacheRawRead returned different values\n");
			fprintf(stderr, "\tmyPhyOffset = %llu, myContiguousBytes = %u\n", myPhyOffset, myContiguousBytes);
			size_t i = 0;
			for (i = 0; i < myContiguousBytes; i++)
			{
				cached = ((uint8_t*)theScanStatePtr->bufferPtr)[i];
				raw = tempBuffer[i];
				if (cached != raw)
				{
					fprintf(stderr, "\tOffset %zu:  cached value = 0x%02x, raw value = 0x%02x\n", i, cached, raw);
					break;
				}
			}
			extern void dumpCache(void*);
			dumpCache(myBTreeCBPtr->fcbPtr->fcbVolume->vcbBlockCache);
			abort();
		}
		free(tempBuffer);
#endif
	}



	if ( myErr != noErr )
	{
		myErr = fsEndOfIterationErr;
		goto ExitThisRoutine;
	}

	theScanStatePtr->nodesLeftInBuffer = myContiguousBytes /
		theScanStatePtr->btcb->nodeSize;
	theScanStatePtr->currentNodePtr = (BTNodeDescriptor *) theScanStatePtr->bufferPtr;

ExitThisRoutine:
	return myErr;
	
} /* ReadMultipleNodes */


//_________________________________________________________________________________
//
//	Routine:	BTScanInitialize
//
//	Purpose:	Prepare to start a new BTree scan.
//
//	Inputs:
//		btreeFile		The B-Tree's file control block
//
//	Outputs:
//		scanState		Scanner's current state; pass to other scanner calls
//
//_________________________________________________________________________________

int		BTScanInitialize(	const SFCB *	btreeFile,
							BTScanState	*	scanState     )
{
	BTreeControlBlock	*btcb;
	u_int32_t			bufferSize;
	
	//
	//	Make sure this is a valid B-Tree file
	//
	btcb = (BTreeControlBlock *) btreeFile->fcbBtree;
	if (btcb == NULL)
		return R_RdErr;
	
	//
	//	Make sure buffer size is big enough, and a multiple of the
	//	B-Tree node size
	//
	bufferSize = (kCatScanBufferSize / btcb->nodeSize) * btcb->nodeSize;

	//
	//	Set up the scanner's state
	//
	scanState->bufferSize			= bufferSize;
	scanState->bufferPtr = (void *) AllocateMemory( bufferSize );
	if ( scanState->bufferPtr == NULL )
		return( R_NoMem );

	scanState->btcb					= btcb;
	scanState->nodeNum				= 0;
	scanState->recordNum			= 0;
	scanState->currentNodePtr		= NULL;
	scanState->nodesLeftInBuffer	= 0;		// no nodes currently in buffer
	scanState->recordsFound			= 0;
		
	return noErr;
	
} /* BTScanInitialize */


//_________________________________________________________________________________
//
//	Routine:	BTScanTerminate
//
//	Purpose:	Return state information about a scan so that it can be resumed
//				later via BTScanInitialize.
//
//	Inputs:
//		scanState		Scanner's current state
//
//	Outputs:
//		nextNode		Node number to resume a scan (pass to BTScanInitialize)
//		nextRecord		Record number to resume a scan (pass to BTScanInitialize)
//		recordsFound	Valid records seen so far (pass to BTScanInitialize)
//_________________________________________________________________________________

int	 BTScanTerminate(	BTScanState *		scanState	)
{
	if ( scanState->bufferPtr != NULL )
	{
		DisposeMemory( scanState->bufferPtr );
		scanState->bufferPtr = NULL;
		scanState->currentNodePtr = NULL;
	}
	
	return noErr;
	
} /* BTScanTerminate */


