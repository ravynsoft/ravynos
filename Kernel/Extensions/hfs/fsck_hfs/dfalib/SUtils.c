/*
 * Copyright (c) 1999-2003, 2005-2008 Apple Inc. All rights reserved.
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
	File:		SUtils.c

	Contains:	xxx put contents here xxx

	Version:	xxx put version here xxx

	Copyright:	© 1997-1999 by Apple Computer, Inc., all rights reserved.
*/

#include "Scavenger.h"

static void 	CompareVolHeaderBTreeSizes(	SGlobPtr GPtr,
											VolumeObjectPtr theVOPtr, 
											HFSPlusVolumeHeader * thePriVHPtr, 
											HFSPlusVolumeHeader * theAltVHPtr );
static void 	GetEmbeddedVolumeHeaders( 	SGlobPtr GPtr, 
											HFSMasterDirectoryBlock * myMDBPtr,
											Boolean isPrimaryMDB );
static OSErr 	GetVolumeObjectBlock( VolumeObjectPtr theVOPtr,
									  UInt64 theBlockNum,
								      BlockDescriptor * theBlockDescPtr );
static OSErr 	VolumeObjectFixPrimaryBlock( void );
		
/*
 * utf_encodestr
 *
 * Encode a UCS-2 (Unicode) string to UTF-8
 */
int utf_encodestr(ucsp, ucslen, utf8p, utf8len, utf8plen)
	const u_int16_t * ucsp;
	size_t ucslen;
	unsigned char * utf8p;
	size_t * utf8len;
	size_t utf8plen;
{
	unsigned char * bufstart;
	u_int16_t ucs_ch;
	size_t charcnt;
	
	bufstart = utf8p;
	charcnt = ucslen / 2;

	while (charcnt-- > 0 && utf8plen > 0) {
		ucs_ch = *ucsp++;

		if (ucs_ch < 0x0080) {
			if (ucs_ch == '\0')
				continue;	/* skip over embedded NULLs */

			*utf8p++ = ucs_ch;
			utf8plen--;
		} else if (ucs_ch < 0x800) {
			if (utf8plen < 2)	/* We're about to over-flow the buffer */
				break;
			utf8plen -= 2;
			*utf8p++ = (ucs_ch >> 6)   | 0xc0;
			*utf8p++ = (ucs_ch & 0x3f) | 0x80;
		} else {	
			if (utf8plen < 3)	/* We're about to over-flow the buffer */
				break;
			utf8plen -= 3;
			*utf8p++ = (ucs_ch >> 12)         | 0xe0;
			*utf8p++ = ((ucs_ch >> 6) & 0x3f) | 0x80;
			*utf8p++ = ((ucs_ch) & 0x3f)      | 0x80;
		}
	}
	
	*utf8len = utf8p - bufstart;

	return (0);
}


/*
 * utf_decodestr
 *
 * Decode a UTF-8 string back to UCS-2 (Unicode)
 *
 * N.B.:  ucslen on input describes the length of the buffer;
 * on return, it describes how many bytes were used.
 */
int
utf_decodestr(utf8p, utf8len, ucsp, ucslen, ucsplen)
	const unsigned char * utf8p;
	size_t utf8len;
	u_int16_t* ucsp;
	size_t *ucslen;
	size_t ucsplen;
{
	u_int16_t* bufstart;
	u_int16_t ucs_ch;
	u_int8_t byte;

	bufstart = ucsp;

	while (utf8len-- > 0 && (byte = *utf8p++) != '\0' && ucsplen > 0) {
		/* check for ascii */
		if (byte < 0x80) {
			*ucsp++ = byte;
			ucsplen--;
			continue;
		}

		switch (byte & 0xf0) {
		/*  2 byte sequence*/
		case 0xc0:
		case 0xd0:
			/* extract bits 6 - 10 from first byte */
			ucs_ch = (byte & 0x1F) << 6;  
			if (ucs_ch < 0x0080)
				return (-1); /* seq not minimal */
			break;
		/* 3 byte sequence*/
		case 0xe0:
			/* extract bits 12 - 15 from first byte */
			ucs_ch = (byte & 0x0F) << 6;

			/* extract bits 6 - 11 from second byte */
			if (((byte = *utf8p++) & 0xc0) != 0x80)
				return (-1);

			utf8len--;
			ucs_ch += (byte & 0x3F);
			ucs_ch <<= 6;
			if (ucs_ch < 0x0800)
				return (-1); /* seq not minimal */
			break;
		default:
			return (-1);
		}

		/* extract bits 0 - 5 from final byte */
		if (((byte = *utf8p++) & 0xc0) != 0x80)
			return (-1);

		utf8len--;
		ucs_ch += (byte & 0x3F);  
		*ucsp++ = ucs_ch;
		ucsplen--;
	}

	*ucslen = (u_int8_t*)ucsp - (u_int8_t*)bufstart;

	return (0);
}


OSErr GetFBlk( SGlobPtr GPtr, SInt16 fileRefNum, SInt32 blockNumber, void **bufferH );


UInt32 gDFAStage;

UInt32	GetDFAStage( void )
{	
	return (gDFAStage);
}

void	SetDFAStage( UInt32 stage )
{
	gDFAStage = stage;
}


/*------------------------------------------------------------------------------

Routine:	RcdError

Function:	Record errors detetected by scavenging operation.
			
Input:		GPtr		-	pointer to scavenger global area.
			ErrCode		-	error code

Output:		None			
------------------------------------------------------------------------------*/

void RcdError( SGlobPtr GPtr, OSErr errorCode )
{
	GPtr->ErrCode = errorCode;
	
	WriteError( GPtr, errorCode, GPtr->TarID, GPtr->TarBlock );	//	log to summary window
}


/*------------------------------------------------------------------------------

Routine:	IntError

Function:	Records an internal Scavenger error.
			
Input:		GPtr		-	pointer to scavenger global area.
			ErrCode		-	internal error code

Output:		IntError	-	function result:			
								(E_IntErr for now)
------------------------------------------------------------------------------*/

int IntError( SGlobPtr GPtr, OSErr errorCode )
{
	GPtr->RepLevel = repairLevelUnrepairable;
	
	if ( errorCode == ioErr )				//	Cast I/O errors as read errors
		errorCode	= R_RdErr;
		
	if( (errorCode == R_RdErr) || (errorCode == R_WrErr) )
	{
		GPtr->ErrCode	= GPtr->volumeErrorCode;
		GPtr->IntErr	= 0;
		return( errorCode );		
	}
	else
	{
		GPtr->ErrCode	= R_IntErr;
		GPtr->IntErr	= errorCode;
		return( R_IntErr );
	}
	
}	//	End of IntError



/*------------------------------------------------------------------------------

Routine:	AllocBTN (Allocate BTree Node)

Function:	Allocates an BTree node in a Scavenger BTree bit map.
			
Input:		GPtr		-	pointer to scavenger global area.
			StABN		-	starting allocation block number.
			NmABlks		-	number of allocation blocks.

Output:		AllocBTN	-	function result:			
								0 = no error
								n = error
------------------------------------------------------------------------------*/

int AllocBTN( SGlobPtr GPtr, SInt16 fileRefNum, UInt32 nodeNumber )
{
	UInt16				bitPos;
	unsigned char		mask;
	char				*byteP;
	BTreeControlBlock	*calculatedBTCB	= GetBTreeControlBlock( fileRefNum );

	//	Allocate the node 
	if ( calculatedBTCB->refCon == 0)
		return( noErr );
		
	byteP = ( (BTreeExtensionsRec*)calculatedBTCB->refCon)->BTCBMPtr + (nodeNumber / 8 );	//	ptr to starting byte
	bitPos = nodeNumber % 8;						//	bit offset
	mask = ( 0x80 >> bitPos );
	if ( (*byteP & mask) != 0 )
	{	
		RcdError( GPtr, E_OvlNode );
		return( E_OvlNode );					//	node already allocated
	}
	*byteP = *byteP | mask;						//	allocate it
	calculatedBTCB->freeNodes--;		//	decrement free count
	
	return( noErr );
}


OSErr	GetBTreeHeader( SGlobPtr GPtr, SFCB *fcb, BTHeaderRec *header )
{
	OSErr err;
	BTHeaderRec *headerRec;
	BlockDescriptor block;

	GPtr->TarBlock = kHeaderNodeNum;

	if (fcb->fcbBlockSize == 0)
		(void) SetFileBlockSize(fcb, 512);

	err = GetFileBlock(fcb, kHeaderNodeNum, kGetBlock, &block);
	ReturnIfError(err);

	err = hfs_swap_BTNode(&block, fcb, kSwapBTNodeHeaderRecordOnly);
	if (err != noErr)
	{
		(void) ReleaseFileBlock(fcb, &block, kReleaseBlock | kTrashBlock);
		return err;
	}

	headerRec = (BTHeaderRec *)((char*)block.buffer + sizeof(BTNodeDescriptor));
	CopyMemory(headerRec, header, sizeof(BTHeaderRec));

	err = hfs_swap_BTNode(&block, fcb, kSwapBTNodeHeaderRecordOnly);
	if (err != noErr)
	{
		(void) ReleaseFileBlock(fcb, &block, kReleaseBlock | kTrashBlock);
		return err;
	}
	
	err = ReleaseFileBlock (fcb, &block, kReleaseBlock);
	ReturnIfError(err);
	
	/* Validate Node Size */
	switch (header->nodeSize) {
		case   512:
		case  1024:
		case  2048:
		case  4096:
		case  8192:
		case 16384:
		case 32768:
			break;

		default:
			RcdError( GPtr, E_InvalidNodeSize );
			err = E_InvalidNodeSize;
	}

	return( err );
}


/*------------------------------------------------------------------------------

Routine:	IsDuplicateRepairOrder

Function:	Search a duplicate repair order node in the GPtr->MinorRepairP 
		list.  This function traverses the entire list of minor 
		repairs, and compares the following fields to determine a 
		match - type, forkType, correct, incorrect, maskBit, hint,
		and parentID.

Input:		GPtr	- scavenger globals
		orig	- repair order to search and compare

Output:		0 - no duplicate was found,
		1 - duplicate was found.
------------------------------------------------------------------------------*/
int IsDuplicateRepairOrder(SGlobPtr GPtr, RepairOrderPtr orig) 
{
	RepairOrderPtr cur;
	int retval = 0;

	cur = GPtr->MinorRepairsP;
	while (cur) {
		if (cur != orig) {
			/* If all these values match, this is a duplicate */
			if ((orig->type == cur->type) &&
			    (orig->correct == cur->correct) &&
			    (orig->incorrect == cur->incorrect) &&
			    (orig->parid == cur->parid) &&
			    (orig->forkType == cur->forkType) &&
			    (orig->maskBit == cur->maskBit) &&
			    (orig->hint == cur->hint)) {
				retval = 1;
				break;
			}
		}
		cur = cur->link;
	}
	
	return retval;
}

/*------------------------------------------------------------------------------

Routine:	DeleteRepairOrder

Function:	Deletes the minor repair order that matches the repair order 
		provided from the list.  This function should be called when 
		a duplicate repair order is detected.  

Input:		GPtr	- scavenger globals
		orig	- repair order to remove

Output:		Nothing
------------------------------------------------------------------------------*/
void DeleteRepairOrder(SGlobPtr GPtr, RepairOrderPtr orig)
{
	RepairOrderPtr cur;
	RepairOrderPtr prev = NULL;

	cur = GPtr->MinorRepairsP;
	while (cur) {
		if (cur == orig) {
			if (prev) {
				prev->link = cur->link;
			}
			if (cur == GPtr->MinorRepairsP) {
				GPtr->MinorRepairsP = cur->link;
			}
			DisposeMemory(cur);
		}
		prev = cur;
		cur = cur->link;
	}

	return;
}


/*------------------------------------------------------------------------------

Routine:	Alloc[Minor/Major]RepairOrder

Function:	Allocate a repair order node and link into the 'GPtr->RepairXxxxxP" list.
			These are descriptions of minor/major repairs that need to be performed;
			they are compiled during verification, and executed during minor/major repair.

Input:		GPtr	- scavenger globals
			n		- number of extra bytes needed, in addition to standard node size.

Output:		Ptr to node, or NULL if out of memory or other error.
------------------------------------------------------------------------------*/

RepairOrderPtr AllocMinorRepairOrder( SGlobPtr GPtr, size_t n )						/* #extra bytes needed */
{
	RepairOrderPtr	p;							//	the node we allocate
	
	n += sizeof( RepairOrder );					//	add in size of basic node
	
	p = (RepairOrderPtr) AllocateClearMemory( n );		//	get the node
	
	if ( p != NULL )							//	if we got one...
	{
		p->link = GPtr->MinorRepairsP;			//	then link into list of repairs
		GPtr->MinorRepairsP = p;
	}
    else if ( fsckGetVerbosity(GPtr->context) >= kDebugLog )
       	plog( "\t%s - AllocateClearMemory failed to allocate %d bytes \n", __FUNCTION__, n);
	
	if ( GPtr->RepLevel == repairLevelNoProblemsFound )
		GPtr->RepLevel = repairLevelVolumeRecoverable;

	return( p );								//	return ptr to node
}



void	InvalidateCalculatedVolumeBitMap( SGlobPtr GPtr )
{

}



//------------------------------------------------------------------------------
//	Routine:	GetVolumeFeatures
//
//	Function:	Sets up some OS and volume specific flags
//
//	Input:		GPtr->DrvNum			The volume to check
//			
//	Output:		GPtr->volumeFeatures	Bit vector
//				GPtr->realVCB			Real in-memory vcb
//------------------------------------------------------------------------------

#if !BSD	
OSErr GetVolumeFeatures( SGlobPtr GPtr )
{
	OSErr					err;
	HParamBlockRec			pb;
	GetVolParmsInfoBuffer	buffer;
	long					response;

	GPtr->volumeFeatures	= 0;					//	Initialize to zero

	//	Get the "real" vcb
	err = GetVCBDriveNum( &GPtr->realVCB, GPtr->DrvNum );
	ReturnIfError( err );

	if ( GPtr->realVCB != nil )
	{
		GPtr->volumeFeatures	|= volumeIsMountedMask;

		pb.ioParam.ioNamePtr	= nil;
		pb.ioParam.ioVRefNum	= GPtr->realVCB->vcbVRefNum;
		pb.ioParam.ioBuffer		= (Ptr) &buffer;
		pb.ioParam.ioReqCount	= sizeof( buffer );
		
		if ( PBHGetVolParms( &pb, false ) == noErr )
		{
			if ( buffer.vMAttrib & (1 << bSupportsTrashVolumeCache) )
				GPtr->volumeFeatures	|= supportsTrashVolumeCacheFeatureMask;
		}
	}
	//	Check if the running system is HFS+ savy
	err = Gestalt( gestaltFSAttr, &response );
	ReturnIfError( err );
	if ( (response & (1 << gestaltFSSupportsHFSPlusVols)) != 0 )
		GPtr->volumeFeatures |= supportsHFSPlusVolsFeatureMask;
	
	return( noErr );
}
#endif



/*-------------------------------------------------------------------------------
Routine:	ClearMemory	-	clear a block of memory

-------------------------------------------------------------------------------*/
#if !BSD
void ClearMemory( void* start, UInt32 length )
{
	UInt32		zero = 0;
	UInt32*		dataPtr;
	UInt8*		bytePtr;
	UInt32		fragCount;		// serves as both a length and quadlong count
								// for the beginning and main fragment
	
	if ( length == 0 )
		return;

	// is request less than 4 bytes?
	if ( length < 4 )				// length = 1,2 or 3
	{
		bytePtr = (UInt8 *) start;
		
		do
		{
			*bytePtr++ = zero;		// clear one byte at a time
		}
		while ( --length );

		return;
	}

	// are we aligned on an odd boundry?
	fragCount = (UInt32) start & 3;

	if ( fragCount )				// fragCount = 1,2 or 3
	{
		bytePtr = (UInt8 *) start;
		
		do
		{
			*bytePtr++ = zero;		// clear one byte at a time
			++fragCount;
			--length;
		}
		while ( (fragCount < 4) && (length > 0) );

		if ( length == 0 )
			return;

		dataPtr = (UInt32*) (((UInt32) start & 0xFFFFFFFC) + 4);	// make it long word aligned
	}
	else
	{
		dataPtr = (UInt32*) ((UInt32) start & 0xFFFFFFFC);			// make it long word aligned
	}

	// At this point dataPtr is long aligned

	// are there odd bytes to copy?
	fragCount = length & 3;
	
	if ( fragCount )
	{
		bytePtr = (UInt8 *) ((UInt32) dataPtr + (UInt32) length - 1);	// point to last byte
		
		length -= fragCount;		// adjust remaining length
		
		do
		{
			*bytePtr-- = zero;		// clear one byte at a time
		}
		while ( --fragCount );

		if ( length == 0 )
			return;
	}

	// At this point length is a multiple of 4

	#if DEBUG_BUILD
	  if ( length < 4 )
		 DebugStr("\p ClearMemory: length < 4");
	#endif

	// fix up beginning to get us on a 64 byte boundary
	fragCount = length & (64-1);
	
	#if DEBUG_BUILD
	  if ( fragCount < 4 && fragCount > 0 )
		  DebugStr("\p ClearMemory: fragCount < 4");
	#endif
	
	if ( fragCount )
	{
		length -= fragCount; 		// subtract fragment from length now
		fragCount >>= 2; 			// divide by 4 to get a count, for DBRA loop
		do
		{
			// clear 4 bytes at a time...
			*dataPtr++ = zero;		
		}
		while (--fragCount);
	}

	// Are we finished yet?
	if ( length == 0 )
		return;
	
	// Time to turn on the fire hose
	length >>= 6;		// divide by 64 to get count
	do
	{
		// spray 64 bytes at a time...
		*dataPtr++ = zero; *dataPtr++ = zero; *dataPtr++ = zero; *dataPtr++ = zero;
		*dataPtr++ = zero; *dataPtr++ = zero; *dataPtr++ = zero; *dataPtr++ = zero;
		*dataPtr++ = zero; *dataPtr++ = zero; *dataPtr++ = zero; *dataPtr++ = zero;
		*dataPtr++ = zero; *dataPtr++ = zero; *dataPtr++ = zero; *dataPtr++ = zero;
	}
	while (--length);
}
#endif



void
CopyCatalogName(const CatalogName *srcName, CatalogName *dstName, Boolean isHFSPLus)
{
	size_t	length;
	
	if ( srcName == NULL )
	{
		if ( dstName != NULL )
			dstName->ustr.length = 0;	// set length byte to zero (works for both unicode and pascal)		
		return;
	}
	
	if (isHFSPLus)
		length = sizeof(UniChar) * (srcName->ustr.length + 1);
	else
		length = sizeof(UInt8) + srcName->pstr[0];

	if ( length > 1 )
		CopyMemory(srcName, dstName, length);
	else
		dstName->ustr.length = 0;	// set length byte to zero (works for both unicode and pascal)		
}


UInt32
CatalogNameLength(const CatalogName *name, Boolean isHFSPlus)
{
	if (isHFSPlus)
		return name->ustr.length;
	else
		return name->pstr[0];
}


UInt32	CatalogNameSize( const CatalogName *name, Boolean isHFSPlus)
{
	UInt32	length = CatalogNameLength( name, isHFSPlus );
	
	if ( isHFSPlus )
		length *= sizeof(UniChar);
	
	return( length );
}


//******************************************************************************
//	Routine:	BuildCatalogKey
//
//	Function: 	Constructs a catalog key record (ckr) given the parent
//				folder ID and CName.  Works for both classic and extended
//				HFS volumes.
//
//******************************************************************************

void
BuildCatalogKey(HFSCatalogNodeID parentID, const CatalogName *cName, Boolean isHFSPlus, CatalogKey *key)
{
	if ( isHFSPlus )
	{
		key->hfsPlus.keyLength		= kHFSPlusCatalogKeyMinimumLength;	// initial key length (4 + 2)
		key->hfsPlus.parentID			= parentID;		// set parent ID
		key->hfsPlus.nodeName.length	= 0;			// null CName length
		if ( cName != NULL )
		{
			CopyCatalogName(cName, (CatalogName *) &key->hfsPlus.nodeName, isHFSPlus);
			key->hfsPlus.keyLength += sizeof(UniChar) * cName->ustr.length;	// add CName size to key length
		}
	}
	else
	{
		key->hfs.keyLength	= kHFSCatalogKeyMinimumLength;	// initial key length (1 + 4 + 1)
		key->hfs.reserved		= 0;				// clear unused byte
		key->hfs.parentID		= parentID;			// set parent ID
		key->hfs.nodeName[0]	= 0;				// null CName length
		if ( cName != NULL )
		{
			UpdateCatalogName(cName->pstr, key->hfs.nodeName);
			key->hfs.keyLength += key->hfs.nodeName[0];		// add CName size to key length
		}
	}
}


//		Defined in BTreesPrivate.h, but not implemented in the BTree code?
//		So... here's the implementation
SInt32	CompareKeys( BTreeControlBlockPtr btreePtr, KeyPtr searchKey, KeyPtr trialKey )
{
	KeyCompareProcPtr	compareProc = (KeyCompareProcPtr)btreePtr->keyCompareProc;
	
	return( compareProc(searchKey, trialKey) );
}


void
UpdateCatalogName(ConstStr31Param srcName, Str31 destName)
{
	Size length = srcName[0];
	
	if (length > kHFSMaxFileNameChars)
		length = kHFSMaxFileNameChars;		// truncate to max

	destName[0] = length;					// set length byte
	
	CopyMemory(&srcName[1], &destName[1], length);
}


void
UpdateVolumeEncodings(SVCB *volume, TextEncoding encoding)
{
	UInt32	index;

	encoding &= 0x7F;
	
	index = MapEncodingToIndex(encoding);

	volume->vcbEncodingsBitmap |= (u_int64_t)(1ULL << index);
		
	// vcb should already be marked dirty
}


//******************************************************************************
//	Routine:	VolumeObjectFixPrimaryBlock
//
//	Function:	Use the alternate Volume Header or Master Directory block (depending
//				on the type of volume) to restore the primary block.  This routine
//				depends upon our intialization code to set up where are blocks are
//				located.  
//
// 	Result:		0 if all is well, noMacDskErr when we do not have a primary block 
//				number or whatever GetVolumeObjectAlternateBlock returns.
//******************************************************************************

static OSErr VolumeObjectFixPrimaryBlock( void )
{
	OSErr				err;
	VolumeObjectPtr		myVOPtr;
	UInt64				myPrimaryBlockNum;
	BlockDescriptor  	myPrimary;
	BlockDescriptor  	myAlternate;

	myVOPtr = GetVolumeObjectPtr( );
	myPrimary.buffer = NULL;
	myAlternate.buffer = NULL;
		
	GetVolumeObjectPrimaryBlockNum( &myPrimaryBlockNum );
	if ( myPrimaryBlockNum == 0 )
		return( noMacDskErr );
		
	// we don't care if this is a valid primary block since we're
	// about to write over it
	err = GetVolumeObjectPrimaryBlock( &myPrimary );
	if ( !(err == noErr || err == badMDBErr || err == noMacDskErr) )
		goto ExitThisRoutine;

	// restore the primary block from the alternate
	err = GetVolumeObjectAlternateBlock( &myAlternate );
	
	// invalidate if we have not marked the alternate as OK
	if ( VolumeObjectIsHFS( ) ) {
		if ( (myVOPtr->flags & kVO_AltMDBOK) == 0 )
			err = badMDBErr;
	}
	else if ( (myVOPtr->flags & kVO_AltVHBOK) == 0 ) {
		err = badMDBErr;
	}
	
	if ( err == noErr ) {
		CopyMemory( myAlternate.buffer, myPrimary.buffer, Blk_Size );
		(void) ReleaseVolumeBlock( myVOPtr->vcbPtr, &myPrimary, kForceWriteBlock );		
		myPrimary.buffer = NULL;
		if ( myVOPtr->volumeType == kHFSVolumeType )
			myVOPtr->flags |= kVO_PriMDBOK;
		else
			myVOPtr->flags |= kVO_PriVHBOK;
	}

ExitThisRoutine:
	if ( myPrimary.buffer != NULL )
		(void) ReleaseVolumeBlock( myVOPtr->vcbPtr, &myPrimary, kReleaseBlock );
	if ( myAlternate.buffer != NULL )
		(void) ReleaseVolumeBlock( myVOPtr->vcbPtr, &myAlternate, kReleaseBlock );

	return( err );
	
} /* VolumeObjectFixPrimaryBlock */


//******************************************************************************
//	Routine:	GetVolumeObjectVHBorMDB
//
//	Function:	Get the Volume Header block or Master Directory block (depending
//				on type of volume).  This will normally return the alternate, but
//				it may return the primary when the alternate is damaged or cannot
//				be found.
//
// 	Result:		returns 0 when all is well.
//******************************************************************************
OSErr GetVolumeObjectVHBorMDB( BlockDescriptor * theBlockDescPtr )
{
	UInt64				myBlockNum;
	VolumeObjectPtr		myVOPtr;
	OSErr  				err;

	myVOPtr = GetVolumeObjectPtr( );
	GetVolumeObjectBlockNum( &myBlockNum );

	err = GetVolumeObjectBlock( myVOPtr, myBlockNum, theBlockDescPtr );
	if ( err == noErr ) 
	{
		if ( myVOPtr->volumeType == kEmbededHFSPlusVolumeType || 
			 myVOPtr->volumeType == kPureHFSPlusVolumeType )
		{
			err = ValidVolumeHeader( (HFSPlusVolumeHeader*) theBlockDescPtr->buffer );
		}
		else if ( myVOPtr->volumeType == kHFSVolumeType ) 
		{
			HFSMasterDirectoryBlock *	myMDBPtr;
			myMDBPtr = (HFSMasterDirectoryBlock	*) theBlockDescPtr->buffer;
			if ( myMDBPtr->drSigWord != kHFSSigWord )
				err = noMacDskErr;
		}
		else
			err = noMacDskErr;
	}

	return( err );

} /* GetVolumeObjectVHBorMDB */


//******************************************************************************
//	Routine:	GetVolumeObjectAlternateBlock
//
//	Function:	Get the alternate Volume Header block or Master Directory block
//				(depending on type of volume).
// 	Result:		returns 0 when all is well.
//******************************************************************************
OSErr GetVolumeObjectAlternateBlock( BlockDescriptor * theBlockDescPtr )
{
	UInt64				myBlockNum;
	VolumeObjectPtr		myVOPtr;
	OSErr  				err;

	myVOPtr = GetVolumeObjectPtr( );
	GetVolumeObjectAlternateBlockNum( &myBlockNum );

	err = GetVolumeObjectBlock( myVOPtr, myBlockNum, theBlockDescPtr );
	if ( err == noErr ) 
	{
		if ( myVOPtr->volumeType == kEmbededHFSPlusVolumeType || 
			 myVOPtr->volumeType == kPureHFSPlusVolumeType )
		{
			err = ValidVolumeHeader( (HFSPlusVolumeHeader*) theBlockDescPtr->buffer );
		}
		else if ( myVOPtr->volumeType == kHFSVolumeType ) 
		{
			HFSMasterDirectoryBlock *	myMDBPtr;
			myMDBPtr = (HFSMasterDirectoryBlock	*) theBlockDescPtr->buffer;
			if ( myMDBPtr->drSigWord != kHFSSigWord )
				err = noMacDskErr;
		}
		else
			err = noMacDskErr;
	}

	return( err );

} /* GetVolumeObjectAlternateBlock */


//******************************************************************************
//	Routine:	GetVolumeObjectPrimaryBlock
//
//	Function:	Get the primary Volume Header block or Master Directory block
//				(depending on type of volume).
// 	Result:		returns 0 when all is well.
//******************************************************************************
OSErr GetVolumeObjectPrimaryBlock( BlockDescriptor * theBlockDescPtr )
{
	UInt64				myBlockNum;
	VolumeObjectPtr		myVOPtr;
	OSErr  				err;

	myVOPtr = GetVolumeObjectPtr( );
	GetVolumeObjectPrimaryBlockNum( &myBlockNum );

	err = GetVolumeObjectBlock( myVOPtr, myBlockNum, theBlockDescPtr );
	if ( err == noErr ) 
	{
		if ( myVOPtr->volumeType == kEmbededHFSPlusVolumeType || 
			 myVOPtr->volumeType == kPureHFSPlusVolumeType )
		{
			err = ValidVolumeHeader( (HFSPlusVolumeHeader*) theBlockDescPtr->buffer );
		}
		else if ( myVOPtr->volumeType == kHFSVolumeType ) 
		{
			HFSMasterDirectoryBlock *	myMDBPtr;
			myMDBPtr = (HFSMasterDirectoryBlock	*) theBlockDescPtr->buffer;
			if ( myMDBPtr->drSigWord != kHFSSigWord )
				err = noMacDskErr;
		}
		else
			err = noMacDskErr;
	}

	return( err );

} /* GetVolumeObjectPrimaryBlock */


//******************************************************************************
//	Routine:	GetVolumeObjectVHB
//
//	Function:	Get the Volume Header block using either the primary or alternate
//				block number as set up by InitializeVolumeObject.  This will normally
//				return the alternate, but it may return the primary when the
//				alternate is damaged or cannot be found.
//
// 	Result:		returns 0 when all is well or passes results of GetVolumeBlock or
//				ValidVolumeHeader.
//******************************************************************************
OSErr GetVolumeObjectVHB( BlockDescriptor * theBlockDescPtr )
{
	UInt64				myBlockNum;
	VolumeObjectPtr		myVOPtr;
	OSErr  				err;

	myVOPtr = GetVolumeObjectPtr( );
	myBlockNum = ((myVOPtr->flags & kVO_AltVHBOK) != 0) ? myVOPtr->alternateVHB : myVOPtr->primaryVHB;
	err = GetVolumeObjectBlock( myVOPtr, myBlockNum, theBlockDescPtr );
	if ( err == noErr )
		err = ValidVolumeHeader( (HFSPlusVolumeHeader*) theBlockDescPtr->buffer );

	return( err );

} /* GetVolumeObjectVHB */


//******************************************************************************
//	Routine:	GetVolumeObjectAlternateMDB
//
//	Function:	Get the Master Directory Block using the alternate master directory
//				block number as set up by InitializeVolumeObject.
//
// 	Result:		returns 0 when all is well.
//******************************************************************************
OSErr GetVolumeObjectAlternateMDB( BlockDescriptor * theBlockDescPtr )
{
	VolumeObjectPtr		myVOPtr;
	OSErr  				err;

	myVOPtr = GetVolumeObjectPtr( );
	err = GetVolumeObjectBlock( NULL, myVOPtr->alternateMDB, theBlockDescPtr );
	if ( err == noErr ) 
	{
		HFSMasterDirectoryBlock *	myMDBPtr;
		myMDBPtr = (HFSMasterDirectoryBlock	*) theBlockDescPtr->buffer;
		if ( myMDBPtr->drSigWord != kHFSSigWord )
			err = noMacDskErr;
	}

	return( err );

} /* GetVolumeObjectAlternateMDB */


//******************************************************************************
//	Routine:	GetVolumeObjectPrimaryMDB
//
//	Function:	Get the Master Directory Block using the primary master directory
//				block number as set up by InitializeVolumeObject.
//
// 	Result:		returns 0 when all is well.
//******************************************************************************
OSErr GetVolumeObjectPrimaryMDB( BlockDescriptor * theBlockDescPtr )
{
	VolumeObjectPtr		myVOPtr;
	OSErr  				err;

	myVOPtr = GetVolumeObjectPtr( );
	err = GetVolumeObjectBlock( NULL, myVOPtr->primaryMDB, theBlockDescPtr );
	if ( err == noErr ) 
	{
		HFSMasterDirectoryBlock *	myMDBPtr;
		myMDBPtr = (HFSMasterDirectoryBlock	*) theBlockDescPtr->buffer;
		if ( myMDBPtr->drSigWord != kHFSSigWord )
			err = noMacDskErr;
	}

	return( err );

} /* GetVolumeObjectPrimaryMDB */


//******************************************************************************
//	Routine:	GetVolumeObjectBlock
//
//	Function:	Get the Volume Header block or Master Directory block using the
//				given block number.
// 	Result:		returns 0 when all is well or passes results of GetVolumeBlock or
//				ValidVolumeHeader.
//******************************************************************************
static OSErr GetVolumeObjectBlock( VolumeObjectPtr theVOPtr,
								   UInt64 theBlockNum,
								   BlockDescriptor * theBlockDescPtr )
{
	OSErr  			err;

	if ( theVOPtr == NULL )
		theVOPtr = GetVolumeObjectPtr( );
		
	err = GetVolumeBlock( theVOPtr->vcbPtr, theBlockNum, kGetBlock, theBlockDescPtr );

	return( err );

} /* GetVolumeObjectBlock */


//******************************************************************************
//	Routine:	GetVolumeObjectBlockNum
//
//	Function:	Extract the appropriate block number for the volume header or
//				master directory (depanding on volume type) from the VolumeObject.
//				NOTE - this routine may return the primary or alternate block
//				depending on which one is valid.  Preference is always given to
//				the alternate.
//
// 	Result:		returns block number of MDB or VHB or 0 if none are valid or
//				if volume type is unknown.
//******************************************************************************
void GetVolumeObjectBlockNum( UInt64 * theBlockNumPtr )
{
	VolumeObjectPtr				myVOPtr;

	myVOPtr = GetVolumeObjectPtr( );
	*theBlockNumPtr = 0;	// default to none

	// NOTE - we use alternate volume header or master directory
	// block before the primary because it is less likely to be damaged.
	if ( myVOPtr->volumeType == kEmbededHFSPlusVolumeType ||
	     myVOPtr->volumeType == kPureHFSPlusVolumeType ) {
		if ( (myVOPtr->flags & kVO_AltVHBOK) != 0 )
			*theBlockNumPtr = myVOPtr->alternateVHB;
		else
			*theBlockNumPtr = myVOPtr->primaryVHB;
	}
	else if ( myVOPtr->volumeType == kHFSVolumeType ) {
		if ( (myVOPtr->flags & kVO_AltMDBOK) != 0 )
			*theBlockNumPtr = myVOPtr->alternateMDB;
		else
			*theBlockNumPtr = myVOPtr->primaryMDB;
	}

	return;

} /* GetVolumeObjectBlockNum */


//******************************************************************************
//	Routine:	GetVolumeObjectAlternateBlockNum
//
//	Function:	Extract the alternate block number for the volume header or
//				master directory (depanding on volume type) from the VolumeObject.
//
// 	Result:		returns block number of alternate MDB or VHB or 0 if none are 
//				valid or if volume type is unknown.
//******************************************************************************
void GetVolumeObjectAlternateBlockNum( UInt64 * theBlockNumPtr )
{
	VolumeObjectPtr				myVOPtr;

	myVOPtr = GetVolumeObjectPtr( );
	*theBlockNumPtr = 0;	// default to none
	
	if ( myVOPtr->volumeType == kEmbededHFSPlusVolumeType ||
	     myVOPtr->volumeType == kPureHFSPlusVolumeType ) {
		*theBlockNumPtr = myVOPtr->alternateVHB;
	}
	else if ( myVOPtr->volumeType == kHFSVolumeType ) {
		*theBlockNumPtr = myVOPtr->alternateMDB;
	}

	return;

} /* GetVolumeObjectAlternateBlockNum */


//******************************************************************************
//	Routine:	GetVolumeObjectPrimaryBlockNum
//
//	Function:	Extract the primary block number for the volume header or
//				master directory (depanding on volume type) from the VolumeObject.
//
// 	Result:		returns block number of primary MDB or VHB or 0 if none are valid
//				or if volume type is unknown.
//******************************************************************************
void GetVolumeObjectPrimaryBlockNum( UInt64 * theBlockNumPtr )
{
	VolumeObjectPtr				myVOPtr;

	myVOPtr = GetVolumeObjectPtr( );
	*theBlockNumPtr = 0;	// default to none
	
	if ( myVOPtr->volumeType == kEmbededHFSPlusVolumeType ||
	     myVOPtr->volumeType == kPureHFSPlusVolumeType ) {
		*theBlockNumPtr = myVOPtr->primaryVHB;
	}
	else if ( myVOPtr->volumeType == kHFSVolumeType ) {
		*theBlockNumPtr = myVOPtr->primaryMDB;
	}

	return;

} /* GetVolumeObjectPrimaryBlockNum */


//******************************************************************************
//	Routine:	InitializeVolumeObject
//
//	Function:	Locate volume headers and / or master directory blocks for this
//				volume and fill where they are located on the volume and the type
//				of volume we are dealing with.  We have three types of HFS volumes:
//				¥ HFS - standard (old format) where primary MDB is 2nd block into
//					the volume and alternate MDB is 2nd to last block on the volume.
//				¥ pure HFS+ - where primary volume header is 2nd block into
//					the volume and alternate volume header is 2nd to last block on
//					the volume.
//				¥ wrapped HFS+ - where primary MDB is 2nd block into the volume and 
//					alternate MDB is 2nd to last block on the volume.   The embedded 
//					HFS+ volume header locations are calculated from drEmbedExtent 
//					(in the MDB).
//
// 	Result:		returns nothing.  Will fill in SGlob.VolumeObject data
//******************************************************************************
void	InitializeVolumeObject( SGlobPtr GPtr )
{
	OSErr						err;
	HFSMasterDirectoryBlock *	myMDBPtr;
	HFSPlusVolumeHeader *		myVHPtr;
	VolumeObjectPtr				myVOPtr;
	HFSPlusVolumeHeader			myPriVolHeader;
	BlockDescriptor				myBlockDescriptor;

	myBlockDescriptor.buffer = NULL;
	myVOPtr = GetVolumeObjectPtr( );
	myVOPtr->flags |= kVO_Inited;
	myVOPtr->vcbPtr = GPtr->calculatedVCB;

	// Determine volume size in sectors
	err = GetDeviceSize( 	GPtr->calculatedVCB->vcbDriveNumber, 
							&myVOPtr->totalDeviceSectors, 
							&myVOPtr->sectorSize );
	if ( (myVOPtr->totalDeviceSectors < 3) || (err != noErr) ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
			plog("\tinvalid device information for volume - total sectors = %qd sector size = %d \n",
				myVOPtr->totalDeviceSectors, myVOPtr->sectorSize);
		}
		goto ExitRoutine;
	}
	
	// get the primary volume header or master directory block (depending on volume type)
	// should always be block 2 (relative to 0) into the volume.
	err = GetVolumeObjectBlock( myVOPtr, MDB_BlkN, &myBlockDescriptor );
	if ( err == noErr ) {
		myMDBPtr = (HFSMasterDirectoryBlock	*) myBlockDescriptor.buffer;
		if ( myMDBPtr->drSigWord == kHFSPlusSigWord || myMDBPtr->drSigWord == kHFSXSigWord) {
			myVHPtr	= (HFSPlusVolumeHeader *) myMDBPtr;
			
			myVOPtr->primaryVHB = MDB_BlkN;								// save location
			myVOPtr->alternateVHB = myVOPtr->totalDeviceSectors - 2;	// save location
			err = ValidVolumeHeader( myVHPtr );
			if ( err == noErr ) {
				myVOPtr->flags |= kVO_PriVHBOK;
				bcopy( myVHPtr, &myPriVolHeader, sizeof( *myVHPtr ) );
			}
			else {
				if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
					plog( "\tInvalid primary volume header - error %d \n", err );
				}
			}
		}
		else if ( myMDBPtr->drSigWord == kHFSSigWord ) {
			// we could have an HFS or wrapped HFS+ volume
			myVOPtr->primaryMDB = MDB_BlkN;								// save location
			myVOPtr->alternateMDB = myVOPtr->totalDeviceSectors - 2;	// save location
			myVOPtr->flags |= kVO_PriMDBOK;
		}
		else {
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
				plog( "\tBlock %d is not an MDB or Volume Header \n", MDB_BlkN );
			}
		}
		(void) ReleaseVolumeBlock( GPtr->calculatedVCB, &myBlockDescriptor, kReleaseBlock );
	} 
	else {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
			plog( "\tcould not get volume block %d, err %d \n", MDB_BlkN, err );
		}
	}
	
	// get the alternate volume header or master directory block (depending on volume type)
	// should always be 2nd to last sector.
	err = GetVolumeObjectBlock( myVOPtr, myVOPtr->totalDeviceSectors - 2, &myBlockDescriptor );
	if ( err == noErr ) {
		myMDBPtr = (HFSMasterDirectoryBlock	*) myBlockDescriptor.buffer;
		if ( myMDBPtr->drSigWord == kHFSPlusSigWord || myMDBPtr->drSigWord == kHFSXSigWord ) {
			myVHPtr	= (HFSPlusVolumeHeader *) myMDBPtr;
			
			myVOPtr->primaryVHB = MDB_BlkN;								// save location
			myVOPtr->alternateVHB = myVOPtr->totalDeviceSectors - 2;	// save location
			err = ValidVolumeHeader( myVHPtr );
			if ( err == noErr ) {
				// check to see if the primary and alternates are in sync.  3137809
				myVOPtr->flags |= kVO_AltVHBOK;
				CompareVolHeaderBTreeSizes( GPtr, myVOPtr, &myPriVolHeader, myVHPtr );
			}
			else {
				if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
					plog( "\tInvalid alternate volume header - error %d \n", err );
				}
			}
		}
		else if ( myMDBPtr->drSigWord == kHFSSigWord ) {
			myVOPtr->primaryMDB = MDB_BlkN;								// save location
			myVOPtr->alternateMDB = myVOPtr->totalDeviceSectors - 2;	// save location
			myVOPtr->flags |= kVO_AltMDBOK;
		}
		else {
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
				plog( "\tBlock %qd is not an MDB or Volume Header \n", myVOPtr->totalDeviceSectors - 2 );
			}
		}

		(void) ReleaseVolumeBlock( GPtr->calculatedVCB, &myBlockDescriptor, kReleaseBlock );
	}
	else {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
			plog( "\tcould not get alternate volume header at %qd, err %d \n", 
					myVOPtr->totalDeviceSectors - 2, err );
		}
	}

	// get the embedded volume header (if applicable).
	if ( (myVOPtr->flags & kVO_AltMDBOK) != 0 ) {
		err = GetVolumeObjectBlock( myVOPtr, myVOPtr->alternateMDB, &myBlockDescriptor );
		if ( err == noErr ) {
			myMDBPtr = (HFSMasterDirectoryBlock	*) myBlockDescriptor.buffer;
			GetEmbeddedVolumeHeaders( GPtr, myMDBPtr, false );
			(void) ReleaseVolumeBlock( GPtr->calculatedVCB, &myBlockDescriptor, kReleaseBlock );
		}
	}
	
	// Now we will look for embedded HFS+ volume headers using the primary MDB if 
	// we haven't already located them.
	if ( (myVOPtr->flags & kVO_PriMDBOK) != 0 && 
		 ((myVOPtr->flags & kVO_PriVHBOK) == 0 || (myVOPtr->flags & kVO_AltVHBOK) == 0) ) {
		err = GetVolumeObjectBlock( myVOPtr, myVOPtr->primaryMDB, &myBlockDescriptor );
		if ( err == noErr ) {
			myMDBPtr = (HFSMasterDirectoryBlock	*) myBlockDescriptor.buffer;
			GetEmbeddedVolumeHeaders( GPtr, myMDBPtr, true );
			(void) ReleaseVolumeBlock( GPtr->calculatedVCB, &myBlockDescriptor, kReleaseBlock );
		}
		else {
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
				plog( "\tcould not get primary MDB at block %qd, err %d \n", myVOPtr->primaryMDB, err );
			}
		}
	}

ExitRoutine:
	// set the type of volume using the flags we set as we located the various header / master
	// blocks.  
	if ( ((myVOPtr->flags & kVO_PriVHBOK) != 0 || (myVOPtr->flags & kVO_AltVHBOK) != 0)  &&
		 ((myVOPtr->flags & kVO_PriMDBOK) != 0 || (myVOPtr->flags & kVO_AltMDBOK) != 0) ) {
		myVOPtr->volumeType = kEmbededHFSPlusVolumeType;
	}
	else if ( ((myVOPtr->flags & kVO_PriVHBOK) != 0 || (myVOPtr->flags & kVO_AltVHBOK) != 0) &&
			  (myVOPtr->flags & kVO_PriMDBOK) == 0 && (myVOPtr->flags & kVO_AltMDBOK) == 0 ) {
		myVOPtr->volumeType = kPureHFSPlusVolumeType;
	}
	else if ( (myVOPtr->flags & kVO_PriVHBOK) == 0 && (myVOPtr->flags & kVO_AltVHBOK) == 0 &&
			  ((myVOPtr->flags & kVO_PriMDBOK) != 0 || (myVOPtr->flags & kVO_AltMDBOK) != 0) ) {
		myVOPtr->volumeType = kHFSVolumeType;
	}
	else
		myVOPtr->volumeType = kUnknownVolumeType;

	return;
	
} /* InitializeVolumeObject */


//******************************************************************************
//	Routine:	PrintVolumeObject
//
//	Function:	Print out some helpful info about the state of our VolumeObject.
//
// 	Result:		returns nothing. 
//******************************************************************************
void PrintVolumeObject( void )
{
	VolumeObjectPtr				myVOPtr;

	myVOPtr = GetVolumeObjectPtr( );

	if ( myVOPtr->volumeType == kHFSVolumeType )
		plog( "\tvolume type is HFS \n" );
	else if ( myVOPtr->volumeType == kEmbededHFSPlusVolumeType )
		plog( "\tvolume type is embedded HFS+ \n" );
	else if ( myVOPtr->volumeType == kPureHFSPlusVolumeType )
		plog( "\tvolume type is pure HFS+ \n" );
	else
		plog( "\tunknown volume type \n" );
	
	plog( "\tprimary MDB is at block %qd 0x%02qx \n", myVOPtr->primaryMDB, myVOPtr->primaryMDB );
	plog( "\talternate MDB is at block %qd 0x%02qx \n", myVOPtr->alternateMDB, myVOPtr->alternateMDB );
	plog( "\tprimary VHB is at block %qd 0x%02qx \n", myVOPtr->primaryVHB, myVOPtr->primaryVHB );
	plog( "\talternate VHB is at block %qd 0x%02qx \n", myVOPtr->alternateVHB, myVOPtr->alternateVHB );
	plog( "\tsector size = %d 0x%02x \n", myVOPtr->sectorSize, myVOPtr->sectorSize );
	plog( "\tVolumeObject flags = 0x%02X \n", myVOPtr->flags );
	plog( "\ttotal sectors for volume = %qd 0x%02qx \n", 
			myVOPtr->totalDeviceSectors, myVOPtr->totalDeviceSectors );
	plog( "\ttotal sectors for embedded volume = %qd 0x%02qx \n", 
			myVOPtr->totalEmbeddedSectors, myVOPtr->totalEmbeddedSectors );
	
	return;
	
} /* PrintVolumeObject */


//******************************************************************************
//	Routine:	GetEmbeddedVolumeHeaders
//
//	Function:	Given a MDB (Master Directory Block) from an HFS volume, check
//				to see if there is an embedded HFS+ volume.  If we find an 
//				embedded HFS+ volume fill in relevant SGlob.VolumeObject data.
//
// 	Result:		returns nothing.  Will fill in VolumeObject data
//******************************************************************************

static void GetEmbeddedVolumeHeaders( 	SGlobPtr GPtr, 
										HFSMasterDirectoryBlock * theMDBPtr,
										Boolean isPrimaryMDB )
{
	OSErr						err;
	HFSPlusVolumeHeader *		myVHPtr;
	VolumeObjectPtr				myVOPtr;
	UInt64  					myHFSPlusSectors;
	UInt64  					myPrimaryBlockNum;
	UInt64  					myAlternateBlockNum;
	HFSPlusVolumeHeader			myAltVolHeader;
	BlockDescriptor				myBlockDescriptor;

	myBlockDescriptor.buffer = NULL;
	myVOPtr = GetVolumeObjectPtr( );

	// NOTE - If all of the embedded volume information is zero, then assume
	// this really is a plain HFS disk like it says.  There could be ghost
	// volume headers left over when someone reinitializes a large HFS Plus 
	// volume as HFS.  The original embedded primary volume header and 
	// alternate volume header are not zeroed out.
	if ( theMDBPtr->drEmbedSigWord == 0 && 
		 theMDBPtr->drEmbedExtent.blockCount == 0 && 
		 theMDBPtr->drEmbedExtent.startBlock == 0 ) {
		 goto ExitRoutine;
	}
	
	// number of sectors in our embedded HFS+ volume
	myHFSPlusSectors = (theMDBPtr->drAlBlkSiz / Blk_Size) * theMDBPtr->drEmbedExtent.blockCount;
		
	// offset of embedded HFS+ volume (in bytes) into HFS wrapper volume
	// NOTE - UInt32 is OK since we don't support HFS Wrappers on TB volumes
	myVOPtr->embeddedOffset = 
		(theMDBPtr->drEmbedExtent.startBlock * theMDBPtr->drAlBlkSiz) + 
		(theMDBPtr->drAlBlSt * Blk_Size);

	// Embedded alternate volume header is always 2nd to last sector
	myAlternateBlockNum =
		theMDBPtr->drAlBlSt + 
		((theMDBPtr->drAlBlkSiz / Blk_Size) * theMDBPtr->drEmbedExtent.startBlock) + 
		myHFSPlusSectors - 2;

	// Embedded primary volume header should always be block 2 (relative to 0) 
	// into the embedded volume
	myPrimaryBlockNum = (theMDBPtr->drEmbedExtent.startBlock * theMDBPtr->drAlBlkSiz / Blk_Size) + 
						theMDBPtr->drAlBlSt + 2;

	// get the embedded alternate volume header
	err = GetVolumeObjectBlock( myVOPtr, myAlternateBlockNum, &myBlockDescriptor );
	if ( err == noErr ) {
		myVHPtr = (HFSPlusVolumeHeader	*) myBlockDescriptor.buffer;
		if ( myVHPtr->signature == kHFSPlusSigWord ) {

			myVOPtr->alternateVHB = myAlternateBlockNum;	// save location
			myVOPtr->primaryVHB = myPrimaryBlockNum;		// save location
			err = ValidVolumeHeader( myVHPtr );
			if ( err == noErr ) {
				myVOPtr->flags |= kVO_AltVHBOK;
				myVOPtr->totalEmbeddedSectors = myHFSPlusSectors;
				bcopy( myVHPtr, &myAltVolHeader, sizeof( *myVHPtr ) );
			}
			else {
				if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
					plog( "\tInvalid embedded alternate volume header at block %qd - error %d \n", myAlternateBlockNum, err );
				}
			}
		}
		else {
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
				plog( "\tBlock number %qd is not embedded alternate volume header \n", myAlternateBlockNum );
			}
		}
		(void) ReleaseVolumeBlock( GPtr->calculatedVCB, &myBlockDescriptor, kReleaseBlock );
	}
	else {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
			plog( "\tcould not get embedded alternate volume header at %qd, err %d \n", 
					myAlternateBlockNum, err );
		}
	}
	
	// get the embedded primary volume header
	err = GetVolumeObjectBlock( myVOPtr, myPrimaryBlockNum, &myBlockDescriptor );
	if ( err == noErr ) {
		myVHPtr = (HFSPlusVolumeHeader	*) myBlockDescriptor.buffer;
		if ( myVHPtr->signature == kHFSPlusSigWord ) {

			myVOPtr->primaryVHB = myPrimaryBlockNum;  		// save location
			myVOPtr->alternateVHB = myAlternateBlockNum;	// save location
			err = ValidVolumeHeader( myVHPtr );
			if ( err == noErr ) {
				myVOPtr->flags |= kVO_PriVHBOK;
				myVOPtr->totalEmbeddedSectors = myHFSPlusSectors;

				// check to see if the primary and alternates are in sync.  3137809
				CompareVolHeaderBTreeSizes( GPtr, myVOPtr, myVHPtr, &myAltVolHeader );
			}
			else {
				if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
					plog( "\tInvalid embedded primary volume header at block %qd - error %d \n", myPrimaryBlockNum, err );
				}
			}
		}
		else {
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
				plog( "\tBlock number %qd is not embedded primary volume header \n", myPrimaryBlockNum );
			}
		}
		(void) ReleaseVolumeBlock( GPtr->calculatedVCB, &myBlockDescriptor, kReleaseBlock );
	}
	else {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
			plog( "\tcould not get embedded primary volume header at %qd, err %d \n", 
					myPrimaryBlockNum, err );
		}
	}

ExitRoutine:
	return;
	
} /* GetEmbeddedVolumeHeaders */


//******************************************************************************
//	Routine:	CompareVolHeaderBTreeSizes
//
//	Function:	checks to see if the primary and alternate volume headers are in
//				sync with regards to the catalog and extents btree file size.  If
//				we find an anomaly we will give preference to the volume header 
//				with the larger of the btree files since these files never shrink.
//				Added for radar #3137809.
//
// 	Result:		returns nothing. 
//******************************************************************************
static void CompareVolHeaderBTreeSizes(	SGlobPtr GPtr,
										VolumeObjectPtr theVOPtr, 
										HFSPlusVolumeHeader * thePriVHPtr, 
										HFSPlusVolumeHeader * theAltVHPtr )
{
	int			weDisagree;
	int			usePrimary;
	int			useAlternate;
	
	weDisagree = usePrimary = useAlternate = 0;
	
	// we only check if both volume headers appear to be OK
	if ( (theVOPtr->flags & kVO_PriVHBOK) == 0 || (theVOPtr->flags & kVO_AltVHBOK) == 0  )
		return;

	if ( thePriVHPtr->catalogFile.totalBlocks != theAltVHPtr->catalogFile.totalBlocks ) {
		// only continue if the B*Tree files both start at the same block number
		if ( thePriVHPtr->catalogFile.extents[0].startBlock == theAltVHPtr->catalogFile.extents[0].startBlock ) {
			weDisagree = 1;
			if ( thePriVHPtr->catalogFile.totalBlocks > theAltVHPtr->catalogFile.totalBlocks )
				usePrimary = 1;
			else
				useAlternate = 1;
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
				plog( "\tvolume headers disagree on catalog file total blocks - primary %d alternate %d \n", 
						thePriVHPtr->catalogFile.totalBlocks, theAltVHPtr->catalogFile.totalBlocks );
			}
		}
	}

	if ( thePriVHPtr->extentsFile.totalBlocks != theAltVHPtr->extentsFile.totalBlocks ) {
		// only continue if the B*Tree files both start at the same block number
		if ( thePriVHPtr->extentsFile.extents[0].startBlock == theAltVHPtr->extentsFile.extents[0].startBlock ) {
			weDisagree = 1;
			if ( thePriVHPtr->extentsFile.totalBlocks > theAltVHPtr->extentsFile.totalBlocks )
				usePrimary = 1;
			else
				useAlternate = 1;
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
				plog( "\tvolume headers disagree on extents file total blocks - primary %d alternate %d \n", 
						thePriVHPtr->extentsFile.totalBlocks, theAltVHPtr->extentsFile.totalBlocks );
			}
		}
	}
	
	if ( weDisagree == 0 )
		return;
		
	// we have a disagreement.  we resolve the issue by using the larger of the two.
	if ( usePrimary == 1 && useAlternate == 1 ) {
		// this should never happen, but if it does, bail without choosing a preference
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
			plog( "\tvolume headers disagree but there is confusion on which to use \n" );
		}
		return; 
	}
	
	if ( usePrimary == 1 ) {
		// mark alternate as bogus
		theVOPtr->flags &= ~kVO_AltVHBOK;
	}
	else if ( useAlternate == 1 ) {
		// mark primary as bogus
		theVOPtr->flags &= ~kVO_PriVHBOK;
	}

	return;

} /* CompareVolHeaderBTreeSizes */


/*
 * This code should be removed after debugging is completed.
 */
#include <ctype.h>

#ifndef MIN
#define MIN(a, b) \
	({ __typeof(a) _a = (a); __typeof(b) _b = (b);	\
		(_a < _b) ? _a : _b; })
#endif


enum { WIDTH = 16, };

void
DumpData(const void *data, size_t len, char *label)
{
	unsigned char *base = (unsigned char*)data;
	unsigned char *end = base + len;
	unsigned char *cp = base;
	int allzeroes = 0;

	while (cp < end) {
		unsigned char *tend = MIN(end, cp + WIDTH);
		unsigned char *tmp;
		int i;
		size_t gap = (cp + WIDTH) - tend;

		if (gap != 0 || tend == end)
			allzeroes = 0;
		if (allzeroes) {
			for (tmp = cp; tmp < tend; tmp++) {
				if (*tmp) {
					allzeroes = 0;
					break;
				}
			}
			if (allzeroes == 1) {
				fprintf(stderr, ". . .\n");
				allzeroes = 2;
			}
			if (allzeroes) {
				cp += WIDTH;
				continue;
			}
		}
		allzeroes = 1;
		if (label == NULL) {
			fprintf(stderr, "%04x:  ", (int)(cp - base));
		}
		else {
			fprintf(stderr, "%s %04x:  ", label, (int)(cp - base));
		}
		for (i = 0, tmp = cp; tmp < tend; tmp++) {
			fprintf(stderr, "%02x", *tmp);
			if (++i % 2 == 0)
				fprintf(stderr, " ");
			if (*tmp)
				allzeroes = 0;
		}
		for (i = (int)gap; i >= 0; i--) {
			fprintf(stderr, "  ");
			if (i % 2 == 1)
				fprintf(stderr, " ");
		}
		fprintf(stderr, "    |");
		for (tmp = cp; tmp < tend; tmp++) {
			fprintf(stderr, "%c", isalnum(*tmp) ? *tmp : '.');
		}
		for (i = 0; i < gap; i++) {
			fprintf(stderr, " ");
		}
		fprintf(stderr, "|\n");
		cp += WIDTH;
	}

	return;

}
//******************************************************************************
//	Routine:	VolumeObjectIsValid
//
//	Function:	determine if the volume represented by our VolumeObject is a 
//				valid volume type (i.e. not unknown type)
//
// 	Result:		returns true if volume is known volume type (i.e. HFS, HFS+)
//				false otherwise.
//******************************************************************************
Boolean VolumeObjectIsValid(SGlobPtr gptr)
{
	VolumeObjectPtr	myVOPtr = GetVolumeObjectPtr();
	Boolean retval = false;
	
	/* Check if the type is unknown type */
	if (myVOPtr->volumeType == kUnknownVolumeType) {
		pwarn("volumeType is %d\n", kUnknownVolumeType);
		goto done;
	} 

	/* Check if it is HFS+ volume */
	if (VolumeObjectIsHFSPlus() == true) {
		retval = true;
		goto done;
	}
		
	/* Check if it is HFS volume */
	if (VolumeObjectIsHFS() == true) {
		retval = true;
		goto done;
	}

done:
	/*
	 * This code should be removed after debugging is done.
	 */
	if (retval == false) {
		UInt64 myBlockNum;
		VolumeObjectPtr myVOPtr;
		BlockDescriptor theBlockDesc;
		OSErr err;

		myVOPtr = GetVolumeObjectPtr();
		GetVolumeObjectBlockNum(&myBlockNum);
		err = GetVolumeBlock(myVOPtr->vcbPtr, myBlockNum, kGetBlock, &theBlockDesc);
		if (err != noErr) {
			fprintf(stderr, "%s: Cannot GetVolumeBlock: %d\n", __FUNCTION__, err);
		} else {
			uint8_t *ptr = (uint8_t*)theBlockDesc.buffer;
			DumpData(ptr, theBlockDesc.blockSize, gptr->deviceNode);
			ReleaseVolumeBlock(myVOPtr->vcbPtr, &theBlockDesc, kReleaseBlock);
		}
	}
	return retval;
} /* VolumeObjectIsValid */

//******************************************************************************
//	Routine:	VolumeObjectIsHFSPlus
//
//	Function:	determine if the volume represented by our VolumeObject is an
//				HFS+ volume (pure or embedded).
//
// 	Result:		returns true if volume is pure HFS+ or embedded HFS+ else false.
//******************************************************************************
Boolean VolumeObjectIsHFSPlus( void )
{
	VolumeObjectPtr				myVOPtr;

	myVOPtr = GetVolumeObjectPtr( );
	
	if ( myVOPtr->volumeType == kEmbededHFSPlusVolumeType ||
	     myVOPtr->volumeType == kPureHFSPlusVolumeType ) {
		return( true );
	}

	return( false );

} /* VolumeObjectIsHFSPlus */


//******************************************************************************
//	Routine:	VolumeObjectIsHFSX
//
//	Function:	determine if the volume represented by our VolumeObject is an
//				HFSX volume (pure or embedded)
//
// 	Result:		returns true if volume is pure HFSX or embedded HFSX else false.
//******************************************************************************

Boolean VolumeObjectIsHFSX(SGlobPtr GPtr)
{
	OSErr err;
	int result = false;
	HFSMasterDirectoryBlock *mdbp;
	SVCB *vcb = GPtr->calculatedVCB;
	BlockDescriptor block;

#define kIDSector 2
	err = GetVolumeBlock(vcb, kIDSector, kGetBlock, &block);
	if (err) return (false);

	mdbp = (HFSMasterDirectoryBlock *)block.buffer;
	if (mdbp->drSigWord == kHFSXSigWord) {
		result = true;
	} else if (mdbp->drSigWord == kHFSSigWord) {
		if (mdbp->drEmbedSigWord == kHFSXSigWord) {
			result = true;
		}
	}

	(void) ReleaseVolumeBlock(vcb, &block, kReleaseBlock);

	return( result );
} /* VolumeObjectIsHFSX */

//******************************************************************************
//	Routine:	VolumeObjectIsHFS
//
//	Function:	determine if the volume represented by our VolumeObject is an
//				HFS (standard) volume.
//
// 	Result:		returns true if HFS (standard) volume.
//******************************************************************************
Boolean VolumeObjectIsHFS( void )
{
	VolumeObjectPtr				myVOPtr;

	myVOPtr = GetVolumeObjectPtr( );
	
	if ( myVOPtr->volumeType == kHFSVolumeType )
		return( true );

	return( false );

} /* VolumeObjectIsHFS */


//******************************************************************************
//	Routine:	VolumeObjectIsEmbeddedHFSPlus
//
//	Function:	determine if the volume represented by our VolumeObject is an
//				embedded HFS plus volume.
//
// 	Result:		returns true if embedded HFS plus volume.
//******************************************************************************
Boolean VolumeObjectIsEmbeddedHFSPlus( void )
{
	VolumeObjectPtr				myVOPtr;

	myVOPtr = GetVolumeObjectPtr( );
	
	if ( myVOPtr->volumeType == kEmbededHFSPlusVolumeType )
		return( true );

	return( false );

} /* VolumeObjectIsEmbeddedHFSPlus */


//******************************************************************************
//	Routine:	VolumeObjectIsPureHFSPlus
//
//	Function:	determine if the volume represented by our VolumeObject is an
//				pure HFS plus volume.
//
// 	Result:		returns true if pure HFS plus volume.
//******************************************************************************
Boolean VolumeObjectIsPureHFSPlus( void )
{
	VolumeObjectPtr				myVOPtr;

	myVOPtr = GetVolumeObjectPtr( );
	
	if ( myVOPtr->volumeType == kPureHFSPlusVolumeType )
		return( true );

	return( false );

} /* VolumeObjectIsPureHFSPlus */


//******************************************************************************
//	Routine:	GetVolumeObjectPtr
//
//	Function:	Accessor routine to get a pointer to our VolumeObject structure.
//
// 	Result:		returns pointer to our VolumeObject.
//******************************************************************************
VolumeObjectPtr GetVolumeObjectPtr( void )
{
	static VolumeObject		myVolumeObject;
	static int				myInited = 0;
	
	if ( myInited == 0 ) {
		myInited++;
		bzero( &myVolumeObject, sizeof(myVolumeObject) );
	}
	
	return( &myVolumeObject );
	 
} /* GetVolumeObjectPtr */


//******************************************************************************
//	Routine:	CheckEmbeddedVolInfoInMDBs
//
//	Function:	Check the primary and alternate MDB to see if the embedded volume
//				information (drEmbedSigWord and drEmbedExtent) match.
//
// 	Result:		NA
//******************************************************************************
void CheckEmbeddedVolInfoInMDBs( SGlobPtr GPtr )
{
	OSErr						err;
	Boolean						primaryIsDamaged = false;
	Boolean						alternateIsDamaged = false;
	VolumeObjectPtr				myVOPtr;
	HFSMasterDirectoryBlock *	myPriMDBPtr;
	HFSMasterDirectoryBlock *	myAltMDBPtr;
	UInt64						myOffset;
	UInt64						mySectors;
	BlockDescriptor  			myPrimary;
	BlockDescriptor  			myAlternate;

	myVOPtr = GetVolumeObjectPtr( );
	myPrimary.buffer = NULL;
	myAlternate.buffer = NULL;

	// we only check this if primary and alternate are OK at this point.  OK means
	// that the primary and alternate MDBs have the correct signature and at least
	// one of them points to a valid embedded HFS+ volume.
	if ( VolumeObjectIsEmbeddedHFSPlus( ) == false || 
		 (myVOPtr->flags & kVO_PriMDBOK) == 0 || (myVOPtr->flags & kVO_AltMDBOK) == 0 )
		return;

	err = GetVolumeObjectPrimaryMDB( &myPrimary );
	if ( err != noErr ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
			plog( "\tcould not get primary MDB \n" );
		}
		goto ExitThisRoutine;
	}
	myPriMDBPtr = (HFSMasterDirectoryBlock	*) myPrimary.buffer;
	err = GetVolumeObjectAlternateMDB( &myAlternate );
	if ( err != noErr ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
			plog( "\tcould not get alternate MDB \n" );
		}
		goto ExitThisRoutine;
	}
	myAltMDBPtr = (HFSMasterDirectoryBlock	*) myAlternate.buffer;

	// bail if everything looks good.  NOTE - we can bail if drEmbedExtent info
	// is the same in the primary and alternate MDB because we know one of them is
	// valid (or VolumeObjectIsEmbeddedHFSPlus would be false and we would not be
	// here).
	if ( myPriMDBPtr->drEmbedSigWord == kHFSPlusSigWord && 
		 myAltMDBPtr->drEmbedSigWord == kHFSPlusSigWord &&
		 myPriMDBPtr->drEmbedExtent.blockCount == myAltMDBPtr->drEmbedExtent.blockCount &&
		 myPriMDBPtr->drEmbedExtent.startBlock == myAltMDBPtr->drEmbedExtent.startBlock )
		goto ExitThisRoutine;

	// we know that VolumeObject.embeddedOffset and VolumeObject.totalEmbeddedSectors 
	// are correct so we will verify the info in each MDB calculates to these values.
	myOffset = (myPriMDBPtr->drEmbedExtent.startBlock * myPriMDBPtr->drAlBlkSiz) + 
			   (myPriMDBPtr->drAlBlSt * Blk_Size);
	mySectors = (myPriMDBPtr->drAlBlkSiz / Blk_Size) * myPriMDBPtr->drEmbedExtent.blockCount;

	if ( myOffset != myVOPtr->embeddedOffset || mySectors != myVOPtr->totalEmbeddedSectors ) 
		primaryIsDamaged = true;
	
	myOffset = (myAltMDBPtr->drEmbedExtent.startBlock * myAltMDBPtr->drAlBlkSiz) + 
			   (myAltMDBPtr->drAlBlSt * Blk_Size);
	mySectors = (myAltMDBPtr->drAlBlkSiz / Blk_Size) * myAltMDBPtr->drEmbedExtent.blockCount;

	if ( myOffset != myVOPtr->embeddedOffset || mySectors != myVOPtr->totalEmbeddedSectors ) 
		alternateIsDamaged = true;
	
	// now check drEmbedSigWord if everything else is OK
	if ( primaryIsDamaged == false && alternateIsDamaged == false ) {
		if ( myPriMDBPtr->drEmbedSigWord != kHFSPlusSigWord )
			primaryIsDamaged = true;
		else if ( myAltMDBPtr->drEmbedSigWord != kHFSPlusSigWord )
			alternateIsDamaged = true;
	}

	if ( primaryIsDamaged || alternateIsDamaged ) {
		GPtr->VIStat |= S_WMDB;
		WriteError( GPtr, E_MDBDamaged, 7, 0 );
		if ( primaryIsDamaged ) {
			myVOPtr->flags &= ~kVO_PriMDBOK; // mark the primary MDB as damaged
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog )
				plog("\tinvalid primary wrapper MDB \n");
		}
		else {
			myVOPtr->flags &= ~kVO_AltMDBOK; // mark the alternate MDB as damaged
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog )
				plog("\tinvalid alternate wrapper MDB \n");
		}
	}
		
ExitThisRoutine:
	if ( myPrimary.buffer != NULL )
		(void) ReleaseVolumeBlock( myVOPtr->vcbPtr, &myPrimary, kReleaseBlock );
	if ( myAlternate.buffer != NULL )
		(void) ReleaseVolumeBlock( myVOPtr->vcbPtr, &myAlternate, kReleaseBlock );

	return;

} /* CheckEmbeddedVolInfoInMDBs */


//******************************************************************************
//	Routine:	ValidVolumeHeader
//
//	Function:	Run some sanity checks to make sure the HFSPlusVolumeHeader is valid
//
// 	Result:		error
//******************************************************************************
OSErr	ValidVolumeHeader( HFSPlusVolumeHeader *volumeHeader )
{
	OSErr	err;
	
	if ((volumeHeader->signature == kHFSPlusSigWord && volumeHeader->version == kHFSPlusVersion) ||
	    (volumeHeader->signature == kHFSXSigWord && volumeHeader->version == kHFSXVersion))
	{
		if ( (volumeHeader->blockSize != 0) && ((volumeHeader->blockSize & 0x01FF) == 0) )			//	non zero multiple of 512
			err = noErr;
		else
			err = badMDBErr;							//¥¥	I want badVolumeHeaderErr in Errors.i
	}
	else
	{
		err = noMacDskErr;
	}
	
	return( err );
}


//_______________________________________________________________________
//
//	InitBTreeHeader
//	
//	This routine initializes a B-Tree header.
//
//	Note: Since large volumes will have bigger b-trees they need to
//	have map nodes setup.
//_______________________________________________________________________

void InitBTreeHeader (UInt32 fileSize, UInt32 clumpSize, UInt16 nodeSize, UInt16 recordCount, UInt16 keySize,
						UInt32 attributes, UInt32 *mapNodes, void *buffer)
{
	UInt32		 nodeCount;
	UInt32		 usedNodes;
	UInt32		 nodeBitsInHeader;
	BTHeaderRec	 *bth;
	BTNodeDescriptor *ndp;
	UInt32		*bitMapPtr;
	SInt16		*offsetPtr;


	ClearMemory(buffer, nodeSize);			// start out with clean node
	
	nodeCount = fileSize / nodeSize;
	nodeBitsInHeader = 8 * (nodeSize - sizeof(BTNodeDescriptor) - sizeof(BTHeaderRec) - kBTreeHeaderUserBytes - 4*sizeof(SInt16));
	
	usedNodes =	1;							// header takes up one node
	*mapNodes = 0;							// number of map nodes initially (0)


	// FILL IN THE NODE DESCRIPTOR:
	ndp = (BTNodeDescriptor*) buffer;	// point to node descriptor

	ndp->kind = kBTHeaderNode;		// this node contains the B-tree header
	ndp->numRecords = 3;			// there are 3 records (header, map, and user)

	if (nodeCount > nodeBitsInHeader)		// do we need additional map nodes?
	{
		UInt32	nodeBitsInMapNode;
		
		nodeBitsInMapNode = 8 * (nodeSize - sizeof(BTNodeDescriptor) - 2*sizeof(SInt16) - 2);	//¥¥ why (-2) at end???

		if (recordCount > 0)			// catalog B-tree?
			ndp->fLink = 2;			// link points to initial map node
											//¥¥ Assumes all records will fit in one node.  It would be better
											//¥¥ to put the map node(s) first, then the records.
		else
			ndp->fLink = 1;			// link points to initial map node

		*mapNodes = (nodeCount - nodeBitsInHeader + (nodeBitsInMapNode - 1)) / nodeBitsInMapNode;
		usedNodes += *mapNodes;
	}

	// FILL IN THE HEADER RECORD:
	bth = (BTHeaderRec*) ((char*)buffer + sizeof(BTNodeDescriptor));	// point to header

	if (recordCount > 0)
	{
		++usedNodes;								// one more node will be used

		bth->treeDepth = 1;							// tree depth is one level (leaf)
		bth->rootNode  = 1;							// root node is also leaf
		bth->firstLeafNode = 1;						// first leaf node
		bth->lastLeafNode = 1;						// last leaf node
	}

	bth->attributes	  = attributes;					// flags for 16-bit key lengths, and variable sized index keys
	bth->leafRecords  = recordCount;				// total number of data records
	bth->nodeSize	  = nodeSize;					// size of a node
	bth->maxKeyLength = keySize;					// maximum length of a key
	bth->totalNodes	  = nodeCount;					// total number of nodes
	bth->freeNodes	  = nodeCount - usedNodes;		// number of free nodes
	bth->clumpSize	  = clumpSize;					//
//	bth->btreeType	  = 0;						// 0 = meta data B-tree


	// FILL IN THE MAP RECORD:
	bitMapPtr = (UInt32*) ((Byte*) buffer + sizeof(BTNodeDescriptor) + sizeof(BTHeaderRec) + kBTreeHeaderUserBytes); // point to bitmap

	// MARK NODES THAT ARE IN USE:
	// Note - worst case (32MB alloc blk) will have only 18 nodes in use.
	*bitMapPtr = ~((UInt32) 0xFFFFFFFF >> usedNodes);
	

	// PLACE RECORD OFFSETS AT THE END OF THE NODE:
	offsetPtr = (SInt16*) ((Byte*) buffer + nodeSize - 4*sizeof(SInt16));

	*offsetPtr++ = sizeof(BTNodeDescriptor) + sizeof(BTHeaderRec) + kBTreeHeaderUserBytes + nodeBitsInHeader/8;	// offset to free space
	*offsetPtr++ = sizeof(BTNodeDescriptor) + sizeof(BTHeaderRec) + kBTreeHeaderUserBytes;						// offset to allocation map
	*offsetPtr++ = sizeof(BTNodeDescriptor) + sizeof(BTHeaderRec);												// offset to user space
	*offsetPtr   = sizeof(BTNodeDescriptor);										// offset to BTH
}

/*------------------------------------------------------------------------------

Routine:	CalculateItemCount

Function:	determines number of items for progress feedback

Input:		vRefNum:  the volume to count items

Output:		number of items

------------------------------------------------------------------------------*/

void	CalculateItemCount( SGlob *GPtr, UInt64 *itemCount, UInt64 *onePercent )
{
	BTreeControlBlock	*btcb;
	VolumeObjectPtr		myVOPtr;
	UInt64				items;
	UInt32				realFreeNodes;
	SVCB				*vcb  = GPtr->calculatedVCB;
	
	/* each bitmap segment is an item */
	myVOPtr = GetVolumeObjectPtr( );
	items = GPtr->calculatedVCB->vcbTotalBlocks / 1024;
	
	//
	// Items is the used node count and leaf record count for each btree...
	//

	btcb = (BTreeControlBlock*) vcb->vcbCatalogFile->fcbBtree;
	realFreeNodes = ((BTreeExtensionsRec*)btcb->refCon)->realFreeNodeCount;
	items += (2 * btcb->leafRecords) + (btcb->totalNodes - realFreeNodes);

	btcb = (BTreeControlBlock*) vcb->vcbExtentsFile->fcbBtree;
	realFreeNodes = ((BTreeExtensionsRec*)btcb->refCon)->realFreeNodeCount;
	items += btcb->leafRecords + (btcb->totalNodes - realFreeNodes);

	if ( vcb->vcbAttributesFile != NULL )
	{
		btcb = (BTreeControlBlock*) vcb->vcbAttributesFile->fcbBtree;
		realFreeNodes = ((BTreeExtensionsRec*)btcb->refCon)->realFreeNodeCount;
	
		items += (btcb->leafRecords + (btcb->totalNodes - realFreeNodes));
	}
	
	*onePercent = items/ 100;
	
	//
	//	[2239291]  We're calculating the progress for the wrapper and the embedded volume separately, which
	//	confuses the caller (since they see the progress jump to a large percentage while checking the wrapper,
	//	then jump to a small percentage when starting to check the embedded volume).  To avoid this behavior,
	//	we pretend the wrapper has 100 times as many items as it really does.  This means the progress will
	//	never exceed 1% for the wrapper.
	//
/* fsck_hfs doesn't deal wih the wrapper at this time (8.29.2002)
	if ( (myVOPtr->volumeType == kEmbededHFSPlusVolumeType) && (GPtr->inputFlags & examineWrapperMask) )
		items *= 100; */
	
	//	Add en extra Å 5% to smooth the progress
	items += *onePercent * 5;
	
	*itemCount = items;
}


SFCB* ResolveFCB(short fileRefNum)
{
	return( (SFCB*)((unsigned long)GetFCBSPtr() + (unsigned long)fileRefNum)  );
}


//******************************************************************************
//	Routine:	SetupFCB fills in the FCB info
//
//	Returns:	The filled up FCB
//******************************************************************************
void	SetupFCB( SVCB *vcb, SInt16 refNum, UInt32 fileID, UInt32 fileClumpSize )
{
	SFCB *fcb;

	fcb = ResolveFCB(refNum);
	
	fcb->fcbFileID		= fileID;
	fcb->fcbVolume		= vcb;
	fcb->fcbClumpSize	= fileClumpSize;
}


//******************************************************************************
//
//	Routine:	ResolveFileRefNum
//
//	Purpose:	Return a file reference number for a given file control block
//				pointer.
//
//	Input:
//		fileCtrlBlockPtr	Pointer to the SFCB
//
//	Output:
//		result				File reference number,
//							or 0 if fileCtrlBlockPtr is invalid
//
pascal short ResolveFileRefNum(SFCB * fileCtrlBlockPtr)
{
	return( (unsigned long)fileCtrlBlockPtr - (unsigned long)GetFCBSPtr() );
}



Ptr gFCBSPtr;

void	SetFCBSPtr( Ptr value )
{
	gFCBSPtr = value;
}

Ptr	GetFCBSPtr( void )
{
	return (gFCBSPtr);
}


//_______________________________________________________________________
//
//	Routine:	FlushVolumeControlBlock
//	Arguments:	SVCB		*vcb
//	Output:		OSErr			err
//
//	Function: 	Flush volume information to either the HFSPlusVolumeHeader 
//	of the Master Directory Block
//_______________________________________________________________________

OSErr	FlushVolumeControlBlock( SVCB *vcb )
{
	OSErr				err;
	HFSPlusVolumeHeader	*volumeHeader;
	SFCB				*fcb;
	BlockDescriptor  	block;
	
	if ( ! IsVCBDirty( vcb ) )			//	if it's not dirty
		return( noErr );

	block.buffer = NULL;
	err = GetVolumeObjectPrimaryBlock( &block );
	if ( err != noErr )
	{
		// attempt to fix the primary with alternate
		if ( block.buffer != NULL ) {
			(void) ReleaseVolumeBlock( vcb, &block, kReleaseBlock );
			block.buffer = NULL;
		}
			
		err = VolumeObjectFixPrimaryBlock( );
		ReturnIfError( err );
		
		// should be able to get it now
		err = GetVolumeObjectPrimaryBlock( &block );
		ReturnIfError( err );
	}

	if ( vcb->vcbSignature == kHFSPlusSigWord )
	{		
		volumeHeader = (HFSPlusVolumeHeader *) block.buffer;
		
		// 2005507, Keep the MDB creation date and HFSPlusVolumeHeader creation date in sync.
		if ( vcb->vcbEmbeddedOffset != 0 )  // It's a wrapped HFS+ volume
		{
			HFSMasterDirectoryBlock		*mdb;
			BlockDescriptor  			mdb_block;

			mdb_block.buffer = NULL;
			err = GetVolumeObjectPrimaryMDB( &mdb_block );
			if ( err == noErr )
			{
				mdb = (HFSMasterDirectoryBlock	*) mdb_block.buffer;
				if ( mdb->drCrDate != vcb->vcbCreateDate )  // The creation date changed
				{
					mdb->drCrDate = vcb->vcbCreateDate;
					(void) ReleaseVolumeBlock(vcb, &mdb_block, kForceWriteBlock);
					mdb_block.buffer = NULL;
				}
			}
			if ( mdb_block.buffer != NULL )
				(void) ReleaseVolumeBlock(vcb, &mdb_block, kReleaseBlock);
		}

		volumeHeader->attributes		= vcb->vcbAttributes;
		volumeHeader->lastMountedVersion = kFSCKMountVersion;
		volumeHeader->createDate		= vcb->vcbCreateDate;  // NOTE: local time, not GMT!
		volumeHeader->modifyDate		= vcb->vcbModifyDate;
		volumeHeader->backupDate		= vcb->vcbBackupDate;
		volumeHeader->checkedDate		= vcb->vcbCheckedDate;
		volumeHeader->fileCount			= vcb->vcbFileCount;
		volumeHeader->folderCount		= vcb->vcbFolderCount;
		volumeHeader->blockSize			= vcb->vcbBlockSize;
		volumeHeader->totalBlocks		= vcb->vcbTotalBlocks;
		volumeHeader->freeBlocks		= vcb->vcbFreeBlocks;
		volumeHeader->nextAllocation		= vcb->vcbNextAllocation;
		volumeHeader->rsrcClumpSize		= vcb->vcbRsrcClumpSize;
		volumeHeader->dataClumpSize		= vcb->vcbDataClumpSize;
		volumeHeader->nextCatalogID		= vcb->vcbNextCatalogID;
		volumeHeader->writeCount		= vcb->vcbWriteCount;
		volumeHeader->encodingsBitmap		= vcb->vcbEncodingsBitmap;

		//¥¥Êshould we use the vcb or fcb clumpSize values ????? -djb
		volumeHeader->allocationFile.clumpSize	= vcb->vcbAllocationFile->fcbClumpSize;
		volumeHeader->extentsFile.clumpSize	= vcb->vcbExtentsFile->fcbClumpSize;
		volumeHeader->catalogFile.clumpSize	= vcb->vcbCatalogFile->fcbClumpSize;
		
		CopyMemory( vcb->vcbFinderInfo, volumeHeader->finderInfo, sizeof(volumeHeader->finderInfo) );
	
		fcb = vcb->vcbExtentsFile;
		CopyMemory( fcb->fcbExtents32, volumeHeader->extentsFile.extents, sizeof(HFSPlusExtentRecord) );
		volumeHeader->extentsFile.logicalSize = fcb->fcbLogicalSize;
		volumeHeader->extentsFile.totalBlocks = (u_int32_t)(fcb->fcbPhysicalSize / vcb->vcbBlockSize);
	
		fcb = vcb->vcbCatalogFile;
		CopyMemory( fcb->fcbExtents32, volumeHeader->catalogFile.extents, sizeof(HFSPlusExtentRecord) );
		volumeHeader->catalogFile.logicalSize = fcb->fcbLogicalSize;
		volumeHeader->catalogFile.totalBlocks = (u_int32_t)(fcb->fcbPhysicalSize / vcb->vcbBlockSize);

		fcb = vcb->vcbAllocationFile;
		CopyMemory( fcb->fcbExtents32, volumeHeader->allocationFile.extents, sizeof(HFSPlusExtentRecord) );
		volumeHeader->allocationFile.logicalSize = fcb->fcbLogicalSize;
		volumeHeader->allocationFile.totalBlocks = (u_int32_t)(fcb->fcbPhysicalSize / vcb->vcbBlockSize);
		
		if (vcb->vcbAttributesFile != NULL)	//	Only update fields if an attributes file existed and was open
		{
			fcb = vcb->vcbAttributesFile;
			CopyMemory( fcb->fcbExtents32, volumeHeader->attributesFile.extents, sizeof(HFSPlusExtentRecord) );
			volumeHeader->attributesFile.logicalSize = fcb->fcbLogicalSize;
			volumeHeader->attributesFile.clumpSize = fcb->fcbClumpSize;
			volumeHeader->attributesFile.totalBlocks = (u_int32_t)(fcb->fcbPhysicalSize / vcb->vcbBlockSize);
		}
	}
	else
	{
		HFSMasterDirectoryBlock	*mdbP;

		mdbP = (HFSMasterDirectoryBlock	*) block.buffer;

		mdbP->drCrDate    = vcb->vcbCreateDate;
		mdbP->drLsMod     = vcb->vcbModifyDate;
		mdbP->drAtrb      = (UInt16)vcb->vcbAttributes;
		mdbP->drClpSiz    = vcb->vcbDataClumpSize;
		mdbP->drNxtCNID   = vcb->vcbNextCatalogID;
		mdbP->drFreeBks   = vcb->vcbFreeBlocks;
		mdbP->drXTClpSiz  = vcb->vcbExtentsFile->fcbClumpSize;
		mdbP->drCTClpSiz  = vcb->vcbCatalogFile->fcbClumpSize;

		mdbP->drNmFls     = vcb->vcbNmFls;
		mdbP->drNmRtDirs  = vcb->vcbNmRtDirs;
		mdbP->drFilCnt    = vcb->vcbFileCount;
		mdbP->drDirCnt    = vcb->vcbFolderCount;

		fcb = vcb->vcbExtentsFile;
		CopyMemory( fcb->fcbExtents16, mdbP->drXTExtRec, sizeof( mdbP->drXTExtRec ) );

		fcb = vcb->vcbCatalogFile;
		CopyMemory( fcb->fcbExtents16, mdbP->drCTExtRec, sizeof( mdbP->drCTExtRec ) );
	}
		
	//--	Write the VHB/MDB out by releasing the block dirty
	if ( block.buffer != NULL ) {
		err = ReleaseVolumeBlock(vcb, &block, kForceWriteBlock);	
		block.buffer = NULL;
	}
	MarkVCBClean( vcb );

	return( err );
}


//_______________________________________________________________________
//
//	Routine:	FlushAlternateVolumeControlBlock
//	Arguments:	SVCB		*vcb
//				Boolean			ifHFSPlus
//	Output:		OSErr			err
//
//	Function: 	Flush volume information to either the Alternate HFSPlusVolumeHeader or the 
//				Alternate Master Directory Block.  Called by the BTree when the catalog
//				or extent files grow.  Simply BlockMoves the original to the alternate
//				location.
//_______________________________________________________________________

OSErr	FlushAlternateVolumeControlBlock( SVCB *vcb, Boolean isHFSPlus )
{
	OSErr  				err;
	VolumeObjectPtr		myVOPtr;
	UInt64				myBlockNum;
	BlockDescriptor  	pri_block, alt_block;
	
	pri_block.buffer = NULL;
	alt_block.buffer = NULL;
	myVOPtr = GetVolumeObjectPtr( );

	err = FlushVolumeControlBlock( vcb );
	err = GetVolumeObjectPrimaryBlock( &pri_block );
	
	// invalidate if we have not marked the primary as OK
	if ( VolumeObjectIsHFS( ) ) {
		if ( (myVOPtr->flags & kVO_PriMDBOK) == 0 )
			err = badMDBErr;
	}
	else if ( (myVOPtr->flags & kVO_PriVHBOK) == 0 ) {
		err = badMDBErr;
	}
	if ( err != noErr )
		goto ExitThisRoutine;

	GetVolumeObjectAlternateBlockNum( &myBlockNum );
	if ( myBlockNum != 0 ) {
		// we don't care if this is an invalid MDB / VHB since we will write over it
		err = GetVolumeObjectAlternateBlock( &alt_block );
		if ( err == noErr || err == badMDBErr || err == noMacDskErr ) {
			CopyMemory( pri_block.buffer, alt_block.buffer, Blk_Size );
			(void) ReleaseVolumeBlock(vcb, &alt_block, kForceWriteBlock);		
			alt_block.buffer = NULL;
		}
	}

ExitThisRoutine:
	if ( pri_block.buffer != NULL )
		(void) ReleaseVolumeBlock( vcb, &pri_block, kReleaseBlock );
	if ( alt_block.buffer != NULL )
		(void) ReleaseVolumeBlock( vcb, &alt_block, kReleaseBlock );

	return( err );
}

void
ConvertToHFSPlusExtent( const HFSExtentRecord oldExtents, HFSPlusExtentRecord newExtents)
{
	UInt16	i;

	// go backwards so we can convert in place!
	
	for (i = kHFSPlusExtentDensity-1; i > 2; --i)
	{
		newExtents[i].blockCount = 0;
		newExtents[i].startBlock = 0;
	}

	newExtents[2].blockCount = oldExtents[2].blockCount;
	newExtents[2].startBlock = oldExtents[2].startBlock;
	newExtents[1].blockCount = oldExtents[1].blockCount;
	newExtents[1].startBlock = oldExtents[1].startBlock;
	newExtents[0].blockCount = oldExtents[0].blockCount;
	newExtents[0].startBlock = oldExtents[0].startBlock;
}



OSErr	CacheWriteInPlace( SVCB *vcb, UInt32 fileRefNum,  HIOParam *iopb, UInt64 currentPosition, UInt32 maximumBytes, UInt32 *actualBytes )
{
	OSErr err;
	UInt64 diskBlock;
	UInt32 contiguousBytes;
	void* buffer;
	
	*actualBytes = 0;
	buffer = (char*)iopb->ioBuffer + iopb->ioActCount;

	err = MapFileBlockC(vcb, ResolveFCB(fileRefNum), maximumBytes, (currentPosition >> kSectorShift),
				&diskBlock, &contiguousBytes );
	if (err)
		return (err);

	err = DeviceWrite(vcb->vcbDriverWriteRef, vcb->vcbDriveNumber, buffer, (diskBlock << Log2BlkLo), contiguousBytes, actualBytes);
	
	return( err );
}


void PrintName( int theCount, const UInt8 *theNamePtr, Boolean isUnicodeString )
{
    int			myCount;
 	int			i;
    
    myCount = (isUnicodeString) ? (theCount * 2) : theCount;
    for ( i = 0; i < myCount; i++ ) 
       	plog( "%02X ", *(theNamePtr + i) );
   	plog( "\n" );

} /* PrintName */

/* Function:	add_prime_bucket_uint32
 *
 * Description:
 * This function increments the prime number buckets in the prime bucket 
 * set based on the uint32_t number provided.  This function increments 
 * each prime number bucket by one at an offset of the corresponding 
 * remainder of the division.  This function is based on Chinese Remainder 
 * Theorem and adds the given number to the set to compare later.
 * 
 * Input:
 *		1. Corresponding prime bucket to increment.
 *		2. uint32_t number to add to the set.
 *
 * Output:	nil
 */
void add_prime_bucket_uint32(PrimeBuckets *cur, uint32_t num)
{
	int r32, r27, r25, r7, r11, r13, r17, r19, r23, r29, r31;

	if (!cur) {
		return;
	}

	/* Perform the necessary divisions here */
	r32 = num % 32;
	r27 = num % 27;
	r25 = num % 25;
	r7  = num % 7;
	r11 = num % 11;
	r13 = num % 13;
	r17 = num % 17;
	r19 = num % 19;
	r23 = num % 23;
	r29 = num % 29;
	r31 = num % 31;
	
	/* Update bucket for attribute bit */
	cur->n32[r32]++;
	cur->n27[r27]++;
	cur->n25[r25]++;
	cur->n7[r7]++;
	cur->n11[r11]++;
	cur->n13[r13]++;
	cur->n17[r17]++;
	cur->n19[r19]++;
	cur->n23[r23]++;
	cur->n29[r29]++;
	cur->n31[r31]++;

	return;
}

/* Function:	add_prime_bucket_uint64
 *
 * Description:
 * This function increments the prime number buckets in the prime bucket 
 * set based on the uint64_t number provided.  This function increments 
 * each prime number bucket by one at an offset of the corresponding 
 * remainder of the division.  This function is based on Chinese Remainder 
 * Theorem and adds the given number to the set to compare later.
 * 
 * Input:
 *		1. Corresponding prime bucket to increment.
 *		2. uint64_t number to add to the set.
 *
 * Output:	nil
 */
void add_prime_bucket_uint64(PrimeBuckets *cur, uint64_t num)
{
	size_t r32, r27, r25, r7, r11, r13, r17, r19, r23, r29, r31;

	if (!cur) {
		return;
	}

	/* Perform the necessary divisions here */
	r32 = num % 32;
	r27 = num % 27;
	r25 = num % 25;
	r7  = num % 7;
	r11 = num % 11;
	r13 = num % 13;
	r17 = num % 17;
	r19 = num % 19;
	r23 = num % 23;
	r29 = num % 29;
	r31 = num % 31;
	
	/* Update bucket for attribute bit */
	cur->n32[r32]++;
	cur->n27[r27]++;
	cur->n25[r25]++;
	cur->n7[r7]++;
	cur->n11[r11]++;
	cur->n13[r13]++;
	cur->n17[r17]++;
	cur->n19[r19]++;
	cur->n23[r23]++;
	cur->n29[r29]++;
	cur->n31[r31]++;

	return;
}

/* Compares the two prime buckets provided. 
 * Returns - 
 * 	zero - If the two buckets are same.
 *  non-zero - If the two buckets do not match.
 */
int compare_prime_buckets(PrimeBuckets *bucket1, PrimeBuckets *bucket2) 
{
	int retval = 1;
	int i;

	for (i=0; i<32; i++) {
		if (bucket1->n32[i] != bucket2->n32[i]) {
			goto out;
		}
	}
	
	for (i=0; i<27; i++) {
		if (bucket1->n27[i] != bucket2->n27[i]) {
			goto out;
		}
	}

	for (i=0; i<25; i++) {
		if (bucket1->n25[i] != bucket2->n25[i]) {
			goto out;
		}
	}

	for (i=0; i<7; i++) {
		if (bucket1->n7[i] != bucket2->n7[i]) {
			goto out;
		}
	}
	
	for (i=0; i<11; i++) {
		if (bucket1->n11[i] != bucket2->n11[i]) {
			goto out;
		}
	}

	for (i=0; i<13; i++) {
		if (bucket1->n13[i] != bucket2->n13[i]) {
			goto out;
		}
	}

	for (i=0; i<17; i++) {
		if (bucket1->n17[i] != bucket2->n17[i]) {
			goto out;
		}
	}

	for (i=0; i<19; i++) {
		if (bucket1->n19[i] != bucket2->n19[i]) {
			goto out;
		}
	}

	for (i=0; i<23; i++) {
		if (bucket1->n23[i] != bucket2->n23[i]) {
			goto out;
		}
	}

	for (i=0; i<29; i++) {
		if (bucket1->n29[i] != bucket2->n29[i]) {
			goto out;
		}
	}

	for (i=0; i<31; i++) {
		if (bucket1->n31[i] != bucket2->n31[i]) {
			goto out;
		}
	}

	retval = 0;

out:
	return retval;
}

/* Prints the prime number bucket for the passed pointer */
void print_prime_buckets(PrimeBuckets *cur) 
{
	int i;
	
	plog ("n32 = { ");
	for (i=0; i<32; i++) {
		plog ("%d,", cur->n32[i]);
	}
	plog ("}\n");

	plog ("n27 = { ");
	for (i=0; i<27; i++) {
		plog ("%d,", cur->n27[i]);
	}
	plog ("}\n");

	plog ("n25 = { ");
	for (i=0; i<25; i++) {
		plog ("%d,", cur->n25[i]);
	}
	plog ("}\n");

	plog ("n7 = { ");
	for (i=0; i<7; i++) {
		plog ("%d,", cur->n7[i]);
	}
	plog ("}\n");
	
	plog ("n11 = { ");
	for (i=0; i<11; i++) {
		plog ("%d,", cur->n11[i]);
	}
	plog ("}\n");

	plog ("n13 = { ");
	for (i=0; i<13; i++) {
		plog ("%d,", cur->n13[i]);
	}
	plog ("}\n");

	plog ("n17 = { ");
	for (i=0; i<17; i++) {
		plog ("%d,", cur->n17[i]);
	}
	plog ("}\n");

	plog ("n19 = { ");
	for (i=0; i<19; i++) {
		plog ("%d,", cur->n19[i]);
	}
	plog ("}\n");

	plog ("n23 = { ");
	for (i=0; i<23; i++) {
		plog ("%d,", cur->n23[i]);
	}
	plog ("}\n");

	plog ("n29 = { ");
	for (i=0; i<29; i++) {
		plog ("%d,", cur->n29[i]);
	}
	plog ("}\n");

	plog ("n31 = { ");
	for (i=0; i<31; i++) {
		plog ("%d,", cur->n31[i]);
	}
	plog ("}\n");
}
