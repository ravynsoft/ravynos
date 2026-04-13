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
	File:		SAllocate.c

	Contains:	Routines for accessing and modifying the volume bitmap.

	Version:	HFS Plus 1.0

	Copyright:	й 1996-1999 by Apple Computer, Inc., all rights reserved.

*/

/*
Public routines:
	BlockAllocate
					Allocate space on a volume.  Can allocate space contiguously.
					If not contiguous, then allocation may be less than what was
					asked for.  Returns the starting block number, and number of
					blocks.  (Will only do a single extent???)
	BlockDeallocate
					Deallocate a contiguous run of allocation blocks.

Internal routines:
	BlockAllocateAny
					Find and allocate a contiguous range of blocks up to a given size.  The
					first range of contiguous free blocks found are allocated, even if there
					are fewer blocks than requested (and even if a contiguous range of blocks
					of the given size exists elsewhere).

	BlockMarkFree
					Mark a contiguous range of blocks as free.  The corresponding
					bits in the volume bitmap will be cleared.
	BlockMarkAllocated
					Mark a contiguous range of blocks as allocated.  The cor-
					responding bits in the volume bitmap are set.  Also tests to see
					if any of the blocks were previously unallocated.
	FindContiguous
					Find a contiguous range of blocks of a given size.  The caller
					specifies where to begin the search (by block number).  The
					block number of the first block in the range is returned.
	BlockAllocateContig
					Find and allocate a contiguous range of blocks of a given size.  If
					a contiguous range of free blocks of the given size isn't found, then
					the allocation fails (i.e. it is "all or nothing").
	ReadBitmapBlock
					Given an allocation block number, read the bitmap block that
					contains that allocation block into a caller-supplied buffer.
*/

#include "Scavenger.h"	


enum {
	kBitsPerByte			=	8, 
	kBitsPerWord			=	32,
	kBitsWithinWordMask		=	kBitsPerWord-1
};

#define kBytesPerBlock		( (vcb->vcbSignature == kHFSSigWord) ? kHFSBlockSize : vcb->vcbAllocationFile->fcbBlockSize )
#define kWordsPerBlock		( kBytesPerBlock / 4 )
#define kBitsPerBlock		( kBytesPerBlock * kBitsPerByte )
#define kBitsWithinBlockMask ( kBitsPerBlock - 1 )
#define kWordsWithinBlockMask ( kWordsPerBlock - 1 )

#define kLowBitInWordMask	0x00000001u
#define kHighBitInWordMask	0x80000000u
#define kAllBitsSetInWord	0xFFFFFFFFu


static OSErr ReadBitmapBlock(
	SVCB		*vcb,
	UInt32		 bit,
	BlockDescriptor *block);

static OSErr ReleaseBitmapBlock(
	SVCB            *vcb,
	OptionBits      options,
	BlockDescriptor  *block);

static OSErr BlockAllocateContig(
	SVCB		*vcb,
	UInt32			startingBlock,
	UInt32			minBlocks,
	UInt32			maxBlocks,
	UInt32			*actualStartBlock,
	UInt32			*actualNumBlocks);

static OSErr BlockAllocateAny(
	SVCB		*vcb,
	UInt32			startingBlock,
	register UInt32	endingBlock,
	UInt32			maxBlocks,
	UInt32			*actualStartBlock,
	UInt32			*actualNumBlocks);

static OSErr BlockFindContiguous(
	SVCB		*vcb,
	UInt32			startingBlock,
	UInt32			endingBlock,
	UInt32			minBlocks,
	UInt32			maxBlocks,
	UInt32			*actualStartBlock,
	UInt32			*actualNumBlocks);

static OSErr BlockMarkAllocated(
	SVCB		*vcb,
	UInt32			startingBlock,
	UInt32			numBlocks);

static OSErr BlockMarkFree(
	SVCB		*vcb,
	UInt32			startingBlock,
	UInt32			numBlocks);

/*
;________________________________________________________________________________
;
; Routine:	   BlockAllocate
;
; Function:    Allocate space on a volume.	If contiguous allocation is requested,
;			   at least the requested number of bytes will be allocated or an
;			   error will be returned.	If contiguous allocation is not forced,
;			   the space will be allocated at the first free fragment following
;			   the requested starting allocation block.  If there is not enough
;			   room there, a block of less than the requested size will be
;			   allocated.
;
;			   If the requested starting block is 0 (for new file allocations),
;			   the volume's allocation block pointer will be used as a starting
;			   point.
;
;				The function uses on-disk volume bitmap for allocation
;				and updates it with newly allocated blocks.  It also 
;				updates the in-memory volume bitmap.
;
; Input Arguments:
;	 vcb			 - Pointer to SVCB for the volume to allocate space on
;	 fcb			 - Pointer to FCB for the file for which storage is being allocated
;	 startingBlock	 - Preferred starting allocation block, 0 = no preference
;	 forceContiguous  -	Force contiguous flag - if bit 0 set, allocation is contiguous
;						or an error is returned
;	 blocksRequested  -	Number of allocation blocks requested.	If the allocation is
;						non-contiguous, less than this may actually be allocated
;	 blocksMaximum	  -	The maximum number of allocation blocks to allocate.  If there
;						is additional free space after blocksRequested, then up to
;						blocksMaximum blocks should really be allocated.  (Used by
;						ExtendFileC to round up allocations to a multiple of the
;						file's clump size.)
;
; Output:
;	 (result)		 - Error code, zero for successful allocation
;	 *startBlock	 - Actual starting allocation block
;	 *actualBlocks	 - Actual number of allocation blocks allocated
;
; Side effects:
;	 The volume bitmap is read and updated; the volume bitmap cache may be changed.
;
; Modification history:
;________________________________________________________________________________
*/

OSErr BlockAllocate (
	SVCB		*vcb,				/* which volume to allocate space on */
	UInt32			startingBlock,		/* preferred starting block, or 0 for no preference */
	UInt32			blocksRequested,	/* desired number of BYTES to allocate */
	UInt32			blocksMaximum,		/* maximum number of bytes to allocate */
	Boolean			forceContiguous,	/* non-zero to force contiguous allocation and to force */
										/* bytesRequested bytes to actually be allocated */
	UInt32			*actualStartBlock,	/* actual first block of allocation */
	UInt32			*actualNumBlocks)	/* number of blocks actually allocated; if forceContiguous */
										/* was zero, then this may represent fewer than bytesRequested */
										/* bytes */
{
	OSErr			err;
	Boolean			updateAllocPtr = false;		//	true if nextAllocation needs to be updated

	//
	//	Initialize outputs in case we get an error
	//
	*actualStartBlock = 0;
	*actualNumBlocks = 0;
	
	//
	//	If the disk is already full, don't bother.
	//
	if (vcb->vcbFreeBlocks == 0) {
		err = dskFulErr;
		goto Exit;
	}
	if (forceContiguous && vcb->vcbFreeBlocks < blocksRequested) {
		err = dskFulErr;
		goto Exit;
	}
	
	//
	//	If caller didn't specify a starting block number, then use the volume's
	//	next block to allocate from.
	//
	if (startingBlock == 0) {
		startingBlock = vcb->vcbNextAllocation;
		updateAllocPtr = true;
	}
	
	//
	//	If the request must be contiguous, then find a sequence of free blocks
	//	that is long enough.  Otherwise, find the first free block.
	//
	if (forceContiguous)
		err = BlockAllocateContig(vcb, startingBlock, blocksRequested, blocksMaximum, actualStartBlock, actualNumBlocks);
	else {
		//	We'll try to allocate contiguous space first.  If that fails, we'll fall back to finding whatever tiny
		//	extents we can find.  It would be nice if we kept track of the largest N free extents so that falling
		//	back grabbed a small number of large extents.
		err = BlockAllocateContig(vcb, startingBlock, blocksRequested, blocksMaximum, actualStartBlock, actualNumBlocks);
		if (err == dskFulErr)
			err = BlockAllocateAny(vcb, startingBlock, vcb->vcbTotalBlocks, blocksMaximum, actualStartBlock, actualNumBlocks);
		if (err == dskFulErr)
			err = BlockAllocateAny(vcb, 0, startingBlock, blocksMaximum, actualStartBlock, actualNumBlocks);
	}

	if (err == noErr) {
		//
		//	If we used the volume's roving allocation pointer, then we need to update it.
		//	Adding in the length of the current allocation might reduce the next allocate
		//	call by avoiding a re-scan of the already allocated space.  However, the clump
		//	just allocated can quite conceivably end up being truncated or released when
		//	the file is closed or its EOF changed.  Leaving the allocation pointer at the
		//	start of the last allocation will avoid unnecessary fragmentation in this case.
		//
		if (updateAllocPtr)
			vcb->vcbNextAllocation = *actualStartBlock;
		
		//
		//	Update the number of free blocks on the volume
		//
		vcb->vcbFreeBlocks -= *actualNumBlocks;
		MarkVCBDirty(vcb);
	}
	
Exit:

	return err;
}



/*
;________________________________________________________________________________
;
; Routine:	   BlockDeallocate
;
; Function:    Update the bitmap to deallocate a run of disk allocation blocks
;	 The on-disk volume bitmap is read and updated; the in-memory volume bitmap 
;	 is also updated.
;
; Input Arguments:
;	 vcb		- Pointer to SVCB for the volume to free space on
;	 firstBlock	- First allocation block to be freed
;	 numBlocks	- Number of allocation blocks to free up (must be > 0!)
;
; Output:
;	 (result)	- Result code
;
; Side effects:
;	 The on-disk volume bitmap is read and updated; the in-memory volume bitmap 
;	 is also changed.
;
; Modification history:
;
;	<06Oct85>  PWD	Changed to check for error after calls to ReadBM and NextWord
;					Now calls NextBit to read successive bits from the bitmap
;________________________________________________________________________________
*/

OSErr BlockDeallocate (
	SVCB		*vcb,			//	Which volume to deallocate space on
	UInt32			firstBlock,		//	First block in range to deallocate
	UInt32			numBlocks)		//	Number of contiguous blocks to deallocate
{
	OSErr			err;
	

	//
	//	If no blocks to deallocate, then exit early
	//
	if (numBlocks == 0) {
		err = noErr;
		goto Exit;
	}

	//
	//	Call internal routine to free the sequence of blocks
	//
	err = BlockMarkFree(vcb, firstBlock, numBlocks);
	if (err)
		goto Exit;

	//
	//	Update the volume's free block count, and mark the VCB as dirty.
	//
	vcb->vcbFreeBlocks += numBlocks;
	MarkVCBDirty(vcb);

Exit:

	return err;
}


/*
;_______________________________________________________________________
;
; Routine:	DivideAndRoundUp
;
; Function:	Divide numerator by denominator, rounding up the result if there
;			was a remainder.  This is frequently used for computing the number
;			of whole and/or partial blocks used by some count of bytes.
;_______________________________________________________________________
*/
UInt32 DivideAndRoundUp(
	UInt32 numerator,
	UInt32 denominator)
{
	UInt32	quotient;
	
	quotient = numerator / denominator;
	if (quotient * denominator != numerator)
		quotient++;
	
	return quotient;
}



/*
;_______________________________________________________________________
;
; Routine:	ReadBitmapBlock
;
; Function:	Read in a bitmap block corresponding to a given allocation
;			block.  Return a pointer to the bitmap block.
;
; Inputs:
;	vcb			--	Pointer to SVCB
;	block		--	Allocation block whose bitmap block is desired
;
; Outputs:
;	buffer		--	Pointer to bitmap block corresonding to "block"
;_______________________________________________________________________
*/
static OSErr ReadBitmapBlock(
	SVCB		*vcb,
	UInt32		bit,
	BlockDescriptor *block)
{
	OSErr err = noErr;
	UInt64	blockNum;

	if (vcb->vcbSignature == kHFSSigWord) {
		//
		//	HFS: Turn block number into physical block offset within the
		//	bitmap, and then the physical block within the volume.
		//
		blockNum = bit / kBitsPerBlock;   /* block offset within bitmap */
		blockNum += vcb->vcbVBMSt;        /* block within whole volume  */
		
		err = GetVolumeBlock(vcb, blockNum, kGetBlock | kSkipEndianSwap, block);

	} else {
		// HFS+:  "bit" is the allocation block number that we are looking for  
		// in the allocation bit map.  GetFileBlock wants a file block number 
		// so we calculate how many bits (kBitsPerBlock) fit in a file  
		// block then convert that to a file block number (bit / kBitsPerBlock) 
		// for our call.
		err = GetFileBlock( vcb->vcbAllocationFile, (bit / kBitsPerBlock), kGetBlock, block );
	}

	return err;
}



static OSErr ReleaseBitmapBlock(
	SVCB		*vcb,
	OptionBits options,
	BlockDescriptor *block)
{
	OSErr err;

	if (vcb->vcbSignature == kHFSSigWord)
		err = ReleaseVolumeBlock (vcb, block, options | kSkipEndianSwap);
	else
		err = ReleaseFileBlock (vcb->vcbAllocationFile, block, options);

	return err;
}



/*
_______________________________________________________________________

Routine:	BlockAllocateContig

Function:	Allocate a contiguous group of allocation blocks.  The
			allocation is all-or-nothing.  The caller guarantees that
			there are enough free blocks (though they may not be
			contiguous, in which case this call will fail).

			The function uses on-disk volume bitmap for allocation
			and updates it with newly allocated blocks.  It also 
			updates the in-memory volume bitmap.

Inputs:
	vcb				Pointer to volume where space is to be allocated
	startingBlock	Preferred first block for allocation
	minBlocks		Minimum number of contiguous blocks to allocate
	maxBlocks		Maximum number of contiguous blocks to allocate

Outputs:
	actualStartBlock	First block of range allocated, or 0 if error
	actualNumBlocks		Number of blocks allocated, or 0 if error
_______________________________________________________________________
*/
static OSErr BlockAllocateContig(
	SVCB		*vcb,
	UInt32			startingBlock,
	UInt32			minBlocks,
	UInt32			maxBlocks,
	UInt32			*actualStartBlock,
	UInt32			*actualNumBlocks)
{
	OSErr	err;

	//
	//	Find a contiguous group of blocks at least minBlocks long.
	//	Determine the number of contiguous blocks available (up
	//	to maxBlocks).
	//
	err = BlockFindContiguous(vcb, startingBlock, vcb->vcbTotalBlocks, minBlocks, maxBlocks,
								  actualStartBlock, actualNumBlocks);
	if (err == dskFulErr) {
		//ее Should constrain the endingBlock here, so we don't bother looking for ranges
		//ее that start after startingBlock, since we already checked those.
		err = BlockFindContiguous(vcb, 0, vcb->vcbTotalBlocks, minBlocks, maxBlocks,
									  actualStartBlock, actualNumBlocks);
	}
	if (err != noErr) goto Exit;

	//
	//	Now mark those blocks allocated.
	//
	err = BlockMarkAllocated(vcb, *actualStartBlock, *actualNumBlocks);
	
Exit:
	if (err != noErr) {
		*actualStartBlock = 0;
		*actualNumBlocks = 0;
	}
	
	return err;
}



/*
_______________________________________________________________________

Routine:	BlockAllocateAny

Function:	Allocate one or more allocation blocks.  If there are fewer
			free blocks than requested, all free blocks will be
			allocated.  The caller guarantees that there is at least
			one free block.

			The function uses on-disk volume bitmap for allocation
			and updates it with newly allocated blocks.  It also 
			updates the in-memory volume bitmap.

Inputs:
	vcb				Pointer to volume where space is to be allocated
	startingBlock	Preferred first block for allocation
	endingBlock		Last block to check + 1
	maxBlocks		Maximum number of contiguous blocks to allocate

Outputs:
	actualStartBlock	First block of range allocated, or 0 if error
	actualNumBlocks		Number of blocks allocated, or 0 if error
_______________________________________________________________________
*/
static OSErr BlockAllocateAny(
	SVCB		*vcb,
	UInt32			startingBlock,
	register UInt32	endingBlock,
	UInt32			maxBlocks,
	UInt32			*actualStartBlock,
	UInt32			*actualNumBlocks)
{
	OSErr			err;
	register UInt32	block = 0;		//	current block number
	register UInt32	currentWord;	//	Pointer to current word within bitmap block
	register UInt32	bitMask;		//	Word with given bits already set (ready to OR in)
	register UInt32	wordsLeft;		//	Number of words left in this bitmap block
	UInt32 *buffer;
	BlockDescriptor bd = {0};
	OptionBits  relOpt = kReleaseBlock;

	//	Since this routine doesn't wrap around
	if (maxBlocks > (endingBlock - startingBlock)) {
		maxBlocks = endingBlock - startingBlock;
	}

	//
	//	Pre-read the first bitmap block
	//
	
	err = ReadBitmapBlock(vcb, startingBlock, &bd);
	if (err != noErr) goto Exit;
	relOpt = kMarkBlockDirty;
	buffer = (UInt32 *) bd.buffer;

	//
	//	Set up the current position within the block
	//
	{
		UInt32 wordIndexInBlock;

		wordIndexInBlock = (startingBlock & kBitsWithinBlockMask) / kBitsPerWord;
		buffer += wordIndexInBlock;
		wordsLeft = kWordsPerBlock - wordIndexInBlock;
		currentWord = SWAP_BE32(*buffer);
		bitMask = kHighBitInWordMask >> (startingBlock & kBitsWithinWordMask);
	}
	
	//
	//	Find the first unallocated block
	//
	block = startingBlock;
	while (block < endingBlock) {
		if ((currentWord & bitMask) == 0)
			break;
		
		//	Next bit
		++block;
		bitMask >>= 1;
		if (bitMask == 0) {
			//	Next word
			bitMask = kHighBitInWordMask;
			++buffer;
			
			if (--wordsLeft == 0) {
				//	Next block
		            	err = ReleaseBitmapBlock(vcb, relOpt, &bd);
		                bd.buffer = NULL;
				if (err != noErr) goto Exit;

				err = ReadBitmapBlock(vcb, block, &bd);
				if (err != noErr) goto Exit;
                		buffer = (UInt32 *) bd.buffer;
				relOpt = kMarkBlockDirty;

				wordsLeft = kWordsPerBlock;
			}
			currentWord = SWAP_BE32(*buffer);
		}
	}

	//	Did we get to the end of the bitmap before finding a free block?
	//	If so, then couldn't allocate anything.
	if (block == endingBlock) {
		err = dskFulErr;
		goto Exit;
	}

	//	Return the first block in the allocated range
	*actualStartBlock = block;
	
	//	If we could get the desired number of blocks before hitting endingBlock,
	//	then adjust endingBlock so we won't keep looking.  Ideally, the comparison
	//	would be (block + maxBlocks) < endingBlock, but that could overflow.  The
	//	comparison below yields identical results, but without overflow.
	if (block < (endingBlock-maxBlocks)) {
		endingBlock = block + maxBlocks;	//	if we get this far, we've found enough
	}
	
	//
	//	Allocate all of the consecutive blocks
	//
	while ((currentWord & bitMask) == 0) {
		//	Allocate this block
		currentWord |= bitMask;
		
		//	Move to the next block.  If no more, then exit.
		++block;
		if (block == endingBlock)
			break;

		//	Next bit
		bitMask >>= 1;
		if (bitMask == 0) {
			*buffer = SWAP_BE32(currentWord);	// update value in bitmap
			
			//	Next word
			bitMask = kHighBitInWordMask;
			++buffer;
			
			if (--wordsLeft == 0) {
				//	Next block
				err = ReleaseBitmapBlock(vcb, relOpt, &bd);
				if (err != noErr) goto Exit;
		                bd.buffer = NULL;

				err = ReadBitmapBlock(vcb, block, &bd);
				if (err != noErr) goto Exit;
               			buffer = (UInt32 *) bd.buffer;
				relOpt = kMarkBlockDirty;

				wordsLeft = kWordsPerBlock;
			}
			currentWord = SWAP_BE32(*buffer);
		}
	}
	*buffer = SWAP_BE32(currentWord);	// update the last change
	
Exit:
	if (err == noErr) {
		*actualNumBlocks = block - *actualStartBlock;
		
		/* Update the in-memory copy of bitmap */
		(void) CaptureBitmapBits (*actualStartBlock, *actualNumBlocks);
	}
	else {
		*actualStartBlock = 0;
		*actualNumBlocks = 0;
	}
	
	if (bd.buffer != NULL)
		(void) ReleaseBitmapBlock(vcb, relOpt, &bd);

	return err;
}



/*
_______________________________________________________________________

Routine:	BlockMarkAllocated

Function:	Mark a contiguous group of blocks as allocated (set in the
			bitmap).  The function sets the bit independent of the 
			previous state (set/clear) of the bit.

			The function uses on-disk volume bitmap for allocation
			and updates it with newly allocated blocks.  It also 
			updates the in-memory volume bitmap.

Inputs:
	vcb				Pointer to volume where space is to be allocated
	startingBlock	First block number to mark as allocated
	numBlocks		Number of blocks to mark as allocated
_______________________________________________________________________
*/
static OSErr BlockMarkAllocated(
	SVCB		*vcb,
	UInt32			startingBlock,
	register UInt32	numBlocks)
{
	OSErr			err;
	register UInt32	*currentWord;	//	Pointer to current word within bitmap block
	register UInt32	wordsLeft;		//	Number of words left in this bitmap block
	register UInt32	bitMask;		//	Word with given bits already set (ready to OR in)
	UInt32			firstBit;		//	Bit index within word of first bit to allocate
	UInt32			numBits;		//	Number of bits in word to allocate
	UInt32			*buffer;
	BlockDescriptor bd = {0};
	OptionBits  relOpt = kReleaseBlock;

	UInt32 saveNumBlocks = numBlocks;
	UInt32 saveStartingBlock = startingBlock;

	//
	//	Pre-read the bitmap block containing the first word of allocation
	//
	
	err = ReadBitmapBlock(vcb, startingBlock, &bd);
	if (err != noErr) goto Exit;
	buffer = (UInt32 *) bd.buffer;
	relOpt = kMarkBlockDirty;

	//
	//	Initialize currentWord, and wordsLeft.
	//
	{
		UInt32 wordIndexInBlock;

		wordIndexInBlock = (startingBlock & kBitsWithinBlockMask) / kBitsPerWord;
		currentWord = buffer + wordIndexInBlock;
		wordsLeft = kWordsPerBlock - wordIndexInBlock;
	}
	
	//
	//	If the first block to allocate doesn't start on a word
	//	boundary in the bitmap, then treat that first word
	//	specially.
	//

	firstBit = startingBlock % kBitsPerWord;
	if (firstBit != 0) {
		bitMask = kAllBitsSetInWord >> firstBit;	//	turn off all bits before firstBit
		numBits = kBitsPerWord - firstBit;			//	number of remaining bits in this word
		if (numBits > numBlocks) {
			numBits = numBlocks;					//	entire allocation is inside this one word
			bitMask &= ~(kAllBitsSetInWord >> (firstBit + numBits));	//	turn off bits after last
		}
#if DEBUG_BUILD
		if ((*currentWord & SWAP_BE32(bitMask)) != 0) {
			DebugStr("\pFATAL: blocks already allocated!");
			err = fsDSIntErr;
			goto Exit;
		}
#endif
		*currentWord |= SWAP_BE32(bitMask);				//	set the bits in the bitmap
		numBlocks -= numBits;						//	adjust number of blocks left to allocate

		++currentWord;								//	move to next word
		--wordsLeft;								//	one less word left in this block
	}

	//
	//	Allocate whole words (32 blocks) at a time.
	//

	bitMask = kAllBitsSetInWord;					//	put this in a register for 68K
	while (numBlocks >= kBitsPerWord) {
		if (wordsLeft == 0) {
			//	Read in the next bitmap block
			startingBlock += kBitsPerBlock;			//	generate a block number in the next bitmap block

			err = ReleaseBitmapBlock(vcb, relOpt, &bd);
			if (err != noErr) goto Exit;
			bd.buffer = NULL;
			
			err = ReadBitmapBlock(vcb, startingBlock, &bd);
			if (err != noErr) goto Exit;
			buffer = (UInt32 *) bd.buffer;
			relOpt = kMarkBlockDirty;
			
			//	Readjust currentWord and wordsLeft
			currentWord = buffer;
			wordsLeft = kWordsPerBlock;
		}
#if DEBUG_BUILD
		if (*currentWord != 0) {
			DebugStr("\pFATAL: blocks already allocated!");
			err = fsDSIntErr;
			goto Exit;
		}
#endif
		*currentWord = SWAP_BE32(bitMask);
		numBlocks -= kBitsPerWord;

		++currentWord;								//	move to next word
		--wordsLeft;								//	one less word left in this block
	}
	
	//
	//	Allocate any remaining blocks.
	//
	
	if (numBlocks != 0) {
		bitMask = ~(kAllBitsSetInWord >> numBlocks);	//	set first numBlocks bits
		if (wordsLeft == 0) {
			//	Read in the next bitmap block
			startingBlock += kBitsPerBlock;				//	generate a block number in the next bitmap block

			err = ReleaseBitmapBlock(vcb, relOpt, &bd);
			if (err != noErr) goto Exit;
			bd.buffer = NULL;

			err = ReadBitmapBlock(vcb, startingBlock, &bd);
			if (err != noErr) goto Exit;
			buffer = (UInt32 *) bd.buffer;
			relOpt = kMarkBlockDirty;
			
			//	Readjust currentWord and wordsLeft
			currentWord = buffer;
			wordsLeft = kWordsPerBlock;
		}
#if DEBUG_BUILD
		if ((*currentWord & SWAP_BE32(bitMask)) != 0) {
			DebugStr("\pFATAL: blocks already allocated!");
			err = fsDSIntErr;
			goto Exit;
		}
#endif
		*currentWord |= SWAP_BE32(bitMask);						//	set the bits in the bitmap

		//	No need to update currentWord or wordsLeft
	}

	/* Update the in-memory copy of the volume bitmap */
	(void) CaptureBitmapBits(saveStartingBlock, saveNumBlocks);

Exit:
	if (bd.buffer != NULL)
		(void) ReleaseBitmapBlock(vcb, relOpt, &bd);

	return err;
}



/*
_______________________________________________________________________

Routine:	BlockMarkFree

Function:	Mark a contiguous group of blocks as free (clear in the
			bitmap).  The function clears the bit independent of the 
			previous state (set/clear) of the bit.

			This function uses the on-disk bitmap and also updates 
			the in-memory bitmap with the deallocated blocks

Inputs:
	vcb				Pointer to volume where space is to be freed
	startingBlock	First block number to mark as freed
	numBlocks		Number of blocks to mark as freed
_______________________________________________________________________
*/
static OSErr BlockMarkFree(
	SVCB		*vcb,
	UInt32			startingBlock,
	register UInt32	numBlocks)
{
	OSErr			err;
	register UInt32	*currentWord;	//	Pointer to current word within bitmap block
	register UInt32	wordsLeft;		//	Number of words left in this bitmap block
	register UInt32	bitMask;		//	Word with given bits already set (ready to OR in)
	UInt32			firstBit;		//	Bit index within word of first bit to allocate
	UInt32			numBits;		//	Number of bits in word to allocate
	UInt32			*buffer;
	BlockDescriptor bd = {0};
	OptionBits  relOpt = kReleaseBlock;

	UInt32 saveNumBlocks = numBlocks;
	UInt32 saveStartingBlock = startingBlock;

	//
	//	Pre-read the bitmap block containing the first word of allocation
	//

	err = ReadBitmapBlock(vcb, startingBlock, &bd);
	if (err != noErr) goto Exit;
	buffer = (UInt32 *) bd.buffer;
	relOpt = kMarkBlockDirty;

	//
	//	Initialize currentWord, and wordsLeft.
	//
	{
		UInt32 wordIndexInBlock;

		wordIndexInBlock = (startingBlock & kBitsWithinBlockMask) / kBitsPerWord;
		currentWord = buffer + wordIndexInBlock;
		wordsLeft = kWordsPerBlock - wordIndexInBlock;
	}
	
	//
	//	If the first block to free doesn't start on a word
	//	boundary in the bitmap, then treat that first word
	//	specially.
	//

	firstBit = startingBlock % kBitsPerWord;
	if (firstBit != 0) {
		bitMask = kAllBitsSetInWord >> firstBit;	//	turn off all bits before firstBit
		numBits = kBitsPerWord - firstBit;			//	number of remaining bits in this word
		if (numBits > numBlocks) {
			numBits = numBlocks;					//	entire allocation is inside this one word
			bitMask &= ~(kAllBitsSetInWord >> (firstBit + numBits));	//	turn off bits after last
		}
#if DEBUG_BUILD
		if ((*currentWord & SWAP_BE32(bitMask)) != SWAP_BE32(bitMask)) {
			DebugStr("\pFATAL: blocks not allocated!");
			err = fsDSIntErr;
			goto Exit;
		}
#endif
		*currentWord &= SWAP_BE32(~bitMask);			// clear the bits in the bitmap
		numBlocks -= numBits;						//	adjust number of blocks left to free

		++currentWord;								//	move to next word
		--wordsLeft;								//	one less word left in this block
	}

	//
	//	Allocate whole words (32 blocks) at a time.
	//

	while (numBlocks >= kBitsPerWord) {
		if (wordsLeft == 0) {
			//	Read in the next bitmap block
			startingBlock += kBitsPerBlock;			//	generate a block number in the next bitmap block

			err = ReleaseBitmapBlock(vcb, relOpt, &bd);
			if (err != noErr) goto Exit;
			bd.buffer = NULL;

			err = ReadBitmapBlock(vcb, startingBlock, &bd);
			if (err != noErr) goto Exit;
			buffer = (UInt32 *) bd.buffer;
			relOpt = kMarkBlockDirty;
			
			//	Readjust currentWord and wordsLeft
			currentWord = buffer;
			wordsLeft = kWordsPerBlock;
		}
#if DEBUG_BUILD
		if (*currentWord != kAllBitsSetInWord) {
			DebugStr("\pFATAL: blocks not allocated!");
			err = fsDSIntErr;
			goto Exit;
		}
#endif
		*currentWord = 0;							//	clear the entire word
		numBlocks -= kBitsPerWord;
		
		++currentWord;								//	move to next word
		--wordsLeft;								//	one less word left in this block
	}
	
	//
	//	Allocate any remaining blocks.
	//
	
	if (numBlocks != 0) {
		bitMask = ~(kAllBitsSetInWord >> numBlocks);	//	set first numBlocks bits
		if (wordsLeft == 0) {
			//	Read in the next bitmap block
			startingBlock += kBitsPerBlock;				//	generate a block number in the next bitmap block

			err = ReleaseBitmapBlock(vcb, relOpt, &bd);
			if (err != noErr) goto Exit;
			bd.buffer = NULL;

			err = ReadBitmapBlock(vcb, startingBlock, &bd);
			if (err != noErr) goto Exit;
			buffer = (UInt32 *) bd.buffer;
			relOpt = kMarkBlockDirty;
			
			//	Readjust currentWord and wordsLeft
			currentWord = buffer;
			wordsLeft = kWordsPerBlock;
		}
#if DEBUG_BUILD
		if ((*currentWord & SWAP_BE32(bitMask)) != SWAP_BE32(bitMask)) {
			DebugStr("\pFATAL: blocks not allocated!");
			err = fsDSIntErr;
			goto Exit;
		}
#endif
		*currentWord &= SWAP_BE32(~bitMask);						//	clear the bits in the bitmap

		//	No need to update currentWord or wordsLeft
	}
	
	/* Update the in-memory copy of the volume bitmap */
	(void) ReleaseBitmapBits(saveStartingBlock, saveNumBlocks);

Exit:
	if (bd.buffer != NULL)
		(void) ReleaseBitmapBlock(vcb, relOpt, &bd);

	return err;
}


/*
_______________________________________________________________________

Routine:	BlockFindContiguous

Function:	Find a contiguous range of blocks that are free (bits
			clear in the bitmap).  If a contiguous range of the
			minimum size can't be found, an error will be returned.
			
			ее It would be nice if we could skip over whole words
			ее with all bits set.
			
			ее When we find a bit set, and are about to set freeBlocks
			ее to 0, we should check to see whether there are still
			ее minBlocks bits left in the bitmap.

Inputs:
	vcb				Pointer to volume where space is to be allocated
	startingBlock	Preferred first block of range
	endingBlock		Last possible block in range + 1
	minBlocks		Minimum number of blocks needed.  Must be > 0.
	maxBlocks		Maximum (ideal) number of blocks desired

Outputs:
	actualStartBlock	First block of range found, or 0 if error
	actualNumBlocks		Number of blocks found, or 0 if error
_______________________________________________________________________
*/
/*
_________________________________________________________________________________________
	(DSH) 5/8/97 Description of BlockFindContiguous() algorithm
	Finds a contiguous range of free blocks by searching back to front.  This
	allows us to skip ranges of bits knowing that they are not candidates for
	a match because they are too small.  The below ascii diagrams illustrate
	the algorithm in action.
	
	Representation of a piece of a volume bitmap file
	If BlockFindContiguous() is called with minBlocks == 10, maxBlocks == 20


Fig. 1 initialization of variables, "<--" represents direction of travel

startingBlock (passed in)
	|
	1  0  1  0  1  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1
	|                       <--|
stopBlock                 currentBlock                        freeBlocks == 0
                                                              countedFreeBlocks == 0

Fig. 2 dirty bit found

	1  0  1  0  1  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1
	|                 |
stopBlock        currentBlock                                 freeBlocks == 3
                                                              countedFreeBlocks == 0

Fig. 3 reset variables to search for remainder of minBlocks

	1  0  1  0  1  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1
	|_________________|           |                 |
	     Unsearched            stopBlock        currentBlock    freeBlocks == 0
	                                                            countedFreeBlocks == 3

Fig. 4 minBlocks contiguous blocks found, *actualStartBlock is set

	1  0  1  0  1  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1
	|_________________|           |
	     Unsearched            stopBlock                      freeBlocks == 7
	                          currentBlock                    countedFreeBlocks == 3

Fig. 5 Now run it forwards trying to accumalate up to maxBlocks if possible

	1  0  1  0  1  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1
	|_________________|                                | -->
	     Unsearched                               currentBlock
		                                                      *actualNumBlocks == 10

Fig. 6 Dirty bit is found, return actual number of contiguous blocks found

	1  0  1  0  1  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1
	|_________________|                                                  |
	     Unsearched                                                 currentBlock
		                                                      *actualNumBlocks == 16
_________________________________________________________________________________________
*/
static OSErr BlockFindContiguous(
	SVCB		*vcb,
	UInt32			startingBlock,
	register UInt32	endingBlock,
	UInt32			minBlocks,
	UInt32			maxBlocks,
	UInt32			*actualStartBlock,
	UInt32			*actualNumBlocks)
{
	OSErr			err;
	register UInt32	bitMask;			//	mask of bit within word for currentBlock
	register UInt32	tempWord;			//	bitmap word currently being examined
	register UInt32	freeBlocks;			//	number of contiguous free blocks so far
	register UInt32	currentBlock;		//	block number we're currently examining
	UInt32			wordsLeft;			//	words remaining in bitmap block
	UInt32			*buffer = NULL;
	register UInt32	*currentWord;
	
	UInt32			stopBlock;			//	when all blocks until stopBlock are free, we found enough
	UInt32			countedFreeBlocks;	//	how many contiguous free block behind stopBlock
	UInt32			currentSector;		//	which allocations file sector
	BlockDescriptor bd = {0};

	if ((endingBlock - startingBlock) < minBlocks) {
		//	The set of blocks we're checking is smaller than the minimum number
		//	of blocks, so we couldn't possibly find a good range.
		err = dskFulErr;
		goto Exit;
	}
	
	//	Search for min blocks from back to front.
	//	If min blocks is found, advance the allocation pointer up to max blocks
	
	//
	//	Pre-read the bitmap block containing currentBlock
	//
	stopBlock    = startingBlock;
	currentBlock = startingBlock + minBlocks - 1;		//	(-1) to include startingBlock
	
	err = ReadBitmapBlock(vcb, currentBlock, &bd);
	if ( err != noErr ) goto Exit;
	buffer = (UInt32 *) bd.buffer;
	//
	//	Init buffer, currentWord, wordsLeft, and bitMask
	//
	{
		UInt32 wordIndexInBlock;

		wordIndexInBlock = ( currentBlock & kBitsWithinBlockMask ) / kBitsPerWord;
		currentWord		= buffer + wordIndexInBlock;
				
		wordsLeft		= wordIndexInBlock;
		tempWord		= SWAP_BE32(*currentWord);
		bitMask			= kHighBitInWordMask >> ( currentBlock & kBitsWithinWordMask );
		currentSector	= currentBlock / kBitsPerBlock;
	}
	
	//
	//	Look for maxBlocks free blocks.  If we find an allocated block,
	//	see if we've found minBlocks.
	//
	freeBlocks        = 0;
	countedFreeBlocks = 0;

	while ( currentBlock >= stopBlock )
	{
		//	Check current bit
		if ((tempWord & bitMask) == 0)
		{
			++freeBlocks;
		}
		else															//	Used bitmap block found
		{
			if ( ( freeBlocks +  countedFreeBlocks ) >= minBlocks )
			{
				break;													//	Found enough
			}
			else
			{
				//	We found a dirty bit, so we want to check if the next (minBlocks-freeBlocks) blocks
				//	are free beyond what we have already checked. At Fig.2 setting up for Fig.3
				
				stopBlock			= currentBlock + 1 + freeBlocks;	//	Advance stop condition
				currentBlock		+= minBlocks;
				if ( currentBlock >= endingBlock ) break;
				countedFreeBlocks	= freeBlocks;
				freeBlocks			= 0;								//	Not enough; look for another range

				if ( currentSector != currentBlock / kBitsPerBlock )
				{
					err = ReleaseBitmapBlock(vcb, kReleaseBlock, &bd);
					if (err != noErr) goto Exit;
					bd.buffer = NULL;

					err = ReadBitmapBlock( vcb, currentBlock, &bd );
					if (err != noErr) goto Exit;
					buffer = (UInt32 *) bd.buffer;
					currentSector = currentBlock / kBitsPerBlock;
				}
					
				wordsLeft	= ( currentBlock & kBitsWithinBlockMask ) / kBitsPerWord;
				currentWord	= buffer + wordsLeft;
				tempWord	= SWAP_BE32(*currentWord);
				bitMask		= kHighBitInWordMask >> ( currentBlock & kBitsWithinWordMask );

				continue;												//	Back to the while loop
			}
		}
		
		//	Move to next bit
		--currentBlock;
		bitMask <<= 1;
		if (bitMask == 0)												//	On a word boundry, start masking words
		{
			bitMask = kLowBitInWordMask;

			//	Move to next word
NextWord:
			if ( wordsLeft != 0 )
			{
				--currentWord;
				--wordsLeft;
			}
			else
			{
				//	Read in the next bitmap block
				err = ReleaseBitmapBlock(vcb, kReleaseBlock, &bd);
				if (err != noErr) goto Exit;
				bd.buffer = NULL;

				err = ReadBitmapBlock( vcb, currentBlock, &bd );
				if (err != noErr) goto Exit;
				buffer = (UInt32 *) bd.buffer;
				//	Adjust currentWord, wordsLeft, currentSector
				currentSector	= currentBlock / kBitsPerBlock;
				currentWord		= buffer + kWordsPerBlock - 1;	//	Last word in buffer
				wordsLeft		= kWordsPerBlock - 1;
			}
			
			tempWord = SWAP_BE32(*currentWord);							//	Grab the current word

			//
			//	If we found a whole word of free blocks, quickly skip over it.
			//	NOTE: we could actually go beyond the end of the bitmap if the
			//	number of allocation blocks on the volume is not a multiple of
			//	32.  If this happens, we'll adjust currentBlock and freeBlocks
			//	after the loop.
			//
			if ( tempWord == 0 )
			{
				freeBlocks		+= kBitsPerWord;
				currentBlock	-= kBitsPerWord;
				if ( freeBlocks + countedFreeBlocks >= minBlocks )
					break;									//	Found enough
				goto NextWord;
			}
		}
	}

	if ( freeBlocks + countedFreeBlocks < minBlocks )
	{
		*actualStartBlock	= 0;
		*actualNumBlocks	= 0;
		err					= dskFulErr;
		goto Exit;
	}

	//
	//	When we get here, we know we've found minBlocks continuous space.
	//	At Fig.4, setting up for Fig.5
	//	From here we do a forward search accumalating additional free blocks.
	//
	
	*actualNumBlocks	= minBlocks;
	*actualStartBlock	= stopBlock - countedFreeBlocks;	//	ActualStartBlock is set to return to the user
	currentBlock		= *actualStartBlock + minBlocks;	//	Right after found free space
	
	//	Now lets see if we can run the actualNumBlocks number all the way up to maxBlocks
	if ( currentSector != currentBlock / kBitsPerBlock )
	{
		err = ReleaseBitmapBlock(vcb, kReleaseBlock, &bd);
		if (err != noErr) goto Exit;
		bd.buffer = NULL;

		err = ReadBitmapBlock( vcb, currentBlock, &bd );
		if (err != noErr)
		{
			err	= noErr;									//	We already found the space
			goto Exit;
		}
		buffer = (UInt32 *) bd.buffer;
		currentSector = currentBlock / kBitsPerBlock;
	}

	//
	//	Init buffer, currentWord, wordsLeft, and bitMask
	//
	{
		UInt32 wordIndexInBlock;

		wordIndexInBlock = (currentBlock & kBitsWithinBlockMask) / kBitsPerWord;
		currentWord		= buffer + wordIndexInBlock;
		tempWord		= SWAP_BE32(*currentWord);
		wordsLeft		= kWordsPerBlock - wordIndexInBlock;
		bitMask			= kHighBitInWordMask >> (currentBlock & kBitsWithinWordMask);
	}

	if ( *actualNumBlocks < maxBlocks )
	{
		while ( currentBlock < endingBlock )
		{
				
			if ( (tempWord & bitMask) == 0 )
			{
				*actualNumBlocks	+= 1;
				
				if ( *actualNumBlocks == maxBlocks )
					break;
			}
			else
			{
				break;
			}
	
			//	Move to next bit
			++currentBlock;
			bitMask >>= 1;
			if (bitMask == 0)
			{
				bitMask = kHighBitInWordMask;
				++currentWord;
	
				if ( --wordsLeft == 0)
				{
					err = ReleaseBitmapBlock(vcb, kReleaseBlock, &bd);
					if (err != noErr) goto Exit;
					bd.buffer = NULL;

					err = ReadBitmapBlock(vcb, currentBlock, &bd);
					if (err != noErr) break;
					buffer = (UInt32 *) bd.buffer;
					
					//	Adjust currentWord, wordsLeft
					currentWord		= buffer;
					wordsLeft		= kWordsPerBlock;
				}
				tempWord = SWAP_BE32(*currentWord);	// grab the current word
			}
		}
	}
	
Exit:

	if (bd.buffer != NULL)
		(void) ReleaseBitmapBlock(vcb, kReleaseBlock, &bd);

	return err;
}

/*
 * Find the smallest extent in the array.
 */
static int
FindMinExt(HFSPlusExtentDescriptor *exts)
{
	int minIndx = -1;
	UInt32 min = (UInt32)-1;
	int i;

	for (i = 0; i < kHFSPlusExtentDensity; i++) {
		if (exts[i].blockCount < min) {
			min = exts[i].blockCount;
			minIndx = i;
		}
	}
	return minIndx;
}

/*
 * Truncate any excess extents.  There should be only one,
 * but we'll go through them all to make sure.
 */
static void
PruneExtents(HFSPlusExtentDescriptor *exts, UInt32 needed)
{
	int i;
	UInt32 total = 0;
	UInt32 excess = 0;

	for (i = 0; i < kHFSPlusExtentDensity; i++) {
		if (excess) {
			exts[i].startBlock = exts[i].blockCount = 0;
			continue;
		}
		total += exts[i].blockCount;
		if (total > needed) {
			exts[i].blockCount -= total - needed;
			excess = 1;
		}
	}
	return;
}

/*
 * A much more specialized function:  simply find the 8 largest extents
 * to hold the needed size.  It will either find enough blocks to
 * fit the needed size, or it will fail.
 */
OSErr
BlockFindAll(
	SFCB	*fcb,
	UInt32	needed)
{
	OSErr			err;
	SVCB		*vcb;
	register UInt32	bitMask;			//	mask of bit within word for currentBlock
	register UInt32	tempWord;			//	bitmap word currently being examined
	HFSPlusExtentDescriptor *exts = fcb->fcbExtents32;
	int	minIndx;
	UInt32	total = 0;
	
	UInt32			firstFreeBlock;
	UInt32			freeBlocks = 0;
	UInt32			currentBlock;
	UInt32			endingBlock;
	UInt32			wordsLeft;			//	words remaining in bitmap block
	UInt32			*buffer = NULL;
	UInt32			contigSize = 1;
	register UInt32	*currentWord;
	struct BlockDescriptor bd = { 0 };
	
	vcb = fcb->fcbVolume;

	if (vcb->vcbFreeBlocks < needed) {
		// Nothing to do
		if (debug)
			plog("%s:  %u blocks free, but need %u; ignoring for now\n", __FUNCTION__, vcb->vcbFreeBlocks, needed);
	}

	memset(exts, 0, sizeof(fcb->fcbExtents32));	// Zero out the extents.
	minIndx = 0;
	if (vcb->vcbBlockSize < fcb->fcbBlockSize) {
		contigSize = fcb->fcbBlockSize / vcb->vcbBlockSize;	// Number of volume blocks in a btree block
	}

	currentBlock = 0;
	endingBlock = vcb->vcbTotalBlocks;

	freeBlocks = 0;

	err = ReadBitmapBlock(vcb, currentBlock, &bd);
	if ( err != noErr ) goto done;
	buffer = (UInt32 *) bd.buffer;
	//
	//	Init buffer, currentWord, wordsLeft, and bitMask
	//
	{
		UInt32 wordIndexInBlock;

		wordIndexInBlock = ( currentBlock & kBitsWithinBlockMask ) / kBitsPerWord;
		currentWord		= buffer + wordIndexInBlock;
				
		wordsLeft		= kWordsPerBlock - wordIndexInBlock - 1;
		tempWord		= SWAP_BE32(*currentWord);
		bitMask			= kHighBitInWordMask >> ( currentBlock & kBitsWithinWordMask );
	}
	
	/*
	 * This macro is used to cycle through the allocation bitmap.
	 * We examine one bit in a word at a time; when we're done with that word,
	 * we go to the next word in the block; when we're done with that block,
	 * we get the next one.  Until we're out of blocks.
	 */
#define	nextblock() do { \
		currentBlock++; \
		if (currentBlock == endingBlock) goto done; \
		bitMask >>= 1; \
		if (bitMask == 0) { \
			bitMask = kHighBitInWordMask; \
			if (wordsLeft != 0) { \
				++currentWord; \
				--wordsLeft; \
			} else { \
				err = ReleaseBitmapBlock(vcb, kReleaseBlock, &bd); \
				if (err != noErr) goto done; \
				bd.buffer = NULL; \
				err = ReadBitmapBlock(vcb, currentBlock, &bd); \
				if (err != noErr) goto done; \
				buffer = (UInt32*)bd.buffer; \
				currentWord = buffer + ((currentBlock & kBitsWithinBlockMask) / kBitsPerWord); \
				wordsLeft = kWordsPerBlock - 1; \
			} \
			tempWord = SWAP_BE32(*currentWord); \
		} \
	} while (0)

loop:

	/*
	 * We have two while loops here.  The first one, at the top, looks for
	 * used blocks.  We ignore those.  The second while loop then looks
	 * for empty blocks, and keeps track of the length of the run.  It creates
	 * an extent from these, and puts them into the exts array.  We use
	 * the funciton FindMinExt() to find the smallest one in the array, and
	 * we only replace it if the new extent is larger.  (When first starting,
	 * all of the extents will be 0 bytes long.)
	 *
	 * We stop when we've run out of blocks (the nextblock macro will jump
	 * to done at that point), or when we've got enough total blocks to
	 * fit our needs.
	 */
	freeBlocks = 0;
	while ((tempWord & bitMask) != 0) {
		nextblock();
	}
	firstFreeBlock = currentBlock;
	while ((tempWord & bitMask) == 0) {
		++freeBlocks;
		if (freeBlocks >= needed)
			break;
		nextblock();
	}

	/*
	 * We need to ensure that nodes are not split across
	 * volume blocks -- journaling will cause an error
	 * if this happens.
	 */
	freeBlocks -= freeBlocks % contigSize;

	if (freeBlocks > exts[minIndx].blockCount) {
		total -= exts[minIndx].blockCount;
		exts[minIndx].blockCount = freeBlocks;
		exts[minIndx].startBlock = firstFreeBlock;
		total += freeBlocks;
		minIndx = FindMinExt(exts);
	}
		
	if (total >= needed) {
		goto done;
	}

	goto loop;

done:
	if (bd.buffer) {
		(void)ReleaseBitmapBlock(vcb, kReleaseBlock, &bd);
	}
	if (err == noErr) {

		if (total < needed) {
			if (debug)
				plog("%s:  found %u blocks but needed %u\n", __FUNCTION__, total, needed);
			err = dskFulErr;
		} else {
			/*
			 * If we've got enough blocks, we need to prune any extra.
			 * PruneExtents() will decrement the extents in the array to
			 * ensure we have only as much as we need.  After that, we
			 * mark them as allocated, and return.
			 */
			int i;
			if (total > needed) {
				PruneExtents(exts, needed);
			}
			for (i = 0; i < kHFSPlusExtentDensity; i++) {
				if (exts[i].blockCount) {
					BlockMarkAllocated(vcb, exts[i].startBlock, exts[i].blockCount);
					vcb->vcbFreeBlocks -= exts[i].blockCount;
				}
			}
			MarkVCBDirty(vcb);
		}
	}
	return err;
}
