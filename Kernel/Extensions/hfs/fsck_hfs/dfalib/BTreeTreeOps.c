/*
 * Copyright (c) 1999-2000, 2002, 2007-2008 Apple Inc. All rights reserved.
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
	File:		BTreeTreeOps.c

	Contains:	Multi-node tree operations for the BTree Module.

	Version:	xxx put the technology version here xxx

	Written by:	Gordon Sheridan and Bill Bruffey

	Copyright:	й 1992-1997, 1999 by Apple Computer, Inc., all rights reserved.
*/

#include "BTreePrivate.h"
#include "../fsck_debug.h"
extern char debug;
extern void plog(const char *fmt, ...);

#define DEBUG_TREEOPS 0

/////////////////////// Routines Internal To BTree Module ///////////////////////
//
//	SearchTree
//	InsertTree
//
////////////////////// Routines Internal To BTreeTreeOps.c //////////////////////

static OSStatus   AddNewRootNode	(BTreeControlBlockPtr		 btreePtr,
									 NodeDescPtr				 leftNode,
									 NodeDescPtr				 rightNode );

static OSStatus   CollapseTree		(BTreeControlBlockPtr		 btreePtr,
									 BlockDescriptor			*blockPtr );

static OSStatus   RotateLeft		(BTreeControlBlockPtr		 btreePtr,
									 NodeDescPtr				 leftNode,
									 NodeDescPtr				 rightNode,
									 UInt16						 rightInsertIndex,
									 KeyPtr						 keyPtr,
									 UInt8 *					 recPtr,
									 UInt16						 recSize,
									 UInt16						*insertIndex,
									 UInt32						*insertNodeNum,
									 Boolean					*recordFit,
									 UInt16						*recsRotated );

static Boolean	   RotateRecordLeft	(BTreeControlBlockPtr		 btreePtr,
									 NodeDescPtr				 leftNode,
									 NodeDescPtr				 rightNode );

#if 0 
static OSStatus	   SplitLeft		(BTreeControlBlockPtr		 btreePtr,
									 BlockDescriptor			*leftNode,
									 BlockDescriptor			*rightNode,
									 UInt32						 rightNodeNum,
									 UInt16						 index,
									 KeyPtr						 keyPtr,
									 UInt8 *					 recPtr,
									 UInt16						 recSize,
									 UInt16						*insertIndex,
									 UInt32						*insertNodeNum,
									 UInt16						*recsRotated );
#endif


static	OSStatus	InsertLevel		(BTreeControlBlockPtr		 btreePtr,
									 TreePathTable				 treePathTable,
									 InsertKey					*primaryKey,
									 InsertKey					*secondaryKey,
									 BlockDescriptor			*targetNode,
									 UInt16						 index,
									 UInt16						 level,
									 UInt32						*insertNode );
						 
static OSErr		InsertNode 		(BTreeControlBlockPtr		 btreePtr,
									 InsertKey					*key,
									 BlockDescriptor			*targetNode,
									 UInt32						 node,
									 UInt16	 					 index,
									 UInt32						*newNode,	
									 UInt16						*newIndex,
									 BlockDescriptor			*leftNode,
									 Boolean					*updateParent,
									 Boolean					*insertParent,
									 Boolean					*rootSplit );
									 
static UInt16		GetKeyLength	(const BTreeControlBlock *btreePtr,
									 const BTreeKey *key,
									 Boolean forLeafNode );

static Boolean 		RotateRecordRight( 	BTreeControlBlockPtr		btreePtr,
								 		NodeDescPtr				leftNode,
							 	 		NodeDescPtr				rightNode );
							 	 		
static OSStatus		RotateRight(	BTreeControlBlockPtr		 btreePtr,
								 	NodeDescPtr					 leftNode,
								 	NodeDescPtr					 rightNode,
								 	UInt16						 leftInsertIndex,
									KeyPtr						 keyPtr,
									UInt8 *						 recPtr,
									UInt16						 recSize,
									UInt16						*insertIndex,
									UInt32						*insertNodeNum,
								 	Boolean						*recordFit,
									UInt16						*recsRotated );
									
static OSStatus		SplitRight(		BTreeControlBlockPtr		 btreePtr,
								 	BlockDescriptor				*nodePtr,
									BlockDescriptor				*rightNodePtr,
									UInt32						 nodeNum,
									UInt16						 index,
									KeyPtr						 keyPtr,
									UInt8  						*recPtr,
									UInt16						 recSize,
									UInt16						*insertIndexPtr,
									UInt32						*newNodeNumPtr,
									UInt16						*recsRotatedPtr );

#if DEBUG_TREEOPS
static int DoKeyCheck( NodeDescPtr nodeP, BTreeControlBlock *btcb );
static int	DoKeyCheckAcrossNodes( 	NodeDescPtr theLeftNodePtr, 
									NodeDescPtr theRightNodePtr, 
									BTreeControlBlock *theTreePtr,
									Boolean printKeys );
static void PrintNodeDescriptor( NodeDescPtr  thePtr );
static void PrintKey( UInt8 *  myPtr, int mySize );
#endif // DEBUG_TREEOPS


/* used by InsertLevel (and called routines) to communicate the state of an insert operation */
enum
{
	kInsertParent 			= 0x0001,
	kUpdateParent 			= 0x0002,
	kNewRoot	 			= 0x0004,
	kInsertedInRight 		= 0x0008,
	kRecordFits		 		= 0x0010
};


//////////////////////// BTree Multi-node Tree Operations ///////////////////////


/*-------------------------------------------------------------------------------

Routine:	SearchTree	-	Search BTree for key and set up Tree Path Table.

Function:	Searches BTree for specified key, setting up the Tree Path Table to
			reflect the search path.


Input:		btreePtr		- pointer to control block of BTree to search
			keyPtr			- pointer to the key to search for
			treePathTable	- pointer to the tree path table to construct
			
Output:		nodeNum			- number of the node containing the key position
			iterator		- BTreeIterator specifying record or insert position
			
Result:		noErr			- key found, index is record index
			fsBTRecordNotFoundErr	- key not found, index is insert index
			fsBTEmptyErr		- key not found, return params are nil
			otherwise			- catastrophic failure (GetNode/ReleaseNode failed)
-------------------------------------------------------------------------------*/

OSStatus	SearchTree	(BTreeControlBlockPtr	 btreePtr,
						 BTreeKeyPtr			 searchKey,
						 TreePathTable			 treePathTable,
						 UInt32					*nodeNum,
						 BlockDescriptor		*nodePtr,
						 UInt16					*returnIndex )
{
	OSStatus	err;
	SInt16		level;
	UInt32		curNodeNum;
	NodeRec		nodeRec;
	UInt16		index;
	Boolean		keyFound;
	SInt8		nodeKind;				//	Kind of current node (index/leaf)
	KeyPtr		keyPtr;
	UInt8 *		dataPtr;
	UInt16		dataSize;
	
	
	curNodeNum		= btreePtr->rootNode;
	level			= btreePtr->treeDepth;
	
	if (level == 0)						// is the tree empty?
	{
		err = fsBTEmptyErr;
		goto ErrorExit;
	}
	
	//ее for debugging...
	treePathTable [0].node		= 0;
	treePathTable [0].index		= 0;

	while (true)
	{
        //
        //	[2550929] Node number 0 is the header node.  It is never a valid
        //	index or leaf node.  If we're ever asked to search through node 0,
        //	something has gone wrong (typically a bad child node number, or
        //	we found a node full of zeroes that we thought was an index node).
        //
        if (curNodeNum == 0)
        {
//          Panic("\pSearchTree: curNodeNum is zero!");
			if (debug) fprintf(stderr, "%s(%d):  curNodeNum is 0\n", __FUNCTION__, __LINE__);
            err = fsBTInvalidNodeErr;
            goto ErrorExit;
        }

		err = GetNode (btreePtr, curNodeNum, &nodeRec);
		if (err != noErr)
		{
			goto ErrorExit;
		}

        //
        //	[2550929] Sanity check the node height and node type.  We expect
        //	particular values at each iteration in the search.  This checking
        //	quickly finds bad pointers, loops, and other damage to the
        //	hierarchy of the B-tree.
        //
        if (((BTNodeDescriptor*)nodeRec.buffer)->height != level)
        {
				if (debug)
				{
					fprintf(stderr, "%s(line %d):  height %d != level %d\n", __FUNCTION__, __LINE__, ((BTNodeDescriptor*)nodeRec.buffer)->height, level);
					fprintf(stderr, "    curNodeNum = %u\n", curNodeNum);
					if (cur_debug_level & d_dump_node)
					{
						HexDump(nodeRec.buffer, nodeRec.blockSize, true);
					}
                }
                err = fsBTInvalidNodeErr;
                goto ReleaseAndExit;
        }
        nodeKind = ((BTNodeDescriptor*)nodeRec.buffer)->kind;
        if (level == 1)
        {
            //	Nodes at level 1 must be leaves, by definition
            if (nodeKind != kBTLeafNode)
            {
				if (debug) fprintf(stderr, "%s(%d):  wrong kind of node\n", __FUNCTION__, __LINE__);
                err = fsBTInvalidNodeErr;
                goto ReleaseAndExit;           
            }
        }
        else
        {
            //	A node at any other depth must be an index node
            if (nodeKind != kBTIndexNode)
            {
				if (debug) fprintf(stderr, "%s(%d):  other wrong kind of node\n", __FUNCTION__, __LINE__);
                err = fsBTInvalidNodeErr;
                goto ReleaseAndExit;
            }
        }
        		
		keyFound = SearchNode (btreePtr, nodeRec.buffer, searchKey, &index);

		treePathTable [level].node		= curNodeNum;

        if (nodeKind == kBTLeafNode)
		{
			treePathTable [level].index = index;
			break;			// were done...
		}
		
		if ( (keyFound != true) && (index != 0))
			--index;

		treePathTable [level].index = index;
		
		err = GetRecordByIndex (btreePtr, nodeRec.buffer, index, &keyPtr, &dataPtr, &dataSize);
        if (err != noErr)
        {
            //	[2550929] If we got an error, it is probably because the index was bad
            //	(typically a corrupt node that confused SearchNode).  Invalidate the node
            //	so we won't accidentally use the corrupted contents.  NOTE: the Mac OS 9
            //	sources call this InvalidateNode.
            
                (void) TrashNode(btreePtr, &nodeRec);
                goto ErrorExit;
        }

        //	Get the child pointer out of this index node.  We're now done with the current
        //	node and can continue the search with the child node.
		curNodeNum = *(UInt32 *)dataPtr;
		err = ReleaseNode (btreePtr, &nodeRec);
		if (err != noErr)
		{
			goto ErrorExit;
		}
        
        //	The child node should be at a level one less than the parent.
        --level;
	}
	
	*nodeNum			= curNodeNum;
	*nodePtr			= nodeRec;
	*returnIndex		= index;

	if (keyFound)
		return	noErr;			// searchKey found, index identifies record in node
	else
		return	fsBTRecordNotFoundErr;	// searchKey not found, index identifies insert point

ReleaseAndExit:
    (void) ReleaseNode(btreePtr, &nodeRec);
    //	fall into ErrorExit

ErrorExit:
	
	*nodeNum					= 0;
	nodePtr->buffer				= nil;
	nodePtr->blockHeader		= nil;
	*returnIndex				= 0;

	return	err;
}




////////////////////////////////// InsertTree ///////////////////////////////////

OSStatus	InsertTree ( BTreeControlBlockPtr		 btreePtr,
						 TreePathTable				 treePathTable,
						 KeyPtr						 keyPtr,
						 UInt8 *					 recPtr,
						 UInt16						 recSize,
						 BlockDescriptor			*targetNode,
						 UInt16						 index,
						 UInt16						 level,
						 Boolean					 replacingKey,
						 UInt32						*insertNode )
{
	InsertKey			primaryKey;
	OSStatus			err;

	primaryKey.keyPtr		= keyPtr;
	primaryKey.keyLength	= GetKeyLength(btreePtr, primaryKey.keyPtr, (level == 1));
	primaryKey.recPtr		= recPtr;
	primaryKey.recSize		= recSize;
	primaryKey.replacingKey	= replacingKey;
	primaryKey.skipRotate	= false;

	err	= InsertLevel (btreePtr, treePathTable, &primaryKey, nil,
					   targetNode, index, level, insertNode );
						
	return err;

} // End of InsertTree


////////////////////////////////// InsertLevel //////////////////////////////////

OSStatus	InsertLevel (BTreeControlBlockPtr		 btreePtr,
						 TreePathTable				 treePathTable,
						 InsertKey					*primaryKey,
						 InsertKey					*secondaryKey,
						 BlockDescriptor			*targetNode,
						 UInt16						 index,
						 UInt16						 level,
						 UInt32						*insertNode )
{
	OSStatus			 err;
	BlockDescriptor		 siblingNode;
	UInt32				 targetNodeNum;
	UInt32				 newNodeNum;
	UInt16				 newIndex;
	Boolean				 insertParent;
	Boolean				 updateParent;
	Boolean				 newRoot;

#if defined(applec) && !defined(__SC__)
	PanicIf ((level == 1) && (((NodeDescPtr)targetNode->buffer)->kind != kBTLeafNode), "\P InsertLevel: non-leaf at level 1! ");
#endif
	siblingNode.buffer = nil;
	targetNodeNum = treePathTable [level].node;

	insertParent = false;
	updateParent = false;
	
	////// process first insert //////
	
	err = InsertNode (btreePtr, primaryKey, targetNode, targetNodeNum, index,
					  &newNodeNum, &newIndex, &siblingNode, &updateParent, &insertParent, &newRoot );
	M_ExitOnError (err);

	if ( newRoot )
	{
		// Extend the treePathTable by adding an entry for the new
		// root node that references the current targetNode.
		// 
		// When we split right the index in the new root is 0 if the new
		// node is the same as the target node or 1 otherwise.  When the
		// new node number and the target node number are the same then
		// we inserted the new record into the left node (the orignial target)
		// after the split.

		treePathTable [level + 1].node  = btreePtr->rootNode;
		if ( targetNodeNum == newNodeNum )
			treePathTable [level + 1].index = 0;  // 
		else
			treePathTable [level + 1].index = 1;
	}
	
	if ( level == 1 )
		*insertNode = newNodeNum;		
	
	////// process second insert (if any) //////

	if  ( secondaryKey != nil )
	{
		Boolean				temp;

		// NOTE - we only get here if we have split a child node to the right and 
		// we are currently updating the child's parent.   newIndex + 1 refers to
		// the location in the parent node where we insert the new index record that
		// represents the new child node (the new right node). 
		err = InsertNode (btreePtr, secondaryKey, targetNode, newNodeNum, newIndex + 1,
						  &newNodeNum, &newIndex, &siblingNode, &updateParent, &insertParent, &temp);
		M_ExitOnError (err);
		
		if ( DEBUG_BUILD && updateParent && newRoot )
			DebugStr("\p InsertLevel: New root from primary key, update from secondary key...");
	}

	//////////////////////// Update Parent(s) ///////////////////////////////

	if ( insertParent || updateParent )
	{
		BlockDescriptor		parentNode;
		UInt32				parentNodeNum;
		KeyPtr				keyPtr;
		UInt8 *				recPtr;
		UInt16				recSize;
		
		secondaryKey = nil;
		
		PanicIf ( (level == btreePtr->treeDepth), "\p InsertLevel: unfinished insert!?");

		++level;

		// Get Parent Node data...
		index = treePathTable [level].index;
		parentNodeNum = treePathTable [level].node;

		PanicIf ( parentNodeNum == 0, "\p InsertLevel: parent node is zero!?");

		err = GetNode (btreePtr, parentNodeNum, &parentNode);	// released as target node in next level up
		M_ExitOnError (err);
#if defined(applec) && !defined(__SC__)
		if (DEBUG_BUILD && level > 1)
			PanicIf ( ((NodeDescPtr)parentNode.buffer)->kind != kBTIndexNode, "\P InsertLevel: parent node not an index node! ");
#endif
		////////////////////////// Update Parent Index //////////////////////////////
	
		if ( updateParent )
		{
			//ее╩debug: check if ptr == targetNodeNum
			GetRecordByIndex (btreePtr, parentNode.buffer, index, &keyPtr, &recPtr, &recSize);
			PanicIf( (*(UInt32 *) recPtr) != targetNodeNum, "\p InsertLevel: parent ptr doesn't match target node!");
			
			// need to delete and re-insert this parent key/ptr
			// we delete it here and it gets re-inserted in the
			// InsertLevel call below.
			DeleteRecord (btreePtr, parentNode.buffer, index);
	
			primaryKey->keyPtr		 = (KeyPtr) GetRecordAddress( btreePtr, targetNode->buffer, 0 );
			primaryKey->keyLength	 = GetKeyLength(btreePtr, primaryKey->keyPtr, false);
			primaryKey->recPtr		 = (UInt8 *) &targetNodeNum;
			primaryKey->recSize		 = sizeof(targetNodeNum);
			primaryKey->replacingKey = kReplaceRecord;
			primaryKey->skipRotate   = insertParent;		// don't rotate left if we have two inserts occuring
		}
	
		////////////////////////// Add New Parent Index /////////////////////////////
	
		if ( insertParent )
		{
			InsertKey	*insertKeyPtr;
			InsertKey	insertKey;
			
			if ( updateParent )
			{
				insertKeyPtr = &insertKey;
				secondaryKey = &insertKey;
			}
			else
			{
				insertKeyPtr = primaryKey;
				// split right but not updating parent for our left node then 
				// we want to insert the key for the new right node just after 
				// the key for our left node.
				index++;
			}
			
			insertKeyPtr->keyPtr		= (KeyPtr) GetRecordAddress (btreePtr, siblingNode.buffer, 0);
			insertKeyPtr->keyLength		= GetKeyLength(btreePtr, insertKeyPtr->keyPtr, false);
			insertKeyPtr->recPtr		= (UInt8 *) &((NodeDescPtr)targetNode->buffer)->fLink;
			insertKeyPtr->recSize		= sizeof(UInt32);
			insertKeyPtr->replacingKey	= kInsertRecord;
			insertKeyPtr->skipRotate	= false;		// a rotate is OK during second insert
		}	
		
		err = InsertLevel (btreePtr, treePathTable, primaryKey, secondaryKey,
						   &parentNode, index, level, insertNode );
		M_ExitOnError (err);
	}

	err = UpdateNode (btreePtr, targetNode);	// all done with target
	M_ExitOnError (err);

	err = UpdateNode (btreePtr, &siblingNode);		// all done with left sibling
	M_ExitOnError (err);

	return	noErr;

ErrorExit:

	(void) ReleaseNode (btreePtr, targetNode);
	(void) ReleaseNode (btreePtr, &siblingNode);

	Panic ("\p InsertLevel: an error occured!");

	return	err;

} // End of InsertLevel



////////////////////////////////// InsertNode ///////////////////////////////////

static OSErr	InsertNode	(BTreeControlBlockPtr	 btreePtr,
							 InsertKey				*key,

							 BlockDescriptor		*targetNode,
							 UInt32					 nodeNum,
							 UInt16	 				 index,

							 UInt32					*newNodeNumPtr,	
							 UInt16					*newIndex,

							 BlockDescriptor		*siblingNode,
							 Boolean				*updateParent,
							 Boolean				*insertParent,
							 Boolean				*rootSplit )
{
	BlockDescriptor		*tempNode;
	UInt32				 leftNodeNum;
	UInt32				 rightNodeNum;
	UInt16				 recsRotated;
	OSErr				 err;
	Boolean				 recordFit;

	*rootSplit = false;
	
	PanicIf ( targetNode->buffer == siblingNode->buffer, "\p InsertNode: targetNode == siblingNode, huh?");
	
	leftNodeNum = ((NodeDescPtr) targetNode->buffer)->bLink;
	rightNodeNum = ((NodeDescPtr) targetNode->buffer)->fLink;


	/////////////////////// Try Simple Insert ///////////////////////////////

	if ( nodeNum == leftNodeNum )
		tempNode = siblingNode;
	else
		tempNode = targetNode;

	recordFit = InsertKeyRecord (btreePtr, tempNode->buffer, index, key->keyPtr, key->keyLength, key->recPtr, key->recSize);

	if ( recordFit )
	{
		*newNodeNumPtr  = nodeNum;
		*newIndex = index;
	
#if DEBUG_TREEOPS
		if ( DoKeyCheck( tempNode->buffer, btreePtr ) != noErr )
		{
			plog( "\n%s - bad key order in node num %d: \n", __FUNCTION__ , nodeNum );
			PrintNodeDescriptor( tempNode->buffer );
			err = fsBTBadRotateErr;
			goto ErrorExit;
		}
#endif // DEBUG_TREEOPS

		if ( (index == 0) && (((NodeDescPtr) tempNode->buffer)->height != btreePtr->treeDepth) )
			*updateParent = true;	// the first record changed so we need to update the parent
		goto ExitThisRoutine;
	}


	//////////////////////// Try Rotate Left ////////////////////////////////
	
	if ( leftNodeNum > 0 )
	{
		PanicIf ( siblingNode->buffer != nil, "\p InsertNode: siblingNode already aquired!");

		if ( siblingNode->buffer == nil )
		{
			err = GetNode (btreePtr, leftNodeNum, siblingNode);	// will be released by caller or a split below
			M_ExitOnError (err);
		}

		PanicIf ( ((NodeDescPtr) siblingNode->buffer)->fLink != nodeNum, "\p InsertNode, RotateLeft: invalid sibling link!" );

		if ( !key->skipRotate )		// are rotates allowed?
		{
			err = RotateLeft (btreePtr, siblingNode->buffer, targetNode->buffer, index, key->keyPtr, key->recPtr,
							  key->recSize, newIndex, newNodeNumPtr, &recordFit, &recsRotated );	
			M_ExitOnError (err);

			if ( recordFit )
			{
				if ( key->replacingKey || (recsRotated > 1) || (index > 0) )
					*updateParent = true;			
				goto ExitThisRoutine;
			}
		}
	}	


	//////////////////////// Try Split Right /////////////////////////////////

	(void) ReleaseNode( btreePtr, siblingNode );
	err = SplitRight( btreePtr, targetNode, siblingNode, nodeNum, index, key->keyPtr,
					  key->recPtr, key->recSize, newIndex, newNodeNumPtr, &recsRotated );
	M_ExitOnError (err);

	// if we split root node - add new root
	if ( ((NodeDescPtr) targetNode->buffer)->height == btreePtr->treeDepth )
	{
		err = AddNewRootNode( btreePtr, targetNode->buffer, siblingNode->buffer );	// Note: does not update TPT
		M_ExitOnError (err);
		*rootSplit = true;
	}
	else
	{
		*insertParent = true;

		// update parent index node when replacingKey is true or when
		// we inserted a new record at the beginning of our target node.
		if ( key->replacingKey || ( index == 0 && *newIndex == 0 ) )
			*updateParent = true;
	}
	
ExitThisRoutine:

	return noErr;

ErrorExit:

	(void) ReleaseNode (btreePtr, siblingNode);
	return err;
	
} // End of InsertNode


/*-------------------------------------------------------------------------------
Routine:	DeleteTree	-	One_line_description.

Function:	Brief_description_of_the_function_and_any_side_effects

ToDo:		

Input:		btreePtr		- description
			treePathTable	- description
			targetNode		- description
			index			- description
						
Result:		noErr		- success
			!= noErr	- failure
-------------------------------------------------------------------------------*/

OSStatus	DeleteTree			(BTreeControlBlockPtr		 btreePtr,
								 TreePathTable				 treePathTable,
								 BlockDescriptor			*targetNode,
								 UInt16						 index,
								 UInt16						 level )
{
	OSStatus			err;
	BlockDescriptor		parentNode;
	BTNodeDescriptor	*targetNodePtr;
	UInt32				targetNodeNum;
	Boolean				deleteRequired;
	Boolean				updateRequired;


	deleteRequired = false;
	updateRequired = false;

	targetNodeNum = treePathTable[level].node;
	targetNodePtr = targetNode->buffer;
	PanicIf (targetNodePtr == nil, "\pDeleteTree: targetNode has nil buffer!");

	DeleteRecord (btreePtr, targetNodePtr, index);
		
	//ее coalesce remaining records?

	if ( targetNodePtr->numRecords == 0 )	// did we delete the last record?
	{
		BlockDescriptor		siblingNode;
		UInt32				siblingNodeNum;

		deleteRequired = true;
		
		////////////////// Get Siblings & Update Links //////////////////////////
		
		siblingNodeNum = targetNodePtr->bLink;				// Left Sibling Node
		if ( siblingNodeNum != 0 )
		{
			err = GetNode (btreePtr, siblingNodeNum, &siblingNode);
			M_ExitOnError (err);
			((NodeDescPtr)siblingNode.buffer)->fLink = targetNodePtr->fLink;
			err = UpdateNode (btreePtr, &siblingNode);
			M_ExitOnError (err);
		}
		else if ( targetNodePtr->kind == kBTLeafNode )		// update firstLeafNode
		{
			btreePtr->firstLeafNode = targetNodePtr->fLink;
		}

		siblingNodeNum = targetNodePtr->fLink;				// Right Sibling Node
		if ( siblingNodeNum != 0 )
		{
			err = GetNode (btreePtr, siblingNodeNum, &siblingNode);
			M_ExitOnError (err);
			((NodeDescPtr)siblingNode.buffer)->bLink = targetNodePtr->bLink;
			err = UpdateNode (btreePtr, &siblingNode);
			M_ExitOnError (err);
		}
		else if ( targetNodePtr->kind == kBTLeafNode )		// update lastLeafNode
		{
			btreePtr->lastLeafNode = targetNodePtr->bLink;
		}
		
		//////////////////////// Free Empty Node ////////////////////////////////

		ClearNode (btreePtr, targetNodePtr);
		
		err = UpdateNode (btreePtr, targetNode);
		M_ExitOnError (err);
		err = FreeNode (btreePtr, targetNodeNum);
		M_ExitOnError (err);
	}
	else if ( index == 0 )			// did we delete the first record?
	{
		updateRequired = true;		// yes, so we need to update parent
	}


	if ( level == btreePtr->treeDepth )		// then targetNode->buffer is the root node
	{
		deleteRequired = false;
		updateRequired = false;
		
		if ( targetNode->buffer == nil )	// then root was freed and the btree is empty
		{
			btreePtr->rootNode  = 0;
			btreePtr->treeDepth = 0;
		}
		else if ( ((NodeDescPtr)targetNode->buffer)->numRecords == 1 )
		{
			err = CollapseTree (btreePtr, targetNode);
			M_ExitOnError (err);
		}
	}


	if ( updateRequired || deleteRequired )
	{
		++level;	// next level

		//// Get Parent Node and index
		index = treePathTable [level].index;
		err = GetNode (btreePtr, treePathTable[level].node, &parentNode);
		M_ExitOnError (err);

		if ( updateRequired )
		{
			 KeyPtr		keyPtr;
			 UInt8 *	recPtr;
			 UInt16		recSize;
			 UInt32		insertNode;
			 
			//ее╩debug: check if ptr == targetNodeNum
			GetRecordByIndex (btreePtr, parentNode.buffer, index, &keyPtr, &recPtr, &recSize);
			PanicIf( (*(UInt32 *) recPtr) != targetNodeNum, "\p DeleteTree: parent ptr doesn't match targetNodeNum!!");
			
			// need to delete and re-insert this parent key/ptr
			DeleteRecord (btreePtr, parentNode.buffer, index);
	
			keyPtr = (KeyPtr) GetRecordAddress( btreePtr, targetNode->buffer, 0 );
			recPtr = (UInt8 *) &targetNodeNum;
			recSize = sizeof(targetNodeNum);
			
			err = InsertTree (btreePtr, treePathTable, keyPtr, recPtr, recSize,
							  &parentNode, index, level, kReplaceRecord, &insertNode);
			M_ExitOnError (err);
		}
		else // deleteRequired
		{
			err = DeleteTree (btreePtr, treePathTable, &parentNode, index, level);
			M_ExitOnError (err);
		}
	}	


	err = UpdateNode (btreePtr, targetNode);
	M_ExitOnError (err);

	return	noErr;

ErrorExit:
	
	(void) ReleaseNode (btreePtr, targetNode);
	(void) ReleaseNode (btreePtr, &parentNode);

	return	err;

} // end DeleteTree



///////////////////////////////// CollapseTree //////////////////////////////////

static OSStatus	CollapseTree	(BTreeControlBlockPtr		btreePtr,
							 	 BlockDescriptor			*blockPtr )
{
	OSStatus		err;
	UInt32			originalRoot;
	UInt32			nodeNum;
	
	originalRoot	= btreePtr->rootNode;
	
	while (true)
	{
		if ( ((NodeDescPtr)blockPtr->buffer)->numRecords > 1)
			break;							// this will make a fine root node
		
		if ( ((NodeDescPtr)blockPtr->buffer)->kind == kBTLeafNode)
			break;							// we've hit bottom
		
		nodeNum				= btreePtr->rootNode;
		btreePtr->rootNode	= GetChildNodeNum (btreePtr, blockPtr->buffer, 0);
		--btreePtr->treeDepth;

		//// Clear and Free Current Old Root Node ////
		ClearNode (btreePtr, blockPtr->buffer);
		err = UpdateNode (btreePtr, blockPtr);
		M_ExitOnError (err);
		err = FreeNode (btreePtr, nodeNum);
		M_ExitOnError (err);
		
		//// Get New Root Node
		err = GetNode (btreePtr, btreePtr->rootNode, blockPtr);
		M_ExitOnError (err);
	}
	
	if (btreePtr->rootNode != originalRoot)
		M_BTreeHeaderDirty (btreePtr);
		
	err = UpdateNode (btreePtr, blockPtr);	// always update!
	M_ExitOnError (err);
	
	return	noErr;
	

/////////////////////////////////// ErrorExit ///////////////////////////////////

ErrorExit:
	(void)	ReleaseNode (btreePtr, blockPtr);
	return	err;
}



////////////////////////////////// RotateLeft ///////////////////////////////////

/*-------------------------------------------------------------------------------

Routine:	RotateLeft	-	One_line_description.

Function:	Brief_description_of_the_function_and_any_side_effects

Algorithm:	if rightIndex > insertIndex, subtract 1 for actual rightIndex

Input:		btreePtr			- description
			leftNode			- description
			rightNode			- description
			rightInsertIndex	- description
			keyPtr				- description
			recPtr				- description
			recSize				- description
			
Output:		insertIndex
			insertNodeNum		- description
			recordFit			- description
			recsRotated
			
Result:		noErr		- success
			!= noErr	- failure
-------------------------------------------------------------------------------*/

static OSStatus	RotateLeft		(BTreeControlBlockPtr		 btreePtr,
								 NodeDescPtr				 leftNode,
								 NodeDescPtr				 rightNode,
								 UInt16						 rightInsertIndex,
								 KeyPtr						 keyPtr,
								 UInt8 *					 recPtr,
								 UInt16						 recSize,
								 UInt16						*insertIndex,
								 UInt32						*insertNodeNum,
								 Boolean					*recordFit,
								 UInt16						*recsRotated )
{
	OSStatus			err;
	SInt32				insertSize;
	SInt32				nodeSize;
	SInt32				leftSize, rightSize;
	SInt32				moveSize;
	UInt16				keyLength;
	UInt16				lengthFieldSize;
	UInt16				index, moveIndex;
	Boolean				didItFit;

	///////////////////// Determine If Record Will Fit //////////////////////////
	
	keyLength = GetKeyLength(btreePtr, keyPtr, (rightNode->kind == kBTLeafNode));

	// the key's length field is 8-bits in HFS and 16-bits in HFS+
	if ( btreePtr->attributes & kBTBigKeysMask )
		lengthFieldSize = sizeof(UInt16);
	else
		lengthFieldSize = sizeof(UInt8);

	insertSize = keyLength + lengthFieldSize + recSize + sizeof(UInt16);

	if ( M_IsOdd (insertSize) )
		++insertSize;	// add pad byte;

	nodeSize		= btreePtr->nodeSize;

	// add size of insert record to right node
	rightSize		= nodeSize - GetNodeFreeSize (btreePtr, rightNode) + insertSize;
	leftSize		= nodeSize - GetNodeFreeSize (btreePtr, leftNode);

	moveIndex	= 0;
	moveSize	= 0;

	while ( leftSize < rightSize )
	{
		if ( moveIndex < rightInsertIndex )
		{
			moveSize = GetRecordSize (btreePtr, rightNode, moveIndex) + 2;
		}
		else if ( moveIndex == rightInsertIndex )
		{
			moveSize = insertSize;
		}
		else // ( moveIndex > rightInsertIndex )
		{
			moveSize = GetRecordSize (btreePtr, rightNode, moveIndex - 1) + 2;
		}
		
		leftSize	+= moveSize;
		rightSize	-= moveSize;
		++moveIndex;
	}	
	
	if ( leftSize > nodeSize )	// undo last move
	{
		leftSize	-= moveSize;
		rightSize	+= moveSize;
		--moveIndex;
	}
	
	if ( rightSize > nodeSize )	// record won't fit - failure, but not error
	{
		*insertIndex	= 0;
		*insertNodeNum	= 0;
		*recordFit		= false;
		*recsRotated	= 0;
		
		return	noErr;
	}
	
	// we've found balance point, moveIndex == number of records moved into leftNode
	

	//////////////////////////// Rotate Records /////////////////////////////////

	*recsRotated	= moveIndex;
	*recordFit		= true;
	index			= 0;

	while ( index < moveIndex )
	{
		if ( index == rightInsertIndex )	// insert new record in left node
		{
			UInt16	leftInsertIndex;
			
			leftInsertIndex = leftNode->numRecords;

			didItFit = InsertKeyRecord (btreePtr, leftNode, leftInsertIndex,
										keyPtr, keyLength, recPtr, recSize);
			if ( !didItFit )
			{
				Panic ("\pRotateLeft: InsertKeyRecord (left) returned false!");
				err = fsBTBadRotateErr;
				goto ErrorExit;
			}
			
			*insertIndex = leftInsertIndex;
			*insertNodeNum = rightNode->bLink;
		}
		else
		{
			didItFit = RotateRecordLeft (btreePtr, leftNode, rightNode);
			if ( !didItFit )
			{
				Panic ("\pRotateLeft: RotateRecordLeft returned false!");
				err = fsBTBadRotateErr;
				goto ErrorExit;
			}
		}
		
		++index;
	}
	
	if ( moveIndex <= rightInsertIndex )	// then insert new record in right node
	{
		rightInsertIndex -= index;			// adjust for records already rotated
		
		didItFit = InsertKeyRecord (btreePtr, rightNode, rightInsertIndex,
									keyPtr, keyLength, recPtr, recSize);
		if ( !didItFit )
		{
			Panic ("\pRotateLeft: InsertKeyRecord (right) returned false!");
			err = fsBTBadRotateErr;
			goto ErrorExit;
		}
	
		*insertIndex = rightInsertIndex;
		*insertNodeNum = leftNode->fLink;
	}

#if DEBUG_TREEOPS
	if ( DoKeyCheck( leftNode, btreePtr ) != noErr )
	{
		plog( "\n%s - bad key order in left node num %d: \n", __FUNCTION__ , rightNode->bLink );
		PrintNodeDescriptor( leftNode );
		err = fsBTBadRotateErr;
		goto ErrorExit;
	}
	if ( DoKeyCheck( rightNode, btreePtr ) != noErr )
	{
		plog( "\n%s - bad key order in left node num %d: \n", __FUNCTION__ , leftNode->fLink );
		PrintNodeDescriptor( rightNode );
		err = fsBTBadRotateErr;
		goto ErrorExit;
	}
#endif // DEBUG_TREEOPS


	return noErr;


	////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:

	*insertIndex	= 0;
	*insertNodeNum	= 0;
	*recordFit		= false;
	*recsRotated	= 0;
	
	return	err;
}


#if 0 
/////////////////////////////////// SplitLeft ///////////////////////////////////

static OSStatus	SplitLeft		(BTreeControlBlockPtr		 btreePtr,
								 BlockDescriptor			*leftNode,
								 BlockDescriptor			*rightNode,
								 UInt32						 rightNodeNum,
								 UInt16						 index,
								 KeyPtr						 keyPtr,
								 UInt8 *					 recPtr,
								 UInt16						 recSize,
								 UInt16						*insertIndex,
								 UInt32						*insertNodeNum,
								 UInt16						*recsRotated )
{
	OSStatus			err;
	NodeDescPtr			left, right;
	UInt32				newNodeNum;
	Boolean				recordFit;
	
	
	///////////////////////////// Compare Nodes /////////////////////////////////

	right = rightNode->buffer;
	left  = leftNode->buffer;
	
	PanicIf ( right->bLink != 0 && left == 0, "\p SplitLeft: left sibling missing!?" );
	
	//ее type should be kLeafNode or kIndexNode
	
	if ( (right->height == 1) && (right->kind != kBTLeafNode) )
		return	fsBTInvalidNodeErr;
	
	if ( left != nil )
	{
		if ( left->fLink != rightNodeNum )
			return fsBTInvalidNodeErr;										//ее E_BadSibling ?
	
		if ( left->height != right->height )
			return	fsBTInvalidNodeErr;										//ее E_BadNodeHeight ?
		
		if ( left->kind != right->kind )
			return	fsBTInvalidNodeErr;										//ее E_BadNodeType ?
	}
	

	///////////////////////////// Allocate Node /////////////////////////////////

	err = AllocateNode (btreePtr, &newNodeNum);
	M_ExitOnError (err);
	

	/////////////// Update Forward Link In Original Left Node ///////////////////

	if ( left != nil )
	{
		left->fLink	= newNodeNum;
		err = UpdateNode (btreePtr, leftNode);
		M_ExitOnError (err);
	}


	/////////////////////// Initialize New Left Node ////////////////////////////

	err = GetNewNode (btreePtr, newNodeNum, leftNode);
	M_ExitOnError (err);
	
	left		= leftNode->buffer;
	left->fLink	= rightNodeNum;
	

	// Steal Info From Right Node
	
	left->bLink  = right->bLink;
	left->kind   = right->kind;
	left->height = right->height;
	
	right->bLink		= newNodeNum;			// update Right bLink

	if ( (left->kind == kBTLeafNode) && (left->bLink == 0) )
	{
		// if we're adding a new first leaf node - update BTreeInfoRec
		
		btreePtr->firstLeafNode = newNodeNum;
		M_BTreeHeaderDirty (btreePtr);		//ее AllocateNode should have set the bit already...
	}

	////////////////////////////// Rotate Left //////////////////////////////////

	err = RotateLeft (btreePtr, left, right, index, keyPtr, recPtr, recSize,
					  insertIndex, insertNodeNum, &recordFit, recsRotated);
	M_ExitOnError (err);
	
	return noErr;
	
ErrorExit:
	
	(void) ReleaseNode (btreePtr, leftNode);
	(void) ReleaseNode (btreePtr, rightNode);
	
	//ее Free new node if allocated?

	*insertIndex	= 0;
	*insertNodeNum	= 0;
	*recsRotated	= 0;
	
	return	err;
}
#endif



/////////////////////////////// RotateRecordLeft ////////////////////////////////

static Boolean RotateRecordLeft (BTreeControlBlockPtr		btreePtr,
								 NodeDescPtr				leftNode,
							 	 NodeDescPtr				rightNode )
{
	UInt16		size;
	UInt8 *		recPtr;
	Boolean		recordFit;
	
	size	= GetRecordSize (btreePtr, rightNode, 0);
	recPtr	= GetRecordAddress (btreePtr, rightNode, 0);
	
	recordFit = InsertRecord (btreePtr, leftNode, leftNode->numRecords, recPtr, size);
	
	if ( !recordFit )
		return false;

	DeleteRecord (btreePtr, rightNode, 0);
	
	return true;
}


//////////////////////////////// AddNewRootNode /////////////////////////////////

static OSStatus	AddNewRootNode	(BTreeControlBlockPtr	 btreePtr,
								 NodeDescPtr			 leftNode,
								 NodeDescPtr			 rightNode )
{
	OSStatus			err;
	BlockDescriptor		rootNode;
	UInt32				rootNum;
	KeyPtr				keyPtr;
	Boolean				didItFit;
	UInt16				keyLength;	
	
	PanicIf (leftNode == nil, "\pAddNewRootNode: leftNode == nil");
	PanicIf (rightNode == nil, "\pAddNewRootNode: rightNode == nil");
	
	
	/////////////////////// Initialize New Root Node ////////////////////////////
	
	err = AllocateNode (btreePtr, &rootNum);
	M_ExitOnError (err);
	
	err = GetNewNode (btreePtr, rootNum, &rootNode);
	M_ExitOnError (err);
		
	((NodeDescPtr)rootNode.buffer)->kind	= kBTIndexNode;
	((NodeDescPtr)rootNode.buffer)->height	= ++btreePtr->treeDepth;
	

	///////////////////// Insert Left Node Index Record /////////////////////////	

	keyPtr = (KeyPtr) GetRecordAddress (btreePtr, leftNode, 0);
	keyLength = GetKeyLength(btreePtr, keyPtr, false);

	didItFit = InsertKeyRecord ( btreePtr, rootNode.buffer, 0, keyPtr, keyLength,
								 (UInt8 *) &rightNode->bLink, 4 );

	PanicIf ( !didItFit, "\pAddNewRootNode:InsertKeyRecord failed for left index record");


	//////////////////// Insert Right Node Index Record /////////////////////////

	keyPtr = (KeyPtr) GetRecordAddress (btreePtr, rightNode, 0);
	keyLength = GetKeyLength(btreePtr, keyPtr, false);

	didItFit = InsertKeyRecord ( btreePtr, rootNode.buffer, 1, keyPtr, keyLength,
								 (UInt8 *) &leftNode->fLink, 4 );

	PanicIf ( !didItFit, "\pAddNewRootNode:InsertKeyRecord failed for right index record");


#if DEBUG_TREEOPS
	if ( DoKeyCheck( rootNode.buffer, btreePtr ) != noErr )
	{
		plog( "\n%s - bad key order in root node num %d: \n", __FUNCTION__ , rootNum );
		PrintNodeDescriptor( rootNode.buffer );
		err = fsBTBadRotateErr;
		goto ErrorExit;
	}
#endif // DEBUG_TREEOPS

	
	/////////////////////////// Release Root Node ///////////////////////////////
	
	err = UpdateNode (btreePtr, &rootNode);
	M_ExitOnError (err);
	
	// update BTreeInfoRec
	
	btreePtr->rootNode	 = rootNum;
	btreePtr->flags		|= kBTHeaderDirty;
	
	return noErr;


	////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:

	return	err;
}


static UInt16	GetKeyLength ( const BTreeControlBlock *btreePtr, const BTreeKey *key, Boolean forLeafNode )
{
	UInt16 length;

	if ( forLeafNode || btreePtr->attributes & kBTVariableIndexKeysMask )
		length = KeyLength (btreePtr, key);		// just use actual key length
	else
		length = btreePtr->maxKeyLength;		// fixed sized index key (i.e. HFS)		//ее shouldn't we clear the pad bytes?

	return length;
}



/////////////////////////////////// SplitRight ///////////////////////////////////

static OSStatus	SplitRight		(BTreeControlBlockPtr		 btreePtr,
								 BlockDescriptor			*nodePtr,
								 BlockDescriptor			*rightNodePtr,
								 UInt32						 nodeNum,
								 UInt16						 index,
								 KeyPtr						 keyPtr,
								 UInt8  					*recPtr,
								 UInt16						 recSize,
								 UInt16						*insertIndexPtr,
								 UInt32						*newNodeNumPtr,
								 UInt16						*recsRotatedPtr )
{
	OSStatus			err;
	NodeDescPtr			leftPtr, rightPtr;
	UInt32				newNodeNum;
	Boolean				recordFit;
	
	
	///////////////////////////// Compare Nodes /////////////////////////////////

	leftPtr  = nodePtr->buffer;
	
	if ( leftPtr->fLink != 0 )
	{
		err = GetNode( btreePtr, leftPtr->fLink, rightNodePtr );
		M_ExitOnError( err );
	}
	rightPtr = rightNodePtr->buffer;
	
	PanicIf ( leftPtr->fLink != 0 && rightPtr == 0, "\p SplitRight: right sibling missing!?" );
	
	//ее type should be kLeafNode or kIndexNode
	
	if ( (leftPtr->height == 1) && (leftPtr->kind != kBTLeafNode) )
		return	fsBTInvalidNodeErr;
	
	if ( rightPtr != nil )
	{
		if ( rightPtr->bLink != nodeNum )
			return fsBTInvalidNodeErr;										//ее E_BadSibling ?
	
		if ( rightPtr->height != leftPtr->height )
			return	fsBTInvalidNodeErr;										//ее E_BadNodeHeight ?
		
		if ( rightPtr->kind != leftPtr->kind )
			return	fsBTInvalidNodeErr;										//ее E_BadNodeType ?
	}
	

	///////////////////////////// Allocate Node /////////////////////////////////

	err = AllocateNode (btreePtr, &newNodeNum);
	M_ExitOnError (err);

	/////////////// Update backward Link In Original Right Node ///////////////////

	if ( rightPtr != nil )
	{
		rightPtr->bLink	= newNodeNum;
		err = UpdateNode (btreePtr, rightNodePtr);
		M_ExitOnError (err);
	}

	/////////////////////// Initialize New Right Node ////////////////////////////

	err = GetNewNode (btreePtr, newNodeNum, rightNodePtr );
	M_ExitOnError (err);
	
	rightPtr			= rightNodePtr->buffer;
	rightPtr->bLink		= nodeNum;
	

	// Steal Info From Left Node
	
	rightPtr->fLink  = leftPtr->fLink;
	rightPtr->kind   = leftPtr->kind;
	rightPtr->height = leftPtr->height;
	
	leftPtr->fLink		= newNodeNum;			// update Left fLink

	if ( (rightPtr->kind == kBTLeafNode) && (rightPtr->fLink == 0) )
	{
		// if we're adding a new last leaf node - update BTreeInfoRec
		
		btreePtr->lastLeafNode = newNodeNum;
		M_BTreeHeaderDirty (btreePtr);		//ее AllocateNode should have set the bit already...
	}

	////////////////////////////// Rotate Right //////////////////////////////////

	err = RotateRight (btreePtr, leftPtr, rightPtr, index, keyPtr, recPtr, recSize,
					  insertIndexPtr, newNodeNumPtr, &recordFit, recsRotatedPtr);
	M_ExitOnError (err);
	
	return noErr;
	
ErrorExit:
	
	(void) ReleaseNode (btreePtr, nodePtr);
	(void) ReleaseNode (btreePtr, rightNodePtr);
	
	//ее Free new node if allocated?

	*insertIndexPtr	= 0;
	*newNodeNumPtr	= 0;
	*recsRotatedPtr	= 0;
	
	return	err;
	
} /* SplitRight */



////////////////////////////////// RotateRight ///////////////////////////////////

/*-------------------------------------------------------------------------------

Routine:	RotateRight	-	rotate half of .

Function:	Brief_description_of_the_function_and_any_side_effects

Algorithm:	if rightIndex > insertIndex, subtract 1 for actual rightIndex

Input:		btreePtr			- description
			leftNode			- description
			rightNode			- description
			leftInsertIndex		- description
			keyPtr				- description
			recPtr				- description
			recSize				- description
			
Output:		insertIndex
			insertNodeNum		- description
			recordFit			- description
			recsRotated
			
Result:		noErr		- success
			!= noErr	- failure
-------------------------------------------------------------------------------*/

static OSStatus	RotateRight		(BTreeControlBlockPtr		 btreePtr,
								 NodeDescPtr				 leftNodePtr,
								 NodeDescPtr				 rightNodePtr,
								 UInt16						 leftInsertIndex,
								 KeyPtr						 keyPtr,
								 UInt8 						*recPtr,
								 UInt16						 recSize,
								 UInt16						*insertIndexPtr,
								 UInt32						*newNodeNumPtr,
								 Boolean					*didRecordFitPtr,
								 UInt16						*recsRotatedPtr )
{
	OSStatus			err;
	SInt32				insertSize;
	SInt32				nodeSize;
	SInt32				leftSize, rightSize;
	SInt32				moveSize;
	UInt16				keyLength;
	UInt16				lengthFieldSize;
	SInt16				index, moveIndex, myInsertIndex;
	Boolean				didItFit;
	Boolean				doIncrement = false;

	///////////////////// Determine If Record Will Fit //////////////////////////
	
	keyLength = GetKeyLength( btreePtr, keyPtr, (leftNodePtr->kind == kBTLeafNode) );

	// the key's length field is 8-bits in HFS and 16-bits in HFS+
	if ( btreePtr->attributes & kBTBigKeysMask )
		lengthFieldSize = sizeof(UInt16);
	else
		lengthFieldSize = sizeof(UInt8);

	/*
	 * A record size in a node is the size of the key, the size of the key length field,
	 * the size of the record, and the size of the record offset index.
	 */
	insertSize = keyLength + lengthFieldSize + recSize + sizeof(UInt16);
	if ( M_IsOdd (insertSize) )
		++insertSize;	// add pad byte;
	nodeSize		= btreePtr->nodeSize;

	// add size of insert record to left node
	rightSize		= nodeSize - GetNodeFreeSize( btreePtr, rightNodePtr );
	leftSize		= nodeSize - GetNodeFreeSize( btreePtr, leftNodePtr ) + insertSize;

	moveIndex	= leftNodePtr->numRecords; // start at last record in the node
	moveSize	= 0;

	/*
	 * The goal here is to attempt to make the nodes as balanced as
	 * possible.  We do this by "moving" records from the left node to
	 * the right node, until the right node is larger than the left
	 * node.
	 *
	 * We also need to factor in the new record for this; what we are
	 * trying to do, as a result, is consider a virtual node that has
	 * all of the old records in it, plus the new record inserted at
	 * the proper place.  (This is the reason for the if cases in the
	 * loop.)
	 */
	while ( rightSize < leftSize )
	{
		/*
		 * We set moveSize to the size of the record being moved in this
		 * pass.  We need to add sizeof(UInt16) because we need to account
		 * for the record offset index, which is two bytes.  This was already
		 * added to insertSize above.
		 */
		if (moveIndex > leftInsertIndex)
			moveSize = GetRecordSize( btreePtr, leftNodePtr, moveIndex - 1) + sizeof(UInt16);
		else if (moveIndex == leftInsertIndex)
			moveSize = insertSize;
		else // (moveIndex < leftInsertIndex)
			moveSize = GetRecordSize( btreePtr, leftNodePtr, moveIndex) + sizeof(UInt16);

		leftSize	-= moveSize;
		rightSize	+= moveSize;
		--moveIndex;
	}	
	
	if ( rightSize > nodeSize )	// undo last move
	{
		leftSize	+= moveSize;
		rightSize	-= moveSize;
		++moveIndex;
	}
	
	if ( leftSize > nodeSize )	// record won't fit - failure, but not error
	{
		*insertIndexPtr		= 0;
		*newNodeNumPtr		= 0;
		*didRecordFitPtr	= false;
		*recsRotatedPtr		= 0;
		
		return	noErr;
	}
	
	// we've found balance point, we rotate up to moveIndex into right node

	//////////////////////////// Rotate Records /////////////////////////////////

	*didRecordFitPtr	= true;
	index				= leftNodePtr->numRecords;
	*recsRotatedPtr		= index - moveIndex;
	myInsertIndex 		= 0;
	
	// handle case where the new record is inserting after the last
	// record in our left node.
	if ( leftNodePtr->numRecords == leftInsertIndex )
	{
		didItFit = InsertKeyRecord (btreePtr, rightNodePtr, 0,
									keyPtr, keyLength, recPtr, recSize);
		if ( !didItFit )
		{
			if (debug) plog ("RotateRight: InsertKeyRecord (left) returned false!\n");
			err = fsBTBadRotateErr;
			goto ErrorExit;
		}
		
		// NOTE - our insert location will slide as we insert more records
		doIncrement = true;
		*newNodeNumPtr = leftNodePtr->fLink;
		index--;
	}

	while ( index > moveIndex )
	{
		if ( index == leftInsertIndex )	// insert new record in right node
		{
			didItFit = InsertKeyRecord (btreePtr, rightNodePtr, 0,
										keyPtr, keyLength, recPtr, recSize);
			if ( !didItFit )
			{
				if (debug) plog ("RotateRight: InsertKeyRecord (right) returned false!\n");
				err = fsBTBadRotateErr;
				goto ErrorExit;
			}
			
			// NOTE - our insert index will slide as we insert more records
			doIncrement = true;
			myInsertIndex = -1;
			*newNodeNumPtr = leftNodePtr->fLink;
		}
		else
		{
			didItFit = RotateRecordRight( btreePtr, leftNodePtr, rightNodePtr );
			if ( !didItFit )
			{
				if (debug) plog ("RotateRight: RotateRecordRight returned false!\n");
				err = fsBTBadRotateErr;
				goto ErrorExit;
			}
		}

		if ( doIncrement )
			myInsertIndex++;
		--index;
	}
	
	*insertIndexPtr = myInsertIndex;
	
	if ( moveIndex >= leftInsertIndex )	// then insert new record in left node
	{
		didItFit = InsertKeyRecord (btreePtr, leftNodePtr, leftInsertIndex,
									keyPtr, keyLength, recPtr, recSize);
		if ( !didItFit )
		{
			if (debug) plog ("RotateRight: InsertKeyRecord (left) returned false!\n");
			err = fsBTBadRotateErr;
			goto ErrorExit;
		}
	
		*insertIndexPtr = leftInsertIndex;
		*newNodeNumPtr = rightNodePtr->bLink;
	}

#if DEBUG_TREEOPS
	if ( DoKeyCheck( rightNodePtr, btreePtr ) != noErr )
	{
		plog( "\n%s - bad key order in right node num %d: \n", __FUNCTION__ , leftNodePtr->fLink);
		PrintNodeDescriptor( rightNodePtr );
		err = fsBTBadRotateErr;
		goto ErrorExit;
	}
	if ( DoKeyCheck( leftNodePtr, btreePtr ) != noErr )
	{
		plog( "\n%s - bad key order in left node num %d: \n", __FUNCTION__ , rightNodePtr->bLink);
		PrintNodeDescriptor( leftNodePtr );
		err = fsBTBadRotateErr;
		goto ErrorExit;
	}
	if ( DoKeyCheckAcrossNodes( leftNodePtr, rightNodePtr, btreePtr, false ) != noErr )
	{
		plog( "\n%s - bad key order across nodes left %d right %d: \n", 
			__FUNCTION__ , rightNodePtr->bLink, leftNodePtr->fLink );
		PrintNodeDescriptor( leftNodePtr );
		PrintNodeDescriptor( rightNodePtr );
		err = fsBTBadRotateErr;
		goto ErrorExit;
	}
#endif // DEBUG_TREEOPS

	return noErr;


	////////////////////////////// Error Exit ///////////////////////////////////

ErrorExit:

	*insertIndexPtr	= 0;
	*newNodeNumPtr	= 0;
	*didRecordFitPtr = false;
	*recsRotatedPtr	= 0;
	
	return	err;
	
} /* RotateRight */



/////////////////////////////// RotateRecordRight ////////////////////////////////

static Boolean RotateRecordRight (BTreeControlBlockPtr		btreePtr,
								 NodeDescPtr				leftNodePtr,
							 	 NodeDescPtr				rightNodePtr )
{
	UInt16		size;
	UInt8 *		recPtr;
	Boolean		didRecordFit;
	
	size	= GetRecordSize( btreePtr, leftNodePtr, leftNodePtr->numRecords - 1 ) ;
	recPtr	= GetRecordAddress( btreePtr, leftNodePtr, leftNodePtr->numRecords - 1 ) ;
	
	didRecordFit = InsertRecord( btreePtr, rightNodePtr, 0, recPtr, size );
	if ( !didRecordFit )
		return false;
	
	DeleteRecord( btreePtr, leftNodePtr, leftNodePtr->numRecords - 1 );
	
	return true;
	
} /* RotateRecordRight */



#if DEBUG_TREEOPS
static int	DoKeyCheckAcrossNodes( 	NodeDescPtr theLeftNodePtr, 
									NodeDescPtr theRightNodePtr, 
									BTreeControlBlock *theTreePtr,
									Boolean printKeys ) 
{
	UInt16				myLeftDataSize;
	UInt16				myRightDataSize;
	UInt16				myRightKeyLen;
	UInt16				myLeftKeyLen;
	KeyPtr 				myLeftKeyPtr;
	KeyPtr 				myRightKeyPtr;
	UInt8 *				myLeftDataPtr;
	UInt8 *				myRightDataPtr;


	GetRecordByIndex( theTreePtr, theLeftNodePtr, (theLeftNodePtr->numRecords - 1), 
					  &myLeftKeyPtr, &myLeftDataPtr, &myLeftDataSize );
	GetRecordByIndex( theTreePtr, theRightNodePtr, 0, 
					  &myRightKeyPtr, &myRightDataPtr, &myRightDataSize );

	if ( theTreePtr->attributes & kBTBigKeysMask )
	{
		myRightKeyLen = myRightKeyPtr->length16;
		myLeftKeyLen = myLeftKeyPtr->length16;
	}
	else
	{
		myRightKeyLen = myRightKeyPtr->length8;
		myLeftKeyLen = myLeftKeyPtr->length8;
	}

	if ( printKeys )
	{
		plog( "%s - left and right keys:\n", __FUNCTION__ );
		PrintKey( (UInt8 *) myLeftKeyPtr, myLeftKeyLen );
		PrintKey( (UInt8 *) myRightKeyPtr, myRightKeyLen );
	}
	
	if ( CompareKeys( theTreePtr, myLeftKeyPtr, myRightKeyPtr ) >= 0 )
		return( -1 );

	return( noErr );
	
} /* DoKeyCheckAcrossNodes */


static int DoKeyCheck( NodeDescPtr nodeP, BTreeControlBlock *btcb )
{
	SInt16				index;
	UInt16				dataSize;
	UInt16				keyLength;
	KeyPtr 				keyPtr;
	UInt8				*dataPtr;
	KeyPtr				prevkeyP	= nil;


	if ( nodeP->numRecords == 0 )
	{
		if ( (nodeP->fLink == 0) && (nodeP->bLink == 0) )
			return( -1 );
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
				return( -1 );
			}
	
			if ( prevkeyP != nil )
			{
				if ( CompareKeys( (BTreeControlBlockPtr)btcb, prevkeyP, keyPtr ) >= 0 )
				{
					return( -1 );
				}
			}
			prevkeyP = keyPtr;
		}
	}

	return( noErr );
	
} /* DoKeyCheck */

static void PrintNodeDescriptor( NodeDescPtr  thePtr )
{
	plog( "    fLink %d bLink %d kind %d height %d numRecords %d \n",
			thePtr->fLink, thePtr->bLink, thePtr->kind, thePtr->height, thePtr->numRecords );
}


static void PrintKey( UInt8 *  myPtr, int mySize )
{
	int		i;
	
	for ( i = 0; i < mySize+2; i++ )
	{
		plog("%02X", *(myPtr + i) );
	}
	plog("\n" );
} /* PrintKey */


#endif // DEBUG_TREEOPS
