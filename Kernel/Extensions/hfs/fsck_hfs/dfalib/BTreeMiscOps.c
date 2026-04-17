/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
	File:		BTreeMiscOps.c

	Contains:	Miscellaneous operations for the BTree Module.

	Version:	xxx put the technology version here xxx

	Written by:	Gordon Sheridan and Bill Bruffey

	Copyright:	й 1992-1999 by Apple Computer, Inc., all rights reserved.
*/

#include "BTreePrivate.h"


////////////////////////////// Routine Definitions //////////////////////////////

/*-------------------------------------------------------------------------------
Routine:	CalcKeyRecordSize	-	Return size of combined key/record structure.

Function:	Rounds keySize and recSize so they will end on word boundaries.
			Does NOT add size of offset.

Input:		keySize		- length of key (including length field)
			recSize		- length of record data

Output:		none
			
Result:		UInt16		- size of combined key/record that will be inserted in btree
-------------------------------------------------------------------------------*/

UInt16		CalcKeyRecordSize		(UInt16					 keySize,
									 UInt16					 recSize )
{
	if ( M_IsOdd (keySize) )	keySize += 1;	// pad byte
	
	if (M_IsOdd (recSize) )		recSize += 1;	// pad byte
	
	return	(keySize + recSize);
}



/*-------------------------------------------------------------------------------
Routine:	VerifyHeader	-	Validate fields of the BTree header record.

Function:	Examines the fields of the BTree header record to determine if the
			fork appears to contain a valid BTree.
			
Input:		forkPtr		- pointer to fork control block
			header		- pointer to BTree header
			
			
Result:		noErr		- success
			!= noErr	- failure
-------------------------------------------------------------------------------*/

OSStatus	VerifyHeader	(SFCB				*filePtr,
							 BTHeaderRec		*header )
{
	UInt32		forkSize;
	UInt32		totalNodes;
	

	switch (header->nodeSize)							// node size == 512*2^n
	{
		case   512:
		case  1024:
		case  2048:
		case  4096:
		case  8192:
		case 16384:
		case 32768:		break;
		default:		return	fsBTInvalidHeaderErr;			//ее E_BadNodeType
	}
	
	totalNodes = header->totalNodes;

	forkSize = totalNodes * header->nodeSize;
	
	if ( forkSize != filePtr->fcbLogicalSize )
		return fsBTInvalidHeaderErr;
	
	if ( header->freeNodes >= totalNodes )
		return fsBTInvalidHeaderErr;
	
	if ( header->rootNode >= totalNodes )
		return fsBTInvalidHeaderErr;
	
	if ( header->firstLeafNode >= totalNodes )
		return fsBTInvalidHeaderErr;
	
	if ( header->lastLeafNode >= totalNodes )
		return fsBTInvalidHeaderErr;
	
	if ( header->treeDepth > kMaxTreeDepth )
		return fsBTInvalidHeaderErr;


	/////////////////////////// Check BTree Type ////////////////////////////////
	
	switch (header->btreeType)
	{
		case	0:					// HFS Type - no Key Descriptor
		case	kUserBTreeType:		// with Key Descriptors etc.
		case	kReservedBTreeType:	// Desktop Mgr BTree ?
									break;

		default:					return fsBTUnknownVersionErr;		
	}
	
	return noErr;
}



/*-------------------------------------------------------------------------------
Routine:	UpdateHeader	-	Write BTreeInfoRec fields to Header node.

Function:	Checks the kBTHeaderDirty flag in the BTreeInfoRec and updates the
			header node if necessary.
			
Input:		btreePtr		- pointer to BTreeInfoRec
			
			
Result:		noErr		- success
			!= noErr	- failure
-------------------------------------------------------------------------------*/

OSStatus	UpdateHeader	(BTreeControlBlockPtr		btreePtr)
{
	OSStatus				err;
	BlockDescriptor			node;
	BTHeaderRec				*header;	

	
	if ((btreePtr->flags & kBTHeaderDirty) == 0)			// btree info already flushed
	return	noErr;
	
	
	err = GetNode (btreePtr, kHeaderNodeNum, &node );
	if (err != noErr)
		return	err;
	
	header = (BTHeaderRec*) (node.buffer + sizeof(BTNodeDescriptor));
	
	header->treeDepth		= btreePtr->treeDepth;
	header->rootNode		= btreePtr->rootNode;
	header->leafRecords		= btreePtr->leafRecords;
	header->firstLeafNode	= btreePtr->firstLeafNode;
	header->lastLeafNode	= btreePtr->lastLeafNode;
	header->nodeSize		= btreePtr->nodeSize;			//ее this shouldn't change
	header->maxKeyLength	= btreePtr->maxKeyLength;		//ее neither should this
	header->totalNodes		= btreePtr->totalNodes;
	header->freeNodes		= btreePtr->freeNodes;
	header->btreeType		= btreePtr->btreeType;

	// ignore	header->clumpSize;							//ее rename this field?

	err = UpdateNode (btreePtr, &node);

	btreePtr->flags &= (~kBTHeaderDirty);

	return	err;
}



/*-------------------------------------------------------------------------------
Routine:	FindIteratorPosition	-	One_line_description.

Function:	Brief_description_of_the_function_and_any_side_effects

Algorithm:	see FSC.BT.BTIterateRecord.PICT

Note:		//ее document side-effects of bad node hints

Input:		btreePtr		- description
			iterator		- description
			

Output:		iterator		- description
			left			- description
			middle			- description
			right			- description
			nodeNum			- description
			returnIndex		- description
			foundRecord		- description
			
			
Result:		noErr		- success
			!= noErr	- failure
-------------------------------------------------------------------------------*/

OSStatus	FindIteratorPosition	(BTreeControlBlockPtr	 btreePtr,
									 BTreeIteratorPtr		 iterator,
									 BlockDescriptor		*left,
									 BlockDescriptor		*middle,
									 BlockDescriptor		*right,
									 UInt32					*returnNodeNum,
									 UInt16					*returnIndex,
									 Boolean				*foundRecord )
{
	OSStatus		err;
	Boolean			foundIt;
	UInt32			nodeNum;
	UInt16			leftIndex,	index,	rightIndex;
	Boolean			validHint;

	// assume btreePtr valid
	// assume left, middle, right point to BlockDescriptors
	// assume nodeNum points to UInt32
	// assume index points to UInt16
	// assume foundRecord points to Boolean
	
	left->buffer		= nil;
	middle->buffer		= nil;
	right->buffer		= nil;
	
	foundIt				= false;
	
	if (iterator == nil)						// do we have an iterator?
	{
		err = fsBTInvalidIteratorErr;
		goto ErrorExit;
	}

#if SupportsKeyDescriptors
	//ее verify iterator key (change CheckKey to take btreePtr instead of keyDescPtr?)
	if (btreePtr->keyDescPtr != nil)
	{
		err = CheckKey (&iterator->key, btreePtr->keyDescPtr, btreePtr->maxKeyLength );
		M_ExitOnError (err);
	}
#endif

	err = IsItAHint (btreePtr, iterator, &validHint);
	M_ExitOnError (err);

	nodeNum = iterator->hint.nodeNum;
	if (! validHint)							// does the hint appear to be valid?
	{
		goto SearchTheTree;
	}
	
	err = GetNode (btreePtr, nodeNum, middle);
	if( err == fsBTInvalidNodeErr )	// returned if nodeNum is out of range
		goto SearchTheTree;
		
	M_ExitOnError (err);
	
	if ( ((NodeDescPtr) middle->buffer)->kind		!=	kBTLeafNode ||
		 ((NodeDescPtr) middle->buffer)->numRecords	<=  0 )
	{	
		goto SearchTheTree;
	}
		
	++btreePtr->numValidHints;
	
	foundIt = SearchNode (btreePtr, middle->buffer, &iterator->key, &index);
	if (foundIt == true)
	{
		goto SuccessfulExit;
	}
	
	if (index == 0)
	{
		if (((NodeDescPtr) middle->buffer)->bLink == 0)		// before 1st btree record
		{
			goto SuccessfulExit;
		}
		
		nodeNum = ((NodeDescPtr) middle->buffer)->bLink;
		
		err = GetLeftSiblingNode (btreePtr, middle->buffer, left);
		M_ExitOnError (err);
		
		if ( ((NodeDescPtr) left->buffer)->kind			!=	kBTLeafNode ||
			 ((NodeDescPtr) left->buffer)->numRecords	<=  0 )
		{	
			goto SearchTheTree;
		}
		
		foundIt = SearchNode (btreePtr, left->buffer, &iterator->key, &leftIndex);
		if (foundIt == true)
		{
			*right			= *middle;
			*middle			= *left;
			left->buffer	= nil;
			index			= leftIndex;
			
			goto SuccessfulExit;
		}
		
		if (leftIndex == 0)									// we're lost!
		{
			goto SearchTheTree;
		}
		else if (leftIndex >= ((NodeDescPtr) left->buffer)->numRecords)
		{
			nodeNum = ((NodeDescPtr) left->buffer)->fLink;
			
			PanicIf (index != 0, "\pFindIteratorPosition: index != 0");	//ее just checking...
			goto SuccessfulExit;
		}
		else
		{
			*right			= *middle;
			*middle			= *left;
			left->buffer	= nil;
			index			= leftIndex;
			
			goto SuccessfulExit;
		}
	}
	else if (index >= ((NodeDescPtr) middle->buffer)->numRecords)
	{
		if (((NodeDescPtr) middle->buffer)->fLink == 0)	// beyond last record
		{
			goto SuccessfulExit;
		}
		
		nodeNum = ((NodeDescPtr) middle->buffer)->fLink;
		
		err = GetRightSiblingNode (btreePtr, middle->buffer, right);
		M_ExitOnError (err);
		
		if ( ((NodeDescPtr) right->buffer)->kind		!= kBTLeafNode ||
			 ((NodeDescPtr) right->buffer)->numRecords	<=  0 )
		{	
			goto SearchTheTree;
		}

		foundIt = SearchNode (btreePtr, right->buffer, &iterator->key, &rightIndex);
		if (rightIndex >= ((NodeDescPtr) right->buffer)->numRecords)		// we're lost
		{
			goto SearchTheTree;
		}
		else	// we found it, or rightIndex==0, or rightIndex<numRecs
		{
			*left			= *middle;
			*middle			= *right;
			right->buffer	= nil;
			index			= rightIndex;
			
			goto SuccessfulExit;
		}
	}

	
	//////////////////////////// Search The Tree ////////////////////////////////	

SearchTheTree:
	{
		TreePathTable	treePathTable;		// so we only use stack space if we need to

		err = ReleaseNode (btreePtr, left);			M_ExitOnError (err);
		err = ReleaseNode (btreePtr, middle);		M_ExitOnError (err);
		err = ReleaseNode (btreePtr, right);		M_ExitOnError (err);
	
		err = SearchTree ( btreePtr, &iterator->key, treePathTable, &nodeNum, middle, &index);
		switch (err)				//ее separate find condition from exceptions
		{
			case noErr:			foundIt = true;				break;
			case fsBTRecordNotFoundErr:						break;
			default:				goto ErrorExit;
		}
	}

	/////////////////////////////// Success! ////////////////////////////////////

SuccessfulExit:
	
	*returnNodeNum	= nodeNum;
	*returnIndex 	= index;
	*foundRecord	= foundIt;
	
	return	noErr;
	
	
	////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:

	(void)	ReleaseNode (btreePtr, left);
	(void)	ReleaseNode (btreePtr, middle);
	(void)	ReleaseNode (btreePtr, right);

	*returnNodeNum	= 0;
	*returnIndex 	= 0;
	*foundRecord	= false;

	return	err;
}



/////////////////////////////// CheckInsertParams ///////////////////////////////

OSStatus	CheckInsertParams		(SFCB						*filePtr,
									 BTreeIterator				*iterator,
									 FSBufferDescriptor			*record,
									 UInt16						 recordLen )
{
	BTreeControlBlockPtr	btreePtr;
	
	if (filePtr == nil)									return	paramErr;

	btreePtr = (BTreeControlBlockPtr) filePtr->fcbBtree;
	if (btreePtr == nil)								return	fsBTInvalidFileErr;
	if (iterator == nil)								return	paramErr;
	if (record	 == nil)								return	paramErr;
	
#if SupportsKeyDescriptors
	if (btreePtr->keyDescPtr != nil)
	{
		OSStatus	err;

		err = CheckKey (&iterator->key, btreePtr->keyDescPtr, btreePtr->maxKeyLength);
		if (err != noErr)
			return	err;
	}
#endif

	//	check total key/record size limit
	if ( CalcKeyRecordSize (CalcKeySize(btreePtr, &iterator->key), recordLen) > (btreePtr->nodeSize >> 1))
		return	fsBTRecordTooLargeErr;
	
	return	noErr;
}



/*-------------------------------------------------------------------------------
Routine:	TrySimpleReplace	-	Attempts a simple insert, set, or replace.

Function:	If a hint exitst for the iterator, attempt to find the key in the hint
			node. If the key is found, an insert operation fails. If the is not
			found, a replace operation fails. If the key was not found, and the
			insert position is greater than 0 and less than numRecords, the record
			is inserted, provided there is enough freeSpace.  If the key was found,
			and there is more freeSpace than the difference between the new record
			and the old record, the old record is deleted and the new record is
			inserted.

Assumptions:	iterator key has already been checked by CheckKey


Input:		btreePtr		- description
			iterator		- description
			record			- description
			recordLen		- description
			operation		- description
			

Output:		recordInserted		- description
			
						
Result:		noErr			- success
			E_RecordExits		- insert operation failure
			!= noErr		- GetNode, ReleaseNode, UpdateNode returned an error
-------------------------------------------------------------------------------*/

OSStatus	TrySimpleReplace		(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr			 nodePtr,
									 BTreeIterator			*iterator,
									 FSBufferDescriptor		*record,
									 UInt16					 recordLen,
									 Boolean				*recordInserted )
{
	UInt32				oldSpace;
	UInt32				spaceNeeded;
	UInt16				index;
	UInt16				keySize;
	Boolean				foundIt;
	Boolean				didItFit;
	
	
	*recordInserted	= false;								// we'll assume this won't work...
	
	if ( nodePtr->kind != kBTLeafNode )
		return	noErr;	// we're in the weeds!

	foundIt	= SearchNode (btreePtr, nodePtr, &iterator->key, &index);	

	if ( foundIt == false )
		return	noErr;	// we might be lost...
		
	keySize = CalcKeySize(btreePtr, &iterator->key);	// includes length field
	
	spaceNeeded	= CalcKeyRecordSize (keySize, recordLen);
	
	oldSpace = GetRecordSize (btreePtr, nodePtr, index);
	
	if ( spaceNeeded == oldSpace )
	{
		UInt8 *		dst;

		dst = GetRecordAddress (btreePtr, nodePtr, index);

		if ( M_IsOdd (keySize) )
			++keySize;			// add pad byte
		
		dst += keySize;		// skip over key to point at record

		CopyMemory(record->bufferAddress, dst, recordLen);	// blast away...

		*recordInserted = true;
	}
	else if ( (GetNodeFreeSize(btreePtr, nodePtr) + oldSpace) >= spaceNeeded)
	{
		DeleteRecord (btreePtr, nodePtr, index);
	
		didItFit = InsertKeyRecord (btreePtr, nodePtr, index,
										&iterator->key, KeyLength(btreePtr, &iterator->key),
										record->bufferAddress, recordLen);
		PanicIf (didItFit == false, "\pTrySimpleInsert: InsertKeyRecord returned false!");

		*recordInserted = true;
	}
	// else not enough space...

	return	noErr;
}


/*-------------------------------------------------------------------------------
Routine:	IsItAHint	-	checks the hint within a BTreeInterator.

Function:	checks the hint within a BTreeInterator.  If it is non-zero, it may 
			possibly be valid. 

Input:		btreePtr	- pointer to control block for BTree file
			iterator	- pointer to BTreeIterator
			
Output:		answer		- true if the hint looks reasonable
						- false if the hint is 0
			
Result:		noErr			- success
-------------------------------------------------------------------------------*/


OSStatus	IsItAHint	(BTreeControlBlockPtr btreePtr, BTreeIterator *iterator, Boolean *answer)
{
	++btreePtr->numHintChecks;
	
	//ее shouldn't we do a range check?
	if (iterator->hint.nodeNum == 0)
	{
		*answer = false;
	}
	else
	{
		*answer = true;
		++btreePtr->numPossibleHints;
	}
	
	return noErr;
}
