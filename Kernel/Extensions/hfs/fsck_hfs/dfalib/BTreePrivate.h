/*
 * Copyright (c) 1999-2008 Apple Inc. All rights reserved.
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
	File:		BTreePrivate.h

	Contains:	Private interface file for the BTree Module.

	Version:	xxx put the technology version here xxx

	Written by:	Gordon Sheridan and Bill Bruffey

	Copyright:	й 1992-1998 by Apple Computer, Inc., all rights reserved.
*/

#ifndef	__BTREEPRIVATE__
#define __BTREEPRIVATE__

#include "BTree.h"

/////////////////////////////////// Constants ///////////////////////////////////

#define		SupportsKeyDescriptors	0

#define		kBTreeVersion		  1
#define		kMaxTreeDepth		 16


#define		kHeaderNodeNum		  0
#define		kKeyDescRecord		  1


// Header Node Record Offsets
enum {
	kHeaderRecOffset	=	0x000E,
	kKeyDescRecOffset	=	0x0078,
	kHeaderMapRecOffset	=	0x00F8
};

#define		kMinNodeSize		512

#define		kMinRecordSize		  6
										//ее where is minimum record size enforced?

// miscellaneous BTree constants
enum {
			kOffsetSize				= 2
};

// Insert Operations
typedef enum {
			kInsertRecord			= 0,
			kReplaceRecord			= 1
} InsertType;

// illegal string attribute bits set in mask
#define		kBadStrAttribMask		0xCF



//////////////////////////////////// Macros /////////////////////////////////////

#define		M_NodesInMap(mapSize)				((mapSize) << 3)

#define		M_ClearBitNum(integer,bitNumber) 	((integer) &= (~(1<<(bitNumber))))
#define		M_SetBitNum(integer,bitNumber) 		((integer) |= (1<<(bitNumber)))
#define		M_IsOdd(integer) 					(((integer) & 1) != 0)
#define		M_IsEven(integer) 					(((integer) & 1) == 0)
#define		M_BTreeHeaderDirty(btreePtr)		btreePtr->flags |= kBTHeaderDirty

#define		M_MapRecordSize(nodeSize)			(nodeSize - sizeof (BTNodeDescriptor) - 6)
#define		M_HeaderMapRecordSize(nodeSize)		(nodeSize - sizeof(BTNodeDescriptor) - sizeof(BTHeaderRec) - 128 - 8)

#define		M_SWAP_BE16_ClearBitNum(integer,bitNumber)  ((integer) &= SWAP_BE16(~(1<<(bitNumber))))
#define		M_SWAP_BE16_SetBitNum(integer,bitNumber)    ((integer) |= SWAP_BE16(1<<(bitNumber)))

#if DEBUG_BUILD
	#define Panic( message )					DebugStr( (ConstStr255Param) message )
	#define PanicIf( condition, message )		if ( (condition) != 0 )	DebugStr( message )
#else
	#define Panic( message )	do { ; } while (0)
	#define PanicIf( condition, message )	do { ; } while (0)
#endif

///////////////////////////////////// Types /////////////////////////////////////
struct BTreeExtensionRec;

typedef struct BTreeControlBlock {					// fields specific to BTree CBs

	UInt8		keyCompareType;   /* Key string Comparison Type */
	UInt8						 btreeType;
	SInt16						 obsolete_fileRefNum;		// refNum of btree file
	KeyCompareProcPtr			 keyCompareProc;
	UInt8						 reserved2[16];		// keep for alignment with old style fields
	UInt16						 treeDepth;
	UInt32						 rootNode;
	UInt32						 leafRecords;
	UInt32						 firstLeafNode;
	UInt32						 lastLeafNode;
	UInt16						 nodeSize;
	UInt16						 maxKeyLength;
	UInt32						 totalNodes;
	UInt32						 freeNodes;
	UInt32 						reserved3[16];				/*	reserved*/

	// new fields
	SInt16						 version;
	UInt32						 flags;				// dynamic flags
	UInt32						 attributes;		// persistent flags
	KeyDescriptorPtr			 keyDescPtr;
	UInt32						 writeCount;

	GetBlockProcPtr			 	 getBlockProc;
	ReleaseBlockProcPtr			 releaseBlockProc;
	SetEndOfForkProcPtr			 setEndOfForkProc;
	BTreeIterator				 lastIterator;		// needed for System 7 iteration context

	// statistical information
	UInt32						 numGetNodes;
	UInt32						 numGetNewNodes;
	UInt32						 numReleaseNodes;
	UInt32						 numUpdateNodes;
	UInt32						 numMapNodesRead;	// map nodes beyond header node
	UInt32						 numHintChecks;
	UInt32						 numPossibleHints;	// Looks like a formated hint
	UInt32						 numValidHints;		// Hint used to find correct record.
	
	struct BTreeExtensionsRec	*refCon;			//	Used by DFA to point to private data.
	SFCB						*fcbPtr;		// fcb of btree file
	
} BTreeControlBlock, *BTreeControlBlockPtr;


UInt32 CalcKeySize(const BTreeControlBlock *btcb, const BTreeKey *key);
#define CalcKeySize(btcb, key)			( ((btcb)->attributes & kBTBigKeysMask) ? ((key)->length16 + 2) : ((key)->length8 + 1) )

UInt32 MaxKeySize(const BTreeControlBlock *btcb);
#define MaxKeySize(btcb)				( (btcb)->maxKeyLength + ((btcb)->attributes & kBTBigKeysMask ? 2 : 1))

UInt32 KeyLength(const BTreeControlBlock *btcb, const BTreeKey *key);
#define KeyLength(btcb, key)			( ((btcb)->attributes & kBTBigKeysMask) ? (key)->length16 : (key)->length8 )


typedef enum {
					kBTHeaderDirty	= 0x00000001
}	BTreeFlags;


typedef	SInt8				*NodeBuffer;
typedef BlockDescriptor		 NodeRec, *NodePtr;		//ее remove this someday...




//// Tree Path Table - constructed by SearchTree, used by InsertTree and DeleteTree

typedef struct {
	UInt32				node;				// node number
	UInt16				index;
	UInt16				reserved;			// align size to a power of 2
} TreePathRecord, *TreePathRecordPtr;

typedef TreePathRecord		TreePathTable [kMaxTreeDepth];


//// InsertKey - used by InsertTree, InsertLevel and InsertNode

struct InsertKey {
	BTreeKeyPtr		keyPtr;
	UInt8 *			recPtr;
	UInt16			keyLength;
	UInt16			recSize;
	Boolean			replacingKey;
	Boolean			skipRotate;
};

typedef struct InsertKey InsertKey;


//// For Notational Convenience

typedef	BTNodeDescriptor*	 NodeDescPtr;
typedef UInt8				*RecordPtr;
typedef BTreeKeyPtr			 KeyPtr;


//////////////////////////////////// Globals ////////////////////////////////////


//////////////////////////////////// Macros /////////////////////////////////////

//	Exit function on error
#define M_ExitOnError( result )	if ( ( result ) != noErr )	goto ErrorExit; else ;

//	Test for passed condition and return if true
#define	M_ReturnErrorIf( condition, error )	if ( condition )	return( error )

#if DEBUG_BUILD
	#define Panic( message )					DebugStr( (ConstStr255Param) message )
	#define PanicIf( condition, message )		if ( (condition) != 0 )	DebugStr( message )
#else
	#define Panic( message )	do { ; } while (0)
	#define PanicIf( condition, message )	do { ; } while (0)
#endif

//////////////////////////////// Key Operations /////////////////////////////////

SInt32		CompareKeys				(BTreeControlBlockPtr	 btreePtr,
									 KeyPtr					 searchKey,
									 KeyPtr					 trialKey );

OSStatus	GetKeyDescriptor		(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr			 headerNode );

OSStatus	CheckKeyDescriptor		(KeyDescriptorPtr		 keyDescPtr,
									 UInt16					 maxKeyLength );

OSStatus	CheckKey				(KeyPtr					 keyPtr,
									 KeyDescriptorPtr		 keyDescPtr,
									 UInt16					 maxKeyLength );



//////////////////////////////// Map Operations /////////////////////////////////

OSStatus	AllocateNode			(BTreeControlBlockPtr	 btreePtr,
									 UInt32					*nodeNum);

OSStatus	FreeNode				(BTreeControlBlockPtr	 btreePtr,
									 UInt32					 nodeNum);

OSStatus	ExtendBTree				(BTreeControlBlockPtr	 btreePtr,
									 UInt32					 nodes );

UInt32		CalcMapBits				(BTreeControlBlockPtr	 btreePtr);


//////////////////////////////// Misc Operations ////////////////////////////////

UInt16		CalcKeyRecordSize		(UInt16					 keySize,
									 UInt16					 recSize );

OSStatus	VerifyHeader			(SFCB			*filePtr,
									 BTHeaderRec			*header );

OSStatus	UpdateHeader			(BTreeControlBlockPtr	 btreePtr );

OSStatus	FindIteratorPosition	(BTreeControlBlockPtr	 btreePtr,
									 BTreeIteratorPtr		 iterator,
									 BlockDescriptor		*left,
									 BlockDescriptor		*middle,
									 BlockDescriptor		*right,
									 UInt32					*nodeNum,
									 UInt16					*index,
									 Boolean				*foundRecord );

OSStatus	CheckInsertParams		(SFCB			*filePtr,
									 BTreeIterator			*iterator,
									 FSBufferDescriptor		*record,
									 UInt16					 recordLen );

OSStatus	TrySimpleReplace		(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr			 nodePtr,
									 BTreeIterator			*iterator,
									 FSBufferDescriptor		*record,
									 UInt16					 recordLen,
									 Boolean				*recordInserted );

OSStatus	IsItAHint				(BTreeControlBlockPtr 	 btreePtr, 
									 BTreeIterator 			*iterator, 
									 Boolean 				*answer );

//////////////////////////////// Node Operations ////////////////////////////////

//// Node Operations

OSStatus	GetNode					(BTreeControlBlockPtr	 btreePtr,
									 UInt32					 nodeNum,
									 NodeRec				*returnNodePtr );

OSStatus	GetLeftSiblingNode		(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr			 node,
									 NodeRec				*left );

#define		GetLeftSiblingNode(btree,node,left)			GetNode ((btree), ((NodeDescPtr)(node))->bLink, (left))

OSStatus	GetRightSiblingNode		(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr			 node,
									 NodeRec				*right );

#define		GetRightSiblingNode(btree,node,right)		GetNode ((btree), ((NodeDescPtr)(node))->fLink, (right))


OSStatus	GetNewNode				(BTreeControlBlockPtr	 btreePtr,
									 UInt32					 nodeNum,
									 NodeRec				*returnNodePtr );

OSStatus	ReleaseNode				(BTreeControlBlockPtr	 btreePtr,
									 NodePtr				 nodePtr );
OSStatus	TrashNode				(BTreeControlBlockPtr	 btreePtr,
									NodePtr				 nodePtr );

OSStatus	UpdateNode				(BTreeControlBlockPtr	 btreePtr,
									 NodePtr				 nodePtr );

OSStatus	GetMapNode				(BTreeControlBlockPtr	 btreePtr,
									 BlockDescriptor		 *nodePtr,
									 UInt16					 **mapPtr,
									 UInt16					 *mapSize );

//// Node Buffer Operations

void		ClearNode				(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr			 node );

UInt16		GetNodeDataSize			(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr			 node );

UInt16		GetNodeFreeSize			(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr			 node );


//// Record Operations

Boolean		InsertRecord			(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr	 		 node,
									 UInt16	 				 index,
									 RecordPtr				 recPtr,
									 UInt16					 recSize );

Boolean		InsertKeyRecord			(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr 			 node,
									 UInt16	 				 index,
									 KeyPtr					 keyPtr,
									 UInt16					 keyLength,
									 RecordPtr				 recPtr,
									 UInt16					 recSize );

void		DeleteRecord			(BTreeControlBlockPtr	btree,
									 NodeDescPtr	 		node,
									 UInt16	 				index );


Boolean		SearchNode				(BTreeControlBlockPtr	 btree,
									 NodeDescPtr			 node,
									 KeyPtr					 searchKey,
									 UInt16					*index );

OSStatus	GetRecordByIndex		(BTreeControlBlockPtr	 btree,
									 NodeDescPtr			 node,
									 UInt16					 index,
									 KeyPtr					*keyPtr,
									 UInt8 *				*dataPtr,
									 UInt16					*dataSize );

UInt8 *		GetRecordAddress		(BTreeControlBlockPtr	 btree,
									 NodeDescPtr			 node,
									 UInt16					 index );

#define GetRecordAddress(btreePtr,node,index)		((UInt8 *)(node) + (*(short *) ((UInt8 *)(node) + (btreePtr)->nodeSize - ((index) << 1) - kOffsetSize)))


UInt16		GetRecordSize			(BTreeControlBlockPtr	 btree,
									 NodeDescPtr			 node,
									 UInt16					 index );

UInt32		GetChildNodeNum			(BTreeControlBlockPtr	 btreePtr,
									 NodeDescPtr			 nodePtr,
									 UInt16					 index );

void		MoveRecordsLeft			(UInt8 *				 src,
									 UInt8 *				 dst,
									 UInt16					 bytesToMove );

#define		MoveRecordsLeft(src,dst,bytes)			CopyMemory((src),(dst),(bytes))

void		MoveRecordsRight		(UInt8 *				 src,
									 UInt8 *				 dst,
									 UInt16					 bytesToMove );

#define		MoveRecordsRight(src,dst,bytes)			CopyMemory((src),(dst),(bytes))



//////////////////////////////// Tree Operations ////////////////////////////////

OSStatus	SearchTree				(BTreeControlBlockPtr	 btreePtr,
									 BTreeKeyPtr			 keyPtr,
									 TreePathTable			 treePathTable,
									 UInt32					*nodeNum,
									 BlockDescriptor		*nodePtr,
									 UInt16					*index );

OSStatus	InsertTree				(BTreeControlBlockPtr	 btreePtr,
									 TreePathTable			 treePathTable,
									 KeyPtr					 keyPtr,
									 UInt8 *				 recPtr,
									 UInt16					 recSize,
									 BlockDescriptor		*targetNode,
									 UInt16					 index,
									 UInt16					 level,
									 Boolean				 replacingKey,
									 UInt32					*insertNode );

OSStatus	DeleteTree				(BTreeControlBlockPtr	 btreePtr,
									 TreePathTable			 treePathTable,
									 BlockDescriptor		*targetNode,
									 UInt16					 index,
									 UInt16					 level );

#endif //__BTREEPRIVATE__
