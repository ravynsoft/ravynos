/*
 * Copyright (c) 1999, 2002, 2006 Apple Computer, Inc. All rights reserved.
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
	File:		SBTree.c

	Contains:	Scavanger B-tree callback procs

	Version:	HFS Plus 1.0

	Copyright:	© 1996-1999 by Apple Computer, Inc., all rights reserved.

*/


#include "BTree.h"
#include "BTreePrivate.h"
#include "Scavenger.h"



// local routines
static void	InvalidateBTreeIterator ( SFCB *fcb );
static OSErr	CheckBTreeKey(const BTreeKey *key, const BTreeControlBlock *btcb);
static Boolean	ValidHFSRecord(const void *record, const BTreeControlBlock *btcb, UInt16 recordSize);


//	This function determines the size of the output buffer depending on
//	the type of the BTree.  It should really be taking the buffer size 
//	as input instead of assuming it (4425231).
//
//	This function may also truncate inline attribute record because caller 
//	sends HFSPlusAttrRecord as output buffer and this function also assumes 
//	the output buffer of size HFSPlusAttrRecord.  It may therefore not be 
//	enough to copy entire inline attribute record (4425232).

OSErr SearchBTreeRecord(SFCB *fcb, const void* key, UInt32 hint, void* foundKey, void* data, UInt16 *dataSize, UInt32 *newHint)
{
	FSBufferDescriptor	 btRecord;
	BTreeIterator		 searchIterator;
	BTreeIterator		*resultIterator;
	BTreeControlBlock	*btcb;
	OSStatus			 result;


	btcb = (BTreeControlBlock*) fcb->fcbBtree;

	btRecord.bufferAddress = data;
	btRecord.itemCount = 1;
	if ( btcb->maxKeyLength == kHFSExtentKeyMaximumLength )
		btRecord.itemSize = sizeof(HFSExtentRecord);
	else if ( btcb->maxKeyLength == kHFSPlusExtentKeyMaximumLength )
		btRecord.itemSize = sizeof(HFSPlusExtentRecord);
	else if ( btcb->maxKeyLength == kHFSPlusAttrKeyMaximumLength )
		btRecord.itemSize = sizeof(HFSPlusAttrRecord);
	else
		btRecord.itemSize = sizeof(CatalogRecord);

	searchIterator.hint.writeCount = 0;	// clear these out for debugging...
	searchIterator.hint.reserved1 = 0;
	searchIterator.hint.reserved2 = 0;

	searchIterator.hint.nodeNum = hint;
	searchIterator.hint.index = 0;

	result = CheckBTreeKey((BTreeKey *) key, btcb);
	ExitOnError(result);

	CopyMemory(key, &searchIterator.key, CalcKeySize(btcb, (BTreeKey *) key));		//¥¥ should we range check against maxkeylen?
	
	resultIterator = &btcb->lastIterator;

	result = BTSearchRecord( fcb, &searchIterator, kInvalidMRUCacheKey, &btRecord, dataSize, resultIterator );
	if (result == noErr)
	{
		if (newHint != NULL)
			*newHint = resultIterator->hint.nodeNum;

		result = CheckBTreeKey(&resultIterator->key, btcb);
		ExitOnError(result);

		if (foundKey != NULL)
			CopyMemory(&resultIterator->key, foundKey, CalcKeySize(btcb, &resultIterator->key));	//¥¥ warning, this could overflow user's buffer!!!

		if ( DEBUG_BUILD && !ValidHFSRecord(data, btcb, *dataSize) )
			DebugStr("\pSearchBTreeRecord: bad record?");
	}

ErrorExit:

	return result;
}



//	Note
//	The new B-tree manager differs from the original b-tree in how it does iteration. We need
//	to account for these differences here.  We save an iterator in the BTree control block so
//	that we have a context in which to perfrom the iteration. Also note that the old B-tree
//	allowed you to specify any number relative to the last operation (including 0) whereas the
//	new B-tree only allows next/previous.
//
//	This function determines the size of the output buffer depending on
//	the type of the BTree.  It should really be taking the buffer size 
//	as input instead of assuming it (4425231).
//
//	This function may also truncate inline attribute record because caller 
//	sends HFSPlusAttrRecord as output buffer and this function also assumes 
//	the output buffer of size HFSPlusAttrRecord.  It may therefore not be
//	enough to copy entire inline attribute record (4425232).

OSErr GetBTreeRecord(SFCB *fcb, SInt16 selectionIndex, void* key, void* data, UInt16 *dataSize, UInt32 *newHint)
{
	FSBufferDescriptor	btRecord;
	BTreeIterator		*iterator;
	BTreeControlBlock	*btcb;
	OSStatus			result;
	UInt16				operation;


	// pick up our iterator in the BTCB for context...

	btcb = (BTreeControlBlock*) fcb->fcbBtree;
	iterator = &btcb->lastIterator;

	btRecord.bufferAddress = data;
	btRecord.itemCount = 1;
	if ( btcb->maxKeyLength == kHFSExtentKeyMaximumLength )
		btRecord.itemSize = sizeof(HFSExtentRecord);
	else if ( btcb->maxKeyLength == kHFSPlusExtentKeyMaximumLength )
		btRecord.itemSize = sizeof(HFSPlusExtentRecord);
	else if ( btcb->maxKeyLength == kHFSPlusAttrKeyMaximumLength )
		btRecord.itemSize = sizeof(HFSPlusAttrRecord);
	else
		btRecord.itemSize = sizeof(CatalogRecord);
	
	// now we have to map index into next/prev operations...
	
	if (selectionIndex == 1)
	{
		operation = kBTreeNextRecord;
	}
	else if (selectionIndex == -1)
	{
		operation = kBTreePrevRecord;
	}
	else if (selectionIndex == 0)
	{
		operation = kBTreeCurrentRecord;
	}
	else if (selectionIndex == (SInt16) 0x8001)
	{
		operation = kBTreeFirstRecord;
	}
	else if (selectionIndex == (SInt16) 0x7FFF)
	{
		operation = kBTreeLastRecord;
	}
	else if (selectionIndex > 1)
	{
		UInt32	i;

		for (i = 1; i < selectionIndex; ++i)
		{
			result = BTIterateRecord( fcb, kBTreeNextRecord, iterator, &btRecord, dataSize );
			ExitOnError(result);
		}
		operation = kBTreeNextRecord;
	}
	else // (selectionIndex < -1)
	{
		SInt32	i;

		for (i = -1; i > selectionIndex; --i)
		{
			result = BTIterateRecord( fcb, kBTreePrevRecord, iterator, &btRecord, dataSize );
			ExitOnError(result);
		}
		operation = kBTreePrevRecord;
	}

	result = BTIterateRecord( fcb, operation, iterator, &btRecord, dataSize );

	if (result == noErr)
	{
		*newHint = iterator->hint.nodeNum;

		result = CheckBTreeKey(&iterator->key, btcb);
		ExitOnError(result);
		
		CopyMemory(&iterator->key, key, CalcKeySize(btcb, &iterator->key));	//¥¥ warning, this could overflow user's buffer!!!
		
		if ( DEBUG_BUILD && !ValidHFSRecord(data, btcb, *dataSize) )
			DebugStr("\pGetBTreeRecord: bad record?");

	}
	
ErrorExit:

	return result;
}


OSErr InsertBTreeRecord(SFCB *fcb, const void* key, const void* data, UInt16 dataSize, UInt32 *newHint)
{
	FSBufferDescriptor	btRecord;
	BTreeIterator		iterator;
	BTreeControlBlock	*btcb;
	OSStatus			result;
	

	btcb = (BTreeControlBlock*) fcb->fcbBtree;

	btRecord.bufferAddress = (void *)data;
	btRecord.itemSize = dataSize;
	btRecord.itemCount = 1;

	iterator.hint.nodeNum = 0;			// no hint

	result = CheckBTreeKey((BTreeKey *) key, btcb);
	ExitOnError(result);

	CopyMemory(key, &iterator.key, CalcKeySize(btcb, (BTreeKey *) key));	//¥¥ should we range check against maxkeylen?

	if ( DEBUG_BUILD && !ValidHFSRecord(data, btcb, dataSize) )
		DebugStr("\pInsertBTreeRecord: bad record?");

	result = BTInsertRecord( fcb, &iterator, &btRecord, dataSize );

	*newHint = iterator.hint.nodeNum;

	InvalidateBTreeIterator(fcb);	// invalidate current record markers
	
ErrorExit:

	return result;
}


OSErr DeleteBTreeRecord(SFCB *fcb, const void* key)
{
	BTreeIterator		iterator;
	BTreeControlBlock	*btcb;
	OSStatus			result;
	

	btcb = (BTreeControlBlock*) fcb->fcbBtree;
	
	iterator.hint.nodeNum = 0;			// no hint

	result = CheckBTreeKey((BTreeKey *) key, btcb);
	ExitOnError(result);

	CopyMemory(key, &iterator.key, CalcKeySize(btcb, (BTreeKey *) key));	//¥¥ should we range check against maxkeylen?

	result = BTDeleteRecord( fcb, &iterator );

	InvalidateBTreeIterator(fcb);	// invalidate current record markers

ErrorExit:

	return result;
}


OSErr ReplaceBTreeRecord(SFCB *fcb, const void* key, UInt32 hint, void *newData, UInt16 dataSize, UInt32 *newHint)
{
	FSBufferDescriptor	btRecord;
	BTreeIterator		iterator;
	BTreeControlBlock	*btcb;
	OSStatus			result;


	btcb = (BTreeControlBlock*) fcb->fcbBtree;

	btRecord.bufferAddress = newData;
	btRecord.itemSize = dataSize;
	btRecord.itemCount = 1;

	iterator.hint.nodeNum = hint;

	result = CheckBTreeKey((BTreeKey *) key, btcb);
	ExitOnError(result);

	CopyMemory(key, &iterator.key, CalcKeySize(btcb, (BTreeKey *) key));		//¥¥ should we range check against maxkeylen?

	if ( DEBUG_BUILD && !ValidHFSRecord(newData, btcb, dataSize) )
		DebugStr("\pReplaceBTreeRecord: bad record?");

	result = BTReplaceRecord( fcb, &iterator, &btRecord, dataSize );

	*newHint = iterator.hint.nodeNum;

	//¥¥Êdo we need to invalidate the iterator?

ErrorExit:

	return result;
}


OSStatus
SetEndOfForkProc ( SFCB *filePtr, FSSize minEOF, FSSize maxEOF )
{
#pragma unused (maxEOF)

	OSStatus	result;
	UInt32		actualSectorsAdded;
	UInt64		bytesToAdd;
	UInt64		fileSize;				//	in sectors
	SVCB *		vcb;
	UInt32		flags;


	if ( minEOF > filePtr->fcbLogicalSize )
	{
		bytesToAdd = minEOF - filePtr->fcbLogicalSize;

		if (bytesToAdd < filePtr->fcbClumpSize)
			bytesToAdd = filePtr->fcbClumpSize;		//¥¥Êwhy not always be a mutiple of clump size ???
	}
	else
	{
		if ( DEBUG_BUILD )
			DebugStr("\pSetEndOfForkProc: minEOF is smaller than current size!");
		return -1;
	}

	vcb = filePtr->fcbVolume;
	
	flags = kEFNoClumpMask;
	
	// Due to time contraints we force the new rebuilt catalog file to be contiguous.
	// It's hard to handle catalog file in extents because we have to do a swap 
	// of the old catalog file with the rebuilt catalog file at the end of
	// the rebuild process.  Extent records use the file ID as part of the key so 
	// it would be messy to fix them after the swap.
	if ( filePtr->fcbFileID == kHFSRepairCatalogFileID)
		flags |= kEFNoExtOvflwMask;
	
	result = ExtendFileC ( vcb, filePtr, (UInt32)((bytesToAdd+511)>>9), flags, &actualSectorsAdded );
	ReturnIfError(result);

	filePtr->fcbLogicalSize = filePtr->fcbPhysicalSize;	// new B-tree looks at fcbEOF
	fileSize = filePtr->fcbLogicalSize >> 9;		// get size in sectors (for calls to ZeroFileBlocks)
	
	//
	//	Make sure we got at least as much space as we needed
	//
	if (filePtr->fcbLogicalSize < minEOF) {
		Panic("\pSetEndOfForkProc: disk too full to extend B-tree file");
		return dskFulErr;
	}
	
	//
	//	Update the Alternate MDB or Alternate HFSPlusVolumeHeader
	//
	if ( vcb->vcbSignature == kHFSPlusSigWord )
	{
		//	If any of the HFS+ private files change size, flush them back to the Alternate volume header
		if (	(filePtr->fcbFileID == kHFSExtentsFileID) 
			 ||	(filePtr->fcbFileID == kHFSCatalogFileID)
			 ||	(filePtr->fcbFileID == kHFSStartupFileID)
			 ||	(filePtr->fcbFileID == kHFSAttributesFileID)
			 ||	(filePtr->fcbFileID == kHFSRepairCatalogFileID) )
		{
			MarkVCBDirty( vcb );
			result = FlushAlternateVolumeControlBlock( vcb, true );
			
			//	Zero newly allocated portion of HFS+ private file.
			if ( result == noErr )
				result = ZeroFileBlocks( vcb, filePtr, (UInt32)(fileSize - actualSectorsAdded), actualSectorsAdded );
		}
	}
	else if ( vcb->vcbSignature == kHFSSigWord )
	{
		if ( filePtr->fcbFileID == kHFSExtentsFileID )
		{
		//	vcb->vcbXTAlBlks = filePtr->fcbPhysicalSize / vcb->vcbBlockSize;
			MarkVCBDirty( vcb );
			result = FlushAlternateVolumeControlBlock( vcb, false );
			if ( result == noErr )
				result = ZeroFileBlocks( vcb, filePtr, (UInt32)(fileSize - actualSectorsAdded), actualSectorsAdded );
		}
		else if ( filePtr->fcbFileID == kHFSCatalogFileID || filePtr->fcbFileID == kHFSRepairCatalogFileID )
		{
		//	vcb->vcbCTAlBlks = filePtr->fcbPhysicalSize / vcb->vcbBlockSize;
			MarkVCBDirty( vcb );
			result = FlushAlternateVolumeControlBlock( vcb, false );
			if ( result == noErr )
				result = ZeroFileBlocks( vcb, filePtr, (UInt32)(fileSize - actualSectorsAdded), actualSectorsAdded );
		}
	}
	
	return result;

} // end SetEndOfForkProc


static void
InvalidateBTreeIterator(SFCB *fcb)
{
	BTreeControlBlock	*btcb;

	btcb = (BTreeControlBlock*) fcb->fcbBtree;

	(void) BTInvalidateHint	( &btcb->lastIterator );
}


static OSErr CheckBTreeKey(const BTreeKey *key, const BTreeControlBlock *btcb)
{
	UInt16	keyLen;
	
	if ( btcb->attributes & kBTBigKeysMask )
		keyLen = key->length16;
	else
		keyLen = key->length8;

	if ( (keyLen < 6) || (keyLen > btcb->maxKeyLength) )
	{
		if ( DEBUG_BUILD )
			DebugStr("\pCheckBTreeKey: bad key length!");
		return fsBTInvalidKeyLengthErr;
	}
	
	return noErr;
}


static Boolean ValidHFSRecord(const void *record, const BTreeControlBlock *btcb, UInt16 recordSize)
{
	UInt32			cNodeID;
	
	if ( btcb->maxKeyLength == kHFSExtentKeyMaximumLength )
	{
		return ( recordSize == sizeof(HFSExtentRecord) );
	}
	else if (btcb->maxKeyLength == kHFSPlusExtentKeyMaximumLength )
	{
		return ( recordSize == sizeof(HFSPlusExtentRecord) );
	}
	else if (btcb->maxKeyLength == kAttributeKeyMaximumLength )
	{
		HFSPlusAttrRecord	*attributeRecord = (HFSPlusAttrRecord *) record;
		
		switch (attributeRecord->recordType) {
			case kHFSPlusAttrInlineData:
				break;
			
			case kHFSPlusAttrForkData:
				break;
			
			case kHFSPlusAttrExtents:
				break;
		}
	}
	else // Catalog record
	{
		CatalogRecord *catalogRecord = (CatalogRecord*) record;

		switch(catalogRecord->recordType)
		{
			case kHFSFolderRecord:
			{
				if ( recordSize != sizeof(HFSCatalogFolder) )
					return false;
				if ( catalogRecord->hfsFolder.flags != 0 )
					return false;
				if ( catalogRecord->hfsFolder.valence > 0x7FFF )
					return false;
					
				cNodeID = catalogRecord->hfsFolder.folderID;
	
				if ( (cNodeID == 0) || (cNodeID < 16 && cNodeID > 2) )
					return false;
			}
			break;

			case kHFSPlusFolderRecord:
			{
				if ( recordSize != sizeof(HFSPlusCatalogFolder) )
					return false;
				if ( (catalogRecord->hfsPlusFolder.flags & (kHFSFileLockedMask | kHFSThreadExistsMask)) != 0 )
					return false;
					
				cNodeID = catalogRecord->hfsPlusFolder.folderID;
	
				if ( (cNodeID == 0) || (cNodeID < 16 && cNodeID > 2) )
					return false;
			}
			break;
	
			case kHFSFileRecord:
			{
				UInt16					i;
				HFSExtentDescriptor	*dataExtent;
				HFSExtentDescriptor	*rsrcExtent;
				
				if ( recordSize != sizeof(HFSCatalogFile) )
					return false;								
				if ( (catalogRecord->hfsFile.flags & ~(0x83)) != 0 )
					return false;
					
				cNodeID = catalogRecord->hfsFile.fileID;
				
				if ( cNodeID < 16 )
					return false;
		
				// make sure 0 ² LEOF ² PEOF for both forks
				
				if ( catalogRecord->hfsFile.dataLogicalSize < 0 )
					return false;
				if ( catalogRecord->hfsFile.dataPhysicalSize < catalogRecord->hfsFile.dataLogicalSize )
					return false;
				if ( catalogRecord->hfsFile.rsrcLogicalSize < 0 )
					return false;
				if ( catalogRecord->hfsFile.rsrcPhysicalSize < catalogRecord->hfsFile.rsrcLogicalSize )
					return false;
		
				dataExtent = (HFSExtentDescriptor*) &catalogRecord->hfsFile.dataExtents;
				rsrcExtent = (HFSExtentDescriptor*) &catalogRecord->hfsFile.rsrcExtents;
	
				for (i = 0; i < kHFSExtentDensity; ++i)
				{
					if ( (dataExtent[i].blockCount > 0) && (dataExtent[i].startBlock == 0) )
						return false;
					if ( (rsrcExtent[i].blockCount > 0) && (rsrcExtent[i].startBlock == 0) )
						return false;
				}
			}
			break;
	
			case kHFSPlusFileRecord:
			{
				UInt16					i;
				HFSPlusExtentDescriptor	*dataExtent;
				HFSPlusExtentDescriptor	*rsrcExtent;
				
				if ( recordSize != sizeof(HFSPlusCatalogFile) )
					return false;								
					
				cNodeID = catalogRecord->hfsPlusFile.fileID;
				
				if ( cNodeID < 16 )
					return false;
		
				//¥¥ make sure 0 ² LEOF ² PEOF for both forks
						
				dataExtent = (HFSPlusExtentDescriptor*) &catalogRecord->hfsPlusFile.dataFork.extents;
				rsrcExtent = (HFSPlusExtentDescriptor*) &catalogRecord->hfsPlusFile.resourceFork.extents;
	
				for (i = 0; i < kHFSPlusExtentDensity; ++i)
				{
					if ( (dataExtent[i].blockCount > 0) && (dataExtent[i].startBlock == 0) )
						return false;
					if ( (rsrcExtent[i].blockCount > 0) && (rsrcExtent[i].startBlock == 0) )
						return false;
				}
			}
			break;

			case kHFSFolderThreadRecord:
			case kHFSFileThreadRecord:
			{
				if ( recordSize != sizeof(HFSCatalogThread) )
					return false;
	
				cNodeID = catalogRecord->hfsThread.parentID;
				if ( (cNodeID == 0) || (cNodeID < 16 && cNodeID > 2) )
					return false;
							
				if ( (catalogRecord->hfsThread.nodeName[0] == 0) ||
					 (catalogRecord->hfsThread.nodeName[0] > 31) )
					return false;
			}
			break;
		
			case kHFSPlusFolderThreadRecord:
			case kHFSPlusFileThreadRecord:
			{
				if ( recordSize > sizeof(HFSPlusCatalogThread) || recordSize < (sizeof(HFSPlusCatalogThread) - sizeof(HFSUniStr255)))
					return false;
	
				cNodeID = catalogRecord->hfsPlusThread.parentID;
				if ( (cNodeID == 0) || (cNodeID < 16 && cNodeID > 2) )
					return false;
							
				if ( (catalogRecord->hfsPlusThread.nodeName.length == 0) ||
					 (catalogRecord->hfsPlusThread.nodeName.length > 255) )
					return false;
			}
			break;

			default:
				return false;
		}
	}
	
	return true;	// record appears to be OK
}

