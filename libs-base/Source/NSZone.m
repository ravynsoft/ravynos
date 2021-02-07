/** Zone memory management. -*- Mode: ObjC -*-
   Copyright (C) 1997,1998 Free Software Foundation, Inc.

   Written by: Yoo C. Chung <wacko@laplace.snu.ac.kr>
   Date: January 1997
   Rewrite by: Richard Frith-Macdonald <richard@brainstrom.co.uk>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   <title>NSZone class reference</title>
   $Date$ $Revision$
*/

/*  Design goals:

    - Allocation and deallocation should be reasonably efficient.
    - We want to catch code that writes outside it's permitted area.

 */


/* Actual design:

   - The default zone uses malloc() and friends.  We assume that
   they're thread safe and that they return NULL if we're out of
   memory (glibc malloc does this, what about other mallocs? FIXME).

    - The OpenStep spec says that when a zone is recycled, any memory in
   use is returned to the default zone.
   Since, in general, we have no control over the system malloc, we can't
   possibly do this.  Instead, we move the recycled zone to a list of
   'dead' zones, and as soon as all memory used in it is released, we
   destroy it and remove it from that list.  In the meantime, we release
   any blocks of memory we can (ie those that don't contain unfreed chunks).

   - For freeable zones, a small linear buffer is used for
   deallocating and allocating.  Anything that can't go into the
   buffer then uses a more general purpose segregated fit algorithm
   after flushing the buffer.

   - For memory chunks in freeable zones, the pointer to the chunk is
   preceded by the a chunk header which contains the size of the chunk
   (plus a couple of flags) and a pointer to the end of the memory
   requested.  This adds 8 bytes for freeable zones, which is usually
   what we need for alignment purposes anyway (assuming we're on a
   32 bit machine).  The granularity for allocation of chunks is quite
   large - a chunk must be big enough to hold the chunk header plus a
   couple of pointers and an unsigned size value.
   The actual memory allocated will be the size of the chunk header plus
   the size of memory requested plus one (a guard byte), all rounded up
   to a multiple of the granularity.

   - For nonfreeable zones, worst-like fit is used.  This is OK since
   we don't have to worry about memory fragmentation. */

/* Other information:

   - This uses some GCC specific extensions.  But since the library is
   supposed to compile on GCC 2.7.2.1 (patched) or higher, and the
   only other Objective-C compiler I know of (other than NeXT's, which
   is based on GCC as far as I know) is the StepStone compiler, which
   I haven't the foggiest idea why anyone would prefer it to GCC ;),
   it should be OK.

   - These functions should be thread safe, but I haven't really
   tested them extensively in multithreaded cases. */


/* Define to turn off NSAssertions. */
#define NS_BLOCK_ASSERTIONS 1

#define IN_NSZONE_M 1

#import "common.h"
#include <stddef.h>
#import "Foundation/NSException.h"
#import "Foundation/NSLock.h"
#import "GSPrivate.h"
#import "GSPThread.h"

static pthread_mutex_t  zoneLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Try to get more memory - the normal process has failed.
 * If we can't do anything, just return a null pointer.
 * Try to do some logging if possible.
 */
void *
GSOutOfMemory(NSUInteger size, BOOL retry)
{
  fprintf(stderr, "GSOutOfMemory ... wanting %"PRIuPTR" bytes.\n", size);
  return 0;
}

/* Default zone functions for default zone. */
static void* default_malloc (NSZone *zone, size_t size);
static void* default_realloc (NSZone *zone, void *ptr, size_t size);
static void default_free (NSZone *zone, void *ptr);
static void default_recycle (NSZone *zone);
static BOOL default_check (NSZone *zone);
static BOOL default_lookup (NSZone *zone, void *ptr);
static struct NSZoneStats default_stats (NSZone *zone);

static void*
default_malloc (NSZone *zone, size_t size)
{
  void *mem;

  mem = malloc(size);
  if (mem != NULL)
    {
      return mem;
    }
  [NSException raise: NSMallocException
              format: @"Default zone has run out of memory"];
  return 0;
}

static void*
default_realloc (NSZone *zone, void *ptr, size_t size)
{
  void *mem;

  mem = realloc(ptr, size);
  if (mem != NULL)
    {
      return mem;
    }
  [NSException raise: NSMallocException
              format: @"Default zone has run out of memory"];
  return 0;
}

static void
default_free (NSZone *zone, void *ptr)
{
  free(ptr);
}

static void
default_recycle (NSZone *zone)
{
  /* Recycle the default zone?  Thou hast got to be kiddin'. */
  [NSException raise: NSGenericException
              format: @"Trying to recycle default zone"];
}

static BOOL
default_check (NSZone *zone)
{
  /* We can't check memory managed by malloc(). */
  [NSException raise: NSGenericException
	      format: @"No checking for default zone"];
  return NO;
}

static BOOL
default_lookup (NSZone *zone, void *ptr)
{
  /* Assume all memory is in default zone. */
  return YES;
}

static struct NSZoneStats
default_stats (NSZone *zone)
{
  struct NSZoneStats dummy = {0,0,0,0,0};

  /* We can't obtain statistics from the memory managed by malloc(). */
  [NSException raise: NSGenericException
	      format: @"No statistics for default zone"];
  return dummy;
}

static NSZone default_zone =
{
  default_malloc, default_realloc, default_free, default_recycle,
  default_check, default_lookup, default_stats, 0, @"default", 0
};

/*
 * For backward compatibility.
 */
NSZone	*__nszone_private_hidden_default_zone = &default_zone;



void
NSSetZoneName (NSZone *zone, NSString *name)
{
  if (!zone)
    zone = NSDefaultMallocZone();
  pthread_mutex_lock(&zoneLock);
  name = [name copy];
  if (zone->name != nil)
    [zone->name release];
  zone->name = name;
  pthread_mutex_unlock(&zoneLock);
}

NSString*
NSZoneName (NSZone *zone)
{
  if (!zone)
    zone = NSDefaultMallocZone();
  return zone->name;
}

/* Alignment */
#ifdef ALIGN
#undef ALIGN
#endif
#define ALIGN ((__alignof__(double) < 8) ? 8 : __alignof__(double))
#define MINGRAN 256 /* Minimum granularity. */
#define DEFBLOCK 16384 /* Default granularity. */
#define BUFFER 4 /* Buffer size.  FIXME?: Is this a reasonable optimum. */
#define MAX_SEG 16 /* Segregated list size. */
#define FBSZ sizeof(ff_block)
#define NBSZ sizeof(nf_chunk)

/* Information bits in size. */
#define INUSE 0x01 /* Current chunk in use. */
#define PREVUSE 0x02 /* Previous chunk in use. */
#define LIVE 0x04

/* Bits to mask off to get size. */
#define SIZE_BITS (INUSE | PREVUSE | LIVE)

#define	NF_HEAD sizeof(nf_block)

typedef struct _ffree_free_link ff_link;
typedef struct _nfree_block_struct nf_block;
typedef struct _ffree_block_struct ff_block;
typedef struct _ffree_zone_struct ffree_zone;
typedef struct _nfree_zone_struct nfree_zone;


/* Header for blocks in nonfreeable zones. */
struct _nfree_block_unpadded
{
  struct _nfree_block_struct *next;
  size_t size; // Size of block
  size_t top; // Position of next memory chunk to allocate
};
#define	NFBPAD	sizeof(struct _nfree_block_unpadded)

struct _nfree_block_struct
{
  struct _nfree_block_struct *next;
  size_t size; // Size of block
  size_t top; // Position of next memory chunk to allocate
  char	padding[ALIGN - ((NFBPAD % ALIGN) ? (NFBPAD % ALIGN) : ALIGN)];
};

struct _ffree_block_unpadded {
  size_t	size;
  struct _ffree_block_struct *next;
};
#define	FFCPAD	sizeof(struct _ffree_block_unpadded)

/* Header for blocks and chunks in freeable zones. */
struct _ffree_block_struct
{
  size_t size;
  struct _ffree_block_struct *next;
  char	padding[ALIGN - ((FFCPAD % ALIGN) ? (FFCPAD % ALIGN) : ALIGN)];
};

struct _ffree_free_link_unpadded
{
  size_t	size;
  ff_link	*prev;
  ff_link	*next;
  size_t	back;	/* Back link at end of 'dead' block.	*/
};
#define	FFDPAD	sizeof(struct _ffree_free_link_unpadded)

struct _ffree_free_link
{
  size_t	size;
  ff_link	*prev;
  ff_link	*next;
  size_t	back;
  char	padding[ALIGN - ((FFDPAD % ALIGN) ? (FFDPAD % ALIGN) : ALIGN)];
};

/* NSZone structure for freeable zones. */
struct _ffree_zone_struct
{
  NSZone common;
  pthread_mutex_t lock;
  ff_block *blocks; // Linked list of blocks
  ff_link *segheadlist[MAX_SEG]; // Segregated list, holds heads
  ff_link *segtaillist[MAX_SEG]; // Segregated list, holds tails
  size_t bufsize; // Buffer size
  size_t size_buf[BUFFER]; // Buffer holding sizes
  ff_block *ptr_buf[BUFFER]; // Buffer holding pointers to chunks
};

/* Rounds up N to nearest multiple of BASE. */
static inline size_t
roundupto (size_t n, size_t base)
{
  size_t a = (n/base)*base;

  return (n-a)? (a+base): n;
}

/*
 *	Minimum chunk size for freeable zones.
 *	Need room for basic chunk header, next and prev pointers for
 *	free-list, and a reverse pointer (size_t) to go at the end of the
 *	chunk while it is waiting to be consolidated with other chunks.
 */
#define MINCHUNK sizeof(ff_link)

#define CLTOSZ(n) ((n)*MINCHUNK) /* Converts classes to sizes. */

static inline void*
chunkToPointer(ff_block *chunk)
{
  return (void*)(&chunk[1]);
}

static inline ff_block*
pointerToChunk(void* ptr)
{
  return &(((ff_block*)ptr)[-1]);
}

static inline size_t
chunkIsLive(ff_block* ptr)
{
  return ptr->size & LIVE;
}

static inline size_t
chunkIsInUse(ff_block* ptr)
{
  return ptr->size & INUSE;
}

static inline size_t
chunkIsPrevInUse(ff_block* ptr)
{
  return ptr->size & PREVUSE;
}

static inline size_t
chunkSize(ff_block* ptr)
{
  return ptr->size & ~SIZE_BITS;
}

static inline size_t
chunkClrLive(ff_block* ptr)
{
  return ptr->size &= ~LIVE;
}

static inline void
chunkClrPrevInUse(ff_block* ptr)
{
  ptr->size &= ~PREVUSE;
}

static inline void
chunkSetInUse(ff_block* ptr)
{
  ptr->size |= INUSE;
}

static inline size_t
chunkSetLive(ff_block* ptr)
{
  return ptr->size |= LIVE;
}

static inline void
chunkSetPrevInUse(ff_block* ptr)
{
  ptr->size |= PREVUSE;
}

static inline void
chunkSetSize(ff_block* ptr, size_t size)
{
  ptr->size = size;
}

static inline ff_block*
chunkNext(ff_block *ptr)
{
  return (ff_block*) ((void*)ptr+chunkSize(ptr));
}

static inline void
chunkMakeLink(ff_block *ptr)
{
  NSAssert(!chunkIsInUse(ptr), NSInternalInconsistencyException);
  NSAssert(!chunkIsLive(ptr), NSInternalInconsistencyException);
  (&(chunkNext(ptr)->size))[-1] = chunkSize(ptr);
}

static inline ff_block*
chunkChop(ff_block *ptr, size_t size)
{
  ff_block	*remainder;
  size_t	left = chunkSize(ptr)-size;

  NSAssert((chunkSize(ptr) % MINCHUNK) == 0, NSInternalInconsistencyException);
  NSAssert(chunkSize(ptr) > size, NSInternalInconsistencyException);
  remainder = (ff_block*)((void*)ptr+size);
  chunkSetSize(remainder, left | PREVUSE);
  chunkMakeLink(remainder);
  chunkSetSize(ptr, size | chunkIsPrevInUse(ptr) | INUSE);
  return remainder;
}

static inline ff_block*
chunkPrev(ff_block *ptr)
{
  size_t	offset;
  ff_block	*prev;

  NSAssert(!chunkIsPrevInUse(ptr), NSInternalInconsistencyException);
  offset = (&(ptr->size))[-1];
  NSAssert(offset > 0 && (offset % MINCHUNK) == 0,
    NSInternalInconsistencyException);
  prev = (ff_block*)((void*)ptr-offset);
  NSAssert(chunkSize(prev) == offset, NSInternalInconsistencyException);
  NSAssert(!chunkIsInUse(prev), NSInternalInconsistencyException);
  return prev;
}

/* NSZone structure for nonfreeable zones. */
struct _nfree_zone_struct
{
  NSZone common;
  pthread_mutex_t lock;
  /* Linked list of blocks in decreasing order of free space,
     except maybe for the first block. */
  nf_block *blocks;
  size_t use;
};

/* Memory management functions for freeable zones. */
static void* fmalloc (NSZone *zone, size_t size);
static void* frealloc (NSZone *zone, void *ptr, size_t size);
static void ffree (NSZone *zone, void *ptr);
static void frecycle (NSZone *zone);
static BOOL fcheck (NSZone *zone);
static BOOL flookup (NSZone *zone, void *ptr);
static struct NSZoneStats fstats (NSZone *zone);

static inline size_t segindex (size_t size);
static ff_block* get_chunk (ffree_zone *zone, size_t size);
static void take_chunk (ffree_zone *zone, ff_block *chunk);
static void put_chunk (ffree_zone *zone, ff_block *chunk);
static inline void add_buf (ffree_zone *zone, ff_block *chunk);
static void flush_buf (ffree_zone *zone);

/* Memory management functions for nonfreeable zones. */
static void* nmalloc (NSZone *zone, size_t size);
static void nrecycle (NSZone *zone);
static void* nrealloc (NSZone *zone, void *ptr, size_t size);
static void nfree (NSZone *zone, void *ptr);
static BOOL ncheck (NSZone *zone);
static BOOL nlookup (NSZone *zone, void *ptr);
static struct NSZoneStats nstats (NSZone *zone);

/* Memory management functions for recycled zones. */
static void* rmalloc (NSZone *zone, size_t size);
static void rrecycle (NSZone *zone);
static void* rrealloc (NSZone *zone, void *ptr, size_t size);
static void rffree (NSZone *zone, void *ptr);
static void rnfree (NSZone *zone, void *ptr);

/*
 *	Lists of zones to be used to determine if a pointer is in a zone.
 */
static NSZone	*zone_list = 0;

static inline void
destroy_zone(NSZone* zone)
{
  if (zone)
    {
      if (zone_list == zone)
        {
          zone_list = zone->next;
        }
      else
        {
          NSZone *ptr = zone_list;

          while (ptr != NULL && ptr->next != zone)
            {
              ptr = ptr->next;
            }
          if (ptr)
            {
              ptr->next = zone->next;
            }
        }
      free((void*)zone);
    }
}

/* Search the buffer to see if there is any memory chunks large enough
   to satisfy request using first fit.  If the memory chunk found has
   a size exactly equal to the one requested, remove it from the buffer
   and return it.  If not, cut off a chunk that does match the size
   and return it.  If there is no chunk large enough in the buffer,
   get a chunk from the general purpose allocator that uses segregated
   fit.  Since a chunk in the buffer is not freed in the general purpose
   allocator, the headers are as if it is still in use. */
static void*
fmalloc (NSZone *zone, size_t size)
{
  size_t i = 0;
  size_t chunksize = roundupto(size+FBSZ+1, MINCHUNK);
  ffree_zone *zptr = (ffree_zone*)zone;
  size_t bufsize;
  size_t *size_buf = zptr->size_buf;
  ff_block **ptr_buf = zptr->ptr_buf;
  ff_block *chunkhead;
  void *result;

  pthread_mutex_lock(&(zptr->lock));
  bufsize = zptr->bufsize;
  while ((i < bufsize) && (chunksize > size_buf[i]))
    i++;
  if (i < bufsize)
    /* Use memory chunk in buffer. */
    {
      if (size_buf[i] == chunksize)
        /* Exact fit. */
        {
          zptr->bufsize--;
          bufsize = zptr->bufsize;
          chunkhead = ptr_buf[i];
          size_buf[i] = size_buf[bufsize];
          ptr_buf[i] = ptr_buf[bufsize];

          NSAssert(chunkIsInUse(chunkhead), NSInternalInconsistencyException);
          NSAssert((chunkSize(chunkhead) % MINCHUNK) == 0,
	    NSInternalInconsistencyException);
        }
      else
        {
          /*
	   *	Break off chunk leaving remainder marked as in use since it
	   *	stays in this buffer rather than on a free-list.
	   */
          chunkhead = ptr_buf[i];
          size_buf[i] -= chunksize;
          ptr_buf[i] = chunkChop(chunkhead, chunksize);
	  chunkSetInUse(ptr_buf[i]);
        }
    }
  else
    /* Get memory from segregate fit allocator. */
    {
      flush_buf(zptr);
      chunkhead = get_chunk(zptr, chunksize);
      if (chunkhead == NULL)
        {
          pthread_mutex_unlock(&(zptr->lock));
          if (zone->name != nil)
            [NSException raise: NSMallocException
                        format: @"Zone %@ has run out of memory", zone->name];
          else
            [NSException raise: NSMallocException
                        format: @"Out of memory"];
        }

      NSAssert(chunkIsInUse(chunkhead), NSInternalInconsistencyException);
      NSAssert(chunkIsPrevInUse(chunkNext(chunkhead)),
	NSInternalInconsistencyException);
      NSAssert((chunkSize(chunkhead) % MINCHUNK) == 0,
	NSInternalInconsistencyException);
    }
  chunkhead->next = (ff_block*)(chunkToPointer(chunkhead)+size);
  *((char*)chunkhead->next) = (char)42;
  chunkSetLive(chunkhead);
  result = chunkToPointer(chunkhead);
  pthread_mutex_unlock(&(zptr->lock));
  return result;
}

/* If PTR == NULL, then it's the same as ordinary memory allocation.
   If a smaller size than it originally had is requested, shrink the
   chunk.  If a larger size is requested, check if there is enough
   space after it.  If there isn't enough space, get a new chunk and
   move it there, releasing the original.  The space before the chunk
   should also be checked, but I'll leave this to a later date. */
static void*
frealloc (NSZone *zone, void *ptr, size_t size)
{
  size_t realsize;
  size_t chunksize = roundupto(size+FBSZ+1, MINCHUNK);
  ffree_zone *zptr = (ffree_zone*)zone;
  ff_block *chunkhead, *slack;
  void *result;

  NSAssert(ptr == NULL || NSZoneFromPointer(ptr) == zone,
    NSInternalInconsistencyException);
  if (ptr == NULL)
    return fmalloc(zone, size);
  chunkhead = pointerToChunk(ptr);
  pthread_mutex_lock(&(zptr->lock));
  realsize = chunkSize(chunkhead);

  NSAssert(chunkIsInUse(chunkhead), NSInternalInconsistencyException);
  NSAssert((realsize % MINCHUNK) == 0, NSInternalInconsistencyException);

  chunkClrLive(chunkhead);
  if (chunksize < realsize)
    {
      /*
       *	Chop tail off existing memory chunk and tell the next chunk
       *	after it that it is no longer in use.  Then put it in the
       *	buffer to be added to the free list later (we can't add it
       *	immediately 'cos we might invalidate the rule that there
       *	must not be two adjacent unused chunks).
       */
      slack = chunkChop(chunkhead, chunksize);
      chunkSetInUse(slack);
      add_buf(zptr, slack);
    }
  else if (chunksize > realsize)
    {
      size_t nextsize;
      ff_block *nextchunk, *farchunk;

      nextchunk = chunkNext(chunkhead);
      nextsize = chunkSize(nextchunk);

      NSAssert((nextsize % MINCHUNK) == 0, NSInternalInconsistencyException);

      if (!chunkIsInUse(nextchunk) && (nextsize+realsize >= chunksize))
        /* Expand to next chunk. */
        {
          take_chunk(zptr, nextchunk);
          if (nextsize+realsize == chunksize)
            {
              farchunk = chunkNext(nextchunk);
              chunkSetPrevInUse(farchunk);
            }
          else
            {
	      chunkSetSize(chunkhead, nextsize+realsize);
	      slack = chunkChop(chunkhead, chunksize);
              put_chunk(zptr, slack);
            }
	  chunkSetSize(chunkhead, chunksize |
		chunkIsPrevInUse(chunkhead) | INUSE);
        }
      else
        /* Get new chunk and copy. */
        {
          ff_block *newchunk;

          newchunk = get_chunk(zptr, chunksize);
          if (newchunk == NULL)
            {
              pthread_mutex_unlock(&(zptr->lock));
              if (zone->name != nil)
                [NSException raise: NSMallocException
                            format: @"Zone %@ has run out of memory",
                             zone->name];
              else
                [NSException raise: NSMallocException
                            format: @"Out of memory"];
            }
          memcpy((void*)(&newchunk[1]), (void*)(&chunkhead[1]), realsize-FBSZ);
          add_buf(zptr, chunkhead);
          chunkhead = newchunk;
        }
    }
  chunkhead->next = (ff_block*)(chunkToPointer(chunkhead)+size);
  *((char*)chunkhead->next) = (char)42;
  chunkSetLive(chunkhead);
  result = chunkToPointer(chunkhead);
  pthread_mutex_unlock(&(zptr->lock));
  return result;
}

/* Frees memory chunk by simply adding it to the buffer. */
static void
ffree (NSZone *zone, void *ptr)
{
  ff_block *chunk;
  NSAssert(NSZoneFromPointer(ptr) == zone, NSInternalInconsistencyException);
  pthread_mutex_lock(&(((ffree_zone*)zone)->lock));
  chunk = pointerToChunk(ptr);
  if (chunkIsLive(chunk) == 0)
    [NSException raise: NSMallocException
	        format: @"Attempt to free freed memory"];
  NSAssert(*((char*)chunk->next) == (char)42, NSInternalInconsistencyException);
  add_buf((ffree_zone*)zone, chunk);
  pthread_mutex_unlock(&(((ffree_zone*)zone)->lock));
}

static BOOL
frecycle1(NSZone *zone)
{
  ffree_zone *zptr = (ffree_zone*)zone;
  ff_block *block;
  ff_block *nextblock;

  pthread_mutex_lock(&(zptr->lock));
  flush_buf(zptr);
  block = zptr->blocks;
  while (block != NULL)
    {
      ff_block	*tmp = &block[1];
      nextblock = block->next;
      if (chunkIsInUse(tmp) == 0 && chunkNext(tmp) == chunkNext(block))
	{
	  if (zptr->blocks == block)
	    zptr->blocks = block->next;
	  else
	    {
	      tmp = zptr->blocks;
	      while (tmp->next != block)
		tmp = tmp->next;
	      tmp->next = block->next;
	    }
          free((void*)block);
	}
      block = nextblock;
    }
  pthread_mutex_unlock(&(zptr->lock));
  if (zptr->blocks == 0)
    {
      pthread_mutex_destroy(&(zptr->lock));
      return YES;
    }
  return NO;
}

/* Recycle the zone. */
static void
frecycle (NSZone *zone)
{
  pthread_mutex_lock(&zoneLock);
  if (zone->name != nil)
    {
      NSString *name = zone->name;
      zone->name = nil;
      [name release];
    }
  if (frecycle1(zone) == YES)
    destroy_zone(zone);
  else
    {
      zone->malloc = rmalloc;
      zone->realloc = rrealloc;
      zone->free = rffree;
      zone->recycle = rrecycle;
    }
  pthread_mutex_unlock(&zoneLock);
}

static void
rffree (NSZone *zone, void *ptr)
{
  ffree(zone, ptr);
  pthread_mutex_lock(&zoneLock);
  if (frecycle1(zone))
    destroy_zone(zone);
  pthread_mutex_unlock(&zoneLock);
}


/* Check integrity of a freeable zone.  Doesn't have to be
   particularly efficient. */
static BOOL
fcheck (NSZone *zone)
{
  size_t i;
  ffree_zone *zptr = (ffree_zone*)zone;
  ff_block *block;

  pthread_mutex_lock(&(zptr->lock));
  /* Check integrity of each block the zone owns. */
  block = zptr->blocks;
  while (block != NULL)
    {
      ff_block *blockstart = &block[1];
      ff_block *blockend = chunkNext(block);
      ff_block *nextchunk = blockstart;

      if (blockend->next != block)
	goto inconsistent;
      if (!chunkIsPrevInUse(blockstart))
	goto inconsistent;

      while (nextchunk < blockend)
        {
          ff_block *chunk = nextchunk;
          size_t chunksize;

	  chunksize = chunkSize(chunk);
	  if ((chunksize % ALIGN) != 0)
	    goto inconsistent;
	  nextchunk = chunkNext(chunk);

          if (chunkIsInUse(chunk))
            /* Check whether this is a valid used chunk. */
            {
              if (!chunkIsPrevInUse(nextchunk))
                goto inconsistent;
	      if (chunkIsLive(chunk))
		{
	          if (chunk->next < &chunk[1] || chunk->next > nextchunk)
                    goto inconsistent;
	          if (*(char*)chunk->next != (char)42)
                    goto inconsistent;
		}
            }
          else
            /* Check whether this is a valid free chunk. */
            {
              if (chunkIsPrevInUse(nextchunk))
                goto inconsistent;
              if (!chunkIsInUse(nextchunk))
                goto inconsistent;
	      if (chunkIsLive(chunk))
                goto inconsistent;
            }
	  if (chunk != blockstart && chunkIsPrevInUse(chunk) == 0)
	    {
	      ff_block *prev = chunkPrev(chunk);

	      if (chunkNext(prev) != chunk)
		goto inconsistent;
	    }
        }
      /* Check whether the block ends properly. */
      if (nextchunk != blockend)
	goto inconsistent;
      if (chunkSize(blockend) != 0)
        goto inconsistent;
      if (chunkIsInUse(blockend) == 0)
        goto inconsistent;

      block = block->next;
    }
  /* Check the integrity of the segregated list. */
  for (i = 0; i < MAX_SEG; i++)
    {
      ff_link	*chunk = zptr->segheadlist[i];

      while (chunk != NULL)
        {
          ff_link *nextchunk;

          nextchunk = chunk->next;
          /* Isn't this one ugly if statement? */
          if (chunkIsInUse((ff_block*)chunk)
              || (segindex(chunkSize((ff_block*)chunk)) != i)
              || ((nextchunk != NULL) && (chunk != nextchunk->prev))
              || ((nextchunk == NULL) && (chunk != zptr->segtaillist[i])))
            goto inconsistent;
          chunk = nextchunk;
        }
    }
  /* Check the buffer. */
  if (zptr->bufsize > BUFFER)
    goto inconsistent;
  for (i = 0; i < zptr->bufsize; i++)
    {
      ff_block *chunk = zptr->ptr_buf[i];
      if ((zptr->size_buf[i] != chunkSize(chunk)) || !chunkIsInUse(chunk))
        goto inconsistent;
    }
  pthread_mutex_unlock(&(zptr->lock));
  return YES;

inconsistent: // Jump here if an inconsistency was found.
  pthread_mutex_unlock(&(zptr->lock));
  return NO;
}

static BOOL
flookup (NSZone *zone, void *ptr)
{
  ffree_zone	*zptr = (ffree_zone*)zone;
  ff_block	*block;
  BOOL		found = NO;

  pthread_mutex_lock(&(zptr->lock));
  for (block = zptr->blocks; block != NULL; block = block->next)
    {
      if (ptr >= (void*)block && ptr < (void*)chunkNext(block))
	{
	  found = YES;
	  break;
	}
    }
  pthread_mutex_unlock(&(zptr->lock));
  return found;
}

/* Obtain statistics about the zone.  Doesn't have to be particularly
   efficient. */
static struct NSZoneStats
fstats (NSZone *zone)
{
  size_t i;
  struct NSZoneStats stats;
  ffree_zone *zptr = (ffree_zone*)zone;
  ff_block *block;

  stats.bytes_total = 0;
  stats.chunks_used = 0;
  stats.bytes_used = 0;
  stats.chunks_free = 0;
  stats.bytes_free = 0;
  pthread_mutex_lock(&(zptr->lock));
  block = zptr->blocks;
  /* Go through each block. */
  while (block != NULL)
    {
      ff_block *blockend = chunkNext(block);
      ff_block *chunk = &block[1];

      stats.bytes_total += chunkSize(block);
      while (chunk < blockend)
        {
          size_t chunksize = chunkSize(chunk);

          if (chunkIsInUse(chunk))
            {
              stats.chunks_used++;
              stats.bytes_used += chunksize;
            }
          else
            {
              stats.chunks_free++;
              stats.bytes_free += chunksize;
            }
          chunk = chunkNext(chunk);
        }
      block = block->next;
    }
  /* Go through buffer. */
  for (i = 0; i < zptr->bufsize; i++)
    {
      stats.chunks_used--;
      stats.chunks_free++;
      stats.bytes_used -= zptr->size_buf[i];
      stats.bytes_free += zptr->size_buf[i];
    }
  pthread_mutex_unlock(&(zptr->lock));
  /* Remove overhead. */
  stats.bytes_used -= FBSZ*stats.chunks_used;
  return stats;
}

/* Calculate which segregation class a certain size should be in.
   FIXME: Optimize code and find a more optimum distribution. */
static inline size_t
segindex (size_t size)
{
  NSAssert(size%MINCHUNK == 0, NSInternalInconsistencyException);

  if (size < CLTOSZ(8))
    return size/MINCHUNK;
  else if (size < CLTOSZ(16))
    return 7;
  else if (size < CLTOSZ(32))
    return 8;
  else if (size < CLTOSZ(64))
    return 9;
  else if (size < CLTOSZ(128))
    return 10;
  else if (size < CLTOSZ(256))
    return 11;
  else if (size < CLTOSZ(512))
    return 12;
  else if (size < CLTOSZ(1024))
    return 13;
  else if (size < CLTOSZ(2048))
    return 14;
  else
    return 15;
}

/* Look through the segregated list with first fit to find a memory
   chunk.  If one is not found, get more memory. */
static ff_block*
get_chunk (ffree_zone *zone, size_t size)
{
  size_t class = segindex(size);
  ff_block *chunk;
  ff_link *link = zone->segheadlist[class];

  NSAssert(size%MINCHUNK == 0, NSInternalInconsistencyException);

  while ((link != NULL) && (chunkSize((ff_block*)link) < size))
    link = link->next;
  if (link == NULL)
    /* Get more memory. */
    {
      class++;
      while ((class < MAX_SEG) && (zone->segheadlist[class] == NULL))
        class++;
      if (class == MAX_SEG)
        /* Absolutely no memory in segregated list. */
        {
          size_t blocksize;
          ff_block *block;

          blocksize = roundupto(size, zone->common.gran);
          block = malloc(blocksize+2*FBSZ);
          if (block == NULL)
            return NULL;

	  /*
	   *	Set up the new block header and add to blocks list.
	   */
          block->size = blocksize+FBSZ;	/* Point to block trailer.	*/
          block->next = zone->blocks;
          zone->blocks = block;
	  /*
	   *	Set up the block trailer.
	   */
          chunk = chunkNext(block);
	  chunk->next = block;		/* Point back to block head.	*/
	  /*
	   *	Now set up block contents.
	   */
          if (size < blocksize)
            {
	      chunkSetSize(chunk, INUSE);	/* Tailer size is zero.	*/
              chunk = &block[1];
	      chunkSetSize(chunk, size | PREVUSE | INUSE);
	      chunk = chunkNext(chunk);
	      chunkSetSize(chunk, (block->size-FBSZ-size) | PREVUSE);
              put_chunk(zone, chunk);
              chunk = &block[1];
            }
          else
	    {
	      chunkSetSize(chunk, PREVUSE | INUSE);
              chunk = &block[1];
	      chunkSetSize(chunk, size | PREVUSE | INUSE);
	    }
        }
      else
        {
          ff_block *slack;

          NSAssert(class < MAX_SEG, NSInternalInconsistencyException);

          chunk = (ff_block*)zone->segheadlist[class];

          NSAssert(!chunkIsInUse(chunk), NSInternalInconsistencyException);
          NSAssert(size < chunkSize(chunk), NSInternalInconsistencyException);
          NSAssert((chunkSize(chunk) % MINCHUNK) == 0,
	    NSInternalInconsistencyException);

          take_chunk(zone, chunk);
	  slack = chunkChop(chunk, size);
          put_chunk(zone, slack);
        }
    }
  else
    {
      size_t chunksize;

      chunk = (ff_block*)link;
      chunksize = chunkSize(chunk);

      NSAssert((chunksize % MINCHUNK) == 0, NSInternalInconsistencyException);
      NSAssert(!chunkIsInUse(chunk), NSInternalInconsistencyException);
      NSAssert(chunkIsPrevInUse(chunk), NSInternalInconsistencyException);
      NSAssert(chunkIsInUse(chunkNext(chunk)),
	NSInternalInconsistencyException);

      take_chunk(zone, chunk);
      if (chunksize > size)
        {
          ff_block *slack;

          slack = chunkChop(chunk, size);
          put_chunk(zone, slack);
        }
      else
        {
          ff_block *nextchunk = chunkNext(chunk);

          NSAssert(!chunkIsInUse(chunk), NSInternalInconsistencyException);
          NSAssert(!chunkIsPrevInUse(nextchunk),
	    NSInternalInconsistencyException);
          NSAssert(chunksize == size, NSInternalInconsistencyException);
	  chunkSetInUse(chunk);
	  chunkSetPrevInUse(nextchunk);
        }
    }
  NSAssert(chunkIsInUse(chunk), NSInternalInconsistencyException);
  NSAssert(chunkIsPrevInUse(chunkNext(chunk)),
    NSInternalInconsistencyException);
  return chunk;
}

/* Take the given chunk out of the free list.  No headers are set. */
static void
take_chunk (ffree_zone *zone, ff_block *chunk)
{
  size_t size = chunkSize(chunk);
  size_t class = segindex(size);
  ff_link *otherlink;
  ff_link *links = (ff_link*)chunk;

  NSAssert((size % MINCHUNK) == 0, NSInternalInconsistencyException);
  NSAssert(!chunkIsInUse(chunk), NSInternalInconsistencyException);

  if (links->prev == NULL)
    zone->segheadlist[class] = links->next;
  else
    {
      otherlink = links->prev;
      otherlink->next = links->next;
    }
  if (links->next == NULL)
    zone->segtaillist[class] = links->prev;
  else
    {
      otherlink = links->next;
      otherlink->prev = links->prev;
    }
}

/*
 *	Add the given chunk to the segregated list.  The header to the
 *	chunk must be set appropriately, but the tailer is set here.
 *	NB.  The chunk must NOT be in use, and the adjacent chunks within
 *	its memory block MUST be in use - the memory coalescing done in
 *	flush_buf() depends on this rule.
 */
static void
put_chunk (ffree_zone *zone, ff_block *chunk)
{
  size_t size = chunkSize(chunk);
  size_t class = segindex(size);
  ff_link *links = (ff_link*)chunk;

  NSAssert((chunkSize(chunk) % MINCHUNK) == 0,
    NSInternalInconsistencyException);
  NSAssert(!chunkIsInUse(chunk), NSInternalInconsistencyException);
  NSAssert(chunkIsPrevInUse(chunk), NSInternalInconsistencyException);
  NSAssert(chunkIsInUse(chunkNext(chunk)), NSInternalInconsistencyException);

  chunkMakeLink(chunk);
  if (zone->segtaillist[class] == NULL)
    {
      NSAssert(zone->segheadlist[class] == NULL,
	NSInternalInconsistencyException);

      zone->segheadlist[class] = zone->segtaillist[class] = links;
      links->prev = links->next = NULL;
    }
  else
    {
      ff_link *prevlink = zone->segtaillist[class];

      NSAssert(zone->segheadlist[class] != NULL,
	NSInternalInconsistencyException);

      links->next = NULL;
      links->prev = prevlink;
      prevlink->next = links;
      zone->segtaillist[class] = links;
    }
}

/* Add the given pointer to the buffer.  If the buffer becomes full,
   flush it.  The given pointer must always be one that points to used
   memory (i.e. chunks with headers that declare them as used). */
static inline void
add_buf (ffree_zone *zone, ff_block *chunk)
{
  size_t bufsize = zone->bufsize;

  NSAssert(bufsize < BUFFER, NSInternalInconsistencyException);
  NSAssert(chunkIsInUse(chunk), NSInternalInconsistencyException);
  NSAssert((chunkSize(chunk) % MINCHUNK) == 0,
    NSInternalInconsistencyException);
  NSAssert(chunkSize(chunk) >= MINCHUNK, NSInternalInconsistencyException);

  zone->bufsize++;
  zone->size_buf[bufsize] = chunkSize(chunk);
  zone->ptr_buf[bufsize] = chunk;
  chunkClrLive(chunk);
  if (bufsize == BUFFER-1)
    flush_buf(zone);
}

/* Flush buffers.  All coalescing is done here. */
static void
flush_buf (ffree_zone *zone)
{
  size_t i, size;
  size_t bufsize = zone->bufsize;
  ff_block *chunk, *nextchunk;
  size_t *size_buf = zone->size_buf;
  ff_block **ptr_buf = zone->ptr_buf;

  NSAssert(bufsize <= BUFFER, NSInternalInconsistencyException);

  for (i = 0; i < bufsize; i++)
    {
      size = size_buf[i];
      chunk = ptr_buf[i];

      NSAssert(chunkSize(chunk) == size, NSInternalInconsistencyException);
      NSAssert(chunkIsInUse(chunk), NSInternalInconsistencyException);

      nextchunk = chunkNext(chunk);
      if (!chunkIsPrevInUse(chunk))
        /* Coalesce with previous chunk. */
        {
	  chunk = chunkPrev(chunk);
	  NSAssert(!chunkIsInUse(chunk), NSInternalInconsistencyException);
	  NSAssert(chunkIsPrevInUse(chunk), NSInternalInconsistencyException);
          size += chunkSize(chunk);
          take_chunk(zone, chunk);
        }
      if (!chunkIsInUse(nextchunk))
        /* Coalesce with next chunk. */
        {
          size_t nextsize = chunkSize(nextchunk);

	  NSAssert(chunkIsPrevInUse(nextchunk),
	    NSInternalInconsistencyException);
          NSAssert((nextsize % MINCHUNK) == 0,
	    NSInternalInconsistencyException);
          size += nextsize;
          take_chunk(zone, nextchunk);
	  nextchunk = chunkNext(nextchunk);
        }
      chunkSetSize(chunk, size | PREVUSE);
      put_chunk(zone, chunk);
      chunkClrPrevInUse(nextchunk);
      NSAssert(chunkNext(chunk) == nextchunk, NSInternalInconsistencyException);
      NSAssert(chunkPrev(nextchunk) == chunk, NSInternalInconsistencyException);
      NSAssert((chunkSize(chunk) % MINCHUNK) == 0,
	NSInternalInconsistencyException);
      NSAssert(!chunkIsInUse(chunk), NSInternalInconsistencyException);
      NSAssert(chunkIsPrevInUse(chunk), NSInternalInconsistencyException);
      NSAssert(chunkIsInUse(nextchunk), NSInternalInconsistencyException);
      NSAssert(!chunkIsPrevInUse(nextchunk), NSInternalInconsistencyException);
    }
  zone->bufsize = 0;
}

/* If the first block in block list has enough space, use that space.
   Otherwise, sort the block list in decreasing free space order (only
   the first block needs to be put in its appropriate place since
   the rest of the list is already sorted).  Then check if the first
   block has enough space for the request.  If it does, use it.  If it
   doesn't, get more memory from the default zone, since none of the
   other blocks in the block list could have enough memory. */
static void*
nmalloc (NSZone *zone, size_t size)
{
  nfree_zone *zptr = (nfree_zone*)zone;
  size_t chunksize = roundupto(size, ALIGN);
  size_t freesize;
  void *chunkhead;
  nf_block *block;
  size_t top;

  pthread_mutex_lock(&(zptr->lock));
  block = zptr->blocks;
  top = block->top;
  freesize = block->size-top;
  if (freesize >= chunksize)
    {
      chunkhead = (void*)(block)+top;
      block->top += chunksize;
    }
  else
    {
      nf_block *preblock;

      /* First, get the block list in decreasing free size order. */
      preblock = NULL;
      while ((block->next != NULL)
        && (freesize < block->next->size-block->next->top))
        {
          preblock = block;
          block = block->next;
        }
      if (preblock != NULL)
        {
          preblock->next = zptr->blocks;
          zptr->blocks = zptr->blocks->next;
          preblock->next->next = block;
        }
      if (zptr->blocks->size-zptr->blocks->top < chunksize)
        /* Get new block. */
        {
          size_t blocksize = roundupto(chunksize+NF_HEAD, zone->gran);

          block = malloc(blocksize);
          if (block == NULL)
            {
              pthread_mutex_unlock(&(zptr->lock));
              if (zone->name != nil)
                [NSException raise: NSMallocException
                            format: @"Zone %@ has run out of memory",
                             zone->name];
              else
                [NSException raise: NSMallocException
                            format: @"Out of memory"];
            }
          block->next = zptr->blocks;
          block->size = blocksize;
          block->top = NF_HEAD;
          zptr->blocks = block;
        }
      chunkhead = (void*)block+block->top;
      block->top += chunksize;
    }
  zptr->use++;
  pthread_mutex_unlock(&(zptr->lock));
  return chunkhead;
}

/* Return the blocks to the default zone, then deallocate mutex, and
   then release zone name if it exists. */
static BOOL
nrecycle1 (NSZone *zone)
{
  nfree_zone *zptr = (nfree_zone*)zone;

  pthread_mutex_lock(&(zptr->lock));
  if (zptr->use == 0)
    {
      nf_block *nextblock;
      nf_block *block = zptr->blocks;

      while (block != NULL)
	{
	  nextblock = block->next;
	  free(block);
	  block = nextblock;
	}
      zptr->blocks = 0;
    }
  pthread_mutex_unlock(&(zptr->lock));
  if (zptr->blocks == 0)
    {
      pthread_mutex_destroy(&(zptr->lock));
      return YES;
    }
  return NO;
}

/* Recycle the zone. */
static void
nrecycle (NSZone *zone)
{
  pthread_mutex_lock(&zoneLock);
  if (zone->name != nil)
    {
      NSString *name = zone->name;
      zone->name = nil;
      [name release];
    }
  if (nrecycle1(zone) == YES)
    destroy_zone(zone);
  else
    {
      zone->malloc = rmalloc;
      zone->realloc = rrealloc;
      zone->free = rnfree;
      zone->recycle = rrecycle;
    }
  pthread_mutex_unlock(&zoneLock);
}

static void*
nrealloc (NSZone *zone, void *ptr, size_t size)
{
  nfree_zone *zptr = (nfree_zone*)zone;
  void *tmp = nmalloc(zone, size);

  if (ptr != 0)
    {
      pthread_mutex_lock(&(zptr->lock));
      if (tmp)
	{
	  nf_block *block;
	  size_t old = 0;

	  for (block = zptr->blocks; block != NULL; block = block->next) {
	    if (ptr >= (void*)block && ptr < ((void*)block)+block->size) {
		old = ((void*)block)+block->size - ptr;
		break;
	    }
	  }
	  if (old > 0)
	    {
	      if (size < old)
		old = size;
	      memcpy(tmp, ptr, old);
	    }
	}
      zptr->use--;
      pthread_mutex_unlock(&(zptr->lock));
    }
  return tmp;
}

/*
 *	The OpenStep spec says we don't release memory - but we have to do
 *	some minimal bookkeeping so that, when the zone is recycled, we can
 *	determine if all the allocated memory has been freed.  Until it is
 *	all freed, we can't actually destroy the zone!
 */
static void
nfree (NSZone *zone, void *ptr)
{
  nfree_zone *zptr = (nfree_zone*)zone;

  pthread_mutex_lock(&(zptr->lock));
  zptr->use--;
  pthread_mutex_unlock(&(zptr->lock));
}

static void
rnfree (NSZone *zone, void *ptr)
{
  nfree_zone *zptr = (nfree_zone*)zone;

  nfree(zone, ptr);
  if (zptr->use == 0)
    {
      pthread_mutex_lock(&zoneLock);
      nrecycle1(zone);
      destroy_zone(zone);
      pthread_mutex_unlock(&zoneLock);
    }
}

/* Check integrity of a nonfreeable zone.  Doesn't have to
   particularly efficient. */
static BOOL
ncheck (NSZone *zone)
{
  nfree_zone *zptr = (nfree_zone*)zone;
  nf_block *block;

  pthread_mutex_lock(&(zptr->lock));
  block = zptr->blocks;
  while (block != NULL)
    {
      if (block->size < block->top)
        {
          pthread_mutex_unlock(&(zptr->lock));
          return NO;
        }
      block = block->next;
    }
  /* FIXME: Do more checking? */
  pthread_mutex_unlock(&(zptr->lock));
  return YES;
}

static BOOL
nlookup (NSZone *zone, void *ptr)
{
  nfree_zone *zptr = (nfree_zone*)zone;
  nf_block *block;
  BOOL found = NO;

  pthread_mutex_lock(&(zptr->lock));
  for (block = zptr->blocks; block != NULL; block = block->next)
    {
      if (ptr >= (void*)block &&  ptr < ((void*)block)+block->size)
	{
	  found = YES;
	  break;
	}
    }
  pthread_mutex_unlock(&(zptr->lock));
  return found;
}

/* Return statistics for a nonfreeable zone.  Doesn't have to
   particularly efficient. */
static struct NSZoneStats
nstats (NSZone *zone)
{
  struct NSZoneStats stats;
  nfree_zone *zptr = (nfree_zone*)zone;
  nf_block *block;

  stats.bytes_total = 0;
  stats.chunks_used = 0;
  stats.bytes_used = 0;
  stats.chunks_free = 0;
  stats.bytes_free = 0;
  pthread_mutex_lock(&(zptr->lock));
  block = zptr->blocks;
  while (block != NULL)
    {
      size_t *chunk;

      stats.bytes_total += block->size;
      chunk = (void*)block+NF_HEAD;
      while ((void*)chunk < (void*)block+block->top)
        {
          stats.chunks_used++;
          stats.bytes_used += *chunk;
          chunk = (void*)chunk+(*chunk);
        }
      if (block->size != block->top)
        {
          stats.chunks_free++;
          stats.bytes_free += block->size-block->top;
        }
      block = block->next;
    }
  pthread_mutex_unlock(&(zptr->lock));
  return stats;
}


static void*
rmalloc (NSZone *zone, size_t size)
{
  [NSException raise: NSMallocException
	      format: @"Attempt to malloc memory in recycled zone"];
  return 0;
}

static void
rrecycle (NSZone *zone)
{
  [NSException raise: NSMallocException
	      format: @"Attempt to recycle a recycled zone"];
}

static void*
rrealloc (NSZone *zone, void *ptr, size_t size)
{
  [NSException raise: NSMallocException
	      format: @"Attempt to realloc memory in recycled zone"];
  return 0;
}

static void rnfree (NSZone *zone, void *ptr);

GS_DECLARE NSZone*
NSZoneFromPointer(void *ptr)
{
  NSZone	*zone;

  if (ptr == 0) return 0;
  if (zone_list == 0) return &default_zone;

  /*
   *	See if we can find the zone in our list of all zones.
   */
  pthread_mutex_lock(&zoneLock);
  for (zone = zone_list; zone != 0; zone = zone->next)
    {
      if ((zone->lookup)(zone, ptr) == YES)
	{
	  break;
	}
    }
  pthread_mutex_unlock(&zoneLock);
  return (zone == 0) ? &default_zone : zone;
}

NSZone*
NSCreateZone (NSUInteger start, NSUInteger gran, BOOL canFree)
{
  size_t i, startsize, granularity;
  NSZone *newZone;

  if (start > 0)
    startsize = roundupto(start, roundupto(MINGRAN, MINCHUNK));
  else
    startsize = roundupto(MINGRAN, MINCHUNK);
  if (gran > 0)
    granularity = roundupto(gran, roundupto(MINGRAN, MINCHUNK));
  else
    granularity = roundupto(MINGRAN, MINCHUNK);
  if (canFree)
    {
      ffree_zone *zone;
      ff_block *block;
      ff_block *chunk;
      ff_block *tailer;

      zone = malloc(sizeof(ffree_zone));
      if (zone == NULL)
        [NSException raise: NSMallocException
                    format: @"No memory to create zone"];
      zone->common.malloc = fmalloc;
      zone->common.realloc = frealloc;
      zone->common.free = ffree;
      zone->common.recycle = frecycle;
      zone->common.check = fcheck;
      zone->common.lookup = flookup;
      zone->common.stats = fstats;
      zone->common.gran = granularity;
      zone->common.name = nil;
      GS_INIT_RECURSIVE_MUTEX(zone->lock);
      for (i = 0; i < MAX_SEG; i++)
        {
          zone->segheadlist[i] = NULL;
          zone->segtaillist[i] = NULL;
        }
      zone->bufsize = 0;
      zone->blocks = malloc(startsize + 2*FBSZ);
      if (zone->blocks == NULL)
        {
          pthread_mutex_destroy(&(zone->lock));
          free(zone);
          [NSException raise: NSMallocException
                      format: @"No memory to create zone"];
        }
      /*
       *	Set up block header.
       */
      block = zone->blocks;
      block->next = NULL;		/* Point to next block.		*/
      block->size = startsize+FBSZ;	/* Point to first chunk.	*/
      /*
       *	Set up block trailer.
       */
      tailer = chunkNext(block);
      chunkSetSize(tailer, PREVUSE|INUSE);
      tailer->next = block;		/* Point back to block start.	*/
      /*
       *	Set up the block as a single chunk and put it in the
       *	buffer for quick allocation.
       */
      chunk = &block[1];
      chunkSetSize(chunk, (block->size-FBSZ) | PREVUSE|INUSE);
      add_buf(zone, chunk);

      newZone = (NSZone*)zone;
    }
  else
    {
      nf_block *block;
      nfree_zone *zone;

      zone = malloc(sizeof(nfree_zone));
      if (zone == NULL)
        [NSException raise: NSMallocException
                    format: @"No memory to create zone"];
      zone->common.malloc = nmalloc;
      zone->common.realloc = nrealloc;
      zone->common.free = nfree;
      zone->common.recycle = nrecycle;
      zone->common.check = ncheck;
      zone->common.lookup = nlookup;
      zone->common.stats = nstats;
      zone->common.gran = granularity;
      zone->common.name = nil;
      GS_INIT_RECURSIVE_MUTEX(zone->lock);
      zone->blocks = malloc(startsize);
      zone->use = 0;
      if (zone->blocks == NULL)
        {
          pthread_mutex_destroy(&(zone->lock));
          free(zone);
          [NSException raise: NSMallocException
                      format: @"No memory to create zone"];
        }

      block = zone->blocks;
      block->next = NULL;
      block->size = startsize;
      block->top = NF_HEAD;
      newZone = (NSZone*)zone;
    }

  pthread_mutex_lock(&zoneLock);
  newZone->next = zone_list;
  zone_list = newZone;
  pthread_mutex_unlock(&zoneLock);

  return newZone;
}

void*
NSZoneCalloc (NSZone *zone, NSUInteger elems, NSUInteger bytes)
{
  void *mem;

  if (0 == zone || NSDefaultMallocZone() == zone)
    {
      mem = calloc(elems, bytes);
      if (mem != NULL)
        {
          return mem;
        }
      [NSException raise: NSMallocException
                  format: @"Default zone has run out of memory"];
    }
  return memset(NSZoneMalloc(zone, elems*bytes), 0, elems*bytes);
}

void *
NSAllocateCollectable(NSUInteger size, NSUInteger options)
{
  return NSZoneCalloc(NSDefaultMallocZone(), 1, size);
}

void *
NSReallocateCollectable(void *ptr, NSUInteger size, NSUInteger options)
{
  return NSZoneRealloc(0, ptr, size);
}

NSZone*
NSDefaultMallocZone (void)
{
  return &default_zone;
}

NSZone*
GSAtomicMallocZone (void)
{
  return &default_zone;
}

void
GSMakeWeakPointer(Class theClass, const char *iVarName)
{
  return;
}

BOOL
GSAssignZeroingWeakPointer(void **destination, void *source)
{
  if (destination == 0)
    {
      return NO;
    }
  *destination = source;
  return YES;
}

void*
NSZoneMalloc (NSZone *zone, NSUInteger size)
{
  if (!zone)
    zone = NSDefaultMallocZone();
  return (zone->malloc)(zone, size);
}

void* 
NSZoneRealloc (NSZone *zone, void *ptr, NSUInteger size)
{
  if (!zone)
    zone = NSDefaultMallocZone();
  return (zone->realloc)(zone, ptr, size);
}

void
NSRecycleZone (NSZone *zone)
{
  if (!zone)
    zone = NSDefaultMallocZone();
  (zone->recycle)(zone);
}

void
NSZoneFree (NSZone *zone, void *ptr)
{
  if (!zone)
    zone = NSDefaultMallocZone();
  (zone->free)(zone, ptr);
}

BOOL
NSZoneCheck (NSZone *zone)
{
  if (!zone)
    zone = NSDefaultMallocZone();
  return (zone->check)(zone);
}

struct NSZoneStats
NSZoneStats (NSZone *zone)
{
  if (!zone)
    zone = NSDefaultMallocZone();
  return (zone->stats)(zone);
}

BOOL
GSPrivateIsCollectable(const void *ptr)
{
  return NO;
}

