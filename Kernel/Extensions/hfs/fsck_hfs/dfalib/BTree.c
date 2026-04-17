/*
 * Copyright (c) 1999, 2005 Apple Computer, Inc. All rights reserved.
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
	File:		BTree.c

	Contains:	Implementation of public interface routines for B-tree manager.

	Version:	HFS Plus 1.0

	Written by:	Gordon Sheridan and Bill Bruffey

	Copyright:	й 1992-1999 by Apple Computer, Inc., all rights reserved.
*/

extern char debug;

#include "BTree.h"
#include "BTreePrivate.h"
//#include "HFSInstrumentation.h"


extern Boolean NodesAreContiguous(SFCB *fcb, UInt32 nodeSize);
extern void fplog(FILE *stream, const char *fmt, ...);

/*-------------------------------------------------------------------------------
Routine:	CopyKey

Function:	Copy a BTree key.  Sanity check the key length; if it is too large,
			then set the copy's length to the BTree's maximum key length.

Inputs:		btcb		BTree whose key is being copied
			srcKey		Source key being copied

Output:		destKey		Destination where copy will be stored

Result:		none (void)
-------------------------------------------------------------------------------*/
static void CopyKey(BTreeControlBlockPtr btcb, const BTreeKey *srcKey, BTreeKey *destKey)
{
	unsigned	keySize = CalcKeySize(btcb, srcKey);
	unsigned	maxKeySize = MaxKeySize(btcb);
	int			fixLength = 0;
	
	/*
	 *	If the key length is too long (corrupted), then constrain the number
	 *	of bytes to copy.  Remember that we did this so we can also update
	 *	the copy's length field later.
	 */
	if (keySize > maxKeySize)
	{
		keySize = maxKeySize;
		fixLength = 1;
	}
	
	CopyMemory(srcKey, destKey, keySize);
	
	/*
	 *	If we had to constrain the key size above, then also update the
	 *	key length in the copy.  This will prevent the caller from dereferencing
	 *	part of the key which we never actually copied.
	 */
	if (fixLength)
	{
		if (btcb->attributes & kBTBigKeysMask)
			destKey->length16 = btcb->maxKeyLength;
		else
			destKey->length8 = btcb->maxKeyLength;
	}
}


//////////////////////////////////// Globals ////////////////////////////////////


/////////////////////////// BTree Module Entry Points ///////////////////////////


/*-------------------------------------------------------------------------------
Routine:	InitBTreeModule	-	Initialize BTree Module Global(s).

Function:	Initialize BTree code, if necessary

Input:		none

Output:		none

Result:		noErr		- success
			!= noErr	- can't happen
-------------------------------------------------------------------------------*/

OSStatus	InitBTreeModule(void)
{
	return noErr;
}


/*-------------------------------------------------------------------------------
Routine:	BTInitialize	-	Initialize a fork for access as a B*Tree.

Function:	Write Header node and create any map nodes necessary to map the fork
			as a B*Tree. If the fork is not large enough for the header node, the
			FS Agent is called to extend the LEOF. Additional map nodes will be
			allocated if necessary to represent the size of the fork. This allows
			the FS Agent to specify the initial size of B*Tree files.


Input:		pathPtr			- pointer to path control block
			maxKeyLength	- maximum length that will be used for any key in this B*Tree
			nodeSize		- node size for B*Tree (must be 2^n, 9 >= n >= 15)
			btreeType		- type of B*Tree
			keyDescPtr		- pointer to key descriptor (optional if key compare proc is used)

Output:		none

Result:		noErr		- success
			paramErr		- mandatory parameter was missing
			E_NoGetBlockProc		- FS Agent CB has no GetNodeProcPtr
			E_NoReleaseBlockProc	- FS Agent CB has no ReleaseNodeProcPtr
			E_NoSetEndOfForkProc	- FS Agent CB has no SetEndOfForkProcPtr
			E_NoSetBlockSizeProc	- FS Agent CB has no SetBlockSizeProcPtr
			fsBTrFileAlreadyOpenErr	- fork is already open as a B*Tree
			fsBTInvalidKeyLengthErr		- maximum key length is out of range
			E_BadNodeType			- node size is an illegal value
			fsBTUnknownVersionErr	- the B*Tree type is unknown by this module
			memFullErr			- not enough memory to initialize B*Tree
			!= noErr	- failure
-------------------------------------------------------------------------------*/
#if 0
OSStatus	BTInitialize		(FCB					*filePtr,
								 UInt16					 maxKeyLength,
								 UInt16					 nodeSize,
								 UInt8					 btreeType,
								 KeyDescriptorPtr		 keyDescPtr )
{
	OSStatus				err;
	FSForkControlBlockPtr	 	forkPtr;
	BTreeControlBlockPtr	btreePtr;
	BlockDescriptor			headerNode;
	HeaderPtr				header;
	Ptr						pos;
	FSSize					minEOF, maxEOF;
	SetEndOfForkProcPtr		setEndOfForkProc;
	SetBlockSizeProcPtr		setBlockSizeProc;

	////////////////////// Preliminary Error Checking ///////////////////////////

	headerNode.buffer	= nil;

	if (pathPtr										== nil)	return  paramErr;

	setEndOfForkProc	= pathPtr->agentPtr->agent.setEndOfForkProc;
	setBlockSizeProc	= pathPtr->agentPtr->agent.setBlockSizeProc;

	if (pathPtr->agentPtr->agent.getBlockProc		== nil)	return  E_NoGetBlockProc;
	if (pathPtr->agentPtr->agent.releaseBlockProc	== nil)	return  E_NoReleaseBlockProc;
	if (setEndOfForkProc							== nil)	return  E_NoSetEndOfForkProc;
	if (setBlockSizeProc							== nil)	return  E_NoSetBlockSizeProc;

	forkPtr = pathPtr->path.forkPtr;

	if (forkPtr->fork.btreePtr != nil)						return	fsBTrFileAlreadyOpenErr;

	if ((maxKeyLength == 0) ||
		(maxKeyLength >  kMaxKeyLength))					return	fsBTInvalidKeyLengthErr;

	if ( M_IsEven (maxKeyLength))	++maxKeyLength;			// len byte + even bytes + pad byte

	switch (nodeSize)										// node size == 512*2^n
	{
		case   512:
		case  1024:
		case  2048:
		case  4096:
		case  8192:
		case 16384:
		case 32768:		break;
		default:		return	E_BadNodeType;
	}

	switch (btreeType)
	{
		case kHFSBTreeType:
		case kUserBTreeType:
		case kReservedBTreeType:	break;

		default:					return	fsBTUnknownVersionErr;		//ее right?
	}


	//////////////////////// Allocate Control Block /////////////////////////////

	M_RESIDENT_ALLOCATE_FIXED_CLEAR( &btreePtr, sizeof( BTreeControlBlock ), kFSBTreeControlBlockType );
	if (btreePtr == nil)
	{
		err = memFullErr;
		goto ErrorExit;
	}

	btreePtr->version			= kBTreeVersion;			//ее what is the version?
	btreePtr->reserved1			= 0;
	btreePtr->flags				= 0;
	btreePtr->attributes		= 0;
	btreePtr->forkPtr			= forkPtr;
	btreePtr->keyCompareProc	= nil;
	btreePtr->keyDescPtr		= keyDescPtr;
	btreePtr->btreeType			= btreeType;
	btreePtr->treeDepth			= 0;
	btreePtr->rootNode			= 0;
	btreePtr->leafRecords		= 0;
	btreePtr->firstLeafNode		= 0;
	btreePtr->lastLeafNode		= 0;
	btreePtr->nodeSize			= nodeSize;
	btreePtr->maxKeyLength		= maxKeyLength;
	btreePtr->totalNodes		= 1;			// so ExtendBTree adds maps nodes properly
	btreePtr->freeNodes			= 0;
	btreePtr->writeCount		= 1;			//	<CS10>, for BTree scanner

	// set block size = nodeSize
	err = setBlockSizeProc (forkPtr, nodeSize);
	M_ExitOnError (err);

	////////////////////////////// Check LEOF ///////////////////////////////////

	minEOF = nodeSize;
	if ( forkPtr->fork.logicalEOF < minEOF )
	{
		// allocate more space if necessary
		maxEOF 0xFFFFFFFFL;

		err = setEndOfForkProc (forkPtr, minEOF, maxEOF);
		M_ExitOnError (err);
	};


	//////////////////////// Initialize Header Node /////////////////////////////

	err = GetNewNode (btreePtr, kHeaderNodeNum, &headerNode);
	M_ExitOnError (err);

	header = headerNode.buffer;

	header->node.type			= kHeaderNode;
	header->node.numRecords		= 3;			// header rec, key desc, map rec

	header->nodeSize			= nodeSize;
	header->maxKeyLength		= maxKeyLength;
	header->btreeType			= btreeType;
	header->totalNodes			= btreePtr->totalNodes;
	header->freeNodes			= btreePtr->totalNodes - 1;
	// ignore					  header->clumpSize;	//ее rename this field?

	// mark header node allocated in map record
	pos		= ((Ptr)headerNode.buffer) + kHeaderMapRecOffset;
	*pos	= 0x80;

	// set node offsets ( nodeSize-8, F8, 78, 0E)
	pos	 = ((Ptr)headerNode.buffer) + nodeSize;
	pos	-= 2;		*((UInt16 *)pos) = kHeaderRecOffset;
	pos	-= 2;		*((UInt16 *)pos) = kKeyDescRecOffset;
	pos -= 2;		*((UInt16 *)pos) = kHeaderMapRecOffset;
	pos -= 2;		*((UInt16 *)pos) = nodeSize - 8;


	///////////////////// Copy Key Descriptor To Header /////////////////////////
#if SupportsKeyDescriptors
	if (keyDescPtr != nil)
	{
		err = CheckKeyDescriptor (keyDescPtr, maxKeyLength);
		M_ExitOnError (err);

		// copy to header node
		pos = ((Ptr)headerNode.buffer) + kKeyDescRecOffset;
		CopyMemory (keyDescPtr, pos, keyDescPtr->length + 1);
	}
#endif

	// write header node
	err = UpdateNode (btreePtr, &headerNode);
	M_ExitOnError (err);


	////////////////////////// Allocate Map Nodes ///////////////////////////////

	err = ExtendBTree (btreePtr, forkPtr->fork.logicalEOF.lo / nodeSize);	// sets totalNodes
	M_ExitOnError (err);


	////////////////////////////// Close BTree //////////////////////////////////

	err = UpdateHeader (btreePtr);
	M_ExitOnError (err);

	pathPtr->path.forkPtr->fork.btreePtr = nil;
	M_RESIDENT_DEALLOCATE_FIXED( btreePtr, sizeof( BTreeControlBlock ), kFSBTreeControlBlockType );

	return	noErr;


	/////////////////////// Error - Clean up and Exit ///////////////////////////

ErrorExit:

	(void) ReleaseNode (btreePtr, &headerNode);
	if (btreePtr != nil)
		M_RESIDENT_DEALLOCATE_FIXED( btreePtr, sizeof( BTreeControlBlock ), kFSBTreeControlBlockType );

	return	err;
}
#endif


/*-------------------------------------------------------------------------------
Routine:	BTOpenPath	-	Open a file for access as a B*Tree.

Function:	Create BTree control block for a file, if necessary. Validates the
			file to be sure it looks like a BTree file.


Input:		filePtr				- pointer to file to open as a B-tree
			keyCompareProc		- pointer to client's KeyCompare function
			getBlockProc		- pointer to client's GetBlock function
			releaseBlockProc	- pointer to client's ReleaseBlock function
			setEndOfForkProc	- pointer to client's SetEOF function

Result:		noErr				- success
			paramErr			- required ptr was nil
			fsBTInvalidFileErr				-
			memFullErr			-
			!= noErr			- failure
-------------------------------------------------------------------------------*/

OSStatus	BTOpenPath			(SFCB					*filePtr,
								 KeyCompareProcPtr		 keyCompareProc,
								 GetBlockProcPtr		 getBlockProc,
								 ReleaseBlockProcPtr	 releaseBlockProc,
								 SetEndOfForkProcPtr	 setEndOfForkProc,
								 SetBlockSizeProcPtr	 setBlockSizeProc )
{
	OSStatus				err;
	BTreeControlBlockPtr	btreePtr;
	BTHeaderRec				*header;
	NodeRec					nodeRec;

//	LogStartTime(kTraceOpenBTree);

	////////////////////// Preliminary Error Checking ///////////////////////////

	if ( filePtr == nil				||
		 getBlockProc == nil		||
		 releaseBlockProc == nil	||
		 setEndOfForkProc == nil	||
		 setBlockSizeProc == nil )
	{
		return  paramErr;
	}

	if ( filePtr->fcbBtree != nil )			// already has a BTreeCB
		return noErr;

												// is file large enough to contain header node?
	if ( filePtr->fcbLogicalSize < kMinNodeSize )
		return fsBTInvalidFileErr;							//ее or E_BadHeader?


	//////////////////////// Allocate Control Block /////////////////////////////

	btreePtr = (BTreeControlBlock*) AllocateClearMemory( sizeof( BTreeControlBlock ) );
	if (btreePtr == nil)
	{
		Panic ("\pBTOpen: no memory for btreePtr.");
		return	memFullErr;
	}

	btreePtr->getBlockProc		= getBlockProc;
	btreePtr->releaseBlockProc	= releaseBlockProc;
	btreePtr->setEndOfForkProc	= setEndOfForkProc;
	btreePtr->keyCompareProc	= keyCompareProc;

	/////////////////////////// Read Header Node ////////////////////////////////

	nodeRec.buffer				= nil;				// so we can call ReleaseNode
	
	btreePtr->fcbPtr			= filePtr;
	filePtr->fcbBtree			= (void *) btreePtr;	// attach btree cb to file

	// it is now safe to call M_ExitOnError (err)

	err = setBlockSizeProc (btreePtr->fcbPtr, kMinNodeSize);
	M_ExitOnError (err);


	err = getBlockProc (btreePtr->fcbPtr,
						kHeaderNodeNum,
						kGetBlock,
						&nodeRec );

	PanicIf (err != noErr, "\pBTOpen: getNodeProc returned error getting header node.");
	M_ExitOnError (err);

	header = (BTHeaderRec*) (nodeRec.buffer + sizeof(BTNodeDescriptor));


	///////////////////////////// verify header /////////////////////////////////

	err = VerifyHeader (filePtr, header);
	M_ExitOnError (err);


	///////////////////// Initalize fields from header //////////////////////////
	
	PanicIf ( (filePtr->fcbVolume->vcbSignature != 0x4244) && (btreePtr->nodeSize == 512), "\p BTOpenPath: wrong node size for HFS+ volume!");

	btreePtr->treeDepth			= header->treeDepth;
	btreePtr->rootNode			= header->rootNode;
	btreePtr->leafRecords		= header->leafRecords;
	btreePtr->firstLeafNode		= header->firstLeafNode;
	btreePtr->lastLeafNode		= header->lastLeafNode;
	btreePtr->nodeSize			= header->nodeSize;
	btreePtr->maxKeyLength		= header->maxKeyLength;
	btreePtr->totalNodes		= header->totalNodes;
	btreePtr->freeNodes			= header->freeNodes;
	// ignore					  header->clumpSize;	//ее rename this field?
	btreePtr->btreeType			= header->btreeType;

	btreePtr->attributes		= header->attributes;

	if ( btreePtr->maxKeyLength > 40 )
		btreePtr->attributes |= (kBTBigKeysMask + kBTVariableIndexKeysMask);	//ее we need a way to save these attributes

	/////////////////////// Initialize dynamic fields ///////////////////////////

	btreePtr->version			= kBTreeVersion;
	btreePtr->flags				= 0;
	btreePtr->writeCount		= 1;		//	<CS10>, for BTree scanner

	btreePtr->numGetNodes		= 1;		// for earlier call to getNodeProc

	/////////////////////////// Check Header Node ///////////////////////////////

	//ее set kBadClose attribute bit, and UpdateNode

	/*
	 * If the actual node size is different than the amount we read,
	 * then release and trash this block, and re-read with the correct
	 * node size.
	 */
	if ( btreePtr->nodeSize != kMinNodeSize )
	{
		err = setBlockSizeProc (btreePtr->fcbPtr, btreePtr->nodeSize);
		M_ExitOnError (err);

#if BSD
		/*
		 * Need to use kTrashBlock option to force the
		 * buffer cache to re-read the entire node
		 */
		err = releaseBlockProc(btreePtr->fcbPtr, &nodeRec, kTrashBlock);
#else
		err = ReleaseNode (btreePtr, &nodeRec);
#endif
	
		err = GetNode (btreePtr, kHeaderNodeNum, &nodeRec );		// calls CheckNode...
		M_ExitOnError (err);
	}

	//ее total nodes * node size <= LEOF?


	////////////////////////// Get Key Descriptor ///////////////////////////////
#if SupportsKeyDescriptors
	if ( keyCompareProc == nil )		// 	if no key compare proc then get key descriptor
	{
		err = GetKeyDescriptor (btreePtr, nodeRec.buffer);	//ее it should check amount of memory allocated...
		M_ExitOnError (err);

		err = CheckKeyDescriptor (btreePtr->keyDescPtr, btreePtr->maxKeyLength);
		M_ExitOnError (err);

	}
	else
#endif
	{
		btreePtr->keyDescPtr = nil;			// clear it so we don't dispose garbage later
	}

	err = ReleaseNode (btreePtr, &nodeRec);
	M_ExitOnError (err);


#if BSD
	/*
	 * Under Mac OS, b-tree nodes can be non-contiguous on disk when the
	 * allocation block size is smaller than the b-tree node size.
	 */
	if ( !NodesAreContiguous(filePtr, btreePtr->nodeSize) ) {
		if (debug) fplog(stderr, "Nodes are not contiguous -- this is fatal\n");
		return fsBTInvalidNodeErr;
	}
#endif

	//////////////////////////////// Success ////////////////////////////////////

	//ее align LEOF to multiple of node size?	- just on close

//	LogEndTime(kTraceOpenBTree, noErr);

	return noErr;


	/////////////////////// Error - Clean up and Exit ///////////////////////////

ErrorExit:

	filePtr->fcbBtree = nil;
	(void) ReleaseNode (btreePtr, &nodeRec);
	DisposeMemory( btreePtr );

//	LogEndTime(kTraceOpenBTree, err);

	return err;
}



/*-------------------------------------------------------------------------------
Routine:	BTClosePath	-	Flush BTree Header and Deallocate Memory for BTree.

Function:	Flush the BTreeControlBlock fields to header node, and delete BTree control
			block and key descriptor associated with the file if filePtr is last
			path of type kBTreeType ('btre').


Input:		filePtr		- pointer to file to delete BTree control block for.

Result:		noErr			- success
			fsBTInvalidFileErr	-
			!= noErr		- failure
-------------------------------------------------------------------------------*/

OSStatus	BTClosePath			(SFCB					*filePtr)
{
	OSStatus				err;
	BTreeControlBlockPtr	btreePtr;
#if 0
	FSPathControlBlockPtr	tempPath;
	Boolean					otherBTreePathsOpen;
#endif

//	LogStartTime(kTraceCloseBTree);

	btreePtr = (BTreeControlBlockPtr) filePtr->fcbBtree;

	if (btreePtr == nil)
		return fsBTInvalidFileErr;

	////////////////////// Check for other BTree Paths //////////////////////////

#if 0
//ее Need replacement field for pathType 
	otherBTreePathsOpen = false;
	tempPath = forkPtr->fork.pathList.head;
	while ( (tempPath != (FSPathControlBlockPtr) &forkPtr->fork.pathList) &&
			(otherBTreePathsOpen == false) )
	{
		if ((tempPath != pathPtr) && (tempPath->path.pathType == kBTreeType))
		{
			otherBTreePathsOpen = true;
			break;							// done with loop check
		}

		tempPath = tempPath->next;
	}

	////////////////////////// Update Header Node ///////////////////////////////


	if (otherBTreePathsOpen == true)
	{
		err = UpdateHeader (btreePtr);			// update header even if we aren't closing
		return err;								// we only clean up after the last user...
	}
#endif

	btreePtr->attributes &= ~kBTBadCloseMask;		// clear "bad close" attribute bit
	err = UpdateHeader (btreePtr);
	M_ExitOnError (err);

#if SupportsKeyDescriptors
	if (btreePtr->keyDescPtr != nil)			// deallocate keyDescriptor, if any
	{
		DisposeMemory( btreePtr->keyDescPtr );
	}
#endif

	DisposeMemory( btreePtr );
	filePtr->fcbBtree = nil;

//	LogEndTime(kTraceCloseBTree, noErr);

	return	noErr;

	/////////////////////// Error - Clean Up and Exit ///////////////////////////

ErrorExit:

//	LogEndTime(kTraceCloseBTree, err);

	return	err;
}



/*-------------------------------------------------------------------------------
Routine:	BTSearchRecord	-	Search BTree for a record with a matching key.

Function:	Search for position in B*Tree indicated by searchKey. If a valid node hint
			is provided, it will be searched first, then SearchTree will be called.
			If a BTreeIterator is provided, it will be set to the position found as
			a result of the search. If a record exists at that position, and a BufferDescriptor
			is supplied, the record will be copied to the buffer (as much as will fit),
			and recordLen will be set to the length of the record.

			If an error other than fsBTRecordNotFoundErr occurs, the BTreeIterator, if any,
			is invalidated, and recordLen is set to 0.


Input:		pathPtr			- pointer to path for BTree file.
			searchKey		- pointer to search key to match.
			hintPtr			- pointer to hint (may be nil)

Output:		record			- pointer to BufferDescriptor containing record
			recordLen		- length of data at recordPtr
			iterator		- pointer to BTreeIterator indicating position result of search

Result:		noErr			- success, record contains copy of record found
			fsBTRecordNotFoundErr	- record was not found, no data copied
			fsBTInvalidFileErr	- no BTreeControlBlock is allocated for the fork
			fsBTInvalidKeyLengthErr		-
			!= noErr		- failure
-------------------------------------------------------------------------------*/

OSStatus	BTSearchRecord		(SFCB						*filePtr,
								 BTreeIterator				*searchIterator,
								 UInt32						heuristicHint,
								 FSBufferDescriptor			*record,
								 UInt16						*recordLen,
								 BTreeIterator				*resultIterator )
{
	OSStatus				err;
	BTreeControlBlockPtr	btreePtr;
	TreePathTable			treePathTable;
	UInt32					nodeNum = 0;
	BlockDescriptor			node;
	UInt16					index;
	BTreeKeyPtr				keyPtr;
	RecordPtr				recordPtr;
	UInt16					len;
	Boolean					foundRecord;
	Boolean					validHint;


//	LogStartTime(kTraceSearchBTree);

	if (filePtr == nil)									return	paramErr;
	if (searchIterator == nil)							return	paramErr;

	btreePtr = (BTreeControlBlockPtr) filePtr->fcbBtree;
	if (btreePtr == nil)								return	fsBTInvalidFileErr;

#if SupportsKeyDescriptors
	if (btreePtr->keyCompareProc == nil)		// CheckKey if we using Key Descriptor
	{
		err = CheckKey (&searchIterator->key, btreePtr->keyDescPtr, btreePtr->maxKeyLength);
		M_ExitOnError (err);
	}
#endif

	foundRecord = false;

	////////////////////////////// Take A Hint //////////////////////////////////

	err = IsItAHint (btreePtr, searchIterator, &validHint);
	M_ExitOnError (err);

	if (validHint)
	{
		nodeNum = searchIterator->hint.nodeNum;
		
		err = GetNode (btreePtr, nodeNum, &node);
		if( err == noErr )
		{
			if ( ((BTNodeDescriptor*) node.buffer)->kind		== kBTLeafNode &&
				 ((BTNodeDescriptor*) node.buffer)->numRecords	>  0 )
			{
				foundRecord = SearchNode (btreePtr, node.buffer, &searchIterator->key, &index);

				//ее if !foundRecord, we could still skip tree search if ( 0 < index < numRecords )
			}

			if (foundRecord == false)
			{
				err = ReleaseNode (btreePtr, &node);
				M_ExitOnError (err);
			}
			else
			{
				++btreePtr->numValidHints;
			}
		}
		
		if( foundRecord == false )
			(void) BTInvalidateHint( searchIterator );
	}

	////////////////////////////// Try the heuristicHint //////////////////////////////////

	if ( (foundRecord == false) && (heuristicHint != kInvalidMRUCacheKey) && (nodeNum != heuristicHint) )
	{
	//	LogStartTime(kHeuristicHint);
		nodeNum = heuristicHint;
		
		err = GetNode (btreePtr, nodeNum, &node);
		if( err == noErr )
		{
			if ( ((BTNodeDescriptor*) node.buffer)->kind		== kBTLeafNode &&
				 ((BTNodeDescriptor*) node.buffer)->numRecords	>  0 )
			{
				foundRecord = SearchNode (btreePtr, node.buffer, &searchIterator->key, &index);
			}

			if (foundRecord == false)
			{
				err = ReleaseNode (btreePtr, &node);
				M_ExitOnError (err);
			}
		}
	//	LogEndTime(kHeuristicHint, (foundRecord == false));
	}

	//////////////////////////// Search The Tree ////////////////////////////////

	if (foundRecord == false)
	{
		err = SearchTree ( btreePtr, &searchIterator->key, treePathTable, &nodeNum, &node, &index);
		switch (err)
		{
			case noErr:			foundRecord = true;				break;
			case fsBTRecordNotFoundErr:									break;
			default:				goto ErrorExit;
		}
	}


	//////////////////////////// Get the Record /////////////////////////////////

	if (foundRecord == true)
	{
		//ее Should check for errors! Or BlockMove could choke on recordPtr!!!
		GetRecordByIndex (btreePtr, node.buffer, index, &keyPtr, &recordPtr, &len);

		if (recordLen != nil)			*recordLen = len;

		if (record != nil)
		{
			ByteCount recordSize;

			recordSize = record->itemCount * record->itemSize;
			
			PanicIf(len > recordSize, "\pBTSearchRecord: truncating record!");

			if (len > recordSize)	len = recordSize;

			CopyMemory (recordPtr, record->bufferAddress, len);
		}
	}


	/////////////////////// Success - Update Iterator ///////////////////////////

	if (resultIterator != nil)
	{
		resultIterator->hint.writeCount	= btreePtr->writeCount;
		resultIterator->hint.nodeNum	= nodeNum;
		resultIterator->hint.index		= index;
		resultIterator->hint.reserved1	= 0;
		resultIterator->hint.reserved2	= 0;

		resultIterator->version			= 0;
		resultIterator->reserved		= 0;

		// copy the key in the BTree when found rather than searchIterator->key to get proper case/diacriticals
		if (foundRecord == true)
			CopyKey(btreePtr, keyPtr, &resultIterator->key);
		else
			CopyKey(btreePtr, &searchIterator->key, &resultIterator->key);
	}

	err = ReleaseNode (btreePtr, &node);
	M_ExitOnError (err);

//	LogEndTime(kTraceSearchBTree, (foundRecord == false));

	if (foundRecord == false)	return	fsBTRecordNotFoundErr;
	else						return	noErr;


	/////////////////////// Error - Clean Up and Exit ///////////////////////////

ErrorExit:

	if (recordLen != nil)
		*recordLen = 0;

	if (resultIterator != nil)
	{
		resultIterator->hint.writeCount	= 0;
		resultIterator->hint.nodeNum	= 0;
		resultIterator->hint.index		= 0;
		resultIterator->hint.reserved1	= 0;
		resultIterator->hint.reserved2	= 0;

		resultIterator->version			= 0;
		resultIterator->reserved		= 0;
		resultIterator->key.length16	= 0;	// zero out two bytes to cover both types of keys
	}

	if ( err == fsBTEmptyErr )
		err = fsBTRecordNotFoundErr;

//	LogEndTime(kTraceSearchBTree, err);

	return err;
}



/*-------------------------------------------------------------------------------
Routine:	BTIterateRecord	-	Find the first, next, previous, or last record.

Function:	Find the first, next, previous, or last record in the BTree

Input:		pathPtr			- pointer to path iterate records for.
			operation		- iteration operation (first,next,prev,last)
			iterator		- pointer to iterator indicating start position

Output:		iterator		- iterator is updated to indicate new position
			newKeyPtr		- pointer to buffer to copy key found by iteration
			record			- pointer to buffer to copy record found by iteration
			recordLen		- length of record

Result:		noErr			- success
			!= noErr		- failure
-------------------------------------------------------------------------------*/

OSStatus	BTIterateRecord		(SFCB						*filePtr,
								 BTreeIterationOperation	 operation,
								 BTreeIterator				*iterator,
								 FSBufferDescriptor			*record,
								 UInt16						*recordLen )
{
	OSStatus					err;
	BTreeControlBlockPtr		btreePtr;
	BTreeKeyPtr					keyPtr;
	RecordPtr					recordPtr;
	UInt16						len;

	Boolean						foundRecord;
	UInt32						nodeNum;

	BlockDescriptor				left,		node,		right;
	UInt16						index;


//	LogStartTime(kTraceGetBTreeRecord);

	////////////////////////// Priliminary Checks ///////////////////////////////

	left.buffer		= nil;
	right.buffer	= nil;
	node.buffer		= nil;


	if (filePtr == nil)
	{
		return	paramErr;
	}

	btreePtr = (BTreeControlBlockPtr) filePtr->fcbBtree;
	if (btreePtr == nil)
	{
		return	fsBTInvalidFileErr;			//ее handle properly
	}

	if ((operation != kBTreeFirstRecord)	&&
		(operation != kBTreeNextRecord)		&&
		(operation != kBTreeCurrentRecord)	&&
		(operation != kBTreePrevRecord)		&&
		(operation != kBTreeLastRecord))
	{
		err = fsInvalidIterationMovmentErr;
		goto ErrorExit;
	}

	/////////////////////// Find First or Last Record ///////////////////////////

	if ((operation == kBTreeFirstRecord) || (operation == kBTreeLastRecord))
	{
		if (operation == kBTreeFirstRecord)		nodeNum = btreePtr->firstLeafNode;
		else									nodeNum = btreePtr->lastLeafNode;

		if (nodeNum == 0)
		{
			err = fsBTEmptyErr;
			goto ErrorExit;
		}

		err = GetNode (btreePtr, nodeNum, &node);
		M_ExitOnError (err);

		if ( ((NodeDescPtr) node.buffer)->kind			!= kBTLeafNode ||
			 ((NodeDescPtr) node.buffer)->numRecords	<=  0 )
		{
			err = ReleaseNode (btreePtr, &node);
			M_ExitOnError (err);

			if (debug) fprintf(stderr, "%s(%d): returning fsBTInvalidNodeErr\n", __FUNCTION__, __LINE__);
			err = fsBTInvalidNodeErr;
			goto ErrorExit;
		}

		if (operation == kBTreeFirstRecord)		index = 0;
		else									index = ((BTNodeDescriptor*) node.buffer)->numRecords - 1;

		goto CopyData;						//ее is there a cleaner way?
	}


	//////////////////////// Find Iterator Position /////////////////////////////

	err = FindIteratorPosition (btreePtr, iterator,
								&left, &node, &right, &nodeNum, &index, &foundRecord);
	M_ExitOnError (err);


	///////////////////// Find Next Or Previous Record //////////////////////////

	if (operation == kBTreePrevRecord)
	{
		if (index > 0)
		{
			--index;
		}
		else
		{
			if (left.buffer == nil)
			{
				nodeNum = ((NodeDescPtr) node.buffer)->bLink;
				if ( nodeNum > 0)
				{
					err = GetNode (btreePtr, nodeNum, &left);
					M_ExitOnError (err);
				} else {
					err = fsBTStartOfIterationErr;
					goto ErrorExit;
				}
			}
			//	Before we stomp on "right", we'd better release it if needed
			if (right.buffer != nil) {
				err = ReleaseNode(btreePtr, &right);
				M_ExitOnError(err);
			}
			right		= node;
			node		= left;
			left.buffer	= nil;
			index 		= ((NodeDescPtr) node.buffer)->numRecords -1;
		}
	}
	else if (operation == kBTreeNextRecord)
	{
		if ((foundRecord != true) &&
			(((NodeDescPtr) node.buffer)->fLink == 0) &&
			(index == ((NodeDescPtr) node.buffer)->numRecords))
		{
			err = fsBTEndOfIterationErr;
			goto ErrorExit;
		} 
	
		// we did not find the record but the index is already positioned correctly
		if ((foundRecord == false) && (index != ((NodeDescPtr) node.buffer)->numRecords)) 
			goto CopyData;

		// we found the record OR we have to look in the next node
		if (index < ((NodeDescPtr) node.buffer)->numRecords -1)
		{
			++index;
		}
		else
		{
			if (right.buffer == nil)
			{
				nodeNum = ((NodeDescPtr) node.buffer)->fLink;
				if ( nodeNum > 0)
				{
					err = GetNode (btreePtr, nodeNum, &right);
					M_ExitOnError (err);
				} else {
					err = fsBTEndOfIterationErr;
					goto ErrorExit;
				}
			}
			//	Before we stomp on "left", we'd better release it if needed
			if (left.buffer != nil) {
				err = ReleaseNode(btreePtr, &left);
				M_ExitOnError(err);
			}
			left		 = node;
			node		 = right;
			right.buffer = nil;
			index		 = 0;
		}
	}
	else // operation == kBTreeCurrentRecord
	{
		// make sure we have something... <CS9>
		if ((foundRecord != true) &&
			(index >= ((NodeDescPtr) node.buffer)->numRecords))
		{
			err = fsBTEndOfIterationErr;
			goto ErrorExit;
		} 
	}

	//////////////////// Copy Record And Update Iterator ////////////////////////

CopyData:

	// added check for errors <CS9>
	err = GetRecordByIndex (btreePtr, node.buffer, index, &keyPtr, &recordPtr, &len);
	M_ExitOnError (err);

	if (recordLen != nil)			*recordLen = len;

	if (record != nil)
	{
		ByteCount recordSize;

		recordSize = record->itemCount * record->itemSize;

		PanicIf(len > recordSize, "\pBTIterateRecord: truncating record!");
	
		if (len > recordSize)	len = recordSize;

		CopyMemory (recordPtr, record->bufferAddress, len);
	}

	if (iterator != nil)						// first & last do not require iterator
	{
		iterator->hint.writeCount	= btreePtr->writeCount;
		iterator->hint.nodeNum		= nodeNum;
		iterator->hint.index		= index;
		iterator->hint.reserved1	= 0;
		iterator->hint.reserved2	= 0;

		iterator->version			= 0;
		iterator->reserved			= 0;

		CopyKey(btreePtr, keyPtr, &iterator->key);
	}


	///////////////////////////// Release Nodes /////////////////////////////////

	err = ReleaseNode (btreePtr, &node);
	M_ExitOnError (err);

	if (left.buffer != nil)
	{
		err = ReleaseNode (btreePtr, &left);
		M_ExitOnError (err);
	}

	if (right.buffer != nil)
	{
		err = ReleaseNode (btreePtr, &right);
		M_ExitOnError (err);
	}

//	LogEndTime(kTraceGetBTreeRecord, noErr);

	return noErr;

	/////////////////////// Error - Clean Up and Exit ///////////////////////////

ErrorExit:

	(void)	ReleaseNode (btreePtr, &left);
	(void)	ReleaseNode (btreePtr, &node);
	(void)	ReleaseNode (btreePtr, &right);

	if (recordLen != nil)
		*recordLen = 0;

	if (iterator != nil)
	{
		iterator->hint.writeCount	= 0;
		iterator->hint.nodeNum		= 0;
		iterator->hint.index		= 0;
		iterator->hint.reserved1	= 0;
		iterator->hint.reserved2	= 0;

		iterator->version			= 0;
		iterator->reserved			= 0;
		iterator->key.length16		= 0;
	}

	if ( err == fsBTEmptyErr || err == fsBTEndOfIterationErr )
		err = fsBTRecordNotFoundErr;

//	LogEndTime(kTraceGetBTreeRecord, err);

	return err;
}


//////////////////////////////// BTInsertRecord /////////////////////////////////

OSStatus	BTInsertRecord		(SFCB						*filePtr,
								 BTreeIterator				*iterator,
								 FSBufferDescriptor			*record,
								 UInt16						 recordLen )
{
	OSStatus				err;
	BTreeControlBlockPtr	btreePtr;
	TreePathTable			treePathTable;
	SInt32					nodesNeeded;
	BlockDescriptor			nodeRec;
	UInt32					insertNodeNum;
	UInt16					index;
	Boolean					recordFit;


	////////////////////////// Priliminary Checks ///////////////////////////////

	nodeRec.buffer = nil;					// so we can call ReleaseNode

	err = CheckInsertParams (filePtr, iterator, record, recordLen);
	if (err != noErr)
		return	err;

//	LogStartTime(kTraceInsertBTreeRecord);

	btreePtr = (BTreeControlBlockPtr) filePtr->fcbBtree;

	///////////////////////// Find Insert Position //////////////////////////////

	// always call SearchTree for Insert
	err = SearchTree (btreePtr, &iterator->key, treePathTable, &insertNodeNum, &nodeRec, &index);

	switch (err)				// set/replace/insert decision point
	{
		case noErr:			err = fsBTDuplicateRecordErr;
							goto ErrorExit;

		case fsBTRecordNotFoundErr:	break;

		case fsBTEmptyErr:	// if tree empty add 1st leaf node

			if (btreePtr->freeNodes == 0)
			{
				err = ExtendBTree (btreePtr, btreePtr->totalNodes + 1);
				M_ExitOnError (err);
			}

			err = AllocateNode (btreePtr, &insertNodeNum);
			M_ExitOnError (err);

			err = GetNewNode (btreePtr, insertNodeNum, &nodeRec);
			M_ExitOnError (err);

			((NodeDescPtr)nodeRec.buffer)->kind		= kBTLeafNode;
			((NodeDescPtr)nodeRec.buffer)->height	= 1;

			recordFit = InsertKeyRecord (btreePtr, nodeRec.buffer, 0,
										 &iterator->key, KeyLength(btreePtr, &iterator->key),
										 record->bufferAddress, recordLen );
			if (recordFit != true)
			{
				err = fsBTRecordTooLargeErr;
				goto ErrorExit;
			}

			err = UpdateNode (btreePtr, &nodeRec);
			M_ExitOnError (err);

			// update BTreeControlBlock
			btreePtr->treeDepth	 		= 1;
			btreePtr->rootNode	 		= insertNodeNum;
			btreePtr->firstLeafNode		= insertNodeNum;
			btreePtr->lastLeafNode		= insertNodeNum;
			M_BTreeHeaderDirty (btreePtr);

			goto Success;

		default:				
			goto ErrorExit;
	}

	if (index > 0) 
	{
		recordFit = InsertKeyRecord (btreePtr, nodeRec.buffer, index,
										&iterator->key, KeyLength(btreePtr, &iterator->key),
										record->bufferAddress, recordLen);
		if (recordFit == true)
		{
			err = UpdateNode (btreePtr, &nodeRec);
			M_ExitOnError (err);

			goto Success;
		}
	}

	/////////////////////// Extend File If Necessary ////////////////////////////

	nodesNeeded =  btreePtr->treeDepth + 1 - btreePtr->freeNodes;	//ее math limit
	if (nodesNeeded > 0)
	{
		nodesNeeded += btreePtr->totalNodes;
		if (nodesNeeded > CalcMapBits (btreePtr))	// we'll need to add a map node too!
			++nodesNeeded;

		err = ExtendBTree (btreePtr, nodesNeeded);
		M_ExitOnError (err);
	}

	// no need to delete existing record

	err = InsertTree (btreePtr, treePathTable, &iterator->key, record->bufferAddress,
					  recordLen, &nodeRec, index, 1, kInsertRecord, &insertNodeNum);
	M_ExitOnError (err);


	//////////////////////////////// Success ////////////////////////////////////

Success:
	++btreePtr->writeCount;				//	<CS10>
	++btreePtr->leafRecords;
	M_BTreeHeaderDirty (btreePtr);

	// create hint
	iterator->hint.writeCount 	= btreePtr->writeCount;		//	unused until <CS10>
	iterator->hint.nodeNum		= insertNodeNum;
	iterator->hint.index		= 0;						// unused
	iterator->hint.reserved1	= 0;
	iterator->hint.reserved2	= 0;

//	LogEndTime(kTraceInsertBTreeRecord, noErr);

	return noErr;


	////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:
	(void) ReleaseNode (btreePtr, &nodeRec);

	iterator->hint.writeCount 	= 0;
	iterator->hint.nodeNum		= 0;
	iterator->hint.index		= 0;
	iterator->hint.reserved1	= 0;
	iterator->hint.reserved2	= 0;
	
	if (err == fsBTEmptyErr)
		err = fsBTRecordNotFoundErr;

//	LogEndTime(kTraceInsertBTreeRecord, err);

	return err;
}



////////////////////////////////// BTSetRecord //////////////////////////////////
#if 0
OSStatus	BTSetRecord			(SFCB						*filePtr,
								 BTreeIterator				*iterator,
								 FSBufferDescriptor			*record,
								 UInt16						 recordLen )
{
	OSStatus				err;
	BTreeControlBlockPtr	btreePtr;
	TreePathTable			treePathTable;
	SInt32					nodesNeeded;
	BlockDescriptor			nodeRec;
	UInt32					insertNodeNum;
	UInt16					index;
	Boolean					recordFound = false;
	Boolean					recordFit;
	Boolean					operation;
	Boolean					validHint;


	////////////////////////// Priliminary Checks ///////////////////////////////

	nodeRec.buffer = nil;					// so we can call ReleaseNode

	err = CheckInsertParams (filePtr, iterator, record, recordLen);
	if (err != noErr)
		return	err;

	btreePtr = (BTreeControlBlockPtr) filePtr->fcbBtree;


	///////////////////////// Find Insert Position //////////////////////////////

	err = IsItAHint (btreePtr, iterator, &validHint);
	M_ExitOnError (err);

	if (validHint)
	{
		insertNodeNum = iterator->hint.nodeNum;

		err = GetNode (btreePtr, insertNodeNum, &nodeRec);
		if( err == noErr )
		{
			err = TrySimpleReplace (btreePtr, nodeRec.buffer, iterator, record, recordLen, &recordFit);
			M_ExitOnError (err);

			if (recordFit)
			{
				err = UpdateNode (btreePtr, &nodeRec);
				M_ExitOnError (err);
				
				recordFound = true;
				++btreePtr->numValidHints;
				goto Success;
			} 											// else
			else
			{
				(void) BTInvalidateHint( iterator );
			}
			
			err = ReleaseNode (btreePtr, &nodeRec);
			M_ExitOnError (err);
		}
	}

	err = SearchTree (btreePtr, &iterator->key, treePathTable, &insertNodeNum, &nodeRec, &index);

	switch (err)				// set/replace/insert decision point
	{
		case noErr:			recordFound = true;
								break;

		case fsBTRecordNotFoundErr:	break;

		case fsBTEmptyErr:	// if tree empty add 1st leaf node

								if (btreePtr->freeNodes == 0)
								{
									err = ExtendBTree (btreePtr, btreePtr->totalNodes + 1);
									M_ExitOnError (err);
								}

								err = AllocateNode (btreePtr, &insertNodeNum);
								M_ExitOnError (err);

								err = GetNewNode (btreePtr, insertNodeNum, &nodeRec);
								M_ExitOnError (err);

								((NodeDescPtr)nodeRec.buffer)->kind		= kBTLeafNode;
								((NodeDescPtr)nodeRec.buffer)->height	= 1;

								recordFit = InsertKeyRecord (btreePtr, nodeRec.buffer, 0,
															 &iterator->key, KeyLength(btreePtr, &iterator->key),
															 record->bufferAddress, recordLen );
								if (recordFit != true)
								{
									err = fsBTRecordTooLargeErr;
									goto ErrorExit;
								}

								err = UpdateNode (btreePtr, &nodeRec);
								M_ExitOnError (err);

								// update BTreeControlBlock
								btreePtr->rootNode	 = insertNodeNum;
								btreePtr->treeDepth	 = 1;
								btreePtr->flags		|= kBTHeaderDirty;

								goto Success;

		default:				goto ErrorExit;
	}


	if (recordFound == true)		// Simple Replace - optimization avoids unecessary ExtendBTree
	{
		err = TrySimpleReplace (btreePtr, nodeRec.buffer, iterator, record, recordLen, &recordFit);
		M_ExitOnError (err);

		if (recordFit)
		{
			err = UpdateNode (btreePtr, &nodeRec);
			M_ExitOnError (err);

			goto Success;
		}
	}


	/////////////////////// Extend File If Necessary ////////////////////////////

	nodesNeeded =  btreePtr->treeDepth + 1 - btreePtr->freeNodes;	//ее math limit
	if (nodesNeeded > 0)
	{
		nodesNeeded += btreePtr->totalNodes;
		if (nodesNeeded > CalcMapBits (btreePtr))	// we'll need to add a map node too!
			++nodesNeeded;

		err = ExtendBTree (btreePtr, nodesNeeded);
		M_ExitOnError (err);
	}


	if (recordFound == true)							// Delete existing record
	{
		DeleteRecord (btreePtr, nodeRec.buffer, index);
		operation = kReplaceRecord;
	} else {
		operation = kInsertRecord;
	}

	err = InsertTree (btreePtr, treePathTable, &iterator->key, record->bufferAddress,
					  recordLen, &nodeRec, index, 1, operation, &insertNodeNum);
	M_ExitOnError (err);

	++btreePtr->writeCount;				//	<CS10> writeCount changes only if the tree structure changed

Success:
	if (recordFound == false)
	{
		++btreePtr->leafRecords;
		M_BTreeHeaderDirty (btreePtr);
	}

	// create hint
	iterator->hint.writeCount 	= btreePtr->writeCount;		//	unused until <CS10>
	iterator->hint.nodeNum		= insertNodeNum;
	iterator->hint.index		= 0;						// unused
	iterator->hint.reserved1	= 0;
	iterator->hint.reserved2	= 0;

	return noErr;


	////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:

	(void) ReleaseNode (btreePtr, &nodeRec);

	iterator->hint.writeCount 	= 0;
	iterator->hint.nodeNum		= 0;
	iterator->hint.index		= 0;
	iterator->hint.reserved1	= 0;
	iterator->hint.reserved2	= 0;

	return err;
}
#endif


//////////////////////////////// BTReplaceRecord ////////////////////////////////

OSStatus	BTReplaceRecord		(SFCB						*filePtr,
								 BTreeIterator				*iterator,
								 FSBufferDescriptor			*record,
								 UInt16						 recordLen )
{
	OSStatus				err;
	BTreeControlBlockPtr	btreePtr;
	TreePathTable			treePathTable;
	SInt32					nodesNeeded;
	BlockDescriptor			nodeRec;
	UInt32					insertNodeNum;
	UInt16					index;
	Boolean					recordFit;
	Boolean					validHint;


	////////////////////////// Priliminary Checks ///////////////////////////////

	nodeRec.buffer = nil;					// so we can call ReleaseNode

	err = CheckInsertParams (filePtr, iterator, record, recordLen);
	if (err != noErr)
		return err;

//	LogStartTime(kTraceReplaceBTreeRecord);

	btreePtr = (BTreeControlBlockPtr) filePtr->fcbBtree;

	////////////////////////////// Take A Hint //////////////////////////////////

	err = IsItAHint (btreePtr, iterator, &validHint);
	M_ExitOnError (err);

	if (validHint)
	{
		insertNodeNum = iterator->hint.nodeNum;

		err = GetNode (btreePtr, insertNodeNum, &nodeRec);
		if( err == noErr )
		{
			err = TrySimpleReplace (btreePtr, nodeRec.buffer, iterator, record, recordLen, &recordFit);
			M_ExitOnError (err);

			if (recordFit)
			{
				err = UpdateNode (btreePtr, &nodeRec);
				M_ExitOnError (err);

				++btreePtr->numValidHints;

				goto Success;
			}
			else
			{
				(void) BTInvalidateHint( iterator );
			}
			
			err = ReleaseNode (btreePtr, &nodeRec);
			M_ExitOnError (err);
		}
		else
		{
			(void) BTInvalidateHint( iterator );
		}
	}


	////////////////////////////// Get A Clue ///////////////////////////////////

	err = SearchTree (btreePtr, &iterator->key, treePathTable, &insertNodeNum, &nodeRec, &index);
	M_ExitOnError (err);					// record must exit for Replace

	// optimization - if simple replace will work then don't extend btree
	// ее if we tried this before, and failed because it wouldn't fit then we shouldn't try this again...

	err = TrySimpleReplace (btreePtr, nodeRec.buffer, iterator, record, recordLen, &recordFit);
	M_ExitOnError (err);

	if (recordFit)
	{
		err = UpdateNode (btreePtr, &nodeRec);
		M_ExitOnError (err);

		goto Success;
	}


	//////////////////////////// Make Some Room /////////////////////////////////

	nodesNeeded =  btreePtr->treeDepth + 1 - btreePtr->freeNodes;	//ее math limit
	if (nodesNeeded > 0)
	{
		nodesNeeded += btreePtr->totalNodes;
		if (nodesNeeded > CalcMapBits (btreePtr))	// we'll need to add a map node too!
			++nodesNeeded;

		err = ExtendBTree (btreePtr, nodesNeeded);
		M_ExitOnError (err);
	}


	DeleteRecord (btreePtr, nodeRec.buffer, index);	// delete existing key/record

	err = InsertTree (btreePtr, treePathTable, &iterator->key, record->bufferAddress,
					  recordLen, &nodeRec, index, 1, kReplaceRecord, &insertNodeNum);
	M_ExitOnError (err);

	++btreePtr->writeCount;				//	<CS10> writeCount changes only if the tree structure changed

Success:
	// create hint
	iterator->hint.writeCount 	= btreePtr->writeCount;		//	unused until <CS10>
	iterator->hint.nodeNum		= insertNodeNum;
	iterator->hint.index		= 0;						// unused
	iterator->hint.reserved1	= 0;
	iterator->hint.reserved2	= 0;

//	LogEndTime(kTraceReplaceBTreeRecord, noErr);

	return noErr;


	////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:

	(void) ReleaseNode (btreePtr, &nodeRec);

	iterator->hint.writeCount 	= 0;
	iterator->hint.nodeNum		= 0;
	iterator->hint.index		= 0;
	iterator->hint.reserved1	= 0;
	iterator->hint.reserved2	= 0;

//	LogEndTime(kTraceReplaceBTreeRecord, err);

	return err;
}



//////////////////////////////// BTDeleteRecord /////////////////////////////////

OSStatus	BTDeleteRecord		(SFCB						*filePtr,
								 BTreeIterator				*iterator )
{
	OSStatus				err;
	BTreeControlBlockPtr	btreePtr;
	TreePathTable			treePathTable;
	BlockDescriptor			nodeRec;
	UInt32					nodeNum;
	UInt16					index;

//	LogStartTime(kTraceDeleteBTreeRecord);

	////////////////////////// Priliminary Checks ///////////////////////////////

	nodeRec.buffer = nil;					// so we can call ReleaseNode

	M_ReturnErrorIf (filePtr == nil, 	paramErr);
	M_ReturnErrorIf (iterator == nil,	paramErr);

	btreePtr = (BTreeControlBlockPtr) filePtr->fcbBtree;
	if (btreePtr == nil)
	{
		err = fsBTInvalidFileErr;
		goto ErrorExit;
	}

#if SupportsKeyDescriptors
	if (btreePtr->keyDescPtr != nil)
	{
		err = CheckKey (&iterator->key, btreePtr->keyDescPtr, btreePtr->maxKeyLength);
		M_ExitOnError (err);
	}
#endif

	/////////////////////////////// Find Key ////////////////////////////////////

	//ее check hint for simple delete case (index > 0, numRecords > 2)

	err = SearchTree (btreePtr, &iterator->key, treePathTable, &nodeNum, &nodeRec, &index);
	M_ExitOnError (err);					// record must exit for Delete


	///////////////////////////// Delete Record /////////////////////////////////

	err = DeleteTree (btreePtr, treePathTable, &nodeRec, index, 1);
	M_ExitOnError (err);

//Success:
	++btreePtr->writeCount;				//	<CS10>
	--btreePtr->leafRecords;
	M_BTreeHeaderDirty (btreePtr);

	iterator->hint.nodeNum	= 0;

//	LogEndTime(kTraceDeleteBTreeRecord, noErr);

	return noErr;

	////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:
	(void) ReleaseNode (btreePtr, &nodeRec);

//	LogEndTime(kTraceDeleteBTreeRecord, err);

	return	err;
}



OSStatus	BTGetInformation	(SFCB					*filePtr,
								 UInt16					 version,
								 BTreeInfoRec			*info )
{
#pragma unused (version)

	BTreeControlBlockPtr	btreePtr;


	M_ReturnErrorIf (filePtr == nil, 	paramErr);

	btreePtr = (BTreeControlBlockPtr) filePtr->fcbBtree;

	M_ReturnErrorIf (btreePtr == nil,	fsBTInvalidFileErr);
	M_ReturnErrorIf (info == nil,		paramErr);

	//ее check version?

	info->nodeSize		= btreePtr->nodeSize;
	info->maxKeyLength	= btreePtr->maxKeyLength;
	info->treeDepth		= btreePtr->treeDepth;
	info->numRecords	= btreePtr->leafRecords;
	info->numNodes		= btreePtr->totalNodes;
	info->numFreeNodes	= btreePtr->freeNodes;
	info->keyDescriptor	= btreePtr->keyDescPtr;	//ее this won't do at all...
	info->reserved		= 0;

	if (btreePtr->keyDescPtr == nil)
		info->keyDescLength	= 0;
	else
		info->keyDescLength	= (UInt32) btreePtr->keyDescPtr->length;

	return noErr;
}



/*-------------------------------------------------------------------------------
Routine:	BTFlushPath	-	Flush BTreeControlBlock to Header Node.

Function:	Brief_description_of_the_function_and_any_side_effects


Input:		pathPtr		- pointer to path control block for B*Tree file to flush

Output:		none

Result:		noErr		- success
			!= noErr	- failure
-------------------------------------------------------------------------------*/

OSStatus	BTFlushPath				(SFCB					*filePtr)
{
	OSStatus				err;
	BTreeControlBlockPtr	btreePtr;


//	LogStartTime(kTraceFlushBTree);

	M_ReturnErrorIf (filePtr == nil, 	paramErr);

	btreePtr = (BTreeControlBlockPtr) filePtr->fcbBtree;

	M_ReturnErrorIf (btreePtr == nil,	fsBTInvalidFileErr);

	err = UpdateHeader (btreePtr);

//	LogEndTime(kTraceFlushBTree, err);

	return	err;
}



/*-------------------------------------------------------------------------------
Routine:	BTInvalidateHint	-	Invalidates the hint within a BTreeInterator.

Function:	Invalidates the hint within a BTreeInterator.


Input:		iterator	- pointer to BTreeIterator

Output:		iterator	- iterator with the hint.nodeNum cleared

Result:		noErr			- success
			paramErr	- iterator == nil
-------------------------------------------------------------------------------*/


OSStatus	BTInvalidateHint	(BTreeIterator				*iterator )
{
	if (iterator == nil)
		return	paramErr;

	iterator->hint.nodeNum = 0;

	return	noErr;
}

