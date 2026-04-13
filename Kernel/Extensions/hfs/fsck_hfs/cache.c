/*
 * Copyright (c) 2000-2012 Apple Computer, Inc. All rights reserved.
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

#include "fsck_hfs.h"
#include "cache.h"

#define true 	1
#define false 	0

#define CACHE_DEBUG  0

/*
 * CacheAllocBlock
 *
 *  Allocate an unused cache block.
 */
void *CacheAllocBlock (Cache_t *cache);

/*
 * CacheFreeBlock
 *
 *  Release an active cache block.
 */
static int 
CacheFreeBlock( Cache_t *cache, Tag_t *tag );

/*
 * CacheLookup
 *
 *  Obtain a cache block. If one already exists, it is returned. Otherwise a
 *  new one is created and inserted into the cache.
 */
int CacheLookup (Cache_t *cache, uint64_t off, Tag_t **tag);

/*
 * CacheRawRead
 *
 *  Perform a direct read on the file.
 */
int CacheRawRead (Cache_t *cache, uint64_t off, uint32_t len, void *buf);

/*
 * CacheRawWrite
 *
 *  Perform a direct write on the file.
 */
int CacheRawWrite (Cache_t *cache, uint64_t off, uint32_t len, void *buf);

/*
 * CacheFlushRange
 *
 * Flush, and optionally remove, all cache blocks that intersect
 * a given range.
 */
static int
CacheFlushRange( Cache_t *cache, uint64_t start, uint64_t len, int remove);

/*
 * LRUInit
 *
 *  Initializes the LRU data structures.
 */
static int LRUInit (LRU_t *lru);

/*
 * LRUDestroy
 *
 *  Shutdown the LRU.
 *
 *  NOTE: This is typically a no-op, since the cache manager will be clearing
 *        all cache tags.
 */
static int LRUDestroy (LRU_t *lru);

/*
 * LRUHit
 *
 *  Registers data activity on the given node. If the node is already in the
 *  LRU, it is moved to the front. Otherwise, it is inserted at the front.
 *
 *  NOTE: If the node is not in the LRU, we assume that its pointers are NULL.
 */
static int LRUHit (LRU_t *lru, LRUNode_t *node, int age);

/*
 * LRUEvict
 *
 *  Chooses a buffer to release.
 *
 *  TODO: Under extreme conditions, it should be possible to release the buffer
 *        of an actively referenced cache buffer, leaving the tag behind as a
 *        placeholder. This would be required for implementing 2Q-LRU
 *        replacement.
 */
static int LRUEvict (LRU_t *lru, LRUNode_t *node);

/*
 * CalculateCacheSizes
 *
 * Determine the cache size values that should be used to initialize the cache.   
 * If the requested value does not validate according to the conditions described
 * below, it is adjusted.
 *
 * If no input values are provided, use default values for cache size
 * and cache block size.
 *
 * Cache size should be -
 *		a. greater than or equal to minimum cache size
 *		b. less than or equal to maximum cache size.  The maximum cache size
 *		   is limited by the maximum value that can be allocated using malloc
 *		   or mmap (maximum value for size_t)
 *		c. multiple of cache block size
 *
 *	Returns: void
 *		  *calcBlockSize:  the size of the blocks in the cache
 *		  *calcTotalBlocks:  the number of blocks in the cache
 */
void CalculateCacheSizes(uint64_t cacheSize, uint32_t *calcBlockSize, uint32_t *calcTotalBlocks, char cache_debug)
{
	uint32_t blockSize = DefaultCacheBlockSize;
	const size_t	max_size_t = ~0;	/* Maximum value represented by size_t */

	/* Simple case - no user cache size, use default values */
	if (!cacheSize) {
		*calcBlockSize = DefaultCacheBlockSize;
		*calcTotalBlocks = DefaultCacheBlocks;
		goto out;
	}

	/* User provided cache size - check with minimum and maximum values */
	if (cacheSize < MinCacheSize) {
		cacheSize = MinCacheSize;
	}
	if (cacheSize > max_size_t ||
		cacheSize > MaxCacheSize) {
		if (cache_debug) {
			printf ("\tCache size should be greater than %uM and less than %luM\n", MinCacheSize/(1024*1024), max_size_t/(1024*1024));
		}
		cacheSize = MaxCacheSize;
	}

	/* Cache size should be multiple of cache block size */
	if (cacheSize % blockSize) {
		if (cache_debug) {
			printf ("\tCache size should be multiple of cache block size (currently %uK)\n", blockSize/1024);
		}
		cacheSize = (cacheSize / blockSize) * blockSize;
	}

	*calcBlockSize = blockSize;
	*calcTotalBlocks = (uint32_t)(cacheSize / blockSize);
	
out:
	return;
}

/*
 * CacheInit
 *
 *  Initializes the cache for use.  If preTouch is non-zero, the cache memory will
 *  be iterated through, with one byte per page touched.  (This is to ensure that
 *  the memory is actually created, and is used to avoid deadlocking due to swapping
 *  during a live verify of the boot volume.)
 */
int CacheInit (Cache_t *cache, int fdRead, int fdWrite, uint32_t devBlockSize,
               uint32_t cacheBlockSize, uint32_t cacheTotalBlocks, uint32_t hashSize, int preTouch)
{
	void **		temp;
	uint32_t	i;
	Buf_t *		buf;
	
	memset (cache, 0x00, sizeof (Cache_t));

	cache->FD_R = fdRead;
	cache->FD_W = fdWrite;
	cache->DevBlockSize = devBlockSize;
	/* CacheFlush requires cleared cache->Hash  */
	cache->Hash = (Tag_t **) calloc( 1, (sizeof (Tag_t *) * hashSize) );
	cache->HashSize = hashSize;
	cache->BlockSize = cacheBlockSize;

	/* Allocate the cache memory */
	/* Break out of the loop on success, or when the proposed cache is < MinCacheSize */
	while (1) {
		cache->FreeHead = mmap (NULL,
					cacheTotalBlocks * cacheBlockSize,
					PROT_READ | PROT_WRITE,
					MAP_ANON | MAP_PRIVATE,
					-1,
					0);
		if (cache->FreeHead == (void *)-1) {
			if ((cacheTotalBlocks * cacheBlockSize) <= MinCacheSize) {
				if (debug)
					printf("\tTried to allocate %dK, minimum is %dK\n",
						(cacheTotalBlocks * cacheBlockSize) / 1024,
						MinCacheSize / 1024);
				break;
			}
			if (debug)
				printf("\tFailed to allocate %uK for cache; trying %uK\n",
					(cacheTotalBlocks * cacheBlockSize) / 1024,
					(cacheTotalBlocks * cacheBlockSize / 2) / 1024);
			CalculateCacheSizes((cacheTotalBlocks * cacheBlockSize) / 2, &cacheBlockSize, &cacheTotalBlocks, debug);
			continue;
		} else {
			if (debug) {
				printf ("\tUsing cacheBlockSize=%uK cacheTotalBlock=%u cacheSize=%uK.\n", cacheBlockSize/1024, cacheTotalBlocks, (cacheBlockSize/1024) * cacheTotalBlocks);
			}
			break;
		}
	}
	if (cache->FreeHead == (void*)-1) {
#if CACHE_DEBUG
		printf("%s(%d):  FreeHead = -1\n", __FUNCTION__, __LINE__);
#endif
		return (ENOMEM);
	}


	/* If necessary, touch a byte in each page */
	if (preTouch) {
		size_t pageSize = getpagesize();
		unsigned char *ptr = (unsigned char *)cache->FreeHead;
		unsigned char *end = ptr + (cacheTotalBlocks * cacheBlockSize);
		while (ptr < end) {
			*ptr = 0;
			ptr += pageSize;
		}
	}

	/* Initialize the cache memory free list */
	temp = cache->FreeHead;
	for (i = 0; i < cacheTotalBlocks - 1; i++) {
		*temp = ((char *)temp + cacheBlockSize);
		temp  = (void **)((char *)temp + cacheBlockSize);
	}
	*temp = NULL;
	cache->FreeSize = cacheTotalBlocks;

	buf = (Buf_t *)malloc(sizeof(Buf_t) * MAXBUFS);
	if (buf == NULL) {
#if CACHE_DEBUG
		printf("%s(%d):  malloc(%zu) failed\n", __FUNCTION__, __LINE__, sizeof(Buf_t) * MAXBUFS);
#endif
		return (ENOMEM);
	}

	memset (&buf[0], 0x00, sizeof (Buf_t) * MAXBUFS);
	for (i = 1 ; i < MAXBUFS ; i++) {
		(&buf[i-1])->Next = &buf[i];
	}
	cache->FreeBufs = &buf[0];

#if CACHE_DEBUG
	printf( "%s - cacheTotalBlocks %d cacheBlockSize %d hashSize %d \n", 
			__FUNCTION__, cacheTotalBlocks, cacheBlockSize, hashSize );
	printf( "%s - cache memory %d \n", __FUNCTION__, (cacheTotalBlocks * cacheBlockSize) );
#endif  

	return (LRUInit (&cache->LRU));
}


/*
 * CacheDestroy
 * 
 *  Shutdown the cache.
 */
int CacheDestroy (Cache_t *cache)
{
	CacheFlush( cache );

#if CACHE_DEBUG
	/* Print cache report */
	printf ("Cache Report:\n");
	printf ("\tRead Requests:  %d\n", cache->ReqRead);
	printf ("\tWrite Requests: %d\n", cache->ReqWrite);
	printf ("\tDisk Reads:     %d\n", cache->DiskRead);
	printf ("\tDisk Writes:    %d\n", cache->DiskWrite);
	printf ("\tSpans:          %d\n", cache->Span);
#endif	
	/* Shutdown the LRU */
	LRUDestroy (&cache->LRU);
	
	/* I'm lazy, I'll come back to it :P */
	return (EOK);
}

/*
 * CacheRead
 *
 *  Reads a range of bytes from the cache, returning a pointer to a buffer
 *  containing the requested bytes.
 *
 *  NOTE: The returned buffer may directly refer to a cache block, or an
 *        anonymous buffer. Do not make any assumptions about the nature of
 *        the returned buffer, except that it is contiguous.
 */
int CacheRead (Cache_t *cache, uint64_t off, uint32_t len, Buf_t **bufp)
{
	Tag_t *		tag;
	Buf_t *		searchBuf;
	Buf_t *		buf;
	uint32_t	coff = (off % cache->BlockSize);
	uint64_t	cblk = (off - coff);
	int			error;

	/* Check for conflicts with other bufs */
	searchBuf = cache->ActiveBufs;
	while (searchBuf != NULL) {
		if ((searchBuf->Offset >= off) && (searchBuf->Offset < off + len)) {
#if CACHE_DEBUG
			printf ("ERROR: CacheRead: Deadlock (searchBuff = <%llu, %u>, off = %llu, off+len = %llu)\n", searchBuf->Offset, searchBuf->Length, off, off+len);
#endif
			return (EDEADLK);
		}
		
		searchBuf = searchBuf->Next;
	}
	
	/* get a free buffer */
	if ((buf = cache->FreeBufs) == NULL) {
#if CACHE_DEBUG
		printf ("ERROR: CacheRead: no more bufs!\n");
#endif
		return (ENOBUFS);
	}
	cache->FreeBufs = buf->Next; 
	*bufp = buf;

	/* Clear the buf structure */
	buf->Next	= NULL;
	buf->Prev	= NULL;
	buf->Flags	= 0;
	buf->Offset	= off;
	buf->Length	= len;
	buf->Buffer	= NULL;
	
	/* If this is unaligned or spans multiple cache blocks */
	if ((cblk / cache->BlockSize) != ((off + len - 1) / cache->BlockSize)) {
		buf->Flags |= BUF_SPAN;
	}
	/* Fetch the first cache block */
#if CACHE_DEBUG
	printf("%s(%d):  Looking up cache block %llu for offset %llu, cache blockSize %u\n", __FUNCTION__, __LINE__, cblk, off, cache->BlockSize);
#endif
	error = CacheLookup (cache, cblk, &tag);
	if (error != EOK) {
#if CACHE_DEBUG
		printf ("ERROR: CacheRead: CacheLookup error %d\n", error);
#endif
		return (error);
	}

	/* If we live nicely inside a cache block */
	if (!(buf->Flags & BUF_SPAN)) {
		/* Offset the buffer into the cache block */
		buf->Buffer = tag->Buffer + coff;

		/* Bump the cache block's reference count */
		tag->Refs++;
		
		/* Kick the node into the right queue */
		LRUHit (&cache->LRU, (LRUNode_t *)tag, 0);

	/* Otherwise, things get ugly */
	} else {
		uint32_t	boff;	/* Offset into the buffer */
		uint32_t	blen;	/* Space to fill in the buffer */
		uint32_t	temp;

		/* Allocate a temp buffer */
		buf->Buffer = (void *)malloc (len);
		if (buf->Buffer == NULL) {
#if CACHE_DEBUG
			printf ("ERROR: CacheRead: No Memory\n");
#endif
			return (ENOMEM);
		}

		/* Blit the first chunk into the buffer */
		boff = cache->BlockSize - coff;
		blen = len - boff;
#if CACHE_DEBUG
		printf("INFO:  memcpy(%p, %p + %u, %u)\n", buf->Buffer, tag->Buffer, coff, boff);
#endif
		memcpy (buf->Buffer, tag->Buffer + coff, boff);
		
		/* Bump the cache block's reference count */
		tag->Refs++;

		/* Kick the node into the right queue */
		LRUHit (&cache->LRU, (LRUNode_t *)tag, 0);

		/* Next cache block */
		cblk += cache->BlockSize;
		
		/* Read data a cache block at a time */
		while (blen) {
			/* Fetch the next cache block */
			error = CacheLookup (cache, cblk, &tag);
			if (error != EOK) {
				/* Free the allocated buffer */
				free (buf->Buffer);
				buf->Buffer = NULL;

				/* Release all the held tags */
				cblk -= cache->BlockSize;
				while (!boff) {
					if (CacheLookup (cache, cblk, &tag) != EOK) {
						fprintf (stderr, "CacheRead: Unrecoverable error\n");
						exit (-1);
					}
					tag->Refs--;
					
					/* Kick the node into the right queue */
					LRUHit (&cache->LRU, (LRUNode_t *)tag, 0);
				}

				return (error);
			}

			/* Blit the cache block into the buffer */
			temp = ((blen > cache->BlockSize) ? cache->BlockSize : blen);
#if CACHE_DEBUG
			printf ("INFO:  memcpy(%p + %u, %p, %u)\n", buf->Buffer, boff, tag->Buffer, temp);
#endif
			memcpy (buf->Buffer + boff,
			        tag->Buffer,
					temp);

			/* Update counters */
			boff += temp;
			blen -= temp;
			tag->Refs++;

			/* Advance to the next cache block */
			cblk += cache->BlockSize;

			/* Kick the node into the right queue */
			LRUHit (&cache->LRU, (LRUNode_t *)tag, 0);
		}

		/* Count the spanned access */
		cache->Span++;
	}

	/* Attach to head of active buffers list */
	if (cache->ActiveBufs != NULL) {
		buf->Next = cache->ActiveBufs;
		buf->Prev = NULL;

		cache->ActiveBufs->Prev = buf;

	} else {
		cache->ActiveBufs = buf;
	}

	/* Update counters */
	cache->ReqRead++;
	return (EOK);
}

/*
 * XXX
 * All of the uses of kLockWrite need to be audited for
 * when the journal replay is writing.
 */
/* 
 * CacheWrite
 *
 *  Writes a buffer through the cache.
 */
int CacheWrite ( Cache_t *cache, Buf_t *buf, int age, uint32_t writeOptions )
{
	Tag_t *		tag;
	uint32_t	coff = (buf->Offset % cache->BlockSize);
	uint64_t	cblk = (buf->Offset - coff);
	int			error;

	/* Fetch the first cache block */
	error = CacheLookup (cache, cblk, &tag);
	if (error != EOK) return (error);
	
	/* If the buffer was a direct reference */
	if (!(buf->Flags & BUF_SPAN)) {
		/* Commit the dirty block */
		if ( (writeOptions & (kLazyWrite | kLockWrite)) != 0 )
		{
			/* Copy flags to tag */
			tag->Flags |= (writeOptions & (kLazyWrite | kLockWrite));
		}
		else
		{
			error = CacheRawWrite (cache,
								   tag->Offset,
								   cache->BlockSize,
								   tag->Buffer);
			if (error != EOK) return (error);
		}
		
		/* Release the reference */
		if ((writeOptions & kLockWrite) == 0)
			tag->Refs--;

		/* Kick the node into the right queue */
		LRUHit (&cache->LRU, (LRUNode_t *)tag, age);

	/* Otherwise, we do the ugly thing again */
	} else {
		uint32_t	boff;	/* Offset into the buffer */
		uint32_t	blen;	/* Space to fill in the buffer */
		uint32_t	temp;

		/* Blit the first chunk back into the cache */
		boff = cache->BlockSize - coff;
		blen = buf->Length - boff;
		memcpy (tag->Buffer + coff, buf->Buffer, boff);
		
		/* Commit the dirty block */
		if ( (writeOptions & (kLazyWrite | kLockWrite)) != 0 ) 
		{
			/* flag this for lazy write */
			tag->Flags |= (writeOptions & (kLazyWrite | kLockWrite));
		}
		else
		{
			error = CacheRawWrite (cache,
								   tag->Offset,
								   cache->BlockSize,
								   tag->Buffer);
			if (error != EOK) return (error);
		}
		
		/* Release the cache block reference */
		if ((writeOptions & kLockWrite) == 0)
			tag->Refs--;

		/* Kick the node into the right queue */
		LRUHit (&cache->LRU, (LRUNode_t *)tag, age);
			
		/* Next cache block */
		cblk += cache->BlockSize;
		
		/* Write data a cache block at a time */
		while (blen) {
			/* Fetch the next cache block */
			error = CacheLookup (cache, cblk, &tag);
			/* We must go through with the write regardless */

			/* Blit the next buffer chunk back into the cache */
			temp = ((blen > cache->BlockSize) ? cache->BlockSize : blen);
			memcpy (tag->Buffer,
					buf->Buffer + boff,
					temp);

			/* Commit the dirty block */
			if ( (writeOptions & (kLazyWrite | kLockWrite)) != 0 ) 
			{
				/* flag this for lazy write */
				tag->Flags |= (writeOptions & (kLazyWrite | kLockWrite));
			}
			else
			{
				error = CacheRawWrite (cache,
									   tag->Offset,
									   cache->BlockSize,
									   tag->Buffer);
				if (error != EOK) return (error);
			}

			/* Update counters */
			boff += temp;
			blen -= temp;
			if ((writeOptions & kLockWrite) == 0)
				tag->Refs--;

			/* Kick the node into the right queue */
			LRUHit (&cache->LRU, (LRUNode_t *)tag, age);
			/* And go to the next cache block */
			cblk += cache->BlockSize;
		}

		/* Release the anonymous buffer */
		free (buf->Buffer);
	}

	/* Detach the buffer */
	if (buf->Next != NULL)
		buf->Next->Prev = buf->Prev;
	if (buf->Prev != NULL)
		buf->Prev->Next = buf->Next;
	if (cache->ActiveBufs == buf)
		cache->ActiveBufs = buf->Next;

	/* Clear the buffer and put it back on free list */
	memset (buf, 0x00, sizeof (Buf_t));
	buf->Next = cache->FreeBufs; 
	cache->FreeBufs = buf; 		

	/* Update counters */
	cache->ReqWrite++;

	return (EOK);
}

/*
 * CacheRelease
 *
 *  Releases a clean buffer.
 *
 *  NOTE: We don't verify whether it's dirty or not.
 */
int CacheRelease (Cache_t *cache, Buf_t *buf, int age)
{
	Tag_t *		tag;
	uint32_t	coff = (buf->Offset % cache->BlockSize);
	uint64_t	cblk = (buf->Offset - coff);
	int			error;

	/* Fetch the first cache block */
	error = CacheLookup (cache, cblk, &tag);
	if (error != EOK) {
#if CACHE_DEBUG
		printf ("ERROR: CacheRelease: CacheLookup error\n");
#endif
		return (error);
	}
	
	/* If the buffer was a direct reference */
	if (!(buf->Flags & BUF_SPAN)) {
		/* Release the reference */
		if ((tag->Flags & kLockWrite) == 0) {
			tag->Refs--;
		}

		/* Kick the node into the right queue */
		LRUHit (&cache->LRU, (LRUNode_t *)tag, age);

	/* Otherwise, we do the ugly thing again */
	} else {
		uint32_t	blen;	/* Space to fill in the buffer */

		/* Blit the first chunk back into the cache */
		blen = buf->Length - cache->BlockSize + coff;
		
		/* Release the cache block reference */
		if ((tag->Flags & kLockWrite) == 0) {
			tag->Refs--;
		}

		/* Kick the node into the right queue */
		LRUHit (&cache->LRU, (LRUNode_t *)tag, age);

		/* Next cache block */
		cblk += cache->BlockSize;
		
		/* Release cache blocks one at a time */
		while (blen) {
			/* Fetch the next cache block */
			error = CacheLookup (cache, cblk, &tag);
			/* We must go through with the write regardless */

			/* Update counters */
			blen -= ((blen > cache->BlockSize) ? cache->BlockSize : blen);
			if ((tag->Flags & kLockWrite) == 0)
				tag->Refs--;

			/* Kick the node into the right queue */
			LRUHit (&cache->LRU, (LRUNode_t *)tag, age);
			/* Advance to the next block */
			cblk += cache->BlockSize;
		}

		/* Release the anonymous buffer */
		free (buf->Buffer);
	}

	/* Detach the buffer */
	if (buf->Next != NULL)
		buf->Next->Prev = buf->Prev;
	if (buf->Prev != NULL)
		buf->Prev->Next = buf->Next;
	if (cache->ActiveBufs == buf)
		cache->ActiveBufs = buf->Next;

	/* Clear the buffer and put it back on free list */
	memset (buf, 0x00, sizeof (Buf_t));
	buf->Next = cache->FreeBufs; 
	cache->FreeBufs = buf; 		

	return (EOK);
}

/*
 * CacheRemove
 *
 *  Disposes of a particular buffer.
 */
int CacheRemove (Cache_t *cache, Tag_t *tag)
{
	int			error;

	/* Make sure it's not busy */
	if (tag->Refs) return (EBUSY);
	
	/* Detach the tag */
	if (tag->Next != NULL)
		tag->Next->Prev = tag->Prev;
	if (tag->Prev != NULL)
		tag->Prev->Next = tag->Next;
	else
		cache->Hash[tag->Offset % cache->HashSize] = tag->Next;
	
	/* Make sure the head node doesn't have a back pointer */
	if ((cache->Hash[tag->Offset % cache->HashSize] != NULL) &&
	    (cache->Hash[tag->Offset % cache->HashSize]->Prev != NULL)) {
#if CACHE_DEBUG
		printf ("ERROR: CacheRemove: Corrupt hash chain\n");
#endif
	}
	
	/* Release it's buffer (if it has one) */
	if (tag->Buffer != NULL)
	{
		error = CacheFreeBlock (cache, tag);
		if ( EOK != error )
			return( error );
	}

	/* Zero the tag (for easy debugging) */
	memset (tag, 0x00, sizeof (Tag_t));

	/* Free the tag */
	free (tag);

	return (EOK);
}

/*
 * CacheEvict
 *
 *  Only dispose of the buffer, leave the tag intact.
 */
int CacheEvict (Cache_t *cache, Tag_t *tag)
{
	int			error;

	/* Make sure it's not busy */
	if (tag->Refs) return (EBUSY);

	/* Release the buffer */
	if (tag->Buffer != NULL)
	{
		error = CacheFreeBlock (cache, tag);
		if ( EOK != error )
			return( error );
	}
	tag->Buffer = NULL;

	return (EOK);
}

/*
 * CacheAllocBlock
 *
 *  Allocate an unused cache block.
 */
void *CacheAllocBlock (Cache_t *cache)
{
	void *	temp;
	
	if (cache->FreeHead == NULL)
		return (NULL);
	if (cache->FreeSize == 0)
		return (NULL);

	temp = cache->FreeHead;
	cache->FreeHead = *((void **)cache->FreeHead);
	cache->FreeSize--;

	return (temp);
}

/*
 * CacheFreeBlock
 *
 *  Release an active cache block.
 */
static int 
CacheFreeBlock( Cache_t *cache, Tag_t *tag )
{
	int			error;
	
	if ( (tag->Flags & kLazyWrite) != 0 )
	{
		/* this cache block has been marked for lazy write - do it now */
		error = CacheRawWrite( cache,
							   tag->Offset,
							   cache->BlockSize,
							   tag->Buffer );
		if ( EOK != error ) 
		{
#if CACHE_DEBUG
			printf( "%s - CacheRawWrite failed with error %d \n", __FUNCTION__, error );
#endif 
			return ( error );
		}
		tag->Flags &= ~kLazyWrite;
	}

	if ((tag->Flags & kLockWrite) == 0)
	{
		*((void **)tag->Buffer) = cache->FreeHead;
		cache->FreeHead = (void **)tag->Buffer;
		cache->FreeSize++;
	}
	return( EOK );
}


/*
 * CacheFlush
 *
 *  Write out any blocks that are marked for lazy write.
 */
int 
CacheFlush( Cache_t *cache )
{
	int			error;
	int			i;
	Tag_t *		myTagPtr;
	
	for ( i = 0; i < cache->HashSize; i++ )
	{
		myTagPtr = cache->Hash[ i ];
		
		while ( NULL != myTagPtr )
		{
			if ( (myTagPtr->Flags & kLazyWrite) != 0 )
			{
				/* this cache block has been marked for lazy write - do it now */
				error = CacheRawWrite( cache,
									   myTagPtr->Offset,
									   cache->BlockSize,
									   myTagPtr->Buffer );
				if ( EOK != error ) 
				{
#if CACHE_DEBUG
					printf( "%s - CacheRawWrite failed with error %d \n", __FUNCTION__, error );
#endif 
					return( error );
				}
				myTagPtr->Flags &= ~kLazyWrite;
			}
			myTagPtr = myTagPtr->Next; 
		} /* while */
	} /* for */

	return( EOK );
		
} /* CacheFlush */


/*
 * RangeIntersect
 *
 * Return true if the two given ranges intersect.
 *
 */
static int
RangeIntersect(uint64_t start1, uint64_t len1, uint64_t start2, uint64_t len2)
{
	uint64_t end1 = start1 + len1 - 1;
	uint64_t end2 = start2 + len2 - 1;
	
	if (end1 < start2 || start1 > end2)
		return 0;
	else
		return 1;
}


/*
 * CacheFlushRange
 *
 * Flush, and optionally remove, all cache blocks that intersect
 * a given range.
 */
static int
CacheFlushRange( Cache_t *cache, uint64_t start, uint64_t len, int remove)
{
	int error;
	int i;
	Tag_t *currentTag, *nextTag;
	
	for ( i = 0; i < cache->HashSize; i++ )
	{
		currentTag = cache->Hash[ i ];
		
		while ( NULL != currentTag )
		{
			/* Keep track of the next block, in case we remove the current block */
			nextTag = currentTag->Next;

			if ( currentTag->Flags & kLazyWrite &&
				 RangeIntersect(currentTag->Offset, cache->BlockSize, start, len))
			{
				error = CacheRawWrite( cache,
									   currentTag->Offset,
									   cache->BlockSize,
									   currentTag->Buffer );
				if ( EOK != error )
				{
#if CACHE_DEBUG
					printf( "%s - CacheRawWrite failed with error %d \n", __FUNCTION__, error );
#endif 
					return error;
				}
				currentTag->Flags &= ~kLazyWrite;

				if ( remove && ((currentTag->Flags & kLockWrite) == 0))
					CacheRemove( cache, currentTag );
			}
			
			currentTag = nextTag;
		} /* while */
	} /* for */
	
	return EOK;
} /* CacheFlushRange */

/* Function: CacheCopyDiskBlocks
 *
 * Description: Perform direct disk block copy from from_offset to to_offset
 * of given length. 
 *
 * The function flushes the cache blocks intersecting with disk blocks
 * belonging to from_offset.  Invalidating the disk blocks belonging to 
 * to_offset from the cache would have been sufficient, but its block 
 * start and end might not lie on cache block size boundary.  Therefore we 
 * flush the disk blocks belonging to to_offset on the disk .
 *
 * The function performs raw read and write on the disk of cache block size,
 * with exception of last operation.
 *
 * Note that the data written to disk does not exist in cache after
 * this function.  This function however ensures that if the device
 * offset being read/written on disk existed in cache, it is invalidated and
 * written to disk before performing any read/write operation.
 *
 * Input:
 *	1. cache - pointer to cache.
 *	2. from_offset - disk offset to copy from.
 *	3. to_offset - disk offset to copy to.
 *	4. len - length in bytes to be copied.  Note that this length should be
 * 		a multiple of disk block size, else read/write will return error.
 *
 * Output:
 *	zero (EOK) on success.
 *	On failure, non-zero value.
 * 	Known error values:
 *		ENOMEM - insufficient memory to allocate intermediate copy buffer.
 * 		EINVAL - the length of data to read/write is not multiple of 
 *				 device block size, or
 *				 the device offset is not multiple of device block size, or
 *		ENXIO  - invalid disk offset
 */
int CacheCopyDiskBlocks (Cache_t *cache, uint64_t from_offset, uint64_t to_offset, uint32_t len) 
{
	int i;
	int error;
	char *tmpBuffer = NULL;
	uint32_t ioReqCount;
	uint32_t numberOfBuffersToWrite;

	/* Return error if length of data to be written on disk is
	 * less than the length of the buffer to be written, or
	 * disk offsets are not multiple of device block size
	 */
	if ((len % cache->DevBlockSize) || 
		(from_offset % cache->DevBlockSize) ||
		(to_offset % cache->DevBlockSize)) {
		error = EINVAL;
		goto out;
	}

	/* Flush contents of from_offset on the disk */
	error = CacheFlushRange(cache, from_offset, len, 1);
	if (error != EOK) goto out;

	/* Flush contents of to_offset on the disk */
	error = CacheFlushRange(cache, to_offset, len, 1);
	if (error != EOK) goto out;
	
	/* Allocate temporary buffer for reading/writing, currently
	 * set to block size of cache. 
	 */
	tmpBuffer = malloc(cache->BlockSize);
	if (!tmpBuffer) {
#if CACHE_DEBUG
		printf("%s(%d):  malloc(%zd) failed\n", __FUNCTION__, __LINE__, (size_t)cache->BlockSize);
#endif
		error = ENOMEM;
		goto out;
	}
	
	ioReqCount = cache->BlockSize;
	numberOfBuffersToWrite = (len + ioReqCount - 1) / ioReqCount;

	for (i=0; i<numberOfBuffersToWrite; i++) {
		if (i == (numberOfBuffersToWrite - 1)) {
			/* last buffer */	
			ioReqCount = len - (i * cache->BlockSize);
		}
		
		/* Read data */
		error = CacheRawRead (cache, from_offset, ioReqCount, tmpBuffer);
		if (error != EOK) goto out;

		/* Write data */
		error = CacheRawWrite (cache, to_offset, ioReqCount, tmpBuffer);
		if (error != EOK) goto out;

#if 0
		printf ("%s: Copying %d bytes from %qd to %qd\n", __FUNCTION__, ioReqCount, from_offset, to_offset);
#endif

		/* Increment offsets with data read/written */
		from_offset += ioReqCount;
		to_offset += ioReqCount;
	}

out:
	if (tmpBuffer) {
		free (tmpBuffer);
	}
	return error;
}

/* Function: CacheWriteBufferToDisk
 *
 * Description: Write data on disk starting at given offset for upto write_len.
 * The data from given buffer upto buf_len is written to the disk starting
 * at given offset.  If the amount of data written on disk is greater than 
 * the length of buffer, all the remaining data is written as zeros.
 * 
 * If no buffer is provided or if length of buffer is zero, the function
 * writes zeros on disk from offset upto write_len bytes.
 * 
 * The function requires the length of buffer is either equal to or less
 * than the data to be written on disk.  It also requires that the length
 * of data to be written on disk is a multiple of device block size.
 *
 * Note that the data written to disk does not exist in cache after
 * this function.  This function however ensures that if the device
 * offset being written on disk existed in cache, it is invalidated and
 * written to disk before performing any read/write operation.
 *
 * Input:
 *	1. cache - pointer to cache
 *	2. offset - offset on disk to write data of buffer
 *	3. buffer - pointer to data to be written on disk
 *	4. len - length of buffer to be written on disk.
 *
 * Output:
 *	zero (EOK) on success.
 *	On failure, non-zero value.
 * 	Known error values:
 *		ENOMEM - insufficient memory to allocate intermediate copy buffer.
 *		EINVAL - the length of data to read/write is not multiple of 
 *				 device block size, or
 *				 the device offset is not multiple of device block size, or
 *				 the length of data to be written on disk is less than
 *				 the length of buffer.
 *		ENXIO  - invalid disk offset
 */
int CacheWriteBufferToDisk (Cache_t *cache, uint64_t offset, uint32_t write_len, u_char *buffer, uint32_t buf_len)
{
	int error;
	u_char *write_buffer = NULL;
	uint32_t io_count;
	uint32_t buf_offset;
	uint32_t bytes_remain;
	uint8_t zero_fill = false;

	/* Check if buffer is provided */
	if (buffer == NULL) {
		buf_len = 0;
	}

	/* Return error if length of data to be written on disk is,
	 * less than the length of the buffer to be written, or
	 * is not a multiple of device block size, or offset to write 
	 * is not multiple of device block size
	 */
	if ((write_len % cache->DevBlockSize) || 
		(offset % cache->DevBlockSize) ||
		(write_len < buf_len)) {
		error = EINVAL;
		goto out;
	}

	/* Flush cache contents of offset range to be written on the disk */
	error = CacheFlushRange(cache, offset, write_len, 1);
	if (error != EOK) {
		goto out;
	}

	/* Calculate correct size of buffer to be written each time */
	io_count = (write_len < cache->BlockSize) ? write_len : cache->BlockSize;

	/* Allocate temporary buffer to write data to disk */
	write_buffer = malloc (io_count);
	if (!write_buffer) {
#if CACHE_DEBUG
		printf("%s(%d):  malloc(%zd) failed\n", __FUNCTION__, __LINE__, (size_t)cache->BlockSize);
#endif
		error = ENOMEM;
		goto out;
	}

	/* Read offset in data buffer to be written to disk */
	buf_offset = 0;

	while (write_len) {
		/* The last buffer might be less than io_count bytes */
		if (write_len < io_count) {
			io_count = write_len;
		}
			
		/* Check whether data buffer was written completely to disk */
		if (buf_offset < buf_len) {
			/* Calculate the bytes from data buffer still to be written */
			bytes_remain = buf_len - buf_offset;

			if (bytes_remain >= io_count) {
				/* Bytes remaining is greater than bytes written in one 
				 * IO request.  Limit bytes read from data buffer in this 
				 * pass to the bytes written in one IO request 
				 */
				bytes_remain = io_count;

				/* Copy data from data buffer to write buffer */
				memcpy (write_buffer, buffer, bytes_remain);
			} else {
				/* Bytes remaining is less than bytes written in one 
				 * IO request.  Zero fill the remaining write buffer.
				 */

				/* Copy data from data buffer to write buffer */
				memcpy (write_buffer, buffer, bytes_remain);

				/* Zero fill remain buffer, if any */
				memset (write_buffer + bytes_remain, 0, io_count - bytes_remain);
			}

			buf_offset += bytes_remain;
			buffer += bytes_remain;
		} else {
			/* Do not zero fill the buffer if we have already done it */
			if (zero_fill == false) {
				/* Zero fill entire write buffer */
				memset (write_buffer, 0, io_count);
				zero_fill = true;
			}
		}

		/* Write data */
		error = CacheRawWrite (cache, offset, io_count, write_buffer); 
		if (error != EOK) goto out;

		offset += io_count;
		write_len -= io_count;
	}

out:
	/* If we allocated a temporary buffer, deallocate it */
	if (write_buffer != NULL) {
		free (write_buffer);
	}
	return error;
}

/*
 * CacheLookup
 *
 *  Obtain a cache block. If one already exists, it is returned. Otherwise a
 *  new one is created and inserted into the cache.
 */
int CacheLookup (Cache_t *cache, uint64_t off, Tag_t **tag)
{
	Tag_t *		temp;
	uint32_t	hash = off % cache->HashSize;
	int			error;

	*tag = NULL;
	
	/* Search the hash table */
	error = 0;
	temp = cache->Hash[hash];
	while (temp != NULL) {
		if (temp->Offset == off) break;
		temp = temp->Next;
	}

	/* If it's a hit */
	if (temp != NULL) {
		/* Perform MTF if necessary */
		if (cache->Hash[hash] != temp) {
			/* Disconnect the tag */
			if (temp->Next != NULL)
				temp->Next->Prev = temp->Prev;
			temp->Prev->Next = temp->Next;
		}
		
	/* Otherwise, it's a miss */
	} else {
		/* Allocate a new tag */
		temp = (Tag_t *)calloc (sizeof (Tag_t), 1);/* We really only need to zero the
													 LRU portion though */
		temp->Offset = off;

		/* Kick the tag onto the LRU */
		//LRUHit (&cache->LRU, (LRUNode_t *)temp, 0);
	}

	/* Insert at the head (if it's not already there) */
	if (cache->Hash[hash] != temp) {
		temp->Prev = NULL;
		temp->Next = cache->Hash[hash];
		if (temp->Next != NULL)
			temp->Next->Prev = temp;
		cache->Hash[hash] = temp;
	}

	/* Make sure there's a buffer */
	if (temp->Buffer == NULL) {
		/* Find a free buffer */
		temp->Buffer = CacheAllocBlock (cache);
		if (temp->Buffer == NULL) {
			/* Try to evict a buffer */
			error = LRUEvict (&cache->LRU, (LRUNode_t *)temp);
			if (error != EOK) return (error);

			/* Try again */
			temp->Buffer = CacheAllocBlock (cache);
			if (temp->Buffer == NULL) {
#if CACHE_DEBUG
				printf("%s(%d):  CacheAllocBlock failed (FreeHead = %p, FreeSize = %u)\n", __FUNCTION__, __LINE__, cache->FreeHead, cache->FreeSize);
#endif
				return (ENOMEM);
			}
		}

		/* Load the block from disk */
		error = CacheRawRead (cache, off, cache->BlockSize, temp->Buffer);
		if (error != EOK) return (error);
	}

#if 0
	if (temp && temp->Flags & kLockWrite) {
		fprintf(stderr, "CacheLookup(%p, %llu, %p):  Found cache-locked block\n", cache, off, tag);
	}
#endif

	*tag = temp;	
	return (EOK);
}

/*
 * CacheRawRead
 *
 *  Perform a direct read on the file.
 */
int CacheRawRead (Cache_t *cache, uint64_t off, uint32_t len, void *buf)
{
	uint64_t	result;
	ssize_t		nread;
		
	/* Both offset and length must be multiples of the device block size */
	if (off % cache->DevBlockSize) return (EINVAL);
	if (len % cache->DevBlockSize) return (EINVAL);
	
	/* Seek to the position */
	errno = 0;
	result = lseek (cache->FD_R, off, SEEK_SET);
	if (result == (off_t)-1 && errno != 0)
		return errno;
	if (result != off) return (ENXIO);
	/* Read into the buffer */
#if CACHE_DEBUG
	printf("%s:  offset %llu, len %u\n", __FUNCTION__, off, len);
#endif
	nread = read (cache->FD_R, buf, len);
	if (nread == -1) return (errno);
	if (nread == 0) return (ENXIO);

	/* Update counters */
	cache->DiskRead++;
	
	return (EOK);
}

/*
 * CacheRawWrite
 *
 *  Perform a direct write on the file.
 */
int CacheRawWrite (Cache_t *cache, uint64_t off, uint32_t len, void *buf)
{
	uint64_t	result;
	ssize_t		nwritten;
	
	/* Both offset and length must be multiples of the device block size */
	if (off % cache->DevBlockSize) return (EINVAL);
	if (len % cache->DevBlockSize) return (EINVAL);
	
	/* Seek to the position */
	errno = 0;
	result = lseek (cache->FD_W, off, SEEK_SET);
	if (result == (off_t)-1 && errno != 0) return (errno);
	if (result != off) return (ENXIO);
	
	/* Write into the buffer */
	nwritten = write (cache->FD_W, buf, len);
	if (nwritten == -1) return (errno);
	if (nwritten == 0) return (ENXIO);
	
	/* Update counters */
	cache->DiskWrite++;
	
	return (EOK);
}



/*
 * LRUInit
 *
 *  Initializes the LRU data structures.
 */
static int LRUInit (LRU_t *lru)
{
	/* Make the dummy nodes point to themselves */
	lru->Head.Next = &lru->Head;
	lru->Head.Prev = &lru->Head;

	lru->Busy.Next = &lru->Busy;
	lru->Busy.Prev = &lru->Busy;

	return (EOK);
}

/*
 * LRUDestroy
 *
 *  Shutdown the LRU.
 *
 *  NOTE: This is typically a no-op, since the cache manager will be clearing
 *        all cache tags.
 */
static int LRUDestroy (LRU_t *lru)
{
	/* Nothing to do */
	return (EOK);
}

/*
 * LRUHit
 *
 *  Registers data activity on the given node. If the node is already in the
 *  LRU, it is moved to the front. Otherwise, it is inserted at the front.
 *
 *  NOTE: If the node is not in the LRU, we assume that its pointers are NULL.
 */
static int LRUHit (LRU_t *lru, LRUNode_t *node, int age)
{
	/* Handle existing nodes */
	if ((node->Next != NULL) && (node->Prev != NULL)) {
		/* Detach the node */
		node->Next->Prev = node->Prev;
		node->Prev->Next = node->Next;
	}

	/* If it's busy (we can't evict it) */
	if (((Tag_t *)node)->Refs) {
		/* Insert at the head of the Busy queue */
		node->Next = lru->Busy.Next;
		node->Prev = &lru->Busy;

	} else if (age) {
		/* Insert at the head of the LRU */
		node->Next = &lru->Head;
		node->Prev = lru->Head.Prev;
		
	} else {
		/* Insert at the head of the LRU */
		node->Next = lru->Head.Next;
		node->Prev = &lru->Head;
	}

	node->Next->Prev = node;
	node->Prev->Next = node;

	return (EOK);
}

/*
 * LRUEvict
 *
 *  Chooses a buffer to release.
 *
 *  TODO: Under extreme conditions, it shoud be possible to release the buffer
 *        of an actively referenced cache buffer, leaving the tag behind as a
 *        placeholder. This would be required for implementing 2Q-LRU
 *        replacement.
 *
 *  NOTE: Make sure we never evict the node we're trying to find a buffer for!
 */
static int LRUEvict (LRU_t *lru, LRUNode_t *node)
{
	LRUNode_t *	temp;

	/* Find a victim */
	while (1) {
		/* Grab the tail */
		temp = lru->Head.Prev;
		
		/* Stop if we're empty */
		if (temp == &lru->Head) {
#if CACHE_DEBUG
			printf("%s(%d):  empty?\n", __FUNCTION__, __LINE__);
#endif
			return (ENOMEM);
		}

		/* Detach the tail */
		temp->Next->Prev = temp->Prev;
		temp->Prev->Next = temp->Next;

		/* If it's not busy, we have a victim */
		if (!((Tag_t *)temp)->Refs) break;

		/* Insert at the head of the Busy queue */
		temp->Next = lru->Busy.Next;
		temp->Prev = &lru->Busy;
				
		temp->Next->Prev = temp;
		temp->Prev->Next = temp;

		/* Try again */
	}

	/* Remove the tag */
	CacheRemove ((Cache_t *)lru, (Tag_t *)temp);

	return (EOK);
}

/*
 * Dump the cache contents.
 * If nobody else calls it, it gets optimized out.  Annoying and yet
 * useful.
 */
void
dumpCache(Cache_t *cache)
{
	int i;
	int numEntries = 0;

	printf("Cache:\n");
	printf("\tDevBlockSize = %u\n", cache->DevBlockSize);
	printf("\tCache Block Size = %u\n", cache->BlockSize);
	printf("\tHash Size = %u\n", cache->HashSize);
	printf("\tHash Table:\n");
	for (i = 0; i < cache->HashSize; i++) {
		Tag_t *tag;

		for (tag = cache->Hash[i]; tag; tag = tag->Next) {
			numEntries++;
			printf("\t\tOffset %llu, refs %u, Flags %#x (%skLazyWrite, %skLockWrite)\n",
			       tag->Offset, tag->Refs, tag->Flags,
			       (tag->Flags & kLazyWrite) ? "" : "no ",
			       (tag->Flags & kLockWrite) ? "" : "no ");
		}
	}
	printf("\tNumber of entries: %u\n", numEntries);
	return;
}

