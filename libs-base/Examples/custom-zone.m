/* Example implementation of a custom zone.

  Copyright (C) 2005 Free Software Foundation

  Copying and distribution of this file, with or without modification,
  are permitted in any medium without royalty provided the copyright
  notice and this notice are preserved.

   by Yoo C. Chung <wacko@power1.snu.ac.kr>

   WARNING: Very little testing has been done, so there is no
   guarantee that it will work at all.  I wouldn't use it for real
   applications, anyway. */

#include <string.h>
#include <objc/thr.h>
#include <Foundation/NSException.h>
#include <Foundation/NSZone.h>

typedef struct _block
{
  struct _block *next;
} block;

typedef struct _free_chunk
{
  size_t size;
  struct _free_chunk *next;
} free_chunk;

typedef struct _custom_zone
{
  NSZone common;
  objc_mutex_t mutex;
  free_chunk *free_list;
  block *block_list;
} custom_zone;

void custom_free (NSZone *zone, void *ptr);

void*
custom_malloc (NSZone *zone, size_t size)
{
  free_chunk *chunk;
  custom_zone *zptr = (custom_zone*)zone;

  /* Round up 'size' for alignment. */
  if (size%sizeof(double) > 0)
    size = (size/sizeof(double)+1)*sizeof(double);

  objc_mutex_lock(zptr->mutex);

  /* Check free list for chunks big enough for request. */
  chunk = zptr->free_list;
  while (chunk != NULL && chunk->size < size)
    chunk = chunk->next;

  if (chunk == NULL)
    /* Get more memory for the zone. */
    {
      size_t new_size, block_size;
      block *new_block;

      /* Consider overhead. */
      new_size = sizeof(block)+sizeof(size_t)+NSZoneChunkOverhead()+size;

      /* Round up to a multiple of the granularity. */
      if (new_size%zone->gran > 0)
        block_size = (new_size/zone->gran+1)*zone->gran;
      else
        block_size = new_size;

      /* Get memory from the default zone.  You can get memory from
         somewhere else if you want.  We catch exceptions here since
         we will want to unlock our mutex. */
      NS_DURING
        new_block = NSZoneMalloc(NSDefaultMallocZone(), block_size);
      NS_HANDLER
        objc_mutex_unlock(zptr->mutex);
        [localException raise];
      NS_ENDHANDLER

      /* Register the block as used by the zone. */
      NSZoneRegisterRegion(zone, new_block, (void*)new_block+block_size);

      /* Add to block list. */
      new_block->next = zptr->block_list;
      zptr->block_list = new_block;

      if (block_size > new_size)
        /* Add slack to the free list. */
        {
          chunk = (void*)new_block+new_size;
          chunk->size =
	    block_size-new_size-sizeof(size_t)-NSZoneChunkOverhead();
          chunk->next = zptr->free_list;
          zptr->free_list = chunk;
        }
      chunk = (void*)new_block+sizeof(block);
      chunk->size = new_size-sizeof(size_t)-NSZoneChunkOverhead();
    }
  objc_mutex_unlock(zptr->mutex);
  return NSZoneRegisterChunk(zone, (void*)chunk+sizeof(size_t));
}

void*
custom_realloc (NSZone *zone, void *ptr, size_t size)
{
  size_t oldsize;

  /* Round up for alignment. */
  if (size%sizeof(double) > 0)
    size = (size/sizeof(double)+1)*sizeof(double);

  /* Get size of given memory chunk. */
  oldsize = *((size_t*)(ptr-(NSZoneChunkOverhead()+sizeof(size_t))));

  if (oldsize < size)
    /* Allocate a larger chunk and move contents there. */
    {
      void *newptr;

      newptr = custom_malloc(zone, size);
      memcpy(newptr, ptr, size);
      custom_free(zone, ptr);
      return newptr;
    }
  else
    return ptr; // Return old memory chunk.
}

void
custom_free (NSZone *zone, void *ptr)
{
  free_chunk *chunk;
  custom_zone *zptr = (custom_zone*)zone;

  objc_mutex_lock(zptr->mutex);
  chunk = ptr-(NSZoneChunkOverhead()+sizeof(size_t));

  /* Add chunk to free list. */
  chunk->next = zptr->free_list;
  zptr->free_list = chunk;

  objc_mutex_unlock(zptr->mutex);
}

void
custom_recycle (NSZone *zone)
{
  block *cur_block, *next_block;
  custom_zone *zptr = (custom_zone*)zone;

  /* Deallocating the mutex first assures us that no other thread will
     be doing anything at the time we recycle the zone.  Though
     another thread using the zone after it's been recycled is another
     matter. :) */
  objc_mutex_deallocate(zptr->mutex);

  NSDeregisterZone(zone);

  /* Go through the block list and return them to the default zone. */
  cur_block = zptr->block_list;
  while (cur_block != NULL)
    {
      next_block = cur_block->next;
      NSZoneFree(NSDefaultMallocZone(), cur_block);
      cur_block = next_block;
    }

  /* Free the zone name if it has been set. */
  if (zone->name != nil)
    [(zone->name) release];

  /* Free the data structure holding the zone. */
  NSZoneFree(NSDefaultMallocZone(), zone);
}

BOOL
custom_check (NSZone *zone)
{
  [NSException raise: NSGenericException format: @"Not implemented"];
  return NO; // Won't be reached.
}

struct NSZoneStats
custom_stats (NSZone *zone)
{
  struct NSZoneStats dummy;

  [NSException raise: NSGenericException format: @"Not implemented"];
  return dummy; // Won't be reached.
}

NSZone*
CreateCustomZone (size_t start, size_t gran)
{
  free_chunk *chunk;
  block *new_block;
  custom_zone *zone;

  /* Round up for alignment. */
  if (start%sizeof(double) > 0)
    start = (start/sizeof(double)+1)*sizeof(double);
  if (gran%sizeof(double) > 0)
    gran = (gran/sizeof(double)+1)*sizeof(double);

  zone = NSZoneMalloc(NSDefaultMallocZone(), sizeof(custom_zone));
  zone->common.malloc = custom_malloc;
  zone->common.realloc = custom_realloc;
  zone->common.free = custom_free;
  zone->common.recycle = custom_recycle;
  zone->common.check = custom_check;
  zone->common.stats = custom_stats;
  zone->common.gran = gran;
  zone->common.name = nil; // We could set a default name, instead.
  zone->mutex = objc_mutex_allocate();

  /* Catch exceptions to prevent memory leaks. */
  NS_DURING
    new_block = NSZoneMalloc(NSDefaultMallocZone(), start);
  NS_HANDLER
    objc_mutex_deallocate(zone->mutex);
    NSZoneFree(NSDefaultMallocZone(), zone);
    [localException raise];
  NS_ENDHANDLER

  new_block->next = NULL;
  chunk = (void*)new_block+sizeof(block);
  chunk->size =
    start-(sizeof(block)+sizeof(size_t)+NSZoneChunkOverhead());
  zone->block_list = new_block;
  zone->free_list = chunk;
  NSZoneRegisterRegion((NSZone*)zone, new_block, (void*)new_block+start);
  return (NSZone*)zone;
}
