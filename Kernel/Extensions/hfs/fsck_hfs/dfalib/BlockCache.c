/*
 * Copyright (c) 2000-2003, 2005, 2007-2008 Apple Inc. All rights reserved.
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

#include "SRuntime.h"
#include "Scavenger.h"
#include "../cache.h"



extern Cache_t fscache;


static OSStatus  ReadFragmentedBlock (SFCB *file, UInt32 blockNum, BlockDescriptor *block);
static OSStatus  WriteFragmentedBlock( 	SFCB *file, 
										BlockDescriptor *block, 
										int age, 
										uint32_t writeOptions );
static OSStatus  ReleaseFragmentedBlock (SFCB *file, BlockDescriptor *block, int age);


void
InitBlockCache(SVCB *volume)
{
	volume->vcbBlockCache = (void *) &fscache;
}


/*
 *  kGetBlock
 *  kForceReadBlock
 *  kGetEmptyBlock
 *  kSkipEndianSwap
 */
OSStatus
GetVolumeBlock (SVCB *volume, UInt64 blockNum, GetBlockOptions options, BlockDescriptor *block)
{
	UInt32  blockSize;
	SInt64  offset;
	UInt16  signature;
	OSStatus result;	
	Buf_t *   buffer;
	Cache_t * cache;

	buffer = NULL;
	cache  = (Cache_t *) volume->vcbBlockCache;
	blockSize = 512;

	offset = (SInt64) ((UInt64) blockNum) << kSectorShift;

	result = CacheRead (cache, offset, blockSize, &buffer);

	if (result == 0) {
		block->blockHeader = buffer;
		block->buffer = buffer->Buffer;
		block->blockNum = blockNum;
		block->blockSize = blockSize;
		block->blockReadFromDisk = 0;
		block->fragmented = 0;
	} else {
		block->blockHeader = NULL;
		block->buffer = NULL;
	}
	
	if (!(options & kSkipEndianSwap) && (result == 0)) {
		HFSMasterDirectoryBlock *mdb;

		mdb = (HFSMasterDirectoryBlock *)block->buffer;
		signature = SWAP_BE16(mdb->drSigWord);
		if (signature == kHFSPlusSigWord || signature == kHFSXSigWord)
			SWAP_HFSPLUSVH(block->buffer);
		else if (signature == kHFSSigWord)
			SWAP_HFSMDB(block->buffer);
	}
	return (result);
}


/*
 *  kReleaseBlock
 *  kForceWriteBlock
 *  kMarkBlockDirty
 *  kTrashBlock
 *  kSkipEndianSwap
 */
OSStatus
ReleaseVolumeBlock (SVCB *volume, BlockDescriptor *block, ReleaseBlockOptions options)
{
	OSStatus  result = 0;
	Cache_t * cache;
	Buf_t *   buffer;
	int       age;
	UInt16  signature;

	cache  = (Cache_t *) volume->vcbBlockCache;
	buffer = (Buf_t *) block->blockHeader;
	age    = ((options & kTrashBlock) != 0);

	/*
	 * Always leave the blocks in the cache in big endian
	 */
	if (!(options & kSkipEndianSwap)) {
		signature = ((HFSMasterDirectoryBlock *)block->buffer)->drSigWord;
		if (signature == kHFSPlusSigWord || signature == kHFSXSigWord)
			SWAP_HFSPLUSVH(block->buffer);
		else if (signature == kHFSSigWord)
			SWAP_HFSMDB(block->buffer);
	}

	if (options & (kMarkBlockDirty | kForceWriteBlock)) {
		result = CacheWrite(cache, buffer, age, 0);
	} else { /* not dirty */
		result = CacheRelease (cache, buffer, age);
	}
	return (result);
}


/*
 *  kGetBlock
 *  kForceReadBlock
 *  kGetEmptyBlock
 */
OSStatus
GetFileBlock (SFCB *file, UInt32 blockNum, GetBlockOptions options, BlockDescriptor *block)
{
	UInt64	diskBlock;
	UInt32	contiguousBytes;
	SInt64  offset;

	OSStatus result;	
	Buf_t *   buffer;
	Cache_t * cache;

	buffer = NULL;
	block->buffer = NULL;
	block->blockHeader = NULL;
	cache  = (Cache_t *)file->fcbVolume->vcbBlockCache;

	/* Map file block to volume block */
	result = MapFileBlockC(file->fcbVolume, file, file->fcbBlockSize,
			(((UInt64)blockNum * (UInt64)file->fcbBlockSize) >> kSectorShift),
			&diskBlock, &contiguousBytes);
	if (result) return (result);

	if (contiguousBytes < file->fcbBlockSize)
		return ( ReadFragmentedBlock(file, blockNum, block) );

	offset = (SInt64) ((UInt64) diskBlock) << kSectorShift;

	result = CacheRead (cache, offset, file->fcbBlockSize, &buffer);
	if (result)  return (result);

	block->blockHeader = buffer;
	block->buffer = buffer->Buffer;
	block->blockNum = blockNum;
	block->blockSize = file->fcbBlockSize;
	block->blockReadFromDisk = 0;
	block->fragmented = 0;
	
	return (noErr);
}


/*
 *  kReleaseBlock
 *  kForceWriteBlock
 *  kMarkBlockDirty
 *  kTrashBlock
 */
OSStatus
ReleaseFileBlock (SFCB *file, BlockDescriptor *block, ReleaseBlockOptions options)
{
	OSStatus  result = 0;
	Cache_t * cache;
	Buf_t *   buffer;
	int       age;
	uint32_t  writeOptions = 0;

	cache  = (Cache_t *)file->fcbVolume->vcbBlockCache;
	buffer = (Buf_t *) block->blockHeader;
	age    = ((options & kTrashBlock) != 0);

	if ( (options & kForceWriteBlock) == 0 )
		/* only write if we're forced to */
		writeOptions |= kLazyWrite;

	if (options & (kMarkBlockDirty | kForceWriteBlock)) {
		if (block->fragmented)
			result = WriteFragmentedBlock(file, block, age, writeOptions);
		else
			result = CacheWrite(cache, buffer, age, writeOptions);
	} else { /* not dirty */

		if (block->fragmented)
			result = ReleaseFragmentedBlock(file, block, age);
		else
			result = CacheRelease (cache, buffer, age);
	}
	return (result);
}


/*
 *
 */
OSStatus
SetFileBlockSize (SFCB *file, ByteCount blockSize)
{
	file->fcbBlockSize = blockSize;

	return (0);
}


/*
 * Read a block that is fragmented across 2 or more allocation blocks
 *
 *  - a block descriptor buffer is allocated here
 *  - the blockHeader field holds a list of Buf_t buffers.
 *  - the fragmented flag is set
 */
static OSStatus
ReadFragmentedBlock (SFCB *file, UInt32 blockNum, BlockDescriptor *block)
{
	UInt64	sector;
	UInt32	fragSize, blockSize;
	UInt64  fileOffset; 
	SInt64  diskOffset;
	SVCB *  volume;
	int     i, maxFrags;
	OSStatus result;	
	Buf_t **   bufs;   /* list of Buf_t pointers */
	Cache_t * cache;
	char *	buffer;

	volume = file->fcbVolume;
	cache  = (Cache_t *)volume->vcbBlockCache;

	blockSize = file->fcbBlockSize;
	maxFrags = blockSize / volume->vcbBlockSize;
	fileOffset = (UInt64)blockNum * (UInt64)blockSize;
	
	buffer = (char *) AllocateMemory(blockSize);
	bufs = (Buf_t **) AllocateClearMemory(maxFrags * sizeof(Buf_t *));
	if (buffer == NULL || bufs == NULL) {
		result = memFullErr;
		return (result);
	}
	
	block->buffer = buffer;
	block->blockHeader = bufs;
	block->blockNum = blockNum;
	block->blockSize = blockSize;
	block->blockReadFromDisk = false;
	block->fragmented = true;
	
	for (i = 0; (i < maxFrags) && (blockSize > 0); ++i) {
		result = MapFileBlockC (volume, file, blockSize,
					fileOffset >> kSectorShift,
					&sector, &fragSize);
		if (result) goto ErrorExit;

		diskOffset = (SInt64) (sector) << kSectorShift;
		result = CacheRead (cache, diskOffset, fragSize, &bufs[i]);
		if (result) goto ErrorExit;
		
		if (bufs[i]->Length != fragSize) {
            if (debug) {
                plog("ReadFragmentedBlock: cache failure (Length != fragSize)\n");
            }
			result = -1;
			goto ErrorExit;
		}

		CopyMemory(bufs[i]->Buffer, buffer, fragSize);
		buffer     += fragSize;
		fileOffset += fragSize;
		blockSize  -= fragSize;
	}
	
	return (noErr);

ErrorExit:
	i = 0;
	while (bufs[i] != NULL) {
		(void) CacheRelease (cache, bufs[i], true);
		++i;
	}

	DisposeMemory(block->buffer);
	DisposeMemory(block->blockHeader);

	block->blockHeader = NULL;
	block->buffer = NULL;

	return (result);
}


/*
 * Write a block that is fragmented across 2 or more allocation blocks
 *
 */
static OSStatus
WriteFragmentedBlock( SFCB *file, BlockDescriptor *block, int age, uint32_t writeOptions )
{
	Cache_t * cache;
	Buf_t **  bufs;  /* list of Buf_t pointers */
	char *	  buffer;
	char *    bufEnd;
	UInt32	  fragSize;
	OSStatus  result;
	int i = 0;

	result = 0;
	cache  = (Cache_t *) file->fcbVolume->vcbBlockCache;
	bufs   = (Buf_t **) block->blockHeader;
	buffer = (char *) block->buffer;
	bufEnd = buffer + file->fcbBlockSize;

	if (bufs == NULL) {
        if (debug) {
            plog("WriteFragmentedBlock: NULL bufs list!\n");
        }
		return (-1);
	}
	
	while ((bufs[i] != NULL) && (buffer < bufEnd)) {
		fragSize = bufs[i]->Length;
		
		/* copy data for this fragment */
		CopyMemory(buffer, bufs[i]->Buffer, fragSize);
		
		/* write it back to cache */
		result = CacheWrite(cache, bufs[i], age, writeOptions);
		if (result) break;

		buffer += fragSize;
		++i;
	}
	
	DisposeMemory(block->buffer);
	DisposeMemory(block->blockHeader);

	block->buffer = NULL;
	block->blockHeader = NULL;
	block->fragmented = false;

	return (result);
}


/*
 * Release a block that is fragmented across 2 or more allocation blocks
 *
 */
static OSStatus
ReleaseFragmentedBlock (SFCB *file, BlockDescriptor *block, int age)
{
	Cache_t * cache;
	Buf_t **   bufs;  /* list of Buf_t pointers */
	char	*buffer;
	char	*bufEnd;
	UInt32	fragSize;
	int i = 0;

	cache = (Cache_t *)file->fcbVolume->vcbBlockCache;
	bufs  = (Buf_t **) block->blockHeader;

	if (bufs == NULL) {
        if (debug) {
            plog("ReleaseFragmentedBlock: NULL buf list!\n");
        }
		return (-1);
	}

	buffer = (char*)block->buffer;
	bufEnd = buffer + file->fcbBlockSize;

	while (bufs[i] != NULL && (buffer < bufEnd)) {
		fragSize = bufs[i]->Length;
		buffer += fragSize;
		(void) CacheRelease (cache, bufs[i], true);
		++i;
	}
	
	DisposeMemory(block->buffer);
	DisposeMemory(block->blockHeader);

	block->buffer = NULL;
	block->blockHeader = NULL;
	block->fragmented = false;

	return (noErr);
}

