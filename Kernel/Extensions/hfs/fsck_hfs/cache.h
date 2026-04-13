/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 * User Land Cache Manager
 *
 *  A user land cache manager.
 */
#ifndef _CACHE_H_
#define _CACHE_H_
#include <stdint.h>

/* Different values for initializing cache */
enum {
	/* Default sizes */
	DefaultCacheBlockSize	=	0x8000,		/* 32K */
	DefaultCacheBlocks		=	1024,
	DefaultCacheSize		=	(DefaultCacheBlockSize * DefaultCacheBlocks),  /* 32MBytes */

	/* Minimum allowed sizes */
	MinCacheBlockSize		=	0x8000,		/* 32K */
	MinCacheBlocks			=	1024,
	MinCacheSize			=	(MinCacheBlockSize * MinCacheBlocks), 	/* 32MBytes */

	/* Maximum allowed sizes */
	MaxCacheBlockSize		=	0x8000,		/* 32K */
#ifdef __LP64__
	MaxCacheBlocks			= 	0x18000,
#else
	MaxCacheBlocks			=	0x8000,
#endif
	/* MaxCacheSize will be 3G for 64-bit, and 1G for 32-bit */
	MaxCacheSize			=	((unsigned)MaxCacheBlockSize * MaxCacheBlocks),
	CacheHashSize			=	257,		/* prime number */
};

/*
 * Some nice lowercase shortcuts.
 */
#define EOK					0

#define BUF_SPAN	0x80000000	/* Buffer spans several cache blocks */

typedef struct LRUNode_t
{
	struct LRUNode_t *	Next;	/* Next node in the LRU */
	struct LRUNode_t *	Prev;	/* Previous node in the LRU */
} LRUNode_t;

typedef struct LRU_t
{
	LRUNode_t			Head;	/* Dummy node for the head of the LRU */
	LRUNode_t			Busy;	/* List of busy nodes */
} LRU_t;


#define MAXBUFS  48
/*
 * Buf_t
 *
 *  Buffer structure exchanged between the cache and client. It contains the
 *  data buffer with the requested data, as well as housekeeping information
 *  that the cache needs.
 */
typedef struct Buf_t
{
	struct Buf_t *	Next;	/* Next active buffer */
	struct Buf_t *	Prev;	/* Previous active buffer */
	
	uint32_t		Flags;	/* Buffer flags */
	uint64_t		Offset;	/* Start offset of the buffer */
	uint32_t		Length;	/* Size of the buffer in bytes */

	void *			Buffer;	/* Buffer */
} Buf_t;

/*
 * Tag_t
 *
 *  The cache tag structure is a header for a cache buffer. It contains a
 *  pointer to the cache block and housekeeping information. The type of LRU
 *  algorithm can be swapped out easily.
 *
 *  NOTE: The LRU field must be the first field, so we can easily cast between
 *        the two.
 */
typedef struct Tag_t
{
	LRUNode_t		LRU;	/* LRU specific data, must be first! */
	
	struct Tag_t *	Next;	/* Next tag in hash chain */
	struct Tag_t *	Prev;	/* Previous tag in hash chain */

	uint32_t		Flags;	
	uint32_t		Refs;	/* Reference count */
	uint64_t		Offset;	/* Offset of the buffer */
	
	void *			Buffer;	/* Cache page */
} Tag_t;


/* Tag_t.Flags bit settings */
enum {
	kLazyWrite		 = 0x00000001, 	/* only write this page when evicting or forced */
	kLockWrite		 = 0x00000002,  /* Never evict this page -- will not work with writing yet! */
};

/*
 * Cache_t
 *
 *  The main cache data structure. The cache manages access between an open
 *  file and the cache client program.
 *
 *  NOTE: The LRU field must be the first field, so we can easily cast between
 *        the two.
 */
typedef struct Cache_t
{
	LRU_t		LRU;		/* LRU replacement data structure */
	
	int		FD_R;		/* File descriptor (read-only) */
	int		FD_W;		/* File descriptor (write-only) */
	uint32_t	DevBlockSize;	/* Device block size */
	
	Tag_t **	Hash;		/* Lookup hash table (move to front) */
	uint32_t	HashSize;	/* Size of the hash table */
	uint32_t	BlockSize;	/* Size of the cache page */

	void *		FreeHead;	/* Head of the free list */
	uint32_t	FreeSize;	/* Size of the free list */

	Buf_t *		ActiveBufs;	/* List of active buffers */
	Buf_t *		FreeBufs;	/* List of free buffers */

	uint32_t	ReqRead;	/* Number of read requests */
	uint32_t	ReqWrite;	/* Number of write requests */
	
	uint32_t	DiskRead;	/* Number of actual disk reads */
	uint32_t	DiskWrite;	/* Number of actual disk writes */

	uint32_t	Span;		/* Requests that spanned cache blocks */
} Cache_t;

extern Cache_t fscache;

/*
 * CalculateCacheSizes
 *
 * Determine the cache size values (block size and total blocks) that should
 * be used to initialize the cache.
 */
void CalculateCacheSizes(uint64_t userCacheSize, uint32_t *calcBlockSize, uint32_t *calcTotalBlocks, 
					   char debug);
/*
 * CacheInit
 *
 *  Initializes the cache for use.
 */
int CacheInit (Cache_t *cache, int fdRead, int fdWrite, uint32_t devBlockSize,
               uint32_t cacheBlockSize, uint32_t cacheSize, uint32_t hashSize,
               int preTouch);

/*
 * CacheDestroy
 * 
 *  Shutdown the cache.
 */
int CacheDestroy (Cache_t *cache);

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
int CacheRead (Cache_t *cache, uint64_t start, uint32_t len, Buf_t **buf);

/* 
 * CacheWrite
 *
 *  Writes a buffer through the cache.
 */
int CacheWrite ( Cache_t *cache, Buf_t *buf, int age, uint32_t writeOptions );

/*
 * CacheRelease
 *
 *  Releases a clean buffer.
 *
 *  NOTE: We don't verify whether it's dirty or not.
 */
int CacheRelease (Cache_t *cache, Buf_t *buf, int age);

/* CacheRemove
 *
 *  Disposes of a particular tag and buffer.
 */
int CacheRemove (Cache_t *cache, Tag_t *tag);

/*
 * CacheEvict
 *
 *  Only dispose of the buffer, leave the tag intact.
 */
int CacheEvict (Cache_t *cache, Tag_t *tag);

/*
 * CacheFlush
 *
 *  Write out any blocks that are marked for lazy write.
 */
int 
CacheFlush( Cache_t *cache );

/* CacheCopyDiskBlocks 
 *
 * Perform direct disk block copy from from_offset to to_offset of given length. 
 */
int CacheCopyDiskBlocks (Cache_t *cache, uint64_t from_offset, uint64_t to_offset, uint32_t len);

/* CacheWriteBufferToDisk 
 *
 * Write data on disk starting at given offset for upto write_len.
 * The data from given buffer upto buf_len is written to the disk starting
 * at given offset.  If the amount of data written on disk is greater than 
 * the length of buffer, all the remaining data is written as zeros.
 * 
 * If no buffer is provided or if length of buffer is zero, the function
 * writes zeros on disk from offset upto write_len bytes.
 */
int CacheWriteBufferToDisk (Cache_t *cache, uint64_t offset, uint32_t write_len, u_char *buffer, uint32_t buf_len);
#endif

