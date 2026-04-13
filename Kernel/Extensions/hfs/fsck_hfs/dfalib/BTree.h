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
	File:		BTreesInternal.h

	Contains:	IPI to File Manager B-tree

	Version:	HFS Plus 1.0

	Copyright:	й 1996-1998 by Apple Computer, Inc., all rights reserved.
*/

#ifndef	__BTREESINTERNAL__
#define __BTREESINTERNAL__

#include "SRuntime.h"

//
// internal error codes
//
enum {
	// FXM errors

	fxRangeErr				= 16,		// file position beyond mapped range
	fxOvFlErr				= 17,		// extents file overflow

	// Unicode errors
	
	uniTooLongErr			= 24,		// Unicode string too long to convert to Str31
	uniBufferTooSmallErr	= 25,		// Unicode output buffer too small
	uniNotMappableErr		= 26,		// Unicode string can't be mapped to given script

	// BTree Manager errors

	btNotFound		 		= 32,		// record not found
	btExists  		 		= 33,		// record already exists
	btNoSpaceAvail			= 34,		// no available space
	btNoFit   		 		= 35,		// record doesn't fit in node 
	btBadNode 		 		= 36,		// bad node detected
	btBadHdr  		 		= 37,		// bad BTree header record detected
	dsBadRotate   	 		= 64,		// bad BTree rotate

	// Catalog Manager errors

	cmNotFound		 		= 48,		// CNode not found
	cmExists  		 		= 49,		// CNode already exists
	cmNotEmpty		 		= 50,		// directory CNode not empty (valence = 0)
	cmRootCN  		 		= 51,		// invalid reference to root CNode
	cmBadNews 		 		= 52,		// detected bad catalog structure
	cmFThdDirErr  	 		= 53,		// thread belongs to a directory not a file
	cmFThdGone		 		= 54,		// file thread doesn't exist
	cmParentNotFound		= 55,		// CNode for parent ID does not exist
	cmNotAFolder			= 56,		// Destination of move is a file, not a folder
	cmUnknownNodeType		= 57,		// Node type isn't recognized

	// Volume Check errors
	
	vcInvalidExtentErr		= 60		// Extent record is out of bounds
};

enum {
	fsBTInvalidHeaderErr			= btBadHdr,
	fsBTBadRotateErr				= dsBadRotate,
	fsBTInvalidNodeErr				= btBadNode,
	fsBTRecordTooLargeErr			= btNoFit,
	fsBTRecordNotFoundErr			= btNotFound,
	fsBTDuplicateRecordErr			= btExists,
	fsBTFullErr						= btNoSpaceAvail,

	fsBTInvalidFileErr				= 0x0302,					/* no BTreeCB has been allocated for fork*/
	fsBTrFileAlreadyOpenErr			= 0x0303,
	fsBTInvalidIteratorErr			= 0x0308,
	fsBTEmptyErr					= 0x030A,
	fsBTNoMoreMapNodesErr			= 0x030B,
	fsBTBadNodeSize					= 0x030C,
	fsBTBadNodeType					= 0x030D,
	fsBTInvalidKeyLengthErr			= 0x030E,
	fsBTInvalidKeyDescriptor		= 0x030F,
	fsBTStartOfIterationErr			= 0x0353,
	fsBTEndOfIterationErr			= 0x0354,
	fsBTUnknownVersionErr			= 0x0355,
	fsBTTreeTooDeepErr				= 0x0357,
	fsBTInvalidKeyDescriptorErr		= 0x0358,
	fsBTInvalidKeyFieldErr			= 0x035E,
	fsBTInvalidKeyAttributeErr		= 0x035F,
	fsIteratorExitedScopeErr		= 0x0A02,			/* iterator exited the scope*/
	fsIteratorScopeExceptionErr		= 0x0A03,			/* iterator is undefined due to error or movement of scope locality*/
	fsUnknownIteratorMovementErr	= 0x0A04,			/* iterator movement is not defined*/
	fsInvalidIterationMovmentErr	= 0x0A05,			/* iterator movement is invalid in current context*/
	fsClientIDMismatchErr			= 0x0A06,			/* wrong client process ID*/
	fsEndOfIterationErr				= 0x0A07,			/* there were no objects left to return on iteration*/
	fsBTTimeOutErr					= 0x0A08			/* BTree scan interrupted -- no time left for physical I/O */
};


struct FSBufferDescriptor {
	LogicalAddress 					bufferAddress;
	ByteCount 						itemSize;
	ItemCount 						itemCount;
};
typedef struct FSBufferDescriptor FSBufferDescriptor;

typedef FSBufferDescriptor *FSBufferDescriptorPtr;



typedef	UInt64	FSSize;
typedef	UInt32	ForkBlockNumber;


/*
	BTreeObjID is used to indicate an access path using the
	BTree access method to a specific fork of a file. This value
	is session relative and not persistent between invocations of
	an application. It is in fact an object ID to which requests
	for the given path should be sent.
 */
typedef UInt32 BTreeObjID;

/*
	B*Tree Information Version
*/

enum BTreeInformationVersion{
	kBTreeInfoVersion	= 0
};

/*
	B*Tree Iteration Operation Constants
*/

enum BTreeIterationOperations{
	kBTreeFirstRecord,
	kBTreeNextRecord,
	kBTreePrevRecord,
	kBTreeLastRecord,
	kBTreeCurrentRecord
};
typedef UInt16 BTreeIterationOperation;

/*
	B*Tree Key Descriptor Limits
*/

enum {
	kMaxKeyDescriptorLength		= 23,
};

/*
	B*Tree Key Descriptor Field Types
*/

enum {
	kBTreeSkip				=  0,
	kBTreeByte				=  1,
	kBTreeSignedByte		=  2,
	kBTreeWord				=  4,
	kBTreeSignedWord		=  5,
	kBTreeLong				=  6,
	kBTreeSignedLong		=  7,
	kBTreeString			=  3,		// Pascal string
	kBTreeFixLenString		=  8,		// Pascal string w/ fixed length buffer
	kBTreeReserved			=  9,		// reserved for Desktop Manager (?)
	kBTreeUseKeyCmpProc		= 10,
	//ее not implemented yet...
	kBTreeCString			= 11,
	kBTreeFixLenCString		= 12,
	kBTreeUniCodeString		= 13,
	kBTreeFixUniCodeString	= 14
};
typedef UInt8		BTreeKeyType;


/*
	B*Tree Key Descriptor String Field Attributes
*/

enum {
	kBTreeCaseSens			= 0x10,		// case sensitive
	kBTreeNotDiacSens		= 0x20		// not diacritical sensitive
};
typedef UInt8		BTreeStringAttributes;

/*
	Btree types: 0 is HFS CAT/EXT file, 1~127 are AppleShare B*Tree files, 128~254 unused
	hfsBtreeType	EQU		0			; control file
	validBTType		EQU		$80			; user btree type starts from 128
	userBT1Type		EQU		$FF			; 255 is our Btree type. Used by BTInit and BTPatch
*/

enum BTreeTypes{
	kHFSBTreeType			=   0,		// control file
	kUserBTreeType			= 128,		// user btree type starts from 128
	kReservedBTreeType		= 255		//
};

enum {
	kInvalidMRUCacheKey		= -1L	/* flag to denote current MRU cache key is invalid*/

};

/*============================================================================
	B*Tree Key Structures
============================================================================*/

/*
	BTreeKeyDescriptor is used to indicate how keys for a particular B*Tree
	are to be compared.
 */
typedef char BTreeKeyDescriptor[26];
typedef char *BTreeKeyDescriptorPtr;

/*
	BTreeInformation is used to describe the public information about a BTree
 */
struct BTreeInformation{
	UInt16						NodeSize;
	UInt16						MaxKeyLength;
	UInt16						Depth;
	UInt16						Reserved;
	ItemCount					NumRecords;
	ItemCount					NumNodes;
	ItemCount					NumFreeNodes;
	ByteCount					ClumpSize;
	BTreeKeyDescriptor			KeyDescriptor;
	};
typedef struct BTreeInformation BTreeInformation;
typedef BTreeInformation *BTreeInformationPtr;

typedef BTreeKey *BTreeKeyPtr;


struct KeyDescriptor{
	UInt8			length;
	UInt8			fieldDesc [kMaxKeyDescriptorLength];
};
typedef struct KeyDescriptor KeyDescriptor;
typedef KeyDescriptor *KeyDescriptorPtr;

struct NumberFieldDescriptor{
	UInt8			fieldType;
	UInt8			occurrence;					// number of consecutive fields of this type
};
typedef struct NumberFieldDescriptor NumberFieldDescriptor;

struct StringFieldDescriptor{
	UInt8			fieldType;					// kBTString
	UInt8			occurrence;					// number of consecutive fields of this type
	UInt8			stringAttribute;
	UInt8			filler;
};
typedef struct StringFieldDescriptor StringFieldDescriptor;

struct FixedLengthStringFieldDescriptor{
	UInt8			fieldType;					// kBTFixLenString
	UInt8			stringLength;
	UInt8			occurrence;
	UInt8			stringAttribute;
};
typedef struct FixedLengthStringFieldDescriptor FixedLengthStringFieldDescriptor;

/*
	BTreeInfoRec Structure - for BTGetInformation
*/
struct BTreeInfoRec{
	UInt16				version;
	UInt16				nodeSize;
	UInt16				maxKeyLength;
	UInt16				treeDepth;
	ItemCount			numRecords;
	ItemCount			numNodes;
	ItemCount			numFreeNodes;
	KeyDescriptorPtr	keyDescriptor;
	ByteCount			keyDescLength;
	UInt32				reserved;
};
typedef struct BTreeInfoRec BTreeInfoRec;
typedef BTreeInfoRec *BTreeInfoPtr;

/*
	BTreeHint can never be exported to the outside. Use UInt32 BTreeHint[4],
	UInt8 BTreeHint[16], etc.
 */
struct BTreeHint{
	ItemCount				writeCount;
	UInt32					nodeNum;			// node the key was last seen in
	UInt16					index;				// index then key was last seen at
	UInt16					reserved1;
	UInt32					reserved2;
};
typedef struct BTreeHint BTreeHint;
typedef BTreeHint *BTreeHintPtr;

/*
	BTree Iterator
*/
struct BTreeIterator{
	BTreeHint				hint;
	UInt16					version;
	UInt16					reserved;
	BTreeKey				key;
};
typedef struct BTreeIterator BTreeIterator;
typedef BTreeIterator *BTreeIteratorPtr;


/*============================================================================
	B*Tree SPI
============================================================================*/

typedef SInt32		(* KeyCompareProcPtr)	(BTreeKeyPtr a, BTreeKeyPtr b);

typedef	OSStatus	(* GetBlockProcPtr)		(SFCB	*filePtr,
											 UInt32						 blockNum,
											 GetBlockOptions			 options,
											 BlockDescriptor			*block );
							 

typedef	OSStatus	(* ReleaseBlockProcPtr)	(SFCB		*filePtr,
											 BlockDescPtr				 blockPtr,
											 ReleaseBlockOptions		 options );

typedef	OSStatus	(* SetEndOfForkProcPtr)	(SFCB		 				*filePtr,
											 FSSize						 minEOF,
											 FSSize						 maxEOF );
								 
typedef	OSStatus	(* SetBlockSizeProcPtr)	(SFCB		 				*filePtr,
											 ByteCount					 blockSize );

OSStatus		SetEndOfForkProc ( SFCB		 				*filePtr, FSSize minEOF, FSSize maxEOF );



extern OSStatus	InitBTreeModule		(void);


extern OSStatus	BTInitialize		(SFCB						*filePtrPtr,
									 UInt16						 maxKeyLength,
									 UInt16						 nodeSize,
									 UInt8						 btreeType,
									 KeyDescriptorPtr			 keyDescPtr );

extern OSStatus	BTOpenPath			(SFCB		 				*filePtr,
									 KeyCompareProcPtr			 keyCompareProc,
									 GetBlockProcPtr			 getBlockProc,
									 ReleaseBlockProcPtr		 releaseBlockProc,
									 SetEndOfForkProcPtr		 setEndOfForkProc,
									 SetBlockSizeProcPtr		 setBlockSizeProc );

extern OSStatus	BTClosePath			(SFCB		 				*filePtr );


extern OSStatus	BTSearchRecord		(SFCB		 				*filePtr,
									 BTreeIterator				*searchIterator,
									 UInt32						heuristicHint,
									 FSBufferDescriptor			*btRecord,
									 UInt16						*recordLen,
									 BTreeIterator				*resultIterator );

extern OSStatus	BTIterateRecord		(SFCB		 				*filePtr,
									 BTreeIterationOperation	 operation,
									 BTreeIterator				*iterator,
									 FSBufferDescriptor			*btRecord,
									 UInt16						*recordLen );

extern OSStatus	BTInsertRecord		(SFCB		 				*filePtr,
									 BTreeIterator				*iterator,
									 FSBufferDescriptor			*btrecord,
									 UInt16						 recordLen );

extern OSStatus	BTReplaceRecord		(SFCB		 				*filePtr,
									 BTreeIterator				*iterator,
									 FSBufferDescriptor			*btRecord,
									 UInt16						 recordLen );

extern OSStatus	BTSetRecord			(SFCB		 				*filePtr,
									 BTreeIterator				*iterator,
									 FSBufferDescriptor			*btRecord,
									 UInt16						 recordLen );

extern OSStatus	BTDeleteRecord		(SFCB		 				*filePtr,
									 BTreeIterator				*iterator );

extern OSStatus	BTGetInformation	(SFCB		 				*filePtr,
									 UInt16						 version,
									 BTreeInfoRec				*info );

extern OSStatus	BTFlushPath			(SFCB		 				*filePtr );

extern OSStatus	BTInvalidateHint	(BTreeIterator				*iterator );

#endif // __BTREESINTERNAL__
