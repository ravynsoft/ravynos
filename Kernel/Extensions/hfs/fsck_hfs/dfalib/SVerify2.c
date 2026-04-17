/*
 * Copyright (c) 1999-2009 Apple Inc. All rights reserved.
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
	File:		SVerify2.c

	Contains:	xxx put contents here xxx

	Version:	xxx put version here xxx

	Copyright:	� 1997-1999 by Apple Computer, Inc., all rights reserved.
*/

#include <sys/ioctl.h>
#include <sys/disk.h>

#include "BTree.h"
#include "BTreePrivate.h"

#include "Scavenger.h"


//	Prototypes for internal subroutines
static int BTKeyChk( SGlobPtr GPtr, NodeDescPtr nodeP, BTreeControlBlock *btcb );


/*------------------------------------------------------------------------------

Routine:	ChkExtRec (Check Extent Record)

Function:	Checks out a generic extent record.
			
Input:		GPtr		-	pointer to scavenger global area.
			extP		-	pointer to extent data record.
			
			
Output:		lastExtentIndex - In normal case, it is set to the maximum number of 
							extents (3 or 8) for given file system.  If the 
							function finds bad extent, it is set to the index 
							of the bad extent entry found.
			ChkExtRec	-	function result:			
								0 = no error
								n = error
------------------------------------------------------------------------------*/
OSErr ChkExtRec ( SGlobPtr GPtr, UInt32 fileID, const void *extents , unsigned int *lastExtentIndex )
{
	short		i;
	Boolean		isHFSPlus;
	UInt32		numABlks;
	UInt32		maxNABlks;
	UInt32		extentBlockCount;
	UInt32		extentStartBlock;

	maxNABlks = GPtr->calculatedVCB->vcbTotalBlocks;
	numABlks = 1;
	isHFSPlus = VolumeObjectIsHFSPlus( );

	/* initialize default output for extent index */
	*lastExtentIndex = GPtr->numExtents;
	
	for ( i=0 ; i<GPtr->numExtents ; i++ )
	{
		if ( isHFSPlus )
		{
			extentBlockCount = ((HFSPlusExtentDescriptor *)extents)[i].blockCount;
			extentStartBlock = ((HFSPlusExtentDescriptor *)extents)[i].startBlock;
		}
		else
		{
			extentBlockCount = ((HFSExtentDescriptor *)extents)[i].blockCount;
			extentStartBlock = ((HFSExtentDescriptor *)extents)[i].startBlock;
		}
		
		if ( extentStartBlock >= maxNABlks )
		{
			*lastExtentIndex = i;
			RcdError( GPtr, E_ExtEnt );
			if (fsckGetVerbosity(GPtr->context) >= kDebugLog) { 
				plog ("\tCheckExtRecord: id=%u %d:(%u,%u), maxBlocks=%u (startBlock > maxBlocks)\n", 
						fileID, i, extentStartBlock, extentBlockCount, maxNABlks);
			}
			return( E_ExtEnt );
		}
		/* Check if end of extent is beyond end of disk */
		if ( extentBlockCount >= (maxNABlks - extentStartBlock) ) 
		{
			*lastExtentIndex = i;
			RcdError( GPtr, E_ExtEnt );
			if (fsckGetVerbosity(GPtr->context) >= kDebugLog) { 
				plog ("\tCheckExtRecord: id=%u %d:(%u,%u), maxBlocks=%u (blockCount > (maxBlocks - startBlock))\n", 
						fileID, i, extentStartBlock, extentBlockCount, maxNABlks);
			}
			return( E_ExtEnt );
		}			
		/* This condition is not checked for standard HFS volumes as it is valid 
		 * to have extent with allocation block number 0 on standard HFS. 
		 */
		if ( isHFSPlus && 
		     ((extentStartBlock == 0) && (extentBlockCount != 0)))
		{
			*lastExtentIndex = i;
			RcdError( GPtr, E_ExtEnt );
			if (fsckGetVerbosity(GPtr->context) >= kDebugLog) { 
				plog ("\tCheckExtRecord: id=%u %d:(%u,%u), (startBlock == 0)\n", 
						fileID, i, extentStartBlock, extentBlockCount);
			}
			return( E_ExtEnt );

		}
		if ((extentStartBlock != 0) && (extentBlockCount == 0))
		{
			*lastExtentIndex = i;
			RcdError( GPtr, E_ExtEnt );
			if (fsckGetVerbosity(GPtr->context) >= kDebugLog) { 
				plog ("\tCheckExtRecord: id=%u %d:(%u,%u), (blockCount == 0)\n", 
						fileID, i, extentStartBlock, extentBlockCount);
			}
			return( E_ExtEnt );
		}	
		if ( numABlks == 0 )
		{
			if ( extentBlockCount != 0 )
			{
				*lastExtentIndex = i;
				RcdError( GPtr, E_ExtEnt );
				if (fsckGetVerbosity(GPtr->context) >= kDebugLog) { 
					plog ("\tCheckExtRecord: id=%u %d:(%u,%u), (blockCount != 0)\n", 
							fileID, i, extentStartBlock, extentBlockCount);
				}
				return( E_ExtEnt );
			}
		}
		numABlks = extentBlockCount;
	}
		
	return( noErr );
}


/*------------------------------------------------------------------------------

Routine:	BTCheck - (BTree Check)

Function Description:	
	Checks out the internal structure of a Btree file.  The BTree 
	structure is enunumerated top down starting from the root node.

	A structure to store the current traversal state of each Btree level
	is used.  The function traverses Btree top to down till it finds
	a leaf node - where it calls checkLeafRecord function for every
	leaf record (if specified).  The function then starts traversing
	down from the next index node at previous BTree level.  If all
	index nodes in given BTree level are traversed top to down,
	it starts traversing the next index node in a previous BTree level -
	until it hits the root node.

	Btree traversal:
	The tree is traversed in depth-first traversal - i.e. we recursively
	traverse the children of a node before visiting its sibling.  
	For the btree shown below, this function will traverse as follows:
	root B C E I H D G F

                     (root node)-----
                                | B |
                                -----
                                  |
                    (node B)-------------
                            | C | D | F |
                            -------------
                            / (node\      \
        (node C)-------------   D)-----    -------- (node F)
                | E | I | H |     | G |    | leaf |
                -------------     -----    --------
            /        /    \         |    
   --------  --------  --------  -------- 
   | leaf |  | leaf |  | leaf |  | leaf |
   --------  --------  --------  -------- 
   (node E)  (node I)  (node H)  (node G)

Input:
	GPtr		-	pointer to scavenger global area
	refNum		-	file refnum
	checkLeafRecord -	pointer to function that should be
				called for every leaf record.


Output:		BTCheck	-	function result:			
		0	= no error
		n 	= error code
------------------------------------------------------------------------------*/

int
BTCheck(SGlobPtr GPtr, short refNum, CheckLeafRecordProcPtr checkLeafRecord)
{
	OSErr			result;
	short			i;
	short			keyLen;
	UInt32			nodeNum;
	short			numRecs;	/* number of records in current node */
	short			index;		/* index to current index record in index node */ 		
	UInt16			recSize;
	UInt8			parKey[ kMaxKeyLength + 2 + 2 ]; /* parent key for comparison */
	Boolean			hasParKey = false;
	UInt8			*dataPtr;
	STPR			*tprP;		/* pointer to store BTree traversal state */
	STPR			*parentP;
	KeyPtr			keyPtr;
	BTHeaderRec		*header;
	NodeRec			node;
	NodeDescPtr		nodeDescP;
	UInt16			*statusFlag = NULL;
	UInt32			leafRecords = 0;
	BTreeControlBlock	*calculatedBTCB	= GetBTreeControlBlock( refNum );
	
	node.buffer = NULL;

	//	Set up
	if ( refNum == kCalculatedCatalogRefNum )
		statusFlag	= &(GPtr->CBTStat);
	else if ( refNum == kCalculatedExtentRefNum )
		statusFlag	= &(GPtr->EBTStat);
	else if ( refNum == kCalculatedAttributesRefNum )
		statusFlag	= &(GPtr->ABTStat);
	else {
		/* BTCheck is currently called only with the above three options.
		 * Initialize status flag correctly if we call BTCheck with other 
		 * options 
		 */
		result = E_BadValue;
		goto exit;
	}

	GPtr->TarBlock = 0;

	/*
	 * Check out BTree header node 
	 */
	result = GetNode( calculatedBTCB, kHeaderNodeNum, &node );
	if ( result != noErr )
	{
		if ( result == fsBTInvalidNodeErr )	/* hfs_swap_BTNode failed */
		{
			RcdError( GPtr, E_BadNode );
			result	= E_BadNode;
		}
		node.buffer = NULL;
		goto exit;
	}

	nodeDescP = node.buffer;

	result = AllocBTN( GPtr, refNum, 0 );
	if (result) goto exit;	/* node already allocated */
	
	/* Check node kind */
	if ( nodeDescP->kind != kBTHeaderNode )
	{
		RcdError( GPtr, E_BadHdrN );
		result = E_BadHdrN;
		goto exit;
	}	
	/* Check total records allowed in header node */
	if ( nodeDescP->numRecords != Num_HRecs )
	{
		RcdError( GPtr, E_BadHdrN );
		result = E_BadHdrN;
		goto exit;
	}	
	/* Check node height */
	if ( nodeDescP->height != 0 )
	{
		RcdError( GPtr, E_NHeight );
		result = E_NHeight;
		goto exit;
	}

	/*		
	 * check out BTree Header record
	 */
	header = (BTHeaderRec*) ((Byte*)nodeDescP + sizeof(BTNodeDescriptor));
	recSize = GetRecordSize( (BTreeControlBlock *)calculatedBTCB, (BTNodeDescriptor *)nodeDescP, 0 );	
	
	/* Check header size */
	if ( recSize != sizeof(BTHeaderRec) )
	{
		RcdError( GPtr, E_LenBTH );
		result = E_LenBTH;
		goto exit;
	}
	/* Check tree depth */
	if ( header->treeDepth > BTMaxDepth )
	{
		RcdError( GPtr, E_BTDepth );
		goto RebuildBTreeExit;
	}
	calculatedBTCB->treeDepth = header->treeDepth;
	
	/* Check validity of root node number */
	if ( header->rootNode >= calculatedBTCB->totalNodes ||
		 (header->treeDepth != 0 && header->rootNode == kHeaderNodeNum) )
	{
		if (debug)
			plog("Header root node %u, calculated total nodes %u, tree depth %u, header node num %u\n",
				header->rootNode, calculatedBTCB->totalNodes,
				header->treeDepth, kHeaderNodeNum);

		RcdError( GPtr, E_BTRoot );
		goto RebuildBTreeExit;
	}
	calculatedBTCB->rootNode = header->rootNode;

	/* Check if tree depth or root node are zero */
	if ( (calculatedBTCB->treeDepth == 0) || (calculatedBTCB->rootNode == 0) )
	{
		/* If both are zero, empty BTree */
		if ( calculatedBTCB->treeDepth != calculatedBTCB->rootNode )
		{
			RcdError( GPtr, E_BTDepth );
			goto RebuildBTreeExit;
		}
	}		

	/*
	 * Check the extents for the btree.
	 * HFS+ considers it an error for a node to be split across
	 * extents, on a journaled filesystem.
	 *
	 * If debug is set, then it continues examining the tree; otherwise,
	 * it exits with a rebuilt error.
	 */
	if (CheckIfJournaled(GPtr, true) &&
	    header->nodeSize > calculatedBTCB->fcbPtr->fcbVolume->vcbBlockSize) {
		/* If it's journaled, it's HFS+ */
		HFSPlusExtentRecord *extp = &calculatedBTCB->fcbPtr->fcbExtents32;
		int i;
		int blocksPerNode = header->nodeSize / calculatedBTCB->fcbPtr->fcbVolume->vcbBlockSize;	// How many blocks in a node
		UInt32 totalBlocks = 0;
		
		/*
		 * First, go through the first 8 extents
		 */
		for (i = 0; i < kHFSPlusExtentDensity; i++) {
			if (((*extp)[i].blockCount % blocksPerNode) != 0) {
				result = errRebuildBtree;
				*statusFlag |= S_RebuildBTree;
				fsckPrint(GPtr->context, E_BTreeSplitNode, calculatedBTCB->fcbPtr->fcbFileID);
				if (debug == 0) {
					goto exit;
				} else {
					plog("Improperly split node in file id %u, offset %u (extent #%d), Extent <%u, %u>\n", calculatedBTCB->fcbPtr->fcbFileID, totalBlocks, i, (*extp)[i].startBlock, (*extp)[i].blockCount);
				}
			}
			totalBlocks += (*extp)[i].blockCount;

		}
		/*
		 * Now, iterate through the extents overflow file if necessary.
		 * Style note:  This is in a block so I can have local variables.
		 * It used to have a conditional, but that wasn't needed.
		 */
		{
			int err;
			BTreeIterator iterator = { 0 };
			FSBufferDescriptor btRecord = { 0 };
			HFSPlusExtentKey *key = (HFSPlusExtentKey*)&iterator.key;
			HFSPlusExtentRecord extRecord = { 0 };
			UInt16	recordSize;
			UInt32	fileID = calculatedBTCB->fcbPtr->fcbFileID;
			static const int kDataForkType = 0;

			BuildExtentKey( true, kDataForkType, fileID, 0, (void*)key );
			btRecord.bufferAddress = &extRecord;
			btRecord.itemCount = 1;
			btRecord.itemSize = sizeof(extRecord);

			while (noErr == (err = BTIterateRecord(GPtr->calculatedExtentsFCB, kBTreeNextRecord, &iterator, &btRecord, &recordSize))) {
				if (key->fileID != fileID ||
				    key->forkType != kDataForkType) {
					break;
				}
				for (i = 0; i < kHFSPlusExtentDensity; i++) {
					if ((extRecord[i].blockCount % blocksPerNode) != 0) {
						result = errRebuildBtree;
						*statusFlag |= S_RebuildBTree;
						fsckPrint(GPtr->context, E_BTreeSplitNode, fileID);
						if (debug == 0) {
							goto exit;
						} else {
							plog("Improperly split node in file id %u, startBlock %u, index %d (offset %u), extent <%u, %u>\n", fileID, key->startBlock, i, totalBlocks, extRecord[i].startBlock, extRecord[i].blockCount);
						}
					}
					totalBlocks += extRecord[i].blockCount;
				}
				memset(&extRecord, 0, sizeof(extRecord));
			}
		}
	}

#if 0
	plog( "\nB-Tree header rec: \n" );
	plog( "    treeDepth     = %d \n", header->treeDepth );
	plog( "    rootNode      = %d \n", header->rootNode );
	plog( "    leafRecords   = %d \n", header->leafRecords );
	plog( "    firstLeafNode = %d \n", header->firstLeafNode );
	plog( "    lastLeafNode  = %d \n", header->lastLeafNode );
	plog( "    totalNodes    = %d \n", header->totalNodes );
	plog( "    freeNodes     = %d \n", header->freeNodes );
#endif
		
	if (calculatedBTCB->rootNode == 0) {
		// Empty btree, no need to continue
		goto exit;
	}
	/*
	 * Set up tree path record for root level
	 */
 	GPtr->BTLevel	= 1;
	/* BTPTPtr is an array of structure which stores the state
	 * of the btree traversal based on the current BTree level.
	 * It helps to traverse to parent node from a child node.
	 * tprP points to the correct offset to read/write.
	 */
	tprP		= &(*GPtr->BTPTPtr)[0];
	tprP->TPRNodeN	= calculatedBTCB->rootNode;
	tprP->TPRRIndx	= -1;	/* last index accessed in a node */
	tprP->TPRLtSib	= 0;
	tprP->TPRRtSib	= 0;
		
	/*
	 * Now enumerate the entire BTree
	 */
	while ( GPtr->BTLevel > 0 )
	{
		tprP	= &(*GPtr->BTPTPtr)[GPtr->BTLevel -1];
		nodeNum	= tprP->TPRNodeN;
		index	= tprP->TPRRIndx;

		GPtr->TarBlock = nodeNum;

		(void) ReleaseNode(calculatedBTCB, &node);
		result = GetNode( calculatedBTCB, nodeNum, &node );
		if ( result != noErr )
		{
			if ( result == fsBTInvalidNodeErr )	/* hfs_swap_BTNode failed */
			{
				RcdError( GPtr, E_BadNode );
				result	= E_BadNode;
			}
			node.buffer = NULL;
			if (debug)
			{
				/* Try to continue checking other nodes.
				 *
				 * Decrement the current btree level as we want to access 
				 * the right sibling index record, if any, of our parent.
				 */
				GPtr->BTLevel--;
				continue;
			}
			goto exit;
		}
		nodeDescP = node.buffer;
			
		/*
		 * Check out and allocate the node if its the first time its been seen
		 */		
		if ( index < 0 )
		{
#if 0 //
			// this will print out our leaf node order
			if ( nodeDescP->kind == kBTLeafNode ) 
			{
				static int		myCounter = 0;
				if ( myCounter > 19 )
				{
					myCounter = 0;
					plog( "\n  " );
				}
				plog( "%d ", nodeNum );
				
				myCounter++;
			}
#endif

			/* Allocate BTree node */
			result = AllocBTN( GPtr, refNum, nodeNum );
			if ( result ) 
			{
				/* node already allocated can be fixed if it is an index node */
				goto RebuildBTreeExit;	
			}
				
			/* Check keys in the node */
			result = BTKeyChk( GPtr, nodeDescP, calculatedBTCB );
			if ( result ) 
			{
				/* we should be able to fix any E_KeyOrd error or any B-Tree key */
				/* errors with an index node. */
				if ( E_KeyOrd == result || nodeDescP->kind == kBTIndexNode )
				{
					*statusFlag |= S_RebuildBTree;
					result = errRebuildBtree;
				}
				else
				{
					goto exit;
				}
			}
				
			/* Check backward link of this node */ 
			if ( nodeDescP->bLink != tprP->TPRLtSib )
			{
				result = E_SibLk;
				RcdError( GPtr, E_SibLk );
				if (debug)
					printf("Node %d's back link is 0x%x; expected 0x%x\n"
						   "    disk offset = 0x%llx, size = 0x%x\n",
						   nodeNum, nodeDescP->bLink, tprP->TPRLtSib,
						   ((Buf_t *)(node.blockHeader))->Offset, ((Buf_t *)(node.blockHeader))->Length);
				if (!debug)
					goto RebuildBTreeExit;	
			}	
			if ( tprP->TPRRtSib == -1 )
			{
				tprP->TPRRtSib = nodeNum;	/* set Rt sibling for later verification */		
			}
			else
			{
				/* Check forward link for this node */
				if ( nodeDescP->fLink != tprP->TPRRtSib )
				{				
					result = E_SibLk;
					RcdError( GPtr, E_SibLk );
					if (debug)
						printf("Node %d's forward link is 0x%x; expected 0x%x\n"
							   "    disk offset = 0x%llx, size = 0x%x\n",
							   nodeNum, nodeDescP->fLink, tprP->TPRRtSib,
							   ((Buf_t *)(node.blockHeader))->Offset, ((Buf_t *)(node.blockHeader))->Length);
				if (!debug)
					goto RebuildBTreeExit;	
				}
			}
			
			/* Check node kind - it should either be index node or leaf node */
			if ( (nodeDescP->kind != kBTIndexNode) && (nodeDescP->kind != kBTLeafNode) )
			{
				result = E_NType;
				RcdError( GPtr, E_NType );
				if (!debug) goto exit;
			}	
			/* Check if the height of this node is correct based on calculated
			 * tree depth and current btree level of the traversal 
			 */
			if ( nodeDescP->height != calculatedBTCB->treeDepth - GPtr->BTLevel + 1 )
			{
				result = E_NHeight;
				RcdError( GPtr, E_NHeight );
				if (!debug) goto RebuildBTreeExit;	
			}
			
			if (result && (cur_debug_level & d_dump_node))
			{
				plog("Node %u:\n", node.blockNum);
				HexDump(node.buffer, node.blockSize, TRUE);
				GPtr->BTLevel--;
				continue;
			}
			
			/* If we saved the first key in the parent (index) node in past, use it to compare 
			 * with the key of the first record in the current node.  This check should 
			 * be performed for all nodes except the root node.
			 */
			if ( hasParKey == true )
			{
				GetRecordByIndex( (BTreeControlBlock *)calculatedBTCB, nodeDescP, 0, &keyPtr, &dataPtr, &recSize );
				if ( CompareKeys( (BTreeControlBlockPtr)calculatedBTCB, (BTreeKey *)parKey, keyPtr ) != 0 )
				{
					if (debug)
					{
						plog("Index key doesn't match first node key\n");
						if (cur_debug_level & d_dump_record)
						{
							plog("Found (child; node %u):\n", tprP->TPRNodeN);
							HexDump(keyPtr, CalcKeySize(calculatedBTCB, keyPtr), FALSE);
							plog("Expected (parent; node %u):\n", tprP[-1].TPRNodeN);
							HexDump(parKey, CalcKeySize(calculatedBTCB, (BTreeKey *)parKey), FALSE);
						}
					}
					RcdError( GPtr, E_IKey );
					*statusFlag |= S_RebuildBTree;
					result = errRebuildBtree;
				}
			}
			if ( nodeDescP->kind == kBTIndexNode )
 			{
				if ( ( result = CheckForStop( GPtr ) ) )
					goto exit;
			}
			
			GPtr->itemsProcessed++;
		}
		
		numRecs = nodeDescP->numRecords;
	
		/* 
		 * for an index node ...
		 */
		if ( nodeDescP->kind == kBTIndexNode )
		{
			index++;	/* on to next index record */
			if ( index >= numRecs )
			{
				/* We have traversed children of all index records in this index node.
				 * Decrement the current btree level to access right sibling index record
				 * of previous btree level 
				 */
				GPtr->BTLevel--;
				continue;	/* No more records */
			}
			
			/* Store current index for current Btree level */
			tprP->TPRRIndx	= index;
			/* Store current pointer as parent for next traversal */
			parentP			= tprP;
			/* Increase the current Btree level because we traverse top to down */
			GPtr->BTLevel++;

			/* Validate current btree traversal level */
			if ( GPtr->BTLevel > BTMaxDepth )
			{
				RcdError( GPtr, E_BTDepth );
				goto RebuildBTreeExit;
			}				
			/* Get the btree traversal state for current btree level */ 
			tprP = &(*GPtr->BTPTPtr)[GPtr->BTLevel -1];
			
			/* Get index record in the current btree level at offset index in the given node */
			GetRecordByIndex( (BTreeControlBlock *)calculatedBTCB, nodeDescP, 
							  index, &keyPtr, &dataPtr, &recSize );
			
			nodeNum = *(UInt32*)dataPtr;
			/* Current node number should not be header node number or greater than total nodes */
			if ( (nodeNum == kHeaderNodeNum) || (nodeNum >= calculatedBTCB->totalNodes) )
			{
				RcdError( GPtr, E_IndxLk );
				goto RebuildBTreeExit;
			}	

			/* 
			 * Make a copy of the parent's key so we can compare it
			 * with the child's key later.
			 */
			keyLen = ( calculatedBTCB->attributes & kBTBigKeysMask )
						? keyPtr->length16 + sizeof(UInt16)
						: keyPtr->length8 + sizeof(UInt8);
			CopyMemory(keyPtr, parKey, keyLen);
			hasParKey = true;
				
			/* Store current node number for the child node */
			tprP->TPRNodeN = nodeNum;
			/* Initialize index to records for the child node */
			tprP->TPRRIndx = -1;

			tprP->TPRLtSib = 0;	/* left sibling */
			if ( index > 0 )
			{
				/* Get node number for the previous index record in current index node */
				GetRecordByIndex( (BTreeControlBlock *)calculatedBTCB, nodeDescP, index-1, &keyPtr, &dataPtr, &recSize );

				nodeNum = *(UInt32*)dataPtr;
				/* node number should not be header node number or greater than total nodes */
				if ( (nodeNum == kHeaderNodeNum) || (nodeNum >= calculatedBTCB->totalNodes) )
				{
					RcdError( GPtr, E_IndxLk );
					goto RebuildBTreeExit;
				}
				/* Store this as left sibling node */
				tprP->TPRLtSib = nodeNum;
			}
			else
			{
				if ( parentP->TPRLtSib != 0 )
					tprP->TPRLtSib = tprP->TPRRtSib;	/* Fill in the missing link */
			}
				
			tprP->TPRRtSib = 0;	/* right sibling */
			if ( index < (numRecs -1) )
			{
				/* Get node number for the next index record in current index node */
				GetRecordByIndex( (BTreeControlBlock *)calculatedBTCB, nodeDescP, index+1, &keyPtr, &dataPtr, &recSize );

				nodeNum = *(UInt32*)dataPtr;
				/* node number should not be header node number or greater than total nodes */
				if ( (nodeNum == kHeaderNodeNum) || (nodeNum >= calculatedBTCB->totalNodes) )
				{
					RcdError( GPtr, E_IndxLk );
					goto RebuildBTreeExit;
				}
				/* Store this as right sibling node */
				tprP->TPRRtSib = nodeNum;
			}
			else
			{
				if ( parentP->TPRRtSib != 0 )
					tprP->TPRRtSib = -1;	/* Link to be filled in later */
			}
		}			
	
		/*
		 * For a leaf node ...
		 */
		else
		{
			/* If left sibling link is zero, this is first leaf node */
			if ( tprP->TPRLtSib == 0 )
				calculatedBTCB->firstLeafNode = nodeNum;
			/* If right sibling link is zero, this is last leaf node */
			if ( tprP->TPRRtSib == 0 )
				calculatedBTCB->lastLeafNode = nodeNum;
			leafRecords	+= nodeDescP->numRecords;

			if (checkLeafRecord != NULL) {
				/* For total number of records in this leaf node, get each record sequentially 
				 * and call function to check individual leaf record through the
				 * function pointer passed by the caller
				 */
				for (i = 0; i < nodeDescP->numRecords; i++) {
					GetRecordByIndex(calculatedBTCB, nodeDescP, i, &keyPtr, &dataPtr, &recSize);
					result = checkLeafRecord(GPtr, keyPtr, dataPtr, recSize);
					if (result) goto exit;
				}
			}
			/* Decrement the current btree level as we want to access 
			 * the right sibling index record, if any, of our parent.
			 */
			GPtr->BTLevel--;
			continue;
		}		
	} /* end while */

	calculatedBTCB->leafRecords = leafRecords;
	
exit:
	if (result == noErr && (*statusFlag & S_RebuildBTree))
		result = errRebuildBtree;
	if (node.buffer != NULL)
		(void) ReleaseNode(calculatedBTCB, &node);
		
	return( result );

RebuildBTreeExit:
	/* force a B-Tree file rebuild */
	*statusFlag |= S_RebuildBTree;
	result = errRebuildBtree;
	goto exit;

} /* end of BTCheck */



/*------------------------------------------------------------------------------

Routine:	BTMapChk - (BTree Map Check)

Function:	Checks out the structure of a BTree allocation map.
			
Input:		GPtr		- pointer to scavenger global area
		fileRefNum	- refnum of BTree file

Output:		BTMapChk	- function result:			
			0 = no error
			n = error
------------------------------------------------------------------------------*/

int BTMapChk( SGlobPtr GPtr, short fileRefNum )
{
	OSErr				result;
	UInt16				recSize;
	SInt32				mapSize;
	UInt32				nodeNum;
	SInt16				recIndx;
	NodeRec				node;
	NodeDescPtr			nodeDescP;
	BTreeControlBlock	*calculatedBTCB	= GetBTreeControlBlock( fileRefNum );

	result = noErr;
	nodeNum	= 0;	/* Start with header node */
	node.buffer = NULL;
	recIndx	= 2;	
	mapSize	= ( calculatedBTCB->totalNodes + 7 ) / 8;	/* size in bytes */

	/*
	 * Enumerate the map structure starting with the map record in the header node
	 */
	while ( mapSize > 0 )
	{
		GPtr->TarBlock = nodeNum;
		
		if (node.buffer != NULL)
			(void) ReleaseNode(calculatedBTCB, &node);
		result = GetNode( calculatedBTCB, nodeNum, &node );
		if ( result != noErr )
		{
			if ( result == fsBTInvalidNodeErr )	/* hfs_swap_BTNode failed */
			{
				RcdError( GPtr, E_BadNode );
				result	= E_BadNode;
			}
			return( result );
		}
		
		nodeDescP = node.buffer;
		
		/* Check out the node if its not the header node */	

		if ( nodeNum != 0 )
		{
			result = AllocBTN( GPtr, fileRefNum, nodeNum );
			if (result) goto exit;	/* Error, node already allocated? */
				
			if ( nodeDescP->kind != kBTMapNode )
			{
				RcdError( GPtr, E_BadMapN );
				if (debug)
					plog("Expected map node, got type %d\n", nodeDescP->kind);
				result = E_BadMapN;
				goto exit;
			}	
			if ( nodeDescP->numRecords != Num_MRecs )
			{
				RcdError( GPtr, E_BadMapN );
				if (debug)
					plog("Expected %d records in node, found %d\n", Num_MRecs, nodeDescP->numRecords);
				result = E_BadMapN;
				goto exit;
			}	
			if ( nodeDescP->height != 0 )
				RcdError( GPtr, E_NHeight );
		}

		//	Move on to the next map node
		recSize  = GetRecordSize( (BTreeControlBlock *)calculatedBTCB, (BTNodeDescriptor *)nodeDescP, recIndx );
		mapSize -= recSize;	/* Adjust remaining map size */

		recIndx	= 0;	/* Map record is now record 0 */	
		nodeNum	= nodeDescP->fLink;						
		if (nodeNum == 0)
			break;
	
	} /* end while */


	if ( (nodeNum != 0) || (mapSize > 0) )
	{
		RcdError( GPtr, E_MapLk);
		result = E_MapLk;	/* bad map node linkage */
	}
exit:
	if (node.buffer != NULL)
		(void) ReleaseNode(calculatedBTCB, &node);
	
	return( result );
	
} /* end BTMapChk */



/*------------------------------------------------------------------------------

Routine:	BTCheckUnusedNodes

Function:	Examines all unused nodes and makes sure they are filled with zeroes.
			If there are any unused nodes which are not zero filled, bit mask
			S_UnusedNodesNotZero is set in output btStat; the function result
			is zero in this case.
			
Input:		GPtr		- pointer to scavenger global area
			fileRefNum	- refnum of BTree file

Output:		*btStat		- bit mask S_UnusedNodesNotZero
			BTCheckUnusedNodes - function result:			
			0 = no error
			n = error
------------------------------------------------------------------------------*/

int BTCheckUnusedNodes(SGlobPtr GPtr, short fileRefNum, UInt16 *btStat)
{
	BTreeControlBlock *btcb	= GetBTreeControlBlock(fileRefNum);
	unsigned char *bitmap = (unsigned char *) ((BTreeExtensionsRec*)btcb->refCon)->BTCBMPtr;
	unsigned char mask = 0x80;
	OSErr err;
	UInt32 nodeNum;
	BlockDescriptor node;
	
	node.buffer = NULL;
	
	for (nodeNum = 0; nodeNum < btcb->totalNodes; ++nodeNum)
	{
		if ((*bitmap & mask) == 0)
		{
			UInt32 i;
			UInt32 bufferSize;
			UInt32 *buffer;
			
			/* Read the raw node, without going through hfs_swap_BTNode. */
			err = btcb->getBlockProc(btcb->fcbPtr, nodeNum, kGetBlock, &node);
			if (err)
			{
				if (debug) plog("Couldn't read node #%u\n", nodeNum);
				return err;
			}
			
			/*
			 * Make sure node->blockSize bytes at address node->buffer are zero.
			 */
			buffer = (UInt32 *) node.buffer;
			bufferSize = node.blockSize / sizeof(UInt32);
			
			for (i = 0; i < bufferSize; ++i)
			{
				if (buffer[i])
				{
					*btStat |= S_UnusedNodesNotZero;
					GPtr->TarBlock = nodeNum;
					fsckPrint(GPtr->context, E_UnusedNodeNotZeroed, nodeNum);
										
					if (!debug)
					{
						/* Stop now; repair will zero all unused nodes. */
						goto done;
					}
					
					/* No need to check the rest of this node. */
					break;
				}
			}
			
			/* Release the node without going through hfs_swap_BTNode. */
			(void) btcb->releaseBlockProc(btcb->fcbPtr, &node, kReleaseBlock);
			node.buffer = NULL;
		}
		
		/* Move to the next bit in the bitmap. */
		mask >>= 1;
		if (mask == 0)
		{
			mask = 0x80;
			++bitmap;
		}
	}
done:
	if (node.buffer)
	{
		(void) btcb->releaseBlockProc(btcb->fcbPtr, &node, kReleaseBlock);
	}
	
	return 0;
} /* end BTCheckUnusedNodes */



/*------------------------------------------------------------------------------

Routine:	CmpBTH - (Compare BTree Header)

Function:	Compares the scavenger BTH info with the BTH on disk.
			
Input:		GPtr - pointer to scavenger global area
		fileRefNum - file refnum

Output:		CmpBTH - function result:			
			0 = no error
			n = error
------------------------------------------------------------------------------*/

OSErr	CmpBTH( SGlobPtr GPtr, SInt16 fileRefNum )
{
	OSErr err;
	BTHeaderRec bTreeHeader;
	BTreeControlBlock *calculatedBTCB = GetBTreeControlBlock( fileRefNum );
	SInt16 *statP;
	SFCB * fcb;
	short isBTHDamaged = 0;
	short printMsg = 0;

	switch (fileRefNum) {
	case kCalculatedCatalogRefNum:
		statP = (SInt16 *)&GPtr->CBTStat;
		fcb = GPtr->calculatedCatalogFCB;
		break;
	case kCalculatedExtentRefNum:
		statP = (SInt16 *)&GPtr->EBTStat;
		fcb = GPtr->calculatedExtentsFCB;
		break;
	case kCalculatedAttributesRefNum:
		statP = (SInt16 *)&GPtr->ABTStat;
		fcb = GPtr->calculatedAttributesFCB;
		break;
	default:
		return (-1);
	};

	/* 
	 * Get BTree header record from disk 
	 */
	GPtr->TarBlock = 0;	//	Set target node number

	err = GetBTreeHeader(GPtr, fcb, &bTreeHeader );
	ReturnIfError( err );

	if (calculatedBTCB->leafRecords != bTreeHeader.leafRecords) {
		char goodStr[32], badStr[32];

		printMsg = 1;
		fsckPrint(GPtr->context, E_LeafCnt);
		sprintf(goodStr, "%ld", (long)calculatedBTCB->leafRecords);
		sprintf(badStr, "%ld", (long)bTreeHeader.leafRecords);
		fsckPrint(GPtr->context, E_BadValue, goodStr, badStr);
	} 
    
	if ( calculatedBTCB->treeDepth != bTreeHeader.treeDepth ) {
   		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
            	plog("\tinvalid tree depth - calculated %d header %d \n", 
                    	calculatedBTCB->treeDepth, bTreeHeader.treeDepth);
			isBTHDamaged = 1;
    	} else if ( calculatedBTCB->rootNode != bTreeHeader.rootNode ) {
        	if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
            	plog("\tinvalid root node - calculated %d header %d \n", 
                    	calculatedBTCB->rootNode, bTreeHeader.rootNode);
			isBTHDamaged = 1;
    	} else if ( calculatedBTCB->firstLeafNode != bTreeHeader.firstLeafNode ) {
        	if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
            	plog("\tinvalid first leaf node - calculated %d header %d \n", 
                    	calculatedBTCB->firstLeafNode, bTreeHeader.firstLeafNode);
			isBTHDamaged = 1;
	} else if ( calculatedBTCB->lastLeafNode != bTreeHeader.lastLeafNode ) {
        	if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
            	plog("\tinvalid last leaf node - calculated %d header %d \n", 
                    	calculatedBTCB->lastLeafNode, bTreeHeader.lastLeafNode);
			isBTHDamaged = 1;
	} else if ( calculatedBTCB->nodeSize != bTreeHeader.nodeSize ) {
        	if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
            	plog("\tinvalid node size - calculated %d header %d \n", 
                    	calculatedBTCB->nodeSize, bTreeHeader.nodeSize);
			isBTHDamaged = 1;
    	} else if ( calculatedBTCB->maxKeyLength != bTreeHeader.maxKeyLength ) {
        	if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
            	plog("\tinvalid max key length - calculated %d header %d \n", 
                    	calculatedBTCB->maxKeyLength, bTreeHeader.maxKeyLength);
			isBTHDamaged = 1;
    	} else if ( calculatedBTCB->totalNodes != bTreeHeader.totalNodes ) {
        	if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
            	plog("\tinvalid total nodes - calculated %d header %d \n", 
                    	calculatedBTCB->totalNodes, bTreeHeader.totalNodes);
			isBTHDamaged = 1;
    	} else if ( calculatedBTCB->freeNodes != bTreeHeader.freeNodes ) {
        	if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
            	plog("\tinvalid free nodes - calculated %d header %d \n", 
                    	calculatedBTCB->freeNodes, bTreeHeader.freeNodes);
			isBTHDamaged = 1;
	}

	if (isBTHDamaged || printMsg) {
    		*statP = *statP | S_BTH;
		if (isBTHDamaged) {
			fsckPrint(GPtr->context, E_InvalidBTreeHeader);
		}
	}
	return( noErr );
}



/*------------------------------------------------------------------------------

Routine:	CmpBlock

Function:	Compares two data blocks for equality.
			
Input:		Blk1Ptr		-	pointer to 1st data block.
			Blk2Ptr		-	pointer to 2nd data block.
			len			-	size of the blocks (in bytes)

Output:		CmpBlock	-	result code	
			0 = equal
			1 = not equal
------------------------------------------------------------------------------*/

OSErr	CmpBlock( void *block1P, void *block2P, size_t length )
{
	Byte	*blk1Ptr = block1P;
	Byte	*blk2Ptr = block2P;

	while ( length-- ) 
		if ( *blk1Ptr++ != *blk2Ptr++ )
			return( -1 );
	
	return( noErr );
	
}



/*------------------------------------------------------------------------------

Routine:	CmpBTM - (Compare BTree Map)

Function:	Compares the scavenger BTM with the BTM on disk.
			
Input:		GPtr		-	pointer to scavenger global area
			fileRefNum	-	file refnum

Output:		CmpBTM	-	function result:			
						0	= no error
						n 	= error
------------------------------------------------------------------------------*/

int CmpBTM( SGlobPtr GPtr, short fileRefNum )
{
	OSErr			result;
	UInt16			recSize;
	SInt32			mapSize;
	SInt32			size;
	UInt32			nodeNum;
	short			recIndx;
	char			*p;
	char			*sbtmP;
	UInt8 *			dataPtr;
	NodeRec			node;
	NodeDescPtr		nodeDescP;
	BTreeControlBlock	*calculatedBTCB;
	UInt16			*statP;

	result = noErr;
	calculatedBTCB	= GetBTreeControlBlock( fileRefNum );

	switch (fileRefNum) {
	case kCalculatedCatalogRefNum:
		statP = &GPtr->CBTStat;
		break;
	case kCalculatedExtentRefNum:
		statP = &GPtr->EBTStat;
		break;
	case kCalculatedAttributesRefNum:
		statP = &GPtr->ABTStat;
		break;
	default:
		return (-1);
	};

	nodeNum	= 0;	/* start with header node */
	node.buffer = NULL;
	recIndx	= 2;
	recSize = size = 0;
	mapSize	= (calculatedBTCB->totalNodes + 7) / 8;		/* size in bytes */
	sbtmP	= ((BTreeExtensionsRec*)calculatedBTCB->refCon)->BTCBMPtr;
	dataPtr = NULL;

	/*
	 * Enumerate BTree map records starting with map record in header node
	 */
	while ( mapSize > 0 )
	{
		GPtr->TarBlock = nodeNum;
		
		if (node.buffer != NULL)	
			(void) ReleaseNode(calculatedBTCB, &node);

		result = GetNode( calculatedBTCB, nodeNum, &node );
		if (result) goto exit;	/* error, could't get map node */

		nodeDescP = node.buffer;

		recSize = GetRecordSize( (BTreeControlBlock *)calculatedBTCB, (BTNodeDescriptor *)nodeDescP, recIndx );
		dataPtr = GetRecordAddress( (BTreeControlBlock *)calculatedBTCB, (BTNodeDescriptor *)nodeDescP, recIndx );
	
		size	= ( recSize  > mapSize ) ? mapSize : recSize;
			
		result = CmpBlock( sbtmP, dataPtr, size );
		if ( result != noErr )
		{ 	
			*statP = *statP | S_BTM;	/* didn't match, mark it damaged */
			RcdError(GPtr, E_BadMapN);
			result = 0;			/* mismatch isn't fatal; let us continue */
			goto exit;
		}
	
		recIndx	= 0;			/* map record is now record 0 */			
		mapSize	-= size;		/* adjust remaining map size */
		sbtmP	= sbtmP + size;
		nodeNum	= nodeDescP->fLink;	/* next node number */						
		if (nodeNum == 0)
			break;
	
	} /* end while */

	/* 
	 * Make sure the unused portion of the last map record is zero
	 */
	for ( p = (Ptr)dataPtr + size ; p < (Ptr)dataPtr + recSize ; p++ )
		if ( *p != 0 ) 
			*statP	= *statP | S_BTM;	/* didn't match, mark it damaged */

exit:
	if (node.buffer != NULL)	
		(void) ReleaseNode(calculatedBTCB, &node);

	return( result );
	
} /* end CmpBTM */


/*------------------------------------------------------------------------------

Routine:	BTKeyChk - (BTree Key Check)

Function:	Checks out the key structure within a Btree node.
			
Input:		GPtr		-	pointer to scavenger global area
		NodePtr		-	pointer to target node
		BTCBPtr		-	pointer to BTreeControlBlock

Output:		BTKeyChk	-	function result:			
			0 = no error
			n = error code
------------------------------------------------------------------------------*/
extern HFSPlusCatalogKey gMetaDataDirKey;

static int BTKeyChk( SGlobPtr GPtr, NodeDescPtr nodeP, BTreeControlBlock *btcb )
{
	SInt16				index;
	UInt16				dataSize;
	UInt16				keyLength;
	UInt16				prevKeyLength = 0;
	KeyPtr 				keyPtr;
	UInt8				*dataPtr;
	KeyPtr				prevkeyP	= nil;
	unsigned			sizeofKeyLength;
	int					result = noErr;
	
	if (btcb->attributes & kBTBigKeysMask)
		sizeofKeyLength = 2;
	else
		sizeofKeyLength = 1;

	if ( nodeP->numRecords == 0 )
	{
		if ( (nodeP->fLink == 0) && (nodeP->bLink == 0) )
		{
			RcdError( GPtr, E_BadNode );
			return( E_BadNode );
		}
	}
	else
	{
		/*
		 * Loop on number of records in node
		 */
		for ( index = 0; index < nodeP->numRecords; index++)
		{
			GetRecordByIndex( (BTreeControlBlock *)btcb, nodeP, (UInt16) index, &keyPtr, &dataPtr, &dataSize );
	
			if (btcb->attributes & kBTBigKeysMask)
				keyLength = keyPtr->length16;
			else
				keyLength = keyPtr->length8;
				
			if ( keyLength > btcb->maxKeyLength )
			{
				RcdError( GPtr, E_KeyLen );
				return( E_KeyLen );
			}
	
			if ( prevkeyP != nil )
			{
				if ( CompareKeys( (BTreeControlBlockPtr)btcb, prevkeyP, keyPtr ) >= 0 )
				{
					/*
					 * When the HFS+ MetaDataDirKey is out of order we mark
					 * the result code so that it can be deleted later.
					 */
					if ((btcb->maxKeyLength == kHFSPlusCatalogKeyMaximumLength)  &&
					    (CompareKeys(btcb, prevkeyP, (KeyPtr)&gMetaDataDirKey) == 0))
					{
						if (fsckGetVerbosity(GPtr->context) > 0)
							plog("Problem: b-tree key for \"HFS+ Private Data\" directory is out of order.\n");
						return( E_KeyOrd + 1000 );
					} 
					else
					{
                        RcdError( GPtr, E_KeyOrd );
                        if (fsckGetVerbosity(GPtr->context) > 0)
                            plog("Records %d and %d (0-based); offsets 0x%04X and 0x%04X\n", index-1, index, (long)prevkeyP - (long)nodeP, (long)keyPtr - (long)nodeP);
						result = E_KeyOrd;
					}
				}
			}
			prevkeyP = keyPtr;
			prevKeyLength = keyLength;
		}
	}

	if (result == E_KeyOrd)
	{
		if (cur_debug_level & d_dump_record)
		{
			for (index = 0; index < nodeP->numRecords; ++index)
			{
				GetRecordByIndex( (BTreeControlBlock *)btcb, nodeP, (UInt16) index, &keyPtr, &dataPtr, &dataSize );
	
				if (btcb->attributes & kBTBigKeysMask)
					keyLength = keyPtr->length16;
				else
					keyLength = keyPtr->length8;
	
				plog("Record %d (offset 0x%04X):\n", index, (long)keyPtr - (long)nodeP);
				HexDump(keyPtr, keyLength + sizeofKeyLength, FALSE);
				plog("--\n");
				HexDump(dataPtr, dataSize, FALSE);
				plog("\n");
			}
		}
		
		if (cur_debug_level & d_dump_node)
		{
			plog("Node:\n");
			HexDump(nodeP, btcb->nodeSize, TRUE);
		}
	}
	
	return( result );
}



/*------------------------------------------------------------------------------

Routine:	ChkCName (Check Catalog Name)

Function:	Checks out a generic catalog name.
			
Input:		GPtr		-	pointer to scavenger global area.
		CNamePtr	-	pointer to CName.
			
Output:		ChkCName	-	function result:			
					0 = CName is OK
					E_CName = invalid CName
------------------------------------------------------------------------------*/

OSErr ChkCName( SGlobPtr GPtr, const CatalogName *name, Boolean unicode )
{
	UInt32	length;
	OSErr	err		= noErr;
	
	length = CatalogNameLength( name, unicode );
	
	if ( unicode )
	{
		if ( (length == 0) || (length > kHFSPlusMaxFileNameChars) )
			err = E_CName;
	}
	else
	{
		if ( (length == 0) || (length > kHFSMaxFileNameChars) )
			err = E_CName;
	}
	
	return( err );
}


/*------------------------------------------------------------------------------

Routine:	CmpMDB - (Compare Master Directory Block)

Function:	Compares the scavenger MDB info with the MDB on disk.
			
Input:		GPtr			-	pointer to scavenger global area

Output:		CmpMDB			- 	function result:
									0 = no error
									n = error
			GPtr->VIStat	-	S_MDB flag set in VIStat if MDB is damaged.
------------------------------------------------------------------------------*/

int CmpMDB( SGlobPtr GPtr,  HFSMasterDirectoryBlock * mdbP)
{
	short	i;
	SFCB *  fcbP;
	SVCB *  vcb;
	short printMsg = 0;
	short isMDBDamaged = 0;

	//	Set up
	GPtr->TarID = MDB_FNum;
	vcb = GPtr->calculatedVCB;
	
	/* 
	 * compare VCB info with MDB
	 */
	if ( mdbP->drSigWord	!= vcb->vcbSignature ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drSigWord \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drCrDate	!= vcb->vcbCreateDate )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drCrDate \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drLsMod	!= vcb->vcbModifyDate )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drLsMod \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drAtrb	!= (UInt16)vcb->vcbAttributes )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drAtrb \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drVBMSt	!= vcb->vcbVBMSt )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drVBMSt \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drNmAlBlks	!= vcb->vcbTotalBlocks ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drNmAlBlks \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drClpSiz	!= vcb->vcbDataClumpSize )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drClpSiz \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drAlBlSt	!= vcb->vcbAlBlSt )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drAlBlSt \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drNxtCNID	!= vcb->vcbNextCatalogID )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drNxtCNID \n" );
		isMDBDamaged = 1;
	}	
	if ( CmpBlock( mdbP->drVN, vcb->vcbVN, mdbP->drVN[0]+1 ) )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drVN \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drVolBkUp	!= vcb->vcbBackupDate )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drVolBkUp \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drVSeqNum	!= vcb->vcbVSeqNum )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drVSeqNum \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drWrCnt	!= vcb->vcbWriteCount )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drWrCnt \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drXTClpSiz	!= vcb->vcbExtentsFile->fcbClumpSize )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drXTClpSiz \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drCTClpSiz	!= vcb->vcbCatalogFile->fcbClumpSize )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drCTClpSiz \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drNmRtDirs	!= vcb->vcbNmRtDirs )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drNmRtDirs \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drFilCnt	!= vcb->vcbFileCount )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drFilCnt \n" );
		isMDBDamaged = 1;
	}	
	if ( mdbP->drDirCnt	!= vcb->vcbFolderCount )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drDirCnt \n" );
		isMDBDamaged = 1;
	}	
	if ( CmpBlock(mdbP->drFndrInfo, vcb->vcbFinderInfo, 32 ) )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid MDB drFndrInfo \n" );
		isMDBDamaged = 1;
	}	

	/* 
	 * compare extent file allocation info with MDB
	 */
	fcbP = vcb->vcbExtentsFile;	/* compare PEOF for extent file */
	if ( mdbP->drXTFlSize != fcbP->fcbPhysicalSize )
	{
		printMsg = 1;
		WriteError ( GPtr, E_MDBDamaged, 3, 0 );
	}
	for ( i = 0; i < GPtr->numExtents; i++ )
	{
		if ( (mdbP->drXTExtRec[i].startBlock != fcbP->fcbExtents16[i].startBlock) ||
		     (mdbP->drXTExtRec[i].blockCount != fcbP->fcbExtents16[i].blockCount) )
		{
			printMsg = 1;
			WriteError ( GPtr, E_MDBDamaged, 4, 0 );
		}
	}

	/*
	 * compare catalog file allocation info with MDB
	 */		
	fcbP = vcb->vcbCatalogFile;	/* compare PEOF for catalog file */
	if ( mdbP->drCTFlSize != fcbP->fcbPhysicalSize )
	{
		printMsg = 1;
		WriteError ( GPtr, E_MDBDamaged, 5, 0 );
	}
	for ( i = 0; i < GPtr->numExtents; i++ )
	{
		if ( (mdbP->drCTExtRec[i].startBlock != fcbP->fcbExtents16[i].startBlock) ||
		     (mdbP->drCTExtRec[i].blockCount != fcbP->fcbExtents16[i].blockCount) )
		{
			printMsg = 1;
			WriteError ( GPtr, E_MDBDamaged, 6, 0 );
		}
	}
	
	if (isMDBDamaged || printMsg) {
		GPtr->VIStat = GPtr->VIStat | S_MDB;
		if (isMDBDamaged)
			WriteError ( GPtr, E_MDBDamaged, 1, 0 );
	}
	return( noErr );
	
} /* end CmpMDB */



/*------------------------------------------------------------------------------

Routine:	CompareVolumeHeader - (Compare VolumeHeader Block)

Function:	Compares the scavenger VolumeHeader info with the VolumeHeader on disk.
			
Input:		GPtr			-	pointer to scavenger global area

Output:		CmpMDB			- 	function result:
									0 = no error
									n = error
			GPtr->VIStat	-	S_MDB flag set in VIStat if MDB is damaged.
------------------------------------------------------------------------------*/

OSErr CompareVolumeHeader( SGlobPtr GPtr, HFSPlusVolumeHeader *volumeHeader )
{
	SInt16			i;
	SVCB			*vcb;
	SFCB			*fcbP;
	UInt32			hfsPlusIOPosOffset;
	UInt32 			goodValue, badValue;
	char 			goodStr[32], badStr[32];
	short 			isVHDamaged;
	short 			printMsg;

	vcb = GPtr->calculatedVCB;
	GPtr->TarID = MDB_FNum;
	
	hfsPlusIOPosOffset = vcb->vcbEmbeddedOffset;

	goodValue = badValue = 0;
	isVHDamaged = 0;
	printMsg = 0;
	
	// CatHChk will flag valence errors and display the good and bad values for
	// our file and folder counts.  It will set S_Valence in CatStat when this
	// problem is detected.  We do NOT want to flag the error here in that case
	// since the volume header counts cannot be trusted and it will lead to 
	// confusing messages.
	if ( volumeHeader->fileCount != vcb->vcbFileCount && 
		 (GPtr->CatStat & S_Valence) == 0 ) {
		fsckPrint(GPtr->context, E_FilCnt);
		sprintf(goodStr, "%u", vcb->vcbFileCount);
		sprintf(badStr, "%u", volumeHeader->fileCount);
		fsckPrint(GPtr->context, E_BadValue, goodStr, badStr);
		printMsg = 1;
	}
        
	if ( volumeHeader->folderCount != vcb->vcbFolderCount && 
		 (GPtr->CatStat & S_Valence) == 0 ) {
		fsckPrint(GPtr->context, E_DirCnt);
		sprintf(goodStr, "%u", vcb->vcbFolderCount);
		sprintf(badStr, "%u", volumeHeader->folderCount);
		fsckPrint(GPtr->context, E_BadValue, goodStr, badStr);

		printMsg = 1;
	}
        
	if (volumeHeader->freeBlocks != vcb->vcbFreeBlocks) {
		fsckPrint(GPtr->context, E_FreeBlocks);
		sprintf(goodStr, "%u", vcb->vcbFreeBlocks); 
		sprintf(badStr, "%u", volumeHeader->freeBlocks);
		fsckPrint(GPtr->context, E_BadValue, goodStr, badStr);
		printMsg = 1;
	}
	
	if ( volumeHeader->catalogFile.clumpSize != vcb->vcbCatalogFile->fcbClumpSize ) {
		fsckPrint(GPtr->context, E_InvalidClumpSize);
		sprintf(goodStr, "%u", vcb->vcbCatalogFile->fcbClumpSize);
		sprintf(badStr, "%u", volumeHeader->catalogFile.clumpSize);
		fsckPrint(GPtr->context, E_BadValue, goodStr, badStr);
		printMsg = 1;
	}

	if ( volumeHeader->signature != kHFSPlusSigWord  &&
	     volumeHeader->signature != kHFSXSigWord) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB signature \n" );
		isVHDamaged = 1;
	}
	/* From HFS Plus Volume Format Specification (TN1150), "It is acceptable 
	 * for a bit in encodingsBitmap to be set even though no names on the 
	 * volume use that encoding".  Therefore we do not report extra bits set in
	 * on-disk encodingsBitmap as error but will repair it silently if any other 
	 * repairs are made.  We complain about extra bits cleared in 
	 * on-disk encodingsBitmap when compared to calculated encodingsBitmap.
	 */
	 if ( (volumeHeader->encodingsBitmap & vcb->vcbEncodingsBitmap) 
	 		!= vcb->vcbEncodingsBitmap ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB encodingsBitmap, disk=0x%qx calculated=0x%qx \n", volumeHeader->encodingsBitmap, vcb->vcbEncodingsBitmap );
		isVHDamaged = 1;
	}
	if ( (UInt16) (hfsPlusIOPosOffset/512)		!= vcb->vcbAlBlSt ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB AlBlSt \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->createDate			!= vcb->vcbCreateDate )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB createDate \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->modifyDate			!= vcb->vcbModifyDate )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB modifyDate \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->backupDate			!= vcb->vcbBackupDate )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB backupDate \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->checkedDate			!= vcb->vcbCheckedDate ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB checkedDate \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->rsrcClumpSize		!= vcb->vcbRsrcClumpSize ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB rsrcClumpSize (VH=%u, vcb=%u)\n", volumeHeader->rsrcClumpSize, vcb->vcbRsrcClumpSize);
		isVHDamaged = 1;
	}
	if ( volumeHeader->dataClumpSize		!= vcb->vcbDataClumpSize ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB dataClumpSize \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->nextCatalogID		!= vcb->vcbNextCatalogID &&
	     (volumeHeader->attributes & kHFSCatalogNodeIDsReused) == 0)  {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB nextCatalogID \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->writeCount			!= vcb->vcbWriteCount )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB writeCount \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->nextAllocation		!= vcb->vcbNextAllocation )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB nextAllocation \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->totalBlocks			!= vcb->vcbTotalBlocks ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB totalBlocks \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->blockSize			!= vcb->vcbBlockSize )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB blockSize \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->attributes			!= vcb->vcbAttributes )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB attributes \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->extentsFile.clumpSize	!= vcb->vcbExtentsFile->fcbClumpSize ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB extentsFile.clumpSize \n" );
		isVHDamaged = 1;
	}
	if ( volumeHeader->allocationFile.clumpSize	!= vcb->vcbAllocationFile->fcbClumpSize ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB allocationFile.clumpSize \n" );
		isVHDamaged = 1;
	}
	if ( (vcb->vcbAttributesFile != NULL) && 
	     (volumeHeader->attributesFile.clumpSize	!= vcb->vcbAttributesFile->fcbClumpSize )) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB attributesFile.clumpSize \n" );
		isVHDamaged = 1;
	}
	if ( CmpBlock( volumeHeader->finderInfo, vcb->vcbFinderInfo, sizeof(vcb->vcbFinderInfo) ) )	{
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) 
			plog( "\tinvalid VHB finderInfo \n" );
		isVHDamaged = 1;
	}

	/*
	 * compare extent file allocation info with VolumeHeader
	 */		
	fcbP = vcb->vcbExtentsFile;
	if ( (UInt64)volumeHeader->extentsFile.totalBlocks * (UInt64)vcb->vcbBlockSize != fcbP->fcbPhysicalSize )
	{
		printMsg = 1;
		WriteError ( GPtr, E_VolumeHeaderDamaged, 3, 0 );
	}
	for ( i=0; i < GPtr->numExtents; i++ )
	{
		if ( (volumeHeader->extentsFile.extents[i].startBlock != fcbP->fcbExtents32[i].startBlock) ||
		     (volumeHeader->extentsFile.extents[i].blockCount != fcbP->fcbExtents32[i].blockCount) )
		{
			printMsg = 1;
			WriteError ( GPtr, E_VolumeHeaderDamaged, 4, 0 );
		}
	}

	/*
	 * compare catalog file allocation info with MDB
	 */	
	fcbP = vcb->vcbCatalogFile;	/* compare PEOF for catalog file */
	if ( (UInt64)volumeHeader->catalogFile.totalBlocks * (UInt64)vcb->vcbBlockSize != fcbP->fcbPhysicalSize )
	{
		printMsg = 1;
		WriteError ( GPtr, E_VolumeHeaderDamaged, 5, 0 );
	}
	for ( i=0; i < GPtr->numExtents; i++ )
	{
		if ( (volumeHeader->catalogFile.extents[i].startBlock != fcbP->fcbExtents32[i].startBlock) ||
		     (volumeHeader->catalogFile.extents[i].blockCount != fcbP->fcbExtents32[i].blockCount) )
		{
			printMsg = 1;
			WriteError ( GPtr, E_VolumeHeaderDamaged, 6, 0 );
		}
	}


	/*
	 * compare bitmap file allocation info with MDB
	 */		
	fcbP = vcb->vcbAllocationFile;
	if ( (UInt64)volumeHeader->allocationFile.totalBlocks * (UInt64)vcb->vcbBlockSize != fcbP->fcbPhysicalSize )
	{
		printMsg = 1;
		WriteError ( GPtr, E_VolumeHeaderDamaged, 7, 0 );
	}
	for ( i=0; i < GPtr->numExtents; i++ )
	{
		if ( (volumeHeader->allocationFile.extents[i].startBlock != fcbP->fcbExtents32[i].startBlock) ||
		     (volumeHeader->allocationFile.extents[i].blockCount != fcbP->fcbExtents32[i].blockCount) )
		{
			printMsg = 1;
			WriteError ( GPtr, E_VolumeHeaderDamaged, 8, 0 );
		}
	}
	
	if (isVHDamaged || printMsg) {
		GPtr->VIStat = GPtr->VIStat | S_MDB;
		if (isVHDamaged)
	        	WriteError ( GPtr, E_VolumeHeaderDamaged, 2, 0 );
	}

	return( noErr );
}

